/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu
    Firebird Bulletin Board System
    Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
                        Peng Piaw Foong, ppfoong@csie.ncu.edu.tw
    Copyright (C) 1999, KCN,Zhou Lin, kcn@cic.tsinghua.edu.cn

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include "bbs.h"
#include "bbstelnet.h"
#include "identify.h"
/*
#define  EMAIL          0x0001
#define  NICK           0x0002
#define  REALNAME       0x0004
#define  ADDR           0x0008
#define  REALEMAIL      0x0010
#define  BADEMAIL       0x0020
#define  NEWREG         0x0040
*/
char *sysconf_str();
char *genpasswd();

extern char fromhost[60];
extern time_t login_start_time;
extern int convcode;

time_t system_time;

static int valid_ident(char *ident);

char * str_to_upper(char *str);

/*
·µ»ØÖµµÄËµÃ÷ÈçÏÂ:
0	ÒÑ¾­³É¹¦É¾³ı´ËÌõ¼ÇÂ¼
-1	ÓÊ¼ş¸ñÊ½ÓĞÎó£¬É¾³ıÊ§°Ü
-2	ÕÒ²»µ½pop_listÎÄ¼ş£¬É¾³ıÊ§°Ü
-3	Ã»ÓĞÕÒµ½Õâ¸öÓÊ¼ş·şÎñÆ÷£¬É¾³ıÊ§°Ü
-4	´ò¿ªÁÙÊ±ÎÄ¼şÊ§°Ü£¬É¾³ıÊ§°Ü
*/
int
release_email(char *userid, char *email) //ÊÍ·ÅÓÊÏä, added by interma 2006.2.21
{
    struct userec* cuser;
    char an[2];
    char genbuf[STRLEN];
    struct active_data act_data;

    //getuser(userid, &cuser);
    read_active(userid, &act_data);


        act_data.status=NO_ACTIVE;
       // strcpy(act_data.operator, currentuser->userid);
        write_active(&act_data);


    return 0;

/*
    char username[50];
    char popserver[50];

    char *p = strchr(email, '@');
    if (p == NULL)
        return -1;

    memset(username, '\0', sizeof(username));
    memset(popserver, '\0', sizeof(popserver));
    strncpy(username, email, p - email);
    strncpy(popserver, p + 1, strlen(email) - 1 - (p - email));

    //printf("[%s][%s]", username, popserver);

    FILE *fp;
    char buf[256];
    int isexist = 0;

	fp = fopen(MY_BBS_HOME "/etc/pop_register/pop_list", "r");
    if (fp == NULL)
        return -2;
    while(fgets(buf, 256, fp) != NULL)
	{
        if (strcmp(buf, "") == 0 || strcmp(buf, " ") == 0 || strcmp(buf, "\n") == 0)
			break;

        buf[strlen(buf) - 1] = '\0';
        if (strcmp(buf, popserver) == 0)
        {
            isexist = 1;
            break;
        }

        fgets(buf, 256, fp);
    }
    fclose(fp);

    if (!isexist)
        return -3;

	strncpy(buf, MY_BBS_HOME "/etc/pop_register/", 256);
	strncat(buf, popserver, 256);
   	fp = fopen(buf, "r");
	strncpy(buf, MY_BBS_HOME "/etc/pop_register/", 256);
   	strncat(buf, popserver, 256);
   	strncat(buf, "_temp", 5);

	int lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX); // ¼ÓËøÀ´±£Ö¤»¥³â²Ù×÷
	FILE *fp2 = fopen(buf, "w");
    if (fp == NULL || fp2 == NULL)
	{
		close(lockfd);
		return -4;
    }
    char username2[50];
    char userid2[20];

    while(fgets(buf, 256, fp) != NULL)
	{
	    strncpy(username2, buf, 50);
	    username2[strlen(username2) - 1] = '\0';
	    fgets(buf, 256, fp);
	    strncpy(userid2, buf, 20);
	    p = strchr(userid2, ' ');
	    userid2[p - userid2] = '\0';

	    //printf("[%s][%s]\n", userid2, username2);
	    if (strcmp(str_to_upper(userid), str_to_upper(userid2)) == 0 &&
            strcmp(str_to_upper(username), str_to_upper(username2)) == 0)
	    {
	        ;
	    }
        else
        {
            fputs(username2, fp2);
            fputs("\n", fp2);
            fputs(buf, fp2);
        }
	}

    fclose(fp);
    fclose(fp2);

 	char buf2[256];
	strncpy(buf2, MY_BBS_HOME "/etc/pop_register/", 256);
	strncat(buf2, popserver, 256);

	strncpy(buf, MY_BBS_HOME "/etc/pop_register/", 256);
   	strncat(buf, popserver, 256);
   	strncat(buf, "_temp", 5);
	rename(buf, buf2);
    close(lockfd);
    return 0;
    */
}


