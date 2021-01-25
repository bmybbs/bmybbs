#include "bbs.h"
#define numboards 296
time_t now;
int
mail_file(char *filename, char *userid, char *title, char *sender)
{
	FILE *fp, *fp2;
	char buf[256], dir[256];
	struct fileheader header;
	time_t t;
	bzero(&header, sizeof (header));
	fh_setowner(&header, sender, 0);
	sprintf(buf, "mail/%c/%s/", mytoupper(userid[0]), userid);
	mkdir(buf, 0770);
	t = trycreatefile(buf, "M.%ld.A", now, 100);
	if (t < 0)
		return -1;
	header.filetime = t;
	ytht_strsncpy(header.title, title, sizeof (header.title));
	fp = fopen(buf, "w");
	if (fp == 0)
		return -2;
	fp2 = fopen(filename, "r");
	if (fp2) {
		while (1) {
			int retv;
			retv = fread(buf, 1, sizeof (buf), fp2);
			if (retv <= 0)
				break;
			fwrite(buf, 1, retv, fp);
		}
		fclose(fp2);
	}
	fclose(fp);
	setmailfile_s(dir, sizeof(dir), userid, ".DIR");
	append_record(dir, &header, sizeof (header));
	return 0;
}

int
mailmsg(char *userid)
{
	char file[200];
	char title[50];
	struct stat st;
	sprintf(file, "home/%c/%s/msgfile", mytoupper(userid[0]), userid);
	sprintf(title, "[May  9 22:00] 所有讯息备份");
	if (stat(file, &st) != -1) {
		printf("mail %s\n", userid);
		mail_file(file, userid, title, userid);
	}
	return 0;
}

int
_mailmsg(void *p, void *_) {
	struct userec *user = p;
	return mailmsg(user->userid);
}

int main()
{
	chdir(MY_BBS_HOME);
	now = time(NULL);
	new_apply_record(".PASSWDS", sizeof (struct userec), _mailmsg, NULL);
	return 0;
}
