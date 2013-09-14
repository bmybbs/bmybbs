#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <sys/mman.h>
#include <time.h>

#include "ythtbbs.h"

const char *NOTIFILE = "Notification";

struct NotifyItem {
	char from_userid[16];
	char *title_gbk;
	time_t noti_time;
	int type;
	struct NotifyItem * next;
};

static struct NotifyItem * parse_to_item(xmlNodePtr xmlItem);
static void addItemtoList(NotifyItemList *List, struct NotifyItem *Item);

int add_post_notification(char * to_userid, char * from_userid, char * board,
						  int article_id, char * title_gbk) {
	char * title_utf8 = malloc(2*strlen(title_gbk));
	if(title_utf8 == NULL)
		return -1;

	g2u(title_gbk, strlen(title_gbk), title_utf8, 2*strlen(title_gbk));

	int ulock = userlock(to_userid, LOCK_EX);
	char notify_file_path[80], article_id_str[16];
	sethomefile(notify_file_path, to_userid, NOTIFILE);
	sprintf(article_id_str, "%d", article_id);

	xmlDocPtr doc;
	xmlNodePtr root;
	const char *empty_doc_string = "<Notify />";
	xmlDocPtr empty_doc = xmlParseMemory(empty_doc_string, strlen(empty_doc_string));

	if(access(notify_file_path, F_OK)) { // file exists
		doc = xmlParseFile(notify_file_path);
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
	free(title_utf8);
	xmlSaveFileEnc(notify_file_path, doc, "UTF8");
	xmlFreeDoc(doc);
	userunlock(to_userid, ulock);

	return 0;
}

int count_notification_num(char *userid) {
	char notify_file_path[80];
	sethomefile(notify_file_path, userid, NOTIFILE);
	const xmlChar *xpathExpr = (xmlChar *)"/Notify/Item";

	xmlDocPtr doc = xmlParseFile(notify_file_path);
	if(NULL == doc) { // 不存在通知文件
		return 0;
	}

	xmlNodePtr root = xmlDocGetRootElement(doc);
	if(NULL == root) { // xml 格式不正确
		xmlFreeDoc(doc);
		return 0;
	}

	xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
	xmlXPathObjectPtr res_obj = xmlXPathEvalExpression(xpathExpr, ctx);
	int res_num = (res_obj->nodesetval) ? res_obj->nodesetval->nodeNr : 0;

	xmlXPathFreeObject(res_obj);
	xmlXPathFreeContext(ctx);
	xmlFreeDoc(doc);

	return res_num;
}

int is_post_in_notification(char * userid, char *board, int article_id) {
	char notify_file_path[80], search_str[80], *p;
	struct stat statbuf;
	int fd;

	sethomefile(notify_file_path, userid, NOTIFILE);

	fd = open(notify_file_path, O_RDONLY);
	if(fd == -1) { // 文件不存在
		return 0;
	}

	if(fstat(fd, &statbuf) == -1) {
		close(fd);
		return 0;
	}

	if(!S_ISREG(statbuf.st_mode)) { // not a file
		close(fd);
		return 0;
	}

	p = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);

	if(p == MAP_FAILED) { // mmap 隐射失败
		return 0;
	}

	sprintf(search_str, "<Item type=\"0\" board=\"%s\" aid=\"%d\" ", board, article_id);
	char *r = strstr(p, search_str);
	munmap(p, statbuf.st_size);

	return (r!=NULL);
}

NotifyItemList parse_notification(char *userid) {

	NotifyItemList niList = NULL;
	const xmlChar * xpathExpr = (xmlChar *)"/Notify/Item";
	char notify_file_path[80];
	int i;
	sethomefile(notify_file_path, userid, NOTIFILE);

	xmlDocPtr doc = xmlParseFile(notify_file_path);
	if(NULL == doc) { // 不存在文件
		return NULL;
	}

	xmlNodePtr root = xmlDocGetRootElement(doc);
	if(NULL == root) { // 格式不正确
		xmlFreeDoc(doc);
		return NULL;
	}

	xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
	xmlXPathObjectPtr res_obj = xmlXPathEvalExpression(xpathExpr, ctx);
	int res_num = (res_obj->nodesetval) ? res_obj->nodesetval->nodeNr : 0;

	if(res_num!=0) {
		for(i=0; i<res_num; ++i) {
			struct NotifyItem * item = parse_to_item(res_obj->nodesetval->nodeTab[i]);
			addItemtoList(&niList, item);
		}
	} // res_num == 0, niList 依然为 NULL

	xmlXPathFreeObject(res_obj);
	xmlXPathFreeContext(ctx);
	xmlFreeDoc(doc);

	return niList;
}

