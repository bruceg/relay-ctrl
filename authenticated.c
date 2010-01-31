#include <stdlib.h>
#include <string.h>
#include "relay-ctrl.h"

int is_authenticated(void)
{
  if (getenv("AUTHUSER") && getenv("AUTHARGV0")) {
    /* Courier IMAP or POP3 */
    char *authenticated = getenv("AUTHENTICATED");
    return authenticated != 0 && *authenticated != 0;
  }
  else
    return 1;
}
