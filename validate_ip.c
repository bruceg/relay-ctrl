#include <ctype.h>
#include <string.h>
#include "relay-ctrl.h"

const char* validate_ip(const char* str)
{
  /* Security by obscurity:
   * do certain simple checks on the remote IP string */
  const char* ptr;

  /* Skip over a IPv6 address prefix inserted by couriertcpd */
  if (!strncmp(str, "::ffff:", 7)) str += 7;
  
  /* Ensure that the IP string contains only digits and periods. */
  for (ptr = str; *ptr != 0; ptr++)
    if (*ptr != '.' && !isdigit(*ptr)) return 0;

  /* A dotted-quad can be a maximum of 15 characters in length. */
  if (ptr - str > 16) return 0;
  
  return str;
}
