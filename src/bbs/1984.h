#ifndef BMYBBS_BBS_1984
#define BMYBBS_BBS_1984

void set1984file(char *path, char *filename);
void post_to_1984(char *file, struct fileheader *fileinfo, int mode);
void do1984menu(void);
#endif

