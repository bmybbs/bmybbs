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

static const char *const STR_ACTIONS_ANNOUNCEMENT[] = {
	"paste",
	"moveitem",
	"additem",
	"import",
	NULL,
};

static const char *get_action_str(const struct bmy_log_token *token, const char *const actions[]);

/**
 * @brief 安全移除分配的空间
 */
static void bmy_log_parser_safe_ptr_cleanup(const char **ptr);
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
static enum bmy_log_parse_status bmy_log_parse_ranged(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result);
static enum bmy_log_parse_status bmy_log_parse_board_usage(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result);
static enum bmy_log_parse_status bmy_log_parse_session_duration(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result);
static enum bmy_log_parse_status bmy_log_parse_login_failure(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result);
static enum bmy_log_parse_status bmy_log_parse_security_event(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result);
static enum bmy_log_parse_status bmy_log_parse_session_login_success(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result);
static enum bmy_log_parse_status bmy_log_parse_session_cleanup(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result);
static enum bmy_log_parse_status bmy_log_parse_session_kick(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result);
static enum bmy_log_parse_status bmy_log_parse_account(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result);
static enum bmy_log_parse_status bmy_log_parse_mail(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result);
static enum bmy_log_parse_status bmy_log_parse_user_interaction(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result);
static enum bmy_log_parse_status bmy_log_parse_user_query(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result);
static enum bmy_log_parse_status bmy_log_parse_announcement(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result);
static enum bmy_log_parse_status bmy_log_parse_board_deny(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result);
static enum bmy_log_parse_status bmy_log_parse_rest(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result);

