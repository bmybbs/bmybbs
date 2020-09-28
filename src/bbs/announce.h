#ifndef BMYBBS_ANNOUNCE_H
#define BMYBBS_ANNOUNCE_H
int valid_fname(char *str);
void a_prompt(int bot, char *pmt, char *buf, int len);
int a_Save(char *path, char *key, struct fileheader *fileinfo, int nomsg);
int check_import(char *anboard);
int a_Import(char *direct, struct fileheader *fileinfo, int nomsg);
int a_menusearch(char *path, char *key, int level);
void a_menu(char *maintitle, char *path, int lastlevel, int lastbmonly);
void linkto(char *path, char *fname, char *title);
int add_grp(char group[80], char gname[80], char bname[80], char title[80]);
int del_grp(char grp[80], char bname[80], char title[80]);
int edit_grp(char bname[80], char grp[80], char title[80], char newtitle[100]);
void Announce(void);
void Personal(char *cmd);
int select_anpath(void);
#endif //BMYBBS_ANNOUNCE_H
