#include <string.h>
#include <onion/request.h>
#include <onion/response.h>
#include "ytht/msg.h"
#include "bmy/2fa.h"
#include "bmy/wechat.h"
#include "bmy/user.h"
#include "ythtbbs/cache.h"
#include "ythtbbs/session.h"
#include "api.h"
#include "apilib.h"

#define BMY_2FA_KEY_SIZE (32 + 1)
static const char *SESSION_2FA_KEY = "TFAKEY";

int api_oauth_2fa_get_key(ONION_FUNC_PROTO_STR) {
	DEFINE_COMMON_SESSION_VARS;
	int rc;
	char key[BMY_2FA_KEY_SIZE], local_buf[128];
	char *old_key = NULL;
	int usernum;
	bmy_2fa_status status;

	if (!api_check_method(req, OR_GET))
		return api_error(p, req, res, API_RT_WRONGMETHOD);

	rc = api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info);
	if (rc != API_RT_SUCCESSFUL)
		return api_error(p, req, res, rc);

	usernum = 1 + ythtbbs_cache_UserIDHashTable_find_idx(ptr_info->userid);
	if (bmy_user_has_openid(usernum)) {
		return api_error(p, req, res, API_RT_HASOPENID);
	}

	old_key = ythtbbs_session_get_value(cookie.sessid, SESSION_2FA_KEY);
	if (old_key && (bmy_2fa_valid(old_key) == BMY_2FA_SUCCESS)) {
		snprintf(local_buf, sizeof(local_buf), "{\"errcode\": 0, \"key\":\"%s\"}", old_key);
		free(old_key);

		api_set_json_header(res);
		onion_response_write0(res, local_buf);
		return OCS_PROCESSED;
	}

	if (old_key)
		free(old_key);

	status = bmy_2fa_create(key, BMY_2FA_KEY_SIZE);
	if (status != BMY_2FA_SUCCESS) {
		snprintf(local_buf, sizeof(local_buf), "generate 2fa failed for %s, status: %d", ptr_info->userid, status);
		newtrace(local_buf);
		return api_error(p, req, res, API_RT_2FA_INTERNAL);
	}

	// 这里 key 的长度是明确的，且是 json 安全的字符，因此就不额外声明缓冲区了，并且直接生成 json
	snprintf(local_buf, sizeof(local_buf), "{\"errcode\": 0, \"key\":\"%s\"}", key);

	api_set_json_header(res);
	onion_response_write0(res, local_buf);

	ythtbbs_session_set_value(cookie.sessid, SESSION_2FA_KEY, key);
	return OCS_PROCESSED;
}

int api_oauth_2fa_get_code(ONION_FUNC_PROTO_STR) {
	if (!api_check_method(req, OR_POST))
		return api_error(p, req, res, API_RT_WRONGMETHOD);

	DEFINE_COMMON_SESSION_VARS;
	if (API_RT_SUCCESSFUL == api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info))
		return api_error(p, req, res, API_RT_HASOPENID);

	const char *wxcode = onion_request_get_query(req, "code");
	const char *tfakey = onion_request_get_query(req, "tfakey");
	char code[7]; // 6位数字

	if (wxcode == NULL || wxcode[0] == '\0' || tfakey == NULL || tfakey[0] == '\0')
		return api_error(p, req, res, API_RT_WRONGPARAM);

	struct bmy_wechat_session wx_session = {
		.openid = NULL,
		.session_key = NULL
	};

	int rc = bmy_wechat_session_get(wxcode, &wx_session);
	if (rc != BMY_WECHAT_ERRCODE_SUCCESS) {
		bmy_wechat_session_free(&wx_session);
		return api_error(p, req, res, API_RT_WXAPIERROR);
	}

	bmy_2fa_status status = bmy_2fa_get_code(tfakey, wx_session.openid, code, sizeof(code));
	bmy_wechat_session_free(&wx_session);

	if (status != BMY_2FA_SUCCESS) {
		return api_error(p, req, res, API_RT_2FA_INTERNAL);
	}

	// openid 拿到了，code 也拿到了，意味着 openid 同时存入了两步验证，这时候返回验证码
	char buf[128];
	snprintf(buf, sizeof(buf), "{\"errcode\":0, \"code\":\"%s\"}", code);

	api_set_json_header(res);
	onion_response_write0(res, buf);
	return OCS_PROCESSED;
}

