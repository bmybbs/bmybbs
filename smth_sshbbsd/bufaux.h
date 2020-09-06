/*

bufaux.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Wed Mar 29 02:18:23 1995 ylo

*/

/*
 * $Id: bufaux.h,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: bufaux.h,v $
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
 * Revision 1.3  2002/08/04 11:39:40  kcn
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
 * Revision 1.1.1.1  1996/02/18 21:38:11  ylo
 * 	Imported ssh-1.2.13.
 *
 * Revision 1.2  1995/07/13  01:18:07  ylo
 * 	Removed "Last modified" header.
 * 	Added cvs log.
 *
 * $Endlog$
 */

#ifndef BUFAUX_H
#define BUFAUX_H

#include "buffer.h"

/* Stores an MP_INT in the buffer with a 2-byte msb first bit count, followed
   by (bits+7)/8 bytes of binary data, msb first. */
void buffer_put_mp_int(Buffer * buffer, MP_INT * value);

/* Retrieves an MP_INT from the buffer. */
void buffer_get_mp_int(Buffer * buffer, MP_INT * value);

/* Returns an integer from the buffer (4 bytes, msb first). */
unsigned int buffer_get_int(Buffer * buffer);

/* Stores an integer in the buffer in 4 bytes, msb first. */
void buffer_put_int(Buffer * buffer, unsigned int value);

/* Returns a character from the buffer (0 - 255). */
int buffer_get_char(Buffer * buffer);

/* Stores a character in the buffer. */
void buffer_put_char(Buffer * buffer, int value);

/* Returns an arbitrary binary string from the buffer.  The string cannot
   be longer than 256k.  The returned value points to memory allocated
   with xmalloc; it is the responsibility of the calling function to free
   the data.  If length_ptr is non-NULL, the length of the returned data
   will be stored there.  A null character will be automatically appended
   to the returned string, and is not counted in length. */
char *buffer_get_string(Buffer * buffer, unsigned int *length_ptr);

/* Stores and arbitrary binary string in the buffer. */
void buffer_put_string(Buffer * buffer, const void *buf, unsigned int len);

#endif                          /* BUFAUX_H */
