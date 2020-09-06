#ifndef __PATHOP_H
#define __PATHOP_H
#if HAVE_DIRENT_H
#  include <dirent.h>
#else
#    define dirent direct
#    define NAMLEN(dirent) (dirent)->d_namlen
#  if HAVE_SYS_NDIR_H
#    include <sys/ndir.h>
#  endif
#  if HAVE_SYS_DIR_H
#    include <sys/dir.h>
#  endif
#  if HAVE_NDIR_H
#    include <ndir.h>
#  endif
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

char *contractpath(char *path);
char *constructpath(const char *fname);
int my_open(const char *filename, int flages, mode_t mode);
int my_stat(const char *file_name, struct stat *buf);
int my_lstat(const char *file_name, struct stat *buf);
FILE *my_fopen(const char *path, const char *mode);
int my_rename(const char *oldpath, const char *newpath);
DIR *my_opendir(const char *name);
int my_chdir(const char *path);
char *my_getcwd(char *buf, size_t size);
int my_unlink(const char *pathname);
int my_mkdir(const char *pathname, mode_t mode);
int my_rmdir(const char *pathname);
int my_chmod(const char *path, mode_t mode);
#define NUM_QUOTA_TYPE 2
struct quota {
	int hasinit;
	long q[NUM_QUOTA_TYPE];
	char d[NUM_QUOTA_TYPE][256];
	int changed;
};
extern struct quota *quota, *quotaf;
int my_quota_init();
int my_quota_type(const char *filename);
#endif
