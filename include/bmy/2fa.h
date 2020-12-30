#ifndef BMYBBS_2FA_H
#define BMYBBS_2FA_H
#include <stddef.h>

typedef enum bmy_2fa_status_e {
	BMY_2FA_SUCCESS = 0,
	BMY_2FA_CANNOT_CONNECT_REDIS = 1,
	BMY_2FA_CANNOT_SET_2FA_CODE  = 2,
	BMY_2FA_NOT_EXISTED          = 3,
	BMY_2FA_CANNOT_SET_OPENID    = 4,
} bmy_2fa_status;

/**
 * @brief 创建两步验证的信息
 * 如果创建成功，则一串随机字符被存储在 key 中，该 key 同时作为 redis 的散列的键。
 * 该散列中存放了另一个6位整数用于第二步验证。
 *
 * @param key 存放键的缓冲区
 * @param len 缓冲区长度
 * @return 状态码，成功返回 BMY_2FA_SUCCESS
 */
bmy_2fa_status bmy_2fa_create(char *key, size_t len);

/**
 * @brief 第一步验证
 * 依据 key，存入 auth 作为身份验证，同时返回 code（6位整数）
 *
 * @param key 两步验证的 key
 * @param auth 第二步验证通过得到的身份信息
 * @param code 存放第二步验证需要的验证码，缓冲区最小需要长度 7
 * @param len 缓冲区长度
 * @return 状态码，成功返回 BMY_2FA_SUCCESS
 */
bmy_2fa_status bmy_2fa_get_code(const char *key, const char *auth, char *code, size_t len);

/**
 * @brief 第二步验证
 * 依据 key 和第一步验证获取的 code，获取第一步存入的身份信息
 *
 * @param key 两步验证的 key
 * @param code 验证码
 * @return 如果验证通过，返回 auth 字符串的副本，需要使用 free(3) 释放
 * @warn free(3)
 */
char *bmy_2fa_check_code(const char *key, const char *code);
#endif

