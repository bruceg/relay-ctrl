#include <bglibs/sysdeps.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <bglibs/msg.h>
#include <bglibs/ipv4.h>
#include <bglibs/socket.h>
#include "relay-ctrl.h"

const char program[] = "relay-ctrl-udp";
const int msg_show_pid = 0;

int main(void)
{
  const char* tmp;
  int sock;
  unsigned short port;

  if (!do_chdir(0)) return 111;
  
  port = 0;
  if ((tmp = getenv("RELAY_CTRL_PORT")) != 0) port = atoi(tmp);
  if (port <= 0) port = DEFAULT_PORT;
  
  if ((sock = socket_udp()) == -1) die1(111, "Could not create socket.");
  if (socket_bind4(sock, &IPV4ADDR_ANY, port) == -1)
    die1(111, "Could not bind socket.");

  for (;;) {
    char buffer[512];
    int len;
    ipv4addr addr;
    if ((len = socket_recv4(sock, buffer, sizeof buffer - 1,
			    &addr, &port)) != -1) {
      const char* ip;
      buffer[len] = 0;
      if ((ip = validate_ip(buffer)) == 0)
	warn3("Invalid IP from '", ipv4_format(&addr), "'.");
      else if (!touch(ip))
	warn3sys("Could not touch '", ip, "'");
      else {
	buffer[0] = 1;
	socket_send4(sock, buffer, 1, &addr, port);
      }
    }
  }
  return 0;
}
