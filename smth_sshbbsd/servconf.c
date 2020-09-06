/*

servconf.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Mon Aug 21 15:48:58 1995 ylo

*/

/*
 * $Id: servconf.c,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: servconf.c,v $
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
 * Revision 1.14  1998/05/23 20:34:11  kivinen
 * 	Added forced_empty_passwd_change, num_deny_shosts,
 * 	num_allow_shosts, password_expire_warning_days,
 * 	account_expire_warning_days. Fixed typo in
 * 	forcedpasswordchange.
 *
 * Revision 1.13  1998/03/27  16:59:58  kivinen
 * 	Added IgnoreRootRhosts option.
 *
 * Revision 1.12  1998/01/03 06:41:55  kivinen
 * 	Added allow/deny groups option.
 *
 * Revision 1.11  1998/01/02 06:20:33  kivinen
 * 	Added xauthlocation and checkmail options.
 *
 * Revision 1.10  1997/04/27 21:51:34  kivinen
 * 	Added F-SECURE stuff. Added {Allow,Deny}Forwarding{To,Port}
 * 	feature. Added {Allow,Deny}Users feature from Steve Kann
 * 	<stevek@SteveK.COM>.
 *
 * Revision 1.9  1997/04/21 01:03:59  kivinen
 * 	Fixed allow_tcp_forwarding option default to yes.
 *
 * Revision 1.8  1997/04/05 21:50:07  kivinen
 * 	Fixed bug in allow_tcp_forwarding code.
 *
 * Revision 1.7  1997/03/27 03:14:16  kivinen
 * 	Changed sAllow_Tcp_Forwarding to sAllowTcpForwarding and
 * 	sKerberos_Or_Local_Passwd to sKerberosOrLocalPasswd.
 *
 * Revision 1.6  1997/03/27 03:12:39  kivinen
 * 	Added kerberos patches from Glenn Machin.
 * 	Added USELOGIN patches from Brian Cully.
 *
 * Revision 1.5  1997/03/26 05:33:16  kivinen
 * 	Added idle_timeout option.
 *
 * Revision 1.4  1997/03/25 05:44:38  kivinen
 * 	Added SilentDeny and Umask options.
 * 	Added = to WHITESPACE to allow options in form foo=bar.
 * 	Changed keywords to be case insensitive.
 *
 * Revision 1.3  1997/03/19 17:55:04  kivinen
 * 	Added TIS authentication code from Andre April
 * 	<Andre.April@cediti.be>.
 * 	Added SECURE_RPC, SECURE_NFS and NIS_PLUS support from Andy
 * 	Polyakov <appro@fy.chalmers.se>.
 *
 * Revision 1.2  1996/11/27 15:38:27  ttsalo
 *     Added X11DisplayOffset-option
 *
 * Revision 1.1.1.1  1996/02/18 21:38:12  ylo
 * 	Imported ssh-1.2.13.
 *
 * $EndLog$
 */

#include "includes.h"
#include "ssh.h"
#include "servconf.h"
#include "xmalloc.h"

/* Initializes the server options to their default values. */

