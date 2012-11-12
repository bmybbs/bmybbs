/*
      BBS implementation dependendent part

      The only two interfaces you must provide
      
      #include "inntobbs.h" 
      int receive_article(); 
      0 success
      not 0 fail
       
      if (storeDB(HEADER[MID_H], hispaths) < 0) {
         .... fail
      }
             
      int cancel_article_front( char *msgid );       
      0 success
      not 0 fail
      
      char *ptr = (char*)DBfetch(msgid);

     the post contents received (body) is in char *BODY, 
     (header) in char *HEADER[]
     SUBJECT_H, FROM_H, DATE_H, MID_H, NEWSGROUPS_H,
     NNTPPOSTINGHOST_H, NNTPHOST_H, CONTROL_H, PATH_H,
     ORGANIZATION_H

     To filter input text, another set of HEADER is processed 
     at first and BODY processed later.
*/

/* 
   Sample Implementation
   
   receive_article()         --> post_article()   --> bbspost_write_post();
   cacnel_article_front(mid) --> cancel_article() --> bbspost_write_cancel();
*/


#ifndef PowerBBS

#include "innbbsconf.h"
#include "daemon.h"
#include "bbslib.h"
#include "inntobbs.h"
#include "lang.h"
#include "bbs.h"
#include "ythtbbs.h"
#include "bshm.h"
#include <sys/mman.h>

extern int Junkhistory;

char *post_article ARG((char *, char *, char *, int (*)(), char *, char *));
int cancel_article ARG((char *, char *, char *));

#define FAILED goto failed

report()
{
        /* Function called from record.o */
        /* Please leave this function empty */
}

#if defined(PalmBBS)
#ifndef PATH
# define PATH XPATH
#endif
#ifndef HEADER
# define HEADER XHEADER
#endif
#endif

/* process post write */
bbspost_write_post(fh, board, filename)
int fh;
char *board;
char *filename;
{
       char *fptr, *ptr;
       FILE *fhfd; 
       char conv_buf[256];

       fhfd = fdopen(fh,"w");
       if (fhfd == NULL) {
          bbslog("can't fdopen, maybe disk full\n");
          return -1;
       }

       if(strlen(SUBJECT) > 255) FAILED;
       str_decode(conv_buf,SUBJECT);       
       if (fprintf(fhfd,"%s%s, %s%s\n",FromTxt, FROM, BoardTxt, board)==EOF ||
           fprintf(fhfd,"%s%.70s\n", SubjectTxt, conv_buf)==EOF ||
           fprintf(fhfd,"%s%.43s (%s)\n",OrganizationTxt,SITE,DATE)==EOF ||
           fprintf(fhfd,"%s%s\n",PathTxt, PATH)==EOF )
           FAILED;

       if (POSTHOST != NULL) {
          if(fprintf(fhfd, "出  处: %.70s\n",POSTHOST)==EOF)
           FAILED;
       }
       if (fprintf(fhfd,"\n")==EOF) FAILED;
       for (fptr = BODY, ptr = strchr(fptr,'\r'); ptr != NULL && *ptr != '\0' ; fptr = ptr+1, ptr = strchr(fptr,'\r')) {
          int ch = *ptr;
          *ptr = '\0';
          if (fputs(fptr, fhfd)== EOF) FAILED;
          *ptr = ch;
       }
       if (fputs(fptr,fhfd)==EOF) FAILED;

       fflush(fhfd);
       fclose(fhfd);
       return 0;
failed:
       fclose(fhfd);
       return -1;
}

