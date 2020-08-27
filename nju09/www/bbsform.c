#include "bbslib.h"
#include "identify.h"

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
int test_mail_valid(char *user, char *pass, char *popip);
//void securityreport(char *str, char *content);
int substitute_record(char *filename, void *rptr, int size, int id);
void register_success(int usernum, char *userid, char *realname, char *dept,
char *addr, char *phone, char *assoc, char *email);
int write_pop_user(char *user, char *userid, char *pop_name);
#endif

int
bbsform_main()
{
	int type;
	html_header(1);

	#ifdef POP_CHECK
	struct stat temp;
	#endif

	type = atoi(getparm("type"));
	if (!loginok || isguest)
		http_fatal("您尚未登录, 请重新登录。");
	changemode(NEW);
	printf("%s -- 填写注册单<hr>\n", BBSNAME);
	check_if_ok();
	if (type == 1) {
		check_submit_form();
		http_quit();
	}
	printf("<style>.hidden { display: none }</style>");
	printf("您好, %s, 注册单通过后即可获得注册用户的权限, 下面各项务必请认真填写<br><hr>\n", currentuser.userid);
	printf("<div id='form-reg' class='hidden'>");
	printf("<form method=post action=bbsform?type=1>\n");
	printf("真实姓名: <input name=realname type=text maxlength=8 size=8 value='%s'><br>\n", nohtml(currentuser.realname));
	printf("居住地址: <input name=address type=text maxlength=32 size=32 value='%s'><br>\n", nohtml(currentuser.address));
	printf("联络电话(可选): <input name=phone type=text maxlength=32 size=32><br>\n");
	printf("学校系级/公司单位: <input name=dept maxlength=60 size=40><br>\n");
	printf("校友会/毕业学校(可选): <input name=assoc maxlength=60 size=42><br><hr><br>\n");

#ifdef POP_CHECK
	printf("以下信息要作为邮件服务器身份验证之用，必须填写<br><hr>\n");
	printf("每个信箱最多可以认证 %d 个bbs帐号 <br><hr>\n", MAX_USER_PER_RECORD);
	printf("<tr><td align=right>*可以信任的邮件服务器列表:<td align=left><SELECT NAME=popserver>\n");
	int n = 1;
	while(n <= DOMAIN_COUNT)
	{
		if (n == 1)
			printf("<OPTION VALUE=%d SELECTED>", n);
		else
			printf("<OPTION VALUE=%d>", n);

		printf("%s", MAIL_DOMAINS[n]);
		n++;
	}
	printf("</select><br>\n");
	printf("<tr><td align=right>*请输入邮箱用户名:<td align=left><input name=user size=20 maxlength=20><br>\n");

#endif

	printf("<input type=submit value=注册> <input type=reset value=取消>");
	printf("</form></div>"); // div#form-reg

	printf("<div id='form-cap' class='hidden'><form method='post' action='bbsform?type=2'");
	printf("您关联的信箱是： %s，验证码已发送到您的信箱。<br>", nohtml(currentuser.email));
	printf("验证码<input name='captcha' maxlength=5><br>");
	printf("<button id='btn-cap-submit'>提交</button> <button id='btn-cap-resent'>重新发送</button> <button id='btn-cap-update'>更新信箱</button>");
	printf("</form></div>");
	printf("<script>var cap_status=%d;</script>", check_captcha_status(currentuser.userid)); // CAPTCHA_OK == 0
	printf("<script src='/bbsform.js'></script>");
	http_quit();
	return 0;
}

void
check_if_ok()
{
	if (user_perm(&currentuser, PERM_LOGINOK))
		http_fatal("您已经通过本站的身份认证, 无需再次填写注册单.");

	if (has_fill_form())
		http_fatal("目前站长尚未处理您的注册单，请耐心等待.");
}

void
check_submit_form()
{

	FILE *fp;
	char dept[80], phone[80], assoc[80];
	struct active_data act_data;

#ifdef POP_CHECK
	char user[USER_LEN + 1];
	char pass[PASS_LEN + 1];
	char popserver[512];
	strsncpy(popserver, getparm("popserver"), 512);
	strsncpy(user, getparm("user"), USER_LEN);
	strsncpy(pass, getparm("pass"), PASS_LEN);

	if (strlen(user) == 0)
		http_fatal("邮箱用户名没添啊");
	if (strlen(pass) == 0)
		http_fatal("邮箱密码没添啊");

	char delims[] = "+";
	char *popname;
	char *popip;
	//char popname[256];
	//char popip[256];

	popname = strtok(popserver, delims);
	popip = strtok(NULL, delims);

	//strncpy(popname, );
	//strncpy();

	char email[60];
	sprintf(email, "%s@%s", user, popname);  // 注意不要将email弄溢出了
	strsncpy(currentuser.email, email, 60);
	str_to_lowercase(email);
#endif


	strsncpy(currentuser.realname, getparm("realname"), 20);
	strsncpy(dept, getparm("dept"), 60);
	strsncpy(currentuser.address, getparm("address"), 60);
	strsncpy(phone, getparm("phone"), 60);
	strsncpy(assoc, getparm("assoc"), 60);
	memset(&act_data, 0, sizeof(act_data));
	strcpy(act_data.name, currentuser.realmail);
	strcpy(act_data.userid, currentuser.userid);
	strcpy(act_data.dept, dept);
	strcpy(act_data.phone, phone);
	strcpy(act_data.email, email);
	strcpy(act_data.ip, currentuser.lasthost);
	strcpy(act_data.operator, currentuser.userid);
	act_data.status=0;
	write_active(&act_data);

#ifndef POP_CHECK
	fp = fopen("new_register", "a");
	if (fp == 0)
		http_fatal("注册文件错误，请通知SYSOP");
	fprintf(fp, "usernum: %d, %s\n", getusernum(currentuser.userid) + 1,
		Ctime(now_t));
	fprintf(fp, "userid: %s\n", currentuser.userid);
	fprintf(fp, "realname: %s\n", currentuser.realname);
	fprintf(fp, "dept: %s\n", dept);
	fprintf(fp, "addr: %s\n", currentuser.address);
	fprintf(fp, "phone: %s\n", phone);
	fprintf(fp, "assoc: %s\n", assoc);
	fprintf(fp, "----\n");
	fclose(fp);
	printf("您的注册单已成功提交. 站长检验过后会给您发信, 请留意您的信箱.<br>" "<a href=bbsboa>浏览" MY_BBS_NAME "</a>");
#else
	int result = test_mail_valid(user, pass, popip);

	switch (result)
    {
	case -1:
	case 0:
		printf("邮件服务器身份审核失败，您将只能使用本bbs的最基本功能，十分抱歉。<br>");
		break;

	case 1:
		if (query_record_num(email, MAIL_ACTIVE)>=MAX_USER_PER_RECORD ) {
			printf("您的信箱已经验证过 %d 个id，无法再用于验证了!\n", MAX_USER_PER_RECORD);
			break;
		}
		int response;
		strcpy(act_data.email, email);
		act_data.status=1;
		response=write_active(&act_data);
		if (response==WRITE_SUCCESS || response==UPDATE_SUCCESS)  {
			printf("身份审核成功，您已经可以使用所用功能了！\n");
			register_success(getusernum(currentuser.userid) + 1, currentuser.userid, currentuser.realname, dept, currentuser.address, phone, assoc, email);
			break;
		}
		printf("  验证失败!");
		break;
	}

#endif

}