static bool bmy_log_parser_is_ip_address(const char *text);
static struct bmy_log_token bmy_log_parser_article_title_after(const struct bmy_log_token *token);
static struct bmy_log_token bmy_log_parser_text_after_empty_field(const struct bmy_log_token *token);
static bool bmy_log_parser_field_is_empty(const struct bmy_log_token *previous, const struct bmy_log_token *next);
static struct bmy_log_token bmy_log_parser_text_between_tokens(const struct bmy_log_token *begin, const struct bmy_log_token *end);
static char *bmy_log_token_to_utf8(const struct bmy_log_token *token);
static bool bmy_log_token_to_int(const struct bmy_log_token *token, int *x);
static bool bmy_log_token_to_signed_int(const struct bmy_log_token *token, int *x);
static bool bmy_log_token_to_long(const struct bmy_log_token *token, long *x);
static bool bmy_log_tokens_eq_at_idx(const struct bmy_log_tokens *tokens, size_t idx, const char *s);

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
	} else if (bmy_log_token_eq(action_token, "ranged") || bmy_log_token_eq(action_token, "rangedmail")) {
		if (bmy_log_parse_ranged(&tokens, result) != BMY_LOG_PARSE_ACCEPTED) {
			goto FAILED_1;
		}

		result->payload.range_delete.userid = userid;
	} else if (bmy_log_token_eq(action_token, "use")) {
		if (bmy_log_parse_board_usage(&tokens, result) != BMY_LOG_PARSE_ACCEPTED) {
			goto FAILED_1;
		}

		result->payload.board_usage.userid = userid;
	} else if (bmy_log_token_eq(action_token, "exitbbs")) {
		if (bmy_log_parse_session_duration(&tokens, result) != BMY_LOG_PARSE_ACCEPTED) {
			goto FAILED_1;
		}

		result->payload.session_duration.userid = userid;
	} else if (bmy_log_token_eq(action_token, "passerr")) {
		if (bmy_log_parse_login_failure(&tokens, result) != BMY_LOG_PARSE_ACCEPTED) {
			goto FAILED_1;
		}

		free(userid);
	} else if (bmy_log_token_eq(action_token, "nju09")) {
		if (bmy_log_parse_security_event(&tokens, result) != BMY_LOG_PARSE_ACCEPTED) {
			goto FAILED_1;
		}

		free(userid);
	} else if (bmy_log_token_eq(action_token, "enter")) {
		if (bmy_log_parse_session_login_success(&tokens, result) != BMY_LOG_PARSE_ACCEPTED) {
			goto FAILED_1;
		}
		result->payload.session.userid = userid;
	} else if (bmy_log_token_eq(action_token, "drop")) {
		if (bmy_log_parse_session_cleanup(&tokens, result) == BMY_LOG_PARSE_ACCEPTED) {
			result->payload.session.userid = userid;
		} else if (bmy_log_parse_session_duration(&tokens, result) == BMY_LOG_PARSE_ACCEPTED) {
			result->payload.session_duration.userid = userid;
		} else {
			goto FAILED_1;
		}
	} else if (bmy_log_token_eq(action_token, "kick")) {
		if (bmy_log_parse_session_kick(&tokens, result) != BMY_LOG_PARSE_ACCEPTED) {
			goto FAILED_1;
		}
		result->payload.session.userid = userid;
	} else if (bmy_log_token_eq(action_token, "newaccount") || (bmy_log_token_eq(action_token, "kill"))) {
		if (bmy_log_parse_account(&tokens, result) != BMY_LOG_PARSE_ACCEPTED) {
			goto FAILED_1;
		}

		if (bmy_log_token_eq(action_token, "kill")) {
			free(userid);
		} else {
			result->payload.account.userid = userid;
		}
	} else if (bmy_log_token_eq(action_token, "mail") || bmy_log_token_eq(action_token, "netmail")) {
		if (bmy_log_parse_mail(&tokens, result) != BMY_LOG_PARSE_ACCEPTED) {
			goto FAILED_1;
		}

		result->payload.mail.sender = userid;
	} else if (bmy_log_token_eq(action_token, "talk") || bmy_log_token_eq(action_token, "sendgoodwish")) {
		if (bmy_log_parse_user_interaction(&tokens, result) != BMY_LOG_PARSE_ACCEPTED) {
			goto FAILED_1;
		}

		result->payload.user_interaction.userid = userid;
	} else if (bmy_log_token_eq(action_token, "finddf")) {
		if (bmy_log_parse_user_query(&tokens, result) != BMY_LOG_PARSE_ACCEPTED) {
			goto FAILED_1;
		}

		result->payload.user_query.userid = userid;
	} else if (bmy_log_token_eq_any(action_token, STR_ACTIONS_ANNOUNCEMENT)) {
		if (bmy_log_parse_announcement(&tokens, result) != BMY_LOG_PARSE_ACCEPTED) {
			goto FAILED_1;
		}

		result->payload.announcement.userid = userid;
	} else if (bmy_log_token_eq(action_token, "deny")) {
		if (bmy_log_parse_board_deny(&tokens, result) != BMY_LOG_PARSE_ACCEPTED) {
			goto FAILED_1;
		}

		result->payload.board_deny.operator_userid = userid;
	} else {
		if (bmy_log_parse_rest(&tokens, result) != BMY_LOG_PARSE_DISCARDED) {
			goto FAILED_1;
		}

		free(userid);
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
			bmy_log_parser_safe_ptr_cleanup(&result->payload.article.actor_userid);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.article.board);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.article.owner_userid);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.article.title);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.article.old_title);
		break;
		case BMY_LOG_EVENT_RANGE_DELETE:
			bmy_log_parser_safe_ptr_cleanup(&result->payload.range_delete.userid);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.range_delete.board);
			break;
		case BMY_LOG_EVENT_BOARD_USAGE:
			bmy_log_parser_safe_ptr_cleanup(&result->payload.board_usage.board);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.board_usage.userid);
			break;
		case BMY_LOG_EVENT_SESSION_DURATION:
			bmy_log_parser_safe_ptr_cleanup(&result->payload.session_duration.userid);
			break;
		case BMY_LOG_EVENT_LOGIN_FAILURE:
			bmy_log_parser_safe_ptr_cleanup(&result->payload.login_failure.from_host);
			break;
		case BMY_LOG_EVENT_SECURITY:
			bmy_log_parser_safe_ptr_cleanup(&result->payload.security.input_value);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.security.from_host);
			break;
		case BMY_LOG_EVENT_SESSION:
			bmy_log_parser_safe_ptr_cleanup(&result->payload.session.userid);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.session.from_host);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.session.login_type);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.session.target_userid);
		break;
		case BMY_LOG_EVENT_ACCOUNT:
			bmy_log_parser_safe_ptr_cleanup(&result->payload.account.userid);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.account.from_host);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.account.login_type);
			break;
		case BMY_LOG_EVENT_MAIL:
			bmy_log_parser_safe_ptr_cleanup(&result->payload.mail.sender);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.mail.target_userid);
			break;
		case BMY_LOG_EVENT_USER_INTERACTION:
			bmy_log_parser_safe_ptr_cleanup(&result->payload.user_interaction.userid);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.user_interaction.target_userid);
			break;
		case BMY_LOG_EVENT_USER_QUERY:
			bmy_log_parser_safe_ptr_cleanup(&result->payload.user_query.userid);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.user_query.target);
			break;
		case BMY_LOG_EVENT_ANNOUNCEMENT:
			bmy_log_parser_safe_ptr_cleanup(&result->payload.announcement.userid);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.announcement.board);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.announcement.path);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.announcement.title);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.announcement.owner_userid);
			break;
		case BMY_LOG_EVENT_BOARD_DENY:
			bmy_log_parser_safe_ptr_cleanup(&result->payload.board_deny.board);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.board_deny.operator_userid);
			bmy_log_parser_safe_ptr_cleanup(&result->payload.board_deny.target_userid);
			break;
	}
}

