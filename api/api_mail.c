#include <string.h>
#include <json-c/json.h>

#include "bbs.h"
#include "ytht/random.h"
#include "ythtbbs/article.h"
#include "ythtbbs/misc.h"
#include "ythtbbs/permissions.h"
#include "ythtbbs/session.h"
#include "bmy/convcode.h"

#include "api.h"
#include "apilib.h"

enum api_mail_box_type {
	API_MAIL_RECIEVE_BOX,
	API_MAIL_SENT_BOX
};

/** 获取用户的邮箱大小
 *
 * @param
 * @return
 */
static int get_user_max_mail_size(struct userec * ue);

static int get_user_mail_size(char * userid);

static int check_user_maxmail(struct userec *currentuser);

static int api_mail_get_content(ONION_FUNC_PROTO_STR, int mode);

static int api_mail_do_post(ONION_FUNC_PROTO_STR, int mode);

static char * bmy_mail_array_to_json_string(struct api_article *ba_list, int count, int mode, struct userec *ue);

static char * parse_mail(char * userid, int filetime, int mode, struct attach_link **attach_link_list);

int api_mail_list(ONION_FUNC_PROTO_STR)
{
	DEFINE_COMMON_SESSION_VARS;
	int rc;
	struct userec ue;

	if (!api_check_method(req, OR_GET))
		return api_error(p, req, res, API_RT_WRONGMETHOD);

	rc = api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info);
	if (rc != API_RT_SUCCESSFUL)
		return api_error(p, req, res, rc);

	if (getuser_s(&ue, ptr_info->userid) < 0) {
		return api_error(p, req, res, API_RT_NOSUCHUSER);
	}

	const char * str_startnum = onion_request_get_query(req, "startnum");
	const char * str_count    = onion_request_get_query(req, "count");
	const char * box_type  = onion_request_get_query(req, "box_type");

	int startnum = (str_startnum) ? atoi(str_startnum) : 999999;
	int count = (str_count) ? atoi(str_count) : 20;

	char mail_dir[80];

	int box_type_i = (box_type != NULL && box_type[0] == '1') ? API_MAIL_SENT_BOX : API_MAIL_RECIEVE_BOX;
	if (box_type_i == API_MAIL_RECIEVE_BOX)
		setmailfile_s(mail_dir, sizeof(mail_dir), ptr_info->userid, ".DIR");
	else
		setsentmailfile_s(mail_dir, sizeof(mail_dir), ptr_info->userid, ".DIR");

	int total = ytht_file_size_s(mail_dir) / sizeof(struct fileheader);

	if (!total) {
		return api_error(p, req, res, API_RT_MAILEMPTY);
	}

	FILE *fp = fopen(mail_dir, "r");
	if (fp == NULL) {
		return api_error(p, req, res, API_RT_MAILDIRERR);
	}

	if (startnum == 0 || startnum > total-count+1)
		startnum = total - count + 1;
	if (startnum <= 0)
		startnum = 1;

	struct fileheader x;
	int i;
	struct api_article *mail_list = calloc(count, sizeof(struct api_article));
	if (mail_list == NULL) {
		fclose(fp);
		return api_error(p, req, res, API_RT_NOTENGMEM);
	}

	if (fseek(fp, (startnum - 1) * sizeof(struct fileheader), SEEK_SET) < 0) {
		fclose(fp);
		free(mail_list);
		return api_error(p, req, res, API_RT_FILEERROR);
	}

	for (i = 0; i < count; ++i) {
		if (fread(&x, sizeof(x), 1, fp) != 1)
			break;

		mail_list[i].sequence_num = i + startnum;
		mail_list[i].mark = x.accessed;
		ytht_strsncpy(mail_list[i].author, fh2owner(&x), sizeof(mail_list[i].author));
		mail_list[i].filetime = x.filetime;
		x.title[sizeof(x.title) - 1] = 0;
		g2u(x.title, strlen(x.title), mail_list[i].title, sizeof(mail_list[i].title));
	}

	fclose(fp);

	char *s = bmy_mail_array_to_json_string(mail_list, count, 0, &ue);

	api_set_json_header(res);
	onion_response_write0(res, s);
	free(s);
	free(mail_list);
	return OCS_PROCESSED;
}

