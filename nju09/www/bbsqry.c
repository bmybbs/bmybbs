#include "bbslib.h"

static int bm_printboard(struct boardmanager *bm, void *farg);
static int bm_printboardapi(struct boardmanager *bm,char *farg);
int show_special_web(char *id2) {
	FILE *fp;
	char id1[80], name[80], buf[256];
	fp=fopen("etc/special", "r");
	if(fp==0)
		return 0;
	while(1) {
		if(fgets(buf, 256, fp)==0) break;
		if(sscanf(buf, "%s %s", id1, name)<2) continue;
		if(!strcasecmp(id1, id2)) hprintf(" \033[1;33m★\033[36m%s\033[33m★\033[m",name);
	}
	fclose(fp);
	return 0;
}

void show_special_api(char *id2, char *output){
	FILE *fp;
	char id1[80], name[80], buf[256];
	fp=fopen("etc/special", "r");
	if(fp!=0){
		while(1){
			if(fgets(buf, 256, fp)==0) break;
			if(sscanf(buf, "%s %s", id1, name)<2) continue;
			if(!strcasecmp(id1, id2)) sstrcat(output, "\"Title\":\"%s\",", name);
		}
	}
	fclose(fp);
}

int
apiqry_main()
{
	FILE *fp;
	char userid[14], filename[80], buf[512], output[4096],output_utf8[4096];
	struct userec *x;
	struct user_info *u;
	int i, tmp2, num;
	json_header();
//	changemode(QUERY);
	ytht_strsncpy(userid, getparm("U"), 13);
	if (!userid[0])
		ytht_strsncpy(userid, getparm("userid"), 13);
	if (userid[0] == '\0')
	{
		printf("{\"User\":null}");
		return 0;
	}
	x = getuser(userid);
	if (x == 0)
	{
		printf("{\"User\":null}");
		return 0;
	}
	// 开始输出用户数据
	sstrcat(output, "{\"User\":{");
	// 显示基本数据
	sstrcat(output, "\"UserID\":\"%s\",\"UserNickName\":\"%s\",\"LoginCounts\":%d,\"PostCounts\":%d,\"LastLogin\":\"%s\",\"LastHost\":\"%s\",", x->userid, x->username, x->numlogins, x->numposts, ytht_ctime(x->lastlogin), x->lasthost);
	// 显示个人数据
	if(!strcasecmp(x->userid, currentuser.userid)){
		sstrcat(output, "\"Exp\":%d,\"ExpLevel\":\"%s\",\"Perf\":%d,\"PerfLevel\":\"%s\"",countexp(x), charexp(countexp(x)), countperf(x), cperf(countperf(x)));
	}
	// 显示版务、站务等信息
	if(x->userlevel & PERM_BOARDS)
	{
		sstrcat(output, "\"BOARDBM\":[");
		sethomefile(filename, x->userid, "mboard");
		new_apply_record(filename, sizeof(struct boardmanager), (void *)bm_printboardapi, output);
		sstrcat(output, "],");
	}
	if((x->userlevel & PERM_SYSOP) && (x->userlevel&PERM_ARBITRATE))
		sstrcat(output, "\"Job\":\"本站顾问团\",");
	else if(x->userlevel & PERM_SYSOP)
		sstrcat(output, "\"Job\":\"现任站长\",");
	else if(x->userlevel & PERM_OBOARDS)
		sstrcat(output, "\"Job\":\"实习站长\",");
	else if(x->userlevel & PERM_ARBITRATE)
		sstrcat(output, "\"Job\":\"现任纪委\",");
	else if(x->userlevel & PERM_SPECIAL4)
		sstrcat(output, "\"Job\":\"区长\",");
	else if(x->userlevel & PERM_WELCOME)
		sstrcat(output, "\"Job\":\"系统美工\",");
	else if(x->userlevel & PERM_SPECIAL7)
	{
		if((x->userlevel & PERM_SPECIAL1) && !(x->userlevel & PERM_CLOAK))
			sstrcat(output, "\"Job\":\"离任程序员\",");
		else
			sstrcat(output, "\"Job\":\"程序组成员\",");
	}
	else if(x->userlevel & PERM_ACCOUNTS)
		sstrcat(output, "\"Job\":\"帐号管理员\",");

	// 显示当前状态，null 或者 array
	num = 0;
	sstrcat(output, "\"States\":");
	for (i=0; i<MAXACTIVE; ++i)
	{
		u = &(shm_utmp->uinfo[i]);
		if(!strcmp(u->userid, x->userid))
		{
			if(u->active == 0 || u->pid ==0 || (u->invisible && !HAS_PERM(PERM_SEECLOAK, currentuser)))
				continue;
			++num;
			if(num == 1)
				sstrcat(output, "[");
			if(u->mode != USERDF4){
				if(u->invisible)
					sstrcat(output, "\"C%s\"", ModeType(u->mode));
				else
					sstrcat(output, "\"%s\"", ModeType(u->mode));
			}
			else
				// 自定义状态
				sstrcat(output, "%s", u->user_state_temp);
		}
		if( (num>0) && (i == MAXACTIVE - 1))
			sstrcat(output, "],");
	}
	if (num == 0 )
	{
		sstrcat(output, "\"null\",");
		if( x->lastlogout != 0 )
			sstrcat(output, "\"LastLogout\":\"%s\",", ytht_ctime(x->lastlogout));
		else
			sstrcat(output, "\"LastLogout\":null,");
	}


	// 显示说明档
	sethomefile(filename,x->userid,"plans");
	fp = fopen(filename, "r");
	sprintf(filename, "00%s-plan", x->userid);
	if (fp) {
		sstrcat(output, "\"PersonalIntro\":\""); // 开始个人说明档
		while(1){
			if(fgets(buf,256,fp) == 0)
				break;
			if (buf[strlen(buf) - 1] == '\n'){
				int currentlength;
				currentlength = strlen(buf);
				buf[currentlength - 1] = '\\';
				buf[currentlength] = 'n';
				buf[currentlength + 1] = 0;
			}
			if(!strncmp(buf,"bigin 644",10)){
				// errlog()
				// fdisplay_attach()
			}
			sstrcat(output, "%s", buf);
		}
		sstrcat(output, "\","); // 结束个人说明档
		fclose(fp);
	}
	else //没有个人说明档
		sstrcat(output, "\"PersonalIntro\":null,");

	// 显示特殊标签
	show_special_api(x->userid, output);
	// 结束输出
	sstrcat(output, "}}");
	g2u(output, strlen(output), output_utf8, sizeof(output_utf8));
	printf("%s", output_utf8);
	//http_quit();
	return 0;
}

