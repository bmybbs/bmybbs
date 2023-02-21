#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <libxml/HTMLtree.h>
#include <libxml/xpath.h>
#include <json-c/json.h>
#include <onion/block.h>

#include "bbs.h"
#include "ytht/numbyte.h"
#include "ytht/common.h"
#include "ytht/fileop.h"
#include "ytht/random.h"
#include "bmy/convcode.h"
#include "bmy/article.h"
#include "ythtbbs/cache.h"
#include "ythtbbs/commend.h"
#include "ythtbbs/docutil.h"
#include "ythtbbs/article.h"
#include "ythtbbs/notification.h"
#include "ythtbbs/session.h"

#include "api.h"
#include "apilib.h"
#include "apiconfig.h"
/**
 * @brief 将 struct api_article 数组序列化为 json 字符串。
 * 这个方法不考虑异常，因此方法里确定了 errcode 为 0，也就是 API_RT_SUCCESSFUL，
 * 相关的异常应该在从 BMY 数据转为 api_article 的过程中判断、处理。
 * @param ba_list struct api_article 数组
 * @param count 数组长度
 * @return json_object 指针
 * @warning 记得调用完成 json_object_put
 */
static struct json_object *api_article_json_array(struct api_article *ba_list, int count);
static struct json_object *api_article_with_num_array_to_json(struct api_article *ba_list, int count, int mode);

/**
 * @brief 将十大、分区热门话题转为 JSON 数据输出
 * @param mode 0 为十大模式，1 为分区模式
 * @param secstr 分区字符
 * @return
 */
static int api_article_list_xmltopfile(ONION_FUNC_PROTO_STR, int mode, const char *secstr);

/**
 * @brief 将美文推荐，或通知公告转为JSON数据输出
 * @param board 版面名
 * @param mode 0 为美文推荐，1为通知公告
 * @param startnum 输出的第一篇文章序号，默认为(最新的文章-number)
 * @param number 总共输出的文章数，暂时默认为20
 * @return 返回json格式的查询结果
 */
static int api_article_list_commend(ONION_FUNC_PROTO_STR, int mode, int startnum, int number);

/**
 * @brief 将版面文章列表转为JSON数据输出
 * @param board 版面名
 * @param mode 0为一般模式， 1为主题模式
 * @param startnum 输出的第一篇文章序号，默认为(最新的文章-number)
 * @param number 总共输出的文章数，由用户设定，暂时默认为20
 * @return 返回json格式的查询结果
 */
static int api_article_list_board(ONION_FUNC_PROTO_STR);

/**
 * @brief 将同主题文章列表转为JSON数据输出
 * @param board 版面名
 * @param thread 主题ID
 * @param startnum 输出的第一篇文章序号，默认为(1)
 * @param number 总共输出的文章数，由用户设定，默认为全部内容
 * @return 返回json格式的查询结果
 */
static int api_article_list_thread(ONION_FUNC_PROTO_STR);

static int api_article_list_boardtop(ONION_FUNC_PROTO_STR);

static int api_article_list_section(ONION_FUNC_PROTO_STR);

/**
 * @brief 实际处理发文的接口。
 * 使用 api_article_post 和 api_article_reply 封装。
 * @param mode 参见 API_POST_TYPE
 * @return
 */
static int api_article_do_post(ONION_FUNC_PROTO_STR, int mode);

/**
 * @brief 通过版面名，文章ID，查找对应主题ID
 * @param board : board name
 * @param filetime : file id
 * @return thread id; return 0 means not find the thread id
 */
static int get_thread_by_filetime(char *board, int filetime);

/**
 * @brief 通过同主题ID查找同主题文章的帖子数、总大小，以及参与评论的用户 ID
 * 更新的字段包括：
 * * th_num
 * * th_size
 * * latest
 * * th_commenter_count
 * * th_commenter
 * @param ba struct api_article，API 中缓存帖子信息的结构体
 * @param pmf 映射的 .DIR mmapfile
 */
static void parse_thread_info(struct api_article *ba, const struct mmapfile *pmf);

/**
 * @brief 通过主题ID查找同主题文章数量
 * @param thread : the thread id
 * @return the nubmer of articles in the thread
 */
static int get_number_of_articles_in_thread(char *board, int thread);

/**
 * @brief
 * @param mode
 * mode = 1 : 通过 boardname 和 主题ID 查找该主题第一篇文章 fileheader
 * mode = 0 : 通过 boardname 和 filetime 查找该文章的 fileheader
 * @param id:
 * mode = 1 : thread = id;
 * mode = 0 : filetime = id;
 * @param fh_for_return : 查找到的fileheader，值全为0 表示未找到
 * @return void
 */
static void get_fileheader_by_filetime_thread(int mode, char *board, int id, struct fileheader * fh_for_return);

/**
 * @brief 获取文章内容。
 * api_article_getHTMLContent() 和 api_article_getRAWContent() 放个方法
 * 实际上是这个方法的封装，通过 mode 参数进行区别。
 * article/getHTMLContent 和 article/getRAWContent 两个接口单独区分开，意
 * 在强调在做修改文章操作时，<strong>应当</strong>调用 getRAWcontent。
 * @param mode 参见 enum article_parse_mode
 * @return
 */
static int api_article_get_content(ONION_FUNC_PROTO_STR, int mode);

/**
 * @brief 从 .DIR 中依据 filetime 寻找文章对应的 fileheader 数据
 * @param mf 映射到内存中的 .DIR 文件内容
 * @param filetime 文件的时间戳
 * @param num
 * @param mode 1表示 .DIR 按时间排序，0表示不排序
 * @return
 */
static struct fileheader * findbarticle(struct mmapfile *mf, time_t filetime, int *num, int mode);

int api_article_list(ONION_FUNC_PROTO_STR)
{
	const char *type = onion_request_get_query(req, "type");
	//type不能为空
	if( type == NULL)
		return api_error(p, req, res, API_RT_WRONGPARAM);

	if(strcasecmp(type, "top10")==0) { // 十大
		return api_article_list_xmltopfile(p, req, res, 0, NULL);
	} else if(strcasecmp(type, "sectop")==0) { // 分区热门
		const char *secstr = onion_request_get_query(req, "secstr");
		if(secstr == NULL)
			return api_error(p, req, res, API_RT_WRONGPARAM);
		if(strlen(secstr)>2)
			return api_error(p, req, res, API_RT_WRONGPARAM);

		return api_article_list_xmltopfile(p, req, res, 1, secstr);

	} else if(strcasecmp(type, "commend")==0) { // 美文推荐
		const char *str_start = onion_request_get_query(req, "startnum");
		const char *str_number = onion_request_get_query(req, "count");
		int start = 0, number = 0;
		if(NULL != str_start)
			start = atoi(str_start);
		if(NULL != str_number)
			number = atoi(str_number);
		return api_article_list_commend(p, req, res, 0, start, number);

	} else if(strcasecmp(type, "announce")==0) { // 通知公告
		const char *str_start = onion_request_get_query(req, "startnum");
		const char *str_number = onion_request_get_query(req, "count");
		int start = 0, number = 0;
		if(NULL != str_start)
			start = atoi(str_start);
		if(NULL != str_number)
			number = atoi(str_number);
		return api_article_list_commend(p, req, res, 1, start, number);

	} else if(strcasecmp(type, "board")==0) { // 版面文章
		return api_article_list_board(p, req, res);

	} else if(strcasecmp(type, "thread")==0) { // 同主题列表
		return api_article_list_thread(p, req, res);
	} else if(strcasecmp(type, "boardtop")==0) {
		return api_article_list_boardtop(p, req, res);
	} else if (strcasecmp(type, "section") == 0) {
		return api_article_list_section(p, req, res);
	} else
		return api_error(p, req, res, API_RT_WRONGPARAM);
}

int api_article_getHTMLContent(ONION_FUNC_PROTO_STR)
{
	return api_article_get_content(p, req, res, ARTICLE_PARSE_WITH_ANSICOLOR);
}

