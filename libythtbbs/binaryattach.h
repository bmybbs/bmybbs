#ifndef __BINARYATTACH_H
#define __BINARYATTACH_H
int appendbinaryattach(char *filename, char *userid, char *attachname);
char *checkbinaryattach(char *buf, FILE * fp, size_t *len);
#endif
