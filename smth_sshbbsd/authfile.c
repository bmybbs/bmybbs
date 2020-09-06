/*

authfile.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Mon Mar 27 03:52:05 1995 ylo

This file contains functions for reading and writing identity files, and
for reading the passphrase from the user.

*/

/*
 * $Id: authfile.c,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: authfile.c,v $
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
 * Revision 1.2  2002/08/04 11:08:44  kcn
 * format C
 *
 * Revision 1.1.1.1  2002/04/27 05:47:25  kxn
 * no message
 *
 * Revision 1.1  2001/07/04 06:07:07  bbsdev
 * bbs sshd
 *
 * Revision 1.2  1997/03/19 22:18:27  kivinen
 * 	Removed check that SSH_CIPHER_NONE is in cipher_mask because
 * 	it is always internally supported, even if not in cipher_mask.
 *
 * Revision 1.1.1.1  1996/02/18 21:38:11  ylo
 * 	Imported ssh-1.2.13.
 *
 * Revision 1.6  1995/10/02  01:19:48  ylo
 * 	Added some casts to avoid compiler warnings.
 *
 * Revision 1.5  1995/09/09  21:26:39  ylo
 * /m/shadows/u2/users/ylo/ssh/README
 *
 * Revision 1.4  1995/08/21  23:21:56  ylo
 * 	Don't complain about bad passphrase if passphrase was empty.
 *
 * Revision 1.3  1995/07/13  01:16:38  ylo
 * 	Removed "Last modified" header.
 *
 * Revision 1.2  1995/07/13  01:11:52  ylo
 * 	Added cvs log.
 *
 * $Endlog$
 */

#include "includes.h"
#include <gmp.h>
#include "xmalloc.h"
#include "idea.h"
#include "buffer.h"
#include "bufaux.h"
#include "cipher.h"
#include "ssh.h"
#include "userfile.h"

/* Version identification string for identity files. */
#define AUTHFILE_ID_STRING "SSH PRIVATE KEY FILE FORMAT 1.1\n"

/* Loads the private key from the file.  Returns 0 if an error is encountered
   (file does not exist or is not readable, or passphrase is bad).
   This initializes the private key.  The I/O will be done using the given
   uid with userfile. */

int load_private_key(uid_t uid, const char *filename, const char *passphrase, RSAPrivateKey * prv, char **comment_return)
{
    int i, check1, check2, cipher_type;
    int uf;
    unsigned long len;
    Buffer buffer, decrypted;
    char *cp;
    CipherContext cipher;

    /* Read the file into the buffer. */
    uf = open(filename, O_RDONLY, 0);
    if (uf < 0)
        return 0;

    len = lseek(uf, (off_t) 0L, 2);
    lseek(uf, (off_t) 0L, 0);

    if (len > 32000) {
        close(uf);
        debug("Authentication file too big: %.200s", filename);
        return 0;
    }

    buffer_init(&buffer);
    buffer_append_space(&buffer, &cp, len);

    if (read(uf, cp, len) != len) {
        debug("Read from key file %.200s failed: %.100s", filename, strerror(errno));
        buffer_free(&buffer);
        close(uf);
        return 0;
    }
    close(uf);

    /* Check that it is at least big enought to contain the ID string. */
    if (len < strlen(AUTHFILE_ID_STRING) + 1) {
        debug("Bad key file %.200s.", filename);
        buffer_free(&buffer);
        return 0;
    }

    /* Make sure it begins with the id string.  Consume the id string from
       the buffer. */
    for (i = 0; i < (unsigned int) strlen(AUTHFILE_ID_STRING) + 1; i++)
        if (buffer_get_char(&buffer) != (unsigned char) AUTHFILE_ID_STRING[i]) {
            debug("Bad key file %.200s.", filename);
            buffer_free(&buffer);
            return 0;
        }

    /* Read cipher type. */
    cipher_type = buffer_get_char(&buffer);
    (void) buffer_get_int(&buffer);     /* Reserved data. */

    /* Read the public key from the buffer. */
    prv->bits = buffer_get_int(&buffer);
    mpz_init(&prv->n);
    buffer_get_mp_int(&buffer, &prv->n);
    mpz_init(&prv->e);
    buffer_get_mp_int(&buffer, &prv->e);
    if (comment_return)
        *comment_return = buffer_get_string(&buffer, NULL);
    else
        xfree(buffer_get_string(&buffer, NULL));

    /* Check that it is a supported cipher. */
    if (cipher_type != SSH_CIPHER_NONE && (cipher_mask() & (1 << cipher_type)) == 0) {
        debug("Unsupported cipher %.100s used in key file %.200s.", cipher_name(cipher_type), filename);
        buffer_free(&buffer);
        goto fail;
    }

    /* Initialize space for decrypted data. */
    buffer_init(&decrypted);
    buffer_append_space(&decrypted, &cp, buffer_len(&buffer));

    /* Rest of the buffer is encrypted.  Decrypt it using the passphrase. */
    cipher_set_key_string(&cipher, cipher_type, passphrase, 0);
    cipher_decrypt(&cipher, (unsigned char *) cp, (unsigned char *) buffer_ptr(&buffer), buffer_len(&buffer));

    buffer_free(&buffer);

    check1 = buffer_get_char(&decrypted);
    check2 = buffer_get_char(&decrypted);
    if (check1 != buffer_get_char(&decrypted) || check2 != buffer_get_char(&decrypted)) {
        if (strcmp(passphrase, "") != 0)
            debug("Bad passphrase supplied for key file %.200s.", filename);
        /* Bad passphrase. */
        buffer_free(&decrypted);
      fail:
        mpz_clear(&prv->n);
        mpz_clear(&prv->e);
        if (comment_return)
            xfree(*comment_return);
        return 0;
    }

    /* Read the rest of the private key. */
    mpz_init(&prv->d);
    buffer_get_mp_int(&decrypted, &prv->d);
    mpz_init(&prv->u);
    buffer_get_mp_int(&decrypted, &prv->u);
    mpz_init(&prv->p);
    buffer_get_mp_int(&decrypted, &prv->p);
    mpz_init(&prv->q);
    buffer_get_mp_int(&decrypted, &prv->q);

    buffer_free(&decrypted);

    return 1;
}
