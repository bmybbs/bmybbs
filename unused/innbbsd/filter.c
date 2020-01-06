#if defined( LINUX )
# include "innbbsconf.h"
# include <varargs.h>
#else
# include <varargs.h>
# include "innbbsconf.h"
#endif

#include "bbslib.h"

/* 
#  filter.conf
#node    in    out  
mynode:b2g:g2b
*/

char *big2gb(), *gb2big(), *gb2hz(), *hz2gb(), *big2hz(), *hz2big();
char *big2jis(), *jis2big();
int big2gb_init(), gb2big_init(), gb2hz_init(); 
int hz2gb_init(),  big2hz_init(), hz2big_init();
int big2jis_init(),jis2big_init();


typedef struct Filternode_t {
  char *name;
  char *(*cmd)();
  int  (*cmd_init)();
} filternode_t;

static filternode_t FILTERS[]={
#ifdef BIG2GB
{"big2gb", big2gb, big2gb_init},
#endif
#ifdef GB2BIG
{"gb2big", gb2big, gb2big_init},
#endif
#ifdef GB2HZ
{"gb2hz",  gb2hz, gb2hz_init},
#endif
#ifdef HZ2GB
{"hz2gb",  hz2gb, hz2gb_init},
#endif
#if defined(GB2BIG) && defined(HZ2GB) 
{"hz2big",  hz2big, hz2big_init},
{"big2hz",  big2hz, big2hz_init},
#endif
#ifdef BIG2JIS
{"big2jis", big2jis, big2jis_init},
#endif
#ifdef JIS2BIG
{"jis2big", jis2big, jis2big_init},
#endif
#ifdef GB2JIS
{"gb2jis",  gb2jis, gb2jis_init},
#endif
#ifdef JIS2GB
{"jis2gb", jis2gb, jis2gb_init},
#endif
{"\0",  NULL, NULL},
};

filtermatch(result,target,pat)
int result;
char *target,*pat;
{
    char *filterp = pat, *ptr;
    char *arg;
    for (filterp = pat, ptr = strchr(filterp,',');
        filterp && *filterp; ptr = strchr(filterp,',')) {
        if (ptr) *ptr = '\0';
	arg = filterp;
        if (*arg == '!') {
	  if (wildmat(target,arg+1)) {
	    result = 0;
	  }
        } else if (wildmat(target,arg)) {
           result = 1;
        }
	if (ptr) {
	   *ptr = ',';
	   filterp = ptr+1;
	} else {
	   break;
	}
    }
    return result;
}


FuncPtr search_filtercmd(cmd)
char *cmd;
{
  filternode_t *nodep;
  char *ptr; 
  int savech;

  for (ptr = cmd; *ptr && strchr("\r\n\t\b ",*ptr) == NULL; ptr++);
  savech = *ptr; *ptr = '\0';
  for (nodep = FILTERS; nodep && nodep->name && *nodep->name; nodep++) {
    if (strcasecmp(nodep->name, cmd)==0) 
      return nodep->cmd;
  }
  *ptr = savech;
  return NULL;
}


#if defined(HZ2GB) && defined(GB2BIG)
int
big2hz_init(arg)
char *arg;
{
  big2gb_init(arg);
  gb2hz_init(arg);
}

int
hz2big_init(arg)
char *arg;
{
  hz2gb_init(arg);
  gb2big_init(arg);
}

char *
hz2big(buf, len, init)
char *buf; 
int *len;
int init;
{
   hz2gb(buf, len, init);
   return gb2big(buf, len, init);
}

char *
big2hz(buf, len, init)
char *buf; 
int *len;
int init;
{
   big2gb(buf, len, init);
   return gb2hz(buf, len, init);
}

#endif


#ifdef MYCONVERSION

static FILTERoutbuffer[8192];

Rfprintf(va_alist)
va_dcl
{
    va_list ap;
    register char* fmt;
    newsfeeds_t *nf;
    FILE *fp;

    va_start(ap);
    nf  = va_arg(ap, newsfeeds_t*);
    fp  = va_arg(ap, FILE *);
    fmt = va_arg(ap, char *) ;
    if (nf->rfilter) {
        vsprintf(FILTERoutbuffer, fmt, ap);
	if (nf->rfilter(FILTERoutbuffer, sizeof FILTERoutbuffer))
           fputs(FILTERoutbuffer, sizeof(FILTERoutbuffer),fp);
        else
           fputs(FILTERoutbuffer, sizeof(FILTERoutbuffer),fp);
    } else {
	fprintf(fp, fmt, ap);
    }
    va_end(ap);
}

static FILTERoutbuffer[8192];
int
Snfprintf(va_alist)
va_dcl
{
    va_list ap;
    register char* fmt;
    newsfeeds_t *nf;
    FILE *fp;
    int n;

    va_start(ap);
    nf  = va_arg(ap, newsfeeds_t *);
    fp  = va_arg(ap, FILE *);
    fmt = va_arg(ap, char *) ;
    if (nf->sfilter) {
        vsprintf(FILTERoutbuffer, fmt, ap);
	if (nf->rfilter(FILTERoutbuffer, sizeof FILTERoutbuffer))
           fputs(FILTERoutbuffer, sizeof(FILTERoutbuffer),fp);
        else
           fputs(FILTERoutbuffer, sizeof(FILTERoutbuffer),fp);
    } else {
	fprintf(fp, fmt, ap);
    }
    va_end(ap);
}


int
Rfputs(string)
char *string;
{
}

int 
Sfputs(string)
char *string;
{
}

#if defined(GB2BIG)
char *gb2big()
{
}
#endif

#if defined(BIG2GB)
char* big2gb()
{
}
#endif

#if defined(GB2BIG)
int g2bfputs()
{
}
int
g2bfprintf()
{
}
#endif

#if defined(BIG2GB)
int b2gfputs()
{
}
int
b2gfprintf()
{
}

#endif

#if defined(JIS2BIG)
char *jis2big() 
{
}
#endif

#if defined(BIG2JIS)
char *big2jis() 
{
}
#endif

#if defined(HZ2GB) 
char *
hz2gb() 
{
}
#endif


#if defined(HC)
#endif

#if defined(WORDG2B)
#endif

#endif
