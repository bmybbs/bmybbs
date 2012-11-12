/*

xmalloc.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Mon Mar 20 22:09:17 1995 ylo

Versions of malloc and friends that check their results, and never return
failure (they call fatal if they encounter an error).

*/

/*
 * $Id: xmalloc.h,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: xmalloc.h,v $
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
 * Revision 1.2  2002/08/04 11:08:50  kcn
 * format C
 *
 * Revision 1.1.1.1  2002/04/27 05:47:25  kxn
 * no message
 *
 * Revision 1.1  2001/07/04 06:07:13  bbsdev
 * bbs sshd
 *
 * Revision 1.1.1.1  1996/02/18 21:38:10  ylo
 * 	Imported ssh-1.2.13.
 *
 * Revision 1.2  1995/07/13  01:41:41  ylo
 * 	Removed "Last modified" header.
 * 	Added cvs log.
 *
 * $Endlog$
 */

#ifndef XMALLOC_H
#define XMALLOC_H

/* Like malloc, but calls fatal() if out of memory. */
void *xmalloc(size_t size);

/* Like realloc, but calls fatal() if out of memory. */
void *xrealloc(void *ptr, size_t new_size);

/* Frees memory allocated using xmalloc or xrealloc. */
void xfree(void *ptr);

/* Allocates memory using xmalloc, and copies the string into that memory. */
char *xstrdup(const char *str);

#endif                          /* XMALLOC_H */
