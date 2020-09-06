/*

signals.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Fri Jan 19 18:09:37 1995 ylo

Manipulation of signal state.  This file also contains code to set the
maximum core dump size.

*/

/*
 * $Log: signals.c,v $
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
 * Revision 1.1  2001/07/04 06:07:12  bbsdev
 * bbs sshd
 *
 * Revision 1.9  1998/05/23 20:24:15  kivinen
 * 	Changed () -> (void).
 *
 * Revision 1.8  1998/05/04  13:37:05  kivinen
 * 	Fixed SIGPWR code so that will check if SIGPWR is same than
 * 	SIGINFO and only include it to switch clause if it is
 * 	different.
 *
 * Revision 1.7  1998/04/30 01:56:32  kivinen
 * 	Added SIGPWR handling.
 *
 * Revision 1.6  1997/04/21 01:07:28  kivinen
 * 	Added HAVE_INCOMPATIBLE_SIGINFO support.
 *
 * Revision 1.5  1997/03/26 07:16:44  kivinen
 * 	Change sig <= NSIG to sig < NSIG.
 *
 * Revision 1.4  1996/08/30 08:44:22  ylo
 * 	Added Sunos/Solaris SIGFREEZE and SIGTHAW to signals with
 * 	default processing.
 *
 * Revision 1.3  1996/07/12 07:27:18  ttsalo
 * 	ifdef:d SIGIO
 *
 * Revision 1.2  1996/04/26 00:25:48  ylo
 * 	Test for SIGURG == SIGIO (which appears to be the case on some
 * 	Linux versions).
 *
 * Revision 1.1.1.1  1996/02/18 21:38:11  ylo
 * 	Imported ssh-1.2.13.
 *
 * $EndLog$
 */

#include "includes.h"
#ifdef HAVE_SETRLIMIT
#include <sys/resource.h>
#endif                          /* HAVE_SETRLIMIT */

#ifndef NSIG
#define NSIG 100
#endif

unsigned long original_core_limit;

static RETSIGTYPE signal_handler(int sig)
{
    fprintf(stderr, "\nReceived signal %d.\n", sig);
    exit(255);
}

/* Sets signal handlers so that core dumps are prevented.  This also
   sets the maximum core dump size to zero as an extra precaution (where
   supported).  The old core dump size limit is saved. */

void signals_prevent_core(void)
{
    int sig;

    for (sig = 1; sig < NSIG; sig++)
        switch (sig) {
        case SIGSTOP:
        case SIGTSTP:
        case SIGCONT:
        case SIGCHLD:
        case SIGTTIN:
        case SIGTTOU:
#ifdef SIGIO
        case SIGIO:
#endif
#if defined(SIGURG) && SIGURG != SIGIO
        case SIGURG:
#endif
#ifdef SIGWINCH
        case SIGWINCH:
#endif
#if defined(SIGINFO) && !defined(HAVE_INCOMPATIBLE_SIGINFO)
        case SIGINFO:
#endif
#if defined(SIGFREEZE)
        case SIGFREEZE:
#endif
#if defined(SIGTHAW)
        case SIGTHAW:
#endif
#if defined(SIGPWR)
#if !defined(SIGINFO) || (SIGINFO != SIGPWR)
        case SIGPWR:
#endif
#endif
            signal(sig, SIG_DFL);
            break;
        default:
            signal(sig, signal_handler);
            break;
        }

#if defined(HAVE_SETRLIMIT) && defined(RLIMIT_CORE)
    {
        struct rlimit rl;

        getrlimit(RLIMIT_CORE, &rl);
        original_core_limit = rl.rlim_cur;
        rl.rlim_cur = 0;
        setrlimit(RLIMIT_CORE, &rl);
    }
#endif                          /* HAVE_SETRLIMIT && RLIMIT_CORE */
}

/* Sets all signals to their default state.  Restores RLIMIT_CORE previously
   saved by prevent_core(). */

void signals_reset(void)
{
    int sig;

    for (sig = 1; sig < NSIG; sig++)
        signal(sig, SIG_DFL);

#if defined(HAVE_SETRLIMIT) && defined(RLIMIT_CORE)
    {
        struct rlimit rl;

        getrlimit(RLIMIT_CORE, &rl);
        rl.rlim_cur = original_core_limit;
        setrlimit(RLIMIT_CORE, &rl);
    }
#endif                          /* HAVE_SETRLIMIT && RLIMIT_CORE */
}