static int
getnewuserid(struct userec *newuser)
{
	struct userec utmp, zerorec;
	struct stat st;
	int fd, size, val, i;

	system_time = time(NULL);
	if (stat("tmp/killuser", &st) == -1 || st.st_mtime < system_time - 3600) {
		if ((fd = open("tmp/killuser", O_RDWR | O_CREAT, 0600)) == -1)
			return -1;
		write(fd, ctime(&system_time), 25);
		close(fd);
		prints("Ñ°ÕÒĞÂÕÊºÅÖĞ, ÇëÉÔ´ıÆ¬¿Ì...\n\r");
		refresh();
		memset(&zerorec, 0, sizeof (zerorec));
		if ((fd = open(PASSFILE, O_RDWR | O_CREAT, 0600)) == -1)
			return -1;
		flock(fd, LOCK_EX);
		size = sizeof (utmp);
		char userid[50];
		char email[50];
		char buf[512];
		for (i = 0; i < MAXUSERS; i++) {
			if (read(fd, &utmp, size) != size)
				break;
			val = countlife(&utmp);
			if (utmp.userid[0] != '\0' && val < 0) {
				sprintf(genbuf, "system kill %s %d",
					utmp.userid, val);
				newtrace(genbuf);
				if (!is_bad_id(utmp.userid)) {
					if ((utmp.userlevel & PERM_OBOARDS))
						retire_allBM(utmp.userid);
					sprintf(genbuf,
						"mail/%c/%s",
						mytoupper(utmp.userid[0]),
						utmp.userid);
					deltree(genbuf);
					sprintf(genbuf,
						"home/%c/%s",
						mytoupper(utmp.userid[0]),
						utmp.userid);
					deltree(genbuf);

				}
				lseek(fd, -size, SEEK_CUR);
				write(fd, &zerorec, sizeof (utmp));

				strncpy(userid, utmp.userid, 50);
				strncpy(email, utmp.email, 50);
				release_email(userid, email); //id¶öËÀÁËÖ®ºó×Ô¶¯ÊÍ·ÅÓÊÏä£¬added by interma 2006.2.21
				// ¸ø´ËÓÃ»§·¢ĞÅ,ÌáĞÑÓÊÏäÒÑ¾­ÊÍ·Å.
				sprintf(buf, MY_BBS_HOME "/bin/sendmail.py '%s' '%s@bmyÒÑ¾­ËÀÍö' '%sÒÑ¾­ËÀÍö£¬ÓÊÏä°ó¶¨ÒÑ¾­½â³ı£¬ÇëÖØĞÂÓÃ´ËÓÊÏä×¢²áid¡£'",
					email, userid, userid);
				int ret = system(buf);

			}
		}
		flock(fd, LOCK_UN);
		close(fd);
		touchnew();
	}
	if ((fd = open(PASSFILE, O_RDWR | O_CREAT, 0600)) == -1)
		return -1;
	flock(fd, LOCK_EX);

	i = searchnewuser();
	if (i <= 0 || i > MAXUSERS) {
		flock(fd, LOCK_UN);
		close(fd);
		if (dashf("etc/user_full")) {
			ansimore("etc/user_full", NA);
		} else {
			prints
			    ("±§Ç¸, Ê¹ÓÃÕßÕÊºÅÒÑ¾­ÂúÁË, ÎŞ·¨×¢²áĞÂµÄÕÊºÅ.\n\r");
		}
		val = (st.st_mtime - system_time + 3660) / 60 + 1;
		prints("ÇëµÈ´ı %d ·ÖÖÓºóÔÙÊÔÒ»´Î, ×£ÄãºÃÔË.\n\r", val);
		refresh();
		exit(1);
	}
	if (lseek(fd, sizeof (*newuser) * (i - 1), SEEK_SET) == -1) {
		flock(fd, LOCK_UN);
		close(fd);
		return -1;
	}
	write(fd, newuser, sizeof (*newuser));
	strsncpy(uidshm->userid[i - 1], newuser->userid,
		 sizeof (uidshm->userid[i - 1]));
	insertuseridhash(uidhashshm->uhi, UCACHE_HASH_SIZE, newuser->userid, i);
	flock(fd, LOCK_UN);
	close(fd);
	//touchnew();
	return i;
}

