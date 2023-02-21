#ifndef __BMYBBS_API_H
#define __BMYBBS_API_H
#include <onion/request.h>
#include <onion/response.h>
#include "error_code.h"

#define ONION_FUNC_PROTO_STR void *p, onion_request *req, onion_response *res

int api_error(ONION_FUNC_PROTO_STR, enum api_error_code errcode);

int api_user_login(ONION_FUNC_PROTO_STR);
int api_user_query(ONION_FUNC_PROTO_STR);
int api_user_logout(ONION_FUNC_PROTO_STR);
int api_user_check_session(ONION_FUNC_PROTO_STR);
int api_user_articlequery(ONION_FUNC_PROTO_STR);
int api_user_friends_list(ONION_FUNC_PROTO_STR);
int api_user_friends_add(ONION_FUNC_PROTO_STR);
int api_user_friends_del(ONION_FUNC_PROTO_STR);
int api_user_rejects_list(ONION_FUNC_PROTO_STR);
int api_user_rejects_add(ONION_FUNC_PROTO_STR);
int api_user_rejects_del(ONION_FUNC_PROTO_STR);
int api_user_autocomplete(ONION_FUNC_PROTO_STR);
int api_oauth_2fa_get_key(ONION_FUNC_PROTO_STR);
int api_oauth_2fa_get_code(ONION_FUNC_PROTO_STR);
int api_oauth_2fa_check_code(ONION_FUNC_PROTO_STR);
int api_oauth_remove_wx(ONION_FUNC_PROTO_STR);
int api_oauth_login(ONION_FUNC_PROTO_STR);

int api_article_list(ONION_FUNC_PROTO_STR);

int api_article_getHTMLContent(ONION_FUNC_PROTO_STR);	// 获取 HTML 格式的内容
int api_article_getRAWContent(ONION_FUNC_PROTO_STR);	// 获取原始内容，'\033' 字符将被转为 "[ESC]" 字符串
int api_article_getContent(ONION_FUNC_PROTO_STR);       // 获取内容，'\033' 字符不做转换，也不处理 html 格式，仅将附件内容进行剥离

int api_article_post(ONION_FUNC_PROTO_STR);				// 发帖接口
int api_article_reply(ONION_FUNC_PROTO_STR);			// 回帖

int api_board_list(ONION_FUNC_PROTO_STR);
int api_board_info(ONION_FUNC_PROTO_STR);
int api_board_fav_add(ONION_FUNC_PROTO_STR);
int api_board_fav_del(ONION_FUNC_PROTO_STR);
int api_board_fav_list(ONION_FUNC_PROTO_STR);			// 收藏夹列表，精简模式
int api_board_autocomplete(ONION_FUNC_PROTO_STR);

int api_mail_list(ONION_FUNC_PROTO_STR);

int api_mail_getHTMLContent(ONION_FUNC_PROTO_STR);
int api_mail_getRAWContent(ONION_FUNC_PROTO_STR);

int api_mail_send(ONION_FUNC_PROTO_STR);
int api_mail_reply(ONION_FUNC_PROTO_STR);

int api_meta_loginpics(ONION_FUNC_PROTO_STR);			// 进站画面

int api_attach_show(ONION_FUNC_PROTO_STR);              // 显示附件
int api_attach_list(ONION_FUNC_PROTO_STR);              // 附件列表
int api_attach_get(ONION_FUNC_PROTO_STR);               // 获取附件
int api_attach_upload(ONION_FUNC_PROTO_STR);            // 上传附件
int api_attach_delete(ONION_FUNC_PROTO_STR);            // 删除附件

int api_notification_list(ONION_FUNC_PROTO_STR);
int api_notification_del(ONION_FUNC_PROTO_STR);

int api_subscription_list(ONION_FUNC_PROTO_STR);

int api_search_content(ONION_FUNC_PROTO_STR);

/**
 * @brief 为 onion_response 添加 json 的 MIME 信息
 * @param res
 * @return
 */
static inline void api_set_json_header(onion_response *res)
{
	onion_response_set_header(res, "Content-Type", "application/json; charset=utf-8");
	onion_response_set_header(res, "access-control-allow-origin", "*");
}

#endif
