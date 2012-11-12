#include <stdio.h>
#include "innbbsconf.h"
#include "daemon.h"
#include "bbslib.h"
#include <time.h>

#define INNTOBBS
#include "inntobbs.h"
#include "lang.h"

typedef struct Header {
  char *name;
  int  id;
} header_t;

/*enum HeaderValue {
SUBJECT_H, FROM_H, DATE_H, MID_H, NEWSGROUPS_H,
NNTPPOSTINGHOST_H, NNTPHOST_H, CONTROL_H, PATH_H,
ORGANIZATION_H, LASTHEADER,
};
*/

/*char *strchr ARG((char*,int));
char *strrchr ARG((char*,int));
char *strstr ARG((char*,char*));
*/

header_t headertable[] = {
"Subject"   ,SUBJECT_H,
"From"      ,FROM_H,
"Date"      ,DATE_H,
"Message-ID",MID_H,
"Newsgroups",NEWSGROUPS_H,
"NNTP-Posting-Host",NNTPPOSTINGHOST_H, 
"NNTP-Host", NNTPHOST_H,
"Control",   CONTROL_H,
"Path",      PATH_H,
"Organization", ORGANIZATION_H,
"X-Auth-From", X_Auth_From_H,
"Approved",  APPROVED_H,
"Distribution",  DISTRIBUTION_H,
"Keywords",  KEYWORDS_H,
"Summary",   SUMMARY_H,
"References",REFERENCES_H,
};

char *HEADER[LASTHEADER];
char *BODY;
char *FROM, *SUBJECT, *SITE, *DATE, *POSTHOST, 
     *NNTPHOST, *PATH, *GROUPS, *MSGID, *CONTROL;

#ifdef PalmBBS
char **XHEADER;
char *XPATH;
#endif


int
isexcluded(path1, nl)
char *path1;
nodelist_t *nl;
{
     char path2[1024];
     /*path2 = (char*)mymalloc(strlen(nl->node) + 3);*/
     sprintf(path2, "!%.*s!",sizeof path2 - 3, nl->node);
     if (strstr(path1, path2) != NULL) return 1;
     if (nl->exclusion && *nl->exclusion) {
        char *exclude, *ptr;
	for (exclude = nl->exclusion, ptr = strchr(exclude,','); 
	     exclude && *exclude; ptr = strchr(exclude,',')) {
	     if (ptr) *ptr = '\0';
	     sprintf(path2, "!%.*s!",sizeof path2 - 3, exclude);
	     if (strstr(path1, path2) != NULL) return 1;
	     if (ptr) {
	       *ptr = ',';
	       exclude = ptr+1;
	     } else {
	       break;
	     }
	}
     }
     return 0;
}

feedfplog(nf, filepath, type)
newsfeeds_t *nf;
char *filepath;
int type;
{
  char *path1, *path2, *hostptr;
  nodelist_t *nl;
  if (nf == NULL) return;
  if( nf->path != NULL ) {
    char *ptr1, *ptr2;
    char savech;
    path1 = (char*)mymalloc(strlen(HEADER[PATH_H]) + 3);
    sprintf(path1, "!%s!",HEADER[PATH_H]);
    for (ptr1 = nf->path; ptr1 && *ptr1;) {
     for (; *ptr1 && isspace(*ptr1); ptr1++);
     if (!*ptr1) break;
     for (ptr2 = ptr1; *ptr2 && !isspace(*ptr2); ptr2++);
     savech = *ptr2;
     *ptr2 = '\0';
     /*
     bbslog("search node %s\n",ptr1);
     */
     nl = (nodelist_t*) search_nodelist_bynode(ptr1);
     /*
     bbslog("search node node %s, host %s fp %d\n",nl->node, nl->host, nl->feedfp);
     */
     *ptr2 = savech;
     ptr1 = ptr2++;
     if (nl == NULL) continue;
     if (nl->feedfp == NULL) continue;
     if (isexcluded(path1,nl)) continue;
     /*path2 = (char*)mymalloc(strlen(nl->node) + 3);
     sprintf(path2, "!%s!",nl->node);
     free(path2);
     */
     /*
     bbslog("path1 %s path2 %s\n",path1, path2);
     */
     /*if (strstr(path1, path2) != NULL) return;*/
     /* to conform to the bntplink batch file */
     {
	char *slash = strrchr(filepath,'/');
	if (slash != NULL) *slash = '\t';
        fprintf(nl->feedfp,"%s\t%s\t\t%s\t%s\t%c\t%s\t%s!%s\n", 
	  filepath == NULL ? "": filepath,
	  GROUPS, FROM, SUBJECT, type, MSGID, MYBBSID, HEADER[PATH_H]);
	if (slash != NULL) *slash = '/';
     }
     fflush(nl->feedfp);
     if (savech == '\0') break;
    }
    free(path1); 
  }
}

