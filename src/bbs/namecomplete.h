#ifndef BMYBBS_NAMECOMPLETE_H
#define BMYBBS_NAMECOMPLETE_H
#include <stddef.h>
void CreateNameList(void);
void AddNameList(const char *name);
int namecomplete(char *prompt, char *data, size_t data_len);
int usercomplete(char *prompt, char *data);
#endif //BMYBBS_NAMECOMPLETE_H
