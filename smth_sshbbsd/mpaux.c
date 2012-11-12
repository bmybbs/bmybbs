/*

mpaux.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Sun Jul 16 04:29:30 1995 ylo

This file contains various auxiliary functions related to multiple
precision integers.

*/

/*
 * $Id: mpaux.c,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: mpaux.c,v $
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
 * Revision 1.3  2002/08/04 11:39:42  kcn
 * format c
 *
 * Revision 1.2  2002/08/04 11:08:47  kcn
 * format C
 *
 * Revision 1.1.1.1  2002/04/27 05:47:26  kxn
 * no message
 *
 * Revision 1.1  2001/07/04 06:07:10  bbsdev
 * bbs sshd
 *
 * Revision 1.2  1996/04/26 00:26:48  ylo
 * 	Fixed problems with 16-bit Windows.
 *
 * Revision 1.1.1.1  1996/02/18 21:38:11  ylo
 * 	Imported ssh-1.2.13.
 *
 * Revision 1.1  1995/07/27  03:28:15  ylo
 * 	Auxiliary functions for manipulating mp-ins.
 *
 * $Endlog$
 */

#include "includes.h"
#include <gmp.h>
#include "getput.h"
#include "xmalloc.h"
#include "md5.h"

/* Converts a multiple-precision integer into bytes to be stored in the buffer.
   The buffer will contain the value of the integer, msb first. */

void mp_linearize_msb_first(unsigned char *buf, unsigned int len, MP_INT * value)
{
    unsigned int i;
    MP_INT aux;

    mpz_init_set(&aux, value);
    for (i = len; i >= 4; i -= 4) {
        unsigned long limb = mpz_get_ui(&aux);

        PUT_32BIT(buf + i - 4, limb);
        mpz_div_2exp(&aux, &aux, 32);
    }
    for (; i > 0; i--) {
        buf[i - 1] = mpz_get_ui(&aux);
        mpz_div_2exp(&aux, &aux, 8);
    }
    mpz_clear(&aux);
}

/* Extract a multiple-precision integer from buffer.  The value is stored
   in the buffer msb first. */

void mp_unlinearize_msb_first(MP_INT * value, const unsigned char *buf, unsigned int len)
{
    unsigned int i;

    mpz_set_ui(value, 0);
    for (i = 0; i + 4 <= len; i += 4) {
        unsigned long limb = GET_32BIT(buf + i);

        mpz_mul_2exp(value, value, 32);
        mpz_add_ui(value, value, limb);
    }
    for (; i < len; i++) {
        mpz_mul_2exp(value, value, 8);
        mpz_add_ui(value, value, buf[i]);
    }
}

/* Computes a 16-byte session id in the global variable session_id.
   The session id is computed by concatenating the linearized, msb
   first representations of host_key_n, session_key_n, and the cookie. */

void compute_session_id(unsigned char session_id[16], unsigned char cookie[8], unsigned int host_key_bits, MP_INT * host_key_n, unsigned int session_key_bits, MP_INT * session_key_n)
{
    unsigned int bytes = (host_key_bits + 7) / 8 + (session_key_bits + 7) / 8 + 8;
    unsigned char *buf = xmalloc(bytes);
    struct MD5Context md;

    mp_linearize_msb_first(buf, (host_key_bits + 7) / 8, host_key_n);
    mp_linearize_msb_first(buf + (host_key_bits + 7) / 8, (session_key_bits + 7) / 8, session_key_n);
    memcpy(buf + (host_key_bits + 7) / 8 + (session_key_bits + 7) / 8, cookie, 8);
    MD5Init(&md);
    MD5Update(&md, buf, bytes);
    MD5Final(session_id, &md);
    xfree(buf);
}
