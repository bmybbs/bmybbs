/*

ssh.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Fri Mar 17 17:09:37 1995 ylo

Generic header file for ssh.

*/

/*
 * $Id: ssh.h,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: ssh.h,v $
 * Revision 1.1.1.1  2009-03-04 06:33:27  bmybbs
 * bmysrc
 *
 * Revision 1.1.1.1  2002/10/01 09:42:05  clearboy
 * update on 20051031
 * by clearboy 
 * for transfering the source codes from main site to the experimental site 
 * for the first time.
 *
 *
 * Revision 1.1.1.1  2002/10/01 09:42:05  ylsdd
 * 水木底sshbbsd导入
 * 然后慢慢改吧
 *
 * Revision 1.3  2002/08/04 11:39:44  kcn
 * format c
 *
 * Revision 1.2  2002/08/04 11:08:49  kcn
 * format C
 *
 * Revision 1.1.1.1  2002/04/27 05:47:25  kxn
 * no message
 *
 * Revision 1.1  2001/07/04 06:07:12  bbsdev
 * bbs sshd
 *
 * Revision 1.27  1999/04/29 07:52:25  tri
 * 	Replaced OSF1/C2 security support with more complete SIA
 *         (Security Integration Architecture) support by Tom Woodburn.
 *
 * Revision 1.26  1998/08/07 12:29:36  tri
 *      Changed agent socket name /tmp/ssh-$USER/agent-socket-<pid>
 *      to /tmp/ssh-$USER/ssh-<pid>-agent in order to avoid
 *      ambiguities on systems with 14 character filename and
 *      to provide concistency with ssh2.
 *
 * Revision 1.25  1998/07/08 01:05:30  kivinen
 *      Added consts.
 *
 * Revision 1.24  1998/07/08 00:50:13  kivinen
 *      Added some prototypes. Added setting PASSWD_PATH if not set in
 *      by the configure.
 *
 * Revision 1.23  1998/05/23  20:37:44  kivinen
 *      Added OSF1 C2 prototypes.
 *
 * Revision 1.22  1998/03/27  17:02:55  kivinen
 *      Added gateway ports option. Added ignore root rhosts option.
 *
 * Revision 1.21  1998/01/02 06:23:01  kivinen
 *      Renamed SSH_AUTHENTICATION_SOCKET to SSH_AUTH_SOCK.
 *
 * Revision 1.20  1997/04/27 21:55:35  kivinen
 *      Added channel_add_{allow,deny}_forwd_{to,port} prototypes.
 *      Added match_port prototype.
 *
 * Revision 1.19  1997/04/21 01:06:44  kivinen
 *      Fixed prototype for server_loop to have cleanup_context
 *      instead of ttyname.
 *
 * Revision 1.18  1997/04/17 04:17:02  kivinen
 *      Added read_confirmation prototype.
 *      Added ttyname to server_loop prototype.
 *
 * Revision 1.17  1997/04/05 22:01:10  kivinen
 *      Fixed typo in SSH_AUTH_KERBEROS.
 *
 * Revision 1.16  1997/03/27 03:11:13  kivinen
 *      Added kerberos patches from Glenn Machin.
 *
 * Revision 1.15  1997/03/26 07:12:09  kivinen
 *      Fixed prototypes.
 *      Added UID_ROOT define.
 *
 * Revision 1.14  1997/03/19 17:44:13  kivinen
 *      Added TISAuthentication stuff from Andre April
 *      <Andre.April@cediti.be>.
 *
 * Revision 1.13  1996/12/04 18:14:30  ttsalo
 *     Added new option, LOCAL_HOSTNAME_IN_DEBUG
 *
 * Revision 1.12  1996/11/24 08:27:48  kivinen
 *      Fixed auth_input_request_forwarding prototype.
 *
 * Revision 1.11  1996/11/05 16:03:00  ttsalo
 *       Removed the get_permanent_fd prototype
 *
 * Revision 1.10  1996/10/29 22:46:55  kivinen
 *      Changed PROTOCOL_MINOR from 4 to 5.
 *      log -> log_msg.
 *
 * Revision 1.9  1996/10/20 16:28:39  ttsalo
 *      Added global variable original_real_uid
 *
 * Revision 1.8  1996/09/24 20:17:54  ylo
 *      Changed identity files to be always encrypted with 3DES (used
 *      to be IDEA, when it is compiled in).  This is to make identity
 *      files more compatible with versions that don't include IDEA
 *      for patent reasons.
 *
 * Revision 1.7  1996/09/08 17:21:09  ttsalo
 *      A lot of changes in agent-socket handling
 *
 * Revision 1.6  1996/09/04 12:42:31  ttsalo
 *      Added pid to agent-socket name
 *
 * Revision 1.5  1996/08/29 14:51:24  ttsalo
 *      Agent-socket directory handling implemented
 *
 * Revision 1.4  1996/08/21 20:43:56  ttsalo
 *      Made ssh-agent use a different, more secure way of storing
 *      it's initial socket.
 *
 * Revision 1.3  1996/08/13 09:04:21  ttsalo
 *      Home directory, .ssh and .ssh/authorized_keys are now
 *      checked for wrong owner and group & world writeability.
 *
 * Revision 1.2  1996/04/22 23:49:45  huima
 * Changed protocol version to 1.4, added calls to emulate module.
 *
 * Revision 1.1.1.1  1996/02/18  21:38:10  ylo
 *      Imported ssh-1.2.13.
 *
 * Revision 1.16  1995/10/02  01:31:13  ylo
 *      Moved sshd.pid to PIDDIR.  Also changed file name.
 *      Added SSH_HOSTS_EQUIV (shosts.equiv).
 *
 * Revision 1.15  1995/09/27  02:16:24  ylo
 *      Added SSH_USER_RC and SSH_SYSTEM_RC.
 *
 * Revision 1.14  1995/09/25  00:02:06  ylo
 *      Added client_loop.
 *      Added screen number arguments.
 *      Added connection_attempts.
 *
 * Revision 1.13  1995/09/22  22:23:41  ylo
 *      Changed argument list of ssh_login.
 *
 * Revision 1.12  1995/09/21  17:14:08  ylo
 *      Added original_real_uid argument to ssh_connect.
 *
 * Revision 1.11  1995/09/10  22:47:59  ylo
 *      Added server_loop.
 *      Added original_real_uid parameter to ssh_login.
 *
 * Revision 1.10  1995/09/09  21:26:46  ylo
 * /m/shadows/u2/users/ylo/ssh/README
 *
 * Revision 1.9  1995/08/31  09:23:42  ylo
 *      Added support for ETCDIR.
 *
 * Revision 1.8  1995/08/29  22:33:52  ylo
 *      Added get_remote_ipaddr, get_permanent_fd.
 *      Deleted SSH_NUM_DUPS.
 *
 * Revision 1.7  1995/08/21  23:28:57  ylo
 *      Added SERVER_CONFIG_FILE.
 *      Added syslog facility definitions.
 *
 * Revision 1.6  1995/08/18  22:57:06  ylo
 *      Now uses 3DES as the authfile cipher if IDEA is not available.
 *
 * Revision 1.5  1995/07/27  00:41:04  ylo
 *      Added GlobalKnownHostsFile and UserKnownHostsFile.
 *
 * Revision 1.4  1995/07/26  23:28:56  ylo
 *      Changed PROTOCOL_MINOR from 0 to 1.
 *
 * Revision 1.3  1995/07/16  01:02:26  ylo
 *      Removed host argument from record_logout.
 *
 * Revision 1.2  1995/07/13  01:40:22  ylo
 *      Removed "Last modified" header.
 *      Added cvs log.
 *
 * $Endlog$
 */

