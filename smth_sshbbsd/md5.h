#ifndef MD5_FAINT_H
#define MD5_FAINT_H
#include <sys/types.h>
typedef unsigned int md5_uint32;


#define MD5_DIGEST_LENGTH 16
    struct MD5Context {
    md5_uint32 buf[4];
    md5_uint32 bits[2];
    unsigned char in[64];
};
void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context *context, unsigned char const *buf, unsigned len);
void MD5Final(unsigned char digest[16], struct MD5Context *context);
void MD5Transform(md5_uint32 buf[4], const unsigned char in[64]);


/*
 * This is needed to make RSAREF happy on some MS-DOS compilers.
 */ 
typedef struct MD5Context MD5_CTX;


#endif  /* !MD5_H */
