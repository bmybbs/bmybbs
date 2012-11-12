#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "curses.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mjdef.h"
#include "qkmj.h"

init_check_mode()
{
  int i;

  if(input_mode==TALK_MODE)
    current_mode=CHECK_MODE;
  else
    input_mode=CHECK_MODE;
  current_check=0;
  check_x=org_check_x;
  attron(A_REVERSE);
  wmvaddstr(stdscr,org_check_y+1,org_check_x,"  ");
  wrefresh(stdscr);
  wmvaddstr(stdscr,org_check_y+1, org_check_x,check_name[0]);
  reset_cursor();
  for(i=1;i<check_number;i++)
  {
    if(check_flag[my_sit][i])
    {
      wmvaddstr(stdscr,org_check_y+1,org_check_x+i*3,"  ");
      wrefresh(stdscr);
      wmvaddstr(stdscr,org_check_y+1,org_check_x+i*3,check_name[i]);
      reset_cursor();
    }
  }
  attroff(A_REVERSE);
  beep1();
  wrefresh(stdscr);
  return_cursor();
}

process_make(sit,card)
char sit;
char card;
{
  int i,j,k,max_sum,max_index;
  char msg_buf[80];
  long change_money[5];

  play_mode=WAIT_CARD;
  for(i=0;i<=4;i++)
    change_money[i]=0;
  current_card=card;
  check_make(sit,card,1);
  set_color(37,40);
  clear_screen_area(THROW_Y,THROW_X,8,34);
  max_index=0;
  max_sum=card_comb[0].tai_sum;
  for(i=0;i<comb_num;i++)
  {
    if(card_comb[i].tai_sum>max_sum)
    {
      max_index=i;
      max_sum=card_comb[i].tai_sum;
    }
  }
  set_color(32,40);
  set_mode(1);
  wmove(stdscr,THROW_Y,THROW_X);
  wprintw(stdscr,"%s ",player[table[sit]].name);
  if(sit==card_owner)
    wprintw(stdscr,"自摸");
  else
  {
    wprintw(stdscr,"胡牌");
    wmove(stdscr,THROW_Y,THROW_X+19);
    wprintw(stdscr,"%s ",player[table[card_owner]].name);
    wprintw(stdscr,"放枪");
  }
  set_color(33,40);
  j=1;
  k=0;
  for(i=0;i<=51;i++)
  {
    if(card_comb[max_index].tai_score[i])
    {
      wmove(stdscr,THROW_Y+j,THROW_X+k);
      wprintw(stdscr,"%-10s%2d 台",tai[i].name,
              card_comb[max_index].tai_score[i]);
      j++;
      if(j==7)
      {
        j=2;
        k=18;
      }
    }
  }
  if(card_comb[max_index].tai_score[52])  /* 连庄 */
  {
    wmove(stdscr,THROW_Y+j,THROW_X+k);
    if(info.cont_dealer<10)
      wprintw(stdscr,"连%s拉%s  %2d 台",number_item[info.cont_dealer],
              number_item[info.cont_dealer],info.cont_dealer*2);
    else
      wprintw(stdscr,"连%2d拉%2d  %2d 台",info.cont_dealer,info.cont_dealer,
              info.cont_dealer*2);
  }
  set_color(31,40);
  wmove(stdscr,THROW_Y+6,THROW_X+26);
  wprintw(stdscr,"共 %2d 台",card_comb[max_index].tai_sum);
  if((sit==card_owner && sit!=info.dealer) || 
     (sit!=card_owner && card_owner==info.dealer))   
  {
      if(info.cont_dealer>0)
      {
        wmove(stdscr,THROW_Y+7,THROW_X+15);
        if(info.cont_dealer<10)
          wprintw(stdscr,"庄家 连%s拉%s %2d 台",number_item[info.cont_dealer],
                  number_item[info.cont_dealer],card_comb[max_index].tai_sum+1
                  +info.cont_dealer*2);
        else
          wprintw(stdscr,"庄家 连%2d拉%2d %2d 台",info.cont_dealer,
                  info.cont_dealer,card_comb[max_index].tai_sum+1+
                  info.cont_dealer*2);
      }
      else
      {
        wmove(stdscr,THROW_Y+7,THROW_X+24);
        wprintw(stdscr,"庄家 %2d 台",
        card_comb[max_index].tai_sum+1);
      }
  }
  wrefresh(stdscr);
  set_color(37,40);
  set_mode(0);
  for(i=1;i<=4;i++)
  {
    if(table[i] && i!=my_sit)
    {
      show_allcard(i);
      show_kang(i);
    }
  }
  show_newcard(sit,4);
  return_cursor();
  /* Process money */
  if(sit==card_owner)  /* 自摸 */
  {
    for(i=1;i<=4;i++)
    {
      if(i!=sit)
      {
        if(i==info.dealer)
          change_money[i]=-(info.base_value+
                           (card_comb[max_index].tai_sum+1+info.cont_dealer*2)*
                           info.tai_value);
        else
          change_money[i]=-(info.base_value+card_comb[max_index].tai_sum*
                            info.tai_value);
        change_money[sit]+=-change_money[i];
      }
    }
  }
  else       /* 别人放枪 */
  { 
    if(card_owner==info.dealer)
      change_money[card_owner]=-(info.base_value+(card_comb[max_index].tai_sum+
                                 1+info.cont_dealer*2)*info.tai_value);
    else 
      change_money[card_owner]=-(info.base_value+card_comb[max_index].tai_sum*
                                 info.tai_value);
    change_money[sit]+=-change_money[card_owner];
  }
  /* Send money info to GPS */
  if(in_serv)
  {
    for(i=1;i<=4;i++)
    {
      if(table[i]) 
      {
        sprintf(msg_buf,"020%5d%ld",player[table[i]].id,
                player[table[i]].money+change_money[i]);
        write_msg(gps_sockfd,msg_buf);
      }
    }
  }
  wait_a_key(PRESS_ANY_KEY_TO_CONTINUE);      
  set_color(37,40);
  clear_screen_area(THROW_Y,THROW_X,8,34);
  attron(A_BOLD);
  wmvaddstr(stdscr,THROW_Y+1,THROW_X+12,"金 额 统 计");
  for(i=1;i<=4;i++)
  {
    wmove(stdscr,THROW_Y+2+i,THROW_X);
    wprintw(stdscr,"%s家：%7ld %c %7ld = %7ld",sit_name[i],
            player[table[i]].money, (change_money[i]<0) ? '-' : '+',
            (change_money[i]<0) ? -change_money[i] : change_money[i],
            player[table[i]].money+change_money[i]);
    player[table[i]].money+=change_money[i];
  }
  my_money=player[my_id].money;
  if(in_serv)
    waiting=1;
  return_cursor();
  attroff(A_BOLD);
  if(sit!=info.dealer)
  {
    info.dealer++;
    info.cont_dealer=0;
    if(info.dealer==5)
    {
      info.dealer=1;
      info.wind++;
      if(info.wind==5)
        info.wind=1;
    }
  }
  else
    info.cont_dealer++;
  wait_a_key(PRESS_ANY_KEY_TO_CONTINUE);
  if(in_serv)
  {
    wait_hit[my_sit]=1;
  }
  else
    write_msg(table_sockfd,"450");
}

