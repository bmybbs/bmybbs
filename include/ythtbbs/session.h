#ifndef BMYBBS_SESSION_H
#define BMYBBS_SESSION_H
#include <stddef.h>

/** @file session.h
 * Session 模块
 * 基于 redis 实现的 session 设置、删除和查找的模块。
 */

/**
 * @brief 在缓冲区中生成 sessionid
 * sessionid 是由 A-Za-z0-9 字符构成的随机字符串。
 * 本函数主要在创建会话的时候设置 user_info 的 session 字段。
 * @param buf 缓冲区
 * @param len 缓冲区大小
 */
void ythtbbs_session_generate_id(char *buf, size_t len);

/**
 * @brief 向 redis 中设置 sessionid
 * redis 作为 session 模块的散列表，用于存放和查找会话索引。
 */
int ythtbbs_session_set(const char *sessionid, const char *userid, const int utmp_idx);

/**
 * @brief 从 redis 中删除 sessionid
 */
int ythtbbs_session_del(const char *sessionid);

/**
 * @brief 依据 sessionid 和 userid 获取 utmp_idx
 * 只有当 redis 连接正常、记录匹配时才会返回 utmp_idx (从0开始索引)，
 * 出错时返回 -1
 */
int ythtbbs_session_get_utmp_idx(const char *sessionid, const char *userid);
#endif
