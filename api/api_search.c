#include <onion/block.h>
#include "bmy/search.h"
#include "api.h"
#include "apilib.h"

struct json_object *convert_search_result_to_json(const struct fileheader_utf *search_result, size_t search_size);

int api_search_content(ONION_FUNC_PROTO_STR) {
	DEFINE_COMMON_SESSION_VARS;
	int rc;
	size_t search_size;

	struct fileheader_utf *search_result;
	struct json_object *req_json, *obj_tmp, *result = NULL;
	const char *body, *board, *query;
	const struct boardmem *bmem;
	const onion_block *block;

	if (!api_check_method(req, OR_POST)) {
		return api_error(p, req, res, API_RT_WRONGMETHOD);
	}

	rc = api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info);
	if (rc != API_RT_SUCCESSFUL) {
		return api_error(p, req, res, rc);
	}

	if ((block = onion_request_get_data(req)) == NULL || (body = onion_block_data(block)) == NULL || body[0] == '\0') {
		return api_error(p, req, res, API_RT_WRONGPARAM);
	}

	if ((req_json = json_tokener_parse(body)) != NULL) {
		if ((obj_tmp = json_object_object_get(req_json, "board")) != NULL) {
			board = json_object_get_string(obj_tmp);
		} else {
			rc = API_RT_WRONGPARAM;
		}

		if (rc == API_RT_SUCCESSFUL) {
			if ((obj_tmp = json_object_object_get(req_json, "query")) != NULL) {
				query = json_object_get_string(obj_tmp);
			} else {
				rc = API_RT_WRONGPARAM;
			}
		}

		if (rc == API_RT_SUCCESSFUL) {
			if ((bmem = ythtbbs_cache_Board_get_board_by_name(board)) != NULL) {
				if (check_user_read_perm_x(ptr_info, bmem)) {
					if ((search_result = bmy_search_board(bmem->header.filename, query, &search_size)) != NULL) {
						result = convert_search_result_to_json(search_result, search_size);
						free(search_result);
					} else {
						rc = API_RT_NOTINRCD;
					}
				} else {
					rc = API_RT_NOSUCHBRD;
				}
			} else {
				rc = API_RT_NOSUCHBRD;
			}
		}
		json_object_put(req_json);
	}

	if (result != NULL) {
		api_set_json_header(res);
		onion_response_write0(res, json_object_to_json_string(result));
		json_object_put(result);
		return OCS_PROCESSED;
	} else {
		return api_error(p, req, res, rc);
	}
}

struct json_object *convert_search_result_to_json(const struct fileheader_utf *search_result, size_t search_size) {
	struct json_object *result = NULL, *array, *obj, *field;
	size_t i;
	const struct fileheader_utf *p;

	if (search_result) {
		if ((result = json_object_new_object()) != NULL) {
			if ((field = json_object_new_int(API_RT_SUCCESSFUL)) != NULL) {
				json_object_object_add(result, "errcode", field);
			}

			if ((array = json_object_new_array_ext(search_size)) != NULL) {
				for (i = 0; i < search_size; i++) {
					p = &search_result[i];

					if ((obj = json_object_new_object()) != NULL) {
						if ((field = json_object_new_string(p->title)) != NULL) {
							json_object_object_add(obj, "title", field);
						}

						if ((field = json_object_new_string(p->owner)) != NULL) {
							json_object_object_add(obj, "owner", field);
						}

						if ((field = json_object_new_int64(p->filetime)) != NULL) {
							json_object_object_add(obj, "aid", field);
						}

						if ((field = json_object_new_int64(p->thread)) != NULL) {
							json_object_object_add(obj, "tid", field);
						}

						json_object_array_add(array, obj);
					}
				}
				json_object_object_add(result, "result", array);
			}
		}
	}
	return result;
}

