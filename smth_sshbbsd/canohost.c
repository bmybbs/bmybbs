/*

canohost.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Sun Jul  2 17:52:22 1995 ylo

Functions for returning the canonical host name of the remote site.

*/

/*
 * $Id: canohost.c,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: canohost.c,v $
 * Revision 1.1.1.1  2009-03-04 06:33:27  bmybbs
 * bmysrc
 *
 * Revision 1.1.1.1  2002/10/01 14:32:16  clearboy
 * update on 20051031
 * by clearboy 
 * for transfering the source codes from main site to the experimental site 
 * for the first time.
 *
 *
 * Revision 1.2  2002/10/01 14:32:16  lepton
 * 1.加入sshbbsd配置文件
 * 2.现在密码验证工作正常
 * 3.可能退出的一些环境清理有问题 以及执行外部程序比如穿梭
 *
 * Revision 1.1.1.1  2002/10/01 09:42:06  ylsdd
 * 水木底sshbbsd导入
 * 然后慢慢改吧
 *
 * Revision 1.3  2002/08/04 11:39:41  kcn
 * format c
 *
 * Revision 1.2  2002/08/04 11:08:45  kcn
 * format C
 *
 * Revision 1.1.1.1  2002/04/27 05:47:26  kxn
 * no message
 *
 * Revision 1.1  2001/07/04 06:07:08  bbsdev
 * bbs sshd
 *
 * Revision 1.5  1999/02/21 19:51:58  ylo
 * 	Intermediate commit of ssh1.2.27 stuff.
 * 	Main change is sprintf -> snprintf; however, there are also
 * 	many other changes.
 *
 * Revision 1.4  1998/05/23 20:21:01  kivinen
 *      Changed () -> (void).
 *
 * Revision 1.3  1997/03/19  15:59:45  kivinen
 *      Limit hostname to 255 characters.
 *
 * Revision 1.2  1996/10/29 22:35:11  kivinen
 *      log -> log_msg.
 *
 * Revision 1.1.1.1  1996/02/18 21:38:12  ylo
 *      Imported ssh-1.2.13.
 *
 * Revision 1.5  1995/09/21  17:08:24  ylo
 *      Added get_remote_port.
 *
 * Revision 1.4  1995/09/06  15:57:59  ylo
 *      Fixed serious bugs.
 *
 * Revision 1.3  1995/08/29  22:20:12  ylo
 *      Added code to get ip number as string.
 *
 * Revision 1.2  1995/07/13  01:19:18  ylo
 *      Removed "Last modified" header.
 *      Added cvs log.
 *
 * $Endlog$
 */

#include "includes.h"
#include "packet.h"
#include "xmalloc.h"
#include "ssh.h"

/* Return the canonical name of the host at the other end of the socket. 
   The caller should free the returned string with xfree. */

