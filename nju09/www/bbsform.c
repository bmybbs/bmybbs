#include "bbslib.h"
#include "ythtbbs/identify.h"

static void check_captcha_form(void);
static void resent_active_mail(void);

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
void register_success(int usernum, char *userid, char *realname, char *dept,
char *addr, char *phone, char *assoc, char *email);
#endif

int
bbsform_main()
{
	int type;

	type = atoi(getparm("type"));

	if (!loginok || isguest) {
		http_fatal("您尚未登录, 请重新登录。");
	}

	check_if_ok();

	if (type == 1) {
		// update register info
		check_submit_form();
		http_quit();
	} else if (type == 2) {
		// check captcha
		check_captcha_form();
		http_quit();
	} else if (type == 3) {
		// resent captcha
		resent_active_mail();
		http_quit();
	}


	html_header(1);
	changemode(NEW);
	printf("%s -- 填写注册单<hr>\n", BBSNAME);
	printf("<link href='/bbsform.css' rel='stylesheet'>");
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
	printf("<tr><td align=right>*请输入邮箱用户名:<td align=left><input name=user size=20 maxlength=20 placeholder='只需要输入@之前的部分'><br>\n");

#endif

	printf("</form>");
	printf("<button id='btn-reg-submit'>注册</button> <button id='btn-reg-reset'>取消</button> <button id='btn-reg-captcha'>输入验证码</button>");
	printf("</div>"); // div#form-reg

	printf("<div id='form-cap' class='hidden'>");
	printf("您关联的信箱是： <span id='cap-email'>%s</span>，验证码已发送到您的信箱。<br>", nohtml(currentuser.email));
	printf("验证码<input name='captcha' maxlength=5><br>");
	printf("<button id='btn-cap-submit'>提交</button> <button id='btn-cap-resent'>重新发送</button> <button id='btn-cap-update'>更新信箱</button>");
	printf("</div>"); //div#form-cap

	printf("<div id='myModal' class='modal'>");
	printf("<div class='modal-content'>");
	printf("<span class='close'>&times;</span>");
	printf("<p></p>");
	printf("</div></div>");
	printf("<script>var cap_status=%d;</script>", check_captcha_status(currentuser.userid, CAPTCHA_FILE_REGISTER)); // CAPTCHA_OK == 0
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
	int count, mail_diff;
	int isprivilege = 0;
	char path[128];

#ifdef POP_CHECK
	char user[USER_LEN + 1];
	char pass[PASS_LEN + 1];
	char popserver[4];
	int popserver_index;
	ytht_strsncpy(popserver, getparm("popserver"), 4);
	ytht_strsncpy(user, getparm("user"), USER_LEN);
	ytht_strsncpy(pass, getparm("pass"), PASS_LEN);

	if (strlen(user) == 0)
		http_fatal("邮箱用户名没添啊");

	const char *popname;
	popserver_index = atoi(popserver);
	if(popserver_index < 1 || popserver_index > 3) popserver_index = 1;
	popname = MAIL_DOMAINS[popserver_index];

	char email[60];
	sprintf(email, "%s@%s", user, popname);  // 注意不要将email弄溢出了
	str_to_lowercase(email);
	count = read_active(currentuser.userid, &act_data);
	if (count == 0) {
		mail_diff = 1;
	} else {
		if (strcasecmp(act_data.email, email) != 0) {
			mail_diff = 1;
		}
	}
	ytht_strsncpy(currentuser.email, email, 60);

	if (check_mail_to_address(email) == MAIL_SENDER_WRONG_EMAIL)
		http_fatal("您的邮箱名不合法，请联系站长或至 https://github.com/bmybbs/bmybbs/issues/ 反馈问题。");
#endif

	ytht_strsncpy(currentuser.realname, getparm("realname"), 20);
	ytht_strsncpy(dept, getparm("dept"), 60);
	ytht_strsncpy(currentuser.address, getparm("address"), 60);
	ytht_strsncpy(phone, getparm("phone"), 60);
	ytht_strsncpy(assoc, getparm("assoc"), 60);
	memset(&act_data, 0, sizeof(act_data));
	snprintf(act_data.name, NAMELEN, "%s", currentuser.realname);
	act_data.name[NAMELEN-1] = '\0';
	strcpy(act_data.userid, currentuser.userid);
	snprintf(act_data.dept, STRLEN, "%s", dept);
	act_data.dept[STRLEN-1] = '\0';
	snprintf(act_data.phone, VALUELEN, "%s", phone);
	act_data.phone[VALUELEN-1] = '\0';
	snprintf(act_data.email, VALUELEN, "%s", email);
	act_data.email[VALUELEN-1] = '\0';
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
	int result;

	snprintf(path, 127, MY_BBS_HOME "/etc/pop_register/%s", popname);
	if (seek_in_file(path, user)) {
		isprivilege = 1;
	}

	if (strcasecmp(user, "test") == 0) {
		result = -2;
	} else if (isprivilege == 0 && query_record_num(email, MAIL_ACTIVE) >= MAX_USER_PER_RECORD) {
		result = -3;
	} else {
		if (mail_diff) {
			unlink_captcha(currentuser.userid, CAPTCHA_FILE_REGISTER);
		}
		result = send_active_mail(currentuser.userid, email);
	}

	html_header(1);

	switch (result)
	{
	default:
	case -2:
	case -1:
	case 0:
		printf("邮件服务器身份审核失败，您将只能使用本bbs的最基本功能，十分抱歉。<br>");
		break;

	case -3:
		printf("您的信箱已经验证过 %d 个id，无法再用于验证了!\n", MAX_USER_PER_RECORD);
		break;
	case 1:
		printf("验证信息已发送至您的邮箱 %s ，及时请查收。<br>"
			"请登录系统后点击左侧边栏“填写注册单”，完成信箱绑定认证操作，成为本站正式用户。", email);
		break;
	}

#endif

}

