/*
 * Server 
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/param.h>

#include "mjgps.h"


char record_file[MAXPATHLEN];

struct player_record record;

FILE *fp;

int
read_user_name (char *name)
{
    struct player_record tmp_rec;

    if ((fp = fopen (record_file, "r+b")) == NULL)
    {
	printf ("Cannot open file %s\n", record_file);
	exit (1);
    }

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
read_user_id (unsigned int id)
{
    if ((fp = fopen (record_file, "r+b")) == NULL)
    {
	printf ("Cannot open file %s\n", record_file);
	exit (1);
    }

    fseek (fp, sizeof (record) * id, 0);
    fread (&record, sizeof (record), 1, fp);
    fclose (fp);
}

void
write_record ()
{
    if ((fp = fopen (record_file, "r+b")) == NULL)
    {
	printf ("Cannot open file %s\n", record_file);
	exit (1);
    }

    fseek (fp, sizeof (record) * record.id, 0);
    fwrite (&record, sizeof (record), 1, fp);
    fclose (fp);
}

void
print_record ()
{
    char *ctime ();
    struct player_record tmprec;
    char time1[40], time2[40];
    int player_num;
    int i;
    int id;
    char name[40];
    long money;

    printf ("(1) 以 id 查看特定使用者\n");
    printf ("(2) 以名称查看特定使用者\n");
    printf ("(3) 查看所有使用者\n");
    printf ("(4) 查看此金额以上的使用者\n");
    printf ("(5) 查看此金额以下的使用者\n");
    printf ("\n请输入你的选择:");
    scanf ("%d", &i);

    switch (i)
    {
    case 1:
	printf ("请输入你要查看的 id:");
	scanf ("%d", &id);

	if (id < 0)
	    return;

	break;

    case 2:
	printf ("请输入你要查看的名称:");
	gets (name);
	gets (name);
	break;

    case 3:
	break;

    case 4:
    case 5:
	printf ("请输入金额:");
	scanf ("%D", &money);
	break;

    default:
	return;
    }

    player_num = 0;
    if ((fp = fopen (record_file, "rb")) == NULL)
    {
	printf ("Cannot open file %s\n", record_file);
	exit (1);
    }

    while (!feof (fp) && fread (&tmprec, sizeof (tmprec), 1, fp))
    {
	if (i == 1)
	{
	    if (id != tmprec.id)
		continue;
	}
	if (i == 2)
	{
	    if (strcmp (name, tmprec.name) != 0 || name[0] == 0)
		continue;
	}
	if (i == 4)
	{
	    if (tmprec.money <= money || tmprec.name[0] == 0)
		continue;
	}
	if (i == 5)
	{
	    if (tmprec.money >= money || tmprec.name[0] == 0)
		continue;
	}

	printf ("%d %10s %15s %ld %d %d %d  %s\n", tmprec.id, tmprec.name,
	    tmprec.password, tmprec.money, tmprec.level, tmprec.login_count,
		tmprec.game_count, tmprec.last_login_from);

	strcpy (time1, ctime (&tmprec.regist_time));
	strcpy (time2, ctime (&tmprec.last_login_time));

	time1[strlen (time1) - 1] = 0;
	time2[strlen (time2) - 1] = 0;
	printf ("              %s    %s\n", time1, time2);

	if (tmprec.name[0] != 0)
	    player_num++;
    }
    printf ("--------------------------------------------------------------\n");
    if (i == 3)
	printf ("共 %d 人注册\n", player_num);
    fclose (fp);
}

void
modify_user ()
{
    int i, id;
    char name[40];
    long money;

    printf ("请输入使用者代号:");
    scanf ("%d", &id);

    if (id < 0)
	return;

    read_user_id (id);
    printf ("\n");
    printf ("(1) 更改名称\n");
    printf ("(2) 重设密码\n");
    printf ("(3) 更改金额\n");
    printf ("(4) 取消更改\n");
    printf ("\n请输入你的选择:");
    scanf ("%d", &i);
    printf ("\n");

    switch (i)
    {
    case 1:
	printf ("请输入要更改的名称:");
	gets (name);
	gets (name);
	strcpy (record.name, name);
	printf ("改名为 %s\n", name);
	break;

    case 2:
	record.password[0] = 0;
	printf ("密码已重设!\n");
	break;

    case 3:
	printf ("请输入要更改的金额:");
	scanf ("%D", &money);
	record.money = money;
	printf ("金额更改为 %ld\n", money);
	break;

    default:
	return;
    }
    write_record ();
}

int
main (int argc, char **argv)
{
    int i, id;

    strcpy (record_file, argc < 2 ? DEFAULT_RECORD_FILE : argv[1]);

    while (1)
    {
	printf ("\n");
	printf ("(1) 列出所有使用者资料\n");
	printf ("(2) 删除使用者\n");
	printf ("(3) 更改使用者资料\n");
	printf ("(4) 离开\n\n");
	printf ("请输入你的选择:");
	scanf ("%d", &i);

	switch (i)
	{
	case 1:
	    print_record ();
	    break;

	case 2:
	    printf ("请输入使用者代号:");
	    scanf ("%d", &id);

	    if (id >= 0)
	    {
		read_user_id (id);
		record.name[0] = 0;
		write_record ();
	    }
	    break;

	case 3:
	    modify_user ();
	    break;

	default:
	    return 0;
	}
    }

    return 0;
}
