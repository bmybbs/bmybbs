/* notification.c */
#ifndef __NOTIFICATION_H
#define __NOTIFICATION_H

const char *NOTIFILE = "Notification";

enum {
	NOTIFY_TYPE_POST = '0'
};

struct NotifyItem {
	char from_userid[16];
	char *title_gbk;
	time_t noti_time;
	int type;
	struct NotifyItem * next;
};

typedef struct NotifyItem* NotifyItemList;

/** 新增一条回帖提醒
 *
 * @param to_userid 通知送达的id
 * @param from_userid 通知来自的id
 * @param board 回帖所在的版面
 * @param article_id 回帖的时间戳
 * @param title_gbk 帖子的标题 @warning gbk 编码，方便调用，内部处理的时候会转为 utf-8 编码
 * @return 添加成功返回0
 * @see is_post_in_notification(char * userid, char * board, int article_id)
 * @see del_post_notification(char * userid, char * board, int article_id)
 */
int add_post_notification(char * to_userid,
						  char * from_userid,
						  char * board,
						  int article_id,
						  char * title_gbk);

/** 将通知解析到内存中
 *
 * @param userid 用户 id
 * @return struct NotifyItem 链表
 */
NotifyItemList parse_notification(char *userid);

/** 释放 NotifyItemList 内存
 *
 * @param niList
 */
void free_notification(NotifyItemList niList);

/** 计算用户的通知条数
 * 该方法使用 libxml2 解析 '/Notify/Item' 的个数。
 * @param userid 用户 id
 * @return 通知条数，该值 >= 0
 */
int count_notification_num(char *userid);

/** 检验某篇文章是否在消息列表中
 * 该方法不使用 libxml2 的方法，仅用于快速判断特征字符串是否存在于通知文件中。
 * @warn 可能存在误判。
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