int api_oauth_2fa_check_code(ONION_FUNC_PROTO_STR) {
	DEFINE_COMMON_SESSION_VARS;
	int rc;
	char *old_key = NULL;
	char *openid = NULL;
	int usernum;
	const char *code = onion_request_get_query(req, "code");

	if (!api_check_method(req, OR_POST)) {
		return api_error(p, req, res, API_RT_WRONGMETHOD);
	}

	if (code == NULL || code[0] == '\0') {
		return api_error(p, req, res, API_RT_WRONGPARAM);
	}

	rc = api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info);
	if (rc != API_RT_SUCCESSFUL)
		return api_error(p, req, res, rc);

	usernum = 1 + ythtbbs_cache_UserIDHashTable_find_idx(ptr_info->userid);
	if (bmy_user_has_openid(usernum)) {
		return api_error(p, req, res, API_RT_HASOPENID);
	}

	old_key = ythtbbs_session_get_value(cookie.sessid, SESSION_2FA_KEY);
	if (old_key == NULL || (bmy_2fa_valid(old_key) != BMY_2FA_SUCCESS)) {
		if (old_key)
			free(old_key);

		return api_error(p, req, res, API_RT_2FA_INVALID);
	}

	openid = bmy_2fa_check_code(old_key, code);
	if (openid == NULL) {
		free(old_key);
		return api_error(p, req, res, API_RT_2FA_INTERNAL);
	}

	bmy_user_associate_openid(usernum, openid);

	ythtbbs_session_clear_key(cookie.sessid, SESSION_2FA_KEY);
	free(old_key);
	free(openid);

	return api_error(p, req, res, API_RT_SUCCESSFUL);
}

int api_oauth_remove_wx(ONION_FUNC_PROTO_STR) {
	DEFINE_COMMON_SESSION_VARS;
	int usernum;
	int rc;

	if (!api_check_method(req, OR_POST)) {
		return api_error(p, req, res, API_RT_WRONGMETHOD);
	}

	rc = api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info);
	if (rc != API_RT_SUCCESSFUL) {
		return api_error(p, req, res, rc);
	}

	usernum = 1 + ythtbbs_cache_UserIDHashTable_find_idx(ptr_info->userid);
	if (!bmy_user_has_openid(usernum)) {
		return api_error(p, req, res, API_RT_NOOPENID);
	}

	bmy_user_dissociate_openid(usernum);
	return api_error(p, req, res, API_RT_SUCCESSFUL);
}

int api_oauth_login(ONION_FUNC_PROTO_STR) {
	DEFINE_COMMON_SESSION_VARS;
	int rc;
	int usernum;
	const char *code = onion_request_get_query(req, "code");
	const char *fromhost = onion_request_get_header(req, "X-Real-IP");

	if (!api_check_method(req, OR_POST))
		return api_error(p, req, res, API_RT_WRONGMETHOD);

	rc = api_check_session(req, cookie_buf, sizeof(cookie_buf), &cookie, &utmp_idx, &ptr_info);
	if (rc == API_RT_SUCCESSFUL) {
		// 已登录
		return api_error(p, req, res, API_RT_SUCCESSFUL);
	}

	if (code == NULL || code[0] == '\0') {
		return api_error(p, req, res, API_RT_WRONGPARAM);
	}

	struct bmy_wechat_session wx_session = {
		.openid = NULL,
		.session_key = NULL
	};

	rc = bmy_wechat_session_get(code, &wx_session);
	if (rc != BMY_WECHAT_ERRCODE_SUCCESS) {
		bmy_wechat_session_free(&wx_session);
		return api_error(p, req, res, API_RT_WXAPIERROR);
	}

	usernum = bmy_user_getusernum_by_openid(wx_session.openid);
	bmy_wechat_session_free(&wx_session);

	if (usernum == 0) {
		return api_error(p, req, res, API_RT_NOOPENID);
	}

	// 验证通过，登录
	struct user_info ui;
	struct userec ue;
	char userid[IDLEN + 1];
	memset(userid, 0, sizeof(userid));
	ythtbbs_cache_UserTable_resolve();
	ythtbbs_cache_UserTable_getuserid(usernum, userid, sizeof(userid));
	rc = ythtbbs_user_login(userid, NULL, fromhost, YTHTBBS_LOGIN_OAUTH, &ui, &ue, &utmp_idx);
	if (rc != YTHTBBS_USER_LOGIN_OK) {
		return api_error(p, req, res, rc); // 暂时偷懒不定义对应的错误码了
	}

	cookie.userid = ui.userid;
	cookie.sessid = ui.sessionid;
	cookie.token  = ui.token;
	bmy_cookie_gen(cookie_buf, sizeof(cookie_buf), &cookie);

	api_set_json_header(res);
	onion_response_add_cookie(res, SMAGIC, cookie_buf, -1, NULL, NULL, 0);
	onion_response_write0(res, "{\"errcode\": 0}");

	return OCS_PROCESSED;
}

