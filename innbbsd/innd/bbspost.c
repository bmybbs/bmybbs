/*
 * File: scanboard.c
 */
char *ProgramUsage = "\
bbspost (list|visit) bbs_home\n\
        post board_path < uid + title + Article...\n\
        mail board_path < uid + title + passwd + realfrom + Article...\n\
        cancel bbs_home board filename\n\
        expire bbs_home board days [max_posts] [min_posts]\n";
#define INNDHOME "/home/bbs/inndlog"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#if !defined(PalmBBS)

#include "bbs.h"
#include "ythtbbs.h"
//#include "../innbbsconf.h"

#define MAXLEN          1024

char *crypt();
char *homepath;
int visitflag;
char realfrom[MAXLEN];

usage()
{
	puts(ProgramUsage);
	exit(0);
}

report()
{
	/* Function called from record.o */
	/* Please leave this function empty */
}

struct boardmem *getbcache(char *bname);

int
ci_strcmp(s1, s2)
register char *s1, *s2;
{
	char c1, c2;

	while (1) {
		c1 = *s1++;
		c2 = *s2++;
		if (c1 >= 'a' && c1 <= 'z')
			c1 += 'A' - 'a';
		if (c2 >= 'a' && c2 <= 'z')
			c2 += 'A' - 'a';
		if (c1 != c2)
			return (c1 - c2);
		if (c1 == 0)
			return 0;
	}
}

void
search_article(brdname)
char *brdname;
{
//fileheader¶¨Òå¸ÄÁË, ÏóÕâÑùµÄÐ´.DIRµÄ·½Ê½²»ºÏÊÊÁË, ¶øÇÒËÆºõÃ»ÓÐÊ²Ã´µØ·½µ÷ÓÃÕâ¸öº¯Êý¹¦ÄÜÁË, ×¢ÊÍµô
#if 0
	struct fileheader head;
	struct stat state;
	char index[MAXLEN], article[MAXLEN];
	int fd, num, offset, type;
	char send;

	offset = (int) &(head.filename[STRLEN - 1]) - (int) &head;
	sprintf(index, "%s/boards/%s/.DIR", homepath, brdname);
	if ((fd = open(index, O_RDWR)) < 0) {
		return;
	}
	fstat(fd, &state);
	num = (state.st_size / sizeof (head)) - 1;
	while (num >= 0) {
		lseek(fd, num * sizeof (head) + offset, 0);
		if (read(fd, &send, 1) > 0 && send == '%')
			break;
		num -= 4;
	}
	num++;
	if (num < 0)
		num = 0;
	lseek(fd, num * sizeof (head), 0);
	for (send = '%'; read(fd, &head, sizeof (head)) > 0; num++) {
		type = head.filename[STRLEN - 1];
		if (type != send && visitflag) {
			lseek(fd, num * sizeof (head) + offset, 0);
			safewrite(fd, &send, 1);
			lseek(fd, (num + 1) * sizeof (head), 0);
		}
		if (type == '\0') {
			printf("%s\t%s\t%s\t%s\n", brdname,
			       head.filename, head.owner, head.title);
		}
	}
	close(fd);
#endif
}

void
search_boards(visit)
{
	struct dirent *de;
	DIR *dirp;
	char buf[8192], *ptr;
	int fd, len;

	return;

	visitflag = visit;
	sprintf(buf, "%s/boards", homepath);

	if ((dirp = opendir(buf)) == NULL) {
		printf(":Err: unable to open %s\n", buf);
		return;
	}

	printf("New article listed:\n");

	while ((de = readdir(dirp)) != NULL) {
		if (de->d_name[0] > ' ' && de->d_name[0] != '.')
			search_article(de->d_name);
	}

	closedir(dirp);
}

check_password(record)
struct userec *record;
{
	FILE *fn;
	char *pw;
	char passwd[MAXLEN];
	char genbuf[MAXLEN];

	fgets(passwd, MAXLEN, stdin);
	tailbr(passwd);
	pw = crypt(passwd, record->passwd);
	if (strcmp(pw, record->passwd) != 0) {
		printf(":Err: user '%s' password incorrect!!\n",
		       record->userid);
		exit(0);
	}
	fgets(realfrom, sizeof (realfrom), stdin);
	tailbr(realfrom);

/*    sprintf( genbuf, "tmp/email_%s", record->userid );
    if( (fn = fopen( genbuf, "w" )) != NULL ) {
        fprintf( fn, "%s\n", realfrom );
        fclose( fn );
    }*/
	if (!strstr(realfrom, "bbs@")) {
		record->ip[15] = '\0';
		strncpy(record->realmail, realfrom, STRLEN - 16);
	}
/*    if( !strstr( homepath, "test" ) ) {
        record->numposts++;
    }*/
}

