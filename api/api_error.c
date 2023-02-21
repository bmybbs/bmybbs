#include "api.h"

int api_error(ONION_FUNC_PROTO_STR, enum api_error_code errcode)
{
	(void) p;
	(void) req;
	api_set_json_header(res);
	onion_response_printf(res, "{\"errcode\":%d}", errcode);
	return OCS_PROCESSED;
}
