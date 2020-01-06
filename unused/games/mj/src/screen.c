#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "curses.h"
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include "mjdef.h"
#include "qkmj.h"
  
set_color(fore,back)
int fore;
int back;
{
  char msg_buf[80];
  wrefresh(stdscr);
  if(color)
  {
    fprintf(stdout,"[%d;%dm",fore,back);
    fflush(stdout);
/*
    sprintf(msg_buf,"[%d;%dm",fore,back);
    waddstr(stdscr,msg_buf);
*/
  }
}

set_mode(mode)
int mode;
{
  char msg_buf[80];
  wrefresh(stdscr);
  if(color)
  {
    printf("[%dm",mode);
    fflush(stdout);
/*
    sprintf(msg_buf,"\033%dm",mode);
    waddstr(stdscr,msg_buf);
*/
  }
}

mvprintstr(win,y,x,msg)
WINDOW *win;
int y;
int x;
char *msg;
{
  wmove(win,y,x);
  printstr(win,msg); 
}

printstr(win,str)
WINDOW *win;
char *str;
{
  int i,len;
  len=strlen(str);
  for(i=0;i<len;i++)
    printch(win,str[i]);
}

printch(win,ch)
WINDOW *win;
char ch;
{
  char msg[3];
    msg[0]=ch;
    msg[1]='\0';
    waddstr(win,msg);
} 
 
mvprintch(win,y,x,ch)
WINDOW *win;
int y,x;
char ch;
{
  wmove(win,y,x);
  printch(win,ch);
}

clear_screen_area (ymin, xmin, height, width)
int ymin, xmin, height, width;
{
	int i;
  char line_buf[255];

  for (i = 0; i < width; i++) line_buf[i] = ' ';
	line_buf[width] = '\0';
  for (i = 0; i < height; i++)
    wmvaddstr (stdscr,ymin+i, xmin, line_buf);
  wrefresh(stdscr);
}

clear_input_line()
{
  werase(inputwin);
  talk_x=0;
  wmove(inputwin,0,talk_x);
  talk_buf_count=0;
  talk_buf[0]='\0';
  wrefresh(inputwin);
}

wait_a_key(msg)
char *msg;
{
  int ch;
  werase(inputwin);
  wmvaddstr(inputwin,0,0,msg);
  wrefresh(inputwin);
  beep1();
  do(ch=my_getch());
  while(ch!=KEY_ENTER && ch!=ENTER);
  werase(inputwin);
  mvwaddstr(inputwin,0,0,talk_buf);
  wrefresh(inputwin);
}

ask_question(question,answer,ans_len,type)
char *question;
char *answer;
int ans_len;
int type;
{
  werase(inputwin);
  wmvaddstr(inputwin,0,0,question);
  wrefresh(inputwin);
  mvwgetstring(inputwin,0,strlen(question),ans_len,answer,type);
  werase(inputwin);
  wmvaddstr(inputwin,0,0,talk_buf);
  wrefresh(inputwin);
}

draw_index(max_item)
int max_item;
{
  int i;
  /* normal();  */
  for(i=1; i<max_item; i++)
  {
    wmvaddstr(stdscr,INDEX_Y,INDEX_X+(17-max_item+i)*2-2,menu_item[i]);
  }
  wmvaddstr(stdscr,INDEX_Y,INDEX_X+16*2+1,menu_item[max_item]);
  return_cursor();
}

current_index(current)
int current;
{
  wmove(stdscr,INDEX_Y, INDEX_X+current*2);
  wrefresh(stdscr);
}

show_cardback(sit)
char sit;
{
  int i;

  switch((sit-my_sit+4)%4)
  {
    case 0:
      break;
    case 1:
      for(i=0;i<pool[sit].num;i++)
      {
        show_card(40,INDEX_X1,INDEX_Y1-i,0);
      }
      break;
    case 2:
      for(i=0;i<pool[sit].num;i++)
      {
        show_card(30,INDEX_X2-i*2,INDEX_Y2,1); 
      }
      break;
    case 3:
      for(i=0;i<pool[sit].num;i++)
      {
        show_card(40,INDEX_X3,INDEX_Y3+i,0);
      }
      break;
  }
  return_cursor();
}

