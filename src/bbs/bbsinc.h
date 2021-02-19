/**
 * 本文件为临时文件，用于从 include/bbs.h 中剥离出仅适
 * 用于 src/bbs 的宏、变量声明等，后续需要进一步重构。
 * TODO
 */

#ifndef BMYBBS_BBSINC_H
#define BMYBBS_BBSINC_H
#include <stddef.h>
#include <stdio.h>
#include <setjmp.h>
#include "ythtbbs/article.h"
#include "postheader.h"
extern int nettyNN;
extern int selboard;           /* THis flag is true if above is active */
extern int inBBSNET;
extern char IScurrBM;
extern char quote_file[120], quote_user[];
extern struct postheader header;

void do_delay(int i);
int underline_post(int ent, struct fileheader *fileinfo, char *direct);
int set_safe_record(void);
void setqtitle(char *stitle);
int chk_currBM(struct boardheader *bh, int isbig);
void setquotefile(char filepath[]);
char *setbpath(char *buf, size_t len, const char *boardname);
char *setbfile(char *buf, size_t len, const char *boardname, const char *filename);
int deny_me(char *bname);
void shownotepad(void);
void make_blist(void);
void make_blist_full(void);
int junkboard(void);
void Select(void);
int postfile(char *filename, char *nboard, char *posttitle, int mode);
int get_a_boardname(char *bname, char *prompt);
int do_cross(int ent, struct fileheader *fileinfo, char *direct);
int cmpfilename(struct fileheader *fhdr);
int super_select_board(char *bname);
int water_post(int ent, struct fileheader *fileinfo, char *dirent);
int digest_post(int ent, struct fileheader *fhdr, char *direct);
int do_reply(struct fileheader *fh);
int transferattach(char *buf, size_t size, FILE *fp, FILE *fpto);
void do_quote(char *filepath, int quote_mode);
int do_post(void);
void add_loginfo(char *filepath);
void add_crossinfo(char *filepath, int mode);
int show_board_notes(char bname[30]);
int stringfilter(char *title, int mode);
int edit_post(int ent, struct fileheader *fileinfo, char *direct);
int edit_title(int ent, struct fileheader *fileinfo, char *direct);
int mark_post(int ent, struct fileheader *fileinfo, char *direct);
int has_perm_commend(char *userid);
int markdel_post(int ent, struct fileheader *fileinfo, char *direct);
int mark_minus_del_post(int ent, struct fileheader *fileinfo, char *direct);
int del_range(int ent, struct fileheader *fileinfo, char *direct);
int del_post(int ent, struct fileheader *fileinfo, char *direct);
int Save_post(int ent, struct fileheader *fileinfo, char *direct);
int Import_post(int ent, struct fileheader *fileinfo, char *direct);
int what_to_do(void);
int into_announce(void);
int forward_post(int ent, struct fileheader *fileinfo, char *direct);
int forward_u_post(int ent, struct fileheader *fileinfo, char *direct);
int Goodbye(void);
int cmpbnames(struct boardheader *brec, char *bname);
void setbdir(char *buf, char *boardname, int Digestmode);
int zmodem_sendfile(int ent, struct fileheader *fileinfo, char *direct);
int deleted_mode(void);
int junk_mode(void);
int marked_mode(void);
int thread_mode(void);
void Add_Combine(char *board, struct fileheader *fileinfo);
int is_in_commend(char *board, struct fileheader *fileinfo);
int show_commend(void);
int is_in_commend2(char *board, struct fileheader *fileinfo);
int show_commend2(void);

#endif //BMYBBS_BBSINC_H
