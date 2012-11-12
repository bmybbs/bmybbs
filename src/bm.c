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
#define DENY 1
#define UNDENY 2
#define CHANGEDENY 3

static int addtodeny(char *uident, char *msg, int ischange, int isglobal,
		     int isanony);
static int deldeny(char *uident, int isglobal, int isanony);
static int delclubmember(char *uident);
static int deny_notice(int action, char *user, int isglobal, int isanony,
		       char *msgbuf);

static int
addtodeny(uident, msg, ischange, isglobal, isanony)
char *uident;
char *msg;
int ischange, isglobal, isanony;
{
	char buf[50], strtosave[256];
	char buf2[50];
	int day;
	time_t nowtime;
	char ans[8];
	int seek;

	if (isglobal)
		strcpy(genbuf, "deny_users");
	else if (isanony)
		setbfile(genbuf, currboard, "deny_anony");
	else
		setbfile(genbuf, currboard, "deny_users");
	seek = seek_in_file(genbuf, uident);
	if ((ischange && !seek) || (!ischange && seek)) {
		move(2, 0);
		prints("输入的ID不对!");
		pressreturn();
		return -1;
	}
	buf[0] = 0;
	move(2, 0);
	prints("封禁对象：%s", (isanony) ? "Anonymous" : uident);
	while (strlen(buf) < 4)
		getdata(3, 0, "输入说明(至少两字): ", buf, 40, DOECHO, YEA);

	do {
		getdata(4, 0, "输入天数(0-手动解封): ", buf2, 4, DOECHO, YEA);
		day = atoi(buf2);
		if (day < 0)
			continue;
		if (!(currentuser.userlevel & PERM_SYSOP) && (!day || day > 20)) {
			move(4, 0);
			prints("超过权限,若需要,请联系站长!");
			pressreturn();
		} else
			break;
	} while (1);
	//add by lepton for deny bm's right

	nowtime = time(NULL);
	if (day) {
		struct tm *tmtime;
		//time_t daytime = nowtime + (day + 1) * 24 * 60 * 60; 
		time_t undenytime = nowtime + day * 24 * 60 * 60;
		//tmtime = gmtime(&daytime); by bjgyt
		tmtime = gmtime(&undenytime);
		sprintf(strtosave, "%-12s %-40s %2d月%2d日解 \x1b[%ldm", uident,
			buf, tmtime->tm_mon + 1, tmtime->tm_mday,
			(long int) undenytime);
		//modified by pzhg
		if (currentuser.userlevel & PERM_SPECIAL5) {
			if (isglobal)
				sprintf(msg,
	                        "封人原因: %s\n被封天数: %d%s\n解封日期: %d月%d日\n"
	                        "本站不接受对此封禁的虚拟投诉\n", buf, day,
	                        "(全站)" , tmtime->tm_mon + 1,
	                        tmtime->tm_mday);
			else
				sprintf(msg,
				"封人原因: %s\n被封天数: %d%s\n解封日期: %d月%d日\n"
				"本站不接受对此封禁的虚拟投诉\n", buf, day,
				isglobal ? "(全站)" : "", tmtime->tm_mon + 1,
				tmtime->tm_mday);
		}
		else if(isglobal || day>20)
		sprintf(msg,
                        "封人原因: %s\n被封天数: %d%s\n解封日期: %d月%d日\n"
                        "如有异议，可向%s提出，或到committee版投诉\n", buf, day,
                        "(全站)" , tmtime->tm_mon + 1,
                        tmtime->tm_mday, "站务");
		else {
			if (seek_in_file(MY_BBS_HOME"/etc/sysboards",currboard)) {
				sprintf(msg,
				"封人原因: %s\n被封天数: %d%s\n解封日期: %d月%d日\n"
				"如有异议，可向%s提出，或到committee版投诉\n", buf, day,
				isglobal ? "(全站)" : "", tmtime->tm_mon + 1,
				tmtime->tm_mday, "站务" );
			}
			else {
				sprintf(msg,
					"封人原因: %s\n被封天数: %d%s\n解封日期: %d月%d日\n"
					"如有异议，可向%s提出，或到Appeal版投诉\n", buf, day,
					isglobal ? "(全站)" : "", tmtime->tm_mon + 1,
					tmtime->tm_mday, isglobal ? "站务" : "版主");
			}
		}
	} else {
		if (currentuser.userlevel & PERM_SPECIAL5) {
			sprintf(strtosave, "%-12s %-35s 手动解封", uident, buf);
			sprintf(msg, "封人原因: %s\n被封天数: 手动解封%s\n"
				"本站不接受对此封禁的虚拟投诉\n",
				buf, isglobal ? "(全站)" : "");
		}
		else {
			sprintf(strtosave, "%-12s %-35s 手动解封", uident, buf);
			sprintf(msg, "封人原因: %s\n被封天数: 手动解封%s\n"
				"如有异议，可向%s提出，或到committee版投诉\n",
				buf, isglobal ? "(全站)" : "",
				isglobal ? "站务" : "版主");
		}
	}
	if (ischange)
		getdata(5, 0, "真的要改变么?[Y/N]: ", ans, 7, DOECHO, YEA);
	else
		getdata(5, 0, "真的要封么?[Y/N]: ", ans, 7, DOECHO, YEA);
	if ((*ans != 'Y') && (*ans != 'y'))
		return -1;
	if (ischange)
		deldeny(uident, isglobal, 0);
	if (isglobal)
		strcpy(genbuf, "deny_users");
	else if (isanony)
		setbfile(genbuf, currboard, "deny_anony");
	else
		setbfile(genbuf, currboard, "deny_users");
	return addtofile(genbuf, strtosave);
}