int api_article_getRAWContent(ONION_FUNC_PROTO_STR)
{
	return api_article_get_content(p, req, res, ARTICLE_PARSE_WITHOUT_ANSICOLOR);
}

int api_article_getContent(ONION_FUNC_PROTO_STR) {
	return api_article_get_content(p, req, res, ARTICLE_PARSE_JAVASCRIPT);
}

static int api_article_list_xmltopfile(ONION_FUNC_PROTO_STR, int mode, const char *secstr)
{
	int listmax;
	char ttfile[40];
	if (mode == 0) { // 十大热门
		listmax = 10;
		sprintf(ttfile, "wwwtmp/ctopten");
	} else { // 分区热门
		listmax = 5;
		sprintf(ttfile, "etc/Area_Dir/%c", (secstr != NULL) ? secstr[0] : '0');
	}

	struct api_article *top_list = calloc(listmax, sizeof(struct api_article));
	if (top_list == NULL) {
		return api_error(p, req, res, API_RT_NOTENGMEM);
	}

	struct fileheader fh;

	htmlDocPtr doc = htmlParseFile(ttfile, "GBK");
	if (doc == NULL) {
		free(top_list);
		return api_error(p, req, res, API_RT_NOTOP10FILE);
	}

	char xpath_links[40], xpath_nums[16];

	if (mode == 0) { //十大
		sprintf(xpath_links, "//div[@class='td-overflow']/a");
		sprintf(xpath_nums, "//tr/td[4]");
	} else { //分区
		sprintf(xpath_links, "//div[@class='bd-overflow']/a");
		sprintf(xpath_nums, "//tr/td[3]");
	}

	xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
	if (ctx == NULL) {
		xmlFreeDoc(doc);
		free(top_list);
		return api_error(p, req, res, API_RT_XMLFMTERROR);
	}

	xmlXPathObjectPtr r_links = xmlXPathEvalExpression((const xmlChar*)xpath_links, ctx);
	xmlXPathObjectPtr r_nums = xmlXPathEvalExpression((const xmlChar*)xpath_nums, ctx);

	if(r_links->nodesetval == 0 || r_nums->nodesetval == 0) {
		goto ERROR;
	}

	int total = r_links->nodesetval->nodeNr;
	if (total == 0 || total>listmax ||
		(mode == 0 && r_nums->nodesetval->nodeNr - total != 1) ||
		(mode == 1 && r_nums->nodesetval->nodeNr != total)) {
		goto ERROR;
	}

	int i;
	xmlNodePtr cur_link, cur_num;
	char *link, *num, *t1, *t2, buf[256], tmp[16];
	for (i = 0; i < total; ++i) {
		cur_link = r_links->nodesetval->nodeTab[i];
		if (mode==0)
			cur_num = r_nums->nodesetval->nodeTab[i+1];
		else
			cur_num = r_nums->nodesetval->nodeTab[i];

		link = (char *)xmlGetProp(cur_link, (const xmlChar*)"href");
		num = (char *)xmlNodeGetContent(cur_num);

		top_list[i].type = (strstr(link, "tfind?board") != NULL);
		if (mode == 0) {
			top_list[i].th_num = atoi(num);
		} else {
			// 分区模式下num格式为 (7)
			snprintf(tmp, strlen(num)-1, "%s", num+1);
			top_list[i].th_num = atoi(tmp);
		}
		ytht_strsncpy(top_list[i].title, (const char*)xmlNodeGetContent(cur_link), 80);

		ytht_strsncpy(buf, link, sizeof(buf));

		if ((t1 = strtok(buf, "&")) == NULL)
			goto ERROR;
		if ((t2 = strchr(t1, '=')) == NULL)
			goto ERROR;
		t2++;
		ytht_strsncpy(top_list[i].board, t2, sizeof(top_list[i].board));

		if ((t1 = strtok(NULL, "&")) == NULL)
			goto ERROR;
		if ((t2 = strchr(t1, '=')) == NULL)
			goto ERROR;

		if (top_list[i].type) {
			t2++;
			ytht_strsncpy(tmp, t2, sizeof(tmp));
			top_list[i].thread = atoi(tmp);
		} else {
			t2 = t2 + 3;
			snprintf(tmp, 11, "%s", t2);
			top_list[i].filetime = atoi(tmp);
		}
		//根据 board、thread 或 filetime 得到 fileheader 补全所有信息
		if (top_list[i].type) {
			get_fileheader_by_filetime_thread(1, top_list[i].board, top_list[i].thread, &fh);
			if (fh.filetime != 0) {
				top_list[i].filetime = fh.filetime;
				strcpy(top_list[i].author, fh2owner(&fh));
			}
		} else {
			get_fileheader_by_filetime_thread(0, top_list[i].board, top_list[i].filetime, &fh);
			if (fh.thread != 0) {
				top_list[i].thread = fh.thread;
				strcpy(top_list[i].author, fh2owner(&fh));
			}
		}
	}

	struct json_object *result = api_article_json_array(top_list, total);

	xmlXPathFreeObject(r_links);
	xmlXPathFreeObject(r_nums);
	xmlXPathFreeContext(ctx);
	xmlFreeDoc(doc);
	free(top_list);

	if (result == NULL) {
		return api_error(p, req, res, API_RT_NOTENGMEM);
	}

	api_set_json_header(res);
	onion_response_write0(res, json_object_to_json_string(result));

	json_object_put(result);

	return OCS_PROCESSED;

ERROR:
	xmlXPathFreeObject(r_links);
	xmlXPathFreeObject(r_nums);
	xmlXPathFreeContext(ctx);
	xmlFreeDoc(doc);
	free(top_list);

	return api_error(p, req, res, API_RT_XMLFMTERROR);
}

static int api_article_list_commend(ONION_FUNC_PROTO_STR, int mode, int startnum, int number)
{
	if(0 >= number)
		number = 20;
	struct api_article *commend_list, EMPTY_ARTICLE;
	struct commend x;
	char dir[80];
	FILE *fp = NULL;
	if(0 == mode)
		strcpy(dir, ".COMMEND");
	else if(1 == mode)
		strcpy(dir, ".COMMEND2");
	int fsize = ytht_file_size_s(dir);
	int total = fsize / sizeof(struct commend);

	commend_list = calloc(number, sizeof(struct api_article));
	if (commend_list == NULL) {
		return api_error(p, req, res, API_RT_NOTENGMEM);
	}

	fp = fopen(dir, "r");
	if (!fp || fsize == 0) {
		free(commend_list);
		if (fp)
			fclose(fp);
		return api_error(p, req, res, API_RT_NOCMMNDFILE);
	}

	if (startnum == 0)
		startnum = total - number + 1;
	if (startnum <= 0)
		startnum = 1;

	if (fseek(fp, (startnum - 1) * sizeof(struct commend), SEEK_SET) == -1) {
		free(commend_list);
		if (fp)
			fclose(fp);
		return api_error(p, req, res, API_RT_FILEERROR);
	}

	int count = 0, length = 0;
	for (int i = 0; i < number; i++) {
		if (fread(&x, sizeof(struct commend), 1, fp) != 1)
			break;

		// 显示添加字符串终止符
		x.board[sizeof(x.board) - 1] = 0;
		x.userid[IDLEN + 1] = 0;
		x.com_user[IDLEN + 1] = 0;
		x.title[sizeof(x.title) - 1] = 0;
		x.filename[sizeof(x.filename) - 1] = 0;

		if(x.accessed & FH_ALLREPLY)
			commend_list[i].mark = x.accessed;
		commend_list[i].type = 0;
		length = strlen(x.title);
		g2u(x.title, length, commend_list[i].title, 80);
		ytht_strsncpy(commend_list[i].author, x.userid, sizeof(EMPTY_ARTICLE.author));
		ytht_strsncpy(commend_list[i].board, x.board, sizeof(EMPTY_ARTICLE.board));
		commend_list[i].filetime = atoi((char *)x.filename + 2);
		commend_list[i].thread = get_thread_by_filetime(commend_list[i].board, commend_list[i].filetime);
		commend_list[i].th_num = get_number_of_articles_in_thread(commend_list[i].board, commend_list[i].thread);
		commend_list[i].type = 0;
		++count;
	}
	fclose(fp);

	struct json_object *result = api_article_json_array(commend_list, count);
	free(commend_list);

	if (result == NULL) {
		return api_error(p, req, res, API_RT_NOTENGMEM);
	}

	api_set_json_header(res);
	onion_response_write0(res, json_object_to_json_string(result));
	json_object_put(result);
	return OCS_PROCESSED;
}

