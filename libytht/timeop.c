#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

char *ytht_ctime(const time_t clock)
{
	char *tmp;
	char *ptr = ctime(&clock);
	tmp = strchr(ptr, '\n');
	if (NULL != tmp)
		*tmp = 0;
	return ptr;
}

char *ytht_ctime_r(const time_t clock, char *buf) {
	char *tmp;
	ctime_r(clock, buf);
	tmp = strchr(buf, '\n');
	if (NULL != tmp)
		*tmp = '\0';

	return buf;
}

char *ytht_Difftime(time_t compared_time) {
	static char ret[64];
	time_t now = time(NULL);
	int now_i = (int)now;
	int tgt_i = (int)compared_time;

	int diff_i = abs(now_i - tgt_i);

	if(diff_i < 60) { //一分钟内 %d秒钟%s 前|后
		sprintf(ret, "%d\xC3\xEB\xD6\xD3%s", diff_i, (now_i>tgt_i) ? "\xC7\xB0" : "\xBA\xF3");
	} else if (diff_i < 3600) { // 一小时内 %d分钟%s
		sprintf(ret, "%d\xB7\xD6\xD6\xD3%s", diff_i/60, (now_i>tgt_i) ? "\xC7\xB0" : "\xBA\xF3");
	} else if (diff_i < 86400) { // 一天以内 %d小时%s
		sprintf(ret, "%d\xD0\xA1\xCA\xB1%s", diff_i/3600, (now_i>tgt_i) ? "\xC7\xB0" : "\xBA\xF3");
	} else { // 超过一天 %d天%s
		sprintf(ret, "%d\xCC\xEC%s", diff_i/86400, (now_i>tgt_i) ? "\xC7\xB0" : "\xBA\xF3");
	}

	return ret;
}

char *ytht_Difftime_s(time_t compared_time, char *buf, size_t buf_len) {
	time_t now = time(NULL);
	long diff = now - compared_time;
	if (diff < 0)
		diff = -diff;

	if(diff < 60) { //一分钟内 %d秒钟%s 前|后
		snprintf(buf, buf_len, "%ld\xC3\xEB\xD6\xD3%s", diff,       (now > compared_time) ? "\xC7\xB0" : "\xBA\xF3");
	} else if (diff < 3600) { // 一小时内 %d分钟%s
		snprintf(buf, buf_len,"%ld\xB7\xD6\xD6\xD3%s",  diff/60,    (now > compared_time) ? "\xC7\xB0" : "\xBA\xF3");
	} else if (diff < 86400) { // 一天以内 %d小时%s
		snprintf(buf, buf_len,"%ld\xD0\xA1\xCA\xB1%s",  diff/3600,  (now > compared_time) ? "\xC7\xB0" : "\xBA\xF3");
	} else { // 超过一天 %d天%s
		snprintf(buf, buf_len,"%ld\xCC\xEC%s",          diff/86400, (now > compared_time) ? "\xC7\xB0" : "\xBA\xF3");
	}

	return buf;
}

time_t get_time_of_the_biginning_of_the_day(struct tm *tm)
{
	// 设置为 UTC 0:00:00
	tm->tm_sec = 0;
	tm->tm_min = 0;
	tm->tm_hour = 0;

	return mktime(tm) - 8*3600; // 返回 UTC+8 0:00:00
}
