/*

servconf.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Mon Aug 21 15:35:03 1995 ylo

Definitions for server configuration data and for the functions reading it.

*/

/*
 * $Id: servconf.h,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: servconf.h,v $
 * Revision 1.1.1.1  2009-03-04 06:33:27  bmybbs
 * bmysrc
 *
 * Revision 1.1.1.1  2002/10/01 09:42:06  clearboy
 * update on 20051031
 * by clearboy 
 * for transfering the source codes from main site to the experimental site 
 * for the first time.
 *
 *
 * Revision 1.1.1.1  2002/10/01 09:42:06  ylsdd
 * 水木底sshbbsd导入
 * 然后慢慢改吧
 *
 * Revision 1.3  2002/08/04 11:39:43  kcn
 * format c
 *
 * Revision 1.2  2002/08/04 11:08:48  kcn
 * format C
 *
 * Revision 1.1.1.1  2002/04/27 05:47:26  kxn
 * no message
 *
 * Revision 1.1  2001/07/04 06:07:11  bbsdev
 * bbs sshd
 *
 * Revision 1.11  1998/05/23 20:37:02  kivinen
 * 	Added forced_empty_passwd_change, num_deny_shosts,
 * 	num_allow_shosts, password_expire_warning_days,
 * 	account_expire_warning_days. Fixed typo in
 * 	forcedpasswordchange.
 *
 * Revision 1.10  1998/03/27  17:00:09  kivinen
 * 	Added IgnoreRootRhosts option.
 *
 * Revision 1.9  1998/01/03 06:42:11  kivinen
 * 	Added allow/deny groups option.
 *
 * Revision 1.8  1998/01/02 06:20:45  kivinen
 * 	Added xauthlocation and checkmail options.
 *
 * Revision 1.7  1997/04/27 21:51:44  kivinen
 * 	Added F-SECURE stuff. Added {Allow,Deny}Forwarding{To,Port}
 * 	feature. Added {Allow,Deny}Users feature from Steve Kann
 * 	<stevek@SteveK.COM>.
 *
 * Revision 1.6  1997/03/27 03:14:31  kivinen
 * 	Added kerberos patches from Glenn Machin.
 * 	Added USELOGIN patches from Brian Cully.
 *
 * Revision 1.5  1997/03/26 05:33:54  kivinen
 * 	Added idle_timeout option.
 *
 * Revision 1.4  1997/03/25 05:44:48  kivinen
 * 	Added silent_deny and umask.
 *
 * Revision 1.3  1997/03/19 17:55:14  kivinen
 * 	Added TIS authentication code from Andre April
 * 	<Andre.April@cediti.be>.
 * 	Added SECURE_RPC, SECURE_NFS and NIS_PLUS support from Andy
 * 	Polyakov <appro@fy.chalmers.se>.
 *
 * Revision 1.2  1996/11/27 15:38:28  ttsalo
 *     Added X11DisplayOffset-option
 *
 * Revision 1.1.1.1  1996/02/18 21:38:10  ylo
 * 	Imported ssh-1.2.13.
 *
 * $EndLog$
 */

#ifndef SERVCONF_H
#define SERVCONF_H

#define MAX_ALLOW_SHOSTS	256     /* Max # hosts on allow shosts list. */
#define MAX_DENY_SHOSTS		256     /* Max # hosts on deny shosts list. */
#define MAX_ALLOW_HOSTS		256     /* Max # hosts on allow list. */
#define MAX_DENY_HOSTS		256     /* Max # hosts on deny list. */
#define MAX_ALLOW_USERS		256     /* Max # users on allow list. */
#define MAX_DENY_USERS		256     /* Max # users on deny list. */
#define MAX_ALLOW_GROUPS	256     /* Max # groups on allow list. */
#define MAX_DENY_GROUPS		256     /* Max # groups on deny list. */

#ifdef F_SECURE_COMMERCIAL
#define MAX_ALLOW_FORWD_TO	256     /* Max # forwardingto on allow list. */
#define MAX_DENY_FORWD_TO	256     /* Max # forwardingto on deny list. */
#define MAX_ALLOW_FORWD_PORT	256     /* Max # forwardingport on allow list. */
#define MAX_DENY_FORWD_PORT	256     /* Max # forwardingport on deny list. */
#endif                          /* F_SECURE_COMMERCIAL */