check_userec(record, name)
struct userec *record;
char *name;
{
	int fh;
	struct userec currentuser;
	struct boardmem *bptr;
	char *ptr;

	if ((fh = open(".PASSWDS", O_RDWR)) == -1) {
		printf(":Err: unable to open .PASSWDS file.\n");
		exit(0);
	}

	ptr = strrchr(homepath, '/');
	if (ptr == NULL)
		ptr = homepath;
	else
		ptr++;
	bptr = getbcache(ptr);
	if (bptr == NULL)
		exit(0);

	while (read(fh, record, sizeof *record) > 0) {
		if (ci_strcmp(name, record->userid) == 0) {
			strcpy(name, record->userid);
			check_password(record);

			currentuser = *record;
			if (!HAS_PERM(PERM_POST))
				exit(0);
			if (!((bptr->header.level & PERM_POSTMASK)
			      || HAS_PERM(bptr->header.level)
			      || (bptr->header.level & PERM_NOZAP)))
				exit(0);
			if (!
			    (HAS_PERM
			     ((bptr->header.level & ~PERM_NOZAP) & ~PERM_POSTMASK)))
			    exit(0);

			if (deny_me(homepath, record->userid)
			    || deny_me("/home/bbs", record->userid))
				exit(0);
			if ((bptr->header.clubnum != 0)
			    && (!club_me(homepath, record->userid)))
				exit(0);

			lseek(fh, -1 * sizeof *record, SEEK_CUR);
			safewrite(fh, record, sizeof *record);
			close(fh);
			return;
		}
	}
	close(fh);
	printf(":Err: unknown userid %s\n", name);
	exit(0);
}

void *
getrealauthor(char *buf, char *author, int len)
{
	char *ptr,*f1,*f2;
	if (strncmp(buf, "¼ÄÐÅÈË: ", 8) && strncmp(buf, "·¢ÐÅÈË: ", 8))
		return;
	ptr = buf + 8;
	f1 = strsep(&ptr, " ,\n\r\t");
	if (f1)
		strsncpy(author, f1, len);
	f2 = strsep(&ptr, " ,\n\r\t");
	if (f2 && f2[0] == '<' && f2[strlen(f2) - 1] == '>'
	    && strchr(f2, '@')) {
		f2[strlen(f2) - 1] = 0;
		strsncpy(author, f2 + 1, len-1);
	}
	ptr = strpbrk(author, "();:!#$\"\'");
	if (ptr)
		*ptr = 0;
	strcat(author,".");
	return;
}

post_article(usermail)
{
	struct userec record;
	struct fileheader header;
	char userid[MAXLEN], subject[MAXLEN];
	char index[MAXLEN], name[MAXLEN], article[MAXLEN];
	char buf[MAXLEN], *ptr;
	FILE *fidx;
	int fh, islocalpost, n = 0,i=0,bypost=0;
	time_t now;

	sprintf(index, "%s/.DIR", homepath);
	if ((fidx = fopen(index, "r")) == NULL) {
		if ((fidx = fopen(index, "w")) == NULL) {
			printf(":Err: Unable to post in %s.\n", homepath);
			return;
		}
	}
	fclose(fidx);

	fgets(userid, sizeof (userid), stdin);
	tailbr(userid);
	fgets(subject, sizeof (subject), stdin);
	tailbr(subject);
	if (usermail) {
		check_userec(&record, userid);
	}
	if(!strcmp(userid,"post"))
		bypost=1;
	now = time(NULL);
	n = 0;
	while (n < 30) {
		sprintf(name, "M.%d.A", now + n);
		sprintf(article, "%s/%s", homepath, name);
		fh = open(article, O_CREAT | O_EXCL | O_WRONLY, 0644);
		if (fh != -1)
			break;
		n++;
	}
	if (fh == -1)
		return;

	printf("post to %s\n", article);
	ptr = strrchr(homepath, '/');
	(ptr == NULL) ? (ptr = homepath) : (ptr++);
	if (usermail) {
		char local[MAXLEN];

		fgets(local, sizeof (local), stdin);
		tailbr(local);
		islocalpost = strcmp(local, "localpost");
		sprintf(buf, "\
·¢ÐÅÈË: %s (%s), ÐÅÇø: %s\n\
±ê  Ìâ: %s\n\
·¢ÐÅÕ¾: Ò»ËúºýÍ¿ BBS (%24.24s), %s\n\n", userid, record.username, ptr, subject, ctime(&now), islocalpost ? "×ªÐÅ" : "Õ¾ÄÚÐÅ¼þ");
		write(fh, buf, strlen(buf));
	}
	while (fgets(buf, MAXLEN, stdin) != NULL) {
		if(bypost&&(++i)==3){
			char tmp[256];
			strsncpy(tmp,buf,256);
			getrealauthor(tmp,userid,14);
		}
		write(fh, buf, strlen(buf));
	}
	if (usermail) {
		FILE *foo;
		char outname[80];

		/*Ð´Èë×ªÐÅµµ */
/*        if(islocalpost)
        {
          sprintf(outname,"%s/out.bntp",INNDHOME);
          if (foo = fopen(outname, "a"))
          {
            fprintf(foo, "%s\t%s\t%s\t%s\t%s\n", ptr,
              name, userid, record.username, subject);
            fclose(foo);
          }
        }
*/
		realfrom[20] = '\0';
		sprintf(buf,
			"\n--\n[1;36m¡î À´Ô´:£®[1;42m[1;31mÒ»ËúºýÍ¿ BBS ytht.net[0m£®[FROM: %s]\n",
			realfrom);
		write(fh, buf, strlen(buf));
	}
	close(fh);

	bzero((void *) &header, sizeof (header));
	header.filetime = atoi(name + 2);
	fh_setowner(&header, userid, 0);
	strsncpy(header.title, subject, sizeof (header.title));
	fh_find_thread(&header,ptr);
	append_record(index, &header, sizeof (header));
	updatelastpost(ptr);
}

