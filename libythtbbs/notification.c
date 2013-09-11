#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "ythtbbs.h"

int add_post_notification(char * to_userid, char * from_userid, char * board,
						  int article_id, char * title_utf8) {
	int ulock = userlock(to_userid, LOCK_EX);
	char notify_file_path_old[80], notify_file_path_new[80],article_id_str[16];
	sethomefile(notify_file_path_old, to_userid, "Notification");
	sprintf(notify_file_path_new, "%s.new", notify_file_path_old);
	sprintf(article_id_str, "%d", article_id);

	xmlDocPtr doc;
	xmlNodePtr root;
	const char *empty_doc_string = "<Notify />";
	xmlDocPtr empty_doc = xmlParseMemory(empty_doc_string, strlen(empty_doc_string));

	if(access(notify_file_path_old, F_OK)) { // file exists
		doc = xmlParseFile(notify_file_path_old);
		if(doc == NULL) { // 文件解析出错
			doc = empty_doc;
		} else {
			root = xmlDocGetRootElement(doc);
			if(root == NULL) { // 获取根节点失败
				doc = empty_doc;
			}
		}
	} else {
		doc = empty_doc;
	}

	root = xmlDocGetRootElement(doc);

	xmlNodePtr notify_item = xmlNewChild(root, NULL, (const xmlChar*)"Item", NULL);
	xmlNewProp(notify_item, (const xmlChar*)"type", (const xmlChar*)NOTIFY_TYPE_POST);
	xmlNewProp(notify_item, (const xmlChar*)"board", (const xmlChar*)board);
	xmlNewProp(notify_item, (const xmlChar*)"aid", (const xmlChar*)article_id_str);
	xmlNewProp(notify_item, (const xmlChar*)"uid", (const xmlChar*)from_userid);
	xmlNewProp(notify_item, (const xmlChar*)"title", (const xmlChar*)title_utf8);

	// 暂未判断磁盘写不下的情况，姑且认为 bmy 硬盘永远足够 IronBlood@bmy 20130912
	xmlSaveFile(notify_file_path_new, doc);
	rename(notify_file_path_new, notify_file_path_old);

	userunlock(to_userid, ulock);

	return 0;
}
