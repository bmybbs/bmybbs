#include <string.h>
#include <json-c/json.h>
#include "ythtbbs/notification.h"

#include "api.h"
#include "apilib.h"

int api_notification_list(ONION_FUNC_PROTO_STR)
{
	DEFINE_COMMON_SESSION_VARS;

	int rc = api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info);
	if (rc != API_RT_SUCCESSFUL)
		return api_error(p, req, res, rc);

	struct json_object *obj = json_tokener_parse("{\"errcode\": 0, \"notifications\": []}");
	struct json_object *noti_array = json_object_object_get(obj, "notifications");
	NotifyItemList allNotifyItems = parse_notification(ptr_info->userid);
	struct json_object * item = NULL;
	struct NotifyItem * currItem;
	struct boardmem *b;
	for (currItem = (struct NotifyItem *)allNotifyItems; currItem != NULL; currItem = currItem->next) {
		item = json_object_new_object();
		json_object_object_add(item, "board", json_object_new_string(currItem->board));
		json_object_object_add(item, "noti_time", json_object_new_int64(currItem->noti_time));
		json_object_object_add(item, "from_userid", json_object_new_string(currItem->from_userid));
		json_object_object_add(item, "title", json_object_new_string(currItem->title_utf));
		json_object_object_add(item, "type", json_object_new_int(currItem->type));
		b = ythtbbs_cache_Board_get_board_by_name(currItem->board);
		json_object_object_add(item, "secstr", json_object_new_string(b->header.sec1));

		json_object_array_add(noti_array, item);
	}

	free_notification(allNotifyItems);

	api_set_json_header(res);
	onion_response_write0(res, json_object_to_json_string(obj));
	json_object_put(obj);

	return OCS_PROCESSED;
}

int api_notification_del(ONION_FUNC_PROTO_STR)
{
	DEFINE_COMMON_SESSION_VARS;

	int rc = api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info);
	if (rc != API_RT_SUCCESSFUL)
		return api_error(p, req, res, rc);

	const char * type = onion_request_get_query(req, "type");
	const char * board = onion_request_get_query(req, "board");
	const char * aid_str = onion_request_get_query(req, "aid");

	if (type == NULL && (board == NULL || aid_str == NULL)) {
		return api_error(p, req, res, API_RT_WRONGPARAM);
	}

	if ((type != NULL) && (strcasecmp(type, "delall") == 0)) {
		del_all_notification(ptr_info->userid);
	} else {
		del_post_notification(ptr_info->userid, board, atoi(aid_str));
	}

	return api_error(p, req, res, API_RT_SUCCESSFUL);
}

