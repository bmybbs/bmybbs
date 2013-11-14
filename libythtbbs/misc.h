/* misc.c */
#ifndef __MISC_H
#define __MISC_H
extern int pu;
void getrandomint(unsigned int *s);
void getrandomstr(unsigned char *s);
/**
 * getrandomstr 方法的变种
 * @param s 字符串
 * @param len 长度
 */
void getrandomstr_r(unsigned char *s, size_t len);
struct mymsgbuf {
	long int mtype;
	char mtext[1];
};
void newtrace(char *s);
int init_newtracelogmsq();
int u2g(char *inbuf,int inlen,char *outbuf,int outlen);
int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen);
int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen);
int is_utf_special_byte(unsigned char c);
int is_utf(char * inbuf, size_t inlen);
#endif
