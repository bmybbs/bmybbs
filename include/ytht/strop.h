#ifndef BMYBBS_STROP_H
#define BMYBBS_STROP_H
void normalize(char *buf);
void strsncpy(char *s1, const char *s2, int n);
char *strltrim(char *s);
char *strrtrim(char *s);
#define strtrim(s) strltrim(strrtrim(s))
#endif //BMYBBS_STROP_H
