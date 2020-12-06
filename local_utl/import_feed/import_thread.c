#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ytht/fileop.h"
#include "ythtbbs/cache.h"
#include "ythtbbs/misc.h"
#include "ythtbbs/article.h"
#include "common.h"
#include "db.h"

struct virtual_board {
	bool should_be_deallocated;
	struct fileheader_utf root; // never change
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
		else
			left = mid;
	}

	return -1;
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

	fprintf(stdout, "start load threads from %s\n", board->header.filename);
	snprintf(buf, sizeof(buf), MY_BBS_HOME "/boards/%s/.DIR", board->header.filename);
	if (mmapfile(buf, &mf) == -1) {
		fprintf(stderr, "%s doesn't exist\n", buf);
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
		if (threads[j].filetime < threads[j+1].filetime) {
			thread_already_sorted = false;
			break;
		}
	}

	// 如果主题列表不是有序的，利用 stdlib 中的快排
	if (!thread_already_sorted) {
		qsort(threads, thread_count, sizeof (struct fileheader), compare_thread_time);
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
	boards[curr_idx].root.next = threads;
	fprintf(stdout, "loaded %d threads from %s\n", thread_count, board->header.filename);
	return 0;
}

int import_thread(void) {

	ythtbbs_cache_utmp_resolve();
	ythtbbs_cache_UserTable_resolve();
	ythtbbs_cache_Board_resolve();

	unsigned int board_count, i;
	struct virtual_board *boards;

	board_count = ythtbbs_cache_Board_get_number();
	boards = calloc(board_count, sizeof (struct virtual_board));

	ythtbbs_cache_Board_foreach_v(load_threads_by_board, boards);
	// TODO merging and insert into database

	// 最后释放资源
	for (i = 0; i < board_count; i++) {
		free(boards[i].root.next);
	}
	free(boards);
	return 0;
}

