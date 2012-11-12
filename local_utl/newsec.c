#include "bbs.h"
main()
{
	int fd, i=0;
	struct boardheader bh;
	fd = open(MY_BBS_HOME "/.BOARDS", O_RDWR);
	while(read(fd, &bh, sizeof(bh))==sizeof(bh)) {
		bzero(bh.sec1, sizeof(bh.sec1));
		bzero(bh.sec2, sizeof(bh.sec2));
		bh.sec1[0]=bh.secnumber1;
		bh.sec2[0]=bh.secnumber2;
		lseek(fd, -sizeof(bh), SEEK_CUR);
		write(fd, &bh, sizeof(bh));
	}
	close(fd);
}
	
		
