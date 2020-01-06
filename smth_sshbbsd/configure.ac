#
# configure.ac
#
# Author: Tatu Ylonen <ylo@cs.hut.fi>
#
# Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
#                    All rights reserved
#
# Created: Wed Mar 22 18:02:48 1995 ylo
#
#

AC_INIT(sshd.c)
AM_INIT_AUTOMAKE(sshbbs, 1.0)
AM_CONFIG_HEADER(config.h)
AC_PREREQ(2.10)

# So many systems seem to need this that it is better do it here automatically.
#LIBS="-L/usr/local/lib $LIBS"

AC_CANONICAL_HOST
AC_MSG_CHECKING(cached information)
hostcheck="$host"
AC_CACHE_VAL(ac_cv_hostcheck, [ ac_cv_hostcheck="$hostcheck" ])
if test "$ac_cv_hostcheck" != "$hostcheck"; then
  AC_MSG_RESULT(changed)
  AC_MSG_WARN(config.cache exists!)
  AC_MSG_ERROR(you must do 'make distclean' first to compile for different host or different parameters.)
else
  AC_MSG_RESULT(ok)
fi

AC_PROG_CC
AC_ISC_POSIX

AC_DEFINE_UNQUOTED(HOSTTYPE, "$host")

case "$host" in
  *-*-sunos4.1.1*)
    os_sunos=yes
    # Tim Adam <tma@osa.com.au> says speed_t is defined in stdtypes.h
    AC_DEFINE(SPEED_T_IN_STDTYPES_H)
    ;;
  *-*-solaris*)
    # solaris stuff. appro@fy.chalmers.se
    AC_DEFINE(SECURE_RPC)
    AC_DEFINE(SECURE_NFS)
    # NIS+ is forced so that we don't have to recompile
    # if we move to NIS+. appro@fy.chalmers.se
    AC_DEFINE(NIS_PLUS)
    ;;
  *-*-sunos*)
    os_sunos=yes
    ;;
  *-sgi-irix5*)
    # Irix stuff from snabb@niksula.hut.fi, tsurmacz@asic.ict.pwr.wroc.pl,
    # C.Martin@sheffield.ac.uk, raistlin@uni-paderborn.de
    no_libsocket=yes
    no_libsun=yes
    no_libnsl=yes
    # force /etc/shadow support. they can run /sbin/pwconv at any time.
    # if they do so, don't let sshd down:-) appro@fy.chalmers.se
    AC_DEFINE(HAVE_ETC_SHADOW)
    no_shadows_password_checking=yes
    ;;
  *-sgi-irix6*)
    # from d-champion@uchicago.edu
    no_libsocket=yes
    no_libnsl=yes
    no_libsun=yes
    if test -z "$GCC";then
        if test "`uname -s`" = "IRIX64"; then
            CFLAGS="-n32 -D_LONG_LONG_LIMB $CFLAGS"
            LDFLAGS="-n32 $LDFLAGS"
        else
            CFLAGS="-n32 $CFLAGS"
            LDFLAGS="-n32 $LDFLAGS"
        fi
    fi
    # force /etc/shadow support. they can run /sbin/pwconv at any time.
    # if they do so, don't let sshd down:-) appro@fy.chalmers.se
    AC_DEFINE(HAVE_ETC_SHADOW)
    no_shadows_password_checking=yes
    # We want support for <proj.h> eivind@ii.uib.no
    AC_TRY_COMPILE([#include <proj.h>],
		[int foo = MAXPROJNAMELEN; ],
		AC_DEFINE(HAVE_SGI_PROJ_H))
    ;;
  *-ibm-aix3.2|*-ibm-aix3.2.0|*-ibm-aix3.2.1|*-ibm-aix3.2.2|*-ibm-aix3.2.3|*-ibm-aix3.2.4)
    os_aix=yes
    AC_CHECK_LIB(s, getuserattr)
    ;;
  *-ibm-aix4.2|*-ibm-aix4.2.*|*-ibm-aix4.3*|*-ibm-aix4.4*)
    no_utmpx=yes
    os_aix=yes
    AC_CHECK_LIB(s, getuserattr)
    ;;
  *-ibm-aix*)
    os_aix=yes
    AC_CHECK_LIB(s, getuserattr)
    ;;
  mips-dec-mach3*)
    # Mach3 stuff from kivinen@hut.fi
    no_vhangup=yes
    no_setsid=yes
    ;;
  *-dec-ultrix*)
    # Ultrix stuff from dmckilli@qc.bell.ca, jbotz@orixa.mtholyoke.edu,
    # corey@cac.washington.edu
    AC_DEFINE(O_NONBLOCK_BROKEN)
    AC_DEFINE(HAVE_ULTRIX_SHADOW_PASSWORDS)
    no_vhangup=yes
    no_utmpx=yes
    no_termios=yes
    # Ultrix shadow passwords implemented in auth-passwd.c.
    no_shadows_password_checking=yes
    AC_CHECK_LIB(auth, authenticate_user)
    AC_TRY_COMPILE([#include <syslog.h>],
		   [int foo = LOG_DAEMON; ],
		   ,
		   AC_DEFINE(NEED_SYS_SYSLOG_H))
    ;;
  *-*-hpux7.*)
    # Kludge for ancient HPUX 7.x (from Nenad Babajic <saca@uis0.uis.kg.ac.yu>)
    AC_DEFINE(HPSUX7_KLUDGES)
    # HPUX flags from jgotts@engin.umich.edu
    if test -z "$GCC"; then
      CFLAGS="$CFLAGS -Aa -D_HPUX_SOURCE"
    fi
    AC_DEFINE(NONSTANDARD_IP_ADDRESS_X11_KLUDGE)
    AC_DEFINE(HPSUX_NONSTANDARD_X11_KLUDGE)
    ;;
  *-*-hpux*)
    # HPUX flags from jgotts@engin.umich.edu
    if test -z "$GCC"; then
      CFLAGS="$CFLAGS -Ae -D_HPUX_SOURCE"
    fi
    AC_MSG_CHECKING(for HPUX tcb auth option)
    if test -f /tcb/files/auth/system/pw_id_map; then
      AC_MSG_RESULT(yes)
      AC_DEFINE(HAVE_HPUX_TCB_AUTH)
      LIBS="$LIBS -lsec"
    else
      AC_MSG_RESULT(no)
    fi
    AC_MSG_CHECKING(for keyserv)
    if test -f /usr/sbin/keyserv; then
      AC_MSG_RESULT(yes)
      AC_DEFINE(SECURE_RPC)
      LIBS="$LIBS -lrpcsvc"
    else
      AC_MSG_RESULT(no)
    fi
    AC_DEFINE(NONSTANDARD_IP_ADDRESS_X11_KLUDGE)
    AC_DEFINE(HPSUX_NONSTANDARD_X11_KLUDGE)
    no_utmpx=yes
    ;;
  alpha-dec-osf*)
    AC_DEFINE(TTY_GROUP, "terminal")
    # The man page says that we need -lsecurity -ldb -laud -lm to quickstart
    # programs using enchanced security. 
    AC_CHECK_LIB(security, set_auth_parameters)
    AC_CHECK_LIB(aud, audgen)
    AC_CHECK_LIB(db, dbopen)
    AC_CHECK_LIB(m, sin)
    AC_CHECK_FUNCS(setluid)
    no_utmpx=yes
    AC_DEFINE(HAVE_SIA)
    SSHDCONFOBJS="$SSHDCONFOBJS sshsia.o"
    OLD_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS -I."
    AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/security.h>