static int api_article_list_board(ONION_FUNC_PROTO_STR)
{
	DEFINE_COMMON_SESSION_VARS;
	int rc = api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info);
	const char * board        = onion_request_get_query(req, "board");
	const char * str_btype    = onion_request_get_query(req, "btype");
	const char * str_startnum = onion_request_get_query(req, "startnum");
	const char * str_count    = onion_request_get_query(req, "count");
	const char * str_page     = onion_request_get_query(req, "page");
	char logbuf[512];

	//判断必要参数
	if(!(board && str_btype))
		return api_error(p, req, res, API_RT_WRONGPARAM);

	struct boardmem *b = ythtbbs_cache_Board_get_board_by_name(board);
	if (b == NULL) {
		return api_error(p, req, res, API_RT_NOSUCHBRD);
	}
	if (rc == API_RT_SUCCESSFUL) {
		if (!check_user_read_perm_x(ptr_info, b)) {
			return api_error(p, req, res, API_RT_NOBRDRPERM);
		}
	} else {
		if (!check_guest_read_perm_x(b)) {
			return api_error(p, req, res, API_RT_NOBRDRPERM);
		}
	}

	int mode = 0, startnum = 0, count = 0;
	if (str_startnum != NULL)
		startnum = atoi(str_startnum);
	if (str_count != NULL)
		count = atoi(str_count);
	if (0 >= count)
		count = COUNT_PER_PAGE;
	if (str_btype[0] == 't')
		mode = 1;
	else
		mode = 0;
	int fd = 0;

	struct api_article *board_list = calloc(count, sizeof(struct api_article));
	if (board_list == NULL) {
		return api_error(p, req, res, API_RT_NOTENGMEM);
	}

	struct fileheader *data = NULL, x2;
	char dir[80], filename[80], fname[16];
	int i = 0, total = 0, total_article = 0;

	snprintf(dir, sizeof(dir), "boards/%s/.DIR", board);
	struct mmapfile mf = { .ptr = NULL };
	if (mmapfile(dir, &mf) == -1 || mf.size == 0) {
		free(board_list);
		return api_error(p, req, res, API_RT_EMPTYBRD);
	}

	data = (struct fileheader *) mf.ptr;
	total = mf.size / sizeof(struct fileheader);
	if (0 == mode) {
		// 一般模式
		total_article = total;
	} else if (1 == mode) {
		// 主题模式
		total_article = 0;
		for (i = 0; i < total; ++i)
			if (data[i].thread == data[i].filetime)
				++total_article;
	}

	// 如果使用分页参数，则首先依据分页计算
	if (str_page != NULL)
		startnum = total_article - count * (atoi(str_page)) + 1;

	if (startnum == 0)
		startnum = total_article - count + 1;
	if (startnum <= 0)
		startnum = 1;
	int sum = 0, num = 0;
	unsigned char c;
	struct api_article EMPTY_ARTICLE;
	for (i = 0; i < total; ++i) {
		// TODO: 高亮标题处理
		if (0 == mode)
			++sum;
		else if (1 == mode && data[i].thread == data[i].filetime)
			++sum;

		if (sum < startnum || (1 == mode && data[i].thread != data[i].filetime)) {
			continue;
		}

		if (data[i].sizebyte == 0) { // 如果内存中数据库记录的 sizebyte 为 0，则修正 .DIR 文件
			snprintf(filename, sizeof(filename), "boards/%s/%s", board, fh2fname_s(&data[i], fname, sizeof(fname)));
			c = ytht_num2byte(eff_size(filename));

			fd = open(dir, O_RDWR);
			if (fd < 0)
				break;

			flock(fd, LOCK_EX);
			lseek(fd, (startnum - 1 + i) * sizeof (struct fileheader),SEEK_SET);
			if (read(fd, &x2, sizeof (x2)) == sizeof (x2) && data[i].filetime == x2.filetime) {
				x2.sizebyte = c;
				if (lseek(fd, -1 * sizeof (x2), SEEK_CUR) != (off_t) -1) {
					if (write(fd, &x2, sizeof (x2)) == -1) {
						snprintf(logbuf, sizeof(logbuf), "write error to fileheader %s, at No. %d record, from file %s. Errno %d: %s.", dir, (startnum-1+i), filename, errno, strerror(errno));
						newtrace(logbuf);
					}
				}
			}
			flock(fd, LOCK_UN);
			close(fd);
		}

		board_list[num].mark = data[i].accessed;
		board_list[num].filetime = data[i].filetime;
		board_list[num].thread = data[i].thread;
		board_list[num].latest = (data[i].edittime ? data[i].edittime : data[i].filetime);
		board_list[num].type = mode;
		board_list[num].sequence_num = i;

		ytht_strsncpy(board_list[num].board, board, sizeof(EMPTY_ARTICLE.board));
		ytht_strsncpy(board_list[num].author, data[i].owner, sizeof(EMPTY_ARTICLE.author));
		g2u(data[i].title, strlen(data[i].title), board_list[num].title, 80);
		++num;
		if (num >= count) {
			break;
		}
	}

	bool brc_inited = false;
	char brc_file[80];
	int ulock;
	struct onebrc onebrc;

	if (ptr_info) {
		if ((ulock = userlock(ptr_info->userid, LOCK_SH)) >= 0) {
			brc_inited = true;
			sethomefile_s(brc_file, sizeof(brc_file), ptr_info->userid, "brc");
			brc_init(&ptr_info->allbrc, ptr_info->userid, brc_file);
			memset(&onebrc, 0, sizeof(struct onebrc));
			brc_getboard(&ptr_info->allbrc, &onebrc, b->header.filename);
			userunlock(ptr_info->userid, ulock);
		}
	}

	for (i = 0; i < num; ++i) {
		if (mode == 1) {
			parse_thread_info(&board_list[i], &mf);
		}

		board_list[i].unread = (ptr_info == NULL || !brc_inited) ? 1 : brc_unreadt(&onebrc, board_list[i].latest);
	}
	mmapfile(NULL, &mf);

	struct json_object *result = api_article_with_num_array_to_json(board_list, num, mode);
	free(board_list);

	if (result == NULL) {
		return api_error(p, req, res, API_RT_NOTENGMEM);
	}

	struct json_object *jp;
	if ((jp = json_object_new_int(total_article)) != NULL) {
		json_object_object_add(result, "total", jp);
	}

	api_set_json_header(res);
	onion_response_write0(res, json_object_to_json_string(result));
	return OCS_PROCESSED;
}

