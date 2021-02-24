#ifndef __BINARYATTACH_H
#define __BINARYATTACH_H
#include <stdio.h>
#include <stdbool.h>

bool hasbinaryattach(const char *userid);
int appendbinaryattach(const char *filename, const char *userid, const char *attachname);
char *checkbinaryattach(char *buf, FILE * fp, size_t *len);
#endif