#include <prot.h>], , , [
      AC_MSG_WARN([Could not include <prot.h> file.
The digital unix 4.0 prot.h tries to include <acl.h> instead of <sys/acl.h>.
Making a local copy of prot.h and patching it.])
      sed 's@<acl.h>@<sys/acl.h>@g' </usr/include/prot.h >prot.h
    ])
    CFLAGS="$OLD_CFLAGS"
    ;;
  *-*-nextstep*)
    # Nextstep support from a person who wants to remain anonymous
    no_termios=yes
    if test -f /usr/include/bsd/sys/termios.h; then
      AC_DEFINE(SPEED_T_IN_STDTYPES_H)
    fi
    ;;
  *-*-linux*|*-*-mklinux*)
    CFLAGS="-D_GNU_SOURCE $CFLAGS"
    AC_CHECK_FUNC(getspnam)
    if test $ac_cv_func_getspnam = no; then
       AC_CHECK_LIB(shadow, getspnam)
    fi
    if test $ac_cv_func_getspnam = yes; then
      AC_DEFINE(HAVE_ETC_SHADOW)
    fi
    no_shadows_password_checking=yes
    AC_CHECK_FUNCS(pw_encrypt, pwencrypt=yes)
    if test $ac_cv_func_pw_encrypt = no; then
      AC_CHECK_LIB(shadow, pw_encrypt, [
	pwencrypt=yes
	pwencryptlib="-lshadow" ])
    fi
    AC_MSG_CHECKING([whether to enable pw_encrypt])
    AC_ARG_ENABLE(deprecated-linux-pw-encrypt,
[  --enable-deprecated-linux-pw-encrypt
                          Enable using of deprecated linx pw_encrypt function.],
[     if test -z "$pwencrypt"; then
	AC_MSG_RESULT(no)
      else
	AC_DEFINE(crypt,pw_encrypt)
	AC_MSG_RESULT(no)
	if test -n "$pwencryptlib"; then
	  LIBS="$LIBS $pwencryptlib"
	fi
      fi ],
      AC_MSG_RESULT(no) )
    ;;
  i*86-*-bsdi2.1*)
    no_pipe=yes
    X_EXTRA_LIBS="$X_EXTRA_LIBS -lipc"
    ;;
  i*86-*-bsdi*)
    no_pipe=yes
    ;;
  i*86-unknown-bsd*)
    # Assume 386BSD.  pgut01@cs.auckland.ac.nz reported this makes it compile.
    AC_DEFINE(__FreeBSD__)
    ;;
  mips-sony-newsos6)
    AC_DEFINE(HAVE_NO_TZ_IN_GETTIMEOFDAY)
    ;;
  m68k-sony-newsos*)
    # From snabb@niksula.hut.fi
    no_vhangup=yes
    ;;
  mips-sony-bsd*)
    no_vhangup=yes
    ;;
  m68k-apple-aux*)
    # Macintosh A/UX tweaks - prune LIB = -L/usr/local/lib, exceeds ld capacity
    #   add -lposix to it, needed for termio stuff
    LIBS="$LIBS -lposix"
    AC_DEFINE(_POSIX_SOURCE)
    ;;
  m88k-dg-dgux*)
    AC_DEFINE(BROKEN_INET_ADDR)
    ;;
  t3e-*-*)
    CFLAGS="$CFLAGS -I."
    LIBS="$LIBS -lrsc"
    ;;
  *-cray-unicos*)
    CFLAGS="$CFLAGS -DCRAY_STACKSEG_END=_getb67"
    LIBS="$LIBS -lrsc"
    ;;
  i386-sni-sysv4*)
    AC_DEFINE(NONSTANDARD_IP_ADDRESS_X11_KLUDGE)
    AC_CHECK_LIB(gen, openlog)
    ;;
  *-*-sysv4*)
    AC_CHECK_LIB(gen, openlog)
    ;;
  *-*-machten*)
    AC_DEFINE(USE_STRLEN_FOR_AF_UNIX)
    no_vhangup=yes
    ;;
  i*86-unknown-sco3.2v4*)
    # From moi@dio.com
    AC_DEFINE(HAVE_SCO_ETC_SHADOW)
    AC_DEFINE(SCO)
    no_ranlib=yes
    LIBS="-L/usr/lib/libp -lprot -lx $LIBS"
    CFLAGS="$CFLAGS -UM_I86SM"
    ;;
  i*86-unknown-sco3.2v5*)
    # From brian@ilinx.com
    AC_DEFINE(HAVE_SCO_ETC_SHADOW)
    AC_DEFINE(SCO)
    AC_DEFINE(SCO5)
    no_ranlib=yes
    LIBS="-L/usr/lib/libp -lprot -lx $LIBS"
    CFLAGS="$CFLAGS -UM_I86SM"
    ;;
  *-convex-bsd*)
    # From mark.martinec@nsc.ijs.si
    # On Convex, getpwnam sets pw_passwd if running as root
    no_shadows_password_checking=yes
    ;;
  *-sequent-ptx*)
    # From geek+@cmu.edu
    test_libinet=yes
    ;;
  *-sequent-dynix*|*-sequent-ptx*)
    LIBS="-lseq -lsec $LIBS"
    X_EXTRA_LIBS="-lsocket -linet -lnsl $X_EXTRA_LIBS"
    ;;
  *-*-freebsd*)
    # From Ollivier Robert: FreeBSD and NetBSD use master.passwd, but set
    # pw_passwd automatically when running as root.
    no_shadow_password_checking=yes
    ;;
  *-*-netbsd*|*-*-openbsd*)
    # From Ollivier Robert: FreeBSD and NetBSD use master.passwd, but set
    # pw_passwd automatically when running as root.
    no_shadow_password_checking=yes
    ;;
  *)
    ;;
