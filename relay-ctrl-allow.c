#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "defines.h"

void touch(const char* filename)
{
  int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if(fd >= 0)
    close(fd);
}

void run_age_cmd()
{
  pid_t pid = fork();
  if(pid == 0) {
    const char* devnul = "/dev/null";
    close(0);
    close(1);
    close(2);
    open(devnul, O_RDONLY); /* fd 0 */
    open(devnul, O_WRONLY); /* fd 1 */
    open(devnul, O_WRONLY); /* fd 2 */
    execl(AGE_CMD, AGE_CMD, 0);
    exit(111);
  } else if(pid < 0) {
    perror("fork");
    exit(111);
  }
}

void authenticated(const char* tcpremoteip)
{
  char* filename = malloc(strlen(SPOOLDIR)+strlen(tcpremoteip)+2);
  strcpy(filename, SPOOLDIR);
  strcat(filename, "/");
  strcat(filename, tcpremoteip);
  touch(filename);
  run_age_cmd();
  
  /* Since this program will be either setuid or setgid,
     revoke our priviledges now. */
  setgid(getgid());
  setuid(getuid());
}

size_t writestr(int fd, const char* msg)
{
  return write(fd, msg, strlen(msg));
}
  
const char* validate(const char* str)
{
  /* Security by obscurity -- do certain simple checks on the remote
     IP string and require that both stdin and stdout are sockets.
     Also require that UID and GID are non-zero. */
  const char* ptr;

  /* Skip over a IPv6 address prefix inserted by couriertcpd */
  if(!strncmp(str, "::ffff:", 7))
    str += 7;
  
  /* Ensure that the IP string contains only digits and periods. */
  for(ptr = str; *ptr != 0; ptr++) {
    char ch = *ptr;
    if(ch != '.' && !isdigit(ch))
      return 0;
  }
  return str;
}

enum { imap, pop3 } authmode;

int main(int argc, char* argv[])
{
  struct stat statbuf;
  const char* tcpremoteip = getenv("TCPREMOTEIP");
  if(!tcpremoteip || !(tcpremoteip = validate(tcpremoteip))) {
    writestr(2,
	     "Error: relay-ctrl-allow must be run from tcp-env or tcpserver");
    return 111;
  }
  if(getuid() == 0 || getgid() == 0) {
    writestr(2,
	     "Error: relay-ctrl-allow cannot be run as root");
    return 111;
  }

  /* Check command-line arguments */
  if(argc < 2) {
    writestr(2, "Error: too few command-line arguments to relay-ctrl-allow");
    writestr(2, "usage: relay-ctrl-allow program args...");
    return 111;
  }

  /* Auto-detect the type of program running relay-ctrl-allow */
  if(getenv("AUTHUSER") && getenv("AUTHARGV0") && fstat(3, &statbuf) != -1)
    authmode = imap;
  else
    authmode = pop3;
  
  switch(authmode) {
  case imap: 
      if(getenv("AUTHENTICATED"))
	authenticated(tcpremoteip);
      break;
  case pop3:
    authenticated(tcpremoteip);
    break;
  }
  execvp(argv[1], argv+1);
  return 111;
}
