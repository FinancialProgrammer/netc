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

  int opt = 1;
  err = G_sockfuncs.setopt(&sock, NC_OPT_REUSEADDR, &opt, sizeof(opt));
  err = G_sockfuncs.setopt(&sock, NC_OPT_CERT_FILE, "./server/certs/server.crt", 0);
  err = G_sockfuncs.setopt(&sock, NC_OPT_PRIV_KEY_FILE, "./server/certs/server.key", 0);

  struct nc_socketaddr sockaddr;

  err = netc_resolve_addrV4(&sockaddr, NC_OPT_INADDR_ANY, "127.0.0.1", 10000); // 127.0.0.1 is a fallback if INADDR_ANY is not implemented on target machine
  if (err != NC_ERR_GOOD) {
    printf("Error creating socket %s\n", nstrerr(err));
    return 1;
  }

  err = G_sockfuncs.bind(&sock, &sockaddr);
  if (err != NC_ERR_GOOD) {
    fprintf(stderr, "Error opening socket %s\n", nstrerr(err));
    return 0;
  }

  err = G_sockfuncs.listen(&sock, 3);
  if (err != NC_ERR_GOOD) {
    fprintf(stderr, "Error opening socket %s\n", nstrerr(err));
    return 0;
  }

  NCSOCKET client;
  struct nc_socketaddr clientaddr;
  err = G_sockfuncs.accept(&sock, &client, &clientaddr);

  size_t _;
  char srvmsg[1024];
  err = G_sockfuncs.read(&client, srvmsg, sizeof(srvmsg), &_, NC_OPT_NULL);
  if (err != NC_ERR_GOOD) {
    fprintf(stderr, "Error opening socket %s\n", nstrerr(err));
    return 0;
  }

  srvmsg[_] = '\0'; // safety first
  printf("client sent = %s", srvmsg);

  err = G_sockfuncs.write(&client, srvmsg, _, &_, NC_OPT_NULL);
  if (err != NC_ERR_GOOD) {
    fprintf(stderr, "Error opening socket %s\n", nstrerr(err));
    return 0;
  }

  G_sockfuncs.close(&sock);
  
  return 0;
}