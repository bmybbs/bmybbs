/*

tildexpand.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Wed Jul 12 01:07:36 1995 ylo

*/

/*
 * $Id: tildexpand.c,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: tildexpand.c,v $
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
 * Revision 1.3  2002/08/04 11:39:44  kcn
 * format c
 *
 * Revision 1.2  2002/08/04 11:08:49  kcn
 * format C
 *
 * Revision 1.1.1.1  2002/04/27 05:47:26  kxn
 * no message
 *
 * Revision 1.1  2001/07/04 06:07:13  bbsdev
 * bbs sshd
 *
 * Revision 1.3  1999/02/22 08:14:14  tri
 * 	Final fixes for 1.2.27.
 *
 * Revision 1.2  1999/02/21 19:52:59  ylo
 *      Intermediate commit of ssh1.2.27 stuff.
 *      Main change is sprintf -> snprintf; however, there are also
 *      many other changes.
 *
 * Revision 1.1.1.1  1996/02/18 21:38:12  ylo
 *      Imported ssh-1.2.13.
 *
 * Revision 1.2  1995/07/13  01:41:03  ylo
 *      Removed "Last modified" header.
 *      Added cvs log.
 *
 * $Endlog$
 */

#include "includes.h"
#include "xmalloc.h"
#include "ssh.h"

/* Expands tildes in the file name.  Returns data allocated by xmalloc.
   Warning: this calls getpw*. */

char *tilde_expand_filename(const char *filename, uid_t my_uid)
{
    const char *cp;
    unsigned int userlen;
    char *expanded;
    struct passwd *pw;
    char user[100];
    const char *homedir;

    /* Return immediately if no tilde. */
    if (filename[0] != '~')
        return xstrdup(filename);


    /* Skip the tilde. */
    filename++;

    /* Find where the username ends. */
    cp = strchr(filename, '/');
    if (cp)
        userlen = cp - filename;        /* Have something after username. */
    else
        userlen = strlen(filename);     /* Nothign after username. */
    if (userlen == 0) {
        strcpy(user, "(self)");
        pw = getpwuid(my_uid);  /* Own home directory. */
    } else {
        /* Tilde refers to someone elses home directory. */
        if (userlen > sizeof(user) - 1)
            fatal("User name after tilde too long.");
        memcpy(user, filename, userlen);
        user[userlen] = 0;
        pw = getpwnam(user);
    }

    /* Check that we found the user. */
    if (!pw)
        fatal("Unknown user %.100s.", user);

    homedir = pw->pw_dir;

    /* If referring to someones home directory, return it now. */
    if (!cp) {                  /* Only home directory specified */
        return xstrdup(homedir);
    }

    /* Build a path combining the specified directory and path. */
    expanded = xmalloc(strlen(homedir) + strlen(cp + 1) + 2);
    snprintf(expanded, strlen(homedir) + strlen(cp + 1) + 2, "%s/%s", homedir, cp + 1);
    return expanded;
}