esac

export CFLAGS CC

# Socket pairs appear to be broken on several systems.  I don't know exactly
# where, so I'll use pipes everywhere for now.
#AC_DEFINE(USE_PIPES)

AC_MSG_CHECKING([that the compiler works])
AC_TRY_RUN([ main(int ac, char **av) { return 0; } ],
    AC_MSG_RESULT(yes),
    AC_MSG_RESULT(no)
    AC_MSG_ERROR(Could not compile and run even a trivial ANSI C program - check CC.),
    AC_MSG_ERROR(Could not compile and run even a trivial ANSI C program - check CC.))

if test -z "$no_pipe"; then
if test -n "$GCC"; then
  AC_MSG_CHECKING([if the compiler understands -pipe])
  OLDCC="$CC"
  CC="$CC -pipe"
  AC_TRY_COMPILE(,,
    AC_MSG_RESULT(yes),
    CC="$OLDCC"
    AC_MSG_RESULT(no))
fi
fi

AC_MSG_CHECKING([whether to enable -Wall])
AC_ARG_ENABLE(warnings,
[  --enable-warnings       Enable -Wall if using gcc.],
[ if test -n "$GCC"; then 
    AC_MSG_RESULT(adding -Wall to CFLAGS.)
    CFLAGS="$CFLAGS -Wall"
  fi],AC_MSG_RESULT(no))

