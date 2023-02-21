#include <string.h>
#include <time.h>
#include <json-c/json.h>
#include "config.h"
#include "ytht/strlib.h"
#include "bmy/cookie.h"
#include "bmy/article.h"
#include "ythtbbs/mybrd.h"
#include "ythtbbs/session.h"
#include "ythtbbs/cache.h"
#include "api.h"
#include "apilib.h"
#include "apiconfig.h"
#include "error_code.h"

// api_board
extern bool api_mybrd_has_read_perm(const struct user_info *ptr_info, const char *boardname);

int api_subscription_list(ONION_FUNC_PROTO_STR) {
	DEFINE_COMMON_SESSION_VARS;
	int rc;
	unsigned long total = 0;

	if (!api_check_method(req, OR_GET))
		return api_error(p, req, res, API_RT_WRONGMETHOD);

	rc = api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info);
	if (rc != API_RT_SUCCESSFUL)
		return api_error(p, req, res, rc);

	const char *page_str = onion_request_get_query(req, "page");
	int page;
	size_t offset;
	if (page_str == NULL || page_str[0] == '\0')
		page = 1;
	else
		page = atol(page_str);

	if (page < 1)
		page = 1;
	offset = COUNT_PER_PAGE * (page - 1);

	struct goodboard g_brd;
	memset(&g_brd, 0, sizeof(struct goodboard));
	ythtbbs_mybrd_load_ext(ptr_info, &g_brd, api_mybrd_has_read_perm);

	int *bid_arr = calloc(g_brd.num, sizeof(int));
	if (bid_arr == NULL) {
		return api_error(p, req, res, API_RT_NOTENGMEM);
	}
	for (int i = 0; i < g_brd.num; i++) {
		bid_arr[i] = ythtbbs_cache_Board_get_idx_by_name(g_brd.ID[i]) + 1;
	}

	struct bmy_articles *articles = bmy_article_list_selected_boards_by_offset(bid_arr, g_brd.num, COUNT_PER_PAGE, offset);
	total = bmy_article_total_selected_boards(bid_arr, g_brd.num);
	free(bid_arr);

	if (articles == NULL || articles->count == 0) {
		bmy_article_list_free(articles);

		return api_error(p, req, res, API_RT_NOMOREFEED);
	}

	struct json_object *obj = json_object_new_object();
	struct json_object *article_array = json_object_new_array_ext(articles->count);
	struct json_object *article_obj;
	struct json_object *total_obj;

	size_t i;
	struct fileheader_utf *ptr_header;
	for (i = 0; i < articles->count; i++) {
		ptr_header = &articles->articles[i];
		article_obj = apilib_convert_fileheader_utf_to_jsonobj(ptr_header);
		json_object_array_put_idx(article_array, i, article_obj);
	}

	json_object_object_add(obj, "articles", article_array);
	json_object_object_add(obj, "errcode", json_object_new_int(API_RT_SUCCESSFUL));
	if ((total_obj = json_object_new_int64(total)) != NULL) {
		json_object_object_add(obj, "total", total_obj);
	}

	api_set_json_header(res);
	onion_response_write0(res, json_object_to_json_string_ext(obj, JSON_C_TO_STRING_NOSLASHESCAPE));
	json_object_put(obj);
	bmy_article_list_free(articles);
	return OCS_PROCESSED;
}

