#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "msg/msg.h"
#include "net/ipv4.h"
#include "net/socket.h"
#include "relay-ctrl.h"

const char program[] = "relay-ctrl-udp";
const int msg_show_pid = 0;

int main(void)
{
  const char* tmp;
  int sock;
  unsigned short port;

  if ((tmp = getenv("RELAY_CTRL_DIR")) == 0)
    warn1("$RELAY_CTRL_DIR is not set.");
  if (chdir(tmp) == -1)
    die3(111, "Could not change directory to '", tmp, "'.");
  
  port = 0;
  if ((tmp = getenv("RELAY_CTRL_PORT")) != 0) port = atoi(tmp);
  if (port <= 0) port = DEFAULT_PORT;
  
  if ((sock = socket_udp()) == -1) die1(111, "Could not create socket.");
  if (socket_bind4(sock, IPV4ADDR_ANY, port) == -1)
    die1(111, "Could not bind socket.");

  for (;;) {
    char buffer[512];
    int len;
    ipv4addr addr;
    if ((len = socket_recv4(sock, buffer, sizeof buffer - 1,
			    addr, &port)) != -1) {
      const char* ip;
      buffer[len] = 0;
      if ((ip = validate_ip(buffer)) == 0)
	warn3("Invalid IP from '", ipv4_format(addr), "'.");
      else
	if (!touch(ip))
	  warn3sys("Could not touch '", ip, "'");
    }
  }
  return 0;
}
