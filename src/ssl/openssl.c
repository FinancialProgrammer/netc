#include <netc.h>
#include <openssl/ssl.h>

#include "src/raw/internalraw.h"

const char *G_cert_file = NULL;
const char *G_privkey_file = NULL;

// socket creation
nc_error_t ntls_socket(void *void_sock, int domain, int type, int protocol) {
  struct nc_openssl_socket *sock = (struct nc_openssl_socket*)void_sock;
  sock->ssl = NULL;
  sock->ctx = NULL;
  sock->is_client = 0;
  nraw_socket(void_sock, domain, type, protocol); // cast to a raw socket which should be compatible
  return NC_ERR_GOOD;
}

nc_error_t ntls_close(void *void_sock) {
  struct nc_openssl_socket *sock = (struct nc_openssl_socket*)void_sock;
  if (sock->ssl) {
    SSL_free(sock->ssl);
  }
  nraw_close(void_sock);
  if (sock->ctx && !sock->is_client) {
    SSL_CTX_free(sock->ctx);
  }
  return NC_ERR_GOOD;
}

// server encryption setup in bind
nc_error_t ntls_bind(void *void_sock, struct nc_socketaddr *addr) {
  struct nc_openssl_socket *sock = (struct nc_openssl_socket*)void_sock;

  // ctx
  const SSL_METHOD *method = TLS_server_method(); /* Create new server-method instance */
  sock->ctx = SSL_CTX_new(method);
  if (sock->ctx == NULL) { return NC_ERR_INVL_CTX; }

  // Load the server's certificate and key

  if (G_cert_file == NULL || SSL_CTX_use_certificate_file(sock->ctx, G_cert_file, SSL_FILETYPE_PEM) <= 0) {
    SSL_CTX_free(sock->ctx);
    return NC_ERR_CERT_FILE;
  }
  if (G_privkey_file == NULL || SSL_CTX_use_PrivateKey_file(sock->ctx, G_privkey_file, SSL_FILETYPE_PEM) <= 0) {
    SSL_CTX_free(sock->ctx);
    return NC_ERR_KEY_FILE;
  }
  if (!SSL_CTX_check_private_key(sock->ctx)) {
    SSL_CTX_free(sock->ctx);
    return NC_ERR_INVL_KEY;
  }

  // bind the raw socket
  nc_error_t bind_err = nraw_bind(void_sock, addr);
  if (bind_err != NC_ERR_GOOD) { return bind_err; }

  return NC_ERR_GOOD;
}

