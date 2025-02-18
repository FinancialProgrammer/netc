#include <netc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/resource.h> // For resource limits
#include <unistd.h>       // For close function

#define MAX_CLIENTS 64

struct nc_functions G_sockfuncs;

#define CHECK_ERROR(err, msg, ret) \
  if ((err) != NC_ERR_GOOD) { \
    fprintf(stderr, "NETC Error msg %s with code %d and translation %s\n", msg, err, nstrerr((err))); \
    ret \
  }

#define CLIENT_BUFFER_SIZE 1024

struct header_polldata {
  int fd;
};
typedef struct header_polldata header_polldata_t;
struct server_polldata {
  int fd;
  void *sock;
};
typedef struct server_polldata server_polldata_t;
struct client_polldata {
  int fd;
  NCSOCKET sock;
  size_t bufsize;
  size_t bufindex;
  size_t sendbufsize;
  size_t sendbufindex;
  char buf[CLIENT_BUFFER_SIZE];
  char sendbuf[CLIENT_BUFFER_SIZE];
};
typedef struct client_polldata client_polldata_t;


int server_accept_client(nc_sockpoll_t *poll, header_polldata_t *server_header_data) {
  nc_error_t err;
  server_polldata_t *sdata = (server_polldata_t*)server_header_data;
  int sfd = sdata->fd;
  void *ssock = sdata->sock;

  // create client data
  client_polldata_t *cdata = (client_polldata_t*)malloc(sizeof(client_polldata_t));
  if (cdata == NULL) {
    fprintf(stderr, "Buy more RAM\n");
    return 0;
  }
  cdata->bufsize = CLIENT_BUFFER_SIZE; cdata->sendbufsize = CLIENT_BUFFER_SIZE;
  cdata->bufindex = 0; cdata->sendbufindex = 0;
  cdata->fd = -1;

  // accept client
  err = G_sockfuncs.accept(ssock, &cdata->sock, NULL);
  if (err == NC_ERR_WOULD_BLOCK) {
    free(cdata);
    return 0;
  }
  CHECK_ERROR(err, "server_accept_client-accept", return 0;)

  // get fd
  cdata->fd = ((nc_socket_t*)&cdata->sock)->fd;
  
  // set to NC_OPT_NON_BLOCKING mode as we are using edge triggered
  err = G_sockfuncs.setopt(&cdata->sock, NC_OPT_NON_BLOCKING, NULL, 0);
  CHECK_ERROR(err, "server_accept_client-setopt NC_OPT_NON_BLOCKING" , free(cdata); return 0;)

  // add the fd to the epoll instance
  // *note*: edge triggered is automatic
  nc_option_t opt = NC_OPT_POLLADD | NC_OPT_POLLCLIENT | NC_OPT_POLLIN;
  err = G_sockfuncs.poll_ctl(poll, cdata->fd, cdata, opt);
  CHECK_ERROR(err, "server_accept_client-poll_ctl add" , free(cdata); return 0;)

  return 1;
}

void client_recv(nc_sockpoll_t *poll, header_polldata_t *client_header_data) {
  client_polldata_t *data = (client_polldata_t*)client_header_data;

  if (data->bufsize - data->bufindex == 0)  { return; }

  // read into available buffer space
  size_t bytesread = 0;
  nc_error_t err = G_sockfuncs.read(&data->sock, data->buf + data->bufindex, data->bufsize - data->bufindex, &bytesread, NC_OPT_NONBLOCK);
  CHECK_ERROR(err, "client_recv-read", return;);
  if (!bytesread) return;
  data->bufindex += bytesread;

  // echo data (update sendbuf and buffer)
  size_t sendbufleftover = data->sendbufsize - data->sendbufindex;
  size_t size = data->bufindex > sendbufleftover ? sendbufleftover : data->bufindex;
  if (size) {
    memcpy(data->sendbuf + data->sendbufindex, data->buf, size);
    data->bufindex -= size;
    memmove(data->buf, data->buf + size, data->bufindex);
    data->sendbufindex += size;
  }

  data->sendbuf[data->sendbufindex] = '\0';
  
  // arm the send procedure
  nc_option_t opt = NC_OPT_POLLMOD | NC_OPT_POLLCLIENT | NC_OPT_POLLIN | NC_OPT_POLLOUT;
  err = G_sockfuncs.poll_ctl(poll, data->fd, data, opt);
  CHECK_ERROR(err, "server_accept_client-poll_ctl add", return;);
}

void client_send(nc_sockpoll_t *poll, header_polldata_t *client_header_data) {
  client_polldata_t *data = (client_polldata_t*)client_header_data;

  // Write syscall with zero is useless
  if (data->sendbufindex == 0) { return; }

  // send all bytes possible
  size_t byteswritten = 0;
  nc_error_t err = G_sockfuncs.write(&data->sock, data->sendbuf, data->sendbufindex, &byteswritten, NC_OPT_NONBLOCK);
  CHECK_ERROR(err, "client_send-write", return;);

  // update sendbuf
  if (byteswritten) {
    data->sendbufindex -= byteswritten;
    memmove(data->sendbuf, data->sendbuf + byteswritten, data->sendbufindex);
  }

  // check if sendbufindex still has some leftover and rearm POLLOUT
  if (data->sendbufindex > 0) {
    nc_option_t opt = NC_OPT_POLLMOD | NC_OPT_POLLCLIENT | NC_OPT_POLLIN | NC_OPT_POLLOUT;
    err = G_sockfuncs.poll_ctl(poll, data->fd, data, opt);
    CHECK_ERROR(err, "client_send-poll_ctl rearm POLLOUT", return;);
  }
}

