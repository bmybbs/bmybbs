/**
 * 本代码是订阅功能的一部分，用于将用户数据从 ythtbbs 系统同步到数据库中。
 */
#ifndef BMY_USER_H
#define BMY_USER_H

/**
 * 创建用户、订阅元数据，以及用户订阅视图
 * 调用存储过程 procedure_insert_user
 * @param usernum 用户编号，从 1 开始
 * @param userid 用户 id
 */
void bmy_user_create(int usernum, char *userid);

/**
 * 删除用户、订阅元数据，以及用户订阅视图
 * 调用存储过程 procedure_delete_user
 * @param usernum 用户编号，从 1 开始
 * @param userid 用户 id
 */
void bmy_user_delete(int usernum, char *userid);

/**
 * @brief 关联微信 openid
 * @param usernum 用户编号
 * @param openid 通过 bmy_wechat_session_get 获取
 */
void bmy_user_associate_openid(int usernum, char *openid);

/**
 * @brief 移除关联的 openid
 */
void bmy_user_dissociate_openid(int usernum);

/**
 * @brief 依据 openid 查询用户编号
 * @param openid 通过 bmy_wechat_session_get 获取
 * @return 用户编号，如果不存在，返回 0
 */
int bmy_user_getusernum_by_openid(char *openid);
#endif

