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

size_t writestr(int fd, const char* msg)
{
  return write(fd, msg, strlen(msg));
}
  
int validate(const char* str) 
{
  /* Security by obscurity -- do certain simple checks on the remote
     IP string and require that both stdin and stdout are sockets. */
  struct stat statbuf;
  const char* ptr;
  /* Ensure that the IP string contains only digits and periods. */
  for(ptr = str; *ptr != 0; ptr++) {
    char ch = *ptr;
    if(ch != '.' && !isdigit(ch)) return 0;
  }
  /* Ensure that both stdin and stdout are sockets */
  if(fstat(0, &statbuf)) return 0;
  if(!S_ISSOCK(statbuf.st_mode)) return 0;
  if(fstat(1, &statbuf)) return 0;
  if(!S_ISSOCK(statbuf.st_mode)) return 0;
  return 1;
}

int main(int argc, char* argv[])
{
  const char* tcpremoteip = getenv("TCPREMOTEIP");
  if(!tcpremoteip || !validate(tcpremoteip)) {
    writestr(2,
	     "Error: relay-ctrl-allow must be run from tcp-env or tcpserver");
    return 111;
  } else {
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
    
    /* Run the program listed on the command line. */
    execvp(argv[1], argv+1);
    return 111;
  }
}