// Internal helpers
static void bmy_log_parser_safe_ptr_cleanup(const char **ptr) {
	if (ptr && *ptr) {
		free((void *) *ptr);
		*ptr = NULL;
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

	if (bmy_log_parser_field_is_empty(&raw_tokens->items[3], &raw_tokens->items[4])) {
		title_token = bmy_log_parser_text_after_empty_field(&raw_tokens->items[3]);
	} else {
		if ((owner = bmy_log_token_dup(&raw_tokens->items[4])) == NULL) {
			goto FAILED_1;
		}
		title_token = bmy_log_parser_article_title_after(&raw_tokens->items[4]);
	}
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

	action_token = &raw_tokens->items[2];
	if ((result->payload.article.action = get_action_str(action_token, STR_ACTIONS_POST_LIKE)) == NULL) {
		goto FAILED_1;
	}

	title_token = bmy_log_parser_article_title_after(&raw_tokens->items[3]);
	if (bmy_log_token_empty(&title_token)) {
		if (!bmy_log_token_eq(action_token, "sametitle")) {
			goto FAILED_1;
		}
	} else if ((title_utf8 = bmy_log_token_to_utf8(&title_token)) == NULL) {
		goto FAILED_1;
	}

	result->table = BMY_LOG_EVENT_ARTICLE;
	result->payload.article.board = board;
	result->payload.article.title = title_utf8;
	return result->status = BMY_LOG_PARSE_ACCEPTED;

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

	if (raw_tokens->count < 6) {
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
	if (raw_tokens->items[4].ptr != ptr_oldtitle) {
		if ((owner = bmy_log_token_dup(&raw_tokens->items[4])) == NULL) {
			goto FAILED_3;
		}
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

static enum bmy_log_parse_status bmy_log_parse_ranged(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result) {
	int from_id = 0, to_id = 0;
	char *board = NULL;
	const struct bmy_log_token *action_token = &raw_tokens->items[2];

	if (bmy_log_token_eq(action_token, "ranged")) {
		if (raw_tokens->count != 6) {
			goto FAILED_0;
		}
		if (!bmy_log_token_to_int(&raw_tokens->items[4], &from_id)) {
			goto FAILED_0;
		}
		if (!bmy_log_token_to_signed_int(&raw_tokens->items[5], &to_id)) {
			goto FAILED_0;
		}
		if ((board = bmy_log_token_dup(&raw_tokens->items[3])) == NULL) {
			return result->status = BMY_LOG_PARSE_FAILED;
		}

		result->table = BMY_LOG_EVENT_RANGE_DELETE;
		// NOTE: 定义在表结构中
		result->payload.range_delete.scope = "article";
		result->payload.range_delete.board = board;
		result->payload.range_delete.from_id = from_id;
		result->payload.range_delete.to_id = to_id;
	} else if (bmy_log_token_eq(action_token, "rangedmail")) {
		if (raw_tokens->count != 5) {
			goto FAILED_0;
		}
		if (!bmy_log_token_to_int(&raw_tokens->items[3], &from_id)) {
			goto FAILED_0;
		}
		if (!bmy_log_token_to_signed_int(&raw_tokens->items[4], &to_id)) {
			goto FAILED_0;
		}

		result->table = BMY_LOG_EVENT_RANGE_DELETE;
		// NOTE: 定义在表结构中
		result->payload.range_delete.scope = "mail";
		result->payload.range_delete.from_id = from_id;
		result->payload.range_delete.to_id = to_id;
	} else {
		goto FAILED_0;
	}

	return result->status = BMY_LOG_PARSE_ACCEPTED;
FAILED_0:
	return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
}

static enum bmy_log_parse_status bmy_log_parse_board_usage(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result) {
	char *board = NULL;
	long stay_seconds = 0;

	if (raw_tokens->count != 5 || !bmy_log_token_to_long(&raw_tokens->items[4], &stay_seconds)) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}
	if ((board = bmy_log_token_dup(&raw_tokens->items[3])) == NULL) {
		return result->status = BMY_LOG_PARSE_FAILED;
	}

	result->table = BMY_LOG_EVENT_BOARD_USAGE;
	result->payload.board_usage.board = board;
	result->payload.board_usage.stay_seconds = stay_seconds;
	return result->status = BMY_LOG_PARSE_ACCEPTED;
}

static enum bmy_log_parse_status bmy_log_parse_session_duration(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result) {
	long stay_seconds = 0;
	bool legacy_www_disconnect;

	legacy_www_disconnect =
		raw_tokens->count == 5 &&
		bmy_log_token_eq(&raw_tokens->items[2], "drop") &&
		bmy_log_token_eq(&raw_tokens->items[4], "www");

	if ((raw_tokens->count != 4 && !legacy_www_disconnect) ||
		!bmy_log_token_to_long(&raw_tokens->items[3], &stay_seconds)) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}

	result->table = BMY_LOG_EVENT_SESSION_DURATION;
	result->payload.session_duration.action = bmy_log_token_eq(&raw_tokens->items[2], "exitbbs") ? "logout" : "disconnect";
	result->payload.session_duration.stay_seconds = stay_seconds;
	return result->status = BMY_LOG_PARSE_ACCEPTED;
}

static enum bmy_log_parse_status bmy_log_parse_login_failure(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result) {
	char *from_host = NULL;

	if (raw_tokens->count != 4 || !bmy_log_token_eq(&raw_tokens->items[1], "system")) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}
	if ((from_host = bmy_log_token_dup(&raw_tokens->items[3])) == NULL) {
		return result->status = BMY_LOG_PARSE_FAILED;
	}
	if (!bmy_log_parser_is_ip_address(from_host)) {
		free(from_host);
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}

	result->table = BMY_LOG_EVENT_LOGIN_FAILURE;
	result->payload.login_failure.from_host = from_host;
	return result->status = BMY_LOG_PARSE_ACCEPTED;
}

static enum bmy_log_parse_status bmy_log_parse_security_event(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result) {
	const struct bmy_log_token *action_token = NULL;
	const struct bmy_log_token *sub_action_token = NULL;
	const struct bmy_log_token *input_start_token = NULL;
	const struct bmy_log_token *from_token = NULL;
	const struct bmy_log_token *ip_colon_token = NULL;
	const struct bmy_log_token *ip_token = NULL;
	struct bmy_log_token input_token = { 0 };
	struct bmy_log_token from_host_token = { 0 };
	char *input_value = NULL;
	char *from_host = NULL;
	const char *action = NULL;

	if (!bmy_log_token_eq(&raw_tokens->items[1], "bot") || raw_tokens->count < 7) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}

	action_token = &raw_tokens->items[3];
	sub_action_token = &raw_tokens->items[4];
	ip_token = &raw_tokens->items[raw_tokens->count - 1];
	ip_colon_token = &raw_tokens->items[raw_tokens->count - 2];
	from_token = &raw_tokens->items[raw_tokens->count - 3];

	if (!bmy_log_token_eq(from_token, "from") || !bmy_log_token_eq(ip_colon_token, "ip:")) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}

	// handle userid
	if (bmy_log_token_eq(action_token, "login") || bmy_log_token_eq(action_token, "register")) {
		input_start_token = &raw_tokens->items[4];
	} else if ((bmy_log_token_eq(action_token, "reset") && bmy_log_token_eq(sub_action_token, "pass")) || (bmy_log_token_eq(action_token, "query") && bmy_log_token_eq(sub_action_token, "email"))) {
		input_start_token = &raw_tokens->items[5];
	} else {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}

	input_token = bmy_log_parser_text_between_tokens(input_start_token, from_token);
	if (!bmy_log_token_empty(&input_token)) {
		if ((input_value = bmy_log_token_dup(&input_token)) == NULL) {
			goto FAILED_0;
		}
	}

	// handle from_host
	if (ip_token->ptr[ip_token->len - 1] == '*') {
		from_host_token.ptr = ip_token->ptr;
		from_host_token.len = ip_token->len - 1;
	} else {
		from_host_token = *ip_token;
	}
	if ((from_host = bmy_log_token_dup(&from_host_token)) == NULL) {
		goto FAILED_1;
	}
	if (!bmy_log_parser_is_ip_address(from_host)) {
		goto FAILED_2;
	}

	if (bmy_log_token_eq(&raw_tokens->items[3], "login")) {
		action = "bot_login";
	} else if (bmy_log_token_eq(&raw_tokens->items[3], "register")) {
		action = "bot_register";
	} else if (bmy_log_token_eq(&raw_tokens->items[3], "query")) {
		action = "bot_query";
	} else {
		action = "bot_reset";
	}
	result->table = BMY_LOG_EVENT_SECURITY;
	result->payload.security.action = action;
	result->payload.security.input_value = input_value;
	result->payload.security.from_host = from_host;
	return result->status = BMY_LOG_PARSE_ACCEPTED;

