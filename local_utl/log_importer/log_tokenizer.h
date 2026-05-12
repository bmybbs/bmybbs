#ifndef BMY_LOG_IMPORTER_LOG_TOKENIZER_H
#define BMY_LOG_IMPORTER_LOG_TOKENIZER_H

#include <stdbool.h>
#include <stddef.h>

#define BMY_LOG_TOKENIZER_MAX_TOKENS 16

struct bmy_log_token {
	/// token 起始指针，指向原内容中的地址，不持有数据
	const char *ptr;
	/// token 的长度
	size_t len;
};

struct bmy_log_tokens {
	/// 解析出的 token
	struct bmy_log_token items[BMY_LOG_TOKENIZER_MAX_TOKENS];
	/// 实际的 token 个数
	size_t count;
	/// 是否有更多未解析的内容
	bool truncated;
};

/**
 * @brief 解析 line 将 token 的起始指针存放在结构体 bmy_log_token 中
 */
bool bmy_log_tokenize(const char *line, struct bmy_log_tokens *tokens);

/**
 * @brief 比较 token 和字符串 text 是否相等
 */
bool bmy_log_token_eq(const struct bmy_log_token *token, const char *text);
/**
 * @brief 比较 token 是否匹配 texts 中的任意一个字符串，texts 以 NULL 结尾
 */
bool bmy_log_token_eq_any(const struct bmy_log_token *token, const char *const texts[]);
/**
 * @brief 判断 token 是否为空
 */
bool bmy_log_token_empty(const struct bmy_log_token *token);

/**
 * @brief 将 token 指向的值释放为实际的字符串，使用完毕需要调用 free
 */
char *bmy_log_token_dup(const struct bmy_log_token *token);

/**
 * @brief 返回某个 token 之后的剩余内容，视为一个 token，不包含结尾的换行符
 */
struct bmy_log_token bmy_log_token_rest_after(const struct bmy_log_token *token);

#endif
