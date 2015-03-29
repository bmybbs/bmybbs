#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/types.h>
#include <dirent.h>

#include "ythtlib.h"

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
		strsncpy(value, ptr, size);
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
savestrvalue(const char *filename, const char *str, const char *value)
{
	FILE *fp;
	char buf[256], *ptr, *tmp;
	int fd, where = -1;
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
		fseek(fp, where, SEEK_SET);
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
mmapfile(char *filename, struct mmapfile *pmf)
{
	static char c;
	struct stat s;
	int fd;
	if (filename == NULL || stat(filename, &s) == -1) {
		cleanmmap(*pmf);
		return filename ? (-1) : 0;
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
	size_t filesize;
	char endchar = 0;

	if ((input = open(from, O_RDONLY)) == -1)
		goto ERROR1;
	if ((output = open(to, O_RDWR | O_CREAT | O_TRUNC, 0666)) == -1)
		goto ERROR2;
	filesize = lseek(input, 0, SEEK_END);
	lseek(output, filesize - 1, SEEK_SET);
	write(output, &endchar, 1);
	if ((source = mmap(0, filesize, PROT_READ, MAP_SHARED, input, 0)) ==
	    (void *) -1)
		goto ERROR3;
	if ((target = mmap(0, filesize, PROT_WRITE, MAP_SHARED, output, 0)) ==
	    (void *) -1)
		goto ERROR4;
	memcpy(target, source, filesize);
	close(input);
	close(output);
	munmap(source, filesize);
	munmap(target, filesize);
	return 0;

      ERROR4:munmap(source, filesize);
      ERROR3:close(output);
      ERROR2:close(input);
      ERROR1:return -1;
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
l_stat(char *file)
{
	static struct stat buf;
	if (lstat(file, &buf) == -1)
		bzero(&buf, sizeof (buf));
	return &buf;
}

int
checkfilename(const char *str)
{
	if (!str[0] || !strcmp(str, ".") || !strcmp(str, ".."))
		return -1;
	while (*str) {
		if ((*str > 0 && *str < ' ') || isspace(*str)
		    || strchr("\\/~`!@#$%^&*()|{}[];:\"'<>,?", *str))
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
	int ret;
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
		ret = unlink(fname);
	}
	closedir(pdir);
	return 0;
}
