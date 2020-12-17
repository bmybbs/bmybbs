#ifndef YTHTBBS_MYBRD_H
#define YTHTBBS_MYBRD_H
#include <stdbool.h>
#include "config.h"

/**
 * 定制版面的代码, 取自fb2000.dhs.org.
 * @author ecnegrevid
 */
struct goodboard {
	char ID[GOOD_BRD_NUM][20]; ///< 版名最多看来是17+1字节
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
#endif

