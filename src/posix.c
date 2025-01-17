#include <netc.h>

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

#include <stdio.h>
#include <stdlib.h>

nc_exit_flag_t G_default_exit_flag = NC_NO_EXIT;
nc_exit_flag_t *G_exit_flag = &G_default_exit_flag;

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

  if (!ipaddr) {
    if (connect(sock->fd, sock->__internal_addr, sock->__internal_addrlen) == -1) {
      nraw_close(sock); 
      return __internal_nraw_convert_errno(); 
    } 
  } else if (sock->domain == NC_OPT_IPV6) { 
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
  if (sock->__internal_addr) {
    free(sock->__internal_addr);
    sock->__internal_addr = NULL;
  }
  return NC_ERR_GOOD; 
}

// write, read
nc_error_t nraw_write(void *void_sock, const void *buf, size_t buf_size, size_t *bytes_written, nc_option_t param) {
  nc_raw_socket_t *sock = (nc_raw_socket_t*)void_sock;
  if (param == NC_OPT_DO_ALL) {
    size_t sent_bytes = 0;
    while (sent_bytes < buf_size) {
      if (*G_exit_flag) {
        *bytes_written = sent_bytes;
        return NC_ERR_EXIT_FLAG;
      }
      ssize_t bwritten = send(sock->fd, buf, buf_size - sent_bytes, 0);
      nc_error_t err = __internal_nraw_convert_errno();
      if (bwritten == -1 && err != NC_ERR_TIMED_OUT && err != NC_ERR_WOULD_BLOCK) {
        *bytes_written = sent_bytes;
        return err;
      }
      buf += bwritten;
      sent_bytes += bwritten;
    }
    *bytes_written = sent_bytes;
  } else {
    ssize_t bwrite = send(sock->fd, buf, buf_size, 0);
    if (bwrite == -1) {
      *bytes_written = 0;
      return __internal_nraw_convert_errno();
    }
    *bytes_written = (size_t)bwrite;
  }
  return NC_ERR_GOOD;
}
nc_error_t nraw_read(void *void_sock, void *buf, size_t buf_size, size_t *bytes_read, nc_option_t param) {
  nc_raw_socket_t *sock = (nc_raw_socket_t*)void_sock;
  if (param == NC_OPT_DO_ALL) {
    size_t bytes_recv = 0;
    while (bytes_recv < buf_size) {
      if (*G_exit_flag) {
        *bytes_read = bytes_recv;
        return NC_ERR_EXIT_FLAG;
      }
      ssize_t bread = recv(sock->fd, buf, buf_size - bytes_recv, 0);
      nc_error_t err = __internal_nraw_convert_errno();
      if (bread == -1 && err != NC_ERR_TIMED_OUT && err != NC_ERR_WOULD_BLOCK) {
        *bytes_read = bytes_recv;
        return err;
      }
      buf += bread;
      bytes_recv += bread;
    }
    *bytes_read = bytes_recv;
  } else {
    ssize_t bread = recv(sock->fd, buf, buf_size, 0);
    if (bread == -1) {
      *bytes_read = 0;
      return __internal_nraw_convert_errno();
    }
    *bytes_read = (size_t)bread;
  }
  return NC_ERR_GOOD;
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
      break;
    } case NC_OPT_SEND_TIMEOUT: {
      struct timeval timeout;
      timeout.tv_sec = ((ns_timeval_t*)data)->sec;
      timeout.tv_usec = ((ns_timeval_t*)data)->usec;
      if (setsockopt(sock->fd, SOL_SOCKET, SO_SNDTIMEO, (struct timeval *)&timeout, sizeof(timeout)) == -1) {
        return NC_ERR_SET_OPT_FAIL;
      }
      break;
    } case NC_OPT_EXIT_FLAG:
      G_exit_flag = (nc_exit_flag_t*)data;
      break;
    default: return NC_ERR_NULL;
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
    } case NC_OPT_EXIT_FLAG:
      nc_exit_flag_t** todata = (nc_exit_flag_t**)null_data;
      *todata = G_exit_flag;
      break;
    default: return NC_ERR_NULL;
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

// auxiliary
nc_error_t nraw_resolvehost(void *void_sock, const char *ipaddr) {
  nc_raw_socket_t *sock = (nc_raw_socket_t*)void_sock;
  struct addrinfo hints, *res;

  // translate raw socket to hints
  memset(&hints, 0, sizeof(hints));
  if (sock->domain == NC_OPT_IPV4) hints.ai_family = AF_INET;
  else if (sock->domain == NC_OPT_IPV6) hints.ai_family = AF_INET6;
  else hints.ai_family = AF_UNSPEC;

  if (sock->type == NC_OPT_SOCK_STREAM) hints.ai_socktype = SOCK_STREAM;
  else if (sock->type == NC_OPT_DGRAM) hints.ai_socktype = SOCK_DGRAM;

  if (sock->protocol == NC_OPT_TCP) hints.ai_protocol = IPPROTO_TCP;
  else if (sock->protocol == NC_OPT_UDP) hints.ai_protocol = IPPROTO_UDP;

  // what a nice standard
  // thanks posix
  // lets constantly switch between stringed and integral versions of erverything
  // i think this makes for a more fun api
  char port_str[6]; // buffer to hold the port number as a string (max for 16-bit integer is 65535)
  snprintf(port_str, sizeof(port_str), "%u", sock->port);

  int status = getaddrinfo(ipaddr, port_str, &hints, &res);
  if (status != 0) {
    return NC_ERR_INVALID_ADDRESS;
  }

  // Translate res back to raw socket
  sock->domain   = res->ai_family   == AF_INET     ? NC_OPT_IPV4        : NC_OPT_IPV6 ;
  sock->type     = res->ai_socktype == SOCK_STREAM ? NC_OPT_SOCK_STREAM : NC_OPT_DGRAM;
  sock->protocol = res->ai_protocol == IPPROTO_TCP ? NC_OPT_TCP         : NC_OPT_UDP  ;

  sock->__internal_addrlen = res->ai_addrlen;
  sock->__internal_addr = (struct sockaddr*)malloc(res->ai_addrlen);
  memcpy(sock->__internal_addr, res->ai_addr, res->ai_addrlen);

  freeaddrinfo(res);

  return NC_ERR_GOOD;
}

