#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

static const char *DEV_RAN = "/dev/urandom";

int ytht_get_random_buf(char *buf, size_t len) {
	int fd;

	fd = open(DEV_RAN, O_RDONLY);
	if (fd < 0)
		return -1;

	read(fd, buf, len);
	close(fd);
	return 0;
}

int ytht_get_random_int(unsigned int *s) {
	int fd;
	fd = open(DEV_RAN, O_RDONLY);
	if (fd < 0)
		return -1;

	read(fd, s, sizeof(unsigned int));
	close(fd);
	return 0;
}

int ytht_get_random_str(char *s) {
	int i;
	int fd;
	fd = open(DEV_RAN, O_RDONLY);
	if (fd < 0)
		return -1;
	read(fd, s, 30);
	close(fd);
	for (i = 0; i < 30; i++)
		s[i] = 65 + ((unsigned char)s[i]) % 26;
	s[30] = 0;
	return 0;
}

int ytht_get_random_str_r(char *s, size_t len) {
	int fd;
	size_t i;
	fd = open(DEV_RAN, O_RDONLY);
	if (fd < 0)
		return -1;

	read(fd, s, len);
	close(fd);
	for(i=0; i<len; ++i) {
		s[i] = ((unsigned char)s[i])%26 + 'A';
	}
	s[len-1] = 0;
	return 0;
}

int ytht_get_salt(char *salt) {
	int s, i, c;

	int fd;
	fd = open(DEV_RAN, O_RDONLY);
	if (fd < 0) {
		return -1;
	}

	read(fd, &s, 4);
	close(fd);
	salt[0] = s & 077;
	salt[1] = (s >> 6) & 077;
	salt[2] = 0;
	for (i = 0; i < 2; i++) {
		c = salt[i] + '.';
		if (c > '9')
			c += 7;
		if (c > 'Z')
			c += 6;
		salt[i] = c;
	}

	return 0;
}

