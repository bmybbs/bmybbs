#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include  "curses.h"
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/errno.h>
#include "mjdef.h" 
#include "socket.h"
#include "input.h" 

/*gloable variables*/
fd_set rfds,afds;
int nfds;
extern int errno;
//extern char *sys_errlist[];
char GPS_IP[50];
int GPS_PORT;
char my_username[20];
char my_address[70];
int PLAYER_NUM=4;
char QKMJ_VERSION[]="094";
char menu_item[25][5]
={"  ","A ","B ","C ","D ","E ","F ","G ","H ","I ","J ",
  "K ","L ","M ","N ","O ","P ","Q ","R ","  "};
char number_item[30][3]
={"０","１","２","３","４","５","６","７","８","９","10","11","12","13",
  "14","15","16","17","18","19","20"};
char mj_item[100][5]
={"＊＊","一万","二万","三万","四万","五万","六万","七万","八万","九万",
  "摸牌","一索","二索","三索","四索","五索","六索","七索","八索","九索",
  "    ","一筒","二筒","三筒","四筒","五筒","六筒","七筒","八筒","九筒",
  "□□","东风","南风","西风","北风","    ","    ","    ","    ","    ",
  "□□","红中","白板","青发","    ","    ","    ","    ","    ","    ",
  "    ","春１","夏２","秋３","冬４","梅１","兰２","菊３","竹４"
 };
struct tai_type {
  char name[20];
  int score;
  char flag;
} tai[100]={
  {"庄家",1,0},
  {"门清",1,0},
  {"自摸",1,0},
  {"断么九",1,0},
  {"一杯口",1,0},
  {"杠上开花",1,0},
  {"海底摸月",1,0},
  {"河底捞鱼",1,0},
  {"抢杠",1,0},
  {"东风",1,0},
  {"南风",1,0},
  {"西风",1,0},
  {"北风",1,0},
  {"红中",1,0},
  {"白板",1,0},
  {"青发",1,0},
  {"花牌",1,0},
  {"东风东",2,0},
  {"南风南",2,0},
  {"西风西",2,0},
  {"北风北",2,0},
  {"春夏秋冬",2,0},
  {"梅兰菊竹",2,0},
  {"全求人",2,0},
  {"平胡",2,0},
  {"混全带么",2,0},
  {"三色同顺",2,0},
  {"一条龙",2,0},
  {"二杯口",2,0},
  {"三暗刻",2,0},
  {"三杠子",2,0},
  {"三色同刻",2,0},
  {"门清自摸",3,0},
  {"碰碰胡",4,0},
  {"混一色",4,0},
  {"纯全带么",4,0},
  {"混老头",4,0},
  {"小三元",4,0},
  {"四暗刻",6,0},
  {"四杠子",6,0},
  {"大三元",8,0},
  {"小四喜",8,0},
  {"清一色",8,0},
  {"字一色",8,0},
  {"七抢一",8,0},
  {"五暗刻",8,0},
  {"清老头",8,0},
  {"大四喜",16,0},
  {"八仙过海",16,0},
  {"天胡",16,0},
  {"地胡",16,0},
  {"人胡",16,0},
  {"连  拉  ",2,0}
  };
struct card_comb_type {
  char info[10][20];
  int set_count;
  int tai_sum;
  int tai_score[100];
} card_comb[20];
int comb_num;
char mj[150];
char sit_name[5][3]
={"  ","东","南","西","北"};
char check_name[7][3]
={"无","吃","碰","杠","胡","听"};

