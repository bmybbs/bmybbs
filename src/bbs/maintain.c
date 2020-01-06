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

#define        BLK_SIZ         4096  //by bjgyt

char cexplain[STRLEN];
char lookgrp[30];

static int valid_brdname(char *brd);
static char *chgrp(void);
static int freeclubnum(void);
static int setsecstr(char *buf, int ln);
static void anno_title(char *buf, struct boardheader *bh);
static void domailclean(struct fileheader *fhdrp);
static int cleanmail(struct userec *urec);
static void trace_state(int flag, char *name, int size);
static int touchfile(char *filename);
static int scan_register_form(char *regfile);

//proto.hÖĞÓĞÁË
//int release_email(char *userid, char *email); //ÊÍ·ÅÓÊÏä

/*
int 
select_mail(char *userid, char *email, char *record) //²éÑ¯ÓÊÏä, added by interma 2006.3.30
{
    char username[50];
    char popserver[50];
    
    char *p = strchr(email, '@');
    if (p == NULL)
        return 0;
    
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
        return 0;
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
        return 0;
    
	strncpy(buf, MY_BBS_HOME "/etc/pop_register/", 256);
	strncat(buf, popserver, 256);
   	fp = fopen(buf, "r");
    
	int lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX); // ¼ÓËøÀ´±£Ö¤»¥³â²Ù×÷

    if (fp == NULL)
	{   
		close(lockfd);
		return 0;  
    }
    char username2[50];
    char userid2[20];
    
	int isfind = 0;
    while(fgets(buf, 256, fp) != NULL)
	{
		strncpy(record, buf, 512);
	    strncpy(username2, buf, 50);
	    username2[strlen(username2) - 1] = '\0';
	    fgets(buf, 256, fp);
		strncat(record, buf, 512);
	    strncpy(userid2, buf, 20);
	    p = strchr(userid2, ' ');
	    userid2[p - userid2] = '\0';
	    
	    //printf("[%s][%s]\n", userid2, username2); 
	    if (strcmp(str_to_upper(userid), str_to_upper(userid2)) == 0 || 
            strcmp(str_to_upper(username), str_to_upper(username2)) == 0)
	    {
	        isfind = 1;
			strncpy(userid, userid2, 50); 
			strncpy(email, username2, 50); strncat(email, "@", 2); strncat(email, popserver, 50); 
			break;
	    }         
	}    

    fclose(fp);
    close(lockfd);  
                            
	return isfind;																
}   
*/

int
check_systempasswd()
{
	FILE *pass;
	char passbuf[20], prepass[STRLEN];

	clear();
	if ((pass = fopen("etc/.syspasswd", "r")) != NULL) {
		fgets(prepass, STRLEN, pass);
		fclose(pass);
		prepass[strlen(prepass) - 1] = '\0';
		getdata(1, 0, "ÇëÊäÈëÏµÍ³ÃÜÂë: ", passbuf, 19, NOECHO, YEA);
		if (passbuf[0] == '\0' || passbuf[0] == '\n')
			return NA;
		if (!checkpasswd(prepass, passbuf)) {
			move(2, 0);
			prints("´íÎóµÄÏµÍ³ÃÜÂë...");
			securityreport("ÏµÍ³ÃÜÂëÊäÈë´íÎó...",
				       "ÏµÍ³ÃÜÂëÊäÈë´íÎó...");
			pressanykey();
			return NA;
		}
	}
	return YEA;
}

int
setsystempasswd()
{
	FILE *pass;
	char passbuf[20], prepass[20];

	modify_user_mode(ADMIN);
	if (strcmp(currentuser.userid, "SYSOP"))
		return -1;
	if (!check_systempasswd())
		return -1;
	getdata(2, 0, "ÇëÊäÈëĞÂµÄÏµÍ³ÃÜÂë: ", passbuf, 19, NOECHO, YEA);
	getdata(3, 0, "È·ÈÏĞÂµÄÏµÍ³ÃÜÂë: ", prepass, 19, NOECHO, YEA);
	if (strcmp(passbuf, prepass))
		return -1;
	if (passbuf[0] == '\0' || passbuf[0] == '\n')
		return NA;
	if ((pass = fopen("etc/.syspasswd", "w")) == NULL) {
		move(4, 0);
		prints("ÏµÍ³ÃÜÂëÎŞ·¨Éè¶¨....");
		pressanykey();
		return -1;
	}
	fprintf(pass, "%s\n", genpasswd(passbuf));
	fclose(pass);
	move(4, 0);
	prints("ÏµÍ³ÃÜÂëÉè¶¨Íê³É....");
	pressanykey();
	return 0;
}

void
deliverreport(title, str)
char *title;
char *str;
{
	FILE *se;
	char fname[STRLEN];
	int savemode;

	savemode = uinfo.mode;
	sprintf(fname, "bbstmpfs/tmp/deliver.%s.%05d", currentuser.userid,
		uinfo.pid);
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "%s", str);
		fclose(se);
		postfile(fname, currboard, title, 1);
		unlink(fname);
		modify_user_mode(savemode);
	}
}

void
securityreport(str, content)
char *str;
char *content;
{
	FILE *se;
	char fname[STRLEN];
	int savemode;

	savemode = uinfo.mode;
	//report(str);
	sprintf(fname, "bbstmpfs/tmp/security.%s.%05d", currentuser.userid,
		uinfo.pid);
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "ÏµÍ³°²È«¼ÇÂ¼ÏµÍ³\nÔ­Òò£º\n%s\n", content);
		fprintf(se, "ÒÔÏÂÊÇ²¿·Ö¸öÈË×ÊÁÏ\n");
		fprintf(se, "×î½ü¹âÁÙ»úÆ÷: %s", currentuser.lasthost);
		fclose(se);
		postfile(fname, "syssecurity", str, 2);
		unlink(fname);
		modify_user_mode(savemode);
	}
}

int
get_grp(seekstr)
char seekstr[STRLEN];
{
	FILE *fp;
	char buf[STRLEN];
	char *namep;

	if ((fp = fopen("0Announce/.Search", "r")) == NULL)
		return 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		namep = strtok(buf, ": \n\r\t");
		if (namep != NULL && strcasecmp(namep, seekstr) == 0) {
			fclose(fp);
			strtok(NULL, "/");
			namep = strtok(NULL, "/");
			if (strlen(namep) < 30) {
				strcpy(lookgrp, namep);
				return 1;
			} else
				return 0;
		}
	}
	fclose(fp);
	return 0;
}

void
stand_title(title)
char *title;
{
	clear();
	//standout();
	prints(title);
	//standend();
}

int
m_info()
{
	struct userec uinfo;
	int id;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();
	stand_title("ĞŞ¸ÄÊ¹ÓÃÕß´úºÅ");
	move(1, 0);
	usercomplete("ÇëÊäÈëÊ¹ÓÃÕß´úºÅ: ", genbuf);
	if (*genbuf == '\0') {
		clear();
		return -1;
	}

	if (!(id = getuser(genbuf))) {
		move(3, 0);
		prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ");
		clrtoeol();
		pressreturn();
		clear();
		return -1;
	}
	memcpy(&uinfo, &lookupuser, sizeof (uinfo));

	move(1, 0);
	clrtobot();
	disply_userinfo(&uinfo, 1);
	uinfo_query(&uinfo, 1, id);
	return 0;
}

static int
valid_brdname(brd)
char *brd;
{
	char ch;

	ch = *brd++;
	if (!isalnum(ch) && ch != '_')
		return 0;
	while ((ch = *brd++) != '\0') {
		if (!isalnum(ch) && ch != '_')
			return 0;
	}
	return 1;
}

static char *
chgrp()
{
	int i, ch;
	static char buf[STRLEN];
	char ans[6];

/*ÏÂÃæÁ½¸öÊı×éÒò·ÖÀà±ä»¯¶øĞŞ¸Ä by ylsdd*/
#if 0
	static char *const explain[] = {
		"±¾Õ¾ÏµÍ³",
                "½»Í¨´óÑ§",
                "¿ª·¢¼¼Êõ",
		"µçÄÔÓ¦ÓÃ",
		"Ñ§Êõ¿ÆÑ§",
		"Éç»á¿ÆÑ§",
		"ÎÄÑ§ÒÕÊõ",
		"ÖªĞÔ¸ĞĞÔ",
		"ÌåÓıÔË¶¯",
		"ĞİÏĞÒôÀÖ",
		"ÓÎÏ·ÌìµØ",
		"ĞÖµÜÔºĞ£",
		"ĞÂÎÅĞÅÏ¢",
		"ÏçÒôÏçÇé",
		"TEMP",
		NULL
	};

	static char *const groups[] = {
		"GROUP_0",
		"GROUP_1",
		"GROUP_2",
		"GROUP_3",
		"GROUP_4",
		"GROUP_5",
		"GROUP_6",
		"GROUP_7",
		"GROUP_8",
		"GROUP_9",
		"GROUP_G",
		"GROUP_B",
		"GROUP_N",
                "GROUP_H",
		"GROUP_S",
		NULL
	};
#endif
	clear();
	move(2, 0);
	prints("Ñ¡Ôñ¾«»ªÇøµÄÄ¿Â¼\n\n");
	for (i = 0; i < sectree.nsubsec; i++) {
		prints("[1;32m%2d[m. %-20s                GROUP_%c\n", i,
		       sectree.subsec[i]->title, sectree.subsec[i]->basestr[0]);
	}
	sprintf(buf, "ÇëÊäÈëÄãµÄÑ¡Ôñ(0~%d): ", --i);
	while (1) {
		getdata(i + 6, 0, buf, ans, 4, DOECHO, YEA);
		if (!isdigit(ans[0]))
			continue;
		ch = atoi(ans);
		if (ch < 0 || ch > i || ans[0] == '\r' || ans[0] == '\0')
			continue;
		else
			break;
	}
	strcpy(cexplain, sectree.subsec[ch]->title);
	snprintf(buf, sizeof (buf), "GROUP_%c", sectree.subsec[ch]->basestr[0]);
	return buf;
}

static int
freeclubnum()
{
	FILE *fp;
	int club[4] = { 0, 0, 0, 0 };
	int i;
	struct boardheader rec;
	if ((fp = fopen(BOARDS, "r")) == NULL) {
		return -1;
	}
	while (!feof(fp)) {
		fread(&rec, sizeof (struct boardheader), 1, fp);
		if (rec.clubnum != 0)
			club[rec.clubnum / 32] |= (1 << (rec.clubnum % 32));
	}
	fclose(fp);
	for (i = 1; i < 32 * 4; i++)
		if ((~club[i / 32]) & (1 << (i % 32))) {
			return i;
		}
	return -1;
}

