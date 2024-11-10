#include <netc.h>

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