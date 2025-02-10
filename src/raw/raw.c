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

#include <poll.h>

#include <stdio.h>
#include <string.h>

#include "src/raw/internalraw.h"

int G_default_exit_flag = 0;
int *G_exit_flag = &G_default_exit_flag;

#ifdef _WIN32
  nc_error_t __nc_convert_os_error() {
    return NC_ERR_NULL; // Not Implemented Yet
  }
#else
  nc_error_t __nc_convert_os_error() {
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
      default: 
        printf("NETC: some errno error: %d\n", errno);
        return NC_ERR_NULL;
    };
  }
#endif

// --- Auxiliary Networking Functions --- //
  nc_error_t netc_resolve_addrV6(struct nc_socketaddr *addr, nc_option_t opt, const char *ipaddr, uint16_t port, uint32_t flowinfo, uint32_t scope_id) {
    struct sockaddr_in6 *address = (struct sockaddr_in6 *)&addr->__internal_addr;
    memset(address, 0, sizeof(*address));
    address->sin6_family = AF_INET6;
    address->sin6_port = htons(port);
    address->sin6_flowinfo = htonl(flowinfo);
    address->sin6_scope_id = htonl(scope_id);

    if (opt == NC_OPT_INADDR_ANY) {
      address->sin6_addr = in6addr_any;
    } else {
      if (inet_pton(AF_INET6, ipaddr, &address->sin6_addr) != 1) {
        return NC_ERR_INVALID_ADDRESS;
      }
    }
    addr->__internal_addrlen = sizeof(struct sockaddr_in6);
    return NC_ERR_GOOD;
  }
  nc_error_t netc_resolve_ipV6(struct nc_socketaddr *ncsockaddr, const char *ipaddr, uint16_t port, uint32_t flowinfo, uint32_t scope_id) {
    struct sockaddr_in6 *address = (struct sockaddr_in6 *)&ncsockaddr->__internal_addr;
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;

    if (getaddrinfo(ipaddr, NULL, &hints, &res) != 0) {
      return NC_ERR_INVALID_ADDRESS;
    }

    memset(address, 0, sizeof(*address));
    address->sin6_family = AF_INET6;
    address->sin6_addr = ((struct sockaddr_in6 *)res->ai_addr)->sin6_addr;
    address->sin6_port = htons(port);
    address->sin6_flowinfo = htonl(flowinfo);
    address->sin6_scope_id = htonl(scope_id);
    ncsockaddr->__internal_addrlen = sizeof(struct sockaddr_in6);
    freeaddrinfo(res);

    return NC_ERR_GOOD;
  }
  nc_error_t netc_resolve_addrV4(struct nc_socketaddr *addr, nc_option_t opt, const char *ipaddr, uint16_t port) {
    struct sockaddr_in *address = (struct sockaddr_in *)&addr->__internal_addr;
    memset(address, 0, sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);

    if (opt == NC_OPT_INADDR_ANY) {
      address->sin_addr.s_addr = INADDR_ANY;
    } else {
      if (inet_pton(AF_INET, ipaddr, &address->sin_addr) != 1) {
        return NC_ERR_INVALID_ADDRESS;
      }
    }
    addr->__internal_addrlen = sizeof(struct sockaddr_in);
    return NC_ERR_GOOD;
  }
  nc_error_t netc_resolve_ipV4(struct nc_socketaddr *ncsockaddr, const char *ipaddr, uint16_t port) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    if (getaddrinfo(ipaddr, NULL, &hints, &res) != 0) {
      return NC_ERR_INVALID_ADDRESS;
    }

    struct sockaddr_in *address = (struct sockaddr_in *)&ncsockaddr->__internal_addr;
    memset(address, 0, sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
    address->sin_port = htons(port);
    ncsockaddr->__internal_addrlen = sizeof(struct sockaddr_in);
    freeaddrinfo(res);

    return NC_ERR_GOOD;
  }
// --- END --- //

// --- SOCKET Functionality --- //
// socket creation
nc_error_t nraw_socket(void *voidsock, int domain, int type, int protocol) {
  nc_socket_t *sock = (nc_socket_t*)voidsock;

  *sock = socket(
    (domain == NC_OPT_IPV6) ? AF_INET6 : AF_INET, // default ipv4
    (type == NC_OPT_DGRAM) ? SOCK_DGRAM : SOCK_STREAM, // default tcp (?)
    (protocol == NC_OPT_UDP) ? IPPROTO_UDP : IPPROTO_TCP // default tcp
  );

  if (*sock == NC_INVL_RAW_SOCK) {
    return __nc_convert_os_error();
  }

  return NC_ERR_GOOD;
}
nc_error_t nraw_close(void *voidsock) { 
  nc_socket_t *sock = (nc_socket_t*)voidsock;
  NCCLOSESOCKET(*sock);
  return NC_ERR_GOOD; 
}

