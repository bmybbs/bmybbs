//ylsdd Nov 7, 2002
#include <unistd.h>
#include <dirent.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include "ythtbbs/ythtbbs.h"

static int
appendonebinaryattach(char *filename, char *attachname, char *attachfname)
{
	FILE *fp;
	int size, origsize;
	struct mmapfile mf = { .ptr = NULL };
	char ch = 0;
	struct stat st_attach, st_file;

	f_stat_s(&st_attach, attachfname);
	// file_isfile
	if (!(st_attach.st_mode & S_IFREG))
		return -1;

	if (mmapfile(attachfname, &mf) < 0)
		return -1;

	f_stat_s(&st_file, filename);
	origsize = st_file.st_size;
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
	if (st_file.st_size != origsize + 5 + mf.size + strlen("\nbeginbinaryattach \n") + strlen(attachname)) {
		truncate(filename, origsize);
		mmapfile(NULL, &mf);
		return -1;
	}
	mmapfile(NULL, &mf);
	unlink(attachfname);
	return 0;
}

int
appendbinaryattach(char *filename, char *userid, char *attachname)
{
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
	int attachcount=0;
	char attachfilenames[128][256];
	char attachfilepaths[128][1024];
	while ((pdent = readdir(pdir))) {
		if (!strcmp(pdent->d_name, "..") || !strcmp(pdent->d_name, "."))
			continue;
		snprintf(attachfilepaths[attachcount], sizeof(attachfname), "%s/%s", path, pdent->d_name);
		snprintf(attachfilenames[attachcount],256,"%s",pdent->d_name);
		attachcount++;
		/*if (appendonebinaryattach(filename, pdent->d_name, attachfname) == 0)
			count++;*/
	}
	attachcount--;
	while(attachcount>=0)
	{
		if(appendonebinaryattach(filename,attachfilenames[attachcount],attachfilepaths[attachcount])==0)
		{
			count++;
			attachcount--;
		}

	}
	closedir(pdir);

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
