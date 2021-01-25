#include "bbs.h"
#include "bbsstatlog.h"
struct bbsstatlogitem item;

int
main(int argc, char *argv[])
{
	int i, j, fd;
	struct tm *ptm;
	time_t t;
	int online[24][10], onlineavg[24][10][2], telnet[24][10],
		telnetavg[24][10][2], www[24][10], wwwavg[24][10][2],
		n162105[24][10], n162105avg[24][10][2], wwwguest[24][10],
		wgavg[24][10][2], netflow[24][10], nfavg[24][10][2];
	float load[24][10], loadavg[24][10][2];
	t = time(NULL);
	bzero(online, sizeof (online));
	bzero(onlineavg, sizeof (onlineavg));
	bzero(telnet, sizeof (telnet));
	bzero(telnetavg, sizeof (telnetavg));
	bzero(www, sizeof (www));
	bzero(wwwavg, sizeof (wwwavg));
	memset(n162105, 0, sizeof(n162105));
	memset(n162105avg, 0, sizeof(n162105avg));
	bzero(wwwguest, sizeof (wwwguest));
	bzero(wgavg, sizeof (wgavg));
	memset(netflow, 0, sizeof(netflow));
	memset(nfavg, 0, sizeof(nfavg));
	bzero(load, sizeof (load));
	bzero(loadavg, sizeof (loadavg));
	fd = open(BBSSTATELOGFILE, O_RDONLY | O_CREAT, 0600);
	if (fd < 0)
		return -1;
	while (read(fd, &item, sizeof (item)) == sizeof (item)) {
		if (item.nonline <= 0)
			continue;
		ptm = localtime(&item.time);
		i = ptm->tm_hour;
		j = ptm->tm_min / 6;
		if (t - item.time < 24 * 3600) {
			online[i][j] = item.nonline;
			telnet[i][j] = item.ntelnet;
			www[i][j] = item.nwww;
			load[i][j] = item.load[1];
			n162105[i][j] = item.n162105;
			wwwguest[i][j] = item.nwwwguest;
			netflow[i][j] = item.netflow;
		}
		if (t - item.time > 24 * 3600 * 7)
			continue;
		onlineavg[i][j][0] += item.nonline;
		onlineavg[i][j][1]++;
		telnetavg[i][j][0] += item.ntelnet;
		telnetavg[i][j][1]++;
		wwwavg[i][j][0] += item.nwww;
		wwwavg[i][j][1]++;
		loadavg[i][j][0] += item.load[1];
		loadavg[i][j][1]++;
		n162105avg[i][j][0] += item.n162105;
		n162105avg[i][j][1]++;
		wgavg[i][j][0] += item.nwwwguest;
		wgavg[i][j][1]++;
		nfavg[i][j][0] += item.netflow;
		nfavg[i][j][1]++;
	}
	close(fd);
	ptm = localtime(&t);
	i = ptm->tm_hour;
	j = ptm->tm_min / 6;
	printf("#now=%d.%d; %ldsec; %s", i, j, t, ctime(&t));
	printf("#time\tonline\tonlineavg\ttelnet\ttelnetavg\twww\twwwavg\tload\tloadavg\tn162105\taverage\twwwguest\twgavg\tnetflow\tnfavg\n");
	for (i = 0; i < 24; i++)
		for (j = 0; j < 10; j++) {
			printf("%d.%d", i, j);
			printf("\t%d\t%d", online[i][j],   (onlineavg[i][j][1] == 0)  ? 0 : (onlineavg[i][j][0]  / onlineavg[i][j][1]));
			printf("\t%d\t%d", telnet[i][j],   (telnetavg[i][j][1] == 0)  ? 0 : (telnetavg[i][j][0]  / telnetavg[i][j][1]));
			printf("\t%d\t%d", www[i][j],      (wwwavg[i][j][1] == 0)     ? 0 : (wwwavg[i][j][0]     / wwwavg[i][j][1]));
			printf("\t%f\t%f", load[i][j],     (loadavg[i][j][1] == 0)    ? 0 : (loadavg[i][j][0]    / loadavg[i][j][1]));
			printf("\t%d\t%d", n162105[i][j],  (n162105avg[i][j][1] == 0) ? 0 : (n162105avg[i][j][0] / n162105avg[i][j][1]));
			printf("\t%d\t%d", wwwguest[i][j], (wgavg[i][j][1] == 0)      ? 0 : (wgavg[i][j][0]      / wgavg[i][j][1]));
			printf("\t%d\t%d", netflow[i][j],  (nfavg[i][j][1] == 0)      ? 0 : (nfavg[i][j][0]      / nfavg[i][j][1]));
			printf("\n");
		}

	return 0;
}
