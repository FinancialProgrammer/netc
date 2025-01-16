#ifndef __NETC_RAW_INCLUDED
#define __NETC_RAW_INCLUDED

#define NC_INVL_RAW_SOCK -1

#ifdef __cplusplus
  extern "C" {
#endif

struct sockaddr; // forward declare
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
} nc_raw_socket_t;


// socket creation
nc_error_t nraw_sockwrap(nc_socket_t *sock);
nc_error_t nraw_socket(void *sock);
nc_error_t nraw_close(void *sock);

// connection
nc_error_t nraw_open(void *sock, const char *ipaddr);

// functionality
nc_error_t nraw_write(void *sock, const void *buf, size_t buf_size, size_t *bytes_read, nc_option_t param);
nc_error_t nraw_read(void *sock, void *buf, size_t buf_size, size_t *bytes_read, nc_option_t param);

// option
nc_error_t nraw_setopt(void *sock, nc_option_t option, void *data, size_t data_size);
nc_error_t nraw_getopt(void *sock, nc_option_t option, void *null_data, size_t data_size); // overwrites null_data

// auxiliary
nc_error_t nraw_resolvehost(void *sock, const char *ipaddr);

#ifdef __cplusplus
  }
#endif

#endif // __NETC_RAW_INCLUDED