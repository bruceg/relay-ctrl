#ifndef RELAY_CTRL__H__
#define RELAY_CTRL__H__

#include "net/socket.h"

extern const char* validate_ip(const char*);
extern int is_authenticated(void);

#define DEFAULT_PORT 811

#endif
