#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <dirent.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

#include "ytht/strlib.h"
#include "ytht/fileop.h"

sigjmp_buf bus_jump;

int
crossfs_rename(const char *oldpath, const char *newpath)
{
	char buf[1024 * 4];
	char *tmpfn;
	int retv, fdr, fdw;
	size_t size = 0;
	if (rename(oldpath, newpath) == 0)
		return 0;
	if (errno != EXDEV)
		return -1;
	if ((fdr = open(oldpath, O_RDONLY)) < 0)
		return -1;
	tmpfn = malloc(strlen(newpath) + 5);
	if (!tmpfn) {
		close(fdr);
		return -1;
	}
	strcpy(tmpfn, newpath);
	strcat(tmpfn, ".new");
	if ((fdw = open(tmpfn, O_WRONLY | O_CREAT, 0660)) < 0) {
		close(fdr);
		free(tmpfn);
		return -1;
	}
	while ((retv = read(fdr, buf, sizeof (buf))) > 0) {
		retv = write(fdw, buf, retv);
		if (retv < 0) {
			close(fdw);
			close(fdr);
			unlink(tmpfn);
			free(tmpfn);
			return -1;
		}
		size += retv;
	}
	close(fdw);
	close(fdr);
	if (rename(tmpfn, newpath) < 0) {
		unlink(tmpfn);
		free(tmpfn);
		return -1;
	}
	free(tmpfn);
	unlink(oldpath);
	return 0;
}

int
readstrvalue(const char *filename, const char *str, char *value, int size)
{
	FILE *fp;
	char buf[512], *ptr;
	int retv = -1, fd;
	fp = fopen(filename, "r");
	if (!fp)
		return -1;
	fd = fileno(fp);
	flock(fd, LOCK_SH);
	while (fgets(buf, sizeof (buf), fp)) {
		if (!(ptr = strchr(buf, ' ')))
			continue;
		*ptr++ = 0;
		if (strcmp(buf, str))
			continue;
		ytht_strsncpy(value, ptr, size);
		if ((ptr = strchr(value, '\n')))
			*ptr = 0;
		retv = 0;
		break;
	}
	flock(fd, LOCK_UN);
	fclose(fp);
	return retv;
}

int
readstrvalue_fp(FILE *fp, const char *str, char *value, size_t size)
{
	fseek(fp, 0, SEEK_SET);
	char buf[512], *ptr;
	int retv = -1;
	while (fgets(buf, sizeof (buf), fp)) {
		if (!(ptr = strchr(buf, ' ')))
			continue;
		*ptr++ = 0;
		if (strcmp(buf, str))
			continue;
		ytht_strsncpy(value, ptr, size);
		if ((ptr = strchr(value, '\n')))
			*ptr = 0;
		retv = 0;
		break;
	}
	return retv;
}

int
savestrvalue(const char *filename, const char *str, const char *value)
{
	FILE *fp;
	char buf[256], *ptr, *tmp;
	int fd, where = -1, rc;
	struct stat s;
	fp = fopen(filename, "r+");
	if (!fp) {
		fd = open(filename, O_CREAT | O_EXCL, 0600);
		if (fd != -1)
			close(fd);
		fp = fopen(filename, "r+");
	}
	if (!fp)
		return -2;
	fd = fileno(fp);
	flock(fd, LOCK_EX);
	if (fstat(fd, &s)) {
		fclose(fp);
		return -3;
	}
	tmp = malloc(s.st_size + 1);
	if (!tmp) {
		fclose(fp);
		return -4;
	}
	*tmp = 0;
	while (fgets(buf, sizeof (buf), fp)) {
		if (!(ptr = strchr(buf, ' ')))
			continue;
		*ptr = 0;
		if (!strcmp(buf, str)) {
			if (where < 0) {
				*ptr = ' ';
				where = ftell(fp) - strlen(buf);
			}
			continue;
		}
		*ptr = ' ';
		if (where < 0)
			continue;
		else
			strcat(tmp, buf);
	}
	if (where >= 0) {
		rc = fseek(fp, where, SEEK_SET);
		if (rc == -1) {
			fclose(fp);
			free(tmp);
			return -5;
		}
		fputs(tmp, fp);
	}
	free(tmp);
	fprintf(fp, "%s %.200s\n", str, value);
	ftruncate(fd, ftell(fp));
	fclose(fp);
	return 0;
}

#define cleanmmap(x) \
	if((x).ptr) {\
		if((x).size)\
			munmap((x).ptr, (x).size);\
		(x).ptr=NULL;\
	} else {;}

