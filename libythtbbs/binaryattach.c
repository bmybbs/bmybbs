//ylsdd Nov 7, 2002
#include <unistd.h>
#include <dirent.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include "ythtbbs/ythtbbs.h"

static int appendonebinaryattach(const char *filename, const char *attachname, const char *attachfname) {
	FILE *fp;
	int size;
	unsigned int origsize;
	struct mmapfile mf = { .ptr = NULL };
	char ch = 0;
	struct stat st_attach, st_file;

	f_stat_s(&st_attach, attachfname);
	// file_isfile
	if (!ytht_file_isfile_x(&st_attach))
		return -1;

	if (mmapfile(attachfname, &mf) < 0)
		return -1;

	f_stat_s(&st_file, filename);
	origsize = (unsigned int) st_file.st_size; /* should be safe */
	fp = fopen(filename, "a");
	if (!fp) {
		mmapfile(NULL, &mf);
		return -1;
	}

	fprintf(fp, "\nbeginbinaryattach %s\n", attachname);
	fwrite(&ch, 1, 1, fp);
	size = htonl(mf.size);
	fwrite(&size, sizeof (size), 1, fp);
	fwrite(mf.ptr, mf.size, 1, fp);
	fclose(fp);

	f_stat_s(&st_file, filename);
	if ((unsigned int) st_file.st_size != origsize + 5 + mf.size + strlen("\nbeginbinaryattach \n") + strlen(attachname)) {
		truncate(filename, origsize);
		mmapfile(NULL, &mf);
		return -1;
	}
	mmapfile(NULL, &mf);
	unlink(attachfname);
	return 0;
}

#define MAX_ATTACH_COUNT 128
#define MAX_ATTACH_PATH  1024
#define MAX_ATTACH_NAME  256

struct AttachFile {
	char name[MAX_ATTACH_NAME];
	char path[MAX_ATTACH_PATH];
};

bool hasbinaryattach(const char *userid) {
	bool result = false;
	DIR *pdir;
	struct dirent *pdent;
	char path[512];
	snprintf(path, sizeof(path), PATHUSERATTACH "/%s", userid);
	if ((pdir = opendir(path)) != NULL) {
		while ((pdent = readdir(pdir)) != NULL) {
			if (!strcmp(pdent->d_name, "..") || !strcmp(pdent->d_name, ".")) {
				continue;
			} else {
				result = true;
				break;
			}
		}
		closedir(pdir);
	}
	return result;
}

int appendbinaryattach(const char *filename, const char *userid, const char *attachname) {
	DIR *pdir;
	struct dirent *pdent;
	char attachfname[1024], path[512];
	int count = 0;
	if (attachname) {
		snprintf(attachfname, sizeof (attachfname), PATHUSERATTACH "/%s/%s", userid, attachname);
		if (strstr(attachfname, "/../"))
			return -1;
		if (appendonebinaryattach(filename, attachname, attachfname) == 0)
			count++;
		return count;
	}
	snprintf(path, sizeof (path), PATHUSERATTACH "/%s", userid);
	pdir = opendir(path);
	if (!pdir)
		return -1;

	int attachcount = 0;
	struct AttachFile *attachfiles = calloc(MAX_ATTACH_COUNT, sizeof(struct AttachFile));
	if (attachfiles == NULL) {
		closedir(pdir);
		return -1;
	}

	// 将文件夹列表先读取到缓冲区
	while ((pdent = readdir(pdir))) {
		if (!strcmp(pdent->d_name, "..") || !strcmp(pdent->d_name, "."))
			continue;
		snprintf(attachfiles[attachcount].path, MAX_ATTACH_PATH, "%s/%s", path, pdent->d_name);
		ytht_strsncpy(attachfiles[attachcount].name, pdent->d_name, MAX_ATTACH_NAME);
		attachcount++;
		if (attachcount >= MAX_ATTACH_COUNT)
			break;
	}
	attachcount--;
	while (attachcount >= 0) {
		if (appendonebinaryattach(filename, attachfiles[attachcount].name, attachfiles[attachcount].path) == 0) {
			count++;
			attachcount--;
		}
	}
	closedir(pdir);
	free(attachfiles);

	return count;
}

char *
checkbinaryattach(char *buf, FILE * fp, size_t *len)
{
	char ch, *ptr;
	if (strncmp(buf, "beginbinaryattach ", 18))
		return NULL;

	fread(&ch, 1, 1, fp);
	if (ch != 0) {
		ungetc(ch, fp);
		return NULL;
	}
	ptr = strchr(buf, '\r');
	if (ptr)
		*ptr = 0;
	ptr = strchr(buf, '\n');
	if (ptr)
		*ptr = 0;
	ptr = buf + 18;
	fread(len, 4, 1, fp);
	*len = ntohl(*len);
	return ptr;
}
