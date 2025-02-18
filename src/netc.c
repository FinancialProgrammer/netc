#include <netc.h>
#include <stdbool.h>

const char *nstrerr(nc_error_t error) {
  switch (error) {
  case NC_ERR_GOOD: return "No error";
  case NC_ERR_NULL: return "Null error";
  case NC_ERR_MEMORY: return "Memory error";
  case NC_ERR_INVALID_ARGUMENT: return "Invalid argument";
  case NC_ERR_INVALID_ADDRESS: return "Invalid address";
  case NC_ERR_CONNECTION_REFUSED: return "Connection refused";
  case NC_ERR_NOT_INITED: return "Not initialized";
  case NC_ERR_TIMED_OUT: return "Timed out";
  case NC_ERR_NOT_IMPLEMENTED: return "Not implemented";
  case NC_ERR_NOT_CONNECTED: return "Not connected";
  case NC_ERR_ILL_FORMED_MESSAGE: return "Ill-formed message";
  case NC_ERR_SOCKET_CLOSED: return "Socket closed";
  case NC_ERR_WOULD_BLOCK: return "Would block";
  case NC_ERR_SET_OPT_FAIL: return "Set option failed";
  case NC_ERR_INVL_CTX: return "Invalid context";
  case NC_ERR_BAD_HANDSHAKE: return "Bad handshake";
  case NC_ERR_EXIT_FLAG: return "Exit flag";
  case NC_ERR_ACCEPTED_ADDR_TOO_LARGE: return "Accepted address too large";
  case NC_ERR_NO_DATA_AVAILABLE: return "No data available";
  case NC_ERR_CERT_FILE: return "Certificate file error";
  case NC_ERR_KEY_FILE: return "Key file error";
  case NC_ERR_INVL_KEY: return "Invalid key";
  case NC_ERR_SIGNAL_INTERRUPT: return "Signal interrupt";
  default: return "Unknown error";
  }
}
