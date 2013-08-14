#include <dirent.h>
//int versionsort(const void *a, const void *b);
#include "bbslib.h"

const static char *field[] =
    { "usernum", "userid", "realname", "dept", "addr", "phone", "assoc",
	NULL
};
const static char *finfo[] =
    { "帐号位置", "申请帐号", "真实姓名", "学校系级", "目前住址",
	"连络电话", "毕业学校", NULL
};

#define NUMREASON 5
const static char *reason[] =
    { "请确实填写真实姓名。", "请填学校科系年级或工作单位。",
	"请填写完整的住址资料。",
	"请详细、确实填写注册资料。",
	"请用中文填写申请单。"
};

static int
countnumforms(char *filename)
{
	int i = 0;
	char buf[256];
	FILE *fp;
	int lockfd;
	lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX);
	fp = fopen(filename, "r");
	if (!fp) {
		close(lockfd);
		return 0;
	}
	while (fgets(buf, sizeof (buf), fp)) {
		if (!strncmp(buf, "usernum:", 8))
			i++;
	}
	fclose(fp);
	close(lockfd);
	return i;
}

static void
scanreg_info()
{
	int i, n;
	char buf[256];
	struct dirent **namelist;
	n = countnumforms("new_register");
	printf("Total number of forms: %d<br>", n);
	printf("<a href=bbsscanreg?STEP=1&N=20>Get 20 forms</a><br>");
	printf("<a href=bbsscanreg?STEP=1&N=30>Get 30 forms</a><br>");
	printf("<a href=bbsscanreg?STEP=1&N=40>Get 40 forms</a><br>");
	printf("<a href=bbsscanreg?STEP=1&N=60>Get 60 forms</a><br>");
	printf("Pick up list<br>");
	i = scandir(SCANREGDIR, &namelist, 0, versionsort);
	if (!i) {
		printf("No file for picking up<br>");
		return;
	}
	while (i--) {
		if (!strncmp(namelist[i]->d_name, "R.", 2)) {
			printf("<li><a href=bbsscanreg?STEP=1&F=%s>%s</a>",
			       namelist[i]->d_name, namelist[i]->d_name);
			sprintf(buf, "%s/%s", SCANREGDIR, namelist[i]->d_name);
			printf(" n=%d", countnumforms(buf));
			printf(" %s", Ctime(atoi(namelist[i]->d_name + 2)));
		}
		free(namelist[i]);
	}
	free(namelist);
}

