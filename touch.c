#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include "relay-ctrl.h"

int touch(const char* filename)
{
  int fd;
  if ((fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0666)) == -1) return 0;
  if (close(fd) == -1) return 0;
  if (utime(filename, 0) == -1) return 0;
  return 1;
}
