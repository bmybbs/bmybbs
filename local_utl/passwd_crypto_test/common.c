#include <stdio.h>
#include <string.h>

#include "passwd_crypto_test.h"

#define PASSWD_CRYPTO_TEST_VERSION 1

void
passwd_crypto_test_init_header(struct passwd_crypto_test_header *header, uint32_t count)
{
	memset(header, 0, sizeof(*header));
	memcpy(header->magic, PASSWD_CRYPTO_TEST_MAGIC, PASSWD_CRYPTO_TEST_MAGIC_LEN);
	header->version = PASSWD_CRYPTO_TEST_VERSION;
	header->count = count;
}

int
passwd_crypto_test_write_header(FILE *fp, uint32_t count)
{
	struct passwd_crypto_test_header header;

	passwd_crypto_test_init_header(&header, count);
	return fwrite(&header, sizeof(header), 1, fp) == 1 ? 0 : -1;
}

int
passwd_crypto_test_read_header(FILE *fp, struct passwd_crypto_test_header *header)
{
	if (fread(header, sizeof(*header), 1, fp) != 1)
		return -1;
	if (memcmp(header->magic, PASSWD_CRYPTO_TEST_MAGIC, PASSWD_CRYPTO_TEST_MAGIC_LEN) != 0)
		return -1;
	if (header->version != PASSWD_CRYPTO_TEST_VERSION)
		return -1;
	return 0;
}

int
passwd_crypto_test_write_record(FILE *fp, const struct passwd_crypto_test_record *record)
{
	return fwrite(record, sizeof(*record), 1, fp) == 1 ? 0 : -1;
}

int
passwd_crypto_test_read_record(FILE *fp, struct passwd_crypto_test_record *record)
{
	return fread(record, sizeof(*record), 1, fp) == 1 ? 0 : -1;
}