static void
scanreg_readforms()
{
	FILE *fp;
	int n, count = 0;
	char buf[256], *ptr;
	char filename[256];
	char fdata[7][STRLEN];
	struct userec uinfo;
	int unum;
	ptr = getparm("F");
	if (*ptr) {
		if (strstr(ptr, "..") || strchr(ptr, '/')
		    || strncmp(ptr, "R.", 2))
			http_fatal("Wrong parameter!");
		snprintf(filename, sizeof (filename), SCANREGDIR "/%s", ptr);
		if (!file_exist(filename))
			http_fatal("No such file!");
		count = countnumforms(filename);
		if (!count) {
			unlink(filename);
			http_fatal("No form in this list (%s)", ptr);
		}
	} else {
		count = countnumforms(NEWREGFILE);
		if (!count)
			http_fatal("No new form");
		n = atoi(getparm("N"));
		if (n < 10 || n > 60)
			n = 20;
		if (getregforms(filename, n, currentuser.userid) < 0)
			http_fatal("failed to get reg forms!");
	}
	count = countnumforms(filename);
	if (!count)
		http_fatal("No forms in this list");
	count = 0;
	printf("<center><form action=bbsscanreg name=regformlist method=post>");
	printf("<input type=hidden name=STEP value='2'>");
	printf("<input type=hidden name=F value=%s>",
	       strrchr(filename, '/') + 1);
	printf("<table border=1>");
	fp = fopen(filename, "r");
	while (fgets(buf, STRLEN, fp) != NULL) {
		if ((ptr = strstr(buf, ": ")) != NULL) {
			*ptr = '\0';
			for (n = 0; field[n] != NULL; n++) {
				if (strcmp(buf, field[n]) != 0)
					continue;
				strsncpy(fdata[n], ptr + 2, sizeof (fdata[n]));
				if ((ptr = strchr(fdata[n], '\n')) != NULL)
					*ptr = '\0';
			}
			continue;
		}
		if ((unum = getusernum(fdata[1]) + 1) <= 0) {
			printf("系统错误, 查无此帐号.%d\n\n", unum);
			for (n = 0; field[n] != NULL; n++)
				printf("%s     : %s\n", finfo[n], nohtml(void1(fdata[n])));
			memset(fdata, 0, sizeof (fdata));
			continue;
		}
		if (atoi(fdata[0]) != unum) {
			printf("系统错误, 查无此帐号. %d\n\n", unum);
			for (n = 0; field[n] != NULL; n++)
				printf("%s     : %s\n", finfo[n], fdata[n]);
			memset(fdata, 0, sizeof (fdata));
			continue;
		}
		uinfo = *getuser(fdata[1]);
		printf("<tr><td width=30><nobr>");
		printf("帐号位置: %s<br>", fdata[0]);
		printf("&nbsp; &nbsp; 代号: <font class=blu>%s</font><br>",
		       uinfo.userid);
		printf("真实姓名: <font class=blu>%s</font><br>", fdata[2]);
		printf("居住住址: <font class=blu>%s</font><br>", fdata[4]);
		printf("学校系级: <font class=blu>%s</font><br>", fdata[3]);
		printf("电子邮箱: %s<br>", uinfo.email);
		printf("连络电话: %s<br>", fdata[5]);
		printf("毕业学校: %s<br>", fdata[6]);
		printf("帐号建立日期: %s<br>", Ctime(uinfo.firstlogin));
		printf("最近光临日期: %s<br>", Ctime(uinfo.lastlogin));
		printf("最近光临机器: %s", uinfo.lasthost);
		printf("</td><td valign=top>");
		if (uinfo.userlevel & PERM_LOGINOK) {
			printf("此帐号不需再填写注册单.");
		} else {
			printf("<input type=hidden name=userid%d value='%s'>",
			       count, uinfo.userid);
			printf
			    ("<input type=radio name=result%d value=PASS checked>PASS",
			     count);
			printf
			    (" <input type=radio name=result%d value=SKIP>SKIP",
			     count);
			printf
			    (" <input type=radio name=result%d value=DEL>DEL<br>",
			     count);
			for (n = 0; n < NUMREASON; n++) {
				printf
				    ("<input type=radio name=result%d value=%d>%s<br>",
				     count, n, reason[n]);
			}
			count++;
		}
		printf("</td></tr>");
		memset(fdata, 0, sizeof (fdata));
	}
	printf("</table><input type=submit value=submit></form>");
	fclose(fp);
}