AC_TYPE_SIGNAL
AC_TYPE_SIZE_T
AC_TYPE_UID_T
AC_TYPE_OFF_T
AC_TYPE_MODE_T
AC_STRUCT_ST_BLKSIZE

AC_C_CONST
AC_C_INLINE
AC_C_BIGENDIAN
AC_CHECK_SIZEOF(long,4)
AC_CHECK_SIZEOF(int,4)
AC_CHECK_SIZEOF(short,2)

if test -z "$no_termios"; then
  AC_CHECK_HEADERS(termios.h)
fi

if test -z "$no_utmpx"; then
  AC_CHECK_HEADERS(utmpx.h)
  AC_MSG_CHECKING([whether utmpx have ut_syslen field])
  AC_EGREP_HEADER(ut_syslen, utmpx.h, [ AC_DEFINE(HAVE_SYSLEN_IN_UTMPX)
    AC_MSG_RESULT(yes)], AC_MSG_RESULT(no))
fi

AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(unistd.h rusage.h sys/time.h lastlog.h utmp.h shadow.h)
AC_CHECK_HEADERS(sgtty.h sys/select.h sys/ioctl.h machine/endian.h)
AC_CHECK_HEADERS(paths.h usersec.h utime.h netinet/in_systm.h)
AC_CHECK_HEADERS(netinet/in_system.h netinet/ip.h netinet/tcp.h ulimit.h)
AC_CHECK_HEADERS(sys/resource.h login_cap.h sys/stream.h sys/conf.h)
AC_HEADER_TIME
AC_HEADER_DIRENT
AC_HEADER_STAT
AC_HEADER_MAJOR
AC_MSG_CHECKING([whether utmp have ut_pid field])
AC_EGREP_HEADER(ut_pid, utmp.h, [ AC_DEFINE(HAVE_PID_IN_UTMP)
  AC_MSG_RESULT(yes)], AC_MSG_RESULT(no))
