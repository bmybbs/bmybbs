//by ecnegrevid 2001.9.29

#include "../include/bbs.h"
#define PASSWDFILE MY_BBS_HOME"/.PASSWDS"

int
compute_user_value(struct userec *urec)
{
	int value;

	/* if (urec) has XEMPT permission, don't kick it */
	if ((urec->userlevel & PERM_XEMPT)
	    || strcmp(urec->userid, "guest") == 0)
		return 999;
	value = (time(0) - urec->lastlogin) / 60;	/* min */
	/* new user should register in 30 mins */
	if (strcmp(urec->userid, "new") == 0)
		return (30 - value) * 60;
	if (urec->numlogins <= 3)
		return (15 * 1440 - value) / 1440;
	if (!(urec->userlevel & PERM_LOGINOK))
		return (30 * 1440 - value) / 1440;
	return (120 * 1440 - value) / 1440;
}

int
sendreminder(struct userec *urec)
{
	char cmd[1024], *ptr;
	time_t t;
	FILE *fp;
	if (strchr(urec->email, '@') == NULL)
		return;
	ptr = urec->email;
	while (*ptr) {
		if (!isalnum(*ptr) && !strchr(".@", *ptr))
			return;
		ptr++;
	}
	if (strcasestr(urec->email, ".bbs@ytht.") != NULL)
		return;
	sprintf(cmd, "mail -s '系统提醒（" MY_BBS_NAME "）' %s", urec->email);
	fp = popen(cmd, "w");
	if (fp == NULL)
		return;
	fprintf(fp,
		"    " MY_BBS_NAME "(" MY_BBS_DOMAIN
		")的用户您好，您在本站注册的\n"
		"帐号 %s 现在生命力已经降低到 10，需要您用\n"
		"该帐号登陆一次才能使生命力恢复。如果该帐户并不是\n"
		"您注册的，忽略这封信就可以了。\n\n" "关于生命力的说明:\n"
		"    在BBS系统上，每个帐号都有一个生命力，在用户不\n"
		"登录的情况下，生命力每天减少1，等生命力减少到0的时\n"
		"候，帐号就会自动消失。帐号每次登录后生命力就恢复到\n"
		"一个固定值，对于通过注册而且已经登录4次的用户，这个\n"
		"固定值至少是120；对于通过注册但登录少于4次的用户，\n"
		"这个固定值是30；对于未通过注册的用户，这个固定值是15。\n",
		urec->userid);
	pclose(fp);
	if ((fp = fopen(MY_BBS_HOME "/reminder.log", "a")) != NULL) {
		t = time(NULL);
		ptr = ctime(&t);
		ptr[strlen(ptr) - 1] = 0;
		fprintf(fp, "%s %s %s\n", ptr, urec->userid, urec->email);
		fclose(fp);
	}
	sleep(5);
}

main(int argc, char *argv[])
{
	int fd1;
	struct userec rec;
	char buf[100];
	int size1 = sizeof (rec);

	if ((fd1 = open(PASSWDFILE, O_RDONLY, 0660)) == -1) {
		perror("open PASSWDFILE");
		return -1;
	}

	while (read(fd1, &rec, size1) == size1) {
		if (!rec.userid[0])
			continue;
		if (compute_user_value(&rec) == 10)
			//printf("%s  \t%s\n", rec.userid, rec.email);
			sendreminder(&rec);
	}
	close(fd1);
}
