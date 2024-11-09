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
