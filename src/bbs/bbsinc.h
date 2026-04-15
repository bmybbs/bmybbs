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
#include "ytht/smth_filter.h"
#include "ythtbbs/article.h"
#include "ythtbbs/board.h"
#include "postheader.h"
extern int nettyNN;
extern int selboard;           /* THis flag is true if above is active */
extern int inBBSNET;
extern char IScurrBM;
extern char quote_file[120], quote_user[];
extern struct postheader header;

void do_delay(int i);
int underline_post(int, void *, char *);
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
int Select(const char *s);
time_t postfile(char *filename, char *nboard, char *posttitle, int mode);
int get_a_boardname(char *bname, size_t bname_len, const char *prompt);
int do_cross(int, void *, char *);
int cmpfilename(struct fileheader *fhdr);
int super_select_board(char *bname);
int water_post(int, void *, char *);
int digest_post(int, void *, char *);
int do_reply(struct fileheader *fh);
int transferattach(char *buf, size_t size, FILE *fp, FILE *fpto);
void do_quote(char *filepath, char quote_mode);
int do_post(int, void *, char *);
void add_loginfo(char *filepath);
void add_crossinfo(char *filepath, int mode);
int show_board_notes(char *bname);
enum ytht_smth_filter_result stringfilter(char *title, enum ytht_smth_filter_option mode);
int edit_post(int, void *, char *);
int edit_title(int, void *, char *);
int mark_post(int, void *, char *);
int has_perm_commend(char *userid);
int markdel_post(int, void *, char *);
int mark_minus_del_post(int, void *, char *);
int del_range(int, void *, char *);
int del_post(int, void *, char *);
int Save_post(int, void *, char *);
int Import_post(int, void *, char *);
int what_to_do(void);
int into_announce(int, void *, char *);
int forward_post(int, void *, char *);
int forward_u_post(int, void *, char *);
int Goodbye(const char *s);
int cmpbnames(struct boardheader *brec, char *bname);
void setbdir(char *buf, char *boardname, int Digestmode);
int zmodem_sendfile(int, void *, char *);
int deleted_mode(int, void *, char *);
int junk_mode(int, void *, char *);
int marked_mode(int, void *, char *);
int thread_mode(int, void *, char *);
void Add_Combine(char *board, struct fileheader *fileinfo);
int is_in_commend(char *board, struct fileheader *fileinfo);
int show_commend(void);
int is_in_commend2(char *board, struct fileheader *fileinfo);
int show_commend2(void);

#endif //BMYBBS_BBSINC_H
