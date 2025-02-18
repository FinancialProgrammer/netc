#ifndef NC_INVL_RAW_SOCK
  #define NC_INVL_RAW_SOCK -1
#endif

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

extern int G_default_exit_flag;
extern int *G_exit_flag;

// socket creation
nc_error_t nraw_socket(void *voidsock, int domain, int type, int protocol);
nc_error_t nraw_close(void *voidsock);

// connection
nc_error_t nraw_open(void *voidsock, struct nc_socketaddr *addr);
nc_error_t nraw_bind(void *voidsock, struct nc_socketaddr *addr);

// write, read
nc_error_t nraw_write(void *voidsock, const void *buf, size_t buf_size, size_t *bytes_written, nc_option_t param);
nc_error_t nraw_read(void *voidsock, void *buf, size_t buf_size, size_t *bytes_read, nc_option_t param);

// server functionality
nc_error_t nraw_listen(void *voidsock, int backlog);
nc_error_t nraw_accept(void *voidsock, void *voidclient, struct nc_socketaddr *clientaddr);

// poll
nc_error_t nraw_poll_create(nc_sockpoll_t *handle);
nc_error_t nraw_poll_ctl(nc_sockpoll_t *handle, int fd, void *data, nc_option_t opt);
nc_error_t nraw_poll_wait(nc_sockpoll_t *handle, nc_sockpoll_event_t *events, size_t eventslen, long int timeout, int *count);

// option
nc_error_t nraw_setopt(void *voidsock, nc_option_t option, const void *data, size_t data_size);
nc_error_t nraw_getopt(void *voidsock, nc_option_t option, void *null_data, size_t data_size);