static int api_article_list_thread(ONION_FUNC_PROTO_STR)
{
	DEFINE_COMMON_SESSION_VARS;
	int rc = api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info);
	const char *board        = onion_request_get_query(req, "board");
	const char *str_thread   = onion_request_get_query(req, "thread");
	const char *str_startnum = onion_request_get_query(req, "startnum");
	const char *str_count    = onion_request_get_query(req, "count");
	//判断必要参数
	if (!(board && str_thread))
		return api_error(p, req, res, API_RT_WRONGPARAM);
	int thread = atoi(str_thread);
	if (thread == 0)
		return api_error(p, req, res, API_RT_WRONGPARAM);

	//判断版面访问权
	struct boardmem *b = ythtbbs_cache_Board_get_board_by_name(board);
	if (b == NULL)
		return api_error(p, req, res, API_RT_NOSUCHBRD);
	if (rc == API_RT_SUCCESSFUL) {
		if (!check_user_read_perm_x(ptr_info, b))
			return api_error(p, req, res, API_RT_FBDNUSER);
	} else {
		if (!check_guest_read_perm_x(b))
			return api_error(p, req, res, API_RT_NOSUCHBRD);
	}

	int startnum = 0,
		count = 0,
		fd = 0,
		ulock,
		i = 0,
		total = 0,
		total_article = 0,
		sum = 0,
		num = 0;
	char dir[80],
		filename[80],
		brc_file[80],
		fname[16],
		logbuf[512];
	bool brc_inited = false;
	unsigned char c;

	struct fileheader *data = NULL, x2;
	struct onebrc onebrc;
	struct api_article *board_list;
	struct api_article EMPTY_ARTICLE;

	enum api_error_code errcode = API_RT_SUCCESSFUL;

	if (str_startnum != NULL)
		startnum = atoi(str_startnum);
	if (str_count != NULL)
		count = atoi(str_count);

	snprintf(dir, sizeof(dir), "boards/%s/.DIR", board);
	struct mmapfile mf = { .ptr = NULL };
	if (mmapfile(dir, &mf) >= 0) {
		data = (struct fileheader *) mf.ptr;
		total = mf.size / sizeof(struct fileheader);
		total_article = 0;
		for (i = 0; i < total; ++i) {
			if (data[i].thread == thread)
				++total_article;
		}

		if (count == 0)
			count = total_article;

		if ((board_list = calloc(count, sizeof(struct api_article))) != NULL) {
			if (startnum == 0)
				startnum = total_article - count + 1;
			if (startnum <= 0)
				startnum = 1;

			if (ptr_info) {
				if ((ulock = userlock(ptr_info->userid, LOCK_EX)) >= 0) {
					brc_inited = true;
					sethomefile_s(brc_file, sizeof(brc_file), ptr_info->userid, "brc");
					brc_init(&ptr_info->allbrc, ptr_info->userid, brc_file);
					brc_getboard(&ptr_info->allbrc, &onebrc, board);
					userunlock(ptr_info->userid, ulock);
				}
			}

			for (i = 0; i < total; ++i) {
				if(data[i].thread != thread)
					continue;
				++sum;
				if(sum < startnum)
					continue;
				if (data[i].sizebyte == 0) {
					snprintf(filename, sizeof(filename), "boards/%s/%s", board, fh2fname_s(&data[i], fname, sizeof(fname)));
					c = ytht_num2byte(eff_size(filename));
					fd = open(dir, O_RDWR);
					if (fd < 0)
						break;
					flock(fd, LOCK_EX);
					lseek(fd, (startnum - 1 + i) * sizeof (struct fileheader), SEEK_SET);
					if (read(fd, &x2, sizeof (x2)) == sizeof (x2) && data[i].filetime == x2.filetime) {
						x2.sizebyte = c;
						lseek(fd, -1 * sizeof (x2), SEEK_CUR);
						if (write(fd, &x2, sizeof (x2)) == -1) {
							snprintf(logbuf, sizeof(logbuf), "write error to fileheader %s, at No. %d record, from file %s. Errno %d: %s.", dir, (startnum-1+i), filename, errno, strerror(errno));
							newtrace(logbuf);
						}
					}
					flock(fd, LOCK_UN);
					close(fd);
				}

				board_list[num].mark = data[i].accessed;
				board_list[num].filetime = data[i].filetime;
				board_list[num].thread = data[i].thread;
				board_list[num].type = 0;
				board_list[num].latest = data[i].edittime ? data[i].edittime : data[i].filetime;
				board_list[num].unread = (ptr_info == NULL || !brc_inited) ? true : brc_unreadt(&onebrc, board_list[num].latest);

				ytht_strsncpy(board_list[num].board, board, sizeof(EMPTY_ARTICLE.board));
				ytht_strsncpy(board_list[num].author, data[i].owner, sizeof(EMPTY_ARTICLE.author));
				g2u(data[i].title, strlen(data[i].title), board_list[num].title, 80);
				++num;
				if (num >= count)
					break;
			}
			struct json_object *result = api_article_json_array(board_list, num);
			free(board_list);

			if (result) {
				api_set_json_header(res);
				onion_response_write0(res, json_object_to_json_string(result));
				json_object_put(result);
			} else {
				errcode = API_RT_NOTENGMEM;
			}
		} else {
			errcode = API_RT_NOTENGMEM;
		}
		mmapfile(NULL, &mf);
	} else {
		errcode = API_RT_EMPTYBRD;
	}

	return (errcode == API_RT_SUCCESSFUL) ? OCS_PROCESSED : api_error(p, req, res, errcode);
}

static int api_article_list_boardtop(ONION_FUNC_PROTO_STR)
{
	DEFINE_COMMON_SESSION_VARS;
	int rc;
	struct userec ue;
	size_t size;

	const char *board = onion_request_get_query(req, "board");
	//判断必要参数
	if (!board)
		return api_error(p, req, res, API_RT_WRONGPARAM);

	rc = api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info);
	if (rc != API_RT_SUCCESSFUL)
		return api_error(p, req, res, rc);

	//判断版面访问权
	if (getuser_s(&ue, ptr_info->userid) < 0)
		return api_error(p, req, res, API_RT_NOSUCHUSER);

	struct boardmem *b = ythtbbs_cache_Board_get_board_by_name(board);
	if (b == NULL)
		return api_error(p, req, res, API_RT_NOSUCHBRD);

	ptr_info->userlevel = ue.userlevel;
	if (!check_user_read_perm_x(ptr_info, b))
		return api_error(p, req, res, API_RT_NOBRDRPERM);

	char topdir[80];
	FILE *fp;
	struct fileheader x;
	snprintf(topdir, sizeof(topdir), "boards/%s/.TOPFILE", b->header.filename);
	fp = fopen(topdir, "r");
	if (fp == 0)
		return api_error(p, req, res, API_RT_NOBRDTPFILE);

	int count = ytht_file_size_s(topdir) / sizeof(struct fileheader);
	struct api_article *board_list = calloc(count, sizeof(struct api_article));
	if (board_list == NULL) {
		fclose(fp);
		return api_error(p, req, res, API_RT_NOTENGMEM);
	}

	int i;
	for (i = 0; i < count; ++i) {
		size = fread(&x, sizeof(x), 1, fp);
		if (size != 1)
			break;

		board_list[i].filetime = x.filetime;
		board_list[i].mark = x.accessed;
		board_list[i].sequence_num = 0;
		board_list[i].thread = x.thread;
		board_list[i].th_num = get_number_of_articles_in_thread(b->header.filename, x.thread);

		strcpy(board_list[i].board, b->header.filename);
		strcpy(board_list[i].author, fh2owner(&x));
		x.title[sizeof(x.title) - 1] = 0;
		g2u(x.title, strlen(x.title), board_list[i].title, 80);
	}

	fclose(fp);

	struct json_object *result = api_article_json_array(board_list, count);
	free(board_list);

	if (result == NULL) {
		return api_error(p, req, res, API_RT_NOTENGMEM);
	}

	api_set_json_header(res);
	onion_response_write0(res, json_object_to_json_string(result));
	json_object_put(result);

	return OCS_PROCESSED;
}

