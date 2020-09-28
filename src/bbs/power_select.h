#ifndef BMYBBS_POWER_SELECT_H
#define BMYBBS_POWER_SELECT_H
int full_search_action(char *whattosearch);
int power_action(char *filename, unsigned int id1, int id2, char *select, int action);
const char *strstr2(const char *s, const char *s2);
int power_select(int ent, struct fileheader *fileinfo, char *direct);
#endif //BMYBBS_POWER_SELECT_H