typedef struct {
    int port;                   /* Port number to listen on. */
    struct in6_addr listen_addr; /* Address on which the server listens. ipv6 */
    char *host_key_file;        /* File containing host key. */
    char *random_seed_file;     /* File containing random seed. */
    char *pid_file;             /* File containing process ID number. */
    int server_key_bits;        /* Size of the server key. */
    int login_grace_time;       /* Disconnect if no auth in this time (sec). */
    int key_regeneration_time;  /* Server key lifetime (seconds). */
    int permit_root_login;      /* 0 = forced cmd only, 1 = no pwd, 2 = yes. */
    int ignore_rhosts;          /* Ignore .rhosts and .shosts. */
    int ignore_root_rhosts;     /* Ignore .rhosts and .shosts for root,
                                   defaults to ignore_rhosts if not given. */
    int quiet_mode;             /* If true, don't log anything but fatals. */
    int fascist_logging;        /* Perform very verbose logging. */
    int print_motd;             /* If true, print /etc/motd. */
    int x11_forwarding;         /* If true, permit inet (spoofing) X11 fwd. */
    int x11_display_offset;     /* How much to offset the DISPLAY number */
    int strict_modes;           /* If true, require string home dir modes. */
    int keepalives;             /* If true, set SO_KEEPALIVE. */
    time_t idle_timeout;        /* If non zero, sets idle-timeout */
    SyslogFacility log_facility;        /* Facility for system logging. */
    int rhosts_authentication;  /* If true, permit rhosts authentication. */
    int rhosts_rsa_authentication;      /* If true, permit rhosts RSA authentication. */
    int rsa_authentication;     /* If true, permit RSA authentication. */
    int kerberos_authentication;        /* If true, permit Kerberos authentication. */
    int kerberos_or_local_passwd;       /* If true, permit kerberos and any other
                                           password authentication mechanism, such
                                           as SecurID or /etc/passwd */
    int kerberos_tgt_passing;   /* If true, permit Kerberos tgt passing. */
    int allow_tcp_forwarding;
    int tis_authentication;     /* If true, permit TIS authsrv auth. */
    int password_authentication;        /* If true, permit password authentication. */
    int permit_empty_passwd;    /* If false, do not permit empty passwords. */
    int use_login;              /* Use /bin/login if possible */
    int silent_deny;            /* 1 = deny by closing sockets. */
    int forced_empty_passwd_change;     /* If true, force password change if empty
                                           password (first login). */
    int forced_passwd_change;   /* If true, force password change if password
                                   too old. */
    int umask;                  /* Umask */
    int check_mail;             /* If true, check mail spool at login */
    unsigned int num_allow_shosts;
    char *allow_shosts[MAX_ALLOW_SHOSTS];
    unsigned int num_deny_shosts;
    char *deny_shosts[MAX_DENY_SHOSTS];
    unsigned int num_allow_hosts;
    char *allow_hosts[MAX_ALLOW_HOSTS];
    unsigned int num_deny_hosts;
    char *deny_hosts[MAX_DENY_HOSTS];
    unsigned int num_allow_users;
    char *allow_users[MAX_ALLOW_USERS];
    unsigned int num_deny_users;
    char *deny_users[MAX_DENY_USERS];
    unsigned int num_allow_groups;
    char *allow_groups[MAX_ALLOW_GROUPS];
    unsigned int num_deny_groups;
    char *deny_groups[MAX_DENY_GROUPS];

    char *xauth_path;

#ifdef F_SECURE_COMMERCIAL








#endif                          /* F_SECURE_COMMERCIAL */
    int password_expire_warning_days;
    int account_expire_warning_days;
} ServerOptions;

/* Initializes the server options to special values that indicate that they
   have not yet been set. */
void initialize_server_options(ServerOptions * options);

/* Reads the server configuration file.  This only sets the values for those
   options that have the special value indicating they have not been set. */
void read_server_config(ServerOptions * options, const char *filename);

/* Sets values for those values that have not yet been set. */
void fill_default_server_options(ServerOptions * options);

#endif                          /* SERVCONF_H */
