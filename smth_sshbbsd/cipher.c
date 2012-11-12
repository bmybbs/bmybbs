/*

cipher.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Wed Apr 19 17:41:39 1995 ylo

*/

/*
 * $Id: cipher.c,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: cipher.c,v $
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
 * Revision 1.12  1998/05/23 20:21:09  kivinen
 * 	Changed () -> (void).
 *
 * Revision 1.11  1998/04/30  01:51:32  kivinen
 * 	Reserved cipher number 7 to Bernard Perrot
 * 	<perrot@lal.in2p3.fr> for some weak 40 bit encryption method.
 *
 * Revision 1.10  1998/03/27 17:23:43  kivinen
 * 	Removed TSS.
 *
 * Revision 1.9  1997/03/26 07:03:04  kivinen
 * 	Added check that warning about arcfour is given only once.
 *
 * Revision 1.8  1997/03/25 05:38:50  kivinen
 * 	#ifndef WITH_IDEA -> #ifdef WITH_IDEA.
 *
 * Revision 1.7  1997/03/19 22:26:38  kivinen
 * 	Removed WITH_3DES ifdefs, as it is mandatory.
 *
 * Revision 1.6  1997/03/19 22:17:11  kivinen
 * 	Enabled none encryption internally anyway, because it is
 * 	required to read host keys.
 *
 * Revision 1.5  1997/03/19 21:29:45  kivinen
 * 	Added missing }.
 *
 * Revision 1.4  1997/03/19 17:34:56  kivinen
 * 	Made all ciphers optional.
 *
 * Revision 1.3  1996/09/28 12:01:04  ylo
 * 	Removed TSS (put inside #ifdef WITH_TSS).
 *
 * Revision 1.2  1996/09/27 13:55:02  ttsalo
 * 	Added blowfish
 *
 * Revision 1.1.1.1  1996/02/18 21:38:11  ylo
 * 	Imported ssh-1.2.13.
 *
 * Revision 1.3  1995/08/18  22:48:01  ylo
 * 	Added code to permit compiling without idea.
 *
 * 	With triple-des, if the key length is only 16 bytes, reuse the
 * 	beginning of the key for the third DES key.
 *
 * Revision 1.2  1995/07/13  01:19:45  ylo
 * 	Removed "Last modified" header.
 * 	Added cvs log.
 *
 * $Endlog$
 */
#include "sys/types.h"
#include "./md5.h"
#include "includes.h"
#include "ssh.h"
#include "cipher.h"

/* Names of all encryption algorithms.  These must match the numbers defined
   int cipher.h. */
static char *cipher_names[] = { "none", "idea", "des", "3des", "used to be tss", "arcfour", "blowfish",
    "reserved"
};

/* Returns a bit mask indicating which ciphers are supported by this
   implementation.  The bit mask has the corresponding bit set of each
   supported cipher. */

unsigned int cipher_mask(void)
{
    unsigned int mask = 0;

#ifdef WITH_NONE
    mask |= 1 << SSH_CIPHER_NONE;
#endif                          /* WITH_NONE */

#ifdef WITH_IDEA
    mask |= 1 << SSH_CIPHER_IDEA;
#endif                          /* WITH_IDEA */

#ifdef WITH_DES
    mask |= 1 << SSH_CIPHER_DES;
#endif                          /* WITH_DES */

    mask |= 1 << SSH_CIPHER_3DES;

#ifdef WITH_ARCFOUR
    mask |= 1 << SSH_CIPHER_ARCFOUR;
#endif                          /* WITH_ARCFOUR */

#ifdef WITH_BLOWFISH
    mask |= 1 << SSH_CIPHER_BLOWFISH;
#endif                          /* WITH_BLOWFISH */
    return mask;
}

/* Returns the name of the cipher. */

const char *cipher_name(int cipher)
{
    if (cipher < 0 || cipher >= sizeof(cipher_names) / sizeof(cipher_names[0]))
        fatal("cipher_name: bad cipher number: %d", cipher);
    return cipher_names[cipher];
}

