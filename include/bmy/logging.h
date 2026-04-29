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

/**
 * @brief 记录用户注册
 * @details 来源
 * - src/bbs/register.c: terminal registration -> "%s newaccount %d %s"
 * - nju09/www/bbsdoreg.c: web registration -> "%s newaccount %d %s www"
 * @param userid 用户 id
 * @param usernum 索引
 * @param fromhost IP
 * @param session_type 会话类型
 */
void bmy_log_account_create(const char *userid, int usernum, const char *fromhost, enum ythtbbs_user_login_type session_type);

/**
 * @brief 记录清理生命力为负的用户
 * @details 来源
 * - libythtbbs/user.c: ythtbbs_user_clean -> "system kill %s %d"
 * @param userid 用户 id
 * @param usernum 索引
 */
void bmy_log_account_expire_cleanup(const char *userid, int usernum);

// board usage and statistics

/**
 * @brief 记录版面停留时间
 * @details 来源
 * - src/bbs/boards.c: board timing -> "%s use %s %ld"
 * - nju09/www/BBSLIB.c: updatelastboard -> "%s use %s %ld"
 */
void bmy_log_board_use(const char *userid, const char *board, long stay);

/**
 * @brief 记录版面搜索
 * @details 来源
 * - nju09/www/bbsfind.c: search result count -> "%s bbsfind %d"
 * @param userid 用户 id
 * @param result_count 搜索结果计数
 */
void bmy_log_search_result_count(const char *userid, int result_count);
#endif