void
new_register()
{
	struct userec newuser;
	char passbuf[STRLEN];
	int allocid, try;

	/* unused
	if (0) {
		now_t = time(0);
		sprintf(genbuf, "etc/no_register_%3.3s", ctime(&now_t));
		if (dashf(genbuf)) {
			ansimore(genbuf, NA);
			pressreturn();
			exit(1);
		}
	}
	*/
	memset(&newuser, 0, sizeof (newuser));
	// getdata(0, 0, "Ê¹ÓÃGB±àÂëÔÄ¶Á?(\xa8\xcf\xa5\xce BIG5\xbd\x58\xbe\x5c\xc5\xaa\xbd\xd0\xbf\xefN)(Y/N)? [Y]: ", passbuf, 4, DOECHO, YEA);
	// if (*passbuf == 'n' || *passbuf == 'N')
	//  if (!convcode)
	//          switch_code();

	ansimore("etc/register", NA);
	try = 0;
	while (1) {
		if (++try >= 9) {
			prints("\nêşêş£¬°´Ì«¶àÏÂ  <Enter> ÁË...\n");
			refresh();
			longjmp(byebye, -1);
		}
		move(t_lines - 6, 0);
		prints("ÕÊºÅÃû³ÆÎªÄúÔÚ±¾Õ¾ËùÊµ¼ÊÏÔÊ¾µÄÓÃ»§Ãû³Æ£¬²»¿ÉĞŞ¸Ä£¬ÇëÉ÷ÖØÑ¡Ôñ ");
		getdata(t_lines - 5, 0,
			"ÇëÊäÈëÕÊºÅÃû³Æ (Enter User ID, \"0\" to abort): ",
			newuser.userid, IDLEN + 1, DOECHO, YEA);
		if (newuser.userid[0] == '0') {
			longjmp(byebye, -1);
		}
//		if (!goodgbid(newuser.userid)) {   by bjgyt
//			prints("²»ÕıÈ·µÄÖĞÓ¢ÎÄÕÊºÅ\n");
		if (id_with_num(newuser.userid)) {
			prints("ÕÊºÅ±ØĞëÈ«ÎªÓ¢ÎÄ×ÖÄ¸!\n");
		} else if (strlen(newuser.userid) < 2) {
			prints("ÕÊºÅÖÁÉÙĞèÓĞÁ½¸öÓ¢ÎÄ×ÖÄ¸!\n");
		} else if ((*newuser.userid == '\0') || is_bad_id(newuser.userid)) {
			prints("±§Ç¸, Äú²»ÄÜÊ¹ÓÃÕâ¸ö×Ö×÷ÎªÕÊºÅ¡£ ÇëÏë¹ıÁíÍâÒ»¸ö¡£\n");
		} else if (dosearchuser(newuser.userid)) {
			prints("´ËÕÊºÅÒÑ¾­ÓĞÈËÊ¹ÓÃ\n");
		} else
			break;
	}
	while (1) {
		getdata(t_lines - 4, 0, "ÇëÉè¶¨ÄúµÄÃÜÂë (Setup Password): ",
			passbuf, PASSLEN, NOECHO, YEA);
		if (strlen(passbuf) < 4 || !strcmp(passbuf, newuser.userid)) {
			prints("ÃÜÂëÌ«¶Ì»òÓëÊ¹ÓÃÕß´úºÅÏàÍ¬, ÇëÖØĞÂÊäÈë\n");
			continue;
		}
		strncpy(newuser.passwd, passbuf, PASSLEN);
		getdata(t_lines - 3, 0,
			"ÇëÔÙÊäÈëÒ»´ÎÄãµÄÃÜÂë (Reconfirm Password): ", passbuf,
			PASSLEN, NOECHO, YEA);
		if (strncmp(passbuf, newuser.passwd, PASSLEN) != 0) {
			prints("ÃÜÂëÊäÈë´íÎó, ÇëÖØĞÂÊäÈëÃÜÂë.\n");
			continue;
		}
		passbuf[8] = '\0';
		strncpy(newuser.passwd, genpasswd(passbuf), PASSLEN);
		break;
	}
	strcpy(newuser.ip, "");
	newuser.userdefine = -1;
	if (!strcmp(newuser.userid, "guest")) {
		newuser.userlevel = 0;
		newuser.userdefine &= ~(DEF_FRIENDCALL | DEF_ALLMSG | DEF_FRIENDMSG);
	} else {
		newuser.userlevel = PERM_BASIC;
		newuser.flags[0] = PAGER_FLAG | BRDSORT_FLAG2;
	}
	newuser.userdefine &= ~(DEF_MAILMSG | DEF_NOLOGINSEND);
	if (convcode)
		newuser.userdefine &= ~DEF_USEGB;

	newuser.flags[1] = 0;
	newuser.firstlogin = newuser.lastlogin = time(NULL);
	newuser.lastlogout = 0;
	allocid = getnewuserid(&newuser);
	if (allocid > MAXUSERS || allocid <= 0) {
		prints("No space for new users on the system!\n\r");
		refresh();
		exit(1);
	}
	setuserid(allocid, newuser.userid);
	if (!dosearchuser(newuser.userid)) {
		prints("User failed to create\n");
		refresh();
		exit(1);
	}
	sethomepath(genbuf, newuser.userid);
	mkdir(genbuf, 0775);
	sprintf(genbuf, "%s newaccount %d %s", newuser.userid, allocid, fromhost);
	newtrace(genbuf);
}

