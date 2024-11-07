#define NC_RAW_SOCKET nc_raw_socket_t
typedef int nc_raw_socket_t; // only the descriptor is needed

// auxiliary
const char *nraw_strerr(nc_error_t); // return stringed error

// socket connection
nc_error_t nraw_open(NC_RAW_SOCKET *sock, nc_option_t info, nc_address_t *addr);
nc_error_t nraw_close(NC_RAW_SOCKET sock);

// functionality
nc_error_t nraw_write(NC_RAW_SOCKET sock, const void *buf, size_t buf_size, size_t *bytes_read, nc_option_t param);
nc_error_t nraw_read(NC_RAW_SOCKET sock, void *buf, size_t buf_size, size_t *bytes_read, nc_option_t param);

// option
nc_error_t nraw_setopt(NC_RAW_SOCKET sock, nc_option_t option, void *data, size_t data_size);
nc_error_t nraw_getopt(NC_RAW_SOCKET sock, nc_option_t option, void *null_data, size_t data_size); // overwrites null_data

#ifdef NC_IMPLEMENTATION
  #include <sys/ioctl.h>
  #include <sys/socket.h>
  #include <sys/types.h>
  #include <unistd.h>

  #include <errno.h>
  #include <ifaddrs.h>
  #include <net/if.h>
  #include <netdb.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>

  const char *nrawerrors[] = {
    "No error spotted",
    "Couldn't parse error",
    "Invalid Memory",
    "Invalid Argument",
    "Invalid Address",
    "Connection Refused",
    "Timed Out",
    "Feature Not Implemented Yet",
    "Not Connected",
    "Ill Formed Message",
    "Socket Closed",
    "Would Block"
  };
  nc_error_t __internal_nraw_convert_errno() {
    switch (errno) {
      case 0: return NC_ERR_GOOD;
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
  const char *nraw_strerr(nc_error_t err) { return nrawerrors[err]; }

  // connection
  nc_error_t nraw_open(NC_RAW_SOCKET *sock, nc_option_t info, nc_address_t *addr) {
    int domain = (info & NC_OPT_IPV6) ? AF_INET6 : AF_INET; 
    int type = (info & NC_OPT_TCP) ? SOCK_STREAM : SOCK_DGRAM; 
    int protocol = (info & NC_OPT_TCP) ? IPPROTO_TCP : IPPROTO_UDP; 

    *sock = socket(domain, type, protocol); 
    if (*sock == -1) { 
      return __internal_nraw_convert_errno(); 
    }

    if (info & NC_OPT_IPV6) { 
      struct sockaddr_in6 address;
      memset(&address, 0, sizeof(address)); // Ensure the structure is zeroed out
      address.sin6_family = AF_INET6; 
      address.sin6_port = htons(addr->af_inet6.port); 
      address.sin6_flowinfo = htonl(addr->af_inet6.flowinfo); 
      address.sin6_scope_id = htonl(addr->af_inet6.scope_id);

      if (inet_pton(AF_INET6, addr->af_inet6.address, &address.sin6_addr) != 1) {
        close(*sock);
        return NC_ERR_INVALID_ADDRESS;
      }

      if (connect(*sock, (struct sockaddr*)&address, sizeof(address)) == -1) { 
        close(*sock); 
        return __internal_nraw_convert_errno(); 
      } 
    } else { 
      struct sockaddr_in address;
      memset(&address, 0, sizeof(address)); // Ensure the structure is zeroed out
      address.sin_family = AF_INET; 
      address.sin_port = htons(addr->af_inet.port); 

      if (inet_pton(AF_INET, addr->af_inet.address, &address.sin_addr) != 1) {
        close(*sock);
        return NC_ERR_INVALID_ADDRESS;
      }

      if (connect(*sock, (struct sockaddr*)&address, sizeof(address)) == -1) { 
        close(*sock);
        return __internal_nraw_convert_errno(); 
      }
    }
    return NC_ERR_GOOD;
  }
  nc_error_t nraw_close(NC_RAW_SOCKET sock) { close(sock); }

  // write, read
  nc_error_t nraw_write(NC_RAW_SOCKET sock, const void *buf, size_t buf_size, size_t *bytes_written, nc_option_t param) {
    ssize_t bwrite = write(sock, buf, buf_size);
    if (bwrite == -1) {
      return __internal_nraw_convert_errno();
    } else {
      *bytes_written = (size_t)bwrite;
      return NC_ERR_GOOD;
    }
  }
  nc_error_t nraw_read(NC_RAW_SOCKET sock, void *buf, size_t buf_size, size_t *bytes_read, nc_option_t param) {
    ssize_t bread = read(sock, buf, buf_size);
    if (bread == -1) {
      return __internal_nraw_convert_errno();
    } else {
      *bytes_read = (size_t)bread;
      return NC_ERR_GOOD;
    }
  }

  // option
  nc_error_t nraw_setopt(NC_RAW_SOCKET sock, nc_option_t option, void *data, size_t data_size) { return NC_ERR_NOT_IMPLEMENTED; }
  nc_error_t nraw_getopt(NC_RAW_SOCKET sock, nc_option_t option, void *null_data, size_t data_size) { return NC_ERR_NOT_IMPLEMENTED; }
#endif // NC_IMPLEMENTATION