static int api_article_get_content(ONION_FUNC_PROTO_STR, int mode)
{
	DEFINE_COMMON_SESSION_VARS;
	int rc = api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info);

	const char * bname = onion_request_get_query(req, "board");
	const char * aid_str = onion_request_get_query(req, "aid");

	if (!bname || !aid_str) {
		return api_error(p, req, res, API_RT_WRONGPARAM);
	}

	struct boardmem *bmem = ythtbbs_cache_Board_get_board_by_name(bname);
	if (!bmem)
		return api_error(p, req, res, API_RT_NOSUCHBRD);

	int aid = atoi(aid_str);

	if (rc == API_RT_SUCCESSFUL) {
		if (!check_user_read_perm_x(ptr_info, bmem)) {
			return api_error(p, req, res, API_RT_NOBRDRPERM);
		}
	} else {
		if (!check_guest_read_perm_x(bmem))
			return api_error(p, req, res, API_RT_NOSUCHBRD);
	}

	// 删除回复提醒
	if (rc == API_RT_SUCCESSFUL && is_post_in_notification(ptr_info->userid, bname, aid))
		del_post_notification(ptr_info->userid, bname, aid);

	int total = bmem->total;
	if (total <= 0) {
		return api_error(p, req, res, API_RT_EMPTYBRD);
	}

	char dir_file[80], filename[80];
	struct fileheader *fh = NULL;
	snprintf(filename, sizeof(filename), "M.%d.A", aid);
	snprintf(dir_file, sizeof(dir_file), "boards/%s/.DIR", bname);

	struct mmapfile mf = { .ptr = NULL };
	if (mmapfile(dir_file, &mf) == -1) {
		return api_error(p, req, res, API_RT_EMPTYBRD);
	}

	const char * num_str = onion_request_get_query(req, "num");
	int num = (num_str == NULL) ? -1 : (atoi(num_str)-1);
	fh = findbarticle(&mf, aid, &num, 1);
	if (fh == NULL) {
		mmapfile(NULL, &mf);
		return api_error(p, req, res, API_RT_NOSUCHATCL);
	}

	if (fh->owner[0] == '-') {
		mmapfile(NULL, &mf);
		return api_error(p, req, res, API_RT_ATCLDELETED);
	}

	int ulock;
	char brc_file[80];
	time_t fh_time;
	struct onebrc onebrc;

	if (rc == API_RT_SUCCESSFUL) {
		// 处理已读
		if ((ulock = userlock(ptr_info->userid, LOCK_EX)) >= 0) {
			sethomefile_s(brc_file, sizeof(brc_file), ptr_info->userid, "brc");
			brc_init(&ptr_info->allbrc, ptr_info->userid, brc_file);
			brc_getboard(&ptr_info->allbrc, &onebrc, bmem->header.filename);
			fh_time = fh->edittime ? fh->edittime : fh->filetime;
			if (brc_unreadt(&onebrc, fh_time)) {
				brc_addlistt(&onebrc, fh_time);
				brc_putboard(&ptr_info->allbrc, &onebrc);
				brc_fini(&ptr_info->allbrc, ptr_info->userid);
			}
			userunlock(ptr_info->userid, ulock);
		}
	}

	char title_utf8[180];
	memset(title_utf8, 0, 180);
	g2u(fh->title, strlen(fh->title), title_utf8, 180);

	struct attach_link *attach_link_list=NULL;
	char *article_content_utf8;
	if (mode == ARTICLE_PARSE_JAVASCRIPT)
		article_content_utf8 = parse_article_js(bmem->header.filename, filename, &attach_link_list);
	else
		article_content_utf8 = parse_article(bmem->header.filename, filename, mode, &attach_link_list);

	char article_json_str[256];
	int curr_permission = (rc == API_RT_SUCCESSFUL) ? !strncmp(ptr_info->userid, fh->owner, IDLEN+1) : 0;
	snprintf(article_json_str, sizeof(article_json_str), "{\"errcode\":0, \"attach\":[], "
			"\"can_edit\":%d, \"can_delete\":%d, \"can_reply\":%d, "
			"\"board\":\"%s\", \"author\":\"%s\", \"thread\":%ld, \"num\":%d}",
			curr_permission, curr_permission,
			!(fh->accessed & FH_NOREPLY), bmem->header.filename,
			fh2owner(fh), fh->thread, num);

	struct json_object * jp = json_tokener_parse(article_json_str);
	json_object_object_add(jp, "content", json_object_new_string(article_content_utf8));
	json_object_object_add(jp, "title", json_object_new_string(title_utf8));
	if (attach_link_list) {
		struct attach_link *alp = attach_link_list;
		struct json_object *attach_array = json_object_object_get(jp, "attach");
		struct json_object *attach = NULL;
		struct json_object *signature = NULL;
		while (alp) {
			if ((attach = json_object_new_object()) != NULL) {
				json_object_object_add(attach, "link", json_object_new_string(alp->link));
				json_object_object_add(attach, "size", json_object_new_int(alp->size));
				if (alp->name) {
					json_object_object_add(attach, "name", json_object_new_string(alp->name));
					if ((signature = json_object_new_array_ext(BMY_SIGNATURE_LEN)) != NULL) {
						for (size_t idx = 0; idx < BMY_SIGNATURE_LEN; idx++) {
							json_object_array_put_idx(signature, idx, json_object_new_int(alp->signature[idx]));
						}
						json_object_object_add(attach, "signature", signature);
					}
				}
				json_object_array_add(attach_array, attach);
			}
			alp = alp->next;
		}
	}

	char * api_output = strdup(json_object_to_json_string(jp));

	mmapfile(NULL, &mf);
	free(article_content_utf8);
	json_object_put(jp);
	free_attach_link_list(attach_link_list);

	api_set_json_header(res);
	onion_response_write0(res, api_output);
	free(api_output);
	return OCS_PROCESSED;
}

int api_article_post(ONION_FUNC_PROTO_STR)
{
	return api_article_do_post(p, req, res, API_POST_TYPE_POST);
}

int api_article_reply(ONION_FUNC_PROTO_STR)
{
	return api_article_do_post(p, req, res, API_POST_TYPE_REPLY);
}

