#ifndef __APP_H
#define __APP_H
#include "ythtbbs.h"

/** bmy app 结构体
 * 存放和app有关的原始数据
 */
struct bmyapp_s {
    char appkey[APPKEYLENGTH];				//!< app key，用于区分每一个 app，app key 一旦生成无法变更，并且需要保证唯一。
    char secretkey[MAXSECRETKEYLENGTH];     //!< 私钥，用于url签名验证
    char userid[IDLEN + 4];                 //!< app 的使用人 \@warning 一般来说bmy代码中使用 IDLEN+2，这里使用 IDLEN+4，确保对齐
    char appname[APPNAMELENGTH];            //!< app 的名称
    time_t createtime;                      //!< 应用的创建时间
    unsigned int status;                    //!< app 的状态
    unsigned int permission;				//!< api 权限
    struct bmyapp_s *next;					//!< 仅用于 struct bmyapp_s * 链表，该值在文件中不存储数据。
};

enum bmyapp_s_status {
	BMYAPP_STATUS_INITIAL	= 1,			//!< 站长初始化创建
	BMYAPP_STATUS_CREATED	= 2,			//!< 开发人员应用创建完成
	BMYAPP_STATUS_PUBLISHED	= 3,			//!< 站长审核通过并发布
	BMYAPP_STATUS_FORBIDDEN	= 4,			//!< 应用被禁用，应用拥有者可以禁用应用，但是只有站长可以恢复到 PUBLISHED 状态
	BMYAPP_STATUS_DELETED	= 5,			//!< 删除状态，但是不删除应用信息，确保 APPKEY 不重复。任何人不能从已删除状态恢复。
};

/** appid hash table
 * 位于共享内存中，用于快速索引查找 appid
 */
struct appidhashitem {
	int num;
	char appkey[APPKEYLENGTH];
	char appname[APPNAMELENGTH];
};

/**
 * @brief 生成 appkey
 * @return
 */
char *generate_app_key(struct bmyapp_s *app);

/**
 * @brief 生成 app 的私钥
 * @return
 */
char *generate_app_secret_key(struct bmyapp_s *app);

/**
 * @brief 创建应用
 * @param appname 应用名称
 * @param applyuserid 应用申请人的 id
 * @param appoveuserid 应用批准人的 id
 * @param permission 应用权限
 * @return 成功返回 0，否则返回负数
 */
int create_new_app(char *appname, char *applyuserid, char *appoveuserid, unsigned int permission);

enum bmyapp_query_mode {
	BMYAPP_QUERYMODE_APPKEY,
	BMYAPP_QUERYMODE_APPNAME,
	BMYAPP_QUERYMODE_USERID
};

/**
 * @brief 查找应用
 * @param querystring 查找的字符串
 * @param querymode 查找的模式
 * @return
 */
struct bmyapp_s *get_app(char *querystring, int querymode);
#endif
