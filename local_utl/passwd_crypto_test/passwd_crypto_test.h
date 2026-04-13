#ifndef PASSWD_CRYPTO_TEST_H
#define PASSWD_CRYPTO_TEST_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define PASSWD_CRYPTO_TEST_MAGIC "PWCTEST"
#define PASSWD_CRYPTO_TEST_MAGIC_LEN 7

#define MAX_PASS_LEN 32
#define MAX_HASH_LEN 128

struct passwd_crypto_test_header {
	char magic[8];
	uint32_t version;
	uint32_t count;
};

struct passwd_crypto_test_record {
	char plain[MAX_PASS_LEN + 1];
	char hash_nju09[MAX_HASH_LEN];
	char hash_bbs[MAX_HASH_LEN];
	char hash_extra[MAX_HASH_LEN];
};

void passwd_crypto_test_init_header(struct passwd_crypto_test_header *header, uint32_t count);
int passwd_crypto_test_write_header(FILE *fp, uint32_t count);
int passwd_crypto_test_read_header(FILE *fp, struct passwd_crypto_test_header *header);
int passwd_crypto_test_write_record(FILE *fp, const struct passwd_crypto_test_record *record);
int passwd_crypto_test_read_record(FILE *fp, struct passwd_crypto_test_record *record);

#endif
