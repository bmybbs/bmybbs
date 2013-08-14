/* record.c */
#ifndef __RECORD_H
#define __RECORD_H
#ifdef SYSV
int flock(int fd, int op);
#endif
void tmpfilename(char *filename, char *tmpfile, char *deleted);
int safewrite(int fd, void *buf, int size);
int delete_record(char *filename, int size, int id);
int append_record(char *filename, void *record, int size);
int new_apply_record(char *filename, int size, int (*fptr) (void *, void *),
		     void *farg);
int new_search_record(char *filename, void *rptr, int size,
		      int (*fptr) (void *, void *), void *farg);
int search_record(char *filename, void *rptr, int size,
		  int (*fptr) (void *, void *), void *farg);
int delete_file(char *filename, int size, int ent, int (*filecheck) (void *));
#endif