static int
setsecstr(char *buf, int ln)
{
	const struct sectree *sec;
	int i = 0, ch, len, choose = 0;
	sec = getsectree(buf);
	move(ln, 0);
	clrtobot();
	while (1) {
		prints
		    ("=======µ±Ç°·ÖÇøÑ¡Ôñ: \033[31m%s\033[0;1m %s\033[m =======\n",
		     sec->basestr, sec->title);
		if (sec->parent) {
			prints(" (\033[4;33m#\033[0m) »ØÉÏ¼¶·ÖÇø\n");
			prints(" (\033[4;33m%%\033[0m) ¾Í·ÅÔÚÕâÀï\n");
		}
		prints
		    (" (\033[4;33m*\033[0m) ±£³ÖÔ­À´Éè¶¨(¿ÉÓÃ»Ø³µÑ¡¶¨±¾Ïî)\n");
		len = strlen(sec->basestr);
		for (i = 0; i < sec->nsubsec; i++) {
			if (i && !(i % 3))
				prints("\n");
			ch = sec->subsec[i]->basestr[len];
			prints
			    (" (\033[4;33m%c\033[0m) \033[31;1m %s\033[0m",
			     ch, sec->subsec[i]->title);
		}
		prints("\nÇë°´À¨ºÅÄÚµÄ×ÖÄ¸Ñ¡Ôñ");
		while (1) {
			ch = igetkey();
			if (ch == '\n' || ch == '\r')
				ch = '*';
			if (sec->parent == NULL && (ch == '#' || ch == '%'))
				continue;
			for (i = 0; i < sec->nsubsec; i++) {
				if (sec->subsec[i]->basestr[len] == ch) {
					choose = i;
					break;
				}
			}
			if (ch != '#' && ch != '*' && ch != '%'
			    && i == sec->nsubsec) continue;
			break;
		}
		move(ln, 0);
		clrtobot();
		switch (ch) {
		case '#':
			sec = sec->parent;
			break;
		case '%':
			strcpy(buf, sec->basestr);
			return 0;
		case '*':
			strcpy(buf, "");
			return 0;
		default:
			sec = sec->subsec[choose];
		}
	}
}

int
m_newbrd()
{
	struct boardheader newboard;
	char ans[4];
	char vbuf[100];
	char *group;
	int bid;
	int now;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();
	stand_title("¿ªÆôĞÂÌÖÂÛÇø");
	memset(&newboard, 0, sizeof (newboard));
	move(2, 0);
	ansimore2("etc/boardref", NA, 1, 11);
	while (1) {
		getdata(10, 0, "ÌÖÂÛÇøÃû³Æ:   ", newboard.filename, 18, DOECHO,
			YEA);
		if (newboard.filename[0] != 0) {
			struct boardheader dh;
			if (new_search_record
			    (BOARDS, &dh, sizeof (dh), (void *) cmpbnames,
			     newboard.filename)) {
				prints("\n´íÎó! ´ËÌÖÂÛÇøÒÑ¾­´æÔÚ!!");
				pressanykey();
				return -1;
			}
		} else
			return -1;
		if (valid_brdname(newboard.filename))
			break;
		prints("\n²»ºÏ·¨Ãû³Æ!!");
	}
	getdata(11, 0, "ÌÖÂÛÇøÖĞÎÄÃû: ", newboard.title,
		sizeof (newboard.title), DOECHO, YEA);
	if (newboard.title[0] == '\0')
		return -1;
	strcpy(vbuf, "vote/");
	strcat(vbuf, newboard.filename);
	setbpath(genbuf, newboard.filename);
	if (getbnum(newboard.filename) > 0 || mkdir(genbuf, 0777) == -1
	    || mkdir(vbuf, 0777) == -1) {
		prints("\n´íÎóµÄÌÖÂÛÇøÃû³Æ!!\n");
		pressreturn();
		rmdir(vbuf);
		rmdir(genbuf);
		clear();
		return -1;
	}
	move(12, 0);
	prints("Ñ¡ÔñÖ÷·ÖÇø: ");
	while (1) {
		genbuf[0] = 0;
		setsecstr(genbuf, 13);
		if (genbuf[0] != '\0')
			break;
	}
	move(12, 0);
	prints("Ö÷·ÖÇøÉè¶¨: %s", genbuf);
	newboard.secnumber1 = genbuf[0];
	strsncpy(newboard.sec1, genbuf, sizeof (newboard.sec1));
	move(12, 30);
	prints("Ñ¡Ôñ·ÖÇøÁ´½Ó: ");
	genbuf[0] = 0;
	setsecstr(genbuf, 13);
	move(12, 30);
	prints("·ÖÇøÁ´½ÓÉè¶¨: %s", genbuf);
	newboard.secnumber2 = genbuf[0];
	strsncpy(newboard.sec2, genbuf, sizeof (newboard.sec2));
	move(13, 0);
	while (1) {
		getdata(13, 0, "ÌÖÂÛÇø·ÖÀà(4×Ö):", newboard.type,
			sizeof (newboard.type), DOECHO, YEA);
		if (strlen(newboard.type) == 4)
			break;
	}
	move(14, 0);
	if (newboard.secnumber2 == 'C') {
		newboard.flag &= ~ANONY_FLAG;
		newboard.level = 0;
		if ((newboard.clubnum = freeclubnum()) == -1) {
			prints("Ã»ÓĞ¿ÕµÄ¾ãÀÖ²¿Î»ÖÃÁË");
			pressreturn();
			clear();
			return -1;
		}
		sprintf(genbuf, "%d", newboard.clubnum);
		if (askyn("ÊÇ·ñÊÇ¿ª·ÅÊ½¾ãÀÖ²¿", YEA, NA) == YEA)
			newboard.flag |= CLUBTYPE_FLAG;
		else
			newboard.flag &= ~CLUBTYPE_FLAG;
	} else {
		if (askyn("ÊÇ·ñÏŞÖÆ´æÈ¡È¨Àû", NA, NA) == YEA) {
			getdata(15, 0, "ÏŞÖÆ Read/Post? [R]: ", ans, 2, DOECHO,
				YEA);
			if (*ans == 'P' || *ans == 'p')
				newboard.level = PERM_POSTMASK;
			else
				newboard.level = 0;
			move(1, 0);
			clrtobot();
			move(2, 0);
			prints("Éè¶¨ %s È¨Àû. ÌÖÂÛÇø: '%s'\n",
			       (newboard.level & PERM_POSTMASK ? "POST" :
				"READ"), newboard.filename);
			newboard.level =
			    setperms(newboard.level, "È¨ÏŞ", NUMPERMS,
				     showperminfo, 0);
			clear();
		} else
			newboard.level = 0;

		move(15, 0);
		if (askyn("ÊÇ·ñ¼ÓÈëÄäÃû°æ", NA, NA) == YEA)
			newboard.flag |= ANONY_FLAG;
		else
			newboard.flag &= ~ANONY_FLAG;
	}
	move(16, 0);
	if (askyn("ÊÇ·ñÊÇ×ªĞÅ°æÃæ", NA, NA) == YEA)
		newboard.flag |= INNBBSD_FLAG;
	else
		newboard.flag &= ~INNBBSD_FLAG;

	if (askyn("ÊÇ·ñÊÇĞèÒª½øĞĞÄÚÈİ¼ì²éµÄ°æÃæ", NA, NA) == YEA)
		newboard.flag |= IS1984_FLAG;
	else
		newboard.flag &= ~IS1984_FLAG;
	if (askyn("°æÃæÄÚÈİÊÇ·ñ¿ÉÄÜºÍÕşÖÎÏà¹Ø", NA, NA) == YEA)
		newboard.flag |= POLITICAL_FLAG;
	else
		newboard.flag &= ~POLITICAL_FLAG;
	now = time(NULL);
	newboard.board_mtime = now;
	newboard.board_ctime = now;

	if ((bid = getbnum("")) > 0) {
		substitute_record(BOARDS, &newboard, sizeof (newboard), bid);
	} else if (append_record(BOARDS, &newboard, sizeof (newboard)) == -1) {
		pressreturn();
		clear();
		return -1;
	}

	reload_boards();
	update_postboards();

	group = chgrp();
	sprintf(vbuf, "%-38.38s", newboard.title);
	if (group != NULL) {
		if (add_grp(group, cexplain, newboard.filename, vbuf) == -1)
			prints("\n³ÉÁ¢¾«»ªÇøÊ§°Ü....\n");
		else
			prints("ÒÑ¾­ÖÃÈë¾«»ªÇø...\n");
	}

	prints("\nĞÂÌÖÂÛÇø³ÉÁ¢\n");
	{
		char secu[STRLEN];
		sprintf(secu, "³ÉÁ¢ĞÂ°æ£º%s", newboard.filename);
		securityreport(secu, secu);
	}
	pressreturn();
	clear();
	return 0;
}

static void
anno_title(buf, bh)
char *buf;
struct boardheader *bh;
{
	char bm[IDLEN * 4 + 4];	//·ÅËÄ¸ö°æÎñ
	sprintf(buf, "%-38.38s", bh->title);
	if (bh->bm[0][0] == 0)
		return;
	else {
		strcat(buf, "(BM:");
		bm2str(bm, bh);
		strcat(buf, bm);
	}
	strcat(buf, ")");
	return;
}

