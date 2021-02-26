#include <stdbool.h>
#include <string.h>
#include "ytht/random.h"
#include "bmy/redis.h"
#include "bmy/2fa.h"

static const char BMY_2FA_KEY_DICT[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789";
static const int BMY_2FA_KEY_LEN = 62;

static const int MAX_TTL = 300; // 5 min
static const int VALID_INTERVAL = 10; // 10s
static const int MAX_ALLOWED_ATTEMPTS = 5;

bmy_2fa_status bmy_2fa_create(char *key, size_t len) {
	size_t i;
	unsigned int code;
	bmy_2fa_status status = BMY_2FA_SUCCESS;
	redisContext *ctx = NULL;
	redisReply   *reply = NULL;

	ytht_get_random_int(&code);
	ytht_get_random_buf(key, len);
	for (i = 0; i < len; i++) {
		key[i] = BMY_2FA_KEY_DICT[((unsigned char) key[i]) % BMY_2FA_KEY_LEN];
	}
	key[len - 1] = '\0';

	ctx = bmy_redisConnect();
	if (ctx == NULL || ctx->err) {
		status = BMY_2FA_CANNOT_CONNECT_REDIS;
		goto END;
	}

	reply = redisCommand(ctx, "HSET BMY:2Fa:%s code %06d count 0", key, (code % 1000000));
	if (!reply || reply->type != REDIS_REPLY_INTEGER || reply->integer < 0) {
		status = BMY_2FA_CANNOT_SET_2FA_CODE;
		goto END;
	}

	freeReplyObject(reply);
	reply = redisCommand(ctx, "EXPIRE BMY:2Fa:%s %d", key, MAX_TTL);

END:
	if (reply) freeReplyObject(reply);
	if (ctx)   redisFree(ctx);
	return status;
}

bmy_2fa_status bmy_2fa_valid(const char *key) {
	redisContext *ctx = NULL;
	redisReply   *reply = NULL;
	bmy_2fa_status status = BMY_2FA_SUCCESS;

	ctx = bmy_redisConnect();
	if (ctx == NULL || ctx->err) {
		status = BMY_2FA_CANNOT_CONNECT_REDIS;
		goto END;
	}

	reply = redisCommand(ctx, "TTL BMY:2Fa:%s", key);
	if (!reply || reply->type != REDIS_REPLY_INTEGER) {
		status = BMY_2FA_WRONG_ANSWER;
		goto END;
	}

	if (reply->integer < VALID_INTERVAL) {
		status = BMY_2FA_NOT_EXISTED;
	}

END:
	if (reply) freeReplyObject(reply);
	if (ctx)   redisFree(ctx);
	return status;
}

bmy_2fa_status bmy_2fa_get_code(const char *key, const char *auth, char *code, size_t len) {
	redisContext *ctx = NULL;
	redisReply   *reply = NULL;
	bmy_2fa_status status = BMY_2FA_SUCCESS;

	if (auth == NULL || key == NULL || code == NULL)
		return BMY_2FA_NOT_EXISTED;

	ctx = bmy_redisConnect();
	if (ctx == NULL || ctx->err) {
		status = BMY_2FA_CANNOT_CONNECT_REDIS;
		goto END;
	}

	reply = redisCommand(ctx, "HGET BMY:2Fa:%s code", key);
	if (!reply || reply->type != REDIS_REPLY_STRING) {
		status = BMY_2FA_NOT_EXISTED;
		goto END;
	}

	strncpy(code, reply->str, len);
	code[len - 1] = 0;

	reply = redisCommand(ctx, "HSET BMY:2Fa:%s auth %s", key, auth);
	if (!reply || reply->type != REDIS_REPLY_INTEGER || reply->integer < 0) {
		status = BMY_2FA_CANNOT_SET_OPENID;
		goto END;
	}

	freeReplyObject(reply);
	reply = redisCommand(ctx, "EXPIRE BMY:2Fa:%s %d", key, MAX_TTL);

END:
	if (reply) freeReplyObject(reply);
	if (ctx)   redisFree(ctx);
	return status;
}

char *bmy_2fa_check_code(const char *key, const char *code) {
	redisContext *ctx = NULL;
	redisReply   *reply = NULL;
	const redisReply *r_key, *r_count = NULL, *r_code = NULL, *r_auth = NULL;
	size_t        i;
	int           count;
	char *s = NULL;

	if (key == NULL || code == NULL)
		return NULL;

	ctx = bmy_redisConnect();
	if (ctx == NULL || ctx->err) {
		goto END;
	}

	reply = redisCommand(ctx, "HGETALL BMY:2Fa:%s", key);
	if (!reply || reply->type != REDIS_REPLY_ARRAY) {
		goto END;
	}

	// get all the elements
	for (i = 0; i < reply->elements / 2; i++) {
		r_key = reply->element[i * 2];
		if (!strcmp(r_key->str, "count")) {
			r_count = reply->element[i * 2 + 1];
		} else if (!strcmp(r_key->str, "code")) {
			r_code  = reply->element[i * 2 + 1];
		} else if (!strcmp(r_key->str, "auth")) {
			r_auth  = reply->element[i * 2 + 1];
		}
	}

	// check count
	if (!r_count || r_count->type != REDIS_REPLY_STRING) {
		// 返回值类型不符合预期
		goto END;
	}
	count = atoi(r_count->str);
	if (count >= MAX_ALLOWED_ATTEMPTS) {
		// 超过最大尝试次数
		goto END;
	}

	// check code
	if (!r_code || r_code->type != REDIS_REPLY_STRING) {
		// 返回值类型不符合预期
		goto END;
	}
	if (strcmp(r_code->str, code) != 0) {
		// code 出错
		freeReplyObject(reply);
		reply = redisCommand(ctx, "HINCRBY BMY:2Fa:%s count 1", key);
		if (reply)
			freeReplyObject(reply);

		reply = redisCommand(ctx, "EXPIRE BMY:2Fa:%s %d", key, MAX_TTL);
		goto END;
	}

	// 处理 auth
	if (!r_auth || r_auth->type != REDIS_REPLY_STRING || r_auth->str == NULL || r_auth->str[0] == '\0') {
		goto END;
	}

	// finally
	s = strdup(r_auth->str);
	freeReplyObject(reply);
	reply = redisCommand(ctx, "DEL BMY:2Fa:%s", key);

END:
	if (reply) freeReplyObject(reply);
	if (ctx)   redisFree(ctx);
	return s;
}

