/*    自动解封系统    KCN 1999.7.26 		  */

#include "bbs.h"

#include "article.h"
/* 以下这段发送邮件的函数，参考src/mail.c中的那个mail_buf函数做成的。interma@BMY 2005.4.24 */

extern void securityreport(char * owner, char * str, char * title);
extern void deliverreport(char * board, char * title, char * str);

static int
mail_buf(buf, userid, title)
char *buf, userid[], title[];
{
	struct fileheader newmessage;
	struct stat st;
	char fname[STRLEN], filepath[STRLEN];
	int count, tmpinmail, fd;
	time_t now;
	FILE *fp;

	memset(&newmessage, 0, sizeof (newmessage));
	strsncpy(newmessage.owner, "XJTU-XANET",
		 sizeof (newmessage.owner));
	strsncpy(newmessage.title, title, sizeof (newmessage.title));
	//strsncpy(save_title, newmessage.title, sizeof (save_title));

	setmailfile(filepath, userid, "");
	if (stat(filepath, &st) == -1) {
		if (mkdir(filepath, 0775) == -1)
			return -1;
	} else {
		if (!(st.st_mode & S_IFDIR))
			return -1;
	}
	now = time(NULL);
	sprintf(fname, "M.%lu.A", now);
	setmailfile(filepath, userid, fname);
	count = 0;
	while ((fd = open(filepath, O_CREAT | O_EXCL | O_WRONLY, 0644)) == -1) {
		now++;
		sprintf(fname, "M.%lu.A", now);
		setmailfile(filepath, userid, fname);
		if (count++ > MAX_POSTRETRY) {
			return -1;
		}
	}
	close(fd);
	newmessage.filetime = now;
	newmessage.thread = now;
	fp = fopen(filepath, "w");
	if (!fp)
		return -1;
	//tmpinmail = in_mail;
	//in_mail = YEA;
	//write_header(fp, 1);
	fprintf(fp, "寄信人: XJTU-XANET \n");
	fprintf(fp, "标  题: %s\n", title);
	fprintf(fp, "发信站: %s (%24.24s), %s", MY_BBS_NAME, ctime(&now),
		"本站(" MY_BBS_DOMAIN ")");
	fprintf(fp, "\n来  源: 兵马俑BBS\n\n");
	//in_mail = tmpinmail;
	fprintf(fp, "%s", buf);
	fclose(fp);

	char genbuf[STRLEN];
	setmailfile(genbuf, userid, DOT_DIR);
	if (append_record(genbuf, &newmessage, sizeof (newmessage)) == -1)
		return -1;
	sprintf(genbuf, "%s mail %s", "XJTU-XANET", userid);
	newtrace(genbuf);
	return 0;
}

/*void report(str)
char *str;
{
//      printf("%s\n",str);
}*/

unsigned long
atoul(p)
char *p;
{
	unsigned long s;
	char *t;
	t = p;
	s = 0;
	while ((*t >= '0') && (*t <= '9')) {
		s = s * 10 + *t - '0';
		t++;
	}
	return s;
}

void
showundenymessage(linebuf, currboard, anony)
char *linebuf;
char *currboard;
int anony;
{
	char msgbuf[256];
	char repbuf[256];
	char uident[14];
	int i;
	strncpy(uident, linebuf, 12);
	uident[12] = 0;
	for (i = 0; i < 12; i++)
		if (uident[i] == ' ') {
			uident[i] = 0;
			break;
		}
	sprintf(repbuf, "恢复 %s 在 %s 的 POST 权利",
		anony ? "Anonymous" : uident, currboard ? currboard : "全站");
	sprintf(msgbuf, "解封原因: 封禁时间已到，请理解%s务管理工作，谢谢!",
        currboard ? "版" : "站");
	//sprintf(msgbuf, "解封原因: 封人时间已到，请理解版务管理工作,谢谢!");
	securityreport("XJTU-XANET", msgbuf, repbuf);
	deliverreport(currboard ? currboard : "sysop", repbuf, msgbuf);
	
	/* 发封信到被封人的信箱 interma@BMY 2005.4.24 */
	sprintf(repbuf,
			"恢复您在 %s 的POST权限！",
			currboard ? currboard : "全站");
	snprintf(msgbuf, 256, "封禁时间已到，因此%s\n请理解版务管理工作,谢谢!\n", repbuf);
	mail_buf(msgbuf, uident, repbuf);
	
}

int
canundeny(linebuf, nowtime)
char *linebuf;
unsigned long nowtime;
{
	char *p;
	unsigned long time2;

	p = linebuf;
	while ((*p != 0) && (*p != 0x1b))
		p++;
	if (*p == 0)
		return 0;
	p++;
	if (*p == 0)
		return 0;
	p++;
	if (*p == 0)
		return 0;
	time2 = atoul(p);
	return nowtime > time2;
}