int api_mail_getHTMLContent(ONION_FUNC_PROTO_STR)
{
	return api_mail_get_content(p, req, res, ARTICLE_PARSE_WITH_ANSICOLOR);
}

int api_mail_getRAWContent(ONION_FUNC_PROTO_STR)
{
	return api_mail_get_content(p, req, res, ARTICLE_PARSE_WITHOUT_ANSICOLOR);
}

int api_mail_send(ONION_FUNC_PROTO_STR)
{
	return api_mail_do_post(p, req, res, API_POST_TYPE_POST);
}

int api_mail_reply(ONION_FUNC_PROTO_STR)
{
	return api_mail_do_post(p, req, res, API_POST_TYPE_REPLY);
}

static int get_user_max_mail_size(struct userec * ue)
{
	int maxsize;
	if(ue->userlevel & PERM_SYSOP)
		maxsize = MAX_SYSOPMAIL_HOLD;
	else if(ue->userlevel & PERM_SPECIAL1)
		maxsize = MAX_MAIL_HOLD * 20;
	else if(ue->userlevel & PERM_BOARDS)
		maxsize = MAX_MAIL_HOLD * 8;
	else
		maxsize = MAX_MAIL_HOLD * 3;

	return maxsize * 10;
}

static int get_user_mail_size(char * userid)
{
	int currsize = 0;
	char currmaildir[STRLEN], tmpmail[STRLEN];
	struct fileheader tmpfh;
	FILE *fp;
	time_t t;

	sethomefile_s(tmpmail, sizeof(tmpmail), userid, "msgindex");
	if(file_time(tmpmail))
		currsize += ytht_file_size_s(tmpmail);

	sethomefile_s(tmpmail, sizeof(tmpmail), userid, "msgindex2");
	if(file_time(tmpmail))
		currsize += ytht_file_size_s(tmpmail);

	sethomefile_s(tmpmail, sizeof(tmpmail), userid, "msgcontent");
	if(file_time(tmpmail))
		currsize += ytht_file_size_s(tmpmail);

	sprintf(currmaildir, "mail/%c/%s/%s", mytoupper(userid[0]), userid, DOT_DIR);
	t = file_time(currmaildir);
	if(!t)
		return (currsize/1024);

	fp = fopen(currmaildir, "r");
	if(!fp)
		return (currsize/1024);

	while(fread(&tmpfh, 1, sizeof(tmpfh), fp) == sizeof(tmpfh)) {
		setmailfile_s(tmpmail, sizeof(tmpmail), userid, fh2fname(&tmpfh));
		currsize += ytht_file_size_s(tmpmail);
	}

	fclose(fp);
	return (currsize/1024);
}

static int check_user_maxmail(struct userec *currentuser)
{
	int currsize, maxsize;
	if(HAS_PERM(PERM_SYSOP|PERM_OBOARDS, (*currentuser)))
		return 0;

	currsize = 0;
	maxsize = get_user_max_mail_size(currentuser);
	currsize = get_user_mail_size(currentuser->userid);

	return (currsize > (maxsize + 20));
}

