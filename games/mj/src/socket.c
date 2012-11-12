#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <pwd.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>

#include "mjdef.h"
#include "qkmj.h"

struct passwd *userdata;

int Check_for_data (fd)
     int fd;
/* Checks the socket descriptor fd to see if any incoming data has
   arrived.  If yes, then returns 1.  If no, then returns 0.
   If an error, returns -1 and stores the error message in socket_error.
*/
{
  int status;                 /* return code from Select call. */
  fd_set wait_set;     /* A set representing the connections that
				 have been established. */
  struct timeval tm;          /* A timelimit of zero for polling for new
				 connections. */

  FD_ZERO (&wait_set);
  FD_SET (fd, &wait_set);

  tm.tv_sec = 0;
  tm.tv_usec = 500;
  status = select (FD_SETSIZE, &wait_set, (fd_set *) 0, (fd_set *) 0, &tm);

/*  if (status < 0)
    sprintf (socket_error, "Error in select: %s", sys_errlist[errno]); */

  return (status);

}

init_serv_socket()
{
  struct sockaddr_in serv_addr;
         
  /* open a TCP socket for internet stream socket */
  if( (serv_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    err("Server: cannot open stream socket");
  
  /* bind our local address */
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  do
  {
    serv_addr.sin_port = htons(SERV_PORT);
    SERV_PORT++;
  }
  while(bind(serv_sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr))<0);
  listen(serv_sockfd, 10);
  FD_SET(serv_sockfd,&afds);
}

get_my_info()
{
  userdata=getpwuid(getuid());
  strcpy(my_username,userdata->pw_name);
}

init_socket(host,portnum,sockfd)
char *host;
int portnum;
int *sockfd;
{
  struct sockaddr_in serv_addr;

  bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family	= AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(host);
  serv_addr.sin_port  = htons(portnum);

	/* open a TCP socket */
  if( (*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		err("client: cannot open stream socket");

	/* connect to server */
  if(connect(*sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
  {
    return -1;
  }
}

accept_new_client()
{
  int alen;
  int i,player_id;
  char msg_buf[255];

/* Find a free space */
  for(i=2;i<MAX_PLAYER;i++)
    if(!player[i].in_table) break;
  if(i==MAX_PLAYER)
    err("Too many players!");
  player_id=i;
/* NOTICE: here we don't know player[player_id].addr */
/* it's ok! just get size. */
  alen=sizeof(player[player_id].addr);
/* NOTICE: so here, player[player_id].sockfd must be wrong!! */
  player[player_id].sockfd=accept(serv_sockfd, (struct sockaddr *)
                                  &player[player_id].addr, &alen);
  FD_SET(player[player_id].sockfd,&afds);
  player[player_id].in_table=1;
  player_in_table++;
/* assign a sit to the new comer */
  if(player_in_table<=PLAYER_NUM)
  {
    for(i=1;i<=4;i++)
    {
      if(!table[i])
      {
        player[player_id].sit=i;
        table[i]=player_id;
        break;
      }
    }
  }
  sprintf(msg_buf,"%s 加入此桌",new_client_name);
  send_gps_line(msg_buf);
  strcpy(player[player_id].name, new_client_name);
  player[player_id].id=new_client_id;
  player[player_id].money=new_client_money;
  /* Send the info of new comer to everyone */
  sprintf(msg_buf,"201%c%c%s", (char) player_id ,player[player_id].sit,
          player[player_id].name);
  broadcast_msg(1,msg_buf); /* NOTICE:including the new comer!!! */
  msg_buf[2]='5';   /* Set msg_id to 205 */
  /* send to himself */
/* NOTICE: player doesn't know his own table[i], right? */
  write_msg(player[player_id].sockfd,msg_buf);
  /* Send more info of new comer */
  sprintf(msg_buf,"202%c%5d%ld",player_id,new_client_id,new_client_money);
  broadcast_msg(1,msg_buf);
  new_client=0;
  write_msg(gps_sockfd,"111");  /* Add one new player into the table */
  /* Send all player info to the new player */
  for(i=1;i<MAX_PLAYER;i++)
  {
    if(player[i].in_table && i!=player_id)
    {
      /* Let the new comer know everyone */
      sprintf(msg_buf,"203%c%c%s",i,player[i].sit,player[i].name);
      write_msg(player[player_id].sockfd,msg_buf);
      sprintf(msg_buf,"202%c%5d%ld",i,player[i].id,player[i].money);
      write_msg(player[player_id].sockfd,msg_buf);
    }
  }
  /* Check the number of player in table */
  if(player_in_table==PLAYER_NUM)
  {
    init_playing_screen();
    broadcast_msg(1,"300");
    opening();
    open_deal();
  }
  
  /* Send table info to the new player */
/*
  sprintf(msg_buf,"204");
  for(i=1;i<=4;i++)
    msg_buf[2+i]=(char) table[i]+'0';
  write_msg(player[player_id].sockfd,msg_buf);
*/
}

int read_msg(fd,msg)
int fd;
char *msg;
{
  do
  {
    if(read(fd,msg,1)<=0)
      return 0;
  } while(*msg++ != '\0');
  return 1;
}

int read_msg_id(fd,msg)
int fd;
char *msg;
{
  int i;

  for(i=0;i<3;i++)
  {
    if(read(fd,msg,1)<=0)
      return 0;
    if(*msg++==0)
      return 0;
  }
  return 1;
}
      
write_msg(fd,msg)
int fd;
char *msg;
{
  int n;
  n=strlen(msg);
  if(write(fd,msg,n)<0)
  {
    return -1;
  }
  if(write(fd,msg+n,1)<0)
  {
    return -1;
  }
}

/* Command for server */
broadcast_msg(id,msg)
int id;
char *msg;
{
  int i;
  for(i=2;i<MAX_PLAYER;i++)
    if(player[i].in_table && i!=id)
      write_msg(player[i].sockfd,msg);
}


close_client(player_id)
int player_id;
{
  char msg_buf[255];

  if(player_in_table==4)
  {
    init_global_screen();
    input_mode=TALK_MODE;
  }
  player_in_table--;
  sprintf(msg_buf,"206%c",player_id);
  broadcast_msg(player_id,msg_buf);
  sprintf(msg_buf,"%s 离开此桌",player[player_id].name);
  display_comment(msg_buf);
  close(player[player_id].sockfd);
  FD_CLR(player[player_id].sockfd,&afds);
  player[player_id].in_table=0;
  table[player[player_id].sit]=0;
}

close_join()
{
  in_join=0;
  write_msg(table_sockfd,"200");
  close(table_sockfd); 
  FD_CLR(table_sockfd,&afds);
/*
  shutdown(table_sockfd,2);
*/
}

close_serv()
{
  int i;
  in_serv=0;
  for(i=2;i<MAX_PLAYER;i++)
  {
    if(player[i].in_table)
    {
      write_msg(player[i].sockfd,"200");
      close_client(i);
/*
      shutdown(player[i].sockfd,2);
*/
    }
  }
  FD_CLR(serv_sockfd,&afds);
}

leave()    /* the ^C trap. */
{
  int i;

  write_msg(gps_sockfd,"200");
/*
  shutdown(gps_sockfd,2);
*/ 
  close(gps_sockfd);
  if(in_join)
    close_join();
  if(in_serv)
    close_serv();
  endwin();
  exit(0);
}


