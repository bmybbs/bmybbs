/************************************************************************
	DBZ Server Query Sample by Samson Chen, May 8, 1995
	Code modified from INNBBSD-0.40 by Shih-Kun Huang
 ------------------------------------------------------------------------

	SYNOPSIS
	  init_dbz_channel()

	  close_dbz_channel()

	  add_mid(mid, path)
		char *mid;
		char *path;

	  query_mid(mid, path)
		char *mid;
		char *path;

	DESCRIPTION
	  make sure that #define DBZ_CHANNEL is the correct UNIX domain
	  socket path.

	  call init_dbz_channel() before using add_mid() and query_mid(),
	  return TRUE(-1) if successful, FALSE(0) if failed.
	  call close_dbz_channel() when program exit.

	  add_mid() put *mid and *path to dbz server. Return TRUE if
	  successful, FALSE if failed(maybe duplicate).

	  query_mid() search *mid from dbz server, if found, path will be
	  put to *path. If you call query_mid(*mid, NULL) then the function
	  just do searching. Return TRUE if found, FALSE if not found.

	  Comment the definition DBZ_CHANNEL (do not declare it) will stop
	  the dbz function automatically.

	  There is a sample main() on the bottom of this prgram, replace
	  it with your own.
 ************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>


#define	DBZ_CHANNEL	"innd/.innbbsd"

#define TRUE	-1
#define	FALSE	0

char INNBBSbuffer[4096];

static FILE *innbbsin, *innbbsout;
static int innbbsfd;
static char dbz_connect=FALSE;


#ifdef SOLARIS
#ifndef bzero
#define bzero(mem, size) memset(mem,'\0',size)
#endif
#endif

/*
	add Message-ID to DBZ server
*/
add_mid(mid, path)
char *mid;	/*Message-ID to be added*/
char *path;	/*path of that Message*/
/*
	return:
		TRUE:	OK
		FALSE:	Failed (maybe duplicate)
*/
{
  char line[4096];

  if( dbz_connect )
  {
	sprintf(line, "ADDHIST %s %s\r\n", mid, path);
	fputs( line,innbbsout );
	fflush( innbbsout);
	fgets(INNBBSbuffer, sizeof INNBBSbuffer, innbbsin);

	if( INNBBSbuffer[0]=='2' )
		return(TRUE);
	else
		return(FALSE);
  }
  else
	return(FALSE);
}
/*end of add_mid*/



/*
	query Message-ID from DBZ server
*/
query_mid(mid, path)
char *mid;	/*Message-ID to be searched*/
char *path;	/*If MID found, path will be put here, NULL for just test mid*/
/*
	return:
		TRUE:	mid found
		FALSE:	not found
*/
{
  char line[4096];
  char *l;
  int c;

  if( dbz_connect )
  {
	sprintf(line, "GREPHIST %s\r\n", mid);
	fputs( line,innbbsout);
	fflush( innbbsout);
	fgets(INNBBSbuffer, sizeof INNBBSbuffer, innbbsin);

	if( INNBBSbuffer[0]=='2' )
	{
	  if( path==(char*)NULL )
	    return(TRUE);

	  sprintf(line, "%s            ", INNBBSbuffer);
	  l=line;

	  /*find path*/
	  c=0;
	  while( l[c]!=0 && l[c]!=0x9 && l[c]!=0x20 )
	    c++;
	  l += c+1;

	  c=0;
	  while( l[c]!=0 && l[c]!=0x9 && l[c]!=0x20 )
	    c++;
	  l += c+1;

	  c=0;
	  while( l[c]!=0 && l[c]!=0x9 && l[c]!=0x20 )
	    c++;
	  l[c]=0;

	  strcpy(path, l);

	  return(TRUE);

	}
	else
	  return(FALSE);
  }
  else
    return(FALSE); 

}
/*end of query_mid*/



/*
	open UNIX DOMAIN socket
*/
int unixclient(path) 
char *path;	/*unix domin socket path*/
/*
	return:
		>0:	OK
		<0:	failed
*/
{
	struct sockaddr_un s_un;/*unix endpoint address*/
	int s;

	bzero((char*)&s_un,sizeof(s_un));
	s_un.sun_family= AF_UNIX;

	if (path==NULL) return(-1);

	strcpy(s_un.sun_path , path);

        /* Allocate a socket */
	s = socket(PF_UNIX,SOCK_STREAM,0);
	if (s<0) return -1;

	/* Connect the socket to the server */
	if (connect(s,(struct sockaddr*)&s_un,sizeof(s_un))<0)
		return -1;

	return s;
}
/*end of unixclient*/



/*
	init dbz unix domain socket
*/
init_dbz_channel()
/*
	return:
		TRUE:	OK
		FALSE:  Failed
*/
{

     dbz_connect=FALSE;

#ifdef DBZ_CHANNEL

     innbbsfd = unixclient(DBZ_CHANNEL);

     if (innbbsfd < 0)
	return(FALSE);

     if ( (innbbsin= fdopen(innbbsfd,"r")) == NULL ||
          (innbbsout= fdopen(innbbsfd,"w"))== NULL )
	return(FALSE);

     dbz_connect=TRUE;

     /*dbz server responsed initial status*/
     fgets(INNBBSbuffer, sizeof(INNBBSbuffer), innbbsin);
     if( strncmp(INNBBSbuffer, "200", 3) )
     {
	dbz_connect=FALSE;
	close_dbz_channel();
     }

#endif

     return(dbz_connect);

}
/*end of initsocket*/



/*
	close dbz channel
*/
close_dbz_channel()
{
  if( dbz_connect )
  {
    if (innbbsin != NULL)
       fclose(innbbsin);
    if (innbbsout != NULL)
       fclose(innbbsout);
    if (innbbsfd >= 0)
       close(innbbsfd);
  }
}
/*end of closesocket*/








/***********************************************************************
		Just a SAMPLE, replace main() with your own
 ***********************************************************************/
main(argc, argv)
int argc;
char **argv;
{
    char test[4096];
    int ret;

    init_dbz_channel();

    ret=add_mid("<tttt.eeee.ssss.tttt>", "THIS_IS_1ST_PATH");
    if( ret )
	printf("add 1st mid OK\n");
    else
	printf("add 1st mid failed\n");

    ret=add_mid("<bbbb.cccc.dddd.eeee>", "THIS_IS_2ND_PATH");
    if( ret )
	printf("add 2st mid OK\n");
    else
	printf("add 2st mid failed\n");

    ret=query_mid("<tttt.eeee.ssss.tttt>", test);
    if( ret )
	printf("query 1st ok : '%s'\n", test);
    else
	printf("query 1st failed\n");

    ret=query_mid("<bbbb.cccc.dddd.eeee>", test);
    if( ret )
	printf("query 2nd ok : '%s'\n", test);
    else
	printf("query 2nd failed\n");

    ret=query_mid("<no.such.item>", test);
    if( ret )
	printf("query no_such_item found ??? : '%s'\n", test);
    else
	printf("query no_such_item not found\n");

    close_dbz_channel();
} 