#ifdef KEEP_NETWORK_CANCEL
/* process cancel write */
bbspost_write_cancel(fh, board, filename)
int fh;
char *board, *filename;
{
       char *fptr, *ptr;
       FILE *fhfd = fdopen(fh,"w"), *fp;
       char buffer[256];

       if (fhfd == NULL) {
          bbslog("can't fdopen, maybe disk full\n");
          return -1;
       }

       if (fprintf(fhfd,"%s%s, %s%s\n",FromTxt, FROM, BoardTxt, board)==EOF)
         FAILED;
       if (fprintf(fhfd,"%s%.70s\n", SubjectTxt, SUBJECT)==EOF) FAILED;
       if (fprintf(fhfd,"%s%.43s (%s)\n",OrganizationTxt, SITE,DATE)==EOF) FAILED;
       if (fprintf(fhfd,"%s%.69s\n",PathTxt, PATH)==EOF) FAILED;
       if (HEADER[CONTROL_H] != NULL) {
          if (fprintf(fhfd, "Control: %s\n",HEADER[CONTROL_H])==EOF) 
             FAILED;
       }
       if (POSTHOST != NULL) {
          if (fprintf(fhfd, "出  处: %s\n",POSTHOST)==EOF)
            FAILED;
       }
       if (fprintf(fhfd,"\n")==EOF) FAILED;
       for (fptr = BODY, ptr = strchr(fptr,'\r'); ptr != NULL && *ptr != '\0' ; fptr = ptr+1, ptr = strchr(fptr,'\r')) {
          int ch = *ptr;
          *ptr = '\0';
          if (fputs(fptr, fhfd)==EOF) FAILED;
          *ptr = ch;
       }
       if (fputs(fptr,fhfd)==EOF) FAILED;
       /*if (POSTHOST != NULL) {
          fprintf(fhfd, "\n * Origin: ● %.26s ● From: %.40s\n",SITE,POSTHOST);
       }
       */
       if (fprintf(fhfd,"\n---------------------\n")==EOF) FAILED;
       fp = fopen(filename,"r");
       if (fp == NULL) {
             bbslog("can't open %s\n", filename);
             fclose(fhfd);
             return -1;
       }
       while (fgets( buffer, sizeof buffer, fp)!=NULL) {
          if (fputs(buffer,fhfd)==EOF) FAILED;
       }
       fclose(fp);
       fflush(fhfd);
       fclose(fhfd);

       {
          fp = fopen(filename,"w");
          if (fp == NULL) {
             bbslog("can't write %s\n", filename);
             return -1;
          }
          fprintf(fp,"%s%s, %s%s\n",FromTxt, FROM, BoardTxt, board);
          fprintf(fp,"%s%.70s\n", SubjectTxt, SUBJECT);
          fprintf(fp,"%s%.43s (%s)\n",OrganizationTxt, SITE,DATE);
          fprintf(fp,"%s%.70s\n",PathTxt, PATH);
          if (POSTHOST != NULL) {
             fprintf(fp, "出  处: %s\n",POSTHOST);
          }
          /*
          if (HEADER[CONTROL_H] != NULL) {
             fprintf(fp, "Control: %s\n",HEADER[CONTROL_H]);
          }
          */
          fprintf(fp,"\n");
          for (fptr = BODY, ptr = strchr(fptr,'\r'); ptr != NULL && *ptr != '\0' ; fptr = ptr+1, ptr = strchr(fptr,'\r')) {
             *ptr = '\0';
             fputs(fptr, fp);
          }
          fputs(fptr,fp);
          /*if (POSTHOST != NULL) {
            fprintf(fp, "\n * Origin: ● %.26s ● From: %.40s\n",SITE,POSTHOST);
          }
          */
          fclose(fp);
       }
       return 0;
failed:
       fclose(fhfd);
       return -1;
}
#endif

