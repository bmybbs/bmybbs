#include <check.h>
#include "bmy/redis.h"

START_TEST(test_redis_connect) {
	redisContext *ctx = bmy_redisConnect();
	ck_assert_ptr_nonnull(ctx);

	if (ctx->err) {
		printf("%s\n", ctx->errstr);
	}
	ck_assert_int_eq(ctx->err, 0);
	redisFree(ctx);
}

START_TEST(test_redis_ping) {
	redisContext *ctx = bmy_redisConnect();
	ck_assert_ptr_nonnull(ctx);
	ck_assert_int_eq(ctx->err, 0);

	redisReply *reply = redisCommand(ctx, "PING");
	ck_assert_int_eq(reply->type, REDIS_REPLY_STATUS);
	ck_assert_str_eq(reply->str, "PONG");

	freeReplyObject(reply);
	redisFree(ctx);
}

END_TEST

Suite * test_suite_redis(void) {
	Suite *s = suite_create("check redis");
	TCase *tc = tcase_create("check redis connection");
	tcase_add_test(tc, test_redis_connect);
	tcase_add_test(tc, test_redis_ping);
	suite_add_tcase(s, tc);

	return s;
}

