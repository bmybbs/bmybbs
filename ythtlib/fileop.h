/* fileop.c */
#ifndef __FILEOP_H
#define __FILEOP_H
#include <setjmp.h>
struct mmapfile {
	char *ptr;
	time_t mtime;
	size_t size;
};

#define MMAP_TRY \
    if (!sigsetjmp(bus_jump, 1)) { \
        signal(SIGBUS, sigbus);

#define MMAP_CATCH \
    } \
    else { \

#define MMAP_END } \
    signal(SIGBUS, SIG_IGN);

#define MMAP_UNTRY {signal(SIGBUS, SIG_IGN);}
#define MMAP_RETURN(x) {signal(SIGBUS, SIG_IGN);return (x);}
#define MMAP_RETURN_VOID {signal(SIGBUS, SIG_IGN);return;}
sigjmp_buf bus_jump;

int crossfs_rename(const char *oldpath, const char *newpath);
int readstrvalue(const char *filename, const char *str, char *value, int size);

/**
 * 从配置文件中读取值
 * 配置文件格式：
 *   key1 value1
 *   key2 value2
 * 作用和 readstrvalue 一样，区别在于若同时从一个文件中读取配置信息，减少 fopen/fclose 的调用次数。
 * @see readstrvalue
 * @param[in]   fp    配置文件描述符
 * @param[in]   str   键
 * @param[out]  value 存放值的缓冲区
 * @param[in]   size  缓冲区大小
 * @return
 */
int readstrvalue_fp(FILE *fp, const char *str, char *value, size_t size);
int savestrvalue(const char *filename, const char *str, const char *value);
void sigbus(int signo);
int mmapfile(char *filename, struct mmapfile *pmf);
int trycreatefile(char *path, char *fnformat, int startnum, int maxtry);
int copyfile(char *source, char *destination);
int openlockfile(const char *filename, int flag, int op);
int checkfilename(const char *filename);
int clearpath(const char *path);

/**
 * @brief 从文件中查找是否包含某个字符串。
 * 该函数来自于 src/talk.c 以及 nju09/bbssnd.c。
 * @param filename 文件路径名称
 * @param seekstr 需要查找的字符串
 * @return 若包含返回 1，否则返回 0。
 */
int seek_in_file(char* filename, char *seekstr);
#endif
