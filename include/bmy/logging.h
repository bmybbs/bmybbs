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
 * @param life_value 清理时由 countlife() 返回的负数生命力值
 */
void bmy_log_account_expire_cleanup(const char *userid, int life_value);

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

// post, mail, and content lifecycle

/**
 * @brief 记录发帖
 * @details 来源
 * - src/bbs/bbs.c: normal post -> "%s post %s %s"
 * - src/bbs/1984.c: special post/check path -> "%s post %s %s"
 * - nju09/www/bbssnd.c: web post -> "%s post %s %s"
 * - api/api_article.c: api post -> "%s post %s %s"
 * @param userid 用户 id
 * @param board 版面
 * @param title 标题 (gbk 编码)
 */
void bmy_log_post_create(const char *userid, const char *board, const char *title);

/**
 * @brief 记录检查1984
 * @details 来源
 * - src/bbs/1984.c: moderation queue -> "%s check1984 %s %s"
 * @param userid 用户 id
 * @param board 版面
 * @param title 标题 (gbk 编码)
 */
void bmy_log_post_check_1984(const char *userid, const char *board, const char *title);

/**
 * @brief 记录编辑帖子
 * @details 来源
 * - src/bbs/bbs.c: edit existing post -> "%s edit %s %s %s"
 * @param userid 操作者 id
 * @param board 版面
 * @param owner 所有者 id
 * @param title 标题 (gbk 编码)
 */
void bmy_log_post_edit(const char *userid, const char *board, const char *owner, const char *title);

/**
 * @brief 记录删除帖子
 * @details 来源
 * - src/bbs/bbs.c: delete post -> "%s del %s %s %s"
 * @param userid 操作者 id
 * @param board 版面
 * @param owner 所有者 id
 * @param title 标题 (gbk 编码)
 */
void bmy_log_post_delete(const char *userid, const char *board, const char *owner, const char *title);

/**
 * @brief 记录复原帖子
 * @details 来源
 * - src/bbs/bbs.c: restore post -> "%s undel %s %s %s"
 * @param userid 操作者 id
 * @param board 版面
 * @param owner 所有者 id
 * @param title 标题 (gbk 编码)
 */
void bmy_log_post_restore(const char *userid, const char *board, const char *owner, const char *title);

/**
 * @brief 记录转贴
 * @details 来源
 * - src/bbs/bbs.c: crosspost -> "%s crosspost %s %s"
 * @param userid 操作者 id
 * @param board 版面
 * @param title 标题 (gbk 编码)
 */
void bmy_log_post_crosspost(const char *userid, const char *board, const char *title);

/**
 * @brief 记录同标题
 * @details 来源
 * - src/bbs/read.c: same-title flow -> "%s sametitle %s %s"
 * @param userid 操作者 id
 * @param board 版面
 * @param title 标题 (gbk 编码)
 */
void bmy_log_post_same_title(const char *userid, const char *board, const char *title);

/**
 * @brief 记录同主题
 * @details 来源
 * - src/bbs/bbs.c: do_thread -> "%s thread %s"
 * @param userid 操作者 id
 * @param board 版面
 */
void bmy_log_thread_view(const char *userid, const char *board);

/**
 * @brief 记录站内信
 * @details 来源
 * - src/bbs/mail.c: station mail -> "%s mail %s"
 * - nju09/www/BBSLIB.c: web mail -> "%s mail %s"
 * @param userid 操作者 id
 * @param target_userid 收信人 id
 */
void bmy_log_mail_send(const char *userid, const char *target_userid);

/**
 * @brief 记录站外信
 * @details 来源
 * - src/bbs/mail.c: outbound netmail -> "%s netmail %s"
 * @param userid 操作者 id
 * @param target_userid 收信人 id
 */
void bmy_log_netmail_send(const char *userid, const char *target_userid);

/**
 * @brief 记录解封站内信
 * @details 来源
 * - local_utl/autoundeny/autoundeny.c: utility mail -> "XJTU-XANET mail %s"
 * @param sender 发信人 id
 * @param target_userid 收信人 id
 */
