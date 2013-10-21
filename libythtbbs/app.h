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
};

/** appid hash table
 * 位于共享内存中，用于快速索引查找 appid
 */
struct appidhashitem {
	int num;
	char appkey[APPKEYLENGTH];
	char appname[APPNAMELENGTH];
};

char *generate_api_key();
char *generate_api_secret_key();

int create_new_app(char *appname, char *userid, char *);
struct bmyapp_s *get_app(char *appname);
#endif
