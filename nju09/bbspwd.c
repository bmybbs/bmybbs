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

// 检查邮件格式的有效性
// 返回1为有效
int vaild_mail(const char *email)
{
	if (!strchr(email, '@'))
		return 0;
	if (strstr(email, ".bbs"))
		return 0;
	return 1;
}

int
bbspwd_main()
{	//modify by mintbaggio 20040829 for new www
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
			printf("<div class=rhead>%s -- 修改密码 [用户: <span class=h11>%s</span>]</div><hr>\n",
				   BBSNAME, currentuser.userid);
			printf("<form action=bbspwd?mode=1&type=1 method=post>\n");
			printf
				("你的旧密码: <input maxlength=12 size=12 type=password name=pw1><br>\n");
			printf
				("你的新密码: <input maxlength=12 size=12 type=password name=pw2><br>\n");
			printf
				("再输入一次: <input maxlength=12 size=12 type=password name=pw3><br><br>\n");
			printf("<input type=submit value=确定修改>\n");
			printf("</body>");
			http_quit();
		}
		strsncpy(pw1, getparm("pw1"), 13);
		strsncpy(pw2, getparm("pw2"), 13);
		strsncpy(pw3, getparm("pw3"), 13);
		if (strcmp(pw2, pw3))
			http_fatal("两次密码不相同");
		if (strlen(pw2) < 2)
			http_fatal("新密码太短");
		if (!checkpasswd(currentuser.passwd, pw1))
			http_fatal("密码不正确");
		getsalt(salt);
		strcpy(currentuser.passwd, crypt1(pw2, salt));
		save_user_data(&currentuser);
		printf("[%s] 密码修改成功.", currentuser.userid);
	}
	//找回他人密码
	else if (mode == 2)
	{
		struct userec *uptr;
		char id[32];
		char code[11];
		char buf[512];
		printf("<body>");
		type = atoi(getparm("type"));
		if (type == 0) {
			printf("<div class=rhead>%s -- 替他人找回密码</div><hr>\n", BBSNAME);
			
			printf("<form action=bbspwd?mode=2&type=1 method=post>\n");
			printf("[1] 第一步，输入欲找回密码的ID: <input maxlength=12 size=12 name=id><br>\n");
			printf("<input type=submit value=发送验证码到此人邮箱>\n");
			printf("</form>");

			printf("<form action=bbspwd?mode=2&type=2 method=post>\n");
			printf("[2] 第二步，输入验证码: <input maxlength=12 size=12 name=code><br>\n");
			printf("<input type=submit value=重置密码>\n");
			printf("</form>");

			printf("</body>");
			http_quit();
		}
		else if (type == 1) 
		{
			strsncpy(id, getparm("id"), 32);
			if (!(uptr=getuser(id))) 
				http_fatal("错误的使用者代号");
			if (!vaild_mail(uptr->email))
				http_fatal("邮箱格式无效，请联系站长手动更改密码");
			
			random_str(code, 10);

			sprintf(buf, MY_BBS_HOME "/etc/findpasswd");
			int lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX);
			FILE *fp = fopen(buf, "a");
			if (fp == NULL) 
			{
				close(lockfd);
				http_fatal("无法打开找回密码文件，请联系站长");
			}
			sprintf(buf, "%s %s\n", code, id);
			fputs(buf, fp);
			fclose(fp);
			close(lockfd);

			sprintf(buf, MY_BBS_HOME "/bin/sendmail.py '%s' 'Authentication Code, From %s@bmy' '%s'", uptr->email, currentuser.userid, code);
			int ret = system(buf);
			if (ret != 0)
				http_fatal("发送邮件失败，请重试或联系站长");

			char titbuf[64];
			sprintf(titbuf, "找回密码第一步: %s", id);
			sprintf(buf, "UserId: %s\n操作者Id: %s\nE-Mail: %s\n", id, currentuser.userid, uptr->email);
			securityreport(titbuf, buf);

			printf("验证码已经发送到 [%s] 中，请收信，并在第二步中填写验证码。\n", uptr->email);
			//printf("%s\n", random_str(code, 10));
		}
		else if (type == 2)
		{
			strsncpy(code, getparm("code"), 11);

			int lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX);
			sprintf(buf, MY_BBS_HOME "/etc/findpasswd");
			FILE *fp1 = fopen(buf, "r");
			sprintf(buf, MY_BBS_HOME "/etc/findpasswd_tmp");
			FILE *fp2 = fopen(buf, "w");
			if (fp1 == NULL || fp2 == NULL) 
			{
				close(lockfd);
				http_fatal("无法打开找回密码文件，请联系站长");
			}

			int isfind = 0;
			while(fgets(buf, 512, fp1) != NULL)
			{
				char idtmp[32];
				char codetmp[11];
				strncpy(codetmp, buf, 11);
				codetmp[10] = '\0';
				strncpy(idtmp, buf+11, 32);
				idtmp[strlen(idtmp) - 1] = '\0';

				if (strcmp(code, codetmp) == 0)
				{
					isfind = 1;
					strncpy(id, idtmp, 32);
				}
				else
				{
					fputs(buf, fp2);
				}
			}

			fclose(fp1);
			fclose(fp2);
			rename(MY_BBS_HOME "/etc/findpasswd_tmp", MY_BBS_HOME "/etc/findpasswd");
			close(lockfd);

			char titbuf[64];
			if (isfind)
			{
				if (!(uptr=getuser(id))) 
					http_fatal("错误的使用者代号");
				
				char newpw[20];

				random_str(newpw, 4);
				//getsalt(salt);
				strcpy(uptr->passwd, genpasswd(newpw));
				save_user_data(uptr);
				
				sprintf(titbuf, "找回密码第二步[成功]: %s", id);
				sprintf(buf, "UserId: %s\n操作者Id: %s\n", id, currentuser.userid);
				securityreport(titbuf, buf);

				printf("[%s] 密码修改成功。\n新密码为: %s，请立即登录修改密码！", uptr->userid, newpw);
			}
			else
			{
				sprintf(titbuf, "找回密码第二步[失败]");
				sprintf(buf, "操作者Id: %s\n", currentuser.userid);
				securityreport(titbuf, buf);
				http_fatal("验证码无效");
			}
		}
	}

	return 0;
}
