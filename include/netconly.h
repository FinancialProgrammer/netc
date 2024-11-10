#ifndef __NETC_INCLUDED
#define __NETC_INCLUDED

// global include
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

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
  const char *nstrerr(nc_error_t); // return stringed error
// OPTIONS
  #define NC_OPT_NULL         ((nc_option_t)0)
  #define NC_OPT_IPV4         ((nc_option_t)1)
  #define NC_OPT_IPV6         ((nc_option_t)2)
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

#endif // __NETC_INCLUDED

#ifdef NC_WRAPPER_IMPL
  // deletion
  nc_error_t nclose(nc_socket_t *sock) { return sock->close(sock->sock); }

  // connect
  nc_error_t nopen(nc_socket_t *sock, const char *ip) { return sock->open(sock->sock, ip); }

  // functionality
  nc_error_t nwrite(nc_socket_t *sock, const void *buf, size_t bufsize, size_t *bytes_written, nc_option_t opt) {
    return sock->write(sock->sock, buf, bufsize, bytes_written, opt);
  }
  nc_error_t nread(nc_socket_t *sock, void *buf, size_t bufsize, size_t *bytes_read, nc_option_t opt) {
    return sock->read(sock->sock, buf, bufsize, bytes_read, opt);
  }

  // options
  nc_error_t nsetopt(nc_socket_t *sock, nc_option_t opt, void *data, size_t data_size) {
    return sock->setopt(sock->sock, opt, data, data_size);
  }
  nc_error_t ngetopt(nc_socket_t *sock, nc_option_t opt, void *data, size_t data_size) {
    return sock->getopt(sock->sock, opt, data, data_size);
  }
  static const char *nstrerrarr[] = {
    "No error spotted",
    "Couldn't parse error",
    "Invalid Memory",
    "Invalid Argument",
    "Invalid Address",
    "Connection Refused",
    "Not Inited",
    "Timed Out",
    "Feature Not Implemented Yet",
    "Not Connected",
    "Ill Formed Message",
    "Socket Closed",
    "Would Block",
    "Option couldn't be set",
    "Context couldn't be created",
    "Couldn't complete handshake"
  };
  const char *nstrerr(nc_error_t err) { return nstrerrarr[err]; }
  #undef NC_WRAPPER_IMPL
#endif // NC_IMPLEMENTATION

// ------------ backend api ------------ // 
// --- RAW --- //
#if defined(WIN32) || defined(__MINGW32__) || defined(WIN64) || defined(__MINGW64__) // check for winsock (32)
  // NOT IMPLEMENTED YET
