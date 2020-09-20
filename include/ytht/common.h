#ifndef BMYBBS_COMMON_H
#define BMYBBS_COMMON_H
void _errlog(char *fmt, ...);
int mystrtok(char *str, int delim, char *result[], int max);

#define errlog(format, args...) _errlog(__FILE__ ":%s line %d " format, __FUNCTION__,__LINE__ , ##args)
#endif //BMYBBS_COMMON_H
