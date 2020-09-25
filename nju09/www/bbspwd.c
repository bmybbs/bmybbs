#include "bbslib.h"


// 生成一个长为len的随机字符串
char *random_str(char *buf, int len)
{
	srand((unsigned)time(NULL));
	int i;
	for (i = 0; i < len; i++)
		buf[i] = rand()%10 + '0';
	buf[len] = '\0';
	return buf;
}

int
bbspwd_main() {
	//modify by mintbaggio 20040829 for new www
	int type;
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("您尚未登录, 请先登录");
	changemode(GMENU);

	int mode = atoi(getparm("mode"));

	//修改本人密码
	if (mode == 1)
	{
		char pw1[20], pw2[20], pw3[20], salt[3];
		printf("<body>");
		type = atoi(getparm("type"));
		if (type == 0) {
			printf("<div class=rhead>%s -- 修改密码 [用户: <span class=h11>%s</span>]</div><hr>\n", BBSNAME, currentuser.userid);
			printf("<form action=bbspwd?mode=1&type=1 method=post>\n");
			printf("你的旧密码: <input maxlength=12 size=12 type=password name=pw1><br>\n");
			printf("你的新密码: <input maxlength=12 size=12 type=password name=pw2><br>\n");
			printf("再输入一次: <input maxlength=12 size=12 type=password name=pw3><br><br>\n");
			printf("<input type=submit value=确定修改>\n");
			printf("</body>");
			http_quit();
		}
		ytht_strsncpy(pw1, getparm("pw1"), 13);
		ytht_strsncpy(pw2, getparm("pw2"), 13);
		ytht_strsncpy(pw3, getparm("pw3"), 13);
		if (strcmp(pw2, pw3))
			http_fatal("两次密码不相同");
		if (strlen(pw2) < 2)
			http_fatal("新密码太短");
		if (!ytht_crypt_checkpasswd(currentuser.passwd, pw1))
			http_fatal("密码不正确");
		getsalt(salt);
		strcpy(currentuser.passwd, ytht_crypt_crypt1(pw2, salt));
		save_user_data(&currentuser);
		printf("[%s] 密码修改成功.", currentuser.userid);
	}

	return 0;
}
