#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "msg/msg.h"
#include "systime.h"
#include "setenv.h"

const char program[] = "relay-ctrl-check";
const int msg_show_pid = 1;
int msg_debug_bits = 0;

#define LOG_IPS 1

int main(int argc, char* argv[])
{
  const char* ip;
  const char* dir;
  const char* rc;
  long expiry;
  time_t now;
  const char* tmp;
  struct stat s;
  
  if (argc < 2) die1(1, "usage: relay-ctrl-check program [arguments]\n");
  if (getenv("RELAY_CTRL_LOG_IPS") != 0) msg_debug_bits |= LOG_IPS;
  if (getenv("RELAYCLIENT") == 0) {
    expiry = 0;
    if ((tmp = getenv("RELAY_CTRL_EXPIRY")) != 0) expiry = atol(tmp);
    if (expiry <= 0) expiry = 900;
    if ((rc = getenv("RELAY_CTRL_RELAYCLIENT")) == 0) rc = "";
    if ((dir = getenv("RELAY_CTRL_DIR")) == 0)
      warn1("$RELAY_CTRL_DIR is not set.");
    else 
      if ((ip = getenv("TCPREMOTEIP")) != 0) {
	if (chdir(dir) == -1)
	  warn1("Could not change directory.");
	else
	  if (stat(ip, &s) == 0) {
	    now = time(0);
	    if (s.st_mtime + expiry >= now) {
	      debug3(LOG_IPS, "IP ", ip, " found, setting $RELAYCLIENT");
	      setenv("RELAYCLIENT", rc, 1);
	    }
	    else {
	      debug3(LOG_IPS, "Expired IP ", ip, " found, removing");
	      unlink(ip);
	    }
	  }
	  else
	    debug3(LOG_IPS, "IP ", ip, " not found");
      }
      else
	debug1(LOG_IPS, "$TCPREMOTEIP not set, not checking IP");
  }
  else
    debug1(LOG_IPS, "$RELAYCLIENT already set, not checking IP");
  execvp(argv[1], argv+1);
  die1(111, "execution of program failed!\n");
  return 111;
  argc = 0;
}
