#include "bbs.h"
#include "identify.h"
#include "mysql_wrapper.h"
//关于链接数据库的一些常量
#define USERREG_TABLE "userreglog"

#ifdef POP_CHECK

static const char *active_style_str[] = {"", "email", "phone", "idnum", NULL};
const char *MAIL_DOMAINS[] = {"", "xjtu.edu.cn", "stu.xjtu.edu.cn", "mail.xjtu.edu.cn", NULL};

static void convert_mysql_time_to_str(char *buf, MYSQL_TIME *mt) {
snprintf(buf, 20,
		"%04d-%02d-%02d %02d:%02d:%02d",
		mt->year, mt->month, mt->day,
		mt->hour, mt->minute, mt->second);
}

char* str_to_uppercase(char *str)
{
	char *h = str;
	while (*str != '\n' && *str != 0) {
		*str = toupper(*str);
		str++;
	}
	return h;
}

char* str_to_lowercase(char *str)
{
	char *h = str;
	while (*str != '\n' && *str != 0) {
		*str = tolower(*str);
		str++;
	}
	return h;
}

const char* style_to_str(int style)
{
	switch (style) {
	case NO_ACTIVE:     return "未验证";
	case MAIL_ACTIVE:   return "信箱";
	case PHONE_ACTIVE:  return "手机号码";
	case IDCARD_ACTIVE: return "其他证件";
	case FORCE_ACTIVE:  return "手工激活";
	default:            return "未知";
	}
}

static const char* style_to_str_utf8(int style)
{
	switch (style) {
	case NO_ACTIVE:     return "\xE6\x9C\xAA\xE9\xAA\x8C\xE8\xAF\x81";
	case MAIL_ACTIVE:   return "\xE4\xBF\xA1\xE7\xAE\xB1";
	case PHONE_ACTIVE:  return "\xE6\x89\x8B\xE6\x9C\xBA\xE5\x8F\xB7\xE7\xA0\x81";
	case IDCARD_ACTIVE: return "\xE5\x85\xB6\xE4\xBB\x96\xE8\xAF\x81\xE4\xBB\xB6";
	case FORCE_ACTIVE:  return "\xE6\x89\x8B\xE5\xB7\xA5\xE6\xBF\x80\xE6\xB4\xBB";
	default:            return "\xE6\x9C\xAA\xE7\x9F\xA5";
	}
}

