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
#include "xyz.h"
#include "smth_screen.h"
#include "stuff.h"
#include "maintain.h"
#include "io.h"
#include "namecomplete.h"
#include "bcache.h"
#include "talk.h"
#include "main.h"
#include "mail.h"
#include "announce.h"
#include "bbsinc.h"
#include "boards.h"

static int d_board() {
	struct boardheader binfo;
	int bid, ans;
	char bname[STRLEN];
	extern char lookgrp[];

	if (!HAS_PERM(PERM_BLEVELS)) {
		return -1;
	}
	modify_user_mode(ADMIN);
	if (!check_systempasswd()) {
		return -1;
	}
	clear();
	stand_title("É¾³ıÌÖÂÛÇø");
	make_blist_full();
	move(1, 0);
	namecomplete("ÇëÊäÈëÌÖÂÛÇø: ", bname);
	if (bname[0] == '\0')
		return -1;
	bid = getbnum(bname);
	if (get_record(BOARDS, &binfo, sizeof (binfo), bid) == -1) {
		move(2, 0);
		prints("²»ÕıÈ·µÄÌÖÂÛÇø\n");
		pressreturn();
		clear();
		return -1;
	}
	ans = askyn("ÄãÈ·¶¨ÒªÉ¾³ıÕâ¸öÌÖÂÛÇø", NA, NA);
	if (ans != 1) {
		move(2, 0);
		prints("È¡ÏûÉ¾³ıĞĞ¶¯\n");
		pressreturn();
		clear();
		return -1;
	}
	{
		char secu[STRLEN];
		sprintf(secu, "É¾³ıÌÖÂÛÇø£º%s", binfo.filename);
		securityreport(secu, secu);
	}
	if (seek_in_file("0Announce/.Search", bname)) {
		move(4, 0);
		if (askyn("ÒÆ³ı¾«»ªÇø", NA, NA) == YEA) {
			get_grp(binfo.filename);
			del_grp(lookgrp, binfo.filename, binfo.title);
		}
	}
	if (seek_in_file("etc/junkboards", bname))
		ytht_del_from_file("etc/junkboards", bname, true);
	if (seek_in_file("0Announce/.Search", bname))
		ytht_del_from_file("0Announce/.Search", bname, true);

	if (binfo.filename[0] == '\0')
		return -1;	/* rrr - precaution */
	sprintf(genbuf, "boards/%s", binfo.filename);
	deltree(genbuf);
	sprintf(genbuf, "vote/%s", binfo.filename);
	deltree(genbuf);
	sprintf(genbuf, " << '%s'±» %s É¾³ı >>",
			binfo.filename, currentuser.userid);
	memset(&binfo, 0, sizeof (binfo));
	ytht_strsncpy(binfo.title, genbuf, sizeof(binfo.title));
	binfo.level = PERM_SYSOP;
	substitute_record(BOARDS, &binfo, sizeof (binfo), bid);

	reload_boards();
	update_postboards();

	move(4, 0);
	prints("\n±¾ÌÖÂÛÇøÒÑ¾­É¾³ı...\n");
	pressreturn();
	clear();
	return 0;
}

