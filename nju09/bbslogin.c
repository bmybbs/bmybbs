#include "bbslib.h"

#define NHASH 67
//#define NSEARCH ((MAXACTIVE / 8) > 100 ? (MAXACTIVE / 8) : 100)
#define NSEARCH MAXACTIVE
static char *makeurlbase(int uent);
static char *check_multi(char *id, int uid);
static int iphash(char *fromhost);
int
bbslogin_main()
{
	int n, t;
	time_t dtime;
	char filename[128], buf[256], id[20], pw[20], url[10], *ub = FIRST_PAGE; // main_page[STRLEN];
	struct userec *x;
	int ipmask;
	html_header(3);
	strsncpy(id, getparm("id"), 13);
	strsncpy(pw, getparm("pw"), 13);
	strsncpy(url, getparm("url"), 3);
	ipmask = atoi(getparm("ipmask"));

	if (loginok && strcasecmp(id, currentuser.userid) && !isguest) {
		http_fatal
		    ("系统检测到目前你的计算机上已经登录有一个帐号 %s，请先退出.(选择正常logout)",
		     currentuser.userid);
	}
	if (!strcmp(id, "")) {
		strcpy(id, "guest");
	}
	x = getuser(id);
	if (x == 0) {
		printf("%s<br>", id);
		http_fatal("错误的使用者帐号");
	}
	strcpy(id, x->userid);
	if (strcasecmp(id, "guest")) {
		if (checkbansite(fromhost)) {
			http_fatal
			    ("对不起, 本站不欢迎来自 [%s] 的登录. <br>若有疑问, 请与SYSOP联系.",
			     fromhost);
		}
		if (userbansite(x->userid, fromhost))
			http_fatal("本ID已设置禁止从%s登录", fromhost);
		if (!checkpasswd(x->passwd, pw)) {
			logattempt(x->userid, fromhost, "WWW", now_t);
			http_fatal("密码错误");
		}
		if (!user_perm(x, PERM_BASIC))
			http_fatal
			    ("此帐号已被停机, 若有疑问, 请用其他帐号在sysop版询问.");
		if (file_has_word(MY_BBS_HOME "/etc/prisonor", x->userid))
			http_fatal("安心改造，不要胡闹");
		if (x->dietime)
			http_fatal("死了?还要做什么? :)");
		t = x->lastlogin;
		x->lastlogin = now_t;
		if (abs(t - now_t) < 20) {
			http_fatal("两次登录间隔过密!");
		}

		dtime = t - 4 * 3600;
		t = localtime(&dtime)->tm_mday;
		dtime = now_t - 4 * 3600;
		if (t < localtime(&dtime)->tm_mday && x->numdays < 800)
			x->numdays++;
		x->numlogins++;
		strsncpy(x->lasthost, fromhost, 16);
		save_user_data(x);
		currentuser = *x;
	}
	sprintf(buf, "%s enter %s www", x->userid, fromhost);
	newtrace(buf);
	n = 0;
	if (loginok && isguest) {
		bzero(u_info, sizeof (struct user_info));
	}
	if (strcasecmp(id, "guest")) {
		sethomepath(filename, x->userid);
		mkdir(filename, 0755);

		strsncpy(buf, getparm("style"), 3);
		wwwstylenum = -1;
		if (isdigit(buf[0]))
			wwwstylenum = atoi(buf);
		if ((wwwstylenum > NWWWSTYLE || wwwstylenum < 0))
			if (!readuservalue
			    (x->userid, "wwwstyle", buf, sizeof (buf)))
				    wwwstylenum = atoi(buf);
		if (wwwstylenum < 0 || wwwstylenum >= NWWWSTYLE)
			wwwstylenum = 1;
		currstyle = &wwwstyle[wwwstylenum];
	} else {
		wwwstylenum = 1;
		currstyle = &wwwstyle[wwwstylenum];

	}

	ub = wwwlogin(x, ipmask);
	if (!strcmp(url, "1")) 
		/*printf("<link href=\"images/@byron.css\" rel=stylesheet type=\"text/css\">\n
			<frameset cols=135,* frameSpacing=0 frameborder=no id=fs0>\n
			<frame src=\"%sbbsleft?t=%ld\" name=f2 frameborder=no scrolling=no>\n
			<frameset id=fs1 rows=0,*,18 frameSpacing=0 frameborder=no border=0>\n
			<frame scrolling=no name=fmsg src=\"%sbbsmsg\">\n
			<frame name=f3 src=\"%sbbsfoot\">\n
			<frame scrolling=no name=f4 src=\"%sbbsfoot.htm\">\n
			</frameset>\n
			</frameset>\n", ub, now_t, ub, ub, ub);*/			//add by mintbaggio 040411 for new www

	//	html_header(3);

		printf
		    ("<script>opener.parent.f2.location.href=\"%sbbsleft?t=%ld\";\n"
		     "opener.parent.fmsg.location.href=\"%sbbsgetmsg\";\n"
		     //"opener.parent.f4.location.href=\"%sbbsfoot\";\n"
		     "a=window.opener.location.href;\n" "l=a.length;\n"
		     "t=a.indexOf('/" SMAGIC "',1);\n" "t=a.indexOf('/',t+1);\n"
		     "nu=\"%s\"+a.substring(t+1,l);\n"
		     "window.opener.location.href=nu;window.close();</script>",
		     ub, now_t, ub, ub, ub);

		
	//}
	else
		redirect(ub);
	//else {
	//	print_session_string(ub);
	//	html_header(3);
	//	
	//	sprintf(main_page, "/%s/", SMAGIC);
	//	redirect(main_page);
	//}
	http_quit();
	return 0;
}