static void
check_captcha_form(void)
{
	char code[6];
	char tmp_email[STRLEN+1], *domain, path[128];
	int rc, isprivilege;
	struct active_data act_data;

	memset(&act_data, 0, sizeof(struct active_data));
	snprintf(code, 6, "%s", getparm("code"));
	rc = verify_captcha_for_user(currentuser.userid, code, CAPTCHA_FILE_REGISTER);
	if (rc == CAPTCHA_OK) {
		read_active(currentuser.userid, &act_data);

		snprintf(tmp_email, STRLEN, "%s", act_data.email);
		domain = strchr(tmp_email, '@');
		if (domain != NULL) {
			*domain = '\0';
			domain++;
			snprintf(path, 127, MY_BBS_HOME "/etc/pop_register/%s", domain);

			if (seek_in_file(path, tmp_email)) {
				isprivilege = 1;
			}
		}
		if ((isprivilege == 1) || (query_record_num(act_data.email, MAIL_ACTIVE) < MAX_USER_PER_RECORD)) {
			act_data.status = 1;
			rc = write_active(&act_data);
			if (rc == WRITE_SUCCESS || rc == UPDATE_SUCCESS) {
				register_success(getusernum(currentuser.userid) + 1, currentuser.userid, currentuser.realname,
						act_data.dept, currentuser.address, act_data.phone, "", act_data.email);
				rc = 0;
			} else {
				// WRITE_FAIL == 0
				rc = -1;
			}
		} else {
			rc = -1;
		}
	}

	json_header();
	printf("{ \"status\": %d }", rc);
}

static void
resent_active_mail(void)
{
	int rc;
	char c;

	if (strlen(currentuser.email) < 5) {
		rc = -2;
	} else {
		c = currentuser.email[4];
		currentuser.email[4] = '\0';
		rc = strcasecmp(currentuser.email, "test");
		currentuser.email[4] = c;
		if (rc == 0) {
			rc = -2;
		} else if (query_record_num(currentuser.email, MAIL_ACTIVE) >= MAX_USER_PER_RECORD) {
			rc = -3;
		} else {
			rc = send_active_mail(currentuser.userid, currentuser.email);
		}
	}

	json_header();
	printf("{ \"status\": %d }", rc);
}