void
offline()
{
	char buf[STRLEN];

	modify_user_mode(OFFLINE);
	clear();
	if (HAS_PERM(PERM_SYSOP) || HAS_PERM(PERM_BOARDS) || HAS_PERM(PERM_ADMINMENU) || HAS_PERM(PERM_SEEULEVELS)) {
		move(1, 0);
		prints("\n\nÄúÓĞÖØÈÎÔÚÉí, ²»ÄÜËæ±ã×ÔÉ±À²!!\n");
		pressreturn();
		clear();
		return;
	}
	if (currentuser.stay < 86400) {
		move(1, 0);

		prints("\n\n¶Ô²»Æğ, Äú»¹Î´¹»×Ê¸ñÖ´ĞĞ´ËÃüÁî!!\n");
		prints("Ö»ÓĞÉÏÕ¾Ê±¼ä³¬¹ı24Ğ¡Ê±µÄÓÃ»§²ÅÄÜ×ÔÉ±.\n");
		pressreturn();
		clear();
		return;
	}

	getdata(1, 0, "ÇëÊäÈëÄãµÄÃÜÂë: ", buf, PASSLEN, NOECHO, YEA);
	if (*buf == '\0' || !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
		prints("\n\nºÜ±§Ç¸, ÄúÊäÈëµÄÃÜÂë²»ÕıÈ·¡£\n");
		pressreturn();
		clear();
		return;
	}
	getdata(3, 0, "ÇëÎÊÄã½ĞÊ²Ã´Ãû×Ö? ", buf, NAMELEN, DOECHO, YEA);
	if (*buf == '\0' || strcmp(buf, currentuser.realname)) {
		prints("\n\nºÜ±§Ç¸, ÎÒ²¢²»ÈÏÊ¶Äã¡£\n");
		pressreturn();
		clear();
		return;
	}
	clear();
	move(1, 0);
	prints("\033[1;5;31m¾¯¸æ\033[0;1;31m ×ÔÉ±ºÍidËÀÍöÊÍ·ÅÓÊÏäÃ»ÓĞÈÎºÎ¹ØÏµ\033[m \n");
	prints("×ÔÉ±½ö½öÊÇÈÃÄúµÄidÎŞ·¨Õı³£Ê¹ÓÃ£¬¶øidµÄËÀÍöÖ»ºÍÉúÃüÁ¦ÓĞ¹ØÏµ\n");
	prints("Èç¹ûÓĞÈË¸æËßÄã×ÔÉ±¿ÉÒÔ°ïÖúÄúÈÃÄúÏÖÓĞµÄidËÀÍö²¢×¢²áĞÂµÄid£¬ÇëÀë¿ªÕâÀï\n");
	prints("È»ºó°Ñ¸æËßÄãµÄÈË´òÒ»¶Ù\n\n");
	prints("\033[1;5;31m¾¯¸æ\033[0;1;31m£º ×ÔÉ±ºó, ÄúµÄÁé»ê½«ÉıÈëÌì¹ú»ò¶éÈëµØÓü, Ô¸Äú°²Ï¢");
	prints("\n\n\n\033[1;32mºÃÄÑ¹ıà¸.....\033[m\n\n\n");
	if (askyn("ÄãÈ·¶¨ÒªÀë¿ªÕâ¸ö´ó¼ÒÍ¥", NA, NA) == 1) {
		clear();
		set_safe_record();
		currentuser.dietime = currentuser.stay + 6*18 * 24 * 60 * 60;//¸Ä×ÔÉ±»Ö¸´Îª6*18Ìì   ÁùµÀÂÖ»Ø+18²ãµØÓü
		substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
		Q_Goodbye();
		return;
		/*if(d_user(currentuser.userid)==1)
		  {
		  mail_info();
		  modify_user_mode( OFFLINE );
		  kick_user(&uinfo);
		  exit(0);
		  }
		  */
	}
}

int
online()
{
	char buf[STRLEN];
	struct tm *nowtime;
	time_t nowtimeins;
	modify_user_mode(OFFLINE);
	clear();
	nowtimeins = time(NULL);
	nowtime = localtime(&nowtimeins);
	if ((currentuser.stay <= currentuser.dietime)) {
		move(1, 0);
		prints("\n\nËÀÆÚÎ´Âú°¡!!\n");
		prints("ÄãÒªÏÅËÀÈË°¡!\n");
		prints("ÄãµÄËÀÆÚ»¹ÓĞ %d ·ÖÖÓ", 1 + (currentuser.dietime - currentuser.stay) / 60);
		pressreturn();
		clear();
		return -1;
	}
	getdata(1, 0, "ÇëÊäÈëÄãµÄÃÜÂë: ", buf, PASSLEN, NOECHO, YEA);
	if (*buf == '\0' || !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
		prints("\n\nºÜ±§Ç¸, ÄúÊäÈëµÄÃÜÂë²»ÕıÈ·¡£\n");
		pressreturn();
		clear();
		return -2;
	}
	getdata(3, 0, "ÇëÎÊÄã½ĞÊ²Ã´Ãû×Ö? ", buf, NAMELEN, DOECHO, YEA);
	if (*buf == '\0' || strcmp(buf, currentuser.realname)) {
		prints("\n\nºÜ±§Ç¸, ÎÒ²¢²»ÈÏÊ¶Äã¡£\n");
		pressreturn();
		clear();
		return -3;
	}
	clear();
	move(1, 0);
	prints("\033[1;5;31m¾¯¸æ\033[0;1;31m£º Éú´æ,»¹ÊÇ»ÙÃğ,ÊÇ¸öÖµµÃ¿¼ÂÇµÄÎÊÌâ");
	prints("\n\n\n\033[1;32mÄú¿ÉÒªÏëÇå³ş.....\033[m\n\n\n");
	if (askyn("ÄãÈ·¶¨Òª·µÊÀÎªÈËÁËÂğ?", NA, NA) == 1) {
		clear();
		currentuser.dietime = 0;
		substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
		return Q_Goodbye();
	}
	return 0;
}

