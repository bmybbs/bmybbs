#ifndef BMYBBS_CONVCODE_H
#define BMYBBS_CONVCODE_H
extern int g_convcode;

void switch_code(void);
void conv_init(void);

char *gb2big(char *s, int *plen, int inst);
char *big2gb(char *s, int *plen, int inst);
#endif //BMYBBS_CONVCODE_H