static int
deldeny(uident, isglobal, isanony)
char *uident;
int isglobal;
int isanony;
{
	char fn[STRLEN];

	if (isglobal)
		strcpy(fn, "deny_users");
	else if (isanony)
		setbfile(fn, currboard, "deny_anony");
	else
		setbfile(fn, currboard, "deny_users");
	return del_from_file(fn, uident);
}

int
deny_user()
{
	char uident[STRLEN];
	char ans[8];
	char msgbuf[256];
	int count, isglobal = 0;

	if (!IScurrBM) {
		return DONOTHING;
	}

	if (!strcmp(currboard, "sysop")) {
		getdata(0, 0,
			"设定哪个无法 Post 的名单？(A) sysop版 (B) 系统 (E)离开 [E]:",
			ans, 7, DOECHO, YEA);
		if (!strchr("AaBb", *ans))
			return FULLUPDATE;
		if (*ans == 'B' || *ans == 'b')
			isglobal = 1;
	}
	if (isglobal)
		strcpy(genbuf, "deny_users");
	else
		setbfile(genbuf, currboard, "deny_users");
//      ansimore(genbuf, YEA);
	while (1) {
		clear();
		prints("设定无法 Post 的名单\n");
		if (isglobal)
			strcpy(genbuf, "deny_users");
		else
			setbfile(genbuf, currboard, "deny_users");
		count = listfilecontent(genbuf);
		if (count)
			getdata(1, 0,
				"(A)增加 (D)删除 (C)改变 or (E)离开 [E]: ", ans,
				7, DOECHO, YEA);
		else
			getdata(1, 0, "(A)增加 or (E)离开 [E]: ", ans, 7,
				DOECHO, YEA);
		if (*ans == 'A' || *ans == 'a') {
			move(1, 0);
			if (isglobal)
				usercomplete("增加无法 POST 的使用者: ",
					     uident);
			else {
				int canpost = 0;
				while (!canpost) {
					move(1, 0);
					clrtoeol();
					usercomplete("增加无法 POST 的使用者：",
						     uident);
					if (*uident == '\0')
						break;
					canpost = posttest(uident, currboard);
				}
			}
			if (*uident != '\0') {
				if (addtodeny(uident, msgbuf, 0, isglobal, 0) ==
				    1) {
					deny_notice(DENY, uident, isglobal, 0,
						    msgbuf);
					sprintf(genbuf, "%s deny %s %s",
						currentuser.userid, currboard,
						uident);
					newtrace(genbuf);

				}
			}
		} else if ((*ans == 'C' || *ans == 'c')) {
			move(1, 0);
			usercomplete("改变谁的封禁时间或说明: ", uident);
			if (*uident != '\0') {
				if (addtodeny(uident, msgbuf, 1, isglobal, 0) ==
				    1) {
					deny_notice(CHANGEDENY, uident,
						    isglobal, 0, msgbuf);
				}
			}
		} else if ((*ans == 'D' || *ans == 'd') && count) {
			move(1, 0);
			namecomplete("删除无法 POST 的使用者: ", uident);
			move(1, 0);
			clrtoeol();
			if (uident[0] != '\0') {
				if (deldeny(uident, isglobal, 0)) {
					deny_notice(UNDENY, uident, isglobal, 0,
						    msgbuf);
				}
			}
		} else
			break;
	}

	clear();
	return FULLUPDATE;
}

