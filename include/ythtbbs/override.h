#ifndef BMYBBS_OVERRIDE_H
#define BMYBBS_OVERRIDE_H

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

int ythtbbs_override_add(const char *userid, const struct ythtbbs_override *ptr_override, enum ythtbbs_override_type override_type);

int ythtbbs_override_count(const char *userid, enum ythtbbs_override_type override_type);

int ythtbbs_override_included(char *userid, enum ythtbbs_override_type override_type, const char *search_id);
#endif
