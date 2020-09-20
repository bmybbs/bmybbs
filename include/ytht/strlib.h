#ifndef BMYBBS_STRLIB_H
#define BMYBBS_STRLIB_H
#include <stdlib.h>

char *strnstr(const char *haystack, const char *needle, size_t haystacklen);
char *strncasestr(const char *haystack, const char *needle, size_t haystacklen);
#endif //BMYBBS_STRLIB_H
