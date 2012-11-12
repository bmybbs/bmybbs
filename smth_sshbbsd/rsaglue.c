/*

rsaglue.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Tue Apr 11 15:17:50 1995 ylo

Glue code to be able to do RSA encryption/decryption with RSAREF.  The purpose
of this file is to permit compiling this software with RSAREF.  Using RSAREF
may make this software legal for noncommercial use without licencing the
RSA patents.  RSAREF code is only used if the system is configured
using the --with-rsaref configure option.

*/

/*
 * $Id: rsaglue.c,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: rsaglue.c,v $
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
 * Revision 1.1  2001/07/04 06:07:11  bbsdev
 * bbs sshd
 *
 * Revision 1.5  1997/03/19 21:29:59  kivinen
 * 	Added missing &.
 *
 * Revision 1.4  1997/03/19 21:14:08  kivinen
 * 	Added checks that public key exponent cannot be less than 3.
 *
 * Revision 1.3  1996/07/31 07:02:32  huima
 * *** empty log message ***
 *
 * Revision 1.2  1996/07/07 12:48:14  ylo
 * 	A fixed size buffer was used to store decrypted value without
 * 	checking bounds, which could cause stack to be overwritten
 * 	with very large keys.  Changed to use xmallocated buffer.
 *
 * Revision 1.1.1.1  1996/02/18 21:38:12  ylo
 * 	Imported ssh-1.2.13.
 *
 * Revision 1.4  1995/07/26  23:29:34  ylo
 * 	Display a fatal error if key size > 1024 with RSAREF.
 *
 * Revision 1.3  1995/07/26  17:08:59  ylo
 * 	Changed to use new functions in mpaux.c for
 * 	linearizing/unlinearizing mp-ints.
 *
 * Revision 1.2  1995/07/13  01:33:27  ylo
 * 	Removed "Last modified" header.
 * 	Added cvs log.
 *
 * $Endlog$
 */

#include "includes.h"
#include <gmp.h>
#include "ssh.h"
#include "rsa.h"
#include "getput.h"
#include "mpaux.h"
#include "xmalloc.h"

#ifdef RSAREF

/* This code uses the following RSAREF functions:
     RSAPublicEncrypt
     RSAPrivateDecrypt
     R_RandomInit
     R_RandomUpdate
     R_RandomFinal
   These functions are exported by RSAREF and are thus part of the available
   interface without modifying RSAREF. */

#define _MD5_H_                 /* Kludge to prevent inclusion of rsaref md5.h. */
#include "rsaref2/source/global.h"
#include "rsaref2/source/rsaref.h"

/* Convert an integer from gmp to rsaref representation. */

void gmp_to_rsaref(unsigned char *buf, unsigned int len, MP_INT * value)
{
    mp_linearize_msb_first(buf, len, value);
}

/* Convert an integer from rsaref to gmp representation. */

void rsaref_to_gmp(MP_INT * value, const unsigned char *buf, unsigned int len)
{
    mp_unlinearize_msb_first(value, buf, len);
}

/* Convert a public key from our representation to rsaref representation. */

void rsaref_public_key(R_RSA_PUBLIC_KEY * rsa, RSAPublicKey * key)
{
    rsa->bits = key->bits;
    gmp_to_rsaref(rsa->modulus, MAX_RSA_MODULUS_LEN, &key->n);
    gmp_to_rsaref(rsa->exponent, MAX_RSA_MODULUS_LEN, &key->e);
}

/* Convert a private key from our representation to rsaref representation. 
   Note that some fo the constants are computed a little dfifferently
   in rsaref. */

void rsaref_private_key(R_RSA_PRIVATE_KEY * rsa, RSAPrivateKey * key)
{
    MP_INT aux;

    rsa->bits = key->bits;
    gmp_to_rsaref(rsa->modulus, MAX_RSA_MODULUS_LEN, &key->n);
    gmp_to_rsaref(rsa->publicExponent, MAX_RSA_MODULUS_LEN, &key->e);
    gmp_to_rsaref(rsa->exponent, MAX_RSA_MODULUS_LEN, &key->d);
    gmp_to_rsaref(rsa->prime[0], MAX_RSA_PRIME_LEN, &key->q);
    gmp_to_rsaref(rsa->prime[1], MAX_RSA_PRIME_LEN, &key->p);
    mpz_init_set(&aux, &key->q);
    mpz_sub_ui(&aux, &aux, 1);
    mpz_mod(&aux, &key->d, &aux);
    gmp_to_rsaref(rsa->primeExponent[0], MAX_RSA_PRIME_LEN, &aux);
    mpz_set(&aux, &key->p);
    mpz_sub_ui(&aux, &aux, 1);
    mpz_mod(&aux, &key->d, &aux);
    gmp_to_rsaref(rsa->primeExponent[1], MAX_RSA_PRIME_LEN, &aux);
    gmp_to_rsaref(rsa->coefficient, MAX_RSA_PRIME_LEN, &key->u);
}

