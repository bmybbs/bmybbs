#include "bbs.h"
int
h(char *id)
{
	int n1 = 0;
	int n2 = 0;
	while (*id) {
		n1 += ((unsigned char) *id) % 26;
		id++;
		if (!*id)
			break;
		n2 += ((unsigned char) *id) % 26;
		id++;
	}
	n1 %= 26;
	n2 %= 26;
	return n1 * 26 + n2;
}

int d[26 * 26];

main()
{
	struct userec u;
	int i, j, k, fd = open(MY_BBS_HOME "/.PASSWDS", O_RDONLY);
	if (fd < 0)
		printf("can't open" MY_BBS_HOME "/.PASSWDS");
	bzero(d, sizeof (d));
	i = 0;
	while (read(fd, &u, sizeof (u)) > 0) {
		if (!u.userid[0])
			continue;
		i++;
		d[h(u.userid)]++;
	}
	printf("---%d---%d--%d--%d\n", i, i / 26 / 26, UCACHE_HASH_SIZE,
	       UCACHE_HASH_SIZE / 26 / 26);
	close(fd);
	for (i = 0; i < 26; i++) {
		k = 0;
		for (j = 0; j < 26; j++) {
			k += d[i * 26 + j];
			printf("%d ", d[i * 26 + j]);
		}
		printf("%d\n", k);
	}
}
