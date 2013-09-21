#include "ythtlib.h"
char *
Ctime(time_t clock)
{
	char *tmp;
	char *ptr = ctime(&clock);
	tmp = strchr(ptr, '\n');
	if (NULL != tmp)
		*tmp = 0;
	return ptr;
}

char * Difftime(time_t compared_time) {
	static char ret[64];
	time_t now = time(NULL);
	int now_i = (int)now;
	int tgt_i = (int)compared_time;

	int diff_i = abs(now_i - tgt_i);

	if(diff_i < 60) { //一分钟内
		sprintf(ret, "%d秒种%s", diff_i, (now_i>tgt_i) ? "前" : "后");
	} else if (diff_i < 3600) { // 一小时内
		sprintf(ret, "%d分钟%s", diff_i/60, (now_i>tgt_i) ? "前" : "后");
	} else if (diff_i < 86400) { // 一天以内
		sprintf(ret, "%d小时%s", diff_i/3600, (now_i>tgt_i) ? "前" : "后");
	} else { // 超过一天
		sprintf(ret, "%d天%s", diff_i/86400, (now_i>tgt_i) ? "前" : "后");
	}

	return ret;
}