#ifndef SSH_H
#define SSH_H

#include <gmp.h>
#include "rsa.h"
#include "randoms.h"
#include "cipher.h"

/* The default cipher used if IDEA is not supported by the remote host. 
   It is recommended that this be one of the mandatory ciphers (DES, 3DES),
   though that is not required. */
#define SSH_FALLBACK_CIPHER     SSH_CIPHER_3DES

/* Cipher used for encrypting authentication files. */
#define SSH_AUTHFILE_CIPHER     SSH_CIPHER_3DES

/* Default port number. */
#define SSH_DEFAULT_PORT        22

/* Maximum number of TCP/IP ports forwarded per direction. */
#define SSH_MAX_FORWARDS_PER_DIRECTION  100

/* Maximum number of RSA authentication identity files that can be specified
   in configuration files or on the command line. */
#define SSH_MAX_IDENTITY_FILES          100

/* Major protocol version.  Different version indicates major incompatiblity
   that prevents communication.  */
#define PROTOCOL_MAJOR          1

/* Minor protocol version.  Different version indicates minor incompatibility
   that does not prevent interoperation. */
#define PROTOCOL_MINOR          5

/* Name for the service.  The port named by this service overrides the default
   port if present. */
#define SSH_SERVICE_NAME        "ssh"

/* System-wide file containing host keys of known hosts.  This file should be
   world-readable. */