void
sigbus(int signo)
{
	siglongjmp(bus_jump, 1);
};

int
mmapfile(const char *filename, struct mmapfile *pmf)
{
	static char c;
	struct stat s;
	int fd;
	if (filename == NULL || stat(filename, &s) == -1) {
		cleanmmap(*pmf);
		return -1;
	}
	if (pmf->ptr != NULL && s.st_mtime == pmf->mtime)
		return 0;
	cleanmmap(*pmf);
	if (s.st_size == 0) {
		pmf->ptr = &c;
		pmf->size = 0;
		pmf->mtime = s.st_mtime;
		return 0;
	}
	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		pmf->ptr = 0;
		return -1;
	}
	pmf->ptr = mmap(NULL, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
	if (pmf->ptr == MAP_FAILED) {
		pmf->ptr = NULL;
		return -1;
	}
	pmf->mtime = s.st_mtime;
	pmf->size = s.st_size;
	return 0;
}

int
trycreatefile(char *path, char *fnformat, int startnum, int maxtry)
{
	int i, fd;
	char *ptr;
	if (startnum < 0)
		return -1;
	ptr = strrchr(path, '/');
	if (!ptr || *(ptr + 1)) {
		ptr = path + strlen(path);
		*ptr = '/';
		ptr++;
	} else
		ptr = path + strlen(path);

	for (i = 0; i < maxtry; i++) {
		sprintf(ptr, fnformat, startnum + i);
		fd = open(path, O_CREAT | O_EXCL | O_WRONLY, 0660);
		if (fd != -1) {
			close(fd);
			return startnum + i;
		}
	}
	return -1;
}

int
copyfile(char *from, char *to)
{
	int input, output;
	void *source, *target;
	off_t filesize;
	char endchar = 0;

	if ((input = open(from, O_RDONLY)) == -1)
		goto ERROR1;
	if ((output = open(to, O_RDWR | O_CREAT | O_TRUNC, 0666)) == -1)
		goto ERROR2;
	filesize = lseek(input, 0, SEEK_END);
	if (filesize == (off_t) -1)
		goto ERROR3;
	lseek(output, filesize - 1, SEEK_SET);
	write(output, &endchar, 1);
	if ((source = mmap(0, filesize, PROT_READ, MAP_SHARED, input, 0)) == (void *) -1)
		goto ERROR3;
	if ((target = mmap(0, filesize, PROT_WRITE, MAP_SHARED, output, 0)) == (void *) -1)
		goto ERROR4;
	memcpy(target, source, filesize);
	close(input);
	close(output);
	munmap(source, filesize);
	munmap(target, filesize);
	return 0;

ERROR4:
	munmap(source, filesize);
ERROR3:
	close(output);
ERROR2:
	close(input);
ERROR1:
	return -1;
}

int
openlockfile(const char *filename, int flag, int op)
{
	int retv;
	retv = open(filename, flag | O_CREAT, 0660);
	if (retv < 0)
		return -1;
	if (flock(retv, op) < 0) {
		close(retv);
		return -1;
	}
	return retv;
}

struct stat *
f_stat(char *file)
{
	static struct stat buf;
	if (stat(file, &buf) == -1)
		bzero(&buf, sizeof (buf));
	return &buf;
}

struct stat *
f_stat_s(struct stat *s, const char *file) {
	if (stat(file, s) == -1) {
		memset(s, 0, sizeof(struct stat));
	}

	return s;
}

struct stat *
l_stat(char *file)
{
	static struct stat buf;
	if (lstat(file, &buf) == -1)
		bzero(&buf, sizeof (buf));
	return &buf;
}

struct stat *
l_stat_s(struct stat *s, const char *file) {
	if (lstat(file, s) == -1) {
		memset(s, 0, sizeof(struct stat));
	}

	return s;
}

int
checkfilename(const char *str)
{
	if (!str[0] || !strcmp(str, ".") || !strcmp(str, ".."))
		return -1;
	while (*str) {
		if ((*str > 0 && *str < ' ') || isspace(*str) || strchr("\\/~`!@#$%^&*()|{}[];:\"'<>,?", *str))
			return -1;
		str++;
	}
	return 0;
}

