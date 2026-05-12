#include "log_parser.h"
#include "log_tokenizer.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "bmy/convcode.h"

static const char STR_ACTION_DIGEST[] = "digest";
static const char STR_ACTION_MARK[] = "mark";
/// 置顶
static const char STR_ACTION_TOP[] = "\xD6\xC3\xB6\xA5";
static const char STR_ACTION_WATER[] = "water";
static const char STR_ACTION_UNDIGEST[] = "undigest";
static const char STR_ACTION_UNMARK[] = "unmark";
/// 去掉置顶
static const char STR_ACTION_UNTOP[] = "\xC8\xA5\xB5\xF4\xD6\xC3\xB6\xA5";
static const char STR_ACTION_UNWATER[] = "unwater";

static const char *const STR_ACTIONS_MARK_LIKE[] = {
	STR_ACTION_MARK,
	STR_ACTION_UNMARK,
	STR_ACTION_DIGEST,
	STR_ACTION_UNDIGEST,
	STR_ACTION_WATER,
	STR_ACTION_UNWATER,
	STR_ACTION_TOP,
	STR_ACTION_UNTOP,
	"edit",
	"del",
	"undel",
	NULL,
};

static const char *const STR_ACTIONS_POST_LIKE[] = {
	"post",
	"check1984",
	"crosspost",
	"sametitle",
	NULL,
};

static const char *get_action_str(const struct bmy_log_token *token, const char *const actions[]);

/**
 * @brief 安全移除分配的空间
 */
static void bmy_log_parser_safe_ptr_cleanup(const void *ptr);
/**
 * @brief 解析时间
 * @param token 时间 token
 * @param line_time 存放解析结果的指针
 * @return 当解析失败的时候，例如格式不正确、数值不正确，返回 false
 */
static bool bmy_log_parser_time(const struct bmy_log_token *token, struct bmy_log_line_time *line_time);
/**
 * @brief 判断字符 x 是否在范围内
 * @param lo 低值（包含）
 * @param hi 高值（包含）
 */
static bool bmy_char_in_range(char x, char lo, char hi);
/**
 * @brief 一个简易版的字符->数字转换
 */
static int bmy_char_to_digit(char x);

static enum bmy_log_parse_status bmy_log_parse_article_mark_like_events(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result);
static enum bmy_log_parse_status bmy_log_parse_article_post_like_events(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result);
/**
 * @brief 解析更新标题的日志
 *
 * @warning 这个解析并不完美，建立在原标题中并不包含 "newtitle:" 的情况
 */
static enum bmy_log_parse_status bmy_log_parse_article_changetitle(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result, const char *raw_line);

static char *bmy_log_token_to_utf8(const struct bmy_log_token *token);

bool bmy_log_parse_line(const char *line, struct bmy_log_parse_result *result) {
	struct bmy_log_tokens tokens;
	const struct bmy_log_token *action_token;
	char *userid;

	if (result == NULL) {
		goto FAILED_0;
	}

	memset(result, 0, sizeof(*result));

	if (line == NULL) {
		result->status = BMY_LOG_PARSE_FAILED;
		result->reason = "null line";
		goto FAILED_0;
	}

	// 仅当这两个指针存在 NULL 的时候才会返回 false
	if (!bmy_log_tokenize(line, &tokens)) {
		result->status = BMY_LOG_PARSE_FAILED;
		result->reason = "NULL parameter";
		goto FAILED_0;
	}

	// 如果长度 < 3 视为不正确的格式：时间 用户名 动作
	if (tokens.count < 3) {
		result->status = BMY_LOG_PARSE_FAILED;
		result->reason = "wrong format";
		goto FAILED_0;
	}

	// 解析时间
	if (bmy_log_token_empty(&tokens.items[0]) || !bmy_log_parser_time(&tokens.items[0], &result->line_time)) {
		result->status = BMY_LOG_PARSE_FAILED;
		result->reason = "wrong format";
		goto FAILED_0;
	}

	// own userid
	if (bmy_log_token_empty(&tokens.items[1]) || (userid = bmy_log_token_dup(&tokens.items[1])) == NULL) {
		result->status = BMY_LOG_PARSE_FAILED;
		result->reason = "wrong format";
		goto FAILED_0;
	}

	action_token = &tokens.items[2];
	if (bmy_log_token_eq_any(action_token, STR_ACTIONS_MARK_LIKE)) {
		if (bmy_log_parse_article_mark_like_events(&tokens, result) != BMY_LOG_PARSE_ACCEPTED) {
			goto FAILED_1;
		}
		result->payload.article.actor_userid = userid;
	} else if (bmy_log_token_eq_any(action_token, STR_ACTIONS_POST_LIKE)) {
		if (bmy_log_parse_article_post_like_events(&tokens, result) != BMY_LOG_PARSE_ACCEPTED) {
			goto FAILED_1;
		}
		result->payload.article.actor_userid = userid;
	} else if (bmy_log_token_eq(action_token, "changetitle")) {
		if (bmy_log_parse_article_changetitle(&tokens, result, line) != BMY_LOG_PARSE_ACCEPTED) {
			goto FAILED_1;
		}
		result->payload.article.actor_userid = userid;
	}

	return true;

FAILED_1:
	free(userid);
FAILED_0:
	return false;
}

