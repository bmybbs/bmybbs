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

void uuencode(FILE * fr, FILE * fw, int len, char *filename);

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
#include "smth_filter.h"

#endif
