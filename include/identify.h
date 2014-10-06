#ifndef _LIBIDEN__H
#define _LIBIDEN__H

#include <mysql/mysql.h>

//关于链接数据库的一些常量
#define SQLDB "bbsreg"
#define SQLUSER "root"
#define SQLPASSWD "xj,bbs,1"
#define USERREG_TABLE "userreglog"
// #define SCHOOLDATA_TABLE "schooldata" 

//一些返回值
#define TOO_MUCH_RECORDS 3
#define WRITE_SUCCESS    2
#define UPDATE_SUCCESS   1
#define WRITE_FAIL       0
#define FILE_NOT_FOUND  -1

//一个记录最多绑多少id
#define MAX_USER_PER_RECORD 4

//允许的邮箱域
#define DOMAIN_COUNT 3
static const char *MAIL_DOMAINS[] = {"", "stu.xjtu.edu.cn", "mail.xjtu.edu.cn", "idp.xjtu6.edu.cn", NULL};
static const char* IP_POP[]={"", "202.117.1.22", "202.117.1.28", "2001:250:1001:2::ca75:1c0", NULL};

//绑定的方式
#define DIED_ACIVE -1 /* 挂了*/
#define NO_ACTIVE      0  /*未绑定*/
#define MAIL_ACTIVE    1  /*信箱验证*/
#define PHONE_ACTIVE   2  /*手机验证*/
#define IDCARD_ACTIVE  3  /*手工上传身份证*/
#define FORCE_ACTIVE   4  /*站务强制激活*/

//验证码的长度
#define CODELEN 8
#define VALUELEN 80

struct active_data{
	char userid[IDLEN+2];
	char name[STRLEN];
	char dept[STRLEN];
	char ip[20];
	char regtime[32];
	char uptime[32];
	char operator[IDLEN+2];
	char email[VALUELEN];
	char phone[VALUELEN];
	char idnum[VALUELEN];
	char stdnum[VALUELEN];
	int status;
};

int invalid_mail(char* mbox);
//void gencode(char* code);
char* str_to_uppercase(char *str);
char* str_to_lowercase(char *str);
const char* style_to_str(int style);
//int send_active_mail(char* mbox, char* code,char* userid, session_t* session);
//int send_active_msg(char* phone, char* code,char* userid);
//int get_active_code(char* userid, char* code, char* value, int* style);
//int set_active_code(char* userid, char* code, char* value, int style);
int query_record_num(char* value, int style);
int write_active(struct active_data* act_data);
//int setactivefile(char* genbuf, char* userid, char* filename);
int read_active(char* userid, struct active_data* act_data);
int get_active_value(char* value, struct active_data* act_data);
//学号和信箱是否对应
// int valid_stunum(char* mbox, char* stunum);
//从学籍数据库获取需要的信息
//int get_official_data(struct active_data* act_data);
MYSQL * my_connect_mysql(MYSQL *s);

#endif

