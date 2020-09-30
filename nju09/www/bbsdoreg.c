#include "bbslib.h"
#include "ythtbbs/identify.h"

#ifdef POP_CHECK
// 登陆邮件服务器用的头文件 added by interma@BMY 2005.5.12
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// 邮件服务器上用户名和密码的长度， added by interma@BMY 2005.5.12
#define USER_LEN 20
#define PASS_LEN 20

#endif

#ifdef POP_CHECK
// 令username用户通过验证， added by interma@BMY 2005.5.12
void register_success(int usernum, char *userid, char *realname, char *dept,
char *addr, char *phone, char *assoc, char *email)
{
	struct userec uinfo;
	FILE *fout, *fn;
	char buf[STRLEN];
	time_t n;
	char genbuf[512];

	//int id = getuser(userid);
	struct userec *u = getuser(userid);

	sethomefile(genbuf, userid, "mailcheck");
	//http_fatal(genbuf);
	if ((fn = fopen(genbuf, "w")) == NULL) {
		fclose(fn);
		return;
	}

	fprintf(fn, "usernum: %d\n", usernum);
	fclose(fn);

	memcpy(&uinfo, u, sizeof (uinfo));

	ytht_strsncpy(uinfo.userid, userid, sizeof(uinfo.userid));
	ytht_strsncpy(uinfo.realname, realname, sizeof(uinfo.realname));
	ytht_strsncpy(uinfo.address, addr, sizeof(uinfo.address));
	sprintf(genbuf, "%s$%s@%s", dept, phone, userid);
	ytht_strsncpy(uinfo.realmail, genbuf, sizeof(uinfo.realmail));

	ytht_strsncpy(uinfo.email, email, sizeof(uinfo.email));

	uinfo.userlevel |= PERM_DEFAULT;	// by ylsdd
	substitute_record(PASSFILE, &uinfo, sizeof (struct userec), usernum);

	sethomefile(buf, uinfo.userid, "sucessreg");
	if ((fout = fopen(buf, "w")) != NULL) {
		fprintf(fout, "\n");
		fclose(fout);
	}

	sethomefile(buf, uinfo.userid, "register");

	if ((fout = fopen(buf, "w")) != NULL) {
		fprintf(fout, "%s: %d\n", "usernum", usernum);
		fprintf(fout, "%s: %s\n", "userid", userid);
		fprintf(fout, "%s: %s\n", "realname", realname);
		fprintf(fout, "%s: %s\n", "dept", dept);
		fprintf(fout, "%s: %s\n", "addr", addr);
		fprintf(fout, "%s: %s\n", "phone", phone);
		fprintf(fout, "%s: %s\n", "assoc", assoc);

		n = time(NULL);
		fprintf(fout, "Date: %s\n", ctime(&n));
		fprintf(fout, "Approved: %s\n", userid);
		fclose(fout);
	}

	mail_file("etc/s_fill", uinfo.userid, "恭禧您通过身份验证", "SYSOP");

	mail_file("etc/s_fill2", uinfo.userid, "欢迎加入" MY_BBS_NAME "大家庭", "SYSOP");
	sethomefile(buf, uinfo.userid, "mailcheck");
	unlink(buf);
	sprintf(genbuf, "让 %s 通过身分确认.", uinfo.userid);
	securityreport(genbuf, genbuf);
	return ;
}

#endif
// -------------------------------------------------------------------------------


#if 0
int
badymd(int y, int m, int d)
{
	int max[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)
		max[2] = 29;
	if (y < 10 || y > 100 || m < 1 || m > 12)
		return 1;
	if (d < 0 || d > max[m])
		return 1;
	return 0;
}
#endif

