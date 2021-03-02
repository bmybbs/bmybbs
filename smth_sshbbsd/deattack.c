/*
 * Id: deattack.c,v 1.9 1998/06/27 00:02:22 futo Exp  Cryptographic attack
 * detector for ssh - source code (C)1998 CORE-SDI, Buenos Aires Argentina
 * Ariel Futoransky(futo@core-sdi.com) <http://www.core-sdi.com>
 */

#include "includes.h"
#include "deattack.h"
#include "ssh.h"
#include "getput.h"
#include "xmalloc.h"
#include "crc32.h"

/* SSH Constants */
#define SSH_MAXBLOCKS (32 * 1024)
#define SSH_BLOCKSIZE (8)

/* Hashing constants */
#define HASH_MINSIZE (8 * 1024)
#define HASH_ENTRYSIZE (2)
#define HASH_FACTOR(x) ((x)*3/2)
#define HASH_UNUSED (0xffff)
#define HASH_IV     (0xfffe)

#define HASH_MINBLOCKS  (7*SSH_BLOCKSIZE)


/* Hash function (Input keys are cipher results) */
#define HASH(x) GET_32BIT(x)

#define CMP(a,b) (memcmp(a, b, SSH_BLOCKSIZE))


void crc_update(word32 * a, word32 b)
{
    b ^= *a;
    *a = ssh_crc32((unsigned char *) &b, sizeof(b));
}

/*
   check_crc
   detects if a block is used in a particular pattern
 */

int check_crc(unsigned char *S, unsigned char *buf, word32 len, unsigned char *IV)
{
    word32 crc;
    unsigned char *c;

    crc = 0;
    if (IV && !CMP(S, IV)) {
        crc_update(&crc, 1);
        crc_update(&crc, 0);
    }
    for (c = buf; c < buf + len; c += SSH_BLOCKSIZE) {
        if (!CMP(S, c)) {
            crc_update(&crc, 1);
            crc_update(&crc, 0);
        } else {
            crc_update(&crc, 0);
            crc_update(&crc, 0);
        }
    }

    return (crc == 0);
}


/*
   detect_attack
   Detects a crc32 compensation attack on a packet
 */
int detect_attack(unsigned char *buf, word32 len, unsigned char *IV)
{
    static word16 *h = (word16 *) NULL;
    static word32 n = HASH_MINSIZE / HASH_ENTRYSIZE;
    register word32 i, j;
    word32 l;
    register unsigned char *c;
    unsigned char *d;


    assert(len <= (SSH_MAXBLOCKS * SSH_BLOCKSIZE));
    assert(len % SSH_BLOCKSIZE == 0);

    for (l = n; l < HASH_FACTOR(len / SSH_BLOCKSIZE); l = l << 2);

    if (h == NULL) {
        debug("Installing crc compensation attack detector.");
        n = l;
        h = (word16 *) xmalloc(n * sizeof(word16));
    } else {
        if (l > n) {
            n = l;
            h = (word16 *) xrealloc(h, n * sizeof(word16));
        }
    }


    if (len <= HASH_MINBLOCKS) {
        for (c = buf; c < buf + len; c += SSH_BLOCKSIZE) {
            if (IV && (!CMP(c, IV))) {
                if ((check_crc(c, buf, len, IV)))
                    return (DEATTACK_DETECTED);
                else
                    break;
            }
            for (d = buf; d < c; d += SSH_BLOCKSIZE) {
                if (!CMP(c, d)) {
                    if ((check_crc(c, buf, len, IV)))
                        return (DEATTACK_DETECTED);
                    else
                        break;
                }
            }
        }
        return (DEATTACK_OK);
    }

    for (i = 0; i < n; i++)
        h[i] = HASH_UNUSED;

    if (IV)
        h[HASH(IV) & (n - 1)] = HASH_IV;


    for (c = buf, j = 0; c < (buf + len); c += SSH_BLOCKSIZE, j++) {
        for (i = HASH(c) & (n - 1); h[i] != HASH_UNUSED; i = (i + 1) & (n - 1)) {
            if (h[i] == HASH_IV) {
                if (!CMP(c, IV)) {
                    if (check_crc(c, buf, len, IV))
                        return (DEATTACK_DETECTED);
                    else
                        break;
                }
            } else if (!CMP(c, buf + h[i] * SSH_BLOCKSIZE)) {
                if (check_crc(c, buf, len, IV))
                    return (DEATTACK_DETECTED);
                else
                    break;
            }
        }
        h[i] = j;
    }

    return (DEATTACK_OK);
}