static FILE* bbsfeedsfp = NULL;
static bbsfeedson = -1;

init_bbsfeedsfp()
{
   if (bbsfeedsfp != NULL) {
      fclose(bbsfeedsfp);
      bbsfeedsfp = NULL;
   }
   bbsfeedson = -1;
}

bbsfeedslog(filepath, type) 
char *filepath;
int type;
{

   char datebuf[40];
   time_t now;

   if (bbsfeedson ==0) return;
   if (bbsfeedson == -1) {
    if (!isfile(BBSFEEDS)) {
      bbsfeedson = 0;
      return;
    }
    bbsfeedson = 1;
   } 

   if (bbsfeedsfp == NULL) {
      bbsfeedsfp = fopen(BBSFEEDS,"a");
   }
   time(&now);
   strftime(datebuf, sizeof(datebuf), "%b %d %X ", localtime(&now));

   if( bbsfeedsfp != NULL ) {
	fprintf(bbsfeedsfp,"%s %c %s %s %s %s!%s %s\n", datebuf, type, 
	      REMOTEHOSTNAME, GROUPS, MSGID, MYBBSID, HEADER[PATH_H], filepath==NULL? "": filepath);
	fflush(bbsfeedsfp);
   }
}

static FILE* echomailfp = NULL;
static echomaillogon = -1;

init_echomailfp()
{
   if (echomailfp != NULL) {
      fclose(echomailfp);
      echomailfp = NULL;
   }
   echomaillogon = -1;
}

echomaillog() 
{

   if (echomaillogon ==0) return;
   if (echomaillogon == -1) {
    if (!isfile(ECHOMAIL)) {
      echomaillogon = 0;
      return;
    }
    echomaillogon = 1;
   } 

   if (echomailfp == NULL) {
      echomailfp = fopen(ECHOMAIL,"a");
   }

   if( echomailfp != NULL ) {
	fprintf(echomailfp,"\n");
	fprintf(echomailfp,"%s%s, %s%s\n", FromTxt, FROM, BoardTxt, GROUPS);
	fprintf(echomailfp,"%s%s\n", SubjectTxt, SUBJECT);
	fprintf(echomailfp,"%s%s (%s)\n", OrganizationTxt, SITE, DATE);
	fprintf(echomailfp,"%s%s (%s)\n", PathTxt, PATH, REMOTEHOSTNAME);
	fflush(echomailfp);
   }
}


#ifdef FILTER

char *
cmdfilter(indata, filterbuffer, cmd)
char *indata, **filterbuffer;
FuncPtr cmd;
{
   char *outdata;
   int len = strlen(indata);
   if (*filterbuffer != NULL)
     *filterbuffer = (char*)myrealloc(*filterbuffer, len + 4096);
   else
     *filterbuffer = (char*)mymalloc(len + 4096);
   if (filterbuffer == NULL) return NULL;
   bcopy(indata, *filterbuffer, len);
   (*cmd)(*filterbuffer, &len, 0);
   (*filterbuffer)[len] = '\0';
   return *filterbuffer;
}