/* Performs a public key encrypt operation. */

void rsa_public_encrypt(MP_INT * output, MP_INT * input, RSAPublicKey * key, RandomState * random_state)
{
    unsigned char input_data[MAX_RSA_MODULUS_LEN];
    unsigned char output_data[MAX_RSA_MODULUS_LEN];
    unsigned char buf[256];
    unsigned int input_len, output_len, i, input_bits;
    R_RSA_PUBLIC_KEY public_key;
    R_RANDOM_STRUCT rands;

    if (key->bits > MAX_RSA_MODULUS_BITS)
        fatal("RSA key has too many bits for RSAREF to handle (max %d).", MAX_RSA_MODULUS_BITS);

    input_bits = mpz_sizeinbase(input, 2);
    input_len = (input_bits + 7) / 8;
    gmp_to_rsaref(input_data, input_len, input);

    rsaref_public_key(&public_key, key);

    R_RandomInit(&rands);
    for (i = 0; i < 256; i++)
        buf[i] = random_get_byte(random_state);
    R_RandomUpdate(&rands, buf, 256);

    if (RSAPublicEncrypt(output_data, &output_len, input_data, input_len, &public_key, &rands) != 0)
        fatal("RSAPublicEncrypt failed");

    R_RandomFinal(&rands);

    rsaref_to_gmp(output, output_data, output_len);
}

/* Performs a private key decrypt operation. */

void rsa_private_decrypt(MP_INT * output, MP_INT * input, RSAPrivateKey * key)
{
    unsigned char input_data[MAX_RSA_MODULUS_LEN];
    unsigned char output_data[MAX_RSA_MODULUS_LEN];
    unsigned int input_len, output_len, input_bits;
    R_RSA_PRIVATE_KEY private_key;

    if (key->bits > MAX_RSA_MODULUS_BITS)
        fatal("RSA key has too many bits for RSAREF to handle (max %d).", MAX_RSA_MODULUS_BITS);

    input_bits = mpz_sizeinbase(input, 2);
    input_len = (input_bits + 7) / 8;
    gmp_to_rsaref(input_data, input_len, input);

    rsaref_private_key(&private_key, key);

    if (RSAPrivateDecrypt(output_data, &output_len, input_data, input_len, &private_key) != 0)
        fatal("RSAPrivateDecrypt failed");

    rsaref_to_gmp(output, output_data, output_len);
}

#else                           /* RSAREF */

/* Encrypt input using the public key.  Input should be a 256 bit value. */

void rsa_public_encrypt(MP_INT * output, MP_INT * input, RSAPublicKey * key, RandomState * state)
{
    MP_INT aux;
    unsigned int i, input_bits, input_len, len;

    if (mpz_cmp_ui(&(key->e), 3) < 0)
        fatal("Bad public key, POTENTIAL BREAK-IN ATTEMPT!");
    input_bits = mpz_sizeinbase(input, 2);
    input_len = (input_bits + 7) / 8;
    len = (key->bits + 7) / 8;

    assert(len >= input_len + 3);

    mpz_init_set_ui(&aux, 2);
    for (i = 2; i < len - input_len - 1; i++) {
        unsigned int byte;

        do
            byte = random_get_byte(state);
        while (byte == 0);
        mpz_mul_2exp(&aux, &aux, 8);
        mpz_add_ui(&aux, &aux, byte);
    }
    mpz_mul_2exp(&aux, &aux, 8 * (input_len + 1));
    mpz_add(&aux, &aux, input);

    rsa_public(output, &aux, key);
    mpz_clear(&aux);
}

/* Decrypt input using the private key.  Output will become a 256 bit value. */

void rsa_private_decrypt(MP_INT * output, MP_INT * input, RSAPrivateKey * key)
{
    MP_INT aux;
    unsigned int len, i;
    unsigned char *value;

    rsa_private(output, input, key);

    len = (key->bits + 7) / 8;
    value = xmalloc(len);

    mpz_init_set(&aux, output);
    for (i = len; i >= 4; i -= 4) {
        unsigned int limb = mpz_get_ui(&aux);

        PUT_32BIT(value + i - 4, limb);
        mpz_div_2exp(&aux, &aux, 32);
    }
    for (; i > 0; i--) {
        value[i - 1] = mpz_get_ui(&aux);
        mpz_div_2exp(&aux, &aux, 8);
    }
    mpz_clear(&aux);

    if (value[0] != 0 || value[1] != 2)
        fatal("Bad result from rsa_private_decrypt");

    for (i = 2; i < len && value[i]; i++);

    xfree(value);

    mpz_mod_2exp(output, output, 8 * (len - i - 1));
}

#endif                          /* RSAREF */
