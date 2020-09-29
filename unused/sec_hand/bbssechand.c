#include "bbslib.h"
#include "sec_hand.h"

void
short_stamp(str, chrono)
char *str;
time_t *chrono;
{
	struct tm *ptime;
	ptime = localtime(chrono);
	sprintf(str, "%2d/%02d", ptime->tm_mon + 1, ptime->tm_mday);
}

int
showheader(char *grp)
{
	char fpath[STRLEN];
	int fd;
	int valid = 0;
	SLOT tempslot;
	sprintf(fpath, "2nd/" FN_GRP);
	fd = open(fpath, O_RDONLY);
	if (strlen(grp))
		printf(" <a href=bbssechand>到市场门口</a> ");
	else
		printf(" [<a href=bbssechand>到市场门口</a>] ");
	if (fd > 0) {
		while (read(fd, &tempslot, sizeof (SLOT)) > 0) {
			if (!(tempslot.prop & PROP_IS_CANCEL))
				if (!strcmp(grp, tempslot.fn)) {
					printf(" [<a href=bbssechand?grp=%s>%s</a>] ", tempslot.fn, void1(titlestr(tempslot.title)));
					valid = 1;
				} else {
					printf(" <a href=bbssechand?grp=%s>%s</a> ", tempslot.fn, void1(titlestr(tempslot.title)));
				}
		}
		close(fd);
	}
	printf("<hr>");
	return valid;
}

void
showwelcome()
{
	showcon("etc/2ndhand_wwwdecl");
	printf("</center><br><font class=f2>");
	printf("相关版面：<br><li><a href=%ssecondhand>网上练摊版(secondhand)</a><br><li><a href=%sShopping>购物版(Shopping)</a>", showByDefMode(), showByDefMode());
}

void
showgroup(char *grp)
{
	char fpath[STRLEN];
	char grpdir[STRLEN];
	int fd;
	int i = 1;
	SLOT tempslot;
	sprintf(grpdir, "2nd/%s", grp);
	sprintf(fpath, "2nd/%s/%s", grp, FN_ITEM);
	if (!file_isdir(grpdir))
		http_fatal("该组不存在或已被删除");
	printf("<a href=bbssechand?grp=%s&mode=1&sell=0>我要购买物品</a> ", grp);
	printf("<a href=bbssechand?grp=%s&mode=1&sell=1>我要卖出物品</a>", grp);
	printhr();
	fd = open(fpath, O_RDONLY);
	if (fd > 0) {
		printf("<table><tr><td>序号</td><td>目标</td><td>作者</td><td>日期</td><td>标题</td><td>回信人数</td></tr>");
		while (read(fd, &tempslot, sizeof (SLOT)) > 0) {
			if (tempslot.prop & PROP_IS_CANCEL)
				continue;
			printf("<tr>");
			printf("<td>%d</td>", i);
			printf("<td>%s</td>", (tempslot.prop & PROP_I_SELL) ?  "<font color=green>转让</font>" : "<font color=red>求购</font>");
			printf("<td><a href=bbsqry?userid=%s>%s</a></td>", tempslot.userid, tempslot.userid);
			printf("<td>%s</td>", tempslot.date);
			printf("<td><a href=bbssechand?grp=%s&item=%s>%s</a></td>", grp, tempslot.fn, void1(titlestr(tempslot.title)));
			printf("<td>%d</td>", tempslot.reply);
			printf("</tr>");
			i++;
		}
		close(fd);
		printf("</table>");
	}
	printhr();
}

void
showitem(char *grp, char *item)
{
	char fpath[STRLEN];
	char dotitem[STRLEN];
	char prev[STRLEN], next[STRLEN];
	int fd;
	time_t t = atoi(item + 2);
	SLOT tempslot;
	sprintf(dotitem, "2nd/%s/%s", grp, FN_ITEM);
	sprintf(fpath, "2nd/%s/%s", grp, item);
	prev[0] = next[0] = '\0';
	fd = open(dotitem, O_RDONLY);
	if (fd > 0) {
		while (read(fd, &tempslot, sizeof (SLOT)) > 0) {
			if (!strcmp(tempslot.fn, item)
			    && !(tempslot.prop & PROP_IS_CANCEL)) {
				printf("<font size=+1>%s</font><br></center><font style='font-size:14px'>发布时间：%s<br>描述：<br>", void1(titlestr(tempslot.title)), ctime(&t));
				showcon(fpath);
				printf("希望价格: %s<br>", tempslot.price);
				printf("<a href=bbssechand?grp=%s&item=%s&mode=3>写信给%s</a><br>", grp, tempslot.fn, tempslot.userid);
				printf("其他联络方式: %s<br>", tempslot.contact);
				while (read(fd, &tempslot, sizeof (SLOT)) > 0) {
					if (!(tempslot.prop & PROP_IS_CANCEL)) {
						strcpy(next, tempslot.fn);
						break;
					}
				}
				break;
			}
			if (!(tempslot.prop & PROP_IS_CANCEL))
				strcpy(prev, tempslot.fn);
		}
		close(fd);
	}
	printf("<hr>");
	if (prev[0] != '\0')
		printf("<a href=bbssechand?grp=%s&item=%s>上一篇</a> ", grp, prev);
	printf("<a href=bbssechand?grp=%s>本组所有信息</a> ", grp);
	if (next[0] != '\0')
		printf("<a href=bbssechand?grp=%s&item=%s>下一篇</a> ", grp, next);
}