/* Parses the name of the cipher.  Returns the number of the corresponding
   cipher, or -1 on error. */

int cipher_number(const char *name)
{
    int i;
    static int warning_given = 0;

    /* Recognize other nam for backward compatibility. */
    if (name[0] == 'r' && name[1] == 'c' && name[2] == '4' && name[3] == '\0') {
#if defined(WITH_ARCFOUR) || !defined(WITH_BLOWFISH)
        return SSH_CIPHER_ARCFOUR;
#else
        if (!warning_given)
            log_msg("Arcfour cipher is disabled in this host, using blowfish cipher instead");
        warning_given = 1;
        return SSH_CIPHER_BLOWFISH;
#endif
    }

    for (i = 0; i < sizeof(cipher_names) / sizeof(cipher_names[0]); i++)
        if (strcmp(cipher_names[i], name) == 0) {
#if !defined(WITH_ARCFOUR) && defined(WITH_BLOWFISH)
            if (i == SSH_CIPHER_ARCFOUR) {
                if (!warning_given)
                    log_msg("Arcfour cipher is disabled in this host, using blowfish cipher instead");
                warning_given = 1;
                return SSH_CIPHER_BLOWFISH;
            }
#endif
            return i;
        }
    return -1;
}

/* Selects the cipher, and keys if by computing the MD5 checksum of the
   passphrase and using the resulting 16 bytes as the key. */

void cipher_set_key_string(CipherContext * context, int cipher, const char *passphrase, int for_encryption)
{
    struct MD5Context md;
    unsigned char digest[16];

    MD5Init(&md);
    MD5Update(&md, (const unsigned char *) passphrase, strlen(passphrase));
    MD5Final(digest, &md);

    cipher_set_key(context, cipher, digest, 16, for_encryption);

    memset(digest, 0, sizeof(digest));
    memset(&md, 0, sizeof(md));
}

/* Selects the cipher to use and sets the key. */

void cipher_set_key(CipherContext * context, int cipher, const unsigned char *key, int keylen, int for_encryption)
{
    unsigned char padded[32];

    /* Clear the context to remove any traces of old keys. */
    memset(context, 0, sizeof(*context));

    /* Set cipher type. */
    context->type = cipher;

    /* Get 32 bytes of key data.  Pad if necessary.  (So that code below does
       not need to worry about key size). */
    memset(padded, 0, sizeof(padded));
    memcpy(padded, key, keylen < sizeof(padded) ? keylen : sizeof(padded));

    /* Initialize the initialization vector. */
    switch (cipher) {
    case SSH_CIPHER_NONE:
        break;

#ifdef WITH_IDEA
    case SSH_CIPHER_IDEA:
        if (keylen < 16)
            error("Key length %d is insufficient for IDEA.", keylen);
        idea_set_key(&context->u.idea.key, padded);
        memset(context->u.idea.iv, 0, sizeof(context->u.idea.iv));
        break;
#endif                          /* WITH_IDEA */

#ifdef WITH_DES
    case SSH_CIPHER_DES:
        /* Note: the least significant bit of each byte of key is parity, 
           and must be ignored by the implementation.  8 bytes of key are
           used. */
        if (keylen < 8)
            error("Key length %d is insufficient for DES.", keylen);
        des_set_key(padded, &context->u.des.key);
        memset(context->u.des.iv, 0, sizeof(context->u.des.iv));
        break;
#endif                          /* WITH_DES */

    case SSH_CIPHER_3DES:
        /* Note: the least significant bit of each byte of key is parity, 
           and must be ignored by the implementation.  16 bytes of key are
           used (first and last keys are the same). */
        if (keylen < 16)
            error("Key length %d is insufficient for 3DES.", keylen);
        des_set_key(padded, &context->u.des3.key1);
        des_set_key(padded + 8, &context->u.des3.key2);
        if (keylen <= 16)
            des_set_key(padded, &context->u.des3.key3);
        else
            des_set_key(padded + 16, &context->u.des3.key3);
        memset(context->u.des3.iv1, 0, sizeof(context->u.des3.iv1));
        memset(context->u.des3.iv2, 0, sizeof(context->u.des3.iv2));
        memset(context->u.des3.iv3, 0, sizeof(context->u.des3.iv3));
        break;

#ifdef WITH_ARCFOUR
    case SSH_CIPHER_ARCFOUR:
        arcfour_init(&context->u.arcfour, key, keylen);
        break;
#endif                          /* WITH_ARCFOUR */

#ifdef WITH_BLOWFISH
    case SSH_CIPHER_BLOWFISH:
        if (keylen < 8)
            error("Key length %d is insufficient for Blowfish", keylen);
        blowfish_set_key(&context->u.blowfish, key, keylen, for_encryption);
        break;
#endif                          /* WITH_BLOWFISH */
    default:
        fatal("cipher_set_key: unknown cipher: %d", cipher);
    }
    memset(padded, 0, sizeof(padded));
}

