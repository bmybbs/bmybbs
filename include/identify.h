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
extern const char *MAIL_DOMAINS[];
extern const char *IP_POP[];

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

struct associated_userid {
	size_t count;
	char **id_array;
	int  *status_array;
};

char* str_to_uppercase(char *str);
char* str_to_lowercase(char *str);
const char* style_to_str(int style);
//int send_active_mail(char* mbox, char* code,char* userid, session_t* session);

int query_record_num(char* value, int style);
int write_active(struct active_data* act_data);

int read_active(char* userid, struct active_data* act_data);
int get_active_value(char* value, struct active_data* act_data);

MYSQL * my_connect_mysql(MYSQL *s);

/**
 * 依据邮箱地址查询关联的id列表，使用结束记得调用 free_associated_userid 释放内存。本函数属于 get_associated_userid_by_style 的封装。
 * @param email 邮件地址
 * @return struct associated_userid 指针
 */
struct associated_userid *get_associated_userid(const char *email);

/**
 * 依据认证方式关联的id列表，使用结束记得调用 free_associated_userid 释放内存。
 * @param style 认证方式，MAIL_ACTIVE | PHONE_ACTIVE | IDCARD_ACTIVE | FORCE_ACTIVE
 * @param value 关联值
 * @return struct associated_userid 指针
 */
struct associated_userid *get_associated_userid_by_style(int style, const char *value);

/**
 * 释放由 get_associated_userid 产生的内存空间。
 * @param au
 */
void free_associated_userid(struct associated_userid* au);
#endif

