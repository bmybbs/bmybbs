#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include "bmy/redis.h"
#include "ytht/random.h"
#include "ythtbbs/cache.h"

static const char SESSION_DICT[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789";
static const int SESSION_DICT_LEN = 62;

void ythtbbs_session_generate_id(char *buf, size_t len) {
	size_t i;
	ytht_get_random_buf(buf, len);

	for (i = 0; i < len; i++) {
		buf[i] = SESSION_DICT[((unsigned char)buf[i]) % SESSION_DICT_LEN];
	}

	buf[len - 1] = '\0';
}

int ythtbbs_session_set(const char *sessionid, const char *userid, const int utmp_idx) {
	redisContext *ctx = NULL;
	redisReply   *reply = NULL;
	int           ret = -1;
	ctx = bmy_redisConnect();
	if (ctx == NULL || ctx->err)
		goto END;

	reply = redisCommand(ctx, "HSET BMY:Session:%s userid %s utmp_idx %d", sessionid, userid, utmp_idx);
	if (!reply || reply->type != REDIS_REPLY_INTEGER || reply->integer < 0)
		goto END;

	freeReplyObject(reply);
	reply = redisCommand(ctx, "EXPIRE BMY:Session:%s %d", sessionid, MAX_SESS_TIME);

	ret = 0;
END:
	if (reply) freeReplyObject(reply);
	if (ctx)   redisFree(ctx);
	return ret;
}

int ythtbbs_session_del(const char *sessionid) {
	redisContext *ctx = NULL;
	redisReply   *reply = NULL;
	int           ret = -1;
	ctx = bmy_redisConnect();
	if (ctx == NULL || ctx->err)
		goto END;

	reply = redisCommand(ctx, "DEL BMY:Session:%s", sessionid);
	if (!reply || reply->type != REDIS_REPLY_INTEGER || reply->integer < 0)
		goto END;

	ret = 0;
END:
	if (reply) freeReplyObject(reply);
	if (ctx)   redisFree(ctx);
	return ret;
}

int ythtbbs_session_get_utmp_idx(const char *sessionid, const char *userid) {
	redisContext *ctx = NULL;
	redisReply   *reply = NULL;
	redisReply   *key, *val;
	int           utmp_idx = -1;
	size_t        i;

	ctx = bmy_redisConnect();
	if (ctx == NULL || ctx->err)
		goto ERROR;

	reply = redisCommand(ctx, "HGETALL BMY:Session:%s", sessionid);
	if (!reply || reply->type != REDIS_REPLY_ARRAY)
		goto ERROR;

	for (i = 0; i < reply->elements/2; i++) {
		key = reply->element[i*2];
		val = reply->element[i*2+1];

		if (key->type != REDIS_REPLY_STRING)
			goto ERROR;

		if (!strcmp(key->str, "userid")) {
			if (val->type != REDIS_REPLY_STRING || strcmp(val->str, userid) != 0) {
				goto ERROR;
			}
		}

		if (!strcmp(key->str, "utmp_idx")) {
			if (val->type != REDIS_REPLY_STRING)
				goto ERROR;

			utmp_idx = atoi(val->str);
		}
	}

	freeReplyObject(reply);
	redisFree(ctx);
	return utmp_idx;
ERROR:
	if (reply) freeReplyObject(reply);
	if (ctx)   redisFree(ctx);
	return -1;
}

static const size_t MAX_SESS_VALID_LENGTH = 8;

// 只允许 [A-Za-z]{8} 的键名
static bool is_valid_key(const char *key) {
	size_t l, i;
	char c;

	if (key == NULL || key[0] == '\0')
		return false;

	// 系统键
	if (strcasecmp(key, "userid") == 0 || strcasecmp(key, "utmp_idx") == 0)
		return false;

	l = strlen(key);
	if (l > MAX_SESS_VALID_LENGTH)
		return false;

	for (i = 0; i < l; i++) {
		c = key[i];
		if (!((c>='a' && c<='z') || (c>='A' && c<='Z')))
			return false;
	}

	return true;
}

void ythtbbs_session_set_value(const char *sessionid, const char *key, const char *value) {
	redisContext *ctx = NULL;
	redisReply   *reply = NULL;

	if (!is_valid_key(key))
		return;

	if (value == NULL || value[0] == '\0')
		return;

	ctx = bmy_redisConnect();
	if (ctx == NULL || ctx->err)
		goto END;

	reply = redisCommand(ctx, "HSET BMY:Session:%s %s %s", sessionid, key, value);

END:
	if (reply) freeReplyObject(reply);
	if (ctx)   redisFree(ctx);
}

char *ythtbbs_session_get_value(const char *session, const char *key) {
	redisContext *ctx = NULL;
	redisReply   *reply = NULL;
	char *s = NULL;

	if (!is_valid_key(key))
		goto END;

	ctx = bmy_redisConnect();
	if (!ctx || ctx->err)
		goto END;

	reply = redisCommand(ctx, "HGET BMY:Session:%s %s", session, key);
	if (!reply || reply->type != REDIS_REPLY_STRING)
		goto END;

	s = strdup(reply->str);

END:
	if (reply) freeReplyObject(reply);
	if (ctx)   redisFree(ctx);
	return s;
}

void ythtbbs_session_clear_key(const char *sessionid, const char *key) {
	redisContext *ctx = NULL;
	redisReply   *reply = NULL;

	if (!is_valid_key(key))
		return;

	ctx = bmy_redisConnect();
	if (ctx == NULL || ctx->err)
		goto END;

	reply = redisCommand(ctx, "HDEL BMY:Session:%s %s", sessionid, key);
END:
	if (reply) freeReplyObject(reply);
	if (ctx)   redisFree(ctx);
}