#define SSH_SYSTEM_HOSTFILE     ETCDIR "/ssh_known_hosts"

/*  HOST_KEY_FILE               /etc/ssh_host_key,
    SERVER_CONFIG_FILE          /etc/sshd_config,
and HOST_CONFIG_FILE            /etc/ssh_config
are all defined in Makefile.in.  Of these, ssh_host_key should be readable
only by root, whereas ssh_config should be world-readable. */

/* Random seed file for the daemon.  This file should be readable only by 
   root. */
#define SSH_DAEMON_SEED_FILE    ETCDIR "/ssh_random_seed"

/* The process id of the daemon listening for connections is saved
   here to make it easier to kill the correct daemon when necessary. */
#define SSH_DAEMON_PID_FILE     PIDDIR "/sshd.pid"

/* The directory in user\'s home directory in which the files reside.
   The directory should be world-readable (though not all files are). */
#define SSH_USER_DIR            ".ssh"

/* Per-user file containing host keys of known hosts.  This file need
   not be readable by anyone except the user him/herself, though this does
   not contain anything particularly secret. */
#define SSH_USER_HOSTFILE       "~/.ssh/known_hosts"

/* Name of the file containing client-side random seed.  This file should
   only be readable by the user him/herself. */
#define SSH_CLIENT_SEEDFILE     ".ssh/random_seed"

/* Name of the default file containing client-side authentication key. 
   This file should only be readable by the user him/herself. */
#define SSH_CLIENT_IDENTITY     ".ssh/identity"

/* Configuration file in user\'s home directory.  This file need not be
   readable by anyone but the user him/herself, but does not contain
   anything particularly secret.  If the user\'s home directory resides
   on an NFS volume where root is mapped to nobody, this may need to be
   world-readable. */
#define SSH_USER_CONFFILE       ".ssh/config"

/* File containing a list of those rsa keys that permit logging in as
   this user.  This file need not be
   readable by anyone but the user him/herself, but does not contain
   anything particularly secret.  If the user\'s home directory resides
   on an NFS volume where root is mapped to nobody, this may need to be
   world-readable.  (This file is read by the daemon which is running as 
   root.) */
#define SSH_USER_PERMITTED_KEYS ".ssh/authorized_keys"

/* Per-user and system-wide ssh "rc" files.  These files are executed with
   /bin/sh before starting the shell or command if they exist.  They
   will be passed "proto cookie" as arguments if X11 forwarding with
   spoofing is in use.  xauth will be run if neither of these exists. */
#define SSH_USER_RC             ".ssh/rc"
#define SSH_SYSTEM_RC           ETCDIR "/sshrc"

/* Ssh-only version of /etc/hosts.equiv. */
#define SSH_HOSTS_EQUIV         ETCDIR "/shosts.equiv"

/* Additionally, the daemon may use ~/.rhosts and /etc/hosts.equiv if 
   rhosts authentication is enabled. */

/* Socket for connecting the authentication agent.  Normally the connection 
   to the authentication agent is passed in a file descriptor; however,
   on some systems, commonly used shells close all open file descriptors.
   To make the agent usable on those systems, configure checks whether
   the shells close all descriptors, and if so, uses a socket instead.
   That socket is an unix-domain socket and must not be accessible by
   anyone but the user him/herself. A directory \'ssh-agent-<loginname>\'
   is created under /tmp, which is supposedly on the local machine,
   and socket is created under the directory. On some systems, sockets\'
   protections are not adequately checked, so this mode 700 per-user
   directory is needed. If socket were in the user\'s home directory,
   the daemon (running as root) might not be able to create and listen
   to the socket.
   
   SSH_AGENT_SOCKET_DIR can be changed to something else ("/tmp/.ssh/ssh-%s"
   for example), but only the last directory in the path will be
   dynamically created and deleted by sshd and ssh-agent. */
#define SSH_AGENT_SOCKET_DIR    "/tmp/ssh-%.50s"
#define SSH_AGENT_SOCKET        "ssh-%d-agent"

/* Name of the environment variable containing the pathname of the
   authentication socket. */
#define SSH_AUTHSOCKET_ENV_NAME         "SSH_AUTH_SOCK"

/* Check that we always have PASSWD_PATH set */
#ifndef PASSWD_PATH
#define PASSWD_PATH "/bin/passwd"
#endif                          /* PASSWD_PATH */

