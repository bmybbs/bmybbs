#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

static const char *DEV_RAN = "/dev/urandom";

int ytht_get_random_buf(char *buf, size_t len) {
	int fd;
	ssize_t size;

	fd = open(DEV_RAN, O_RDONLY);
	if (fd < 0)
		return -1;

	size = read(fd, buf, len);
	close(fd);

	if (size <= 0 || (unsigned) size < len)
		return -1;
	return 0;
}

int ytht_get_random_int(unsigned int *s) {
	int fd;
	ssize_t size;
	fd = open(DEV_RAN, O_RDONLY);
	if (fd < 0)
		return -1;

	size = read(fd, s, sizeof(unsigned int));
	close(fd);

	if (size <= 0 || (unsigned) size < sizeof(unsigned int))
		return -1;
	return 0;
}

int ytht_get_random_str_r(char *s, size_t len) {
	int fd;
	size_t i;
	ssize_t size;
	fd = open(DEV_RAN, O_RDONLY);
	if (fd < 0)
		return -1;

	size = read(fd, s, len);
	close(fd);

	if (size <= 0 || (unsigned) size < len)
		return -1;

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

