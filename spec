Name: @PACKAGE@
Summary: SMTP Relaying Control for qmail & tcpserver
Version: @VERSION@
Release: 1
Copyright: GPL
Group: Utilities/System
Source: http://em.ca/~bruceg/@PACKAGE@/@PACKAGE@-@VERSION@.tar.gz
BuildRoot: /tmp/@PACKAGE@-root
URL: http://em.ca/~bruceg/@PACKAGE@/
Packager: Bruce Guenter <bruceg@em.ca>
Requires: qmail >= 1.03+patches
Requires: ucspi-tcp >= 0.84
Requires: vixie-cron >= 3.0
Obsoletes: open-smtp

%description
This package allows SMTP relaying for any host that authenticates with POP3.

%prep
%setup

%build
make CFLAGS="$RPM_OPT_FLAGS -DTCPRULES=\\\"/usr/bin/tcprules\\\"" LDFLAGS="-s" prefix=/usr

%install
rm -fr $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/{etc/cron.d,etc/relay-ctrl,usr/sbin,var/spool/relay-ctrl}

make prefix=$RPM_BUILD_ROOT/usr install

echo '* * * * * root /usr/sbin/relay-ctrl-age' >$RPM_BUILD_ROOT/etc/cron.d/relay-ctrl

%clean
rm -rf $RPM_BUILD_ROOT

%post
if [ "$1" = 1 ]; then
  cpfile=/etc/qmail/control/checkpassword
  if [ -f $cpfile ]; then
    cp=`cat $cpfile`
  else
    cp=checkpassword
  fi
  echo "$cp /usr/sbin/relay-ctrl-allow" > $cpfile
fi

%preun
if [ "$1" = 0 ]; then
  cpfile=/etc/qmail/control/checkpassword
  cp=`sed -e 's| /usr/sbin/relay-ctrl-allow||' $cpfile`
  echo "$cp" >$cpfile
  # Remove old relay-ctrl entries
  rm -f /var/spool/relay-ctrl/*
  /usr/sbin/relay-ctrl-age
fi

%files
%defattr(-,root,root)
%doc COPYING ChangeLog README NEWS YEAR2000
%config /etc/cron.d/*
%dir /etc/relay-ctrl
# %config /etc/relay-ctrl/*
/usr/sbin/relay-ctrl-age
%attr(4711,root,root) /usr/sbin/relay-ctrl-allow
%dir /var/spool/relay-ctrl
/usr/man/man8/*

