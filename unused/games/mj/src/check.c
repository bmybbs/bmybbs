#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "curses.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define THREE_CARD 1
#define STRAIGHT_CARD 2
#define PAIR_CARD 3

#include "mjdef.h"
#include "qkmj.h"


clear_check_flag(sit)
char sit;
{
  int i,j;

  for(j=1;j<6;j++)
    check_flag[sit][j]=0;
}

int search_card(sit,card)
char sit;
char card;
{
  int i;
 
  if(card==0)
    return(0);
  for(i=0;i<pool[sit].num;i++)
  {
    if(pool[sit].card[i]==card)
      goto found;
  }
  i=-1;
  found:;
  return(i);
}

int check_kang(sit,card)
char sit;
char card;
{
  int i,kang;

  kang=1;
  if(card==0)
    return(0);
  for(i=0;i<pool[sit].num-2;i++)
    if(pool[sit].card[i]==card)
        goto found;
  for(i=0;i<pool[sit].out_card_index;i++)
    if(pool[sit].out_card[i][0]==2 && pool[sit].out_card[i][1]==card &&
       sit==card_owner)
    {
      return 2;
    }
  kang=0;
  found:;
  if(pool[sit].card[i+1]!=card || pool[sit].card[i+2]!=card)
    kang=0;
  return(kang);
}

int check_pong(sit,card)
char sit;
char card;
{
  int i,pong;

  pong=1;
  if(card==0)
    return(0);
  for(i=0;i<pool[sit].num-1;i++)
    if(pool[sit].card[i]==card)
      goto found;
  pong=0;
  found:;
  if(pool[sit].card[i+1]!=card)
    pong=0;
  return(pong);
}

int check_eat(sit,card)
char sit;
char card;
{
  int i,j,eat;
  char msg_buf[80];
  
  eat=0;
  i=next_turn(turn);
  if(i!=sit)
    return(0);
  if(card<1 || card>29)
    return(0);
  j=card%10;
  switch(j)
  {
    case 1:
      if(search_card(sit,card+1)>=0 && search_card(sit,card+2)>=0)
        eat=1;
      break;
    case 2:
      if(search_card(sit,card-1)>=0 && search_card(sit,card+1)>=0)
        eat=1;
      if(search_card(sit,card+1)>=0 && search_card(sit,card+2)>=0)
        eat=1;
      break;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
      if(search_card(sit,card-2)>=0 && search_card(sit,card-1)>=0)
        eat=1;
      if(search_card(sit,card-1)>=0 && search_card(sit,card+1)>=0)
        eat=1;
      if(search_card(sit,card+1)>=0 && search_card(sit,card+2)>=0)
        eat=1;
      break;
    case 8:
      if(search_card(sit,card-2)>=0 && search_card(sit,card-1)>=0)
        eat=1;
      if(search_card(sit,card-1)>=0 && search_card(sit,card+1)>=0)
        eat=1;
      break;
    case 9:
      if(search_card(sit,card-2)>=0 && search_card(sit,card-1)>=0)
        eat=1;
      break;
    default:
      break;
  }
  return(eat);
}

check_card(sit,card)
char sit;
char card;
{
  char msg_buf[80];

  clear_check_flag(sit);
  check_flag[sit][1]=check_eat(sit,card);
  check_flag[sit][2]=check_pong(sit,card);
  check_flag[sit][3]=check_kang(sit,card);
  check_flag[sit][4]=check_make(sit,card,0);
}

check_begin_flower(sit,card,position)  /* command for server */
char sit;
char card;
char position;
{
  int i;
  char msg_buf[80];

  if(card<=58 && card>=51)
  {
    sprintf(msg_buf,"525%c%c",sit,pool[sit].card[position]);
    broadcast_msg(1,msg_buf);
    draw_flower(sit,card);
    card=mj[card_point++];
    if(sit==my_sit)
      change_card(position,card);
    else
    {
      pool[sit].card[position]=card;
      sprintf(msg_buf,"301%c%c",position,card);
      write_msg(player[table[sit]].sockfd,msg_buf);
    }
    return(1);
  }
  else
    return(0);
}

