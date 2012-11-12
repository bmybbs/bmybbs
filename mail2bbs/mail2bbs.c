#include "bbs.h"
#define BLOCKFILE ".blockmail"
#define BUFLEN 256

int has_a = 0;

int str_decode(register unsigned char *dst, register unsigned char *src);

struct userec checkuser;

void
chop(char *s)
{
	int i;
	i = strlen(s);
	if (s[i - 1] == '\n')
		s[i - 1] = 0;
	return;
}

int
cmpuids(uid, up)
char *uid;
struct userec *up;
{
	if (!strncasecmp(uid, up->userid, sizeof (up->userid))) {
		strncpy(uid, up->userid, sizeof (up->userid));
		return 1;
	} else {
		return 0;
	}
}

int
dosearchuser(userid)
char *userid;
{
	return search_record(PASSFILE, &checkuser, sizeof (currentuser),
			     cmpuids, userid);
}

void
decode_mail(FILE * fin, FILE * fout)
{
	char filename[BUFLEN];
	char encoding[BUFLEN];
	char buf[BUFLEN];
	char sbuf[BUFLEN + 20], dbuf[BUFLEN + 20];
	char ch;
	long sizep, wc;
	while (fgets(filename, sizeof (filename), fin)) {
		if (!fgets(encoding, sizeof (encoding), fin))
			return;
		chop(filename);
		chop(encoding);
		ch = 0;
		sizep = 0;
		if (filename[0]) {
			has_a = 1;
			str_decode(buf, filename);
			strsncpy(filename, buf, sizeof (filename));
			printf("f:%s\n", filename);
			fprintf(fout, "\nbeginbinaryattach %s\n", filename);
			fwrite(&ch, 1, 1, fout);
			sizep = ftell(fout);
			fwrite(&sizep, sizeof (sizep), 1, fout);
		}
		if (!strcmp(encoding, "quoted-printable")
		    || !strcmp(encoding, "base64")) {
			ch = encoding[0];
			sprintf(sbuf, "=??%c?", ch);
			while (fgets(buf, sizeof (buf), fin)) {
				int hasr = 0;
				if (!buf[0] && buf[1] == '\n')
					break;
				strsncpy(sbuf + 5, buf, sizeof (buf));	//sbuf must be larger than buf
				chop(sbuf);
				if (ch == 'q') {
					if (sbuf[strlen(sbuf) - 1] == '=')
						sbuf[strlen(sbuf) - 1] = 0;
					else
						hasr = 1;
				}
				strcat(sbuf, "?=");
				wc = str_decode(dbuf, sbuf);
				if (wc >= 0) {
					fwrite(dbuf, wc, 1, fout);
					if (hasr)
						fputc('\n', fout);
				} else
					fputs(buf, fout);
			}
		} else {
			while (fgets(buf, sizeof (buf), fin)) {
				if (!buf[0] && buf[1] == '\n')
					break;
				fputs(buf, fout);
			}
		}
		if (sizep) {
			sizep = ftell(fout) - sizep - sizeof (long);
			fseek(fout, -sizep - 4, SEEK_CUR);
			sizep = htonl(sizep);
			fwrite(&sizep, sizeof (sizep), 1, fout);
			sizep = ntohl(sizep);
			fseek(fout, sizep, SEEK_CUR);
		}
	}
	if (sizep)
		fputs("\n\n--\n", fout);
}

