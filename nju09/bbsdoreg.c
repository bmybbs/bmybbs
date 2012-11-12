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
// 登陆邮件服务器，进行身份验证， added by interma@BMY 2005.5.12
// 返回值为1表示有效，0表示无效, -1表示和pop服务器连接出错 
int test_mail_valid(char *user, char *pass, char *popip)
{
    char buffer[512]; 
    int sockfd;
    struct sockaddr_in server_addr; 
    struct hostent *host; 

	if (user[0] == ' ' || pass[0] == ' ')
		return 0;

    // 客户程序开始建立 sockfd描述符
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) 
    {  
        return -1;
    } 
    if (strcmp(user, "test")==0) {
	return -2;
    }
    int i;
    for ( i = 0; i < 8; i++)
    server_addr.sin_zero[i] = 0;
    server_addr.sin_family=AF_INET; 
    server_addr.sin_port=htons(110);
    // 202.117.1.22 == stu.xjtu.edu.cn 
    if(inet_aton(popip, &server_addr.sin_addr) == 0) 
    {  
        return -1;
    }

    // 客户程序发起连接请求
    if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1) 
    {  
        return -1; 
    } 

    if(read(sockfd,buffer,512) == -1 )
    {  
        return -1; 
    } 
    if (buffer[0] == '-')
        return -1;
    
    sprintf(buffer, "USER %s\r\n\0", user);
    if (write(sockfd, buffer, strlen(buffer)) == -1)
    { 
        return -1; 
    }   
     
    if(read(sockfd,buffer,512) == -1 )
    {  
        return -1; 
    } 
    if (buffer[0] == '-')
    {   
        return 0;
    }   
     
    sprintf(buffer, "PASS %s\r\n\0", pass);
    if (write(sockfd, buffer, strlen(buffer)) == -1)
    { 
        return -1; 
    }
     
    if(read(sockfd,buffer,512) == -1 )
    {  
        return -1; 
    } 
    if (buffer[0] == '-')
    {
        return 0;
    }
            
    write(sockfd, "QUIT\r\n", strlen("QUIT\r\n"));          
    return 1;
}    

//void securityreport(char *str, char *content);

/*
char *
sethomefile(char *buf, const char *userid, const char *filename)
{
	sprintf(buf, MY_BBS_HOME "/home/%c/%s/%s", mytoupper(userid[0]), userid,
		filename);
	return buf;
}
*/

int
substitute_record(filename, rptr, size, id)
char *filename;
void *rptr;
int size, id;
{
#ifdef LINUX
	struct flock ldata;
#endif
	int retv = 0;
	int fd;
	//add by hace
	struct stat st;
	if(stat(filename,&st)==-1)
	    return -1;
	else{
	    if(st.st_size/size <id)
		return -1;
	}
	//end 
	if ((fd = open(filename, O_WRONLY | O_CREAT, 0660)) == -1)
		return -1;
#ifdef LINUX
	ldata.l_type = F_WRLCK;
	ldata.l_whence = 0;
	ldata.l_len = size;
	ldata.l_start = size * (id - 1);
	if (fcntl(fd, F_SETLKW, &ldata) == -1) {
		errlog("reclock error %d", errno);
		return -1;
	}
#else
	flock(fd, LOCK_EX);
#endif
	if (lseek(fd, size * (id - 1), SEEK_SET) == -1) {
		errlog("subrec seek err %d", errno);
		retv = -1;
		goto FAIL;
	}
	if (safewrite(fd, rptr, size) != size) {
		errlog("subrec write err %d", errno);
		retv = -1;
		goto FAIL;
	}
      FAIL:
#ifdef LINUX
	ldata.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &ldata);
#else
	flock(fd, LOCK_UN);
#endif
	close(fd);
	return retv;
}

