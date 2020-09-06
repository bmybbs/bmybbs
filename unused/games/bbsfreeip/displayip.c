#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "bbs.h"
#include "displayip.h"
static unsigned int free_addr[2000], free_mask[2000], free_num = 0;

int
display_ip(char *ip)
{
	struct hostent *he;
	struct in_addr i;
	int addr;

	if ((he = gethostbyname(ip)) != 0) {
		addr = *(int *) he->h_addr;
	} else {
		addr = inet_addr(ip);
	}
	i.s_addr = addr;
	if (addr == -1) {
		printf("\n错误的地址\n");
		return -1;
	}
	if (is_free(addr)) {
		printf
		    ("\n\n根据CERNET 2002年4月免费IP列表\n%s(%s)是一个免费ip.\n\n",
		     ip, inet_ntoa(i));
	} else {
		printf
		    ("\n\n根据CERNET 2002年4月免费IP列表\n%s(%s)不是免费ip.\n\n",
		     ip, inet_ntoa(i));
	}
	search_ip(addr);
	return 0;
}

int
search_ip(unsigned int addr)
{
	FILE *fp;
	char buf[512], buf2[80];
	int b1, b2, b3, b4, c1, c2, c3, c4, find = 0;
	unsigned int i1 = addr / (256 * 256 * 256), i2 = (addr / 65536) % 256;
	unsigned int i3 = (addr / 256) % 256, i4 = (addr % 256);
	unsigned int v =
	    i4 * 65536 * 256 + i3 * 256 * 256 + i2 * 256 + i1, v1, v2;
	fp = fopen(MY_BBS_HOME "/etc/ip_arrange.txt", "r");
	if (fp == 0)
		return;
	printf("IP地址 [%d.%d.%d.%d]\n", i4, i3, i2, i1);
	while (1) {
		buf[0] = 0;
		if (fgets(buf, 500, fp) <= 0)
			break;
		if (strlen(buf) < 10)
			continue;
		sscanf(buf, "%d.%d.%d.%d %d.%d.%d.%d %s",
		       &b1, &b2, &b3, &b4, &c1, &c2, &c3, &c4, buf2);
		v1 = b1 * 65536 * 256 + b2 * 256 * 256 + b3 * 256 + b4;
		v2 = c1 * 65536 * 256 + c2 * 256 * 256 + c3 * 256 + c4;
		if (v1 <= v && v <= v2) {
			printf("该IP属于: %s\n", buf2);
			find++;
		}
	}
	fclose(fp);
	if (find == 0)
		printf("没有找到匹配的记录！\n");
	else
		printf("一共找到 %d 个匹配记录。\n", find);
	printf
	    ("\n\n\n欢迎使用本查询系统。如果发现错误之处或有改进意见，请在sysop版发文。\n");
}

int
get_free_list()
{
	FILE *fp;
	char buf1[100], buf2[100], buf3[100], buf[100];
	static int inited = 0, r;
	if (inited)
		return;
	inited = 1;
	fp = fopen(MY_BBS_HOME "/etc/free.txt", "r");
	if (fp == 0)
		return;
	while (1) {
		if (!fgets(buf, sizeof (buf), fp))
			break;
		r = sscanf(buf, "%s%s%s", buf1, buf2, buf3);
		if (r <= 0)
			break;
		if (r != 3)
			continue;
		free_addr[free_num] = inet_addr(buf1);
		free_mask[free_num] = inet_addr(buf2);
		free_num++;
		if (free_num >= 2000)
			break;
	}
	fclose(fp);
}

int
is_free(unsigned int x)
{
	int n;
	get_free_list();
	for (n = 0; n < free_num; n++)
		if (((x ^ free_addr[n]) | free_mask[n]) == free_mask[n])
			return 1;
	return 0;
}
