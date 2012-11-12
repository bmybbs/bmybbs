#include <stdio.h>
#include <sys/file.h>
#include <sys/mman.h>
#include "ythtbbs.h"

static int comfakedecode(FILE * fp, int base64, int len);
static int comdecode(FILE * fp, char *filename, int base64, int len);

void
filter_attach(char *path)
{
	FILE *fp;
	int hasa, fd, len;
	char buf[2048];
#ifdef LOCAL_UTL
	static char *tmp = NULL;
	static unsigned int nlen = 0;
#else
	char *tmp = NULL;
#endif
	struct stat s;
	fp = fopen(path, "r+");
	if (fp == NULL)
		return;
	fd = fileno(fp);
	flock(fd, LOCK_EX);
	if (fstat(fd, &s)) {
		errlog("can't fstat!");
		fclose(fp);
		return;
	}
#ifdef LOCAL_UTL
	if (s.st_size + 1 > nlen) {
		char *ptr;
		ptr = realloc(tmp, s.st_size + 1);
		if (NULL == ptr) {
			errlog("no enough memory!");
			return;
		}
		tmp = ptr;
		nlen = s.st_size + 1;
	}
#else
	{
		tmp = malloc(s.st_size + 1);
		if (NULL == tmp) {
			errlog("no enough memory!");
			return;
		}
	}
#endif
	hasa = 0;
	*tmp = 0;
	while (fgets(buf, sizeof (buf), fp) != NULL) {
		if (!strncmp(buf, "begin 644 ", 10)) {
			hasa = 1;
			fakedecode(fp);
		} else if (checkbinaryattach(buf, fp, &len)) {
			hasa = 1;
			fseek(fp, len, SEEK_CUR);
		} else
			strcat(tmp, buf);
	}
	if (hasa) {
		fseek(fp, 0L, SEEK_SET);
		fprintf(fp, "%s", tmp);
		ftruncate(fd, ftell(fp));
	}
#ifndef LOCAL_UTL
	free(tmp);
#endif
	fclose(fp);
}

int
insertattachments(char *filename, char *content, char *userid)
{
	int retv = 0;
	char *ptr, *ptr1, *p0 = content;
	FILE *fp;
	unlink(filename);
	fp = fopen(filename, "a");
	if (!fp)
		return 0;
	while ((ptr = strsep(&p0, "\n"))) {
		if (strncmp(ptr, "#attach ", 8)) {
			if (ptr != content)
				fputc('\n', fp);
			fputs(ptr, fp);
			continue;
		}
		if ((ptr1 = strchr(ptr, '\r')))
			*ptr1 = 0;
		if ((ptr1 = strchr(ptr, '\n')))
			*ptr1 = 0;
		if ((ptr1 = strchr(ptr, '/')))
			*ptr1 = 0;
		ptr1 = strrchr(ptr + 7, ' ');
		while (*ptr1 && strchr(" \t", *ptr1))
			ptr1++;
		if (!*ptr1)
			continue;
		fflush(fp);
		if (appendbinaryattach(filename, userid, ptr1))
			retv = 1;
	}
	fclose(fp);
	if (appendbinaryattach(filename, userid, NULL) > 0)
		retv = 1;
	return retv;
}

int
insertattachments_byfile(char *filename, char *tmpfile, char *userid)
{
	int retv = 0;
	char *ptr1;
	char ptr[256];
	FILE *fp, *fpr;
	int i = 0;
	fpr = fopen(tmpfile, "r");
	if (!fpr) {
		errlog("empty source to insert attach");
		return 0;
	}
	unlink(filename);
	fp = fopen(filename, "a");
	if (!fp) {
		fclose(fpr);
		return 0;
	}

	while (fgets(ptr, 256, fpr) != NULL) {
		if ((ptr1 = strchr(ptr, '\n')))
			*ptr1 = 0;
		i++;
		if (strncmp(ptr, "#attach ", 8)) {
			if (i != 1)
				fputc('\n', fp);
			fputs(ptr, fp);
			continue;
		}
		if ((ptr1 = strchr(ptr, '\r')))
			*ptr1 = 0;
		if ((ptr1 = strchr(ptr, '/')))
			*ptr1 = 0;
		ptr1 = strrchr(ptr + 7, ' ');
		while (*ptr1 && strchr(" \t", *ptr1))
			ptr1++;
		if (!*ptr1)
			continue;
		fflush(fp);
		if (appendbinaryattach(filename, userid, ptr1))
			retv = 1;
	}
	fclose(fp);
	fclose(fpr);
	if (appendbinaryattach(filename, userid, NULL) > 0)
		retv = 1;
	return retv;
}

