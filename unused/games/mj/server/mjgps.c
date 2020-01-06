/*
 * Server 
 */
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <string.h>
#include <netdb.h>
#include <sys/errno.h>
#include <termio.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/file.h>
#include <pwd.h>
#include <ctype.h>
#include <unistd.h>
#include "mjgps.h"


/*
 * Global variables 
 */
int timeup = 0;
extern int errno;
fd_set rfds, afds;
int login_limit;
char gps_ip[20];
int gps_port;
int log_level;
char number_map[20][5]
=
{"０", "１", "２", "３", "４", "５", "６", "７", "８", "９"};

int gps_sockfd;

char climark[30];

struct player_info player[MAX_PLAYER];

struct player_record record;

struct record_index_type record_index;

FILE *fp, *log_fp;

struct ask_mode_info ask;

struct rlimit fd_limit;


int
err (char *errmsg)
{
/*
    if ((log_fp = fopen (LOG_FILE, "a")) == NULL)
    {
	printf ("Cannot open logfile\n");
	return -1;
    }
    printf ("%s", errmsg);

    if (log_level == 0)
	fprintf (log_fp, "%s", errmsg);

    log_level = 0;
    fclose (log_fp);
*/  
}

int
read_msg (int fd, char *msg)
{
    int n;
    char msg_buf[255];
    int read_code;

    n = 0;
    if (Check_for_data (fd) == 0)
    {
	err ("WRONG READ\n");
	return 2;
    }
    timeup = 0;
    alarm (3);
    do
    {
      recheck:;
	read_code = read (fd, msg, 1);
	if (read_code == -1)
	{
	    if (errno != EWOULDBLOCK)
	    {
		alarm (0);
		return 0;
	    }
	    else if (timeup)
	    {
		alarm (0);
		err ("TIME UP!\n");
		return 0;
	    }
	    else
		goto recheck;
	}
	else if (read_code == 0)
	{
	    alarm (0);
	    return 0;
	}
	else
	{
	    n++;
	}
	if (n > 79)
	{
	    alarm (0);
	    return 0;
	}
    }
    while (*msg++ != '\0');
    alarm (0);
    return 1;
/*
 * n=read(fd,msg_buf,80);
 * printf("-%d- ",n);
 * if(n<=0)
 * return 0;
 * else
 * {
 * printf("(%d)",strlen(msg_buf));
 * msg_buf[n]=0;
 * printf("%d %d %s\n",n,strlen(msg_buf),msg_buf);
 * strcpy(msg,msg_buf);
 * return 1;
 * }
 */
}

void
write_msg (int fd, char *msg)
{
    int n;

    n = strlen (msg);
    if (write (fd, msg, n) < 0)
    {
	close (fd);
	FD_CLR (fd, &afds);
    }
    if (write (fd, msg + n, 1) < 0)
    {
	close (fd);
	FD_CLR (fd, &afds);
    }
}

void
display_msg (int player_id, char *msg)
{
    char msg_buf[255];

    sprintf (msg_buf, "101%s", msg);
    write_msg (player[player_id].sockfd, msg_buf);
}

int
Check_for_data (int fd)
/*
 * Checks the socket descriptor fd to see if any incoming data has
 * arrived.  If yes, then returns 1.  If no, then returns 0.
 * If an error, returns -1 and stores the error message in socket_error.
 */
{
    int status;			/* return code from Select call. */
    fd_set wait_set;	/* A set representing the connections that
				 * have been established. 
				 */
    struct timeval tm;		/* A timelimit of zero for polling for new
				 * connections. 
				 */

    FD_ZERO (&wait_set);
    FD_SET (fd, &wait_set);

    tm.tv_sec = 0;
    tm.tv_usec = 0;
    status = select (FD_SETSIZE, &wait_set, (fd_set *) 0, (fd_set *) 0, &tm);

/*
 * if (status < 0)
 * sprintf (socket_error, "Error in select: %s", sys_errlist[errno]); 
 */

    return (status);
}

int
convert_msg_id (int player_id, char *msg)
{
    int i;
    char msg_buf[255];

    if (strlen (msg) < 3)
    {
	sprintf (msg_buf, "Error msg: %s", msg);
	err (msg_buf);
	return 0;
    }
    for (i = 0; i < 3; i++)
	if (msg[i] < '0' || msg[i] > '9')
	{
	    sprintf (msg_buf, "%d", msg[i]);
	    err (msg_buf);
	}
    return (msg[0] - '0') * 100 + (msg[1] - '0') * 10 + (msg[2] - '0');
}