// connection
nc_error_t ntls_open(void *void_sock, struct nc_socketaddr *addr) {
  struct nc_openssl_socket *sock = (struct nc_openssl_socket*)void_sock;

  // ctx
  const SSL_METHOD *method = TLS_client_method(); /* Create new client-method instance */
  sock->ctx = SSL_CTX_new(method);
  if (sock->ctx == NULL) { return NC_ERR_INVL_CTX; }

  // connect
  nc_error_t open_err = nraw_open(void_sock, addr);
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

// server functionality
nc_error_t ntls_accept(void *void_sock, void *void_client, struct nc_socketaddr *clientaddr) {
  struct nc_openssl_socket *sock = (struct nc_openssl_socket*)void_sock;
  struct nc_openssl_socket *client = (struct nc_openssl_socket*)void_client;

  // Accept the connection using nraw_accept
  nc_error_t accept_err = nraw_accept(void_sock, void_client, clientaddr);
  if (accept_err != NC_ERR_GOOD) {
    return accept_err;
  }

  // SSL setup for the accepted client connection
  client->is_client = 1;
  client->ctx = sock->ctx;  // Reuse the server's SSL context
  client->ssl = SSL_new(client->ctx);
  if (client->ssl == NULL) {
    return NC_ERR_MEMORY;
  }

  // Bind the new socket file descriptor to the SSL structure
  SSL_set_fd(client->ssl, client->fd);

  // Set the SSL structure to work in server mode
  SSL_set_accept_state(client->ssl);

  // Start SSL connection and perform the handshake
  if (SSL_accept(client->ssl) <= 0) {
    return NC_ERR_BAD_HANDSHAKE;
  }

  return NC_ERR_GOOD;
}

// functionality
// write, read
nc_error_t ntls_write(void *voidsock, const void *buf, size_t bufsize, size_t *bytes_written, nc_option_t param) {
  struct nc_openssl_socket *sock = (struct nc_openssl_socket*)voidsock;
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
      ssize_t tmp_bwritten = SSL_write(sock->ssl, buf, bufsize - bwritten);
      if (tmp_bwritten == -1 && __nc_convert_os_error() != NC_ERR_WOULD_BLOCK) {
        err = __nc_convert_os_error();
        break;
      }

      // update pointer and size left
      buf += tmp_bwritten;
      bwritten += tmp_bwritten;
    }
  } else if (param == NC_OPT_NONBLOCK) {
    while (bwritten < bufsize) {
      if (*G_exit_flag) {
        err = NC_ERR_EXIT_FLAG;
        break;
      }

      // send all data currently possible
      ssize_t tmp_bwritten = SSL_write(sock->ssl, buf, bufsize - bwritten);
      if (tmp_bwritten == -1) {
        err = __nc_convert_os_error();
        if (err == NC_ERR_WOULD_BLOCK) err = NC_ERR_GOOD;
        break;
      } else if (tmp_bwritten == 0) {
        break;
      }

      // update pointer and size left
      buf += tmp_bwritten;
      bwritten += tmp_bwritten;
    }
  } else {
    bwritten = SSL_write(sock->ssl, buf, bufsize);
    if (bwritten == -1) {
      bwritten = 0;
      err = __nc_convert_os_error();
    }
  }

  if (bytes_written != NULL) { *bytes_written = (size_t)bwritten; }
  return err;
}
nc_error_t ntls_read(void *voidsock, void *buf, size_t bufsize, size_t *bytes_read, nc_option_t param) {
  struct nc_openssl_socket *sock = (struct nc_openssl_socket*)voidsock;
  ssize_t bread = 0;
  nc_error_t err = NC_ERR_GOOD;

  if (param == NC_OPT_DO_ALL) {
    while (bread < bufsize) {
      if (*G_exit_flag) {
        err = NC_ERR_EXIT_FLAG;
        break;
      }

      // recv all data currently possible
      ssize_t tmp_bread = SSL_read(sock->ssl, buf, bufsize - bread);
      if (tmp_bread == -1 && __nc_convert_os_error() != NC_ERR_WOULD_BLOCK) {
        err = __nc_convert_os_error();
        break;
      }

      // update pointer and size left
      buf += tmp_bread;
      bread += tmp_bread;
    }
  } else if (param == NC_OPT_NONBLOCK) {
    while (bread < bufsize) {
      if (*G_exit_flag) {
        err = NC_ERR_EXIT_FLAG;
        break;
      }

      // recv all data currently possible
      ssize_t tmp_bread = SSL_read(sock->ssl, buf, bufsize - bread);
      if (tmp_bread == -1) {
        err = __nc_convert_os_error();
        if (err == NC_ERR_WOULD_BLOCK) err = NC_ERR_GOOD;
        break;
      } else if (tmp_bread == 0) {
        break;
      }

      // update pointer and size left
      buf += tmp_bread;
      bread += tmp_bread;
    }
  } else {
    bread = SSL_read(sock->ssl, buf, bufsize);
    if (bread == -1) {
      bread = 0;
      err = __nc_convert_os_error();
    }
  }

  if (bytes_read != NULL) { *bytes_read = (size_t)bread; }
  return err;

  return NC_ERR_GOOD;
}

// option
nc_error_t ntls_setopt(void *voidsock, nc_option_t opt, const void *data, size_t datalen) {
  if (opt == NC_OPT_CERT_FILE) {
    G_cert_file = (const char*)data;
    return NC_ERR_GOOD;
  } else if (opt == NC_OPT_PRIV_KEY_FILE) {
    G_privkey_file = (const char*)data;
    return NC_ERR_GOOD;
  }
  return nraw_setopt(voidsock, opt, data, datalen);
}
nc_error_t ntls_getopt(void *voidsock, nc_option_t opt, void *data, size_t datalen) {
  if (opt == NC_OPT_CERT_FILE) {
    data = (void*)G_cert_file;
    return NC_ERR_GOOD;
  } else if (opt == NC_OPT_PRIV_KEY_FILE) {
    data = (void*)G_privkey_file;
    return NC_ERR_GOOD;
  }
  return nraw_getopt(voidsock, opt, data, datalen);
}


struct nc_functions nc_functions_openssl() {
  struct nc_functions funcs;

  funcs.socket = &ntls_socket;
  funcs.close  = &ntls_close;
  
  funcs.open   = &ntls_open;
  funcs.bind   = &ntls_bind; // updated to use the new bind function

  funcs.write  = &ntls_write;
  funcs.read   = &ntls_read;
  
  funcs.listen = &nraw_listen;
  funcs.accept = &ntls_accept;
  
  funcs.poll_create   = &nraw_poll_create;
  funcs.poll_ctl      = &nraw_poll_ctl;
  funcs.poll_wait     = &nraw_poll_wait;
  
  funcs.setopt = &ntls_setopt;
  funcs.getopt = &ntls_getopt;
  
  return funcs;
}
