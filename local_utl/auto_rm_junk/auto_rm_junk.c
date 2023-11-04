//自动删除.DELETED和.JUNK中所列过期文章. ylsdd 2000.6.7

#include "bbs.h"
#define LOCAL_UTL

int digestmode;
char currboard[50];

int
delete_old_junk(char *filename)
{
	struct fileheader fhdr;
	char tmpfile[STRLEN], deleted[STRLEN];
	int fdr, fdw;
	int count, mday, fmday, total, ndeleted;
	time_t now;
	struct stat statbuf;

	if (stat(filename, &statbuf)) {
		if (errno == ENOENT)
			return 0;
		return -1;
	}
	total = statbuf.st_size / sizeof (fhdr);

	time(&now);
	mday = now / (3600 * 24) % 100;

	tmpfilename(filename, tmpfile, deleted);
	if (digestmode == 4 || digestmode == 5) {
		tmpfile[strlen(tmpfile) - 1] = (digestmode == 4) ? 'D' : 'J';
		deleted[strlen(deleted) - 1] = (digestmode == 4) ? 'D' : 'J';
	}

	if ((fdr = open(filename, O_RDONLY, 0)) == -1) {
		if (errno == ENOENT)
			return 0;
		return -2;
	}

	ndeleted = 0;
	while (read(fdr, &fhdr, sizeof (fhdr)) == sizeof (fhdr)) {
		fmday = fhdr.deltime;
		while (fmday > mday)
			fmday -= 100;
		if (fhdr.accessed & FH_MARKED || mday - fmday < 30)
			continue;
		ndeleted++;
		break;
	}
	if (ndeleted == 0) {
		close(fdr);
		return 0;
	}
	if (lseek(fdr, 0, SEEK_SET) < 0) {
		close(fdr);
		return -3;
	}

	if ((fdw = open(tmpfile, O_WRONLY | O_CREAT | O_EXCL, 0660)) == -1) {
		close(fdr);
		return -4;
	}
	flock(fdw, LOCK_EX);

	count = 1;
	ndeleted = 0;
	while (read(fdr, &fhdr, sizeof (fhdr)) == sizeof (fhdr)) {
		char fullpath[STRLEN];
		snprintf(fullpath, sizeof fullpath, MY_BBS_HOME "/boards/%s/%s", currboard,
			fh2fname(&fhdr));
		fmday = fhdr.deltime;
		while (fmday > mday)
			fmday -= 100;
		if (fhdr.accessed & FH_MARKED || mday - fmday < 20) {//lanboy change 30 to 10
			if (mday - fmday > 3 && mday - fmday < 7
			    && file_size(fullpath) > 30000 && fhdr.accessed & FH_ATTACHED) {
				filter_attach(fullpath);
				fhdr.accessed &= ~FH_ATTACHED;
			}
			if ((safewrite(fdw, &fhdr, sizeof (fhdr)) == -1)) {
				unlink(tmpfile);
				flock(fdw, LOCK_UN);
				close(fdw);
				close(fdr);
				return -5;
			}
		} else {
			//   char fullpath[STRLEN];
			//   sprintf(fullpath,"/home/bbs/boards/%s/%s",currboard,fh2fname(&fhdr));
			//printf("%d %d %s\n", count, (int)fhdr.accessed, fullpath);
			// unlink(fullpath);
			ndeleted++;
		}
		count++;
	}
	close(fdr);

	if (rename(filename, deleted) == -1) {
		flock(fdw, LOCK_UN);
		close(fdw);
		return -6;
	}
	if (rename(tmpfile, filename) == -1) {
		flock(fdw, LOCK_UN);
		close(fdw);
		return -7;
	}

	flock(fdw, LOCK_UN);
	close(fdw);
	printf("%d %d ", count, ndeleted);
	return 0;
}

int
rm_junk(struct boardheader *bhp)
{
	char buf[200];
//   printf("%s\n",bhp->filename);
//   if(!strcmp(bhp->filename,"test")) {
	snprintf(buf, sizeof buf, MY_BBS_HOME "/boards/%s/.DELETED", bhp->filename);
	digestmode = 4;
	snprintf(currboard, sizeof currboard, "%s", bhp->filename);
	printf(".DELETED ");
	if (delete_old_junk(buf) != 0)
		return -1;
	snprintf(buf, sizeof buf, MY_BBS_HOME "/boards/%s/.JUNK", bhp->filename);
	digestmode = 5;
	snprintf(currboard, sizeof currboard, "%s", bhp->filename);
	printf(".JUNK ");
	if (delete_old_junk(buf) != 0)
		return -1;
//   }
	return 0;
}

int
main()
{
	int b_fd;
	struct boardheader bh;
	int size;
	time_t nowtime;

	size = sizeof (bh);

	chdir(MY_BBS_HOME);
	nowtime = time(NULL);
	printf("\033[1mbbs home=%s now time = %s\033[0m\n", MY_BBS_HOME,
	       ctime(&nowtime));
	if ((b_fd = open(MY_BBS_HOME "/.BOARDS", O_RDONLY)) == -1)
		return -1;
	flock(b_fd, LOCK_EX);
	while (read(b_fd, &bh, size) == size) {
		if (!bh.filename[0])
			continue;
		printf("processing %s...", bh.filename);
		if (rm_junk(&bh) < 0) {
			time(&nowtime);
			printf("FAILED! %s", ctime(&nowtime));
		} else {
			time(&nowtime);
			printf("....OK. %s", ctime(&nowtime));
		}
		sync();
	}
	flock(b_fd, LOCK_UN);
	close(b_fd);
	return 0;
}