FAILED_2:
	free(from_host);
FAILED_1:
	free(input_value);
FAILED_0:
	return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
}

static enum bmy_log_parse_status bmy_log_parse_session_login_success(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result) {
	char *from_host = NULL, *login_type = NULL;

	if (raw_tokens->count != 4 /* w/o using */ &&
		raw_tokens->count != 5 /* legacy www */ &&
		raw_tokens->count != 6 /* w/ using */) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}

	if ((from_host = bmy_log_token_dup(&raw_tokens->items[3])) == NULL) {
		goto FAILED_0;
	}
	if (!bmy_log_parser_is_ip_address(from_host)) {
		goto FAILED_1;
	}

	if (raw_tokens->count == 5) {
		if (!bmy_log_token_eq(&raw_tokens->items[4], "www")) {
			free(from_host);
			return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
		}
		if ((login_type = strdup("NJU09")) == NULL) {
			goto FAILED_1;
		}
	} else if (raw_tokens->count == 6) {
		if (!bmy_log_token_eq(&raw_tokens->items[4], "using")) {
			free(from_host);
			return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
		}
		if ((login_type = bmy_log_token_dup(&raw_tokens->items[5])) == NULL) {
			goto FAILED_1;
		}
	}

	result->table = BMY_LOG_EVENT_SESSION;
	// NOTE: 常量，定义在表 `log_session_events` 中
	result->payload.session.action = "login_success";
	result->payload.session.from_host = from_host;
	result->payload.session.login_type = login_type;

	return result->status = BMY_LOG_PARSE_ACCEPTED;
