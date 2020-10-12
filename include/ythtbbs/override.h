#ifndef BMYBBS_OVERRIDE_H
#define BMYBBS_OVERRIDE_H
#include <stddef.h>

/**
 * @brief 用于存放好友、拒绝信息的结构体
 * 原名 struct override，重构阶段增加了 ythtbbs_ 前缀
 */
struct ythtbbs_override {
	char id[13];   ///< 用户 id
	char exp[40];  ///< 说明
};

enum ythtbbs_override_type {
	YTHTBBS_OVERRIDE_FRIENDS,
	YTHTBBS_OVERRIDE_REJECTS
};

// 使用文件锁简单实现多线程、多进程互斥
int ythtbbs_override_lock(const char *userid, const enum ythtbbs_override_type override_type);

void ythtbbs_override_unlock(int lockfd);

int ythtbbs_override_add(const char *userid, const struct ythtbbs_override *ptr_override, const enum ythtbbs_override_type override_type);

int ythtbbs_override_count(const char *userid, const enum ythtbbs_override_type override_type);

int ythtbbs_override_included(char *userid, const enum ythtbbs_override_type override_type, const char *search_id);

long ythtbbs_override_get_records(const char *userid, struct ythtbbs_override *array, const size_t count, const enum ythtbbs_override_type override_type);

void ythtbbs_override_set_records(const char *userid, const struct ythtbbs_override *array, const size_t count, const enum ythtbbs_override_type override_type);
#endif