void initialize_server_options(ServerOptions * options)
{
    memset(options, 0, sizeof(*options));
    options->port = -1;
    options->listen_addr = in6addr_any;	 //ipv6 by leoncom
    options->host_key_file = NULL;
    options->random_seed_file = NULL;
    options->pid_file = NULL;
    options->server_key_bits = -1;
    options->login_grace_time = -1;
    options->key_regeneration_time = -1;
    options->permit_root_login = -1;
    options->ignore_rhosts = -1;
    options->ignore_root_rhosts = -1;
    options->quiet_mode = -1;
    options->fascist_logging = -1;
    options->print_motd = -1;
    options->x11_forwarding = -1;
    options->x11_display_offset = -1;
    options->strict_modes = -1;
    options->keepalives = -1;
    options->log_facility = (SyslogFacility) - 1;
    options->rhosts_authentication = -1;
    options->rhosts_rsa_authentication = -1;
    options->rsa_authentication = -1;
    options->kerberos_authentication = -1;
    options->kerberos_or_local_passwd = -1;
    options->kerberos_tgt_passing = -1;
    options->tis_authentication = -1;
    options->allow_tcp_forwarding = -1;
    options->password_authentication = -1;
    options->permit_empty_passwd = -1;
    options->use_login = -1;
    options->silent_deny = -1;
    options->forced_passwd_change = -1;
    options->forced_empty_passwd_change = -1;
    options->num_allow_shosts = 0;
    options->num_deny_shosts = 0;
    options->num_allow_hosts = 0;
    options->num_deny_hosts = 0;
    options->num_allow_users = 0;
    options->num_deny_users = 0;
    options->num_allow_groups = 0;
    options->num_deny_groups = 0;
#ifdef F_SECURE_COMMERCIAL




#endif                          /* F_SECURE_COMMERCIAL */
    options->umask = -1;
    options->idle_timeout = -1;
    options->xauth_path = NULL;
    options->check_mail = -1;
    options->password_expire_warning_days = -1;
    options->account_expire_warning_days = -1;
}

void fill_default_server_options(ServerOptions * options)
{
    if (options->port == -1) {
        struct servent *sp;

        sp = getservbyname(SSH_SERVICE_NAME, "tcp");
        if (sp)
            options->port = ntohs(sp->s_port);
        else
            options->port = SSH_DEFAULT_PORT;
        endservent();
    }
    if (options->host_key_file == NULL)
        options->host_key_file = HOST_KEY_FILE;
    if (options->random_seed_file == NULL)
        options->random_seed_file = SSH_DAEMON_SEED_FILE;
    if (options->pid_file == NULL)
        options->pid_file = SSH_DAEMON_PID_FILE;
    if (options->server_key_bits == -1)
        options->server_key_bits = 768;
    if (options->login_grace_time == -1)
        options->login_grace_time = 600;
    if (options->key_regeneration_time == -1)
        options->key_regeneration_time = 3600;
    if (options->permit_root_login == -1)
        options->permit_root_login = 2;
    if (options->ignore_rhosts == -1)
        options->ignore_rhosts = 0;
    if (options->ignore_root_rhosts == -1)
        options->ignore_root_rhosts = options->ignore_rhosts;
    if (options->quiet_mode == -1)
        options->quiet_mode = 0;
    if (options->fascist_logging == -1)
        options->fascist_logging = 1;
    if (options->print_motd == -1)
        options->print_motd = 1;
    if (options->x11_forwarding == -1)
        options->x11_forwarding = 1;
    if (options->x11_display_offset == -1)
        options->x11_display_offset = 1;
    if (options->strict_modes == -1)
        options->strict_modes = 1;
    if (options->keepalives == -1)
        options->keepalives = 1;
    if (options->log_facility == (SyslogFacility) (-1))
        options->log_facility = SYSLOG_FACILITY_DAEMON;
    if (options->rhosts_authentication == -1)
        options->rhosts_authentication = 0;
    if (options->rhosts_rsa_authentication == -1)
        options->rhosts_rsa_authentication = 1;
    if (options->rsa_authentication == -1)
        options->rsa_authentication = 1;
    if (options->kerberos_authentication == -1)
#if defined(KERBEROS) && defined(KRB5)
        options->kerberos_authentication = 1;
#else                           /* defined(KERBEROS) && defined(KRB5) */
        options->kerberos_authentication = 0;
#endif                          /* defined(KERBEROS) && defined(KRB5) */
    if (options->kerberos_or_local_passwd == -1)
        options->kerberos_or_local_passwd = 0;
    if (options->kerberos_tgt_passing == -1)
#if defined(KERBEROS_TGT_PASSING) && defined(KRB5)
        options->kerberos_tgt_passing = 1;
#else                           /* defined(KERBEROS_TGT_PASSING) && defined(KRB5) */
        options->kerberos_tgt_passing = 0;
#endif                          /* defined(KERBEROS_TGT_PASSING) && defined(KRB5) */
    if (options->allow_tcp_forwarding == -1)
        options->allow_tcp_forwarding = 1;
    if (options->tis_authentication == -1)
        options->tis_authentication = 0;
    if (options->password_authentication == -1)
        options->password_authentication = 1;
    if (options->permit_empty_passwd == -1)
        options->permit_empty_passwd = 1;
    if (options->use_login == -1)
        options->use_login = 0;
    if (options->silent_deny == -1)
        options->silent_deny = 0;
    if (options->forced_passwd_change == -1)
        options->forced_passwd_change = 1;
    if (options->forced_empty_passwd_change == -1)
        options->forced_empty_passwd_change = 0;
    if (options->idle_timeout == -1)
        options->idle_timeout = 0;
    if (options->check_mail == -1)
        options->check_mail = 1;
#ifdef XAUTH_PATH
    if (options->xauth_path == NULL)
        options->xauth_path = XAUTH_PATH;
#else                           /* !XAUTH_PATH */
    if (options->xauth_path == NULL)
        options->xauth_path = "xauth";
#endif                          /* !XAUTH_PATH */
    if (options->password_expire_warning_days == -1)
        options->password_expire_warning_days = 14;
    if (options->account_expire_warning_days == -1)
        options->account_expire_warning_days = 14;
}