int SERV_PORT=DEFAULT_SERV_PORT;
int set_beep;
int pass_login;
int pass_count;
int current_item;
int card_in_pool[5];
int pos_x, pos_y;
int current_check;
int check_number;
int check_x,check_y;
int eat_x,eat_y;
int card_count=0;
int card_point;
int card_index;
int gps_sockfd,serv_sockfd,table_sockfd;
int in_serv,in_join;
char talk_buf[255]="\0";
int talk_buf_count=0;
char history[HISTORY_SIZE+1][255];
int h_head,h_tail,h_point;
int talk_x,talk_y;
int talk_left,talk_right;
int comment_x,comment_y;
int comment_left, comment_right, comment_bottom, comment_up;
char comment_lines[24][255];
int talk_mode;
int play_mode;
int screen_mode;
unsigned char key_buf[255];
char wait_hit[5];
int waiting;
unsigned char *str;
int key_num;
int input_mode;
int current_mode;
unsigned char cmd_argv[40][100];
int arglenv[40];
int narg;
int my_id;
int my_sit;
long my_money;
unsigned int my_gps_id;
unsigned char my_name[11];
unsigned char my_pass[9];
unsigned char my_note[255];
struct ask_mode_info {
  int question;
  int answer_ok;
  char *answer;
} ask;
struct player_info {
  int sockfd;
  int in_table;
  int sit;
  unsigned int id;
  char name[30];
  long money;
  char pool[20];
  struct sockaddr_in addr;
} player[MAX_PLAYER];
struct pool_info {
  char name[30];
  int num;
  char card[20];
  char out_card_index;
  char out_card[10][6];
  char flower[10];
  char door_wind;
  int first_round;
  long money;
  float time;
} pool[5];
struct table_info {
  int cardnum;
  int wind;
  int dealer;
  int cont_dealer;
  int base_value;
  int tai_value;
} info;
struct timeval before,after;
int table[5];
int new_client;
char new_client_name[30];
long new_client_money;
unsigned int new_client_id;
int player_num;
WINDOW *commentwin, *inputwin,*global_win,*playing_win;
int turn;
int card_owner;
int in_kang;
int current_id;
int current_card;
int in_play;
int on_seat;
int player_in_table;
int check_flag[5][8],check_on,in_check[6],check_for[6];
int go_to_check;
int send_card_on;
int send_card_request;
int getting_card;
int next_player_on;
int next_player_request;
int color=1;
int cheat_mode=0;
char table_card[6][17];

/* Request a card */
request_card()
{
  if(in_join)
  {
    write_msg(table_sockfd,"313");
    return(0);
  }
  else
  {
    return(mj[card_point++]);
  }
}
  
/* Change a card */
change_card(position,card)
char position;
char card;
{
  int i;

  i=(16-pool[my_sit].num+position)*2;
  if(position==pool[my_sit].num)
    i++;
  pool[my_sit].card[position]=card;
  show_card(20,INDEX_X+i,INDEX_Y+1,1);
  wrefresh(stdscr);
  show_card(card,INDEX_X+i,INDEX_Y+1,1);
  wrefresh(stdscr);
}

/* Get a card */
get_card(card)
char card;
{
  pool[my_sit].card[pool[my_sit].num]=card;
  show_card(20,INDEX_X+16*2+1,INDEX_Y+1,1);
  wrefresh(stdscr);
  show_card(card,INDEX_X+16*2+1,INDEX_Y+1,1);
  wrefresh(stdscr);
}

process_new_card(sit,card)
char sit;
char card;
{
  char msg_buf[255];

  current_card=card;
  show_cardmsg(sit,0);
  pool[sit].card[pool[sit].num]=card;
  get_card(card);
  return_cursor();
  if(!check_flower(sit,card))
    play_mode=THROW_CARD;
}

/* Throw cards to the table */
throw_card(card)
char card;
{
  int x,y;
  if(card==20)   
  {
    card_count--;
  }
  table_card[card_count/17][card_count%17]=card;
  x=THROW_X+(card_count%17)*2;
  y=THROW_Y+card_count/17*2;
  if(y%4==3)
  {
    if(!color)
      attron(A_BOLD); 
    show_card(card,x,y,1);
    if(!color)
      attroff(A_BOLD);
  }
  else
    show_card(card,x,y,1);
  if(card!=20)
  {
    card_count++;
  }
}

