#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "msg/msg.h"
#include "relay-ctrl.h"

const char program[] = "relay-ctrl-allow";
const int msg_show_pid = 1;
static const char* ip;
static const char* dir;

static void make_file(const char* filename)
{
  int cwd;

  if ((cwd = open(".", O_RDONLY)) == -1) {
    warn1("Could not open current directory.");
    return;
  }
  if (chdir(dir) == -1) {
    warn3("Could not change directory to '", dir, "'.");
    return;
  }
  if (!touch(filename))
    warn3sys("Could not toucn '", filename, "'");
  if (fchdir(cwd) == -1)
    die1(111, "Could not change back to start directory.");
}

int main(int argc, char* argv[])
{
  if ((ip = getenv("TCPREMOTEIP")) == 0 || (ip = validate_ip(ip)) == 0)
    die1(111, "Must be run from tcp-env or tcpserver.");
  if ((dir = getenv("RELAY_CTRL_DIR")) == 0)
    warn1("$RELAY_CTRL_DIR is not set.");
  else
    if (is_authenticated()) {
      make_file(ip);
      /* Since this program will be either setuid or setgid,
	 revoke our priviledges now. */
      setgid(getgid());
      setuid(getuid());
    }

  if (argc > 1) {
    execvp(argv[1], argv+1);
    return 111;
  }
  return 0;
}