show_allcard(sit)
char sit;
{
  int i;
  
  switch((sit-my_sit+4)%4)
  {
    case 0:
      break;
    case 1:
      for(i=0;i<pool[sit].num;i++)
      {
        show_card(pool[sit].card[i],INDEX_X1,INDEX_Y1-(16-pool[sit].num+i),0);
      }
      break;
    case 2:
      for(i=0;i<pool[sit].num;i++)
      {
        show_card(pool[sit].card[i],INDEX_X2-(16-pool[sit].num+i)*2,INDEX_Y2,1);
      }
      break;
    case 3:
      for(i=0;i<pool[sit].num;i++)
      {
        show_card(pool[sit].card[i],INDEX_X3,INDEX_Y3+(16-pool[sit].num+i),0);
      }
      break;
  }
  return_cursor();
}

show_kang(sit)
char sit;
{
  int i;

  switch((sit-my_sit+4)%4)
  {
    case 0:
      break;
    case 1:
      for(i=0;i<pool[sit].out_card_index;i++)
        if(pool[sit].out_card[i][0]==11)  /* °µ¸Ü */
        {
          show_card(pool[sit].out_card[i][2],INDEX_X1,INDEX_Y1-i*3-1,0);
        }
      break;
    case 2:
      for(i=0;i<pool[sit].out_card_index;i++)
        if(pool[sit].out_card[i][0]==11)
        {
          show_card(pool[sit].out_card[i][2],INDEX_X2-i*6-2,INDEX_Y2,1);
        }
      break;
    case 3:
      for(i=0;i<pool[sit].out_card_index;i++)
        if(pool[sit].out_card[i][0]==11)
        {
          show_card(pool[sit].out_card[i][2],INDEX_X3,INDEX_Y3+i*3+1,0);
        }
      break;
  }
}
       
show_newcard(sit,type)
char sit;
char type;
/*  type 1 : ÃþÅÆ  */
/*  type 2 : ÃþÈë  */
/*  type 3 : ¶ª³ö  */
/*  type 4 : ÏÔÊ¾  */
{
  int i;

  switch((sit-my_sit+4)%4)
  {
    case 0:
      switch(type)
      {
        case 2:
          show_card(current_card,INDEX_X,INDEX_Y+1,1);
        case 4:
          show_card(current_card,INDEX_X+16*2+1,INDEX_Y+1,1);
          break;
      }
      break;
    case 1:
      switch(type)
      {
        case 1:
/*
          attron(A_BOLD);
          show_card(10,INDEX_X1,INDEX_Y1-17,0);
          attroff(A_BOLD);
*/
          break;
        case 2:
          show_card(40,INDEX_X1,INDEX_Y1-17,0);
          break;
        case 3:
          show_card(20,INDEX_X1,INDEX_Y1-17,0);
          break;
        case 4:
          show_card(current_card,INDEX_X1,INDEX_Y1-17,0);
          break;
      }
      break;
    case 2:
      switch(type)
      {
        case 1:
/*
          attron(A_BOLD);
          show_card(10,INDEX_X2-16*2-1,INDEX_Y2,1);
          attroff(A_BOLD);
*/
          break;
        case 2:
          show_card(30,INDEX_X2-16*2-1,INDEX_Y2,1);
          show_card(20,INDEX_X2-16*2-1,INDEX_Y2,1);
          show_card(30,INDEX_X2-16*2-1,INDEX_Y2,1);
          break;
        case 3:
          show_card(20,INDEX_X2-16*2-1,INDEX_Y2,1);
          break;
        case 4:
          show_card(current_card,INDEX_X2-16*2-1,INDEX_Y2,1);
          break;
      }
      break;
    case 3:
      switch(type)
      {
        case 1:
/*
          attron(A_BOLD);
          show_card(10,INDEX_X3,INDEX_Y3+17,0);
          attroff(A_BOLD);
*/
          break;
        case 2:
/* Îª´¦Àí color µÄÎÊÌâ */
          show_card(40,INDEX_X3,INDEX_Y3+17,0);
        /*  show_card(20,INDEX_X3,INDEX_Y3+17,0); */
          show_card(40,INDEX_X3,INDEX_Y3+17,0);
          break;
        case 3:
          show_card(20,INDEX_X3,INDEX_Y3+17,0);
          break;
        case 4:
          show_card(current_card,INDEX_X3,INDEX_Y3+17,0);
          break;
      }
      break;
  }
}
      