bbspost_write_control(fh, board, filename)
int fh;
char *board;
char *filename;
{
       char *fptr, *ptr;
       FILE *fhfd = fdopen(fh,"w");

       if (fhfd == NULL) {
          bbslog("can't fdopen, maybe disk full\n");
          return -1;
       }

       if (fprintf(fhfd,"Path: %s!%s\n",MYBBSID, HEADER[PATH_H])==EOF ||
           fprintf(fhfd,"From: %s\n",FROM)==EOF ||
           fprintf(fhfd,"Newsgroups: %s\n", GROUPS)==EOF ||
           fprintf(fhfd,"Subject: %s\n", SUBJECT)==EOF ||
           fprintf(fhfd,"Date: %s\n",DATE)== EOF ||
           fprintf(fhfd,"Organization: %s\n",SITE)==EOF)
       {
          fclose(fhfd);
          return -1;
       }
       if (POSTHOST != NULL) {
          if (fprintf(fhfd, "NNTP-Posting-Host: %.70s\n",POSTHOST)==EOF)
            FAILED;
       }
       if (HEADER[CONTROL_H] != NULL) {
          if (fprintf(fhfd, "Control: %s\n",HEADER[CONTROL_H])==EOF)
            FAILED;
       }
       if (HEADER[APPROVED_H] != NULL) {
          if (fprintf(fhfd, "Approved: %s\n",HEADER[APPROVED_H])==EOF)
            FAILED;
       }
       if (HEADER[DISTRIBUTION_H] != NULL) {
          if (fprintf(fhfd, "Distribution: %s\n",HEADER[DISTRIBUTION_H])==EOF)
            FAILED;
       }
       if (fprintf(fhfd,"\n")==EOF) FAILED;
       for (fptr = BODY, ptr = strchr(fptr,'\r'); ptr != NULL && *ptr != '\0' ; fptr = ptr+1, ptr = strchr(fptr,'\r')) {
          int ch = *ptr;
          *ptr = '\0';
          if (fputs(fptr, fhfd)==EOF) FAILED;
          *ptr = ch;
       }
       if (fputs(fptr,fhfd)==EOF) FAILED;


       fflush(fhfd);
       fclose(fhfd);
       return 0;
failed:
       fclose(fhfd);
       return -1;
}

