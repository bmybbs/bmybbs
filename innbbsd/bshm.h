#include "bbs.h"
#include "ythtbbs.h"

struct BCACHE   *brdshm=NULL;
struct boardmem *bcache=NULL;

int resolveshm()
{  int shmid;   
   shmid = shmget (BCACHE_SHMKEY, sizeof (*brdshm), 0);
   if (shmid < 0) return -1;
   brdshm = (struct BCACHE *) shmat (shmid, NULL, 0);
   if (brdshm == (struct BCACHE *)-1) return -1;
   bcache=brdshm->bcache;  
   return 0;
}
 
 
int getlastpost(char *board, int *lastpost, int *total)
{   struct fileheader fh ;
    struct stat st;
    char        filename[ STRLEN*2 ];
    int         fd, atotal, ftime;   
    
    sprintf(filename,MY_BBS_HOME "/boards/%s/.DIR", board);
    if( (fd = open( filename, O_RDWR )) < 0 )
        return -1;
    fstat( fd, &st );
    atotal = st.st_size / sizeof( fh );
    if( total <= 0 ) {
        *lastpost=0;  
        *total=0;     
        close( fd );  
        return 0;     
    }
    *total=atotal;
    lseek( fd,  (atotal-1) * sizeof(fh), SEEK_SET );
    if( read( fd, &fh, sizeof(fh) ) > 0 ) {
	if (fh.edittime==0)
		*lastpost = fh.filetime;
    	else
	    *lastpost=fh.edittime;
    }
    close(fd);
    return 0; 
}  
   
struct boardmem *
getbcache( bname )
char *bname ;
{
    int i ;
    int numboards;
    numboards=brdshm->number;
    for(i=0;i<numboards;i++) 
       if( !strncasecmp( bname, bcache[i].header.filename, STRLEN ) )
           return &bcache[i];
    return NULL;
}
 
int updatelastpost(char *board)
{   
    struct boardmem *bptr;
    if(brdshm==NULL) resolveshm();
    if(!brdshm)
	    return -1;
    bptr=getbcache(board); 
    if(bptr==NULL) return -1;
    return getlastpost(bptr->header.filename,&bptr->lastpost,&bptr->total);
}