/* Show cards on the screen. */
/* type   0: row             */
/* type   1: column          */
show_card(card,x,y,type)
  char card;
  int x;
  int y;
  int type;
{
  char card1[3];
  char card2[3];
  
  reset_cursor();
  mvwaddstr(stdscr,y,x,"  "); 
  wrefresh(stdscr);
  wmove(stdscr,y,x);
if(card==30 || card==40)
  set_color(32,40);
if(card>=1 && card<=9)
{
  set_mode(1);
  set_color(31,40);
}
else if(card>=11 && card<=19)
{
  set_mode(1);
  set_color(32,40);
}
else if(card>=21 && card<=29)
{
  set_mode(1);
  set_color(36,40);
}
else if(card>=31 && card<=34)
{
  set_mode(1);
  set_color(33,40);
}
else if(card>=41 && card<=43)
{
  set_mode(1);
  set_color(35,40);
}
  if(type==1)
  {
    card1[0]=mj_item[card][0];
    card1[1]=mj_item[card][1];
    card1[2]=0;
    card2[0]=mj_item[card][2];
    card2[1]=mj_item[card][3];
    card2[2]=0;
    mvwaddstr(stdscr,y,x,card1);
    mvwaddstr(stdscr,y+1,x,card2);
  }
  else
    mvwaddstr(stdscr,y,x,mj_item[card]);
wrefresh(stdscr);
set_color(37,40);
set_mode(0);
}

draw_title()
{
  int x,y;

  for(y=0;y<24;y++)
  {
    mvprintstr(stdscr,y,0,"©¦");
    mvprintstr(stdscr,y,76,"©¦");
  }
  mvprintstr(stdscr,22,0,"©À");
  for(x=2;x<=75;x+=2)
  {
   printstr(stdscr,"©¤");
  }
  mvprintstr(stdscr,22,76,"©È");
  wmvaddstr(stdscr,23,2,"¡¾¶Ô»°¡¿");
  wrefresh(stdscr);
}

init_playing_screen()
{
  info.wind=1;
  info.dealer=1;
  info.cont_dealer=0;
  talk_right=55;
  inputwin=playing_win;
  talk_x=0;
  talk_y=0;
  talk_buf_count=0;
  talk_buf[0]='\0';
  comment_right=55;
  comment_up=19;
  comment_bottom=22;
  comment_y=19;
  commentwin=newwin(comment_bottom-comment_up+1,comment_right-comment_left+1,
                    comment_up,comment_left);
  scrollok(commentwin,TRUE);
  screen_mode=PLAYING_SCREEN_MODE;
  draw_playing_screen();
  wmove(inputwin,talk_y,talk_x);
  wrefresh(inputwin);
}

init_global_screen()
{
  char msg_buf[255];
  char ans_buf[255];

  comment_left=comment_x=org_comment_x;
  comment_right=72;
  comment_up=0;
  comment_bottom=21;
  comment_y=org_comment_y;
  talk_left=11;
  talk_right=74;
  talk_x=0;
  talk_buf_count=0;
  talk_buf[0]='\0';
  screen_mode=GLOBAL_SCREEN_MODE;
  commentwin=newwin(22,74,0,2);
  scrollok(commentwin,TRUE);
  inputwin=newwin(1,talk_right-talk_left,org_talk_y,talk_left);
  draw_global_screen();
  input_mode=TALK_MODE;
  talk_left=11;
  inputwin=global_win;
  wmvaddstr(stdscr,23,2,"¡¾¶Ô»°¡¿");
  return_cursor();
}
  
int wmvaddstr(win,y,x,str)
WINDOW *win;
int y;
int x;
char *str;
{
  wmove(win,y,x);
  waddstr(win,str);
}

draw_table()
{
  int x,y;

  wmvaddstr(stdscr,4,10,"¡õ");
  for(x=13;x<=45;x+=2)
  {
    waddstr(stdscr,"©¤");
  }
  waddstr(stdscr,"¡õ");
  for(y=5;y<=12;y++)
  {
    wmvaddstr(stdscr,y,10,"©¦");
    wmvaddstr(stdscr,y,46,"©¦");
  }
  wmvaddstr(stdscr,13,10,"¡õ");
  for(x=12;x<=44;x+=2)
  {
    waddstr(stdscr,"©¤");
  }
  waddstr(stdscr,"¡õ");
}

draw_global_screen()
{
  clear();
  draw_title(); 
}

