#ifndef __BINARYATTACH_H
#define __BINARYATTACH_H
#include <stdio.h>
int appendbinaryattach(char *filename, char *userid, char *attachname);
char *checkbinaryattach(char *buf, FILE * fp, unsigned int *len);
#endif