//发送验证信函
int send_active_mail(char *userid, char *email)
{
	int rc;
	struct BMYCaptcha c;
	char mail[1024];
	char url[80];
	const char *mail_subject = "BMYBBS \xe6\xb3\xa8\xe5\x86\x8c\xe7\xa1\xae\xe8\xae\xa4\xe5\x87\xbd";
	const char *mail_body_template = "\xe6\x82\xa8\xe5\xa5\xbd\xef\xbc\x8c\xe8\xbf\x99\xe6\x98\xaf%s\xe7\x9a\x84\xe5"
		"\xae\x9e\xe5\x90\x8d\xe5\x88\xb6\xe8\xae\xa4\xe8\xaf\x81\xe7\xa1\xae\xe8\xae\xa4"
		"\xe4\xbf\xa1\xe5\x87\xbd\xe3\x80\x82\xe5\xa6\x82\xe6\x9e\x9c\xe6\x82\xa8\xe6\xb2"
		"\xa1\xe6\x9c\x89\xe5\x9c\xa8\xe6\x9c\xac\xe7\xab\x99\xe6\xb3\xa8\xe5\x86\x8c\xef"
		"\xbc\x8c\xe8\xaf\xb7\xe4\xb8\x8d\xe7\x94\xa8\xe7\x90\x86\xe4\xbc\x9a\xe6\x9c\xac"
		"\xe4\xbf\xa1\xe4\xbb\xb6\xe3\x80\x82\x0a\xe6\x82\xa8\xe6\xb3\xa8\xe5\x86\x8c\xe7"
		"\x9a\x84\xe4\xbd\xbf\xe7\x94\xa8\xe8\x80\x85\xe4\xbb\xa3\xe5\x8f\xb7\xe6\x98\xaf"
		"\xef\xbc\x9a%s\x0a\xe6\x82\xa8\xe7\x9a\x84\xe5\xae\x9e\xe5\x90\x8d\xe8\xae\xa4"
		"\xe8\xaf\x81\xe9\xaa\x8c\xe8\xaf\x81\xe7\xa0\x81\xe6\x98\xaf\xef\xbc\x9a%s\x0a"
		"\xe6\x82\xa8\xe5\x8f\xaf\xe4\xbb\xa5\xe4\xbd\xbf\xe7\x94\xa8\x20\x77\x77\x77\x20"
		"\xe6\x88\x96\xe8\x80\x85\x20\x74\x65\x6c\x6e\x65\x74\x20\xe6\x96\xb9\xe5\xbc\x8f"
		"\xe7\x99\xbb\xe5\xbd\x95\xe5\x90\x8e\xe5\xa1\xab\xe5\x86\x99\xe9\xaa\x8c\xe8\xaf"
		"\x81\xe4\xbf\xa1\xe6\x81\xaf\xe3\x80\x82\x0a\x0a\x77\x77\x77\x20\xe6\x96\xb9\xe5"
		"\xbc\x8f\xef\xbc\x9a\xe7\x82\xb9\xe5\x87\xbb\xe5\xb7\xa6\xe4\xbe\xa7\xe8\xbe\xb9"
		"\xe6\xa0\x8f\xe2\x80\x9c\xe5\xa1\xab\xe5\x86\x99\xe6\xb3\xa8\xe5\x86\x8c\xe5\x8d"
		"\x95\xe2\x80\x9d\xef\xbc\x8c\xe5\xae\x8c\xe6\x88\x90\xe4\xbf\xa1\xe7\xae\xb1\xe7"
		"\xbb\x91\xe5\xae\x9a\xe8\xae\xa4\xe8\xaf\x81\xe6\x93\x8d\xe4\xbd\x9c\xe3\x80\x82"
		"\x0a\x74\x65\x6c\x6e\x65\x74\x20\xe6\x96\xb9\xe5\xbc\x8f\xef\xbc\x9a\xe8\xbf\x9b"
		"\xe5\x85\xa5\xe4\xb8\xaa\xe4\xba\xba\xe5\xb7\xa5\xe5\x85\xb7\xe7\xae\xb1\xe5\xa1"
		"\xab\xe5\x86\x99\xe6\xb3\xa8\xe5\x86\x8c\xe5\x8d\x95\xe3\x80\x82";
/*
您好，这是%s的实名制认证确认信函。如果您没有在本站注册，请不用理会本信件。
您注册的使用者代号是：%s
您的实名认证验证码是：%s
您可以使用 www 或者 telnet 方式登录后填写验证信息。

www 方式：点击左侧边栏“填写注册单”，完成信箱绑定认证操作。
telnet 方式：进入个人工具箱填写注册单。
*/
	rc = gen_captcha_for_user(userid, &c, CAPTCHA_FILE_REGISTER);
	if (rc != CAPTCHA_OK)
		return -1;

	gen_captcha_url(url, sizeof(url), c.timestamp);
	snprintf(mail, sizeof(mail), mail_body_template, " BMYBBS ", userid, url);

	rc = send_mail(email, userid, mail_subject, mail);
	if (rc != MAIL_SENDER_SUCCESS)
		return -1;

	return 1;
}

