#include <netc.h>

#include <stdio.h>

struct nc_functions G_sockfuncs;

int main() {
  nc_error_t err;

#ifdef _USEOPENSSL
  G_sockfuncs = nc_functions_openssl(); 
#else 
  G_sockfuncs = nc_functions_raw();
#endif

  NCSOCKET sock;

  err = G_sockfuncs.socket(&sock, NC_OPT_IPV4, NC_OPT_SOCK_STREAM, NC_OPT_TCP);
  if (err != NC_ERR_GOOD) {
    printf("Error creating socket %s\n", nstrerr(err));
    return 1;
  }

  nc_socketaddr_t sockaddr;
  err = netc_resolve_addrV4(&sockaddr, NC_OPT_NULL, "127.0.0.1", 10000);
  if (err != NC_ERR_GOOD) {
    printf("Error creating socket %s\n", nstrerr(err));
    return 1;
  }

  err = G_sockfuncs.open(&sock, &sockaddr);
  if (err != NC_ERR_GOOD) {
    fprintf(stderr, "Error opening socket %s\n", nstrerr(err));
    return 0;
  }

  char hello[] = "Hello Server, i am a responsible client!";
  size_t _;
  G_sockfuncs.write(&sock, hello, sizeof(hello), &_, NC_OPT_DO_ALL);

  char srvmsg[1024];
  G_sockfuncs.read(&sock, srvmsg, sizeof(srvmsg), &_, NC_OPT_NULL);
  srvmsg[_] = '\0';
  printf("server sent = %s\n", srvmsg);

  G_sockfuncs.close(&sock);

  return 0;
}