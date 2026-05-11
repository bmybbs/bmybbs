#include "log_tokenizer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 判断 `ch` 是否为空格
 */
static bool bmy_log_tokenizer_is_space(char ch) {
	return isspace((unsigned char) ch);
}

/**
 * @brief trim 结尾的换行符
 * @returns 不包含换行符的长度
 */
static size_t bmy_log_tokenizer_trim_line_end(const char *text, size_t len) {
	while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
		len--;
	}
	return len;
}

bool bmy_log_tokenize(const char *line, struct bmy_log_tokens *tokens) {
	const char *cursor;

	if (tokens == NULL)
		return false;

	memset(tokens, 0, sizeof(*tokens));
	if (line == NULL)
		return false;

	cursor = line;
	while (*cursor != '\0') {
		const char *start;
		size_t len;

		while (*cursor != '\0' && bmy_log_tokenizer_is_space(*cursor))
			cursor++;

		if (*cursor == '\0')
			break;

		start = cursor;
		while (*cursor != '\0' && !bmy_log_tokenizer_is_space(*cursor))
			cursor++;

		if (tokens->count >= BMY_LOG_TOKENIZER_MAX_TOKENS) {
			tokens->truncated = true;
			break;
		}

		len = (size_t)(cursor - start);
		tokens->items[tokens->count].ptr = start;
		tokens->items[tokens->count].len = len;
		tokens->count++;
	}

	return true;
}

bool bmy_log_token_eq(const struct bmy_log_token *token, const char *text) {
	size_t len;

	if (token == NULL || token->ptr == NULL || text == NULL)
		return false;

	len = strlen(text);
	if (token->len != len)
		return false;

	return strncmp(token->ptr, text, len) == 0;
}

bool bmy_log_token_empty(const struct bmy_log_token *token) {
	return token == NULL || token->ptr == NULL || token->len == 0;
}

char * bmy_log_token_dup(const struct bmy_log_token *token) {
	char *text;

	if (token == NULL || token->ptr == NULL)
		return NULL;

	text = malloc(token->len + 1);
	if (text == NULL)
		return NULL;

	memcpy(text, token->ptr, token->len);
	text[token->len] = '\0';
	return text;
}

struct bmy_log_token bmy_log_token_rest_after(const struct bmy_log_token *token) {
	struct bmy_log_token rest = {0};
	const char *cursor;
	size_t len;

	if (token == NULL || token->ptr == NULL)
		return rest;

	cursor = token->ptr + token->len;
	while (*cursor != '\0' && bmy_log_tokenizer_is_space(*cursor))
		cursor++;

	len = strlen(cursor);
	len = bmy_log_tokenizer_trim_line_end(cursor, len);

	rest.ptr = cursor;
	rest.len = len;
	return rest;
}