int send_resetpass_mail(char *userid, char *email) {
	int rc;
	struct BMYCaptcha c;
	char mail[1024];
	char url[80];
	const char *mail_subject = "BMYBBS \xE9\x87\x8D\xE7\xBD\xAE\xE5\xAF\x86\xE7\xA0\x81\xE7\xA1\xAE\xE8\xAE\xA4\xE5\x87\xBD";
	const char *mail_body_template = "\xE6\x82\xA8\xE5\xA5\xBD\xEF\xBC\x8C\xE8\xBF\x99\xE6\x98\xAF%s\xE7\x9A\x84\xE9\x87\x8D\xE7\xBD\xAE\xE5\xAF\x86\xE7\xA0\x81\xE7\xA1\xAE\xE8\xAE\xA4\xE4\xBF\xA1\xE5\x87\xBD\xE3\x80\x82\xE5\xA6\x82\xE6\x9E\x9C\xE6\x82\xA8\xE6\xB2\xA1\xE6\x9C\x89\xE4\xBD\xBF\xE7\x94\xA8\xE9\x87\x8D\xE7\xBD\xAE\xE5\xAF\x86\xE7\xA0\x81\xE5\x8A\x9F\xE8\x83\xBD\xEF\xBC\x8C\xE8\xAF\xB7\xE4\xB8\x8D\xE7\x94\xA8\xE7\x90\x86\xE4\xBC\x9A\xE6\x9C\xAC\xE4\xBF\xA1\xE4\xBB\xB6\xE3\x80\x82\n"
		"\xE6\x82\xA8\xE7\x94\xB3\xE8\xAF\xB7\xE9\x87\x8D\xE7\xBD\xAE\xE5\xAF\x86\xE7\xA0\x81\xE7\x9A\x84\xE4\xBD\xBF\xE7\x94\xA8\xE8\x80\x85\xE4\xBB\xA3\xE5\x8F\xB7\xE6\x98\xAF\xEF\xBC\x9A%s\n"
		"\xE6\x82\xA8\xE7\x9A\x84\xE9\x87\x8D\xE7\xBD\xAE\xE5\xAF\x86\xE7\xA0\x81\xE9\xAA\x8C\xE8\xAF\x81\xE7\xA0\x81\xE6\x98\xAF\xEF\xBC\x9A%s\n"
		"\xE6\x82\xA8\xE5\x8F\xAF\xE4\xBB\xA5\xE4\xBD\xBF\xE7\x94\xA8 www \xE8\xAE\xBF\xE9\x97\xAE\xE6\x9C\xAC\xE7\xAB\x99\xEF\xBC\x8C\xE7\x82\xB9\xE5\x87\xBB\xE7\x99\xBB\xE5\xBD\x95\xE9\xA1\xB5\xE9\x9D\xA2\xE2\x80\x9C\xE6\x89\xBE\xE5\x9B\x9E\xE7\x94\xA8\xE6\x88\xB7\xE5\x90\x8D\xE6\x88\x96\xE5\xAF\x86\xE7\xA0\x81\xE2\x80\x9C\xE5\x90\x8E\xE5\xA1\xAB\xE5\x86\x99\xE9\xAA\x8C\xE8\xAF\x81\xE4\xBF\xA1\xE6\x81\xAF\xEF\xBC\x8C\xE9\x87\x8D\xE7\xBD\xAE\xE5\xAF\x86\xE7\xA0\x81\xE3\x80\x82\n";
/*
您好，这是%s的重置密码确认信函。如果您没有使用重置密码功能，请不用理会本信件。
您申请重置密码的使用者代号是：%s
您的重置密码验证码是：%s
您可以使用 www 访问本站，点击登录页面“找回用户名或密码“后填写验证信息，重置密码。
*/
	rc = gen_captcha_for_user(userid, &c, CAPTCHA_FILE_RESET);
	if (rc != CAPTCHA_OK)
		return -1;

	gen_captcha_url(url, sizeof(url), c.timestamp);
	snprintf(mail, sizeof(mail), mail_body_template, " BMYBBS ", userid, url);

	rc = send_mail(email, userid, mail_subject, mail);
	if (rc != MAIL_SENDER_SUCCESS)
		return -1;

	return 1;
}