/* Encrypts data using the cipher. */

void cipher_encrypt(CipherContext * context, unsigned char *dest, const unsigned char *src, unsigned int len)
{
    switch (context->type) {
    case SSH_CIPHER_NONE:
        memcpy(dest, src, len);
        break;

#ifdef WITH_IDEA
    case SSH_CIPHER_IDEA:
        idea_cfb_encrypt(&context->u.idea.key, context->u.idea.iv, dest, src, len);
        break;
#endif                          /* WITH_IDEA */

#ifdef WITH_DES
    case SSH_CIPHER_DES:
        des_cbc_encrypt(&context->u.des.key, context->u.des.iv, dest, src, len);
        break;
#endif                          /* WITH_DES */

    case SSH_CIPHER_3DES:
        des_3cbc_encrypt(&context->u.des3.key1, context->u.des3.iv1, &context->u.des3.key2, context->u.des3.iv2, &context->u.des3.key3, context->u.des3.iv3, dest, src, len);
        break;

#ifdef WITH_ARCFOUR
    case SSH_CIPHER_ARCFOUR:
        arcfour_encrypt(&context->u.arcfour, dest, src, len);
        break;
#endif                          /* WITH_ARCFOUR */

#ifdef WITH_BLOWFISH
    case SSH_CIPHER_BLOWFISH:
        blowfish_cbc_encrypt(&context->u.blowfish, dest, src, len);
        break;
#endif                          /* WITH_BLOWFISH */

    default:
        fatal("cipher_encrypt: unknown cipher: %d", context->type);
    }
}

/* Decrypts data using the cipher. */

void cipher_decrypt(CipherContext * context, unsigned char *dest, const unsigned char *src, unsigned int len)
{
    switch (context->type) {
    case SSH_CIPHER_NONE:
        memcpy(dest, src, len);
        break;

#ifdef WITH_IDEA
    case SSH_CIPHER_IDEA:
        idea_cfb_decrypt(&context->u.idea.key, context->u.idea.iv, dest, src, len);
        break;
#endif                          /* WITH_IDEA */

#ifdef WITH_DES
    case SSH_CIPHER_DES:
        des_cbc_decrypt(&context->u.des.key, context->u.des.iv, dest, src, len);
        break;
#endif                          /* WITH_DES */

    case SSH_CIPHER_3DES:
        des_3cbc_decrypt(&context->u.des3.key1, context->u.des3.iv1, &context->u.des3.key2, context->u.des3.iv2, &context->u.des3.key3, context->u.des3.iv3, dest, src, len);
        break;

#ifdef WITH_ARCFOUR
    case SSH_CIPHER_ARCFOUR:
        arcfour_decrypt(&context->u.arcfour, dest, src, len);
        break;
#endif                          /* WITH_ARCFOUR */

#ifdef WITH_BLOWFISH
    case SSH_CIPHER_BLOWFISH:
        blowfish_cbc_decrypt(&context->u.blowfish, dest, src, len);
        break;
#endif                          /* WITH_BLOWFISH */

    default:
        fatal("cipher_decrypt: unknown cipher: %d", context->type);
    }
}