void
postnewslot(char *grp)
{
	printf("%s -- 二手市场 [使用者: %s]<hr>\n", BBSNAME, currentuser.userid);
	printf("<table border=1>\n");
	printf("<tr><td>");
	printf("<font color=green>发文注意事项: <br>\n");
	printf("发文时应慎重考虑文章内容是否适合公开场合发表，请勿肆意灌水。谢谢您的合作。<br>"
	     "在此发表的广告同时会张贴到Secondhand版版面，所以不必重复发文。\n</font>");
	printf("<tr><td><form name=form1 method=post action=bbssechand?grp=%s&mode=2>\n", grp);
	printf("使用标题: <input type=text name=title size=30 maxlength=30>");
	printf(" 作者：%s", currentuser.userid);
	printf("<br>您所希望的价格：<input type=text name=price size=10 maxlength=10>");
	printf(" 除回信外其他联络方式：<input type=text name=contact size=20 maxlength=20>");
	if (atoi(getparm("sell")) == 0) {
		printf("<input type=radio name=sell value=0 checked>买 <input type=radio name=sell value=1>卖");
	} else {
		printf("<input type=radio name=sell value=0>买 <input type=radio name=sell value=1 checked>卖");
	}
	printf("<br>\n<textarea  onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.form1.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.form1.submit()' name=text rows=20 cols=76 wrap=physical class=f2>\n\n");
	printf("</textarea>\n");
	printf("<tr><td>");
	printf("<tr><td class=post align=center><input type=submit value=发表> ");
	printf("<input type=reset value=清除></form>\n");
	printf("</table>");
}

int
savenewslot(char *grp)
{

	FILE *fp, *fp2;
	char filename[80], title[30], buf[256], buf3[1024], *content;
	size_t i;
	int sell, fd;
	struct SLOT tosave;
	time_t t = 0;		//to make gcc happy,let t has a initial value.
	bzero(&tosave, sizeof (tosave));
	ytht_strsncpy(title, getparm("title"), 30);
	sell = atoi(getparm("sell"));
	ytht_strsncpy(tosave.price, getparm("price"), 10);
	ytht_strsncpy(tosave.contact, getparm("contact"), 20);
	for (i = 0; i < strlen(title); i++)
		if (title[i] <= 27 && title[i] >= -1)
			title[i] = ' ';
	i = strlen(title) - 1;
	while (i > 0 && isspace(title[i]))
		title[i--] = 0;
	content = getparm("text");
	if (title[0] == 0)
		http_fatal("文章必须要有标题");
	if ((now_t - w_info->lastposttime) < 6) {
		w_info->lastposttime = now_t;
		http_fatal("两次发文间隔过密, 请休息几秒后再试");
	}
	w_info->lastposttime = now_t;
	sprintf(filename, "bbstmpfs/tmp/%d.tmp", thispid);
	f_write(filename, content);
	for (i = 0; i < 100; i++) {
		t = now_t + i;
		sprintf(buf3, "2nd/%s/M.%ld.A", grp, t);
		if (!file_exist(buf3))
			break;
	}
	if (i >= 100)
		return -1;
	ytht_strsncpy(tosave.title, title, 30);
	fp = fopen(buf3, "w");
	fp2 = fopen(filename, "r");
	fprintf(fp, "%s(%s), 发布信息时ip: \033[35m%s\033[m\n\n", currentuser.userid, currentuser.username, fromhost);
	if (fp2 != 0) {
		while (1) {
			if (fgets(buf3, 1000, fp2) == NULL)
				break;
			fprintf2(fp, buf3);
		}
		fclose(fp2);
	}
	fclose(fp);
	sprintf(buf3, "\n希望价格: %s\n联系方式: %s\n(本文为 WWW 二手市场广告的复本)", tosave.price, tosave.contact);
	f_append(filename, buf3);
	post_article("SecondHand", title, filename, currentuser.userid, currentuser.username, fromhost, 0, 0, 0, currentuser.userid, -1);
	unlink(filename);
	tosave.chrono = t;
	sprintf(tosave.fn, "M.%ld.A", t);
	short_stamp(tosave.date, &(tosave.chrono));
	if (!sell)
		tosave.prop = PROP_I_WANT;
	else
		tosave.prop = PROP_I_SELL;
	tosave.reply = 0;
	strcpy(tosave.userid, currentuser.userid);
	sprintf(buf3, "2nd/%s/.ITEM", grp);
	if (insert_record(buf3, &tosave, sizeof (SLOT), 0, 1))
		return -1;	// the newer the upper
	sprintf(buf3, "2nd/" FN_GRP);
	fd = open(buf3, O_RDWR);
	if (fd > 0) {
		while (read(fd, &tosave, sizeof (SLOT)) > 0) {
			if (!strcmp(tosave.fn, grp)) {
				tosave.reply++;
				lseek(fd, -sizeof (SLOT), SEEK_CUR);
				write(fd, &tosave, sizeof (SLOT));
				break;
			}
		}
		close(fd);
	}
	sprintf(buf, "bbssechand?grp=%s", grp);
	redirect(buf);
	return 0;
}

