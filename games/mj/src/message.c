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
   
int tt;

int convert_msg_id(msg)
unsigned char *msg;
{
   int i;
   char msg_buf[255];

   for(i=0;i<3;i++)
     if(msg[i]<'0' || msg[i]>'9')
     {
       display_comment("Invalid message id");
       sprintf(msg_buf,"From %d (%d) id=%d len=%d %s", tt,gps_sockfd,msg[i],strlen(msg),msg);
       display_comment(msg_buf);
      }
   return(msg[0]-'0')*100+(msg[1]-'0')*10+(msg[2]-'0');
}

process_msg(player_id,id_buf,msg_type)
int player_id;
unsigned char *id_buf;
int msg_type;
{
  int msg_id;
  unsigned char buf[255];
  char msg_buf[255];
  char ans_buf[255];
  char ans_buf1[255];
  int i,j,sit;
  int alen;

  tt=player_id;
  strcpy(buf,id_buf);
  msg_id=convert_msg_id(id_buf);
/*
  sprintf(msg_buf,"%d from %d %d",msg_id,player_id,msg_type);
  display_comment(msg_buf);  */
  switch(msg_type)
  {
    case(FROM_GPS):
        if(msg_id!=102)
          read_msg(gps_sockfd,buf+3);
        switch(msg_id)
        {
          case 2:
            if(my_pass[0]!=0)
              strcpy(ans_buf,my_pass);
            else
            {
              ans_buf[0]=0;
              ask_question("请输入你的密码：",ans_buf,8,0);
              ans_buf[8]=0;
            }
            sprintf(msg_buf,"102%s",ans_buf);
            write_msg(gps_sockfd,msg_buf);
            strcpy(my_pass,ans_buf);
            break;
          case 3:
            pass_login=1;
            input_mode=TALK_MODE;
            display_comment("请打 /HELP 查看简单指令说明");
            sprintf(msg_buf,"004%s",my_note);
            write_msg(gps_sockfd,msg_buf);
            break;
          case 4:
            pass_count++;
            my_pass[0]=0;
            if(pass_count==3)
              leave();
            do
            {
              ans_buf[0]=0;
              ask_question("密码错误! 请重新输入你的名称：",ans_buf,10,1);
            } while(ans_buf[0]==0);
            ans_buf[10]=0;
            sprintf(msg_buf,"101%s",ans_buf);
            write_msg(gps_sockfd,msg_buf);
            strcpy(my_name,ans_buf);
            break;
          case 5:   /* creat a new account */
            ans_buf[0]=0;
            ask_question("看来你是个新朋友, 你要使用这个名称吗？",ans_buf,1,1);
            if(ans_buf[0]=='y' || ans_buf[0]=='Y')
            {
              ans_buf[0]=0; 
              ask_question("请输入你的密码：",ans_buf,8,0);
              ans_buf1[0]=0;
              ask_question("请再输入一次确认：",ans_buf1,8,0);
              ans_buf[8]=0;
              ans_buf1[8]=0;
              while(1)
              {
                if(strcmp(ans_buf,ans_buf1)==0)
                {
                  sprintf(msg_buf,"103%s",ans_buf);
                  write_msg(gps_sockfd,msg_buf);
                  strcpy(my_pass,ans_buf);
                  break;
                }
                else
                {
                  ans_buf[0]=0;
                  ask_question("两次密码不同! 请重新输入你的密码：",ans_buf,8,0);
                  ans_buf1[0]=0;
                  ask_question("请再输入一次确认：",ans_buf1,8,0);
                  ans_buf[8]=0;
                  ans_buf1[8]=0;
                }
              }
            }
            else
            {
              do 
              {
                ans_buf[0]=0;
                ask_question("请重新输入你的名称：",ans_buf,10,1);
              } while(ans_buf[0]==0);
              ans_buf[10]=0;
              sprintf(msg_buf,"101%s",ans_buf);
              write_msg(gps_sockfd,msg_buf);
              strcpy(my_name,ans_buf);
            }
            break;
          case 6:
            ans_buf[0]=0;
            ask_question("重覆进入! 你要杀掉另一个帐号吗?",ans_buf,1,1);
            if(ans_buf[0]=='y' || ans_buf[0]=='Y')
            {
              ans_buf[0]=0;
              ask_question("请输入你的密码：",ans_buf,8,0);
              ans_buf[8]=0;
              sprintf(msg_buf,"102%s",ans_buf);
              write_msg(gps_sockfd,msg_buf);
              strcpy(my_pass,ans_buf);
            }
            else
              leave();
            break;
          case 10:  /* 离线 */
            leave();
            break;
          case 11:
            switch(buf[3])
            {
              case '0':
                Tokenize(buf+4,1);
                sprintf(msg_buf,"连往 %s port %s",cmd_argv[1],cmd_argv[2]);
                send_gps_line(msg_buf);
                init_socket(cmd_argv[1],atoi(cmd_argv[2]),&table_sockfd);
                FD_SET(table_sockfd,&afds);
                in_join=1;
                break;
              case '1':
                send_gps_line("没有这个名字");
                break;
              case '2':
                send_gps_line("连不上去");
                break;
            }
            break;
          case 101:
            send_gps_line(buf+3);
            break;
          case 102:
            display_news(gps_sockfd);
            break;
          case 120:
            strcpy(msg_buf,buf+3);
            *(msg_buf+5)=0;
            new_client_id=atoi(msg_buf);
            new_client_money=atol(buf+8);
            if(!in_serv)
            {
              my_gps_id=new_client_id;
              my_money=new_client_money;
            }
            break;
          case 200:
            close(gps_sockfd);
            endwin();
            break;
          case 211:
            strcpy(new_client_name,buf+3);
            new_client=1;
            break;
          default:
            sprintf(msg_buf,"msg_id=%d",msg_id);
            display_comment(msg_buf);
        }
        break;
    case(FROM_CLIENT):
      read_msg(player[player_id].sockfd,buf+3);
              switch(msg_id)
              {
                case 101:
                  send_gps_line(buf+3);
                  broadcast_msg(player_id,buf);
                  break;
                case 102:
                  display_comment(buf+3);
                  broadcast_msg(player_id,buf);
                  break;
                case 200:
                  close_client(player_id);
                  break;
                case 290:
                  broadcast_msg(player_id,buf);
                  opening();
                  open_deal();
                  break;
                case 313:
                  /* Send a card to client */
                  send_card_request=1;
                  break;
                case 315:
                  /* Client finished */
                  /* next_player_request=1; */
                  break;
                case 401:
                  /* Others throw a card */
                  pool[player[player_id].sit].time+=thinktime();
                  display_time(player[player_id].sit);
                  sprintf(msg_buf,"312%c%f",player[player_id].sit,
                          pool[player[player_id].sit].time);
                  broadcast_msg(1,msg_buf);
                  pool[player[player_id].sit].first_round=0;
                  in_kang=0;
                  show_newcard(player[player_id].sit,3);
                  sprintf(msg_buf,"314%c%c",player[player_id].sit,3);
                  broadcast_msg(player_id,msg_buf);
                  sprintf(msg_buf,"402%c%c",player_id,buf[3]);
                  broadcast_msg(player_id,msg_buf);
                  current_id=player_id;
                  current_card=buf[3];
                  show_cardmsg(player[player_id].sit,buf[3]);
                  throw_card(buf[3]);
                  return_cursor();
                  sit=player[player_id].sit;
                  for(i=0;i<pool[sit].num;i++)
                    if(pool[sit].card[i]==current_card)
                      break;
                  pool[sit].card[i]=pool[sit].card[pool[sit].num];
                  sort_pool(sit);
                  check_on=1;
                  send_card_on=0;
                  next_player_on=0;
                  /* set in_check flag for players except the current player */
                  for(i=1;i<=4;i++)        
                  {
                    if(table[i]>0 && table[i]!=player_id)
                      check_card(i,buf[3]);
                  }
                  for(i=1;i<=4;i++)
                  {
                  if(table[i]!=1 && i!=turn)
                  for(j=1;j<check_number;j++)
                  {
                    if(check_flag[i][j])
                    {
                      sprintf(msg_buf,"501%c%c%c%c",check_flag[i][1]+'0',
                        check_flag[i][2]+'0',check_flag[i][3]+'0',
                        check_flag[i][4]+'0');
                      write_msg(player[table[i]].sockfd,msg_buf);
                      in_check[i]=1;
                      break;  /* check next player */
                    }
                    else
                      in_check[i]=0;
                  }
                  }
                  for(j=1;j<check_number;j++)
                    if(check_flag[my_sit][j])
                    {
                      init_check_mode();
                      in_check[1]=1;
                      goto in_check_now1;
                    }
                  in_check[1]=0;
                  in_check_now1:;
                  break;
                case 450:
                  wait_hit[player[player_id].sit]=1;
                  break;
                case 501:
                  who(player[player_id].sockfd);
                  break;
                case 510:
                  in_check[player[player_id].sit]=0;
                  check_for[player[player_id].sit]=buf[3]-'0';
                  break;
                case 515:
                  next_player_request=0;
                  turn=player[player_id].sit;
                  sprintf(msg_buf,"310%c",turn);
                  broadcast_msg(player_id,msg_buf);
                  display_point(turn);
                  return_cursor();
                  break;
                case 525:
                  send_card_request=1;
                  draw_flower(buf[3],buf[4]);
                  broadcast_msg(player_id,buf);
                  break;
                case 530:  /* others epk a card */
                  gettimeofday(&before, (struct timezone *) 0);
                  turn=player[buf[3]].sit;
                  display_point(turn);
                  if(buf[4]==12)
                  {
                    for(i=0;i<pool[turn].out_card_index;i++)
                      if(pool[turn].out_card[i][0]==2 &&
                         pool[turn].out_card[i][1]==buf[5])
                      {
                        pool[turn].out_card[i][0]=12;
                        pool[turn].out_card[i][4]=buf[5];
                        pool[turn].out_card[i][5]=0;
                        break;
                      }
                  }
                  else
                  {
                  for(i=0;i<=3;i++)
                    pool[turn].out_card[pool[turn].out_card_index][i]=buf[i+4];
                  if(buf[4]==3 || buf[4]==11)  /* 杠牌 */
                  {
                    pool[turn].out_card[pool[turn].out_card_index][4]=buf[7];
                    pool[turn].out_card[pool[turn].out_card_index][5]=0;

                  }
                  else
                    pool[turn].out_card[pool[turn].out_card_index][4]=0;
                  pool[turn].out_card_index++;
                  }
                  draw_epk(buf[3],buf[4],buf[5],buf[6],buf[7]);
                  broadcast_msg(player_id,buf);
                  return_cursor();
                  switch(buf[4])
                  {
                    case 2:
                      for(i=0;i<pool[turn].num;i++)
                        if(pool[turn].card[i]==buf[6])
                        {
                          pool[turn].card[i]=pool[turn].card[pool[turn].num-1];
                          pool[turn].card[i+1]=pool[turn].card[pool[turn].num-2];
                          break;
                        }
                      pool[turn].num-=3;
                      sort_pool(turn);
                      break;
                    case 3:
                    case 11:
                      for(i=0;i<pool[turn].num;i++)
                        if(pool[turn].card[i]==buf[6])
                        {
                          pool[turn].card[i]=pool[turn].card[pool[turn].num-1];
                          pool[turn].card[i+1]=pool[turn].card[pool[turn].num-2];
                          pool[turn].card[i+2]=pool[turn].card[pool[turn].num-3];
                          break;
                        }
                      pool[turn].num-=3;
                      sort_pool(turn);
                      break;
                    case 7:
                    case 8:
                    case 9:
                      pool[turn].card[search_card(turn,buf[5])]
                          =pool[turn].card[pool[turn].num-1];
                      pool[turn].card[search_card(turn,buf[7])]
                          =pool[turn].card[pool[turn].num-2];
                      pool[turn].num-=3;
                      sort_pool(turn);
                      break;
                  }
                  break; 
                default:
                  break;
              }
              break;
    case (FROM_SERV):
      read_msg(table_sockfd,buf+3);
          switch(msg_id)
          {
            case 101:
              send_gps_line(buf+3);
              break;
            case 102:
              display_comment(buf+3);
              break;
            case 200:
              write_msg(gps_sockfd,"205");
              close(table_sockfd);
              FD_CLR(table_sockfd,&afds);
              in_join=0;
              input_mode=TALK_MODE;
              init_global_screen();
              break;
            case 201:  /* get the new comer's info */
              strcpy(player[buf[3]].name,buf+5);
              player[buf[3]].in_table=1;
              player[buf[3]].sit=buf[4];
              sprintf(msg_buf,"%s 加入此桌",player[buf[3]].name);
              send_gps_line(msg_buf);
              player_in_table++;
              if(player[buf[3]].sit)
                table[player[buf[3]].sit]=buf[3];
              break;
            case 202:
              strcpy(msg_buf,buf+4);
              *(msg_buf+5)=0;
              player[buf[3]].id=atoi(msg_buf);
              player[buf[3]].money=atol(buf+9);
              break;
            case 203:  /* get others info */
              strcpy(player[buf[3]].name,buf+5);
              player[buf[3]].sit=buf[4];
              player[buf[3]].in_table=1;
              player_in_table++;
              table[buf[4]]=buf[3];
              break;
            case 204:
              for(i=1;i<=4;i++)
              {
                table[i]=buf[2+i]-'0';
                if( table[i])
                {
                  player[table[i]].sit=i;
                  on_seat++;
                }
              }
              break;
            case 205:  /* NOTICE: need player_in_table++ ? */
                       /* NOTICE: did he get table[]???? */
              my_id=buf[3];
              strcpy(player[my_id].name,buf+5);
              my_sit=buf[4];
              player[my_id].sit=my_sit;
              player[my_id].in_table=1;
              player[my_id].id=my_gps_id;
              player[my_id].money=my_money;
              table[my_sit]=my_id;
              break;
            case 206:
              if(player_in_table==4)
              {
                init_global_screen();
                input_mode=TALK_MODE;
              }
              sprintf(msg_buf,"%s 离开此桌",player[buf[3]].name);
              display_comment(msg_buf);
              player[buf[3]].in_table=0;
              player_in_table--;
              break;
            case 290:
              opening();
              break;
            case 300:
              if(screen_mode==GLOBAL_SCREEN_MODE)
              {
                init_playing_screen();
              }
              opening();
              break;
            case 301:   /* change card */
              change_card(buf[3],buf[4]);
              break;
            case 302:   /* get 16 cards */
              for(i=0;i<16;i++)
                pool[my_sit].card[i]=buf[i+3];
              sort_card(0);
              break;
            case 303:   /* can get a card */
              play_mode=GET_CARD;
              attron(A_REVERSE);
              show_card(10,INDEX_X+16*2+1,INDEX_Y+1,1);
              attroff(A_REVERSE);
              wrefresh(stdscr);
              beep1();
              return_cursor();
              break;
            case 304:  /* get a card */
              process_new_card(my_sit,buf[3]);
              break;
            case 305:
              card_owner=buf[3];
              if(card_owner!=my_sit)
                show_cardmsg(card_owner,0);
              break;
            case 306:  /* others got a card ---> change the card number */
              card_point=buf[3];
              show_num(2,70,144-card_point-16,2);
              return_cursor();
              break;
            case 308:  /* sort cards */
              sort_card(buf[3]-'0');
              break;
            case 310:  /* Player pointer ---  point to new player */
              turn=buf[3];
              display_point(buf[3]);
              return_cursor();
              break;
            case 312:
              pool[buf[3]].time=atof(buf+4);
              display_time(buf[3]);
              break;
            case 314:  /* process new cardback */
              show_newcard(buf[3],buf[4]);
              return_cursor();
              break;
            case 330:  /* 海底流局 */
              for(i=1;i<=4;i++)
              {
                if(table[i] && i!=my_sit)
                {
                  show_allcard(i);
                  show_kang(i);
                }
              }
              info.cont_dealer++;
              clear_screen_area(THROW_Y,THROW_X,8,34);
              wmvaddstr(stdscr,THROW_Y+3,THROW_X+12,"海 底 流 局");
              return_cursor();
              wait_a_key(PRESS_ANY_KEY_TO_CONTINUE);
              break;
            case 402:  /* others throw a card */
              in_kang=0;
              pool[player[buf[3]].sit].first_round=0;
              show_cardmsg(player[buf[3]].sit,buf[4]);
              current_card=buf[4];
              throw_card(buf[4]); 
              return_cursor();
              current_id=buf[3];
              break;
            case 501:  /* ask for check card */
              go_to_check=0;
              for(i=1;i<=4;i++)
              {
                check_flag[my_sit][i]=buf[2+i]-'0';
                if(check_flag[my_sit][i])
                  go_to_check=1;
              }
              if(go_to_check)
                init_check_mode();
              go_to_check=0;
              break;
            case 518:
              for(i=1;i<=4;i++)
                pool[i].door_wind=buf[2+i];
              wmvaddstr(stdscr,2,64,sit_name[pool[my_sit].door_wind]);
              return_cursor();
              break;
            case 520:
              process_epk(buf[3]);
              break;
            case 521:
              for(i=0;i<pool[1].num;i++)
                pool[1].card[i]=buf[3+i];
              for(i=0;i<pool[2].num;i++)
                pool[2].card[i]=buf[19+i];
              for(i=0;i<pool[3].num;i++)
                pool[3].card[i]=buf[35+i];
              for(i=0;i<pool[4].num;i++)
                pool[4].card[i]=buf[51+i];
              break;
            case 522:
              process_make(buf[3],buf[4]);
              break;
            case 525:
              draw_flower(buf[3],buf[4]);
              break;
            case 530:  /* from server */
              turn=player[buf[3]].sit;
              card_owner=turn;
              display_point(turn);
              if(buf[4]==12)
              {
                for(i=0;i<pool[turn].out_card_index;i++)
                  if(pool[turn].out_card[i][1]==buf[5] && 
                     pool[turn].out_card[i][2]==buf[6])
                  {
                    pool[turn].out_card[i][0]=12;
                    pool[turn].out_card[i][4]=buf[5];
                    pool[turn].out_card[i][5]=0;
                  }
                  draw_epk(buf[3],buf[4],buf[5],buf[6],buf[7]);
              }
              else
              { 
              for(i=0;i<=3;i++)
                pool[turn].out_card[pool[turn].out_card_index][i]=buf[i+4];
              if(buf[4]==3 || buf[4]==11)
              {
                pool[turn].out_card[pool[turn].out_card_index][4]=buf[7];
                pool[turn].out_card[pool[turn].out_card_index][5]=0;
              }
              else
                pool[turn].out_card[pool[turn].out_card_index][4]=0;
              draw_epk(buf[3],buf[4],buf[5],buf[6],buf[7]);
              pool[turn].out_card_index++;
              pool[turn].num-=3;
              }
              return_cursor();
              break;
            default:
              break;
          }
          break;
    default:
      break;
  }
}