int
getattach(FILE * fp, char *currline, char *attachfile, char *nowfile,
	  int base64, int len, int fake)
{
	char buf[PATH_MAX + 1], *ext;
	strncpy(buf, nowfile, sizeof (buf));
	buf[PATH_MAX] = 0;
	
	ext = strstr(buf, "..");
	if (ext != NULL)
		*ext = 0;
	if (access(buf, F_OK) && mkdir(buf, 0755)) {
		if (comfakedecode(fp, base64, len))
			return -1;
		else
			return -11;
	}
	if (attachfile[strlen(attachfile) - 1] == '\n')
		attachfile[strlen(attachfile) - 1] = 0;
	if (strlen(attachfile) > NAME_MAX
	    || strlen(buf) + strlen(attachfile) + 1 > PATH_MAX) {
		if (comfakedecode(fp, base64, len))
			return -2;
		else
			return -12;
	}
	strcat(buf, "/");
	strcat(buf, attachfile);
	if (fake) {
		if (comfakedecode(fp, base64, len))
			return -3;
		else
			return 0;
	}
	if (comdecode(fp, buf, base64, len))
		return -3;
	else
		return 0;
}

static int
comdecode(FILE * fp, char *filename, int base64, int len)
{
	if (base64)
		return uudecode(fp, filename);
	else {
		int output;
		char endchar = 0;
		void *target;
		if ((output =
		     open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666)) == -1)
			goto ERROR1;
		lseek(output, len - 1, SEEK_SET);
		write(output, &endchar, 1);
		if ((target =
		     mmap(0, len, PROT_WRITE, MAP_SHARED, output,
			  0)) == (void *) -1)
			goto ERROR2;
		fread(target, len, 1, fp);
		close(output);
		munmap(target, len);
		return 0;
	      ERROR2:close(output);
		unlink(filename);
	      ERROR1:return -1;
	}
}

static int
comfakedecode(FILE * fp, int base64, int len)
{
	if (base64)
		return fakedecode(fp);
	else
		return fseek(fp, len, SEEK_CUR);
}

int
decode_attach(char *filename, char *path)
{
	FILE *fp;
	char buf[500];
	int isa = 0, base64, len;
	char *fn = NULL;
	fp = fopen(filename, "r");
	if (fp == NULL)
		return -1;
	while (1) {
		if (fgets(buf, 500, fp) == 0)
			break;
		if (isa && (!strcmp(buf, "\r\n") || !strcmp(buf, "\n")))	//附件之后吞一个空行
			continue;
		base64 = isa = 0;
		if (!strncmp(buf, "begin 644", 10)) {
			isa = 1;
			base64 = 1;
			len = 0;
			fn = buf + 10;
		} else if (checkbinaryattach(buf, fp, &len)) {
			isa = 1;
			base64 = 0;
			fn = buf + 18;
		}
		if (isa) {
			getattach(fp, buf, fn, path, base64, len, 0);
		}
	}
	fclose(fp);
	return 0;
}

int
copyfile_attach(char *source, char *target)
{
	FILE *fpr, *fpw;
	char buf[500];
	int isa = 0, base64, len;
	char *fn = NULL;
	fpr = fopen(source, "r");
	if (fpr == NULL)
		return -1;
	fpw = fopen(target, "w");
	if (fpw == NULL) {
		fclose(fpr);
		return -1;
	}
        while (1) {
                if (fgets(buf, 500, fpr) == 0)
                        break;
                if (isa && (!strcmp(buf, "\r\n") || !strcmp(buf, "\n")))        //附件之后吞一个空行
                        continue;
                base64 = isa = 0;
                if (!strncmp(buf, "begin 644", 10)) {
                        isa = 1;
                        base64 = 1;
                        len = 0;
                        fn = buf + 10;
                } else if (checkbinaryattach(buf, fpr, &len)) {
                        isa = 1;
                        base64 = 0;
                        fn = buf + 18;
                }
                if (isa) {
                        if (!getattach(fpr, buf, fn, "/tmp" , base64, len, 1)) {
                                fprintf(fpw, "#attach %s\n", fn);
                        }
                } else
                        fprintf(fpw, "%s", buf);
        }
	fclose(fpw);
	fclose(fpr);
	return 0;
}