/* process cancel write */
receive_article()
{
   char *user, *userptr; 
   char *ngptr, *nngptr, *pathptr;
   char **splitptr;
   static char userid[32];
   static char xdate[32];
   static char xpath[180];
   time_t datevalue;
   newsfeeds_t *nf;
   char *boardhome;
   char hispaths[4096];
   char firstpath[MAXPATHLEN], *firstpathbase;
   char *lesssym, *nameptrleft, *nameptrright;
   static char sitebuf[80];

   if (FROM == NULL) {
     bbslog( ":Err: article without usrid %s\n",MSGID);
     return 0;
   }
   if (strchr(FROM,'<') && (FROM[strlen(FROM)-1] == '>'))
     user = (char*)strrchr(FROM,'@');
   else user = (char*)strchr(FROM,'@');
   lesssym = (char*)strchr(FROM,'<');
   nameptrleft = NULL, nameptrright = NULL;
   if (lesssym == NULL || lesssym >= user) {
      lesssym = FROM;
      nameptrleft = strchr(FROM,'(');
      if (nameptrleft != NULL) nameptrleft++;
      nameptrright = strrchr(FROM,')');
   } else {
      nameptrleft = FROM;
      nameptrright = strrchr(FROM,'<');
      lesssym ++;
   }
   if (user != NULL) {
     *user = '\0';
     userptr = (char*)strchr(FROM,'.');
     if (userptr != NULL) {
       *userptr = '\0';
       strncpy(userid, lesssym, sizeof userid);
       *userptr = '.';
     } else {
       strncpy(userid, lesssym, sizeof userid);
     }
     *user = '@';
   } else {
     strncpy(userid, lesssym, sizeof userid);
   }
   //if(!isalnum(userid[0])) strcpy(userid,"Unknown");
   if( (unsigned)(userid[0]) < ' '  ) strcpy(userid,"Unknown");
   strcat(userid,".");
   datevalue = parsedate(DATE,NULL);

   if (datevalue > 0) {
     char *p ; 
     strncpy(xdate, ctime(&datevalue), sizeof(xdate));
     p = (char*)strchr(xdate,'\n');
     if (p != NULL) *p = '\0';
     DATE = xdate;
   } 
   if (SITE && strcasecmp("Computer Science & Information Engineering NCTU",SITE) ==0) {
      SITE = NCTUCSIETxt;
   } else if (SITE && strcasecmp("Dep. Computer Sci. & Information Eng., Chiao Tung Univ., Taiwan, R.O.C",SITE) ==0) {
      SITE = NCTUCSIETxt;
   } else if ( SITE == NULL || *SITE == '\0') {
      if (nameptrleft != NULL && nameptrright != NULL) {
         char savech = *nameptrright;
         *nameptrright = '\0';
         strncpy(sitebuf, nameptrleft, sizeof sitebuf); 
         *nameptrright = savech;
        SITE = sitebuf;   
      } else
        /*SITE = "(Unknown)";*/
        SITE = "";
   } 
   if (strlen(MYBBSID) > 70) {
      bbslog(" :Err: your bbsid %s too long\n", MYBBSID);
      return 0;
   }

   sprintf(xpath,"%s!%.*s",MYBBSID, sizeof(xpath) - strlen(MYBBSID) -2 , PATH); 
   PATH = xpath;
   for (pathptr = PATH; pathptr != NULL && (pathptr = strstr(pathptr,".edu.tw")) != NULL; ) {
           if (pathptr != NULL) {
              strcpy(pathptr,pathptr+7);
           }
   }
   xpath[71] = '\0';

   echomaillog(); 
   *hispaths = '\0';
   splitptr = (char**) BNGsplit(GROUPS);
   firstpath[0]= '\0';
   firstpathbase = firstpath;
   /* try to split newsgroups into separate group and
      check if any duplicated */

   /* try to use hardlink */
   /*for ( ngptr = GROUPS, nngptr = (char*) strchr(ngptr,','); ngptr != NULL && *ngptr != '\0'; nngptr = (char*)strchr(ngptr,',')) {*/
   for (ngptr = *splitptr; ngptr != NULL; ngptr = *(++splitptr)) {
      char *boardptr, *nboardptr;
          
          /*if (nngptr != NULL) {
            *nngptr = '\0';
          }*/
      if (*ngptr == '\0') continue;
      nf = (newsfeeds_t*)search_group(ngptr);
      /*printf("board %s\n",nf->board); */
      if (nf == NULL) {
         bbslog( "unwanted \'%s\'\n", ngptr );
         /*if( strstr( ngptr, "tw.bbs" ) != NULL ) {
         }*/
         continue;
      }
      if (nf->board == NULL || !*nf->board) continue;
      if (nf->path  == NULL || !*nf->path) continue;
      for ( boardptr = nf->board, nboardptr = (char*) strchr(boardptr,','); boardptr != NULL && *boardptr != '\0'; nboardptr = (char*)strchr(boardptr,',')) {
          if (nboardptr != NULL) {
            *nboardptr = '\0';
          }
          if (*boardptr == '\t') {
             goto boardcont;
          }
          boardhome = (char*)fileglue("%s/boards/%s",BBSHOME, boardptr);
          if (!isdir( boardhome)) {
              bbslog( ":Err: unable to write %s\n",boardhome);
          } else {
            char *fname;
            /*if ( !isdir( boardhome )) {
              bbslog( ":Err: unable to write %s\n",boardhome);
              testandmkdir(boardhome);
            }*/
            fname = (char*)post_article(boardhome,userid, boardptr, bbspost_write_post,NULL, firstpath);  
            if (fname != NULL) {
              fname = (char*)fileglue("%s/%s",boardptr,fname);
              if (firstpath[0] == '\0') {
                sprintf(firstpath,"%s/boards/%s",BBSHOME,fname);
                firstpathbase = firstpath + strlen(BBSHOME) + strlen("/boards/");
              }
              if (strlen(fname) + strlen(hispaths) + 1 < sizeof(hispaths)) { 
               strcat(hispaths,fname);
               strcat(hispaths," ");
              }
            } else {
              bbslog("fname is null %s\n",boardhome);
              return -1;
            }
          }

boardcont:
          if (nboardptr != NULL) {
            *nboardptr = ',';
            boardptr = nboardptr + 1; 
          } else
            break;
          
       }/* for board1,board2,... */
       /*if (nngptr != NULL) 
            ngptr = nngptr + 1; 
          else
            break;
       */
      if (*firstpathbase)
        feedfplog(nf, firstpathbase,'P');
   }
   if ( *hispaths )
      bbsfeedslog(hispaths,'P');

   if (Junkhistory || *hispaths) {
    if (storeDB(HEADER[MID_H], hispaths) < 0) {
     bbslog("store DB fail\n");
     /* I suspect here will introduce duplicated articles */
     /*return -1;*/
    }
   }
   return 0;
}

