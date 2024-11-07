// NOT IMPLEMENTED YET


#ifdef NC_IMPLEMENTATION
  const char *nrawerrors[] = {
    "No error spotted",
    "Couldn't parse error",
    "Invalid Memory",
    "Invalid Argument",
    "Invalid Kernel Error",
    "Address Lookup Failed",
    "Connection Refused",
    "Timed Out",
    "Feature Not Implemented Yet",
    "Not Connected",
    "Ill Formed Message",
    "Socket Closed",
    "Would Block"
  };
  void __internal_nraw_null_errno() { WSASetLastError(0); } 
  nc_error_t __internal_nraw_convert_errno() { 
    switch (WSAGetLastError()) { 
      case 0: return NC_ERR_GOOD; 
      case WSAEADDRNOTAVAIL: return NC_ERR_ADDRESS_LOOKUP_FAILED; // Cannot assign requested address 
      case WSAECONNREFUSED: return NC_ERR_CONNECTION_REFUSED; // Connection refused 
      case WSAEHOSTUNREACH: return NC_ERR_NOT_INITED; // No route to host 
      case WSAETIMEDOUT: return NC_ERR_TIMED_OUT; // Connection timed out 
      case WSAENOTCONN: return NC_ERR_NOT_CONNECTED; // Transport endpoint is not connected 
      case WSAEBADMSG: return NC_ERR_ILL_FORMED_MESSAGE; // Bad message 
      case WSAECONNRESET: return NC_ERR_SOCKET_CLOSED; // Connection reset by peer 
      case WSAEWOULDBLOCK: return NC_ERR_WOULD_BLOCK; 
      default: return NC_ERR_NULL; 
    }
  }
  const char *nraw_strerr(nc_error_t err) { return nrawerrors[err]; }
#endif