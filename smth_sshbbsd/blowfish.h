/*

blowfish.h

Author: Mika Kojo

Copyright (c) 1996 SSH Communications Security Oy

Created: Wed May 28 20:25 1996

The blowfish encryption algorithm, created by Bruce Schneier.

*/

#ifndef BLOWFISH_H
#define BLOWFISH_H

#include "includes.h"

#define MAX_KEY_BYTES 56
#define KEYBYTES      8

typedef struct {
    word32 S[4 * 256], P[16 + 2];
    unsigned char iv[8];
} BlowfishContext;

/* Prototypes */

#if 0
void blowfish_encrypt(BlowfishContext * context, word32 xl, word32 xr, word32 * output);

void blowfish_decrypt(BlowfishContext * context, word32 xl, word32 xr, word32 * output);
#endif

void blowfish_set_key(BlowfishContext *, const unsigned char *, short, int);

void blowfish_transform(word32, word32, word32 *, int, void *);

void blowfish_cbc_encrypt(BlowfishContext *, unsigned char *, const unsigned char *, unsigned int);

void blowfish_cbc_decrypt(BlowfishContext *, unsigned char *, const unsigned char *, unsigned int);

#endif
