#include <string.h>
#include <stddef.h>
#include "bmy/redis.h"
#include "ythtbbs/misc.h"

static const char SESSION_DICT[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789";
static const int SESSION_DICT_LEN = 62;

void ythtbbs_session_generate_id(char *buf, size_t len) {
	size_t i;
	ythtbbs_get_random_buf(buf, len);

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
	int           utmp_idx;
	size_t        i;

	ctx = bmy_redisConnect();
	if (ctx == NULL || ctx->err)
		goto ERROR;

	reply = redisCommand(ctx, "HGETALL BMY:Session:%s", sessionid);
	if (!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements != 4)
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

