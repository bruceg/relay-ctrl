Name: @PACKAGE@
Summary: SMTP Relaying Control for qmail & tcpserver
Version: @VERSION@
Release: 1
Copyright: GPL
Group: Utilities/System
Source: http://untroubled.org/@PACKAGE@/@PACKAGE@-@VERSION@.tar.gz
BuildRoot: %{_tmppath}/@PACKAGE@-root
URL: http://untroubled.org/@PACKAGE@/
Packager: Bruce Guenter <bruceg@em.ca>
Requires: qmail >= 1.03+patches
Requires: vixie-cron >= 3.0
Obsoletes: open-smtp

%description
This package allows SMTP relaying for any host that authenticates with
POP3 or IMAP.

%prep
%setup

%build
echo "gcc %{optflags}" >conf-cc
echo "gcc %{optflags} -s" >conf-ld
make programs

%install
rm -fr %{buildroot}
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_mandir}
mkdir -p %{buildroot}/etc/cron.d
mkdir -p %{buildroot}/etc/relay-ctrl
mkdir -p -m 700 %{buildroot}/var/spool/relay-ctrl
mkdir -p -m 777 %{buildroot}/var/spool/relay-ctrl/allow

echo %{buildroot}%{_bindir} >conf-bin
echo %{buildroot}%{_mandir} >conf-man
make installer instcheck
./installer
./instcheck

pushd %{buildroot}/etc/relay-ctrl
  echo /var/spool/relay-ctrl/allow >RELAY_CTRL_DIR
  echo 900 >RELAY_CTRL_EXPIRY
  echo >RELAY_CTRL_RELAYCLIENT
  echo 811 >RELAY_CTRL_PORT
popd

echo '* * * * * root /usr/local/bin/envdir /etc/relay-ctrl %{_bindir}/relay-ctrl-age' \
	>%{buildroot}/etc/cron.d/relay-ctrl

%clean
rm -rf %{buildroot}

#%post
#if [ "$1" = 1 ]; then
#  cpfile=/etc/qmail/control/checkpassword
#  if [ -f $cpfile ]; then
#    cp=`cat $cpfile`
#  else
#    cp=checkpassword
#  fi
#  echo "$cp %{_bindir}/relay-ctrl-allow" > $cpfile
#fi

#%preun
#if [ "$1" = 0 ]; then
#  cpfile=/etc/qmail/control/checkpassword
#  cp=`sed -e 's| %{_bindir}/relay-ctrl-allow||' $cpfile`
#  echo "$cp" >$cpfile
#  # Remove old relay-ctrl entries
#  rm -f /var/spool/relay-ctrl/*
#fi

%files
%defattr(-,root,root)
%doc COPYING README NEWS
%config(noreplace) /etc/cron.d/*
%dir /etc/relay-ctrl
%config(noreplace) /etc/relay-ctrl/*
%{_bindir}/*
%{_mandir}/man8/*
%dir /var/spool/relay-ctrl
%dir /var/spool/relay-ctrl/allow

