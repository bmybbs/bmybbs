/* libncce.c */
#ifndef __NCCE_H
#define __NCCE_H
char *mmapfile(char *fn);
int compare_word(char *word, char *item);
void cat_item(char *str, char *item, int maxlen);
char *search_dict(char *word);
#endif