// 令username用户通过验证， added by interma@BMY 2005.5.12
void register_success(int usernum, char *userid, char *realname, char *dept, 
char *addr, char *phone, char *assoc, char *email)
{
	struct userec uinfo;
	FILE *fout, *fn;
	char buf[STRLEN];
	int n;
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

			strsncpy(uinfo.userid, userid,
				 sizeof (uinfo.userid));
			strsncpy(uinfo.realname, realname,
				 sizeof (uinfo.realname));
			strsncpy(uinfo.address, addr,
				 sizeof (uinfo.address));
			sprintf(genbuf, "%s$%s@%s", dept, phone, userid);
			strsncpy(uinfo.realmail, genbuf,
				 sizeof (uinfo.realmail));

			strsncpy(uinfo.email, email, sizeof (uinfo.email));

			uinfo.userlevel |= PERM_DEFAULT;	// by ylsdd
			substitute_record(PASSFILE, &uinfo, sizeof (struct userec), usernum);

			sethomefile(buf, uinfo.userid, "sucessreg");
			if ((fout = fopen(buf, "w")) != NULL) {
				fprintf(fout, "\n");
				fclose(fout);
			}

			sethomefile(buf, uinfo.userid, "register");
	
			if ((fout = fopen(buf, "w")) != NULL) 
			{
				
				fprintf(fout, "%s: %d\n", "usernum", usernum);
				fprintf(fout, "%s: %s\n", "userid", userid);
				fprintf(fout, "%s: %s\n", "realname", realname);
				fprintf(fout, "%s: %s\n", "dept", dept);
				fprintf(fout, "%s: %s\n", "addr", addr);
				fprintf(fout, "%s: %s\n", "phone", phone);
				fprintf(fout, "%s: %s\n", "assoc", assoc);

				n = time(NULL);
				fprintf(fout, "Date: %s",
					ctime((time_t *) & n));
				fprintf(fout, "Approved: %s\n", userid);
				fclose(fout);
			}

			mail_file("etc/s_fill", uinfo.userid,
				  "恭禧您通过身份验证", "SYSOP"); 

			mail_file("etc/s_fill2", uinfo.userid,
				  "欢迎加入" MY_BBS_NAME "大家庭", "STSOP");
			sethomefile(buf, uinfo.userid, "mailcheck");
			unlink(buf);
			sprintf(genbuf, "让 %s 通过身分确认.", uinfo.userid);
			securityreport(genbuf, genbuf);
	return ;
}


char * str_to_upper(char *str)
{
	char *h = str;
	while (*str != '\n' && *str != 0)
	{
		*str = toupper(*str);
		str++;
	}
	return h;
}

