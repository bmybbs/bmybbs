#ifndef _REGULAR_H_
#define _REGULAR_H_

/*
usage:

syntax of str {see regular.c}

return value {0: false, 1: true, -x: syntax error at position x}

*/
int checkf(char *str);
typedef int (ExtF) (char *);
typedef struct {
	ExtF *f;
	char *first, *second;
} ExtStru;
extern ExtStru *extstru;
#define NotWord "²»"
#define Pair( a, b ) a b , a NotWord b
#endif				/*  */
