#ifndef BMYBBS_COOKIE_H
#define BMYBBS_COOKIE_H
#include <stddef.h>

/**
 * @brief 用于解析 cookie 字段的结构体
 * 配合 bmy_cookie_parse 接口，对 buf 处理后，各个字段指向 buf 内的地址。
 */
struct bmy_cookie {
	const char *userid;    ///< 用户 ID
	const char *sessid;    ///< 会话 ID
	const char *token;     ///< token
	const char *extraparam;
};

/**
 * @brief 解析 cookie
 * @param buf 缓冲区，依据截断字符分割为多段字符串
 * @param cookie 存放解析结果的结构体
 */
void bmy_cookie_parse(char *buf, struct bmy_cookie *cookie);

/**
 * @brief 生成 cookie 并保存在 buf 中
 * @param buf 缓冲区，至少 60 Byte
 * @param len 缓冲区长度
 * @param cookie 携带相关信息的结构体
 * @return buf 中保存的 cookie 的实际长度
 */
int bmy_cookie_gen(char *buf, size_t len, const struct bmy_cookie *cookie);

/**
 * @brief 检查域名
 * @param host 传入域名
 * @return 若域名不合法，返回 NULL
 */
const char *bmy_cookie_check_host(const char *host);
#endif

