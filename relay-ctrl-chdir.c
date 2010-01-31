#include <sysdeps.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <misc/misc.h>
#include <msg/msg.h>

const char program[] = "relay-ctrl-chdir";
const int msg_show_pid = 1;

/* Move a file descriptor into the highest available slot */
static int move_high(int fd)
{
  struct stat s;
  int newfd;

  for (newfd = fd + 1;; ++newfd) {
    if (fstat(newfd, &s) != -1) continue;
    if (dup2(fd, newfd) == -1) break;
    close(fd);
    fd = newfd;
  }
  return fd;
}

int main(int argc, char* argv[])
{
  const char* dir;
  int fd;
  struct stat s;
  char fdstr[32];
  
  if (argc < 2) die1(1, "usage: relay-ctrl-chdir program [arguments]\n");
  if ((dir = getenv("RELAY_CTRL_DIR")) == 0)
    die1(111, "$RELAY_CTRL_DIR is not set.");
  else if ((fd = open(dir, O_RDONLY)) == -1)
    die3sys(111, "Could not open dir '", dir, "'");
  else if (fstat(fd, &s) == -1)
    die1sys(111, "Could not stat opened file");
  else if (!S_ISDIR(s.st_mode))
    die3(111, "'", dir, "' is not a directory");
  else {
    fd = move_high(fd);
    utoa2(fd, fdstr);
    if (setenv("RELAY_CTRL_DIR_FD", fdstr, 1) == -1)
      die1(111, "Could not set environment variable");
  }
  execvp(argv[1], argv+1);
  die1(111, "execution of program failed!\n");
  return 111;
  argc = 0;
}