/* Force host key length and server key length to differ by at least this
   many bits.  This is to make double encryption with rsaref work. */
#define SSH_KEY_BITS_RESERVED           128

/* Length of the session key in bytes.  (Specified as 256 bits in the 
   protocol.)  */
#define SSH_SESSION_KEY_LENGTH          32

#ifdef KERBEROS
#ifdef KRB5
#include <krb5.h>
#define KRB_SERVICE_NAME                "host"
#endif                          /* KRB5 */
#endif                          /* KERBEROS */

/* Authentication methods.  New types can be added, but old types should not
   be removed for compatibility.  The maximum allowed value is 31. */
#define SSH_AUTH_RHOSTS         1
#define SSH_AUTH_RSA            2
#define SSH_AUTH_PASSWORD       3
#define SSH_AUTH_RHOSTS_RSA     4
#define SSH_AUTH_TIS            5
#define SSH_AUTH_KERBEROS       6
#define SSH_PASS_KERBEROS_TGT   7

/* These are reserved for official patches, do not use them */
#define SSH_AUTH_RESERVED_1     8
#define SSH_AUTH_RESERVED_2     9
#define SSH_AUTH_RESERVED_3     10
#define SSH_AUTH_RESERVED_4     11
#define SSH_AUTH_RESERVED_5     12
#define SSH_AUTH_RESERVED_6     13
#define SSH_AUTH_RESERVED_7     14
#define SSH_AUTH_RESERVED_8     15

/* If you add new methods add them after this using random number between 16-31
   so if someone else adds also new methods you dont use same number. */

/* Protocol flags.  These are bit masks. */
#define SSH_PROTOFLAG_SCREEN_NUMBER     1       /* X11 forwarding includes screen */
#define SSH_PROTOFLAG_HOST_IN_FWD_OPEN  2       /* forwarding opens contain host */

/* Definition of message types.  New values can be added, but old values
   should not be removed or without careful consideration of the consequences
   for compatibility.  The maximum value is 254; value 255 is reserved
   for future extension. */
/* Message name *//* msg code *//* arguments */
#define SSH_MSG_NONE                            0       /* no message */
#define SSH_MSG_DISCONNECT                      1       /* cause (string) */
#define SSH_SMSG_PUBLIC_KEY                     2       /* ck,msk,srvk,hostk */
#define SSH_CMSG_SESSION_KEY                    3       /* key (MP_INT) */
#define SSH_CMSG_USER                           4       /* user (string) */
#define SSH_CMSG_AUTH_RHOSTS                    5       /* user (string) */
#define SSH_CMSG_AUTH_RSA                       6       /* modulus (MP_INT) */
#define SSH_SMSG_AUTH_RSA_CHALLENGE             7       /* int (MP_INT) */
#define SSH_CMSG_AUTH_RSA_RESPONSE              8       /* int (MP_INT) */
#define SSH_CMSG_AUTH_PASSWORD                  9       /* pass (string) */
#define SSH_CMSG_REQUEST_PTY                    10      /* TERM, tty modes */
#define SSH_CMSG_WINDOW_SIZE                    11      /* row,col,xpix,ypix */
#define SSH_CMSG_EXEC_SHELL                     12      /* */
#define SSH_CMSG_EXEC_CMD                       13      /* cmd (string) */
#define SSH_SMSG_SUCCESS                        14      /* */
#define SSH_SMSG_FAILURE                        15      /* */
#define SSH_CMSG_STDIN_DATA                     16      /* data (string) */
#define SSH_SMSG_STDOUT_DATA                    17      /* data (string) */
#define SSH_SMSG_STDERR_DATA                    18      /* data (string) */
#define SSH_CMSG_EOF                            19      /* */
#define SSH_SMSG_EXITSTATUS                     20      /* status (int) */
#define SSH_MSG_CHANNEL_OPEN_CONFIRMATION       21      /* channel (int) */
#define SSH_MSG_CHANNEL_OPEN_FAILURE            22      /* channel (int) */
#define SSH_MSG_CHANNEL_DATA                    23      /* ch,data (int,str) */
#define SSH_MSG_CHANNEL_CLOSE                   24      /* channel (int) */
#define SSH_MSG_CHANNEL_CLOSE_CONFIRMATION      25      /* channel (int) */

/* new channel protocol */
#define SSH_MSG_CHANNEL_INPUT_EOF               24
#define SSH_MSG_CHANNEL_OUTPUT_CLOSED           25

