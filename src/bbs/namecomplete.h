#ifndef BMYBBS_NAMECOMPLETE_H
#define BMYBBS_NAMECOMPLETE_H
void CreateNameList(void);
void AddNameList(char *name);
int chkstr(char *otag, char *tag, char *name);
int namecomplete(char *prompt, char *data);
int usercomplete(char *prompt, char *data);
#endif //BMYBBS_NAMECOMPLETE_H