int send_findacc_mail(struct associated_userid *au, char *email) {
	int rc;
	char mail[2048];
	char buf1[30], *buf2;
	size_t idx;
	// BMYBBS 关联ID列表查询
	const char *mail_subject = "BMYBBS \xE5\x85\xB3\xE8\x81\x94ID\xE5\x88\x97\xE8\xA1\xA8\xE6\x9F\xA5\xE8\xAF\xA2";
/*
您正在使用%s信箱关联ID列表查询功能。如果不是您本人请求的操作，也无须担心，本信件不产生任何影响。
您当前的信箱共关联了%ld个用户。
由于系统限制，无法为您列出完整ID列表。您可以联系站长寻求帮助。
*/
	const char *mail_body_template_too_many = "\xE6\x82\xA8\xE6\xAD\xA3\xE5\x9C\xA8\xE4\xBD\xBF\xE7\x94\xA8%s\xE4\xBF\xA1\xE7\xAE\xB1\xE5\x85\xB3\xE8\x81\x94ID\xE5\x88\x97\xE8\xA1\xA8\xE6\x9F\xA5\xE8\xAF\xA2\xE5\x8A\x9F\xE8\x83\xBD\xE3\x80\x82\xE5\xA6\x82\xE6\x9E\x9C\xE4\xB8\x8D\xE6\x98\xAF\xE6\x82\xA8\xE6\x9C\xAC\xE4\xBA\xBA\xE8\xAF\xB7\xE6\xB1\x82\xE7\x9A\x84\xE6\x93\x8D\xE4\xBD\x9C\xEF\xBC\x8C\xE4\xB9\x9F\xE6\x97\xA0\xE9\xA1\xBB\xE6\x8B\x85\xE5\xBF\x83\xEF\xBC\x8C\xE6\x9C\xAC\xE4\xBF\xA1\xE4\xBB\xB6\xE4\xB8\x8D\xE4\xBA\xA7\xE7\x94\x9F\xE4\xBB\xBB\xE4\xBD\x95\xE5\xBD\xB1\xE5\x93\x8D\xE3\x80\x82\n"
		"\xE6\x82\xA8\xE5\xBD\x93\xE5\x89\x8D\xE7\x9A\x84\xE4\xBF\xA1\xE7\xAE\xB1\xE5\x85\xB1\xE5\x85\xB3\xE8\x81\x94\xE4\xBA\x86%ld\xE4\xB8\xAA\xE7\x94\xA8\xE6\x88\xB7\xE3\x80\x82\n"
		"\xE7\x94\xB1\xE4\xBA\x8E\xE7\xB3\xBB\xE7\xBB\x9F\xE9\x99\x90\xE5\x88\xB6\xEF\xBC\x8C\xE6\x97\xA0\xE6\xB3\x95\xE4\xB8\xBA\xE6\x82\xA8\xE5\x88\x97\xE5\x87\xBA\xE5\xAE\x8C\xE6\x95\xB4ID\xE5\x88\x97\xE8\xA1\xA8\xE3\x80\x82\xE6\x82\xA8\xE5\x8F\xAF\xE4\xBB\xA5\xE8\x81\x94\xE7\xB3\xBB\xE7\xAB\x99\xE9\x95\xBF\xE5\xAF\xBB\xE6\xB1\x82\xE5\xB8\xAE\xE5\x8A\xA9\xE3\x80\x82";
/*
您正在使用%s信箱关联ID列表查询功能。如果不是您本人请求的操作，也无须担心，本信件不产生任何影响。
您当前的信箱共关联了%ld个用户。
详细列表如下：
%s
*/
	const char *mail_body_template_normal = "\xE6\x82\xA8\xE6\xAD\xA3\xE5\x9C\xA8\xE4\xBD\xBF\xE7\x94\xA8%s\xE4\xBF\xA1\xE7\xAE\xB1\xE5\x85\xB3\xE8\x81\x94ID\xE5\x88\x97\xE8\xA1\xA8\xE6\x9F\xA5\xE8\xAF\xA2\xE5\x8A\x9F\xE8\x83\xBD\xE3\x80\x82\xE5\xA6\x82\xE6\x9E\x9C\xE4\xB8\x8D\xE6\x98\xAF\xE6\x82\xA8\xE6\x9C\xAC\xE4\xBA\xBA\xE8\xAF\xB7\xE6\xB1\x82\xE7\x9A\x84\xE6\x93\x8D\xE4\xBD\x9C\xEF\xBC\x8C\xE4\xB9\x9F\xE6\x97\xA0\xE9\xA1\xBB\xE6\x8B\x85\xE5\xBF\x83\xEF\xBC\x8C\xE6\x9C\xAC\xE4\xBF\xA1\xE4\xBB\xB6\xE4\xB8\x8D\xE4\xBA\xA7\xE7\x94\x9F\xE4\xBB\xBB\xE4\xBD\x95\xE5\xBD\xB1\xE5\x93\x8D\xE3\x80\x82\n"
		"\xE6\x82\xA8\xE5\xBD\x93\xE5\x89\x8D\xE7\x9A\x84\xE4\xBF\xA1\xE7\xAE\xB1\xE5\x85\xB1\xE5\x85\xB3\xE8\x81\x94\xE4\xBA\x86%ld\xE4\xB8\xAA\xE7\x94\xA8\xE6\x88\xB7\xE3\x80\x82\n"
		"\xE8\xAF\xA6\xE7\xBB\x86\xE5\x88\x97\xE8\xA1\xA8\xE5\xA6\x82\xE4\xB8\x8B\xEF\xBC\x9A\n%s";
	if (au == NULL || au->count == 0) return -2;
	if (au->count > 40) {
		snprintf(mail, 2048, mail_body_template_too_many, " BMYBBS ", au->count);
	} else {
		buf2 = calloc(au->count, 30);
		if (buf2 == NULL) {
			return -3;
		}

		for (idx=0; idx<au->count; idx++) {
			sprintf(buf1, "%s\t%s\n", au->id_array[idx], style_to_str_utf8(au->status_array[idx]));
			strcat(buf2, buf1);
		}

		snprintf(mail, 2048, mail_body_template_normal, " BMYBBS ", au->count, buf2);
		free(buf2);
	}

	rc = send_mail(email, "BMYBBS USER", mail_subject, mail);
	if (rc != MAIL_SENDER_SUCCESS)
		return -4;

	return 1;
}

