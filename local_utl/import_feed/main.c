#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include "config.h"
#include "ytht/ytht.h"
#include "ythtbbs/article.h"
#include "ythtbbs/misc.h"

struct fileheader_utf {
	time_t filetime;
	time_t edittime;
	time_t thread;
	unsigned int accessed;
	char title[120];
	char owner[14];
	unsigned short viewtime;
	unsigned char sizebyte;
	unsigned char staravg50;
	unsigned char hasvoted;
	char deltime;
	unsigned int count;
};

struct list_item {
	const char *title;
	unsigned idx;
	struct list_item *next;
};

static void insert_thread_title(struct list_item **thread_titles, const char *title, unsigned idx) {
	struct list_item *item = malloc(sizeof(struct list_item));
	struct list_item *ptr;

	item->title = title;
	item->idx = idx;
	item->next = NULL;

	if ((ptr = thread_titles[(unsigned char)title[0]]) == NULL) {
		thread_titles[(unsigned char)title[0]] = item;
	} else {
		while (ptr->next != NULL)
			ptr = ptr->next;

		ptr->next = item;
	}
}

static unsigned int find_likely_thread_idx(struct list_item **thread_titles, const char *title, unsigned int idx) {
	struct list_item *ptr;
	size_t len;
	unsigned int list_idx = (unsigned char)title[4]; // "Re: "

	ptr = thread_titles[list_idx];
	while (ptr != NULL) {
		// 第一次完整匹配
		if (ptr->idx < idx && strcmp(ptr->title, title + 4) == 0)
			return ptr->idx;
		else
			ptr = ptr->next;
	}

	ptr = thread_titles[list_idx];
	while (ptr != NULL) {
		// 第二次模糊匹配，可能出现乱码导致回帖标题被截断
		len = strlen(ptr->title) * 0.6;
		if (ptr->idx < idx && strncmp(ptr->title, title + 4, len) == 0)
			return ptr->idx;
		else
			ptr = ptr->next;
	}

	// 没有找到返回 idx
	return idx;
}

static void copy_to_utf_header(struct fileheader_utf *dest, struct fileheader *src);

static int count_threads_of_board(const char *boardname) {
	char buf[256];
	struct mmapfile mf = { .ptr = NULL };
	struct fileheader *ptr_header = NULL;
	struct fileheader_utf *threads = NULL;

	struct list_item *thread_titles[256];
	unsigned int i, j;
	unsigned int post_count = 0;
	unsigned int thread_count = 0;
	unsigned int likely_idx = 0;

	memset(thread_titles, 0, sizeof(void *) * 256);
	snprintf(buf, sizeof(buf), MY_BBS_HOME "/boards/%s/.DIR", boardname);
	if (mmapfile(buf, &mf) == -1) {
		fprintf(stderr, "%s doesn't exist\n", buf);
		return -1;
	}

	post_count = mf.size / sizeof(struct fileheader);
	fprintf(stderr, "There're %u posts on board %s\n", post_count, boardname);

	for (i = 0; i < post_count; i++) {
		ptr_header = &((struct fileheader *) mf.ptr)[i];
		if (ptr_header->thread == ptr_header->filetime)
			thread_count++;
	}
	fprintf(stderr, "There're %u threads on board %s\n", thread_count, boardname);

	threads = malloc(thread_count * sizeof(struct fileheader_utf));
	for (i = 0, j = 0; i < post_count; i++) {
		ptr_header = &((struct fileheader *) mf.ptr)[i];
		if (ptr_header->thread != ptr_header->filetime) {
			continue; // TODO count + 1
		}

		copy_to_utf_header(&threads[j], ptr_header);
		if (strncmp(ptr_header->title, "Re: ", 4) == 0) {
			likely_idx = find_likely_thread_idx(thread_titles, threads[j].title, i);
			if (likely_idx < i) {
				fprintf(stdout, "[%s-%d] 可能的主题贴为：%d(%ld)\n", boardname, i+1, likely_idx+1, ((struct fileheader *) mf.ptr)[likely_idx].filetime);
			} else {
				fprintf(stdout, "[%s-%d] 没有找到可能的主题帖\n", boardname, i+1);
			}
		} else {
			insert_thread_title(thread_titles, threads[j].title, i);
		}

		j++;
	}

	for (j = 0; j < 5 && j < thread_count; j++) {
		fprintf(stderr, "[%s-%d] %s\n", boardname, j, threads[j].title);
	}

	mmapfile(NULL, &mf);
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc > 3) {
		if (strcmp(argv[1], "test") == 0 && strcmp(argv[2], "board") == 0) {
			count_threads_of_board(argv[3]);
		}
	}
	return 0;
}

static void copy_to_utf_header(struct fileheader_utf *dest, struct fileheader *src) {
	memset(dest, 0, sizeof(struct fileheader_utf));
	dest->filetime = src->filetime;
	dest->edittime = src->edittime;
	dest->thread   = src->thread;
	dest->accessed = src->accessed;
	dest->sizebyte = src->sizebyte;
	dest->viewtime = src->viewtime;
	dest->hasvoted = src->hasvoted;
	dest->deltime  = src->deltime;
	dest->staravg50 = src->staravg50;
	memcpy(dest->owner, src->owner, 14);

	int len = strlen(src->title);
	if (len > 60)
		len = 60;
	g2u(src->title, len, dest->title, 120);

	dest->count = 1; // the first one!
}