int
append_mail(fin, sender1, sender, userid, title, received)
FILE *fin;
char *userid, *sender1, *sender, *title, *received;
{
	struct fileheader newmessage;
	char buf[BUFLEN], genbuf[BUFLEN];
	char maildir[BUFLEN];
	struct stat st;
	int filetime;
	FILE *fout, *dp, *rmail;
	int passcheck = 0;
	char conv_buf[BUFLEN], *p1, *p2;

/* check if the userid is in our bbs now */
	if (!dosearchuser(userid))
		return -1;
	if (!(checkuser.userdefine & DEF_INTERNETMAIL))
		return -1;
	if (!strcasecmp(userid, "guest"))
		return -1;

/* check for the mail dir for the userid */
	snprintf(genbuf, sizeof (genbuf), "mail/%c/%s", mytoupper(userid[0]),
		 userid);

	if (stat(genbuf, &st) == -1) {
		if (mkdir(genbuf, 0755) == -1)
			return -1;
	} else {
		if (!(st.st_mode & S_IFDIR))
			return -1;
	}

	printf("Ok, dir is %s\n", genbuf);

	str_decode(conv_buf, sender);
	strsncpy(sender, conv_buf, BUFLEN);
	str_decode(conv_buf, title);

	while (!passcheck && !strcmp(userid, "SYSOP")
	       && strstr(conv_buf, " mail check.")) {
		passcheck = 1;
		if ((!strstr(sender, "bbs")) && (strchr(conv_buf, '@'))) {

			p1 = strchr(conv_buf, '@');
			if (!p1)
				break;
			p2 = strchr(p1 + 1, '@');
			if (!p2)
				break;
			*p2 = 0;
			strsncpy(sender1, p1 + 1, IDLEN + 1);
			strcpy(userid, sender1);
			snprintf(genbuf, sizeof (genbuf),
				 "home/%c/%s/mailcheck", mytoupper(sender1[0]),
				 sender1);
			if ((dp = fopen(genbuf, "r")) != NULL) {
				printf("open mailcheck\n");
				fgets(buf, sizeof (buf), dp);
				fclose(dp);
				sprintf(buf, "%9.9s", buf);
				if (dosearchuser(sender1)
				    && strstr(conv_buf, buf)
				    /*&&strstr(sender,checkuser.email) */
				    ) {
					printf("pass1\n");

					unlink(genbuf);
					passcheck = 5;
					/*Modify for SmallPig */
					snprintf(genbuf, sizeof (genbuf),
						 "home/%c/%s/register",
						 mytoupper(sender1[0]),
						 sender1);
					if (file_isfile(genbuf)) {
						snprintf(buf, sizeof (buf),
							 "home/%c/%s/register.old",
							 mytoupper(sender1[0]),
							 sender1);
						rename(genbuf, buf);
					}
					if ((fout = fopen(genbuf, "w")) != NULL) {
						fprintf(fout, "%s\n", sender);
						fclose(fout);
					}
				}
			}
		}
	}

/* allocate a record for the new mail */
	bzero(&newmessage, sizeof (newmessage));
	snprintf(maildir, sizeof(maildir), "mail/%c/%s", mytoupper(userid[0]), userid);
	if (!file_isdir(maildir)) {
		mkdir(maildir, 0755);
		chmod(maildir, 0755);
	}
	filetime = trycreatefile(maildir, "M.%d.A", time(NULL), 1000);
	if (filetime < 0)
		return -1;
	newmessage.filetime = filetime;
	newmessage.thread = filetime;
	strsncpy(newmessage.title, conv_buf, sizeof (newmessage.title));
	fh_setowner(&newmessage, sender, 0);

	printf("Ok, the file is %s\n", maildir);

/* copy the stdin to the specified file */
	if ((fout = fopen(maildir, "w")) == NULL) {
		printf("Cannot open %s \n", maildir);
		return -1;
	} else {
		time_t tmp_time;
		time(&tmp_time);
		fprintf(fout, "寄信人: %-.70s \n", sender);
		fprintf(fout, "标  题: %-.70s\n", conv_buf);
		fprintf(fout, "发信站: %s BBS 信差\n", MY_BBS_NAME);
		if (received[0] != '\0')
			fprintf(fout, "来  源: %-.70s\n", received);
		fprintf(fout, "日  期: %s\n", ctime(&tmp_time));
		if (passcheck >= 1) {
			fprintf(fout, "亲爱的 %s:\n", sender1);
			sprintf(maildir, "etc/%s",
				(passcheck == 5) ? "smail" : "fmail");
			if ((rmail = fopen(maildir, "r")) != NULL) {
				while (fgets(genbuf, sizeof (genbuf), rmail) !=
				       NULL)
					fputs(genbuf, fout);
				fclose(rmail);
			}
		} else
			decode_mail(fin, fout);
		fclose(fout);

	}

/* append the record to the MAIL control file */
	if (has_a)
		newmessage.accessed |= FH_ATTACHED;
	sprintf(genbuf, "mail/%c/%s/%s", mytoupper(userid[0]), userid, DOT_DIR);
	if (append_record(genbuf, &newmessage, sizeof (newmessage)) == -1)
		return 1;
	else
		return 0;
}

int
block_mail(addr)
char *addr;
{
	FILE *fp;
	char temp[STRLEN];

	if ((fp = fopen(BLOCKFILE, "r")) != NULL) {
		while (fgets(temp, STRLEN, fp) != NULL) {
			strtok(temp, "\n");
			if (strstr(addr, temp)) {
				fclose(fp);
				return 1;
			}
		}
		fclose(fp);
	}
	return 0;
}

int
main(argc, argv)
int argc;
char *argv[];
{

	char myarg[4][BUFLEN];
	char nettyp[BUFLEN];
	char *p;
	int i;

	for (i = 0; i < 4; i++)
		if (!fgets(myarg[i], sizeof (myarg[i]), stdin))
			return i + 1;
	for (i = 0; i < 4; i++)
		chop(myarg[i]);

	chdir(MY_BBS_HOME);
	setreuid(BBSUID, BBSUID);
	setregid(BBSGID, BBSGID);
	strsncpy(nettyp, myarg[0], sizeof (nettyp));
	p = strchr(nettyp, '@');
	if (NULL != p)
		*p = 0;

	if (block_mail(myarg[0]) == YEA)
		return -2;
	return append_mail(stdin, nettyp, myarg[0], myarg[1], myarg[2],
			   myarg[3]);
}