FAILED_1:
	free(from_host);
FAILED_0:
	return result->status = BMY_LOG_PARSE_FAILED;
}

static bool bmy_log_parser_is_ip_address(const char *text) {
	struct in_addr ipv4;
	struct in6_addr ipv6;

	return inet_pton(AF_INET, text, &ipv4) == 1 || inet_pton(AF_INET6, text, &ipv6) == 1;
}

static struct bmy_log_token bmy_log_parser_article_title_after(const struct bmy_log_token *token) {
	struct bmy_log_token title = {0};
	const char *cursor;

	if (token == NULL || token->ptr == NULL)
		return title;

	cursor = token->ptr + token->len;
	if (*cursor != ' ')
		return title;

	title.ptr = cursor + 1;
	title.len = strlen(title.ptr);
	while (title.len > 0 &&
		(title.ptr[title.len - 1] == '\n' || title.ptr[title.len - 1] == '\r')) {
		title.len--;
	}

	return title;
}

static struct bmy_log_token bmy_log_parser_text_after_empty_field(const struct bmy_log_token *token) {
	struct bmy_log_token text = bmy_log_parser_article_title_after(token);

	if (text.len > 0 && text.ptr[0] == ' ') {
		text.ptr++;
		text.len--;
	}

	return text;
}

static bool bmy_log_parser_field_is_empty(const struct bmy_log_token *previous, const struct bmy_log_token *next) {
	if (previous == NULL || previous->ptr == NULL || next == NULL || next->ptr == NULL) {
		return false;
	}

	return next->ptr > previous->ptr + previous->len + 1;
}

static struct bmy_log_token bmy_log_parser_text_between_tokens(const struct bmy_log_token *begin, const struct bmy_log_token *end) {
	struct bmy_log_token text = {0};
	const char *ptr;
	const char *limit;

	if (begin == NULL || begin->ptr == NULL || end == NULL || end->ptr == NULL || begin->ptr > end->ptr) {
		return text;
	}

	ptr = begin->ptr;
	limit = end->ptr;
	while (ptr < limit && *ptr == ' ') {
		ptr++;
	}
	while (limit > ptr && limit[-1] == ' ') {
		limit--;
	}

	text.ptr = ptr;
	text.len = (size_t)(limit - ptr);
	return text;
}

static enum bmy_log_parse_status bmy_log_parse_session_cleanup(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result) {
	const struct bmy_log_token *ending_token;

	if (raw_tokens->count != 4) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}
	ending_token = &raw_tokens->items[3];
	if (!bmy_log_token_eq(ending_token, "www") && !bmy_log_token_eq(ending_token, "www/api")) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}

	result->table = BMY_LOG_EVENT_SESSION;
	// NOTE: 常量，定义在表 `log_session_events` 中
	result->payload.session.action = "session_cleanup";
	return result->status = BMY_LOG_PARSE_ACCEPTED;
}

static enum bmy_log_parse_status bmy_log_parse_session_kick(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result) {
	if (raw_tokens->count == 5 && bmy_log_token_eq(&raw_tokens->items[4], "multi-login")) {
		result->table = BMY_LOG_EVENT_SESSION;
		// NOTE: 常量，定义在表 `log_session_events` 中
		result->payload.session.action = "multi_login_kick";

		return result->status = BMY_LOG_PARSE_ACCEPTED;
	} else if (raw_tokens->count == 4) {
		if ((result->payload.session.target_userid = bmy_log_token_dup(&raw_tokens->items[3])) == NULL) {
			return result->status = BMY_LOG_PARSE_FAILED;
		}

		result->table = BMY_LOG_EVENT_SESSION;
		// NOTE: 常量，定义在表 `log_session_events` 中
		result->payload.session.action = "user_kick";
		return result->status = BMY_LOG_PARSE_ACCEPTED;
	} else {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}
}

