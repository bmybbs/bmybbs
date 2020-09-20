#ifndef __YTHTLIB_H
#define __YTHTLIB_H
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>

/*#ifndef sizeof
#define sizeof(x) ((int)sizeof(x))
#endif*/

#ifndef ATTACHCACHE
#define ATTACHCACHE "/home/bbsattach/cache"
#endif

int uudecode(FILE * fp, char *outname);
int fakedecode(FILE * fp);
char *attachdecode(FILE * fp, char *articlename, char *filename);
void uuencode(FILE * fr, FILE * fw, int len, char *filename);
#define errlog(format, args...) _errlog(__FILE__ ":%s line %d " format, __FUNCTION__,__LINE__ , ##args)

#define file_size(x) (f_stat(x)->st_size)
#define file_time(x) (f_stat(x)->st_mtime)
#define file_rtime(x) (f_stat(x)->st_atime)
//#define file_exist(x) (file_time(x)!=0)
#define file_exist(x) (access(x, F_OK)==0)
#define file_isdir(x) ((f_stat(x)->st_mode & S_IFDIR)!=0)
#define file_isfile(x) ((f_stat(x)->st_mode & S_IFREG)!=0)
#define lfile_isdir(x) ((l_stat(x)->st_mode & S_IFDIR)!=0)

unsigned char numbyte(int n);
int bytenum(unsigned char c);

char *strnstr(const char *haystack, const char *needle, size_t haystacklen);
char *strncasestr(const char *haystack, const char *needle, size_t haystacklen);

void *try_get_shm(int key, int size, int flag);
void *get_shm(int key, int size);
#define get_old_shm(x,y) try_get_shm(x,y,0)
//Copy from Linux 2.4 kernel...
#define min(x,y) ({ \
	const typeof(x) _x = (x);	\
	const typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x < _y ? _x : _y; })

#define max(x,y) ({ \
	const typeof(x) _x = (x);	\
	const typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x > _y ? _x : _y; })
//end
#include "fileop.h"
#include "named_socket.h"
#include "crypt.h"
#include "limitcpu.h"
#include "timeop.h"
#include "common.h"
#include "strop.h"

#define BAD_WORD_NOTICE "您的文章可能含有按照国家有关规定" \
                       "不适宜在公共场合发表的内容\n本文须" \
                       "经过有关人士审查后发表\n"            \
                       "如果您有疑问，请联系在线的站长审核该文章"
#define DO1984_NOTICE  "按照有关部门要求,本版文章必须经过有关" \
                       "人士审查后发表,请耐心等候!\n"           \
                       "如果您有疑问，请联系在线的站长审核该文章"

int reload_badwords(char *wordlistf, char *imgf);
int filter_file(char *checkfile, struct mmapfile *badword_img);
int filter_string(char *string, struct mmapfile *badword_img);
int filter_article(char *title, char *filename, struct mmapfile *badword_img);

#endif