int
bbsqry_main()
{
	FILE *fp;
	char userid[14], filename[80], buf[512];
	struct userec *x;
	struct user_info *u;
	int i, tmp2, num;
	html_header(1);
	check_msg();
	changemode(QUERY);
	ytht_strsncpy(userid, getparm("U"), 13);
	if (!userid[0])
		ytht_strsncpy(userid, getparm("userid"), 13);
	printf("<body><center>");
	printf("<div class=rhead>%s -- 查询网友</div><hr>\n", BBSNAME);
	if (userid[0] == 0) {
		printf("<form action=bbsqry>\n");
		printf("请输入用户名: <input name=userid maxlength=12 size=12>\n");
		printf("<input type=submit value=查询用户>\n");
		printf("</form><hr>\n");
		http_quit();
	}
	x = getuser(userid);
	if (x == 0) {
		int i, j = 0;
		printf("没有这个用户啊，难道是这些:<p>");
		printf("<table width=600>");
		for (i = 0; i < shm_ucache->number; i++)
			if (strcasestr(shm_ucache->userid[i], userid) == shm_ucache->userid[i]) {
				j++;
				if (j % 6 == 1)
					printf("<tr>");
				printf("<td>");
				printf("<a href=bbsqry?userid=%s>%s</a>", shm_ucache->userid[i], shm_ucache->userid[i]);
				printf("</td>");
				sprintf(buf, "bbsqry?userid=%s", shm_ucache->userid[i]);
				if (j % 6 == 0)
					printf("</tr>");
				if (j >= 12 * 6)
					break;
			}
		printf("</table>");
		if (!j)
			printf("不可能，肯定是你敲错了，根本没这人啊");
		if (j == 1)
			redirect(buf);
		printf("<p><a href=javascript:history.go(-1)>快速返回</a>");
		http_quit();
	}
	printf("</center><pre style='font-size:14px'>\n");
	sprintf(buf,
		"%s (\033[33m%s\033[37m) 共上站 \033[1;32m%d\033[m 次，发表文章 \033[1;32m%d\033[m 篇",
		x->userid, x->username, x->numlogins, x->numposts);
	hprintf("%s", buf);
	show_special_web(x->userid);//add by wjbta@bmy  增加id标识
	printf("\n");
	hprintf("上次在 [\033[1;32m%s\033[m] 从 [\033[1;32m%s\033[m] 到本站一游。\n", ytht_ctime(x->lastlogin), x->lasthost);
	mails(userid, &tmp2);
	hprintf("信箱：[\033[1;32m%s\033[m]，", tmp2 ? "⊙" : "  ");
	if (!strcasecmp(x->userid, currentuser.userid)) {
		hprintf("经验值：[\033[1;32m%d\033[m](\033[33m%s\033[m) ", countexp(x), charexp(countexp(x)));
		hprintf("表现值：[\033[1;32m%d\033[m](\033[33m%s\033[m) ", countperf(x), cperf(countperf(x)));
	}
	hprintf("生命力：[\033[1;32m%d\033[m]。\n", count_life_value(x));
	if (x->userlevel & PERM_BOARDS) {
		hprintf("担任版务：");
		sethomefile(filename, x->userid, "mboard");
		new_apply_record(filename, sizeof (struct boardmanager), (void *) bm_printboard, NULL);
		if (x->userlevel & !strcmp(x->userid, "SYSOP"))
			hprintf("[\033[1;36m系统管理员\033[m]");
		else if (x->userlevel & !strcmp(x->userid, "lanboy"))
			hprintf("[\033[1;36m系统管理员\033[m]");
		else if ((x->userlevel&PERM_SYSOP) && (x->userlevel&PERM_ARBITRATE) )
			hprintf("[\033[1;36m本站顾问团\033[m]");
		else if (x->userlevel & PERM_SYSOP)
			hprintf("[\033[1;36m现任站长\033[m]");
		else if (x->userlevel & PERM_OBOARDS)
			hprintf("[\033[1;36m实习站长\033[m]");
		else if (x->userlevel & PERM_ARBITRATE)
			hprintf("[\033[1;36m现任纪委\033[m]");
		else if (x->userlevel & PERM_SPECIAL4)
			hprintf("[\033[1;36m区长\033[m]");
		else if (x->userlevel & PERM_WELCOME)
			hprintf("[\033[1;36m系统美工\033[m]");
		else if (x->userlevel & PERM_SPECIAL7) {
			if ( (x->userlevel & PERM_SPECIAL1) && !(x->userlevel & PERM_CLOAK) )
				hprintf("[\033[1;36m离任程序员\033[m]");
			else
				hprintf("[\033[1;36m程序组成员\033[m]");
		}
		else if (x->userlevel & PERM_ACCOUNTS)
			hprintf ("[\033[1;36m帐号管理员\033[m]");
		hprintf("\n");
	}
	num = 0;
	for (i = 0; i < MAXACTIVE; i++) {
		u = &(shm_utmp->uinfo[i]);
		if (!strcmp(u->userid, x->userid)) {
			if (u->active == 0 || u->pid == 0 || (u->invisible && !HAS_PERM(PERM_SEECLOAK, currentuser)))
				continue;
			num++;
			if (num == 1)
				hprintf("目前在站上, 状态如下:\n");
			if (u->invisible)
				hprintf("\033[36mC\033[37m");
			if (u->mode != USERDF4)
				hprintf("\033[%dm%s\033[m ", u->pid == 1 ? 35 : 32, ModeType(u->mode));
			else							/* 自定义状态 */
				hprintf("\033[%dm%s\033[m ", u->pid == 1 ? 35 : 32, u->user_state_temp);
			if (num % 5 == 0)
				printf("\n");
		}
	}
	if (num == 0) {
		hprintf("目前不在站上, 上次离站时间 [\033[1;32m%s\033[m]\n\n",
				x->lastlogout ? ytht_ctime(x->lastlogout) :
				"因在线上或不正常断线不详");
	}
	printf("\n");
	printf("</pre><table width=100%%><tr><td class=f2>");
	sethomefile(filename, x->userid, "plans");
	fp = fopen(filename, "r");
	sprintf(filename, "00%s-plan", x->userid);
	fdisplay_attach(NULL, NULL, NULL, NULL);
	if (fp) {
		while (1) {
			if (fgets(buf, 256, fp) == 0)
				break;
			if (!strncmp(buf, "begin 644 ", 10)) {
				errlog("old attach %s", filename);
				fdisplay_attach(stdout, fp, buf, filename);
				continue;
			}
			fhhprintf(stdout, "%s", buf);
		}
		fclose(fp);
	} else {
		hprintf("\033[36m没有个人说明档\033[37m\n");
	}
	printf("</td></tr></table>");
	printf("<br><br><a href=bbspstmail?userid=%s&title=没主题>[书灯絮语]</a> ", x->userid);
	printf("<a href=bbssendmsg?destid=%s>[发送讯息]</a> ", x->userid);
	printf("<a href=bbsfadd?userid=%s>[加入好友]</a> ", x->userid);
	printf("<a href=bbsfdel?userid=%s>[删除好友]</a>", x->userid);
	printf("<hr>");
	printf("<center><form action=bbsqry>\n");
	printf("请输入用户名: <input name=userid maxlength=12 size=12>\n");
	printf("<input type=submit value=查询用户>\n");
	printf("</form><hr>\n");
	printf("</body>\n");
	http_quit();
	return 0;
}

void
show_special(char *id2)
{
	FILE *fp;
	char id1[80], name[80];
	fp = fopen("etc/sysops", "r");
	if (fp == 0)
		return;
	while (1) {
		id1[0] = 0;
		name[0] = 0;
		if (fscanf(fp, "%s %s", id1, name) <= 0)
			break;
		if (!strcmp(id1, id2))
			hprintf(" \033[1;31m★\033[0;36m%s\033[1;31m★\033[m", name);
	}
	fclose(fp);
}

static int bm_printboard(struct boardmanager *bm, void *farg) {
	if (getboard(bm->board)){
		printf("<a href=%s%s target=f3>", showByDefMode(), bm->board);
		hprintf("%s", bm->board);
		printf("</a> ");
		hprintf(" ");
	}
	return 0;
}

static int bm_printboardapi(struct boardmanager *bm,char *farg) {
	if (getboard(bm->board)){
		sstrcat(farg, "\"%s\",",bm->board);
	}
	return 0;
}

