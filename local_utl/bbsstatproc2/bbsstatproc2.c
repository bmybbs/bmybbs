#include "bbs.h"
#include "bbsstatlog.h"
struct bbsstatlogitem item;
int
main()
{
	int i, j, fd, nowday, count = 0;
	struct tm *ptm;
	time_t t;
	int sonline[90], stelnet[90], swww[90], num[90], swwwguest[90],
	    s162105[90], s162105telnet[90], snetflow[90];
	float sdayload[90];
	bzero(num, sizeof (num));
	bzero(sonline, sizeof (sonline));
	bzero(stelnet, sizeof (stelnet));
	bzero(swww, sizeof (swww));
	bzero(sdayload, sizeof (sdayload));
	bzero(swwwguest, sizeof (swwwguest));
	bzero(s162105, sizeof (s162105));
	bzero(s162105telnet, sizeof (s162105telnet));
	bzero(snetflow, sizeof (snetflow));
	t = time(NULL);
	//nowday=t/(24*3600);
	ptm = localtime(&t);
	nowday = ptm->tm_yday;
	fd = open(BBSSTATELOGFILE, O_RDONLY | O_CREAT, 0600);
	if (fd < 0)
		return;
	while (read(fd, &item, sizeof (item)) == sizeof (item)) {
		count++;
		if (item.nonline <= 0)
			continue;
		//i=item.time/(24*3600);
		ptm = localtime(&item.time);
		i = ptm->tm_yday;
		if (i > nowday)
			i -= 365;
		if (i > nowday || nowday - i >= 90)
			continue;
		j = nowday - i;
		num[j]++;
		sonline[j] += item.nonline;
		sonline[j] -= item.nwwwguest;
		stelnet[j] += item.ntelnet;
		swwwguest[j] += item.nwwwguest;
		s162105[j] += item.n162105;
		s162105telnet[j] += item.n162105telnet;
		swww[j] += item.nwww;
		snetflow[j] += item.netflow;
		ptm = localtime(&item.time);
		i = ptm->tm_hour;
		if (i >= 8)
			sdayload[j] += item.load[1];
	}
	close(fd);
	ptm = localtime(&t);
	i = ptm->tm_hour;
	j = ptm->tm_min / 6;
	printf("#now=%d.%d; %dsec; %s; count=%d\n", i, j, t, ctime(&t), count);
	printf
	    ("#day\tsonline\tstelnet\tswww\tsdayloadi\twwwguest\ts162105\ts162105telnet\tsnetflow\n");
	for (i = 0; i < 40; i++) {
		j = 40 - 1 - i;
		//if(num[j]==0)
		if (0)
			if (num[j] < 220)
				continue;
		if (num[j] != 240)
			printf("%d\n", num[j]);
		printf("-%d", j);
		//printf("\t%d", num[j]);
		if (0)
			printf("\t%f\t%f\t%f\t%f\t%f\n",
			       sonline[j] * 1. / num[j],
			       stelnet[j] * 1. / num[j], swww[j] * 1. / num[j],
			       sdayload[j] * 1. / num[j],
			       swwwguest[j] * 1. / num[j]);
		printf("\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",
		       sonline[j] * 1. / 240, stelnet[j] * 1. / 240,
		       swww[j] * 1. / 240, sdayload[j] * 1. / 240,
		       swwwguest[j] * 1. / 240, s162105[j] * 1. / 240,
		       s162105telnet[j] * 1. / 240, snetflow[j] * 1. / 240);
	}
}