void bmy_log_utility_mail_send(const char *sender, const char *target_userid);

// moderation and content state

/**
 * @brief 记录封禁
 * @details 来源
 * - src/bbs/bm.c: terminal deny -> "%s deny %s %s"
 * - nju09/www/bbsdenyadd.c: web deny -> "%s deny %s %s"
 * @param operator_userid 操作者 id
 * @param board 版面
 * @param target_userid 被封禁 id
 */
void bmy_log_board_deny(const char *operator_userid, const char *board, const char *target_userid);

/**
 * @brief 记录m标记
 * @details 来源
 * - src/bbs/bbs.c and nju09/www/bbsman.c: "%s mark %s %s %s"
 * @param userid 操作者 id
 * @param board 版面
 * @param owner 帖子所有者 id
 * @param title 标题 (gbk 编码)
 */
void bmy_log_post_mark(const char *userid, const char *board, const char *owner, const char *title);

/**
 * @brief 记录解除m标记
 * @details 来源
 * - src/bbs/bbs.c and nju09/www/bbsman.c: "%s unmark %s %s %s"
 * @param userid 操作者 id
 * @param board 版面
 * @param owner 帖子所有者 id
 * @param title 标题 (gbk 编码)
 */
void bmy_log_post_unmark(const char *userid, const char *board, const char *owner, const char *title);

/**
 * @brief 记录g标记
 * @details 来源
 * - src/bbs/bbs.c and nju09/www/bbsman.c: "%s digest %s %s %s"
 * @param userid 操作者 id
 * @param board 版面
 * @param owner 帖子所有者 id
 * @param title 标题 (gbk 编码)
 */
void bmy_log_post_digest(const char *userid, const char *board, const char *owner, const char *title);

/**
 * @brief 记录解除g标记
 * @details 来源
 * - src/bbs/bbs.c and nju09/www/bbsman.c: "%s undigest %s %s %s"
 * @param userid 操作者 id
 * @param board 版面
 * @param owner 帖子所有者 id
 * @param title 标题 (gbk 编码)
 */
void bmy_log_post_undigest(const char *userid, const char *board, const char *owner, const char *title);

/**
 * @brief 记录水文标记
 * @details 来源
 * - src/bbs/bbs.c: "%s water %s %s %s"
 * @param userid 操作者 id
 * @param board 版面
 * @param owner 帖子所有者 id
 * @param title 标题 (gbk 编码)
 */
void bmy_log_post_water(const char *userid, const char *board, const char *owner, const char *title);

/**
 * @brief 记录解除水文标记
 * @details 来源
 * - src/bbs/bbs.c: "%s unwater %s %s %s"
 * @param userid 操作者 id
 * @param board 版面
 * @param owner 帖子所有者 id
 * @param title 标题 (gbk 编码)
 */
void bmy_log_post_unwater(const char *userid, const char *board, const char *owner, const char *title);

/**
 * @brief 记录标题更新
 * @details 来源
 * src/bbs/bbs.c: "%s changetitle %s %s oldtitle:%s newtitle:%s"
 * @param userid 操作者 id
 * @param board 版面
 * @param owner 帖子所有者 id
 * @param old_title 旧标题 (gbk 编码)
 * @param new_title 新标题 (gbk 编码)
 */
void bmy_log_post_title_change(const char *userid, const char *board, const char *owner, const char *old_title, const char *new_title);

/**
 * @brief 记录区段删除
 * @details 来源
 * - src/bbs/bbs.c: "%s ranged %s %d %d"
 * @param userid 操作者 id
 * @param board 版面
 * @param from_id 起始编号
 * @param to_id 结束编号
 */
void bmy_log_post_range_delete(const char *userid, const char *board, int from_id, int to_id);

/**
 * @brief 记录站内信区段删除
 * @details 来源
 * - src/bbs/bbs.c: "%s rangedmail %d %d"
 * @param userid 操作者 id
 * @param from_id 起始编号
 * @param to_id 结束编号
 */