draw_playing_screen()
{
  int x,y;
  clear();
  /* Draw outline */
  for(y=0;y<=23;y++)
  {
    wmvaddstr(stdscr,y,0,"©¦");
    wmvaddstr(stdscr,y,56,"©¦");
    wmvaddstr(stdscr,y,76,"©¦");
  }
  wmvaddstr(stdscr,18,0,"©À");
  for(x=3;x<=77;x+=2)
  {
    waddstr(stdscr,"©¤");
  }
  wmvaddstr(stdscr,18,76,"©È");
  wmvaddstr(stdscr,18,56,"©à");
  wmvaddstr(stdscr,23,2,"¡¾¶Ô»°¡¿ ");
  /* Draw table outline */
  /* Information section */
  wmvaddstr(stdscr,1,56,"©À");
  for(x=58;x<=74;x+=2)
    waddstr(stdscr,"©¤");
  waddstr(stdscr,"©È");
  wmvaddstr(stdscr,3,56,"©À");
  for(x=58;x<=74;x+=2)
    waddstr(stdscr,"©¤");
  waddstr(stdscr,"©È");
  wmvaddstr(stdscr,12,56,"©À");
  for(x=58;x<=74;x+=2)
    waddstr(stdscr,"©¤");
  waddstr(stdscr,"©È");
  wmvaddstr(stdscr,0,66,"©¦");
  wmvaddstr(stdscr,1,66,"©à");
  wmvaddstr(stdscr,2,66,"©¦");
  wmvaddstr(stdscr,3,66,"©Ø");
  /* Characters */
  wmvaddstr(stdscr,0,60,"·ç  ¾Ö");
  wmvaddstr(stdscr,0,68,"Á¬    ×¯");
  wmvaddstr(stdscr,2,58,"ÃÅ·ç£º");
  wmvaddstr(stdscr,2,68,"Ê£    ÕÅ");
  wmvaddstr(stdscr,4,58,"¶«¼Ò£º");
  wmvaddstr(stdscr,6,58,"ÄÏ¼Ò£º");
  wmvaddstr(stdscr,8,58,"Î÷¼Ò£º");
  wmvaddstr(stdscr,10,58,"±±¼Ò£º");
  wmvaddstr(stdscr,5,62,"¡ç");
  wmvaddstr(stdscr,7,62,"¡ç");
  wmvaddstr(stdscr,9,62,"¡ç");
  wmvaddstr(stdscr,11,62,"¡ç");
  wmvaddstr(stdscr,13,60,"0  1  2  3  4");
  wmvaddstr(stdscr,14,60,"ÎÞ ³Ô Åö ¸Ü ºú");
  wmvaddstr(stdscr,20,64,"©°  ©´");
  wmvaddstr(stdscr,22,64,"©¸  ©¼");
  wrefresh(stdscr);
  wmove(inputwin,talk_y,talk_x);
  wrefresh(inputwin);
}

find_point(pos)
int pos;
{
  switch(pos)
  {
    case 0:
      wmove(stdscr,22,66);
      break;
    case 1:
      wmove(stdscr,21,68);
      break;
    case 2:
      wmove(stdscr,20,66);
      break;
    case 3:
      wmove(stdscr,21,64);
      break;
  }
}

display_point(current_turn)
int current_turn;
{
  static int last_turn=0;
  char msg_buf[255];

  if(last_turn)
  {
    find_point((last_turn+4-my_sit)%4);
    waddstr(stdscr,"  ");
    wrefresh(stdscr);
    find_point((last_turn+4-my_sit)%4);
    waddstr(stdscr,sit_name[last_turn]);
  }
  find_point((current_turn+4-my_sit)%4);
  waddstr(stdscr,"  ");
  wrefresh(stdscr);
  find_point((current_turn+4-my_sit)%4);
  attron(A_REVERSE);
  waddstr(stdscr,sit_name[current_turn]);
  attroff(A_REVERSE);
  last_turn=current_turn;
}

display_time(sit)
char sit;
{
  char msg_buf[255];
  char pos;
  int min,sec;

  pos=(sit-my_sit+4)%4;
  min=(int) pool[sit].time/60;
  min=min%60;  
  sec=(int) pool[sit].time%60;
  sprintf(msg_buf,"%2d:%02d",min,sec);
  switch(pos)
  {
    case 0:
      wmvaddstr(stdscr,23,64,"     ");
      wmvaddstr(stdscr,23,64,msg_buf);
      break;
    case 1:
      wmvaddstr(stdscr,21,71,"     ");
      sprintf(msg_buf,"%d:%02d",min,sec);
      wmvaddstr(stdscr,21,71,msg_buf);
      break;
    case 2:
      wmvaddstr(stdscr,19,64,"     ");
      wmvaddstr(stdscr,19,64,msg_buf);
      break;
    case 3:
      wmvaddstr(stdscr,21,58,"     ");
      wmvaddstr(stdscr,21,58,msg_buf);
      break;
  }
  return_cursor();
}

