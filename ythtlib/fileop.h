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
int savestrvalue(const char *filename, const char *str, const char *value);
void sigbus(int signo);
int mmapfile(char *filename, struct mmapfile *pmf);
int trycreatefile(char *path, char *fnformat, int startnum, int maxtry);
int copyfile(char *source, char *destination);
int openlockfile(const char *filename, int flag, int op);
int checkfilename(const char *filename);
int clearpath(const char *path);
#endif
