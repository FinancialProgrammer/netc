#include <netc.h>
#include <ncsrc/tls_src.h>

#include <ncsrc/posix_src.h>
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