#ifndef BMYBBS_REDIS_H
#define BMYBBS_REDIS_H
#include <hiredis/hiredis.h>
#define REDIS_SOCK "/var/run/redis/redis-server.sock"
#define REDIS_IP   "127.0.0.1"
#define REDIS_PORT 6379

/**
 * @brief 建立 redis 连接
 * 属于 redisConnect* 的封装，当前 redisConnectUnix 还会遇到权限配置问题，
 * 因此使用 TCP 作为备选项。
 */
static inline redisContext *bmy_redisConnect() {
	redisContext *ctx;
	ctx = redisConnectUnix(REDIS_SOCK);
	if (ctx == NULL || ctx->err) {
		if (ctx != NULL)
			redisFree(ctx);

		ctx = redisConnect(REDIS_IP, REDIS_PORT);
	}

	return ctx;
}
#endif