static int api_article_do_post(ONION_FUNC_PROTO_STR, int mode)
{
	DEFINE_COMMON_SESSION_VARS;
	int rc, rid = 0, mark = 0, ulock;
	size_t title_len, data_len, i;
	time_t ref, thread = -1, r;
	bool replyMode = false,
		is_anony = false,
		is_norep = false,
		using_math = false;
	char session_token[TOKENLENGTH + 1],
		cookie_buf2[512],
		dir[80],
		buf[256],
		noti_userid[IDLEN + 1] = { '\0' },
		*title_gbk = NULL,
		*data_gbk = NULL;
	const char *board = NULL,
		*title = NULL,
		*data = NULL,
		*cookie_token,
		*host,
		*fromhost,
		*body;
	struct userec local_ue;
	struct boardmem *bmem;
	struct json_object *req_json, *obj_tmp;
	struct mmapfile mf = { .ptr = NULL };
	enum ytht_smth_filter_option option;
	enum ytht_smth_filter_result dangerous;
	const onion_block *block;
	const struct fileheader *x;

	if (!api_check_method(req, OR_POST))
		return api_error(p, req, res, API_RT_WRONGMETHOD);

	rc = api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info);
	if (rc != API_RT_SUCCESSFUL) {
		return api_error(p, req, res, rc);
	}

	host = bmy_cookie_check_host(onion_request_get_header(req, "Host"));
	if (host == NULL) {
		return api_error(p, req, res, API_RT_INVALIDHST);
	}

	fromhost = onion_request_get_header(req, "X-Real-IP");

	if ((block = onion_request_get_data(req)) == NULL) {
		return api_error(p, req, res, API_RT_WRONGPARAM);
	}

	if ((body = onion_block_data(block)) == NULL || body[0] == '\0') {
		return api_error(p, req, res, API_RT_WRONGPARAM);
	}

	if ((req_json = json_tokener_parse(body)) != NULL) {
		if ((obj_tmp = json_object_object_get(req_json, "board")) != NULL) {
			board = json_object_get_string(obj_tmp);
		}

		if ((obj_tmp = json_object_object_get(req_json, "title")) != NULL) {
			title = json_object_get_string(obj_tmp);
		}

		if ((obj_tmp = json_object_object_get(req_json, "content")) == NULL) {
			data = " ";
		} else {
			data = json_object_get_string(obj_tmp);
		}
		data_len = strlen(data);
		if ((data_gbk = malloc(data_len * 2)) != NULL) {
			u2g(data, data_len, data_gbk, data_len * 2);
		} else {
			rc = API_RT_NOTENGMEM;
		}

		if ((obj_tmp = json_object_object_get(req_json, "ref")) != NULL) {
			replyMode = true;
			ref = json_object_get_int64(obj_tmp);

			if ((obj_tmp = json_object_object_get(req_json, "rid")) != NULL) {
				rid = json_object_get_int(obj_tmp);
			}
		}

		if (!board || !title || board[0] == 0 || (mode == API_POST_TYPE_REPLY && !replyMode)) {
			rc = API_RT_WRONGPARAM;
		}

		if (rc == API_RT_SUCCESSFUL) {
			if (title[0] == 0) {
				rc = API_RT_ATCLNOTITLE;
			} else {
				title_len = strlen(title);
				if ((title_gbk = malloc(title_len * 2)) != NULL) {
					memset(title_gbk, 0, title_len * 2);
					u2g(title, title_len, title_gbk, title_len * 2);
					for (i = 0; i < strlen(title_gbk); ++i) {
						if (title_gbk[i] <= 27 && title_gbk[i] >= -1)
							title_gbk[i] = ' ';
					}
					i = strlen(title_gbk) - 1;
					while (i > 0 && isspace(title_gbk[i]))
						title_gbk[i--] = 0;
				} else {
					rc = API_RT_NOTENGMEM;
				}
			}
		}

		if (rc == API_RT_SUCCESSFUL) {
			if ((bmem = ythtbbs_cache_Board_get_board_by_name(board)) == NULL) {
				rc = API_RT_NOSUCHBRD;
			} else {
				if (!ythtbbs_board_is_hidden_x(bmem)) {
					option = ythtbbs_board_is_political_x(bmem) ? YTHT_SMTH_FILTER_OPTION_NORMAL : YTHT_SMTH_FILTER_OPTION_SIMPLE;
					if (title_gbk && data_gbk) {
						if ((dangerous = api_stringfilter(title_gbk, option)) == YTHT_SMTH_FILTER_RESULT_SAFE) {
							if ((dangerous = api_stringfilter(data_gbk, option)) != YTHT_SMTH_FILTER_RESULT_SAFE) {
								rc = API_RT_ATCL1984;
							}
						}
					}
				}
			}
		}

		if (rc == API_RT_SUCCESSFUL) {
			if (getuser_s(&local_ue, ptr_info->userid) < 0) {
				rc = API_RT_NOSUCHUSER;
			}
		}

		if (rc == API_RT_SUCCESSFUL) {
			ptr_info->userlevel = local_ue.userlevel;
			if (!check_user_post_perm_x(ptr_info, bmem)) {
				rc = API_RT_NOBRDPPERM;
			}
		}

		if (rc == API_RT_SUCCESSFUL) {
			if (mode == API_POST_TYPE_REPLY) { // 已通过参数校验
				snprintf(dir, sizeof(dir), "boards/%s/.DIR", bmem->header.filename);
				if (mmapfile(dir, &mf) >= 0) {
					if ((x = findbarticle(&mf, ref, &rid, 1)) != NULL) {
						if (x->accessed & FH_NOREPLY) {
							rc = API_RT_ATCLFBDREPLY;
						} else {
							if (x->accessed & FH_ALLREPLY) {
								mark |= FH_ALLREPLY;
							}
							thread = x->thread;
							if (strchr(x->owner, '.') == NULL) {
								ytht_strsncpy(noti_userid, (x->owner[0] == '\0') ? &x->owner[1] : x->owner, sizeof(noti_userid));
							}
						}
					} else {
						thread = -1;
					}
					mmapfile(NULL, &mf);
				} else {
					rc = API_RT_CNTMAPBRDIR;
				}
			}
		}

		// TOKEN 相关开始
		if (rc == API_RT_SUCCESSFUL) {
			if ((ulock = userlock(ptr_info->userid, LOCK_EX)) >= 0) {
				// 不管 cookie_token 是否和 session_token 相同，生成新的 token
				cookie_token = cookie.token;
				ytht_strsncpy(session_token, ptr_info->token, sizeof(session_token));
				ythtbbs_session_generate_id(ptr_info->token, sizeof(ptr_info->token));
				userunlock(ptr_info->userid, ulock);
			} else {
				rc = API_RT_USERLOCKFAIL;
			}
		}

		if (rc == API_RT_SUCCESSFUL) {
			cookie.token = ptr_info->token;
			bmy_cookie_gen(cookie_buf2, sizeof(cookie_buf2), &cookie);
			onion_response_add_cookie(res, SMAGIC, cookie_buf2, MAX_SESS_TIME - 10, "/", host, OC_HTTP_ONLY);
			if (strcmp(session_token, cookie_token) != 0) {
				rc = API_RT_WRONGTOKEN;
			}
		}
		// TOKEN 相关结束

		if (rc == API_RT_SUCCESSFUL) {
			if (!strcasecmp(ptr_info->userid, "guest") && seek_in_file(MY_BBS_HOME "/etc/guestbanip", fromhost)) {
				rc = API_RT_FBDGSTPIP;
			}
		}

		if ((obj_tmp = json_object_object_get(req_json, "anony")) != NULL) {
			is_anony = json_object_get_boolean(obj_tmp);
		}

		if ((obj_tmp = json_object_object_get(req_json, "norep")) != NULL) {
			is_norep = json_object_get_boolean(obj_tmp);
		}

		if ((obj_tmp = json_object_object_get(req_json, "math")) != NULL) {
			using_math = json_object_get_boolean(obj_tmp);
		}
		json_object_put(req_json);
	} else {
		return api_error(p, req, res, API_RT_WRONGPARAM);
	}

	if (title_gbk == NULL || data_gbk == NULL) {
		if (title_gbk)
			free(title_gbk);
		if (data_gbk)
			free(data_gbk);
		return api_error(p, req, res, API_RT_NOTENGMEM);
	}

	if (rc != API_RT_SUCCESSFUL) {
		free(title_gbk);
		free(data_gbk);
		return api_error(p, req, res, rc);
	}

	if (is_norep)
		mark |= FH_NOREPLY;
	if (using_math)
		mark |= FH_MATH;

	if (is_anony && (bmem->header.flag & ANONY_FLAG))
		is_anony = 1;
	else
		is_anony = 0;

	if (bmem->header.flag & IS1984_FLAG) {
		mark |= FH_1984;
	}

	// TODO: 处理签名档

	// TODO: 缺少 nju09/bbssnd.c:143 有关报警的逻辑

	//if(insertattachments(filename, data_gbk, ue->userid))
		//mark = mark | FH_ATTACHED;

	r = do_article_post(bmem->header.filename,
			title_gbk,
			data_gbk,
			is_anony ? "Anonymous" : ptr_info->userid,
			is_anony ? "\xCE\xD2\xCA\xC7\xC4\xE4\xC3\xFB\xCC\xEC\xCA\xB9" : ptr_info->username,
			is_anony ? "\xC4\xE4\xC3\xFB\xCC\xEC\xCA\xB9\xB5\xC4\xBC\xD2" : fromhost,
			0,
			mark,
			0,
			ptr_info->userid,
			thread);
	if (r <= 0) {
		free(title_gbk);
		free(data_gbk);
		return api_error(p, req, res, API_RT_ATCLINNERR);
	}

	snprintf(buf, sizeof(buf), "%s post %s %s", ptr_info->userid, bmem->header.filename, title_gbk);
	newtrace(buf);

	ythtbbs_cache_Board_updatelastpost_x(bmem);

	if (bmem->header.clubnum == 0 && !board_is_junkboard(bmem->header.filename)) {
		local_ue.numposts++;
		save_user_data(&local_ue);
	}

	//回帖提醒
	if (mode == API_POST_TYPE_REPLY && !strcmp(ptr_info->userid, noti_userid)) {
		add_post_notification(noti_userid, (is_anony) ? "Anonymous" : ptr_info->userid,
				bmem->header.filename, r, title_gbk);
	}

	free(title_gbk);
	free(data_gbk);
	memset(ptr_info->from, 0, sizeof(ptr_info->from));
	ytht_strsncpy(ptr_info->from, fromhost, sizeof(ptr_info->from));
	api_set_json_header(res);
	onion_response_printf(res, "{ \"errcode\":0, \"aid\":%ld, \"tid\":%ld }", r, thread);

	return OCS_PROCESSED;
}