// connection
nc_error_t nraw_open(void *voidsock, struct nc_socketaddr *addr) {
  nc_socket_t *sock = (nc_socket_t*)voidsock;

  if (connect(*sock, (struct sockaddr*)&addr->__internal_addr, addr->__internal_addrlen) == -1) {
    return __nc_convert_os_error(); 
  }

  return NC_ERR_GOOD;
}
nc_error_t nraw_bind(void *voidsock, struct nc_socketaddr *addr) {
  nc_socket_t *sock = (nc_socket_t*)voidsock;

  if (bind(*sock, (struct sockaddr*)&addr->__internal_addr, addr->__internal_addrlen) == -1) {
    return __nc_convert_os_error();
  }

  return NC_ERR_GOOD;
}

// write, read
nc_error_t nraw_write(void *voidsock, const void *buf, size_t bufsize, size_t *bytes_written, nc_option_t param) {
  nc_socket_t *sock = (nc_socket_t*)voidsock;
  ssize_t bwritten = 0;
  nc_error_t err = NC_ERR_GOOD;

  if (param == NC_OPT_DO_ALL) {
    while (bwritten < bufsize) {
      // check the Global exit flag for better safety with infinite loops
      // A timeout should also be possible here
      if (*G_exit_flag) {
        err = NC_ERR_EXIT_FLAG;
        break;
      }

      // send all data currently possible
      ssize_t tmp_bwritten = send(*sock, buf, bufsize - bwritten, 0);
      if (tmp_bwritten == -1 && __nc_convert_os_error() != NC_ERR_TIMED_OUT && __nc_convert_os_error() != NC_ERR_WOULD_BLOCK) {
        err = __nc_convert_os_error();
        break;
      }

      // update pointer and size left
      buf += tmp_bwritten;
      bwritten += tmp_bwritten;
    }
  } else {
    bwritten = send(*sock, buf, bufsize, 0);
    if (bwritten == -1) {
      bwritten = 0;
      err = __nc_convert_os_error();
    }
  }
  if (bytes_written != NULL) { *bytes_written = (size_t)bwritten; }
  return err;
}
nc_error_t nraw_read(void *voidsock, void *buf, size_t bufsize, size_t *bytes_read, nc_option_t param) {
  nc_socket_t *sock = (nc_socket_t*)voidsock;
  ssize_t bread = 0;
  nc_error_t err = NC_ERR_GOOD;

  if (param == NC_OPT_DO_ALL) {
    while (bread < bufsize) {
      // check the Global exit flag for better safety with infinite loops
      // A timeout should also be possible here
      if (*G_exit_flag) {
        err = NC_ERR_EXIT_FLAG;
        break;
      }

      // send all data currently possible
      ssize_t tmp_bread = recv(*sock, buf, bufsize - bread, 0);
      if (tmp_bread == -1 && __nc_convert_os_error() != NC_ERR_TIMED_OUT && __nc_convert_os_error() != NC_ERR_WOULD_BLOCK) {
        err = __nc_convert_os_error();
        break;
      }

      // update pointer and size left
      buf += tmp_bread;
      bread += tmp_bread;
    }
  } else if (param == NC_OPT_MSG_PEAK) {
    bread = recv(*sock, buf, bufsize, MSG_PEEK); 
    if (bread == -1) {
      bread = 0;
      err = __nc_convert_os_error();
    }
  } else {
    bread = recv(*sock, buf, bufsize, 0);
    if (bread == -1) {
      bread = 0;
      err = __nc_convert_os_error();
    }
  }

  if (bytes_read != NULL) { *bytes_read = (size_t)bread; }
  return err;

  return NC_ERR_GOOD;
}
nc_error_t nraw_writeto(void *voidsock, nc_socketaddr_t *addr, const void *buf, size_t bufsize, size_t *bytes_written, nc_option_t param) {
  nc_socket_t *sock = (nc_socket_t*)voidsock;
  ssize_t bwritten = 0;
  nc_error_t err = NC_ERR_GOOD;

  bwritten = sendto(*sock, buf, bufsize, 0, (struct sockaddr*)&addr->__internal_addr, addr->__internal_addrlen);
  if (bwritten == -1) {
    bwritten = 0;
    err = __nc_convert_os_error();
  }

  if (bytes_written != NULL) { *bytes_written = (size_t)bwritten; }
  return err;
}
nc_error_t nraw_readfrom(void *voidsock, nc_socketaddr_t *addr, void *buf, size_t bufsize, size_t *bytes_read, nc_option_t param) {
  nc_socket_t *sock = (nc_socket_t*)voidsock;
  ssize_t bread = 0;
  nc_error_t err = NC_ERR_GOOD;

  addr->__internal_addrlen = sizeof(addr->__internal_addrlen);
  bread = recvfrom(*sock, buf, bufsize, 0, (struct sockaddr*)&addr->__internal_addr, &addr->__internal_addrlen);
  if (bread == -1) {
    bread = 0;
    err = __nc_convert_os_error();
  }

  if (bytes_read != NULL) { *bytes_read = (size_t)bread; }
  return err;

  return NC_ERR_GOOD;
}

