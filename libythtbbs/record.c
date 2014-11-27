#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/mman.h>
#include "ythtbbs.h"
#include "ythtlib.h"
#ifdef NUMBUFFER
#undef NUMBUFFER
#endif
#define NUMBUFFER 100
#define BUFSIZE (8192)
#define PATHLEN 256

#ifdef SYSV
int
flock(fd, op)
int fd, op;
{
	switch (op) {
	case LOCK_EX:
		return lockf(fd, F_LOCK, 0);
	case LOCK_UN:
		return lockf(fd, F_ULOCK, 0);
	default:
		return -1;
	}
}
#endif

void
tmpfilename(filename, tmpfile, deleted)
char *filename, *tmpfile, *deleted;
{
	char *ptr, *delfname, *tmpfname;

	strcpy(tmpfile, filename);
	delfname = ".deleted";
	tmpfname = ".tmpfile";
	if ((ptr = strrchr(tmpfile, '/')) != NULL) {
		strcpy(ptr + 1, delfname);
		strcpy(deleted, tmpfile);
		strcpy(ptr + 1, tmpfname);
	} else {
		strcpy(deleted, delfname);
		strcpy(tmpfile, tmpfname);
	}
}

int
safewrite(fd, buf, size)
int fd;
void *buf;
int size;
{
	int cc, sz = size, origsz = size;
	char *bp = buf;

	do {
		cc = write(fd, bp, sz);
		if ((cc < 0) && (errno != EINTR)) {
			errlog("safewrite err! %d", errno);
			return -1;
		}
		if (cc > 0) {
			bp += cc;
			sz -= cc;
		}
	} while (sz > 0);
	return origsz;
}

//id=1 refer to the first record
int
delete_record(filename, size, id)
char *filename;
int size, id;
{
	char tmpfile[PATHLEN], deleted[PATHLEN];
	char abuf[BUFSIZE];
	int fdr, fdw;
	int count;

	if (size > BUFSIZE)
		return -1;
	if (strlen(filename) - 8 > sizeof (tmpfile))
		return -1;
	tmpfilename(filename, tmpfile, deleted);

	if ((fdr = open(filename, O_RDONLY, 0)) == -1) {
		errlog("delrec open err %d", errno);
		return -1;
	}
	if ((fdw = open(tmpfile, O_WRONLY | O_CREAT | O_EXCL, 0660)) == -1) {
		errlog("delrec tmp err %d", errno);
		close(fdr);
		return -1;
	}
	count = 1;
	while (read(fdr, abuf, size) == size) {
		if (id != count++ && (safewrite(fdw, abuf, size) == -1)) {
			unlink(tmpfile);
			close(fdr);
			close(fdw);
			errlog("delrec write err");
			return -1;
		}
	}
	close(fdr);
	close(fdw);
	if (rename(filename, deleted) == -1 || rename(tmpfile, filename) == -1) {
		errlog("delrec rename err");
		return -1;
	}
	return 0;
}

int
append_record(char *filename, void *record, int size)
{
	int fd;
	struct flock ldata;
	if ((fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0660)) == -1) {
		return -1;
	}
	ldata.l_type = F_WRLCK;
	ldata.l_start = 0;
	ldata.l_whence = SEEK_END;
	ldata.l_len = 0;
	fcntl(fd, F_SETLKW, &ldata);

	write(fd, record, size);
	close(fd);
	return 0;
}

int
new_apply_record(char *filename, int size, int (*fptr) (void *, void *),
		 void *farg)
{
	char *buf;
	int fd, sizeread, n, i, retv;

	if ((fd = open(filename, O_RDONLY, 0)) == -1)
		return -1;

	if ((buf = malloc(size * NUMBUFFER)) == NULL) {
		close(fd);
		return -1;
	}

	while ((sizeread = read(fd, buf, size * NUMBUFFER)) > 0) {
		n = sizeread / size;
		for (i = 0; i < n; i++) {
			retv = (*fptr) (buf + i * size, farg);
			if (retv > 0) {
				close(fd);
				free(buf);
				return retv;
			}
		}
		if (sizeread % size != 0) {
			close(fd);
			free(buf);
			return -1;
		}
	}
	close(fd);
	free(buf);
	return 0;
}

int
new_search_record(char *filename, void *rptr, int size,
		  int (*fptr) (void *, void *), void *farg)
{
	int fd;
	int id = 1;
	char *buf;
	int sizeread, n, i;

	if ((buf = malloc(size * NUMBUFFER)) == NULL) {
		return -1;
	}
	if ((fd = open(filename, O_RDONLY, 0)) == -1) {
		free(buf);
		return 0;
	}
	while ((sizeread = read(fd, buf, size * NUMBUFFER)) > 0) {
		n = sizeread / size;
		for (i = 0; i < n; i++) {
			if ((*fptr) (buf + i * size, farg)) {
				memcpy(rptr, buf + i * size, size);
				close(fd);
				free(buf);
				return id;
			}
			id++;
		}
		if (sizeread % size != 0) {
			close(fd);
			free(buf);
			return -1;
		}
	}
	close(fd);
	free(buf);
	return 0;
}

int
search_record(char *filename, void *rptr, int size,
	      int (*fptr) (void *, void *), void *farg)
{
	int fd;
	int id = 1;
	char *buf;
	int sizeread, n, i;

	if ((buf = malloc(size * NUMBUFFER)) == NULL) {
		return -1;
	}
	if ((fd = open(filename, O_RDONLY, 0)) == -1) {
		free(buf);
		return 0;
	}
	while ((sizeread = read(fd, buf, size * NUMBUFFER)) > 0) {
		n = sizeread / size;
		for (i = 0; i < n; i++) {
			if ((*fptr) (farg, buf + i * size)) {
				memcpy(rptr, buf + i * size, size);
				close(fd);
				free(buf);
				return id;
			}
			id++;
		}
		if (sizeread % size != 0) {
			close(fd);
			free(buf);
			return -1;
		}
	}
	close(fd);
	free(buf);
	return 0;
}

int
delete_file(dirname, size, ent, filecheck)
char *dirname;
int size, ent;
int (*filecheck) (void *);
{
	int fd;
	struct stat st;
	int ret = 0, pos;
	char *ptr = NULL;

	if ((fd = open(dirname, O_RDWR)) == -1)
		return -1;
	flock(fd, LOCK_EX);
	fstat(fd, &st);
	MMAP_TRY {
		ptr =
		    mmap(0, st.st_size, PROT_READ | PROT_WRITE,
			 MAP_FILE | MAP_SHARED, fd, 0);
		ret = 0;
		pos = ent;
		if (pos * size > st.st_size) {
			ret = -2;
		} else {
			if (filecheck) {
				for (pos = ent; pos * size <= st.st_size; pos++)
					if ((*filecheck)
					    (ptr + (pos - 1) * size))
						break;
				if (pos * size > st.st_size)
					ret = -2;
			}
		}
		if (ret == 0) {
			char *tmp_dup = malloc(st.st_size);
			memcpy(tmp_dup, ptr, st.st_size);
			memcpy(ptr + (pos - 1) * size, tmp_dup + pos * size,
			       st.st_size - size * pos);
			free(tmp_dup);
			ftruncate(fd, st.st_size - size);
		}
	}
	MMAP_CATCH {
	}
	MMAP_END munmap(ptr, st.st_size);
	flock(fd, LOCK_UN);
	close(fd);
	return ret;
}