void bmy_log_parse_result_cleanup(struct bmy_log_parse_result *result) {
	if (!result) {
		return;
	}

	switch (result->table) {
		case BMY_LOG_EVENT_ARTICLE:
			bmy_log_parser_safe_ptr_cleanup(result->payload.article.actor_userid);
			bmy_log_parser_safe_ptr_cleanup(result->payload.article.board);
			bmy_log_parser_safe_ptr_cleanup(result->payload.article.owner_userid);
			bmy_log_parser_safe_ptr_cleanup(result->payload.article.title);
			bmy_log_parser_safe_ptr_cleanup(result->payload.article.old_title);
		break;
		default:
			// TODO
			(void) result;
	}
}

// Internal helpers
static void bmy_log_parser_safe_ptr_cleanup(const void *ptr) {
	if (ptr) {
		free((void *) ptr);
	}
}

static bool bmy_log_parser_time(const struct bmy_log_token *token, struct bmy_log_line_time *line_time) {
	const char *s = token->ptr;
	if (token->len != 8 /* "00:00:00" */) {
		return false;
	}

	if (
		!bmy_char_in_range(s[0], '0', '2') ||
		!bmy_char_in_range(s[1], '0', '9') ||
		s[2] != ':' ||
		!bmy_char_in_range(s[3], '0', '5') ||
		!bmy_char_in_range(s[4], '0', '9') ||
		s[5] != ':' ||
		!bmy_char_in_range(s[6], '0', '5') ||
		!bmy_char_in_range(s[7], '0', '9')) {
		return false;
	}

	line_time->hour = 10 * bmy_char_to_digit(s[0]) + bmy_char_to_digit(s[1]);
	line_time->minute = 10 * bmy_char_to_digit(s[3]) + bmy_char_to_digit(s[4]);
	line_time->second = 10 * bmy_char_to_digit(s[6]) + bmy_char_to_digit(s[7]);

	return (line_time->hour >= 0 && line_time->hour <= 23) && (line_time->minute >= 0 && line_time->minute <= 59) && (line_time->second >= 0 && line_time->second <= 59);
}

static bool bmy_char_in_range(char x, char lo, char hi) {
	if (lo < hi) {
		return x >= lo && x <= hi;
	} else {
		return x >= hi && x <= lo;
	}
}

static int bmy_char_to_digit(char x) {
	return x - '0';
}

static enum bmy_log_parse_status bmy_log_parse_article_mark_like_events(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result) {
	char *board = NULL;
	char *owner = NULL;
	char *title_utf8 = NULL;
	struct bmy_log_token title_token;
	const struct bmy_log_token *action_token;

	if (raw_tokens->count < 5) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}

	if ((board = bmy_log_token_dup(&raw_tokens->items[3])) == NULL) {
		goto FAILED_0;
	}
	if ((owner = bmy_log_token_dup(&raw_tokens->items[4])) == NULL) {
		goto FAILED_1;
	}

	title_token = bmy_log_token_rest_after(&raw_tokens->items[4]);
	if (bmy_log_token_empty(&title_token)) {
		goto FAILED_1;
	}
	if ((title_utf8 = bmy_log_token_to_utf8(&title_token)) == NULL) {
		goto FAILED_2;
	}

	action_token = &raw_tokens->items[2];
	if (bmy_log_token_eq(action_token, STR_ACTION_TOP)) {
		// NOTE: 定义在数据库中，取缔原 GBK 编码
		result->payload.article.action = "top";
	} else if (bmy_log_token_eq(action_token, STR_ACTION_UNTOP)) {
		// NOTE: 定义在数据库中，取缔原 GBK 编码
		result->payload.article.action = "untop";
	} else {
		// NOTE: 保护性校验
		if ((result->payload.article.action = get_action_str(action_token, STR_ACTIONS_MARK_LIKE)) == NULL) {
			free(title_utf8);
			goto FAILED_2;
		}
	}

	result->table = BMY_LOG_EVENT_ARTICLE;
	result->payload.article.board = board;
	result->payload.article.owner_userid = owner;
	result->payload.article.title = title_utf8;
	return result->status = BMY_LOG_PARSE_ACCEPTED;

