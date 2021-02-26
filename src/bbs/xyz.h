#ifndef BMYBBS_XYZ_H
#define BMYBBS_XYZ_H
void loaduserkeys(void);
int modify_user_mode(int mode);
int showperminfo(unsigned int pbits, int i, int use_define);
int x_copykeys(const char *s);
int x_setkeys(const char *s);
int x_setkeys2(const char *s);
int x_setkeys3(const char *s);
int x_setkeys4(const char *s);
int x_setkeys5(const char *s);
unsigned int setperms(unsigned int pbits, char *prompt, int numbers, int (*showfunc)(unsigned int, int, int), int param);
int x_level(const char *s);
int x_userdefine(const char *s);
int x_cloak(const char *s);
int x_edits(const char *s);
int a_edits(const char *s);
int a_edits2(const char *s);
int x_lockscreen(const char *s);
int heavyload(float maxload);
int sendgoodwish(const char *uid);
int ent_bnet(const char *cmd);
int x_denylevel(const char *s);
char *directfile(char *fpath, char *direct, char *filename);
int zsend_file(char *from, char *title);
#endif //BMYBBS_XYZ_H