#if 0
char *get_remote_hostname(int socket)
{
    struct sockaddr_in6 from;
    int fromlen, i;
    struct hostent *hp;
    char name[255];

    /* Get IP address of client. */
    fromlen = sizeof(from);
    memset(&from, 0, sizeof(from));
    if (getpeername(socket, (struct sockaddr *) &from, &fromlen) < 0) {
        error("getpeername failed: %.100s", strerror(errno));
        strcpy(name, "UNKNOWN");
        goto check_ip_options;
    }

    /* Map the IP address to a host name. ipv6 */
    hp = gethostbyaddr((char *) &from.sin6_addr, sizeof(struct in6_addr), from.sin6_family);
    if (hp) {
        /* Got host name. */
        strncpy(name, hp->h_name, sizeof(name));
        name[sizeof(name) - 1] = '\0';

        /* Convert it to all lowercase (which is expected by the rest of this
           software). */
        for (i = 0; name[i]; i++)
            if (isupper(name[i]))
                name[i] = tolower(name[i]);

        /* Map it back to an IP address and check that the given address actually
           is an address of this host.  This is necessary because anyone with
           access to a name server can define arbitrary names for an IP address.
           Mapping from name to IP address can be trusted better (but can still
           be fooled if the intruder has access to the name server of the
           domain). */
        hp = gethostbyname(name);
        if (!hp) {
            log_msg("reverse mapping checking gethostbyname for %.700s failed - POSSIBLE BREAKIN ATTEMPT!", name);
            //strcpy(name, inet_ntoa(from.sin_addr));
	    //ipv6 by leoncom
	    inet_ntop(AF_INET6,&from.sin6_addr,name,INET6_ADDRSTRLEN);
            goto check_ip_options;
        }
        /* Look for the address from the list of addresses. */
        for (i = 0; hp->h_addr_list[i]; i++)
            if (memcmp(hp->h_addr_list[i], &from.sin6_addr, sizeof(from.sin6_addr))
                == 0)
                break;
        /* If we reached the end of the list, the address was not there. */
        if (!hp->h_addr_list[i]) {
            /* Address not found for the host name. */
            //log_msg("Address %.100s maps to %.600s, but this does not map back to the address - POSSIBLE BREAKIN ATTEMPT!", inet_ntoa(from.sin_addr), name);
            //strcpy(name, inet_ntoa(from.sin_addr));
	    inet_ntop(AF_INET6,&from.sin6_addr,name,INET6_ADDRSTRLEN);
            goto check_ip_options;
        }
        /* Address was found for the host name.  We accept the host name. */
    } else {
        /* Host name not found.  Use ascii representation of the address. */
        //strcpy(name, inet_ntoa(from.sin_addr));
	inet_ntop(AF_INET6,&from.sin6_addr,name,INET6_ADDRSTRLEN);
        log_msg("Could not reverse map address %.100s.", name);
    }

  check_ip_options:

#ifdef IP_OPTIONS
    /* If IP options are supported, make sure there are none (log and clear
       them if any are found).  Basically we are worried about source routing;
       it can be used to pretend you are somebody (ip-address) you are not.
       That itself may be "almost acceptable" under certain circumstances,
       but rhosts autentication is useless if source routing is accepted.
       Notice also that if we just dropped source routing here, the other
       side could use IP spoofing to do rest of the interaction and could still
       bypass security.  So we exit here if we detect any IP options. */
    {
        unsigned char options[200], *ucp;
        char text[1024], *cp;
        int option_size, ipproto;
        struct protoent *ip;

        if ((ip = getprotobyname("ip")) != NULL)
            ipproto = ip->p_proto;
        else
            ipproto = IPPROTO_IP;
        option_size = sizeof(options);
        if (getsockopt(socket, ipproto, IP_OPTIONS, (char *) options, &option_size) >= 0 && option_size != 0) {
            cp = text;
            if (option_size > 256)
                option_size = 256;
            /* Note: "text" buffer must be at least 3x as big as options. */
            for (ucp = options; option_size > 0; ucp++, option_size--, cp += 3)
                sprintf(cp, " %2.2x", *ucp);
	    char tmpname[256];  
	    inet_ntop(AF_INET6,&from.sin6_addr,tmpname,INET6_ADDRSTRLEN);
            log_msg("Connection from %.100s with IP options:%.800s", tmpname, text);
            packet_disconnect("Connection from %.100s with IP options:%.800s", tmpname, text);
        }
    }
#endif

    return xstrdup(name);
}
static char *canonical_host_name = NULL;
#endif
static char *canonical_host_ip = NULL;
#if 0
/* Return the canonical name of the host in the other side of the current
   connection.  The host name is cached, so it is efficient to call this 
   several times. */

const char *get_canonical_hostname(void)
{
    int fromlen, tolen;
    struct sockaddr_in6 from, to;

    /* Check if we have previously retrieved this same name. */
    if (canonical_host_name != NULL)
        return canonical_host_name;

    /* If using different descriptors for the two directions, check if
       both have the same remote address.  If so, get the address; otherwise
       return UNKNOWN. */
    if (packet_get_connection_in() != packet_get_connection_out()) {
        fromlen = sizeof(from);
        memset(&from, 0, sizeof(from));
        if (getpeername(packet_get_connection_in(), (struct sockaddr *) &from, &fromlen) < 0)
            goto no_ip_addr;

        tolen = sizeof(to);
        memset(&to, 0, sizeof(to));
        if (getpeername(packet_get_connection_out(), (struct sockaddr *) &to, &tolen) < 0)
            goto no_ip_addr;

        if (from.sin6_family == AF_INET6 && to.sin6_family == AF_INET6 && memcmp(&from, &to, sizeof(from)) == 0)
            goto return_ip_addr;

      no_ip_addr:
        canonical_host_name = xstrdup("UNKNOWN");
        return canonical_host_name;
    }

  return_ip_addr:

    /* Get the real hostname. */
    canonical_host_name = get_remote_hostname(packet_get_connection_in());
    return canonical_host_name;
}
#endif
/* Returns the IP-address of the remote host as a string.  The returned
   string need not be freed. */

