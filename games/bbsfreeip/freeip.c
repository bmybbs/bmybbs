#include "bbs.h"
#include "displayip.h"

main()
{
	char buf[80], *ptr;
	int times = 0;
	printf("\n\033[1m" MY_BBS_NAME " IP 查询程序\033[m\n请输入要查询的 IP:\n");
	while(fgets(buf, sizeof(buf), stdin)) {
		if(buf[0]==0) {
			buf[0]=' ';
			buf[sizeof(buf)-1]=0;
		}
		if((ptr=strchr(buf,'\r')))
			*ptr=0;
		if((ptr=strchr(buf,'\n')))
			*ptr=0;
		while(buf[0]==' ')
			memmove(buf, buf+1, sizeof(buf)-1);
		if((ptr=strchr(buf,' ')))
			*ptr=0;
		
		if(!isalnum(buf[0]))
			break;
		display_ip(buf);
		times++;
		if (times >= 10) {
			printf("查的太多了，按一下回车退出吧");
			fgets(buf, sizeof(buf), stdin);
			return;
		}
		printf("\n\033[1m" MY_BBS_NAME " IP 查询程序\033[m\n请输入要查询的 IP:\n");
	}
}
		
				