send_one_card(id)
int id;
{
  char msg_buf[255];
  char card;
  char sit;
  int i;

  sit=player[id].sit;
  card=mj[card_point++];
  current_card=card;
  pool[sit].card[pool[sit].num]=card;
  show_num(2,70,144-card_point-16,2);
  sprintf(msg_buf,"314%c%c",sit,2);
  broadcast_msg(id,msg_buf);
  show_newcard(sit,2);
  return_cursor();
  sprintf(msg_buf,"306%c",card_point);
  broadcast_msg(1,msg_buf);
  show_cardmsg(sit,0);
  card_owner=sit;
  sprintf(msg_buf,"305%c",(char) sit);
  broadcast_msg(1,msg_buf);
  clear_check_flag(sit);
  check_flag[sit][3]=check_kang(sit,card);
  check_flag[sit][4]=check_make(sit,card,0);
  in_check[sit]=0;
  for(i=1;i<check_number;i++)
    if(check_flag[sit][i])
    {
      getting_card=1;   
      in_check[sit]=1;
      check_on=1;
      sprintf(msg_buf,"501%c%c%c%c",check_flag[sit][1]+'0',
        check_flag[sit][2]+'0',check_flag[sit][3]+'0',check_flag[sit][4]+'0');
      write_msg(player[table[sit]].sockfd,msg_buf);
      break;
    }
  sprintf(msg_buf,"304%c",card);
  write_msg(player[id].sockfd,msg_buf);
  gettimeofday(&before, (struct timezone *) 0);
}

next_player()
{
  char msg_buf[255];
  turn=next_turn(turn);
  sprintf(msg_buf,"310%c",turn);
  broadcast_msg(1,msg_buf);
  display_point(turn);
  return_cursor();
  if(table[turn]!=1)
  {
    strcpy(msg_buf,"303");
    write_msg(player[table[turn]].sockfd,msg_buf);
    show_newcard(turn,1);
    return_cursor();
  }
  else 
  {
    attron(A_REVERSE);
    show_card(10,INDEX_X+16*2+1,INDEX_Y+1,1);
    attroff(A_REVERSE);
    wrefresh(stdscr);
    return_cursor();
    play_mode=GET_CARD;
    beep1();
  }
  sprintf(msg_buf,"314%c%c",turn,1);
  broadcast_msg(table[turn],msg_buf);
}  

next_turn(current_turn)
int current_turn;
{
  current_turn++;
  if(current_turn==5)
    current_turn=1;
  if(!table[current_turn])
    current_turn=next_turn(current_turn);
  return(current_turn);
}

display_pool(sit)
int sit;
{
  int i;
  char buf[5],msg_buf[255];
  
  msg_buf[0]=0;
  for(i=0;i<pool[sit].num;i++)
  {
    sprintf(buf,"%2d",pool[sit].card[i]);
    strcat(msg_buf,buf);
  }
  display_comment(msg_buf);  
}
sort_pool(sit)
int sit;
{
  int i,j,max;
  char tmp;

  max=pool[sit].num;
  for(i=0;i<max;i++)
    for(j=0;j<max-i-1;j++)
      if(pool[sit].card[j]>pool[sit].card[j+1])
      {
        tmp=pool[sit].card[j];
        pool[sit].card[j]=pool[sit].card[j+1];
        pool[sit].card[j+1]=tmp;
      }
}

