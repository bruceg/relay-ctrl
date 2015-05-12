#include <bglibs/sysdeps.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <bglibs/msg.h>
#include <bglibs/systime.h>
#include "relay-ctrl.h"

const char program[] = "relay-ctrl-check";
const int msg_show_pid = 1;
int msg_debug_bits = 0;

#define LOG_IPS 1

static const char* rc;
static long expiry;

static void load_env(int fd)
{
  static char buf[8192];
  int rd;
  char* ptr;
  char* end;
  int len;
  
  if ((rd = read(fd, buf, sizeof buf)) == -1)
    warn1sys("Could not read from opened IP file?!?");
  else if (rd > 0) {
    for (ptr = buf; rd > 0; ptr += len, rd -= len) {
      if ((end = memchr(ptr, 0, rd)) == 0) break;
      len = end - ptr + 1;
      if (memchr(ptr, '=', len) != 0)
	if (putenv(ptr) == -1)
	  warn1("Could not set environment string");
    }
  }
}

static void stat_ip(const char* ip)
{
  time_t now;
  struct stat s;
  int fd;
  if ((fd = open(ip, O_RDONLY)) == -1) {
    if (errno == ENOENT)
      debug3(LOG_IPS, "IP ", ip, " not found");
    else
      warn3sys("Could not open IP file '", ip, "'");
  }
  else {
    if (fstat(fd, &s) == -1)
      warn1sys("Could not fstat opened IP file?!?");
    else {
      now = time(0);
      if (s.st_mtime + expiry >= now) {
	debug3(LOG_IPS, "IP ", ip, " found, setting $RELAYCLIENT");
	setenv("RELAYCLIENT", rc, 1);
	load_env(fd);
	close(fd);
      }
      else {
	debug3(LOG_IPS, "Expired IP ", ip, " found, removing");
	unlink(ip);
      }
    }
  }
}

int main(int argc, char* argv[])
{
  const char* ip;
  const char* tmp;
  
  if (argc < 2) die1(1, "usage: relay-ctrl-check program [arguments]\n");
  if (getenv("RELAY_CTRL_LOG_IPS") != 0) msg_debug_bits |= LOG_IPS;
  if (getenv("RELAYCLIENT") == 0) {
    expiry = 0;
    if ((tmp = getenv("RELAY_CTRL_EXPIRY")) != 0) expiry = atol(tmp);
    if (expiry <= 0) expiry = 900;
    if ((rc = getenv("RELAY_CTRL_RELAYCLIENT")) == 0) rc = "";
    if ((ip = getenv("TCPREMOTEIP")) == 0)
      warn1("$TCPREMOTEIP not set, not checking IP");
    else if (do_chdir(0))
      stat_ip(ip);
  }
  else
    debug1(LOG_IPS, "$RELAYCLIENT already set, not checking IP");
  execvp(argv[1], argv+1);
  die1(111, "execution of program failed!\n");
  return 111;
  argc = 0;
}
