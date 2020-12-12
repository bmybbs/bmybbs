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
#endif