int get_active_value(char* value, struct active_data* act_data)
{
	if (act_data->status==0) {
		return 0;
	}
	else if (act_data->status==MAIL_ACTIVE) {
		strcpy(value, act_data->email);
	}
	else if (act_data->status==PHONE_ACTIVE) {
		strcpy(value, act_data->phone);
	}
	else if (act_data->status==IDCARD_ACTIVE) {
		strcpy(value, act_data->idnum);
	}
	return 1;
}

static void query_record_num_callback(MYSQL_STMT *stmt, MYSQL_BIND *result_col, void *result_set) {
	mysql_stmt_fetch(stmt);
}

//查询某个记录绑定了几个id
int query_record_num(char* value, int style)
{
	char sqlbuf[512];
	char *str;
	MYSQL_BIND params[1], results[1];
	int status;
	char count[8];

	memset(count, 0, sizeof(count));
	str = strdup(value);
	str_to_lowercase(str);
	memset(params, 0, sizeof(params));
	memset(results, 0, sizeof(results));

	params[0].buffer_type = MYSQL_TYPE_STRING;
	params[0].buffer = str;
	params[0].buffer_length = strlen(str);

	results[0].buffer_type = MYSQL_TYPE_STRING;
	results[0].buffer = count;
	results[0].buffer_length = sizeof(count);

	sprintf(sqlbuf,"SELECT count(*) FROM %s WHERE %s=? AND status>0;", USERREG_TABLE, active_style_str[style]);
	status = execute_prep_stmt(sqlbuf, params, results, NULL, query_record_num_callback);

	free(str);
	return (status != MYSQL_OK) ? -1 : atoi(count);
}

static void write_active_callback_count(MYSQL_STMT *stmt, MYSQL_BIND *result_cols, void *result_set) {
	mysql_stmt_fetch(stmt);
}

static void write_active_callback_I_U(MYSQL_STMT *stmt, MYSQL_BIND *result_cols, void *result_set) {
	*(int *) result_set = mysql_affected_rows(stmt->mysql);
}

/*写入数据库
 */