// server functionality
nc_error_t nraw_listen(void *voidsock, int backlog) {
  nc_socket_t *sock = (nc_socket_t*)voidsock;

  if (listen(*sock, backlog) == -1) {
    return __nc_convert_os_error();
  }

  return NC_ERR_GOOD;
}
nc_error_t nraw_accept(void *voidsock, void *voidclient, struct nc_socketaddr *clientaddr) {
  nc_socket_t *sock = (nc_socket_t*)voidsock;
  nc_socket_t *client = (nc_socket_t*)voidclient;

  struct nc_socketaddr addr;
  socklen_t addrlen = sizeof(addr);
  int fd = accept(*sock, (struct sockaddr*)&addr, &addrlen);
  if (fd == -1) {
    return __nc_convert_os_error();
  }

  if (clientaddr != NULL) {
    *clientaddr = addr;
  }

  // Copy client fd
  *client = fd;

  return NC_ERR_GOOD;
}

// poll
nc_error_t nraw_poll(struct nc_sockpoll *polled, size_t polled_len, int timeout, nc_option_t param) {
  int retval = poll((struct pollfd *)polled, polled_len, timeout);
  if (retval == -1) return __nc_convert_os_error();
  return NC_ERR_GOOD;
}

// option
nc_error_t nraw_setopt(void *voidsock, nc_option_t option, const void *data, size_t data_size) {
  nc_socket_t *sock = (nc_socket_t*)voidsock;
  switch (option) {
    case NC_OPT_RECV_TIMEOUT: {
      struct timeval timeout;
      timeout.tv_sec = ((const struct nc_timeval_t*)data)->sec;
      timeout.tv_usec = ((const struct nc_timeval_t*)data)->usec;
      if (setsockopt(*sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&timeout, sizeof(timeout)) == -1) {
        return NC_ERR_SET_OPT_FAIL;
      }
      break;
    } case NC_OPT_SEND_TIMEOUT: {
      struct timeval timeout;
      timeout.tv_sec = ((const struct nc_timeval_t*)data)->sec;
      timeout.tv_usec = ((const struct nc_timeval_t*)data)->usec;
      if (setsockopt(*sock, SOL_SOCKET, SO_SNDTIMEO, (struct timeval *)&timeout, sizeof(timeout)) == -1) {
        return NC_ERR_SET_OPT_FAIL;
      }
      break;
    } case NC_OPT_EXIT_FLAG:
      G_exit_flag = (int*)data;
      break;
    case NC_OPT_REUSEADDR: {
      nc_opt_bool_t opt = *((const nc_opt_bool_t*)data);
      if (setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        return NC_ERR_SET_OPT_FAIL;
      }
      break;
    } default: return NC_ERR_NULL;
  }
  return NC_ERR_GOOD;
}
nc_error_t nraw_getopt(void *voidsock, nc_option_t option, void *null_data, size_t data_size) {
  nc_socket_t *sock = (nc_socket_t*)voidsock;
  switch (option) {
    case NC_OPT_RECV_TIMEOUT: {
      socklen_t timeout_size;
      struct timeval timeout;
      if (getsockopt(*sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&timeout, &timeout_size) == -1) {
        return NC_ERR_SET_OPT_FAIL;
      }
      struct nc_timeval_t *nsts = (struct nc_timeval_t*)null_data;
      nsts->sec = timeout.tv_sec;
      nsts->usec = timeout.tv_usec;
      return NC_ERR_GOOD;
    } case NC_OPT_SEND_TIMEOUT: {
      socklen_t timeout_size;
      struct timeval timeout;
      if (getsockopt(*sock, SOL_SOCKET, SO_SNDTIMEO, (struct timeval *)&timeout, &timeout_size) == -1) {
        return NC_ERR_SET_OPT_FAIL;
      }
      struct nc_timeval_t *nsts = (struct nc_timeval_t*)null_data;
      nsts->sec = timeout.tv_sec;
      nsts->usec = timeout.tv_usec;
      return NC_ERR_GOOD;
    } case NC_OPT_EXIT_FLAG:
      int* todata = (int*)null_data;
      *todata = *G_exit_flag;
      break;
    case NC_OPT_REUSEADDR: {
      socklen_t nullsize = 0;
      if (getsockopt(*sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, null_data, &nullsize)) {
        return NC_ERR_SET_OPT_FAIL;
      }
      break;
    } default: return NC_ERR_NULL;
  }
  return NC_ERR_GOOD;
}

// --- END --- //

struct nc_functions nc_functions_raw() {
  struct nc_functions funcs;

  funcs.socket = &nraw_socket;
  funcs.close  = &nraw_close;
  
  funcs.open   = &nraw_open;
  funcs.bind   = &nraw_bind;
  
  funcs.write  = &nraw_write;
  funcs.read   = &nraw_read;
  funcs.writeto = &nraw_writeto;
  funcs.readfrom = &nraw_readfrom;
  
  funcs.listen = &nraw_listen;
  funcs.accept = &nraw_accept;
  
  funcs.poll   = &nraw_poll;
  
  funcs.setopt = &nraw_setopt;
  funcs.getopt = &nraw_getopt;
  
  return funcs;
}