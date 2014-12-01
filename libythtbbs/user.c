#include <stdio.h>
#include <ctype.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ythtbbs.h"

static int isoverride(struct override *o, char *id);

/* mytoupper: ������IDӳ�䵽A-Z��Ŀ¼�� */
char
mytoupper(unsigned char ch)
{
	if (isalpha(ch))
		return toupper(ch);
	else
		return ch % ('Z' - 'A') + 'A';
}

char *
sethomepath(char *buf, const char *userid)
{
	sprintf(buf, MY_BBS_HOME "/home/%c/%s", mytoupper(userid[0]), userid);
	return buf;
}

char *
sethomefile(char *buf, const char *userid, const char *filename)
{
	sprintf(buf, MY_BBS_HOME "/home/%c/%s/%s", mytoupper(userid[0]), userid,
		filename);
	return buf;
}

char *
setmailfile(char *buf, const char *userid, const char *filename)
{
	sprintf(buf, MY_BBS_HOME "/mail/%c/%s/%s", mytoupper(userid[0]), userid,
		filename);
	return buf;
}

/**
get the file path of sent mail box
*/
char *
setsentmailfile(char *buf, const char *userid, const char *filename)
{
	sprintf(buf, MY_BBS_HOME "/sent_mail/%c/%s/%s", mytoupper(userid[0]), userid,
		filename);
	return buf;
}

int
saveuservalue(char *userid, char *key, char *value)
{
	char path[256];
	sethomefile(path, userid, "values");
	return savestrvalue(path, key, value);
}

int
readuservalue(char *userid, char *key, char *value, int size)
{
	char path[256];
	sethomefile(path, userid, "values");
	return readstrvalue(path, key, value, size);
}

char *
charexp(int exp)
{
	int expbase = 0;

	if (exp == -9999)
		return "û�ȼ�";
	if (exp <= 100 + expbase)
		return "������·";
	if (exp <= 450 + expbase)
		return "һ��վ��";
	if (exp <= 850 + expbase)
		return "�м�վ��";
	if (exp <= 1500 + expbase)
		return "�߼�վ��";
	if (exp <= 2500 + expbase)
		return "��վ��";
	if (exp <= 3000 + expbase)
		return "���ϼ�";
	if (exp <= 5000 + expbase)
		return "��վԪ��";
	return "��������";
}

char *
cperf(int perf)
{
	if (perf == -9999)
		return "û�ȼ�";
	if (perf <= 5)
		return "�Ͽ����";
	if (perf <= 12)
		return "Ŭ����";
	if (perf <= 35)
		return "������";
	if (perf <= 50)
		return "�ܺ�";
	if (perf <= 90)
		return "�ŵ���";
	if (perf <= 140)
		return "̫������";
	if (perf <= 200)
		return "��վ֧��";
	if (perf <= 500)
		return "�񡫡�";
	return "�����ˣ�";
}

int
countexp(struct userec *udata)
{
	int exp;

	if (!strcmp(udata->userid, "guest"))
		return -9999;
	exp = udata->numposts /*+post_in_tin( udata->userid ) */  +
	    udata->numlogins / 5 + (time(0) - udata->firstlogin) / 86400 +
	    udata->stay / 3600;
	return exp > 0 ? exp : 0;
}

int
countperf(struct userec *udata)
{
	int perf;
	int reg_days;

	if (!strcmp(udata->userid, "guest"))
		return -9999;
	reg_days = (time(0) - udata->firstlogin) / 86400 + 1;
	perf = ((float) (udata->numposts /*+post_in_tin( udata->userid ) */ ) /
		(float) udata->numlogins +
		(float) udata->numlogins / (float) reg_days) * 10;
	return perf > 0 ? perf : 0;
}

int life_special(char *id)
{
	FILE *fp , *fp2;
	char buf[128];
	fp=fopen("etc/life", "r");
	if(fp==0) return 0;
	while(1) {
		if(fgets(buf, 128, fp)==0) break;
		//fprintf(fp2, "buf=%s ",buf);
		//if(sscanf(buf, "%s", id1)>0) continue;
		buf[strlen(buf)-1] = 0;
		if(!strcmp(buf, id)) return 1;
	}
	fclose(fp);
	return 0;
}
int
countlife(struct userec *urec)
{
	int value, res;

	/* if (urec) has XEMPT permission, don't kick it */
	if ((urec->userlevel & PERM_XEMPT)
	    || strcmp(urec->userid, "guest") == 0)
		return 999;
//	if (life_special(urec->userid)) return 666;
//	life_special(urec->userid);
	value = (time(0) - urec->lastlogin) / 60;	/* min */
	/* new user should register in 30 mins */
	if (strcmp(urec->userid, "new") == 0) {
		return (30 - value) * 60;
	}
	if (urec->numlogins <= 1)
		return (60 * 1440 - value) / 1440;
	if (!(urec->userlevel & PERM_LOGINOK))
		return (60 * 1440 - value) / 1440;
	if (((time(0)-urec->firstlogin)/86400)>365*8)
		return  888;
	if (((time(0)-urec->firstlogin)/86400)>365*5)
		return  666;
	if (((time(0)-urec->firstlogin)/86400)>365*2)
		return  365;
	
	
	//if (urec->stay > 1000000)
      	//	return (365 * 1440 - value) / 1440;
	res=(180 * 1440 - value) / 1440 + urec->numdays;
	if (res>364) res=364;
	return res;
}

