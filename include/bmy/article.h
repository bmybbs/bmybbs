#ifndef BMY_ARTICLE_H
#define BMY_ARTICLE_H
#include <time.h>
#include "ythtbbs/article.h"

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
	int boardnum;
	char boardname_en[40];
	char boardname_zh[40];

	struct fileheader_utf *next;
};

struct bmy_articles {
	struct fileheader_utf *articles;
	size_t count;
};

void copy_to_utf_header(struct fileheader_utf *dest, struct fileheader *src);

/**
 * 添加评论，讨论数 + 1
 */
void bmy_article_add_comment(int boardnum, time_t tid);

/**
 * 删除评论，讨论数 - 1
 */
void bmy_article_del_comment(int boardnum, time_t tid);

void bmy_article_add_thread(int boardnum, time_t tid, char *title_gbk, char *author, int accessed);

void bmy_article_del_thread(int boardnum, time_t tid);

void bmy_article_update_thread_title(int boardnum, time_t tid, char *title_gbk);

void bmy_article_update_thread_accessed(int boardnum, time_t tid, int accessed);

struct bmy_articles *bmy_article_list_subscription(const char *userid, size_t limit, size_t offset);
struct bmy_articles *bmy_article_list_subscription_by_time(const char *userid, size_t limit, time_t t);

void bmy_article_list_free(struct bmy_articles *ptr);
#endif

