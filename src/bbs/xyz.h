#ifndef BMYBBS_XYZ_H
#define BMYBBS_XYZ_H
void loaduserkeys(void);
int modify_user_mode(int mode);
int showperminfo(unsigned int pbits, int i, int use_define);
int x_copykeys(void);
int x_setkeys(void);
int x_setkeys2(void);
int x_setkeys3(void);
int x_setkeys4(void);
int x_setkeys5(void);
unsigned int setperms(unsigned int pbits, char *prompt, int numbers, int (*showfunc)(unsigned int, int, int), int param);
int x_level(void);
int x_userdefine(void);
int x_cloak(void);
void x_edits(void);
void a_edits(void);
void a_edits2(void);
void x_lockscreen(void);
int heavyload(float maxload);
int sendgoodwish(char *uid);
void x_recite(void);
void x_quickcalc(void);
void x_freeip(void);
int ent_bnet(char *cmd);
int x_denylevel(void);
char *directfile(char *fpath, char *direct, char *filename);
int zsend_file(char *from, char *title);
void inn_reload(void);
#endif //BMYBBS_XYZ_H
