#include <stdio.h>
#include <json-c/json.h>
#include "ythtbbs/misc.h"
#include "bbslib.h"

int api_loginpics(void) {
	char buf[512];
	struct json_object *obj;

	get_no_more_than_four_login_pics(buf, sizeof(buf));
	obj = json_object_new_object();
	json_object_object_add(obj, "data", json_object_new_string(buf));
	json_header();
	printf("%s", json_object_to_json_string(obj));
	json_object_put(obj);
	return 0;
}

