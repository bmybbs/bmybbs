/*

buffer.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Sat Mar 18 04:12:25 1995 ylo

Code for manipulating FIFO buffers.

*/

/*
 * $Id: buffer.h,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: buffer.h,v $
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
 * Revision 1.2  1995/07/13  01:18:55  ylo
 * 	Removed "Last modified" header.
 * 	Added cvs log.
 *
 * $Endlog$
 */

#ifndef BUFFER_H
#define BUFFER_H

typedef struct {
    char *buf;                  /* Buffer for data. */
    unsigned int alloc;         /* Number of bytes allocated for data. */
    unsigned int offset;        /* Offset of first byte containing data. */
    unsigned int end;           /* Offset of last byte containing data. */
} Buffer;

/* Initializes the buffer structure. */
void buffer_init(Buffer * buffer);

/* Frees any memory used for the buffer. */
void buffer_free(Buffer * buffer);

/* Clears any data from the buffer, making it empty.  This does not actually
   zero the memory. */
void buffer_clear(Buffer * buffer);

/* Appends data to the buffer, expanding it if necessary. */
void buffer_append(Buffer * buffer, const char *data, unsigned int len);

/* Appends space to the buffer, expanding the buffer if necessary.
   This does not actually copy the data into the buffer, but instead
   returns a pointer to the allocated region. */
void buffer_append_space(Buffer * buffer, char **datap, unsigned int len);

/* Returns the number of bytes of data in the buffer. */
unsigned int buffer_len(Buffer * buffer);

/* Gets data from the beginning of the buffer. */
void buffer_get(Buffer * buffer, char *buf, unsigned int len);

/* Consumes the given number of bytes from the beginning of the buffer. */
void buffer_consume(Buffer * buffer, unsigned int bytes);

/* Consumes the given number of bytes from the end of the buffer. */
void buffer_consume_end(Buffer * buffer, unsigned int bytes);

/* Returns a pointer to the first used byte in the buffer. */
char *buffer_ptr(Buffer * buffer);

/* Dumps the contents of the buffer to stderr in hex.  This intended for
   debugging purposes only. */
void buffer_dump(Buffer * buffer);

#endif                          /* BUFFER_H */
