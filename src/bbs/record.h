#ifndef BMYBBS_RECORD_H
#define BMYBBS_RECORD_H
long get_num_records(char *filename, int size);
long get_num_records_excludeBottom(char *filename, int size);
int apply_record(char *filename, int (*fptr)(void *), int size);
int get_records(char *filename, void *rptr, int size, int id, int number);
int insert_record(char *fpath, void *data, int size, int pos, int num);
int delete_range(char *filename, int id1, int id2);
int update_file(char *dirname, int size, int ent, int (*filecheck)(void *), void (*fileupdate)(void *));
#endif //BMYBBS_RECORD_H