static int api_mail_get_content(ONION_FUNC_PROTO_STR, int mode)
{
	DEFINE_COMMON_SESSION_VARS;
	int rc;

	if (!api_check_method(req, OR_GET))
		return api_error(p, req, res, API_RT_WRONGMETHOD);

	rc = api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info);
	if (rc != API_RT_SUCCESSFUL)
		return api_error(p, req, res, rc);

	const char *str_num = onion_request_get_query(req, "num");
	const char *box_type  = onion_request_get_query(req, "box_type");

	if (!str_num)
		return api_error(p, req, res, API_RT_WRONGPARAM);

	char mail_dir[80];
	struct fileheader fh;

	int box_type_i = (box_type != NULL && box_type[0] == '1') ? API_MAIL_SENT_BOX : API_MAIL_RECIEVE_BOX;
	if(box_type_i == API_MAIL_RECIEVE_BOX)
		setmailfile_s(mail_dir, sizeof(mail_dir), ptr_info->userid, ".DIR");
	else
		setsentmailfile_s(mail_dir, sizeof(mail_dir), ptr_info->userid, ".DIR");

	FILE *fp = fopen(mail_dir, "r");
	if (fp == 0) {
		return api_error(p, req, res, API_RT_MAILINNERR);
	}

	int total = ytht_file_size_s(mail_dir) / sizeof(struct fileheader);
	int num = atoi(str_num);

	if (num <= 0)
		num = 1;
	if (num > total)
		num = total;

	if (fseek(fp, (num - 1) * sizeof(struct fileheader), SEEK_SET) < 0 || fread(&fh, sizeof(fh), 1, fp) != 1) {
		fclose(fp);
		return api_error(p, req, res, API_RT_MAILINNERR);
	}

	fclose(fp);

	char title_utf[240];
	fh.title[sizeof(fh.title) - 1] = 0;
	g2u(fh.title, strlen(fh.title), title_utf, 240);

	struct attach_link *attach_link_list = NULL;
	char * mail_content_utf8 = parse_mail(ptr_info->userid, fh.filetime, mode, &attach_link_list);

	if (!mail_content_utf8) {
		// 文件不存在
		free_attach_link_list(attach_link_list);
		return api_error(p, req, res, API_RT_MAILEMPTY);
	}

	char *mail_json_str = (char *) malloc(strlen(mail_content_utf8) + 512);

	if (!mail_json_str) {
		free(mail_content_utf8);
		free_attach_link_list(attach_link_list);
		return api_error(p, req, res, API_RT_NOTENGMEM);
	}

	memset(mail_json_str, 0, strlen(mail_content_utf8)+512);
	sprintf(mail_json_str, "{\"errcode\": 0, \"attach\":[]}");
	struct json_object * jp = json_tokener_parse(mail_json_str);
	if (!jp) {
		free(mail_content_utf8);
		free_attach_link_list(attach_link_list);
		free(mail_json_str);
		return api_error(p, req, res, API_RT_NOTENGMEM);
	}

	json_object_object_add(jp, "content", json_object_new_string(mail_content_utf8));
	json_object_object_add(jp, "title", json_object_new_string(title_utf));
	if(attach_link_list) {
		struct json_object * attach_array = json_object_object_get(jp, "attach");
		char at_buf[320];
		struct attach_link * alp = attach_link_list;
		while(alp) {
			memset(at_buf, 0, 320);
			sprintf(at_buf, "{\"link\": \"%s\", \"size\": %d}", alp->link, alp->size);
			json_object_array_add(attach_array, json_tokener_parse(at_buf));
			alp=alp->next;
		}
	}

	char * api_output = strdup(json_object_to_json_string(jp));

	free(mail_content_utf8);
	free_attach_link_list(attach_link_list);
	free(mail_json_str);
	json_object_put(jp);

	api_set_json_header(res);
	onion_response_write0(res, api_output);
	free(api_output);

	return OCS_PROCESSED;
}

static char * bmy_mail_array_to_json_string(struct api_article *ba_list, int count, int mode, struct userec *ue)
{
	char buf[512];
	int i;
	struct api_article *p;
	struct json_object *jp;

	sprintf(buf, "{\"errcode\":0,\"max_size\":%d, \"current_size\":%d, \"maillist\":[]}", get_user_max_mail_size(ue), get_user_mail_size(ue->userid));
	struct json_object *obj = json_tokener_parse(buf);
	struct json_object *json_array = json_object_object_get(obj, "maillist");

	for(i=0; i<count; ++i) {
		p = &(ba_list[i]);
		if(p->filetime<=0)	// 通过文件时间判断是否为空
			break;

		memset(buf, 0, 512);
		sprintf(buf, "{ \"num\": %d, \"mark\": %d, \"mid\":%ld }",
				p->sequence_num, p->mark, p->filetime);
		jp = json_tokener_parse(buf);
		if(jp) {
			json_object_object_add(jp, "title", json_object_new_string(p->title));
			json_object_object_add(jp, "author", json_object_new_string(p->author));
			json_object_array_add(json_array, jp);
		}
	}

	char *r = strdup(json_object_to_json_string(obj));
	json_object_put(obj);

	return r;
}

