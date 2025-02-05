#ifndef __NETC_INCLUDED
#define __NETC_INCLUDED

#ifdef __cplusplus
  extern "C" {
#endif

#include <stddef.h> // size_t
#include <stdint.h> // int*_t uint*_t
#include <time.h>   // time_t (in nc_timeval_t)
#include <signal.h> // sig_atomic_t
// #include <string.h>

// --- STATIC --- //
// Types
  typedef int nc_error_t;
  typedef int nc_option_t;
  typedef int nc_opt_bool_t;
  struct nc_timeval_t {
    time_t sec; // seconds
    time_t usec; // micro seconds
  };
// ERRORS
  #define NC_ERR_GOOD                    ((nc_error_t)0)
  #define NC_ERR_NULL                    ((nc_error_t)1)
  #define NC_ERR_MEMORY                  ((nc_error_t)2)
  #define NC_ERR_INVALID_ARGUMENT        ((nc_error_t)3)
  #define NC_ERR_INVALID_ADDRESS         ((nc_error_t)4)
  #define NC_ERR_CONNECTION_REFUSED      ((nc_error_t)5)
  #define NC_ERR_NOT_INITED              ((nc_error_t)6)
  #define NC_ERR_TIMED_OUT               ((nc_error_t)7)
  #define NC_ERR_NOT_IMPLEMENTED         ((nc_error_t)8)
  #define NC_ERR_NOT_CONNECTED           ((nc_error_t)9)
  #define NC_ERR_ILL_FORMED_MESSAGE      ((nc_error_t)10)
  #define NC_ERR_SOCKET_CLOSED           ((nc_error_t)11)
  #define NC_ERR_WOULD_BLOCK             ((nc_error_t)12)
  #define NC_ERR_SET_OPT_FAIL            ((nc_error_t)13)
  #define NC_ERR_INVL_CTX                ((nc_error_t)14)
  #define NC_ERR_BAD_HANDSHAKE           ((nc_error_t)15)
  #define NC_ERR_EXIT_FLAG               ((nc_error_t)16)
  #define NC_ERR_ACCEPTED_ADDR_TOO_LARGE ((nc_error_t)17)
  #define NC_ERR_NO_DATA_AVAILABLE       ((nc_error_t)18)
  #define NC_ERR_CERT_FILE               ((nc_error_t)19)
  #define NC_ERR_KEY_FILE                ((nc_error_t)20)
  #define NC_ERR_INVL_KEY                ((nc_error_t)21)
// OPTIONS
  #define NC_OPT_NULL         ((nc_option_t)0)
  // socket 
  #define NC_OPT_IPV4         ((nc_option_t)1)
  #define NC_OPT_IPV6         ((nc_option_t)2)
  #define NC_OPT_SOCK_STREAM  ((nc_option_t)3)
  #define NC_OPT_DGRAM        ((nc_option_t)4)
  #define NC_OPT_TCP          ((nc_option_t)5)
  #define NC_OPT_UDP          ((nc_option_t)6)
  // setopt / getopt
  #define NC_OPT_RECV_TIMEOUT  ((nc_option_t)100)
  #define NC_OPT_SEND_TIMEOUT  ((nc_option_t)101)
  #define NC_OPT_EXIT_FLAG     ((nc_option_t)102)
  #define NC_OPT_REUSEADDR     ((nc_option_t)103) // sets SO_REUSEADDR | SO_REUSEPORT
  #define NC_OPT_CERT_FILE     ((nc_option_t)104)
  #define NC_OPT_PRIV_KEY_FILE ((nc_option_t)105)
  // parameters
  #define NC_OPT_DO_ALL   ((nc_option_t)200)
  #define NC_OPT_MSG_PEAK ((nc_option_t)201)
  #define NC_OPT_POLLIN   ((nc_option_t)201)
// --- END(STATIC) --- //

/* TODO (Windows OS)
* since windows can't keep a standard and SOCKET could change at any time,
* it is probably best to include the windows api if windows is detected.
* Kind of annoying but an accurate description for windows programmers.
* When i actually start implementing windows i will worry about this.
*/

// --- Socket Structures --- //
  struct nc_socketaddr; // forward declare
  size_t netc_sizeof_socketaddr();
  #ifdef __NETC_EXPOSED_SOCKETADDR
    #ifdef _WIN32
      #include <winsock2.h>
      #include <ws2tcpip.h>
    #else
      #include <sys/types.h>
      #include <sys/socket.h>
      #include <netinet/in.h>
      #include <arpa/inet.h>
      #include <netdb.h>
    #endif
    struct nc_socketaddr {
      struct sockaddr_storage __internal_addr;
      size_t __internal_addrlen;
    };
  #endif
  #define NCSOCKET struct nc_openssl_socket // with more sockets this will change to some char[64]
  struct nc_openssl_socket {
    int fd;
    void *ssl;
    void *ctx;
  };
  struct nc_socket {
    int fd;
  };
  struct nc_sockpoll {
    int   fd;         /* file descriptor */
    short events;     /* requested events */
    short revents;    /* returned events */
  };
  typedef struct nc_socket nc_socket_t;
// --- END(Socket Structures) --- //

// --- Socket Functionality --- //
  struct nc_functions {
    nc_error_t (*socket)(void*, int domain, int type, int protocol);
    nc_error_t (*close)(void*);

    nc_error_t (*open)(void*, struct nc_socketaddr*);
    nc_error_t (*bind)(void*, struct nc_socketaddr*);

    nc_error_t (*write)(void*, const void*, size_t, size_t*, nc_option_t);
    nc_error_t (*read)(void*, void*, size_t, size_t*, nc_option_t);

    nc_error_t (*listen)(void*, int);
    nc_error_t (*accept)(void*, void*, struct nc_socketaddr*);

    nc_error_t (*poll)(struct nc_sockpoll *polled, size_t polled_len, int timeout, nc_option_t param);

    nc_error_t (*setopt)(void*, nc_option_t, const void*, size_t);
    nc_error_t (*getopt)(void*, nc_option_t, void*, size_t); // overwrites null_data
  };
  struct nc_functions nc_functions_raw();
  struct nc_functions nc_functions_openssl();
// --- END(Socket Functionality) --- //

// --- Netowrking Auxiliary --- //
  nc_error_t netc_resolve_addrV6(struct nc_socketaddr *addr, const char *ipaddr, uint16_t port, uint32_t flowinfo, uint32_t scope_id);
  nc_error_t netc_resolve_ipV6(struct nc_socketaddr *ncsockaddr, const char *ipaddr, uint16_t port, uint32_t flowinfo, uint32_t scope_id);
  nc_error_t netc_resolve_addrV4(struct nc_socketaddr *addr, const char *ipaddr, uint16_t port);
  nc_error_t netc_resolve_ipV4(struct nc_socketaddr *ncsockaddr, const char *ipaddr, uint16_t port);
// --- END(Netowrking Auxiliary) --- // 

// --- Netc Auxiliary --- //
  const char *nstrerr(nc_error_t); // return stringed error
// --- END(Netc Auxiliary) --- //

#ifdef __cplusplus
  }
#endif

#endif // __NETC_INCLUDED