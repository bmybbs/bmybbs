#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "bbs.h"
#define PASSWDFILE MY_BBS_HOME"/.PASSWDS"

char *basestr =
    "$TTL 3D\n"
    "@		IN	SOA	ns.ytht.net.	root.ytht.net. (\n"
    "			%d       ; serial, todays date + todays serial #\n"
    "			1H	       ; refresh, seconds\n"
    "			900            ; retry, seconds\n"
    "			4W	       ; expire, seconds\n"
    "			1D )	       ; minimum, seconds\n"
    "		IN      NS		ns.ytht.net.\n"
    "		IN      NS		dns1.popv.net.\n"
    "		MX	10		ytht.net.\n"
    "		MX	20		mail.popv.net.\n"
    "ytht.net.  IN      A               162.105.31.222\n"
    "root		IN	A		162.105.31.222\n"
    "ns		IN	A		162.105.31.222\n"
    "www		IN	A		162.105.31.222\n"
    "mail		IN	A		162.105.31.222\n"
    "bbs		IN	A		162.105.31.222\n"
    "ftp		IN	A		162.105.31.222\n"
    "wenxue		IN	A		162.105.31.222\n";

int
ascii(char *s)
{
	int i;
	i = 0;
	while (1) {
		if (!s[i])
			break;
		if (!isascii(s[i]))
			return 0;
		i++;
	}
	return 1;
}

int
seek_in_file(filename, seekstr)
char filename[STRLEN], seekstr[STRLEN];
{

	FILE *fp;
	char buf[STRLEN];
	char *namep;
	if ((fp = fopen(filename, "r")) == NULL)
		return 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		namep = (char *) strtok(buf, ": \n\r\t");
		if (namep != NULL && strcasecmp(namep, seekstr) == 0) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

main(int argc, char *argv[])
{
	int fd1;
	struct userec rec;
	struct in_addr inaddr;
	int size1 = sizeof (rec);
	char *ptr;
	time_t now;
	time(&now);
	printf(basestr, now);
	if ((fd1 = open(PASSWDFILE, O_RDONLY, 0660)) == -1) {
		perror("open PASSWDFILE");
		return -1;
	}

	while (read(fd1, &rec, size1) == size1) {
		if (inet_aton(rec.ip, &inaddr) == 0 || !ascii(rec.userid))
			continue;
		ptr = inet_ntoa(inaddr);
		if (strncmp(rec.ip, ptr, strlen(ptr)) != 0)
			continue;
		printf("%s\t\tIN\tA\t\t%s\n", rec.userid, ptr);
		if (seek_in_file(MY_BBS_HOME "/etc/subdomain", rec.userid))
			printf("%s\t\tIN\tNS\t\t%s\n", rec.userid, rec.userid);
	}
	close(fd1);
	printf("*\t\tIN\tCNAME\t\tytht.net.\n");
}