int write_active(struct active_data* act_data)
{
	char sqlbuf[512];
	char count_s[8];
	int count, rc, row_affected;
	MYSQL_BIND params_count[1], params_I_U[10], results[1];

	memset(params_count, 0, sizeof(params_count));
	memset(params_I_U, 0, sizeof(params_I_U));
	memset(results, 0, sizeof(results));
	memset(count_s, 0, sizeof(count_s));
	rc = MYSQL_OK;
	row_affected = 0;

	sprintf(sqlbuf,"SELECT count(*) FROM %s WHERE userid=?;", USERREG_TABLE);
	params_count->buffer_type = MYSQL_TYPE_STRING;
	params_count->buffer = act_data->userid;
	params_count->buffer_length = strlen(act_data->userid);
	results->buffer_type = MYSQL_TYPE_STRING;
	results->buffer = count_s;
	results->buffer_length = sizeof(count_s);

	rc = execute_prep_stmt(sqlbuf, params_count, results, NULL, write_active_callback_count);
	if (rc != MYSQL_OK)
		return WRITE_FAIL;

	count = atoi(count_s);
	if (count == 0) {
		sprintf(sqlbuf,
			"INSERT INTO %s(name, ip, regtime, updatetime, operator, email, phone, idnum, studnum, dept, status, userid) "
			"VALUES(?, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, ?, ?, ?, ?, ?, ?, ?, ?);", USERREG_TABLE);
	} else{
		sprintf(sqlbuf,
			"UPDATE %s SET updatetime=CURRENT_TIMESTAMP, name=?, ip=?, operator=?, email=?, phone=?, idnum=?, "
			"studnum=?, dept=?, status=? WHERE userid=?;", USERREG_TABLE);
	}

	params_I_U[0].buffer = act_data->name;
	params_I_U[0].buffer_length = strlen(act_data->name);
	params_I_U[0].buffer_type = MYSQL_TYPE_STRING;

	params_I_U[1].buffer = act_data->ip;
	params_I_U[1].buffer_length = strlen(act_data->ip);
	params_I_U[1].buffer_type = MYSQL_TYPE_STRING;

	params_I_U[2].buffer = act_data->operator;
	params_I_U[2].buffer_length = strlen(act_data->operator);
	params_I_U[2].buffer_type = MYSQL_TYPE_STRING;

	params_I_U[3].buffer = act_data->email;
	params_I_U[3].buffer_length = strlen(act_data->email);
	params_I_U[3].buffer_type = MYSQL_TYPE_STRING;

	params_I_U[4].buffer = act_data->phone;
	params_I_U[4].buffer_length = strlen(act_data->phone);
	params_I_U[4].buffer_type = MYSQL_TYPE_STRING;

	params_I_U[5].buffer = act_data->idnum;
	params_I_U[5].buffer_length = strlen(act_data->idnum);
	params_I_U[5].buffer_type = MYSQL_TYPE_STRING;

	params_I_U[6].buffer = act_data->stdnum;
	params_I_U[6].buffer_length = strlen(act_data->stdnum);
	params_I_U[6].buffer_type = MYSQL_TYPE_STRING;

	params_I_U[7].buffer = act_data->dept;
	params_I_U[7].buffer_length = strlen(act_data->dept);
	params_I_U[7].buffer_type = MYSQL_TYPE_STRING;

	params_I_U[8].buffer = &(act_data->status);
	params_I_U[8].buffer_length = sizeof(int);
	params_I_U[8].buffer_type = MYSQL_TYPE_LONG;

	params_I_U[9].buffer = act_data->userid;
	params_I_U[9].buffer_length = strlen(act_data->userid);
	params_I_U[9].buffer_type = MYSQL_TYPE_STRING;

	rc = execute_prep_stmt(sqlbuf, params_I_U, NULL, &row_affected, write_active_callback_I_U);
	return (rc != MYSQL_OK || row_affected != 1) ? WRITE_FAIL : ((count == 0) ? WRITE_SUCCESS : UPDATE_SUCCESS);
}

static void read_active_callback(MYSQL_STMT *stmt, MYSQL_BIND *result_col, void *result_set) {
	*(int *)result_set = mysql_stmt_num_rows(stmt);
	if (*(int *)result_set > 0) {
		mysql_stmt_fetch(stmt);
	}
}