void free_notification(NotifyItemList niList) {
	if(niList == NULL)
		return;

	struct NotifyItem * cur = niList;
	struct NotifyItem * next = cur->next;
	while(next!=NULL) {
		free(cur->title_gbk);
		free(cur);
		cur = next;
		next = cur->next;
	}
}

int del_post_notification(char * userid, char * board, int article_id) {
	char notify_file_path[80], search_str[96];
	sprintf(search_str, "/Notify/Item[@board=\"%s\" and @aid=\"%d\"]", board, article_id);
	sethomefile(notify_file_path, userid, NOTIFILE);

	int ulock = userlock(userid, LOCK_EX); // todo
	xmlDocPtr doc = xmlParseFile(notify_file_path);
	if(NULL == doc) { // 文件不存在
		userunlock(userid, ulock);
		return -1;
	}

	xmlNodePtr root = xmlDocGetRootElement(doc);
	if(NULL == root) { // 格式不正确
		xmlFreeDoc(doc);
		userunlock(userid, ulock);
		return -1;
	}

	xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
	xmlXPathObjectPtr res_obj = xmlXPathEvalExpression((const xmlChar*)search_str, ctx);

	if(res_obj->nodesetval) {
		xmlNodePtr curr;
		int res_num = res_obj->nodesetval->nodeNr;
		int i;
		for(i=0; i<res_num; ++i) { // 依次删除 xpath 选中的 node
			curr = res_obj->nodesetval->nodeTab[i];
			xmlUnlinkNode(curr);
			xmlFreeNode(curr);
		}
	}

	xmlSaveFileEnc(notify_file_path, doc, "UTF8");
	xmlXPathFreeObject(res_obj);
	xmlXPathFreeContext(ctx);
	xmlFreeDoc(doc);
	userunlock(userid, ulock);

	return 0;
}

static struct NotifyItem * parse_to_item(xmlNodePtr xmlItem) {
	if(xmlItem == NULL)
		return NULL;

	struct NotifyItem * item = malloc(sizeof(struct NotifyItem));
	if(item==NULL)
		return NULL;

	memset(item, 0, sizeof(struct NotifyItem));

	// these vars need to be free!!!
	xmlChar *xml_str_type = xmlGetProp(xmlItem, (const xmlChar *)"type");
	item->type = atoi((char *)xml_str_type);
	xmlFree(xml_str_type);

	xmlChar *xml_str_userid = xmlGetProp(xmlItem, (const xmlChar *)"uid");
	memcpy(item->from_userid, (char *)xml_str_userid, 16);
	xmlFree(xml_str_userid);

	xmlChar *xml_str_timestamp = xmlGetProp(xmlItem, (const xmlChar *)"aid");
	item->noti_time = (time_t) atoi((char *)xml_str_timestamp);
	xmlFree(xml_str_timestamp);

	xmlChar * xml_str_title_utf8 = xmlGetProp(xmlItem, (const xmlChar *)"title");
	size_t title_len = strlen((char *)xml_str_title_utf8);
	item->title_gbk = malloc(title_len);  //gbk长度<utf8
	if(item->title_gbk == NULL) { //内存分配失败
		xmlFree(xml_str_title_utf8);
		free(item);
		return NULL;
	}
	u2g((char *)xml_str_title_utf8, title_len, item->title_gbk, title_len);
	xmlFree(xml_str_title_utf8);

	return item;
}

static void addItemtoList(NotifyItemList *List, struct NotifyItem *Item) {
	if(NULL==Item)
		return;

	if(*List == NULL)
		*List = Item;
	else {
		struct NotifyItem * cur = (struct NotifyItem *)*List;

		while(cur->next)
			cur = cur->next;

		cur->next = Item;
	}
}
