#include <netc.h>

#include <stdio.h>

int main() {
  nc_error_t err;

  nc_raw_socket_t backend_sock;
  backend_sock.port = 10000;
  backend_sock.domain = NC_OPT_IPV4;
  backend_sock.type = NC_OPT_SOCK_STREAM;
  backend_sock.protocol = NC_OPT_TCP;

  nc_socket_t sock;
  sock.sock = &backend_sock;

  err = nraw_sockwrap(&sock);
  if (err != NC_ERR_GOOD) {
    printf("Error creating socket %s\n", nstrerr(err));
    return 1;
  }

  err = nopen(&sock, "127.0.0.1");
  if (err != NC_ERR_GOOD) {
    printf("Error opening socket %s\n", nstrerr(err));
    return 1;
  }

  ns_timeval_t ts = {
    .sec = 3,
    .usec = 0
  };
  err = nsetopt(&sock, NC_OPT_RECV_TIMEOUT, &ts, sizeof(ts));
  if (err != NC_ERR_GOOD) {
    printf("Error setting option on socket %s\n", nstrerr(err));
    return 1;
  }

  size_t bytes_sent;
  char req[] = "Hello, World!";
  err = nwrite(&sock, req, sizeof(req), &bytes_sent, NC_OPT_NULL);
  if (err != NC_ERR_GOOD) {
    printf("Error reading from socket %s\n", nstrerr(err));
    return 1;
  }

  size_t bytes_recv;
  char buffer[256];
  err = nread(&sock, buffer, sizeof(buffer), &bytes_recv, NC_OPT_NULL);
  if (err != NC_ERR_GOOD) {
    printf("Error reading from socket %s\n", nstrerr(err));
    return 1;
  }
  buffer[bytes_recv] = '\0';
  printf("%s\n", buffer);

  nclose(&sock);
  return 0;
}