sort_card(mode)
int mode;
{
  int i,j,max;
  char tmp;

  if(mode)
    max=pool[my_sit].num+1;
  else  
    max=pool[my_sit].num;
  for(i=0;i<max;i++)
    for(j=0;j<max-i-1;j++)
      if(pool[my_sit].card[j]>pool[my_sit].card[j+1])
      {
        tmp=pool[my_sit].card[j];
        pool[my_sit].card[j]=pool[my_sit].card[j+1];
        pool[my_sit].card[j+1]=tmp;
      }
  for(i=0;i<pool[my_sit].num;i++)
  {
    show_card(20,INDEX_X+(16-pool[my_sit].num+i)*2,INDEX_Y+1,1);
    wrefresh(stdscr);
    show_card(pool[my_sit].card[i],INDEX_X+(16-pool[my_sit].num+i)*2
              ,INDEX_Y+1,1);
    wrefresh(stdscr);
  }
  if(mode)
  {
    show_card(20,INDEX_X+(16-pool[my_sit].num+i)*2+1,INDEX_Y+1,1);
    wrefresh(stdscr);
    show_card(pool[my_sit].card[pool[my_sit].num]
              ,INDEX_X+(16-pool[my_sit].num+i)*2+1,INDEX_Y+1,1);
    wrefresh(stdscr);
  }
  return_cursor();
}

new_game()
{
  clear_screen_area(0,2,18,54);
  show_cardmsg(my_sit,0);
  draw_table();
  wrefresh(stdscr);
}

opening()
{
  char msg_buf[255];
  int i,j;

  new_game();
  input_mode=PLAY_MODE;
  for(i=1;i<=4;i++)
  {
    in_check[i]=0;
    pool[i].first_round=1;
    wait_hit[i]=0;
    pool[i].num=16;
    check_for[i]=0;
    pool[i].out_card_index=0;
    for(j=0;j<8;j++)
      pool[i].flower[j]=0;
    pool[i].time=0;
  }
  display_info();
  current_item=pool[my_sit].num;
  draw_index(pool[my_sit].num+1);
  for(i=0;i<16;i++)
    pool[my_sit].card[i]=0;
  pos_x=INDEX_X+current_item*2+1;
  pos_y=INDEX_Y;
  play_mode=0;
  card_count=0;
  check_on=0;
  for(i=1;i<=4;i++)
  {
    if(i!=my_sit && table[i])
      show_cardback(i);
  }
}

