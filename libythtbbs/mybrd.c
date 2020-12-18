#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include "ytht/strlib.h"
#include "bmy/subscription.h"
#include "ythtbbs/user.h"
#include "ythtbbs/mybrd.h"

static const char *GOODBRD = ".goodbrd";

static void ythtbbs_mybrd_sync(const char *userid, ythtbbs_mybrd_has_read_perm func);

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

int ythtbbs_mybrd_save(const char *userid, struct goodboard *mybrd, ythtbbs_mybrd_has_read_perm func) {
	int i;
	int count = 0;
	char buf[STRLEN];
	FILE *fp;
	struct boardmem *x;

	if (func == NULL)
		return count;

	sethomefile_s(buf, sizeof(buf), userid, GOODBRD);
	fp = fopen(buf, "w");
	if (fp) {
		flock(fileno(fp), LOCK_EX);
		for (i = 0; i < mybrd->num && i < GOOD_BRD_NUM; i++) {
			if (!func(userid, mybrd->ID[i]))
				continue;

			x = ythtbbs_cache_Board_get_board_by_name(mybrd->ID[i]);
			fprintf(fp, "%s\n", x->header.filename);
			count++;
		}
		fclose(fp);
	}

	ythtbbs_mybrd_sync(userid, func);
	return count;
}

void ythtbbs_mybrd_append(struct goodboard *mybrd, const char *boardname) {
	if (mybrd == NULL || mybrd->num >= GOOD_BRD_NUM || boardname == NULL || boardname[0] == '\0')
		return;

	ytht_strsncpy(mybrd->ID[mybrd->num], boardname, sizeof(mybrd->ID[mybrd->num]));
	mybrd->num = mybrd->num + 1;
}

void ythtbbs_mybrd_remove(struct goodboard *mybrd, const char *boardname) {
	int i;

	if (mybrd == NULL || mybrd->num >= GOOD_BRD_NUM || boardname == NULL || boardname[0] == '\0')
		return;

	for (i = 0; i < mybrd->num; i++) {
		if (!strcasecmp(boardname, mybrd->ID[i]))
			break;
	}

	if (i < mybrd->num) {
		// 表明存在记录
		if (i == mybrd->num - 1) {
			// 如果是最后一条记录
			mybrd->ID[i][0] = '\0';
		} else {
			// 依次拷贝覆盖前面的记录
			for (; i < mybrd->num - 1; i++) {
				strcpy(mybrd->ID[i], mybrd->ID[i + 1]);
			}
		}

		mybrd->num = mybrd->num - 1;
	}
}

bool ythtbbs_mybrd_exists(struct goodboard *mybrd, const char *boardname) {
	int i;

	if (mybrd == NULL || mybrd->num >= GOOD_BRD_NUM || boardname == NULL || boardname[0] == '\0')
		return false;

	for (i = 0; i < mybrd->num; i++) {
		if (!strcasecmp(boardname, mybrd->ID[i]))
			return true;
	}

	return false;
}

static void ythtbbs_mybrd_sync(const char *userid, ythtbbs_mybrd_has_read_perm func) {
	// 本函数只在 save 里调用，这里暂时不验证 userid 是否存在了...
	struct goodboard local_mybrd;
	int *bnums, i;

	memset(&local_mybrd, 0, sizeof(struct goodboard));
	ythtbbs_mybrd_load(userid, &local_mybrd, func);

	if (local_mybrd.num == 0)
		return;

	bnums = calloc(local_mybrd.num, sizeof(int));
	for (i = 0; i < local_mybrd.num; i++) {
		bnums[i] = 1 + ythtbbs_cache_Board_get_idx_by_name(local_mybrd.ID[i]);
	}
	bmy_subscription_sync(1 + ythtbbs_cache_UserIDHashTable_find_idx(userid), bnums, local_mybrd.num);

	free(bnums);
}