int
m_editbrd()
{
	char bname[STRLEN], buf[STRLEN], oldtitle[STRLEN], vbuf[256], *group;
	char oldpath[STRLEN], newpath[STRLEN], tmp_grp[30];
	int pos, noidboard, a_mv, isclub, innboard, isopenclub, is1984;
	int political;
	struct boardheader fh, newfh;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();
	stand_title("ĞŞ¸ÄÌÖÂÛÇø×ÊÑ¶");
	move(1, 0);
	make_blist_full();
	namecomplete("ÊäÈëÌÖÂÛÇøÃû³Æ: ", bname);
	if (*bname == '\0') {
		move(2, 0);
		prints("´íÎóµÄÌÖÂÛÇøÃû³Æ");
		pressreturn();
		clear();
		return -1;
	}
	pos =
	    new_search_record(BOARDS, &fh, sizeof (fh), (void *) cmpbnames,
			      bname);
	if (!pos) {
		move(2, 0);
		prints("´íÎóµÄÌÖÂÛÇøÃû³Æ");
		pressreturn();
		clear();
		return -1;
	}
	noidboard = fh.flag & ANONY_FLAG;
	isclub = (fh.clubnum > 0);
	innboard = (fh.flag & INNBBSD_FLAG) ? YEA : NA;
	isopenclub = fh.flag & CLUBTYPE_FLAG;
	is1984 = fh.flag & IS1984_FLAG;
	political = fh.flag & POLITICAL_FLAG;
	move(2, 0);
	memcpy(&newfh, &fh, sizeof (newfh));
	prints("ÌÖÂÛÇøÃû³Æ:   %s", fh.filename);
	move(2, 40);
	prints("ÌÖÂÛÇøËµÃ÷:   %s\n", fh.title);
	prints("ÄäÃûÌÖÂÛÇø:   %s  ¾ãÀÖ²¿°æÃæ£º  %s  ×ªĞÅÌÖÂÛÇø£º  %s\n",
	       (noidboard) ? "Yes" : "No", (isclub) ? "Yes" : "No",
	       (innboard) ? "Yes" : "No");
	strcpy(oldtitle, fh.title);
	prints("ÏŞÖÆ %s È¨Àû: %s",
	       (fh.level & PERM_POSTMASK) ? "POST" :
	       (fh.level & PERM_NOZAP) ? "ZAP" : "READ",
	       (fh.level & ~PERM_POSTMASK) == 0 ? "²»ÉèÏŞ" : "ÓĞÉèÏŞ");
	prints(" %s½øĞĞÈË¹¤ÎÄÕÂÉó²é", is1984 ? "Òª" : "²»");
	if (political)
		prints(" ÄÚÈİ¿ÉÄÜºÍÕşÖÎÏà¹Ø");
	move(5, 0);
	if (askyn("ÊÇ·ñ¸ü¸ÄÒÔÉÏ×ÊÑ¶", NA, NA) == YEA) {
		move(6, 0);
		prints("Ö±½Ó°´ <Return> ²»ĞŞ¸Ä´ËÀ¸×ÊÑ¶...");
	      enterbname:
		getdata(7, 0, "ĞÂÌÖÂÛÇøÃû³Æ: ", genbuf, 18, DOECHO, YEA);
		if (genbuf[0] != 0) {
			struct boardheader dh;
			if (new_search_record
			    (BOARDS, &dh, sizeof (dh), (void *) cmpbnames,
			     genbuf)) {
				move(2, 0);
				prints("´íÎó! ´ËÌÖÂÛÇøÒÑ¾­´æÔÚ!!");
				move(7, 0);
				clrtoeol();
				goto enterbname;
			}
			if (valid_brdname(genbuf)) {
				strsncpy(newfh.filename, genbuf,
					 sizeof (newfh.filename));
				strcpy(bname, genbuf);
			} else {
				move(2, 0);
				prints("²»ºÏ·¨µÄÌÖÂÛÇøÃû³Æ!");
				move(7, 0);
				clrtoeol();
				goto enterbname;
			}
		}
		getdata(8, 0, "ĞÂÌÖÂÛÇøÖĞÎÄÃû: ", genbuf, 24, DOECHO, YEA);
		if (genbuf[0] != 0)
			strsncpy(newfh.title, genbuf, sizeof (newfh.title));
		ansimore2("etc/boardref", NA, 9, 7);
		strcpy(genbuf, newfh.sec1);
		move(16, 0);
		prints("Ñ¡ÔñĞÂ·ÖÇø: %s", genbuf);
		setsecstr(genbuf, 17);
		if (genbuf[0] != 0) {
			newfh.secnumber1 = genbuf[0];
			strsncpy(newfh.sec1, genbuf, sizeof (newfh.sec1));
		}
		move(16, 0);
		prints("ĞÂ·ÖÇøÉè¶¨: %s", genbuf);
		move(16, 40);
		strcpy(genbuf, newfh.sec2);
		prints("Ñ¡ÔñĞÂ·ÖÇøÁ´½Ó: %s", genbuf);
		setsecstr(genbuf, 17);
		newfh.secnumber2 = genbuf[0];
		strsncpy(newfh.sec2, genbuf, sizeof (newfh.sec2));
		move(16, 40);
		prints("ĞÂ·ÖÇøÁ´½ÓÉè¶¨: %s", genbuf);
		getdata(17, 0, "ĞÂÌÖÂÛÇø·ÖÀà(4×Ö): ", genbuf, 5, DOECHO, YEA);
		if (genbuf[0] != 0)
			strsncpy(newfh.type, genbuf, sizeof (newfh.type));
		move(18, 0);
		if (askyn("ÊÇ·ñÊÇ×ªĞÅ°æÃæ", innboard, NA) == YEA)
			newfh.flag |= INNBBSD_FLAG;
		else
			newfh.flag &= ~INNBBSD_FLAG;
		move(18, 28);
		if (askyn("ÊÇ·ñÊÇĞèÒª½øĞĞÄÚÈİ¼ì²éµÄ°æÃæ", is1984, NA) == YEA)
			newfh.flag |= IS1984_FLAG;
		else
			newfh.flag &= ~IS1984_FLAG;
		if (askyn("°æÃæÄÚÈİÊÇ·ñ¿ÉÄÜºÍÕşÖÎÏà¹Ø", political, NA) == YEA)
			newfh.flag |= POLITICAL_FLAG;
		else
			newfh.flag &= ~POLITICAL_FLAG;

		genbuf[0] = 0;
		move(19, 0);
		if (askyn("ÊÇ·ñÒÆ¶¯¾«»ªÇøµÄÎ»ÖÃ", NA, NA) == YEA)
			a_mv = 2;
		else
			a_mv = 0;
		move(20, 0);
		if (newfh.secnumber2 == 'C')	//ÊÇ¾ãÀÖ²¿°æÃæ
		{
			newfh.flag &= ~ANONY_FLAG;
			newfh.level = 0;
			if (fh.clubnum)
				newfh.clubnum = fh.clubnum;
			else
				newfh.clubnum = freeclubnum();
			if (askyn("ÊÇ·ñÊÇ¿ª·ÅÊ½¾ãÀÖ²¿", isopenclub, NA) == YEA)
				newfh.flag |= CLUBTYPE_FLAG;
			else
				newfh.flag &= ~CLUBTYPE_FLAG;
			getdata(21, 0, "È·¶¨Òª¸ü¸ÄÂğ? (Y/N) [N]: ", genbuf, 4,
				DOECHO, YEA);
		} else {
			newfh.clubnum = 0;
			sprintf(buf, "ÄäÃû°æ (Y/N)? [%c]: ",
				(noidboard) ? 'Y' : 'N');
			getdata(20, 0, buf, genbuf, 4, DOECHO, YEA);
			if (*genbuf == 'y' || *genbuf == 'Y' || *genbuf == 'N'
			    || *genbuf == 'n') {
				if (*genbuf == 'y' || *genbuf == 'Y')
					newfh.flag |= ANONY_FLAG;
				else
					newfh.flag &= ~ANONY_FLAG;
			}
			if (askyn("ÊÇ·ñ¸ü¸Ä´æÈ¡È¨ÏŞ", NA, NA) == YEA) {
				char ans[4];
				sprintf(genbuf,
					"ÏŞÖÆ (R)ÔÄ¶Á »ò (P)ÕÅÌù ÎÄÕÂ [%c]: ",
					(newfh.level & PERM_POSTMASK ? 'P' :
					 'R'));
				getdata(21, 0, genbuf, ans, 2, DOECHO, YEA);
				if ((newfh.level & PERM_POSTMASK)
				    && (*ans == 'R' || *ans == 'r'))
					newfh.level &= ~PERM_POSTMASK;
				else if (!(newfh.level & PERM_POSTMASK)
					 && (*ans == 'P' || *ans == 'p'))
					newfh.level |= PERM_POSTMASK;
				clear();
				move(2, 0);
				prints("Éè¶¨ %s '%s' ÌÖÂÛÇøµÄÈ¨ÏŞ\n",
				       newfh.level & PERM_POSTMASK ? "ÕÅÌù" :
				       "ÔÄ¶Á", newfh.filename);
				newfh.level =
				    setperms(newfh.level, "È¨ÏŞ", NUMPERMS,
					     showperminfo, 0);
				clear();
				getdata(0, 0, "È·¶¨Òª¸ü¸ÄÂğ? (Y/N) [N]: ",
					genbuf, 4, DOECHO, YEA);
			} else {
				getdata(22, 0, "È·¶¨Òª¸ü¸ÄÂğ? (Y/N) [N]: ",
					genbuf, 4, DOECHO, YEA);
			}
		}
		clear();
		if (*genbuf == 'Y' || *genbuf == 'y') {
			{
				char secu[STRLEN];
				sprintf(secu, "ĞŞ¸ÄÌÖÂÛÇø£º%s(%s)", fh.filename,
					newfh.filename);
				securityreport(secu, secu);
			}
			newfh.board_mtime = time(NULL);
			if (strcmp(fh.filename, newfh.filename)) {
				char old[256], tar[256];
				a_mv = 1;
				setbpath(old, fh.filename);
				setbpath(tar, newfh.filename);
				rename(old, tar);
				sprintf(old, "vote/%s", fh.filename);
				sprintf(tar, "vote/%s", newfh.filename);
				rename(old, tar);
				if (seek_in_file("etc/junkboards", fh.filename)) {
					del_from_file("etc/junkboards",
						      fh.filename);
					addtofile("etc/junkboards",
						  newfh.filename);
				}
			}
			get_grp(fh.filename);
			anno_title(vbuf, &newfh);
			edit_grp(fh.filename, lookgrp, oldtitle, vbuf);
			if (a_mv >= 1) {
				group = chgrp();
				get_grp(fh.filename);
				strcpy(tmp_grp, lookgrp);
				if (strcmp(tmp_grp, group) || a_mv != 2) {
					del_from_file("0Announce/.Search",
						      fh.filename);
					if (group != NULL) {
						if (add_grp
						    (group, cexplain,
						     newfh.filename,
						     vbuf) == -1)
							    prints
							    ("\n³ÉÁ¢¾«»ªÇøÊ§°Ü....\n");
						else
							prints
							    ("ÒÑ¾­ÖÃÈë¾«»ªÇø...\n");
						sprintf(newpath,
							"0Announce/groups/%s/%s",
							group, newfh.filename);
						sprintf(oldpath,
							"0Announce/groups/%s/%s",
							tmp_grp, fh.filename);
						if (dashd(oldpath)) {
							deltree(newpath);
						}
						rename(oldpath, newpath);
						del_grp(tmp_grp, fh.filename,
							fh.title);
					}
				}
			}
			substitute_record(BOARDS, &newfh, sizeof (newfh), pos);
			reload_boards();
			update_postboards();
		}
	}
	clear();
	return 0;
}

