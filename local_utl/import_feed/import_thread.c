#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ytht/fileop.h"
#include "bmy/article.h"
#include "bmy/board.h"
#include "ythtbbs/cache.h"
#include "ythtbbs/misc.h"
#include "ythtbbs/article.h"
#include "common.h"
#include "db.h"

struct virtual_board {
	bool should_be_deallocated;
	struct fileheader_utf root; // never change
	unsigned long total;
};

static int compare_thread_time(const void *v1, const void *v2) {
	const struct fileheader_utf *h1 = (const struct fileheader_utf *) v1;
	const struct fileheader_utf *h2 = (const struct fileheader_utf *) v2;

	// h1->filetime != h2->filetime
	if (h1->filetime < h2->filetime)
		return -1;
	else
		return 1;
}

static int bfind_thread_idx(struct fileheader_utf *threads, time_t target, unsigned int left, unsigned int right) {
	unsigned int mid;

	// out of range
	if (threads[left].filetime > target || threads[right].filetime < target)
		return -1;

	while (left < right) {
		mid = (left + right) / 2;

		if (threads[mid].filetime == target)
			return mid;

		if (threads[mid].filetime > target)
			right = mid;
		else {
			if (left == mid)
				break;
			else
				left = mid;
		}
	}

	if (threads[left].filetime == target) {
		return left;
	} else if (threads[right].filetime == target) {
		return right;
	} else {
		return -1;
	}
}

static int load_threads_by_board(struct boardmem *board, int curr_idx, va_list ap) {
	char buf[256];
	struct mmapfile mf = { .ptr = NULL };
	struct fileheader *ptr_header = NULL;
	struct fileheader_utf *threads = NULL;
	struct virtual_board *boards = va_arg(ap, struct virtual_board *);

	unsigned int post_count = 0;
	unsigned int thread_count = 0;
	unsigned int i, j;
	int idx;

	bool thread_already_sorted = true;

	if (bmy_board_is_system_board(board->header.filename))
		return 0;

	fprintf(stdout, "start load threads from %s, ", board->header.filename);
	snprintf(buf, sizeof(buf), MY_BBS_HOME "/boards/%s/.DIR", board->header.filename);
	if (mmapfile(buf, &mf) == -1) {
		fprintf(stdout, "%s doesn't exist\n", buf);
		return -1;
	}

	// 第一次遍历，先计数便于分配存储空间
	post_count = mf.size / sizeof(struct fileheader);
	for (i = 0; i < post_count; i++) {
		ptr_header = &((struct fileheader*) mf.ptr)[i];
		if (ptr_header->thread == ptr_header->filetime)
			thread_count++;
	}

	// 第二次遍历，拷贝主题帖，检查主题的 timestamp 是否存在错乱
	threads = malloc(thread_count * sizeof(struct fileheader_utf));
	for (i = 0, j = 0; i < post_count; i++) {
		ptr_header = &((struct fileheader*) mf.ptr)[i];
		if (ptr_header->thread != ptr_header->filetime) {
			continue;
		}

		copy_to_utf_header(&threads[j], ptr_header);
		threads[j].boardnum = curr_idx + 1;
		// 这里和 test_board 检查版面不同，不对主题贴做额外校验
		j++;
	}

	for (j = 0; j < thread_count - 1; j++) {
		if (threads[j].filetime > threads[j+1].filetime) {
			fprintf(stdout,"\033[1;32mwrong order?\033[0m\n");
			fprintf(stdout, "[%s-%d]\t%ld\t%s\n", board->header.filename, j, threads[j].thread, threads[j].title);
			fprintf(stdout, "[%s-%d]\t%ld\t%s\n", board->header.filename, j+1, threads[j+1].thread, threads[j].title);
			thread_already_sorted = false;
			break;
		}
	}

	// 如果主题列表不是有序的，利用 stdlib 中的快排
	if (!thread_already_sorted) {
		fprintf(stdout, "\033[1;31musing qsort to sort board %s\033[0m\n", board->header.filename);
		qsort(threads, thread_count, sizeof (struct fileheader_utf), compare_thread_time);
	}

	// 第三次遍历原始文章列表，用于统计主题讨论数
	for (i = 0; i < post_count; i++) {
		ptr_header = &((struct fileheader*) mf.ptr)[i];
		if (ptr_header->thread == ptr_header->filetime) {
			continue;
		}

		idx = bfind_thread_idx(threads, ptr_header->thread, 0, thread_count - 1);

		// thread head post was deleted
		if (idx == -1)
			continue;

		threads[idx].count++;
	}

	// 结束使用，从 mmap 中释放
	mmapfile(NULL, &mf);

	// 最后将 threads 由数组转换为链表，便于下一步合并
	for (j = 0; j < thread_count-1; j++) {
		threads[j].next = &threads[j+1];
	}
	boards[curr_idx].total = thread_count;
	boards[curr_idx].root.next = threads;
	fprintf(stdout, "loaded %d threads from %s\n", thread_count, board->header.filename);
	return 0;
}