int
replymail(char *grp, char *item)
{
	char fpath[STRLEN];
	char dotitem[STRLEN];
	int fd;
	SLOT tempslot;
	int found = 0;
	char userid[IDLEN + 1];
	char title[30];
	sprintf(dotitem, "2nd/%s/%s", grp, FN_ITEM);
	sprintf(fpath, "2nd/%s/%s", grp, item);
	fd = open(dotitem, O_RDWR);
	if (fd > 0) {
		while (read(fd, &tempslot, sizeof (SLOT)) > 0) {
			if (!strcmp(tempslot.fn, item)) {
				found = 1;
				strcpy(userid, tempslot.userid);
				strcpy(title, tempslot.title);
				tempslot.reply++;
				lseek(fd, -sizeof (SLOT), SEEK_CUR);
				write(fd, &tempslot, sizeof (SLOT));
				break;
			}
		}
		close(fd);
	}
	if (found) {
		printf("<hr>");
		printf("<center>\n");
		printf("%s -- 寄语信鸽 [使用者: %s]<hr>\n", BBSNAME, currentuser.userid);
		printf("<table border=1><tr><td>\n");
		printf("<form name=form1 method=post action=bbssndmail?userid=%s>\n", userid);
		printf("信件标题: <input type=text name=title size=40 maxlength=100 value='%s'> ", (noquote_html(title)));
		printf("发信人: &nbsp;%s<br>\n", currentuser.userid);
		printf("收信人: &nbsp;&nbsp;<input type=text name=userid value='%s'> ", nohtml(userid));
		printselsignature();
		printf(" 备份<input type=checkbox name=backup>\n");
		printf("<br>\n");
		printf("<textarea  onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.form1.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.form1.submit()' name=text rows=20 cols=80 wrap=physical>\n\n");
		{
			int lines = 0, i;
			FILE *fp;
			char buf[500];
			printf("【 在 %s 的大作中提到: 】\n", userid);
			fp = fopen(fpath, "r");
			if (fp) {
				for (i = 0; i < 1; i++)
					if (fgets(buf, 500, fp) == 0)
						break;
				while (1) {
					if (fgets(buf, 500, fp) == 0)
						break;
					if (!strncmp(buf, ": 【", 4))
						continue;
					if (!strncmp(buf, ": : ", 4))
						continue;
					if (!strncmp(buf, "--\n", 3)
					    || !strncmp(buf, "begin 644 ", 10))
						break;
					if (buf[0] == '\n')
						continue;;
					printf(": %s", nohtml(void1(buf)));
					if (lines++ >= 20) {
						printf(": (以下引言省略...)\n");
						break;
					}
				}
				fclose(fp);
			}
			printf("</textarea><br><div align=center>\n");
			printf("<tr><td align=center><input type=submit value=发送> ");
			printf("<input type=reset value=清除></form>\n");
			printf("</div></table>");
		}
	}
	http_quit();
	return 0;
}

int
bbssechand_main()
{
	char curr_grp[15], curr_item[15];
	int mode;
	int valid_grp;
	html_header(1);
	check_msg();
	changemode(M_2NDHAND);
	strncpy(curr_grp, getparm("grp"), 15);
	strncpy(curr_item, getparm("item"), 15);
	mode = atoi(getparm("mode"));
	printf("<body><center>");
	if (strstr(curr_item, ".GRP") || strstr(curr_item, ".ITEM") || strstr(curr_item, "..") || strstr(curr_item, "SYSHome") || strstr(curr_grp, "..") || strstr(curr_grp, "SYSHome"))
		http_fatal("错误的参数");
	valid_grp = showheader(curr_grp);
	if (mode && valid_grp) {
		if (!loginok || isguest)
			http_fatal("匆匆过客不能进行交易，请先登录");
		if (mode == 1)
			postnewslot(curr_grp);
		else if (mode == 2)
			savenewslot(curr_grp);
		else if (mode == 3)
			replymail(curr_grp, curr_item);
	} else if (curr_item[0] == '\0' && valid_grp)
		showgroup(curr_grp);
	else if (valid_grp)
		showitem(curr_grp, curr_item);
	else
		showwelcome();
	printf("</center></body>");
	http_quit();
	return 0;
}