display_info()
{
  int i;

  wmvaddstr(stdscr,0,58,"  ");
  wmvaddstr(stdscr,0,62,"  ");
  wmvaddstr(stdscr,0,70,"    ");
  wrefresh(stdscr);
  wmvaddstr(stdscr,0,58,sit_name[info.wind]);
  wmvaddstr(stdscr,0,62,sit_name[info.dealer]);
  if(info.cont_dealer<10)
    show_num(0,71,info.cont_dealer,1);
  else
    show_num(0,70,info.cont_dealer,2);
  wmvaddstr(stdscr,22,66,sit_name[my_sit]);
  wmvaddstr(stdscr,21,68,sit_name[(my_sit)%4+1]);
  wmvaddstr(stdscr,20,66,sit_name[(my_sit+1)%4+1]);
  wmvaddstr(stdscr,21,64,sit_name[(my_sit+2)%4+1]);
  wmvaddstr(stdscr,4,64,player[table[EAST]].name);
  wmvaddstr(stdscr,6,64,player[table[SOUTH]].name);
  wmvaddstr(stdscr,8,64,player[table[WEST]].name);
  wmvaddstr(stdscr,10,64,player[table[NORTH]].name);
  for(i=1;i<=4;i++)
  {
    display_time(i);
  }
  mvwprintw(stdscr,5,64,"         ");
  mvwprintw(stdscr,5,64,"%ld",player[table[EAST]].money);
  mvwprintw(stdscr,7,64,"         ");
  mvwprintw(stdscr,7,64,"%ld",player[table[SOUTH]].money);
  mvwprintw(stdscr,9,64,"         ");
  mvwprintw(stdscr,9,64,"%ld",player[table[WEST]].money);
  mvwprintw(stdscr,11,64,"         ");
  mvwprintw(stdscr,11,64,"%ld",player[table[NORTH]].money);
  mvwprintw(stdscr,4,74,"  ");
  mvwprintw(stdscr,6,74,"  ");
  mvwprintw(stdscr,8,74,"  ");
  mvwprintw(stdscr,10,74,"  ");
  attron(A_REVERSE);
  mvwprintw(stdscr,2+info.dealer*2,74,"×¯");
  attroff(A_REVERSE);
  wrefresh(stdscr);
  wmove(inputwin,talk_y,talk_x);
  wrefresh(inputwin);
}

int more_size=0, more_num=0;
char more_buf[4096];
int readln(fd,buf,end_flag)
int fd ;
char *buf ;
int *end_flag;
{
    int len, bytes, in_esc, ch;

    len = bytes = in_esc = 0;
    *end_flag=0;
    while( 1 ) {
        if( more_num >= more_size ) {
            more_size = read( fd, more_buf, 1);
            if( more_size == 0 ) {
                break;
            }
            more_num = 0;
        }
        ch = more_buf[ more_num++ ];
        bytes++;
        if( ch == '\n' || len >= 74 ) {
            break;
        } else if( ch == '\0' ) {
            *end_flag=1;
            *buf='\0'; 
            return bytes;
/*
        } else if( ch == '\t' ) {
            do {
                len++, *buf++ = ' ';
            } while( (len % 8) != 0 );
        } else if( ch == '\033' ) {
            if( showansi )  *buf++ = ch;
            in_esc = 1;
        } else if( in_esc ) {
            if( showansi )  *buf++ = ch;
            if( strchr( "[0123456789;,", ch ) == NULL ) {
                in_esc = 0;
            }
*/
        } else {
            len++, *buf++ = ch;
        }
    }
    *buf++ = ch;
    *buf = '\0';
    return bytes;
}

display_news(fd)
int fd;
{
  char buf[256];
  int bytes;
  int line_count;
  int end_flag;
  WINDOW *news_win;
  
  line_count=0;
  news_win=newwin(22,74,0,2); 
  wclear(news_win);
  wmove(news_win,0,0);
  while(readln(fd,buf,&end_flag))
  {
    if(end_flag==1)
      break;
    if(line_count==22)
    {
      wrefresh(news_win);
      wait_a_key("Çë°´ <ENTER> ÖÁÏÂÒ»Ò³......");
      line_count=0;
      wclear(news_win);
      wmove(news_win,0,0);
    }
    printstr(news_win,buf);
wrefresh(news_win);
    line_count++;
  }
  wrefresh(news_win);
  wait_a_key("Çë°´ <ENTER> ¼ÌÐø.......");
  delwin(news_win);
  redraw_screen();
}

