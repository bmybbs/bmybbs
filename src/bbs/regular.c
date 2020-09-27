#include <stdlib.h>
#include <string.h>
#include "regular.h"

///////////////////////////////////////////////////////////////////////////////
//Interface
///////////////////////////////////////////////////////////////////////////////

//"not" inserted in identifier
static const char *And = "\xC7\xD2";  // ÇÒ
static const char *Or  = "\xBB\xF2";  // »ò
static const char *Not = "\xB7\xC7";  // ·Ç
static const char LeftBracket = '(';
static const char RightBracket = ')';
static const char Separator[] = { ' ', '\t' };	//seperator, you can add ',', ';' to it, etc

////////////////////////////////////////////////////////////////////////////////////////
//Implementation
////////////////////////////////////////////////////////////////////////////////////////
static char *exp;
static int start;
static int end;

static int is_str(int temp, const char *Str);
static int is_sep(char c);
static int next_token(void);
static int do_extf(void);
static int do_term(void);
static int do_and(void);
static int do_or(void);

ExtStru *extstru;

static __inline int
is_str(int temp, const char *Str)
{
	if ((temp - start + 1 == (int) strlen(Str))
	    && (start == strstr(exp + start, Str) - exp))
		return 1;

	else
		return 0;
}
static __inline int
is_sep(char c)
{
	int i;
	int num = sizeof (Separator) / sizeof (char);
	for (i = 0; i < num; i++) {
		if (c == Separator[i])
			return 1;
	}
	return 0;
}
static __inline int
next_token()
{
	int i;
	while (start <= end && is_sep(exp[start]))
		start++;
	i = start;
	while (i <= end && !is_sep(exp[i]) && (exp[i] != LeftBracket)
	       && (exp[i] != RightBracket))
		i++;
	return i - 1;
}
static __inline int
do_extf()
{
	int temp = next_token();
	char *strt;
	int i, t;
	for (i = 0; extstru[i].f != NULL; i++) {
		if (is_str(temp, extstru[i].first)) {
			start = temp + 1;
			temp = next_token();
			strt = malloc(sizeof (char) * (temp - start + 2));
			memcpy(strt, exp + start, temp - start + 1);
			strt[temp - start + 1] = '\0';
			start = temp + 1;
			t = extstru[i].f(strt);
			free(strt);
			return t;
		} else if (is_str(temp, extstru[i].second)) {
			start = temp + 1;
			temp = next_token();
			strt = malloc(sizeof (char) * (temp - start + 2));
			memcpy(strt, exp + start, temp - start + 1);
			strt[temp - start + 1] = '\0';
			start = temp + 1;
			t = 1 - extstru[i].f(strt);
			free(strt);
			return t;
		}
	}
	return -start - 1;
}

static int
do_term()
{
	int t;
	int temp = next_token();
	if (is_str(temp, Not)) {
		start = temp + 1;
		t = do_term();
		if (t < 0)
			return t;

		else
			return (1 - t);
	} else if (exp[start] == LeftBracket) {
		start++;
		t = do_or();
		if (exp[start] != RightBracket)
			return -start - 1;
		start++;
		return t;
	} else {
		return do_extf();
	}
}
static int
do_and()
{
	int t, temp;
	int result = -start - 1;
	while (start <= end) {
		t = do_term();
		if (t < 0)
			return t;

		else if (result < 0)
			result = t;

		else {
			result *= t;
		}
		temp = next_token();
		if (is_str(temp, And))
			start = temp + 1;

		else
			break;
	}
	return result;
}

static int
do_or()
{
	int t, temp;
	int result = -start - 1;
	while (start <= end) {
		t = do_and();
		if (t < 0)
			return t;

		else if (result < 0)
			result = t;

		else {
			result |= t;
		}
		temp = next_token();
		if (is_str(temp, Or))
			start = temp + 1;

		else
			break;
	}
	return result;
}

int
checkf(char *str)
{
	int result;
	exp = str;
	start = 0;
	end = strlen(exp) - 1;
	result = do_or();
	if (start > end)
		return result;

	else
		return -start - 1;
}
