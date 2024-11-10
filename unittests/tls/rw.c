#define NC_TLS
#include <netc.h>
#include <stdio.h>

int main() {
  nc_error_t err;

  nc_tls_socket_t sock;
  sock.port = 8443;
  sock.domain = NC_OPT_IPV4;
  sock.type = NC_OPT_SOCK_STREAM;
  sock.protocol = NC_OPT_TCP;

  err = ntls_socket(&sock);
  if (err != NC_ERR_GOOD) {
    printf("Error creating socket %s\n", nstrerr(err));
    return 1;
  }

  err = ntls_open(&sock, "127.0.0.1");
  if (err != NC_ERR_GOOD) {
    fprintf(stderr, "Error opening socket %s\n", nstrerr(err));
    return 0;
  }

  ns_timeval_t ts = {
    .sec = 3,
    .usec = 0
  };
  err = ntls_setopt(&sock, NC_OPT_RECV_TIMEOUT, &ts, sizeof(ts));
  if (err != NC_ERR_GOOD) {
    printf("Error setting option on socket %s\n", nstrerr(err));
    return 1;
  }

  size_t bytes_sent;
  char req[] = "Hello, World!";
  err = ntls_write(&sock, req, sizeof(req), &bytes_sent, NC_OPT_NULL);
  if (err != NC_ERR_GOOD) {
    printf("Error reading from socket %s\n", nstrerr(err));
    return 1;
  }

  size_t bytes_recv;
  char buffer[256];
  err = ntls_read(&sock, buffer, sizeof(buffer), &bytes_recv, NC_OPT_NULL);
  if (err != NC_ERR_GOOD) {
    printf("Error reading from socket %s\n", nstrerr(err));
    return 1;
  }
  buffer[bytes_recv] = '\0';
  printf("%s\n", buffer);

  ntls_close(&sock);

  return 0;
}