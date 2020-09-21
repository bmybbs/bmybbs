/* docutil.c */
#ifndef __DOCUTIL_H
#define __DOCUTIL_H
#include <stdio.h>
int eff_size(char *file);
char *getdocauthor(char *filename, char *author, int len);
enum {
	KEEPHEADER,
	RESTOREHEADER,
	SKIPHEADER
};
int keepoldheader(FILE * fp, int dowhat);
int copyheadertofile(FILE *from_fp, FILE *to_fp);
#endif