receive_control()
{
  char *boardhome, *fname;
  char firstpath[MAXPATHLEN], *firstpathbase;
  char **splitptr, *ngptr;
  newsfeeds_t *nf;

  bbslog( "control post %s\n", HEADER[CONTROL_H] );
  boardhome = (char*)fileglue("%s/boards/control", BBSHOME);
  testandmkdir(boardhome);
  *firstpath='\0';
  if (isdir(boardhome)) {
    fname = (char*)post_article(boardhome, FROM, "control", bbspost_write_control,NULL, firstpath);  
    if (fname != NULL) {
           if (firstpath[0] == '\0')
             sprintf(firstpath,"%s/boards/control/%s",BBSHOME,fname);
           if (storeDB(HEADER[MID_H], (char*)fileglue("control/%s",fname)) < 0) {
           }
           bbsfeedslog(fileglue("control/%s",fname),'C');
           firstpathbase = firstpath + strlen(BBSHOME) + strlen("/boards/");
           splitptr = (char**) BNGsplit(GROUPS);
           for (ngptr = *splitptr; ngptr != NULL; ngptr = *(++splitptr)) {
              if (*ngptr == '\0') continue;
              nf = (newsfeeds_t*)search_group(ngptr);
              if (nf == NULL) continue;
              if (nf->board == NULL) continue;
              if (nf->path == NULL) continue;
              feedfplog(nf, firstpathbase,'C');
           }
    }
  }
  return 0;
}

