# Password Crypto Algorithm Test

## Introduction

`crypt.c` from `libytht` provides the crypto algorithm for storing passwords in `struct userec`. It has served the system for many years, however:

* The file itself should be copied from somewhere else, it's unmaintainable.
* The algorithm used looks like based on DES, which is not strong enough at the moment.
* The algorithms used by `/src/bbs` and `/nju09/www` to generate salt are in fact different and should be tested as two separate input paths. In the later context, we will use `bbs-salted` and `nju09-salted` to refer the two different paths.
	* `/nju09/www` path: `ytht_get_salt()` from `libytht/random.c` generates a 2-character DES salt from `/dev/urandom`, then `ytht_crypt_crypt1()` hashes with that salt.
	* `/src/bbs` path: `ytht_crypt_genpasswd()` from `libytht/crypt.c` generates its own salt internally from `random()` and `gettimeofday()`. Under the current default build it produces an 8-character DES-style salt and then hashes through the same DES implementation.
* A more reliable and standard library should be used instead of the in-tree implementation. The system target should be `crypt(3)` from `libcrypt`, because `libytht/crypt.c` already models that replacement with `__OPT_CRYPT_SYS__`.

## Goal

In this project we will:

1. Generate a persistent test vector set first. Each record should store at least: plaintext password and hashed passwords.
2. Validate the hashed passwords first using the in-tree implementation.
3. Validate the hashed passwords with `libcrypt`.

## Implementation

This subproject will generate 3 binaries.

1. `generator`
2. `validator_in_tree`
3. `validator_libcrypt`

### Data structure

Use a simple binary format based on C structs. This matches the style already used in BMYBBS and allows the utilities to read and write files with `fread` and `fwrite` without introducing any 3rd party library.

For the first iteration, define a file header and a fixed-size record. For example:

```c
#define MAX_PASS_LEN 32
#define MAX_HASH_LEN 128

struct passwd_crypto_test_header {
	char magic[8];     /* "PWCTEST" */
	uint32_t version;  /* currently 1 */
	uint32_t count;    /* number of records */
};

struct passwd_crypto_test_record {
	char plain[MAX_PASS_LEN + 1];
	char hash_nju09[MAX_HASH_LEN];
	char hash_bbs[MAX_HASH_LEN];
	char hash_extra[MAX_HASH_LEN];
};
```

`hash_extra` is reserved for a later iteration, for example when testing another algorithm such as `bcrypt`.

The 3 binaries should share the same struct definitions so they can read and write the same file format directly.

### Password Generator

After running the following command:

```bash
./generator -n num -out passwd
```

A file `passwd` will be created. This set contains `num` entries. Each entry contains:

- A plaintext password. This password should be generated randomly first. Both content and length. Can reuse `ytht_get_random_int`.
	1. Let's say we have a macro `MAX_PASS_LEN` to `32`.
	2. Then we have a buffer `passwd_buf[MAX_PASS_LEN + 1]`.
	3. Before creating a password, call `memset` first to reset the buffer to all zeros.
	4. Generate a random number first, then modulo it with `MAX_PASS_LEN` and add `1`, so the length of the password we are going to generate will be from `1` to `MAX_PASS_LEN`.
	5. Use a constant string as the dictionary. The dictionary should contain `[A-Za-z0-9]` plus characters " !@#$%^&*()-_+=[]{}\\|;:'\",./<>?`~".
	6. For each index from `0` to `len - 1`, generate a random integer, modulo it with the length of the dictionary, and store the selected character into `passwd_buf`.
	7. After the loop, write `'\0'` to `passwd_buf[len]`.
- Hashed password in the nju09-salted approach.
- Hashed password in the bbs-salted approach.

### Password Validator (in-tree)

With the following command:

```bash
./validator_in_tree -in /path/to/passwd
```

The program will load the file which generated from the `generator` and loop through each entry:

- For each entry, there are two hashed passwords, one from the bbs-salted path and the other from the nju09-salted path.
- Use the function `ytht_crypt_checkpasswd` to check both passwords against the associated plaintext passwords.
- If a hashed password fails, print a simple log line in the format `PW $pw_in_plaintext failed ($path)`, where `$path` is either `bbs` or `nju09`.
- The final count should be the number of failed records, not the number of failed checks. If both hashed passwords fail for one plaintext password, it still counts as one failed record.

In the end, print the number of failed records.

This binary uses the in-tree implementation, so it should be linked against `libytht`.

### Password Validator (libcrypt)

The workflow and options passed to this binary is similar to the in-tree version, except:

1. It doesn't link against `libytht`, `libcrypt` is used instead.
2. So it doesn't have the API `ytht_crypt_checkpasswd`.

Which means maybe a simple wrapper should implemented.