#define WHITESPACE " \t\r\n="

/* Keyword tokens. */
typedef enum {
    sPort, sHostKeyFile, sServerKeyBits, sLoginGraceTime, sKeyRegenerationTime,
    sPermitRootLogin, sQuietMode, sFascistLogging, sLogFacility,
    sRhostsAuthentication, sRhostsRSAAuthentication, sRSAAuthentication,
    sTISAuthentication, sPasswordAuthentication, sAllowHosts, sDenyHosts,
    sListenAddress, sPrintMotd, sIgnoreRhosts, sX11Forwarding, sX11DisplayOffset,
    sStrictModes, sEmptyPasswd, sRandomSeedFile, sKeepAlives, sPidFile,
    sForcedPasswd, sForcedEmptyPasswd, sUmask, sSilentDeny, sIdleTimeout,
    sUseLogin, sKerberosAuthentication, sKerberosOrLocalPasswd,
    sKerberosTgtPassing, sAllowTcpForwarding, sAllowUsers, sDenyUsers,
    sXauthPath, sCheckMail, sDenyGroups, sAllowGroups, sIgnoreRootRhosts,
    sAllowSHosts, sDenySHosts, sPasswordExpireWarningDays,
    sAccountExpireWarningDays
#ifdef F_SECURE_COMMERCIAL
#endif                          /* F_SECURE_COMMERCIAL */
} ServerOpCodes;