static enum bmy_log_parse_status bmy_log_parse_account(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result) {
	char *from_host = NULL;
	char *login_type = NULL;
	char *userid = NULL;
	int user_index_value = 0;
	int life_value = 0;
	const struct bmy_log_token *action_token = &raw_tokens->items[2];

	if (bmy_log_token_eq(action_token, "newaccount")) {
		if (raw_tokens->count != 6 && raw_tokens->count != 5) {
			return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
		}
		if (!bmy_log_token_to_signed_int(&raw_tokens->items[3], &user_index_value)) {
			return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
		}
		if ((from_host = bmy_log_token_dup(&raw_tokens->items[4])) == NULL) {
			return result->status = BMY_LOG_PARSE_FAILED;
		}

		if (!bmy_log_parser_is_ip_address(from_host)) {
			free(from_host);
			return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
		}
		if (raw_tokens->count == 6) {
			if (!bmy_log_token_eq(&raw_tokens->items[5], "www")) {
				free(from_host);
				return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
			}
			if ((login_type = strdup("NJU09")) == NULL) {
				free(from_host);
				return result->status = BMY_LOG_PARSE_FAILED;
			}
		}

		result->table = BMY_LOG_EVENT_ACCOUNT;
		// NOTE: 定义在表结构中
		result->payload.account.action = "create";
		result->payload.account.user_index_value = user_index_value;
		result->payload.account.from_host = from_host;
		result->payload.account.login_type = login_type;
		return result->status = BMY_LOG_PARSE_ACCEPTED;
	} else {
		if (raw_tokens->count != 5) {
			return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
		}
		if (!bmy_log_token_to_signed_int(&raw_tokens->items[4], &life_value) || life_value >= 0) {
			return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
		}
		if ((userid = bmy_log_token_dup(&raw_tokens->items[3])) == NULL) {
			return result->status = BMY_LOG_PARSE_FAILED;
		}
		result->table = BMY_LOG_EVENT_ACCOUNT;
		// NOTE: 定义在表结构中
		result->payload.account.action = "expire_cleanup";
		result->payload.account.userid = userid;
		result->payload.account.life_value = life_value;
		return result->status = BMY_LOG_PARSE_ACCEPTED;
	}
};

static enum bmy_log_parse_status bmy_log_parse_mail(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result) {
	char *target_userid = NULL;

	if (raw_tokens->count != 4) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}
	if ((target_userid = bmy_log_token_dup(&raw_tokens->items[3])) == NULL) {
		return result->status = BMY_LOG_PARSE_FAILED;
	}

	result->table = BMY_LOG_EVENT_MAIL;
	result->payload.mail.target_userid = target_userid;
	return result->status = BMY_LOG_PARSE_ACCEPTED;
}

static enum bmy_log_parse_status bmy_log_parse_user_interaction(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result) {
	char *target_user = NULL;

	if (raw_tokens->count != 4) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}
	if ((target_user = bmy_log_token_dup(&raw_tokens->items[3])) == NULL) {
		return result->status = BMY_LOG_PARSE_FAILED;
	}

	result->table = BMY_LOG_EVENT_USER_INTERACTION;
	result->payload.user_interaction.action = bmy_log_token_eq(&raw_tokens->items[2], "talk") ? "talk" : "goodwish";
	result->payload.user_interaction.target_userid = target_user;
	return result->status = BMY_LOG_PARSE_ACCEPTED;
}

static enum bmy_log_parse_status bmy_log_parse_user_query(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result) {
	char *target = NULL;
	int count = 0;

	if (raw_tokens->count != 5) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}
	if (!bmy_log_token_to_int(&raw_tokens->items[4], &count)) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}
	if ((target = bmy_log_token_dup(&raw_tokens->items[3])) == NULL) {
		return result->status = BMY_LOG_PARSE_FAILED;
	}

	result->table = BMY_LOG_EVENT_USER_QUERY;
	result->payload.user_query.target = target;
	result->payload.user_query.action = "finddf";
	result->payload.user_query.day_count = count;

	return result->status = BMY_LOG_PARSE_ACCEPTED;
}

static enum bmy_log_parse_status bmy_log_parse_announcement(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result) {
	const char *action;
	char *str1 = NULL, *str2 = NULL, *str3 = NULL;
	struct bmy_log_token rest;