void
list_player (int fd)
{
    int i;
    char msg_buf[255];
    int total_num = 0;

    write_msg (fd, "101-------------    目前上线使用者    ---------------");
    strcpy (msg_buf, "101");
    for (i = 1; i < MAX_PLAYER; i++)
    {
	if (player[i].login == 2)
	{
	    total_num++;
	    if ((strlen (msg_buf) + strlen (player[i].name)) > 50)
	    {
		write_msg (fd, msg_buf);
		strcpy (msg_buf, "101");
	    }
	    strcat (msg_buf, player[i].name);
	    strcat (msg_buf, "  ");
	}
    }
    if (strlen (msg_buf) > 4)
	write_msg (fd, msg_buf);

    write_msg (fd, "101--------------------------------------------------");
    sprintf (msg_buf, "101共 %d 人", total_num);
    write_msg (fd, msg_buf);
}

void
list_table (int fd, int mode)
{
    int i;
    char msg_buf[255];
    int total_num = 0;

    write_msg (fd, "101   桌长       人数  附注");
    write_msg (fd, "101--------------------------------------------------");
    for (i = 1; i < MAX_PLAYER; i++)
    {
	if (player[i].login && player[i].serv > 0)
	{
	    if (player[i].serv > 4)
	    {
		if (player[i].serv == 5)
		    err ("SERV=5\n");
		else
		{
		    err ("LIST TABLE ERROR!");
		    sprintf (msg_buf, "serv=%d\n", player[i].serv);
		    close_id (i);
		    err (msg_buf);
		}
	    }
	    if (mode == 2 && player[i].serv >= 4)
		continue;
	    total_num++;
	    sprintf (msg_buf, "101   %-10s %-4s  %s"
		     ,player[i].name, number_map[player[i].serv]
		     ,player[i].note);
	    write_msg (fd, msg_buf);
	}
    }
    write_msg (fd, "101--------------------------------------------------");
    sprintf (msg_buf, "101共 %d 桌", total_num);
    write_msg (fd, msg_buf);
}

void
list_stat (int fd, char *name)
{
    char msg_buf[255];
    char msg_buf1[255];
    char order_buf[30];
    int i;
    int total_num;
    int order;
    struct player_record tmp_rec;

    total_num = 0;
    order = 1;
    if (!read_user_name (name))
    {
	write_msg (fd, "101找不到这个人!");
	return;
    }
    sprintf (msg_buf, "101◇名称:%s  %s", record.name, record.last_login_from);
    if ((fp = fopen (RECORD_FILE, "rb")) == NULL)
    {
	sprintf (msg_buf, "(stat) Cannot open file\n");
	err (msg_buf);
	return;
    }
    rewind (fp);
    if (record.game_count >= 16)
	while (!feof (fp) && fread (&tmp_rec, sizeof (tmp_rec), 1, fp))
	{
	    if (tmp_rec.name[0] != 0 && tmp_rec.game_count >= 16)
	    {
		total_num++;
		if (tmp_rec.money > record.money)
		    order++;
	    }
	}
    if (record.game_count < 16)
	strcpy (order_buf, "无");
    else
	sprintf (order_buf, "%d/%d", order, total_num);
    sprintf (msg_buf1, "101◇金额:%ld 排名:%s 上线次数:%d 已玩局数:%d",
	     record.money, order_buf, record.login_count, record.game_count);
    write_msg (fd, msg_buf);
    write_msg (fd, msg_buf1);
    fclose (fp);
}

who (fd, name)
     int fd;
     char *name;
{
    char msg_buf[255];
    int i;
    int serv_id;

    for (i = 1; i < MAX_PLAYER; i++)
	if (player[i].login && player[i].serv)
	    if (strcmp (player[i].name, name) == 0)
	    {
		serv_id = i;
		goto found_serv;
	    }
    write_msg (fd, "101找不到此桌");
    return;
  found_serv:;
    sprintf (msg_buf, "101%s  ", player[serv_id].name);
    write_msg (fd, "101----------------   此桌使用者   ------------------");
    for (i = 1; i < MAX_PLAYER; i++)
	if (player[i].join == serv_id)
	{
	    if (strlen (msg_buf) + strlen (player[i].name) > 53)
	    {
		write_msg (fd, msg_buf);
		strcpy (msg_buf, "101");
	    }
	    strcat (msg_buf, player[i].name);
	    strcat (msg_buf, "  ");
	}
    if (strlen (msg_buf) > 4)
	write_msg (fd, msg_buf);
    write_msg (fd, "101--------------------------------------------------");
}

