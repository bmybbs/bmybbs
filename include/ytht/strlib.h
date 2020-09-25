#ifndef BMYBBS_STRLIB_H
#define BMYBBS_STRLIB_H
#include <stdlib.h>

// 类似于 strstr(3)
char *ytht_strnstr(const char *haystack, const char *needle, size_t haystacklen);
char *strncasestr(const char *haystack, const char *needle, size_t haystacklen);
void normalize(char *buf);
void strsncpy(char *s1, const char *s2, int n);
char *strltrim(char *s);
char *strrtrim(char *s);
#define strtrim(s) strltrim(strrtrim(s))
#endif //BMYBBS_STRLIB_H