/*      SSH_CMSG_X11_REQUEST_FORWARDING         26         OBSOLETE */
#define SSH_SMSG_X11_OPEN                       27      /* channel (int) */
#define SSH_CMSG_PORT_FORWARD_REQUEST           28      /* p,host,hp (i,s,i) */
#define SSH_MSG_PORT_OPEN                       29      /* ch,h,p (i,s,i) */
#define SSH_CMSG_AGENT_REQUEST_FORWARDING       30      /* */
#define SSH_SMSG_AGENT_OPEN                     31      /* port (int) */
#define SSH_MSG_IGNORE                          32      /* string */
#define SSH_CMSG_EXIT_CONFIRMATION              33      /* */
#define SSH_CMSG_X11_REQUEST_FORWARDING         34      /* proto,data (s,s) */
#define SSH_CMSG_AUTH_RHOSTS_RSA                35      /* user,mod (s,mpi) */
#define SSH_MSG_DEBUG                           36      /* string */
#define SSH_CMSG_REQUEST_COMPRESSION            37      /* level 1-9 (int) */
#define SSH_CMSG_MAX_PACKET_SIZE                38      /* max_size (int) */

/* Support for TIS authentication server
   Contributed by Andre April <Andre.April@cediti.be>. */
#define SSH_CMSG_AUTH_TIS                       39      /* */
#define SSH_SMSG_AUTH_TIS_CHALLENGE             40      /* string */
#define SSH_CMSG_AUTH_TIS_RESPONSE              41      /* pass (string) */

/* Support for kerberos authentication by Glenn Machin and Dug Song
   <dugsong@umich.edu> */
#define SSH_CMSG_AUTH_KERBEROS                  42      /* string (KTEXT) */
#define SSH_SMSG_AUTH_KERBEROS_RESPONSE         43      /* string (KTEXT) */
#define SSH_CMSG_HAVE_KERBEROS_TGT              44      /* string (credentials) */

/* Reserved for official extensions, do not use these */
#define SSH_CMSG_RESERVED_START                 45
#define SSH_CMSG_RESERVED_END                   63

/* If ou add new messages add them starting from something after 64, better to
   use some random number between 64-127 so if someone else adds something else
   you dont use same numbers */


/* define this and debug() will print local hostname */
#define LOCAL_HOSTNAME_IN_DEBUG 1

/* Includes that need definitions above. */


/*------------ definitions for login.c -------------*/

/* Returns the time when the user last logged in.  Returns 0 if the 
   information is not available.  This must be called before record_login. 
   The host from which the user logged in is stored in buf. */
unsigned long get_last_login_time(uid_t uid, const char *logname, char *buf, unsigned int bufsize);

/* Records that the user has logged in.  This does many things normally
   done by login(1). */

/*------------ definitions for sshconnect.c ----------*/

/*------------ Definitions for various authentication methods. -------*/

/* Tries to authenticate the user using the .rhosts file.  Returns true if
   authentication succeeds.  If ignore_rhosts is non-zero, this will not
   consider .rhosts and .shosts (/etc/hosts.equiv will still be used). 
   If strict_modes is true, checks ownership and modes of .rhosts/.shosts. */
int auth_rhosts(struct passwd *pw, const char *client_user, int ignore_rhosts, int ignore_root_rhosts, int strict_modes);

/* Tries to authenticate the user using the .rhosts file and the host using
   its host key.  Returns true if authentication succeeds. */
int auth_rhosts_rsa(RandomState * state,
                    struct passwd *pw, const char *client_user, unsigned int bits, MP_INT * client_host_key_e, MP_INT * client_host_key_n, int ignore_rhosts, int ignore_root_rhosts, int strict_modes);

/* Tries to authenticate the user using password.  Returns true if
   authentication succeeds. */
int auth_password(const char *server_user, const char *password);

/* Performs the RSA authentication dialog with the client.  This returns
   0 if the client could not be authenticated, and 1 if authentication was
   successful.  This may exit if there is a serious protocol violation. */
int auth_rsa(struct passwd *pw, MP_INT * client_n, RandomState * state, int strict_modes);

/* Parses an RSA key (number of bits, e, n) from a string.  Moves the pointer
   over the key.  Skips any whitespace at the beginning and at end. */
int auth_rsa_read_key(char **cpp, unsigned int *bitsp, MP_INT * e, MP_INT * n);

/* Returns the name of the machine at the other end of the socket.  The
   returned string should be freed by the caller. */
