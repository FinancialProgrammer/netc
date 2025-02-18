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
  char message[] = "Hello Server i am a respectful UDP client";
  nc_socketaddr_t servaddr;
    
  err = G_sockfuncs.socket(&sock, NC_OPT_IPV4, NC_OPT_DGRAM, NC_OPT_UDP);
  if (err != NC_ERR_GOOD) {
    printf("Error creating socket %s\n", nstrerr(err));
    return 1;
  }

  memset(&servaddr, 0, sizeof(servaddr));
  err = netc_resolve_ipV4(&servaddr, "127.0.0.1", 10000);
  if (err != NC_ERR_GOOD) {
    printf("Error creating socket %s\n", nstrerr(err));
    return 1;
  }
    
  size_t recieved, sent;

  err = G_sockfuncs.writeto(&sock, &servaddr, message, sizeof(message), &sent, NC_OPT_NULL);
  if (err != NC_ERR_GOOD) {
    fprintf(stderr, "Error opening socket %s\n", nstrerr(err));
    return 0;
  }

  err = G_sockfuncs.readfrom(&sock, &servaddr, buffer, 1024, &recieved, NC_OPT_NULL);
  if (err != NC_ERR_GOOD) {
    fprintf(stderr, "Error opening socket %s\n", nstrerr(err));
    return 0;
  }
  buffer[recieved] = '\0';
  printf("Server: %s\n", buffer);
  
  G_sockfuncs.close(&sock);
  return 0;
}