static void getuinfo(FILE *fn) {
	int num;
	char buf[40];

	fprintf(fn, "\n\nËûµÄ´úºÅ     : %s\n", currentuser.userid);
	fprintf(fn, "ËûµÄêÇ³Æ     : %s\n", currentuser.username);
	fprintf(fn, "ÕæÊµĞÕÃû     : %s\n", currentuser.realname);
	fprintf(fn, "¾Ó×¡×¡Ö·     : %s\n", currentuser.address);
	fprintf(fn, "µç×ÓÓÊ¼şĞÅÏä : %s\n", currentuser.email);
	fprintf(fn, "ÕæÊµ E-mail  : %s\n", currentuser.realmail);
	//fprintf(fn,"Ident ×ÊÁÏ   : %s\n", currentuser.ident);
	fprintf(fn, "ÓòÃûÖ¸Ïò     : %s\n", currentuser.ip);
	fprintf(fn, "ÕÊºÅ½¨Á¢ÈÕÆÚ : %s", ctime(&currentuser.firstlogin));
	fprintf(fn, "×î½ü¹âÁÙÈÕÆÚ : %s", ctime(&currentuser.lastlogin));
	fprintf(fn, "×î½ü¹âÁÙ»úÆ÷ : %s\n", currentuser.lasthost);
	fprintf(fn, "×î½üÀëÕ¾Ê±¼ä : %s", ctime(&currentuser.lastlogout));
	fprintf(fn, "ÉÏÕ¾´ÎÊı     : %d ´Î\n", currentuser.numlogins);
	fprintf(fn, "ÎÄÕÂÊıÄ¿     : %d\n", currentuser.numposts);
	fprintf(fn, "ÉÏÕ¾×ÜÊ±Êı   : %ld Ğ¡Ê± %ld ·ÖÖÓ\n",
			(long int) (currentuser.stay / 3600),
			(long int) ((currentuser.stay / 60) % 60));
	strcpy(buf, "bTCPRp#@XWBA#VS-DOM-F012345678");
	for (num = 0; num < 30; num++)
		if (!(currentuser.userlevel & (1 << num)))
			buf[num] = '-';
	buf[num] = '\0';
	fprintf(fn, "Ê¹ÓÃÕßÈ¨ÏŞ   : %s\n\n", buf);
}

#if 0
	static void
