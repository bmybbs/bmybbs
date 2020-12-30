#ifndef BMY_WECHAT_H
#define BMY_WECHAT_H

enum bmy_wechat_errcode_e {
	BMY_WECHAT_REQUEST_ERROR                       = -999,    /** curl 调用失败 */
	BMY_WECHAT_RESPONSE_FORMAT_ERROR               = -998,    /** 格式错误 */
	BMY_WECHAT_ERRCODE_WRONG_PARAM                 = -3,
	BMY_WECHAT_ERRCODE_NO_CFGFILE                  = -2,
	BMY_WECHAT_ERRCODE_BUSY                        = -1,
	BMY_WECHAT_ERRCODE_SUCCESS                     = 0,
	BMY_WECHAT_ERRCODE_INVALID_CODE                = 40029,
	BMY_WECHAT_ERRCODE_TOO_FREQUENT                = 45011,
};

/**
 * 对微信 auth.code2Session 返回值的封装，这里忽略 unionid
 */
struct bmy_wechat_session {
	char *openid;
	char *session_key;
};

/**
 * @brief 依据传入的 code，向微信开放平台请求 session 信息
 * @param code 依据 wx.login 获得的代码
 * @param s    用于存放结果的结构体，使用结束调用 bmy_wechat_session_free()
 * @return 错误码，成功返回 BMY_WECHAT_ERRCODE_SUCCESS，其他参见系统日志
 * @see bmy_wechat_session_free
 */
int bmy_wechat_session_get(const char *code, struct bmy_wechat_session *s);
void bmy_wechat_session_free(struct bmy_wechat_session *s);

#endif