display_comment(comment)
char *comment;
{
  waddstr(commentwin,"\n"); 
  printstr(commentwin,comment);
  wrefresh(commentwin);
  return_cursor();
}

send_talk_line(talk)
char *talk;
{
  char comment[255];
  char msg_buf[255];
  int i;

  sprintf(comment,"<%s> ",my_name);
  strcat(comment,talk);
  display_comment(comment);
  sprintf(msg_buf,"102%s",comment);
  if(in_serv)
  {
    broadcast_msg(1,msg_buf);
  }
  if(in_join)
  {
    write_msg(table_sockfd,msg_buf);
  }
}

send_gps_line(msg)
char *msg;
{
  char comment[255];
/*
  sprintf(comment,"¡õ ");
  strcat(comment,msg);
*/
strcpy(comment,msg);
  display_comment(comment);
}

intlog10(num)
int num;
{
  int i;

  i=0;
  do
  {
    num/=10;
    if(num>=1)
      i++;
  } while(num>=1);
  return(i);
}

convert_num(str,number,digit)
char *str;
int number;
int digit;
{
  int i;
  int tmp[10];
  for(i=digit-1;i>=0;i--)
  {
    tmp[i]=number%10;
    number/=10;
  }
  for(i=0;i<digit;i++)
    strcpy(str+i*2,number_item[tmp[i]]);
}
    
show_num(y,x,number,digit) 
int y;
int x;
int number;
int digit;
{
  int i;
  char msg_buf[255];
  wmove(stdscr,y,x);
  for(i=0;i<digit;i++)
    waddstr(stdscr,"  ");
  wrefresh(stdscr);
  convert_num(msg_buf,number,digit);
  wmvaddstr(stdscr,y,x,msg_buf);
  wrefresh(stdscr);
}

show_cardmsg(sit,card,type)
int sit;
char card;
int type;
{
  int pos;

  pos=(sit-my_sit+4)%4;
  clear_screen_area(15,58,3,18);
  if(card)
  { 
    wmvaddstr(stdscr,15,58,"©°©¤©¤©¤©¤©¤©¤©¤©´");
    wmvaddstr(stdscr,16,58,"©¦              ©¦");
    wmvaddstr(stdscr,17,58,"©¸©¤©¤©¤©¤©¤©¤©¤©¼");
    switch(pos)
    {
      case 0:
        wmvaddstr(stdscr,16,60,"Íæ");
        break;
      case 1:
        wmvaddstr(stdscr,16,60,"ÏÂ");
        break;
      case 2:
        wmvaddstr(stdscr,16,60,"¶Ô");
        break;
      case 3:
        wmvaddstr(stdscr,16,60,"ÉÏ");
        break;
    }
    wmvaddstr(stdscr,16,62,"¼Ò´ò£¨    £©");
    wrefresh(stdscr);
    show_card(card,68,16,0);
  }
  return_cursor();
}

redraw_screen()
{
  int i,j;

  clearok(stdscr,TRUE);
  wrefresh(stdscr);
  touchwin(commentwin);
  wrefresh(commentwin);
  touchwin(inputwin);
  wrefresh(inputwin);
  return_cursor();
/*
  if(screen_mode==PLAYING_SCREEN_MODE)
  {
    for(i=1;i<=4;i++)
    {
      if(table[i] && i!=my_sit)
        show_cardback(i);
    }
    if(turn==card_owner)
      show_newcard(turn,2);
    for(i=0;i<3;i++)
      for(j=0;j<17;j++)
      {
        if(table_card[i][j]!=0 && table_card[i][j]!=20)
        {
          show_card(20,THROW_X+j*2,THROW_Y+i*2,1);
          show_card(table_card[i][j],THROW_X+j*2,THROW_Y+i*2,1);
        }
      }
    sort_card(0);
  }
*/
}

reset_cursor()
{
  mvwaddstr(stdscr,23,75," ");
  wrefresh(stdscr);
}

return_cursor()
{
  switch(input_mode)
  {
    case ASK_MODE:
    case TALK_MODE:
      reset_cursor();
      wmove(inputwin,talk_y,talk_x);
      wrefresh(inputwin);
      break;
    case CHECK_MODE:
      wmove(stdscr,org_check_y,check_x);
      wrefresh(stdscr);
      break;
    case PLAY_MODE:
      wmove(stdscr,pos_y,pos_x);
      wrefresh(stdscr);
      break;
    case EAT_MODE:
      wmove(stdscr,eat_y,eat_x);
      wrefresh(stdscr);
      break;
  }
}


