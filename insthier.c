#include "conf_bin.c"
#include "conf_man.c"
#include "installer.h"

void insthier(void) {
  int bin = opendir(conf_bin);
  int man = opendir(conf_man);
  int man8;
  
  c(bin, "relay-ctrl-allow", -1, -1, 0755);
  c(bin, "relay-ctrl-age",   -1, -1, 0755);
  c(bin, "relay-ctrl-check", -1, -1, 0755);
  c(bin, "relay-ctrl-send",  -1, -1, 0755);
  c(bin, "relay-ctrl-udp",   -1, -1, 0755);
  
  man8 = d(man, "man8", -1, -1, 0755);
  c(man8, "relay-ctrl-allow.8", -1, -1, 0644);
  c(man8, "relay-ctrl-age.8",   -1, -1, 0644);
  c(man8, "relay-ctrl-check.8", -1, -1, 0644);
  c(man8, "relay-ctrl-send.8",  -1, -1, 0644);
  c(man8, "relay-ctrl-udp.8",   -1, -1, 0644);
}