int
userlock(char *userid, int locktype)
{
	char path[256];
	int fd;
	sethomefile(path, userid, ".lock");
	fd = open(path, O_RDONLY | O_CREAT, 0660);
	if (fd == -1)
		return -1;
	flock(fd, locktype);
	return fd;
}

int
userunlock(char *userid, int fd)
{
	flock(fd, LOCK_UN);
	close(fd);
	return 0;
}

static int
checkbansitefile(const char *addr, const char *filename)
{
	FILE *fp;
	char temp[STRLEN];
	if ((fp = fopen(filename, "r")) == NULL)
		return 0;
	while (fgets(temp, STRLEN, fp) != NULL) {
		strtok(temp, " \n");
		if ((!strncmp(addr, temp, 16))
		    || (!strncmp(temp, addr, strlen(temp))
			&& temp[strlen(temp) - 1] == '.')
		    || (temp[0] == '.' && strstr(addr, temp) != NULL)) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

int
checkbansite(const char *addr)
{
	return checkbansitefile(addr, MY_BBS_HOME "/.bansite")
	    || checkbansitefile(addr, MY_BBS_HOME "/bbstmpfs/dynamic/bansite");
}

int
userbansite(const char *userid, const char *fromhost)
{
	char path[STRLEN];
	FILE *fp;
	char buf[STRLEN];
	int i, deny;
	char addr[STRLEN], mask[STRLEN], allow[STRLEN];
	char *tmp[3] = { addr, mask, allow };
	unsigned int banaddr, banmask;
	unsigned int from;
	from = inet_addr(fromhost);
	sethomefile(path, userid, "bansite");
	if ((fp = fopen(path, "r")) == NULL)
		return 0;
	while (fgets(buf, STRLEN, fp) != NULL) {
		i = mystrtok(buf, ' ', tmp, 3);
		if (i == 1) {	//���� ip
			banaddr = inet_addr(addr);
			banmask = inet_addr("255.255.255.255");
			deny = 1;
		} else if (i == 2) {
			banaddr = inet_addr(addr);
			banmask = inet_addr(mask);
			deny = 1;
		} else if (i == 3) {	//�� allow ��
			banaddr = inet_addr(addr);
			banmask = inet_addr(mask);
			deny = !strcmp(allow, "allow");
		} else		//���У�
			continue;
		if ((from & banmask) == (banaddr & banmask)) {
			fclose(fp);
			return deny;
		}
	}
	fclose(fp);
	return 0;
}

void
logattempt(char *user, char *from, char *zone, time_t time)
{
	char buf[256], filename[80];
	int fd, len;

	sprintf(buf, "system passerr %s", from);
	newtrace(buf);
	snprintf(buf, 256, "%-12.12s  %-30s %-16s %-6s\n",
		 user, Ctime(time), from, zone);
	len = strlen(buf);
	if ((fd =
	     open(MY_BBS_HOME "/" BADLOGINFILE, O_WRONLY | O_CREAT | O_APPEND,
		  0644)) >= 0) {
		write(fd, buf, len);
		close(fd);
	}
	sethomefile(filename, user, BADLOGINFILE);
	if ((fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644)) >= 0) {
		write(fd, buf, len);
		close(fd);
	}
}

static int
isoverride(struct override *o, char *id)
{
        if (strcasecmp(o->id, id) == 0)
                return 1;
        return 0;
}

int
inoverride(char *who, char *owner, char *file)
{
	char buf[80];
        struct override o;
	sethomefile(buf, owner, file);
        if (search_record(buf, &o, sizeof (o), (void *) isoverride, who) != 0)
                return 1;
        return 0;
}

//ipv6
int is4map6addr(char *s){
	return !strncasecmp(s,"::ffff:",7);
}

char *getv4addr(char *fromhost){
		char *addr;
		addr=rindex(fromhost,':');
		return ++addr;
	}	
