#include <stdlib.h>
#include "relay-ctrl.h"

int is_authenticated(void)
{
  if (getenv("AUTHUSER") && getenv("AUTHARGV0")) {
    /* Courier IMAP or POP3 */
    if (getenv("AUTHENTICATED"))
      return 1;
  }
  else
    return 1;
  return 0;
}