FILE *cleanlog;
char curruser[IDLEN + 2];
extern int delmsgs[];
extern int delcnt;

static void
domailclean(fhdrp)
struct fileheader *fhdrp;
{
	static int newcnt, savecnt, deleted, idc;
	char buf[STRLEN];

	if (fhdrp == NULL) {
		fprintf(cleanlog, "new = %d, saved = %d, deleted = %d\n",
			newcnt, savecnt, deleted);
		newcnt = savecnt = deleted = idc = 0;
		if (delcnt) {
			setmailfile(buf, curruser, DOT_DIR);
			while (delcnt--)
				delete_record(buf, sizeof (struct fileheader),
					      delmsgs[delcnt]);
		}
		delcnt = 0;
		return;
	}
	idc++;
	if (!(fhdrp->accessed & FH_READ))
		newcnt++;
	else if (fhdrp->accessed & FH_MARKED)
		savecnt++;
	else {
		deleted++;
		setmailfile(buf, curruser, fh2fname(fhdrp));
		unlink(buf);
		delmsgs[delcnt++] = idc;
	}
}

static int
cleanmail(urec)
struct userec *urec;
{
	struct stat statb;
	if (urec->userid[0] == '\0' || !strcmp(urec->userid, "new"))
		return 0;
	sprintf(genbuf, "mail/%c/%s/%s", mytoupper(urec->userid[0]),
		urec->userid, DOT_DIR);
	fprintf(cleanlog, "%s: ", urec->userid);
	if (stat(genbuf, &statb) == -1)
		fprintf(cleanlog, "no mail\n");
	else if (statb.st_size == 0)
		fprintf(cleanlog, "no mail\n");
	else {
		strcpy(curruser, urec->userid);
		delcnt = 0;
		apply_record(genbuf, (void *) domailclean,
			     sizeof (struct fileheader));
		domailclean(NULL);
	}
	return 0;
}

int
m_mclean()
{
	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();
	stand_title("Çå³ıË½ÈËĞÅ¼ş");
	move(1, 0);
	prints("Çå³ıËùÓĞÒÑ¶ÁÇÒÎ´ mark µÄĞÅ¼ş\n");
	if (askyn("È·¶¨Âğ", NA, NA) == NA) {
		clear();
		return -1;
	}
	{
		char secu[STRLEN];
		sprintf(secu, "Çå³ıËùÓĞÊ¹ÓÃÕßÒÑ¶ÁĞÅ¼ş¡£");
		securityreport(secu, secu);
	}

	cleanlog = fopen("mailclean.log", "w");
	move(3, 0);
	prints("ÇëÄÍĞÄµÈºò.\n");
	refresh();
	if (apply_record(PASSFILE, (void *) cleanmail, sizeof (struct userec))
	    == -1) {
		move(4, 0);
		prints("apply PASSFILE err...\n");
		pressreturn();
		clear();
		return -1;
	}
	move(4, 0);
	fclose(cleanlog);
	prints("Çå³ıÍê³É! ¼ÇÂ¼µµ mailclean.log.\n");
	pressreturn();
	clear();
	return 0;
}

static void
trace_state(flag, name, size)
int flag, size;
char *name;
{
	char buf[STRLEN];

	if (flag != -1) {
		sprintf(buf, "ON (size = %d)", size);
	} else {
		strcpy(buf, "OFF");
	}
	prints("%s¼ÇÂ¼ %s\n", name, buf);
}

static int
touchfile(filename)
char *filename;
{
	int fd;

	if ((fd = open(filename, O_RDWR | O_CREAT, 0600)) > 0) {
		close(fd);
	}
	return fd;
}

int
m_trace()
{
	struct stat ostatb, cstatb;
	int otflag, ctflag, done = 0;
	char ans[3];
	char *msg;

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();
	stand_title("Set Trace Options");
	while (!done) {
		move(2, 0);
		otflag = stat("trace", &ostatb);
		ctflag = stat("trace.chatd", &cstatb);
		prints("Ä¿Ç°Éè¶¨:\n");
		trace_state(otflag, "Ò»°ã", ostatb.st_size);
		trace_state(ctflag, "ÁÄÌì", cstatb.st_size);
		move(9, 0);
		prints("<1> ÇĞ»»Ò»°ã¼ÇÂ¼\n");
		prints("<2> ÇĞ»»ÁÄÌì¼ÇÂ¼\n");
		getdata(12, 0, "ÇëÑ¡Ôñ (1/2/Exit) [E]: ", ans, 2, DOECHO, YEA);

		switch (ans[0]) {
		case '1':
			if (otflag) {
				touchfile("trace");
				msg = "Ò»°ã¼ÇÂ¼ ON";
			} else {
				rename("trace", "trace.old");
				msg = "Ò»°ã¼ÇÂ¼ OFF";
			}
			break;
		case '2':
			if (ctflag) {
				touchfile("trace.chatd");
				msg = "ÁÄÌì¼ÇÂ¼ ON";
			} else {
				rename("trace.chatd", "trace.chatd.old");
				msg = "ÁÄÌì¼ÇÂ¼ OFF";
			}
			break;
		default:
			msg = NULL;
			done = 1;
		}
		move(t_lines - 2, 0);
		if (msg) {
			prints("%s\n", msg);
			//report(msg);
		}
	}
	clear();
	return 0;
}

/* mode == O_EXCL / O_APPEND / O_TRUNC by bjgyt*/
int f_cp(char *src, char *dst, int mode)
{
   int     fsrc, fdst, ret;
   ret = 0;
   if ((fsrc = open(src, O_RDONLY)) >= 0) {
      ret = -1;
      if ((fdst = open(dst, O_WRONLY | O_CREAT | mode, 0600)) >= 0) {
         char    pool[BLK_SIZ];
         src = pool;
         do {
            ret = read(fsrc, src, BLK_SIZ);
            if (ret <= 0) break;
         } while (write(fdst, src, ret) > 0);
         close(fdst);
      }
      close(fsrc);
   }
   return ret;
}

