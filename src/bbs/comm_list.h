#ifndef BMYBBS_COMM_LIST_H
#define BMYBBS_COMM_LIST_H
char *sysconf_str(char *key);
int sysconf_eval(const char *key);
void load_sysconf(void);
int domenu(const char *menu_name);
#endif //BMYBBS_COMM_LIST_H