process_epk(check)
char check;
{
  char card1,card2,card3;
  int i;
  char msg_buf[80];

  switch(check)
  {
    case 2:
      play_mode=THROW_CARD;
      for(i=0;i<pool[my_sit].num;i++)
      {
        if(pool[my_sit].card[i]==current_card)
        {
          pool[my_sit].card[i]=pool[my_sit].card[pool[my_sit].num-1];
          pool[my_sit].card[i+1]=pool[my_sit].card[pool[my_sit].num-2];
          break;
        }
      }
      card1=card2=card3=current_card;
      draw_epk(my_id,check,card1,card2,card3);
      pool[my_sit].num-=3;
      sort_card(1);
      break;
    case 3:
    case 11:
      for(i=0;i<pool[my_sit].num;i++)
      {
        if(pool[my_sit].card[i]==current_card)
        {
          pool[my_sit].card[i]=pool[my_sit].card[pool[my_sit].num-1];
          pool[my_sit].card[i+1]=pool[my_sit].card[pool[my_sit].num-2];
          pool[my_sit].card[i+2]=pool[my_sit].card[pool[my_sit].num-3];
        }
      }
      card1=current_card;
      card2=current_card;
      card3=current_card;
      draw_epk(my_id,check,card1,card2,card3);
      pool[my_sit].num-=3;
      sort_card(0);
      play_mode=GET_CARD;
      attron(A_REVERSE);
      show_card(10,INDEX_X+16*2+1,INDEX_Y+1,1);
      attroff(A_REVERSE);
      break;
    case 12:
      for(i=0;i<pool[my_sit].out_card_index;i++)
      {
        if(pool[my_sit].out_card[i][1]==current_card ||
           pool[my_sit].out_card[i][2]==current_card)
          break;
      }
      pool[my_sit].out_card[i][0]=12;
      if(i==pool[my_sit].out_card_index)
        break;
      card1=card2=card3=current_card;
      draw_epk(my_id,check,card1,card2,card3);
      play_mode=GET_CARD;
      attron(A_REVERSE);
      show_card(10,INDEX_X+16*2+1,INDEX_Y+1,1);
      attroff(A_REVERSE);
      break;
    case 7:
      play_mode=THROW_CARD;
      pool[my_sit].card[search_card(my_sit,current_card+1)]
           =pool[my_sit].card[pool[my_sit].num-1];
      pool[my_sit].card[search_card(my_sit,current_card+2)]
           =pool[my_sit].card[pool[my_sit].num-2];
      card1=current_card+1;
      card2=current_card;
      card3=current_card+2;
      draw_epk(my_id,check,card1,card2,card3);
      pool[my_sit].num-=3;
      sort_card(1);
      break;
    case 8:
      play_mode=THROW_CARD;
      pool[my_sit].card[search_card(my_sit,current_card-1)]
          =pool[my_sit].card[pool[my_sit].num-1];
      pool[my_sit].card[search_card(my_sit,current_card+1)]
          =pool[my_sit].card[pool[my_sit].num-2];
      card1=current_card-1;
      card2=current_card;
      card3=current_card+1;
      draw_epk(my_id,check,card1,card2,card3);
      pool[my_sit].num-=3;
      sort_card(1);
      break;
    case 9:
      play_mode=THROW_CARD;
      pool[my_sit].card[search_card(my_sit,current_card-1)]
          =pool[my_sit].card[pool[my_sit].num-1];
      pool[my_sit].card[search_card(my_sit,current_card-2)]
          =pool[my_sit].card[pool[my_sit].num-2];
      card1=current_card-2;
      card2=current_card;
      card3=current_card-1;
      draw_epk(my_id,check,card1,card2,card3);
      pool[my_sit].num-=3;
      sort_card(1);
      break;
  }
  if(check!=12)
  {
    pool[my_sit].out_card[pool[my_sit].out_card_index][0]=check;
    pool[my_sit].out_card[pool[my_sit].out_card_index][1]=card1;
    pool[my_sit].out_card[pool[my_sit].out_card_index][2]=card2;
    pool[my_sit].out_card[pool[my_sit].out_card_index][3]=card3;
    if(check==3 || check==11)
    {
      pool[my_sit].out_card[pool[my_sit].out_card_index][4]=card3;
      pool[my_sit].out_card[pool[my_sit].out_card_index][5]=0;
    }
    else
      pool[my_sit].out_card[pool[my_sit].out_card_index][4]=0;
    pool[my_sit].out_card_index++;
  }
  sprintf(msg_buf,"530%c%c%c%c%c",my_id,check,card1,card2,card3);
  turn=my_sit;
  card_owner=my_sit;
  if(in_serv)
  {
    gettimeofday(&before,(struct timezone *) 0);
    broadcast_msg(1,msg_buf);
  }
  else
    write_msg(table_sockfd,msg_buf);
  current_item=pool[my_sit].num;
  pos_x=INDEX_X+16*2+1;
  draw_index(pool[my_sit].num+1);
  display_point(my_sit);
  show_num(2,70,144-card_point-16,2);
  return_cursor();
}