static int
scan_register_form(regfile)
char *regfile;
{
	static char *const field[] = { "usernum", "userid", "realname", "dept",
		"addr", "phone", "assoc", NULL
	};
	static char *const finfo[] =
	    { "ÕÊºÅÎ»ÖÃ", "ÉêÇëÕÊºÅ", "ÕæÊµĞÕÃû", "Ñ§Ğ£Ïµ¼¶",
		"Ä¿Ç°×¡Ö·", "Á¬Âçµç»°", "½é ÉÜ ÈË", NULL
	};
	static char *const reason[] =
	    { "ÇëÈ·ÊµÌîĞ´ÕæÊµĞÕÃû.", "ÇëÌîÑ§Ğ£¿ÆÏµÄê¼¶(»ò¹¤×÷µ¥Î»)",
		"ÇëÌîĞ´ÍêÕûµÄ×¡Ö·×ÊÁÏ.", "ÇëÏêÏ¸/È·ÊµÌîĞ´×¢²á×ÊÁÏ.",
		"ÇëÓÃÖĞÎÄÌîĞ´ÉêÇëµ¥.", NULL
	};
	struct userec uinfo;
	FILE *fn, *fout, *freg;
	struct stat st;
	char fdata[7][STRLEN];
	char fname[STRLEN], buf[STRLEN];
	char ans[5], *ptr, *uid;
	int n, unum, lockfd, numline = 0;

	uid = currentuser.userid;
	stand_title("ÒÀĞòÉè¶¨ËùÓĞĞÂ×¢²á×ÊÁÏ");
	sprintf(fname, "%s.tmp", regfile);
	move(2, 0);
	lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX);
	if (dashf(fname)) {
		close(lockfd);
		if (stat(fname, &st) != -1 && st.st_atime > time(0) - 86400) {
			prints
			    ("ÆäËû SYSOP ÕıÔÚ²é¿´×¢²áÉêÇëµ¥, Çë¼ì²éÊ¹ÓÃÕß×´Ì¬.\n");
			getdata(2, 0, "ÄãÈ·¶¨Ã»ÓĞÆäËû SYSOP ÔÚÉóºË×¢²áµ¥Âğ £¿ [y/N]: ", ans, 2, DOECHO, YEA);//by bjgyt
                	if (ans[0] == 'Y' || ans[0] == 'y')
                        	f_cp(fname, regfile, O_APPEND);
                	else {
                        	pressreturn();
                        	return -1;
			}
		} else {
			prints
			    ("ÆäËû SYSOP ÕıÔÚ²é¿´×¢²áÉêÇëµ¥, Çë¼ì²éÊ¹ÓÃÕß×´Ì¬.\n");
			getdata(2, 0, "ÄãÈ·¶¨Ã»ÓĞÆäËû SYSOP ÔÚÉóºË×¢²áµ¥Âğ £¿ [y/N]: ", ans, 2, DOECHO, YEA);
                        if (ans[0] == 'Y' || ans[0] == 'y')
                                f_cp(fname, regfile, O_APPEND);
                        else {
                                pressreturn();
                                return -1;
                        }
		}
	} else {
		rename(regfile, fname);
		close(lockfd);
	}
	if ((fn = fopen(fname, "r")) == NULL) {
		move(2, 0);
		prints("ÏµÍ³´íÎó, ÎŞ·¨¶ÁÈ¡×¢²á×ÊÁÏµµ, ÇëÁªÏµÏµÍ³Î¬»¤\n", fname);
		pressreturn();
		return -1;
	}
	memset(fdata, 0, sizeof (fdata));
	while (fgets(genbuf, STRLEN, fn) != NULL) {
		if ((ptr = (char *) strstr(genbuf, ": ")) != NULL) {
			*ptr = '\0';
			for (n = 0; field[n] != NULL; n++) {
				if (strcmp(genbuf, field[n]) == 0) {
					strcpy(fdata[n], ptr + 2);
					if ((ptr =
					     (char *) strchr(fdata[n],
							     '\n')) != NULL)
						    *ptr = '\0';
				}
			}
			numline++;
			continue;
		}
		if (numline == 0)
			continue;
		numline = 0;

		if ((unum = getuser(fdata[1])) == 0) {
			move(2, 0);
			clrtobot();
			prints("ÏµÍ³´íÎó, ²éÎŞ´ËÕÊºÅ.\n\n");
			for (n = 0; field[n] != NULL; n++)
				prints("%s     : %s\n", finfo[n], fdata[n]);
			pressreturn();
			memset(fdata, 0, sizeof (fdata));
			continue;
		}
		memcpy(&uinfo, &lookupuser, sizeof (uinfo));
		move(1, 0);
		prints("ÕÊºÅÎ»ÖÃ     : %d\n", unum);
		disply_userinfo(&uinfo, 1);
		move(15, 0);
		printdash(NULL);
		clrtobot();
		for (n = 0; field[n] != NULL; n++)
			prints("%s     : %s\n", finfo[n], fdata[n]);
		if (uinfo.userlevel & PERM_LOGINOK) {
			move(t_lines - 1, 0);
			prints("´ËÕÊºÅ²»ĞèÔÙÌîĞ´×¢²áµ¥.\n");
			igetkey();
			ans[0] = 'E';//by bjgyt ans[0] = 'S'
		} else {
			getdata(t_lines - 1, 0,
				"ÊÇ·ñ½ÓÊÜ´Ë×ÊÁÏ (Y/N/Q/Del/Skip)? [Y]: ", ans,
				3, DOECHO, YEA);
		}
		move(1, 0);
		clrtobot();
		switch (ans[0]) {
		case 'S':
		case 's':
			if ((freg = fopen(regfile, "a")) != NULL) {
                                for (n = 0; field[n] != NULL; n++)
                                        fprintf(freg, "%s: %s\n", field[n],
                                                fdata[n]);
                                fprintf(freg, "----\n");
                                fclose(freg);
                        }
			break;	
		case 'D':
		case 'd':
			prints
			    ("²»ÔÙ½ÓÊÜÀ´×Ô´ËÓÃ»§ÃûµÄ×¢²á,´ËÓÃ»§½«±»·â½ûÉÏÕ¾È¨ÏŞ.\n");
			uinfo.userlevel &= ~PERM_DEFAULT;
			substitute_record(PASSFILE, &uinfo, sizeof (uinfo),
					  unum);
			sprintf(genbuf, "É¾³ı %s µÄ×¢²áµ¥.", uinfo.userid);
			securityreport(genbuf, genbuf);
			break;
		case 'E':                                       //add by mintbaggio@BMY
                case 'e':
                        substitute_record(PASSFILE, &uinfo, sizeof (uinfo),
                                          unum);
                        sprintf(genbuf, "É¾³ı %s µÄ×¢²áµ¥.", uinfo.userid);
                        securityreport(genbuf, genbuf);
                        break;
		case 'Y':
		case 'y':
			prints("ÒÔÏÂÊ¹ÓÃÕß×ÊÁÏÒÑ¾­¸üĞÂ:\n");
			n = strlen(fdata[5]);
			if (n + strlen(fdata[3]) > 60) {
				if (n > 40)
					fdata[5][n = 40] = '\0';
				fdata[3][60 - n] = '\0';
			}
			strsncpy(uinfo.realname, fdata[2],
				 sizeof (uinfo.realname));
			strsncpy(uinfo.address, fdata[4],
				 sizeof (uinfo.address));
			sprintf(genbuf, "%s$%s@%s", fdata[3], fdata[5], uid);
			strsncpy(uinfo.realmail, genbuf,
				 sizeof (uinfo.realmail));
			uinfo.userlevel |= PERM_DEFAULT;	// by ylsdd
			substitute_record(PASSFILE, &uinfo, sizeof (uinfo),
					  unum);

			sethomefile(buf, uinfo.userid, "sucessreg");
			if ((fout = fopen(buf, "w")) != NULL) {
				fprintf(fout, "\n");
				fclose(fout);
			}

			sethomefile(buf, uinfo.userid, "register");
			if (dashf(buf)) {
				sethomefile(genbuf, uinfo.userid,
					    "register.old");
				rename(buf, genbuf);
			}
			if ((fout = fopen(buf, "w")) != NULL) {
				for (n = 0; field[n] != NULL; n++)
					fprintf(fout, "%s: %s\n", field[n],
						fdata[n]);
				n = time(NULL);
				fprintf(fout, "Date: %s",
					ctime((time_t *) & n));
				fprintf(fout, "Approved: %s\n", uid);
				fclose(fout);
			}
			mail_file("etc/s_fill", uinfo.userid,
				  "¹§ìûÄúÍ¨¹ıÉí·İÑéÖ¤");

			mail_file("etc/s_fill2", uinfo.userid,
				  "»¶Ó­¼ÓÈë" MY_BBS_NAME "´ó¼ÒÍ¥");
			sethomefile(buf, uinfo.userid, "mailcheck");
			unlink(buf);
			sprintf(genbuf, "ÈÃ %s Í¨¹ıÉí·ÖÈ·ÈÏ.", uinfo.userid);
			securityreport(genbuf, genbuf);
			break;
		case 'Q':
		case 'q':
			if ((freg = fopen(regfile, "a")) != NULL) {
				for (n = 0; field[n] != NULL; n++)
					fprintf(freg, "%s: %s\n", field[n],
						fdata[n]);
				fprintf(freg, "----\n");
				while (fgets(genbuf, STRLEN, fn) != NULL)
					fputs(genbuf, freg);
				fclose(freg);
			}
			break;
		case 'N':
		case 'n':
			for (n = 0; field[n] != NULL; n++)
				prints("%s: %s\n", finfo[n], fdata[n]);
			printdash(NULL);
			move(9, 0);
			prints
			    ("ÇëÑ¡Ôñ/ÊäÈëÍË»ØÉêÇë±íÔ­Òò, °´ <enter> È¡Ïû.\n\n");
			for (n = 0; reason[n] != NULL; n++)
				prints("%d) %s\n", n + 1, reason[n]);
			getdata(12 + n, 0, "ÍË»ØÔ­Òò: ", buf, 60, DOECHO, YEA);
			if (buf[0] != '\0') {
				if (buf[0] >= '1' && buf[0] < '1' + n) {
					strcpy(buf, reason[buf[0] - '1']);
				}
				sprintf(genbuf, "<×¢²áÊ§°Ü>-%s", buf);
				strsncpy(uinfo.address, genbuf,
					 sizeof (uinfo.address));
				substitute_record(PASSFILE, &uinfo,
						  sizeof (uinfo), unum);
				mail_file("etc/f_fill", uinfo.userid,
					  uinfo.address);
				/* user_display( &uinfo, 1 ); */
				/* pressreturn(); */
				break;
			}
			move(10, 0);
			clrtobot();
			prints("È¡ÏûÍË»Ø´Ë×¢²áÉêÇë±í.\n");
		default://change default by bjgyt
			prints("ÒÔÏÂÊ¹ÓÃÕß×ÊÁÏÒÑ¾­¸üĞÂ:\n");
                        n = strlen(fdata[5]);
                        if (n + strlen(fdata[3]) > 60) {
                                if (n > 40)
                                        fdata[5][n = 40] = '\0';
                                fdata[3][60 - n] = '\0';
                        }
                        strsncpy(uinfo.realname, fdata[2],
                                 sizeof (uinfo.realname));
                        strsncpy(uinfo.address, fdata[4],
                                 sizeof (uinfo.address));
                        sprintf(genbuf, "%s$%s@%s", fdata[3], fdata[5], uid);
                        strsncpy(uinfo.realmail, genbuf,
                                 sizeof (uinfo.realmail));
                        uinfo.userlevel |= PERM_DEFAULT;        // by ylsdd
                        substitute_record(PASSFILE, &uinfo, sizeof (uinfo),
                                          unum);

                        sethomefile(buf, uinfo.userid, "sucessreg");
                        if ((fout = fopen(buf, "w")) != NULL) {
                                fprintf(fout, "\n");
                                fclose(fout);
                        }

                        sethomefile(buf, uinfo.userid, "register");
                        if (dashf(buf)) {
                                sethomefile(genbuf, uinfo.userid,
                                            "register.old");
                                rename(buf, genbuf);
                        }
                        if ((fout = fopen(buf, "w")) != NULL) {
                                for (n = 0; field[n] != NULL; n++)
                                        fprintf(fout, "%s: %s\n", field[n],
                                                fdata[n]);
                                n = time(NULL);
				fprintf(fout, "Date: %s",
                                        ctime((time_t *) & n));
                                fprintf(fout, "Approved: %s\n", uid);
                                fclose(fout);
                        }
                        mail_file("etc/s_fill", uinfo.userid,
                                  "¹§ìûÄúÍ¨¹ıÉí·İÑéÖ¤");

                        mail_file("etc/s_fill2", uinfo.userid,
                                  "»¶Ó­¼ÓÈë" MY_BBS_NAME "´ó¼ÒÍ¥");
                        sethomefile(buf, uinfo.userid, "mailcheck");
                        unlink(buf);
                        sprintf(genbuf, "ÈÃ %s Í¨¹ıÉí·ÖÈ·ÈÏ.", uinfo.userid);
                        securityreport(genbuf, genbuf);
                        break;
		}
		memset(fdata, 0, sizeof (fdata));
	}
	fclose(fn);
	unlink(fname);
	return (0);
}

