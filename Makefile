prefix	= /usr
sbindir	= $(prefix)/sbin
mandir	= $(prefix)/man
man8dir	= $(mandir)/man8

install	= /usr/bin/install
installdata = $(install) -m 644
installbin = $(install) -m 755
installdir = $(install) -d
chmod	= chmod

PACKAGE = relay-ctrl
VERSION	= 2.1
distdir = $(PACKAGE)-$(VERSION)

CC	= gcc
CFLAGS	= -O -Wall -DAGE_CMD='"$(sbindir)/relay-ctrl-age"'
LD	= $(CC)
LDFLAGS	= -g
LIBS	=
RM	= rm -f

PROGS	= relay-ctrl-allow relay-ctrl-age
MAN8S	= relay-ctrl-age.8 relay-ctrl-allow.8

all: $(PROGS)

relay-ctrl-allow: relay-ctrl-allow.o
	$(LD) $(LDFLAGS) -o $@ relay-ctrl-allow.o $(LIBS)

relay-ctrl-age: relay-ctrl-age.o
	$(LD) $(LDFLAGS) -o $@ relay-ctrl-age.o $(LIBS)

relay-ctrl-age.o: relay-ctrl-age.c defines.h
relay-ctrl-allow.o: relay-ctrl-allow.c defines.h

install: install.bin install.man

install.bin: $(PROGS)
	$(installdir) $(sbindir)
	$(installbin) $(PROGS) $(sbindir)

install.man:
	$(installdir) $(man8dir)
	$(installdata) $(MAN8S) $(man8dir)

root-install: install
	$(chmod) u+s $(sbindir)/relay-ctrl-allow

clean:
	$(RM) *.o $(PROGS)
