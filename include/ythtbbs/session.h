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

/**
 * @brief 向 session 中存放值
 * 若键不合法，跳过处理
 * @param key 允许 [A-Za-z]{1,8} 的键
 * @param value 对应的值
 */
void ythtbbs_session_set_value(const char *sessionid, const char *key, const char *value);

/**
 * @brief 从 session 中读取存放的值
 * @return NULL 或者使用 strdup(3) 生成的副本，需要使用 free(3) 释放
 */
char *ythtbbs_session_get_value(const char *session, const char *key);

/**
 * @brief 从 session 移除键
 * 是 ythtbbs_session_set_value 相反的操作
 * @param sessionid
 * @param key
 */
void ythtbbs_session_clear_key(const char *sessionid, const char *key);
#endif