open_deal()
{
   char msg_buf[255];
   int i,j,sit;
   char card;
   
   turn=info.dealer;
   i=generate_random(4);
   pool[i%4+1].door_wind=1;
   pool[(i+1)%4+1].door_wind=2;
   pool[(i+2)%4+1].door_wind=3;
   pool[(i+3)%4+1].door_wind=4;
   sprintf(msg_buf,"518%c%c%c%c",pool[1].door_wind,pool[2].door_wind,
           pool[3].door_wind,pool[4].door_wind);
   broadcast_msg(1,msg_buf);
   wmvaddstr(stdscr,2,64,sit_name[pool[my_sit].door_wind]);
   if(!table[turn])
     turn=next_turn(turn);
   card_owner=turn;
   sprintf(msg_buf,"310%c",turn);
   broadcast_msg(1,msg_buf);
   sprintf(msg_buf,"305%c",card_owner);
   broadcast_msg(1,msg_buf);
   display_point(turn);
   card_point=0;
   card_index=143;
   strcpy(msg_buf,"302");
   generate_card();
   /* send 16 cards to 4 people */
   for(j=1;j<=4;j++)
   {
     if(table[j])
     {
       pool[j].first_round=1;
       for(i=0;i<16;i++)
       {
         card=mj[card_point++];
         pool[j].card[i]=card;
         msg_buf[3+i]=card;
       }
       sort_pool(j);
       msg_buf[3+16]='\0';
       if(table[j]!=1)  /* not server */
       {
         write_msg(player[table[j]].sockfd,msg_buf);
       }
       else
       {
         sort_card(0);
       }
     }
   }
   /* send an additional card to dealer */
   card=mj[card_point++];
   current_card=card;
   sprintf(msg_buf,"304%c",card);
   if(table[turn]!=1)
   {
     show_newcard(turn,2);
     return_cursor();
     write_msg(player[table[turn]].sockfd,msg_buf);
     pool[turn].card[pool[turn].num]=card;
   }
   if(table[turn]==1)  /* turn==server */
   {
     pool[1].card[pool[1].num]=card;
     show_card(pool[1].card[pool[my_sit].num],INDEX_X+16*2+1,pos_y+1,1);
     wrefresh(stdscr);
     return_cursor();
     play_mode=THROW_CARD;
   }
   sprintf(msg_buf,"314%c%c",turn,2);
   broadcast_msg(table[turn],msg_buf);
   in_play=1;
   /* check for flowers for 4 players */
   sit=turn;   /* check dealer first */
   do
   {
   } while(check_begin_flower(sit,pool[sit].card[16],16));
   for(i=0;i<4;i++)
   {
     for(j=0;j<16;j++)
     {
       if(table[sit]) 
       do
       {
       } while(check_begin_flower(sit,pool[sit].card[j],j));
     }
     sort_pool(sit);
     sit=next_turn(sit);
   }
   current_card=pool[turn].card[16];
   show_num(2,70,144-card_point-16,2);
   return_cursor();
   sprintf(msg_buf,"306%c",card_point);
   broadcast_msg(1,msg_buf);
   clear_check_flag(turn);
   check_flag[turn][3]=check_kang(turn,current_card);
   check_flag[turn][4]=check_make(turn,current_card);
   in_check[turn]=0;
   for(i=1;i<check_number;i++)
   {
     if(check_flag[turn][i])
     {
       getting_card=1;
       in_check[turn]=1;
       check_on=1;
       if(in_serv)
       {
         init_check_mode();
       }
       else
       {
         sprintf(msg_buf,"501%c%c%c%c",'0','0',check_flag[turn][3]+'0',
                          check_flag[turn][4]+'0');
         write_msg(player[table[turn]].sockfd,msg_buf);
       }
     }
   }
/*
   sprintf(msg_buf,"304%c",card);
   if(table[turn]!=1)
   {
     show_newcard(turn,2);
     return_cursor();
     write_msg(player[table[turn]].sockfd,msg_buf);
     pool[turn].card[pool[turn].num]=card;
   }
*/
   broadcast_msg(table[turn],"3080");
   if(turn!=my_sit)
     write_msg(player[table[turn]].sockfd,"3081");
   if(turn==my_sit)
     sort_card(1);
   else
     sort_card(0);
   gettimeofday(&before, (struct timezone *) 0);
}

err(errmsg)
char 	*errmsg;
{
  display_comment(errmsg);
}

init_variable()
{
  int i,j;

  my_name[0]=0;
  my_pass[0]=0;
  pass_login=0;
  set_beep=1;
  in_play=0;
  in_serv=0;
  in_join=0;
  in_kang=0;
  new_client=0;
  player_num=0;
  check_x=org_check_x;
  check_y=org_check_y;
  check_number=5;
  input_mode=ASK_MODE;
  info.wind=1;
  info.dealer=1;
  info.cont_dealer=0;
  info.base_value=DEFAULT_BASE;
  info.tai_value=DEFAULT_TAI;
  for(i=0;i<5;i++)
  {
    table[i]=0;
  }
  player[0].money=0;
  on_seat=0;
  check_on=0;
  send_card_on=0;
  send_card_request=0;
  next_player_on=0;
  global_win=newwin(1,63,org_talk_y,11);
  playing_win=newwin(1,43,org_talk_y,11);
  ask.question=1;
  h_head=0;
  h_tail=h_point=1;
  keypad(stdscr,TRUE);
  meta(stdscr,TRUE);
}

clear_variable()
{
  int i;

  for(i=2;i<MAX_PLAYER;i++)
  {
    if(player[i].in_table)
      player[i].in_table=0;
  }
  for(i=1;i<=4;i++)
    table[i]=0;
  player_in_table=0;
}

