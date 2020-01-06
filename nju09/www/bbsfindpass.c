#include "bbslib.h"
#include "identify.h"

int
bbsfindpass_main()
{
	html_header(1);

	printf("<body>");
	printf("<nobr><center>%s -- 找回帐号或密码<hr>\n", BBSNAME);
	printf
	    ("<font color=green>采用交大信箱实名认证的用户，可在此重置密码。重置密码需验证信箱.</font>");
	printf("<form method=post action=bbsresetpass>\n");
	printf("<table width=100%%>\n");
	printf
	    ("<tr><td align=right>*请输入ID  :<td align=left><input name=userid size=12 maxlength=12> (2-12字符, 必须全为英文字母)\n");
	printf
	    ("<tr><td align=right>*请输入新密码:<td align=left><input type=password name=pass1 size=12 maxlength=12> (4-12字符)\n");
	printf
	    ("<tr><td align=right>*请确认新密码:<td align=left><input type=password name=pass2 size=12 maxlength=12>\n");

#ifdef POP_CHECK
	char bufpop[256];
	int numpop = 0;
	char namepop[10][256]; // 注意：最多信任10个pop服务器，要不就溢出了！
	char ippop[10][256];
	printf("<tr><td align=right>*可以信任的邮件服务器列表:<td align=left><SELECT NAME=popserver>\n");
	int n = 1;
	while(n <= DOMAIN_COUNT)
	{
//		namepop[n - 1][strlen(namepop[n - 1]) - 1] = 0;
//		ippop[n - 1][strlen(ippop[n - 1]) - 1] = 0;
		
		char tempbuf[512];
		strncpy(tempbuf, MAIL_DOMAINS[n], 256);
		strcat(tempbuf, "+");
		strcat(tempbuf, IP_POP[n]);
		if (n == 1)
			printf("<OPTION VALUE=%s SELECTED>", tempbuf);
		else
			printf("<OPTION VALUE=%s>", tempbuf);

		printf("%s", MAIL_DOMAINS[n]);
		n++;

	}
	printf("</select>\n");
	printf
	    ("<tr><td align=right>*请输入邮箱用户名:<td align=left><input name=user size=20 maxlength=20> \n");
	printf
	    ("<tr><td align=right>*请输入邮箱密码:<td align=left><input type=password name=pass size=20 maxlength=20> \n");
	
#endif	

	printf
	    ("<input type=submit value=确认重置密码>\n");
	printf("</form></center>");
	printf("</table><br><hr>\n");
	


	printf
	    ("<font color=green>采用交大信箱实名认证的用户，若您已忘记id，可在此查询您的信箱下认证的id.</font>");
	printf("<form method=post action=bbsfindacc>\n");
	printf("<table width=100%%>\n");
	
#ifdef POP_CHECK
//	char bufpop[256];
//	int numpop = 0;
//	char namepop[10][256]; // 注意：最多信任10个pop服务器，要不就溢出了！
//	char ippop[10][256];

	printf("<tr><td align=right>*可以信任的邮件服务器列表:<td align=left><SELECT NAME=popserver1>\n");
	n = 1;
	while(n <= DOMAIN_COUNT)
	{
//		namepop[n - 1][strlen(namepop[n - 1]) - 1] = 0;
//		ippop[n - 1][strlen(ippop[n - 1]) - 1] = 0;
		
		char tempbuf[512];
		strncpy(tempbuf, MAIL_DOMAINS[n], 256);
		strcat(tempbuf, "+");
		strcat(tempbuf, IP_POP[n]);
		if (n == 1)
			printf("<OPTION VALUE=%s SELECTED>", tempbuf);
		else
			printf("<OPTION VALUE=%s>", tempbuf);

		printf("%s", MAIL_DOMAINS[n]);
		n++;

	}
	printf("</select>\n");
	printf
	    ("<tr><td align=right>*请输入邮箱用户名:<td align=left><input name=user1 size=20 maxlength=20> \n");
	printf
	    ("<tr><td align=right>*请输入邮箱密码:<td align=left><input type=password name=pass1 size=20 maxlength=20> \n");

	
#endif	

	printf
	    ("<input type=submit value=确认查询>\n");
	printf("</form></center>");
	printf("</table><br><hr>\n");

	
	http_quit();
	return 0;
}