check_flower(sit,card)
char sit;
char card;
{
  int i;
  char msg_buf[80];

  if(card<=58 && card>=51)
  {
    sprintf(msg_buf,"525%c%c",sit,card);
    draw_flower(sit,card);
    if(in_join)
    {
      write_msg(table_sockfd,msg_buf);
    }
    else
    {
      broadcast_msg(1,msg_buf);
      card=mj[card_point++];
      show_num(2,70,144-card_point-16,2);
      card_owner=my_sit;
      sprintf(msg_buf,"305%c",(char) my_sit);
      broadcast_msg(1,msg_buf);
      sprintf(msg_buf,"306%c",card_point);
      broadcast_msg(1,msg_buf);
      play_mode=GET_CARD;
      attron(A_REVERSE);
      show_card(10,INDEX_X+16*2+1,INDEX_Y+1,1);
      attroff(A_REVERSE);
      return_cursor();
    }
    return(1);
  }
  else
    return(0);  
}

write_check(check)  /* Finished checking! */
char check;
{
  char msg_buf[80];

  if(in_join)
  {
    sprintf(msg_buf,"510%c",check+'0');
    write_msg(table_sockfd,msg_buf);
  }
  else
  {
    in_check[player[1].sit]=0;
    check_for[player[1].sit]=check;
  }
}

send_pool_card()
{
  int j;
  char msg_buf[80];

  /* ----------------------- */
  /*   0-2: 512 公布四家的牌 */
  /*  3-18: 东家的牌         */
  /* 19-34: 南家的牌         */   
  /* 35-50: 西家的牌         */
  /* 51-66: 北家的牌         */
  /*    67: 0                */
  /* ----------------------- */
  sprintf(msg_buf,"521");
  for(j=0;j<16;j++)
    msg_buf[3+j]=pool[1].card[j];
  for(j=0;j<16;j++)
    msg_buf[19+j]=pool[2].card[j];
  for(j=0;j<16;j++)
    msg_buf[35+j]=pool[3].card[j];
  for(j=0;j<16;j++)
    msg_buf[51+j]=pool[4].card[j];
  msg_buf[67]=0;
  broadcast_msg(1,msg_buf);
}

compare_check()
{
  int i,j;
  char msg_buf[80];
  
  next_player_request=0;
  /* Check for make */
  i=card_owner;
  for(j=1;j<=4;j++)
  {
    i=next_turn(i);
    if(check_for[i]==MAKE)
    {
      send_pool_card(i);
      sprintf(msg_buf,"522%c%c",i,current_card);
      broadcast_msg(1,msg_buf);
      process_make(i,current_card);
      goto finish;
    }
  }
  /* Check for kang */
  for(i=1;i<=4;i++)
  {
    if(check_for[i]==KANG)
    {
      card_owner=i;
      if(table[i]==1) /* Server wants to process_epk */
      {
        if(search_card(i,current_card)>=0)
        {
        if(turn==i)  /* 自己摸牌 */
          process_epk(11);
        else
          process_epk(KANG);
        }
        else 
          process_epk(12);
      }
      else
      {
        if(search_card(i,current_card)>=0)
        {
        if(turn==i)
          sprintf(msg_buf,"520%c",11); 
        else
          sprintf(msg_buf,"520%c",KANG);
        }
        else
          sprintf(msg_buf,"520%c",12);
        write_msg(player[table[i]].sockfd,msg_buf);
        show_newcard(turn,1);
        return_cursor();
      }
      turn=i;
      sprintf(msg_buf,"314%c%c",i,1);
      broadcast_msg(table[i],msg_buf);
      goto finish;
    }
  }
  /* Check for pong */
  for(i=1;i<=4;i++)
  {
    if(check_for[i]==PONG)
    {
      card_owner=i;
      if(table[i]==1)
        process_epk(PONG);
      else
      {
        sprintf(msg_buf,"520%c",PONG);
        write_msg(player[table[i]].sockfd,msg_buf);
        show_newcard(i,2);
        return_cursor();
      }
      turn=i;
      sprintf(msg_buf,"314%c%c",i,2);
      broadcast_msg(table[i],msg_buf);
      goto finish;
    }
  }
  /* Check for eat */
  for(i=1;i<=4;i++)
  {
    if(check_for[i]>=7 && check_for[i]<=9)
    {
      card_owner=i;
      if(table[i]==1)
        process_epk(check_for[i]);
      else
      {
        sprintf(msg_buf,"520%c",check_for[i]);
        write_msg(player[table[i]].sockfd,msg_buf);
        show_newcard(i,2);
        return_cursor();
      }
      turn=i;
      sprintf(msg_buf,"314%c%c",i,2);
      broadcast_msg(table[i],msg_buf);
      goto finish;
    }
  }
  if(!getting_card)
    next_player_request=1;
  finish:;
  for(i=1;i<=4;i++)
    check_for[i]=0;
  getting_card=0;
}



