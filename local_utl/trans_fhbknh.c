//by ylsdd
//only for reference, should be modified according to different system

#include "bbs.h"
#include "ythtbbs.h"

int
transfh(struct fileheader *nfh, struct oldfileheader *fh)
{
	bzero(nfh, sizeof (*nfh));
	if (!strchr("GMF", fh->filename[0]) || fh->filename[1] != '.')
		return -1;
	if (fh->filename[0] == 'G')
		nfh->accessed |= FH_ISDIGEST;
	nfh->filetime = atoi(fh->filename + 2);
	if (!nfh->filetime)
		return -2;
	//nfh->edittime = fh->edittime;
	if ((fh->accessed[0] & FILE_READ))
		nfh->accessed |= FH_READ;
	if ((fh->accessed[0] & FILE_VISIT))
		nfh->accessed |= FH_HIDE;
	if ((fh->accessed[0] & FILE_MARKED))
		nfh->accessed |= FH_MARKED;
	if ((fh->accessed[0] & FILE_DIGEST))
		nfh->accessed |= FH_DIGEST;
	if ((fh->accessed[0] & FILE_NOREPLY))
		nfh->accessed |= FH_NOREPLY;
	if ((fh->accessed[0] & FILE_ATTACHED))
		nfh->accessed |= FH_ATTACHED;
	if ((fh->accessed[0] & MAIL_REPLY))
		nfh->accessed |= FH_REPLIED;
	if ((fh->accessed[1] & FILE1_DEL))
		nfh->accessed |= FH_DEL;
	if ((fh->accessed[1] & FILE1_SPEC))
		nfh->accessed |= FH_SPEC;
	if ((fh->accessed[1] & FILE1_INND))
		nfh->accessed |= FH_INND;
	if ((fh->accessed[1] & FILE1_ANNOUNCE))
		nfh->accessed |= FH_ANNOUNCE;
	if ((fh->accessed[1] & FILE1_1984))
		nfh->accessed |= FH_1984;
	strsncpy(nfh->title, fh->title, sizeof (nfh->title));

//	if (!strcasecmp(fh->owner, "Anonymous"))
//		fh_setowner(nfh, fh->realauthor, 1);
//	else
		fh_setowner(nfh, fh->owner, 0);

//	if (fh->sizebyte >= 255)
//		fh->sizebyte--;
//	nfh->sizebyte = fh->sizebyte + 1;	//让nfh->sizebyte=0表示大小未知吧
        nfh->sizebyte = 1;
	nfh->staravg50 = 0;
//	nfh->hasvoted = fh->hasvoted;
	nfh->deltime = fh->accessed[11];
	return 0;
}

int
transbknh(struct bknheader *nfh, struct oldfileheader *fh)
{
	char *ptr;
	bzero(nfh, sizeof (*nfh));
	if (fh->filename[0] != 'B')
		return -1;
	nfh->filetime = atoi(fh->filename + 2);
	if (!nfh->filetime)
		return -1;
	strsncpy(nfh->title, fh->title, sizeof (nfh->title));
	strsncpy(nfh->boardname, fh->owner, sizeof (nfh->boardname));
	return 0;
}

int
trans_fileheaderfile(char *from, char *to)
{
	int fdr, retv, retv2, errorno;
	FILE *fpw;
	struct oldfileheader ofh;
	struct fileheader nfh;
	printf("FH--F=%s T=%s\n", from, to);
	fdr = open(from, O_RDONLY);
	if (fdr < 0) {
		perror("open from");
		exit(1);
	}
	fpw = fopen(to, "wb");
	if (!fpw) {
		printf("fopen to\n");
		exit(1);
	}
	while ((retv = read(fdr, &ofh, sizeof (ofh))) == sizeof (ofh)) {
		if ((errorno=transfh(&nfh, &ofh)) < 0) {
			printf("transfh ofh fname %s#%s#%d\n", ofh.filename,
			       ofh.title,errorno);
			if (strlen(ofh.filename) <= 2 || strlen(ofh.title) == 0)
				continue;
			exit(1);
		}
		fwrite(&nfh, sizeof (nfh), 1, fpw);
	}
	if (retv != 0) {
		printf("trans_fileheaderfile: retv=%d\n", retv);
		printf("Abort? Y/*\n");
		if ('Y' == getc(stdin))
			exit(1);
	}
	fclose(fpw);
	close(fdr);
	return 0;
}

int
trans_bknheaderfile(char *from, char *to)
{
	int fdr, retv;
	FILE *fpw;
	struct oldfileheader ofh;
	struct bknheader nbh;
	printf("BKNH--F=%s T=%s\n", from, to);
	fdr = open(from, O_RDONLY);
	if (fdr < 0) {
		perror("open from");
		exit(1);
	}
	fpw = fopen(to, "wb");
	if (!fpw) {
		printf("fopen to\n");
		exit(1);
	}
	while ((retv = read(fdr, &ofh, sizeof (ofh))) == sizeof (ofh)) {
		if (transbknh(&nbh, &ofh) < 0) {
			printf("transbknh\n");
			exit(1);
		}
		fwrite(&nbh, sizeof (nbh), 1, fpw);
	}
	if (retv != 0) {
		printf("trans_bknheaderfile: retv=%d\n", retv);
		printf("Abort? Y/*");
		if ('Y' == fgetc(stdin))
			exit(1);
	}
	fclose(fpw);
	close(fdr);
	return 0;
}

int
main()
{
	char buf[1024], buf2[1024], *ptr;
	FILE *fp;
	fp = fopen("filelist.txt", "rt");
	if (!fp)
		exit(1);
	while (fgets(buf, sizeof (buf), fp)) {
		if ((ptr = strchr(buf, '\n')))
			*ptr = 0;
		if (!file_exist(buf)) {
			printf("error f_exist %s", buf);
			exit(1);
		}
		strcpy(buf2, buf);
		strcat(buf2, ".org");
		if (strstr(buf, ".backnumbers/") && !strstr(buf, "/B.")) {
			//printf("bknheader: %s\n", buf);
			trans_bknheaderfile(buf2, buf);
		} else {
			//printf("fileheader: %s\n", buf);
			trans_fileheaderfile(buf2, buf);
		}
		fflush(stdout);
	}
	return 0;
}
