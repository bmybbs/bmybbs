/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Authors: Jeffrey Stedfast <fejj@ximian.com>
 *
 *  Copyright 2001 Ximain, Inc. (www.ximian.com)
 *
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define lowercase(c)            (isupper ((int) (c)) ? tolower ((int) (c)) : (int) (c))
#define bm_index(c, icase)      ((icase) ? lowercase (c) : (int) (c))
#define bm_equal(c1, c2, icase) ((icase) ? lowercase (c1) == lowercase (c2) : (c1) == (c2))

/* FIXME: this is just a guess... should really do some performace tests to get an accurate measure */
#define bm_optimal(hlen, nlen)  (((hlen) ? (hlen) > 20 : 1) && (nlen) > 10 ? 1 : 0)

static unsigned char *
__boyer_moore(const unsigned char *haystack, size_t haystacklen,
		const unsigned char *needle, size_t needlelen, int icase)
{
	register unsigned char *hc_ptr, *nc_ptr;
	unsigned char *he_ptr, *ne_ptr, *h_ptr;
	size_t skiptable[256], n;
	register int i;

#ifdef BOYER_MOORE_CHECKS
	/* we don't need to do these checks since memmem/strstr/etc do it already */

	/* if the haystack is shorter than the needle then we can't possibly match */
	if (haystacklen < needlelen)
		return NULL;

	/* instant match if the pattern buffer is 0-length */
	if (needlelen == 0)
		return (unsigned char *) haystack;
#endif				/* BOYER_MOORE_CHECKS */

	/* set a pointer at the end of each string */
	ne_ptr = (unsigned char *) needle + needlelen - 1;
	he_ptr = (unsigned char *) haystack + haystacklen - 1;

	/* create our skip table */
	for (i = 0; i < 256; i++)
		skiptable[i] = needlelen;
	for (nc_ptr = (unsigned char *) needle; nc_ptr < ne_ptr; nc_ptr++)
		skiptable[bm_index(*nc_ptr, icase)] = (size_t) (ne_ptr - nc_ptr);

	h_ptr = (unsigned char *) haystack;
	while (haystacklen >= needlelen) {
		hc_ptr = h_ptr + needlelen - 1;	/* set the haystack compare pointer */
		nc_ptr = ne_ptr;	/* set the needle compare pointer */

		/* work our way backwards till they don't match */
		for (i = 0; nc_ptr > (unsigned char *) needle; nc_ptr--, hc_ptr--, i++)
			if (!bm_equal(*nc_ptr, *hc_ptr, icase))
				break;

		if (!bm_equal(*nc_ptr, *hc_ptr, icase)) {
			n = skiptable[bm_index(*hc_ptr, icase)];
			if (n == needlelen && i)
				if (bm_equal(*ne_ptr, ((unsigned char *) needle)[0], icase))
					n--;
			h_ptr += n;
			haystacklen -= n;
		} else
			return (unsigned char *) h_ptr;
	}

	return NULL;
}

/**
 * ytht_strnstr:
 * @haystack: string to search
 * @needle: substring to search for
 * @haystacklen: length of the haystack to search
 *
 * Finds the first occurence of the substring @needle within the
 * bounds of string @haystack.
 *
 * Returns a pointer to the beginning of the substring match within
 * @haystack, or NULL if the substring is not found.
 **/
char *
ytht_strnstr(const char *haystack, const char *needle, size_t haystacklen)
{
	register unsigned char *h, *n, *hc, *nc;
	size_t needlelen;

	needlelen = strlen(needle);

	if (haystacklen < needlelen) {
		return NULL;
	} else if (needlelen == 0) {
		return (char *) haystack;
	} else if (needlelen == 1) {
		return memchr(haystack, (int) ((unsigned char *) needle)[0], haystacklen);
	} else if (bm_optimal(haystacklen, needlelen)) {
		return (char *) __boyer_moore((const unsigned char *) haystack,
				haystacklen,
				(const unsigned char *) needle,
				needlelen, 0);
	}

	h = (unsigned char *) haystack;
	n = (unsigned char *) needle;

	while (haystacklen >= needlelen) {
		if (*h == *n) {
			for (hc = h + 1, nc = n + 1; *nc; hc++, nc++)
				if (*hc != *nc)
					break;

			if (!*nc)
				return (char *) h;
		}

		haystacklen--;
		h++;
	}

	return NULL;
}

char *
ytht_strncasestr(const char *haystack, const char *needle, size_t haystacklen)
{
	register unsigned char *h, *n, *hc, *nc;
	size_t needlelen;

	needlelen = strlen(needle);

	if (haystacklen < needlelen) {
		return NULL;
	} else if (needlelen == 0) {
		return (char *) haystack;
	} else if (bm_optimal(haystacklen, needlelen)) {
		return (char *) __boyer_moore((const unsigned char *) haystack,
				haystacklen,
				(const unsigned char *) needle,
				needlelen, 1);
	}

	h = (unsigned char *) haystack;
	n = (unsigned char *) needle;

	while (haystacklen >= needlelen) {
		if (lowercase(*h) == lowercase(*n)) {
			for (hc = h + 1, nc = n + 1; *nc; hc++, nc++)
				if (lowercase(*hc) != lowercase(*nc))
					break;

			if (!*nc)
				return (char *) h;
		}

		haystacklen--;
		h++;
	}

	return NULL;
}

void ytht_strsncpy(char *s1, const char *s2, int n)
{
	int l = strlen(s2);
	if (n < 0)
		return;
	if (n > l + 1)
		n = l + 1;
	strncpy(s1, s2, n - 1);
	s1[n - 1] = 0;
}

void ytht_strncat(char *s1, size_t s1_len, const char *s2, size_t n)
{
	size_t l = strlen(s1);

	if (l + n + 1 > s1_len)
		return;

	strncat(s1, s2, n);
}

char *ytht_strltrim(char *s)
{
	char *s2 = s;
	if (s[0] == 0)
		return s;
	while (s2[0] && strchr(" \t\r\n", s2[0]))
		s2++;
	return s2;
}

char *ytht_strrtrim(char *s)
{
	static char t[1024], *t2;
	if (s[0] == 0)
		return s;
	ytht_strsncpy(t, s, 1024);
	t2 = t + strlen(s) - 1;
	while (strchr(" \t\r\n", t2[0]) && t2 > t)
		t2--;
	t2[1] = 0;
	return t;
}

char *ytht_strrtrim_s(const char *s) {
	char *t = NULL, *t2;

	if (s[0] == 0)
		return NULL;

	if ((t = strdup(s)) == NULL)
		return NULL;

	t2 = t + strlen(s) - 1;
	while (strchr(" \t\r\n", *t2) && t2 > t) {
		t2--;
	}

	t2[1] = 0;
	return t;
}

void ytht_normalize(char *buf)
{
	int i = 0;
	while (buf[i]) {
		if (buf[i] == '/')
			buf[i] = ':';
		i++;
	}
}

char* ytht_str_to_uppercase(char *str) {
	char *h = str;
	while (*str != '\n' && *str != 0) {
		*str = toupper(*str);
		str++;
	}
	return h;
}

char* ytht_str_to_lowercase(char *str) {
	char *h = str;
	while (*str != '\n' && *str != 0) {
		*str = tolower(*str);
		str++;
	}
	return h;
}

int ytht_badstr(const char *s) {
	int i;
	const unsigned char *t = (const unsigned char *)s;
	for (i = 0; s[i]; i++)
		if (t[i] != 9 && (t[i] < 32 || t[i] == 255))
			return 1;
	return 0;
}

