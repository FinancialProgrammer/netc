#include <netc.h>
#include <stdbool.h>

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
  "Couldn't complete handshake",
  "Exit flag was set before completion",
  "The accepted ipv6 adress in the accept function is larger than a uint64_t",
  "No Data Available",
  "SSL CTX manager couldn't use ssl certificate"
  "SSL CTX manager couldn't use private key certificate"
  "SSL CTX manager completed a check of private key and found it dubious"
  "PLACEHOLDER 0", // to handle programming mistakes
  "PLACEHOLDER 1",
  "PLACEHOLDER 2",
  "PLACEHOLDER 3",
  "PLACEHOLDER 4"
};
const char *nstrerr(nc_error_t err) { return nstrerrarr[err]; }