cancel_article(board, file, message)
char *board, *file, *message;
{
	struct fileheader header;
	struct stat state;
	char dirname[MAXLEN];
	char buf[MAXLEN];
	long numents, size, filetime, now;
	int fd, lower, ent;

	if (file == NULL || file[0] != 'M' || file[1] != '.' ||
	    (filetime = atoi(file + 2)) <= 0)
		return;
	size = sizeof (header);
	sprintf(dirname, "%s/boards/%s/.DIR", homepath, board);
	if ((fd = open(dirname, O_RDWR)) == -1)
		return;
	flock(fd, LOCK_EX);
	fstat(fd, &state);
	ent = ((long) state.st_size) / size;
	lower = 0;
	while (1) {
		ent -= 8;
		if (ent <= 0 || lower >= 2)
			break;
		lseek(fd, size * ent, SEEK_SET);
		if (read(fd, &header, size) != size) {
			ent = 0;
			break;
		}
		now = header.filetime;
		lower = (now < filetime) ? lower + 1 : 0;
	}
	if (ent < 0)
		ent = 0;
	while (read(fd, &header, size) == size) {
		if (filetime == header.filetime) {
			sprintf(buf, "-%s", header.owner);
			fh_setowner(&header, buf, 0);
			if (message && *message)
				strsncpy(header.title,
					 "<< E-mail post ÎÄÕÂÉóºËÖÐ... >>",
					 sizeof (header.title));
			else
				strsncpy(header.title, "<< article canceled >>",
					 sizeof (header.title));
			lseek(fd, -size, SEEK_CUR);
			safewrite(fd, &header, size);
			break;
		}
		if (header.filetime > filetime)
			break;
	}
	flock(fd, LOCK_UN);
	close(fd);
}

expire_article(brdname, days_str, maxpost, minpost)
char *brdname, *days_str;
{
	//¾ÝÈÏÎª, Õâ¸öº¯Ë÷Ã»ÓÐÓÃÁË
#if 0
	struct fileheader head;
	struct stat state;
	char lockfile[MAXLEN], index[MAXLEN];
	char tmpfile[MAXLEN], delfile[MAXLEN];
	int days, total;
	int fd, fdr, fdw, done, keep;
	int duetime, ftime;

	days = atoi(days_str);
	if (days < 1) {
		printf(":Err: expire time must more than 1 day.\n");
		return;
	} else if (maxpost < 100) {
		printf(":Err: maxmum posts number must more than 100.\n");
		return;
	}
	sprintf(lockfile, "%s/.dellock", homepath, brdname);
	sprintf(index, "%s/boards/%s/.DIR", homepath, brdname);
	sprintf(tmpfile, "%s/.tmpfile", homepath, brdname);
	sprintf(delfile, "%s/.deleted", homepath, brdname);

	if ((fd = open(lockfile, O_RDWR | O_CREAT | O_APPEND, 0644)) == -1)
		return;
	flock(fd, LOCK_EX);
	unlink(tmpfile);

	duetime = time(NULL) - days * 24 * 60 * 60;
	done = 0;
	if ((fdr = open(index, O_RDONLY, 0)) > 0) {
		fstat(fdr, &state);
		total = state.st_size / sizeof (head);
		if ((fdw = open(tmpfile, O_WRONLY | O_CREAT | O_EXCL, 0644)) >
		    0) {
			while (read(fdr, &head, sizeof head) == sizeof head) {
				done = 1;
				ftime = atoi(head.filename + 2);
				if (head.owner[0] == '-')
					keep = 0;
				else if (head.accessed[0] & FILE_MARKED
					 || total <= minpost)
					keep = 1;
				else if (ftime < duetime || total > maxpost)
					keep = 1;
				else
					keep = 1;
				if (keep) {
					if (safewrite(fdw, &head, sizeof head)
					    == -1) {
						done = 0;
						break;
					}
				} else {
					printf("Unlink %s\n", head.filename);
					if (head.owner[0] == '-')
						printf("Unlink %s.cancel\n",
						       head.filename);
					total--;
				}
			}
			close(fdw);
		}
		close(fdr);
	}
	if (done) {
		unlink(delfile);
		if (rename(index, delfile) != -1) {
			rename(tmpfile, index);
		}
	}
	flock(fd, LOCK_UN);
	close(fd);
#endif
}

