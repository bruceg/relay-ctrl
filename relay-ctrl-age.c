#include <sysdeps.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <systime.h>
#include <msg/msg.h>

const char program[] = "relay-ctrl-age";
const int msg_show_pid = 0;

int main(void)
{
  const char* tmp;
  DIR* dir;
  direntry* entry;
  long expiry;
  time_t oldtime;
  
  expiry = 0;
  if ((tmp = getenv("RELAY_CTRL_EXPIRY")) != 0) expiry = atol(tmp);
  if (expiry <= 0) expiry = 900;
  oldtime = time(0) - expiry;
  
  if ((tmp = getenv("RELAY_CTRL_DIR")) == 0)
    die1(1, "$RELAY_CTRL_DIR is not set.");
  if (chdir(tmp) == -1)
    die3(1, "Could not change directory to '", tmp, "'.");
  if ((dir = opendir(".")) == 0)
    die3(1, "Could not open directory '", tmp, "'.");
  while ((entry = readdir(dir)) != 0) {
    const char* name;
    struct stat buf;
    name = entry->d_name;
    if (name[0] == '.' && (name[1] == 0 ||
			   (name[1] == '.' && name[2] == 0)))
      continue;
    if (stat(name, &buf) == -1) continue;
    if (buf.st_mtime < oldtime) unlink(name);
  }
  closedir(dir);
  return 0;
}
