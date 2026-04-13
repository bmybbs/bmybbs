#define _GNU_SOURCE

#include <crypt.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "passwd_crypto_test.h"

static void
usage(const char *progname)
{
	fprintf(stderr, "usage: %s -in /path/to/passwd\n", progname);
}

static int
checkpasswd_libcrypt(const char *pw_crypted, const char *pw_try)
{
	char *crypted = crypt(pw_try, pw_crypted);

	return crypted != NULL && strcmp(crypted, pw_crypted) == 0;
}

int
main(int argc, char *argv[])
{
	struct passwd_crypto_test_header header;
	FILE *fp;
	uint32_t i;
	uint32_t failed_records = 0;

	if (argc != 3 || strcmp(argv[1], "-in") != 0) {
		usage(argv[0]);
		return 1;
	}

	fp = fopen(argv[2], "rb");
	if (fp == NULL) {
		fprintf(stderr, "failed to open %s: %s\n", argv[2], strerror(errno));
		return 1;
	}

	if (passwd_crypto_test_read_header(fp, &header) < 0) {
		fprintf(stderr, "failed to read header from %s\n", argv[2]);
		fclose(fp);
		return 1;
	}

	for (i = 0; i < header.count; ++i) {
		struct passwd_crypto_test_record record;
		int record_failed = 0;

		if (passwd_crypto_test_read_record(fp, &record) < 0) {
			fprintf(stderr, "failed to read record %u from %s\n", i, argv[2]);
			fclose(fp);
			return 1;
		}
		record.plain[sizeof record.plain - 1] = 0;
		if (!checkpasswd_libcrypt(record.hash_bbs, record.plain)) {
			printf("PW %s failed (bbs)\n", record.plain);
			record_failed = 1;
		}
		if (!checkpasswd_libcrypt(record.hash_nju09, record.plain)) {
			printf("PW %s failed (nju09)\n", record.plain);
			record_failed = 1;
		}
		if (record_failed)
			++failed_records;
	}

	printf("%u failed records\n", failed_records);

	fclose(fp);
	return 0;
}