AC_MSG_CHECKING([whether utmp have ut_name field])
AC_EGREP_HEADER(ut_name, utmp.h, [ AC_DEFINE(HAVE_NAME_IN_UTMP)
  AC_MSG_RESULT(yes)], AC_MSG_RESULT(no))
AC_MSG_CHECKING([whether utmp have ut_id field])
AC_EGREP_HEADER(ut_id, utmp.h, [ AC_DEFINE(HAVE_ID_IN_UTMP)
  AC_MSG_RESULT(yes)], AC_MSG_RESULT(no))
AC_MSG_CHECKING([whether utmp have ut_host field])
AC_EGREP_HEADER(ut_host, utmp.h, [ AC_DEFINE(HAVE_HOST_IN_UTMP)
  AC_MSG_RESULT(yes)], AC_MSG_RESULT(no))
AC_MSG_CHECKING([whether utmp have ut_addr field])
AC_EGREP_HEADER(ut_addr, utmp.h, [ AC_DEFINE(HAVE_ADDR_IN_UTMP)
  AC_MSG_RESULT(yes)], AC_MSG_RESULT(no))
AC_MSG_CHECKING([whether you have incompatible SIGINFO macro])
AC_EGREP_CPP([p_siginfo], [#include <sys/siginfo.h>
SIGINFO(p,1)], 
[ AC_DEFINE(HAVE_INCOMPATIBLE_SIGINFO)
  AC_MSG_RESULT(yes)] , AC_MSG_RESULT(no))

AC_CHECK_LIB(sec, getspnam)
AC_CHECK_LIB(seq, get_process_stats)
AC_CHECK_LIB(bsd, bcopy)
if test -z "$no_libnsl"; then
  AC_CHECK_LIB(nsl, main)
fi
if test -n "$test_libinet"; then
  AC_CHECK_LIB(inet, inet_network)
fi
if test -z "$no_libsocket"; then
  AC_CHECK_LIB(socket, socket)
fi
if test -z "$no_libsun"; then
  AC_CHECK_LIB(sun, getpwnam)
fi

if test -z "$no_vhangup"; then
  AC_CHECK_FUNCS(vhangup)
fi

if test -z "$no_setsid"; then
  AC_CHECK_FUNCS(setsid)
fi

AC_CHECK_FUNCS(gettimeofday times getrusage ftruncate revoke makeutx)
AC_CHECK_FUNCS(strchr memcpy setlogin openpty _getpty clock fchmod ulimit)
AC_CHECK_FUNCS(gethostname getdtablesize umask innetgr initgroups setpgrp)
AC_CHECK_FUNCS(setpgid daemon waitpid ttyslot authenticate getpt isastream)

AC_REPLACE_FUNCS(strerror memmove remove random putenv socketpair snprintf)

AC_PROG_LN_S
AC_PROG_INSTALL
AC_CHECK_PROG(AR, ar, ar, echo)
if test -z "$no_ranlib"; then
  AC_PROG_RANLIB
else
  RANLIB=":"
fi
AC_PROGRAMS_CHECK(MAKEDEP, makedepend makedep, makedepend)

AC_MSG_CHECKING(whether to include the IDEA encryption algorithm)
AC_ARG_WITH(idea,
[  --with-idea             Use IDEA (default).
  --without-idea          Don't use IDEA: avoids patent problems in commercial use],
[ case "$withval" in
  no)
    AC_MSG_RESULT(no)
    ;;
  *)
    AC_MSG_RESULT(yes)
    AC_DEFINE(WITH_IDEA)
    CONFOBJS="$CONFOBJS idea.o"
    ;;
  esac ],
#ifndef F_SECURE_COMMERCIAL
  AC_MSG_RESULT(yes)
  AC_DEFINE(WITH_IDEA)
  CONFOBJS="$CONFOBJS idea.o"
#endif F_SECURE_COMMERCIAL
#ifdef F_SECURE_COMMERCIAL
#
#endif F_SECURE_COMMERCIAL
)

AC_MSG_CHECKING(whether to include the Blowfish encryption algorithm)
AC_ARG_WITH(blowfish,
[  --with-blowfish         Include blowfish (default).
  --without-blowfish      Don't include blowfish],
[ case "$withval" in
  no)
    AC_MSG_RESULT(no)
    ;;
  *)
    AC_MSG_RESULT(yes)
    AC_DEFINE(WITH_BLOWFISH)
    ;;
  esac ],
  AC_MSG_RESULT(yes)
  AC_DEFINE(WITH_BLOWFISH)
)

