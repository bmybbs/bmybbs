/*

sshd.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Fri Mar 17 17:09:28 1995 ylo

This program is the ssh daemon.  It listens for connections from clients, and
performs authentication, executes use commands or shell, and forwards
information to/from the application to the user client over an encrypted
connection.  This can also handle forwarding of X11, TCP/IP, and authentication
agent connections.

*/

/*
 * $Id: sshd.c,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: sshd.c,v $
 * Revision 1.1.1.1  2009-03-04 06:33:27  bmybbs
 * bmysrc
 *
 * Revision 1.1.1.1  2003/05/11 11:48:10  clearboy
 * update on 20051031
 * by clearboy 
 * for transfering the source codes from main site to the experimental site 
 * for the first time.
 *
 *
 * Revision 1.9  2003/05/11 11:48:10  yuhuan
 * ssh 也用 smth_screen.c
 *
 * Revision 1.8  2003/04/18 16:44:12  yuhuan
 * ssh多行，copy from bad
 * 还有分区笔误一处
 *
 * Revision 1.7  2003/04/18 15:00:21  lepton
 * logattempt统一处理
 *
 * Revision 1.6  2003/02/22 07:45:44  yuhuan
 * 想清楚了再做ssh的relogin
 *
 * Revision 1.5  2003/02/21 17:26:33  yuhuan
 * sshd版本的relogin，现在只是简单的再次调用bbs_entry函数
 * 没有重新exec，或者是否应使用setjmp ?
 *
 * Revision 1.4  2002/11/10 10:36:26  lepton
 * sshbbsd退出时候清理一下垃圾
 *
 * Revision 1.3  2002/10/01 14:32:16  lepton
 * 1.加入sshbbsd配置文件
 * 2.现在密码验证工作正常
 * 3.可能退出的一些环境清理有问题 以及执行外部程序比如穿梭
 *
 * Revision 1.2  2002/10/01 11:50:26  lepton
 * 现在sshbbsd倒是在我这里能运行了
 * 不过还远没有实用...
 * 需要加入密码验证 外部程序处理 ip显示等等
 * 目前只能演示一下而已
 *
 * Revision 1.1.1.1  2002/10/01 09:42:06  ylsdd
 * 水木底sshbbsd导入
 * 然后慢慢改吧
 *
 * Revision 1.4  2002/08/04 11:39:44  kcn
 * format c
 *
 * Revision 1.3  2002/08/04 11:08:49  kcn
 * format C
 *
 * Revision 1.2  2002/05/25 02:06:27  kcn
 * do sshbbsd autoconf
 *
 * Revision 1.1.1.1  2002/04/27 05:47:26  kxn
 * no message
 *
 * Revision 1.4  2002/04/25 10:47:37  kxn
 * removed libBBS.a
 * fixed logattempt failure
 * added username display in proctitle
 *
 * Revision 1.3  2002/04/25 05:37:26  kxn
 * bugs fixed: disconnect, chinese ime
 * features added : run as normal user
 *
 * Revision 1.2  2001/08/13 13:39:03  bbsdev
 * fix modetype.c to site.c
 *
 * Revision 1.1  2001/07/04 06:07:13  bbsdev
 * bbs sshd
 *
 * Revision 1.61  1999/04/29 11:29:47  tri
 * 	Added a syslog call.
 *
 * Revision 1.60  1999/04/29 07:52:29  tri
 *      Replaced OSF1/C2 security support with more complete SIA
 *         (Security Integration Architecture) support by Tom Woodburn.
 *
 * Revision 1.59  1999/02/25 06:25:37  tri
 *      Added an unnecessary but illustrative patch.
 *
 * Revision 1.58  1999/02/22 14:44:33  kivinen
 *      Added code that will check that environment will always be
 *      allocated (freebsd + use login).
 *
 * Revision 1.57  1999/02/22 08:14:10  tri
 *      Final fixes for 1.2.27.
 *
 * Revision 1.56  1999/02/21 19:52:56  ylo
 *      Intermediate commit of ssh1.2.27 stuff.
 *      Main change is sprintf -> snprintf; however, there are also
 *      many other changes.
 *
 * Revision 1.55  1998/07/08 14:55:22  tri
 *      Fixed version negotiation so, that ssh 2
 *      compatibility is even remotedly possible.
 *
 * Revision 1.54  1998/07/08 00:48:46  kivinen
 *      Added better HPUX TCB auth support. Added SGI proj support.
 *      Changed to use match_host in the allow/deny checking. Changed
 *      to use PASSWD_PATH. Added checking that if allow/deny group is
 *      set then the group must exists.
 *
 * Revision 1.53  1998/06/11 00:11:24  kivinen
 *      Added ENABLE_SO_LINGER ifdef. Added username to /bin/password
 *      commands. Added user@host support.
 *
 * Revision 1.52  1998/05/23  20:28:12  kivinen
 *      Changed () -> (void). Added HAVE_OSF1_C2_SECURITY include
 *      files. Added days_before_{account,password}_expires support.
 *      Added chalnecho TIS authentication server response code
 *      support. Added call to osf1c2_check_account_and_terminal
 *      function. Added SSH_BINDIR to path read from
 *      /etc/default/login. Fixed BSDI login_getclass code for BSDI
 *      2.1.
 *
 * Revision 1.51  1998/05/11  18:51:07  kivinen
 *      Fixed AIX authstate code.
 *
 * Revision 1.50  1998/04/30 01:58:40  kivinen
 *      Fixed osflim handling so that now it allows setting resource
 *      to 0. Added -V option (for ssh version 2 compat mode). Added
 *      LIBWRAP code to also when in debugging mode. Added BSDI
 *      setusercontext code.
 *
 * Revision 1.49  1998/04/17 00:42:36  kivinen
 *      Freebsd login capabilities support. Added REMOTEUSER
 *      environment variable setting. Changed locked account checking
 *      so that it will not care if the account is locked if
 *      kerberos_or_local_password is not set. Added nologin-allow
 *      support. Added setting of AUTHSTATE and KRB5CCNAME enviroment
 *      variables if AIX authenticate() function is used.
 *
 * Revision 1.48  1998/03/27 17:05:01  kivinen
 *      Added SIGDANGER code. Fixed kerberos initialization code so
 *      ssh will check the error codes of initialization function.
 *      added ignore_root_rhosts code. Moved initgroups before closing
 *      all filedescriptors.
 *
 * Revision 1.47  1998/01/03 06:42:43  kivinen
 *      Added allow/deny groups option support.
 *
 * Revision 1.46  1998/01/02 06:39:36  kivinen
 *      Added new mail checking. Added expiration checkind for bsdi,
 *      and warning when password is about to expire. Fixed kerberos
 *      ticket name handling. Added support for XAuthLocation option.
 *      Added support for login capabilities for bsdi, only support
 *      ignorelogin option.
 *      Added osfc2 resource limit setting.
 *
 * Revision 1.45  1997/10/01 19:16:32  ylo
 *      Clarified error message about xauth not being in path.
 *
 * Revision 1.44  1997/05/08 03:06:51  kivinen
 *      Fixed sighup handling (added select before accept, changed
 *      execv to execvp so sshd is searched from path).
 *
 * Revision 1.43  1997/04/27 21:51:11  kivinen
 *      Added F-SECURE stuff. Added {Allow,Deny}Forwarding{To,Port}
 *      feature. Added {Allow,Deny}Users feature from Steve Kann
 *      <stevek@SteveK.COM>.
 *
 * Revision 1.42  1997/04/23 00:05:35  kivinen
 *      Added ifdefs around password expiration and inactivity checks,
 *      because some systems dont have sp_expire and sp_inact fields.
 *
 * Revision 1.41  1997/04/21 01:05:56  kivinen
 *      Added waitpid loop to main_sigchld_handler if we have it.
 *      Added check to pty_cleanup_proc so it will not cleanup pty
 *      twice.
 *      Changed argument to server_loop from ttyname to
 *      cleanup_context.
 *
 * Revision 1.40  1997/04/17 04:04:58  kivinen
 *      Removed extra variable err.
 *
 * Revision 1.39  1997/04/17 04:04:13  kivinen
 *      Added fatal: to all errors that cause sshd to exit.
 *      Added resetting of SIGCHLD before running libwrap code.
 *      Moved pty/pipe closing to server_loop. Added ttyname argument
 *      to server_loop.
 *      Server_loop will also now release the pty if it is allocated.
 *
 * Revision 1.38  1997/04/05 22:03:38  kivinen
 *      Added check that userfile_get_des_1_magic_phrase succeeded,
 *      before using the passphrase. Moved closing of pty after the
 *      pty_release.
 *
 * Revision 1.37  1997/04/05 17:28:31  ylo
 *      Added a workaround for the Windows SSH problem with X11
 *      forwarding.
 *
 * Revision 1.36  1997/03/27 05:59:50  kivinen
 *      Fixed bug in HAVE_USERSEC_H code.
 *
 * Revision 1.35  1997/03/27 03:12:22  kivinen
 *      Added kerberos patches from Glenn Machin.
 *      Added USELOGIN patches from Brian Cully.
 *
 * Revision 1.34  1997/03/26 05:32:42  kivinen
 *      Added idle_timeout variable.
 *      If debug_flag is given set rsa to verbose.
 *      Changed uid 0 to bee UID_ROOT.
 *
 * Revision 1.33  1997/03/25 05:48:29  kivinen
 *      Implemented SilentDeny and umask options. Added HAVE_DAEMON
 *      support.
 *      Moved LIBWRAP code to child.
 *      Moved closing of sockets/pipes out from server_loop.
 *
 * Revision 1.32  1997/03/19 23:04:43  kivinen
 *      Fixed typo.
 *
 * Revision 1.31  1997/03/19 21:17:57  kivinen
 *      Added some errno printing to all fatal calls.
 *      Added SSH_ORIGINAL_COMMAND environment variable setting. It
 *      will have the original command from the network when using
 *      forced command. It can be used to get arguments for forced
 *      command.
 *
 * Revision 1.30  1997/03/19 19:25:57  kivinen
 *      Added input buffer clearing for error conditions, so packet.c
 *      can check that buffer must be empty before new packet is read
 *      in.
 *
 * Revision 1.29  1997/03/19 17:53:17  kivinen
 *      Added more ETC_SHADOW support and SECURE_RPC, SECURE_NFS and
 *      NIS_PLUS support from Andy Polyakov <appro@fy.chalmers.se>.
 *      Added TIS authentication code from Andre April
 *      <Andre.April@cediti.be>.
 *      Moved authentication fail loop to do_authentication_fail_loop
 *      function. Added checks that username isn't longer than 255
 *      characters.
 *      Changed do_authentication to get cipher_type, so it can
 *      disable RhostsRsa authentication if using unsecure cipher
 *      (NONE, or ARCFOUR).
 *      Changed order of environment variables set to child, because
 *      digital unixes telnet dumps core if USER is the first
 *      environment variable set.
 *      Added code that will set all ip-address to xauth so it should
 *      work for multihosted machines too. Dont use xauth add
 *      host/unix:0 on crays, because it complains about it. Patch
 *      from Arne Henrik Juul <arnej@imf.unit.no>
 *
 * Revision 1.28  1996/11/24 08:26:15  kivinen
 *      Added SSHD_NO_{PORT,X11}_FORWARDING support.
 *
 * Revision 1.27  1996/11/04 06:35:01  ylo
 *      Updated processing of check_emulation output.
 *
 * Revision 1.26  1996/10/29 22:46:25  kivinen
 *      log -> log_msg. Added old agent emulation code (disable agent
 *      forwarding if the other end is too old).
 *
 * Revision 1.25  1996/10/23 15:59:13  ttsalo
 *       Changed BINDIR's name to SSH_BINDIR to prevent conflicts
 *
 * Revision 1.24  1996/10/21 16:35:23  ttsalo
 *       Removed some fd auth code
 *
 * Revision 1.23  1996/10/21 16:18:34  ttsalo
 *       Had to remove BINDIR from line 2518
 *
 * Revision 1.22  1996/10/20 16:19:36  ttsalo
 *      Added global variable 'original_real_uid' and it's initialization
 *
 * Revision 1.20  1996/09/27 17:19:16  ylo
 *      Merged ultrix patches from Corey Satten.
 *
 * Revision 1.19  1996/09/22 22:38:49  ylo
 *      Added endgrent() before closing all file descriptors.
 *
 * Revision 1.18  1996/09/08 17:40:31  ttsalo
 *      BSD4.4Lite's _PATH_DEFPATH is checked when defining DEFAULT_PATH.
 *      (Patch from Andrey A. Chernov <ache@lsd.relcom.eu.net>)
 *
 * Revision 1.17  1996/08/29 14:51:23  ttsalo
 *      Agent-socket directory handling implemented
 *
 * Revision 1.16  1996/08/22 22:16:24  ylo
 *      Log remote commands executed by root, and log the fact that a
 *      remote command was executed by an ordinary user, but not the
 *      actual command (for privacy reasons).
 *
 * Revision 1.15  1996/08/16 02:47:18  ylo
 *      Log root logins at LOG_NOTICE.
 *
 * Revision 1.14  1996/08/13 09:04:23  ttsalo
 *      Home directory, .ssh and .ssh/authorized_keys are now
 *      checked for wrong owner and group & world writeability.
 *
 * Revision 1.13  1996/08/13 00:23:31  ylo
 *      When doing X11 forwarding, check the existence of xauth and
 *      deny forwarding if it doesn't exist.  This makes copying
 *      binaries compiled on one system to other systems easier.
 *
 *      Run /etc/sshrc with /bin/sh instead of the user's shell.
 *
 * Revision 1.12  1996/07/29 04:58:54  ylo
 *      Add xauth data also for `hostname`/unix:$display as some X
 *      servers actually seem to use this version.  (Kludge to work
 *      around X11 bug.)
 *
 * Revision 1.11  1996/07/15 23:21:55  ylo
 *      Don't allow more than five password authentication attempts,
 *      and log attempts after the first one.
 *
 * Revision 1.10  1996/07/12 07:28:02  ttsalo
 *      Small ultrix patch
 *
 * Revision 1.9  1996/06/05 17:57:34  ylo
 *      If /etc/nologin exists, print that fact in plain text before
 *      printing the actual contents.  I am getting too many
 *      complaints about it.
 *
 * Revision 1.8  1996/06/03 19:25:49  ylo
 *      Fixed a typo.
 *
 * Revision 1.7  1996/05/29 07:41:46  ylo
 *      Added arguments to userfile_init.
 *
 * Revision 1.6  1996/05/29 07:16:38  ylo
 *      Disallow any user names that start with a '-' or '+' (or '@',
 *      just to be sure).  There is some indication that getpw* might
 *      returns such names on some systems with NIS.  Ouuuch!
 *
 * Revision 1.5  1996/05/28 16:41:14  ylo
 *      Merged Cray patches from Wayne Schroeder.
 *      Use setsid instead of setpgrp on ultrix.
 *
 * Revision 1.4  1996/04/26 00:22:51  ylo
 *      Improved error messages related to reading host key.
 *      Fixed ip addr in "Closing connection" message.
 *
 * Revision 1.3  1996/04/22 23:49:47  huima
 * Changed protocol version to 1.4, added calls to emulate module.
 *
 * Revision 1.2  1996/02/18  21:49:51  ylo
 *      Moved userfile_uninit to proper place.
 *      Use setluid if it exists (at least OSF/1).
 *
 * Revision 1.1.1.1  1996/02/18 21:38:13  ylo
 *      Imported ssh-1.2.13.
 *
 * Revision 1.31  1995/10/02  01:28:59  ylo
 *      Include sys/syslog.h if NEED_SYS_SYSLOG_H.
 *      Print proper ETCDIR in usage().
 *
 * Revision 1.30  1995/09/27  02:54:43  ylo
 *      Fixed a minor error.
 *
 * Revision 1.29  1995/09/27  02:49:06  ylo
 *      Fixed syntax errors.
 *
 * Revision 1.28  1995/09/27  02:18:51  ylo
 *      Added support for SCO unix.
 *      Added support for .hushlogin.
 *      Read $HOME/.environment.
 *      Pass X11 proto and cookie in stdin instead of command line.
 *      Added support for $HOME/.ssh/rc and /etc/sshrc.
 *
 * Revision 1.27  1995/09/25  00:03:53  ylo
 *      Added screen number.
 *      Don't display motd and login time if executing a command.
 *
 * Revision 1.26  1995/09/22  22:22:34  ylo
 *      Fixed a bug in the new environment code.
 *
 * Revision 1.25  1995/09/21  17:16:49  ylo
 *      Fixes to libwrap code.
 *      Fixed problem in wait() in key regeneration.  Now only
 *      ackquires light noise at regeneration.
 *      Support for ignore_rhosts.
 *      Don't use X11 forwarding with spoofing if no xauth.
 *      Rewrote the code to initialize the environment in the child.
 *      Added code to read /etc/environment into child environment.
 *      Fixed setpcred argument type.
 *
 * Revision 1.24  1995/09/11  17:35:53  ylo
 *      Added libwrap support.
 *      Log daemon name without path.
 *
 * Revision 1.23  1995/09/10  23:43:32  ylo
 *      Added a newline in xauth message.
 *
 * Revision 1.22  1995/09/10  23:29:43  ylo
 *      Renamed sigchld_handler main_sigchld_handler to avoid
 *      conflict.
 *
 * Revision 1.21  1995/09/10  23:26:53  ylo
 *      Child xauth line printed with fprintf instead of debug().
 *
 * Revision 1.20  1995/09/10  22:43:17  ylo
 *      Added uid-swapping stuff.
 *      Moved do_session to serverloop.c and renamed it server_loop.
 *      Changed SIGCHLD handling.
 *      Merged OSF/1 C2 security stuff.
 *
 * Revision 1.19  1995/09/09  21:26:47  ylo
 * /m/shadows/u2/users/ylo/ssh/README
 *
 * Revision 1.18  1995/09/06  19:53:19  ylo
 *      Fixed spelling of fascist.
 *
 * Revision 1.17  1995/09/06  16:02:40  ylo
 *      Added /usr/bin/X11 to default DEFAULT_PATH.
 *      Fixed inetd_flag & debug_flag together.
 *      Fixed -i.
 *
 * Revision 1.16  1995/08/31  09:43:14  ylo
 *      Fixed LOGNAME.
 *
 * Revision 1.15  1995/08/31  09:26:22  ylo
 *      Copy struct pw.
 *      Use socketpairs for communicating with the shell/command.
 *      Use same socket for stdin and stdout. (may help rdist)
 *      Put LOGNAME in environment.
 *      Run xauth directly, without the shell in between.
 *      Fixed the HPSUX kludge.
 *
 * Revision 1.14  1995/08/29  22:36:12  ylo
 *      Added SIGHUP handling.  Added SIGTERM and SIGQUIT handling.
 *      Permit root login if forced command.
 *      Added DenyHosts, AllowHosts.  Added PrintMotd.
 *      New file descriptor code.
 *      Use HPSUX and SIGCHLD kludges only on HPUX.
 *
 * Revision 1.13  1995/08/22  14:06:11  ylo
 *      Added /usr/local/bin in default DEFAULT_PATH.
 *
 * Revision 1.12  1995/08/21  23:33:48  ylo
 *      Added "-f conffile" option.
 *      Added support for the server configuration file.
 *      Added allow/deny host code.
 *      Added code to optionally deny root logins.
 *      Added code to configure allowed authentication methods.
 *      Changes to log initialization arguments.
 *      Eliminated NO_RHOSTS_AUTHENTICATION.
 *
 * Revision 1.11  1995/08/18  22:58:06  ylo
 *      Added support for O_NONBLOCK_BROKEN.
 *      Added support for TTY_GROUP.
 *
 * Revision 1.10  1995/07/27  02:19:09  ylo
 *      Tell packet_set_encryption_key that we are the server.
 *
 *      Temporary kludge to make TCP/IP port forwarding work
 *      properly.  This kludge will increase idle CPU usage because
 *      sshd wakes up every 300ms.
 *
 * Revision 1.9  1995/07/27  00:41:34  ylo
 *      If DEFAULT_PATH defined by configure, use that value.
 *
 * Revision 1.8  1995/07/26  23:21:06  ylo
 *      Removed include version.h.  Added include mpaux.h.
 *
 *      Print software version with -d.
 *
 *      Added support for protocol version 1.1.  Fixes minor security
 *      problems, and updates the protocol to match the draft RFC.
 *      Compatibility code makes it possible to use old clients with
 *      this server.
 *
 * Revision 1.7  1995/07/16  01:01:41  ylo
 *      Removed hostname argument from record_logout.
 *      Added call to pty_release.
 *      Set tty mode depending on whether we have tty group.
 *
 * Revision 1.6  1995/07/15  22:27:04  ylo
 *      Added printing of /etc/motd.
 *
 * Revision 1.5  1995/07/15  21:41:04  ylo
 *      Changed the HPSUX kludge (child_has_terminated).  It caused
 *      sshd to busy-loop if the program exited but there were open
 *      connections.
 *
 * Revision 1.4  1995/07/14  23:37:43  ylo
 *      Limit outgoing packet size to 512 bytes for interactive
 *      connections.
 *
 * Revision 1.3  1995/07/13  17:33:17  ylo
 *      Only record the pid in /etc/sshd_pid if running without the
 *      debugging flag.
 *
 * Revision 1.2  1995/07/13  01:40:47  ylo
 *      Removed "Last modified" header.
 *      Added cvs log.
 *
 * $Endlog$
 */