char *get_remote_hostname(int socket);

/* Return the canonical name of the host in the other side of the current
   connection (as returned by packet_get_connection).  The host name is
   cached, so it is efficient to call this several times. */
const char *get_canonical_hostname(void);

/* Returns the remote IP address as an ascii string.  The value need not be
   freed by the caller. */
const char *get_remote_ipaddr(void);

/* Returns the port number of the peer of the socket. */
int get_peer_port(int sock);

/* Returns the port number of the remote host. */
int get_remote_port(void);

/* Tries to match the host name (which must be in all lowercase) against the
   comma-separated sequence of subpatterns (each possibly preceded by ! to 
   indicate negation).  Returns true if there is a positive match; zero
   otherwise. */
int match_hostname(const char *host, const char *ip, const char *pattern, unsigned int len);

/* Checks whether the given host is already in the list of our known hosts.
   Returns HOST_OK if the host is known and has the specified key,
   HOST_NEW if the host is not known, and HOST_CHANGED if the host is known
   but used to have a different host key.  The host must be in all lowercase. 
   The check (file accesses) will be performed using the given uid with
   userfile. */
typedef enum { HOST_OK, HOST_NEW, HOST_CHANGED } HostStatus;
HostStatus check_host_in_hostfile(uid_t uid, const char *filename, const char *host, unsigned int bits, MP_INT * e, MP_INT * n);

/* Appends an entry to the host file.  Returns false if the entry
   could not be appended.  The operation will be performed with the given
   uid using userfile. */
int add_host_to_hostfile(uid_t uid, const char *filename, const char *host, unsigned int bits, MP_INT * e, MP_INT * n);

/* Performs the RSA authentication challenge-response dialog with the client,
   and returns true (non-zero) if the client gave the correct answer to
   our challenge; returns zero if the client gives a wrong answer. */
int auth_rsa_challenge_dialog(RandomState * state, unsigned int bits, MP_INT * e, MP_INT * n);

/* Reads a passphrase from /dev/tty with echo turned off.  Returns the 
   passphrase (allocated with xmalloc).  Exits if EOF is encountered. 
   If from_stdin is true, the passphrase will be read from stdin instead. 
   If this needs to use an auxiliary program to read the passphrase,
   this will run it with the given uid using userfile. */
char *read_passphrase(uid_t uid, const char *prompt, int from_stdin);

/* Reads a yes/no confirmation from /dev/tty.  Exits if EOF or "no" is
   encountered. */
void read_confirmation(const char *prompt);

/* Saves the authentication (private) key in a file, encrypting it with
   passphrase.  The identification of the file (lowest 64 bits of n)
   will precede the key to provide identification of the key without
   needing a passphrase.  File I/O will be done using the given uid with
   userfile. */
/* Loads the private key from the file.  Returns 0 if an error is encountered
   (file does not exist or is not readable, or passphrase is bad).
   This initializes the private key.  The comment of the key is returned
   in comment_return if it is non-NULL; the caller must free the value
   with xfree.  File I/O will be done with the given uid using userfile. */
int load_private_key(uid_t uid, const char *filename, const char *passphrase, RSAPrivateKey * private_key, char **comment_return);

/*------------ Definitions for logging. -----------------------*/

/* Supported syslog facilities. */
typedef enum {
    SYSLOG_FACILITY_DAEMON,
    SYSLOG_FACILITY_USER,
    SYSLOG_FACILITY_AUTH,
    SYSLOG_FACILITY_LOCAL0,
    SYSLOG_FACILITY_LOCAL1,
    SYSLOG_FACILITY_LOCAL2,
    SYSLOG_FACILITY_LOCAL3,
    SYSLOG_FACILITY_LOCAL4,
    SYSLOG_FACILITY_LOCAL5,
    SYSLOG_FACILITY_LOCAL6,
    SYSLOG_FACILITY_LOCAL7
} SyslogFacility;

typedef enum {
    SYSLOG_SEVERITY_DEBUG,
    SYSLOG_SEVERITY_INFO,
    SYSLOG_SEVERITY_NOTICE,
    SYSLOG_SEVERITY_WARNING,
    SYSLOG_SEVERITY_ERR,
    SYSLOG_SEVERITY_CRIT
} SyslogSeverity;

/* Initializes logging.  If debug is non-zero, debug() will output something.
   If quiet is non-zero, none of these will log send anything to syslog
   (but maybe to stderr). */
void log_init(char *av0, int on_stderr, int debug, int quiet, SyslogFacility facility);