gps()
{
  int status;
  int i;
  int key;
  int msg_id;
  char msg_buf[255];
  char buf[128];
  char ans_buf[255];

  init_global_screen();
  input_mode=0;
  sprintf(msg_buf,"连往 QKMJ Server %s %d",GPS_IP,GPS_PORT);
  display_comment(msg_buf);
  status=init_socket(GPS_IP,GPS_PORT,&gps_sockfd);
  if(status<0)
  {
    err("无法连往 QKMJ Server");
    endwin();
    exit(0);
  }
  send_gps_line("连线顺利!");
  sprintf(msg_buf,"QKMJ 休闲麻将 Ver %c.%2s Beta",QKMJ_VERSION[0],QKMJ_VERSION+1);
  display_comment(msg_buf);
  display_comment("可以用^C退出, 退格是del键");
  get_my_info();
  sprintf(msg_buf,"100%s",QKMJ_VERSION);
  write_msg(gps_sockfd,msg_buf);
  sprintf(msg_buf,"099%s",my_username);
  write_msg(gps_sockfd,msg_buf);
  pass_count=0;
  if(my_name[0]!=0 && my_pass[0]!=0)
    strcpy(ans_buf,my_name);
  else
  {
    strcpy(ans_buf,my_name);
    do
    {
      ask_question("请输入你的名字：",ans_buf,10,1);
    } while(ans_buf[0]==0);
    ans_buf[10]=0;
  }
  sprintf(msg_buf,"101%s",ans_buf);
  write_msg(gps_sockfd,msg_buf);
  strcpy(my_name,ans_buf);
  nfds=getdtablesize();
  FD_ZERO(&afds);
  FD_SET(gps_sockfd,&afds);
  FD_SET(0,&afds);
  for(;;)
  {
    bcopy((char *) &afds,(char *) &rfds,sizeof(rfds));
    if(select(nfds,&rfds,(fd_set *)0, (fd_set *)0, 0)<0)
    {
      if(errno!=EINTR)
        display_comment("Select Error!");
      continue;
    }
    if(FD_ISSET(0,&rfds))
    {
      if(input_mode)
        process_key();
    }
    /* Check for data from GPS */
    if(FD_ISSET(gps_sockfd,&rfds))
    {
      if(!read_msg_id(gps_sockfd,buf))
      {
        display_comment("Closed by QKMJ Server.");
        shutdown(gps_sockfd,2);
        if(in_join)
          close_join();
        if(in_serv)
          close_serv();
        endwin();
        exit(0);
      }
      else
      {
        process_msg(0,buf,FROM_GPS);
        buf[0]='\0';
      }
    }
    if(in_serv)
    {
      /* Check for new connections */
      if(FD_ISSET(serv_sockfd,&rfds))
      {
        if(new_client)
        {
          accept_new_client();
        }
/*
          else
            display_comment("Error from new client!");
*/
      }
      /* Check for data from client */
      for(i=2;i<MAX_PLAYER;i++)
      {
        if(player[i].in_table)
        {
          if(FD_ISSET(player[i].sockfd,&rfds))
          {
            if(read_msg_id(player[i].sockfd,buf)==0)
              close_client(i);
            else
              process_msg(i,buf,FROM_CLIENT);
          }
        }
      }
      /* Waiting for signals from each sit */
      if(waiting)
      {
        for(i=1;i<=4;i++)
        {
          if(table[i] && !wait_hit[i])
            goto continue_waiting;
        }
        waiting=0;
        broadcast_msg(1,"290");
        opening();
        open_deal();
      }
      continue_waiting:;
      /* Process the cards */
      if(check_on)
      {
        /* find if there are still player in check */
        for(i=1;i<=4;i++)
        {
          if(table[i] && in_check[i])
            goto still_in_check;
        }
        check_on=0;
        next_player_on=1;
        send_card_on=1;
        compare_check();
        still_in_check:;
      }
      if(next_player_request && next_player_on)
      {
        if(144-card_point<=16)
        {
          for(i=1;i<=4;i++)
          {
            if(table[i] && i!=my_sit)
            {
              show_allcard(i);
              show_kang(i);
            }
          }
          info.cont_dealer++;
          send_pool_card();
          broadcast_msg(1,"330");
          clear_screen_area(THROW_Y,THROW_X,8,34);
          wmvaddstr(stdscr,THROW_Y+3,THROW_X+12,"海 底 流 局");
          return_cursor();
          wait_a_key(PRESS_ANY_KEY_TO_CONTINUE);
          broadcast_msg(1,"290");
          opening();
          open_deal(); 
        }
        else
        {  
          next_player();
          next_player_request=0;
        }
      }
      if(send_card_request && send_card_on)
      {
        send_one_card(table[turn]);
        send_card_request=0;
      }
    }
    if(in_join)
    {
      if(FD_ISSET(table_sockfd,&rfds))
      {
        if(!read_msg_id(table_sockfd,buf))
        {
          close(table_sockfd);
          FD_CLR(table_sockfd,&afds);
          in_join=0;
          input_mode=TALK_MODE;
          init_global_screen();
        }
        else
          process_msg(1,buf,FROM_SERV);
      }
    }
  }
}