lurker (fd)
     int fd;
{
    int i, total_num = 0;
    char msg_buf[255];

    strcpy (msg_buf, "101");
    write_msg (fd, "101-------------   目前□置之使用者   ---------------");
    for (i = 1; i < MAX_PLAYER; i++)
	if (player[i].login == 2 && (player[i].join == 0 && player[i].serv == 0))
	{
	    total_num++;
	    if ((strlen (msg_buf) + strlen (player[i].name)) > 53)
	    {
		write_msg (fd, msg_buf);
		strcpy (msg_buf, "101");
	    }
	    strcat (msg_buf, player[i].name);
	    strcat (msg_buf, "  ");
	}
    if (strlen (msg_buf) > 4)
	write_msg (fd, msg_buf);
    write_msg (fd, "101--------------------------------------------------");
    sprintf (msg_buf, "101共 %d 人", total_num);
    write_msg (fd, msg_buf);
}

find_user (fd, name)
     int fd;
     char *name;
{
    int i;
    char msg_buf[255];
    int id;
    char *ctime ();
    char last_login_time[80];

    id = find_user_name (name);
    if (id > 0)
    {
	if (player[id].login == 2)
	{
	    if (player[id].join == 0 && player[id].serv == 0)
	    {
		sprintf (msg_buf, "101◇%s □置中", name);
		write_msg (fd, msg_buf);
	    }
	    if (player[id].join)
	    {
		sprintf (msg_buf, "101◇%s 在 %s 桌内", name, player[player[id].join].name);
		write_msg (fd, msg_buf);
	    }
	    if (player[id].serv)
	    {
		sprintf (msg_buf, "101◇%s 在 %s 桌内", name, player[id].name);
		write_msg (fd, msg_buf);
	    }
	    return;
	}
    }
    if (!read_user_name (name))
    {
	sprintf (msg_buf, "101◇没有 %s 这个人", name);
	write_msg (fd, msg_buf);
    }
    else
    {
	sprintf (msg_buf, "101◇%s 不在线上", name);
	write_msg (fd, msg_buf);
	strcpy (last_login_time, ctime (&record.last_login_time));
	last_login_time[strlen (last_login_time) - 1] = 0;
	sprintf (msg_buf, "101◇上次连线时间: %s", last_login_time);
	write_msg (fd, msg_buf);
    }
}

broadcast (player_id, msg)
     int player_id;
     char *msg;
{
    int i;
    char msg_buf[255];

    if (strcmp (player[player_id].name, "candle") != 0)
	return;
    for (i = 1; i < MAX_PLAYER; i++)
	if (player[i].login == 2)
	{
	    sprintf (msg_buf, "101%s", msg);
	    write_msg (player[i].sockfd, msg_buf);
	}
}

send_msg (player_id, msg)
     int player_id;
     char *msg;
{
    char *str1, *str2;
    int i;
    char msg_buf[255];

    str1 = strtok (msg, " ");
    str2 = msg + strlen (str1) + 1;
    for (i = 1; i < MAX_PLAYER; i++)
	if (player[i].login == 2 && strcmp (player[i].name, str1) == 0)
	{
	    sprintf (msg_buf, "101*%s* %s", player[player_id].name, str2);
	    write_msg (player[i].sockfd, msg_buf);
	    return;
	}
    write_msg (player[player_id].sockfd, "101找不到这个人");
}

invite (player_id, name)
     int player_id;
     char *name;
{
    int i;
    char msg_buf[255];

    for (i = 1; i < MAX_PLAYER; i++)
	if (player[i].login == 2 && strcmp (player[i].name, name) == 0)
	{
	    sprintf (msg_buf, "101%s 邀请你加入 %s", player[player_id].name,
		     (player[player_id].join == 0) ?
	      player[player_id].name : player[player[player_id].join].name);
	    write_msg (player[i].sockfd, msg_buf);
	    return;
	}
    write_msg (player[player_id].sockfd, "101找不到这个人");
}

