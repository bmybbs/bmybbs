/* notification.c */
#ifndef __NOTIFICATION_H
#define __NOTIFICATION_H

enum {
	NOTIFY_TYPE_POST = '0'
};

/** 新增一条回帖提醒
 *
 * @param to_userid 通知送达的id
 * @param from_userid 通知来自的id
 * @param board 回帖所在的版面
 * @param article_id 回帖的时间戳
 * @param title_utf8 帖子的标题 @warning utf8编码，否则 libxml2 处理中会出错
 * @return 添加成功返回0
 */
int add_post_notification(char * to_userid,
						  char * from_userid,
						  char * board,
						  int article_id,
						  char * title_utf8);

#endif