static struct json_object *api_article_json_array(struct api_article *ba_list, int count)
{
	char buf[512];
	int i;
	const struct boardmem *b;
	const struct api_article *p;
	struct json_object *jp, *jp2;
	struct json_object *obj = json_tokener_parse("{\"errcode\":0, \"articlelist\":[]}");

	if (obj == NULL) {
		return NULL;
	}

	struct json_object *json_array = json_object_object_get(obj, "articlelist");

	for (i = 0; i < count; ++i) {
		p = &(ba_list[i]);
		b = ythtbbs_cache_Board_get_board_by_name(p->board);
		snprintf(buf, sizeof(buf), "{ \"type\":%d, \"aid\":%ld, \"tid\":%ld, \"unread\":%d, "
			"\"th_num\":%d, \"mark\":%d, \"secstr\":\"%s\" }",
			p->type, p->filetime, p->thread, p->unread, p->th_num, p->mark, b->header.sec1);
		jp = json_tokener_parse(buf);
		if (jp) {
			if ((jp2 = json_object_new_string(p->board)) != NULL)
				json_object_object_add(jp, "board", jp2);
			if ((jp2 = json_object_new_string(p->title)) != NULL)
				json_object_object_add(jp, "title", jp2);
			if ((jp2 = json_object_new_string(p->author)) != NULL)
				json_object_object_add(jp, "author", jp2);
			json_object_array_add(json_array, jp);
		}
	}

	return obj;
}

static struct json_object *api_article_with_num_array_to_json(struct api_article *ba_list, int count, int mode)
{
	char buf[512];
	int i, j;
	struct api_article *p;
	struct json_object *jp, *jp2;
	struct json_object *obj = json_tokener_parse("{\"errcode\":0, \"articlelist\":[]}");
	if (obj == NULL) {
		return NULL;
	}

	struct json_object *json_array = json_object_object_get(obj, "articlelist");

	for (i = 0; i < count; ++i) {
		p = &(ba_list[i]);

		snprintf(buf, sizeof(buf), "{ \"type\":%d, \"aid\":%ld, \"tid\":%ld, \"unread\": %d, "
				"\"th_num\":%d, \"mark\":%d ,\"num\":%d, \"th_size\":%d, \"th_commenter\":[] }",
				p->type, p->filetime, p->thread, p->unread,
				p->th_num, p->mark, p->sequence_num, p->th_size);
		jp = json_tokener_parse(buf);
		if (jp) {
			if ((jp2 = json_object_new_string(p->board)) != NULL)
				json_object_object_add(jp, "board", jp2);
			if ((jp2 = json_object_new_string(p->title)) != NULL)
				json_object_object_add(jp, "title", jp2);
			if ((jp2 = json_object_new_string(p->author)) != NULL)
				json_object_object_add(jp, "author", jp2);

			if (mode == 1) {
				// 主题模式下输出评论者
				struct json_object *json_array_commenter = json_object_object_get(jp, "th_commenter");
				for (j = 0; j < p->th_commenter_count; ++j) {
					if(p->th_commenter[j][0] == 0)
						break;
					json_object_array_add(json_array_commenter, json_object_new_string(p->th_commenter[j]));
				}
			}

			json_object_array_add(json_array, jp);
		}
	}

	return obj;
}

static int get_thread_by_filetime(char *board, int filetime)
{
	char dir[80];
	struct mmapfile mf = { .ptr = NULL };
	struct fileheader *p_fh;
	int thread;

	snprintf(dir, sizeof(dir), "boards/%s/.DIR", board);

	if(mmapfile(dir, &mf) == -1)
		return 0;

	if(mf.size == 0) {
		mmapfile(NULL, &mf);
		return 0;
	}

	int total;
	total = mf.size / sizeof(struct fileheader);
	int num = Search_Bin(mf.ptr, filetime, 0, total - 1);
	if(num >=  0){
		p_fh = (struct fileheader *)(mf.ptr + num * sizeof(struct fileheader));

		thread = p_fh->thread;
		mmapfile(NULL, &mf);
		return thread;
	}
	return 0;
}

static void parse_thread_info(struct api_article *ba, const struct mmapfile *pmf)
{
	// TODO
	int i = 0, j = 0, num_records = 0, is_in_commenter_list = 0;
	time_t curr_time;
	struct fileheader *curr_article = NULL;
	if (NULL == ba || ba->board[0] == '\0')
		return ;

	num_records = pmf->size / sizeof(struct fileheader);
	if (0 != ba->thread) {
		i = Search_Bin(pmf->ptr, ba->thread, 0, num_records - 1);
		if (i < 0)
			i = -(i + 1);
	} else
		i = 0;

	for (; i < num_records; ++i) {
		curr_article = &((struct fileheader *) pmf->ptr)[i];
		if (curr_article->thread != ba->thread)
			continue;
		else {
			++ba->th_num;
			ba->th_size += ytht_num2byte(curr_article->sizebyte);
			curr_time = curr_article->edittime ? curr_article->edittime : curr_article->filetime;
			if (curr_time > ba->latest) {
				ba->latest = curr_time;
			}
			char * curr_userid = curr_article->owner;
			// 判断是否在参与评论的人之中
			if (ba->th_commenter_count < MAX_COMMENTER_COUNT) {
				is_in_commenter_list = 0;	// 对于每一篇帖子重置判断状态

				if (strcasecmp(curr_userid, ba->author) == 0) {
					continue;	// 主题作者自己不参与统计
				} else {
					for (j = 0; j < ba->th_commenter_count; ++j) {
						if (strcasecmp(curr_userid, ba->th_commenter[j]) == 0) {
							// 已统计过
							is_in_commenter_list = 1;
							break;	// 跳出循环，否则继续
						}
					}

					if (is_in_commenter_list == 0) {
						strcpy (ba->th_commenter[ba->th_commenter_count], curr_userid);
						ba->th_commenter_count++;
					}
				}
			}
		}
	}
}

static int get_number_of_articles_in_thread(char *board, int thread)
{
	char dir[80];
	int i = 0, num_in_thread = 0, num_records = 0;
	struct mmapfile mf = { .ptr = NULL };
	if(NULL == board)
		return 0;
	snprintf(dir, sizeof(dir), "boards/%s/.DIR",board);

	if(-1 == mmapfile(dir, &mf))
		return 0;

	if(mf.size == 0) {
		mmapfile(NULL, &mf);
		return 0;
	}

	num_records = mf.size / sizeof(struct fileheader);
	if(0 != thread) {
		i = Search_Bin(mf.ptr, thread, 0, num_records - 1);
		if(i < 0)
			i = -(i + 1);
	} else
		i = 0;

	for(; i < num_records; ++i) {
		if(((struct fileheader *)(mf.ptr + i * sizeof(struct fileheader)))->thread != thread)
			continue;
		else
			++num_in_thread;
	}

	mmapfile(NULL, &mf);
	return num_in_thread;
}

