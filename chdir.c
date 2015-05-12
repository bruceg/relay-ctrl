#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include "relay-ctrl.h"
#include <bglibs/misc.h>
#include <bglibs/msg.h>

static int cwd = -1;

int do_chdir(int save_cwd)
{
  const char* tmp;
  int fd;
  
  if (save_cwd && (cwd = open(".", O_RDONLY)) == -1)
    warn1sys("Could not open current directory");
  else if ((tmp = getenv("RELAY_CTRL_DIR_FD")) != 0) {
    fd = strtou(tmp, &tmp);
    if (*tmp != 0)
      warn1("Invalid value for $RELAY_CTRL_DIR_FD");
    else if (fchdir(fd) == -1)
      warn2sys("Could not chdir to FD ", tmp);
    else {
      close(fd);
      return 2;
    }
  }
  else if ((tmp = getenv("RELAY_CTRL_DIR")) != 0) {
    if (chdir(tmp) == -1)
      warn3sys("Could not change directory to '", tmp, "'");
    return 1;
  }
  else
    warn1("$RELAY_CTRL_DIR is not set");
  return 0;
}

void do_chdir_back(void)
{
  if (fchdir(cwd) == -1)
    die1sys(111, "Could not fchdir back to original directory");
  close(cwd);
}
