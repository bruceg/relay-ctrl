#ifndef RELAY_CTRL__H__
#define RELAY_CTRL__H__

#include <bglibs/socket.h>

extern int is_authenticated(void);
extern int touch(const char*);
extern const char* validate_ip(const char*);
extern int do_chdir(int save_cwd);
extern void do_chdir_back(void);

#define DEFAULT_PORT 811

#endif