#include "bbs.h"
#include "includes.h"
#include <gmp.h>
#include "xmalloc.h"
#include "rsa.h"
#include "ssh.h"
#include "packet.h"
#include "buffer.h"
#include "cipher.h"
#include "mpaux.h"
#include "servconf.h"
#include "userfile.h"
#include "emulate.h"

#ifdef HAVE_ULIMIT_H
#include <ulimit.h>
#endif                          /* HAVE_ULIMIT_H */
#ifdef HAVE_ETC_SHADOW
#ifdef HAVE_SHADOW_H
#include <shadow.h>
#endif                          /* HAVE_SHADOW_H */
#ifndef SHADOW
#define SHADOW "/etc/shadow"
#endif
#endif                          /* HAVE_ETC_SHADOW */


#ifdef LIBWRAP
#include <tcpd.h>
#include <syslog.h>
#ifdef NEED_SYS_SYSLOG_H
#include <sys/syslog.h>
#endif                          /* NEED_SYS_SYSLOG_H */
int allow_severity = LOG_INFO;
int deny_severity = LOG_WARNING;
#endif                          /* LIBWRAP */


#ifdef _PATH_BSHELL
#define DEFAULT_SHELL           _PATH_BSHELL
#else
#define DEFAULT_SHELL           "/bin/sh"
#endif

