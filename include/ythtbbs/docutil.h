/* docutil.c */
#ifndef __DOCUTIL_H
#define __DOCUTIL_H
#include <stdio.h>
size_t eff_size(const char *file);
char *getdocauthor(char *filename, char *author, int len);
enum {
	KEEPHEADER,
	RESTOREHEADER,
	SKIPHEADER
};
int keepoldheader(FILE * fp, int dowhat);
int copyheadertofile(FILE *from_fp, FILE *to_fp);
#endif
