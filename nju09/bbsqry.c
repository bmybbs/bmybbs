#include "bbslib.h"

int show_special_web(char *id2) {
	FILE *fp;
	char id1[80], name[80], buf[256];
	fp=fopen("etc/special", "r");
	if(fp==0)
		return 0;
	while(1) {
		if(fgets(buf, 256, fp)==0) break;
		if(sscanf(buf, "%s %s", id1, name)<2) continue;
		if(!strcasecmp(id1, id2)) hprintf(" [1;33m¡ï[36m%s[33m¡ï[m",name);
	}
	fclose(fp);
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
	close(fp);
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
	strsncpy(userid, getparm("U"), 13);
	if (!userid[0])
		strsncpy(userid,getparm("userid"), 13);
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
	// ¿ªÊ¼Êä³öÓÃ»§Êı¾İ
	sstrcat(output, "{\"User\":{");
	// ÏÔÊ¾»ù±¾Êı¾İ
	sstrcat(output, "\"UserID\":\"%s\",\"UserNickName\":\"%s\",\"LoginCounts\":%d,\"PostCounts\":%d,\"LastLogin\":\"%s\",\"LastHost\":\"%s\",", x->userid, x->username, x->numlogins, x->numposts,Ctime(x->lastlogin), x->lasthost);
	// ÏÔÊ¾¸öÈËÊı¾İ
	if(!strcasecmp(x->userid, currentuser.userid)){
		sstrcat(output, "\"Exp\":%d,\"ExpLevel\":\"%s\",\"Perf\":%d,\"PerfLevel\":\"%s\"",countexp(x), charexp(countexp(x)), countperf(x), cperf(countperf(x)));
	}
	// ÏÔÊ¾°æÎñ¡¢Õ¾ÎñµÈĞÅÏ¢
	if(x->userlevel & PERM_BOARDS)
	{
		sstrcat(output, "\"BOARDBM\":[");
		sethomefile(filename, x->userid, "mboard");
		new_apply_record(filename, sizeof(struct boardmanager), (void *)bm_printboardapi, output);
		sstrcat(output, "],");
	}
	if((x->userlevel & PERM_SYSOP) && (x->userlevel&PERM_ARBITRATE))
		sstrcat(output, "\"Job\":\"±¾Õ¾¹ËÎÊÍÅ\",");
	else if(x->userlevel & PERM_SYSOP)
		sstrcat(output, "\"Job\":\"ÏÖÈÎÕ¾³¤\",");
	else if(x->userlevel & PERM_OBOARDS)
		sstrcat(output, "\"Job\":\"ÊµÏ°Õ¾³¤\",");
	else if(x->userlevel & PERM_ARBITRATE)
		sstrcat(output, "\"Job\":\"ÏÖÈÎ¼ÍÎ¯\",");
	else if(x->userlevel & PERM_SPECIAL4)
		sstrcat(output, "\"Job\":\"Çø³¤\",");
	else if(x->userlevel & PERM_WELCOME)
		sstrcat(output, "\"Job\":\"ÏµÍ³ÃÀ¹¤\",");
	else if(x->userlevel & PERM_SPECIAL7)
	{
		if((x->userlevel & PERM_SPECIAL1) && !(x->userlevel & PERM_CLOAK))
			sstrcat(output, "\"Job\":\"ÀëÈÎ³ÌĞòÔ±\",");
		else
			sstrcat(output, "\"Job\":\"³ÌĞò×é³ÉÔ±\",");
	}
	else if(x->userlevel & PERM_ACCOUNTS)
		sstrcat(output, "\"Job\":\"ÕÊºÅ¹ÜÀíÔ±\",");
		
	// ÏÔÊ¾µ±Ç°×´Ì¬£¬null »òÕß array
	num = 0;
	sstrcat(output, "\"States\":");
	for (i=0; i<MAXACTIVE; ++i)
	{
		u = &(shm_utmp->uinfo[i]);
		if(!strcmp(u->userid, x->userid))
		{
			if(u->active == 0 || u->pid ==0 || (u->invisible && !HAS_PERM(PERM_SEECLOAK)))
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
				// ×Ô¶¨Òå×´Ì¬
				sstrcat(output, "%s", u->user_state_temp);
		}
		if( (num>0) && (i == MAXACTIVE - 1))
			sstrcat(output, "],");
	}
	if (num == 0 )
	{
		sstrcat(output, "\"null\",");
		if( x->lastlogout != NULL )
			sstrcat(output, "\"LastLogout\":\"%s\",",Ctime(x->lastlogout));
		else
			sstrcat(output, "\"LastLogout\":null,");
	}
		
	
	// ÏÔÊ¾ËµÃ÷µµ
	sethomefile(filename,x->userid,"plans");
	fp = fopen(filename, "r");
	sprintf(filename, "00%s-plan", x->userid);
	if (fp) {
		sstrcat(output, "\"PersonalIntro\":\""); // ¿ªÊ¼¸öÈËËµÃ÷µµ
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
		sstrcat(output, "\","); // ½áÊø¸öÈËËµÃ÷µµ
		fclose(fp);
	}
	else //Ã»ÓĞ¸öÈËËµÃ÷µµ
		sstrcat(output, "\"PersonalIntro\":null,");
		
	// ÏÔÊ¾ÌØÊâ±êÇ©
	show_special_api(x->userid, output);
	// ½áÊøÊä³ö
	sstrcat(output, "}}"); 
	g2u(output,sizeof(output), output_utf8,sizeof(output_utf8));
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
	strsncpy(userid, getparm("U"), 13);
	if (!userid[0])
		strsncpy(userid, getparm("userid"), 13);
	printf("<body><center>");
	printf("<div class=rhead>%s -- ²éÑ¯ÍøÓÑ</div><hr>\n", BBSNAME);
	if (userid[0] == 0) {
		printf("<form action=bbsqry>\n");
		printf
		    ("ÇëÊäÈëÓÃ»§Ãû: <input name=userid maxlength=12 size=12>\n");
		printf("<input type=submit value=²éÑ¯ÓÃ»§>\n");
		printf("</form><hr>\n");
		http_quit();
	}
	x = getuser(userid);
	if (x == 0) {
		int i, j = 0;
		printf("Ã»ÓĞÕâ¸öÓÃ»§°¡£¬ÄÑµÀÊÇÕâĞ©:<p>");
		printf("<table width=600>");
		for (i = 0; i < shm_ucache->number; i++)
			if (strcasestr(shm_ucache->userid[i], userid) ==
			    shm_ucache->userid[i]) {
				j++;
				if (j % 6 == 1)
					printf("<tr>");
				printf("<td>");
				printf("<a href=bbsqry?userid=%s>%s</a>",
				       shm_ucache->userid[i],
				       shm_ucache->userid[i]);
				printf("</td>");
				sprintf(buf, "bbsqry?userid=%s",
					shm_ucache->userid[i]);
				if (j % 6 == 0)
					printf("</tr>");
				if (j >= 12 * 6)
					break;
			}
		printf("</table>");
		if (!j)
			printf("²»¿ÉÄÜ£¬¿Ï¶¨ÊÇÄãÇÃ´íÁË£¬¸ù±¾Ã»ÕâÈË°¡");
		if (j == 1)
			redirect(buf);
		printf("<p><a href=javascript:history.go(-1)>¿ìËÙ·µ»Ø</a>");
		http_quit();
	}
	printf("</center><pre style='font-size:14px'>\n");
	sprintf(buf,
		"%s ([33m%s[37m) ¹²ÉÏÕ¾ [1;32m%d[m ´Î£¬·¢±íÎÄÕÂ [1;32m%d[m Æª",
		x->userid, x->username, x->numlogins, x->numposts);
	hprintf("%s", buf);
	show_special_web(x->userid);//add by wjbta@bmy  Ôö¼Óid±êÊ¶
	printf("\n");
	hprintf("ÉÏ´ÎÔÚ [[1;32m%s[m] ´Ó [[1;32m%s[m] µ½±¾Õ¾Ò»ÓÎ¡£\n",
		Ctime(x->lastlogin), x->lasthost);
	mails(userid, &tmp2);
	hprintf("ĞÅÏä£º[[1;32m%s[m]£¬", tmp2 ? "¡Ñ" : "  ");
	if (!strcasecmp(x->userid, currentuser.userid)) {
		hprintf("¾­ÑéÖµ£º[[1;32m%d[m]([33m%s[m) ", countexp(x),
			charexp(countexp(x)));
		hprintf("±íÏÖÖµ£º[[1;32m%d[m]([33m%s[m) ", countperf(x),
			cperf(countperf(x)));
	}
	hprintf("ÉúÃüÁ¦£º[[1;32m%d[m]¡£\n", count_life_value(x));
	if (x->userlevel & PERM_BOARDS) {
		hprintf("µ£ÈÎ°æÎñ£º");
		sethomefile(filename, x->userid, "mboard");
		new_apply_record(filename, sizeof (struct boardmanager),
				 (void *) bm_printboard, NULL);
		if (x->userlevel & !strcmp(x->userid, "SYSOP")) hprintf("[[1;36mÏµÍ³¹ÜÀíÔ±[m]");
		else if (x->userlevel & !strcmp(x->userid, "lanboy")) hprintf("[[1;36mÏµÍ³¹ÜÀíÔ±[m]");
		else if ((x->userlevel&PERM_SYSOP) && (x->userlevel&PERM_ARBITRATE) )	hprintf("[[1;36m±¾Õ¾¹ËÎÊÍÅ[m]");
		else if (x->userlevel & PERM_SYSOP)	hprintf("[[1;36mÏÖÈÎÕ¾³¤[m]");
		else if (x->userlevel & PERM_OBOARDS)   hprintf("[[1;36mÊµÏ°Õ¾³¤[m]");
		else if (x->userlevel & PERM_ARBITRATE)	hprintf("[[1;36mÏÖÈÎ¼ÍÎ¯[m]");
		else if (x->userlevel & PERM_SPECIAL4)	hprintf("[[1;36mÇø³¤[m]");
		else if (x->userlevel & PERM_WELCOME) hprintf("[[1;36mÏµÍ³ÃÀ¹¤[m]");
		else if (x->userlevel & PERM_SPECIAL7)
		{
		if ( (x->userlevel & PERM_SPECIAL1) && !(x->userlevel & PERM_CLOAK) ) 
		hprintf("[[1;36mÀëÈÎ³ÌĞòÔ±[m]");		
		else		
		hprintf("[[1;36m³ÌĞò×é³ÉÔ±[m]");
		}
		else if (x->userlevel & PERM_ACCOUNTS) hprintf ("[[1;36mÕÊºÅ¹ÜÀíÔ±[m]"); 
		hprintf("\n");
	}
	num = 0;
	for (i = 0; i < MAXACTIVE; i++) {
		u = &(shm_utmp->uinfo[i]);
		if (!strcmp(u->userid, x->userid)) {
			if (u->active == 0 || u->pid == 0
			    || (u->invisible && !HAS_PERM(PERM_SEECLOAK)))
				continue;
			num++;
			if (num == 1)
				hprintf("Ä¿Ç°ÔÚÕ¾ÉÏ, ×´Ì¬ÈçÏÂ:\n");
			if (u->invisible)
				hprintf("[36mC[37m");
			if (u->mode != USERDF4)
			  hprintf("\033[%dm%s[m ", u->pid == 1 ? 35 : 32,
				ModeType(u->mode));
			else							/* ×Ô¶¨Òå×´Ì¬ */
			  hprintf("\033[%dm%s[m ", u->pid == 1 ? 35 : 32,
				u->user_state_temp);
			if (num % 5 == 0)
				printf("\n");
		}
	}
	if (num == 0) {
		hprintf("Ä¿Ç°²»ÔÚÕ¾ÉÏ, ÉÏ´ÎÀëÕ¾Ê±¼ä [[1;32m%s[m]\n\n",
			x->lastlogout ? Ctime(x->lastlogout) :
			"ÒòÔÚÏßÉÏ»ò²»Õı³£¶ÏÏß²»Ïê");
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
		hprintf("[36mÃ»ÓĞ¸öÈËËµÃ÷µµ[37m\n");
	}
	printf("</td></tr></table>");
	printf
	    ("<br><br><a href=bbspstmail?userid=%s&title=Ã»Ö÷Ìâ>[ÊéµÆĞõÓï]</a> ",
	     x->userid);
	printf("<a href=bbssendmsg?destid=%s>[·¢ËÍÑ¶Ï¢]</a> ", x->userid);
	printf("<a href=bbsfadd?userid=%s>[¼ÓÈëºÃÓÑ]</a> ", x->userid);
	printf("<a href=bbsfdel?userid=%s>[É¾³ıºÃÓÑ]</a>", x->userid);
	printf("<hr>");
	printf("<center><form action=bbsqry>\n");
	printf("ÇëÊäÈëÓÃ»§Ãû: <input name=userid maxlength=12 size=12>\n");
	printf("<input type=submit value=²éÑ¯ÓÃ»§>\n");
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
		if (fscanf(FCGI_ToFILE(fp), "%s %s", id1, name) <= 0)
			break;
		if (!strcmp(id1, id2))
			hprintf(" [1;31m¡ï[0;36m%s[1;31m¡ï[m", name);
	}
	fclose(fp);
}

int
bm_printboard(struct boardmanager *bm, void *farg)
{
	if (getboard(bm->board)){
		printf("<a href=%s%s target=f3>", showByDefMode(), bm->board);
		hprintf("%s", bm->board);
		printf("</a> ");
		hprintf(" ");
	} 
	return 0;
}

int bm_printboardapi(struct boardmanager *bm,char *farg)
{
	if (getboard(bm->board)){
		sstrcat(farg, "\"%s\",",bm->board);
	}
	return 0;
}