static void get_fileheader_by_filetime_thread(int mode, char *board, int id, struct fileheader * fh_for_return)
{
	char dir[80];
	int i = 0, num_records = 0;
	struct mmapfile mf = { .ptr = NULL };
	struct fileheader * p_fh = NULL;
	if(NULL == fh_for_return)
		return;
	memset(fh_for_return, 0, sizeof(struct fileheader));
	if(NULL == board)
		return ;
	snprintf(dir, sizeof(dir), "boards/%s/.DIR",board);

	if(-1 == mmapfile(dir, &mf))
		return;

	if(mf.size == 0) {
		mmapfile(NULL, &mf);
		return;
	}

	num_records = mf.size / sizeof(struct fileheader);
	if(0 != id) {
		i = Search_Bin(mf.ptr, id, 0, num_records - 1);
		if(i < 0)
			i = -(i + 1);
	} else
		i = 0;

	for(; i < num_records; ++i) {
		p_fh = (struct fileheader *)(mf.ptr + i * sizeof(struct fileheader));
		if((mode == 0 && p_fh->filetime == id) ||
				(mode == 1 && p_fh->thread == id)) {
			memcpy(fh_for_return, (struct fileheader *)(mf.ptr + i * sizeof(struct fileheader)), sizeof(struct fileheader));
			break;
		}
	}
	mmapfile(NULL, &mf);
	return;
}

static struct fileheader * findbarticle(struct mmapfile *mf, time_t filetime, int *num, int mode)
{
	struct fileheader *ptr;
	size_t local_num;
	size_t total = mf->size / sizeof(struct fileheader);
	if (total == 0)
		return NULL;

	if (*num < 0)
		*num = 0;
	local_num = (unsigned) *num; /* safe */
	if (local_num >= total) {
		local_num = total - 1;
		*num = local_num;
	}
	ptr = (struct fileheader *) (mf->ptr + (*num) * sizeof(struct fileheader));
	if (ptr->filetime == filetime) {
		return ptr;
	} else {
		*num = Search_Bin(mf->ptr, filetime, 0, total - 1);
		if (*num >= 0) {
			ptr = (struct fileheader *) (mf->ptr + *num * sizeof(struct fileheader));
			if (ptr->filetime == filetime) {
				return ptr;
			}
		}
	}

	// num 对应的编号不正确，且 binary search 没有找到，完整遍历
	for (local_num = 0; local_num < total; local_num++) {
		ptr = (struct fileheader *) (mf->ptr + local_num * sizeof(struct fileheader));
		if (mode && ptr->filetime < filetime)
			return NULL;

		if (ptr->filetime == filetime) {
			*num = local_num;
			return ptr;
		}
	}

	return NULL;
}

static int count_board_in_section(struct boardmem *board, int curr_idx, va_list ap) {
	(void) curr_idx;
	int rc = va_arg(ap, int);
	struct user_info *ptr_info = va_arg(ap, struct user_info *);
	int *count = va_arg(ap, int *);
	int hasintro = va_arg(ap, int);
	const char *secstr = va_arg(ap, const char *);
	int len = strlen(secstr);

	if (board->header.filename[0] <= 32 || board->header.filename[0] > 'z')
		return 0;

	if (hasintro) {
		if (strcmp(secstr, board->header.sec1) && strcmp(secstr, board->header.sec2))
			return 0;
	} else {
		if (strncmp(secstr, board->header.sec1, len) && strncmp(secstr, board->header.sec2, len))
			return 0;
	}

	if (check_guest_read_perm_x(board) || (rc == API_RT_SUCCESSFUL && check_user_read_perm_x(ptr_info, board))) {
		*count = *count + 1;
	}

	return 0;
}

static int put_boardnum_in_section(struct boardmem *board, int curr_idx, va_list ap) {
	int rc = va_arg(ap, int);
	struct user_info *ptr_info = va_arg(ap, struct user_info *);
	int *count = va_arg(ap, int *);
	int hasintro = va_arg(ap, int);
	const char *secstr = va_arg(ap, const char *);
	int *boardnum_array = va_arg(ap, int *);
	int len = strlen(secstr);

	if (board->header.filename[0] <= 32 || board->header.filename[0] > 'z')
		return 0;

	if (hasintro) {
		if (strcmp(secstr, board->header.sec1) && strcmp(secstr, board->header.sec2))
			return 0;
	} else {
		if (strncmp(secstr, board->header.sec1, len) && strncmp(secstr, board->header.sec2, len))
			return 0;
	}

	if (check_guest_read_perm_x(board) || (rc == API_RT_SUCCESSFUL && check_user_read_perm_x(ptr_info, board))) {
		boardnum_array[*count] = curr_idx + 1;
		*count = *count + 1;
	}

	return 0;
}

static int api_article_list_section(ONION_FUNC_PROTO_STR) {
	DEFINE_COMMON_SESSION_VARS;
	int rc;
	size_t count, i, board_count;
	int page;
	unsigned int offset;
	int *boardnum_array;
	char c;
	const struct sectree *sec;
	int hasintro = 0;
	unsigned long total = 0;
	const char *secstr   = onion_request_get_query(req, "secstr");
	const char *page_str = onion_request_get_query(req, "page");

	if (secstr == NULL || secstr[0] == '\0')
		return api_error(p, req, res, API_RT_WRONGPARAM);

	c = secstr[0];
	if (!((c >= '0' && c <= '9') || c == 'G' || c == 'N' || c == 'H' || c == 'A' || c == 'C'))
		return api_error(p, req, res, API_RT_WRONGPARAM);

	rc = api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info);
	if (page_str != NULL)
		page = atoi(page_str);
	else
		page = 1;

	if (page < 1)
		page = 1;

	offset = (page - 1) * COUNT_PER_PAGE;

	struct bmy_articles *articles;

	sec = getsectree(secstr);
	if (sec->introstr[0])
		hasintro = 1;

	board_count = 0;
	ythtbbs_cache_Board_foreach_v(count_board_in_section, rc, ptr_info, &board_count, hasintro, secstr);
	if (board_count == 0)
		return api_error(p, req, res, API_RT_SUCCESSFUL);

	boardnum_array = calloc(board_count, sizeof(int));
	board_count = 0;
	ythtbbs_cache_Board_foreach_v(put_boardnum_in_section, rc, ptr_info, &board_count, hasintro, secstr, boardnum_array);
	articles = bmy_article_list_selected_boards_by_offset(boardnum_array, board_count, COUNT_PER_PAGE, offset);
	total = bmy_article_total_selected_boards(boardnum_array, board_count);
	free(boardnum_array);

	if (articles == NULL || articles->count == 0) {
		goto EMPTY;
	}

	struct boardmem *b;
	for (i = 0, count = 0; i < articles->count; i++) {
		b = ythtbbs_cache_Board_get_board_by_name(articles->articles[i].boardname_en);
		if (check_guest_read_perm_x(b) || (rc == API_RT_SUCCESSFUL && check_user_read_perm_x(ptr_info, b))) {
			count++;
		}
	}

	if (count == 0) {
		goto EMPTY;
	}

	struct json_object *obj = json_object_new_object();
	struct json_object *jp_total = json_object_new_int64(total);
	struct json_object *article_array = json_object_new_array_ext(count);
	struct json_object *article_obj;

	for (i = 0, count = 0; i < articles->count; i++) {
		b = ythtbbs_cache_Board_get_board_by_name(articles->articles[i].boardname_en);
		if (check_guest_read_perm_x(b) || (rc == API_RT_SUCCESSFUL && check_user_read_perm_x(ptr_info, b))) {
			article_obj = apilib_convert_fileheader_utf_to_jsonobj(&articles->articles[i]);
			json_object_array_put_idx(article_array, count, article_obj);
			count++;
		}
	}

	json_object_object_add(obj, "articles", article_array);
	if (jp_total) {
		json_object_object_add(obj, "total", jp_total);
	}

	api_set_json_header(res);
	onion_response_write0(res, json_object_to_json_string_ext(obj, JSON_C_TO_STRING_NOSLASHESCAPE));
	json_object_put(obj);
	bmy_article_list_free(articles);
	return OCS_PROCESSED;

EMPTY:
	bmy_article_list_free(articles);
	api_set_json_header(res);
	onion_response_write0(res, "{ \"errcode\": 0 }"); // TODO
	return OCS_PROCESSED;
}