int
invalid_email(char *addr)
{
	FILE *fp;
	char temp[STRLEN];

	if ((fp = fopen(".bad_email", "r")) != NULL) {
		while (fgets(temp, STRLEN, fp) != NULL) {
			strtok(temp, "\n");
			if (strstr(addr, temp) != NULL) {
				fclose(fp);
				return 1;
			}
		}
		fclose(fp);
	}
	return 0;
}

static int
invalid_realmail(char *userid, char *email, int msize)
{
	FILE *fn;
	char fname[STRLEN];
	struct stat st;

	//ÅĞ¶ÏÊÇ·ñÊ¹ÓÃemail×¢²á... ²»¹ıÊÇ²»ÊÇÕæµÄÓÃÕâ¸öÅĞ¶ÏµÄ? --ylsdd
	if (sysconf_str("EMAILFILE") == NULL)
		return 0;

	if (strchr(email, '@') && valid_ident(email) && HAS_PERM(PERM_LOGINOK))
		return 0;

	sethomefile(fname, userid, "register");
	if (stat(fname, &st) == 0) {
#ifdef REG_EXPIRED
		now_t = time(0);
		if (now_t - st.st_mtime >= REG_EXPIRED * 86400) {
			sethomefile(fname, userid, "register.old");
			if (stat(fname, &st) == -1
			    || now_t - st.st_mtime >= REG_EXPIRED * 86400)
				return 1;
		}
#endif
	}
	sethomefile(fname, userid, "register");
	if ((fn = fopen(fname, "r")) != NULL) {
		fgets(genbuf, STRLEN, fn);
		fclose(fn);
		strtok(genbuf, "\n");
		if (valid_ident(genbuf) && ((strchr(genbuf, '@') != NULL)
					    || strstr(genbuf, "usernum"))) {
			if (strchr(genbuf, '@') != NULL)
				strncpy(email, genbuf, msize);
			move(21, 0);

			#ifndef POP_CHECK
			prints("¹§ºØÄú!! ÄúÒÑË³ÀûÍê³É±¾Õ¾µÄÊ¹ÓÃÕß×¢²áÊÖĞø,\n");
			prints("´ÓÏÖÔÚÆğÄú½«ÓµÓĞÒ»°ãÊ¹ÓÃÕßµÄÈ¨ÀûÓëÒåÎñ...\n");
			#endif

			pressanykey();
			return 0;
		}
	}
	return 1;
}

