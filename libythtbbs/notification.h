/* notification.c */
#ifndef __NOTIFICATION_H
#define __NOTIFICATION_H

enum {
	NOTIFY_TYPE_NONSPECIFIED = -1,	// ����ɾ���������ѵ�ʱ��ʹ��
	NOTIFY_TYPE_POST = 0
};

struct NotifyItem {
	char from_userid[16];
	char board[32];
	char *title_gbk;
	time_t noti_time;
	int type;
	struct NotifyItem * next;
};

typedef struct NotifyItem* NotifyItemList;

/** ����һ����������
 *
 * @param to_userid ֪ͨ�ʹ��id
 * @param from_userid ֪ͨ���Ե�id
 * @param board �������ڵİ���
 * @param article_id ������ʱ���
 * @param title_gbk ���ӵı��� @warning gbk ���룬������ã��ڲ������ʱ���תΪ utf-8 ����
 * @return ��ӳɹ�����0
 * @see is_post_in_notification(char * userid, char * board, int article_id)
 * @see del_post_notification(char * userid, char * board, int article_id)
 */
int add_post_notification(char * to_userid,
						  char * from_userid,
						  char * board,
						  int article_id,
						  char * title_gbk);

/** ��֪ͨ�������ڴ���
 *
 * @param userid �û� id
 * @return struct NotifyItem ����
 */
NotifyItemList parse_notification(char *userid);

/** �ͷ� NotifyItemList �ڴ�
 *
 * @param niList
 */
void free_notification(NotifyItemList niList);

/** �����û���֪ͨ����
 * �÷���ʹ�� libxml2 ���� '/Notify/Item' �ĸ�����
 * @param userid �û� id
 * @return ֪ͨ��������ֵ >= 0
 */
int count_notification_num(char *userid);

/** ����ĳƪ�����Ƿ�����Ϣ�б���
 * �÷�����ʹ�� libxml2 �ķ����������ڿ����ж������ַ����Ƿ������֪ͨ�ļ��С�
 * @warn ���ܴ������С�
 * @param userid �û� id
 * @param board ��������
 * @param article_id ����id
 * @return �����Ӵ�����֪ͨ�ļ��У��򷵻�1�����򷵻�0
 * @see del_post_notification(char * userid, char * board, int article_id)
 */
int is_post_in_notification(char * userid, char * board, int article_id);

/** ɾ��һ����������
 *
 * @param userid
 * @param board
 * @param article_id
 * @return <ul><li>0: ɾ���ɹ�</li><li>-1: ���Ӳ��������ļ���</li><li>-2: ɾ��ʧ��</li></ul>
 * @see is_post_in_notification(char * userid, char * board, int article_id)
 */
int del_post_notification(char * userid, char * board, int article_id);

/** ɾ��ĳ���û�����������
 *
 * @param userid �û�id
 * @return ɾ���ɹ�����0
 */
int del_all_notification(char *userid);

#endif
