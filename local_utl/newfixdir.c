#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "bbs.h"
#include "ythtbbs.h"
extern int errno;

const int MAX_ARTICLE_COUNT = 200000;
struct fileheader * data;
int len = 0;

void insertfile(struct fileheader  fh)
{
	data[len] = fh;
	len++;
}

int
cmpfile(f1, f2)
struct fileheader *f1, *f2;
{
	return f1->filetime - f2->filetime;
}

main(int argc, char **argv)
{
	data = (struct fileheader * )malloc(sizeof(struct fileheader) * MAX_ARTICLE_COUNT);

	DIR *pdir;
	char *name;
	char buf1[256], buf2[256];
	struct dirent *ent;
	int i, lastcount;
	FILE *art;
	int file, file1, flag;
	struct fileheader fh, *x;
	struct mmapfile mf = { ptr:NULL };

	if (argc > 1)
		name = argv[1];
	else
		return -1;

	pdir = opendir(name);
	chdir(name);

//	sprintf(buf1, "chmod 744 %s", name);
//	system(buf1);

	file = open(".tmpfile", O_CREAT | O_TRUNC | O_WRONLY, 0600);
	if (file == 0) {
		perror("open .tmpfile error!\n");
		closedir(pdir);
		return -1;
	}
	i = 1;

	while ((ent = readdir(pdir))!=NULL) {
		if ((strcmp(ent->d_name, ".DIR"))
		    && (strcmp(ent->d_name, "."))
		    && (strcmp(ent->d_name, ".."))
		    && (ent->d_name[0] == 'M' || ent->d_name[0] == 'G')) {
			struct stat st;
			if (stat(ent->d_name, &st))
				continue;
			if ((art = fopen(ent->d_name, "r")) != NULL) {
				char *p;
				bzero(&fh, sizeof (fh));
				fgets(buf1, 256, art);
				if (buf1 == 0)
					continue;
				p = strchr(buf1 + 8, ' ');
				if (p)
					*p = 0;
				if (p = strchr(buf1 + 8, '('))
					*p = 0;
				if (p = strchr(buf1 + 8, '\n'))
					*p = 0;
				fh_setowner(&fh, buf1 + 8, 0);
				fgets(buf2, 256, art);
				if (buf2 == 0)
					continue;
				printf("%s", buf2);
				if (p = strchr(buf2 + 8, '\n'))
					*p = 0;
				if (p = strchr(buf2 + 8, '\r'))
					*p = 0;
				fh.filetime = atoi(ent->d_name + 2);
				fh.thread = fh.filetime;
				if (ent->d_name[0] == 'G')
					fh.accessed |= FH_DIGEST;
				strsncpy(fh.title, buf2 + 8, sizeof (fh.title));
				if (strncmp(buf1, "发信站", 6)
				    && strncmp(buf1, "寄信人: ", 8)
				    && strncmp(buf1, "发信人: ", 8))
					continue;
				if ((strncmp(buf2, "标  题: ", 8))
				    && (strncmp(buf2, "标　题: ", 8)))
					continue;
				//append_record(".tmpfile", &fh, sizeof(fh));
				insertfile(fh);
				fclose(art);
			}
		}
	}
	qsort(data, len, sizeof (struct fileheader), cmpfile);

	MMAP_TRY {
		if(mmapfile(".DIR", &mf) < 0) {
			MMAP_UNTRY;
		}
		int total = mf.size / sizeof(struct fileheader);
		for(i=0; i<len; ++i) {
			int num = Search_Bin(mf.ptr, data[i].filetime, 0, total-1);
			if(num >= 0) {
				x = mf.ptr + num * sizeof(struct fileheader);
				data[i].accessed = x->accessed;
				data[i].thread = x->thread;
			}
		}
	} MMAP_CATCH {

	}
	MMAP_END mmapfile(NULL, &mf);

	printf("end.len=%d %d", len, len * sizeof (struct fileheader));
	if (write(file, data, len * sizeof (struct fileheader)) == 0)
		perror("write error");
	if (close(file))
		printf("close error=%d\n", errno);
	closedir(pdir);
//	if (!flag) {
//		sprintf(buf1, "chmod 755 %s", name);
//		system(buf1);
//		sprintf(buf1, "mv -f .tmpfile .DIR");
//		system(buf1);
//	}
}