extern char fromhost[256];
// 纪录pop服务器上的用户名，防止重复注册多个id， added by interma@BMY 2005.5.16
// 返回值为0表示已纪录，1表示已存在 
// 纪录pop服务器上的用户名，防止重复注册多个id， added by interma@BMY 2005.5.16
/* 返回值为0表示已纪录（未存在），1表示已存在 */
int write_pop_user(char *user, char *userid, char *pop_name)
{	
	FILE *fp;
	char buf[256];
	char path[256];
	int isprivilege = 0; 

	char username[USER_LEN + 2];
	sprintf(username, "%s\n", user);

	// 首先进行特权用户（privilege）检验
	sprintf(path, MY_BBS_HOME "/etc/pop_register/%s_privilege" , pop_name);

	fp = fopen(path, "r");
	if (fp != NULL)
	{
		while(fgets(buf, 256, fp) != NULL)
		{
			if (strcmp(str_to_upper(username), str_to_upper(buf)) == 0)
			{
				isprivilege = 1;
				break;
			}
		}
			
		fclose(fp);
	}


	// 以下进行普通用户检验
	sprintf(path, MY_BBS_HOME "/etc/pop_register/%s", pop_name);

	int lockfd = openlockfile(".lock_new_register", O_RDONLY, LOCK_EX); // 加锁来保证互斥操作
	
	fp = fopen(path, "a+");

	if (fp == NULL) 
	{	
		close(lockfd);
		return 0;
	}

	if (isprivilege == 0)
	{
		fseek(fp, 0, SEEK_SET);
		while(fgets(buf, 256, fp) != NULL)
		{
			if (strcmp(str_to_upper(username), str_to_upper(buf)) == 0)
			{
				fclose(fp);
				close(lockfd);
				return 1;
			}
			fgets(buf, 256, fp);
		}
	}

	fseek(fp, 0, SEEK_END);
	fputs(user, fp);

	time_t t;
	time(&t);

	sprintf(buf, "\n%s : %s : %s", userid, fromhost, ctime(&t));
	fputs(buf, fp);

	fclose(fp);
	close(lockfd);
	return 0;
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

int id_with_num(userid)
char    userid[IDLEN + 1];
{
   char   *s;
   for (s = userid; *s != '\0'; s++)
      if (*s < 1 || !isalpha(*s)) return 1;
   return 0;            
} 

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
    char pass[PASS_LEN + 1];
	char popserver[512];
	strsncpy(popserver, getparm("popserver"), 512);	
	strsncpy(user, getparm("user"), USER_LEN);
	strsncpy(pass, getparm("pass"), PASS_LEN);
#endif

	strsncpy(x.userid, getparm("userid"), 13);
	strsncpy(pass1, getparm("pass1"), 13);
	strsncpy(pass2, getparm("pass2"), 13);
	strsncpy(x.username, getparm("username"), 32);
	strsncpy(x.realname, getparm("realname"), 32);
	strsncpy(dept, getparm("dept"), 60);
	strsncpy(x.address, getparm("address"), 60);

#ifndef POP_CHECK
	strsncpy(x.email, getparm("email"), 60);
#else
	char delims[] = "+";
    	char *popname;
	char *popip;
	//char popname[256];
	//char popip[256];

	popname = strtok(popserver, delims);
	popip = strtok(NULL, delims);

	// 防止注入漏洞
	struct stat temp;
	/*
	if (stat(MY_BBS_HOME "/etc/pop_register/pop_list", &temp) == -1)
	{
		http_fatal("目前没有可以信任的邮件服务器列表, 因此无法验证用户\n");
	}
	
	fp = fopen(MY_BBS_HOME "/etc/pop_register/pop_list", "r");
	if (fp == NULL)
	{
		http_fatal("打开可以信任的邮件服务器列表出错, 因此无法验证用户\n");
	}
	*/

	//if (!seek_in_file(MY_BBS_HOME "/etc/pop_register/pop_list", popname)) {
	//	http_fatal("不是可信任的邮件服务器列表!");
	//}
	int vaild = 0;
	char bufpop[256];
	int numpop = 0;
	char namepop[10][256]; // 注意：最多信任10个pop服务器，要不就溢出了！
	char ippop[10][256];
	/*
	while(fgets(bufpop, 256, fp) != NULL)
	{
		if (strcmp(bufpop, "") == 0 || strcmp(bufpop, " ") == 0 || strcmp(bufpop, "\n") == 0)
			break;
		strcpy(namepop[numpop], bufpop);
		fgets(bufpop, 256, fp);
		strcpy(ippop[numpop], bufpop);
		
		namepop[numpop][strlen(namepop[numpop]) - 1] = 0;
		ippop[numpop][strlen(ippop[numpop]) - 1] = 0;

		if (strcmp(namepop[numpop], popname) == 0 &&
			strcmp(ippop[numpop], popip) == 0 )
		{
			vaild = 1;
			break;
		}
		
		numpop ++;
	}
	fclose(fp);	
	*/
	
	//if (!vaild)
	//	http_fatal("-_-bb \n");
	//
	

	char email[60];
	sprintf(email, "%s@%s", user, popname);  // 注意不要将email弄溢出了
	str_to_lowercase(email);
	strsncpy(x.email, email, 60);
#endif	
	
	strsncpy(phone, getparm("phone"), 60);
	strsncpy(assoc, getparm("assoc"), 60);
	strsncpy(words, getparm("words"), 1000);


//      x.gender='M';
//      if(atoi(getparm("gender"))) x.gender='F';
//      x.birthyear=atoi(getparm("year"))-1900;
//      x.birthmonth=atoi(getparm("month"));
//      x.birthday=atoi(getparm("day"));

	//if (!goodgbid(x.userid))  by bjgyt
        if (id_with_num(x.userid))
		http_fatal("帐号只能由英文字母组成");
	if (strlen(x.userid) < 2)
		http_fatal("帐号长度太短(2-12字符)");
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
//      if(badymd(x.birthyear, x.birthmonth, x.birthday)) http_fatal("请输入您的出生年月");
	if (is_bad_id(x.userid))
		http_fatal("不雅帐号或禁止注册的id, 请重新选择");
	if (getuser(x.userid))
		http_fatal("此帐号已经有人使用,请重新选择。");
//      sprintf(salt, "%c%c", 65+rand()*26, 65+rand()*26);
//add by lepton


#ifdef POP_CHECK
	if (strlen(user) == 0)
		http_fatal("邮箱用户名没添啊");
	if (strlen(pass) == 0)
		http_fatal("邮箱密码没添啊");
#endif


	getsalt(salt);
	strsncpy(x.passwd, crypt1(pass1, salt), 14);
	strncpy(x.lasthost, fromhost,15);	//ipv6 by leoncom 
						//不能赋值太多，就影响后面的数据
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
			Ctime(now_t));
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
	printf
	    ("如果您有任何疑问，可以去sysop(站长的工作室)版发文求助。\n\n</pre></table>");
	printf("<hr><br>您的基本资料如下:<br>\n");
	printf("<table border=1 width=400>");
	printf("<tr><td>帐号位置: <td>%d\n", getusernum(x.userid));
	printf("<tr><td>使用者代号: <td>%s (%s)\n", x.userid, x.username);
	printf("<tr><td>姓  名: <td>%s<br>\n", x.realname);
	printf("<tr><td>昵  称: <td>%s<br>\n", x.username);
	printf("<tr><td>上站位置: <td>%s<br>\n", x.lasthost);
	printf("<tr><td>电子邮件: <td>%s<br></table><br>\n", x.email);

	printf
	    ("<center><form><input type=button onclick='window.close()' value=关闭本窗口></form></center>\n");