// WARNING: 这里就不做 NULL 校验了
static struct virtual_board *do_merge(struct virtual_board *b1, struct virtual_board *b2) {
	struct fileheader_utf *it1, *it2, *it;
	struct virtual_board *b = malloc(sizeof(struct virtual_board));
	memset(b, 0, sizeof(struct virtual_board));
	b->should_be_deallocated = true;

	b->total = b1->total + b2->total;

	it  = &b->root;
	it1 = b1->root.next;
	it2 = b2->root.next;
	while (it1 && it2) {
		if (it1->thread < it2->thread) {
			it->next = it1;
			it1 = it1->next;
		} else {
			it->next = it2;
			it2 = it2->next;
		}

		it = it->next;
	}

	if (it1) {
		it->next = it1;
	} else if (it2) {
		it->next = it2;
	} else {
		fprintf(stderr, "[wtf] didn't reach at least one of the end?\n");
	}

	return b;
}

static struct virtual_board *merge_threads(struct virtual_board boards[], unsigned int left, unsigned right) {
	if (left == right) {
		return &boards[left];
	} else if (right - left == 1) {
		// the last two, merge
		return do_merge(&boards[left], &boards[right]);
	} else {
		unsigned int mid = (left + right) / 2;

		struct virtual_board *b_left = merge_threads(boards, left, mid);
		struct virtual_board *b_right = merge_threads(boards, mid + 1, right);
		struct virtual_board *b = do_merge(b_left, b_right);

		if (b_left->should_be_deallocated) free(b_left);
		if (b_right->should_be_deallocated) free(b_right);
		return b;
	}
}

/*
static void dump_mega_thread(struct virtual_board *b) {
	FILE *fp = fopen(MY_BBS_HOME "/dump_mega_thread.txt", "w");
	fprintf(fp, "===================\n");
	fprintf(fp, "here're %lu threads\n", b->total);
	fprintf(fp, "===================\n");

	struct fileheader_utf *it = b->root.next;

	while (it) {
		fprintf(fp, "%d,%ld,%s,%s,%u\n", it->boardnum, it->thread, it->title, it->owner, it->count);
		it = it->next;
	}
	fclose(fp);
}
*/

