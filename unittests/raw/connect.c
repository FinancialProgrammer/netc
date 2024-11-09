#include <netc.h>

#include <stdio.h>

int main() {
  nc_error_t err;

  nc_raw_socket_t sock;
  sock.port = 10000;
  sock.domain = NC_OPT_IPV4;
  sock.type = NC_OPT_SOCK_STREAM;
  sock.protocol = NC_OPT_TCP;

  err = nraw_socket(&sock);
  if (err != NC_ERR_GOOD) {
    printf("Error creating socket %s\n", nstrerr(err));
    return 1;
  }

  err = nraw_open(&sock, "127.0.0.1");
  if (err != NC_ERR_GOOD) {
    fprintf(stderr, "Error opening socket %s\n", nstrerr(err));
    return 0;
  }

  printf("Press Enter To Continue: ");
  fgetc(stdin);

  nraw_close(&sock);
  return 0;
}