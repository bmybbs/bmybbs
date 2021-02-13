/* fileop.c */
#ifndef __FILEOP_H
#define __FILEOP_H
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <setjmp.h>
#include <sys/stat.h>
#include <fcntl.h>
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
extern sigjmp_buf bus_jump;

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
int mmapfile(const char *filename, struct mmapfile *pmf);
time_t trycreatefile(char *path, char *fnformat, time_t startnum, int maxtry);
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
int seek_in_file(const char* filename, const char *seekstr);

#ifdef BMYBBS_MT
/**
 * A wrapper of stat(2)
 * @warning Thread UNsafe
 * @param file
 * @return
 */
struct stat *f_stat(char *file) __attribute__((deprecated("use f_stat_s instead")));
#else
/**
 * A wrapper of stat(2)
 * @warning Thread UNsafe
 * @param file
 * @return
 */
struct stat *f_stat(char *file);
#endif

#ifdef BMYBBS_MT
/**
 * A wrapper of lstat(2)
 * @warning Thread UNsafe
 * @param file
 * @return
 */
struct stat *l_stat(char *file) __attribute__((deprecated("use l_stat_s instead")));
#else
/**
 * A wrapper of lstat(2)
 * @warning Thread UNsafe
 * @param file
 * @return
 */
struct stat *l_stat(char *file);
#endif

/**
 * A thread-safe wrapper of stat(2)
 * @param s
 * @param file
 * @return
 */
struct stat *f_stat_s(struct stat *s, const char *file);

/**
 * A thread-safe wrapper of lstat(2)
 * @param s
 * @param file
 * @return
 */
struct stat *l_stat_s(struct stat *s, const char *file);

#define file_size(x) (f_stat(x)->st_size)
#define file_time(x) (f_stat(x)->st_mtime)
#define file_rtime(x) (f_stat(x)->st_atime)
//#define file_exist(x) (file_time(x)!=0)
#define file_exist(x) (access(x, F_OK)==0)
#define file_isdir(x) ((f_stat(x)->st_mode & S_IFDIR)!=0)
#define file_isfile(x) ((f_stat(x)->st_mode & S_IFREG)!=0)
#define lfile_isdir(x) ((l_stat(x)->st_mode & S_IFDIR)!=0)

bool ytht_file_isfile_x(struct stat *st);
/**
 * @brief 将 str 追加到 filename 结尾
 *
 * filename 内容形如："$str1\n$str2\n"。操作中对 filename 使用
 * 独占锁。
 * @param filename
 * @param str
 * @return
 */
int ytht_add_to_file(char *filename, char *str);

/**
 * @brief 基于 mmap(2) 和 strstr(3) 实现的从文件中移除字符串。
 *
 * 仅会移除 str 在文件中的首次出现。strstr(3) 可能不适用于文件或者
 * 待搜索字符串较长的情况，幸好 bbs 的数据文件一般都不大。
 *
 * 用于取代原 src/bbs/talk.c::del_from_file() 实现，增加了独占锁。
 * @param filename
 * @param str
 * @param include_lf
 * @return
 *    -1 文件不存在，或无法创建临时文件，或者 str 不存在于原文件中
 *     0 重命名临时文件替换原文件失败
 *     1 替换成功
 */
int ytht_del_from_file(char *filename, char *str, bool include_lf);

/**
 * @brief 获取文件大小
 * 对应于宏 file_size 的函数实现，以便多线程安全
 */
off_t ytht_file_size_s(const char *filepath);

/**
 * @brief 来自 nju09 原 file_has_word 函数
 */
int ytht_file_has_word(char *file, char *word);
#endif