#else
	printf("<center><table><td><td><pre>\n");
	memset(&act_data, 0, sizeof(act_data));
	strcpy(act_data.name, x.realname);
	strcpy(act_data.userid, x.userid);
	strcpy(act_data.dept, dept);
	strcpy(act_data.phone, phone);
	strcpy(act_data.email, email);
	strcpy(act_data.ip, fromhost);
	strcpy(act_data.operator, x.userid);
	
	act_data.status=0;
	write_active(&act_data);


	int result;
	//int result = test_mail_valid(user, pass, popip);
	if (strstr(popname, "idp.xjtu6.edu.cn")) {
		if (!strcmp(fromhost, "202.117.1.190") || !strcmp(fromhost, "2001:250:1001:2::ca75:1be"))
			result=1;
		else {
			http_fatal("非可信的认证域!");
			result=0;
		}
	}
	else {
		result = test_mail_valid(user, pass, popip);
	}

	switch (result)
    {
		case -2:
		printf("<tr><td>%s<br></table><br>\n", 
			"欢迎您加入交大，来到兵马俑BBS。<br>您采用了新生测试信箱注册，目前您是新生用户身份。"
			"目前您没有发文、信件、消息等权限。<br><br>"
			"请在开学取得stu.xjtu.edu.cn信箱后，<br>点击左侧边栏“填写注册单”，完成信箱绑定认证操作，成为本站正式用户。");	
		  break;
		  case -1:
		  case 0:
		  printf("<tr><td>%s<br></table><br>\n", 
			  "邮件服务器身份审核失败，您将只能使用本bbs的最基本功能，十分抱歉。");
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
			register_success(getusernum(x.userid) + 1, x.userid, x.realname, dept, x.address, phone, assoc, email);
			break;
		}
    		printf("  验证失败!");
			break;

     
    }

	printf
	    ("<center><form><input type=button onclick='window.close()' value=关闭本窗口></form></center>\n");
#endif
	
	// 以下这行（newcomer）可能将会引起www（ia64）下的问题。interma@BMY
	newcomer(&x, words); 


//      sprintf(buf, "%s %-12s %d\n", Ctime(now_t)+4, x.userid, getusernum(x.userid));
//      f_append("wwwreg.log", buf);
	sprintf(buf, "%s newaccount %d %s www", x.userid, getusernum(x.userid),
		fromhost);
	newtrace(buf);
	//wwwstylenum = 1;

	//don't login with reg by leoncom for ipv6
	//ub = wwwlogin(&x,0);
	//sprintf(buf, "%s enter %s www", x.userid, fromhost);
	//newtrace(buf);
	//printf("<script>opener.top.location.href=\"%s\";</script>", ub);
	return 0;
}

int
badstr(unsigned char *s)
{
	int i;
	for (i = 0; s[i]; i++)
		if (s[i] != 9 && (s[i] < 32 || s[i] == 255))
			return 1;
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
	post_article("newcomers", "WWW新手上路", filename, x->userid,
		     x->username, fromhost, -1, 0, 0, x->userid, -1);
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
			insertuseridhash(uidhashshm->uhi, UCACHE_HASH_SIZE,
					 x->userid, i + 1);
			save_user_data(x);
			break;
		}
	}
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	//utime(FLUSH, NULL);
}
