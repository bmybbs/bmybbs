#ifndef YTHTBBS_ATTACH_H
#define YTHTBBS_ATTACH_H
#include <stdio.h>
void filter_attach(char *filename);
int insertattachments(const char *filename, char *content, const char *userid);
int getattach(FILE * fp, char *attachfile, char *nowfile, int base64, int len, int fake);
int insertattachments_byfile(const char *filename, char *tmpfile, const char *userid);
int decode_attach(char *filename, char *path);
int copyfile_attach(char *source, char *target);
#endif
