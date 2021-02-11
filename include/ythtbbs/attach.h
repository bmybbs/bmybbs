#ifndef YTHTBBS_ATTACH_H
#define YTHTBBS_ATTACH_H
#include <stdio.h>
void filter_attach(char *filename);
int insertattachments(char *filename, char *content, char *userid);
int getattach(FILE * fp, char *currline, char *attachfile, char *nowfile, int base64, int len, int fake);
int insertattachments_byfile(char *filename, char *tmpfile, char *userid);
int decode_attach(char *filename, char *path);
int copyfile_attach(char *source, char *target);
#endif
