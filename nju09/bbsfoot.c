#include "bbslib.h"


struct user_info *
query_f(int uid)
{
	int i, uent, testreject = 0;
	struct user_info *uentp;
	if (uid <= 0 || uid > MAXUSERS)
		return 0;
	for (i = 0; i < 6; i++) {
		uent = uindexshm->user[uid - 1][i];
		if (uent <= 0)
			continue;
		uentp = &shm_utmp->uinfo[uent - 1];
		if (!uentp->active || !uentp->pid || uentp->uid != uid)
			continue;
		if (!testreject) {
			if (isbad(uentp->userid))
				return 0;
			testreject = 1;
		}
		if (shm_utmp->uinfo[uent - 1].invisible
		    && !HAS_PERM(PERM_SYSOP | PERM_SEECLOAK))
			continue;
		return uentp;
	}
	return 0;
}
//Add by liuche 20120616

void footInfo(){
	char buf[1030],buf2[1030];
	FILE *fp2;

	printf("<span id=\'foot_msg\' style=\"position:fixed;padding:0; margin-left:5px;overflow:hidden;\">\n");
	printf("    <MARQUEE scrollAmount=1 scrollDelay=180 direction=up  height=17 onmouseover= \"this.stop(); \"   onmouseout= \"this.start()\";>\n");
	printf("    <align=left>\n");
	fp2 = fopen(MY_BBS_HOME "/etc/endline", "r");
	if (fp2 != 0) 
	{
		while(fgets(buf, 1030, fp2) != NULL)
		{
			buf[strlen(buf) - 1] = 0;
			hsprintf(buf2,buf);
			printf("    %s<BR><BR>\n",buf2);
		}
		fclose(fp2);
	}
	printf("    </MARQUEE>\n");
	printf("</span>\n");

}

int
bbsfoot_main()
{

	int dt = 0, mail_total = 0, mail_unread = 0, lasttime = 0;
	int count_friends=0, i=0;
	char *id = "guest";
	static int r = 0;
	html_header(2);
	
	printf("<script>function t(){return (new Date()).valueOf();}</script>\n");
	printf("<body bgcolor=#efefef>\n");
	printf("<div id='bbsfoot'>");
    if (loginok) {
		id = currentuser.userid;
		dt = abs(now_t - w_info->login_start_time) / 60;
	}

	printf("<span id='bbsfoot_time'>时间[<span class=0011>%16.16s</span>]</span>\n", Ctime(now_t));
	printf("<span id='bbsfoot_online'>");
	if (loginok && !isguest){
		for (i = 0; i < u_info->fnum; i++)
			count_friends += query_f(u_info->friend[i]) ? 1 : 0;
		printf("在线/好友[<a href=bbsufind?search=A&limit=20 target=f3 class=1011>%d</a> ",
		       count_online());
		printf("/<a href=bbsfriend target=f3 class=1011>%d</a>] ",
			count_friends);
	}else
		printf("在线[<a href=bbsufind?search=A&limit=20 target=f3 class=1011>%d</a>] ",
	       	count_online());
	//add by macintosh 050619
	printf("</span>\n");

	printf("<span id='bbsfoot_acc'>帐号[<a href=bbsqry?userid=%s target=f3 class=1011>%s</a>]</span>", id, id);

	printf("<span id='bbsfoot_mail'>");
	if (loginok && !isguest) {
		int thistime;
		lasttime = atoi(getparm("lt"));
		thistime = mails_time(id);
		if (thistime <= lasttime) {
			mail_total = atoi(getparm("mt"));
			mail_unread = atoi(getparm("mu"));
		} else {
			mail_total = mails(id, &mail_unread);
			lasttime = thistime;
		}
		if (mail_unread == 0) {
			printf("信箱[<a href=bbsmail target=f3 class=1011>%d封</a>] ",
			       mail_total);
		} else {
			printf
			    ("信箱[<a href=bbsmail target=f3 class=1011>%d(<font color=red>新信%d</font>)</a>] ",
			     mail_total, mail_unread);
		}
	}
	printf("</span>\n");

	printf("<span id='bbsfoot_stay'>停留[<font style=\"font-size:10px\" color=#ff6600>%d</font>小时<font style=\"font-size:10px\" color=#ff6600>%d</font>分钟]</span>", dt / 60, dt % 60);
	//Add by liuche 20120616
	footInfo();
	printf("</div>\n");
	//Add by liuche 20120616
	printf("<script>setTimeout('self.location.replace("
	       "\"bbsfoot?lt=%d&mt=%d&mu=%d&sn='+t()+'\")', %d);</script>",
	       lasttime, mail_total, mail_unread, 300000 + r * 1000);
	r = (r + dt + now_t) % 30;
	printf("</body>\n");
	return 0;
}

int
mails_time(char *id)
{
	char path[80];
	if (!loginok || isguest)
		return 0;
	sprintf(path, "mail/%c/%s/.DIR", mytoupper(id[0]), id);
	return file_time(path);
}

int
mails(char *id, int *unread)
{
	struct fileheader *x;
	char path[80];
	int total = 0, i;
	struct mmapfile mf = { ptr:NULL };
	*unread = 0;
	if (!loginok || isguest)
		return 0;
	setmailfile(path, id, ".DIR");
	MMAP_TRY {
		if (mmapfile(path, &mf) < 0) {
			MMAP_UNTRY;
			MMAP_RETURN(0);
		}
		total = mf.size / sizeof (struct fileheader);
		x = (struct fileheader*)mf.ptr;
		for (i = 0; i < total; i++) {
			if (!(x->accessed & FH_READ))
				(*unread)++;
			x++;
		}
	}
	MMAP_CATCH {
		total = 0;
		*unread = 0;
	}
	MMAP_END mmapfile(NULL, &mf);
	return total;

}