#ifndef DEFAULT_PATH
#ifdef _PATH_USERPATH
#define DEFAULT_PATH            _PATH_USERPATH
#else
#ifdef _PATH_DEFPATH
#define DEFAULT_PATH            _PATH_DEFPATH
#else
#define DEFAULT_PATH    "/bin:/usr/bin:/usr/ucb:/usr/bin/X11:/usr/local/bin"
#endif
#endif
#endif                          /* DEFAULT_PATH */

#ifndef O_NOCTTY
#define O_NOCTTY        0
#endif

/* Server configuration options. */
ServerOptions options;

/* Name of the server configuration file. */
char *config_file_name = SERVER_CONFIG_FILE;

/* Debug mode flag.  This can be set on the command line.  If debug
   mode is enabled, extra debugging output will be sent to the system
   log, the daemon will not go to background, and will exit after processing
   the first connection. */
int debug_flag = 0;

/* Flag indicating that the daemon is being started from inetd. */
int inetd_flag = 0;

/* argv[0] without path. */
char *av0;

/* Saved arguments to main(). */
char **saved_argv;

/* This is set to the socket that the server is listening; this is used in
   the SIGHUP signal handler. */
int listen_sock;

/* This is not really needed, and could be eliminated if server-specific
   and client-specific code were removed from newchannels.c */
uid_t original_real_uid = 0;

/* Flags set in auth-rsa from authorized_keys flags.  These are set in
  auth-rsa.c. */