int
m_register()
{
	FILE *fn;
	char ans[3], *fname;
	int x, y, wid, len;
	char uident[STRLEN];

	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();

	stand_title("Éè¶¨Ê¹ÓÃÕß×¢²á×ÊÁÏ(ÇëÊ¹ÓÃĞÂµÄÊµÃûÈÏÖ¤¹ÜÀíÑ¡µ¥)");
	for (;;) {
		getdata(1, 0,
			"[0]Àë¿ª [1]ÓÊÏä°ó¶¨²Ù×÷ [2]²éÑ¯Ê¹ÓÃÕß×¢²á×ÊÁÏ (Ä¬ÈÏ[2]):",
			ans, 2, DOECHO, YEA);
		if (ans[0] == '0')
			return 0;
		if (ans[0] == '\n' || ans[0] == '\0')
			ans[0] = '2';

		if (ans[0] == '1' || ans[0] == '2')
			break;

	}
	if (ans[0] == '1') {
		/* ÕâÊÇÒÔÇ°£¨Ã»ÓĞÓÊÏä°ó¶¨µÄÊ±ºò£©ÊÖ¶¯ÉóÅú×¢²áµ¥µÄ´úÂë
		fname = "new_register";
		if ((fn = fopen(fname, "r")) == NULL) {
			prints("\n\nÄ¿Ç°²¢ÎŞĞÂ×¢²á×ÊÁÏ.");
			pressreturn();
		} else {
			y = 3, x = wid = 0;
			while (fgets(genbuf, STRLEN, fn) != NULL && x < 65) {
				if (strncmp(genbuf, "userid: ", 8) == 0) {
					move(y++, x);
					prints(genbuf + 8);
					len = strlen(genbuf + 8);
					if (len > wid)
						wid = len;
					if (y >= t_lines - 2) {
						y = 3;
						x += wid + 2;
					}
				}
			}
			fclose(fn);
			if (askyn("Éè¶¨×ÊÁÏÂğ", NA, YEA) == YEA) {
				securityreport("Éè¶¨Ê¹ÓÃÕß×¢²á×ÊÁÏ",
					       "Éè¶¨Ê¹ÓÃÕß×¢²á×ÊÁÏ");
				scan_register_form(fname);
			}
		}
		*/
		clear();
		move(3, 0);
		prints("´Ë¹¦ÄÜÒÑ¾­·ÏÆú¡£ÇëÊ¹ÓÃĞÂµÄÊµÃûÈÏÖ¤¹ÜÀíÑ¡µ¥!");
		pressreturn();

		/*
		char userid[50];
		char userid2[50];
		char popserver[35];
		char popname[25];
		char email[50];
		char record[512];

		do{
		move(2, 0);
		prints("[×¢Òâ]BMY IDºÍÓÊÏäµÄpop·şÎñÆ÷ÓÃ»§ÃûÖÁÉÙÒªÌîĞ´Ò»¸ö!");
		getdata(4, 0, "ÇëÊäÈëBMY ID:", userid, 49, DOECHO, YEA);		
		getdata(5, 0, "ÇëÊäÈëpop·şÎñÆ÷ÓÃ»§Ãû:", popname, 49, DOECHO, YEA);
		getdata(6, 0, "ÇëÊäÈëpop·şÎñÆ÷µØÖ·[stu.xjtu.edu.cn]:", popserver, 49, DOECHO, YEA);
		strncpy(userid2, userid, 50);
		if (popserver[0] == '\0')
			strncpy(popserver, "stu.xjtu.edu.cn", 49);


		strncpy(email, popname, 24); strncat(email, "@", 2); strncat(email, popserver, 34);
		int isfind = select_mail(userid, email, record);	
		//move(5,0);
		if (isfind)
		{
			prints("ÕÒµ½ÈçÏÂ¼ÇÂ¼:\n");
			prints(record);
			getdata(11, 0, "ÊÇ·ñÉ¾³ıÕâÌõ¼ÇÂ¼£¿[Y or N]£¨Ä¬ÈÏ£ºN£©", ans, 2, DOECHO, YEA);
			if (ans[0] == 'y' || ans[0] == 'Y')
			{
				int ret = release_email(userid, email);
				switch(ret)
				{
					case 0: 
						prints("ÒÑ¾­É¾³ıÕâÌõ¼ÇÂ¼");
						char titbuf[64];
						sprintf(titbuf, "É¾³ıÓÊÏä¼ÇÂ¼: %s", email);
						sprintf(genbuf, "%s É¾³ıÓÊÏä¼ÇÂ¼\nUserId: %s\nMail: %s\n", currentuser.userid, userid, email);
						securityreport(titbuf, genbuf);
						break;
					case -1: 
						prints("ÓÊ¼ş¸ñÊ½ÓĞÎó£¬É¾³ıÊ§°Ü");
						break;
					case -2:
						prints("ÕÒ²»µ½pop_listÎÄ¼ş£¬É¾³ıÊ§°Ü");
						break;
					case -3: 
						prints("Ã»ÓĞÕÒµ½Õâ¸öÓÊ¼ş·şÎñÆ÷£¬É¾³ıÊ§°Ü");
						break;
					case -4: 
						prints("´ò¿ªÁÙÊ±ÎÄ¼şÊ§°Ü£¬É¾³ıÊ§°Ü");
						break;
					default:
						prints("·¢ÉúÎ´Öª´íÎó£¬É¾³ıÊ§°Ü");
						break;
				}
				
			}
			else
				prints("·ÅÆúÉ¾³ı");
		}
		else
		{
			prints("Ã»ÕÒµ½ÕâÌõ¼ÇÂ¼");
		}

		getdata(14, 0, "»¹Ïë¼ÌĞø²Ù×÷Âğ£¿[Y or N]£¨Ä¬ÈÏ£ºN£©", ans, 2, DOECHO, YEA);
		}
		while (ans[0] == 'y' || ans[0] == 'Y');
	*/

		//pressreturn();

	} else {
		move(1, 0);
		usercomplete("ÇëÊäÈëÒª²éÑ¯µÄ´úºÅ: ", uident);
		if (uident[0] != '\0') {
			if (!getuser(uident)) {
				move(2, 0);
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
			} else {
				sprintf(genbuf, "home/%c/%s/register",
					mytoupper(lookupuser.userid[0]),
					lookupuser.userid);
				if ((fn = fopen(genbuf, "r")) != NULL) {
					prints("\n×¢²á×ÊÁÏÈçÏÂ:\n\n");
					for (x = 1; x <= 15; x++) {
						if (fgets(genbuf, STRLEN, fn))
							prints("%s", genbuf);
						else
							break;
					}
					fclose(fn);
				} else
					prints("\n\nÕÒ²»µ½Ëû/ËıµÄ×¢²á×ÊÁÏ!!\n");
			}
		}
		pressanykey();
	}
	clear();
	return 0;
}

int
m_ordainBM()
{
	return do_ordainBM(NULL, NULL);
}

