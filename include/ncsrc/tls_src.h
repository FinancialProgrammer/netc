#ifndef __NETC_TLS_INCLUDED
#define __NETC_TLS_INCLUDED

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct {
  int fd;

  int domain;   // ipv6 / ipv4 / (url comming soon?...)
  int type;     // stream (tcp) / dgram (udp)
  int protocol; // ip protocol tcp / ip protocol udp (maybe both arn't needed) (can bet set to 0 for automatic)

  uint16_t port;
  uint32_t flowinfo;
  uint64_t addr;
  uint32_t scope_id;

  struct sockaddr *__internal_addr;
  size_t __internal_addrlen;

  nc_error_t last_error;

  void *ssl; // only needed for client and clients from accept
  void *ctx; // needed for both server and clients but not the client from accept
} nc_tls_socket_t;

// socket creation
nc_error_t ntls_sockwrap(nc_socket_t *sock);
nc_error_t ntls_socket(void *void_sock);
nc_error_t ntls_close(void *void_sock);

// connection
nc_error_t ntls_open(void *void_sock, const char *ipaddr);

// functionality
nc_error_t ntls_write(void *void_sock, const void *buf, size_t buf_size, size_t *bytes_read, nc_option_t param);
nc_error_t ntls_read(void *void_sock, void *buf, size_t buf_size, size_t *bytes_read, nc_option_t param);

// option
nc_error_t ntls_setopt(void *void_sock, nc_option_t option, void *data, size_t data_size);
nc_error_t ntls_getopt(void *void_sock, nc_option_t option, void *null_data, size_t data_size); // overwrites null_data

#ifdef __cplusplus
  }
#endif

#endif