/* Outputs a message to syslog or stderr, depending on the implementation. 
   The format must guarantee that the final message does not exceed 1024 
   characters.  The message should not contain newline. */
void log_msg(const char *fmt, ...);

/* Outputs a message to syslog or stderr, depending on the implementation. 
   The format must guarantee that the final message does not exceed 1024 
   characters.  The message should not contain newline.  The message
   is logged at the given severity level. */
void log_severity(SyslogSeverity severity, const char *fmt, ...);

/* Outputs a message to syslog or stderr, depending on the implementation. 
   The format must guarantee that the final message does not exceed 1024 
   characters.  The message should not contain newline. */
void debug(const char *fmt, ...);

/* Outputs a message to syslog or stderr, depending on the implementation. 
   The format must guarantee that the final message does not exceed 1024 
   characters.  The message should not contain newline. */
void error(const char *fmt, ...);

/* Outputs a message to syslog or stderr, depending on the implementation. 
   The format must guarantee that the final message does not exceed 1024 
   characters.  The message should not contain newline.  
   This call never returns. */
void fatal(const char *fmt, ...);

/* Outputs a message to syslog or stderr, depending on the implementation. 
   The format must guarantee that the final message does not exceed 1024 
   characters.  The message should not contain newline.  
   This call never returns.  The message is logged with the specified
   severity level. */
void fatal_severity(SyslogSeverity severity, const char *fmt, ...);

/* Registers a cleanup function to be called by fatal() before exiting. 
   It is permissible to call fatal_remove_cleanup for the function itself
   from the function. */
void fatal_add_cleanup(void (*proc) (void *context), void *context);

/* Removes a cleanup frunction to be called at fatal(). */
void fatal_remove_cleanup(void (*proc) (void *context), void *context);

/*---------------- definitions for x11.c ------------------*/


/* Sets specific protocol options. */

/* Allocate a new channel object and set its type and socket.  Remote_name
   must have been allocated with xmalloc; this will free it when the channel
   is freed. */
int channel_allocate(int type, int sock, char *remote_name);

/* Free the channel and close its socket. */
void channel_free(int channel);

/* Add any bits relevant to channels in select bitmasks. */
void channel_prepare_select(fd_set * readset, fd_set * writeset);

/* After select, perform any appropriate operations for channels which
   have events pending. */
void channel_after_select(fd_set * readset, fd_set * writeset);

/* If there is data to send to the connection, send some of it now. */
void channel_output_poll(void);

/* This is called when a packet of type CHANNEL_DATA has just been received.
   The message type has already been consumed, but channel number and data
   is still there. */
void channel_input_data(void);

/* Returns true if no channel has too much buffered data. */
int channel_not_very_much_buffered_data(void);

/* This is called after receiving CHANNEL_CLOSE. */
void channel_input_close(void);

/* This is called after receiving CHANNEL_CLOSE_CONFIRMATION. */
void channel_input_close_confirmation(void);

/* This is called after receiving CHANNEL_OPEN_CONFIRMATION. */
void channel_input_open_confirmation(void);

/* This is called after receiving CHANNEL_OPEN_FAILURE from the other side. */
void channel_input_open_failure(void);

/* This closes any sockets that are listening for connections; this removes
   any unix domain sockets. */
void channel_stop_listening(void);

/* Closes the sockets of all channels.  This is used to close extra file
   descriptors after a fork. */
void channel_close_all(void);

/* Returns the maximum file descriptor number used by the channels. */
int channel_max_fd(void);

/* Returns true if there is still an open channel over the connection. */
int channel_still_open(void);

/* Returns a string containing a list of all open channels.  The list is
   suitable for displaying to the user.  It uses crlf instead of newlines.
   The caller should free the string with xfree. */
char *channel_open_message(void);

/* Initiate forwarding of connections to local port "port" through the secure
   channel to host:port from remote side.  This never returns if there
   was an error. */
void channel_request_local_forwarding(int port, const char *host, int remote_port, int gateway_ports);

/* Initiate forwarding of connections to port "port" on remote host through
   the secure channel to host:port from local side.  This never returns
   if there was an error.  This registers that open requests for that
   port are permitted. */
void channel_request_remote_forwarding(int port, const char *host, int remote_port);

/* Permits opening to any host/port in SSH_MSG_PORT_OPEN.  This is usually
   called by the server, because the user could connect to any port anyway,
   and the server has no way to know but to trust the client anyway. */