char *
wwwlogin(struct userec *user, int ipmask)
{
	FILE *fp, *fp1;
	int n, dolog = 0, st, clubnum, uid, i, nsearch;
	struct user_info *u;
	char ULIST[STRLEN];
	char genbuf[256], *urlbase, fname[80];
	uid = getusernum(user->userid) + 1;

	if ((urlbase = check_multi(user->userid, uid)))
		return urlbase;

	if (strcasecmp(user->userid, "guest") && count_uindex(uid) >= 3)
		http_fatal("您已经登录了三个帐号,不能再登录了");
//      如果要限制WWW登录窗口数 就打开这个注释. lepton

	gethostname(genbuf, 256);
	sprintf(ULIST, MY_BBS_HOME "/%s.%s", ULIST_BASE, genbuf);

	fp = fopen(ULIST, "a");
	flock(fileno(fp), LOCK_EX);
	nsearch = NSEARCH;
	//if (strcasecmp(user->userid, "guest"))
	//      nsearch = MAXACTIVE / 4;
	for (i = 0, n = iphash(fromhost) * (MAXACTIVE / NHASH); i < nsearch;
	     i++, n++) {
		if (n >= MAXACTIVE)
			n = 0;
		u = &(shm_utmp->uinfo[n]);
		if (u->active && u->pid == 1
		    && ((now_t - u->lasttime) > 20 * 60 || u->wwwinfo.iskicked)) {
			st = u->lasttime - u->wwwinfo.login_start_time;
			if (st > 86400) {
				errlog("Strange long stay time,%d!, drop %s",
				       st, u->userid);
				st = 86400;
			}
			sprintf(genbuf, "%s drop %d www", u->userid, st);
			newtrace(genbuf);
			remove_uindex(u->uid, n + 1);
			bzero(u, sizeof (struct user_info));
		}
		if (!dolog && u->active == 0) {
			u_info = u;
			bzero(u, sizeof (struct user_info));
			u->active = 1;
			u->uid = uid;
			u->pid = 1;
			//u->pid = thispid;			//modify by mintbaggio@BMY for kill www user
			u->mode = LOGIN;
			if (strcasecmp(user->userid, "guest"))
				u_info->unreadmsg = get_unreadmsg(user->userid);
			else
				u_info->unreadmsg = 0;
			u->userlevel = user->userlevel;
			u->lasttime = now_t;
			u->curboard = 0;
			if (user_perm(user, PERM_LOGINCLOAK) &&
			    (user->flags[0] & CLOAK_FLAG))
				u->invisible = YEA;
			u->pager = 0;
			if (user->userdefine & DEF_FRIENDCALL)
				u->pager |= FRIEND_PAGER;
			if (user->flags[0] & PAGER_FLAG) {
				u->pager |= ALL_PAGER;
				u->pager |= FRIEND_PAGER;
			}
			if (user->userdefine & DEF_FRIENDMSG)
				u->pager |= FRIENDMSG_PAGER;
			if (user->userdefine & DEF_ALLMSG) {
				u->pager |= ALLMSG_PAGER;
				u->pager |= FRIENDMSG_PAGER;
			}
			strsncpy(u->from, fromhost, 24);
			strsncpy(u->username, user->username, NAMELEN);
			strsncpy(u->userid, user->userid, IDLEN + 1);
			getrandomstr(u->sessionid);
			if (strcasecmp(user->userid, "guest"))
				initfriends(u);
			else
				memset(u->friend, 0, sizeof (u->friend));
			urlbase = makeurlbase(n);
			w_info = &(u_info->wwwinfo);
			w_info->login_start_time = now_t;
			w_info->ipmask = ipmask;
			if (strcasecmp(user->userid, "guest")) {
				sethomefile(fname, user->userid, "clubrights");
				if ((fp1 = fopen(fname, "r")) == NULL) {
					memset(u_info->clubrights, 0,
					       4 * sizeof (int));
				} else {
					while (fgets(genbuf, STRLEN, fp1) !=
					       NULL) {
						clubnum = atoi(genbuf);
						u_info->clubrights[clubnum /
								   32] |=
						    (1 << clubnum % 32);
					}
					fclose(fp1);
				}

				set_my_cookie();
			} else {
				memset(u_info->clubrights, 0, 4 * sizeof (int));
				w_info->t_lines = 20;
				w_info->att_mode = 0;
				w_info->doc_mode = 1;
			}
			dolog = 1;
			add_uindex(u->uid, n + 1);
		}
	}
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	if (!dolog)
		http_fatal
		    ("抱歉，目前在线用户数已达上限，无法登录。请稍后再来。");
	if ((user->userlevel & PERM_BOARDS))
		setbmstatus(user, 1);
	return urlbase;
}