static int api_mail_do_post(ONION_FUNC_PROTO_STR, int mode)
{
	const char * to_userid = onion_request_get_query(req, "to_userid");
	const char * title = onion_request_get_query(req, "title");
	const char * backup = onion_request_get_query(req, "backup");

	DEFINE_COMMON_SESSION_VARS;
	int rc;

	if (!api_check_method(req, OR_POST))
		return api_error(p, req, res, API_RT_WRONGMETHOD);

	rc = api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info);
	if (rc != API_RT_SUCCESSFUL)
		return api_error(p, req, res, rc);
	if (!title || !to_userid)
		return api_error(p, req, res, API_RT_WRONGPARAM);

	struct userec currentuser;
	if (getuser_s(&currentuser, ptr_info->userid) < 0) {
		return api_error(p, req, res, API_RT_NOSUCHUSER);
	}

	if (HAS_PERM(PERM_DENYMAIL, currentuser)) {
		return api_error(p, req, res, API_RT_MAILNOPPERM);
	}

	// TODO 重构、重新生成
	if (strcmp(ptr_info->token, cookie.token) != 0) {
		return api_error(p, req, res, API_RT_WRONGTOKEN);
	}

	// 更新 token 和来源 IP
	ythtbbs_session_generate_id(ptr_info->token, TOKENLENGTH+1);
	const char * fromhost = onion_request_get_header(req, "X-Real-IP");
	ytht_strsncpy(ptr_info->from, fromhost, sizeof(ptr_info->from));

	if (check_user_maxmail(&currentuser)) {
		return api_error(p, req, res, API_RT_MAILFULL);
	}

	struct userec to_user;
	if (getuser_s(&to_user, to_userid) < 0) {
		return api_error(p, req, res, API_RT_NOSUCHUSER);
	}

	if (ythtbbs_override_included(to_user.userid, YTHTBBS_OVERRIDE_REJECTS, currentuser.userid)) {
		return api_error(p, req, res, API_RT_INUSERBLIST);
	}

	const char * data = onion_request_get_post(req, "content");

	char filename[80];
	sprintf(filename, "bbstmpfs/tmp/%s_%s.tmp", currentuser.userid, ptr_info->token);

	char *data2 = strdup(data);
	while (strstr(data2, "[ESC]") != NULL)
		data2 = string_replace(data2, "[ESC]", "\033");

	char *data_gbk = (char *) malloc(strlen(data2) * 2);
	u2g(data2, strlen(data2), data_gbk, strlen(data2) * 2);

	f_write(filename, data_gbk);
	free(data2);

	int mark=0;		// 文件标记
	//if(insertattachments(filename, data_gbk, currentuser->userid)>0)
		//mark |= FH_ATTACHED;

	free(data_gbk);

	char * title_tmp = (char *) malloc(strlen(title)*2);
	u2g(title, strlen(title), title_tmp, strlen(title)*2);
	char title_gbk[80], title_tmp2[80];
	strncpy(title_gbk, title_tmp[0]==0 ? "No Subject" : title_tmp, 80);
	snprintf(title_tmp2, 80, "{%s} %s", to_user.userid, title);
	free(title_tmp);

	rc = do_mail_post(to_user.userid, title, filename, currentuser.userid,
			currentuser.username, fromhost, 0, mark);
	if (backup && strcasecmp(backup, "true") == 0) {
		do_mail_post_to_sent_box(currentuser.userid, title_tmp2, filename, currentuser.userid,
			currentuser.username, fromhost, 0, mark);
	}

	unlink(filename);

	if (rc < 0) {
		return api_error(p, req, res, API_RT_MAILINNERR);
	}

	api_set_json_header(res);
	onion_response_printf(res, "{ \"errcode\":0, \"token\":\"%s\" }", ptr_info->token);

	return OCS_PROCESSED;
}

