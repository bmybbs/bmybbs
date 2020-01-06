/*
  NoCeM-INNBBSD
  Yen-Ming Lee <leeym@cae.ce.ntu.edu.tw>
*/
/*
$Id: nocem.h,v 1.1.1.1 2009-03-04 06:33:28 bmybbs Exp $
*/

#ifndef NOCEM_H
#define NOCEM_H

#include "innbbsconf.h"
#include "bbslib.h"
#include "inntobbs.h"
#include "version.h"		 /* for FB3 */

#ifdef SOLARIS
#include <stdarg.h>          /* for va_start() problem */
#else
#include <varargs.h>
#endif      

typedef struct ncmperm_t
{
  char *issuer;
  char *type;
  int perm;
} ncmperm_t;

ncmperm_t *NCMPERM=NULL, **NCMPERM_BYTYPE=NULL;
static char *NCMPERM_BUF;
int NCMCOUNT = 0;

#define TEXT    0
#define NCMHDR  1
#define NCMBDY  2

#define NOPGP   -1
#define PGPGOOD 0
#define PGPBAD  1
#define PGPUN   2

#define P_OKAY  0
#define P_FAIL  -1
#define P_UNKNOWN       -2
#define P_DISALLOW      -3

#define STRLEN          80
#define MAXSPAMMID      10000
#define	LINELEN			512

#define LeeymBBS        "ytht.net"
#define LeeymEMAIL      ADMINUSER
#define NCMINNBBSVER    VERSION
#undef	DONT_REGISTER

extern char NCMVER[];
extern char ISSUER[];
extern char TYPE[];
extern char ACTION[];
extern char NCMID[];
extern char COUNT[];
extern char THRESHOLD[];
extern char KEYID[];
extern char SPAMMID_NOW[];
extern char SPAMMID[][];

#endif /* NOCEM_H */
