#include <netc.h>

#include <stdio.h>
#include <stdbool.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

int main() {
  nc_error_t err;

  nc_raw_socket_t sock;
  sock.port = 80;
  sock.domain = NC_OPT_IPV4;
  sock.type = NC_OPT_SOCK_STREAM;
  sock.protocol = NC_OPT_TCP;

  err = nraw_resolvehost(&sock, "google.com");
  if (err != NC_ERR_GOOD) {
    printf("Error resolving host %s\n", nstrerr(err));
    return 1;
  }

  printf("port - %hu\n", sock.port);
  printf("%d: %d %d %d\n", true, sock.domain == NC_OPT_IPV4, sock.type == NC_OPT_SOCK_STREAM, sock.protocol == NC_OPT_TCP);

  err = nraw_socket(&sock);
  if (err != NC_ERR_GOOD) {
    printf("Error creating socket %s\n", nstrerr(err));
    return 1;
  }

  err = nraw_open(&sock, NULL);
  if (err != NC_ERR_GOOD) {
    fprintf(stderr, "Error opening socket %s\n", nstrerr(err));
    return 0;
  }

  printf("Press Enter To Continue: ");
  fgetc(stdin);

  nraw_close(&sock);
  return 0;
}