int no_port_forwarding_flag = 0;
int no_agent_forwarding_flag = 0;
int no_x11_forwarding_flag = 0;
int no_pty_flag = 0;
time_t idle_timeout = 0;
char *forced_command = NULL;    /* RSA authentication "command=" option. */
char *original_command = NULL;  /* original command from protocol. */
struct envstring *custom_environment = NULL;

                          /* RSA authentication "environment=" options. */

/* Session id for the current session. */
unsigned char session_id[16];

/* Any really sensitive data in the application is contained in this structure.
   The idea is that this structure could be locked into memory so that the
   pages do not get written into swap.  However, there are some problems.
   The private key contains MP_INTs, and we do not (in principle) have
   access to the internals of them, and locking just the structure is not
   very useful.  Currently, memory locking is not implemented. */
struct {
    /* Random number generator. */
    RandomState random_state;

    /* Private part of server key. */
    RSAPrivateKey private_key;

    /* Private part of host key. */
    RSAPrivateKey host_key;
} sensitive_data;

/* Flag indicating whether the current session key has been used.  This flag
   is set whenever the key is used, and cleared when the key is regenerated. */
int key_used = 0;

/* This is set to true when SIGHUP is received. */
int received_sighup = 0;

/* Public side of the server key.  This value is regenerated regularly with
   the private key. */
RSAPublicKey public_key;

/* Remote end username (mallocated) or NULL if not available */
char *remote_user_name;

/* Days before the password / account expires, or -1 if information not
   available */
int days_before_account_expires = -1;
int days_before_password_expires = -1;

/* Prototypes for various functions defined later in this file. */
void do_connection(int privileged_port);
void do_authentication(char *user, int privileged_port, int cipher_type);
void do_authenticated(char *pw);
void do_exec_no_pty(const char *command, char *pw, const char *display, const char *auth_proto, const char *auth_data, int quick_login);
void do_child(const char *command, char *pw, const char *term, const char *display, const char *auth_proto, const char *auth_data, const char *ttyname);


/* Signal handler for SIGHUP.  Sshd execs itself when it receives SIGHUP;
   the effect is to reread the configuration file (and to regenerate
   the server key). */

RETSIGTYPE sighup_handler(int sig)
{
    received_sighup = 1;
    signal(SIGHUP, sighup_handler);
}

/* Called from the main program after receiving SIGHUP.  Restarts the 
   server. */

void sighup_restart(void)
{
    log_msg("Received SIGHUP; restarting.");
    close(listen_sock);
    execvp(saved_argv[0], saved_argv);
    log_msg("RESTART FAILED: av[0]='%.100s', error: %.100s.", saved_argv[0], strerror(errno));
    exit(1);
}

/* Generic signal handler for terminating signals in the master daemon. 
   These close the listen socket; not closing it seems to cause "Address
   already in use" problems on some machines, which is inconvenient. */

RETSIGTYPE sigterm_handler(int sig)
{
    log_msg("Received signal %d; terminating.", sig);
    close(listen_sock);
    exit(255);
}

#ifdef SIGDANGER
/* Signal handler for AIX's SIGDANGER low-memory signal
   It logs the signal and ignores the message. */
RETSIGTYPE sigdanger_handler(int sig)
{
    log_msg("Received signal %d (SIGDANGER, means memory is low); ignoring.", sig);
}
#endif                          /* SIGDANGER */

/* SIGCHLD handler.  This is called whenever a child dies.  This will then 
   reap any zombies left by exited c. */

RETSIGTYPE main_sigchld_handler(int sig)
{
    int status;

#ifdef HAVE_WAITPID
    /* Reap all childrens */
    while (waitpid(-1, &status, WNOHANG) > 0);
#else
    wait(&status);
#endif
    signal(SIGCHLD, main_sigchld_handler);
}

/* Signal handler for the alarm after the login grace period has expired. */

RETSIGTYPE grace_alarm_handler(int sig)
{
    /* Close the connection. */
    packet_close();

    /* Log error and exit. */
    fatal_severity(SYSLOG_SEVERITY_INFO, "Timeout before authentication.");
}

/* Signal handler for the key regeneration alarm.  Note that this
   alarm only occurs in the daemon waiting for connections, and it does not
   do anything with the private key or random state before forking.  Thus there
   should be no concurrency control/asynchronous execution problems. */

RETSIGTYPE key_regeneration_alarm(int sig)
{
    /* Check if we should generate a new key. */
    if (key_used) {
        /* This should really be done in the background. */
        log_msg("Generating new %d bit RSA key.", options.server_key_bits);
        random_acquire_light_environmental_noise(&sensitive_data.random_state);
        rsa_generate_key(&sensitive_data.private_key, &public_key, &sensitive_data.random_state, options.server_key_bits);
        random_save(&sensitive_data.random_state, geteuid(), options.random_seed_file);
        key_used = 0;
        log_msg("RSA key generation complete.");
    }

    /* Reschedule the alarm. */
    signal(SIGALRM, key_regeneration_alarm);
    alarm(options.key_regeneration_time);
}

/* Main program for the daemon. */

