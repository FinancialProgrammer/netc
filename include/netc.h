#ifndef __NETC_INCLUDED
#define __NETC_INCLUDED

#ifdef __cplusplus
  extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <signal.h> // sig_atomic_t

// STATIC
// Types
  typedef int nc_error_t;
  typedef int nc_option_t;
// ERRORS
  #define NC_ERR_GOOD ((nc_error_t)0)
  #define NC_ERR_NULL ((nc_error_t)1)
  #define NC_ERR_MEMORY ((nc_error_t)2)
  #define NC_ERR_INVALID_ARGUMENT ((nc_error_t)3)
  #define NC_ERR_INVALID_ADDRESS ((nc_error_t)4)
  #define NC_ERR_CONNECTION_REFUSED ((nc_error_t)5)
  #define NC_ERR_NOT_INITED ((nc_error_t)6)
  #define NC_ERR_TIMED_OUT ((nc_error_t)7)
  #define NC_ERR_NOT_IMPLEMENTED ((nc_error_t)8)
  #define NC_ERR_NOT_CONNECTED ((nc_error_t)9)
  #define NC_ERR_ILL_FORMED_MESSAGE ((nc_error_t)10)
  #define NC_ERR_SOCKET_CLOSED ((nc_error_t)11)
  #define NC_ERR_WOULD_BLOCK ((nc_error_t)12)
  #define NC_ERR_SET_OPT_FAIL ((nc_error_t)13)
  #define NC_ERR_INVL_CTX ((nc_error_t)14)
  #define NC_ERR_BAD_HANDSHAKE ((nc_error_t)15)
  #define NC_ERR_EXIT_FLAG ((nc_error_t)16)
  const char *nstrerr(nc_error_t); // return stringed error
// OPTIONS
  #define NC_OPT_NULL         ((nc_option_t)0)
  #define NC_OPT_IPV4         ((nc_option_t)1)
  #define NC_OPT_IPV6         ((nc_option_t)2)
  #define NC_OPT_HOSTNAME     ((nc_option_t)3)
  #define NC_OPT_TCP          ((nc_option_t)4)
  #define NC_OPT_UDP          ((nc_option_t)5)
  #define NC_OPT_SOCK_STREAM  ((nc_option_t)6)
  #define NC_OPT_DGRAM        ((nc_option_t)7)
  #define NC_OPT_RECV_TIMEOUT ((nc_option_t)8)
  #define NC_OPT_SEND_TIMEOUT ((nc_option_t)9)
    typedef struct {
      time_t sec; // seconds
      time_t usec; // micro seconds
    } ns_timeval_t;
  #define NC_OPT_EXIT_FLAG ((nc_option_t)10)
    typedef sig_atomic_t nc_exit_flag_t;
    #define NC_EXIT 1
    #define NC_NO_EXIT 0
  #define NC_OPT_DO_ALL ((nc_option_t)11)
// ------------ wrapper api ------------ // 
#define NCSOCKET void
struct nc_socket_t {
  nc_error_t (*open)(void*, char const*);
  nc_error_t (*close)(void*);
  nc_error_t (*write)(void*, const void *, size_t, size_t*, nc_option_t);
  nc_error_t (*read)(void*, void *, size_t, size_t*, nc_option_t);
  nc_error_t (*setopt)(void*, nc_option_t, void*, size_t);
  nc_error_t (*getopt)(void*, nc_option_t, void*, size_t);
  void *sock;
};
typedef struct nc_socket_t nc_socket_t;

// deletion
nc_error_t nclose(nc_socket_t *sock);

// connect
nc_error_t nopen(nc_socket_t *sock, const char *ip);

// functionality
nc_error_t nwrite(nc_socket_t *sock, const void *buf, size_t bufsize, size_t *bytes_written, nc_option_t opt);
nc_error_t nread(nc_socket_t *sock, void *buf, size_t bufsize, size_t *bytes_read, nc_option_t opt);

// options
nc_error_t nsetopt(nc_socket_t *sock, nc_option_t opt, void *data, size_t data_size);
nc_error_t ngetopt(nc_socket_t *sock, nc_option_t opt, void *data, size_t data_size);

#ifdef __cplusplus
  }
#endif

#endif // __NETC_INCLUDED

// ------------ backend api ------------ // 
// --- RAW --- //
#include <ncsrc/raw_src.h>

// --- TLS/SSL --- //
#ifdef NC_TLS
  #include <ncsrc/tls_src.h>
#endif