main(argc, argv)
char *argv[];
{
	char *progmode;
	int max, min;

	if (argc < 3)
		usage();
	progmode = argv[1];
	homepath = argv[2];
	if (ci_strcmp(progmode, "list") == 0) {
		search_boards(0);
	} else if (ci_strcmp(progmode, "visit") == 0) {
		search_boards(1);
	} else if (ci_strcmp(progmode, "post") == 0) {
		post_article(0);
	} else if (ci_strcmp(progmode, "mail") == 0) {
		post_article(1);
	} else if (ci_strcmp(progmode, "cancel") == 0) {
		if (argc < 5)
			usage();
		if (argc >= 6)
			cancel_article(argv[3], argv[4], argv[5]);
		else
			cancel_article(argv[3], argv[4]);
	} else if (ci_strcmp(progmode, "expire") == 0) {
		if (argc < 5)
			usage();
		max = atoi(argc > 5 ? argv[5] : "9999");
		min = atoi(argc > 6 ? argv[6] : "10");
		expire_article(argv[3], argv[4], max, min);
	}
}

#else
main()
{
	printf("You must use the bbspost in palmbbs source\n");
}

#endif

struct BCACHE *brdshm = NULL;
struct boardmem *bcache = NULL;

int
resolveshm()
{
	int shmid;
	shmid = shmget(BCACHE_SHMKEY, sizeof (*brdshm), 0);
	if (shmid < 0)
		return -1;
	brdshm = (struct BCACHE *) shmat(shmid, NULL, 0);
	if (brdshm == (struct BCACHE *) -1)
		return -1;
	bcache = brdshm->bcache;
	return 0;
}

int
getlastpost(char *board, int *lastpost, int *total)
{
	struct fileheader fh;
	struct stat st;
	char filename[STRLEN * 2];
	int fd, atotal, ftime;

	sprintf(filename, MY_BBS_HOME "/boards/%s/.DIR", board);
	if ((fd = open(filename, O_RDWR)) < 0)
		return -1;
	fstat(fd, &st);
	atotal = st.st_size / sizeof (fh);
	if (total <= 0) {
		*lastpost = 0;
		*total = 0;
		close(fd);
		return 0;
	}
	*total = atotal;
	lseek(fd, (atotal - 1) * sizeof (fh), SEEK_SET);
	if (read(fd, &fh, sizeof (fh)) > 0) {
		if (fh.edittime)
			*lastpost = fh.edittime;
		else
			*lastpost = fh.filetime;
	}
	close(fd);
	return 0;
}

struct boardmem *
getbcache(bname)
char *bname;
{
	int i;
	int numboards;
	if (brdshm == NULL)
		resolveshm();
	numboards = brdshm->number;
	for (i = 0; i < numboards; i++)
		if (!strncasecmp(bname, bcache[i].header.filename, STRLEN))
			return &bcache[i];
	return NULL;
}

int
updatelastpost(char *board)
{
	struct boardmem *bptr;
	bptr = getbcache(board);
	if (bptr == NULL)
		return -1;
	return getlastpost(bptr->header.filename, &bptr->lastpost, &bptr->total);
}

int
tailbr(char *str)
{
	int len;
	len = strlen(str);
	while (len && (str[len - 1] == '\r' || str[len - 1] == '\n'))
		str[--len] = 0;
}

int
seek_in_file(filename, seekstr)
char filename[STRLEN], seekstr[STRLEN];
{

	FILE *fp;
	char buf[STRLEN];
	char *namep;
	if ((fp = fopen(filename, "r")) == NULL)
		return 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		namep = (char *) strtok(buf, ": \n\r\t");
		if (namep != NULL && strcasecmp(namep, seekstr) == 0) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

int
deny_me(char *path, char *userid)
{
	char buf[STRLEN];
	sprintf(buf, "%s/deny_users", path);
	return seek_in_file(buf, userid);
}

int
club_me(char *path, char *userid)
{
	char buf[STRLEN];
	sprintf(buf, "%s/club_users", path);
	return seek_in_file(buf, userid);
}