void
check_register_info()
{
	struct userec *urec = &currentuser;
	char *newregfile;
	int perm;
	FILE *fout;
	char buf[192], buf2[STRLEN];

	clear();
	sprintf(buf, "%s", email_domain());
	if (!(urec->userlevel & PERM_BASIC)) {
		urec->userlevel = 0;
		return;
	}
//20060313ĞŞ¸Ä by clearboy(ÍõÏşîÚ)
//½ñÌìÓöµ½ÁËÒ»¸öÂé·³µÄÎÊÌâ,telnetµÇÂ½ºó¾ÍÂíÉÏÍË³öÁË
//´ó¼Ò¸ú×ÙÁËºÜ¾Ã£¬·¢ÏÖÎÊÌâÔÚsysconf_eval
//½øÒ»²½µÄÎÊÌâ£¬ÔÚÓÚ/home/bbs/deverrlogÕâ¸ö´íÎó¼ÇÂ¼ÎÄ¼ş³¬¹ıÁË2åG
//·¢ÏÖsysconf_eval²»ÊÇ±ØĞëµÄ£¬ËùÒÔÉ¾³ıµô¡£
	perm = PERM_DEFAULT;// & sysconf_eval("AUTOSET_PERM");


	while ((strlen(urec->username) < 2)) {
		getdata(2, 0, "ÇëÊäÈëÄúµÄêÇ³Æ (Enter nickname): ",
			urec->username, NAMELEN, DOECHO, YEA);
		strcpy(uinfo.username, urec->username);
		update_utmp();
	}
	while ((strlen(urec->realname) < 4)
	       || (strstr(urec->realname, "  "))
	       || (strstr(urec->realname, "¡¡"))) {
		move(3, 0);
		prints("ÇëÊäÈëÄúµÄÕæÊµĞÕÃû (Enter realname):\n");
		getdata(4, 0, "> ", urec->realname, NAMELEN, DOECHO, YEA);
	}
	while ((strlen(urec->address) < 10) || (strstr(urec->address, "   "))) {
		move(5, 0);
		prints("ÇëÊäÈëÄúµÄÍ¨Ñ¶µØÖ· (Enter home address)£º\n");
		getdata(6, 0, "> ", urec->address, NAMELEN, DOECHO, YEA);
	}
	#ifndef POP_CHECK
	if (strchr(urec->email, '@') == NULL) {
		move(8, 0);
		prints("µç×ÓĞÅÏä¸ñÊ½Îª: [1;37muserid@your.domain.name[m\n");
		prints("ÇëÊäÈëµç×ÓĞÅÏä (²»ÄÜÌá¹©Õß°´ <Enter>)");
		getdata(10, 0, "> ", urec->email, STRLEN - 10, DOECHO, YEA);
		if (strchr(urec->email, '@') == NULL) {
			sprintf(genbuf, "%s.bbs@%s", urec->userid, buf);
			strsncpy(urec->email, genbuf, sizeof (urec->email));
		}
	}
	#endif
	if (!strcmp(currentuser.userid, "SYSOP")) {
		currentuser.userlevel = ~0;
		set_safe_record();
		substitute_record(PASSFILE, &currentuser,
				  sizeof (struct userec), usernum);
	}
	if (!(currentuser.userlevel & PERM_LOGINOK)) {
		if (!invalid_realmail
		    (urec->userid, urec->realmail, STRLEN - 16))
		{
			#ifndef POP_CHECK /* ·ÀÖ¹¼ğ»ØÊ¬Ìåºó£¬²»ÓÃÊäÈëĞÅÏäÃû¡£interma@BMY*/
			sethomefile(buf, urec->userid, "sucessreg");
			if (((dashf(buf)) && !sysconf_str("EMAILFILE"))
			    || (sysconf_str("EMAILFILE"))) {
				set_safe_record();
				urec->userlevel |= PERM_DEFAULT;
				substitute_record(PASSFILE, urec,
						  sizeof (struct userec),
						  usernum);
			}
			#endif
		} else {
#ifdef EMAILREG
			if ((!strstr(urec->email, buf)) &&
			    (!invalidaddr(urec->email)) &&
			    (!invalid_email(urec->email))) {
				move(13, 0);
				prints("ÄúµÄµç×ÓĞÅÏä ÉĞĞëÍ¨¹ı»ØĞÅÑéÖ¤...  \n");
				prints("    ±¾Õ¾½«ÂíÉÏ¼ÄÒ»·âÑéÖ¤ĞÅ¸øÄú,\n");
				prints
				    ("    ÄúÖ»Òª´Ó %s »ØĞÅ, ¾Í¿ÉÒÔ³ÉÎª±¾Õ¾ºÏ¸ñ¹«Ãñ.\n\n",
				     urec->email);
				prints
				    ("    ³ÉÎª±¾Õ¾ºÏ¸ñ¹«Ãñ, ¾ÍÄÜÏíÓĞ¸ü¶àµÄÈ¨Òæà¸!\n");
				move(20, 0);
				if (askyn("ÄúÒªÎÒÃÇÏÖÔÚ¾Í¼ÄÕâÒ»·âĞÅÂğ", YEA, NA)
				    == YEA) {
					randomize();
					code = (time(0) / 2) + (rand() / 10);
					sethomefile(genbuf, urec->userid,
						    "mailcheck");
					if ((dp = fopen(genbuf, "w")) == NULL) {
						fclose(dp);
						return;
					}
					fprintf(dp, "%9.9d\n", code);
					fclose(dp);
					sprintf(buf,
						"/usr/lib/sendmail -f %s.bbs@%s %s ",
						urec->userid, email_domain(),
						urec->email);
					fout = popen(buf, "w");
					fin =
					    fopen(sysconf_str("EMAILFILE"),
						  "r");
					/* begin of sending a mail to user to check email-addr */
					if ((fin != NULL) && (fout != NULL)) {
						fprintf(fout,
							"Reply-To: SYSOP.bbs@%s\n",
							email_domain());
						fprintf(fout,
							"From: SYSOP.bbs@%s\n",
							email_domain());
						fprintf(fout, "To: %s\n",
							urec->email);
						fprintf(fout,
							"Subject: @%s@[-%9.9d-]%s mail check.\n",
							urec->userid, code,
							MY_BBS_ID);
						fprintf(fout,
							"X-Forwarded-By: SYSOP \n");
						fprintf(fout,
							"X-Disclaimer: %s registration mail.\n",
							MY_BBS_ID);
						fprintf(fout, "\n");
						fprintf(fout,
							"BBS LOCATION     : %s (%s)\n",
							email_domain(),
							MY_BBS_IP);
						fprintf(fout,
							"YOUR BBS USER ID : %s\n",
							urec->userid);
						fprintf(fout,
							"APPLICATION DATE : %s",
							ctime
							(&urec->firstlogin));
						fprintf(fout,
							"LOGIN HOST       : %s\n",
							fromhost);
						fprintf(fout,
							"YOUR NICK NAME   : %s\n",
							urec->username);
						fprintf(fout,
							"YOUR NAME        : %s\n",
							urec->realname);
						while (fgets(buf, 255, fin) !=
						       NULL) {
							if (buf[0] == '.'
							    && buf[1] == '\n')
								fputs(". \n",
								      fout);
							else
								fputs(buf,
								      fout);
						}
						fprintf(fout, ".\n");
						fclose(fin);
						fclose(fout);
					}	/* end of sending a mail to user to check email-addr */
					getdata(21, 0,
						"È·ÈÏĞÅÒÑ¼Ä³ö, µÈÄú»ØĞÅÅ¶!! Çë°´ <Enter> : ",
						ans, 2, DOECHO, YEA);
				}
			} else {
				showansi = 1;
				if (sysconf_str("EMAILFILE") != NULL) {
					prints
					    ("\nÄúËùÌîĞ´µÄµç×ÓÓÊ¼şµØÖ· ¡¾[1;33m%s[m¡¿\n",
					     urec->email);
					prints
					    ("²¢·ÇºÏ·¨Ö® UNIX ÕÊºÅ£¬ÏµÍ³²»»áÍ¶µİ×¢²áĞÅ£¬Çëµ½[1;32mInfoEdit->Info[mÖĞĞŞ¸Ä...\n");
					pressanykey();
				}
			}
#endif
		}
	}
	if (urec->lastlogin - urec->firstlogin < 3 * 86400) {
		if (urec->numlogins == 1) {

			clear();
			sprintf(buf, "tmp/newcomer.%s", currentuser.userid);
			if ((fout = fopen(buf, "w")) != NULL) {
				fprintf(fout, "´ó¼ÒºÃ,\n\n");
				fprintf(fout, "ÎÒÊÇ %s (%s), À´×Ô %s\n",
					currentuser.userid, urec->username,
					fromhost);
				fprintf(fout, "½ñÌìÎÒ³õÀ´´ËÕ¾±¨µ½, Çë´ó¼Ò¶à¶àÖ¸½Ì¡£\n");
				move(5, 0);
				prints("Çë×÷¸ö¼ò¶ÌµÄ¸öÈË¼ò½é, Ïò±¾Õ¾ÆäËûÊ¹ÓÃÕß´ò¸öÕĞºô\n");
				prints("(×î¶àÈıĞĞ, Ğ´Íê¿ÉÖ±½Ó°´ <Enter> ÌøÀë)....");
				getdata(7, 0, ":", buf2, 75, DOECHO, YEA);
				if (buf2[0] != '\0') {
					fprintf(fout, "\n\n×ÔÎÒ½éÉÜ:\n\n");
					fprintf(fout, "%s\n", buf2);
					getdata(8, 0, ":", buf2, 75, DOECHO, YEA);
					if (buf2[0] != '\0') {
						fprintf(fout, "%s\n", buf2);
						getdata(9, 0, ":", buf2, 75, DOECHO, YEA);
						if (buf2[0] != '\0') {
							fprintf(fout, "%s\n", buf2);
						}
					}
				}
				fclose(fout);
				postfile(buf, "newcomers", "ĞÂÊÖÉÏÂ·....", 2);
				unlink(buf);
			}
			pressanykey();
		}
		newregfile = sysconf_str("NEWREGFILE");
		if (!HAS_PERM(PERM_SYSOP) && newregfile != NULL) {
			set_safe_record();
			urec->userlevel &= ~(perm);
			substitute_record(PASSFILE, urec, sizeof (struct userec), usernum);
			ansimore(newregfile, YEA);
		}
	}
}

static int
valid_ident(char *ident)
{
	static char *const invalid[] = {
		"unknown@", "root@", "gopher@", "bbs@",
		"guest@", "nobody@", "www@", NULL
	};
	int i;
	if (ident[0] == '@')
		return 0;
	for (i = 0; invalid[i] != NULL; i++)
		if (strstr(ident, invalid[i]) != NULL)
			return 0;
	return 1;
}
