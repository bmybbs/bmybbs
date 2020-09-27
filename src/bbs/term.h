#ifndef BMYBBS_TERM_H
#define BMYBBS_TERM_H
void init_tty(void);
void term_init(void);
void do_move(int destcol, int destline, int (*outc) ());
#endif //BMYBBS_TERM_H
