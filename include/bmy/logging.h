#ifndef BMY_LOGGING_H
#define BMY_LOGGING_H

#include "bmy/user.h"

/*
 * Phase 1 semantic logging interfaces.
 *
 * In Phase 1, implementations should preserve the existing emitted text while
 * removing direct `newtrace` calls from subsystem code.
 */

// account and session

/*
 * @brief 记录用户登录
 *
 * @details 来源
 * - src/bbs/main.c: user_login -> "%s enter %s"
 * - libythtbbs/user.c: ythtbbs_user_login -> "%s enter %s using %s"
 *
 * @param userid 用户 id，应该来自于结构体 userec 或者 user_info
 * @param fromhost IPv4 或者 IPv6，应该来自于结构体 user_info 或者 userec 或相关字段
 * @param login_type 登录类型
 */
void bmy_log_login_success(const char *userid, const char *fromhost, enum ythtbbs_user_login_type login_type);

/**
 * @brief 记录登录失败
 * @details 来源:
 * - libythtbbs/user.c: logattempt -> "system passerr %s"
 * @param fromhost 来源 IP，应该是 IPv4 或者 IPv6
 */
void bmy_log_login_failure(const char *fromhost);

/**
 * @brief 记录用户正常登出
 * @details 来源
 * - src/bbs/main.c: Q_Goodbye -> "%s exitbbs %ld"
 * - libythtbbs/user.c: ythtbbs_user_logout -> "%s exitbbs %ld"
 * @param userid 用户 id
 * @param stay 停留时间
 * @see bmy_log_disconnect
 * @see bmy_log_session_cleanup
 */
void bmy_log_logout(const char *userid, long stay);

/**
 * @brief 记录用户断开连接
 * @details 来源
 * - src/bbs/main.c: do_abort_bbs -> "%s drop %ld"
 * @param userid 用户 id
 * @param stay 停留时间
 * @see bmy_log_logout
 * @see bmy_log_session_cleanup
 */
void bmy_log_disconnect(const char *userid, long stay);

/**
 * @brief 记录 www/api 会话被清理
 * @details 来源
 * - libythtbbs/cache/utmp.c: stale session cleanup -> "%s drop www/api"
 * @param userid 用户 id
 * @see bmy_log_logout()
 * @see bmy_log_disconnect()
 */
void bmy_log_session_cleanup(const char *userid);

/**
 * @brief 记录用户踢出自己会话
 * @detais 来源
 * - src/bbs/main.c: multi_user_check -> "%s kick %s multi-login"
 * @param userid 用户 id
 */
void bmy_log_multi_login_kick(const char *userid);

/**
 * @brief 记录管理者踢出用户会话
 * @detais 来源
 * - src/bbs/delete.c: kick out user -> "%s kick %s"
 * @param operator_userid 操作者 id
 * @param target_userid 被踢用户 id
 */
void bmy_log_user_kick(const char *operator_userid, const char *target_userid);
#endif
