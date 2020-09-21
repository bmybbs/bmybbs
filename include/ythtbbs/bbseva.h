#ifndef __BBSEVA_H
#define __BBSEVA_H
#define BBSEVA_SOCKETFILE ".bbseva_socket"
int init_bbsevamsq(void);
int bbseva_askoneid(int ent, char *board, char *filename, char *id);
int bbseva_askavg(int ent, char *board, char *filename, float *avg);
int bbseva_set(int ent, char *board, char *filename, char *id, int star);
int bbseva_qset(int ent, char *board, char *filename, char *id, int star,
int *oldstar, int *count, float *avg);
#endif