/*  Draw eat,pong or kang */
draw_epk(id,kind,card1,card2,card3)
char id,kind,card1,card2,card3;
{
  int sit,i;
  char msg_buf[80];

  beep1();
  sit=player[id].sit;
  if(kind==KANG || kind==11 || kind==12)
    in_kang=1;
  if(kind!=11 && kind!=12)
    throw_card(20);
  if(kind==12)
  {
    for(i=0;i<pool[sit].out_card_index;i++)
    {
      if(pool[sit].out_card[i][1]==card1 && 
         pool[sit].out_card[i][2]==card2)
        break;
    }
    attron(A_REVERSE);
    switch((sit-my_sit+4)%4)
    {
      case 0:
        wmvaddstr(stdscr,INDEX_Y,INDEX_X+i*6,"杠");
        break;
      case 1:
        wmvaddstr(stdscr,INDEX_Y1-i*3-1,INDEX_X1-2,"杠");
        break;
      case 2:
        wmvaddstr(stdscr,INDEX_Y2+2,INDEX_X2-i*6-2,"杠");
        break;
      case 3:
        wmvaddstr(stdscr,INDEX_Y3+i*3+1,INDEX_X3+4,"杠");
        break;
    }
    attroff(A_REVERSE);
    return_cursor();
  }
  else
  switch((sit-my_sit+4)%4)
  {
    case 0:
      show_card(20,INDEX_X+(16-pool[my_sit].num)*2+4,INDEX_Y+1,1);
      wrefresh(stdscr);
      wmvaddstr(stdscr,INDEX_Y,INDEX_X+(16-pool[my_sit].num)*2-2,"┌");
      if(kind==KANG || kind==11)
      {
        attron(A_REVERSE);
        wmvaddstr(stdscr,INDEX_Y,INDEX_X+(16-pool[my_sit].num)*2,"杠");
        attroff(A_REVERSE);
      }
      else
        wmvaddstr(stdscr,INDEX_Y,INDEX_X+(16-pool[my_sit].num)*2,"─");
      wmvaddstr(stdscr,INDEX_Y,INDEX_X+(16-pool[my_sit].num)*2+2,"┐  ");
      wrefresh(stdscr);
      if(kind==11)
        card1=card3=30;
      show_card(card1,INDEX_X+(16-pool[my_sit].num)*2-2,INDEX_Y+1,1);
      show_card(card2,INDEX_X+(16-pool[my_sit].num)*2,INDEX_Y+1,1);
      show_card(card3,INDEX_X+(16-pool[my_sit].num)*2+2,INDEX_Y+1,1);
      break;
    case 1:
      wmvaddstr(stdscr,INDEX_Y1-(16-pool[sit].num)-2,INDEX_X1-2,"┌");
      if(kind==KANG || kind==11)
      {
        attron(A_REVERSE);
        wmvaddstr(stdscr,INDEX_Y1-(16-pool[sit].num)-1,INDEX_X1-2,"杠");
        attroff(A_REVERSE);
      }
      else
        wmvaddstr(stdscr,INDEX_Y1-(16-pool[sit].num)-1,INDEX_X1-2,"│");
      wmvaddstr(stdscr, INDEX_Y1-(16-pool[sit].num), INDEX_X1-2,"└");
      wrefresh(stdscr);
      if(kind==11)
        card1=card2=card3=40;
      show_card(card1,INDEX_X1,INDEX_Y1-(16-pool[sit].num),0);
      show_card(card2,INDEX_X1,INDEX_Y1-(16-pool[sit].num)-1,0);
      show_card(card3,INDEX_X1,INDEX_Y1-(16-pool[sit].num)-2,0);
      break;
    case 2:
      wmvaddstr(stdscr,INDEX_Y2+2,INDEX_X2-(16-pool[sit].num)*2-2,"└");
      if(kind==KANG || kind==11)
      {
        attron(A_REVERSE);
        wmvaddstr(stdscr,INDEX_Y2+2,INDEX_X2-(16-pool[sit].num)*2,"杠");
        attroff(A_REVERSE);
      }
      else
        wmvaddstr(stdscr,INDEX_Y2+2,INDEX_X2-(16-pool[sit].num)*2,"─");
      if(kind==11)
        card1=card2=card3=30;
      wmvaddstr(stdscr,INDEX_Y2+2,INDEX_X2-(16-pool[sit].num)*2+2,"┘");
      wrefresh(stdscr);
      show_card(20,INDEX_X2-(16-pool[sit].num)*2-4,INDEX_Y2,1);
      show_card(card1,INDEX_X2-(16-pool[sit].num)*2+2,INDEX_Y2,1);
      show_card(card2,INDEX_X2-(16-pool[sit].num)*2,INDEX_Y2,1);
      show_card(card3,INDEX_X2-(16-pool[sit].num)*2-2,INDEX_Y2,1);
      break;
    case 3:
      wmvaddstr(stdscr, INDEX_Y3+(16-pool[sit].num), INDEX_X3+4,"┐");
      if(kind==KANG || kind==11)
      {
        attron(A_REVERSE);
        wmvaddstr(stdscr,INDEX_Y3+(16-pool[sit].num)+1,INDEX_X3+4,"杠");
        attroff(A_REVERSE);
      }
      else
        wmvaddstr(stdscr,INDEX_Y3+(16-pool[sit].num)+1,INDEX_X3+4,"│");
      if(kind==11)
        card1=card2=card3=40;
      wmvaddstr(stdscr,INDEX_Y3+(16-pool[sit].num)+2,INDEX_X3+4,"┘");
      wrefresh(stdscr);
      show_card(card1,INDEX_X3,INDEX_Y3+(16-pool[sit].num),0);
      show_card(card2,INDEX_X3,INDEX_Y3+(16-pool[sit].num)+1,0);
      show_card(card3,INDEX_X3,INDEX_Y3+(16-pool[sit].num)+2,0);
      break;
  }
}

draw_flower(sit,card)
char sit;
char card;
{
  char msg_buf[80];

/*
  set_mode(1);
  set_color(33,40);
*/
  attron(A_BOLD);
  in_kang=1;
  pool[sit].flower[card-51]=1;
  strcpy(msg_buf,mj_item[card]);
  msg_buf[2]=0;
  reset_cursor();
  switch((sit-my_sit+4)%4)
  {
    case 0:
      wmvaddstr(stdscr,FLOWER_Y,FLOWER_X+(card-51)*2,msg_buf);
      break;
    case 1:
      wmvaddstr(stdscr,FLOWER_Y1-(card-51),FLOWER_X1,msg_buf);
      break;
    case 2:
      wmvaddstr(stdscr,FLOWER_Y2,FLOWER_X2-(card-51)*2,msg_buf);
      break;
    case 3:
      wmvaddstr(stdscr,FLOWER_Y3+(card-51),FLOWER_X3,msg_buf);
      break;
  }
  return_cursor();
  attroff(A_BOLD);
/*
  set_mode(0);
  set_color(37,40);
*/
}

