Name:     	libiconv
Version: 	1.8
Release:	ytht
Copyright:	LGPL
URL:		http://www.gnu.org/software/libiconv/
Source:		%{name}-%{version}.tar.gz
Summary:	libiconv - character set conversion library
Group:		System Environment/Libraries
Buildroot: %{_tmppath}/%{name}-%{version}-root

%description
This library provides an iconv() implementation, for use on systems which don't have one, or whose
implementation cannot convert from/to Unicode. 

%files
%defattr(-, root, root)
/usr/lib/lib*.so.*
/usr/lib/libiconv_plug.so
#/usr/bin/iconv
#/usr/man/man1/*


%package -n libiconv-devel
Summary:	Development files for libiconv
Group:		Development/Libraries
Requires:	%{name} = %{PACKAGE_VERSION}

%description -n libiconv-devel
This package provides the development files required to compile programs that use the iconv character conversion library

%files -n libiconv-devel
%defattr(-, root, root)
/usr/lib/libiconv.so
/usr/lib/libcharset.so
/usr/lib/lib*.a
/usr/lib/lib*.la
/usr/include/*
#/usr/man/man3/*


# $RPM_COMMAND is an environment variable used by the Ximian build
# system to control the build process with finer granularity than RPM
# normally allows.  This specfile will function as expected by RPM if
# $RPM_COMMAND is unset.  If you are not the Ximian build system,
# feel free to ignore it.

%prep
case "${RPM_COMMAND:-all}" in
dist)
%setup  -q -D -n %{name}-%{version}
    ;;
all)
%setup  -q -n %{name}-%{version}
    ;;
esac

%build
MAKE=${MAKE:-make}
RPM_COMMAND=${RPM_COMMAND:-all}
DESTDIR=${DESTDIR:-"$RPM_BUILD_ROOT"}
ARCH=%{_target_platform}
export MAKE RPM_COMMAND DESTDIR ARCH
case "$RPM_COMMAND" in
prepare|all)
    ./configure --prefix=/usr --enable-static=yes
    ;;
esac
case "$RPM_COMMAND" in
clean|all)
    if [ "/" != "$DESTDIR" ]; then
	rm -rf "$DESTDIR"
    fi
    ;;
esac
case "$RPM_COMMAND" in
build|all)
    ${MAKE}
    ;;
esac

%install
MAKE=${MAKE:-make}
DESTDIR=${DESTDIR:-"$RPM_BUILD_ROOT"}
# export DESTDIR
case "${RPM_COMMAND:-all}" in
install|all)
    ${MAKE} install prefix=${DESTDIR}/usr
    ;;
esac

%clean
DESTDIR=${DESTDIR:-"$RPM_BUILD_ROOT"}
export DESTDIR
case "${RPM_COMMAND:-all}" in
clean|all)
    if [ "/" != "$DESTDIR" ]; then
	rm -rf "$DESTDIR"
    fi
    ;;
esac


%changelog
* Tue Sep 10 2002 lepton

- Version: 1.8-ytht
- Imported into ytht system.
