#define NC_IMPLEMENTATION
#include <netc.h>

#include <stdio.h>

int main() {
  nc_error_t err;

  NC_RAW_SOCKET sock;
  nc_option_t info = NC_OPT_IPV4 | NC_OPT_TCP; 
  nc_address_t addr = {.af_inet = {
    .port=8080,
    .address="127.0.0.1"
  }};

  err = nraw_open(&sock, info, &addr);
  if (err != NC_ERR_GOOD) {
    fprintf(stderr, "Error opening socket %s\n", nraw_strerr(err));
    return 0;
  }

  nraw_close(sock);
  return 1;
}