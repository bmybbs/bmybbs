#ifndef RZSZ_H
#define RZSZ_H
int bbs_zsendfile(char *filename, char *remote,
		  int *func_write(int, char *, int), void *func_oflush(void),
		  int *func_read(int, char *, int));
#endif
