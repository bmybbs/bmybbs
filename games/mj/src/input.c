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
#include "misc.h"

process_key()
{
  int i,j,key;
  static m,n,current_eat;
  static eat_pool[5];
  char msg_buf[255];
  char card,card1;

  while(Check_for_data(0))
  {
    key=my_getch();
      /* ---------  PLAY_MODE  ------------ */
      if(input_mode==PLAY_MODE)
      {   
        if(key>='a' && key<='a'+pool[my_sit].num)
        {
          if(play_mode==THROW_CARD)
          {
            current_item=key-'a';
            return_cursor();
            goto quick_throw;
          }
        }
        switch(key)
        {
          case 0:
            break;
          case TAB:
            input_mode=TALK_MODE;
            current_mode=PLAY_MODE;
            return_cursor();
            break;
          case '3':
            break;
          case ',':
          case KEY_LEFT:
            if(play_mode!=THROW_CARD)
              break;
            current_item--;
            if(current_item==-1)
              current_item=pool[my_sit].num;
            pos_x=INDEX_X+(16-pool[my_sit].num+current_item)*2;
            if(current_item==pool[my_sit].num)
              pos_x++;
            return_cursor();
            break;
          case '.':
          case KEY_RIGHT:
            if(play_mode!=THROW_CARD)
              break;
            current_item++;
            if(current_item==pool[my_sit].num+1)
              current_item=0;
            pos_x=INDEX_X+(16-pool[my_sit].num+current_item)*2;
            if(current_item==pool[my_sit].num)
              pos_x++;
            return_cursor();
            break;
          case '`':
            if(!cheat_mode)
              break;
            if(play_mode==THROW_CARD)
            {
              if(in_join)
              {
              }
              else if(in_serv)
              {
                card_index--;
                if(card_index<card_point)
                  card_index=143;
                card=mj[card_index];
                change_card(current_item,card);
                mj[card_index]=pool[my_sit].card[current_item];
                return_cursor();
              }
            }
            break;
          case CTRL_L:
            redraw_screen();
            break;
          case KEY_ENTER:
          case ENTER:
          case SPACE:
            if(play_mode==GET_CARD)
            {
              play_mode=WAIT_CARD;
              if(in_join)
              {
                write_msg(table_sockfd,"313");
                break;
              }
              else
              {
                card=mj[card_point++];
                current_card=card;
                show_num(2,70,144-card_point-16,2);
                /* change turn */
                card_owner=my_sit;
                sprintf(msg_buf,"305%c",(char) my_sit);
                broadcast_msg(1,msg_buf);
                /* show new cardback */
                sprintf(msg_buf,"314%c%c",my_sit,2);
                broadcast_msg(1,msg_buf);
                /* change card number */
                sprintf(msg_buf,"306%c",card_point);
                broadcast_msg(1,msg_buf);
                /* get the card */
                process_new_card(my_sit,card);
                clear_check_flag(my_sit);
                check_flag[my_sit][3]=check_kang(my_sit,card);
                check_flag[my_sit][4]=check_make(my_sit,card,0);
                in_check[1]=0;
                for(i=1;i<check_number;i++)
                  if(check_flag[my_sit][i])
                  {
                    getting_card=1;
                    init_check_mode();
                    in_check[1]=1;
                    check_on=1;
                  }
                gettimeofday(&before, (struct timezone *) 0);
              }
                break;
            }
            else if(play_mode==THROW_CARD)
            {
            quick_throw:;
            play_mode=WAIT_CARD;
            in_kang=0;
            pool[my_sit].first_round=0;
            if(in_join)
            {
              sprintf(msg_buf,"401%c",pool[my_sit].card[current_item]);
              write_msg(table_sockfd,msg_buf);
              current_id=my_id;
              current_card=pool[my_sit].card[current_item];
            }
            else if(in_serv)   /* need not to check card for itself */
            {
              pool[my_sit].time+=thinktime();
              display_time(my_sit);
              sprintf(msg_buf,"312%c%f",my_sit,pool[my_sit].time);
              broadcast_msg(1,msg_buf);
              sprintf(msg_buf,"314%c%c",my_sit,3);
              broadcast_msg(1,msg_buf);
              sprintf(msg_buf,"402%c%c",1,pool[my_sit].card[current_item]);
              broadcast_msg(1,msg_buf);
              current_card=pool[my_sit].card[current_item];
              for(i=1;i<=4;i++)
              {
                if(table[i]>1)   /* clients */
                {
                  check_card(i,current_card);
                }
              }
              for(i=1;i<=4;i++)
              {
                if(table[i]>1)
                for(j=1;j<check_number;j++)
                {
                  if(check_flag[i][j])
                  {
                    sprintf(msg_buf,"501%c%c%c%c",check_flag[i][1]+'0',
                      check_flag[i][2]+'0',check_flag[i][3]+'0',
                      check_flag[i][4]+'0');
                    write_msg(player[table[i]].sockfd,msg_buf);
                    in_check[i]=1;
                    break;
                  }
                  else
                    in_check[i]=0;
                }
              }
              in_check[1]=0;
              check_on=1;
              current_id=1;
              send_card_on=0;
              next_player_request=1;
              next_player_on=0;
            }
            throw_card(pool[my_sit].card[current_item]);
            show_cardmsg(my_sit,pool[my_sit].card[current_item]);
            pool[my_sit].card[current_item]=pool[my_sit].card[pool[my_sit].num];
            current_item=pool[my_sit].num;
            pos_x=INDEX_X+16*2+1;
            play_mode=WAIT_CARD;
            show_card(20,pos_x,INDEX_Y+1,1);
            sort_card(0);
            wrefresh(stdscr);
            return_cursor();
            break;
            }
            else
              break;
          default:
            break;
        }
      }
      else if(input_mode==CHECK_MODE)
      {
        if(key>='0' && key<='0'+check_number-1)
        {
          current_check=key-'0';
          check_x=org_check_x+current_check*3;
          return_cursor();
          goto quick_choose;
        }
        switch(key)
        {
          case 0:
            break;
          case TAB:
            input_mode=TALK_MODE;
            current_mode=CHECK_MODE;
            return_cursor();
            break; 
          case CTRL_L:
            redraw_screen();
            break;
          case ',':
          case KEY_LEFT:
            current_check--;
            if(current_check==-1)
              current_check=check_number-1;
            check_x=org_check_x+current_check*3;
            return_cursor();
            break;
          case '.':
          case KEY_RIGHT:
            current_check++;
            if(current_check==check_number)
              current_check=0;
            check_x=org_check_x+current_check*3;
            return_cursor();
            break;
          case KEY_ENTER:
          case ENTER:
          case SPACE:
            quick_choose:;
            if(current_check && !check_flag[my_sit][current_check])
              break;
            for(i=0;i<check_number;i++)
            {
              wmvaddstr(stdscr,org_check_y+1,org_check_x+i*3,"  ");
              wrefresh(stdscr);
              wmvaddstr(stdscr,org_check_y+1,org_check_x+i*3,check_name[i]);
              wrefresh(stdscr);
            } 
            show_cardmsg(0,0);
            if(current_check==EAT)
            {
              m=0;
              eat_pool[m]=0;
              if(current_card%10>=3)
                if(search_card(my_sit,current_card-2)>=0 &&
                   search_card(my_sit,current_card-1)>=0)
                {
                  eat_pool[m]=current_card-2;
                  m++;
                }
              if(current_card%10>=2 && current_card%10<=8)
                if(search_card(my_sit,current_card-1)>=0 &&
                   search_card(my_sit,current_card+1)>=0 )
                {
                  eat_pool[m]=current_card-1;
                  m++;
                }
              if(current_card%10<=7)
                if(search_card(my_sit,current_card+1)>=0 &&
                   search_card(my_sit,current_card+2)>=0)
                {
                  eat_pool[m]=current_card;
                  m++;
                }
              eat_pool[m]=0;
              if(m>1)
              {  
                input_mode=EAT_MODE;
                /* Display different eating choices */
                wmvaddstr(stdscr,15,58,"©°©¤©¤©¤©¤©¤©¤©¤©´");
                wmvaddstr(stdscr,16,58,"©¦              ©¦");
                wmvaddstr(stdscr,17,58,"©¸©¤©¤©¤©¤©¤©¤©¤©¼");
                for(i=0;i<m;i++)
                {
                  if(eat_pool[i])
                  {
                    sprintf(msg_buf,"%d%d%d",eat_pool[i]%10,eat_pool[i]%10+1,
                            eat_pool[i]%10+2);
                    wmvaddstr(stdscr,org_eat_y,org_eat_x+i*4,msg_buf);
                  }
                }
                current_eat=0;
                eat_x=org_eat_x;
                eat_y=org_eat_y;
                return_cursor();
              }
              else
              {
                i=current_card-eat_pool[m-1]+7; 
                write_check(i);
                input_mode=PLAY_MODE;
                return_cursor();
              }
            }
            else
            {
              write_check(current_check);
              input_mode=PLAY_MODE;
              return_cursor();
            }
            break;
          default:
            break;
        }
      }
      /* --------- CHOOSE_EAT  ------------ */
      else if(input_mode==EAT_MODE)
      {
        for(i=0;i<m;i++)
        {
          if(key=='0'+eat_pool[i]%10)
          {
            current_eat=i;
            goto quick_eat;
          }
        }
        switch(key)
        {
          case TAB:
            input_mode=TALK_MODE;
            current_mode=EAT_MODE;
            return_cursor();
            break;
          case ',':
          case KEY_LEFT:
            if(current_eat<=0)
              break;
            current_eat--;
            eat_x-=4;
            return_cursor();
            break;
          case '.':
          case KEY_RIGHT:
            if(current_eat>=m-1)
              break;
            current_eat++;
            eat_x+=4;
            return_cursor();
            break;
          case CTRL_L:
            redraw_screen();
            break;
          case KEY_ENTER:
          case ENTER:
          case SPACE:
            quick_eat:;
            input_mode=PLAY_MODE;
            i=current_card-eat_pool[current_eat]+7;
            write_check(i);
            show_cardmsg(0,0);
            break;
        }
      }
      /* ---------  TALK_MODE  ------------ */
      else if(input_mode==TALK_MODE)
        switch(key)
        {
        case 0:
          break;
        case TAB:
          if(screen_mode==PLAYING_SCREEN_MODE)
          {
            input_mode=current_mode;
            return_cursor();
          }
          break;
        case KEY_LEFT:
          if(talk_x==0)
            break;
          talk_x--;
          return_cursor();
          break;
        case KEY_RIGHT:
          if(talk_x==talk_buf_count)
            break;
          talk_x++;
          return_cursor();
          break;
        case CTRL_P:
        case KEY_UP:
          if(h_point==(h_head+1)%HISTORY_SIZE || h_point==h_head)
            break;
          else
            h_point=(h_point-1+HISTORY_SIZE)%HISTORY_SIZE;
          werase(inputwin);
          mvwaddstr(inputwin,0,0,history[h_point]);
          wrefresh(inputwin);
          strcpy(talk_buf,history[h_point]);
          talk_buf[talk_right-talk_left-1]=0;
          talk_buf_count=strlen(talk_buf);
          talk_x=talk_buf_count;
          break;
        case CTRL_N:
        case KEY_DOWN:
          if(h_point==h_tail)
            break;
          else
            h_point=(h_point+1)%HISTORY_SIZE;
          werase(inputwin);
          mvwaddstr(inputwin,0,0,history[h_point]);
          wrefresh(inputwin);
          strcpy(talk_buf,history[h_point]);
          talk_buf[talk_right-talk_left-1]=0;
          talk_buf_count=strlen(talk_buf);
          talk_x=talk_buf_count;
          break;
        case CTRL_A:
          talk_x=0;
          wmove(inputwin,talk_y,talk_x); 
          wrefresh(inputwin);
          break;
        case CTRL_E:
          talk_x=strlen(talk_buf);
          wmove(inputwin,talk_y,talk_x);
          wrefresh(inputwin);
          break;
        case CTRL_D:
          talk_x++;
        case CTRL_H:
        case BACKSPACE:
          if(talk_x==0)
            break;
          talk_x--;
          for(i=talk_x;i<talk_buf_count;i++)
            talk_buf[i]=talk_buf[i+1];
          talk_buf[talk_buf_count--]='\0';
          strcpy(history[h_tail],talk_buf);
          mvprintstr(inputwin,talk_y,talk_x,talk_buf+talk_x);
          printch(inputwin,' ');
          return_cursor();
          break;
        case KEY_ENTER:
        case ENTER:
          switch(input_mode)
          {
            case TALK_MODE:
              if(talk_buf_count==0)
                break;
              strcpy(history[h_tail],talk_buf);
              command_parser(talk_buf);
              h_tail=(h_tail+1)%HISTORY_SIZE;
              history[h_tail][0]=0;
              if(h_tail==h_head)
                h_head=(h_head+1)%HISTORY_SIZE;
              h_point=h_tail;
              clear_input_line();
              talk_x=0;
              return_cursor();
              if(input_mode==PLAY_MODE)
              {
                return_cursor();
              }
              break;
            case PLAY_MODE:
              break;
            default:
              err("Unknow input mode");
              break;
          }
          break;
        case CTRL_U:
          clear_input_line(); 
          break;
        case CTRL_J:
          break;
        case CTRL_K:
          break;
        case CTRL_L:
          redraw_screen();
          break;
        case CTRL_G:
          break;
        default:
          if(talk_x==talk_right-talk_left)
            break;
          /* Cursor is in the right most side of characters */
          if(talk_buf_count==talk_x)
          {
            talk_buf[talk_buf_count]=key;
            talk_buf_count++;
            talk_buf[talk_buf_count]='\0';
            mvprintch(inputwin,talk_y,talk_x,key);
            talk_x++;
            wrefresh(inputwin);
          }
          else
          {
            if(talk_buf_count<talk_right-talk_left)
              talk_buf[++talk_buf_count]='\0';
            else
              talk_buf[talk_buf_count-1]='\0';
            for(i=talk_buf_count;i>talk_x;i--)
              talk_buf[i]=talk_buf[i-1];
            talk_buf[talk_x]=key;
            mvprintstr(inputwin,talk_y,talk_x,talk_buf+talk_x);
            talk_x++;
            return_cursor();
          }
          strcpy(history[h_tail],talk_buf);
          break;
      }
/*
    }
*/
  }
}

my_getch()
{
	int i;
	static int l=0;
AGAIN:
	i=getch();
	if((i==10&&l==ENTER)||(i==ENTER&&l==10)) {
		l=0;
		goto AGAIN;
	}
	if(i==10||i==ENTER) {
		l=i;
		return ENTER;
	}
	l=0;
	if(i==-1) leave();
	return i;
}