int
sgetline(buf, linebuf, idx, maxlen)
char *buf, *linebuf;
int *idx;
int maxlen;
{
	int len = 0;
	while (len < maxlen) {
		char ch;
		linebuf[len] = buf[*idx];
		ch = buf[*idx];
		(*idx)++;
		if (ch == 0x0d) {
			linebuf[len] = 0;
			if (buf[*idx] == 0x0a)
				(*idx)++;
			break;
		}
		if (ch == 0x0a) {
			linebuf[len] = 0;
			break;
		}
		if (ch == 0)
			break;
		len++;
	}
	return len;
}

int
main()
{
	int b_fd, d_fd;
	struct boardheader bh;
	char denyfile[256];
	char *buf;
	int bufsize;
	int size;
	int anony;
	struct stat st;
	time_t nowtime;
	int idx1, idx2;
	char linebuf[256];

	size = sizeof (bh);

	chdir(MY_BBS_HOME);
	bufsize = 0;
//      sprintf(board_file,"%s/.BOARDS",MY_BBS_HOME);
	nowtime = time(NULL);
	printf("bbs home=%s now time = %lu\n", MY_BBS_HOME, nowtime);
	if ((b_fd = open(".BOARDS", O_RDONLY)) == -1)
		return -1;
	flock(b_fd, LOCK_EX);
	buf = NULL;
	sprintf(denyfile, "deny_users");
	if (stat(denyfile, &st) == 0 && st.st_size != 0) {
		if (bufsize < st.st_size + 1) {
			if (buf)
				free(buf);
			buf = malloc(st.st_size + 1);
			buf[st.st_size] = 0;
			bufsize = st.st_size + 1;
		}
		if ((d_fd = open(denyfile, O_RDWR)) != -1) {
			flock(d_fd, LOCK_EX);
			if (read(d_fd, buf, st.st_size) == st.st_size) {
				idx1 = 0;
				idx2 = 0;
				while (idx2 < st.st_size) {
					int len =
					    sgetline(buf, linebuf, &idx2, 255);
					puts(linebuf);
					if (!canundeny(linebuf, nowtime)) {
						if (idx1 != 0) {
							buf[idx1] = 0x0a;
							idx1++;
						}
						memcpy(buf + idx1, linebuf,
						       len);
						idx1 += len;
					} else {
						showundenymessage(linebuf,
								  NULL, 0);
					}
				}
				buf[idx1] = 0x0a;
				idx1++;
				lseek(d_fd, 0, SEEK_SET);
				write(d_fd, buf, idx1);
				ftruncate(d_fd, idx1);
			}
			flock(d_fd, LOCK_UN);
			close(d_fd);
		}
	}

	for (anony = 0; anony <= 1; anony++) {
		while (read(b_fd, &bh, size) == size) {
			if (bh.filename[0] == 0)
				continue;
//                      if (strcmp(bh.filename,"test")) continue;
			if (anony)
				sprintf(denyfile, "boards/%s/deny_anony",
					bh.filename);
			else
				sprintf(denyfile, "boards/%s/deny_users",
					bh.filename);
			if (stat(denyfile, &st) == -1)
				continue;
			if (st.st_size == 0)
				continue;
			//printf("process %s ...\n",bh.filename);
			if (bufsize < st.st_size + 1) {
				if (buf)
					free(buf);
				buf = malloc(st.st_size + 1);
				buf[st.st_size] = 0;
				bufsize = st.st_size + 1;
			}
			if ((d_fd = open(denyfile, O_RDWR)) == -1)
				continue;
			flock(d_fd, LOCK_EX);
			if (read(d_fd, buf, st.st_size) == st.st_size) {
				idx1 = 0;
				idx2 = 0;
				while (idx2 < st.st_size) {
					int len =
					    sgetline(buf, linebuf, &idx2, 255);
					puts(linebuf);
					if (!canundeny(linebuf, nowtime)) {
						if (idx1 != 0) {
							buf[idx1] = 0x0a;
							idx1++;
						}
						memcpy(buf + idx1, linebuf,
						       len);
						idx1 += len;
					} else {
						showundenymessage(linebuf,
								  bh.filename,
								  anony);
					}
				}
				buf[idx1] = 0x0a;
				idx1++;
				lseek(d_fd, 0, SEEK_SET);
				write(d_fd, buf, idx1);
				ftruncate(d_fd, idx1);
			}
			flock(d_fd, LOCK_UN);
			close(d_fd);
		}
	}
	if (buf)
		free(buf);
	flock(b_fd, LOCK_UN);
	close(b_fd);

	return 0;
}