AC_MSG_CHECKING(whether to include the DES encryption algorithm)
AC_ARG_WITH(des,
[  --with-des              Include single-DES support.
  --without-des           Don't allow single DES (default)],
[ case "$withval" in
  yes)
    AC_MSG_RESULT(yes)
    AC_DEFINE(WITH_DES)
    ;;
  *)
    AC_MSG_RESULT(no)
    ;;
  esac ],
  AC_MSG_RESULT(no)
)

AC_MSG_CHECKING(whether to include the ARCFOUR encryption algorithm)
AC_ARG_WITH(arcfour,
[  --with-arcfour          Include arcfour (DO NOT ENABLE, unless you know the security implications of this settings. See README.CIPHERS for more info).
  --without-arcfour       Don't include arcfour (default)],
[ case "$withval" in
  yes)
    AC_MSG_RESULT(yes)
    AC_DEFINE(WITH_ARCFOUR)
    ;;
  *)
    AC_MSG_RESULT(no)
    ;;
  esac ],
  AC_MSG_RESULT(no)
)

AC_MSG_CHECKING(whether to include the none encryption algorithm)
AC_ARG_WITH(none,
[  --with-none             Include support for unencrypted connections
  --without-none          Don't allow unencrypted connections (default)],
[ case "$withval" in
  yes)
    AC_MSG_RESULT(yes)
    AC_DEFINE(WITH_NONE)
    ;;
  *)
    AC_MSG_RESULT(no)
    ;;
  esac ],
  AC_MSG_RESULT(no)
)

AC_MSG_CHECKING(whether to enable TCP_NODELAY)
AC_ARG_ENABLE(tcp-nodelay,
[  --disable-tcp-nodelay   Disable TCP_NODELAY socket option],
[ case "$enableval" in
  no)
       AC_MSG_RESULT(no)
       ;;
  *)
       AC_DEFINE(ENABLE_TCP_NODELAY)
       AC_MSG_RESULT(yes)
  esac ],
  AC_MSG_RESULT(yes)
  AC_DEFINE(ENABLE_TCP_NODELAY)
)

AC_MSG_CHECKING(whether to enable SO_LINGER)
AC_ARG_ENABLE(so-linger,
[  --enable-so-linger      Enable setting SO_LINGER socket option],
[ case "$enableval" in
  yes)
       AC_MSG_RESULT(yes)
       AC_DEFINE(ENABLE_SO_LINGER)
       ;;
  *)
       AC_MSG_RESULT(no)
  esac ],
  AC_MSG_RESULT(no)
)

if test '!' -d ../src; then
	AC_MSG_ERROR([can't find bbs source directory])
fi


AH_TEMPLATE([RSAREF],
			[Define to use RSAREF.])
AH_TEMPLATE([_POSIX_SOURCE],
			[Define if you need to in order for stat and other things to work.])

m4_include(config.lib)
OLDPWD=`pwd`
cd ../src
BBSOBJS=
BBSCFILE="bbs.c record.c delete.c main.c stuff.c mail.c register.c \
           xyz.c talk.c read.c comm_lists.c namecomplete.c chat.c \
           maintain.c edit.c more.c help.c bcache.c boards.c \
           smth_screen.c io.c term.c userinfo.c vote.c announce.c sendmsg.c\
           bm.c list.c goodbye.c bbsgopher.c fileshm.c postheader.c \
           convcode.c five.c tmachine.c addressbook.c\
           backnumber.c common.c sec_hand.c regular.c 1984.c moneycenter.c\
	   editboard.c power_select.c tmpl.c"

for i in $BBSCFILE; do
	AC_CONFIG_LINKS($i:../src/$i)
	BBSOBJS="$BBSOBJS `echo $i|sed 's/.c$/.o/'`"
done
cd $OLDPWD

AC_SUBST(CONFOBJS)
AC_SUBST(SSHINSTALLMODE)
AC_SUBST(BBSOBJS)

AC_OUTPUT(Makefile)
