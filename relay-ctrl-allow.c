#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <bglibs/systime.h>
#include <unistd.h>
#include <utime.h>
#include <bglibs/misc.h>
#include <bglibs/msg.h>
#include "relay-ctrl.h"

const char program[] = "relay-ctrl-allow";
const int msg_show_pid = 1;
static const char* ip;
static const char* dir;

static int write_env(int fd, const char* var)
{
  const char* env;
  int len;
  if ((env = getenv(var)) != 0) {
    len = strlen(var);
    if (write(fd, var, len) != len) return 0;
    if (write(fd, "=", 1) != 1) return 0;
    len = strlen(env) + 1;
    if (write(fd, env, len) != len) return 0;
  }
  return 1;
}

static void make_file(const char* filename, int save_cwd)
{
  int fd;
  int error;
  int saved_umask;
  int mode;
  char tmpname[256];
  char* ptr;
  struct timeval t;
  
  switch (do_chdir(save_cwd)) {
  case 0: return;
  case 2: mode = 0666; break;
  default: mode = 0600; break;
  }
  saved_umask = umask(0);

  gettimeofday(&t, 0);
  ptr = tmpname; *ptr++ = '.';
  ptr = utoa2(t.tv_sec, ptr); *ptr++ = '.';
  ptr = utoa2(t.tv_usec, ptr); *ptr++ = ':';
  ptr = utoa2(getpid(), ptr);
  
  if ((fd = open(tmpname, O_WRONLY|O_CREAT|O_EXCL, mode)) == -1)
    warn3sys("Could not open '", tmpname, "' for writing");
  else {
    error = 0;
    if (!write_env(fd, "USER") ||
	!write_env(fd, "DOMAIN") ||
	close(fd) == -1) {
      warn3sys("Could not write to '", filename, "'");
      close(fd);
    }
    else if (rename(tmpname, filename) == -1)
      warn5sys("Could not rename '", tmpname, "' to '", filename, "'");
    unlink(tmpname);
  }
  if (save_cwd) {
    umask(saved_umask);
    do_chdir_back();
  }
}

int main(int argc, char* argv[])
{
  if ((ip = getenv("TCPREMOTEIP")) == 0 || (ip = validate_ip(ip)) == 0)
    die1(111, "Must be run from tcp-env or tcpserver.");
  if ((dir = getenv("RELAY_CTRL_DIR")) == 0)
    warn1("$RELAY_CTRL_DIR is not set.");
  else
    if (is_authenticated())
      make_file(ip, argc > 1);

  if (argc > 1) {
    execvp(argv[1], argv+1);
    return 111;
  }
  return 0;
}