int read_active(char* userid, struct active_data* act_data)
{
	char sqlbuf[128];
	int count, rc;
	MYSQL_BIND params[1], results[11];
	MYSQL_TIME regtime, uptime;

	strncpy(act_data->userid, userid, IDLEN);
	sprintf(sqlbuf,"SELECT name, dept, ip, regtime, updatetime, operator, email, phone, idnum, studnum, status FROM %s WHERE userid=?;", USERREG_TABLE);

	memset(params, 0, sizeof(params));
	memset(results, 0, sizeof(results));

	params[0].buffer_type = MYSQL_TYPE_STRING;
	params[0].buffer = act_data->userid;
	params[0].buffer_length = strlen(act_data->userid);

	results[0].buffer_type = MYSQL_TYPE_STRING;
	results[0].buffer = act_data->name;
	results[0].buffer_length = STRLEN;

	results[1].buffer_type = MYSQL_TYPE_STRING;
	results[1].buffer = act_data->dept;
	results[1].buffer_length = STRLEN;

	results[2].buffer_type = MYSQL_TYPE_STRING;
	results[2].buffer = act_data->ip;
	results[2].buffer_length = BMY_IPV6_LEN;

	results[3].buffer_type = MYSQL_TYPE_TIMESTAMP;
	results[3].buffer = &regtime;
	results[3].buffer_length = sizeof(MYSQL_TIME);

	results[4].buffer_type = MYSQL_TYPE_TIMESTAMP;
	results[4].buffer = &uptime;
	results[4].buffer_length = sizeof(MYSQL_TIME);

	results[5].buffer_type = MYSQL_TYPE_STRING;
	results[5].buffer = act_data->operator;
	results[5].buffer_length = IDLEN+2;

	results[6].buffer_type = MYSQL_TYPE_STRING;
	results[6].buffer = act_data->email;
	results[6].buffer_length = VALUELEN;

	results[7].buffer_type = MYSQL_TYPE_STRING;
	results[7].buffer = act_data->phone;
	results[7].buffer_length = VALUELEN;

	results[8].buffer_type = MYSQL_TYPE_STRING;
	results[8].buffer = act_data->idnum;
	results[8].buffer_length = VALUELEN;

	results[9].buffer_type = MYSQL_TYPE_STRING;
	results[9].buffer = act_data->stdnum;
	results[9].buffer_length = VALUELEN;

	results[10].buffer_type = MYSQL_TYPE_INT24;
	results[10].buffer = &(act_data->status);
	results[10].buffer_length = sizeof(int);

	rc = execute_prep_stmt(sqlbuf, params, results, &count, read_active_callback);
	convert_mysql_time_to_str(act_data->regtime, &regtime);
	convert_mysql_time_to_str(act_data->uptime, &uptime);

	return (rc != MYSQL_OK) ? 0 : count;
}

static void get_associated_userid_callback(MYSQL_STMT *stmt, MYSQL_BIND *result_cols, void *result_set) {
	struct associated_userid **au = result_set;
	size_t idx;
	size_t rows;

	char *userid;
	long *user_status;

	*au = NULL;
	rows = mysql_stmt_num_rows(stmt);
	if (rows == 0) return;

	userid = result_cols[0].buffer;
	user_status = (long *) result_cols[1].buffer;

	*au = (struct associated_userid *) calloc(1, sizeof(struct associated_userid));
	(*au)->count = rows;
	(*au)->id_array = (char**) calloc(rows, sizeof(void *));
	(*au)->status_array = (int *) calloc(rows, sizeof(int));

	for (idx=0; idx < rows; idx++) {
		mysql_stmt_fetch(stmt);
		(*au)->id_array[idx] = strdup(userid);
		(*au)->status_array[idx] = (*user_status) % 10;
	}
}

struct associated_userid *get_associated_userid(const char *email) {
	return get_associated_userid_by_style(MAIL_ACTIVE, email);
}

struct associated_userid *get_associated_userid_by_style(int style, const char *value) {
	if (style <= 0 || style >= 4) return NULL;
	char sqlbuf[128];
	char userid[IDLEN + 1];
	long user_status;
	MYSQL_BIND params[1], results[2];
	struct associated_userid *au;

	user_status = 0;
	snprintf(sqlbuf, 128, "SELECT userid, status FROM " USERREG_TABLE " WHERE %s=?;", active_style_str[style]);

	memset(params, 0, sizeof(params));
	memset(results, 0, sizeof(results));

	params[0].buffer_type = MYSQL_TYPE_STRING;
	params[0].buffer = (void *)value;
	params[0].buffer_length = strlen(value);

	results[0].buffer_type = MYSQL_TYPE_STRING;
	results[0].buffer = (void *)userid;
	results[0].buffer_length = IDLEN;

	results[1].buffer_type = MYSQL_TYPE_LONG;
	results[1].buffer = (void *) &user_status;
	results[1].buffer_length = sizeof(long);

	au = NULL;
	execute_prep_stmt(sqlbuf, params, results, (void *) &au, get_associated_userid_callback);
	return au;
}

void free_associated_userid(struct associated_userid *au) {
	size_t i;
	if (au == NULL) return;

	for (i = 0; i < au->count; i++) {
		if (au->id_array[i])
			free(au->id_array[i]);
	}

	if (au->count > 0) {
		free(au->id_array);
		free(au->status_array);
	}

	free(au);
}

#endif