int
do_ordainBM(const char *userid, const char *abname)
{
	int id, pos, oldbm = 0, i, bm = 0, bigbm, bmpos, minpos, maxpos;
	struct boardheader fh;
	char bname[STRLEN], tmp[STRLEN], buf[5][STRLEN];
	char content[1024], title[STRLEN];
	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();
	stand_title("ÈÎÃü°æÖ÷\n");
	clrtoeol();
	move(2, 0);
	if (userid)
		strsncpy(genbuf, userid, sizeof (genbuf));
	else
		usercomplete("ÊäÈëÓûÈÎÃüµÄÊ¹ÓÃÕßÕÊºÅ: ", genbuf);
	if (genbuf[0] == '\0') {
		clear();
		return 0;
	}
	if (!(id = getuser(genbuf))) {
		move(4, 0);
		prints("ÎŞĞ§µÄÊ¹ÓÃÕßÕÊºÅ");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	if (abname)
		strsncpy(bname, abname, sizeof (bname));
	else {
		make_blist_full();
		namecomplete("ÊäÈë¸ÃÊ¹ÓÃÕß½«¹ÜÀíµÄÌÖÂÛÇøÃû³Æ: ", bname);
	}
	if (*bname == '\0') {
		move(5, 0);
		prints("´íÎóµÄÌÖÂÛÇøÃû³Æ");
		pressreturn();
		clear();
		return -1;
	}
	pos =
	    new_search_record(BOARDS, &fh, sizeof (fh), (void *) cmpbnames,
			      bname);
	if (!pos) {
		move(5, 0);
		prints("´íÎóµÄÌÖÂÛÇøÃû³Æ");
		pressreturn();
		clear();
		return -1;
	}
	oldbm = getbmnum(lookupuser.userid);
	if (oldbm >= 3 && strcmp(lookupuser.userid, "SYSOP")
	    && normal_board(bname)) {
		move(5, 0);
		prints(" %s ÒÑ¾­ÊÇ%d¸ö°æµÄ°æÖ÷ÁË", lookupuser.userid, oldbm);
		if (askyn("\nÒ»¶¨ÒªÈÎÃüÃ´? ", NA, NA) == NA){
			pressanykey();
			clear();
			return -1;
		}
	}
	if (askyn("ÈÎÃüÎª´ó°æÖ÷Ã´? (¿ÉÒÔÕûÀí¾«»ªÇø)", YEA, NA) == YEA) {
		bigbm = 1;
		minpos = 0;
		maxpos = 3;
	} else {
		bigbm = 0;
		minpos = 4;
		maxpos = BMNUM - 1;
	}
	bmpos = -1;
	for (i = 0; i < BMNUM; i++) {
		if (fh.bm[i][0] == 0 && (i >= minpos) && (i <= maxpos)
		    && (bmpos == -1)) {
			bmpos = i;
		}
		if (!strncmp(fh.bm[i], lookupuser.userid, IDLEN)) {
			prints(" %s ÒÑ¾­ÊÇ¸Ã°æ°æÖ÷", lookupuser.userid);
			pressanykey();
			clear();
			return -1;
		}
	}
	if (bmpos == -1) {
		prints(" %s Ã»ÓĞ¿ÕÓà°æÖ÷Î»ÖÃ", bname);
		pressanykey();
		clear();
		return -1;
	}
	prints("\nÄã½«ÈÎÃü %s Îª %s °æ°æÖ÷.\n", lookupuser.userid, bname);
	if (askyn("ÄãÈ·¶¨ÒªÈÎÃüÂğ?", YEA, NA) == NA) {
		prints("È¡ÏûÈÎÃü°æÖ÷");
		pressanykey();
		clear();
		return -1;
	}
	for (i = 0; i < 5; i++)
		buf[i][0] = '\0';
	move(12, 0);
	prints("ÇëÊäÈëÈÎÃü¸½ÑÔ(×î¶àÎåĞĞ£¬°´ Enter ½áÊø)");
	for (i = 0; i < 5; i++) {
		getdata(i + 13, 0, ": ", buf[i], STRLEN - 5, DOECHO, YEA);
		if (buf[i][0] == '\0')
			break;
	}

	if (!oldbm) {
		char secu[STRLEN];
		lookupuser.userlevel |= PERM_BOARDS;
		substitute_record(PASSFILE, &lookupuser, sizeof (struct userec),
				  id);
		sprintf(secu, "°æÖ÷ÈÎÃü, ¸øÓè %s µÄ°æÖ÷È¨ÏŞ",
			lookupuser.userid);
		securityreport(secu, secu);
		move(19, 0);
		mail_file("etc/bmhelp", lookupuser.userid, "°æÎñ²Ù×÷ÊÖ²á");
		mail_file("etc/backnumbers", lookupuser.userid, "¹ı¿¯Ê¹ÓÃËµÃ÷");
		prints(secu);
	}
	strncpy(fh.bm[bmpos], lookupuser.userid, IDLEN);
	fh.hiretime[bmpos] = time(NULL);
	if (bigbm) {
		anno_title(tmp, &fh);
		get_grp(fh.filename);
		edit_grp(fh.filename, lookgrp, fh.title, tmp);
	}
	substitute_record(BOARDS, &fh, sizeof (fh), pos);
	if (fh.clubnum) {
		char tmpb[30];
		strsncpy(tmpb, currboard, 30);
		strsncpy(currboard, fh.filename, 30);
		addclubmember(lookupuser.userid, fh.clubnum);
		strsncpy(currboard, tmpb, 30);
	}
	reload_boards();
	sprintf(genbuf, "ÈÎÃü %s Îª %s ÌÖÂÛÇø°æÖ÷", lookupuser.userid,
		fh.filename);
	securityreport(genbuf, genbuf);
	move(19, 0);
	prints("%s", genbuf);
	sprintf(title, "[¹«¸æ]ÈÎÃü%s °æ%s %s ", bname, (!bm) ? "°æÖ÷" : "°æ¸±",
		lookupuser.userid);
	if(strcmp(bname,"BM_exam")&&strcmp(bname,"BM_examII")&&strcmp(bname,"BM_examIII"))
		sprintf(content,
			"\n\t\t    ¡¾ °æÎñÈÎÃü¹«¸æ ¡¿\n\n\n" "\t  %s ÍøÓÑ£º\n\n"
			"\t      ¾­±¾Õ¾Õ¾Îñ×éÉóÅú¡¢¼ÍÂÉÎ¯Ô±»á¿¼ºËÍ¨¹ı£¬\n\t  ÏÖÕıÊ½ÈÎÃüÄãÎª %s °æ°æÎñ¡£\n\n"
			"\t      ÇëÔÚ 3 ÌìÖ®ÄÚÔÚ BM_home °æÃæ±¨µ½¡£\n",
			lookupuser.userid,
			bname);
	else
		sprintf(content,
			"\n\t\t    ¡¾ ÊµÏ°°æÎñÈÎÃü¹«¸æ ¡¿\n\n\n" "\t  %s ÍøÓÑ£º\n\n"
			"\t      ¾­±¾Õ¾Õ¾Îñ×éÉóÅú¡¢ ÏÖÈÎÃüÄãÎª %s °æÊµÏ°°æÎñ¡£\n\n"
			"\t      ÇëÓÚÈıÌìµÄÅàÑµÆÚÄÚÊìÏ¤°æÎñ¿¼ÊÔÏà¹ØÖªÊ¶£¬\n\n\t      ¼°Ê±ÁªÏµ¼ÍÎ¯²Î¼Ó¿¼ÊÔ¡£\n",
			lookupuser.userid,
			bname);
	for (i = 0; i < 5; i++) {
		if (buf[i][0] == '\0')
			break;
		if (i == 0)
			strcat(content, "\n\nÈÎÃü¸½ÑÔ£º\n");
		strcat(content, buf[i]);
		strcat(content, "\n");
	}
	strcpy(currboard, bname);
	deliverreport(title, content);
	if (normal_board(bname)) {
		strcpy(currboard, "Board");
		deliverreport(title, content);
	}
	pressanykey();
	return 0;
}

int
m_retireBM()
{
	return do_retireBM(NULL, NULL);
}

int
do_retireBM(const char *userid, const char *abname)
{
	int id, pos, bmpos, right = 0, oldbm = 0, i;
	int bm = 1;
	struct boardheader fh;
	char buf[5][STRLEN];
	char bname[STRLEN];
	char content[1024], title[STRLEN];
	char tmp[STRLEN];
	modify_user_mode(ADMIN);
	if (!check_systempasswd())
		return -1;

	clear();
	stand_title("°æÖ÷ÀëÖ°\n");
	clrtoeol();
	if (userid)
		strsncpy(genbuf, userid, sizeof (genbuf));
	else
		usercomplete("ÊäÈëÓûÀëÈÎµÄÊ¹ÓÃÕßÕÊºÅ: ", genbuf);
	if (genbuf[0] == '\0') {
		clear();
		return 0;
	}
	if (!(id = getuser(genbuf))) {
		move(4, 0);
		prints("ÎŞĞ§µÄÊ¹ÓÃÕßÕÊºÅ");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	if (abname)
		strsncpy(bname, abname, sizeof (bname));
	else {
		make_blist_full();
		namecomplete("ÊäÈë¸ÃÊ¹ÓÃÕß½«¹ÜÀíµÄÌÖÂÛÇøÃû³Æ: ", bname);
	}
	if (*bname == '\0') {
		move(5, 0);
		prints("´íÎóµÄÌÖÂÛÇøÃû³Æ");
		pressreturn();
		clear();
		return -1;
	}
	pos =
	    new_search_record(BOARDS, &fh, sizeof (fh), (void *) cmpbnames,
			      bname);
	if (!pos) {
		move(5, 0);
		prints("´íÎóµÄÌÖÂÛÇøÃû³Æ");
		pressreturn();
		clear();
		return -1;
	}
	bmpos = -1;
	for (i = 0; i < BMNUM; i++) {
		if (!strcasecmp(fh.bm[i], lookupuser.userid)) {
			bmpos = i;
			if (i < 4)
				bm = 1;
			else
				bm = 0;
		}
	}

	oldbm = getbmnum(lookupuser.userid);
	if (bmpos == -1) {
		move(5, 0);
		prints(" °æÖ÷Ãûµ¥ÖĞÃ»ÓĞ%s£¬ÈçÓĞ´íÎó£¬ÇëÍ¨ÖªÏµÍ³Î¬»¤¡£",
		       lookupuser.userid);
		pressanykey();
		clear();
		return -1;
	}
	prints("\nÄã½«È¡Ïû %s µÄ %s °æ%s°æÖ°Îñ.\n",
	       lookupuser.userid, bname, bm ? "´ó" : "");
	if (askyn("ÄãÈ·¶¨ÒªÈ¡ÏûËûµÄ¸Ã°æ°æÖ÷Ö°ÎñÂğ?", YEA, NA) == NA) {
		prints("\nºÇºÇ£¬Äã¸Ä±äĞÄÒâÁË£¿ %s ¼ÌĞøÁôÈÎ %s °æ°æÖ÷Ö°Îñ£¡",
		       lookupuser.userid, bname);
		pressanykey();
		clear();
		return -1;
	}
	anno_title(title, &fh);
	fh.bm[bmpos][0] = 0;	//ÏÈÇåÀíµô, ÃâµÄÓĞÎÊÌâ
	fh.hiretime[bmpos] = 0;
	for (i = bmpos; i < (bm ? 4 : BMNUM); i++) {
		if (i == (bm ? 3 : BMNUM - 1)) {	//×îºóÒ»¸öBM
			fh.bm[i][0] = 0;
			fh.hiretime[i] = 0;
		} else {
			strcpy(fh.bm[i], fh.bm[i + 1]);
			fh.bm[i][strlen(fh.bm[i + 1])] = 0;
			fh.hiretime[i] = fh.hiretime[i + 1];
		}
	}
	if (bm) {
		anno_title(tmp, &fh);
		get_grp(fh.filename);
		edit_grp(fh.filename, lookgrp, title, tmp);
	}
	substitute_record(BOARDS, &fh, sizeof (fh), pos);
	reload_boards();
	sprintf(genbuf, "È¡Ïû %s µÄ %s ÌÖÂÛÇø°æÖ÷Ö°Îñ", lookupuser.userid,
		fh.filename);
	securityreport(genbuf, genbuf);
	move(8, 0);
	prints("%s", genbuf);
	if (oldbm == 1 || oldbm == 0) {
		char secu[STRLEN];
		if (!(lookupuser.userlevel & PERM_OBOARDS)
		    && !(lookupuser.userlevel & PERM_SYSOP)) {
			lookupuser.userlevel &= ~PERM_BOARDS;
			substitute_record(PASSFILE, &lookupuser,
					  sizeof (struct userec), id);
			sprintf(secu, "°æÖ÷Ğ¶Ö°, È¡Ïû %s µÄ°æÖ÷È¨ÏŞ",
				lookupuser.userid);
			securityreport(secu, secu);
			move(9, 0);
			prints(secu);
		}
	}
	prints("\n\n");
	if (askyn("ĞèÒªÔÚÏà¹Ø°æÃæ·¢ËÍÍ¨¸æÂğ?", YEA, NA) == NA) {
		pressanykey();
		return 0;
	}
	prints("\n");
	if (askyn("Õı³£ÀëÈÎÇë°´ Enter ¼üÈ·ÈÏ£¬³·Ö°³Í·£°´ N ¼ü", YEA, NA) == YEA)
		right = 1;
	else
		right = 0;
	if (right)
		sprintf(title, "[¹«¸æ]%s °æ%s %s ÀëÈÎ", bname,
			bm ? "´ó°æÖ÷" : "°æÖ÷", lookupuser.userid);
	else
		sprintf(title, "[¹«¸æ]³·³ı %s °æ%s %s ", bname,
			bm ? "´ó°æÖ÷" : "°æÖ÷", lookupuser.userid);
	strcpy(currboard, bname);
	if (right) {
		sprintf(content, "\n\t\t\t¡¾ ¹«¸æ ¡¿\n\n"
			"\t¾­Õ¾Îñ×éÌÖÂÛ£º\n"
			"\tÍ¬Òâ %s ´ÇÈ¥ %s °æµÄ%sÖ°Îñ¡£\n"
			"\tÔÚ´Ë£¬¶ÔËûÔø¾­ÔÚ %s °æµÄĞÁ¿àÀÍ×÷±íÊ¾¸ĞĞ»¡£\n\n"
			"\tÏ£Íû½ñºóÒ²ÄÜÖ§³Ö±¾°æµÄ¹¤×÷.",
			lookupuser.userid, bname, bm ? "´ó°æÖ÷" : "°æÖ÷",
			bname);
	} else {
		sprintf(content, "\n\t\t\t¡¾³·Ö°¹«¸æ¡¿\n\n"
			"\t¾­Õ¾Îñ×éÌÖÂÛ¾ö¶¨£º\n"
			"\t³·³ı %s °æ%s %s µÄ%sÖ°Îñ¡£\n",
			bname, bm ? "´ó°æÖ÷" : "°æÖ÷", lookupuser.userid,
			bm ? "´ó°æÖ÷" : "°æÖ÷");
	}
	for (i = 0; i < 5; i++)
		buf[i][0] = '\0';
	move(14, 0);
	prints("ÇëÊäÈë%s¸½ÑÔ(×î¶àÎåĞĞ£¬°´ Enter ½áÊø)",
	       right ? "°æÖ÷ÀëÈÎ" : "°æÖ÷³·Ö°");
	for (i = 0; i < 5; i++) {
		getdata(i + 15, 0, ": ", buf[i], STRLEN - 5, DOECHO, YEA);
		if (buf[i][0] == '\0')
			break;
		if (i == 0)
			strcat(content,
			       right ? "\n\nÀëÈÎ¸½ÑÔ£º\n" : "\n\n³·Ö°ËµÃ÷£º\n");
		strcat(content, buf[i]);
		strcat(content, "\n");
	}
	deliverreport(title, content);
	if (normal_board(currboard)) {
		strcpy(currboard, "Board");
		deliverreport(title, content);
	}
	prints("\nÖ´ĞĞÍê±Ï£¡");
	pressanykey();
	return 0;
}

int
retireBM(uid, bname)
char *uid;
char *bname;
{
	char tmp[STRLEN];
	char content[1024], title[STRLEN];
	int i, oldbm, id, pos, bmpos = -1, bm = 0;
	struct boardheader fh;
	if (!(id = getuser(uid)))
		return -1;
	pos =
	    new_search_record(BOARDS, &fh, sizeof (fh), (void *) cmpbnames,
			      bname);
	if (!pos)
		return -2;
	oldbm = getbmnum(lookupuser.userid);
	for (i = 0; i < BMNUM; i++) {
		if (!strcasecmp(fh.bm[i], lookupuser.userid)) {
			bmpos = i;
			if (i < 4)
				bm = 1;
			else
				bm = 0;
		}
	}
	if (bmpos == -1)
		return -3;
	anno_title(title, &fh);
	fh.bm[bmpos][0] = 0;	//ÏÈÇåÀíµô, ÃâµÄÓĞÎÊÌâ
	fh.hiretime[bmpos] = 0;
	for (i = bmpos; i < (bm ? 4 : BMNUM); i++) {
		if (i == bm ? 3 : BMNUM - 1) {	//×îºóÒ»¸öBM
			fh.bm[i][0] = 0;
			fh.hiretime[i] = 0;
		} else {
			strcpy(fh.bm[i], fh.bm[i + 1]);
			fh.hiretime[i] = fh.hiretime[i + 1];
		}
	}
	if (bm) {
		anno_title(tmp, &fh);
		get_grp(fh.filename);
		edit_grp(fh.filename, lookgrp, title, tmp);
	}
	substitute_record(BOARDS, &fh, sizeof (fh), pos);
	reload_boards();
	sprintf(genbuf, "È¡Ïû %s µÄ %s ÌÖÂÛÇø°æÖ÷Ö°Îñ", lookupuser.userid,
		fh.filename);
	securityreport(genbuf, genbuf);
	if (!(oldbm - 1)) {
		char secu[STRLEN];
		if (!(lookupuser.userlevel & PERM_OBOARDS)
		    && !(lookupuser.userlevel & PERM_SYSOP)) {
			lookupuser.userlevel &= ~PERM_BOARDS;
			substitute_record(PASSFILE, &lookupuser,
					  sizeof (struct userec), id);
			sprintf(secu, "°æÖ÷Ğ¶Ö°, È¡Ïû %s µÄ°æÖ÷È¨ÏŞ",
				lookupuser.userid);
			securityreport(secu, secu);
		}
	}
	sprintf(title, "[¹«¸æ]³·³ı %s °æ%s %s ", bname,
		bm ? "°æÖ÷" : "°æ¸±", lookupuser.userid);
	strcpy(currboard, bname);
	sprintf(content, "\n\t\t\t¡¾³·Ö°¹«¸æ¡¿\n\n"
		"\tÏµÍ³³·Ö°£º\n"
		"\tÓÉÓÚIDËÀÍö£¬³·³ı %s °æ%s %s µÄ%sÖ°Îñ¡£\n",
		bname, bm ? "°æÖ÷" : "°æ¸±", lookupuser.userid,
		bm ? "°æÖ÷" : "°æ¸±");
	deliverreport(title, content);
	if (normal_board(currboard)) {
		strcpy(currboard, "Board");
		deliverreport(title, content);
	}
	return 0;
}

int
retire_allBM(uid)
char *uid;
{
	struct boardheader bh;
	int fd, size;
	size = sizeof (bh);
	if ((fd = open(BOARDS, O_RDONLY, 0)) == -1)
		return -1;
	while (read(fd, &bh, size) > 0)
		retireBM(uid, bh.filename);
	close(fd);
	return 0;
}

int
m_addpersonal()
{
	FILE *fn;
	char digestpath[80] = "0Announce/groups/GROUP_0/PersonalCorpus";
	char personalpath[80], title[STRLEN];
	char firstchar[1];
	int id;
	modify_user_mode(DIGEST);
	if (!check_systempasswd()) {
		return 1;
	}
	clear();
	if (!dashd(digestpath)) {		//add by mintbaggio@BMY
		prints("ÇëÏÈ½¨Á¢¸öÈËÎÄ¼¯ÌÖÂÛÇø£ºPersonal_Corpus, Â·¾¶ÊÇ:%s", digestpath);
		pressanykey();
		return 1;
	}
	stand_title("´´½¨¸öÈËÎÄ¼¯");
	clrtoeol();
	move(2, 0);
	usercomplete("ÇëÊäÈëÊ¹ÓÃÕß´úºÅ: ", genbuf);
	if (*genbuf == '\0') {
		clear();
		return 1;
	}
	if (!(id = getuser(genbuf))) {
		prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ");
		clrtoeol();
		pressreturn();
		clear();
		return 1;
	}
	if (!isalpha(lookupuser.userid[0])) {
		getdata(3, 0, "·ÇÓ¢ÎÄID£¬ÇëÊäÈëÆ´ÒôÊ××ÖÄ¸:", firstchar, 2,
			DOECHO, YEA);
	} else
		firstchar[0] = lookupuser.userid[0];
	printf("%c", firstchar[0]);
	sprintf(personalpath, "%s/%c", digestpath, firstchar[0]);	//add by mintbaggio@BMY
	if (!dashd(personalpath)) {		//add by mintbaggio@BMY
		mkdir(personalpath, 0755);
		sprintf(personalpath, "%s/.Names", personalpath);
		if ((fn = fopen(personalpath, "w")) == NULL) {
			return -1;
		}
		fprintf(fn, "#\n");
		fprintf(fn, "# Title=%s\n", firstchar);
		fprintf(fn, "#\n");
		fclose(fn);
		linkto(digestpath, firstchar, firstchar);
		sprintf(personalpath, "%s/%c", digestpath, firstchar[0]);
	}
	sprintf(personalpath, "%s/%c/%s", digestpath,
		toupper(firstchar[0]), lookupuser.userid);
	if (dashd(personalpath)) {
		prints("¸ÃÓÃ»§µÄ¸öÈËÎÄ¼¯Ä¿Â¼ÒÑ´æÔÚ, °´ÈÎÒâ¼üÈ¡Ïû..");
		pressanykey();
		return 1;
	}
	if (lookupuser.stay / 60 / 60 < 24) {
		prints("¸ÃÓÃ»§ÉÏÕ¾Ê±¼ä²»¹»,ÎŞ·¨ÉêÇë¸öÈËÎÄ¼¯, °´ÈÎÒâ¼üÈ¡Ïû..");
		pressanykey();
		return 1;
	}
	move(4, 0);
	if (askyn("È·¶¨ÒªÎª¸ÃÓÃ»§´´½¨Ò»¸ö¸öÈËÎÄ¼¯Âğ?", YEA, NA) == NA) {
		prints("ÄãÑ¡ÔñÈ¡Ïû´´½¨. °´ÈÎÒâ¼üÈ¡Ïû...");
		pressanykey();
		return 1;
	}
	mkdir(personalpath, 0755);
	chmod(personalpath, 0755);

	move(7, 0);
	prints("[Ö±½Ó°´ ENTER ¼ü, Ôò±êÌâÈ±Ê¡Îª: [32m%sÎÄ¼¯[m]",
	       lookupuser.userid);
	getdata(6, 0, "ÇëÊäÈë¸öÈËÎÄ¼¯Ö®±êÌâ: ", genbuf, 39, DOECHO, YEA);
	if (genbuf[0] == '\0')
		sprintf(title, "%sÎÄ¼¯", lookupuser.userid);
	else
		sprintf(title, "%sÎÄ¼¯¡ª¡ª%s", lookupuser.userid, genbuf);
	sprintf(title, "%-38.38s(BM: %s _Personal)", title, lookupuser.userid);
	//by bjgyt sprintf(title, "%-38.38s(BM: %s)", title, lookupuser.userid);
	sprintf(digestpath, "%s/%c", digestpath, toupper(firstchar[0]));
	linkto(digestpath, lookupuser.userid, title);
	sprintf(personalpath, "%s/.Names", personalpath);
	if ((fn = fopen(personalpath, "w")) == NULL) {
		return -1;
	}
	fprintf(fn, "#\n");
	fprintf(fn, "# Title=%s\n", title);
	fprintf(fn, "#\n");
	fclose(fn);
	if (!(lookupuser.userlevel & PERM_SPECIAL8)) {
		char secu[STRLEN];
		lookupuser.userlevel |= PERM_SPECIAL8;
		substitute_record(PASSFILE, &lookupuser, sizeof (struct userec),
				  id);
		sprintf(secu, "´´½¨¸öÈËÎÄ¼¯, ¸øÓè %s ÎÄ¼¯¹ÜÀíÈ¨ÏŞ",
			lookupuser.userid);
		securityreport(secu, secu);
		move(10, 0);
		prints(secu);

	}
	prints("ÒÑ¾­´´½¨¸öÈËÎÄ¼¯, Çë°´ÈÎÒâ¼ü¼ÌĞø...");
	pressanykey();
	return 0;
}