/* Textual representation of the tokens. */
static struct {
    const char *name;
    ServerOpCodes opcode;
} keywords[] = {
    {
    "port", sPort}, {
    "hostkey", sHostKeyFile}, {
    "serverkeybits", sServerKeyBits}, {
    "logingracetime", sLoginGraceTime}, {
    "keyregenerationinterval", sKeyRegenerationTime}, {
    "permitrootlogin", sPermitRootLogin}, {
    "quietmode", sQuietMode}, {
    "fascistlogging", sFascistLogging}, {
    "syslogfacility", sLogFacility}, {
    "rhostsauthentication", sRhostsAuthentication}, {
    "rhostsrsaauthentication", sRhostsRSAAuthentication}, {
    "rsaauthentication", sRSAAuthentication}, {
    "tisauthentication", sTISAuthentication}, {
    "passwordauthentication", sPasswordAuthentication}, {
    "uselogin", sUseLogin}, {
    "allowshosts", sAllowSHosts}, {
    "denyshosts", sDenySHosts}, {
    "allowhosts", sAllowHosts}, {
    "denyhosts", sDenyHosts}, {
    "allowusers", sAllowUsers}, {
    "denyusers", sDenyUsers}, {
    "allowgroups", sAllowGroups}, {
    "denygroups", sDenyGroups},
#ifdef F_SECURE_COMMERCIAL
#endif                          /* F_SECURE_COMMERCIAL */
    {
    "listenaddress", sListenAddress}, {
    "printmotd", sPrintMotd}, {
    "ignorerhosts", sIgnoreRhosts}, {
    "ignorerootrhosts", sIgnoreRootRhosts}, {
    "x11forwarding", sX11Forwarding}, {
    "x11displayoffset", sX11DisplayOffset}, {
    "strictmodes", sStrictModes}, {
    "permitemptypasswords", sEmptyPasswd}, {
    "forcedpasswdchange", sForcedPasswd}, {
    "randomseed", sRandomSeedFile}, {
    "keepalive", sKeepAlives}, {
    "pidfile", sPidFile}, {
    "umask", sUmask}, {
    "silentdeny", sSilentDeny}, {
    "idletimeout", sIdleTimeout}, {
    "kerberosauthentication", sKerberosAuthentication}, {
    "kerberosorlocalpasswd", sKerberosOrLocalPasswd}, {
    "kerberostgtpassing", sKerberosTgtPassing}, {
    "allowtcpforwarding", sAllowTcpForwarding}, {
    "xauthlocation", sXauthPath}, {
    "checkmail", sCheckMail}, {
    "passwordexpirewarningdays", sPasswordExpireWarningDays}, {
    "accountexpirewarningdays", sAccountExpireWarningDays}, {
    NULL, 0}
};

static struct {
    const char *name;
    SyslogFacility facility;
} log_facilities[] = {
    {
    "daemon", SYSLOG_FACILITY_DAEMON}, {
    "user", SYSLOG_FACILITY_USER}, {
    "auth", SYSLOG_FACILITY_AUTH}, {
    "local0", SYSLOG_FACILITY_LOCAL0}, {
    "local1", SYSLOG_FACILITY_LOCAL1}, {
    "local2", SYSLOG_FACILITY_LOCAL2}, {
    "local3", SYSLOG_FACILITY_LOCAL3}, {
    "local4", SYSLOG_FACILITY_LOCAL4}, {
    "local5", SYSLOG_FACILITY_LOCAL5}, {
    "local6", SYSLOG_FACILITY_LOCAL6}, {
    "local7", SYSLOG_FACILITY_LOCAL7}, {
    NULL, 0}
};

/* Returns the number of the token pointed to by cp of length len.
   Never returns if the token is not known. */

static ServerOpCodes parse_token(const char *cp, const char *filename, int linenum)
{
    unsigned int i;

    for (i = 0; keywords[i].name; i++)
        if (strcmp(cp, keywords[i].name) == 0)
            return keywords[i].opcode;

    fprintf(stderr, "%s line %d: Bad configuration option: %s\n", filename, linenum, cp);
    exit(1);
}

/* Reads the server configuration file. */