cancel_article_front(msgid)
char* msgid;
{
        char *ptr = (char*)DBfetch(msgid);
        char *filelist, filename[2048];
        char histent[4096];
        char firstpath[MAXPATHLEN], *firstpathbase;
        if (ptr == NULL) {
                return 0;
        }
        strncpy(histent, ptr, sizeof histent);
        ptr = histent;
#ifdef DEBUG
        printf("**** try to cancel %s *****\n",ptr);
#endif
        bbslog("**** try to cancel %s *****\n",ptr);
        filelist = strchr(ptr,'\t');
        if (filelist != NULL) {
           filelist++;
        }
        *firstpath = '\0';
        for ( ptr =  filelist; ptr && *ptr; ) {
          char *file;
          for (; *ptr && isspace(*ptr) ; ptr++); 
          if (*ptr == '\0') break;
          file = ptr;
          for (ptr++; *ptr && !isspace(*ptr) ; ptr++); 
          if (*ptr != '\0') {
             *ptr++ = '\0';
          } 
          sprintf(filename,"%s/boards/%s",BBSHOME, file);
          bbslog("**** Get file %s ****\n",ptr);
          if (isfile(filename)) {
/*              FILE *fp = fopen(filename,"r");
              char buffer[1024];
              char *xfrom, *xpath, *boardhome;

              if (fp == NULL) continue;
              while (fgets(buffer,sizeof (buffer), fp) != NULL) {
                 char *hptr; 
                 if (buffer[0]=='\n') break;
                 hptr = strchr(buffer,'\n');
                 if (hptr != NULL) *hptr = '\0'; 
                 if (strncmp(buffer,FromTxt,8)==0) {
                    char* n;
                    n = strrchr(buffer,',');
                    if (n!=NULL) *n = '\0'; 
                    xfrom = buffer+8;
                 } else if (strncmp(buffer,PathTxt,8)==0) {
                    xpath = buffer+8;
                 }
              }
              fclose(fp);
*/

              FILE *fp;
              char buffer[1024];
              char *xfrom, *boardhome;

              fp = fopen(filename,"r");
              if (fp != NULL) {
                 fgets(buffer,sizeof(buffer),fp);
                 fclose(fp);
              }
              xfrom = strtok(buffer, " ");
              if (strncmp(xfrom, FromTxt, 7) == 0) {
                 xfrom = (char *)strtok(NULL, ",\r\n");
                 } else {
                 xfrom="\0";
                 }
              if(strcmp(HEADER[FROM_H],xfrom) && !search_issuer(FROM)) {
                bbslog( "Invalid cancel %s, path: %s!%s\n",FROM,MYBBSID,PATH);
                return 0;
              }
              bbslog( "cancel post %s\n",filename );

#ifdef KEEP_NETWORK_CANCEL

              boardhome = (char*)fileglue("%s/boards/deleted", BBSHOME);
              testandmkdir(boardhome);
              if (isdir(boardhome)) {
                 char subject[1024];
                 char *fname;
                 if (POSTHOST) {
                   sprintf(subject,"cancel by: %.1000s", POSTHOST);
                 } else {
                   char *body, *body2;
                   body = strchr(BODY,'\r');
                   if (body != NULL) *body = '\0';
                   body2 = strchr(BODY,'\n');
                   if (body2 != NULL) *body = '\0';
                   sprintf(subject,"%.1000s", BODY);
                   if (body != NULL) *body = '\r';
                   if (body2 != NULL) *body = '\n';
                 } 
                 if (*subject) 
                   SUBJECT = subject;
                 fname = (char*)post_article(boardhome, FROM, "deleted", bbspost_write_cancel,filename,firstpath);  
                 if (fname != NULL) {
                   if (firstpath[0] == '\0') {
                     sprintf(firstpath,"%s/boards/deleted/%s",BBSHOME,fname);
                     firstpathbase = firstpath + strlen(BBSHOME) + strlen("/boards/");
                   }
                   if (storeDB(HEADER[MID_H], (char*)fileglue("deleted/%s",fname)) < 0) {
                       /* should do something */
                       bbslog("store DB fail\n");
                       /*return -1;*/
                   }
                   bbsfeedslog(fileglue("deleted/%s",fname),'D');

#ifdef OLDDISPATCH
                   {
                      char board[256];
                      newsfeeds_t *nf; 
                      char *filebase = filename + strlen(BBSHOME) + strlen("/boards/");
                      char *filetail = strrchr(filename,'/');
                      if (filetail != NULL) {
                        strncpy(board, filebase, filetail - filebase);
                        nf = (newsfeeds_t *)search_board(board);
                        if (nf != NULL && nf->board && nf->path) {
                          feedfplog(nf, firstpathbase,'D');
                        }
                      }
                   }
#endif
                 } else {
                   bbslog(" fname is null %s %s\n",boardhome, filename);
                   return -1;
                 }
              }
              /*bbslog("**** %s should be removed\n", filename);*/
              /*unlink(filename);*/
#endif
              {
                 char *fp = strrchr(file,'/');
                 if (fp != NULL) { 
                   *fp = '\0';
                   cancel_article(BBSHOME, file, fp+1);
                   *fp = '/';
                 }
              }
          }
        }
        if (*firstpath) {
         char **splitptr, *ngptr;
         newsfeeds_t *nf;
         splitptr = (char**) BNGsplit(GROUPS);
         for (ngptr = *splitptr; ngptr != NULL; ngptr = *(++splitptr)) {
              if (*ngptr == '\0') continue;
              nf = (newsfeeds_t*)search_group(ngptr);
              if (nf == NULL) continue;
              if (nf->board == NULL) continue;
              if (nf->path == NULL) continue;
              feedfplog(nf, firstpathbase,'D');
         }
        }
        return 0;
}