read_qkmjrc()
{
  FILE *qkmjrc_fp;
  char msg_buf[256];
  char msg_buf1[256];
  char event[80];
  char *str1;
  char rc_name[255];

  sprintf(rc_name,"%s/%s",getenv("HOME"),QKMJRC);
  if((qkmjrc_fp=fopen(rc_name,"r"))!=NULL)
  {
    while(fgets(msg_buf,80,qkmjrc_fp)!=NULL)
    {
      Tokenize(msg_buf);
      strupr(event,cmd_argv[1]);
      if(strcmp(event,"LOGIN")==0)
      {
        if(narg>1)
        {
          cmd_argv[2][10]=0;
          strcpy(my_name,cmd_argv[2]);
        }
      }
      else if(strcmp(event,"PASSWORD")==0)
      {
        if(narg>1)
        {
          cmd_argv[2][8]=0;
          strcpy(my_pass,cmd_argv[2]);
        }
      }
      else if(strcmp(event,"SERVER")==0)
      {
        if(narg>1)
        {
          strcpy(GPS_IP,cmd_argv[2]);
        }
        if(narg>2)
        {
          GPS_PORT=atoi(cmd_argv[3]);
        }
      }
      else if(strcmp(event,"NOTE")==0)
      {
        if(narg>1)
        {
          str1=strtok(msg_buf," \n\t\r");
          str1=strtok('\0',"\n\t\r");
          strcpy(my_note,str1);
        }
      } 
      else if(strcmp(event,"BEEP")==0)
      {
        if(narg>1)
        {
          if(strcmp(strupr(msg_buf1,cmd_argv[2]),"OFF")==0)
          {
            set_beep=0;
          }
        }
      }  
    }
    fclose(qkmjrc_fp);
  }
}


main(argc, argv)
int	argc;
char	*argv[];
{
  setenv("TERM", "vt100", 1);
  initscr();
  cbreak();
  noecho();
  nonl();
  /*
  attrset(A_NORMAL);
  */
  clear();
  signal(SIGINT,leave);
  signal(SIGIOT,leave);
  signal(SIGPIPE,leave);
  init_variable();
  strcpy(GPS_IP,DEFAULT_GPS_IP);
  GPS_PORT=DEFAULT_GPS_PORT;
  read_qkmjrc();
  /*if(argc>=3)
  {
    strcpy(GPS_IP,argv[1]);
    GPS_PORT=atoi(argv[2]);
  }
  else if(argc==2)
  {
    strcpy(GPS_IP,argv[1]);
    GPS_PORT=DEFAULT_GPS_PORT;
  }*/
  gps();
  exit(0); 
}

