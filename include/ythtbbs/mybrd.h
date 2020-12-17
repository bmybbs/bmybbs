#ifndef YTHTBBS_MYBRD_H
#define YTHTBBS_MYBRD_H
#include <stdbool.h>
#include "config.h"

/**
 * 定制版面的代码, 取自fb2000.dhs.org.
 * @author ecnegrevid
 */
struct goodboard {
	char ID[GOOD_BRD_NUM][32]; ///< 在 struct boardheader.filename 里定义的长度是 24
	int num;
};

/**
 * @brief 用于处理版面读取权限的回调函数
 * 由于本库函数功能有限，版面读取权限需要标志位、封闭俱乐部权限两方面
 * 数据处理，因此通过回调函数，在运行时态交给具体调用方来处理
 * @param userid
 * @param boardname
 * @return true: 有权限
 */
typedef bool (*ythtbbs_mybrd_has_read_perm)(const char *userid, const char *boardname);

void ythtbbs_mybrd_load(const char *userid, struct goodboard *mybrd, ythtbbs_mybrd_has_read_perm func);

/**
 * 保存用户订阅版面
 * @return 实际保存的个数
 */
int ythtbbs_mybrd_save(const char *userid, struct goodboard *mybrd, ythtbbs_mybrd_has_read_perm func);
/**
 * 向列表中追加版面名称
 * @warning 本函数不做版面读取权限校验，在应用中自行判断
 */
void ythtbbs_mybrd_append(struct goodboard *mybrd, const char *boardname);
#endif