int
addclubmember(uident, clubnum)
char *uident;
int clubnum;
{
	char genbuf1[80], genbuf2[80];
	int id;
	int i;
	char ans[8];
	int seek;

	if (clubnum == 0) {
		if (!(id = getuser(uident))) {
			move(3, 0);
			prints("Invalid User Id");
			clrtoeol();
			pressreturn();
			clear();
			return 0;
		}
		setbfile(genbuf, currboard, "club_users");
		seek = seek_in_file(genbuf, uident);
		if (seek) {
			move(2, 0);
			prints("输入的ID 已经存在!");
			pressreturn();
			return -1;
		}

		getdata(4, 0, "真的要添加么?[Y/N]: ", ans, 7, DOECHO, YEA);
		if ((*ans != 'Y') && (*ans != 'y'))
			return -1;
		setbfile(genbuf, currboard, "club_users");
		sethomefile(genbuf1, uident, "clubrights");
		if ((i = getbnum(currboard)) == 0)
			return DONOTHING;
		sprintf(genbuf2, "%d", bcache[i - 1].header.clubnum);
		addtofile(genbuf1, genbuf2);
		return addtofile(genbuf, uident);
	} else {
		setbfile(genbuf, currboard, "club_users");
		seek = seek_in_file(genbuf, uident);
		if (seek)
			return DONOTHING;
		sethomefile(genbuf1, uident, "clubrights");
		sprintf(genbuf2, "%d", clubnum);
		addtofile(genbuf1, genbuf2);
		return addtofile(genbuf, uident);
	}
}

