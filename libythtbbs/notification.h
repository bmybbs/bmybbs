/* notification.c */
#ifndef __NOTIFICATION_H
#define __NOTIFICATION_H

enum {
	NOTIFY_TYPE_POST = '0'
};

struct NotifyItem {
	char from_userid[16];
	char time_str[32];		 // e.g. Thu Sep 12 18:28:57 2013
	char *link_gbk;
	struct NotifyItem * next;
};

typedef struct NotifyItem* NotifyItemList;

/** 新增一条回帖提醒
 *
 * @param to_userid 通知送达的id
 * @param from_userid 通知来自的id
 * @param board 回帖所在的版面
 * @param article_id 回帖的时间戳
 * @param title_utf8 帖子的标题 @warning utf8编码，否则 libxml2 处理中会出错
 * @return 添加成功返回0
 * @see is_post_in_notification(char * userid, char * board, int article_id)
 * @see del_post_notification(char * userid, char * board, int article_id)
 */
int add_post_notification(char * to_userid,
						  char * from_userid,
						  char * board,
						  int article_id,
						  char * title_utf8);

/** 将通知解析到内存中
 *
 * @param userid 用户 id
 * @return struct NotifyItem 链表
 */
NotifyItemList parse_notification(char *userid);

/** 计算用户的通知条数
 *
 * @param userid 用户 id
 * @return 通知条数，该值 >= 0
 */
int count_notification_num(char *userid);

/** 检验某篇文章是否在消息列表中
 * 该方法不确定是否需要。
 * @param userid 用户 id
 * @param board 版面名称
 * @param article_id 帖子id
 * @return 若帖子存在在通知文件中，则返回1，否则返回0
 * @see del_post_notification(char * userid, char * board, int article_id)
 */
int is_post_in_notification(char * userid, char * board, int article_id);

/** 删除一条回帖提醒
 *
 * @param userid
 * @param board
 * @param article_id
 * @return <ul><li>0: 删除成功</li><li>-1: 帖子不在提醒文件中</li><li>-2: 删除失败</li></ul>
 * @see is_post_in_notification(char * userid, char * board, int article_id)
 */
int del_post_notification(char * userid, char * board, int article_id);

#endif
