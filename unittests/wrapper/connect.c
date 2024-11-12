#include <netc.h>

#include <stdio.h>

int main() {
  nc_error_t err;

  nc_raw_socket_t backend_sock;
  sock.__internal_addr = NULL; // needed for internal use
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
    fprintf(stderr, "Error opening socket %s\n", nstrerr(err));
    return 0;
  }

  printf("Press Enter To Continue: ");
  fgetc(stdin);

  nclose(&sock);
  return 0;
}