	action = get_action_str(&raw_tokens->items[2], STR_ACTIONS_ANNOUNCEMENT);

	if (!strcmp(action, "import")) {
		if (raw_tokens->count < 5) {
			return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
		}
		if ((str1 = bmy_log_token_dup(&raw_tokens->items[3])) == NULL) {
			goto FAILED_0;
		}
		if (bmy_log_parser_field_is_empty(&raw_tokens->items[3], &raw_tokens->items[4])) {
			rest = bmy_log_parser_text_after_empty_field(&raw_tokens->items[3]);
		} else {
			if ((str2 = bmy_log_token_dup(&raw_tokens->items[4])) == NULL) {
				goto FAILED_1;
			}
			rest = bmy_log_parser_article_title_after(&raw_tokens->items[4]);
		}
		if ((str3 = bmy_log_token_to_utf8(&rest)) == NULL) {
			goto FAILED_2;
		}

		result->payload.announcement.board = str1;
		result->payload.announcement.owner_userid = str2;
		result->payload.announcement.title = str3;
	} else {
		if (raw_tokens->count < 5) {
			return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
		}
		if ((str1 = bmy_log_token_dup(&raw_tokens->items[3])) == NULL) {
			goto FAILED_0;
		}
		rest = bmy_log_token_rest_after(&raw_tokens->items[3]);
		if ((str2 = bmy_log_token_to_utf8(&rest)) == NULL) {
			goto FAILED_2;
		}
		result->payload.announcement.board = str1;
		result->payload.announcement.path = str2;
	}

	result->table = BMY_LOG_EVENT_ANNOUNCEMENT;
	result->payload.announcement.action = action;

	return result->status = BMY_LOG_PARSE_ACCEPTED;
FAILED_2:
	free(str2);
FAILED_1:
	free(str1);
FAILED_0:
	return result->status = BMY_LOG_PARSE_FAILED;
}

static enum bmy_log_parse_status bmy_log_parse_board_deny(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result) {
	char *board = NULL;
	char *target_userid = NULL;

	if (raw_tokens->count != 5) {
		return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
	}

	if ((board = bmy_log_token_dup(&raw_tokens->items[3])) == NULL) {
		goto FAILED_0;
	}
	if ((target_userid = bmy_log_token_dup(&raw_tokens->items[4])) == NULL) {
		goto FAILED_1;
	}

	result->table = BMY_LOG_EVENT_BOARD_DENY;
	result->payload.board_deny.board = board;
	result->payload.board_deny.target_userid = target_userid;
	return result->status = BMY_LOG_PARSE_ACCEPTED;

FAILED_1:
	free(board);
FAILED_0:
	return result->status = BMY_LOG_PARSE_FAILED;
}

static enum bmy_log_parse_status bmy_log_parse_rest(const struct bmy_log_tokens *raw_tokens, struct bmy_log_parse_result *result) {
	const struct bmy_log_token *action_token = &raw_tokens->items[2];
	struct bmy_log_token full_msg;

	// bmy_log_search_result_count
	if (bmy_log_token_eq(action_token, "bbsfind") && raw_tokens->count == 4) {
		return result->status = BMY_LOG_PARSE_DISCARDED;
	}

	// bmy_log_cache_reload
	// bmy_log_system_reload
	if (bmy_log_token_eq(action_token, "reload")) {
		return result->status = BMY_LOG_PARSE_DISCARDED;
	}

	// bmy_log_thread_view
	if (bmy_log_token_eq(action_token, "thread")) {
		return result->status = BMY_LOG_PARSE_DISCARDED;
	}

	// bmy_log_search_trace
	if (bmy_log_token_eq(action_token, "full_search")) {
		return result->status = BMY_LOG_PARSE_DISCARDED;
	}

	// bmy_log_selection_trace
	if (bmy_log_token_eq(action_token, "select")) {
		return result->status = BMY_LOG_PARSE_DISCARDED;
	}

	// Legacy board-entry traces.
	if (bmy_log_token_eq(action_token, "enterboard") ||
		bmy_log_token_eq(action_token, "enterboard_t")) {
		return result->status = BMY_LOG_PARSE_DISCARDED;
	}

	// Legacy content-reading trace.
	if (bmy_log_token_eq(action_token, "readcon") ||
		bmy_log_token_eq(action_token, "readcon_t")) {
		return result->status = BMY_LOG_PARSE_DISCARDED;
	}