int main(int ac, char **av)
{
    extern char *optarg;
    extern int optind;
    int opt, sock_in, sock_out, newsock, i, pid, on = 1;
    socklen_t aux;
    int remote_major, remote_minor;
    int perm_denied = 0;
    int ret;
    fd_set fdset;
    //struct sockaddr_in sin;  
    struct sockaddr_in6 sin;    /* ipv6 */
    char buf[100];              /* Must not be larger than remote_version. */
    char remote_version[100];   /* Must be at least as big as buf. */
    char *comment;
    char *ssh_remote_version_string = NULL;
    FILE *f;

#if defined(SO_LINGER) && defined(ENABLE_SO_LINGER)
    struct linger linger;
#endif                          /* SO_LINGER */
    int done;
    int quick_login = 0;
    chdir(MY_BBS_HOME);
    /* Save argv[0]. */
    saved_argv = av;
    if (strchr(av[0], '/'))
        av0 = strrchr(av[0], '/') + 1;
    else
        av0 = av[0];

    /* Prevent core dumps to avoid revealing sensitive information. */
    signals_prevent_core();

    /* Set SIGPIPE to be ignored. */
    signal(SIGPIPE, SIG_IGN);

    /* Initialize configuration options to their default values. */
    initialize_server_options(&options);

    /* Parse command-line arguments. */
    while ((opt = getopt(ac, av, "f:p:b:k:h:g:diqV:")) != EOF) {
        switch (opt) {
        case 'f':
            config_file_name = optarg;
            break;
        case 'd':
            debug_flag = 1;
            break;
        case 'i':
            inetd_flag = 1;
            break;
        case 'q':
            options.quiet_mode = 1;
            break;
        case 'b':
            options.server_key_bits = atoi(optarg);
            break;
        case 'p':
            options.port = atoi(optarg);
            break;
        case 'g':
            options.login_grace_time = atoi(optarg);
            break;
        case 'k':
            options.key_regeneration_time = atoi(optarg);
            break;
        case 'h':
            options.host_key_file = optarg;
            break;
        case 'V':
            ssh_remote_version_string = optarg;
            break;
        case '?':
        default:
#ifdef F_SECURE_COMMERCIAL

#endif                          /* F_SECURE_COMMERCIAL */
            fprintf(stderr, "sshd version %s [%s]\n", SSH_VERSION, HOSTTYPE);
            fprintf(stderr, "Usage: %s [options]\n", av0);
            fprintf(stderr, "Options:\n");
            fprintf(stderr, "  -f file    Configuration file (default %s/sshd_config)\n", ETCDIR);
            fprintf(stderr, "  -d         Debugging mode\n");
            fprintf(stderr, "  -i         Started from inetd\n");
            fprintf(stderr, "  -q         Quiet (no logging)\n");
            fprintf(stderr, "  -p port    Listen on the specified port (default: 22)\n");
            fprintf(stderr, "  -k seconds Regenerate server key every this many seconds (default: 3600)\n");
            fprintf(stderr, "  -g seconds Grace period for authentication (default: 300)\n");
            fprintf(stderr, "  -b bits    Size of server RSA key (default: 768 bits)\n");
            fprintf(stderr, "  -h file    File from which to read host key (default: %s)\n", HOST_KEY_FILE);
            fprintf(stderr, "  -V str     Remote version string already read from the socket\n");
            exit(1);
        }
    }

    /* Read server configuration options from the configuration file. */
    read_server_config(&options, config_file_name);

    /* Fill in default values for those options not explicitly set. */
    fill_default_server_options(&options);

    /* Check certain values for sanity. */
    if (options.server_key_bits < 512 || options.server_key_bits > 32768) {
        fprintf(stderr, "fatal: Bad server key size.\n");
        exit(1);
    }
    if (options.port < 1 || options.port > 65535) {
        fprintf(stderr, "fatal: Bad port number.\n");
        exit(1);
    }
    if (options.umask != -1) {
        umask(options.umask);
    }

    /* Check that there are no remaining arguments. */
    if (optind < ac) {
        fprintf(stderr, "fatal: Extra argument %.100s.\n", av[optind]);
        exit(1);
    }

    /* Initialize the log (it is reinitialized below in case we forked). */
    log_init(av0, debug_flag && !inetd_flag, debug_flag || options.fascist_logging, options.quiet_mode, options.log_facility);

    debug("sshd version %.100s [%.100s]", SSH_VERSION, HOSTTYPE);

    /* Load the host key.  It must have empty passphrase. */
    done = load_private_key(geteuid(), options.host_key_file, "", &sensitive_data.host_key, &comment);

    if (!done) {
        if (debug_flag) {
            fprintf(stderr, "Could not load host key: %.200s\n", options.host_key_file);
            fprintf(stderr, "fatal: Please check that you have sufficient permissions and the file exists.\n");
        } else {
            log_init(av0, !inetd_flag, 1, 0, options.log_facility);
            error("fatal: Could not load host key: %.200s.  Check path and permissions.", options.host_key_file);
        }
        exit(1);
    }
    xfree(comment);

    /* If not in debugging mode, and not started from inetd, disconnect from
       the controlling terminal, and fork.  The original process exits. */
    if (!debug_flag && !inetd_flag)
#ifdef HAVE_DAEMON
        if (daemon(0, 0) < 0)
            error("daemon: %.100s", strerror(errno));
    chdir(MY_BBS_HOME);
#else                           /* HAVE_DAEMON */
    {
#ifdef TIOCNOTTY
        int fd;
#endif                          /* TIOCNOTTY */

        /* Fork, and have the parent exit.  The child becomes the server. */
        if (fork())
            exit(0);

        /* Redirect stdin, stdout, and stderr to /dev/null. */
        freopen("/dev/null", "r", stdin);
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        /* Disconnect from the controlling tty. */
#ifdef TIOCNOTTY
        fd = open("/dev/tty", O_RDWR | O_NOCTTY);
        if (fd >= 0) {
            (void) ioctl(fd, TIOCNOTTY, NULL);
            close(fd);
        }
#endif                          /* TIOCNOTTY */
#ifdef HAVE_SETSID
#ifdef ultrix
        setpgrp(0, 0);
#else                           /* ultrix */
        if (setsid() < 0)
            error("setsid: %.100s", strerror(errno));
#endif
#endif                          /* HAVE_SETSID */
    }
#endif                          /* HAVE_DAEMON */

    /* Reinitialize the log (because of the fork above). */
    log_init(av0, debug_flag && !inetd_flag, debug_flag || options.fascist_logging, options.quiet_mode, options.log_facility);

    /* Check that server and host key lengths differ sufficiently.  This is
       necessary to make double encryption work with rsaref.  Oh, I hate
       software patents. */
    if (options.server_key_bits > sensitive_data.host_key.bits - SSH_KEY_BITS_RESERVED && options.server_key_bits < sensitive_data.host_key.bits + SSH_KEY_BITS_RESERVED) {
        options.server_key_bits = sensitive_data.host_key.bits + SSH_KEY_BITS_RESERVED;
        debug("Forcing server key to %d bits to make it differ from host key.", options.server_key_bits);
    }

    /* Initialize memory allocation so that any freed MP_INT data will be
       zeroed. */
    rsa_set_mp_memory_allocation();

    /* Do not display messages to stdout in RSA code. */
    rsa_set_verbose(debug_flag);

    /* Initialize the random number generator. */
    debug("Initializing random number generator; seed file %.200s", options.random_seed_file);
    random_initialize(&sensitive_data.random_state, geteuid(), options.random_seed_file);

    /* Chdir to the root directory so that the current disk can be unmounted
       if desired. */

    idle_timeout = options.idle_timeout;

    /* Start listening for a socket, unless started from inetd. */
    if (inetd_flag) {
        int s1, s2;

        s1 = dup(0);            /* Make sure descriptors 0, 1, and 2 are in use. */
        s2 = dup(s1);
        sock_in = dup(0);
        sock_out = dup(1);
        /* We intentionally do not close the descriptors 0, 1, and 2 as our
           code for setting the descriptors won\'t work if ttyfd happens to
           be one of those. */
        debug("inetd sockets after dupping: %d, %d", sock_in, sock_out);

        /* Generate an rsa key. */
        log_msg("Generating %d bit RSA key.", options.server_key_bits);
        rsa_generate_key(&sensitive_data.private_key, &public_key, &sensitive_data.random_state, options.server_key_bits);
        random_save(&sensitive_data.random_state, geteuid(), options.random_seed_file);
        log_msg("RSA key generation complete.");
    } else {
        /* Create socket for listening. */
        listen_sock = socket(AF_INET6, SOCK_STREAM, 0); //ipv6
        if (listen_sock < 0)
            fatal("socket: %.100s", strerror(errno));

        /* Set socket options.  We try to make the port reusable and have it
           close as fast as possible without waiting in unnecessary wait states
           on close. */
        setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof(on));
#if defined(SO_LINGER) && defined(ENABLE_SO_LINGER)
        linger.l_onoff = 1;
        linger.l_linger = 15;
        setsockopt(listen_sock, SOL_SOCKET, SO_LINGER, (void *) &linger, sizeof(linger));
#endif                          /* SO_LINGER */

        /* Initialize the socket address. */
        memset(&sin, 0, sizeof(sin));
	/*
        sin.sin_family = AF_INET;
        sin.sin_addr = options.listen_addr;
        sin.sin_port = htons(options.port);
	*/
        sin.sin6_family = AF_INET6;
        //sin.sin6_addr = options.listen_addr;
	memcpy(&sin.sin6_addr, &options.listen_addr,sizeof(struct in6_addr));
        sin.sin6_port = htons(options.port);
        /* Bind the socket to the desired port. */
        if (bind(listen_sock, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
            error("bind: %.100s", strerror(errno));
            shutdown(listen_sock, 2);
            close(listen_sock);
            fatal("Bind to port %d failed: %.200s.", options.port, strerror(errno));
        }
        /* COMMAN : setuid to bbs */

        setreuid(BBSUID, BBSUID);
        setregid(BBSGID, BBSGID);

        if (!debug_flag) {
            /* Record our pid in /etc/sshd_pid to make it easier to kill the
               correct sshd.  We don\'t want to do this before the bind above
               because the bind will fail if there already is a daemon, and this
               will overwrite any old pid in the file. */
            f = fopen(options.pid_file, "w");
            if (f) {
                fprintf(f, "%u\n", (unsigned int) getpid());
                fclose(f);
            }
        }

        /* Start listening on the port. */
        log_msg("Server listening on port %d.", options.port);
        if (listen(listen_sock, 5) < 0)
            fatal("listen: %.100s", strerror(errno));

        /* Generate an rsa key. */
        log_msg("Generating %d bit RSA key.", options.server_key_bits);
        rsa_generate_key(&sensitive_data.private_key, &public_key, &sensitive_data.random_state, options.server_key_bits);
        random_save(&sensitive_data.random_state, geteuid(), options.random_seed_file);
        log_msg("RSA key generation complete.");

        /* Schedule server key regeneration alarm. */
        signal(SIGALRM, key_regeneration_alarm);
        alarm(options.key_regeneration_time);

        /* Arrange to restart on SIGHUP.  The handler needs listen_sock. */
        signal(SIGHUP, sighup_handler);
        signal(SIGTERM, sigterm_handler);
        signal(SIGQUIT, sigterm_handler);

        /* AIX sends SIGDANGER when memory runs low.  The default action is
           to terminate the process.  This sometimes makes it difficult to
           log in and fix the problem. */

#ifdef SIGDANGER
        signal(SIGDANGER, sigdanger_handler);
#endif                          /* SIGDANGER */

        /* Arrange SIGCHLD to be caught. */
        signal(SIGCHLD, main_sigchld_handler);

        /* Stay listening for connections until the system crashes or the
           daemon is killed with a signal. */
        for (;;) {
            if (received_sighup)
                sighup_restart();

            /* Wait in select until there is a connection. */
            FD_ZERO(&fdset);
            FD_SET(listen_sock, &fdset);
            ret = select(listen_sock + 1, &fdset, NULL, NULL, NULL);
            if (ret < 0 || !FD_ISSET(listen_sock, &fdset)) {
                if (errno == EINTR)
                    continue;
                error("select: %.100s", strerror(errno));
                continue;
            }

            aux = sizeof(sin);
            newsock = accept(listen_sock, (struct sockaddr *) &sin, &aux);
            if (newsock < 0) {
                if (errno == EINTR)
                    continue;
                error("accept: %.100s", strerror(errno));
                continue;
            }

            /* Got connection.  Fork a child to handle it, unless we are in
               debugging mode. */
            if (debug_flag) {
                /* In debugging mode.  Close the listening socket, and start
                   processing the connection without forking. */
                debug("Server will not fork when running in debugging mode.");
                close(listen_sock);
                sock_in = newsock;
                sock_out = newsock;
                pid = getpid();
#ifdef LIBWRAP
                {
                    struct request_info req;

                    signal(SIGCHLD, SIG_DFL);

                    request_init(&req, RQ_DAEMON, av0, RQ_FILE, newsock, NULL);
                    fromhost(&req);
                    if (!hosts_access(&req))
                        refuse(&req);
                    syslog(allow_severity, "connect from %s", eval_client(&req));
                }
#endif                          /* LIBWRAP */
                break;
            } else {
                /* Normal production daemon.  Fork, and have the child process
                   the connection.  The parent continues listening. */
                if ((pid = fork()) == 0) {
                    /* Child.  Close the listening socket, and start using
                       the accepted socket.  Reinitialize logging (since our
                       pid has changed).  We break out of the loop to handle
                       the connection. */
                    close(listen_sock);
                    sock_in = newsock;
                    sock_out = newsock;
#ifdef LIBWRAP
                    {
                        struct request_info req;

                        signal(SIGCHLD, SIG_DFL);

                        request_init(&req, RQ_DAEMON, av0, RQ_FILE, newsock, NULL);
                        fromhost(&req);
                        if (!hosts_access(&req))
                            refuse(&req);
                        syslog(allow_severity, "connect from %s", eval_client(&req));
                    }
#endif                          /* LIBWRAP */

                    log_init(av0, debug_flag && !inetd_flag, options.fascist_logging || debug_flag, options.quiet_mode, options.log_facility);
                    break;
                }
            }

            /* Parent.  Stay in the loop. */
            if (pid < 0)
                error("fork: %.100s", strerror(errno));
            else
                debug("Forked child %d.", pid);

            /* Mark that the key has been used (it was "given" to the child). */
            key_used = 1;

            random_acquire_light_environmental_noise(&sensitive_data.random_state);

            /* Close the new socket (the child is now taking care of it). */
            close(newsock);
        }
    }

    /* This is the child processing a new connection. */

    /* Disable the key regeneration alarm.  We will not regenerate the key
       since we are no longer in a position to give it to anyone.  We will
       not restart on SIGHUP since it no longer makes sense. */
    alarm(0);
    signal(SIGALRM, SIG_DFL);
    signal(SIGHUP, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);

    /* Set socket options for the connection.  We want the socket to close
       as fast as possible without waiting for anything.  If the connection
       is not a socket, these will do nothing. */
    /* setsockopt(sock_in, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on)); */
#if defined(SO_LINGER) && defined(ENABLE_SO_LINGER)
    linger.l_onoff = 1;
    linger.l_linger = 15;
    setsockopt(sock_in, SOL_SOCKET, SO_LINGER, (void *) &linger, sizeof(linger));
#endif                          /* SO_LINGER */

    /* Register our connection.  This turns encryption off because we do not
       have a key. */
    packet_set_connection(sock_in, sock_out, &sensitive_data.random_state);

    /* Log the connection. */
    log_msg("Connection from %.100s port %d", get_remote_ipaddr(), get_remote_port());

    /* Check whether logins are denied from this host. */
    {
        const char *ipaddr = get_remote_ipaddr();
        int i;

	if(checkbansite(ipaddr))
		perm_denied=1;
	else if (options.num_deny_hosts > 0) {
            for (i = 0; i < options.num_deny_hosts; i++)
                if (match_host(ipaddr, ipaddr, options.deny_hosts[i]))
                    perm_denied = 1;
        }
        if ((!perm_denied) && options.num_allow_hosts > 0) {
            for (i = 0; i < options.num_allow_hosts; i++)
                if (match_host(ipaddr, ipaddr, options.allow_hosts[i]))
                    break;
            if (i >= options.num_allow_hosts)
                perm_denied = 1;
        }
        if (perm_denied && options.silent_deny) {
            close(sock_in);
            close(sock_out);
            exit(0);
        }
    }

    /* We don't want to listen forever unless the other side successfully
       authenticates itself.  So we set up an alarm which is cleared after
       successful authentication.  A limit of zero indicates no limit.
       Note that we don't set the alarm in debugging mode; it is just annoying
       to have the server exit just when you are about to discover the bug. */
    signal(SIGALRM, grace_alarm_handler);
    if (!debug_flag)
        alarm(options.login_grace_time);


    if (ssh_remote_version_string == NULL) {
        /* Send our protocol version identification. */
        snprintf(buf, sizeof(buf), "SSH-%d.%d-%.50s", PROTOCOL_MAJOR, PROTOCOL_MINOR, SSH_VERSION);
        strcat(buf, "\n");
        if (write(sock_out, buf, strlen(buf)) != strlen(buf))
            fatal_severity(SYSLOG_SEVERITY_INFO, "Could not write ident string.");
    }

    if (ssh_remote_version_string == NULL) {
        /* Read other side\'s version identification. */
        for (i = 0; i < sizeof(buf) - 1; i++) {
            if (read(sock_in, &buf[i], 1) != 1)
                fatal_severity(SYSLOG_SEVERITY_INFO, "Did not receive ident string.");
            if (buf[i] == '\r') {
                buf[i] = '\n';
                buf[i + 1] = 0;
                break;
            }
            if (buf[i] == '\n') {
                /* buf[i] == '\n' */
                buf[i + 1] = 0;
                break;
            }
        }
        buf[sizeof(buf) - 1] = 0;
    } else {
        strncpy(buf, ssh_remote_version_string, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = 0;
    }

    /* Check that the versions match.  In future this might accept several
       versions and set appropriate flags to handle them. */
    if (sscanf(buf, "SSH-%d.%d-%[^\n]\n", &remote_major, &remote_minor, remote_version) != 3) {
        const char *s = "Protocol mismatch.\n";

        (void) write(sock_out, s, strlen(s));
        close(sock_in);
        close(sock_out);
        fatal_severity(SYSLOG_SEVERITY_INFO, "Bad protocol version identification: %.100s", buf);
    }
    debug("Client protocol version %d.%d; client software version %.100s", remote_major, remote_minor, remote_version);

    switch (check_emulation(remote_major, remote_minor, NULL, NULL)) {
    case EMULATE_MAJOR_VERSION_MISMATCH:
        {
            const char *s = "Protocol major versions differ.\n";

            (void) write(sock_out, s, strlen(s));
            close(sock_in);
            close(sock_out);
            fatal_severity(SYSLOG_SEVERITY_INFO, "Protocol major versions differ: %d vs. %d", PROTOCOL_MAJOR, remote_major);
        }
        break;
    case EMULATE_VERSION_TOO_OLD:
        packet_disconnect("Your ssh version is too old and is no " "longer supported.  Please install a newer version.");
        break;
    case EMULATE_VERSION_NEWER:
        packet_disconnect("This server does not support your " "new ssh version.");
        break;
    case EMULATE_VERSION_OK:
        break;
    default:
        fatal("Unexpected return value from check_emulation.");
    }

    if (perm_denied) {
        const char *hostname = get_remote_ipaddr();

        log_msg("Connection from %.200s not allowed.\n", hostname);
        packet_disconnect("Sorry, you are not allowed to connect.");
     /*NOTREACHED*/}

    packet_set_nonblocking();

    /* Handle the connection.   We pass as argument whether the connection
       came from a privileged port. */
    do_connection(get_remote_port() < 1024);

    /* The connection has been terminated. */
    log_msg("Closing connection to %.100s", get_remote_ipaddr());
    packet_close();
    exit(0);
}

/* Process an incoming connection.  Protocol version identifiers have already
   been exchanged.  This sends server key and performs the key exchange.
   Server and host keys will no longer be needed after this functions. */

void do_connection(int privileged_port)
{
    int i;
    MP_INT session_key_int;
    unsigned char session_key[SSH_SESSION_KEY_LENGTH];
    unsigned char check_bytes[8];
    char *user;
    unsigned int cipher_type, auth_mask, protocol_flags;

    /* Generate check bytes that the client must send back in the user packet
       in order for it to be accepted; this is used to defy ip spoofing 
       attacks.  Note that this only works against somebody doing IP spoofing 
       from a remote machine; any machine on the local network can still see 
       outgoing packets and catch the random cookie.  This only affects
       rhosts authentication, and this is one of the reasons why it is
       inherently insecure. */
    for (i = 0; i < 8; i++)
        check_bytes[i] = random_get_byte(&sensitive_data.random_state);

    /* Send our public key.  We include in the packet 64 bits of random
       data that must be matched in the reply in order to prevent IP spoofing. */
    packet_start(SSH_SMSG_PUBLIC_KEY);
    for (i = 0; i < 8; i++)
        packet_put_char(check_bytes[i]);

    /* Store our public server RSA key. */
    packet_put_int(public_key.bits);
    packet_put_mp_int(&public_key.e);
    packet_put_mp_int(&public_key.n);

    /* Store our public host RSA key. */
    packet_put_int(sensitive_data.host_key.bits);
    packet_put_mp_int(&sensitive_data.host_key.e);
    packet_put_mp_int(&sensitive_data.host_key.n);

    /* Put protocol flags. */
    packet_put_int(SSH_PROTOFLAG_HOST_IN_FWD_OPEN);

    /* Declare which ciphers we support. */
    packet_put_int(cipher_mask());

    /* Declare supported authentication types. */
    auth_mask = 0;
    if (options.rhosts_authentication)
        auth_mask |= 1 << SSH_AUTH_RHOSTS;
    if (options.rhosts_rsa_authentication)
        auth_mask |= 1 << SSH_AUTH_RHOSTS_RSA;
    if (options.rsa_authentication)
        auth_mask |= 1 << SSH_AUTH_RSA;
    if (options.password_authentication)
        auth_mask |= 1 << SSH_AUTH_PASSWORD;
    packet_put_int(auth_mask);

    /* Send the packet and wait for it to be sent. */
    packet_send();
    packet_write_wait();

    debug("Sent %d bit public key and %d bit host key.", public_key.bits, sensitive_data.host_key.bits);

    /* Read clients reply (cipher type and session key). */
    packet_read_expect(SSH_CMSG_SESSION_KEY);

    /* Get cipher type. */
    cipher_type = packet_get_char();

    /* Get check bytes from the packet.  These must match those we sent earlier
       with the public key packet. */
    for (i = 0; i < 8; i++)
        if (check_bytes[i] != packet_get_char())
            packet_disconnect("IP Spoofing check bytes do not match.");

    debug("Encryption type: %.200s", cipher_name(cipher_type));

    /* Get the encrypted integer. */
    mpz_init(&session_key_int);
    packet_get_mp_int(&session_key_int);

    /* Get protocol flags. */
    protocol_flags = packet_get_int();
    packet_set_protocol_flags(protocol_flags);

    /* Decrypt it using our private server key and private host key (key with 
       larger modulus first). */
    if (mpz_cmp(&sensitive_data.private_key.n, &sensitive_data.host_key.n) > 0) {
        /* Private key has bigger modulus. */
        assert(sensitive_data.private_key.bits >= sensitive_data.host_key.bits + SSH_KEY_BITS_RESERVED);
        rsa_private_decrypt(&session_key_int, &session_key_int, &sensitive_data.private_key);
        rsa_private_decrypt(&session_key_int, &session_key_int, &sensitive_data.host_key);
    } else {
        /* Host key has bigger modulus (or they are equal). */
        assert(sensitive_data.host_key.bits >= sensitive_data.private_key.bits + SSH_KEY_BITS_RESERVED);
        rsa_private_decrypt(&session_key_int, &session_key_int, &sensitive_data.host_key);
        rsa_private_decrypt(&session_key_int, &session_key_int, &sensitive_data.private_key);
    }

    /* Compute session id for this session. */
    compute_session_id(session_id, check_bytes, sensitive_data.host_key.bits, &sensitive_data.host_key.n, sensitive_data.private_key.bits, &sensitive_data.private_key.n);

    /* Extract session key from the decrypted integer.  The key is in the 
       least significant 256 bits of the integer; the first byte of the 
       key is in the highest bits. */
    mp_linearize_msb_first(session_key, sizeof(session_key), &session_key_int);

    /* Xor the first 16 bytes of the session key with the session id. */
    for (i = 0; i < 16; i++)
        session_key[i] ^= session_id[i];

    /* Destroy the decrypted integer.  It is no longer needed. */
    mpz_clear(&session_key_int);

    /* Set the session key.  From this on all communications will be
       encrypted. */
    packet_set_encryption_key(session_key, SSH_SESSION_KEY_LENGTH, cipher_type, 0);

    /* Destroy our copy of the session key.  It is no longer needed. */
    memset(session_key, 0, sizeof(session_key));

    debug("Received session key; encryption turned on.");

    /* Send an acknowledgement packet.  Note that this packet is sent
       encrypted. */
    packet_start(SSH_SMSG_SUCCESS);
    packet_send();
    packet_write_wait();

    /* Get the name of the user that we wish to log in as. */
    packet_read_expect(SSH_CMSG_USER);

    /* Get the user name. */
    user = packet_get_string(NULL);

    /* Destroy the private and public keys.  They will no longer be needed. */
    rsa_clear_public_key(&public_key);
    rsa_clear_private_key(&sensitive_data.private_key);
    rsa_clear_private_key(&sensitive_data.host_key);

    /* Do the authentication. */
    do_authentication(user, privileged_port, cipher_type);
}

/* Fails all authentication requests */
void do_authentication_fail_loop(void)
{
    /* The user does not exist. */
    packet_start(SSH_SMSG_FAILURE);
    packet_send();
    packet_write_wait();

    /* Keep reading packets, and always respond with a failure.  This is to
       avoid disclosing whether such a user really exists. */
    for (;;) {
        /* Read a packet.  This will not return if the client disconnects. */
        (void) packet_read();
        packet_get_all();

        /* Send failure.  This should be indistinguishable from a failed
           authentication. */
        packet_start(SSH_SMSG_FAILURE);
        packet_send();
        packet_write_wait();
    }
     /*NOTREACHED*/ abort();
}

/* Performs authentication of an incoming connection.  Session key has already
   been exchanged and encryption is enabled.  User is the user name to log
   in as (received from the clinet).  Privileged_port is true if the
   connection comes from a privileged port (used for .rhosts authentication).*/

void do_authentication(char *user, int privileged_port, int cipher_type)
{
    int type;
    int authenticated = 0;
    int authentication_type = 0;
    char *password;
    char *client_user;
    int password_attempts = 0;

    if (strlen(user) > IDLEN)
        do_authentication_fail_loop();

    /* Verify that the user is a valid user.  We disallow usernames starting
       with any characters that are commonly used to start NIS entries. */
    if (user[0] == '-' || user[0] == '+' || user[0] == '@')
        do_authentication_fail_loop();

    resolve_ucache();
    resolve_utmp();
    if (*user == '\0' || !dosearchuser(user) || userbansite(user,get_remote_ipaddr()))
	do_authentication_fail_loop();

    debug("Attempting authentication for %.100s.", user);

    /* If the user has no password, accept authentication immediately. */
    if (auth_password(user, "")) {
        /* Authentication with empty password succeeded. */
        authentication_type = SSH_AUTH_PASSWORD;
        authenticated = 1;
        /* Success packet will be sent after loop below. */
    } else {
        /* Indicate that authentication is needed. */
        packet_start(SSH_SMSG_FAILURE);
        packet_send();
        packet_write_wait();
    }

    /* Loop until the user has been authenticated or the connection is closed. */
    while (!authenticated) {
        /* Get a packet from the client. */
        type = packet_read();

        /* Process the packet. */
        switch (type) {

        case SSH_CMSG_AUTH_RHOSTS:
            packet_get_all();
            log_msg("Rhosts authentication disabled.");
            break;

        case SSH_CMSG_AUTH_RHOSTS_RSA:
            packet_get_all();
            log_msg("Rhosts with RSA authentication disabled.");
            break;

        case SSH_CMSG_AUTH_RSA:
            packet_get_all();
            log_msg("RSA authentication disabled.");
            break;

        case SSH_CMSG_AUTH_PASSWORD:
            if (cipher_type == SSH_CIPHER_NONE) {
                packet_get_all();
                log_msg("Password authentication not available for unencrypted session.");
                break;
            }

            /* Password authentication requested. */
            /* Read user password.  It is in plain text, but was transmitted
               over the encrypted channel so it is not visible to an outside
               observer. */
            password = packet_get_string(NULL);

            if (password_attempts >= 5) {       /* Too many password authentication attempts. */
                packet_disconnect("Too many password authentication attempts from %.100s for user %.100s.", get_remote_ipaddr(), user);
             /*NOTREACHED*/}

            /* Count password authentication attempts, and log if appropriate. */
            if (password_attempts > 0) {
                /* Log failures if attempted more than once. */
                debug("Password authentication failed for user %.100s from %.100s.", user, get_remote_ipaddr());
            }
            password_attempts++;

            /* Try authentication with the password. */
            if (auth_password(user, password)) {
                /* Successful authentication. */
                /* Clear the password from memory. */
                memset(password, 0, strlen(password));
                xfree(password);
                log_msg("Password authentication for %.100s accepted.", user);
                authentication_type = SSH_AUTH_PASSWORD;
                authenticated = 1;
                break;
            }
            debug("Password authentication for %.100s failed.", user);
            memset(password, 0, strlen(password));
            xfree(password);
            break;

        default:
            /* Any unknown messages will be ignored (and failure returned)
               during authentication. */
            packet_get_all();
            log_msg("Unknown message during authentication: type %d", type);
            break;              /* Respond with a failure message. */
        }
        /* If successfully authenticated, break out of loop. */
        if (authenticated)
            break;

        /* Send a message indicating that the authentication attempt failed. */
        packet_start(SSH_SMSG_FAILURE);
        packet_send();
        packet_write_wait();
    }


    /* The user has been authenticated and accepted. */
    packet_start(SSH_SMSG_SUCCESS);
    packet_send();
    packet_write_wait();

    /* Perform session preparation. */
    do_authenticated(NULL);
}

/* Prepares for an interactive session.  This is called after the user has
   been successfully authenticated.  During this message exchange, pseudo
   terminals are allocated, X11, TCP/IP, and authentication agent forwardings
   are requested, etc. */

void do_authenticated(char *pw)
{
    int type;
    int compression_level = 0, enable_compression_after_reply = 0;
    int have_pty = 0, ptyfd = -1, ttyfd = -1;
    int row, col, xpixel, ypixel, screen;
    unsigned long max_size;
    char ttyname[64];
    char *command, *term = NULL, *display = NULL, *proto = NULL, *data = NULL;
    int i;

    /* Cancel the alarm we set to limit the time taken for authentication. */
    alarm(0);

    /* Inform the channel mechanism that we are the server side and that
       the client may request to connect to any port at all.  (The user could
       do it anyway, and we wouldn\'t know what is permitted except by the
       client telling us, so we can equally well trust the client not to request
       anything bogus.) */

    /* We stay in this loop until the client requests to execute a shell or a
       command. */
    while (1) {
        /* Get a packet from the client. */
        type = packet_read();

        /* Process the packet. */
        switch (type) {
        case SSH_CMSG_REQUEST_COMPRESSION:
            /* COMMAN: k core said that compression is not useful */
            goto fail;
            compression_level = packet_get_int();
            if (compression_level < 1 || compression_level > 9) {
                packet_send_debug("Received illegal compression level %d.", compression_level);
                goto fail;
            }
            /* Enable compression after we have responded with SUCCESS. */
            enable_compression_after_reply = 1;
            break;

        case SSH_CMSG_MAX_PACKET_SIZE:
            /* Get maximum size from paket. */
            max_size = packet_get_int();

            /* Make sure that it is acceptable. */
            if (max_size < 4096 || max_size > 256 * 1024) {
                packet_send_debug("Received illegal max packet size %lu.", max_size);
                goto fail;
            }

            /* Set the size and return success. */
            packet_set_max_size(max_size);
            break;

        case SSH_CMSG_REQUEST_PTY:
packet_get_string(NULL);
  	             row = packet_get_int();
 	             col = packet_get_int();
 	             xpixel = packet_get_int();
  	             ypixel = packet_get_int();
  	             do_naws(row, col);
            packet_get_all();
    //        debug("Allocating a pty not permitted for this authentication.");
            break;

        case SSH_CMSG_X11_REQUEST_FORWARDING:
            packet_get_all();
            debug("X11 forwarding disabled in this site.");
            packet_send_debug("X11 forwarding disabled in this site.");
            goto fail;

        case SSH_CMSG_AGENT_REQUEST_FORWARDING:
            packet_get_all();
            debug("Authentication agent forwarding not permitted for this authentication.");
            goto fail;
        case SSH_CMSG_PORT_FORWARD_REQUEST:
            packet_get_all();
            debug("All port forwardings disabled in this site.");
            packet_send_debug("All port forwardings disabled in this site.");
            goto fail;

        case SSH_CMSG_EXEC_SHELL:
            /* Set interactive/non-interactive mode. */
            packet_set_interactive(1, options.keepalives);

            if (forced_command != NULL)
                goto do_forced_command;
            debug("Forking shell.");
            do_exec_no_pty(NULL, pw, display, proto, data, 0);
            return;

        case SSH_CMSG_EXEC_CMD:
            packet_get_all();
            debug("command executing disabled in this site.");
            packet_send_debug("command executing disabled in this site.");
            goto fail;

        default:
            /* Any unknown messages in this phase are ignored, and a failure
               message is returned. */
            packet_get_all();
            log_msg("Unknown packet type received after authentication: %d", type);
            goto fail;
        }

        /* The request was successfully processed. */
        packet_start(SSH_SMSG_SUCCESS);
        packet_send();
        packet_write_wait();

        /* Enable compression now that we have replied if appropriate. */
        if (enable_compression_after_reply) {
            enable_compression_after_reply = 0;
            packet_start_compression(compression_level);
        }

        continue;

      fail:
        /* The request failed. */
        packet_get_all();
        packet_start(SSH_SMSG_FAILURE);
        packet_send();
        packet_write_wait();
        continue;

      do_forced_command:
        /* There is a forced command specified for this login.  Execute it. */
        debug("Executing forced command: %.900s", forced_command);
        return;
    }
}

/* This is called to fork and execute a command when we have no tty.  This
   will call do_child from the child, and server_loop from the parent after
   setting up file descriptors and such. */

void sshbbs_end(void)
{
    do_abort_bbs();	
    packet_disconnect("sshd exit");
}

extern int ssh_init(void);
extern int bbs_entry(int argc, char *argv[]);
void do_exec_no_pty(const char *command, char *pw, const char *display, const char *auth_proto, const char *auth_data, int quick_login)
{
    char *argv[3]={"bbs","d",get_remote_ipaddr()};
    ssh_init();
    chdir(MY_BBS_HOME);
    dup2(packet_get_connection_in(), 0);
    atexit(sshbbs_end);
    bbs_entry(3,argv);
    exit(0);
}