void read_server_config(ServerOptions * options, const char *filename)
{
    FILE *f;
    char line[1024];
    char *cp, **charptr;
    int linenum, *intptr, i, value;
    ServerOpCodes opcode;

    f = fopen(filename, "r");
    if (!f) {
        perror(filename);
        return;
    }

    linenum = 0;
    while (fgets(line, sizeof(line), f)) {
        linenum++;
        cp = line + strspn(line, WHITESPACE);
        if (!*cp || *cp == '#')
            continue;
        cp = strtok(cp, WHITESPACE);
        for (i = 0; cp[i]; i++)
            cp[i] = tolower(cp[i]);
        opcode = parse_token(cp, filename, linenum);
        switch (opcode) {
        case sPort:
            intptr = &options->port;
          parse_int:
            cp = strtok(NULL, WHITESPACE);
            if (!cp) {
                fprintf(stderr, "%s line %d: missing integer value.\n", filename, linenum);
                exit(1);
            }
            if (*cp == '0') {   /* Octal or hex */
                int base;

                cp++;
                if (*cp == 'x') {       /* Hex */
                    cp++;
                    base = 16;
                } else
                    base = 8;
                value = 0;
                while ((base == 16 && isxdigit(*cp)) || (base == 8 && isdigit(*cp) && *cp < '8')) {
                    value *= base;
                    if (*cp >= 'a' && *cp <= 'f')
                        value += *cp - 'a' + 10;
                    else if (*cp >= 'A' && *cp <= 'F')
                        value += *cp - 'A' + 10;
                    else
                        value += *cp - '0';
                    cp++;
                }
            } else {
                value = atoi(cp);
            }
            if (*intptr == -1)
                *intptr = value;
            break;

        case sServerKeyBits:
            intptr = &options->server_key_bits;
            goto parse_int;

        case sLoginGraceTime:
            intptr = &options->login_grace_time;
            goto parse_int;

        case sKeyRegenerationTime:
            intptr = &options->key_regeneration_time;
            goto parse_int;

        case sListenAddress:
            cp = strtok(NULL, WHITESPACE);
            if (!cp) {
                fprintf(stderr, "%s line %d: missing inet addr.\n", filename, linenum);
                exit(1);
            }
#ifdef BROKEN_INET_ADDR
            //options->listen_addr.s_addr = inet_network(cp);
	    inet_pton(AF_INET6,cp,&(options->listen_addr));
#else                           /* BROKEN_INET_ADDR */
            //options->listen_addr.s_addr = inet_addr(cp);
	    inet_pton(AF_INET6,cp,&(options->listen_addr));  //ipv6 by leoncom
#endif                          /* BROKEN_INET_ADDR */
            break;

        case sHostKeyFile:
            charptr = &options->host_key_file;
          parse_pathname:
            cp = strtok(NULL, WHITESPACE);
            if (!cp) {
                fprintf(stderr, "%s line %d: missing file name.\n", filename, linenum);
                exit(1);
            }
            if (*charptr == NULL)
                *charptr = tilde_expand_filename(cp, getuid());
            break;

        case sRandomSeedFile:
            charptr = &options->random_seed_file;
            goto parse_pathname;

        case sPidFile:
            charptr = &options->pid_file;
            goto parse_pathname;

        case sPermitRootLogin:
            cp = strtok(NULL, WHITESPACE);
            if (!cp) {
                fprintf(stderr, "%s line %d: missing yes/nopwd/no argument.\n", filename, linenum);
                exit(1);
            }
            for (i = 0; cp[i]; i++)
                cp[i] = tolower(cp[i]);
            if (strcmp(cp, "yes") == 0)
                value = 2;
            else if (strcmp(cp, "nopwd") == 0)
                value = 1;
            else if (strcmp(cp, "no") == 0)
                value = 0;
            else {
                fprintf(stderr, "%s line %d: Bad yes/nopwd/no argument: %s\n", filename, linenum, cp);
                exit(1);
            }
            if (options->permit_root_login == -1)
                options->permit_root_login = value;
            break;

          parse_flag:
            cp = strtok(NULL, WHITESPACE);
            if (!cp) {
                fprintf(stderr, "%s line %d: missing yes/no argument.\n", filename, linenum);
                exit(1);
            }
            for (i = 0; cp[i]; i++)
                cp[i] = tolower(cp[i]);
            if (strcmp(cp, "yes") == 0 || strcmp(cp, "true") == 0)
                value = 1;
            else if (strcmp(cp, "no") == 0 || strcmp(cp, "false") == 0)
                value = 0;
            else {
                fprintf(stderr, "%s line %d: Bad yes/no argument: %s\n", filename, linenum, cp);
                exit(1);
            }
            if (*intptr == -1)
                *intptr = value;
            break;

        case sIgnoreRhosts:
            intptr = &options->ignore_rhosts;
            goto parse_flag;

        case sIgnoreRootRhosts:
            intptr = &options->ignore_root_rhosts;
            goto parse_flag;

        case sQuietMode:
            intptr = &options->quiet_mode;
            goto parse_flag;

        case sFascistLogging:
            intptr = &options->fascist_logging;
            goto parse_flag;

        case sRhostsAuthentication:
            intptr = &options->rhosts_authentication;
            goto parse_flag;

        case sRhostsRSAAuthentication:
            intptr = &options->rhosts_rsa_authentication;
            goto parse_flag;

        case sRSAAuthentication:
            intptr = &options->rsa_authentication;
            goto parse_flag;

        case sKerberosAuthentication:
            intptr = &options->kerberos_authentication;
            goto parse_flag;

        case sKerberosOrLocalPasswd:
            intptr = &options->kerberos_or_local_passwd;
            goto parse_flag;

        case sKerberosTgtPassing:
            intptr = &options->kerberos_tgt_passing;
            goto parse_flag;

        case sAllowTcpForwarding:
            intptr = &options->allow_tcp_forwarding;
            goto parse_flag;

        case sTISAuthentication:
            intptr = &options->tis_authentication;
            goto parse_flag;

        case sPasswordAuthentication:
            intptr = &options->password_authentication;
            goto parse_flag;

        case sUseLogin:
            intptr = &options->use_login;
            goto parse_flag;

        case sPrintMotd:
            intptr = &options->print_motd;
            goto parse_flag;

        case sX11Forwarding:
            intptr = &options->x11_forwarding;
            goto parse_flag;

        case sX11DisplayOffset:
            intptr = &options->x11_display_offset;
            goto parse_int;

        case sStrictModes:
            intptr = &options->strict_modes;
            goto parse_flag;

        case sKeepAlives:
            intptr = &options->keepalives;
            goto parse_flag;

        case sEmptyPasswd:
            intptr = &options->permit_empty_passwd;
            goto parse_flag;

        case sSilentDeny:
            intptr = &options->silent_deny;
            goto parse_flag;

        case sForcedPasswd:
            intptr = &options->forced_passwd_change;
            goto parse_flag;

        case sForcedEmptyPasswd:
            intptr = &options->forced_empty_passwd_change;
            goto parse_flag;

        case sUmask:
            intptr = &options->umask;
            goto parse_int;

        case sIdleTimeout:
            cp = strtok(NULL, WHITESPACE);
            if (!cp) {
                fprintf(stderr, "%s line %d: missing integer value.\n", filename, linenum);
                exit(1);
            }
            value = 0;
            while (isdigit(*cp)) {
                value *= 10;
                value += *cp - '0';
                cp++;
            }
            *cp = tolower(*cp);
            if (*cp == 'w') {   /* Weeks */
                value *= 7 * 24 * 60 * 60;
                cp++;
            } else if (*cp == 'd') {    /* Days */
                value *= 24 * 60 * 60;
                cp++;
            } else if (*cp == 'h') {    /* Hours */
                value *= 60 * 60;
                cp++;
            } else if (*cp == 'm') {    /* Minutes */
                value *= 60;
                cp++;
            } else if (*cp == 's') {
                cp++;
            }
            options->idle_timeout = value;
            break;

        case sLogFacility:
            cp = strtok(NULL, WHITESPACE);
            if (!cp) {
                fprintf(stderr, "%s line %d: missing facility name.\n", filename, linenum);
                exit(1);
            }
            for (i = 0; cp[i]; i++)
                cp[i] = tolower(cp[i]);
            for (i = 0; log_facilities[i].name; i++)
                if (strcmp(log_facilities[i].name, cp) == 0)
                    break;
            if (!log_facilities[i].name) {
                fprintf(stderr, "%s line %d: unsupported log facility %s\n", filename, linenum, cp);
                exit(1);
            }
            if (options->log_facility == (SyslogFacility) (-1))
                options->log_facility = log_facilities[i].facility;
            break;

        case sAllowSHosts:
            while ((cp = strtok(NULL, WHITESPACE))) {
                if (options->num_allow_shosts >= MAX_ALLOW_SHOSTS) {
                    fprintf(stderr, "%s line %d: too many allow shosts.\n", filename, linenum);
                    exit(1);
                }
                options->allow_shosts[options->num_allow_shosts++] = xstrdup(cp);
            }
            break;

        case sDenySHosts:
            while ((cp = strtok(NULL, WHITESPACE))) {
                if (options->num_deny_shosts >= MAX_DENY_SHOSTS) {
                    fprintf(stderr, "%s line %d: too many deny shosts.\n", filename, linenum);
                    exit(1);
                }
                options->deny_shosts[options->num_deny_shosts++] = xstrdup(cp);
            }
            break;

        case sAllowHosts:
            while ((cp = strtok(NULL, WHITESPACE))) {
                if (options->num_allow_hosts >= MAX_ALLOW_HOSTS) {
                    fprintf(stderr, "%s line %d: too many allow hosts.\n", filename, linenum);
                    exit(1);
                }
                options->allow_hosts[options->num_allow_hosts++] = xstrdup(cp);
            }
            break;

        case sDenyHosts:
            while ((cp = strtok(NULL, WHITESPACE))) {
                if (options->num_deny_hosts >= MAX_DENY_HOSTS) {
                    fprintf(stderr, "%s line %d: too many deny hosts.\n", filename, linenum);
                    exit(1);
                }
                options->deny_hosts[options->num_deny_hosts++] = xstrdup(cp);
            }
            break;

        case sAllowUsers:
            while ((cp = strtok(NULL, WHITESPACE))) {
                if (options->num_allow_users >= MAX_ALLOW_USERS) {
                    fprintf(stderr, "%s line %d: too many allow users.\n", filename, linenum);
                    exit(1);
                }
                options->allow_users[options->num_allow_users++] = xstrdup(cp);
            }
            break;

        case sDenyUsers:
            while ((cp = strtok(NULL, WHITESPACE))) {
                if (options->num_deny_users >= MAX_DENY_USERS) {
                    fprintf(stderr, "%s line %d: too many deny users.\n", filename, linenum);
                    exit(1);
                }
                options->deny_users[options->num_deny_users++] = xstrdup(cp);
            }
            break;

        case sAllowGroups:
            while ((cp = strtok(NULL, WHITESPACE))) {
                if (options->num_allow_groups >= MAX_ALLOW_GROUPS) {
                    fprintf(stderr, "%s line %d: too many allow groups.\n", filename, linenum);
                    exit(1);
                }
                options->allow_groups[options->num_allow_groups++] = xstrdup(cp);
            }
            break;

        case sDenyGroups:
            while ((cp = strtok(NULL, WHITESPACE))) {
                if (options->num_deny_groups >= MAX_DENY_GROUPS) {
                    fprintf(stderr, "%s line %d: too many deny groups.\n", filename, linenum);
                    exit(1);
                }
                options->deny_groups[options->num_deny_groups++] = xstrdup(cp);
            }
            break;

        case sXauthPath:
            charptr = &options->xauth_path;
            goto parse_pathname;

        case sCheckMail:
            intptr = &options->check_mail;
            goto parse_flag;

        case sPasswordExpireWarningDays:
            intptr = &options->password_expire_warning_days;
            goto parse_int;

        case sAccountExpireWarningDays:
            intptr = &options->account_expire_warning_days;
            goto parse_int;

#ifdef F_SECURE_COMMERCIAL























































#endif                          /* F_SECURE_COMMERCIAL */

        default:
            fprintf(stderr, "%s line %d: Missing handler for opcode %s (%d)\n", filename, linenum, cp, opcode);
            exit(1);
        }
        if (strtok(NULL, WHITESPACE) != NULL) {
            fprintf(stderr, "%s line %d: garbage at end of line.\n", filename, linenum);
            exit(1);
        }
    }
    fclose(f);
}