	// bmy_log_runtime_error
	{
		full_msg = bmy_log_token_rest_after(&raw_tokens->items[0]);

		// exec zmodem
		if (bmy_log_tokens_eq_at_idx(raw_tokens, 2, "exec")) {
			return result->status = BMY_LOG_PARSE_DISCARDED;
		}

		// [mail] foo send ...
		if (bmy_log_tokens_eq_at_idx(raw_tokens, 1, "[mail]")) {
			return result->status = BMY_LOG_PARSE_DISCARDED;
		}

		// Legacy miscellaneous diagnostics.
		if (bmy_log_tokens_eq_at_idx(raw_tokens, 1, "[wtf]") ||
			bmy_log_tokens_eq_at_idx(raw_tokens, 1, "[WTF]")) {
			return result->status = BMY_LOG_PARSE_DISCARDED;
		}

		// failed to insert UT for
		if (bmy_log_tokens_eq_at_idx(raw_tokens, 1, "failed") && bmy_log_tokens_eq_at_idx(raw_tokens, 2, "to") && bmy_log_tokens_eq_at_idx(raw_tokens, 3, "insert")) {
			return result->status = BMY_LOG_PARSE_DISCARDED;
		}

		// user_idx changed?
		if (bmy_log_tokens_eq_at_idx(raw_tokens, 1, "user_idx")) {
			return result->status = BMY_LOG_PARSE_DISCARDED;
		}


		// SHM Error!
		if (bmy_log_tokens_eq_at_idx(raw_tokens, 1, "SHM") && bmy_log_tokens_eq_at_idx(raw_tokens, 2, "Error!")) {
			return result->status = BMY_LOG_PARSE_DISCARDED;
		}

		// wechat
		if (bmy_log_token_eq(&full_msg, "[bmy/wechat] cannot parse response as JSON")) {
			return result->status = BMY_LOG_PARSE_DISCARDED;
		}
		if (bmy_log_token_starts_with(&full_msg, "[bmy/wechat] request session errcode[")) {
			return result->status = BMY_LOG_PARSE_DISCARDED;
		}
		if (bmy_log_token_starts_with(&full_msg, "[bmy/wechat] request session with curl result: ")) {
			return result->status = BMY_LOG_PARSE_DISCARDED;
		}
		if (bmy_log_token_eq(&full_msg, "[bmy/wechat] WriteMemoryCallback failed to realloc")) {
			return result->status = BMY_LOG_PARSE_DISCARDED;
		}

		// search
		if (bmy_log_token_eq(&full_msg, "[bmy/search] has problem of reading stdout") || bmy_log_token_eq(&full_msg, "[bmy/search] cannot realloc")) {
			return result->status = BMY_LOG_PARSE_DISCARDED;
		}

		// redis
		if (bmy_log_token_starts_with(&full_msg, "[redis] SET")) {
			return result->status = BMY_LOG_PARSE_DISCARDED;
		}

		// 2fa
		if (bmy_log_token_starts_with(&full_msg, "generate 2fa failed for ")) {
			return result->status = BMY_LOG_PARSE_DISCARDED;
		}

		// api
		if (bmy_log_token_starts_with(&full_msg, "write error to fileheader ") || bmy_log_token_starts_with(&full_msg, "lseek error on ")) {
			return result->status = BMY_LOG_PARSE_DISCARDED;
		}
	}

	return result->status = BMY_LOG_PARSE_UNRECOGNIZED;
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

static bool bmy_log_token_to_int(const struct bmy_log_token *token, int *x) {
	size_t i;
	*x = 0;

	if (bmy_log_token_empty(token)) {
		return false;
	}

	for (i = 0; i < token->len; i++) {
		if (!bmy_char_in_range(token->ptr[i], '0', '9')) {
			// invalid
			return false;
		}

		*x *= 10;
		*x += bmy_char_to_digit(token->ptr[i]);
	}

	return true;
}

static bool bmy_log_token_to_signed_int(const struct bmy_log_token *token, int *x) {
	struct bmy_log_token magnitude;

	if (bmy_log_token_empty(token)) {
		return false;
	}

	if (token->ptr[0] != '-') {
		return bmy_log_token_to_int(token, x);
	}

	magnitude.ptr = token->ptr + 1;
	magnitude.len = token->len - 1;
	if (!bmy_log_token_to_int(&magnitude, x)) {
		return false;
	}

	*x = -*x;
	return true;
}

static bool bmy_log_token_to_long(const struct bmy_log_token *token, long *x) {
	size_t i;
	*x = 0;

	for (i = 0; i < token->len; i++) {
		if (!bmy_char_in_range(token->ptr[i], '0', '9')) {
			// invalid
			return false;
		}

		*x *= 10;
		*x += bmy_char_to_digit(token->ptr[i]);
	}

	return true;
}

static bool bmy_log_tokens_eq_at_idx(const struct bmy_log_tokens *tokens, size_t idx, const char *s) {
	return tokens && s && idx < tokens->count && bmy_log_token_eq(&tokens->items[idx], s);
}
