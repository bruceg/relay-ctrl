prefix	= /usr
sbindir	= $(prefix)/sbin
mandir	= $(prefix)/man
man8dir	= $(mandir)/man8

install	= /usr/bin/install
installdata = $(install) -m 644
installbin = $(install) -m 755
installdir = $(install) -d

progs	= relay-ctrl-allow relay-ctrl-age

version	= 1.3

CC	= gcc
CFLAGS	= -O -Wall -DAGE_CMD='"$(sbindir)/relay-ctrl-age"'
LD	= $(CC)
LDFLAGS	= -g
LIBS	=
RM	= rm -f

all: $(progs)

relay-ctrl-allow: relay-ctrl-allow.o
	$(LD) $(LDFLAGS) -o $@ relay-ctrl-allow.o $(LIBS)

relay-ctrl-age: relay-ctrl-age.o
	$(LD) $(LDFLAGS) -o $@ relay-ctrl-age.o $(LIBS)

install: install.bin install.man

install.bin: $(progs)
	$(installdir) $(sbindir)
	$(installbin) $(progs) $(sbindir)

install.man:
	$(installdir) $(man8dir)
	$(installdata) *.8 $(man8dir)

root-install: install
	$(chmod) u+s $(sbindir)/relay-ctrl-allow

rpmdir	= $(HOME)/redhat

dist: relay-ctrl.spec Makefile
	mkdir relay-ctrl-$(version)
	cp *.c *.h *.8 *.spec COPYING ChangeLog README Makefile YEAR2000 \
		relay-ctrl-$(version)
	tar -czvf relay-ctrl-$(version).tar.gz relay-ctrl-$(version)
	rm -rf relay-ctrl-$(version)
	rpm -ta --clean relay-ctrl-$(version).tar.gz
	mv $(rpmdir)/RPMS/i386/relay-ctrl-$(version)-?.i386.rpm .
	mv $(rpmdir)/SRPMS/relay-ctrl-$(version)-?.src.rpm .
	install -m 444 relay-ctrl-$(version).tar.gz historical
#	scp README YEAR2000 \
#		relay-ctrl-$(version).tar.gz relay-ctrl-$(version)-?.*.rpm \
#		bruceg@em.ca:www/relay-ctrl

clean:
	$(RM) *.o $(progs)
