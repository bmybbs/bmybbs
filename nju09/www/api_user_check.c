#include "bbslib.h"

/**
 * @brief 简易的判断 cookie 是否合法的接口
 * 主要的判断过程在 cookie_parse 后存放在全局变量中
 * 这里取用全局变量并返回 json。
 */
int api_user_check() {
	json_header();
	if (loginok) {
		printf("{ \"code\": 0, \"userid\": \"%s\" }", currentuser.userid);
	} else {
		printf("{ \"code\": -1 }");
	}

	return 0;
}

