#ifndef RELAY_CTRL__H__
#define RELAY_CTRL__H__

#include "net/socket.h"

extern int is_authenticated(void);
extern int touch(const char*);
extern const char* validate_ip(const char*);

#define DEFAULT_PORT 811

#endif