static int
delclubmember(uident)
char *uident;
{
	char genbuf1[80], genbuf2[80];
	char fn[STRLEN];
	int id;
	int i;
	if (!(id = getuser(uident))) {
		move(3, 0);
		prints("Invalid User Id");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	if ((i = getbnum(currboard)) == 0)
		return DONOTHING;
	setbfile(fn, currboard, "club_users");
	sethomefile(genbuf1, uident, "clubrights");
	sprintf(genbuf2, "%d", bcache[i - 1].header.clubnum);
	del_from_file(genbuf1, genbuf2);
	return del_from_file(fn, uident);
}

int
clubmember()
{
	char uident[STRLEN];
	char ans[8], repbuf[STRLEN], buf[STRLEN], titlebuf[STRLEN];
	int count, i;

	if (!(IScurrBM)) {
		return DONOTHING;
	}
	if ((i = getbnum(currboard)) == 0)
		return DONOTHING;
	if (bcache[i - 1].header.clubnum == 0)
		return DONOTHING;
	setbfile(genbuf, currboard, "club_users");
	ansimore(genbuf, YEA);
	while (1) {
		clear();
		prints("设定俱乐部名单\n");
		setbfile(genbuf, currboard, "club_users");
		count = listfilecontent(genbuf);
		if (count)
			getdata(1, 0,
				"(A)增加 (D)删除or (E)离开or (M)写信给所有成员 [E]: ",
				ans, 7, DOECHO, YEA);
		else
			getdata(1, 0, "(A)增加 or (E)离开 [E]: ", ans, 7,
				DOECHO, YEA);
		if (*ans == 'A' || *ans == 'a') {
			move(1, 0);
			usercomplete("增加俱乐部成员: ", uident);
			if (*uident != '\0') {
				if (addclubmember(uident, 0) == 1) {
					getdata(5, 0, "加入原因：", buf, 50, DOECHO, YEA);
					sprintf(titlebuf,
						"%s由%s授予%s俱乐部权利",
						uident, currentuser.userid,
						currboard);
					sprintf(repbuf,	
						"%s%s%s", titlebuf, buf[0] ? "\n\n原因：":"", buf);
					securityreport(titlebuf, buf);
					deliverreport(titlebuf, repbuf);
					mail_buf(repbuf, uident, titlebuf);
				}
			}
		} else if ((*ans == 'D' || *ans == 'd') && count) {
			move(1, 0);
			namecomplete("删除俱乐部使用者: ", uident);
			move(1, 0);
			clrtoeol();
			if (uident[0] != '\0') {
				sprintf(genbuf, "真的要取消%s的俱乐部权利么？",
					uident);
				if (askyn(genbuf, YEA, NA))
					if (delclubmember(uident)) {
						getdata(5, 0, "删除原因：", buf, 50, DOECHO, YEA);
						sprintf(titlebuf,
							"%s被%s取消%s俱乐部权利",
							uident, currentuser.userid, currboard);
						sprintf(repbuf,	
							"%s%s%s", titlebuf, buf[0] ? "\n\n原因：":"", buf);
						securityreport(titlebuf, buf);
						deliverreport(titlebuf, repbuf);
						mail_buf(repbuf, uident, titlebuf);
					}
			}
		} else if ((*ans == 'M' || *ans == 'm') && count) {
			club_send();
		} else
			break;
	}
	clear();
	return FULLUPDATE;
}

int
deny_from_article(ent, fileinfo, direct)
int ent;
struct fileheader *fileinfo;
char *direct;
{
	char msgbuf[512];
	char user[STRLEN];
	char ans[8];
	int seek, canpost, isanony;
	if (!IScurrBM) {
		return DONOTHING;
	}
	if (!strcmp(fh2owner(fileinfo), "Anonymous")) {	/* 对匿名文章 */
		isanony = 1;
		setbfile(genbuf, currboard, "deny_anony");
		strcpy(user, fh2realauthor(fileinfo));
	} else {
		isanony = 0;
		setbfile(genbuf, currboard, "deny_users");
		strcpy(user, fileinfo->owner);
	}
	seek = seek_in_file(genbuf, user);
	if (seek) {		/* 解封 */
		move(2, 0);
		getdata(4, 0, "真的要解封么?[Y/N]: ", ans, 7, DOECHO, YEA);
		if ((*ans != 'Y') && (*ans != 'y'))
			return -1;
		if (deldeny(user, 0, isanony) == 1)
			deny_notice(UNDENY, user, 0, isanony, msgbuf);

	} else {		/* 匿名封禁 */
		canpost = posttest(user, currboard);
		if ((canpost) && (addtodeny(user, msgbuf, 0, 0, isanony) == 1))
			deny_notice(DENY, user, 0, isanony, msgbuf);
	}
	return 0;
}

static int
deny_notice(action, user, isglobal, isanony, msgbuf)
char *user, *msgbuf;
int action, isglobal, isanony;
{
	char repbuf[STRLEN];
	char tmpbuf[STRLEN], tmpbuf2[STRLEN];
	int i;
	char repuser[IDLEN + 1];
	if (isanony)
		strcpy(repuser, "Anonymous");
	else
		strcpy(repuser, user);
	switch (action) {
	case DENY:
		if (isglobal) {
			sprintf(repbuf,
				"%s被%s取消在全站的POST权利",
				user, currentuser.userid);
			securityreport(repbuf, msgbuf);
			deliverreport(repbuf, msgbuf);
			mail_buf(msgbuf, user, repbuf);
		} else if (((currentuser.userlevel & PERM_SYSOP)
			    || (currentuser.userlevel & PERM_OBOARDS))
			   && (msgbuf[10] == 'W' || msgbuf[10] == 'w')) {
			for (i = 10; msgbuf[i]; i++)
				if (msgbuf[i + 1] == '\n')
					msgbuf[i + 1] = 0;
			strcpy(tmpbuf, msgbuf + 11);
			strcpy(tmpbuf2, msgbuf + i + 1);
			sprintf(repbuf,
				"%s被站务%s暂时限制在%s的POST权利",
				repuser, currentuser.userid, currboard);
			sprintf(msgbuf,
				"站务%s认为%s有%s嫌疑.请本版版主/副\n"
				"及时对%s的行为按本版管理标准进行确认.\n"
				"恢复POST权或者给出正确封禁期限.\n\n%s",
				currentuser.userid, repuser, tmpbuf, repuser,
				tmpbuf2);
			securityreport(repbuf, msgbuf);
			deliverreport(repbuf, msgbuf);
			sprintf(repbuf,
				"%s被站务%s暂时限制在%s的POST权利",
				user, currentuser.userid, currboard);
			sprintf(msgbuf,
				"站务%s认为%s有%s嫌疑.请本版版主/副\n"
				"及时对%s的行为按本版管理标准进行确认.\n"
				"恢复POST权或者给出正确封禁期限.\n\n%s",
				currentuser.userid, user, tmpbuf, user,
				tmpbuf2);
			mail_buf(msgbuf, user, repbuf);
		}
		/*old action */
		else {
			sprintf(repbuf,
				"%s被%s取消在%s的POST权利",
				repuser, currentuser.userid, currboard);
			securityreport(repbuf, msgbuf);
			deliverreport(repbuf, msgbuf);
			sprintf(repbuf,
				"%s被%s取消在%s的POST权利",
				user, currentuser.userid, currboard);
			mail_buf(msgbuf, user, repbuf);
		}
		break;
	case CHANGEDENY:
		sprintf(repbuf,
			"%s改变封%s的时间或说明", currentuser.userid, user);
		securityreport(repbuf, msgbuf);
		deliverreport(repbuf, msgbuf);
		mail_buf(msgbuf, user, repbuf);
		break;
	case UNDENY:
		sprintf(repbuf,
			"恢复 %s 在 %s 的POST权",
			repuser, isglobal ? "全站" : currboard);
		snprintf(msgbuf, 256, "%s %s\n"
			 "请理解版务管理工作,谢谢!\n", currentuser.userid,
			 repbuf);
		securityreport(repbuf, msgbuf);
		deliverreport(repbuf, msgbuf);
		sprintf(repbuf,
			"恢复 %s 在 %s 的POST权",
			user, isglobal ? "全站" : currboard);
		snprintf(msgbuf, 256, "%s %s\n"
			 "请理解版务管理工作,谢谢!\n", currentuser.userid,
			 repbuf);
		mail_buf(msgbuf, user, repbuf);
		break;
	}
	return 0;
}

int                                                                                          
mail_buf_slow(char *userid, char *title, char *content, char *sender)                        
{                                                                                            
        FILE *fp;                                                                            
        char buf[256], dir[256];                                                             
        struct fileheader header;                                                            
        int t;                                                                               
        int now;                                                                             
        bzero(&header, sizeof (header));                                                     
        fh_setowner(&header, sender, 0);                                                     
        sprintf(buf, "mail/%c/%s/", mytoupper(userid[0]), userid);                           
        if (!file_isdir(buf))                                                                
                return -1;                                                                   
        now = time(NULL);                                                                    
        t = trycreatefile(buf, "M.%d.A", now, 100);                                          
        if (t < 0)                                                                           
                return -1;                                                                   
        header.filetime = t;                                                                 
        strsncpy(header.title, title, sizeof (header.title));                                
        fp = fopen(buf, "w");                                                                
        if (fp == 0)                                                                         
                return -2;                                                                   
        fprintf(fp, "%s", content);                                                          
        fclose(fp);                                                                          
        setmailfile(dir, userid, ".DIR");                                                    
        append_record(dir, &header, sizeof (header));                                        
        return 0;                                                                            
}                                                                                            