#else // else assume posix (posix has no actual way to check)
  // ignore error if operating system is likely to use the posix standard
  #if defined(__linux__) || defined(__sun) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
    (defined(__APPLE__) && defined(__MACH__)) || defined(__MSYS__) || defined(__unix__)
    // do nothing
  #else
    #pragma message("Unknown operating system attempting to use POSIX")
  #endif
  #ifndef __NETC_RAW_INCLUDED
  #define __NETC_RAW_INCLUDED

  #define NC_INVL_RAW_SOCK -1

  typedef struct {
    int fd;

    int domain;   // ipv6 / ipv4 / (url comming soon?...)
    int type;     // stream (tcp) / dgram (udp)
    int protocol; // ip protocol tcp / ip protocol udp (maybe both arn't needed) (can bet set to 0 for automatic)

    uint16_t port;
    uint32_t flowinfo;
    uint64_t addr;
    uint32_t scope_id;

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

  #endif // __NETC_RAW_INCLUDED

  #ifdef NC_RAW_IMPL
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

    nc_error_t __internal_nraw_convert_errno() {
      switch (errno) {
        case EADDRNOTAVAIL: return NC_ERR_INVALID_ADDRESS; // Cannot assign requested address 
        case ECONNREFUSED: return NC_ERR_CONNECTION_REFUSED; // Connection refused 
        case EHOSTUNREACH: return NC_ERR_NOT_INITED; // No route to host 
        case ETIMEDOUT: return NC_ERR_TIMED_OUT; // Connection timed out 
        case ENOSYS: return NC_ERR_NOT_IMPLEMENTED; // Function not implemented 
        case ENOTCONN: return NC_ERR_NOT_CONNECTED; // Transport endpoint is not connected 
        case EBADMSG: return NC_ERR_ILL_FORMED_MESSAGE; // Bad message 
        case ECONNRESET: return NC_ERR_SOCKET_CLOSED; // Connection reset by peer 
        case EWOULDBLOCK: return NC_ERR_WOULD_BLOCK;
        default: return NC_ERR_NULL;
      };
    }

    // connection
    nc_error_t nraw_socket(void *void_sock) {
      nc_raw_socket_t *sock = (nc_raw_socket_t*)void_sock;
      int posix_protocol = 0;
      if (sock->protocol == NC_OPT_TCP) {
        posix_protocol = IPPROTO_TCP;
      } else if (sock->protocol == NC_OPT_UDP) {
        posix_protocol = IPPROTO_UDP;
      }

      sock->fd = socket(
        (sock->domain == NC_OPT_IPV4) ?  AF_INET : AF_INET6,
        (sock->type == NC_OPT_SOCK_STREAM) ? SOCK_STREAM : SOCK_DGRAM,
        posix_protocol
      );
      if (sock->fd == NC_INVL_RAW_SOCK) {
        return __internal_nraw_convert_errno();
      }

      return NC_ERR_GOOD;
    }
    nc_error_t nraw_open(void *void_sock, const char *ipaddr) {
      nc_raw_socket_t *sock = (nc_raw_socket_t*)void_sock;
      if (sock->domain == NC_OPT_IPV6) { 
        struct sockaddr_in6 address;
        memset(&address, 0, sizeof(address)); // Ensure the structure is zeroed out
        address.sin6_family = AF_INET6; 
        address.sin6_port = htons(sock->port);
        address.sin6_flowinfo = htonl(sock->flowinfo); 
        address.sin6_scope_id = htonl(sock->scope_id);

        if (inet_pton(AF_INET6, ipaddr, &address.sin6_addr) != 1) {
          nraw_close(sock);
          return NC_ERR_INVALID_ADDRESS;
        }

        if (connect(sock->fd, (struct sockaddr*)&address, sizeof(address)) == -1) { 
          nraw_close(sock); 
          return __internal_nraw_convert_errno(); 
        } 
      } else if (sock->domain == NC_OPT_IPV4) { 
        struct sockaddr_in address;
        address.sin_family = AF_INET; 
        address.sin_port = htons(sock->port);
        // address.sin_addr.s_addr = inet_addr(ipaddr);

        if (inet_pton(AF_INET, ipaddr, &address.sin_addr.s_addr) != 1) {
          nraw_close(sock);
          return NC_ERR_INVALID_ADDRESS;
        }

        if (connect(sock->fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
          nraw_close(sock);
          return __internal_nraw_convert_errno(); 
        }
      }
      return NC_ERR_GOOD;
    }
    nc_error_t nraw_close(void *void_sock) { 
      nc_raw_socket_t *sock = (nc_raw_socket_t*)void_sock; 
      close(sock->fd); 
      return NC_ERR_GOOD; 
    }

    // write, read
    nc_error_t nraw_write(void *void_sock, const void *buf, size_t buf_size, size_t *bytes_written, nc_option_t param) {
      nc_raw_socket_t *sock = (nc_raw_socket_t*)void_sock;
      ssize_t bwrite = send(sock->fd, buf, buf_size, 0);
      if (bwrite == -1) {
        *bytes_written = 0;
        return __internal_nraw_convert_errno();
      } else {
        *bytes_written = (size_t)bwrite;
        return NC_ERR_GOOD;
      }
    }
    nc_error_t nraw_read(void *void_sock, void *buf, size_t buf_size, size_t *bytes_read, nc_option_t param) {
      nc_raw_socket_t *sock = (nc_raw_socket_t*)void_sock;
      ssize_t bread = recv(sock->fd, buf, buf_size, 0);
      if (bread == -1) {
        *bytes_read = 0;
        return __internal_nraw_convert_errno();
      } else {
        *bytes_read = (size_t)bread;
        return NC_ERR_GOOD;
      }
    }

    // option
    nc_error_t nraw_setopt(void *void_sock, nc_option_t option, void *data, size_t data_size) {
      nc_raw_socket_t *sock = (nc_raw_socket_t*)void_sock;
      switch (option) {
        case NC_OPT_RECV_TIMEOUT: {
          struct timeval timeout;
          timeout.tv_sec = ((ns_timeval_t*)data)->sec;
          timeout.tv_usec = ((ns_timeval_t*)data)->usec;
          if (setsockopt(sock->fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&timeout, sizeof(timeout)) == -1) {
            return NC_ERR_SET_OPT_FAIL;
          }
          return NC_ERR_GOOD;
        } case NC_OPT_SEND_TIMEOUT: {
          struct timeval timeout;
          timeout.tv_sec = ((ns_timeval_t*)data)->sec;
          timeout.tv_usec = ((ns_timeval_t*)data)->usec;
          if (setsockopt(sock->fd, SOL_SOCKET, SO_SNDTIMEO, (struct timeval *)&timeout, sizeof(timeout)) == -1) {
            return NC_ERR_SET_OPT_FAIL;
          }
          return NC_ERR_GOOD;
        } default: return NC_ERR_NULL;
      }
      return NC_ERR_GOOD;
    }
    nc_error_t nraw_getopt(void *void_sock, nc_option_t option, void *null_data, size_t data_size) {
      nc_raw_socket_t *sock = (nc_raw_socket_t*)void_sock;
      switch (option) {
        case NC_OPT_RECV_TIMEOUT: {
          socklen_t timeout_size;
          struct timeval timeout;
          if (getsockopt(sock->fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&timeout, &timeout_size) == -1) {
            return NC_ERR_SET_OPT_FAIL;
          }
          ns_timeval_t *nsts = (ns_timeval_t*)null_data;
          nsts->sec = timeout.tv_sec;
          nsts->usec = timeout.tv_usec;
          return NC_ERR_GOOD;
        } case NC_OPT_SEND_TIMEOUT: {
          socklen_t timeout_size;
          struct timeval timeout;
          if (getsockopt(sock->fd, SOL_SOCKET, SO_SNDTIMEO, (struct timeval *)&timeout, &timeout_size) == -1) {
            return NC_ERR_SET_OPT_FAIL;
          }
          ns_timeval_t *nsts = (ns_timeval_t*)null_data;
          nsts->sec = timeout.tv_sec;
          nsts->usec = timeout.tv_usec;
          return NC_ERR_GOOD;
        } default: return NC_ERR_NULL;
      }
      return NC_ERR_GOOD;
    }

    // netc wrapper compatibility
    nc_error_t nraw_sockwrap(nc_socket_t *sock) {
      sock->open = &nraw_open;
      sock->close = &nraw_close;
      sock->write = &nraw_write;
      sock->read = &nraw_read;
      sock->setopt = &nraw_setopt;
      sock->getopt = &nraw_getopt;
      return nraw_socket(sock->sock);
    }
    #undef NC_RAW_IMPL
  #endif // NC_IMPLEMENTATION
#endif // end

// --- TLS/SSL --- //
#ifdef NC_TLS
  #ifndef __NETC_TLS_INCLUDED
  #define __NETC_TLS_INCLUDED
  typedef struct {
    int fd;

    int domain;   // ipv6 / ipv4 / (url comming soon?...)
    int type;     // stream (tcp) / dgram (udp)
    int protocol; // ip protocol tcp / ip protocol udp (maybe both arn't needed) (can bet set to 0 for automatic)

    uint16_t port;
    uint32_t flowinfo;
    uint64_t addr;
    uint32_t scope_id;

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
  #endif

  #ifdef NC_TLS_IMPL
    #include <openssl/ssl.h>

    // socket creation
    nc_error_t ntls_sockwrap(nc_socket_t *sock) {
      sock->open = &ntls_open;
      sock->close = &ntls_close;
      sock->write = &ntls_write;
      sock->read = &ntls_read;
      sock->setopt = &ntls_setopt;
      sock->getopt = &ntls_getopt;
      return ntls_socket(sock->sock);
    }
    nc_error_t ntls_socket(void *void_sock) {
      nc_tls_socket_t *sock = (nc_tls_socket_t*)void_sock;
      sock->ssl = NULL;
      sock->ctx = NULL;
      nraw_socket(void_sock); // cast to a raw socket which should be compatible
      return NC_ERR_GOOD;
    }
    nc_error_t ntls_close(void *void_sock) {
      nc_tls_socket_t *sock = (nc_tls_socket_t*)void_sock;
      if (sock->ssl) {
        SSL_free(sock->ssl);
      }
      nraw_close(void_sock);
      if (sock->ctx) {
        SSL_CTX_free(sock->ctx);
      }
      return NC_ERR_GOOD;
    }

    // connection
    nc_error_t ntls_open(void *void_sock, const char *ipaddr) {
      nc_tls_socket_t *sock = (nc_tls_socket_t*)void_sock;

      // ctx
      const SSL_METHOD *method = TLS_client_method(); /* Create new client-method instance */
      sock->ctx = SSL_CTX_new(method);
      if (sock->ctx == NULL) { return NC_ERR_INVL_CTX; }

      // connect
      nc_error_t open_err = nraw_open(void_sock, ipaddr);
      if (open_err != NC_ERR_GOOD) { return open_err; }

      // ssl
      sock->ssl = SSL_new(sock->ctx);
      if (sock->ssl == NULL) { return NC_ERR_MEMORY; }
      
      // bind fd to sock
      SSL_set_fd(sock->ssl, sock->fd);

      // set the connection state
      SSL_set_connect_state(sock->ssl);
      
      // start ssl connection    
      if (SSL_connect(sock->ssl) <= 0) {
        return NC_ERR_BAD_HANDSHAKE;
      }

      return NC_ERR_GOOD;
    }

    // functionality
    nc_error_t ntls_write(void *void_sock, const void *buf, size_t buf_size, size_t *bytes_read, nc_option_t param) {
      nc_tls_socket_t *sock = (nc_tls_socket_t*)void_sock;
      *bytes_read = SSL_write(sock->ssl, buf, buf_size);
      return NC_ERR_GOOD;
    }
    nc_error_t ntls_read(void *void_sock, void *buf, size_t buf_size, size_t *bytes_read, nc_option_t param) {
      nc_tls_socket_t *sock = (nc_tls_socket_t*)void_sock;
      *bytes_read = SSL_read(sock->ssl, buf, buf_size);
      return NC_ERR_GOOD;
    }

    // option
    nc_error_t ntls_setopt(void *void_sock, nc_option_t option, void *data, size_t data_size) {
      return nraw_setopt(void_sock, option, data, data_size);
    }
    nc_error_t ntls_getopt(void *void_sock, nc_option_t option, void *null_data, size_t data_size) { // overwrites null_data
      return nraw_getopt(void_sock, option, null_data, data_size);
    }
    #undef NC_TLS_IMPL
  #endif
#endif
