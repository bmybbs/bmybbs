#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ytht/crypt.h"
#include "ytht/random.h"

#include "passwd_crypto_test.h"

static const char passwd_dict[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789"
	" !@#$%^&*()-_+=[]{}\\|;:'\",./<>?`~";

static void
usage(const char *progname)
{
	fprintf(stderr, "usage: %s -n num -out passwd\n", progname);
}

static int
generate_plain(char *buf, size_t buf_size)
{
	unsigned int len_rand;
	size_t dict_len = sizeof(passwd_dict) - 1;
	size_t len;
	size_t i;

	if (buf == NULL || buf_size < MAX_PASS_LEN + 1)
		return -1;
	if (ytht_get_random_int(&len_rand) < 0)
		return -1;

	memset(buf, 0, buf_size);
	len = len_rand % MAX_PASS_LEN + 1;

	for (i = 0; i < len; ++i) {
		unsigned int ch_rand;

		if (ytht_get_random_int(&ch_rand) < 0)
			return -1;
		buf[i] = passwd_dict[ch_rand % dict_len];
	}
	buf[len] = '\0';

	return 0;
}

static int
fill_record(struct passwd_crypto_test_record *record)
{
	char bbs_plain[9];
	char salt[3];
	char plain[MAX_PASS_LEN + 1];
	char *hash;

	memset(record, 0, sizeof(*record));

	if (generate_plain(plain, sizeof(plain)) < 0)
		return -1;
	memcpy(record->plain, plain, sizeof(record->plain));

	if (ytht_get_salt(salt) < 0)
		return -1;
	hash = ytht_crypt_crypt1(plain, salt);
	if (hash == NULL)
		return -1;
	snprintf(record->hash_nju09, sizeof(record->hash_nju09), "%s", hash);

	memset(bbs_plain, 0, sizeof(bbs_plain));
	strncpy(bbs_plain, plain, sizeof(bbs_plain) - 1);

	hash = ytht_crypt_genpasswd(bbs_plain);
	if (hash == NULL)
		return -1;
	snprintf(record->hash_bbs, sizeof(record->hash_bbs), "%s", hash);

	return 0;
}

int
main(int argc, char *argv[])
{
	const char *out_path = NULL;
	char *endptr = NULL;
	unsigned long count_ul = 0;
	uint32_t count;
	FILE *fp;
	uint32_t i;

	if (argc != 5) {
		usage(argv[0]);
		return 1;
	}

	if (strcmp(argv[1], "-n") == 0 && strcmp(argv[3], "-out") == 0) {
		count_ul = strtoul(argv[2], &endptr, 10);
		out_path = argv[4];
	} else if (strcmp(argv[1], "-out") == 0 && strcmp(argv[3], "-n") == 0) {
		out_path = argv[2];
		count_ul = strtoul(argv[4], &endptr, 10);
	} else {
		usage(argv[0]);
		return 1;
	}

	if (endptr == NULL || *endptr != '\0' || count_ul == 0 || count_ul > UINT32_MAX || out_path == NULL) {
		usage(argv[0]);
		return 1;
	}

	count = (uint32_t) count_ul;
	fp = fopen(out_path, "wb");
	if (fp == NULL) {
		fprintf(stderr, "failed to open %s: %s\n", out_path, strerror(errno));
		return 1;
	}

	if (passwd_crypto_test_write_header(fp, count) < 0) {
		fprintf(stderr, "failed to write header to %s\n", out_path);
		fclose(fp);
		return 1;
	}

	for (i = 0; i < count; ++i) {
		struct passwd_crypto_test_record record;

		if (fill_record(&record) < 0) {
			fprintf(stderr, "failed to generate record %u\n", i);
			fclose(fp);
			return 1;
		}
		if (passwd_crypto_test_write_record(fp, &record) < 0) {
			fprintf(stderr, "failed to write record %u to %s\n", i, out_path);
			fclose(fp);
			return 1;
		}
	}

	if (fclose(fp) != 0) {
		fprintf(stderr, "failed to close %s\n", out_path);
		return 1;
	}

	return 0;
}
