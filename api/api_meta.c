#include "api.h"
#include "ythtbbs/misc.h"

int api_meta_loginpics(ONION_FUNC_PROTO_STR)
{
	char pics[512];
	get_no_more_than_four_login_pics(pics, sizeof(pics));

	api_set_json_header(res);
	onion_response_printf(res, "{\"errcode\":0, \"pics\":\"%s\"}", pics);

	return OCS_PROCESSED;
}
