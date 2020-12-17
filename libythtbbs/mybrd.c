#include <stdio.h>
#include <string.h>
#include "ytht/strlib.h"
#include "ythtbbs/user.h"
#include "ythtbbs/mybrd.h"

static const char *GOODBRD = ".goodbrd";

void ythtbbs_mybrd_load(const char *userid, struct goodboard *mybrd, ythtbbs_mybrd_has_read_perm func) {
	char buf[STRLEN];
	FILE *fp;

	if (func == NULL)
		return;

	sethomefile_s(buf, sizeof(buf), userid, GOODBRD);
	if ((fp = fopen(buf, "r"))) {
		for (mybrd->num = 0; mybrd->num < GOOD_BRD_NUM;) {
			if (!fgets(buf, sizeof(buf), fp))
				break;

			ytht_strsncpy(mybrd->ID[mybrd->num], ytht_strtrim(buf), sizeof(mybrd->ID[mybrd->num]));
			if (func(userid, mybrd->ID[mybrd->num])) {
				mybrd->num = mybrd->num + 1;
			}
		}

		fclose(fp);
	}

	if (mybrd->num == 0) {
		// 这里重构时就省略 currboard 步骤了
		// 如果按照文档安装程序，DEFAULTBOARD 也就是 "sysop" 版面是默认存在的
		// 如果变更了 DEFAULTBOARD，请提前在 .BOARDS 中添加对应的版面
		if (ythtbbs_cache_Board_get_board_by_name(DEFAULTBOARD)) {
			strcpy(mybrd->ID[0], DEFAULTBOARD);
			mybrd->num = 1;
		}
	}
}