int
bbsdoreg_main()
{
	FILE *fp;
	struct userec x;
	char buf[256], filename[80], pass1[80], pass2[80], dept[80], phone[80],
	    assoc[80], salt[3], words[1024], *ub = FIRST_PAGE;
	int lockfd;
	struct active_data act_data;
	html_header(1);
	printf("<body>");
	bzero(&x, sizeof (x));
//      xz=atoi(getparm("xz"));

#ifdef POP_CHECK
	char user[USER_LEN + 1];
	char popserver[4];
	int popserver_index;
	ytht_strsncpy(popserver, getparm("popserver"), 4);
	ytht_strsncpy(user, getparm("user"), USER_LEN);
#endif

	ytht_strsncpy(x.userid, getparm("userid"), 13);
	ytht_strsncpy(pass1, getparm("pass1"), 13);
	ytht_strsncpy(pass2, getparm("pass2"), 13);
	ytht_strsncpy(x.username, getparm("username"), 32);
	ytht_strsncpy(x.realname, getparm("realname"), 32);
	ytht_strsncpy(dept, getparm("dept"), 60);
	ytht_strsncpy(x.address, getparm("address"), 60);

#ifndef POP_CHECK
	strsncpy(x.email, getparm("email"), 60);
#else
	const char *popname;
	popserver_index = atoi(popserver);
	if(popserver_index < 1 || popserver_index > 3) popserver_index = 1;

	popname = MAIL_DOMAINS[popserver_index];

	char email[60];
	snprintf(email, 60, "%s@%s", user, popname);  // 注意不要将email弄溢出了
	ytht_str_to_lowercase(email);
	ytht_strsncpy(x.email, email, 60);
#endif

	ytht_strsncpy(phone, getparm("phone"), 60);
	ytht_strsncpy(assoc, getparm("assoc"), 60);
	ytht_strsncpy(words, getparm("words"), 1000);

    if (id_with_num(x.userid))
		http_fatal("帐号只能由英文字母组成");
	if (strlen(x.userid) < 2)
		http_fatal("帐号长度太短(2-12字符)");
	if (strcasecmp(x.userid, "new") == 0)
		http_fatal("禁止使用 new 作为账号名");
	if (strlen(pass1) < 4)
		http_fatal("密码太短(至少4字符)");
	if (strcmp(pass1, pass2))
		http_fatal("两次输入的密码不一致, 请确认密码");
	if (strlen(x.username) < 2)
		http_fatal("请输入昵称(昵称长度至少2个字符)");
	if (strlen(x.realname) < 4)
		http_fatal("请输入真实姓名(请用中文, 至少2个字)");
//      if(strlen(dept)<6) http_fatal("工作单位的名称长度至少要6个字符(或3个汉字)");
	if (strlen(x.address) < 6)
		http_fatal("通讯地址长度至少要6个字符(或3个汉字)");
	if (badstr(x.passwd) || badstr(x.username) || badstr(x.realname))
		http_fatal("您的注册单中含有非法字符");
	if (badstr(x.address) || badstr(x.email))
		http_fatal("您的注册单中含有非法字符");
	if (is_bad_id(x.userid))
		http_fatal("不雅帐号或禁止注册的id, 请重新选择");
	if (getuser(x.userid))
		http_fatal("此帐号已经有人使用,请重新选择。");
//      sprintf(salt, "%c%c", 65+rand()*26, 65+rand()*26);
//add by lepton


#ifdef POP_CHECK
	if (strlen(user) == 0)
		http_fatal("邮箱用户名没添啊");
	if (check_mail_to_address(email) == MAIL_SENDER_WRONG_EMAIL)
		http_fatal("您的邮箱名不合法，请联系站长或至 https://github.com/bmybbs/bmybbs/issues/ 反馈问题。");
#endif

	getsalt(salt);
	ytht_strsncpy(x.passwd, ytht_crypt_crypt1(pass1, salt), 14);
	//ipv6 by leoncom 不能赋值太多，就影响后面的数据 fixed by IronBlood 2020.09.11
	strncpy(x.lasthost, fromhost,BMY_IPV6_LEN);
	x.userlevel = PERM_BASIC;
	x.firstlogin = now_t;
	x.lastlogin = now_t - 3600;  //ipv6 by leoncom 注册后手动登录
	x.userdefine = -1;
	x.flags[0] = CURSOR_FLAG | PAGER_FLAG;
//      if(xz==1) currentuser.userdefine ^= DEF_COLOREDSEX;
//      if(xz==2) currentuser.userdefine ^= DEF_S_HOROSCOPE;
	adduser(&x);

#ifndef POP_CHECK
	lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX);
	fp = fopen("new_register", "a");
	if (fp) {
		fprintf(fp, "usernum: %d, %s\n", getusernum(x.userid) + 1,
			ytht_ctime(now_t));
		fprintf(fp, "userid: %s\n", x.userid);
		fprintf(fp, "realname: %s\n", x.realname);
		fprintf(fp, "dept: %s\n", dept);
		fprintf(fp, "addr: %s\n", x.address);
		fprintf(fp, "phone: %s\n", phone);
		fprintf(fp, "assoc: %s\n", assoc);
		fprintf(fp, "----\n");
		fclose(fp);
	}
	close(lockfd);
#endif

	sprintf(filename, "home/%c/%s", mytoupper(x.userid[0]), x.userid);
	mkdir(filename, 0755);

#ifndef POP_CHECK
	printf("<center><table><td><td><pre>\n");
	printf("亲爱的新使用者，您好！\n\n");
	printf("欢迎光临 本站, 您的新帐号已经成功被登记了。\n");
	printf("您目前拥有本站基本的权限, 包括阅读文章、环顾四方、接收私人\n");
	printf("信件、接收他人的消息、进入聊天室等等。当您通过本站的身份确\n");
	printf("认手续之后，您还会获得更多的权限。目前您的注册单已经被提交\n");
	printf("等待审阅。一般情况24小时以内就会有答复，请耐心等待。同时请\n");
	printf("留意您的站内信箱。\n");
	printf("如果您有任何疑问，可以去sysop(站长的工作室)版发文求助。\n\n</pre></table>");
	printf("<hr><br>您的基本资料如下:<br>\n");
	printf("<table border=1 width=400>");
	printf("<tr><td>帐号位置: <td>%d\n", getusernum(x.userid));
	printf("<tr><td>使用者代号: <td>%s (%s)\n", x.userid, x.username);
	printf("<tr><td>姓  名: <td>%s<br>\n", x.realname);
	printf("<tr><td>昵  称: <td>%s<br>\n", x.username);
	printf("<tr><td>上站位置: <td>%s<br>\n", x.lasthost);
	printf("<tr><td>电子邮件: <td>%s<br></table><br>\n", x.email);

	printf("<center><form><input type=button onclick='window.close()' value=关闭本窗口></form></center>\n");
