#include <ctype.h>

unsigned int ythtbbs_cache_hash_userid(char *id) {
	unsigned int n1 = 0;
	unsigned int n2 = 0;
	const unsigned int _HASH_SIZE = 26;

	while (*id) {
		n1 += ((unsigned int) toupper(*id)) % _HASH_SIZE;
		id++;
		if (!*id) break;

		n2 += ((unsigned int ) toupper(*id)) % _HASH_SIZE;
		id++;
	}

	n1 %= _HASH_SIZE;
	n2 %= _HASH_SIZE;
	return n1 * _HASH_SIZE + n2;
}
