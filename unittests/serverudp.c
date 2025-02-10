#include <netc.h>

#include <stdio.h>
#include <string.h>

struct nc_functions G_sockfuncs;

int main() {
  nc_error_t err;

#ifdef _USEOPENSSL
  G_sockfuncs = nc_functions_openssl(); 
#else 
  G_sockfuncs = nc_functions_raw();
#endif

  NCSOCKET sock;
  char buffer[1024];
  struct nc_socketaddr servaddr, cliaddr;

  err = G_sockfuncs.socket(&sock, NC_OPT_IPV4, NC_OPT_DGRAM, NC_OPT_UDP);
  if (err != NC_ERR_GOOD) {
    printf("Error creating socket %s\n", nstrerr(err));
    return 1;
  }

  int opt = 1;
  err = G_sockfuncs.setopt(&sock, NC_OPT_REUSEADDR, &opt, sizeof(opt));

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));
    
  err = netc_resolve_addrV4(&servaddr, NC_OPT_INADDR_ANY, "127.0.0.1", 10000); // 127.0.0.1 is a fallback if INADDR_ANY is not implemented on target machine
  if (err != NC_ERR_GOOD) {
    printf("Error creating socket %s\n", nstrerr(err));
    return 1;
  }

  err = G_sockfuncs.bind(&sock, &servaddr);
  if (err != NC_ERR_GOOD) {
    fprintf(stderr, "Error opening socket %s\n", nstrerr(err));
    return 0;
  }
    
  size_t recieved, sent;
  
  err = G_sockfuncs.readfrom(&sock, &cliaddr, buffer, 1024, &recieved, NC_OPT_NULL);
  if (err != NC_ERR_GOOD) {
    fprintf(stderr, "Error opening socket %s\n", nstrerr(err));
    return 0;
  }
  buffer[recieved] = '\0';
  printf("Client: %s\n", buffer);

  err = G_sockfuncs.writeto(&sock, &cliaddr, buffer, recieved, &sent, NC_OPT_NULL);
  if (err != NC_ERR_GOOD) {
    fprintf(stderr, "Error opening socket %s\n", nstrerr(err));
    return 0;
  }

  G_sockfuncs.close(&sock);
  return 0;
}
