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

#ifndef BUFSIZE
#define BUFSIZE 4096
#endif

const char* relayclient = ":allow,RELAYCLIENT=''\n";
long expiry = 900;
const char* rulesdir = "/etc/tcpcontrol";
const char* smtprules = "smtp.rules";
const char* smtpcdb = "smtp.cdb";
const char* tcprules = "/usr/bin/tcprules";
const char* spooldir = "/var/spool/relay-ctrl";

const char* read_line(const char* filename)
{
  char buf[BUFSIZE];
  char* copy = 0;
  int in = open(filename, O_RDONLY);
  ssize_t rd;
  if(in == -1)
    return 0;
  rd = read(in, buf, BUFSIZE-1);
  if(rd != -1 && rd > 0) {
    char* end = strchr(buf, '\n');
    if(!end)
      end = buf + rd;
    *end = 0;
    copy = malloc(end-buf+1);
    strcpy(copy, buf);
  }
  close(in);
  return copy;
}

int read_config(void)
{
  const char* tmp;
  if(chdir(CONFIGDIR))
    return 0;
  if((tmp = read_line("rule")) != 0)
    relayclient = tmp;
  if((tmp = read_line("tcprules")) != 0)
    tcprules = tmp;
  if((tmp = read_line("spooldir")) != 0)
    spooldir = tmp;
  if((tmp = read_line("expiry")) != 0) {
    char* end;
    expiry = strtol(tmp, &end, 10);
    if(*end || expiry <= 0)
      return 1;
    free((char*)tmp);
  }
  return 0;
}

void touch(const char* filename)
{
  int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if(fd >= 0)
    close(fd);
}

int age_addresses(char** remotes)
{
  DIR* dir;
  time_t oldtime = time(0) - expiry;
  if(chdir(spooldir))
    return 1;
  while(*remotes)
    touch(*remotes++);
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
	write(1, relayclient, strlen(relayclient));
	write(1, "\n", 1);
      }
    }
    closedir(dir);
  }
  return 0;
}

int show_rules(void) 
{
  int fd;
  char buf[BUFSIZE];
  size_t r;
  if(chdir(rulesdir))
    return 1;
  fd = open(smtprules, O_RDONLY);
  if(fd < 0)
    return 1;
  while((r = read(fd, buf, BUFSIZE)) > 0)
    write(1, buf, r);
  close(fd);
  return 0;
}

char* pathjoin(const char* part1, const char* part2)
{
  size_t len1 = strlen(part1);
  size_t len2 = strlen(part2);
  char* s = malloc(len1+len2+1);
  memcpy(s, part1, len1);
  s[len1] = '/';
  memcpy(s+len1+1, part2, len2);
  s[len1+len2+1] = 0;
  return s;
}

char* itoa(char* ptr, unsigned i)
{
  if(i > 10)
    ptr = itoa(ptr, i / 10);
  *ptr++ = i % 10;
  return ptr;
}

char* pathjoinuniq(const char* part1, const char* part2)
{
  size_t len1 = strlen(part1);
  size_t len2 = strlen(part2);
  char* str = malloc(len1+len2+20);
  char* ptr = str;
  memcpy(ptr, part1, len1);
  ptr += len1;
  *ptr++ = '/';
  memcpy(ptr, part2, len2);
  ptr += len2;
  ptr = itoa(ptr, getpid());
  *ptr = 0;
  return str;
}

int child(int fdin, int fdout)
{
  umask(022);
  close(0);
  dup2(fdin, 0);
  close(fdin);
  close(fdout);
  execl(tcprules, tcprules,
	pathjoin(rulesdir, smtpcdb),
	pathjoinuniq(rulesdir, smtpcdb), 0);
  perror(tcprules);
  return 111;
}

int parent(int fdin, int fdout, int pid, char** remotes)
{
  int status;
  close(1);
  dup2(fdout, 1);
  close(fdin);
  close(fdout);
  if(age_addresses(remotes) || show_rules())
    return 1;
  close(1);
  waitpid(pid, &status, WUNTRACED);
  return !WIFEXITED(status) || WEXITSTATUS(status) ? 111 : 0;
}

int main(int argc, char* argv[])
{
  int fd[2];
  pid_t pid;
  if(read_config())
    return 111;
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
    return parent(fd[0], fd[1], pid, argv+1);
  }
}