FAILED_2:
	free(owner);
FAILED_1:
	free(board);
FAILED_0:
	return result->status = BMY_LOG_PARSE_FAILED;
}

static enum bmy_log_parse_status bmy_log_parse_article_post_like_events(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result) {
	char *board = NULL;
	char *title_utf8 = NULL;
	struct bmy_log_token title_token;
	const struct bmy_log_token *action_token;

	if (raw_tokens->count < 4) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}

	if ((board = bmy_log_token_dup(&raw_tokens->items[3])) == NULL) {
		goto FAILED_0;
	}
	title_token = bmy_log_token_rest_after(&raw_tokens->items[3]);
	if (bmy_log_token_empty(&title_token)) {
		goto FAILED_0;
	}
	if ((title_utf8 = bmy_log_token_to_utf8(&title_token)) == NULL) {
		goto FAILED_1;
	}

	action_token = &raw_tokens->items[2];
	if ((result->payload.article.action = get_action_str(action_token, STR_ACTIONS_POST_LIKE)) == NULL) {
		goto FAILED_2;
	}

	result->table = BMY_LOG_EVENT_ARTICLE;
	result->payload.article.board = board;
	result->payload.article.title = title_utf8;
	return result->status = BMY_LOG_PARSE_ACCEPTED;

FAILED_2:
	free(title_utf8);
FAILED_1:
	free(board);
FAILED_0:
	return result->status = BMY_LOG_PARSE_FAILED;
}

static enum bmy_log_parse_status bmy_log_parse_article_changetitle(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result, const char *raw_line) {
	char *board = NULL;
	char *owner = NULL;
	char *oldtitle_utf8 = NULL;
	char *newtitle_utf8 = NULL;
	struct bmy_log_token oldtitle_token, newtitle_token;
	const char *ptr_oldtitle, *ptr_newtitle;
	const char *prefix_oldtitle, *prefix_newtitle;

	prefix_oldtitle = "oldtitle:";
	prefix_newtitle = " newtitle:";

	if (raw_tokens->count < 7) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}

	if ((ptr_oldtitle = strstr(raw_line, prefix_oldtitle)) == NULL) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}
	if ((ptr_newtitle = strstr(ptr_oldtitle, prefix_newtitle)) == NULL) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}
	oldtitle_token.ptr = ptr_oldtitle + strlen(prefix_oldtitle);
	oldtitle_token.len = ptr_newtitle - oldtitle_token.ptr;
	newtitle_token.ptr = ptr_newtitle + strlen(prefix_newtitle);
	newtitle_token.len = strlen(raw_line) + raw_line - newtitle_token.ptr;

	if ((oldtitle_utf8 = bmy_log_token_to_utf8(&oldtitle_token)) == NULL) {
		goto FAILED_0;
	}
	if ((newtitle_utf8 = bmy_log_token_to_utf8(&newtitle_token)) == NULL) {
		goto FAILED_1;
	}
	if ((board = bmy_log_token_dup(&raw_tokens->items[3])) == NULL) {
		goto FAILED_2;
	}
	if ((owner = bmy_log_token_dup(&raw_tokens->items[4])) == NULL) {
		goto FAILED_3;
	}

	result->table = BMY_LOG_EVENT_ARTICLE;
	// NOTE: 定义在表结构中
	result->payload.article.action = "changetitle";
	result->payload.article.board = board;
	result->payload.article.owner_userid = owner;
	result->payload.article.title = newtitle_utf8;
	result->payload.article.old_title = oldtitle_utf8;

	return result->status = BMY_LOG_PARSE_ACCEPTED;

FAILED_3:
	free(board);
FAILED_2:
	free(newtitle_utf8);
FAILED_1:
	free(oldtitle_utf8);
FAILED_0:
	return result->status = BMY_LOG_PARSE_FAILED;
}

static const char *get_action_str(const struct bmy_log_token *token, const char *const actions[]) {
	size_t i;

	for (i = 0; actions[i] != NULL; i++) {
		if (bmy_log_token_eq(token, actions[i])) {
			return actions[i];
		}
	}

	return NULL;
}

static char *bmy_log_token_to_utf8(const struct bmy_log_token *token) {
	char *str_gbk = NULL;
	char *str_utf8 = NULL;
	size_t len_utf8;

	if (bmy_log_token_empty(token)) {
		return NULL;
	}

	if ((str_gbk = bmy_log_token_dup(token)) == NULL) {
		return NULL;
	}

	len_utf8 = token->len * 2;
	if ((str_utf8 = malloc(len_utf8)) == NULL) {
		free(str_gbk);
		return NULL;
	}

	g2u(str_gbk, token->len, str_utf8, len_utf8);
	free(str_gbk);

	return str_utf8;
}