static char * parse_mail(char * userid, int filetime, int mode, struct attach_link **attach_link_list)
{
	if(mode != ARTICLE_PARSE_WITHOUT_ANSICOLOR && mode != ARTICLE_PARSE_WITH_ANSICOLOR)
		return NULL;

	if(!userid || filetime<=0)
		return NULL;

	char path[STRLEN];
	snprintf(path, STRLEN, "mail/%c/%s/M.%d.A", mytoupper(userid[0]), userid, filetime);
	FILE *article_stream = fopen(path, "r");
	if(!article_stream)
		return NULL;

	FILE *mem_stream, *html_stream;
	char *mem_buf, *html_buf, buf[512], attach_link[256], *tmp_buf, *attach_filename;
	size_t mem_buf_len, html_buf_len, attach_file_size;
	int attach_no = 0;

	mem_stream = open_memstream(&mem_buf, &mem_buf_len);
	fseek(article_stream, 0, SEEK_SET);
	keepoldheader(article_stream, SKIPHEADER);

	while(1) {
		if(fgets(buf, 500, article_stream) == 0)
			break;

		// RAW 模式下跳过 qmd
		if(mode == ARTICLE_PARSE_WITHOUT_ANSICOLOR && (strncmp(buf, "--\n", 3)==0))
			break;

		// 附件处理
		if(!strncmp(buf, "begin 644", 10)) {
			// TODO 老方式暂不实现
			fflush(mem_stream);
			fclose(mem_stream);
			free(mem_buf);
			fclose(article_stream);
			return NULL;
		} else if(checkbinaryattach(buf, article_stream, &attach_file_size)) {
			attach_no++;
			attach_filename = buf + 18;
			fprintf(mem_stream, "#attach %s\n", attach_filename);
			memset(attach_link, 0, 256);
			snprintf(attach_link, 256, "/api/attach/get?mid=%d&pos=%d&attname=%s",
					filetime, -4+(int)ftell(article_stream), attach_filename);
			add_attach_link(attach_link_list, attach_link, attach_file_size);
			fseek(article_stream, attach_file_size, SEEK_CUR);
			continue;
		}

		// 常规字符处理
		if(mode == ARTICLE_PARSE_WITHOUT_ANSICOLOR && strchr(buf, '\033')!=NULL) {
			tmp_buf = strdup(buf);

			while(strchr(tmp_buf, '\033') != NULL)
				tmp_buf = string_replace(tmp_buf, "\033", "[ESC]");

			fprintf(mem_stream, "%s", tmp_buf);
			free(tmp_buf);
			tmp_buf = NULL;
		} else {
			fprintf(mem_stream, "%s", buf[0]==0 ? "" : buf);
		}
	}
	fflush(mem_stream);
	fclose(article_stream);

	char *utf_content;
	if(mode == ARTICLE_PARSE_WITHOUT_ANSICOLOR) {
		if(strlen(mem_buf)==0) {
			utf_content = strdup("");
		} else {
			utf_content = (char *)malloc(3*mem_buf_len);
			memset(utf_content, 0, 3*mem_buf_len);
			g2u(mem_buf, mem_buf_len, utf_content, 3*mem_buf_len);
		}
	} else {
		html_stream = open_memstream(&html_buf, &html_buf_len);
		fseek(mem_stream, 0, SEEK_SET);

		fprintf(html_stream, "<article>\n");
		aha_convert(mem_stream, html_stream);
		fprintf(html_stream, "</article>");

		fflush(html_stream);
		fclose(html_stream);

		utf_content = (char*)malloc(3*html_buf_len);
		memset(utf_content, 0, 3*html_buf_len);
		g2u(html_buf, html_buf_len, utf_content, 3*html_buf_len);
		free(html_buf);
	}

	// 释放资源
	fclose(mem_stream);
	free(mem_buf);

	return utf_content;
}