static int
do_check(int uent, int uid)
{
	return (shm_utmp->uinfo[uent].active == 1
		&& shm_utmp->uinfo[uent].pid == 1
		&& now_t - shm_utmp->uinfo[uent].lasttime < 18 * 60
		&& shm_utmp->uinfo[uent].uid == uid
		&& !strncmp(fromhost, shm_utmp->uinfo[uent].from, 24));
}

static char *
makeurlbase(int uent)
{
	static char urlbase[STRLEN];
	//max 26*26*26-1 = 17575 online,enough?
	sprintf(urlbase, "/%s%c%c%c%s", SMAGIC,
		uent / (26 * 26) + 'A', uent / 26 % 26 + 'A',
		uent % 26 + 'A', shm_utmp->uinfo[uent].sessionid);
	addextraparam(urlbase, sizeof (urlbase), 0, wwwstylenum);
	strcat(urlbase, "/");
	return urlbase;
}

static char *
check_multi(char *id, int uid)
{
	int i, uent;
	if (uid <= 0 || uid > MAXUSERS)
		return NULL;
	if (strcasecmp(id, "guest") && 1) {
		//这种算法, wwwlogin必须限制登录窗口数目, 否则
		//上线名单会被轻易冲爆
		for (i = 3; i < 6; i++) {
			uent = uindexshm->user[uid - 1][i] - 1;
			if (do_check(uent, uid))
				return makeurlbase(uent);
		}
		return NULL;
	} else {
		for (i = 0, uent = iphash(fromhost) * (MAXACTIVE / NHASH);
		     i < NSEARCH; i++, uent++) {
			if (uent >= MAXACTIVE)
				uent = 0;
			if (do_check(uent, uid))
				return makeurlbase(uent);
		}
	}
	return NULL;
}

static int
iphash(char *fromhost)
{
	return 0;
	/* ipv6 by leoncom 无法将in6_addr转为整形
	struct in_addr addr;
	inet_aton(fromhost, &addr);
	return addr.s_addr % NHASH;
	*/
}