static void insert_mega_thread(struct virtual_board *b) {
	MYSQL *s;
	MYSQL_STMT *stmt;
	MYSQL_BIND params[6];
	char *anonymous = "Anonymous";
	int mysql_status = 0;

	s = mysql_init(NULL);
	mysql_options(s, MYSQL_SET_CHARSET_NAME, "utf8");
	if (!my_connect_mysql(s)) {
		return;
	}

	stmt = mysql_stmt_init(s);
	if (!stmt) {
		mysql_close(s);
		return;
	}

	const char *sql = "INSERT INTO `t_threads`(`boardnum`, `timestamp`, `title`, `author`, `comments`, `accessed`) VALUES(?, ?, ?, ?, ?, ?)";
	mysql_status = mysql_stmt_prepare(stmt, sql, strlen(sql));
	if (mysql_status != 0) {
		mysql_error_stmt(stmt);
		mysql_stmt_close(stmt);
		mysql_close(s);
		return;
	}

	struct fileheader_utf *it = b->root.next;
	while (it) {
		memset(params, 0, sizeof(params));

		params[0].buffer_type = MYSQL_TYPE_LONG;
		params[0].buffer = &it->boardnum;
		params[0].buffer_length = sizeof(int);

		params[1].buffer_type = MYSQL_TYPE_LONGLONG;
		params[1].buffer = &it->thread;
		params[1].buffer_length = sizeof(int);

		params[2].buffer_type = MYSQL_TYPE_STRING;
		params[2].buffer = it->title;
		params[2].buffer_length = strlen(it->title);

		params[3].buffer_type = MYSQL_TYPE_STRING;
		params[3].buffer = (it->owner[0] ? it->owner : anonymous);
		params[3].buffer_length = strlen(params[3].buffer);

		params[4].buffer_type = MYSQL_TYPE_LONG;
		params[4].buffer = &it->count;
		params[4].buffer_length = sizeof(int);

		params[5].buffer_type = MYSQL_TYPE_LONG;
		params[5].buffer = &it->accessed;
		params[5].buffer_length = sizeof(int);

		mysql_status = mysql_stmt_bind_param(stmt, params);
		if (mysql_status != 0) {
			mysql_error_stmt(stmt);
		} else {
			mysql_status = mysql_stmt_execute(stmt);
			if (mysql_status != 0) {
				mysql_error_stmt(stmt);
			}
		}
		it = it->next;
	}
	mysql_stmt_close(stmt);
	mysql_close(s);
	return;
}

int import_thread(void) {

	ythtbbs_cache_utmp_resolve();
	ythtbbs_cache_UserTable_resolve();
	ythtbbs_cache_Board_resolve();

	unsigned int board_count, i;
	struct virtual_board *boards;
	bool ordered = true;

	board_count = ythtbbs_cache_Board_get_number();
	boards = calloc(board_count, sizeof (struct virtual_board));

	time_t t1 = time(NULL);
	ythtbbs_cache_Board_foreach_v(load_threads_by_board, boards);
	time_t t2 = time(NULL);

	// merging and insert into database
	time_t t3 = time(NULL);
	struct virtual_board *mega_board = merge_threads(boards, 0, board_count - 1);
	time_t t4 = time(NULL);


	fprintf(stderr, "start checking mega thread order\n");
	struct fileheader_utf *it = mega_board->root.next;
	while (it && it->next) {
		if (it->thread > it->next->thread) {
			ordered = false;
			fprintf(stdout, "%d\t%ld\t%s\t%s\n", it->boardnum, it->thread, it->title, it->owner);
			fprintf(stdout, "%d\t%ld\t%s\t%s\n", it->next->boardnum, it->next->thread, it->next->title, it->next->owner);
			fprintf(stdout, "total: %lu\n", mega_board->total);
			break;
		}
		it = it->next;
	}
	fprintf(stderr, "finished checking mega thread order\n");

	if (ordered) {
		//dump_mega_thread(mega_board);
		insert_mega_thread(mega_board);
	} else {
		fprintf(stderr, "mega threads are not in ascending order\n");
	}

	// 最后释放资源
	if (mega_board->should_be_deallocated)
		free(mega_board);
	for (i = 0; i < board_count; i++) {
		free(boards[i].root.next);
	}
	free(boards);
	fprintf(stderr, "[debug] time for loading[%ld], for merging[%ld]\n", (t2 - t1), (t4 - t3));
	return 0;
}

