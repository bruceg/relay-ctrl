#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include "defines.h"

const char* magic1 = ":allow,RELAYCLIENT='";
const char* magic2 = "'\n";
const char* fixup = "";

void read_fixup(void)
{
  char buf[BUFSIZE];
  int in = open(RULESDIR "/" SMTPFIXUP, O_RDONLY);
  ssize_t rd;
  if(in == -1)
    return;
  rd = read(in, buf, BUFSIZE-1);
  if(rd != -1 && rd > 0) {
    char* copy;
    char* end = strchr(buf, '\n');
    if(!end)
      end = buf + rd;
    *end = 0;
    copy = malloc(end-buf+1);
    strcpy(copy, buf);
    fixup = copy;
  }
  close(in);
}

void age_addresses(void) 
{
  DIR* dir;
  time_t oldtime = time(0) - AGE_MINUTES*60;
  if(chdir(SPOOLDIR))
    return;
  dir = opendir(".");
  if(dir) {
    struct stat buf;
    struct dirent* entry;
    while((entry = readdir(dir)) != 0) {
      const char* name = entry->d_name;
      if(name[0] == '.' && (name[1] == 0 ||
			    (name[1] == '.' && name[2] == 0)))
	continue;
      if(stat(name, &buf))
	continue;
      if(!S_ISREG(buf.st_mode) ||
	 buf.st_mtime < oldtime ||
	 buf.st_ctime < oldtime)
	unlink(name);
      else {
	write(1, name, strlen(name));
	write(1, magic1, strlen(magic1));
	write(1, fixup, strlen(fixup));
	write(1, magic2, strlen(magic2));
      }
    }
    closedir(dir);
  }
}

void show_rules(void) 
{
  int fd = open(RULESDIR "/" SMTPRULES, O_RDONLY);
  char buf[BUFSIZE];
  size_t r;
  if(fd < 0) return;
  while((r = read(fd, buf, BUFSIZE)) > 0)
    write(1, buf, r);
  close(fd);
}

int child(int fdin, int fdout)
{
  size_t length = strlen(RULESDIR)+20;
  char* tmpfile = malloc(length+1);
  snprintf(tmpfile, length, "%s/%s.%d", RULESDIR, SMTPCDB, getpid());
  close(0);
  dup2(fdin, 0);
  close(fdin);
  close(fdout);
  execl(TCPRULES, TCPRULES, RULESDIR "/" SMTPCDB, tmpfile, 0);
  perror(TCPRULES);
  return 111;
}

int parent(int fdin, int fdout, int pid)
{
  int status;
  read_fixup();
  close(1);
  dup2(fdout, 1);
  close(fdin);
  close(fdout);
  age_addresses();
  show_rules();
  close(1);
  waitpid(pid, &status, WUNTRACED);
  return !WIFEXITED(status) || WEXITSTATUS(status) ? 111 : 0;
}

int main(void)
{
  int fd[2];
  pid_t pid;
  if(pipe(fd)) {
    perror("pipe");
    return 1;
  }
  pid = fork();
  switch(pid) {
  case -1: /* error occurred */
    perror("fork");
    return 111;
  case 0: /* child process */
    return child(fd[0], fd[1]);
  default: /* parent process */
    return parent(fd[0], fd[1], pid);
  }
}