#else
	printf("<center><table><td><td><pre>\n");

	memset(&act_data, 0, sizeof(act_data));
	snprintf(act_data.name, NAMELEN, "%s", x.realname);
	act_data.name[NAMELEN-1] = '\0';
	strcpy(act_data.userid, x.userid);
	snprintf(act_data.dept, STRLEN, "%s", dept);
	act_data.dept[STRLEN-1] = '\0';
	snprintf(act_data.phone, VALUELEN, "%s", phone);
	act_data.phone[VALUELEN-1] = '\0';
	snprintf(act_data.email, VALUELEN, "%s", email);
	act_data.email[VALUELEN-1] = '\0';
	strcpy(act_data.ip, fromhost);
	strcpy(act_data.operator, x.userid);

	act_data.status=0;
	write_active(&act_data);

	int result;
	if (strcasecmp(user, "test") == 0) {
		result = -2; // 新生
	} else if (query_record_num(email, MAIL_ACTIVE) >= MAX_USER_PER_RECORD) {
		result = -3;
	} else {
		// smtp
		result = send_active_mail(x.userid, email);
	}
	switch (result)
	{
	case -2:
		printf("<tr><td>%s<br></table><br>\n",
				"欢迎您加入交大，来到兵马俑BBS。<br>您采用了新生测试信箱注册，目前您是新生用户身份。"
				"目前您没有发文、信件、消息等权限。<br><br>"
				"请在开学取得stu.xjtu.edu.cn信箱后，<br>点击左侧边栏“填写注册单”，完成信箱绑定认证操作，成为本站正式用户。");
		break;
	case -3:
		printf("您的信箱已经验证过 %d 个id，无法再用于验证了!\n", MAX_USER_PER_RECORD);
		break;
	case -1:
	case 0:
		printf("<tr><td>%s<br></table><br>\n", "邮件服务器身份审核失败，您将只能使用本bbs的最基本功能，十分抱歉。");
		break;

	case 1:
		printf("欢迎您加入交大，来到兵马俑BBS。<br>"
			"目前您没有发文、信件、消息等权限。<br>"
			"验证信息已发送至您的邮箱 %s ，及时请查收。<br>"
			"请登录系统后点击左侧边栏“填写注册单”，完成信箱绑定认证操作，成为本站正式用户。", email);
		break;
	}

	printf("<center><form><input type=button onclick='window.close()' value=关闭本窗口></form></center>\n");
#endif

	// 以下这行（newcomer）可能将会引起www（ia64）下的问题。interma@BMY
	newcomer(&x, words);


//      sprintf(buf, "%s %-12s %d\n", ytht_ctime(now_t)+4, x.userid, getusernum(x.userid));
//      f_append("wwwreg.log", buf);
	sprintf(buf, "%s newaccount %d %s www", x.userid, getusernum(x.userid), fromhost);
	newtrace(buf);
	//wwwstylenum = 1;

	//don't login with reg by leoncom for ipv6
	//ub = wwwlogin(&x,0);
	//sprintf(buf, "%s enter %s www", x.userid, fromhost);
	//newtrace(buf);
	//printf("<script>opener.top.location.href=\"%s\";</script>", ub);
	return 0;
}

void
newcomer(struct userec *x, char *words)
{
	FILE *fp;
	char filename[80];
	sprintf(filename, "bbstmpfs/tmp/%d.tmp", thispid);
	fp = fopen(filename, "w");
	fprintf(fp, "大家好, \n\n");
	fprintf(fp, "我是 %s(%s), 来自 %s\n", x->userid, x->username, fromhost);
	fprintf(fp, "今天初来此地报到, 请大家多多指教.\n\n");
	fprintf(fp, "自我介绍:\n\n");
	fprintf(fp, "%s", words);
	fclose(fp);
	post_article("newcomers", "WWW新手上路", filename, x->userid, x->username, fromhost, -1, 0, 0, x->userid, -1);
	unlink(filename);
}

void
adduser(struct userec *x)
{
	int i;
	FILE *fp;
	fp = fopen(".PASSWDS", "r+");
	flock(fileno(fp), LOCK_EX);
	for (i = 0; i < MAXUSERS; i++) {
		if (shm_ucache->userid[i][0] == 0) {
			if (i + 1 > shm_ucache->number)
				shm_ucache->number = i + 1;
			strncpy(shm_ucache->userid[i], x->userid, 13);
			insertuseridhash(uidhashshm->uhi, UCACHE_HASH_SIZE, x->userid, i + 1);
			save_user_data(x);
			break;
		}
	}
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
}