char *externfilter(indata, filterbuffer, cmd)
char *indata, **filterbuffer, *cmd;
{
   char *outdata;
   FILE *fp;
   int fd;
   char *tmpfile;

   int len = strlen(indata);
   int size ;
   struct stat st;
    
   fp = (FILE*)popen((char*)fileglue("%s > /tmp/.innbbsdfilter%d%d",cmd,getpid(), getuid()),"w");
   if (fp == NULL) {
      bbslog("Err: can't popen %s\n",cmd);
      return NULL;
   }
   if (fwrite(indata, 1, len, fp) != len) {
      bbslog("Err: fwrite to %s failed, len %d\n",cmd, len);
      return NULL;
   }
   pclose(fp);

   tmpfile = (char*)fileglue("/tmp/.innbbsdfilter%d%d", getpid(), getuid());
   if (stat(tmpfile, &st) != 0) return NULL;

   if (*filterbuffer != NULL)
     *filterbuffer = myrealloc(*filterbuffer, st.st_size + 1);
   else
     *filterbuffer = mymalloc( st.st_size + 1);
   (*filterbuffer)[st.st_size] = '\0';
   if (*filterbuffer == NULL) return NULL;

   fd = open(tmpfile, O_RDONLY);
   if (fd < 0) return NULL;
   read(fd, *filterbuffer, st.st_size);
   close(fd);
   unlink(tmpfile);
   return *filterbuffer;
}

char *
filterdata(arp, direction, indata, filterdata)
newsfeeds_t *arp;
int direction;
char *indata, **filterdata;
{
   char *outdata; 
   FuncPtr fptr = NULL; 
   char *cmdptr = NULL;
   if (direction) {
      fptr = arp->sfilter;
      cmdptr = arp->scmdfilter;
   } else {
      fptr = arp->rfilter;
      cmdptr = arp->rcmdfilter;
   }
   if (fptr)
     outdata = cmdfilter(indata, filterdata, fptr);
   else if (cmdptr)
     outdata = externfilter(indata, filterdata, cmdptr);
   else
     outdata = NULL;
   return outdata;
}

char *
processfilter(indata, filterbuffer, direction)
char *indata, **filterbuffer;
int direction;
{
    char *front = indata, *ptr, *hptr;
    int i;
    char *outdata = indata;
    int filterflag=0;

    for (ptr = (char*)strchr(indata,'\n'); ptr != NULL && *ptr != '\0' ; front = ptr+1, ptr = (char*)strchr(front,'\n')) {
	*ptr = '\0';
        if (front[0] == '\r' || front[1] == '\n') {
	   *ptr= '\n';
	   break;
        }
        hptr = (char*)strchr(front,':');
	if (hptr != NULL && hptr[1] == ' ') {
	  *hptr = '\0';
	  if (strcmp(front,"Newsgroups")==0) {
	     char *ngp, *ongp; 
	     newsfeeds_t *arp;
	     char *tp;
	     if ((tp = (char*)strchr(hptr+2,'\r'))!=NULL)
	         *tp = '\0';
	     for (ngp = hptr+2, ongp = strchr(ngp,',');
		  ngp && *ngp; ongp = strchr(ngp,',')) {
		  if (ongp) *ongp = '\0';
		  arp = search_group(ngp);
		  if (arp) {
		    if (ongp) *ongp = ',';
		    if (tp != NULL) *tp = '\r';
		    if (hptr) *hptr = ':';
		    if (ptr) *ptr = '\n';
		    outdata = filterdata(arp, direction, indata, filterbuffer);
		    if (outdata) filterflag = 1;
		    if (ongp) *ongp = '\0';
		    if (tp != NULL) tp = '\0';
		    if (hptr) *hptr = '\0';
		    if (ptr) *ptr = '\0';
		  }
 		  if (ongp) {
		      *ongp = ',';
		      ngp = ongp+1;
		  } else {
		      break;
		  }
		  if (filterflag) break;
             }
	     if (tp != NULL) *tp='\r';
	     filterflag = 1;
	  }
	  *hptr = ':';
	}
	*ptr = '\n';
	if (filterflag) break;
    }
    if (outdata) return outdata;
    else return indata;
}
#endif

