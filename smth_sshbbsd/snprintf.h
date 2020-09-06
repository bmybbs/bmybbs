/*

  Author: Tomi Salo <ttsalo@ssh.fi>

  Copyright (C) 1996 SSH Communications Security Oy, Espoo, Finland
  See COPYING for distribution conditions.

  Header file for snprintf.c

  */

/*
 * $Id:
 * $Log: snprintf.h,v $
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
 * Revision 1.2  2002/08/04 11:08:49  kcn
 * format C
 *
 * Revision 1.1.1.1  2002/04/27 05:47:26  kxn
 * no message
 *
 * Revision 1.1  2001/07/04 06:07:12  bbsdev
 * bbs sshd
 *
 * Revision 1.1  1999/02/21 19:52:38  ylo
 * 	Intermediate commit of ssh1.2.27 stuff.
 * 	Main change is sprintf -> snprintf; however, there are also
 * 	many other changes.
 *
 * Revision 1.7  1998/10/04 02:54:00  ylo
 *      Removed #include "sshincludes.h"
 *
 * Revision 1.6  1998/06/24 13:46:46  kivinen
 *      Fixed Log entry.
 *
 * $EndLog$
 */

#ifndef SNPRINTF_H
#define SNPRINTF_H

/* Write formatted text to buffer 'str', using format string 'format'.
   Returns number of characters written, or negative if error
   occurred. SshBuffer's size is given in 'size'. Format string is
   understood as defined in ANSI C.

   NOTE: This does NOT work identically with BDS's snprintf.

   Integers: Ansi C says that precision specifies the minimun
   number of digits to print. BSD's version however counts the
   prefixes (+, -, ' ', '0x', '0X', octal prefix '0'...) as
   'digits'.

   Also, BSD implementation does not permit padding integers
   to specified width with zeros on left (in front of the prefixes),
   it uses spaces instead, even when Ansi C only forbids padding
   with zeros on the right side of numbers.
   
   */

int snprintf(char *str, size_t size, const char *format, ...);

int vsnprintf(char *str, size_t size, const char *format, va_list ap);

#endif                          /* SNPRINTF_H */