int
clearpath(const char *path)
{
	DIR *pdir;
	struct dirent *pdent;
	char fname[1024];
	// int ret; // XXX: fix me
	pdir = opendir(path);
	if (!pdir)
		return -1;
	while ((pdent = readdir(pdir))) {
		if (!strcmp(pdent->d_name, "..") || !strcmp(pdent->d_name, "."))
			continue;
		if (checkfilename(pdent->d_name))
			continue;
		if (strlen(pdent->d_name) + strlen(path) >= sizeof (fname)) {
			break;
		}
		sprintf(fname, "%s/%s", path, pdent->d_name);
		//ret = unlink(fname);
		unlink(fname);
	}
	closedir(pdir);
	return 0;
}

// 此处原使用的宏 STRLEN 替换成 80
int seek_in_file(const char *filename, const char *seekstr)
{
	FILE *fp;
	char buf[80];
	char *namep;

	if ((fp = fopen(filename, "r")) == NULL)
		return 0;
	while (fgets(buf, 80, fp) != NULL) {
		namep = (char *) strtok(buf, ": \n\r\t");
		if (namep != NULL && strcasecmp(namep, seekstr) == 0) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

int ytht_add_to_file(char *filename, char *str) {
	FILE *fp;
	int rc;

	if ((fp = fopen(filename, "a")) == NULL)
		return -1;
	flock(fileno(fp), LOCK_EX);
	rc = fprintf(fp, "%s\n", str);
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return (rc == EOF ? -1 : 1);
}

int ytht_del_from_file(char *filename, char *str, bool include_lf) {
	int fdin, fdout;
	void *src, *dst;
	struct stat statbuf;
	char fnnew[256];
	char *p;
	size_t len, offset;
	char *local_buf;

	if ((fdin = open(filename, O_RDONLY)) < 0) {
		return -1;
	}

	flock(fdin, LOCK_EX); // released by close(fdin)

	if (fstat(fdin, &statbuf) < 0) {
		close(fdin);
		return -1;
	}

	if ((src = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0)) == MAP_FAILED) {
		close(fdin);
		return -1;
	}

	len = strlen(str);
	if (include_lf) {
		if ((local_buf = (char *) calloc(1, len+2)) == NULL) {
			munmap(src, statbuf.st_size);
			close(fdin);
		}
		sprintf(local_buf, "%s\n", str);
		len++;
	}

	if ((p = strstr((char*)src, include_lf ? local_buf : str)) == NULL) {
		if (include_lf) free(local_buf);
		munmap(src, statbuf.st_size);
		close(fdin);
		return -1;
	}


	// temporary file will be created at /tmp
	sprintf(fnnew, "/tmp/bbs-del_from_file-XXXXXX");
	if ((fdout = mkstemp(fnnew)) < 0) {
		if (include_lf) free(local_buf);
		munmap(src, statbuf.st_size);
		close(fdin);
		return -1;
	}

	if (lseek(fdout, statbuf.st_size - 1 - len, SEEK_SET) == -1) {
		if (include_lf) free(local_buf);
		close(fdout);
		unlink(fnnew);
		munmap(src, statbuf.st_size);
		close(fdin);
		return -1;
	}

	if (write(fdout, "", 1) != 1) {
		if (include_lf) free(local_buf);
		close(fdout);
		unlink(fnnew);
		munmap(src, statbuf.st_size);
		close(fdin);
		return -1;
	}

	if ((dst = mmap(0, statbuf.st_size - len, PROT_READ | PROT_WRITE, MAP_SHARED, fdout, 0)) == MAP_FAILED) {
		if (include_lf) free(local_buf);
		close(fdout);
		unlink(fnnew);
		munmap(src, statbuf.st_size);
		close(fdin);
		return -1;
	}

	offset = (void *)p - (void *)src;
	memcpy(dst, src, offset);
	memcpy(dst + offset, p + len, statbuf.st_size -  len - offset);

	if (include_lf) free(local_buf);
	munmap(dst, statbuf.st_size - len);
	munmap(src, statbuf.st_size);
	close(fdin);
	close(fdout);

	return (rename(fnnew, filename) + 1);
}

off_t ytht_file_size_s(const char *filepath) {
	struct stat buf;
	if(stat(filepath, &buf) == -1)
		memset(&buf, 0, sizeof(buf));

	return buf.st_size;
}

int ytht_file_has_word(char *file, char *word) {
	FILE *fp;
	char buf[256], buf2[256];
	fp = fopen(file, "r");
	if (fp == 0)
		return 0;
	while (1) {
		bzero(buf, 256);
		if (fgets(buf, 255, fp) == 0)
			break;
		sscanf(buf, "%s", buf2);
		if (!strcasecmp(buf2, word)) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

