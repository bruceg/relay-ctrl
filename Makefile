prefix	= /usr
sbindir	= $(prefix)/sbin
mandir	= $(prefix)/man
man8dir	= $(mandir)/man8

install	= /usr/bin/install
installdata = $(install) -m 644
installbin = $(install) -m 755
installdir = $(install) -d

progs	= relay-ctrl-allow relay-ctrl-age

PACKAGE = relay-ctrl
VERSION	= 1.4
distdir = $(PACKAGE)-$(VERSION)

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
	mkdir $(distdir)
	cp *.c *.h *.8 *.spec COPYING ChangeLog README Makefile YEAR2000 \
		$(distdir)
	tar -czvf $(distdir).tar.gz $(distdir)
	rm -rf $(distdir)

rpms: dist
	rpm -ta --clean $(distdir).tar.gz
	mv $(rpmdir)/RPMS/i386/$(distdir)-?.i386.rpm .
	mv $(rpmdir)/SRPMS/$(distdir)-?.src.rpm .

dist-final: dist rpms
	cvs commit
	cvs rtag rel-`echo $(VERSION) | sed -e 's/\./-/g'` $(PACKAGE)
	scp $(distdir).tar.gz $(distdir)-?.*.rpm \
		bruceg@em.ca:www/$(PACKAGE)/
	install -m 444 $(distdir).tar.gz historical

clean:
	$(RM) *.o $(progs)