void client_close(nc_sockpoll_t *poll, header_polldata_t *client_header_data, nc_error_t err, const char *reason) {
  client_polldata_t *data = (client_polldata_t*)client_header_data;

  if (err != NC_ERR_GOOD) {
    fprintf(stderr, "Client closed for %s\n", reason);
  }

  G_sockfuncs.poll_ctl(poll, data->fd, NULL, NC_OPT_POLLDLT);
  G_sockfuncs.close(&data->sock);
  free(data);
}

sig_atomic_t G_exit = 0;
void SigIntHandler(int sig) {
  G_exit = 1;
}

int main() {
  signal(SIGINT, SigIntHandler);
  nc_error_t err;

#ifdef _USEOPENSSL
  G_sockfuncs = nc_functions_openssl();
#else
  G_sockfuncs = nc_functions_raw();
#endif

  // Server Socket
  NCSOCKET sock;
  err = G_sockfuncs.socket(&sock, NC_OPT_IPV4, NC_OPT_SOCK_STREAM, NC_OPT_TCP);
  CHECK_ERROR(err, "socket", return 1;)

  int opt = 1;
  err = G_sockfuncs.setopt(&sock, NC_OPT_REUSEADDR, &opt, sizeof(opt));
  CHECK_ERROR(err, "setopt NC_OPT_REUSEADDR", return 1;)
  err = G_sockfuncs.setopt(&sock, NC_OPT_NON_BLOCKING, NULL, 0);
  CHECK_ERROR(err, "setopt NC_OPT_NON_BLOCKING" , return 1;)

#ifdef _USEOPENSSL 
  err = G_sockfuncs.setopt(&sock, NC_OPT_CERT_FILE, "./server/certs/server.crt", 0);
  CHECK_ERROR(err, "setopt NC_OPT_CERT_FILE" , return 1;)
  err = G_sockfuncs.setopt(&sock, NC_OPT_PRIV_KEY_FILE, "./server/certs/server.key", 0);
  CHECK_ERROR(err, "setopt NC_OPT_PRIV_KEY_FILE" , return 1;)
#endif

  struct nc_socketaddr sockaddr;
  err = netc_resolve_addrV4(&sockaddr, NC_OPT_INADDR_ANY, "127.0.0.1", 10000);
  CHECK_ERROR(err, "resolve addrV4", return 1;)

  err = G_sockfuncs.bind(&sock, &sockaddr);
  CHECK_ERROR(err, "bind", return 1;)
  err = G_sockfuncs.listen(&sock, MAX_CLIENTS);
  CHECK_ERROR(err, "listen", return 1;)

  // Polling Structure
  nc_sockpoll_t poll;
  err = G_sockfuncs.poll_create(&poll);
  CHECK_ERROR(err, "poll_create", return 1;)

  nc_option_t pollopt = NC_OPT_POLLADD | NC_OPT_POLLIN;
  server_polldata_t server_data;
  server_data.fd = sock.fd;
  server_data.sock = (void*)&sock;
  err = G_sockfuncs.poll_ctl(&poll, sock.fd, &server_data, pollopt);
  CHECK_ERROR(err, "poll_ctl add", return 1;)

  // Polling Events
  nc_sockpoll_event_t events[MAX_CLIENTS];

  while (!G_exit) {
    int nfds = 0;
    err = G_sockfuncs.poll_wait(&poll, events, MAX_CLIENTS, 3000, &nfds);
    CHECK_ERROR(err, "poll_wait", break;)

    for (int i = 0; i < nfds; ++i) {
      header_polldata_t *header = (header_polldata_t*)events[i].data.ptr;
      int fd = header->fd;
      if (fd == sock.fd) {
        while (server_accept_client(&poll, header));
        continue;
      }
      if (events[i].events & EPOLLIN) {
        client_recv(&poll, header);
      } 
      if (events[i].events & EPOLLOUT) {
        client_send(&poll, header);
      }
      if (events[i].events & EPOLLRDHUP) { // graceful
        client_close(&poll, header, NC_ERR_GOOD, "RDHUP");
      }
      if (events[i].events & EPOLLHUP) { // not graceful
        client_close(&poll, header, NC_ERR_NULL, "HUP");
      }
    }
  }

  // Cleanup
  err = G_sockfuncs.poll_ctl(&poll, sock.fd, NULL, NC_OPT_POLLDLT);
  CHECK_ERROR(err, "poll_ctl delete", return 1;)
  G_sockfuncs.close(&sock); // close server socket
  return 0;
}
