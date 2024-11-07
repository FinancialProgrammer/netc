// global include
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// STATIC
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
// OPTIONS
  #define NC_OPT_NULL ((nc_option_t)0)
  #define NC_OPT_IPV4 ((nc_option_t)1)
  #define NC_OPT_IPV6 ((nc_option_t)2)
  // #define NC_OPT_URL  ((nc_option_t)3)
  #define NC_OPT_TCP  ((nc_option_t)4)
  #define NC_OPT_UDP  ((nc_option_t)5)
// Types
typedef int nc_error_t;
typedef int nc_option_t;

typedef union {
  struct {
    uint16_t port;
    char address[16];
  } af_inet;
  struct {
    uint16_t port;
    uint32_t flowinfo;
    char address[46];
    uint32_t scope_id;
  } af_inet6;
} nc_address_t;

// ------------ backend api ------------ // 
// --- RAW --- //
#if defined(WIN32) || defined(__MINGW32__) || defined(WIN64) || defined(__MINGW64__) // check for winsock (32)
  #include <ncsrc/win_src.h>
#else // else assume posix (posix has no actual way to check)
  // ignore error if operating system is likely to use the posix standard
  #if defined(__linux__) || defined(__sun) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
    (defined(__APPLE__) && defined(__MACH__)) || defined(__MSYS__) || defined(__unix__)
    #include <ncsrc/posix_src.h>
  #else
    #pragma message("Unknown operating system attempting to use POSIX")
    #include <ncsrc/posix_src.h>
  #endif
#endif // end

// --- TLS/SSL --- //
#ifdef NC_TLS
  #include <ncsrc/tls_src.h>
#endif



// ------------ wrapper api ------------ // 
/*
// (if only pure socket functionality is wanted checkout the backend api instead, it is still easy to use but doesn't allow for easy encryption use)

// backend struct
struct nsock;
struct nsock {
  // point to raw socket or extenstion socket 
  // (i don't like using malloc as it is incredibly slow compared to everything else but since this wrapper api is made for usability it shouldn't matter)
  // (unfortunately dynamicisity is needed in this case)
  void *socket;
  nc_error_t (*close)(struct nsock*);
  nc_error_t (*write)(struct nsock*, const void *, size_t, ssize_t*, nc_option_t);
  nc_error_t (*read)(struct nsock*, void *, size_t, ssize_t*, nc_option_t);
  nc_error_t (*setopt)(struct nsock*, nc_option_t, void*, size_t);
  nc_error_t (*getopt)(struct nsock*, nc_option_t, void*, size_t); // get information
};

// open (unique to each extenstion)
// sock_type[tcp,udp], addr_type[ipv4,ipv6,url]
nc_error_t nopen(struct nsock *sock, nc_option_t info); // open raw socket
// nc_error_t nopentls(struct nsock *sock);

// wrapper callback (just calls internal nsock functions) (if wanted the names are standard up there and could be called manually)
nc_error_t nclose(struct nsock *sock);
nc_error_t nwrite(struct nsock *sock, const void *buf, size_t buf_size, size_t *bytes_read, nc_option_t param);
nc_error_t nread(struct nsock *sock, void *buf, size_t buf_size, size_t *bytes_read, nc_option_t param);
nc_error_t nsetopt(struct nsock *sock, nc_option_t option, void *data, size_t data_size);
nc_error_t ngetopt(struct nsock *sock, nc_option_t option, void *null_data, size_t data_size); // overwrites null_data
*/