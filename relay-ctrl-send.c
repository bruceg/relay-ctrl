#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "net/ipv4.h"
#include "net/socket.h"
#include "msg/msg.h"
#include "relay-ctrl.h"
#include "fork.h"
#include "iopoll.h"

const char program[] = "relay-ctrl-send";
const int msg_show_pid = 1;
static char packet[512];
static long packetlen;
static long tries;
static long timeout;
static unsigned short port;

struct remote 
{
  struct remote* next;
  int rcvd;
  unsigned short port;
  ipv4addr addr;
};

static struct remote* parse_remotes(char* str)
{
  struct remote* remotes;
  const char* remote;

  remotes = 0;
  remote = strtok(str, ",");
  while (remote != 0) {
    struct remote* nr;
    if ((nr = malloc(sizeof *nr)) == 0)
      die1(111, "Could not allocate remote structure.");
    nr->next = remotes;
    if (!ipv4_parse(remote, nr->addr, &remote))
      warn3("Could not parse IP '", remote, "'.");
    else {
      nr->rcvd = 0;
      nr->port = port;
      remotes = nr;
    }
    remote = strtok(0, ",");
  }
  return remotes;
}

static void send_one(struct remote* remote, int sock)
{
  for (; remote != 0; remote = remote->next)
    if (!remote->rcvd)
      if (socket_send4(sock, packet, packetlen,
		       remote->addr, remote->port) != packetlen)
	warn3("Sending IP to '", ipv4_format(remote->addr), "' failed.");
}

static int recv_wait(struct remote* remotes, int sock)
{
  iopoll_fd io;
  io.fd = sock;
  io.events = IOPOLL_READ;
  while (iopoll(&io, 1, timeout*1000) == 1) {
    ipv4addr addr;
    unsigned short port;
    char buf[1];
    int len;
    int done;
    if ((len = socket_recv4(sock, buf, 1, addr, &port)) == 1) {
      struct remote* r;
      for (done = 1, r = remotes; r != 0; r = r->next) {
	if (memcmp(r->addr, addr, 4) == 0)
	  r->rcvd = 1;
	if (!r->rcvd) done = 0;
      }
      if (done) return 1;
    }
  }
  return 0;
}

static void send_ip(char* str)
{
  int sock;
  struct remote* remotes;
  struct remote* r;
  
  if ((remotes = parse_remotes(str)) == 0) return;
  if ((sock = socket_udp()) == -1)
    die1(111, "Could not create UDP socket.");
  if (socket_bind4(sock, IPV4ADDR_ANY, 0) == -1)
    die1(111, "Could not bind UDP socket.");

  for (; tries > 0; --tries) {
    send_one(remotes, sock);
    if (recv_wait(remotes, sock)) break;
  }
  for (r = remotes; r != 0; r = r->next)
    if (!r->rcvd)
      warn3("Timed out waiting for response from '",
	    ipv4_format(r->addr), "'.");
}

int make_packet(const char* ip)
{
  if ((packetlen = strlen(ip)) > (long)sizeof packet) return 0;
  memcpy(packet, ip, packetlen);
  return 1;
}

int main(int argc, char* argv[])
{
  char* remotes;
  const char* tmp;

  if (argc < 2) die1(111, "usage: relay-ctrl-send program [args ...]");
  if ((tmp = getenv("TCPREMOTEIP")) == 0 || (tmp = validate_ip(tmp)) == 0)
    die1(111, "Must be run from tcp-env or tcpserver.");
  if (!make_packet(tmp))
    die1(111, "Failed to build packet data");
  strcpy(packet, tmp);
  packetlen = strlen(packet);
  
  port = 0;
  if ((tmp = getenv("RELAY_CTRL_PORT")) != 0) port = atoi(tmp);
  if (port <= 0) port = DEFAULT_PORT;

  tries = 0;
  if ((tmp = getenv("RELAY_CTRL_TRIES")) != 0) tries = atoi(tmp);
  if (tries <= 0) tries = 5;
  
  timeout = 0;
  if ((tmp = getenv("RELAY_CTRL_TIMEOUT")) != 0) timeout = atoi(tmp);
  if (timeout <= 0) timeout = 1;
  
  if ((remotes = getenv("RELAY_CTRL_REMOTES")) == 0)
    warn1("$RELAY_CTRL_REMOTES is not set.");
  else 
    if (is_authenticated()) {
      switch (fork()) {
      case -1:
	warn1("Could not fork.");
	break;
      case 0: /* child */
	send_ip(remotes);
	return 0;
      }
    }
  execvp(argv[1], argv+1);
  return 111;
}