void bmy_log_mail_range_delete(const char *userid, int from_id, int to_id);

/**
 * @brief 记录置顶
 * @details 来源
 * - src/bbs/bbs.c: top action with title-bearing payload
 * @param userid 操作者 id
 * @param board 版面
 * @param owner 作者
 * @param title 标题
 */
void bmy_log_post_top(const char *userid, const char *board, const char *owner, const char *title);

/**
 * @brief 记录取消置顶
 * @details 来源
 * - src/bbs/bbs.c: un-top action with title-bearing payload
 * @param userid 操作者 id
 * @param board 版面
 * @param owner 作者
 * @param title 标题
 */
void bmy_log_post_untop(const char *userid, const char *board, const char *owner, const char *title);

/**
 * @brief 记录精华区操作
 * @details 来源
 * - src/bbs/announce.c: an_log -> "%s %s %s %s"
 * @param userid 操作者 id
 * @param action 操作
 * @param board 版面
 * @param path 路径
 */
void bmy_log_announce_action(const char *userid, const char *action, const char *board, const char *path);

/**
 * @brief 记录精华区操作
 * @details 来源
 * - src/bbs/announce.c: a_Import -> "%s import %s %s %s"
 * @param userid 操作者 id
 * @param board 版面
 * @param owner 所有者
 * @param title 标题
 */
void bmy_log_announce_import(const char *userid, const char *board, const char *owner, const char *title);

// lower-priority social and query events

/**
 * @brief 记录聊天请求
 * @details 来源
 * - src/bbs/talk.c: "%s five %s" and "%s talk %s"
 * @param userid 操作者 id
 * @param target_userid 接收者 id
 * @param kind 聊天类型 NOTE 实际中仅仅为 "talk"
 */
void bmy_log_talk_request(const char *userid, const char *target_userid, const char *kind);

/**
 * @brief 记录发送祝福
 * @details 来源
 * - src/bbs/xyz.c: "%s sendgoodwish %s"
 * @param userid 操作者 id
 * @param target_userid 接收者 id
 */
void bmy_log_send_goodwish(const char *userid, const char *target_userid);

/**
 * @brief 记录查找某用户最近 n 天的发帖
 * @details 来源
 * - src/bbs/xyz.c: "%s finddf %s %d"
 * @param userid 操作者 id
 * @param target 被调查者 id
 * @param count 检查天数
 */
void bmy_log_finddf(const char *userid, const char *target, int count);

// operational and integration diagnostics

/**
 * @brief 记录系统缓存重载
 * @details 相关模块 libythtbbs/cache/board.c and cache/user.c:
 * "system reload bcache %d", "system reload ucache %d", similar cache reloads
 * @param cache_name 缓存名
 * @param count 缓存对应的计数（不适用于 bmonline）
 */
void bmy_log_cache_reload(const char *cache_name, int count);

/**
 * @brief 记录系统配置重载
 * @details 相关模块 src/bbs/comm_lists.c and src/bbs/more.c:
 * "system reload sysconf.img2", "system reload movie %d", similar system reloads
 * @param resource_name 资源名
 * @param count 计数，仅适用于 movie
 */
void bmy_log_system_reload(const char *resource_name, int count);

/**
 * @brief 记录运行时错误（包括日志级别）
 * @details 目前仅仅是 `newtrace` 的封装 libythtbbs/cache/cache-internal.h, api/api_article.c, and similar runtime failures
 */
void bmy_log_runtime_error(const char *message);

/**
 * @brief 记录超级选择
 * src/bbs/power_select.c: "%s full_search %s %s"
 * @param userid 操作者 id
 * @param board 版面
 * @param query 查询
 */
void bmy_log_search_trace(const char *userid, const char *board, const char *query);

/**
 * @brief 记录范围选择
 * src/bbs/power_select.c: "%s select %s %d %d"
 * @param userid 操作者 id
 * @param board 版面
 * @param int id1 开始
 * @param int id2 结束
 */
void bmy_log_selection_trace(const char *userid, const char *board, int id1, int id2);
#endif