int headercmp(a,b)
header_t *a, *b;
{
   return strcasecmp(a->name, b->name);
}

int readlines(client)
ClientType *client;
{
    int fd = client->fd;
    char *buffer = client->buffer;
    buffer_t *in = &client->in;
    char *indata = in->data;
    char *front = in->data, *ptr, *hptr;
    int i;

    for (i=0; i < LASTHEADER; i++ ) 
      HEADER[i] = NULL;

#ifdef FILTER
    indata = processfilter(in->data, &client->filter_buffer, 0);
#endif

    if (indata != NULL) {
       front = indata;
    } else {
       front = indata = in->data;
    }

    for (ptr = (char*)strchr(indata,'\n'); ptr != NULL && *ptr != '\0' ; front = ptr+1, ptr = (char*)strchr(front,'\n')) {
	*ptr = '\0';
        if (front[0] == '\r' || front[1] == '\n') {
	   BODY = front+2;
	   break;
        }
        hptr = (char*)strchr(front,':');
	if (hptr != NULL && hptr[1] == ' ') {
	   int value;
	   *hptr = '\0';
           value = headervalue(front); 
	   if (value != -1) {
	      char *tp;
	      HEADER[value] = hptr + 2;
	      if ((tp = (char*)strchr(HEADER[value],'\r'))!=NULL)
                 *tp = '\0';
	   }
	   *hptr = ':';
	}
	/**ptr = '\n';*/
    }
    NNTPHOST = HEADER[NNTPHOST_H];
    PATH     = HEADER[PATH_H];
    FROM     = HEADER[FROM_H];
    GROUPS   = HEADER[NEWSGROUPS_H];
    SUBJECT  = HEADER[SUBJECT_H];
    DATE     = HEADER[DATE_H];
    SITE     = HEADER[ORGANIZATION_H];
    MSGID    = HEADER[MID_H];
    CONTROL  = HEADER[CONTROL_H];
    POSTHOST = HEADER[NNTPPOSTINGHOST_H];
    if (POSTHOST == NULL) {
      if (HEADER[X_Auth_From_H] != NULL) {
        POSTHOST = HEADER[X_Auth_From_H];
        HEADER[NNTPPOSTINGHOST_H] = POSTHOST;
      }
    }
#ifdef PalmBBS
    XPATH    = PATH;
    XHEADER  = HEADER;
#endif
}

int headervalue(inputheader)
char *inputheader;
{
   header_t key, *findkey;
   static int hasinit=0;

   if (hasinit == 0) {
      article_init();
      hasinit = 1;
   }

   key.name = inputheader;
   findkey = ( header_t *)bsearch (
		   (char *) &key, (char *) headertable, 
		   sizeof(headertable)/ sizeof(header_t), sizeof (key), 
		   headercmp);
   if (findkey != NULL) return findkey->id;
   return -1;
}

article_init()
{
   int i;
   static int article_inited = 0;

   if (article_inited) return;
   article_inited = 1;

   qsort(headertable, sizeof(headertable)/ sizeof(header_t), sizeof(header_t),
	 headercmp);
   for (i=0; i < LASTHEADER; i++ ) 
      HEADER[i] = NULL;
}

#ifdef INNTOBBS_MAIN
main()
{
    int i,j,k,l,m,n,o,p,q;
    article_init(); 
    i = headervalue("Subject");
    j = headervalue("From");
    k = headervalue("Date");
    l = headervalue("NNTP-Posting-Host");
    m = headervalue("Newsgroups");
    n = headervalue("Message-ID");
}
#endif