mail_info()
{
	FILE *fn;
	time_t now;
	char filename[STRLEN];

	now = time(0);
	sprintf(filename, "tmp/suicide.%s", currentuser.userid);
	if ((fn = fopen(filename, "w")) != NULL) {
		fprintf(fn,
				"[1m%s[m ÒÑ¾­ÔÚ [1m%24.24s[m ×ÔÉ±ÁË£¬ÒÔÏÂÊÇËûµÄ×ÊÁÏ£¬Çë±£Áô...",
				currentuser.userid, ctime(&now));
		getuinfo(fn);
		fprintf(fn,
				"\n                      [1m ÏµÍ³×Ô¶¯·¢ĞÅÏµÍ³Áô[m\n\n");
		fclose(fn);
		postfile(filename, "syssecurity", "×ÔÉ±Í¨Öª....", 2);
		unlink(filename);
	}
	if ((fn = fopen(filename, "w")) != NULL) {
		fprintf(fn, "´ó¼ÒºÃ,\n\n");
		fprintf(fn, "ÎÒÊÇ %s (%s)¡£ ÎÒÒÑ¾­Àë¿ªÕâÀïÁË¡£\n\n",
				currentuser.userid, currentuser.username);
		fprintf(fn,
				"ÎÒ²»»á¸ü²»¿ÉÄÜÍü¼Ç×Ô %sÒÔÀ´ÎÒÔÚ±¾Õ¾ %d ´Î login ÖĞ×Ü¹² %d ·ÖÖÓ¶ºÁôÆÚ¼äµÄµãµãµÎµÎ¡£\n",
				ctime(&currentuser.firstlogin), currentuser.numlogins,
				(int) currentuser.stay / 60);
		fprintf(fn, "ÇëÎÒµÄºÃÓÑ°Ñ %s ´ÓÄãÃÇµÄºÃÓÑÃûµ¥ÖĞÄÃµô°É¡£\n\n",
				currentuser.userid);
		fprintf(fn, "»òĞíÓĞ³¯Ò»ÈÕÎÒ»á»ØÀ´µÄ¡£ ÕäÖØ!! ÔÙ¼û!!\n\n\n");
		fprintf(fn, "%s ÓÚ %24.24s Áô.\n\n", currentuser.userid,
				ctime(&now));
		fclose(fn);
		postfile(filename, "notepad", "×ÔÉ±ÁôÑÔ....", 2);
		unlink(filename);
	}
}
#endif
static int d_user(char *cid) {
	int id;

	if (uinfo.mode != OFFLINE) {
		modify_user_mode(ADMIN);
		if (!check_systempasswd()) {
			return -1;
		}
		clear();
		stand_title("É¾³ıÊ¹ÓÃÕßÕÊºÅ");
		move(1, 0);
		usercomplete("ÇëÊäÈëÓûÉ¾³ıµÄÊ¹ÓÃÕß´úºÅ: ", genbuf);
		if (*genbuf == '\0') {
			clear();
			return 0;
		}
	} else
		strcpy(genbuf, cid);
	if (!(id = getuser(genbuf))) {
		move(3, 0);
		prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	/*    if (!isalpha(lookupuser.userid[0])) return 0;*/
	/* rrr - don't know how... */
	move(1, 0);
	if (uinfo.mode != OFFLINE)
		prints("É¾³ıÊ¹ÓÃÕß '%s'.", genbuf);
	else
		prints(" %s ½«Àë¿ªÕâÀï", cid);
	clrtoeol();
	getdata(2, 0, "(Yes, or No) [N]: ", genbuf, 4, DOECHO, YEA);
	if (genbuf[0] != 'Y' && genbuf[0] != 'y') {	/* if not yes quit */
		move(2, 0);
		if (uinfo.mode != OFFLINE)
			prints("È¡ÏûÉ¾³ıÊ¹ÓÃÕß...\n");
		else
			prints("ÄãÖÕÓÚ»ØĞÄ×ªÒâÁË£¬ºÃ¸ßĞËà¸...");
		pressreturn();
		clear();
		return 0;
	}
	if (lookupuser.userid[0] == '\0' || !strcmp(lookupuser.userid, "SYSOP")) {
		prints("ÎŞ·¨É¾³ı!!\n");
		pressreturn();
		clear();
		return 0;
	}
	if (uinfo.mode != OFFLINE) {
		char secu[STRLEN];
		sprintf(secu, "É¾³ıÊ¹ÓÃÕß£º%s", lookupuser.userid);
		securityreport(secu, secu);
	}
	sprintf(genbuf, "mail/%c/%s", mytoupper(lookupuser.userid[0]), lookupuser.userid);
	deltree(genbuf);
	sprintf(genbuf, "home/%c/%s", mytoupper(lookupuser.userid[0]), lookupuser.userid);
	deltree(genbuf);
	lookupuser.userlevel = 0;
	strcpy(lookupuser.address, "");
	strcpy(lookupuser.username, "");
	strcpy(lookupuser.realname, "");
	strcpy(lookupuser.ip, "");
	strcpy(lookupuser.realmail, "");
	lookupuser.userid[0] = '\0';
	substitute_record(PASSFILE, &lookupuser, sizeof (lookupuser), id);
	setuserid(id, lookupuser.userid);
	move(2, 0);
	prints("%s ÒÑ¾­±»Ãğ¾øÁË...\n", lookupuser.userid);
	pressreturn();

	clear();
	return 1;
}

int
kick_user(struct user_info *userinfo, int mode)
{
	int id, ind;
	struct user_info uin;
	struct userec kuinfo;
	char kickuser[40];
	char kickreason[STRLEN];
	char titlebuf[STRLEN];
	char contentbuf[STRLEN];
	char repbuf[STRLEN];
	char msgbuf[STRLEN];
	if (uinfo.mode != LUSERS && uinfo.mode != OFFLINE && uinfo.mode != FRIEND) {
		modify_user_mode(ADMIN);
		stand_title("Kick User");
		move(1, 0);
		usercomplete("Enter userid to be kicked: ", kickuser);
		if (*kickuser == '\0') {
			clear();
			return 0;
		}
		if (!(id = getuser(kickuser))) {
			move(3, 0);
			prints("Invalid User Id");
			clrtoeol();
			pressreturn();
			clear();
			return 0;
		}
		move(1, 0);
		prints("Kick User '%s'.", kickuser);
		clrtoeol();
		getdata(2, 0, "(Yes, or No) [N]: ", genbuf, 4, DOECHO, YEA);
		if (genbuf[0] != 'Y' && genbuf[0] != 'y') {	/* if not yes quit */
			move(2, 0);
			prints("Aborting Kick User\n");
			pressreturn();
			clear();
			return 0;
		}
		search_record(PASSFILE, &kuinfo, sizeof (kuinfo), (void *) cmpuids, kickuser);
		ind = search_ulist(&uin, t_cmpuids, id);
	} else {
		uin = *userinfo;
		strcpy(kickuser, uin.userid);
		/*        id = getuser(kickuser);
				  search_record(PASSFILE, &kuinfo, sizeof(kuinfo), cmpuids, kickuser);
				  ind = search_ulist( &uin, t_cmpuids, id ); */
		ind = YEA;
	}


	//Ìß×Ô¼ºÊ± mode=1
	if(mode!=1)
	{
		clear();
		move(1,0);
		getdata(2,0,"ÌßÈËÔ­Òò: ", kickreason,STRLEN,DOECHO,YEA);
	} else {
		sprintf(kickreason, "");
	}
	if (uin.pid != 1 && (!ind || !uin.active || uin.pid <= 0 || (kill(uin.pid, 0) == -1))) {
		if (uinfo.mode != LUSERS && uinfo.mode != OFFLINE && uinfo.mode != FRIEND) {
			move(3, 0);
			prints("User Has Logged Out");
			clrtoeol();
			pressreturn();
			clear();
		}
		return 0;
	} else if (kill(uin.pid, SIGHUP) < 0) {
		prints("User can't be kicked");
		pressreturn();
		clear();
		return 1;
	}
	sprintf(contentbuf, "%s",kickreason);
	sprintf(titlebuf,"%s½«%sÌß³öÕ¾Íâ", currentuser.userid, kickuser);
	securityreport(titlebuf,contentbuf);

	sprintf(repbuf,"Äú±»%sÇ¿ÖÆÀë¿ª±¾Õ¾",currentuser.userid);
	sprintf(msgbuf,"ÀíÓÉ:%s\n",kickreason);
	mail_buf(msgbuf,kickuser, repbuf);
	sprintf(genbuf, "%s kick %s", currentuser.userid, kickuser);
	newtrace(genbuf);
	move(2, 0);
	if (uinfo.mode != LUSERS && uinfo.mode != OFFLINE && uinfo.mode != FRIEND) {
		prints("User has been Kicked\n");
		pressreturn();
		clear();
	}
	return 1;
}