extern int is4map6addr(char *s);
extern char *getv4addr(char *fromhost);

const char *get_remote_ipaddr(void)
{
    struct sockaddr_in6 from, to;
    socklen_t fromlen, tolen;
    int socket;

    /* Check if we have previously retrieved this same name. */
    if (canonical_host_ip != NULL)
        return canonical_host_ip;

    /* If using different descriptors for the two directions, check if
       both have the same remote address.  If so, get the address; otherwise
       return UNKNOWN. */
    if (packet_get_connection_in() != packet_get_connection_out()) {
        fromlen = sizeof(from);
        memset(&from, 0, sizeof(from));
        if (getpeername(packet_get_connection_in(), (struct sockaddr *) &from, &fromlen) < 0)
            goto no_ip_addr;

        tolen = sizeof(to);
        memset(&to, 0, sizeof(to));
        if (getpeername(packet_get_connection_out(), (struct sockaddr *) &to, &tolen) < 0)
            goto no_ip_addr;

        if (from.sin6_family == AF_INET6 && to.sin6_family == AF_INET6 && memcmp(&from, &to, sizeof(from)) == 0)
            goto return_ip_addr;

      no_ip_addr:
        canonical_host_ip = xstrdup("UNKNOWN");
        return canonical_host_ip;
    }

  return_ip_addr:

    /* Get client socket. */
    socket = packet_get_connection_in();

    /* Get IP address of client. */
    fromlen = sizeof(from);
    memset(&from, 0, sizeof(from));
    if (getpeername(socket, (struct sockaddr *) &from, &fromlen) < 0) {
        error("getpeername failed: %.100s", strerror(errno));
        return NULL;
    }

    /* Get the IP address in ascii. */
    char tmpname[256];
    inet_ntop(AF_INET6,&from.sin6_addr,tmpname,INET6_ADDRSTRLEN);
    canonical_host_ip = xstrdup(tmpname);
    //canonical_host_ip = xstrdup(inet_ntoa(from.sin_addr));

    /* Return ip address string. */
    if(is4map6addr(canonical_host_ip))    //if ipv6 addr
	canonical_host_ip = getv4addr(canonical_host_ip);
    return canonical_host_ip;
}

/* Returns the port of the peer of the socket. */

int get_peer_port(int sock)
{
    struct sockaddr_in6 from;
	socklen_t fromlen;

    /* Get IP address of client. */
    fromlen = sizeof(from);
    memset(&from, 0, sizeof(from));
    if (getpeername(sock, (struct sockaddr *) &from, &fromlen) < 0) {
        error("getpeername failed: %.100s", strerror(errno));
        return 0;
    }

    /* Return port number. */
    return ntohs(from.sin6_port);
}

/* Returns the port number of the remote host.  */

int get_remote_port(void)
{
    int socket;
	socklen_t fromlen, tolen;
    struct sockaddr_in6 from, to;

    /* If two different descriptors, check if they are internet-domain, and
       have the same address. */
    if (packet_get_connection_in() != packet_get_connection_out()) {
        fromlen = sizeof(from);
        memset(&from, 0, sizeof(from));
        if (getpeername(packet_get_connection_in(), (struct sockaddr *) &from, &fromlen) < 0)
            goto no_ip_addr;

        tolen = sizeof(to);
        memset(&to, 0, sizeof(to));
        if (getpeername(packet_get_connection_out(), (struct sockaddr *) &to, &tolen) < 0)
            goto no_ip_addr;

        if (from.sin6_family == AF_INET6 && to.sin6_family == AF_INET6 && memcmp(&from, &to, sizeof(from)) == 0)
            goto return_port;

      no_ip_addr:
        return 65535;
    }

  return_port:

    /* Get client socket. */
    socket = packet_get_connection_in();

    /* Get and return the peer port number. */
    return get_peer_port(socket);
}
