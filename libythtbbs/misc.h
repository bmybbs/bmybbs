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
int u2g(char *inbuf,size_t inlen,char *outbuf,size_t outlen);
int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen);
int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen);
int is_utf_special_byte(unsigned char c);
int is_utf(char * inbuf, size_t inlen);

/**
 * @brief 加盐的方法
 * 来自 nju09
 * @param salt
 */
void getsalt(char salt[3]);

int badstr(unsigned char *s);

/**
 * @brief 获取不超过4张进站图片
 * 该方法将把当前进站图片的信息输出到 pics_list 中。每条信息使用单个半角分号将图片和
 * 链接隔开。多条信息使用两个半角分号隔开。
 * @warning 该函数最早于 2011.09.05 由 IronBlood 编写在 nju09/bbsindex.c 中。为了
 * api 复用，调整到了 libythtlib/misc.h 中，并变更了函数原型。2014.10.22 再次做了变
 * 更，记得使用 free() 函数释放返回的字符串。
 */
char * get_no_more_than_four_login_pics();
#endif