#if defined(FirebirdBBS)|| defined(PhoenixBBS) || defined(SecretBBS) || defined(PivotBBS) || defined(MapleBBS)
/* for PhoenixBBS's post article and cancel article */
#include "bbs.h"

char *post_article( homepath, userid,  board, writebody, pathname, firstpath )
char *homepath;
char *userid, *board ;
int (*writebody)();
char *pathname, *firstpath;
{
    struct userec       record;
    struct fileheader   header;
/*    char        *subject = SUBJECT;  */
    char        index[ MAXPATHLEN ]; 
    static char name[ MAXPATHLEN ]; 
    char        article[ MAXPATHLEN ];
    char        buf[ MAXPATHLEN ], *ptr;
    FILE        *fidx;
    int         fh;
    time_t      now;
    int         linkflag;
    char        conv_buf[256];

    sprintf( index, "%s/.DIR", homepath );
    if( (fidx = fopen( index, "r" )) == NULL ) {
        if( (fidx = fopen( index, "w" )) == NULL ) {
            bbslog( ":Err: Unable to post in %s.\n", homepath );
            return NULL;
        }
    }
    fclose( fidx );

    now = time( NULL );
    sprintf( name, "M.%d.A", now );
    ptr = strrchr( name, 'A' );
    while( 1 ) {
        sprintf( article, "%s/%s", homepath, name );
        fh = open( article, O_CREAT | O_EXCL | O_WRONLY, 0644 );
        /*if( fh != -1 )  break;*/
        if( fh >=0 )  break;
        if (errno != EEXIST) {
           bbslog(" Err: can't writable or other errors\n");
           return NULL;
        }
/* to solve client gateway problem, add now instead of add A, */
        now += 1; 
        sprintf( name, "M.%d.A", now );
/*
        if( *ptr < 'Z' )  (*ptr)++;
        else  ptr++, *ptr = 'A', ptr[1] = '\0';
*/
    }

#ifdef DEBUG
    printf( "post to %s\n", article );
#endif

    linkflag = 1;
    if (firstpath && *firstpath) {
          close(fh);
          unlink(article);
#ifdef DEBUGLINK
          bbslog("try to link %s to %s",firstpath,article);
#endif
          linkflag = link(firstpath,article);
          if (linkflag) {
              fh = open( article, O_CREAT | O_EXCL | O_WRONLY, 0644 );
          }
    } 
    if (linkflag != 0) {
            if (writebody) {
              if ((*writebody)(fh, board, pathname) < 0)
               return NULL;
            } else {
              if (bbspost_write_post(fh, board, pathname) < 0)
               return NULL;
            }
            close( fh );
    }
    bzero( (void *)&header, sizeof( header ) );
    header.filetime = atoi(name+2);
    fh_setowner(&header, userid, 0);
    str_decode(conv_buf,SUBJECT);
    strsncpy( header.title, conv_buf, sizeof(header.title));
    fh_find_thread(&header,board);
    /* if append record record, should return fail message */
    if (append_record( index, &header, sizeof( header ) ) < 0) {
      return NULL;
    }
    updatelastpost(board);
    return name;
}

