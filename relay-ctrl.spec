Name: relay-ctrl
Summary: SMTP Relaying Control for qmail & tcpserver
Version: 2.0
Release: 1
Copyright: GPL
Group: Utilities/System
Source: http://em.ca/~bruceg/relay-ctrl/relay-ctrl-%{PACKAGE_VERSION}.tar.gz
BuildRoot: /tmp/relay-ctrl
URL: http://em.ca/~bruceg/relay-ctrl/
Packager: Bruce Guenter <bruceg@em.ca>
Requires: qmail >= 1.03+patches, ucspi-tcp >= 0.84
Obsoletes: open-smtp

%description
This package allows SMTP relaying for any host that authenticates with POP3.

%prep
%setup

%build
make CFLAGS="$RPM_OPT_FLAGS -DTCPRULES=\\\"/usr/bin/tcprules\\\"" LDFLAGS="-s" prefix=/usr

%install
rm -fr $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/{usr/sbin,var/spool/relay-ctrl}

make prefix=$RPM_BUILD_ROOT/usr install

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
  # Install a root crontab entry
  { EDITOR=cat crontab -e 2>/dev/null;
    echo '0-59/5 * * * * /usr/sbin/relay-ctrl-age' } | crontab -
fi

%preun
if [ "$1" = 0 ]; then
  cpfile=/etc/qmail/control/checkpassword
  cp=`sed -e 's| /usr/sbin/relay-ctrl-allow||' $cpfile`
  echo "$cp" >$cpfile
  # Uninstall the crontab entry
  EDITOR="grep -v ' /usr/sbin/relay-ctrl-age$'" crontab -e 2>/dev/null | \
    crontab -
  rm -f /var/spool/relay-ctrl/*
  /usr/sbin/relay-ctrl-age
fi

%files
%attr(-,root,root) %doc COPYING ChangeLog README NEWS YEAR2000
%attr(0755,root,root) /usr/sbin/relay-ctrl-age
%attr(4711,root,root) /usr/sbin/relay-ctrl-allow
%attr(-,root,root) %dir /var/spool/relay-ctrl
%attr(-,root,root) /usr/man/man8/*.8

