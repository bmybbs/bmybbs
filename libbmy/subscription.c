#include <string.h>
#include <stdlib.h>
#include "bmy/mysql_wrapper.h"

struct sql_bnum_buf {
	size_t count;
	int *boardnums;
};

static void bmy_subscription_add(int usernum, int boardnum);
static void bmy_subscription_del(int usernum, int boardnum);

static void bmy_subscription_sync_callback(MYSQL_STMT *stmt, MYSQL_BIND *result_cols, void *result_set) {
	struct sql_bnum_buf *p_buf = result_set;
	size_t idx;

	p_buf->count = mysql_stmt_num_rows(stmt);
	if (p_buf->count == 0)
		return;

	p_buf->boardnums = (int *) calloc(p_buf->count, sizeof(int));
	for(idx = 0; idx < p_buf->count; idx++) {
		mysql_stmt_fetch(stmt);
		p_buf->boardnums[idx] = *(int *)result_cols[0].buffer;
	}
}

static int compar(const void *a, const void *b) {
	int *aa = (int *)a;
	int *bb = (int *)b;
	return *aa - *bb;
}

void bmy_subscription_sync(int usernum, int boardnums[], size_t nmemb) {
	MYSQL_BIND params[1], results[1];
	int bnum;
	size_t i, j;
	struct sql_bnum_buf bnum_buf;
	const char *sql_get_bnums = "SELECT boardnum from t_user_subscriptions where usernum = ?";

	memset(params, 0, sizeof(params));
	memset(results, 0, sizeof(results));
	params[0].buffer_type = MYSQL_TYPE_LONG;
	params[0].buffer = &usernum;
	params[0].buffer_length = sizeof(int);
	results[0].buffer_type = MYSQL_TYPE_LONG;
	results[0].buffer = &bnum;
	results[0].buffer_length = sizeof(int);

	bnum_buf.count = 0;
	execute_prep_stmt(sql_get_bnums, MYSQL_CHARSET_UTF8, params, results, (void *) &bnum_buf, bmy_subscription_sync_callback);

	if (bnum_buf.count == 0) {
		// 不存在原始记录，逐一添加
		for (i = 0; i < nmemb; i++) {
			bmy_subscription_add(usernum, boardnums[i]);
		}
	} else {
		// 交叉对比
		qsort(boardnums, nmemb, sizeof(int), compar);
		qsort(bnum_buf.boardnums, bnum_buf.count, sizeof(int), compar);

		i = 0;
		j = 0;

		while (i < nmemb && j < bnum_buf.count) {
			if (boardnums[i] == bnum_buf.boardnums[j]) {
				// 共同存在，不做处理
				i++;
				j++;
				continue;
			} else if (boardnums[i] < bnum_buf.boardnums[j]) {
				// 新增
				bmy_subscription_add(usernum, boardnums[i]);
				i++;
				continue;
			} else {
				// 也就是 boardnums[i] > bnum_buf.boardnums[j] 的情况，删除
				bmy_subscription_del(usernum, bnum_buf.boardnums[j]);
				j++;
				continue;
			}
		}

		// 队列中剩余的项目，两种情况应该只存在一种
		for (; i < nmemb; i++) {
			bmy_subscription_add(usernum, boardnums[i]);
		}
		for (; j < bnum_buf.count; j++) {
			bmy_subscription_del(usernum, bnum_buf.boardnums[j]);
		}

		free(bnum_buf.boardnums);
	}
}

static void bmy_subscription_internal_call(const char *sql, int usernum, int boardnum) {
	MYSQL_BIND params[2];

	memset(params, 0, sizeof(params));
	params[0].buffer_type = MYSQL_TYPE_LONG;
	params[0].buffer = &usernum;
	params[0].buffer_length = sizeof(int);
	params[1].buffer_type = MYSQL_TYPE_LONG;
	params[1].buffer = &boardnum;
	params[1].buffer_length = sizeof(int);

	execute_prep_stmt(sql, MYSQL_CHARSET_UTF8, params, NULL, NULL, NULL);
}

static void bmy_subscription_add(int usernum, int boardnum) {
	const char *sql = "INSERT INTO t_user_subscriptions(usernum, boardnum) value(?, ?)";
	bmy_subscription_internal_call(sql, usernum, boardnum);
}

static void bmy_subscription_del(int usernum, int boardnum) {
	const char *sql = "DELETE FROM t_user_subscriptions WHERE usernum = ? AND boardnum = ?";
	bmy_subscription_internal_call(sql, usernum, boardnum);
}