static void
scanreg_done()
{
	const char *(results[60]), *(userids[60]);
	char fdata[7][STRLEN];
	char buf[256], filename[256], *ptr;
	int nresult = 0, i, n, lockfd, unum;
	FILE *fp, *fpskip;
	struct userec uinfo;
	for (i = 0; i < 60; i++) {
		sprintf(buf, "result%d", i);
		results[nresult] = getparm(buf);
		if (!*results[nresult])
			continue;
		if (isdigit(*results[nresult])) {
			n = atoi(results[nresult]);
			if (n < 0 || n >= NUMREASON)
				continue;
			results[nresult] = reason[n];
		}
		sprintf(buf, "userid%d", i);
		userids[nresult] = getparm(buf);
		if (!*userids[nresult])
			continue;
		nresult++;
	}
	for (i = 0; i < nresult; i++) {
		printf("%s %s<br>", results[i], userids[i]);
	}
	ptr = getparm("F");
	if (strstr(ptr, "..") || strchr(ptr, '/') || strncmp(ptr, "R.", 2))
		http_fatal("Wrong parameter!");
	snprintf(filename, sizeof (filename), SCANREGDIR "/%s", ptr);

	fp = fopen(filename, "r");
	if (!fp)
		http_fatal("File Not Found!");
	lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX);
	fpskip = fopen(NEWREGFILE, "a");
	while (fgets(buf, STRLEN, fp) != NULL) {
		if ((ptr = strstr(buf, ": ")) != NULL) {
			*ptr = '\0';
			for (i = 0; field[i] != NULL; i++) {
				if (strcmp(buf, field[i]) != 0)
					continue;
				strsncpy(fdata[i], ptr + 2, sizeof (fdata[i]));
				if ((ptr = strchr(fdata[i], '\n')) != NULL)
					*ptr = '\0';
			}
			continue;
		}
		if ((unum = getusernum(fdata[1]) + 1) <= 0) {
			bzero(fdata, sizeof (fdata));
			continue;
		}
		if (atoi(fdata[0]) != unum) {
			memset(fdata, 0, sizeof (fdata));
			continue;
		}
		uinfo = *getuser(fdata[1]);
		if (uinfo.userlevel & PERM_LOGINOK)
			continue;
		for (n = 0; n < nresult; n++) {
			if (!strcmp(userids[n], uinfo.userid))
				break;
		}
		if (n >= nresult || !strcmp(results[n], "SKIP")) {
			for (n = 0; field[n] != NULL; n++) {
				fprintf(fpskip, "%s: %s\n", field[n], fdata[n]);
			}
			fprintf(fpskip, "----\n");
			bzero(fdata, sizeof (fdata));
			continue;
		}
		if (!strcmp(results[n], "DEL")) {
			bzero(fdata, sizeof (fdata));
			uinfo.userlevel &= ~PERM_DEFAULT;
			save_user_data(&uinfo);
			sprintf(buf, "删除 %s 的注册单.", uinfo.userid);
			securityreport(buf, buf);
			continue;
		}
		if (!strcmp(results[n], "PASS")) {
			FILE *fout;
			char buf2[256];
			i = strlen(fdata[5]);
			if (i + strlen(fdata[3]) > 60) {
				if (i > 40)
					fdata[5][i = 40] = '\0';
				fdata[3][60 - i] = '\0';
			}
			strsncpy(uinfo.realname, fdata[2],
				 sizeof (uinfo.realname));
			strsncpy(uinfo.address, fdata[4],
				 sizeof (uinfo.address));
			sprintf(buf, "%s$%s@%s", fdata[3], fdata[5],
				currentuser.userid);
			strsncpy(uinfo.realmail, buf, sizeof (uinfo.realmail));
			uinfo.userlevel |= PERM_DEFAULT;	// by ylsdd
			save_user_data(&uinfo);
			sethomefile(buf, uinfo.userid, "sucessreg");
			f_write(buf, "\n");
			sethomefile(buf, uinfo.userid, "register");
			if (file_exist(buf)) {
				sethomefile(buf2, uinfo.userid, "register.old");
				rename(buf, buf2);
			}
			if ((fout = fopen(buf, "w")) != NULL) {
				for (i = 0; field[i] != NULL; i++)
					fprintf(fout, "%s: %s\n", field[i],
						fdata[i]);
				fprintf(fout, "Date: %s\n", Ctime(time(NULL)));
				fprintf(fout, "Approved: %s\n",
					currentuser.userid);
				fclose(fout);
			}
			mail_file("etc/s_fill", uinfo.userid,
				  "恭禧您通过身份验证", currentuser.userid);
			mail_file("etc/s_fill2", uinfo.userid,
				  "欢迎加入" MY_BBS_NAME "大家庭",
				  currentuser.userid);
			sethomefile(buf, uinfo.userid, "mailcheck");
			unlink(buf);
			sprintf(buf, "让 %s 通过身分确认.", uinfo.userid);
			securityreport(buf, buf);
		} else {
			snprintf(buf, sizeof (buf), "<注册失败>-%s",
				 results[n]);
			strsncpy(uinfo.address, buf, sizeof (uinfo.address));
			save_user_data(&uinfo);
			mail_file("etc/f_fill", uinfo.userid, buf,
				  currentuser.userid);
		}
		bzero(fdata, sizeof (fdata));
	}
	unlink(filename);
	fclose(fp);
	fclose(fpskip);
	close(lockfd);
}

int
bbsscanreg_main()
{	//modify by mintbaggio 20040829 for new www
	html_header(1);
	check_msg();
	printf("<body>");
	if (!loginok || isguest || !HAS_PERM(PERM_ACCOUNTS))
		http_fatal("unknown request");
	printf("<nobr><center><div class=rhead>%s -- scan reg form</div></center><hr>\n", BBSNAME);
	switch (atoi(getparm("STEP"))) {
	case 1:
		scanreg_readforms();
		break;
	case 2:
		scanreg_done();
		printf("<a href=bbsscanreg>Continue</a>");
		break;
	case 0:
	default:
		scanreg_info();
	}
	http_quit();
	return 0;
}