void channel_permit_all_opens(void);

#ifdef F_SECURE_COMMERCIAL











#endif                          /* F_SECURE_COMMERCIAL */

/* This is called after receiving CHANNEL_FORWARDING_REQUEST.  This initates
   listening for the port, and sends back a success reply (or disconnect
   message if there was an error).  This never returns if there was an 
   error. */
void channel_input_port_forward_request(int is_root);

/* This is called after receiving PORT_OPEN message.  This attempts to connect
   to the given host:port, and sends back CHANNEL_OPEN_CONFIRMATION or
   CHANNEL_OPEN_FAILURE. */
void channel_input_port_open(void);

/* Creates a port for X11 connections, and starts listening for it.
   Returns the display name, or NULL if an error was encountered. */
char *x11_create_display(int screen);

/* Creates an internet domain socket for listening for X11 connections. 
   Returns a suitable value for the DISPLAY variable, or NULL if an error
   occurs. */
char *x11_create_display_inet(int screen);

/* This is called when SSH_SMSG_X11_OPEN is received.  The packet contains
   the remote channel number.  We should do whatever we want, and respond
   with either SSH_MSG_OPEN_CONFIRMATION or SSH_MSG_OPEN_FAILURE. */
void x11_input_open(void);

/* Requests forwarding of X11 connections.  This should be called on the 
   client only. */
void x11_request_forwarding(void);

/* Requests forwarding for X11 connections, with authentication spoofing.
   This should be called in the client only.  */
void x11_request_forwarding_with_spoofing(RandomState * state, const char *proto, const char *data);

/* Sends a message to the server to request authentication fd forwarding. */
void auth_request_forwarding(void);

/* Returns the number of the file descriptor to pass to child programs as
   the authentication fd. */
int auth_get_fd(void);

/* Returns the name of the forwarded authentication socket.  Returns NULL
   if there is no forwarded authentication socket.  The returned value points
   to a static buffer. */
char *auth_get_socket_name(void);

/* Tries to delete the authentication agent proxy socket and
   directory */
void auth_delete_socket(void *context);

/* This if called to process SSH_CMSG_AGENT_REQUEST_FORWARDING on the server.
   This starts forwarding authentication requests. This returns true if
   everything succeeds, otherwise it will return false (agent forwarding
   disabled). */
int auth_input_request_forwarding(struct passwd *pw);

/* This is called to process an SSH_SMSG_AGENT_OPEN message. */
void auth_input_open_request(void);

/* Returns true if the given string matches the pattern (which may contain
   ? and * as wildcards), and zero if it does not match. */
int match_pattern(const char *s, const char *pattern);

/* this combines the effect of match_pattern on a username, hostname
   and IP address. */
int match_user(const char *user, const char *host, const char *ip, const char *pattern);

/* Check that host name matches the pattern. If the pattern only contains
   numbers and periods, and wildcards compare it against the ip address
   otherwise assume it is host name */
int match_host(const char *host, const char *ip, const char *pattern);

#ifdef F_SECURE_COMMERCIAL

#endif                          /* F_SECURE_COMMERCIAL */

/* Expands tildes in the file name.  Returns data allocated by xmalloc.
   Warning: this calls getpw*. */
char *tilde_expand_filename(const char *filename, uid_t my_uid);

/* Performs the interactive session.  This handles data transmission between
   the client and the program.  Note that the notion of stdin, stdout, and
   stderr in this function is sort of reversed: this function writes to
   stdin (of the child program), and reads from stdout and stderr (of the
   child program).
   This will close fdin, fdout and fderr after releasing pty (if ttyname is non
   NULL) */
void server_loop(int pid, int fdin, int fdout, int fderr, void *cleanup_context);

/* Client side main loop for the interactive session. */
int client_loop(int have_pty, int escape_char);

/* Linked list of custom environment strings (see auth-rsa.c). */
struct envstring {
    struct envstring *next;
    char *s;
};


/* Functions from signals.c. */

/* Sets signal handlers so that core dumps are prevented.  This also
   sets the maximum core dump size to zero as an extra precaution (where
   supported).  The old core dump size limit is saved. */
void signals_prevent_core(void);

/* Sets all signals to their default state.  Restores RLIMIT_CORE previously
   saved by prevent_core(). */
void signals_reset(void);

/* Global variables */

extern uid_t original_real_uid;

#ifdef AMIGA
#define UID_ROOT 65535
#else
#define UID_ROOT 0
#endif

#endif                          /* SSH_H */
