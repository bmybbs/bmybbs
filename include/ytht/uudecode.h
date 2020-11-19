#ifndef BMYBBS_UUDECODE_H
#define BMYBBS_UUDECODE_H
#include <stdio.h>

int uudecode(FILE * fp, char *outname);
int fakedecode(FILE * fp);
char *attachdecode(FILE * fp, char *articlename, char *filename);
#endif //BMYBBS_UUDECODE_H
