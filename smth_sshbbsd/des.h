/*

DES implementation; 1995 Tatu Ylonen <ylo@cs.hut.fi>

This implementation is derived from libdes-3.06, which is copyright
(c) 1993 Eric Young, and distributed under the GNU GPL or the ARTISTIC licence
(at the user's option).  The original distribution can be found e.g. from
ftp://ftp.dsi.unimi.it/pub/security/crypt/libdes/libdes-3.06.tar.gz.

This implementation is distributed under the same terms.  See
libdes-README, libdes-ARTISTIC, and libdes-COPYING for more
information.

*/

/*
 * $Id: des.h,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: des.h,v $
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
 * Revision 1.3  2002/08/04 11:39:41  kcn
 * format c
 *
 * Revision 1.2  2002/08/04 11:08:46  kcn
 * format C
 *
 * Revision 1.1.1.1  2002/04/27 05:47:25  kxn
 * no message
 *
 * Revision 1.1  2001/07/04 06:07:09  bbsdev
 * bbs sshd
 *
 * Revision 1.1.1.1  1996/02/18 21:38:11  ylo
 * 	Imported ssh-1.2.13.
 *
 * Revision 1.2  1995/07/13  01:22:57  ylo
 * 	Added cvs log.
 *
 * $Endlog$
 */

#ifndef DES_H
#define DES_H

typedef struct {
    word32 key_schedule[32];
} DESContext;

/* Sets the des key for the context.  Initializes the context.  The least
   significant bit of each byte of the key is ignored as parity. */
void des_set_key(unsigned char *key, DESContext * ks);

/* Encrypts 32 bits in l,r, and stores the result in output[0] and output[1].
   Performs encryption if encrypt is non-zero, and decryption if it is zero.
   The key context must have been initialized previously with des_set_key. */
void des_encrypt(word32 l, word32 r, word32 * output, DESContext * ks, int encrypt);

/* Encrypts len bytes from src to dest in CBC modes.  Len must be a multiple
   of 8.  iv will be modified at end to a value suitable for continuing
   encryption. */
void des_cbc_encrypt(DESContext * ks, unsigned char *iv, unsigned char *dest, const unsigned char *src, unsigned int len);

/* Decrypts len bytes from src to dest in CBC modes.  Len must be a multiple
   of 8.  iv will be modified at end to a value suitable for continuing
   decryption. */
void des_cbc_decrypt(DESContext * ks, unsigned char *iv, unsigned char *dest, const unsigned char *src, unsigned int len);

/* Encrypts in CBC mode using triple-DES. */
void des_3cbc_encrypt(DESContext * ks1, unsigned char *iv1,
                      DESContext * ks2, unsigned char *iv2, DESContext * ks3, unsigned char *iv3, unsigned char *dest, const unsigned char *src, unsigned int len);

/* Decrypts in CBC mode using triple-DES. */
void des_3cbc_decrypt(DESContext * ks1, unsigned char *iv1,
                      DESContext * ks2, unsigned char *iv2, DESContext * ks3, unsigned char *iv3, unsigned char *dest, const unsigned char *src, unsigned int len);

#endif                          /* DES_H */