cancel_article( homepath, board, file )
char    *homepath;
char    *board, *file;
{
    struct fileheader   header;
    struct stat         state;
    char        dirname[ MAXPATHLEN ];
    char        buf[ MAXPATHLEN ];
    long        numents, size, time, now;
    int         fd, i , ent, filetime;
    int pos, ret;
    char *ptr;

    if( file == NULL || file[0] != 'M' || file[1] != '.' ||
        (time = atoi( file+2 )) <= 0 )
        return 0;
    filetime = atoi(file +2);
    size = sizeof( header );
    sprintf( dirname, "%s/boards/%s/.DIR", homepath, board );
    if( (fd = open( dirname, O_RDWR )) == -1 )
        return 0;
    flock( fd, LOCK_EX );
    fstat( fd, &state );
	numents = ((long)state.st_size) / size;
	MMAP_TRY {
        ptr = mmap(0, state.st_size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0);
        ret = 0;
        pos = Search_Bin(ptr, filetime, 0, numents - 1);
	if (pos < 0)
		ret = -1;
        if ((pos + 1)* size > state.st_size) 
            ret = -2;
        if (ret == 0) {
            memcpy(ptr + pos * size, ptr + (pos + 1) * size, state.st_size - size * (pos + 1));
            ftruncate(fd, state.st_size - size);
        }
	}
	MMAP_CATCH {
	}
	MMAP_END munmap(ptr, state.st_size);
        flock(fd, LOCK_UN);
        close(fd);
    sprintf(buf,"%s/boards/%s/%s",homepath,board,file);
    unlink(buf);
    updatelastpost(board);
    return ret;
}

#elif defined(PalmBBS)
# undef PATH XPATH
# undef HEADER XHEADER
#include "server.h"

char *post_article( homepath, userid,  board, writebody, pathname, firstpath )
char *homepath;
char *userid, *board ;
int (*writebody)();
char *pathname, *firstpath;
{
    PATH  msgdir,msgfile;
    static PATH name; 

    READINFO readinfo;
    SHORT fileid;
    char buf[MAXPATHLEN];
    struct stat stbuf;
    int fh;

    strcpy(msgdir, homepath);
    if (stat(msgdir, &stbuf) == -1 || !S_ISDIR(stbuf.st_mode)) {
    /* A directory is missing! */
      bbslog(":Err: Unable to post in %s.\n", msgdir);
      return NULL;
    }
    get_filelist_ids(msgdir, &readinfo);

    for (fileid = 1; fileid <= BBS_MAX_FILES; fileid++) {
         int oumask;
         if (test_readbit(&readinfo, fileid)) continue;
         fileid_to_fname(msgdir, fileid, msgfile);
         sprintf(name,"%04x", fileid);
#ifdef DEBUG
         printf("post to %s\n",msgfile);
#endif
         if (firstpath && *firstpath) {
#ifdef DEBUGLINK
                bbslog("try to link %s to %s",firstpath,msgfile);
#endif
                if (link(firstpath,msgfile)==0) break;
         }
         oumask = umask(0);
         fh = open( msgfile, O_CREAT | O_EXCL | O_WRONLY, 0664 );
         umask(oumask);
         if (writebody) {
              if ((*writebody)(fh, board, pathname) < 0)
               return NULL;
         } else {
              if (bbspost_write_post(fh, board, pathname) < 0)
               return NULL;
         }
         close( fh );
         break;
    }
#ifdef CACHED_OPENBOARD
    {  char *bname;
       bname = strrchr(msgdir, '/');
       if(bname) notify_new_post(++bname, 1, fileid, stbuf.st_mtime);
    }
#endif
    return name;
}

cancel_article( homepath, board, file )
char    *homepath;
char    *board, *file;
{
        PATH fname;
#ifdef  CACHED_OPENBOARD
        PATH bdir;
        struct stat stbuf;

        sprintf(bdir, "%s/boards/%s", homepath, board);
        stat(bdir, &stbuf);
#endif
        sprintf(fname,"%s/boards/%s/%s",homepath,board,file);
        unlink(fname);
        /* kill it now! the function is far small then original..  :) */
        /* because it won't make system load heavy like before */
#ifdef CACHED_OPENBOARD
        notify_new_post(board, -1, hex2SHORT(file), stbuf.st_mtime);
#endif
}

#else
error("You should choose one of the systems: PhoenixBBS, PowerBBS, or PalmBBS") 
#endif

#else

receive_article()
{
}

receive_control()
{
}

cancel_article_front(msgid)
char* msgid;
{
}

#endif