init_socket ()
{
    struct sockaddr_in serv_addr;
    int on = 1;

    /*
     * open a TCP socket for internet stream socket 
     */
    if ((gps_sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	err ("Server: cannot open stream socket");

    /*
     * bind our local address 
     */
    bzero ((char *) &serv_addr, sizeof (serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    serv_addr.sin_port = htons (gps_port);
    setsockopt (gps_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof (on));
    if (bind (gps_sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0)
    {
	printf ("server: cannot bind local address\n");
	exit (1);
    }
    listen (gps_sockfd, 10);
    printf ("Listen for client...\n");
}

char *
lookup (struct sockaddr_in *cli_addrp)
{
    struct hostent *hp;
    char *hostname;

    hp = gethostbyaddr ((char *) &cli_addrp->sin_addr,
			sizeof (struct in_addr), cli_addrp->sin_family);

    if (hp)
	hostname = (char *) hp->h_name;
    else
	hostname = inet_ntoa (cli_addrp->sin_addr);
    return hostname;
}

init_variable ()
{
    int i;

    login_limit = 100;
    for (i = 0; i < MAX_PLAYER; i++)
    {
	player[i].login = 0;
	player[i].serv = 0;
	player[i].join = 0;
	player[i].type = 16;
	player[i].note[0] = 0;
	player[i].username[0] = 0;
    }
}

int
read_user_name (name)
     char *name;
{
    struct player_record tmp_rec;
    char msg_buf[255];

    if ((fp = fopen (RECORD_FILE, "a+b")) == NULL)
    {
	sprintf (msg_buf, "(read_user_name) Cannot open file!\n");
	err (msg_buf);
	return;
    }
    rewind (fp);
    while (!feof (fp) && fread (&tmp_rec, sizeof (tmp_rec), 1, fp))
    {
	if (strcmp (name, tmp_rec.name) == 0)
	{
	    record = tmp_rec;
	    fclose (fp);
	    return 1;
	}
    }
    fclose (fp);
    return 0;
}

int
read_user_id (id)
     unsigned int id;
{
    char msg_buf[255];

    if ((fp = fopen (RECORD_FILE, "a+b")) == NULL)
    {
	sprintf (msg_buf, "(read_user_id) Cannot open file!\n");
	err (msg_buf);
	return;
    }
    rewind (fp);
    fseek (fp, sizeof (record) * id, 0);
    fread (&record, sizeof (record), 1, fp);
    fclose (fp);
}

int
add_user (int player_id, char *name, char *passwd)
{
    long time ();
    struct stat status;

    stat (RECORD_FILE, &status);
    if (!read_user_name (""))
	record.id = status.st_size / sizeof (record);
    strcpy (record.name, name);
    strcpy (record.password, genpasswd (passwd));
    record.money = DEFAULT_MONEY;
    record.level = 0;
    record.login_count = 1;
    record.game_count = 0;
    time (&record.regist_time);
    record.last_login_time = record.regist_time;
    record.last_login_from[0] = 0;
    if (player[player_id].username[0] != 0)
    {
	sprintf (record.last_login_from, "%s@", player[player_id].username);
    }
    strcat (record.last_login_from, 
    		lookup (&(player[player_id].addr)));
    if (check_user (player_id))
    {
	write_record ();
	return 1;
    }
    else
	return 0;
}

int
check_user (int player_id)
{
    char msg_buf[255];
    char from[80];
    char email[80];
    FILE *baduser_fp;

    if ((baduser_fp = fopen (BADUSER_FILE, "r")) == NULL)
    {
	sprintf (msg_buf, "Cannot open file %s", BADUSER_FILE);
	err (msg_buf);
	return 1;
    }
    strcpy (from, lookup (&(player[player_id].addr)));
    sprintf (email, "%s@", player[player_id].username);
    strcat (email, from);
    
    while (fgets (msg_buf, 80, baduser_fp) != NULL)
    {
	msg_buf[strlen (msg_buf) - 1] = 0;
	if (strcmp (email, msg_buf) == 0 ||
	    strcmp (player[player_id].username, msg_buf) == 0)
	{
	    display_msg (player_id,
		  "你已被限制进入, 有问题请 mail 给 sywu@csie.nctu.edu.tw");
	    fclose (baduser_fp);
	    return 0;
	}
    }
    fclose (baduser_fp);
    return 1;
}

void
write_record ()
{
    char msg_buf[255];

    if ((fp = fopen (RECORD_FILE, "r+b")) == NULL)
    {
	sprintf (msg_buf, "(write_record) Cannot open file!");
	err (msg_buf);
	return;
    }
    fseek (fp, sizeof (record) * record.id, 0);
    fwrite (&record, sizeof (record), 1, fp);
    fclose (fp);
}

void
print_news (int fd, char *name)
{
    FILE *news_fp;
    char msg[255];
    char msg_buf[255];

    if ((news_fp = fopen (name, "r")) == NULL)
    {
	sprintf (msg_buf, "Cannot open file %s\n", NEWS_FILE);
	err (msg_buf);
	return;
    }
    while (fgets (msg, 80, news_fp) != NULL)
    {
	msg[strlen (msg) - 1] = 0;
	strcpy (msg_buf, "101");
	strcat (msg_buf, msg);
	write_msg (fd, msg_buf);
    }
    fclose (news_fp);
}

void
welcome_user (int player_id)
{
    char msg_buf[255];
    int fd;
    int total_num = 0;
    int online_num = 0;
    int i;
    struct player_record tmp_rec;

    fd = player[player_id].sockfd;
    if (strcmp (player[player_id].version, "093") < 0 ||
	player[player_id].version[0] == 0)
    {
	write_msg (player[player_id].sockfd,
		   "101请使用 QKMJ Ver 0.93 Beta 以上版本上线");
	write_msg (player[player_id].sockfd, "010");
	return;
    }
    sprintf (msg_buf, "101★★★★★　欢迎 %s 来到ＱＫ麻将  ★★★★★",
	     player[player_id].name);
    write_msg (player[player_id].sockfd, msg_buf);
    print_news (player[player_id].sockfd, NEWS_FILE);
    if (record.money < 15000 && record.game_count >= 16)
    {
	record.money = 15000;
	write_msg (fd, "101运气不太好是吗? 将你的金额提升为 15000, 好好加油!");
	write_record ();
    }
    player[player_id].id = record.id;
    player[player_id].money = record.money;
    player[player_id].login = 2;
    player[player_id].note[0] = 0;
    if ((fp = fopen (RECORD_FILE, "rb")) == NULL)
    {
	sprintf (msg_buf, "(welcome) cannot open file\n");
	err (msg_buf);
    }
    else
    {
	rewind (fp);
	while (!feof (fp) && fread (&tmp_rec, sizeof (tmp_rec), 1, fp))
	{
	    if (tmp_rec.name[0] != 0)
		total_num++;
	}
	fclose (fp);
    }
    for (i = 1; i < MAX_PLAYER; i++)
    {
	if (player[i].login == 2)
	    online_num++;
    }
    sprintf (msg_buf, "101◇目前上线人数: %d 人       注册人数: %d 人", online_num,
	     total_num);
    write_msg (player[player_id].sockfd, msg_buf);
    list_stat (player[player_id].sockfd, player[player_id].name);
    write_msg (player[player_id].sockfd, "003");
    sprintf (msg_buf, "120%5d%ld", player[player_id].id, player[player_id].money);
    write_msg (player[player_id].sockfd, msg_buf);
    player[player_id].input_mode = CMD_MODE;
}

int
find_user_name (char *name)
{
    int i;

    for (i = 1; i < MAX_PLAYER; i++)
    {
	if (strcmp (player[i].name, name) == 0)
	    return i;
    }
    return -1;
}

void
gps_processing ()
{
    int alen;
    int fd, nfds;
    int player_id;
    int player_num = 0;
    int i, j;
    int msg_id;
    int read_code;
    char tmp_buf[80];
    char msg_buf[255];
    unsigned char buf[256];
    struct timeval timeout;
    struct hostent *hp;
    long time ();
    int id;
    struct timeval tm;
    long current_time;
    struct tm *tim;

    log_level = 0;
    nfds = getdtablesize ();
    nfds = 256;
    printf ("%d\n", nfds);
    FD_ZERO (&afds);
    FD_SET (gps_sockfd, &afds);
    bcopy ((char *) &afds, (char *) &rfds, sizeof (rfds));
    tm.tv_sec = 0;
    tm.tv_usec = 0;
    /*
     * Waiting for connections 
     */
    for (;;)
    {
	bcopy ((char *) &afds, (char *) &rfds, sizeof (rfds));
	if (select (nfds, &rfds, (fd_set *) 0, (fd_set *) 0, 0) < 0)
	{
	    sprintf (msg_buf, "select: %d %s\n", errno, sys_errlist[errno]);
	    err (msg_buf);
	    continue;
	}
	if (FD_ISSET (gps_sockfd, &rfds))
	{
	    for (player_num = 1; player_num < MAX_PLAYER; player_num++)
		if (!player[player_num].login)
		    break;
	    if (player_num == MAX_PLAYER - 1)
		err ("Too many users");
	    player_id = player_num;
	    alen = sizeof (player[player_num].addr);
	    player[player_id].sockfd = accept (gps_sockfd, (struct sockaddr *)
					   &player[player_num].addr, &alen);
	    FD_SET (player[player_id].sockfd, &afds);
	    fcntl (player[player_id].sockfd, F_SETFL, FNDELAY);
	    player[player_id].login = 1;
	    strcpy (climark, lookup (&(player[player_id].addr)));
	    sprintf (msg_buf, "Connectted with %s\n", climark);
	    err (msg_buf);

	    time (&current_time);
	    tim = localtime (&current_time);
/*
 * if(tim->tm_hour>=2 && tim->tm_hour<6)
 * {
 * for(i=1;i<MAX_PLAYER;i++)
 * {
 * if(player[i].login)
 * {
 * print_news(player[i].sockfd,"opentime.lst");
 * close_id(i);
 * }
 * }
 * }
 */
	    if (player_id > login_limit)
	    {
		if (strcmp (climark, "ccsun34") != 0)
		{
		    write_msg (player[player_id].sockfd, "101对不起,目前使用人数超过上限, 请稍後再进来.");
		    print_news (player[player_id].sockfd, "server.lst");
		    close_id (player_id);
		}
	    }
	}
	for (player_id = 1; player_id < MAX_PLAYER; player_id++)
	{
	    if (player[player_id].login)
		if (FD_ISSET (player[player_id].sockfd, &rfds))
		{
		    /*
		     * Processing the player's information 
		     */
		    read_code = read_msg (player[player_id].sockfd, buf);
		    if (!read_code)
		    {
			close_id (player_id);
		    }
		    else if (read_code == 1)
		    {
			msg_id = convert_msg_id (player_id, buf);
			switch (msg_id)
			{
			case 99:	/*
					 * get username 
					 */
			    buf[15] = 0;
			    strcpy (player[player_id].username, buf + 3);
			    break;
			case 100:	/*
					 * check version 
					 */
			    *(buf + 6) = 0;
			    strcpy (player[player_id].version, buf + 3);
			    break;
			case 101:	/*
					 * user login 
					 */
			    buf[13] = 0;
			    strcpy (player[player_id].name, buf + 3);
			    for (i = 0; i < strlen (buf) - 3; i++)
			    {
				if (buf[3 + i] <= 32 && buf[3 + i] != 0)
				{
				    write_msg (player[player_id].sockfd, "101Invalid username!");
				    close_id (player_id);
				    break;
				}
			    }
			    for (i = 1; i < MAX_PLAYER; i++)
			    {
				if (player[i].login == 2 && strcmp (player[i].name, buf + 3) == 0)
				{
				    write_msg (player[player_id].sockfd, "006");
				    goto multi_login;
				}
			    }
			    if (read_user_name (player[player_id].name))
			    {
				write_msg (player[player_id].sockfd, "002");
			    }
			    else
			    {
				write_msg (player[player_id].sockfd, "005");
			    }
			  multi_login:;
			    break;
			case 102:	/*
					 * Check password 
					 */
			    if (read_user_name (player[player_id].name))
			    {
				*(buf + 11) = 0;
				if (checkpasswd (record.password, buf + 3))
				{
				    for (i = 1; i < MAX_PLAYER; i++)
				    {
					if (player[i].login == 2 &&
					    strcmp (player[i].name, player[player_id].name) == 0)
					{
					    close_id (i);
					    break;
					}
				    }
				    time (&record.last_login_time);
				    record.last_login_from[0] = 0;
				    if (player[player_id].username[0] != 0)
				    {
					sprintf (record.last_login_from, "%s@",
						 player[player_id].username);
				    }
				    strcat (record.last_login_from,
					    lookup (&player[player_id].addr));
				    record.login_count++;
				    write_record ();
				    if (check_user (player_id))
					welcome_user (player_id);
				    else
					close_id (player_id);
				}
				else
				{
				    write_msg (player[player_id].sockfd, "004");
				}
			    }
			    break;
			case 103:	/*
					 * Create new account 
					 */
			    *(buf + 11) = 0;
			    if (!add_user (player_id, player[player_id].name, buf + 3))
			    {
				close_id (player_id);
				break;
			    }
			    welcome_user (player_id);
			    break;
			case 104:	/*
					 * Change password 
					 */
			    *(buf + 11) = 0;
			    read_user_name (player[player_id].name);
			    strcpy (record.password, genpasswd (buf + 3));
			    write_record ();
			    break;
			case 2:
			    list_player (player[player_id].sockfd);
			    break;
			case 3:
			    list_table (player[player_id].sockfd, 1);
			    break;
			case 4:
			    strcpy (player[player_id].note, buf + 3);
			    break;
			case 5:
			    list_stat (player[player_id].sockfd, buf + 3);
			    break;
			case 6:
			    who (player[player_id].sockfd, buf + 3);
			    break;
			case 7:
			    broadcast (player_id, buf + 3);
			    break;
			case 8:
			    invite (player_id, buf + 3);
			    break;
			case 9:
			    send_msg (player_id, buf + 3);
			    break;
			case 10:
			    lurker (player[player_id].sockfd);
			    break;
			case 11:
			    /*
			     * Check for table server  
			     */
			    for (i = 1; i < MAX_PLAYER; i++)
			    {
				if (player[i].login == 2 && player[i].serv)
				{
				    /*
				     * Find the name of table server 
				     */
				    if (strcmp (player[i].name, buf + 3) == 0)
				    {
					if (player[i].serv >= 4)
					{
					    write_msg (player[player_id].sockfd, "101此桌人数已满!");
					    goto full;
					}
					sprintf (msg_buf, "120%5d%ld", player[player_id].id,
						 player[player_id].money);
					write_msg (player[i].sockfd, msg_buf);
					sprintf (msg_buf, "211%s", player[player_id].name);
					write_msg (player[i].sockfd, msg_buf);
					sprintf (msg_buf, "0110%s %d",
						 inet_ntoa (player[i].addr.sin_addr), player[i].port);
					write_msg (player[player_id].sockfd, msg_buf);
					player[player_id].join = i;
					player[player_id].serv = 0;
					player[i].serv++;
					break;
				    }
				}
			    }
			    if (i == MAX_PLAYER)
				write_msg (player[player_id].sockfd, "0111");
			  full:;
			    break;
			case 12:
			    player[player_id].port = atoi (buf + 3);
			    if (player[player_id].join)
			    {
				if (player[player[player_id].join].serv > 0)
				    player[player[player_id].join].serv--;
				player[player_id].join = 0;
			    }
			    /*
			     * clear all client 
			     */
			    for (i = 1; i < MAX_PLAYER; i++)
			    {
				if (player[i].join == player_id)
				    player[i].join = 0;
			    }
			    player[player_id].serv = 1;
			    break;
			case 13:
			    list_table (player[player_id].sockfd, 2);
			    break;
			case 20:
			    strcpy (msg_buf, buf + 3);
			    *(msg_buf + 5) = 0;
			    id = atoi (msg_buf);
			    read_user_id (id);
			    record.money = atol (buf + 8);
			    record.game_count++;
			    write_record ();
			    for (i = 1; i < MAX_PLAYER; i++)
			    {
				if (player[i].login == 2 && player[i].id == id)
				{
				    player[i].money = record.money;
				    break;
				}
			    }
			    break;
			case 21:	/*
					 * FIND 
					 */
			    find_user (player[player_id].sockfd, buf + 3);
			    break;
			case 111:
/*
 * player[player_id].serv++;
 */
			    break;
			case 200:
			    close_id (player_id);
			    break;
			case 202:
			    if (strcmp (player[player_id].name, "candle") != 0)
				break;
			    id = find_user_name (buf + 3);
			    if (id >= 0)
			    {
				write_msg (player[id].sockfd, "200");
				close_id (id);
			    }
			    break;
			case 205:
			    if (player[player_id].serv)
			    {
				/*
				 * clear all client 
				 */
				for (i = 1; i < MAX_PLAYER; i++)
				{
				    if (player[i].join == player_id)
					player[i].join = 0;
				}
				player[player_id].serv = 0;
				player[player_id].join = 0;
			    }
			    else if (player[player_id].join)
			    {
				if (player[player[player_id].join].serv > 0)
				    player[player[player_id].join].serv--;
				player[player_id].join = 0;
			    }
			    break;
			case 500:
			    if (strcmp (player[player_id].name, "candle") == 0)
				shutdown_server ();
			    break;
			default:
			    sprintf (msg_buf, "### cmd=%d player_id=%d sockfd=%d ###\n", msg_id, player_id, player[player_id].sockfd);
			    err (msg_buf);
			    close_connection (player_id);
			    sprintf (msg_buf, "Connection to %s error, closed it\n",
				     lookup (&(player[player_id].addr)));
			    err (msg_buf);
			    break;
			}
			buf[0] = '\0';
		    }
		}
	}
    }
}

void
close_id (int player_id)
{
    char msg_buf[255];

    close_connection (player_id);
    sprintf (msg_buf, "Connection to %s closed\n",
	     lookup (&(player[player_id].addr)));
    err (msg_buf);
}

void
close_connection (int player_id)
{
    close (player[player_id].sockfd);
    FD_CLR (player[player_id].sockfd, &afds);
    if (player[player_id].join && player[player[player_id].join].serv)
	player[player[player_id].join].serv--;

    player[player_id].login = 0;
    player[player_id].serv = 0;
    player[player_id].join = 0;
    player[player_id].version[0] = 0;
    player[player_id].note[0] = 0;
    player[player_id].name[0] = 0;
    player[player_id].username[0] = 0;
}

void
shutdown_server ()
{
    int i;
    char msg_buf[255];

    for (i = 1; i < MAX_PLAYER; i++)
    {
	if (player[i].login)
	    shutdown (player[i].sockfd, 2);
    }
    sprintf (msg_buf, "QKMJ Server shutdown\n");
    err (msg_buf);
    exit (0);
}

void
core_dump ()
{
    err ("CORE DUMP!\n");
    exit (0);
}

void
bus_err ()
{
    err ("BUS ERROR!\n");
    exit (0);
}

void
broken_pipe ()
{
    err ("Broken PIPE!!\n");
}

void
time_out ()
{
    timeup = 1;
}

char *
genpasswd (char *pw)
{
    char saltc[2];
    long salt;
    int i, c;
    static char pwbuf[14];
    long time ();
    char *crypt ();

    if (strlen (pw) == 0)
	return "";
    time (&salt);
    salt = 9 * getpid ();
#ifndef lint
    saltc[0] = salt & 077;
    saltc[1] = (salt >> 6) & 077;
#endif
    for (i = 0; i < 2; i++)
    {
	c = saltc[i] + '.';
	if (c > '9')
	    c += 7;
	if (c > 'Z')
	    c += 6;
	saltc[i] = c;
    }
    strcpy (pwbuf, pw);
    return crypt (pwbuf, saltc);
}

int
checkpasswd (char *passwd, char *test)
{
    static char pwbuf[14];
    char *pw;

    strncpy (pwbuf, test, 14);
    pw = crypt (pwbuf, passwd);
    return (!strcmp (pw, passwd));
}

void
main (int argc, char **argv)
{
    int i;

    /*
     * Set fd to be the maximum number 
     */
    getrlimit (RLIMIT_NOFILE, &fd_limit);
    fd_limit.rlim_cur = fd_limit.rlim_max;
    setrlimit (RLIMIT_NOFILE, &fd_limit);
    i = getdtablesize ();
    printf ("FD_SIZE=%d\n", i);
    signal (SIGSEGV, core_dump);
    signal (SIGBUS, bus_err);
    signal (SIGPIPE, broken_pipe);
    signal (SIGALRM, time_out);
    if (argc < 2)
	gps_port = DEFAULT_GPS_PORT;
    else
    {
	gps_port = atoi (argv[1]);
	printf ("Using port %s\n", argv[1]);
    }
    strcpy (gps_ip, DEFAULT_GPS_IP);
    init_socket ();
    init_variable ();
    gps_processing ();
}
