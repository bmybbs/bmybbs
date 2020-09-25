#include "bbs.h"
#include "bbstelnet.h"
#include <sys/mman.h>	// for mmap
#include <math.h>

#define MC_BOARD        "millionaires"
#define DIR_MC          MY_BBS_HOME "/etc/moneyCenter/"
#define DIR_MC_TEMP     MY_BBS_HOME "/bbstmpfs/dynamic/"
#define MC_ADMIN_FILE   DIR_MC "mc_admin"
#define MC_BOSS_FILE    DIR_MC "mc_boss"
#define MC_ASS_FILE     DIR_MC "mc_ass"
#define MC_RATE_FILE    DIR_MC "mc_rate"
#define MC_STOCK_FILE   DIR_MC "mc_stock"
#define MC_PRICE_FILE   DIR_MC "mc_price"
#define MC_DENY_FILE	DIR_MC "mc_deny"
#define MC_MARRYADMIN_FILE DIR_MC "mc_marryadmin"
#define MC_STOCK_BOARDS  DIR_MC "stockboards" //ÉÏÊĞ°æÃæÃûµ¥
#define MC_STOCK_STOPBUY  DIR_MC "st_stopbuy" //ÔİÍ£½»Ò×µÄ°æÃæÃûµ¥
#define DIR_CONTRIBUTIONS  DIR_MC "contributions/" //¸÷°æÃæÄ¼¾èÃûµ¥´æ·ÅÄ¿Â¼
#define MC_JIJIN_CTRL_FILE  DIR_MC "jijin_ctrl" //³öÏÖÔÚ¾è¿îÖĞµÄ»ù½ğid

#define MAX_RECORD_LINE 100	//¼ÇÂ¼ÎÄ¼şĞĞ×î´ó³¤¶È
#define MAX_BET_LENGTH  80	//¶Ä×¢ÊäÈë×î´ó³¤¶È

//»õ±Ò´æ´¢Ãû³Æ
#define MONEY_NAME	"bmy_money"
#define CREDIT_NAME	"bmy_credit"
#define LEND_NAME       "lend_money"
#define INTEREST_NAME   "interest"

//¸÷ÖÖ½ğ¶îÏŞÖÆ
#define PRIZE_PER     3000000
#define MAX_POOL_MONEY 90000000
#define RUMOR_MONEY    500000
#define MAX_MONEY_NUM 500000000
#define MAX_CTRBT_NUM 1000000000
#define BIG_PRIZE             0.7
#define I_PRIZE               0.18
#define II_PRIZE              0.12
#define III_PRIZE             0.00
#define CMFT_PRIZE        20000
#define SALARY_I         10000000
#define SALARY_II        300000
#define SALARY_III       500000

#define MAX_STOCK_NUM 16
#define MAX_STOCK_NUM2 8
/*ĞŞ¸ÄMAX_STOCK_NUM Òª¼ÇµÃÔÙmoney_stock_boardº¯ÊıÖĞ
  ¸østock_name¼ÓÉÏÏàÓ¦µÄ¹ÉÆ±Ãû³Æ*/

//»éÀñÊı¾İ½á¹¹¼°¶¨Òå
/*
½á»é³ÌĞòÊÇÕâÑùµÄ:
ÄĞ·½Ñ¡Ôñ"Çó»é"ÏòÅ®·½Çó»é,ÕâÊ±µÇ¼Ç½á»é¼ÇÂ¼£¬×´Ì¬Îª MAR_COURT£¬Ò»ĞÇÆÚºó×Ô¶¯Ê§°Ü
Å®·½È¥½ÌÌÃ£¬×Ô¶¯ÌáÊ¾ÊÇ·ñ½ÓÊÜÇó»é£¿
    Èç¹û²»½ÓÊÜ£¬½á»é¼ÇÂ¼enableÉèÎª0,Ğû¸æÊ§°Ü
	Èç¹û½ÓÊÜ£¬½á»é¼ÇÂ¼×´Ì¬ÉèÎª MAR_MARRYING ½á»éÖĞ,½á»éÈÕÆÚÄ¬ÈÏÉèÎª1Ììºó
È»ºóÄĞÅ®·½¶¼¿ÉÒÔµ½½ÌÌÃ×¼±¸»éÀñ£¬°üÀ¨ÉèÖÃ½á»éÈÕÆÚ£¬Ğ´Çë¼í£¬·¢Çë¼í£¬²¼ÖÃ±³¾°µÈ
´ËÊ±¿ÉÒÔÔÚ²Î¼Ó»éÀñÊ±µÄ»éÀñµÈ¼¶±íÖĞ¿´µ½ÕâÌõ¼ÇÂ¼£¬Ê±¼äÒ»µ½£¬»éÀñ×Ô¶¯¿ªÊ¼
ÅóÓÑÃÇ¿ÉÒÔÀ´²Î¼ûËûÃÇµÄ»éÀñÁË£¬²Î¼û»éÀñÊ±¿ÉÒÔËÍÀñ½ğ£¬ËÍ»¨£¬ËÍºØ¿¨
»éÀñÔÚËÄĞ¡Ê±ºó×Ô¶¯½áÊø£¬·¢ĞÅµ½½ğÈÚÖĞĞÄ°æÃæ
ÔÚMC_MARRY_RECORDS(100Ìõ¼ÇÂ¼)ÖĞ±£´æÇó»éCOURTºÍÔÚ»éMARRYINGµÄ¼ÇÂ¼
ÔÚË¢ĞÂ¼ÇÂ¼Ê±°ÑÇó»éÊ§°ÜºÍ½á»é³É¹¦µÄ¼ÇÂ¼×ªµ½MC_MARRY_RECORDS_ALL

*/
#define DIR_MC_MARRY			MY_BBS_HOME"/etc/moneyCenter/marry"
#define MC_MARRY_RECORDS        MY_BBS_HOME"/etc/moneyCenter/marryrecords"
#define MC_MARRY_RECORDS_ALL	MY_BBS_HOME"/etc/moneyCenter/marryrecords_all"
#define MC_MARRIED_LIST 	MY_BBS_HOME"/etc/moneyCenter/marriedlist"
//Ä¬ÈÏÑûÇëº¯
#define MC_MAEEY_INVITATION		MY_BBS_HOME"/0Announce/groups/GROUP_0/" MC_BOARD "/system/welcome/invitation"
//Ä¬ÈÏ»éÀñ²¼¾°
#define MC_MAEEY_SET			MY_BBS_HOME"/0Announce/groups/GROUP_0/" MC_BOARD "/system/welcome/frontpage"
#define MAR_COURT		1	//Çó»é
#define MAR_MARRIED	2	//ÒÑ»é
#define MAR_MARRYING	3	//½á»éÖĞ		//»éÀñÔÚmarry_tËÄĞ¡Ê±ºó½áÊø
#define MAR_DIVORCE		4	//Àë»é
#define MAR_COURT_FAIL	5	//Çó»éÊ§°Ü

struct MC_Marry{
	int enable;					//ÊÇ·ñÓĞĞ§
	char bride[14];				//ĞÂÄï
	char bridegroom[14];		//ĞÂÀÉ
	int status;					//»éÒö×´¿öMAR_...
	int giftmoney;				//Àñ½ğ
	int attendmen;				//²Î¼ûÈËÊı
	time_t court_t;				//Çó»éÊ±¼ä
	time_t marry_t;				//½á»éÊ±¼ä
	time_t divorce_t;			//Àë»éÊ±¼ä
	char subject[60];			//Ö÷ÌâÏŞ30ºº×Ö
	int setfile;			//»éÀñ²¼ÖÃµÄÏÔÊ¾ÎÄ¼ş	Ê±¼äÖµ
	int invitationfile;		//Çë¼íÎÄ¼ş	Ê±¼äÖµ
	int visitfile;			//µ½·ÃÈËÔ±´æ´¢ÎÄ¼ş
	int visitcount;			//²Î¼ÓÈËÊı
	char unused[18];
}; // 150 bytes


extern struct UTMPFILE *utmpshm;
extern struct boardmem *bcache;
extern int numboards;
char marry_status[][20] = {"Î´Öª","Çó»é","ÒÑ»é","»éÀñÖĞ","ÒÑÀë»é","Çó»éÊ§°Ü",""};
int multex=0;

void *loadData(char *filepath, void *buffer, size_t filesize);
void saveData(void *buffer, size_t filesize);
static int loadValue(char *user, char *valueName, int sup);
static int saveValue(char *user, char *valueName, int valueToAdd, int sup);
int show_welcome(char *filepath,int startline,int endline);
static int shop_present(int order, char *kind, char *touserid);
static int buy_present(int order, char *kind, char *cardname, char *filepath, int price_per,char *touserid);

static void moneycenter_welcome(void);
static void moneycenter_byebye(void);
int millionairesrec(char *title, char *str, char *owner);
static int limitValue(int value, int sup);
static int money_bank(void);
static int money_lottery(void);
static int money_shop(void);
static int money_check_guard(void);
static int money_dice(void);
static int money_robber(void);
static int money_killer(void);
static int money_stock(void);
static int money_stock_board(void);
//static int money_stock_board2(void);
//static int money_stock_change(void);//slow
static void money_show_stat(char *position);
static void nomoney_show_stat(char *position);
static int money_gamble(void);
static int money_777(void);
static int calc777(int t1, int t2, int t3);
static int guess_number(void);
static int an(char *a, char *b);
static int bn(char *a, char *b);
static void itoa(int i, char *a);
static void time2string(int num, char *str);
static int money_police(void);
static void persenal_stock_info(int stock_num[15], int stock_price[15],
				int money, char stockboard[STRLEN][MAX_STOCK_NUM],
				int stock_board[15]);
/*atic void persenal_stock_info2(int stock_num[15], int stock_price[15],
				int money, char *stockboard[],
				int stock_board[15]);*/
//static int shop_card_show(char *card[][2], int group);
//static int buy_card(char *cardname, int cardnumber);
static int forq(char *a, char *b);
static void p_gp(void);
static void russian_gun();
static void show_card(int isDealer, int c, int x);
static void money_cpu(void);
static int gp_win(void);
static int complex(char *cc, char *x, char *y);
static void money_suoha_tran(char *a, char *b, char *c);
static void money_suoha_check(char *p, char *q, char *r, char *cc);
static void show_style(int my, int cpu);
static int valid367Bet(char *buf);
static int make367Prize(char *bet, char *prizeSeq);
static void make367Seq(char *prizeSeq);
static int open_36_7();
static int validSoccerBet(char *buf);
static int computeSum(char *complexBet);
static void saveSoccerRecord(char *complexBet);
static int makeSoccerPrize(char *bet, char *prizeSeq);
static int open_soccer(char *prizeSeq);
static int makeInterest(int credit, char *valueName, float rate);
static int makeRumor(int num);
static int newSalary();
static int money_admin();
static void policereport(char *str);
static int money_cop();
static int check_allow_in();
static int money_beggar();
static void whoTakeCharge(int pos, char *boss);
static void whoTakeCharge2(int pos, char *boss);
static void sackOrAppoint(int pos, char *boss, int msgType, char *msg);
static void sackOrAppoint2(int pos, char *boss, int msgType, char *msg);
static int Allclubtest(char *id);
static int slowclubtest(char *board,char *id);
static int stop_buy();//slow
//½á»é
static int money_marry();
static int PutMarryRecord(struct MC_Marry *marryMem, int n, struct MC_Marry *new_mm);
static int marry_attend(struct MC_Marry *marryMem, int n);
static int marry_court(struct MC_Marry *marryMem, int n);
static int marry_perpare(struct MC_Marry *marryMem, int n);
static int marry_divorce();
char *get_date_str(time_t *tt);
char *get_simple_date_str(time_t *tt);
static int marry_refresh(struct MC_Marry *marryMem, int n);
static int marry_recordlist(struct MC_Marry *marryMem, int n);
static int marry_all_records();
static int marry_active_records(struct MC_Marry *marryMem, int n);
static int marry_query_records(char *id);
static int marry_admin(struct MC_Marry *marryMem, int n);
//ºÚÃûµ¥
static int money_deny();
static int mc_addtodeny(char *uident, char *msg, int ischange);
static int mc_denynotice(int action, char *user, char *msgbuf);
static int mc_autoundeny(void);
static int addstockboard(char *sbname, char *fname);
static int delstockboard(char *sbname, char *fname);
static int stockboards();
//static int calc_ticket_price();

static int money_office();

static void
showAt(int line, int col, char *str, int flag)
{
	move(line, col);
	prints("%s", str);
	if (flag == 1)
		pressanykey();
	else if (flag == 2)
		pressreturn();
}

int
moneycenter()  //moneycenter½øÈë½çÃæ
{
	int ch;
	int quit = 0;
	modify_user_mode(MONEY);
	strcpy(currboard, MC_BOARD);
	if (!file_exist(DIR_MC"MoneyValues"))
		mkdir(DIR_MC"MoneyValues", 0770);
	if (!file_exist(DIR_CONTRIBUTIONS))
		mkdir(DIR_CONTRIBUTIONS, 0770);
	if (!seek_in_file(MC_ADMIN_FILE, currentuser.userid)
		&& !(currentuser.userlevel & PERM_SYSOP)
		&& strcmp(currentuser.userid, "macintosh"))
		if (utmpshm->mc.isMCclosed){
			clear();
			move(6, 4);
			prints("\033[1;31m±øÂíÙ¸½ğÈÚÖĞĞÄ½ñÌìĞİÏ¢\033[m");
			pressanykey();
			return 0;
		}
	moneycenter_welcome();
	multex = loadValue(currentuser.userid, "multex", 9);
	if (!check_allow_in())
		return 0;
	saveValue(currentuser.userid,"multex",1, 9);

	if (seek_in_file(DIR_MC "jijin", currentuser.userid))
       {
		money_bank();
		moneycenter_byebye();
		return 0;
	}
	clear();
	while (!quit) {
		nomoney_show_stat("±øÂíÙ¸½ğÈÚÖĞĞÄ");
		move(6, 4);
		prints("±øÂíÙ¸½ğÈÚÖĞĞÄ¾­¹ı¸ÄÔì£¬»ÀÈ»Ò»ĞÂ£¬»¶Ó­µ½´¦¿´¿´£¡");
		move(8, 4);
		prints("Ñ§Ï°´ó¸»ÎÌÓÎÏ·¹æÔòÇëÈ¥±¾Õ¾ÏµÍ³Çømillionaires°æ¡£");
		move(t_lines - 2, 0);
		prints("\033[1;44m Ñ¡ \033[1;46m [1]ÒøĞĞ [2]²ÊÆ± [3]¶Ä³¡ [4]ºÚ°ï [5]Ø¤°ï [6]¹ÉÊĞ [7]ÉÌ³¡ [8]¾¯Êğ            \n"
			   "\033[1;44m µ¥ \033[1;46m [9]É±ÊÖ [C]½ÌÌÃ [A]´ó¸»ÎÌ¹ÜÀíÖĞĞÄ [Q]Àë¿ª                                                 ");
		ch = igetkey();
		switch (ch) {
		case '1':
			money_bank();
			break;
		case '2':
			money_lottery();
			break;
		case '3':
			money_gamble();
			break;
		case '4':
			money_robber();
			break;
		case '5':
			money_beggar();
			break;
		case '6':
			money_stock();
			break;
		case '7':
			money_shop();
			break;
		case '8':
			money_cop();
			break;
		case '9':
			money_killer();
			break;
		case '0':
			money_admin();	//Òş²Ø£¬ÇÒÈ¨ÏŞ¼ì²é
			break;
		case 'c':
		case 'C':
			money_marry(); //added by macintosh 20051203
			break;
		case 'a':
		case 'A':
			money_office(); //added by macintosh 20071105
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}
	moneycenter_byebye();
	return 0;
}  // moneycenter½øÈë½çÃæ

static void
moneycenter_welcome()
{
	clear();
	move(4, 4);
	prints("±øÂíÙ¸½ğÈÚÖĞĞÄÃÅ¿ÚÁ¢×ÅÒ»¿é´óÅÆ×Ó£º");
	move(6, 4);
	prints("\033[1;31m´ò»÷Í¶»úµ¹°Ñ\033[m \033[1;31mÎÈ¶¨½ğÈÚÖÈĞò\033[m");
	move(8, 4);
	prints("\033[1;33m½ğÈÚÖĞĞÄ»ÀÈ»Ò»ĞÂ£¬¿ìÀ´Åõ³¡£¡\033[0m");
	pressanykey();
} // »¶Ó­½çÃæ

static void
moneycenter_byebye()
{
	clear();
	saveValue(currentuser.userid, "multex", -9,9);
	move(5, 14);
	prints("\033[1;32m»¶Ó­ÔÙ´Î¹âÁÙ½ğÈÚÖĞĞÄ£¬ÄúµÄ¸»ÓĞÊÇÎÒÃÇµÄÈÙĞÒ¡£\033[m");
	pressanykey();
} //Àë¿ª½çÃæ

//added by macintosh 20051202
int
millionairesrec(char *title, char *str, char *owner)
{
	struct fileheader postfile;
	char filepath[STRLEN], fname[STRLEN];
	char buf[256];
	time_t now;
	FILE *inf, *of;

	now = time(0);
	sprintf(fname, "tmp/deliver.millionairesrec.%d", (int)now);
	if ((inf = fopen(fname, "w")) == NULL)
		return -1;
	fprintf(inf, "%s", str);
	fclose(inf);

	//postfile(fname, owner, "millionairesrec", title);
	memset(&postfile, 0, sizeof (postfile));
	sprintf(filepath, MY_BBS_HOME "/boards/%s/", "millionairesrec");
	now = trycreatefile(filepath, "M.%d.A", now, 100);
	if (now < 0)
		return -1;
	postfile.filetime = now;
	postfile.thread = now;
	fh_setowner(&postfile, owner[0] ? owner : "millionaires", 0);
	ytht_strsncpy(postfile.title, title, sizeof(postfile.title));

	//getcross(filepath, fname, "millionairesrec", title);
	now = time(0);
	inf = fopen(fname, "r");
	if (inf == NULL)
		return -2;
	of = fopen(filepath, "w");
	if (of == NULL) {
		fclose(inf);
		return -3;
	}
	fprintf(of, "·¢ĞÅÈË: %s (±øÂíÙ¸´ó¸»ÎÌÏµÍ³¼ÇÂ¼), ĞÅÇø: millionairesrec\n", owner[0] ? owner : "millionaires");
	fprintf(of, "±ê  Ìâ: %s\n", title);
	fprintf(of, "·¢ĞÅÕ¾: ±øÂíÙ¸BBS (%24.24s), ±¾Õ¾(bbs.xjtu.edu.cn)\n\n", ctime(&now));
	fprintf(of, "¡¾´ËÆªÎÄÕÂÓÉ±øÂíÙ¸´ó¸»ÎÌ×Ô¶¯ÕÅÌùÏµÍ³·¢±í¡¿\n\n");
	while (fgets(buf, 256, inf) != NULL)
		fprintf(of, "%s", buf);
	fprintf(of, "\n\n");
	fprintf(of, "×î½ü·ÃÎÊIP: %s\n\n", currentuser.lasthost);
	fclose(inf);
	fclose(of);

	sprintf(buf, MY_BBS_HOME "/boards/%s/%s", "millionairesrec", DOT_DIR);
	if (append_record(buf, &postfile, sizeof (postfile)) == -1) {
		//errlog("Posting '%s' on '%s': append_record failed!", postfile.title, "millionairesrec");
		return 0;
	}
	updatelastpost("millionairesrec");
	unlink(fname);
	return 1;
}//ÓÃÓÚÏµÍ³¼ÇÂ¼Çø·¢ÎÄ

static int
limitValue(int value, int sup)
{
	if (value > sup)
		return sup;
	if (value < 0)
		return 0;
	return value;
}

static int
savemoneyvalue(char *userid, char *key, char *value)
{
	char path[256];
	sprintf(path, DIR_MC"MoneyValues/%s", userid);
	return savestrvalue(path, key, value);
}

static int
readmoneyvalue(char *userid, char *key, char *value, int size)
{
	char path[256];
	sprintf(path, DIR_MC"MoneyValues/%s", userid);
	return readstrvalue(path, key, value, size);
}

static int
loadValue(char *user, char *valueName, int sup)
{
	char value[20];
	if (readmoneyvalue(user, valueName, value, 20) != 0)
		return 0;
	else
		return limitValue(atoi(value), sup);
}  //¶ÁÈ¡Ïà¹ØÊıÖµ

static int
saveValue(char *user, char *valueName, int valueToAdd, int sup)
{
	int valueInt;
	int retv;
	char value[20];
	valueInt = loadValue(user, valueName, sup);
	valueInt += valueToAdd;
	valueInt = limitValue(valueInt, sup);
	snprintf(value, 20, "%d", valueInt);
	if ((retv = savemoneyvalue(user, valueName, value)) != 0) {
		errlog("save %s %s %s retv=%d err=%s", currentuser.userid,
		       valueName, value, retv, strerror(errno));
	}
	return 0;
}  //±£´æÏà¹ØÊıÖµ

void *loadData(char *filepath, void *buffer, size_t filesize) {
	int fd;

	if ((fd = open(filepath, O_RDWR, 0660)) == -1)
		return (void *)-1;
	buffer = mmap(0, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd , 0);
	close(fd);
	return buffer;
}

void saveData(void *buffer, size_t filesize) {
	if (buffer != NULL)
		munmap(buffer, filesize);
}

static void // ¹ÜÀíÖ°ÎñÏµÍ³
whoTakeCharge(int pos, char *boss)
{
	const char feaStr[][20] =
	    { "bank", "lottery", "gambling", "gang", "beggar", "stock", "shop",
		"police","killer","marriage","office"
	};
	if (readstrvalue(MC_BOSS_FILE, feaStr[pos - 1], boss, IDLEN + 1) != 0)
		*boss = '\0';
}
static void//slowaction ÃØÊéÏµÍ³
whoTakeCharge2(int pos, char *boss)
{
	const char feaStr[][20] =
	    { "bank", "lottery", "gambling", "gang", "beggar", "stock", "shop",
		"police","killer","marriage","office"
	};
	if (readstrvalue(MC_ASS_FILE, feaStr[pos - 1], boss, IDLEN + 1) != 0)
		*boss = '\0';
}

static int //¼ì²é½øÈëÈ¨ÏŞ
check_allow_in()
{
	int backTime;
	int freeTime;
	int currentTime = time(0);
	int num,money;
	int robTimes;

	mc_autoundeny();
	if (seek_in_file(MC_DENY_FILE, currentuser.userid)){
		clear();
		move(10, 10);
		prints("ÄúÒÑ¾­±»ÁĞÈë²»ÊÜ´ó¸»ÎÌ»¶Ó­ÕßÃûµ¥£¬±§Ç¸\n");
		pressanykey();
		return 0;
	}

	/* ±ÜÃâ¶à´°¿Ú*/
	if (multex && count_uindex_telnet(usernum) > 1) {
		clear();
		move(10, 10);
		prints("ÄúÒÑ¾­ÔÚ½ğÈÚÖĞĞÄÀïÀ²!\n");
		pressanykey();
		return 0;
	}
	set_safe_record();
	if (currentuser.dietime > 0) {
		clear();
		move(10, 10);
		prints("ÄúÒÑ²»ÔÚÈË¼ä£¬ÎÒÃÇ²»´ø¹íÍæµÄ~~\n");
		pressanykey();
		return 0;
	}

	/* ·¸×ï±»¼à½û */
	freeTime = loadValue(currentuser.userid, "freeTime", 2000000000);
	if (currentTime < freeTime) {
		clear();
		move(10, 10);
        if((freeTime - currentTime) / 86400>50)
         saveValue(currentuser.userid, "freeTime",
					  time(0) + 86400 *1,
					  2000000000);
	prints("ÄãÒÑ¾­±»±øÂíÙ¸¾¯Êğ¼à½ûÁË¡£»¹ÓĞ%dÌìµÄ¼à½û¡£\n",
	       (freeTime - currentTime) / 86400);
	num=((freeTime - currentTime) / 86400)*25000+25000;
	sprintf(genbuf, "ÊÇ·ñÒªÇó±£ÊÍ£¬±£ÊÍ½ğ%d",num);
	if (askyn(genbuf, NA, NA) == YEA) {
		money =	loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
		 if (money < num) {
			move(8, 4);
			prints ("ÄúµÄÇ®²»¹»£¬¹²Ğè %d ±øÂíÙ¸±Ò", num);
			pressanykey();
			return 0;
		 	}
		 else {
		 	saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
		  	saveValue("BMYpolice", MONEY_NAME, +num, MAX_MONEY_NUM);
			move(8, 4);
			prints("±£ÊÍ³É¹¦");
			robTimes = loadValue(currentuser.userid, "rob", 50);
                	saveValue(currentuser.userid, "rob", -robTimes, 50);
			saveValue(currentuser.userid, "freeTime", -2000000000, 2000000000);
			del_from_file(DIR_MC "imprison_list", currentuser.userid);
		 	}
		}else{
		pressanykey();
		return 0;
		}
	} else
	if (currentTime > freeTime && freeTime > 0) {
		clear();
		move(10, 10);
		prints("¼à½ûÆÚÂú£¬¹§Ï²ÄãÖØĞÂ»ñµÃ×ÔÓÉ£¡");
		saveValue(currentuser.userid, "freeTime", -2000000000, 2000000000);
		del_from_file(DIR_MC "imprison_list", currentuser.userid);
		pressanykey();
		}

	/* Ç·¿î²»»¹ */
	int total_num, lendMoney;
	backTime = loadValue(currentuser.userid, "back_time", 2000000000);
	if((backTime - (int) time(0)) / 3600>5000)
	saveValue(currentuser.userid, "back_time",
					  time(0) + 1* 86400,
					  2000000000);
	lendMoney = loadValue(currentuser.userid, LEND_NAME,
				  		  MAX_MONEY_NUM);
	if (backTime < 0 || lendMoney < 0 ) {
		saveValue(currentuser.userid, LEND_NAME, -lendMoney, MAX_MONEY_NUM);
		saveValue(currentuser.userid, "lend_time", -2000000000, 2000000000);
		saveValue(currentuser.userid, "back_time", -2000000000, 2000000000);
		}
	if (currentTime > backTime && backTime > 0) {
		clear();
		move(10, 10);
		if (askyn("ÄãÇ·ÒøĞĞµÄ´û¿îµ½ÆÚÁË£¬¸Ï½ô»¹°É£¿", YEA, NA) == YEA) {
			total_num =
			    lendMoney + makeInterest(lendMoney, "lend_time",
						     utmpshm->mc.lend_rate/10000.0);
			money = loadValue(currentuser.userid, MONEY_NAME,
					  MAX_MONEY_NUM);
			if (money < total_num) {
				move(11, 10);
				prints("ÄãµÄÇ®²»¹»³¥»¹´û¿î¡£");
				pressanykey();
				return 0;
			}
			saveValue(currentuser.userid,
				  MONEY_NAME, -total_num, MAX_MONEY_NUM);
			saveValue(currentuser.userid, LEND_NAME,
				  -MAX_MONEY_NUM, MAX_MONEY_NUM);
			saveValue(currentuser.userid,
				  "lend_time", -2000000000, 2000000000);
			saveValue(currentuser.userid,
				  "back_time", -2000000000, 2000000000);
			del_from_file(DIR_MC "special_lend",
				      currentuser.userid);
			move(12, 10);
			prints("ºÃÁË£¬ÄãÏÖÔÚÎŞÕ®Ò»ÉíÇáÀ²¡£");
			pressanykey();
			return 1;
		}
		return 0;
	}

	/* ÆäËü²»ÈÃ½øµÄÇé¿ö´ıĞø */

	return 1;
}

static int  //¼ÆËãÀûÏ¢
makeInterest(int basicMoney, char *valueName, float rate)
{
	int lastTime;
	int day;
	int maxDay;

	if (basicMoney <= 0 || rate <= 0) {
		return 0;
	}
	maxDay = MAX_MONEY_NUM / (1 + rate * basicMoney);

	lastTime = loadValue(currentuser.userid, valueName, 2000000000);
	if (lastTime > 0 && time(0) > lastTime) {
		day = (time(0) - lastTime) / 86400;
		day = limitValue(day, maxDay);
		return basicMoney * rate * day;
	}
	return 0;
}

static int
makeRumor(int num)
{
	if (random() % 3) {
		num += (random() % num) / 5;
	} else {
		num -= (random() % num) / 5;
	}
	return limitValue(num, MAX_MONEY_NUM);
}

static void
time2string(int num, char *str)
{
	int i;
	for (i = 0; num > 0; i++, num /= 10) {
		str[9 - i] = num % 10 + '0';
	}
	str[10] = '\0';
}

static int //¼ÆËãÊÇ·ñµ½ÁìÈ¡Ğ½Ë®µÄÊ±ºò
newSalary()
{
	char lastSalaryTime[20];
	return 1;//ÔİÊ±×÷·Ï

	if (!readstrvalue(DIR_MC "etc_time", "salary_time", lastSalaryTime, 20)) {
		if (time(0) < atoi(lastSalaryTime) + 30 * 86400)
			return 0;
		return 1;
/*
		time2string(time(0), genbuf);
		if (savestrvalue(DIR_MC "etc_time", "salary_time", genbuf) == 0) {
			return 1;
		}
		return 0;
*/
	}
	return 0;
}

static int //¼ÆËãĞ½Ë®
makeSalary()
{
	if (currentuser.userlevel & PERM_SYSOP) {
                return SALARY_I;
		}
	if (currentuser.userlevel & PERM_BOARDS) {
		return SALARY_III;
		}
	if (currentuser.userlevel & PERM_OBOARDS ||
            currentuser.userlevel & PERM_ACCOUNTS ||
            currentuser.userlevel & PERM_ARBITRATE ||
            currentuser.userlevel & PERM_SPECIAL7 ||
            currentuser.userlevel & PERM_ACBOARD) {
                return SALARY_II;
		}
	return 0;
}

static void //Ö°ÎñÈÎÃâÏµÍ³
sackOrAppoint(int pos, char *boss, int msgType, char *msg)
{

	char head[10];
	char in[10];
	char end[10];
	char posDesc[][40] =
	    { "±øÂíÙ¸ÒøĞĞĞĞ³¤", "±øÂíÙ¸²©²Ê¹«Ë¾¾­Àí", "±øÂíÙ¸¶Ä³¡¾­Àí",
		"±øÂíÙ¸ºÚ°ï°ïÖ÷", "±øÂíÙ¸Ø¤°ï°ïÖ÷", "±øÂíÙ¸Ö¤¼à»áÖ÷Ï¯",
		"±øÂíÙ¸ÉÌ³¡¾­Àí", "±øÂíÙ¸¾¯ÊğÊğ³¤","±øÂíÙ¸É±ÊÖ°ïÖ÷",
		"±øÂíÙ¸»éÒö¹ÜÀí°ì¹«ÊÒÖ÷ÈÎ", "±øÂíÙ¸´ó¸»ÎÌ¹ÜÀíÖĞĞÄÖ÷ÈÎ"
	};
	if (msgType == 0) {
		strcpy(head, "ÈÎÃü");
		strcpy(in, "Îª");
		strcpy(end, "");
	} else {
		strcpy(head, "ÃâÈ¥");
		strcpy(in, "µÄ");
		strcpy(end, "Ö°Îñ");
	}
	sprintf(msg, "%s %s %s%s%s", head, boss, in, posDesc[pos - 1], end);

}
static void //ÃØÊéÈÎÃâÏµÍ³
sackOrAppoint2(int pos, char *boss, int msgType, char *msg)
{

	char head[10];
	char in[10];
	char end[10];
	char posDesc[][40] =
	    { "±øÂíÙ¸ÒøĞĞĞĞ³¤ÃØÊé", "±øÂíÙ¸²©²Ê¹«Ë¾¾­ÀíÃØÊé", "±øÂíÙ¸¶Ä³¡¾­ÀíÃØÊé",
		"±øÂíÙ¸ºÚ°ï°ïÖ÷ÃØÊé", "±øÂíÙ¸Ø¤°ï°ïÖ÷ÃØÊé", "±øÂíÙ¸Ö¤¼à»áÖ÷Ï¯ÃØÊé",
		"±øÂíÙ¸ÉÌ³¡¾­ÀíÃØÊé", "±øÂíÙ¸¾¯ÊğÊğ³¤ÃØÊé","±øÂíÙ¸É±ÊÖ°ïÖ÷ÃØÊé",
		"±øÂíÙ¸»éÒö¹ÜÀí°ì¹«ÊÒÖ÷ÈÎÃØÊé", "±øÂíÙ¸´ó¸»ÎÌ¹ÜÀíÖĞĞÄÃØÊé"
	};
	if (msgType == 0) {
		strcpy(head, "ÈÎÃü");
		strcpy(in, "Îª");
		strcpy(end, "");
	} else {
		strcpy(head, "ÃâÈ¥");
		strcpy(in, "µÄ");
		strcpy(end, "Ö°Îñ");
	}
	sprintf(msg, "%s %s %s%s%s", head, boss, in, posDesc[pos - 1], end);

}

static int //ÒøĞĞÏµÍ³
money_bank()
{
	int ch;
	int quit = 0;
	int num, money, credit, total_num;
	char uident[IDLEN + 1];
	char title[80];
	char buf[100], letter[256];
	int convert_rate;
	int lendTime;
	int lendMoney, salary;
	int inputValid, withdrawAll;
	float transfer_rate, deposit_rate, lend_rate;
	double backTime;

	while (!quit) {
		money_show_stat("±øÂíÙ¸ÒøĞĞ");
		move(8, 16);
		prints("±øÂíÙ¸ÒøĞĞ»¶Ó­ÄúµÄ¹âÁÙ£¡");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]»»Ç® [2]×ªÕË [3]´¢Ğî [4]´û¿î [5]¹¤×Ê [6]ĞĞ³¤°ì¹«ÊÒ [Q]Àë¿ª\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			clear();
			money_show_stat("±øÂíÙ¸ÒøĞĞ¶Ò»»´°¿Ú");
//			convert_rate = utmpshm->mc.ave_score / 50;
			convert_rate = 100;
			move(4, 4);
			prints
			    ("Äú¿ÉÒÔÍ¨¹ı±äÂôÎÄÕÂÊı»ñµÃ±øÂíÙ¸±Ò¡£½ñÌìµÄ»ãÂÊÊÇ 1:%d",
			     convert_rate);
			move(5, 4);
			prints("\033[1;31m×¢Òâ:ÎÄÕÂÊıÒ»µ©±äÂô¸Å²»ÍË»¹!\033[0m");
			getdata(6, 4, "ÄúÒª±äÂô¶àÉÙÎÄÕÂÊı£¿[0]: ", genbuf, 7,
				DOECHO, YEA);
			num = atoi(genbuf);
			if (num <= 0) {
				break;
			}
			move(7, 4);
			sprintf(genbuf,
				"È·¶¨Òª±äÂô %d ÎÄÕÂÊı£¬»»È¡ %d ±øÂíÙ¸±ÒÂğ£¿",
				num, num * convert_rate);
			if (askyn(genbuf, NA, NA) == YEA) {
				set_safe_record();
				if (currentuser.numposts < num) {
					move(8, 4);
					prints("ÄúÃ»ÓĞÄÇÃ´¶àÎÄÕÂÊı...");
					pressanykey();
					break;
				}
				currentuser.numposts -= num;
				substitute_record(PASSFILE, &currentuser,
						  sizeof (currentuser),
						  usernum);
				saveValue(currentuser.userid, MONEY_NAME,
					  num * convert_rate, MAX_MONEY_NUM);
				move(8, 4);
				prints("½»Ò×³É¹¦£¬ÕâÀïÊÇÄúµÄ %d ±øÂíÙ¸±Ò¡£",
				       num * convert_rate);
				sprintf(genbuf, "%s½øĞĞÒøĞĞ½»Ò×(ÂôÎÄÕÂ)", currentuser.userid);
				sprintf(buf, "%sÂôµô%dÎÄÕÂÊı, »»µÃ %d±øÂíÙ¸±Ò", currentuser.userid, num, num * convert_rate);
				millionairesrec(genbuf, buf, "ÒøĞĞ½»Ò×");
				pressanykey();
			}
			break;
		case '2':
			money_show_stat("±øÂíÙ¸ÒøĞĞ×ªÕË´°¿Ú");
			move(4, 4);
            if(utmpshm->mc.transfer_rate == 0){
                // ÖØÆôbbsdºóÖØĞÂ¶ÁÈ¡×ªÕËÊÖĞø·Ñµ½ÄÚ´æÖĞ by IronBlood@bmy 20120118
                char tmp_transfer_rate[512];
                readstrvalue(MC_RATE_FILE, "transfer_rate", tmp_transfer_rate, sizeof(512));
                utmpshm->mc.transfer_rate = atoi(tmp_transfer_rate);
            }
			transfer_rate = utmpshm->mc.transfer_rate / 10000.0;
			sprintf(genbuf,
				"×îĞ¡×ªÕË½ğ¶î 1000 ±øÂíÙ¸±Ò¡£ÊÖĞø·Ñ %.2f£¥£¨×î¸ßÊÕÈ¡ 100000 ±øÂíÙ¸±Ò£©",
				transfer_rate * 100);
			prints("%s", genbuf);
			move(5, 4);
			usercomplete("×ªÕË¸øË­£¿", uident);
			if (uident[0] == '\0')
				break;
			if (!getuser(uident)) {
				move(6, 4);
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressreturn();
				break;
			}
			if (lookupuser.dietime > 0) {
				move(6, 4);
				prints("Ñô¼äµÄÇ®Ö»ÓĞÉÕ²ÅÄÜ¸øËÀÈË...");
				pressreturn();
				break;
			}
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
				if (seek_in_file(DIR_MC "jijin", currentuser.userid));
				else if (!seek_in_file(DIR_MC "mingren", uident)) {
					move(6, 4);
					prints
					    ("¶Ô²»Æğ£¬ÒøĞĞ²»ÔÊĞí»ÆÂí¹ÓÏòÍâ×ªÕÊ¡£");
					pressreturn();
					break;
				}
			}
			getdata(6, 4, "×ªÕË¶àÉÙ±øÂíÙ¸±Ò£¿[0]", buf, 10,
				DOECHO, YEA);
			if (buf[0] == '\0') {
				break;
			}
			num = atoi(buf);
			if (num < 1000) {
				move(7, 4);
				prints("¶Ô²»Æğ£¬Î´´ïµ½×îĞ¡½»Ò×½ğ¶î¡£");
				pressanykey();
				break;
			}
			if (currentuser.stay < 86400) {
				move(7, 4);
				prints
				    ("¶Ô²»Æğ£¬ÒøĞĞ²»ÏòÎ´³ÉÄêÈËÌá¹©×ªÕÊÒµÎñ¡£");
				pressanykey();
				break;
			}
			move(7, 4);
			sprintf(genbuf, "È·¶¨×ªÕË¸ø %s %d ±øÂíÙ¸±ÒÂğ£¿", uident,
				num);
			if (askyn(genbuf, NA, NA) == YEA) {
				money =
				    loadValue(currentuser.userid, MONEY_NAME,
					      MAX_MONEY_NUM);
				if (num * transfer_rate >= 100000) {
					total_num = num + 100000;
				} else {
					total_num = num * (1.0 + transfer_rate);
				}
				if (money < total_num) {
					move(8, 4);
					prints
					    ("ÄúµÄÇ®²»¹»£¬¼ÓÊÖĞø·Ñ´Ë´Î½»Ò×¹²Ğè %d ±øÂíÙ¸±Ò",
					     total_num);
					pressanykey();
					break;
				}
				saveValue(currentuser.userid, MONEY_NAME,
					  -total_num, MAX_MONEY_NUM);
				saveValue(uident, MONEY_NAME, num,
					  MAX_MONEY_NUM);

				char notebuf[512];
		 		char note[3][STRLEN];
				int i, j;
				move(9, 0);
				prints("»¹ÓĞÊ²Ã´»°Òª¸½ÉÏÂğ£¿[¿ÉÒÔĞ´3ĞĞ]");
				bzero(note, sizeof (note));
				for (i = 0; i < 3; i++) {
					getdata(10 + i, 0, ": ", note[i], STRLEN - 1, DOECHO, NA);
					if (note[i][0] == '\0')
						break;
				}
		 		move(15, 4);
				prints("×ªÕÊ³É¹¦£¬ÎÒÃÇÒÑ¾­Í¨ÖªÁËÄúµÄÅóÓÑ¡£");
				sprintf(title, "ÄúµÄÅóÓÑ %s ¸øÄúËÍÇ®À´ÁË",
					currentuser.userid);
				sprintf(notebuf,
					"ÄúµÄÅóÓÑ %s Í¨¹ı±øÂíÙ¸ÒøĞĞ¸øÄú×ªÕÊÁË %d ±øÂíÙ¸±Ò£¬Çë²éÊÕ¡£"
					"\n\nÒÔÏÂÊÇ %s µÄ¸½ÑÔ:\n",
					currentuser.userid, num, currentuser.userid);
				for (j = 0; j < i; j++){
					strcat(notebuf, note[j]);
					strcat(notebuf, "\n");
				}
				mail_buf(notebuf, uident, title);
				if (seek_in_file(DIR_MC "mingren", currentuser.userid))
				{
					sprintf(title, "%s Ïò %s ×ªÕÊ", currentuser.userid, uident);
					sprintf(buf, " %s Í¨¹ı±øÂíÙ¸ÒøĞĞÏò %s ×ªÕÊÁË %d ±øÂíÙ¸±Ò", currentuser.userid, uident, num);
					mail_buf(buf, "millionaires", title);
				}
				if (num >= RUMOR_MONEY && random() % 2) {
					sprintf(genbuf,
						"¾İËµ %s ÊÕµ½ÁËÒ»±Ê %d ±øÂíÙ¸±ÒµÄ×ªÕÊ£¡",
						uident, makeRumor(num));
					deliverreport
					    ("[Ò¥ÑÔ]À´×Ô±øÂíÙ¸ÒøĞĞµÄÏûÏ¢",
					     genbuf);
				}
				sprintf(genbuf, "%s½øĞĞÒøĞĞ½»Ò×(×ªÕË)", currentuser.userid);
				sprintf(buf,"%s×ªÕÊ¸ø%s %d±øÂíÙ¸±Ò", currentuser.userid, uident, num);
				millionairesrec(genbuf, buf, "ÒøĞĞ½»Ò×");
				pressanykey();
			}
			break;
		case '3':
			clear();
			money_show_stat("±øÂíÙ¸ÒøĞĞ´¢Ğî´°¿Ú");
			move(4, 4);
            if(utmpshm->mc.deposit_rate == 0){
                // ÖØÆôbbsdºóÖØĞÂ¶ÁÈ¡´æ¿îÀûÂÊµ½ÄÚ´æÖĞ by IronBlood@bmy 20120118
                char tmp_deposit_rate[512];
                readstrvalue(MC_RATE_FILE, "deposit_rate", tmp_deposit_rate, sizeof(512));
                utmpshm->mc.deposit_rate = atoi(tmp_deposit_rate);
            }
			deposit_rate = utmpshm->mc.deposit_rate / 10000.0;
			sprintf(genbuf,
				"×îĞ¡´æÈ¡¿î½ğ¶î 1000 ±øÂíÙ¸±Ò¡£´æ¿îÀûÂÊ£¨ÈÕ£©Îª %.2f£¥",
				deposit_rate * 100);
			prints("%s", genbuf);
			move(t_lines - 1, 0);
			prints
			    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]´æ¿î [2]È¡¿î [Q]Àë¿ª\033[m");
			ch = igetkey();
			switch (ch) {
			case '1':
				getdata(6, 4, "ÄúÒª´æ¶àÉÙ±øÂíÙ¸±Ò?[0]", buf,
					10, DOECHO, YEA);
				if (buf[0] == '\0') {
					break;
				}
				num = atoi(buf);
				if (num < 1000) {
					move(7, 4);
					prints("¶Ô²»Æğ£¬Î´´ïµ½×îĞ¡½»Ò×½ğ¶î¡£");
					pressanykey();
					break;
				}
				move(7, 4);
				sprintf(genbuf, "È·¶¨´æ %d ±øÂíÙ¸±ÒÂğ?", num);
				if (askyn(genbuf, NA, NA) == NA) {
					break;
				}
				money =
				    loadValue(currentuser.userid, MONEY_NAME,
					      MAX_MONEY_NUM);
				credit =
				    loadValue(currentuser.userid, CREDIT_NAME,
					      MAX_MONEY_NUM);
				if (money < num) {
					move(8, 4);
					prints("ÄúÃ»ÓĞÕâÃ´¶àÇ®¿ÉÒÔ´æ¡£");
					pressanykey();
					break;
				}
				if (credit + num > MAX_MONEY_NUM) {
					move(8, 4);
					prints("¿ÕÊØ×ÅÕâÃ´¶à´æ¿î×öÊ²Ã´ÄØ£¿");
					pressanykey();
					break;
				}
				/* ¿ÛÇ® */
				saveValue(currentuser.userid, MONEY_NAME, -num,
					  MAX_MONEY_NUM);
				/* ¼ÆËãÔ­ÏÈ´æ¿îµÄÀûÏ¢ */
				saveValue(currentuser.userid, INTEREST_NAME,
					  makeInterest(credit, "deposit_time",
						       deposit_rate),
					  MAX_MONEY_NUM);
				/* ´æ¿î */
				saveValue(currentuser.userid, CREDIT_NAME,
					  num, MAX_MONEY_NUM);
				saveValue(currentuser.userid,
					  "deposit_time", -2000000000,
					  2000000000);
				/* ĞÂµÄ´æ¿î¿ªÊ¼Ê±¼ä */
				saveValue(currentuser.userid, "deposit_time",
					  time(0), 2000000000);
				move(8, 4);
				prints
				    ("½»Ò×³É¹¦£¬ÄúÏÖÔÚ´æÓĞ %d ±øÂíÙ¸±Ò£¬ÀûÏ¢¹²¼Æ %d ±øÂíÙ¸±Ò¡£",
				     loadValue(currentuser.userid, CREDIT_NAME,
					       MAX_MONEY_NUM),
				     loadValue(currentuser.userid,
					       INTEREST_NAME, MAX_MONEY_NUM));
				if (num >= RUMOR_MONEY && random() % 2) {
					sprintf(genbuf,
						"ÓĞÈËÄ¿»÷ %s ÔÚ±øÂíÙ¸ÒøĞĞ´æÈëÁË %d µÄ±øÂíÙ¸±Ò£¡",
						currentuser.userid,
						makeRumor(num));
					deliverreport
					    ("[Ò¥ÑÔ]À´×Ô±øÂíÙ¸ÒøĞĞµÄÏûÏ¢",
					     genbuf);
				}
				pressanykey();
				break;
			case '2':
				getdata(6, 4, "ÄúÒªÈ¡¶àÉÙ±øÂíÙ¸±Ò?[0]", buf,
					10, DOECHO, YEA);
				if (buf[0] == '\0') {
					break;
				}
				num = atoi(buf);
				if (num < 1000) {
					move(7, 4);
					prints("¶Ô²»Æğ£¬Î´´ïµ½×îĞ¡½»Ò×½ğ¶î¡£");
					pressanykey();
					break;
				}
				move(7, 4);
				sprintf(genbuf, "È·¶¨È¡ %d ±øÂíÙ¸±ÒÂğ?", num);
				if (askyn(genbuf, NA, NA) == NA) {
					break;
				}
				credit =
				    loadValue(currentuser.userid, CREDIT_NAME,
					      MAX_MONEY_NUM);
				if (num > credit) {
					move(8, 4);
					prints("ÄúÃ»ÓĞÄÇÃ´¶à´æ¿î¡£");
					pressanykey();
					break;
				}
				withdrawAll = 0;
				total_num = num;
				if (num == credit) {
					move(8, 4);
					sprintf(genbuf,	"ÊÇ·ñÒ»²¢È¡³ö %d ±øÂíÙ¸±ÒµÄ´æ¿îÀûÏ¢£¿",
						loadValue(currentuser.userid, INTEREST_NAME, MAX_MONEY_NUM)
                        + makeInterest(num, "deposit_time", deposit_rate));
					if (askyn(genbuf, NA, NA) == YEA) {
						/* ´æ¿î¼ÓÀûÏ¢ */
						total_num =
						    num + makeInterest(num,
								       "deposit_time",
								       deposit_rate)
						    +
						    loadValue(currentuser.
							      userid,
							      INTEREST_NAME,
							      MAX_MONEY_NUM);
						withdrawAll = 1;
					}
				}

				credit =
				    loadValue(currentuser.userid, CREDIT_NAME,
					      MAX_MONEY_NUM);
				if (num > credit) {
					move(9, 4);
					prints("ÄúÃ»ÓĞÄÇÃ´¶à´æ¿î¡£");
					pressanykey();
					break;
				}
				/* ¼õÈ¥È¡¿î */
				saveValue(currentuser.userid, CREDIT_NAME,
					  -num, MAX_MONEY_NUM);
				/* È¡µÃÏÖ½ğ */
				saveValue(currentuser.userid, MONEY_NAME,
					  total_num, MAX_MONEY_NUM);
				/* ¼ÆËãËùÈ¡µÄÇ®µÄÀûÏ¢ */
				if (withdrawAll) {
					saveValue(currentuser.userid,
						  INTEREST_NAME, -MAX_MONEY_NUM,
						  MAX_MONEY_NUM);
				} else {
					saveValue(currentuser.userid,
						  INTEREST_NAME,
						  makeInterest(num,
							       "deposit_time",
							       deposit_rate),
						  MAX_MONEY_NUM);
				}
				move(8, 4);
				prints
				    ("½»Ò×³É¹¦£¬ÄúÏÖÔÚ´æÓĞ %d ±øÂíÙ¸±Ò£¬´æ¿îÀûÏ¢¹²¼Æ %d ±øÂíÙ¸±Ò¡£",
				     loadValue(currentuser.userid, CREDIT_NAME,
					       MAX_MONEY_NUM),
				     loadValue(currentuser.userid,
					       INTEREST_NAME, MAX_MONEY_NUM));
				pressanykey();
				break;
			case 'Q':
			case 'q':
				break;
			}
			break;
		case '4':
			clear();
			money_show_stat("±øÂíÙ¸ÒøĞĞ´û¿î´°¿Ú");
			move(4, 4);
            if(utmpshm->mc.lend_rate == 0){
                // ÖØÆôbbsdºóÖØĞÂ¶ÁÈ¡´û¿îÀûÂÊµ½ÄÚ´æÖĞ by IronBlood@bmy 20120118
                char tmp_lend_rate[512];
                readstrvalue(MC_RATE_FILE, "lend_rate", tmp_lend_rate, sizeof(512));
                utmpshm->mc.lend_rate = atoi(tmp_lend_rate);
            }
			lend_rate = utmpshm->mc.lend_rate / 10000.0;
			sprintf(genbuf,
				"×îĞ¡½»Ò×½ğ¶î 1000 ±øÂíÙ¸±Ò¡£´û¿îÀûÂÊ£¨ÈÕ£©Îª %.2f£¥",
				lend_rate * 100);
			prints("%s", genbuf);
			move(5, 4);
			lendMoney =
			    loadValue(currentuser.userid, LEND_NAME,
				      MAX_MONEY_NUM);
			total_num =
			    lendMoney + makeInterest(lendMoney, "lend_time",
						     lend_rate);
			lendTime =
			    loadValue(currentuser.userid, "lend_time",
				      2000000000);
			if (lendTime > 0) {
				sprintf(genbuf,
					"Äú´û¿î %d ±øÂíÙ¸±Ò£¬µ±Ç°±¾Ï¢¹²¼Æ %d ±øÂíÙ¸±Ò£¬¾àµ½ÆÚ %d Ğ¡Ê±¡£",
					lendMoney,
					total_num,
					(loadValue
					 (currentuser.userid, "back_time",
					  2000000000) - (int) time(0)) / 3600);
			} else {
				sprintf(genbuf, "ÄúÄ¿Ç°Ã»ÓĞ´û¿î¡£");
			}
			prints("%s", genbuf);
			move(t_lines - 1, 0);
			prints
			    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]´û¿î [2]»¹´û [Q]Àë¿ª\033[m");
			ch = igetkey();
			switch (ch) {
			case '1':
				move(6, 4);
				sprintf(genbuf,
					"°´ÕÕÒøĞĞµÄ¹æ¶¨£¬ÄúÄ¿Ç°×î¶à¿ÉÒÔÉêÇë´û¿î %d ±øÂíÙ¸±Ò¡£",
					countexp(&currentuser) * 100);
				prints("%s", genbuf);
				getdata(7, 4, "ÄúÒª´û¿î¶àÉÙ±øÂíÙ¸±Ò?[0]", buf,
					10, DOECHO, YEA);
				if (buf[0] == '\0') {
					break;
				}
				num = atoi(buf);

				if (lendMoney > 0) {
					move(8, 4);
					prints("ÇëÏÈ»¹Çå´û¿î¡£");
					pressanykey();
					break;
				}
				if (num < 1000) {
					move(8, 4);
					prints("¶Ô²»Æğ£¬Î´´ïµ½×îĞ¡½»Ò×½ğ¶î¡£");
					pressanykey();
					break;
				}
				if (num > countexp(&currentuser) * 100) {
					move(8, 4);
					prints
					    ("¶Ô²»Æğ£¬ÄúÒªÇó´û¿îµÄ½ğ¶î³¬¹ıÒøĞĞ¹æ¶¨¡£");
					pressanykey();
					break;
				}
				inputValid = 0;
				while (!inputValid) {
					getdata(8, 4,
						"ÄúÒª´û¿î¶àÉÙÌì£¿[3-30]: ", buf,
						3, DOECHO, YEA);
					if (atoi(buf) > 2 && atoi(buf) < 31) {
						inputValid = 1;
					}
				}
				saveValue(currentuser.userid, MONEY_NAME, num,
					  MAX_MONEY_NUM);
				saveValue(currentuser.userid, LEND_NAME, num,
					  MAX_MONEY_NUM);
				saveValue(currentuser.userid, "lend_time",
					  time(0), 2000000000);
				saveValue(currentuser.userid, "back_time",
					  time(0) + atoi(buf) * 86400,
					  2000000000);
				move(9, 4);
				prints("ÄúµÄ´û¿îÊÖĞøÒÑ¾­Íê³É¡£Çëµ½ÆÚ»¹¿î¡£");
				sprintf(genbuf, "%s½øĞĞÒøĞĞ½»Ò×(´û¿î)", currentuser.userid);
				sprintf(buf, "%s´û¿î%d±øÂíÙ¸±Ò%dÌì", currentuser.userid, num, atoi(buf));
				millionairesrec(genbuf, buf, "ÒøĞĞ½»Ò×");
				pressanykey();
				break;
			case '2':
				move(6, 4);
				backTime =
				    loadValue(currentuser.userid, "back_time", 2000000000);
				if((backTime - (int) time(0)) / 3600>5000||(backTime - (int) time(0)) / 3600<-30)
                			saveValue(currentuser.userid, "back_time", time(0) + 1* 86400, 2000000000);
				lendTime =
				    loadValue(currentuser.userid, "lend_time", 2000000000);
				if (lendTime == 0) {
					prints("Äú¼Ç´íÁË°É£¿Ã»ÓĞÕÒµ½ÄúµÄ´û¿î¼ÇÂ¼°¡¡£");
					pressanykey();
					break;
				}
				if (time(0) - lendTime < 86400) {
					prints ("¶Ô²»Æğ£¬ÒøĞĞ²»½ÓÊÜÎ´²úÉúÀûÏ¢µÄ»¹´û¡£");
					pressanykey();
					break;
				}
				if (askyn("ÄúÒªÏÖÔÚ³¥»¹´û¿îÂğ£¿", NA, NA) == YEA) {
					money = loadValue(currentuser.userid,
						      MONEY_NAME,
						      MAX_MONEY_NUM);
					move(7, 4);
					if (money < total_num) {
						prints ("¶Ô²»Æğ£¬ÄúµÄÇ®²»¹»³¥»¹´û¿î¡£");
						pressanykey();
						break;
					}
					saveValue(currentuser.userid,
						  MONEY_NAME, -total_num,
						  MAX_MONEY_NUM);
					saveValue(currentuser.userid, LEND_NAME,
						  -MAX_MONEY_NUM,
						  MAX_MONEY_NUM);
					saveValue(currentuser.userid,
						  "lend_time", -2000000000,
						  2000000000);
					saveValue(currentuser.userid,
						  "back_time", -2000000000,
						  2000000000);
					del_from_file(DIR_MC "special_lend",
						      currentuser.userid);
					sprintf(genbuf, "%s½øĞĞÒøĞĞ½»Ò×(»¹´û)", currentuser.userid);
					sprintf(buf,"%s³¥»¹´û¿î±¾Àû¹²%d±øÂíÙ¸±Ò", currentuser.userid, total_num);
					millionairesrec(genbuf, buf, "ÒøĞĞ½»Ò×");
					prints
					    ("ÄúµÄ´û¿îÒÑ¾­»¹Çå¡£ÒøĞĞÀÖ¼û²¢Ãú¼ÇÄúµÄ³ÏĞÅ¡£");
					pressanykey();
				}
				break;
			case 'q':
			case 'Q':
				break;
			}
			break;
		case '5':
			clear();
			money_show_stat("±øÂíÙ¸ÒøĞĞ¹¤×Ê´ú°ì´°¿Ú");
			salary = makeSalary();
			if (salary == 0) {
				move(10, 10);
				prints("¶Ô²»Æğ£¬Äú²»ÊÇ±¾Õ¾¹«ÎñÔ±£¬Ã»ÓĞ¹¤×Ê¡£");
				pressanykey();
				break;
			}
			if (utmpshm->mc.isSalaryTime == 0) {
				move(10, 10);
				prints
				    ("¶Ô²»Æğ£¬ÒøĞĞ»¹Ã»ÓĞÊÕµ½¹¤×Ê»®¿î¡£Çë¹ı¼¸ÌìÔÙÀ´¡£");
				pressanykey();
				break;
			}
			if (seek_in_file
			    (DIR_MC "salary_list", currentuser.userid)) {
				move(10, 10);
				prints("ÄúÒÑ¾­Áì¹ı±¾ÔÂ¹¤×Ê¡£»¹ÊÇÇÚ·Ü¹¤×÷°É£¡");
				pressanykey();
				break;
			}
			move(6, 4);
			sprintf(genbuf, "Äú±¾ÔÂµÄ¹¤×Ê %d ±øÂíÙ¸±ÒÒÑ¾­»®µ½ÒøĞĞ¡£ÏÖÔÚÁìÈ¡Âğ£¿",	salary);
			if (askyn(genbuf, NA, NA) == YEA) {
				saveValue(currentuser.userid,
					  MONEY_NAME, salary, MAX_MONEY_NUM);
				addtofile(DIR_MC "salary_list",
					  currentuser.userid);
				move(8, 4);
				prints
				    ("ÕâÀïÊÇÄúµÄ¹¤×Ê¡£¸ĞĞ»ÄúÎª±øÂíÙ¸¸¶³öµÄ¹¤×÷!");
				pressanykey();
				break;
			}
			break;
		case '6':
			clear();
			money_show_stat("±øÂíÙ¸ÒøĞĞĞĞ³¤°ì¹«ÊÒ");
			move(6, 4);
			char name[20];
			whoTakeCharge2(1, name);
			whoTakeCharge(1, uident);
			if (strcmp(currentuser.userid, uident)) {
				prints
				    ("ÃØÊé%sÀ¹×¡ÁËÄã£¬ÈáÉùËµµÀ:¡°ĞĞ³¤%sÕıÔÚÀïÃæ°ì¹«£¬ÇëÎğ´òÈÅ¡£¡±",
				    name,uident);
				pressanykey();
				break;
			} else {
				prints("ÇëÑ¡Ôñ²Ù×÷´úºÅ:");
				move(7, 6);
				prints
				    ("1. µ÷Õû´æ¿îÀûÂÊ                  2. µ÷Õû´û¿îÀûÂÊ");
				move(8, 6);
				prints
				    ("3. µ÷Õû×ªÕÊ·ÑÂÊ                  4. ÉóÅú´û¿î");
				move(9, 6);
				prints
				    ("5. µ÷²éÓÃ»§ÕÊ»§                  6. ÌØ±ğ´û¿îÃûµ¥");
				move(10, 6);
				prints
				    ("7. ·¢·Å¹¤×Ê                      8. ÌØÊâ²¦¿î");
				move(11,6);
				prints
				    ("9. ´ÇÖ°                          Q. ÍË³ö");
				ch = igetkey();
				switch (ch) {
				case '1':
					getdata(12, 4,
						"Éè¶¨ĞÂµÄ´æ¿îÀûÂÊ[10-250]: ",
						buf, 4, DOECHO, YEA);
					if (atoi(buf) < 10 || atoi(buf) > 250) {
						break;
					}
					move(14, 4);
					sprintf(genbuf,
						"ĞÂµÄ´æ¿îÀûÂÊÊÇ %.2f£¥£¬È·¶¨Âğ£¿",
						atoi(buf) / 100.0);
					if (askyn(genbuf, NA, NA) == YEA) {
						savestrvalue(MC_RATE_FILE,
							     "deposit_rate",
							     buf);
						utmpshm->mc.deposit_rate =
						    atoi(buf);
						move(15, 4);
						prints("ÉèÖÃÍê±Ï¡£");
						sprintf(genbuf,
							"ĞÂµÄ´æ¿îÀûÂÊÎª %.2f£¥ ¡£",
							utmpshm->mc.
							deposit_rate / 100.0);
						deliverreport
						    ("[¹«¸æ]±øÂíÙ¸ÒøĞĞµ÷Õû´æ¿îÀûÂÊ",
						     genbuf);
						sprintf(genbuf, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ",currentuser.userid);
						sprintf(buf, "ÉèÖÃĞÂµÄ´æ¿îÀûÂÊÎª %.2f£¥ ¡£", utmpshm->mc.
							deposit_rate / 100.0);
						millionairesrec(genbuf, buf, "");
						pressanykey();
					}
					break;
				case '2':
					getdata(12, 4,
						"Éè¶¨ĞÂµÄ´û¿îÀûÂÊ[10-250]: ",
						buf, 4, DOECHO, YEA);
					if (atoi(buf) < 10 || atoi(buf) > 250) {
						break;
					}
					move(14, 4);
					sprintf(genbuf,
						"ĞÂµÄ´û¿îÀûÂÊÊÇ %.2f£¥£¬È·¶¨Âğ£¿",
						atoi(buf) / 100.0);
					if (askyn(genbuf, NA, NA) == YEA) {
						savestrvalue(MC_RATE_FILE,
							     "lend_rate", buf);
						utmpshm->mc.lend_rate =
						    atoi(buf);
						move(15, 4);
						prints("ÉèÖÃÍê±Ï¡£");
						sprintf(genbuf,
							"ĞÂµÄ´û¿îÀûÂÊÎª %.2f£¥ ¡£",
							utmpshm->mc.lend_rate /
							100.0);
						deliverreport
						    ("[¹«¸æ]±øÂíÙ¸ÒøĞĞµ÷Õû´û¿îÀûÂÊ",
						     genbuf);
						sprintf(genbuf, "%sĞĞÊ¹ÒøĞĞ¹ÜÀíÈ¨ÏŞ",currentuser.userid);
						sprintf(buf, "ÉèÖÃĞÂµÄ´û¿îÀûÂÊÎª %.2f£¥ ¡£", utmpshm->mc.
							lend_rate / 100.0);
						millionairesrec(genbuf, buf, "");
						pressanykey();
					}
					break;
				case '3':
					getdata(12, 4,
						"Éè¶¨ĞÂµÄ×ªÕÊ·ÑÂÊ[10-250]: ",
						buf, 4, DOECHO, YEA);
					if (atoi(buf) < 10 || atoi(buf) > 250) {
						break;
					}
					move(14, 4);
					sprintf(genbuf,
						"ĞÂµÄ×ªÕÊ·ÑÂÊÊÇ %.2f£¥£¬È·¶¨Âğ£¿",
						atoi(buf) / 100.0);
					if (askyn(genbuf, NA, NA) == YEA) {
						savestrvalue(MC_RATE_FILE,
							     "transfer_rate",
							     buf);
						utmpshm->mc.transfer_rate =
						    atoi(buf);
						move(15, 4);
						prints("ÉèÖÃÍê±Ï¡£");
						sprintf(genbuf,
							"ĞÂµÄ×ªÕÊ·ÑÂÊÎª %.2f£¥ ¡£",
							utmpshm->mc.
							transfer_rate / 100.0);
						deliverreport
						    ("[¹«¸æ]±øÂíÙ¸ÒøĞĞµ÷Õû×ªÕÊ·ÑÂÊ",
						     genbuf);
						sprintf(genbuf, "%sĞĞÊ¹ÒøĞĞ¹ÜÀíÈ¨ÏŞ",currentuser.userid);
						sprintf(buf, "ÉèÖÃĞÂµÄ×ªÕÊ·ÑÂÊÎª %.2f£¥ ¡£", utmpshm->mc.
							transfer_rate / 100.0);
						millionairesrec(genbuf, buf, "");
						pressanykey();
					}
					break;
				case '4':
					move(12, 4);
					usercomplete("ÏòË­Ìá¹©ÌØ±ğ´û¿î£¿",
						     uident);
					if (uident[0] == '\0')
						break;
					if (!getuser(uident)) {
						move(13, 4);
						prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
						pressreturn();
						break;
					}
					if (lookupuser.dietime > 0) {
						move(13, 4);
						prints("Õâ¸öÊÇËÀÈË...");
						pressreturn();
						break;
					}
					if (loadValue
					    (uident, "lend_time", 2000000000) > 0) {
						move(13, 4);
						prints
						    ("¸Ã¿Í»§ÒÑ¾­´û¿î£¬²»ÒË×·¼Ó´û¿î¡£");
						pressanykey();
						break;
					}
					getdata(13, 4, "´û¿î½ğ¶î[0]: ", buf, 10, DOECHO, YEA);
					if (buf[0] == '\0')
						break;

					if (atoi(buf) < 100000) {
						move(14, 4);
						prints
						    ("ÕâÃ´µãÇ®£¬ÓªÒµÌü¾Í¿ÉÒÔ°ìÀí¡£");
						pressanykey();
						break;
					}
					if (atoi(buf) > 100000000) {
						move(14, 4);
						prints
						    ("Èç´ËÊı¶î¾Ş´óµÄ´û¿î£¬¿ÖÅÂ¶­ÊÂ»á²»»áÍ¬ÒâµÄ¡£");
						pressanykey();
						break;
					}
					num = atoi(buf);
					getdata(14, 4, "´û¿îÌìÊı[3-30]: ", buf, 3, DOECHO, YEA);
					if (atoi(buf) < 1 || atoi(buf) > 30)
						break;
					move(15, 4);
					if (askyn("È·¶¨Ïò¸Ã¿Í»§Ìá¹©´û¿îÂğ£¿", NA, NA) == NA)
						break;
					time_t t = time(0) + 86400 * atoi(buf);
					sprintf(genbuf, "%s\t%s", uident, ctime(&t));
					addtofile(DIR_MC "special_lend", genbuf);
					saveValue(uident, MONEY_NAME, num,
						  MAX_MONEY_NUM);
					saveValue(uident, LEND_NAME, num,
						  MAX_MONEY_NUM);
					saveValue(uident, "lend_time", time(0),
						  2000000000);
					saveValue(uident, "back_time",
						  time(0) + atoi(buf) * 86400,
						  2000000000);
					sprintf(genbuf,
						"´û¿î½ğ¶î %d ±øÂíÙ¸±Ò£¬ÇëÎñ±ØÓÚ %s ÌìÄÚ³¥»¹´û¿î¡£",
						num, buf);
					mail_buf(genbuf, uident,
						 "±øÂíÙ¸ÒøĞĞĞĞ³¤Í¬ÒâÁËÄúµÄ´û¿îÉêÇë");
					move(16, 4);
					prints
					    ("´û¿îÉóÅúÍê³É¡£ÇëÈ·±£¿Í»§¼°Ê±»¹¿î¡£");
					sprintf(buf, "¸ø%sÌØ±ğ´û¿î£¬%s",uident, genbuf);
					sprintf(genbuf, "%sĞĞÊ¹ÒøĞĞ¹ÜÀíÈ¨ÏŞ",currentuser.userid);
					millionairesrec(genbuf, buf, "");
					pressanykey();
					break;
				case '5':
					move(12, 4);
					usercomplete("µ÷²éË­µÄÕÊ»§£º", uident);
					if (uident[0] == '\0')
						break;
					if (!getuser(uident)) {
						move(13, 4);
						prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
						pressreturn();
						break;
					}
					move(14, 4);
					sprintf(genbuf,
						"¸Ã¿Í»§ÓĞÏÖ½ğ%d ±øÂíÙ¸±Ò£¬´æ¿î %d ±øÂíÙ¸±Ò,´û¿î %d ±øÂíÙ¸±Ò¡£",
						loadValue(uident, MONEY_NAME,
							MAX_MONEY_NUM),
						loadValue(uident, CREDIT_NAME,
							MAX_MONEY_NUM),
						loadValue(uident, LEND_NAME,
							MAX_MONEY_NUM));
					prints("%s", genbuf);
					pressanykey();
					break;
				case '6':
					clear();
					move(1, 0);
					prints("Ä¿Ç°ÒøĞĞµÄÌØ±ğ´û¿îÇé¿ö£º");
					move(2, 0);
					prints("¿Í»§ID\t»¹¿îÊ±¼ä");
					listfilecontent(DIR_MC "special_lend");
					pressanykey();
					break;
				case '7':
					move(12, 4);
					if (newSalary()) {
						if (askyn("È·¶¨·¢·Å±¾ÔÂ¹¤×ÊÂğ£¿", NA, NA) == YEA) {
							time2string(time(0), genbuf);
							if (savestrvalue(DIR_MC "etc_time", "salary_time", genbuf)){
								move(14, 4);
								prints("´íÎó!²»ÄÜĞ´ÎÄ¼ş!");
								pressanykey();
								break;
							}
							strcpy(currboard, "sysop");
							deliverreport
							    ("[¹«¸æ]±¾Õ¾¹«ÎñÔ±ÁìÈ¡±¾ÔÂ¹¤×Ê",
							     "ÇëÓÚ7ÌìÄÚµ½±øÂíÙ¸ÒøĞĞÁìÈ¡£¬¹ıÆÚÊÓÎª·ÅÆú¡£\n");
							strcpy(currboard,	 MC_BOARD);
							remove(DIR_MC "salary_list");
							utmpshm->mc.isSalaryTime = 1;
							move(14, 4);
							prints("²Ù×÷Íê³É¡£");
							sprintf(genbuf, "%sĞĞÊ¹ÒøĞĞ¹ÜÀíÈ¨ÏŞ", currentuser.userid);
							millionairesrec(genbuf, "·¢·Å¹¤×Ê", "");
							pressanykey();
						}
					} else {
						prints("»¹Î´µ½·¢·ÅÊ±¼ä¡£");
						pressanykey();
					}
					break;
				case '8':
					move(12, 4);
					usercomplete("ÏòË­Ìá¹©ÌØ±ğ²¦¿î£¿",
						     uident);
					if (uident[0] == '\0')
						break;
					if (!getuser(uident)) {
						move(13, 4);
						prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
						pressreturn();
						break;
					}
					if (lookupuser.dietime > 0) {
						move(13, 4);
						prints("Õâ¸öÊÇËÀÈË...");
						pressreturn();
						break;
					}
					getdata(13, 4,"²¦¿î½ğ¶î[0]: ", buf, 10, DOECHO, YEA);
					if (buf[0] == '\0') {
						break;
					}
					if (atoi(buf) < 100000) {
						move(14, 4);
						prints
						    ("ÕâÃ´µãÇ®£¬ÓªÒµÌü¾Í¿ÉÒÔ°ìÀí¡£");
						pressanykey();
						break;
					}
					if (atoi(buf) > 100000000) {
						move(14, 4);
						prints
						    ("Èç´ËÊı¶î¾Ş´óµÄ´û¿î£¬¿ÖÅÂ¶­ÊÂ»á²»»áÍ¬ÒâµÄ¡£");
						pressanykey();
						break;
					}
					num = atoi(buf);
					getdata(15, 4, "²¦¿îÔ­Òò£º", buf, 50, DOECHO, YEA);
					sprintf(letter, "²¦¿îÓÃÓÚ×ÔÉíµÄ½¨Éè·¢Õ¹£¬ÍûÆä°´ÕÕ¹æ¶¨Ê¹ÓÃ£¬²»µÃ½øĞĞÎ¥·¨ÂÒ¼Í»î¶¯£¡\n\n²¦¿îÔ­Òò£º%s", buf);
					move(16, 4);
					if (askyn("È·¶¨Ïò¸Ã¿Í»§²¦¿îÂğ£¿", NA, NA) == NA)
						break;
					saveValue(uident, MONEY_NAME, num, MAX_MONEY_NUM);
					sprintf(genbuf,"ÊÚÓè%s %d±øÂíÙ¸±ÒÔ®Öú²¦¿î",uident, num);
					deliverreport(genbuf, letter);
					mail_buf(genbuf, uident,
						 "±øÂíÙ¸ÒøĞĞĞĞ³¤Í¬ÒâÁËÄúµÄ²¦¿îÉêÇë");
					move(17, 4);
					prints("²¦¿îÍê³É¡£");
					sprintf(buf, "¸ø%sÌØ±ğ²¦¿î%d±øÂíÙ¸±Ò",uident, num);
					sprintf(genbuf, "%sĞĞÊ¹ÒøĞĞ¹ÜÀíÈ¨ÏŞ", currentuser.userid);
					millionairesrec(genbuf, buf, "");
					pressanykey();
					break;
				case '9':
					move(12, 4);
					if (askyn("ÄúÕæµÄÒª´ÇÖ°Âğ£¿", NA, NA) ==
					    YEA) {
					/*	del_from_file(MC_BOSS_FILE, "bank");
						sprintf(genbuf,
							"%s Ğû²¼´ÇÈ¥±øÂíÙ¸ÒøĞĞĞĞ³¤Ö°Îñ",
							currentuser.userid);
						deliverreport(genbuf,
							      "±øÂíÙ¸½ğÈÚÖĞĞÄ¶ÔÆäÒ»Ö±ÒÔÀ´µÄ¹¤×÷±íÊ¾¸ĞĞ»£¬×£ÒÔºóË³Àû£¡");
						move(14, 4);
						prints
						    ("ºÃ°É£¬¼ÈÈ»ÄãÒâÒÑ¾ö£¬¶­ÊÂ»áÒ²²»±ãÇ¿Áô¡£ÔÙ¼û£¡");
						quit = 1;
					*/
						sprintf(genbuf, "%s Òª´ÇÈ¥±øÂíÙ¸ÒøĞĞĞĞ³¤Ö°Îñ",
							currentuser.userid);
						mail_buf(genbuf, "millionaires", genbuf);
						move(14, 4);
						prints("ºÃ°É£¬ÒÑ¾­·¢ĞÅ¸æÖª×Ü¹ÜÁË");
						pressanykey();
					}
					break;
				case 'q':
				case 'Q':
					break;
				}
			}
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

static int//²ÊÆ±ÏµÍ³
money_lottery()
{
	int ch, money;
	int quit = 0, quitRoom = 0;
	int inputValid;
	char buf[100], uident[IDLEN + 1];
	char letter[200];
	char title[80];
	FILE *fp;
	long openTime;
	char name[20];

	clear();
	while (!quit) {
		nomoney_show_stat("±øÂíÙ¸²ÊÆ±ÖĞĞÄ");
		move(6, 4);
		prints("²ÊÆ±ÖĞĞÄÂ¡ÖØ¿ªÕÅ£¬»¶Ó­´ó¼ÒÓ»Ô¾¹ºÂò²ÊÆ±¡«¡«");
		move(8, 4);
		prints("²ÊÆ±¹æÔòÇëµ½millionaires°æ²éÑ¯¡£");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]36Ñ¡7 [2]×ã²Ê [3]¾­ÀíÊÒ [Q]Àë¿ª\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			nomoney_show_stat("36Ñ¡7");
			if (access(DIR_MC_TEMP "36_7_start", 0)) {
				move(6, 4);
				prints("±§Ç¸£¡ĞÂÒ»ÆÚµÄ36Ñ¡7²ÊÆ±»¹Î´¿ªÊ¼ÏúÊÛ¡£");
				pressanykey();
				break;
			}
			move(5, 4);
			prints("Êı×Ö¼äÓÃ-¸ô¿ª£¬ÀıÈç 08-13-01-25-34-17-18");
			move(7, 4);
			sprintf(genbuf,
				"µ±Ç°½±½ğ³ØÀÛ»ı½±½ğ£º\033[1;31m%d\033[m   ¹Ì¶¨½±½ğ£º\033[1;31m%d\033[m",
				utmpshm->mc.prize367, PRIZE_PER);
			prints("%s", genbuf);
			move(9, 4);
			sprintf(genbuf, "Ã¿×¢ 10000 ±øÂíÙ¸±Ò¡£È·¶¨Âò×¢Âğ?");
			if (askyn(genbuf, NA, NA) == YEA) {
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				move(10, 4);
				if (money < 10000) {
					prints
					    ("Ã»ÓĞÇ®¾Í±ğµ·ÂÒ£¬Ò»±ßÈ¥£¡ÏÂÒ»¸ö£¡");
					pressanykey();
					break;
				}

				saveValue(currentuser.userid, MONEY_NAME, -10000, MAX_MONEY_NUM);	//¿ÛÇ®
				utmpshm->mc.prize367 += 10000;
				utmpshm->mc.prize367 =
				    limitValue(utmpshm->mc.prize367, MAX_POOL_MONEY);
				inputValid = 0;
				while (!inputValid) {
					getdata(10, 4, "ÇëÌîĞ´Âò×¢µ¥: ", buf, 21, DOECHO, YEA);	/* 2¡Á7£«6£«1£½21 */

					if (!valid367Bet(buf)) {	// ¼ìÑéÏÂ×¢µÄºÏÀíĞÔ
						move(11, 4);
						prints
						    ("¶Ô²»Æğ£¬ÄúµÄÏÂ×¢µ¥ÌîĞ´ºÃÏñÓĞÎÊÌâÒ®¡£ÇëÖØÌîÒ»´Î¡£");
						pressanykey();
					} else {
						sprintf(genbuf, "%s %s", currentuser.userid, buf);
						addtofile(DIR_MC_TEMP "36_7_list", genbuf);
						move(11, 4);
						prints
						    ("                                                             ");
						move(11, 4);
						prints
						    ("¹ºÂò³É¹¦¡£×£ÄúÖĞ´ó½±£¡");
						inputValid = 1;
						sprintf(letter,
							"Äú¹ºÂòÁËÒ»×¢36Ñ¡7¡£×¢ºÅÊÇ£º%s¡£ÇëÍ×ÉÆ±£´æ£¬µ½ÆÚ¶Ò½±¡£", buf);
						sprintf(title,
							"²ÊÆ±ÖĞĞÄ¹ºÂòÆ¾Ö¤");
						mail_buf(letter, currentuser.userid, title);
						pressanykey();
					}

				}

			}
			break;
		case '2':
			nomoney_show_stat("×ã²Ê");
			move(6, 4);
			if (access(DIR_MC_TEMP "soccer_start", 0)) {
				prints("±§Ç¸£¡ĞÂÒ»ÆÚµÄ×ãÇò²ÊÆ±»¹Î´¿ªÊ¼ÏúÊÛ¡£");
				pressanykey();
				break;
			}
			if (utmpshm->mc.isSoccerSelling == 0) {
				prints("±§Ç¸£¡ÏúÊÛÆÚÒÑ¾­½áÊø£¬ÇëµÈ´ı¿ª½±¡£");
				pressanykey();
				break;
			}
			move(4, 4);
			prints("×ã²ÊËù²Â±ÈÈüÇë¼ûmillionaires°æ¹«¸æÎÄÕÂ¡£");
			move(5, 4);
			prints
			    ("Ö÷³¡Ê¤Îª3£¬Ö÷³¡Æ½Îª1£¬Ö÷³¡¸ºÎª0¡£¸÷³¡±ÈÈü½á¹ûÓÃ-¸ô¿ª£¬Ö§³Ö¸´Ê½Âò×¢¡£");
			move(6, 4);
			prints
			    ("ÀıÈç²Â6³¡±ÈÈüÊ±£¬Ò»¸ö½ÓÊÜµÄÂò×¢·¶ÀıÎª£º 1-310-1-10-3-0");
			move(8, 4);
			sprintf(genbuf,
				"µ±Ç°½±½ğ³ØÀÛ¼Æ½±½ğ£º\033[1;31m%d\033[m  ¹Ì¶¨½±½ğ£º\033[1;31m%d\033[m",
				utmpshm->mc.prizeSoccer, PRIZE_PER);
			prints("%s", genbuf);
			move(10, 4);
			sprintf(genbuf, "Ã¿×¢10000±øÂíÙ¸±Ò¡£È·¶¨Âò×¢Âğ?");
			if (askyn(genbuf, NA, NA) == YEA) {
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				move(11, 4);
				if (money < 10000) {
					prints("Ã»ÓĞÇ®¾Í±ğµ·ÂÒ£¬Ò»±ßÈ¥£¡ÏÂÒ»¸ö£¡");
					pressanykey();
					break;
				}

				inputValid = 0;
				while (!inputValid) {
					int sum;
					getdata(11, 4, "ÇëÌîĞ´Âò×¢µ¥: ", buf, 55, DOECHO, YEA);
					if (!validSoccerBet(buf)) {	/* ¼ìÑéÏÂ×¢µÄºÏÀíĞÔ */
						move(12, 4);
						prints
						    ("¶Ô²»Æğ£¬ÄúµÄÏÂ×¢µ¥ÌîĞ´ºÃÏñÓĞÎÊÌâÒ®¡£ÇëÖØÌîÒ»´Î¡£");
						pressanykey();
					} else {
						int money;
						inputValid = 1;
						sum = computeSum(buf);	/*¼ÆËã¸´Ê½Âò×¢µÄ×¢Êı */
						money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
						if (sum > money / 10000) {
							move(12, 4);
							prints("                                                     ");
							move(12, 4);
							sprintf(genbuf,
								"ÄúµÄÇ®²»¹»Âò %d ×¢¡£ÔÙÕå×ÃÒ»ÏÂ°É£¡",
								sum);
							prints("%s", genbuf);
							pressanykey();
							break;
						}
						saveValue(currentuser.userid, MONEY_NAME, -sum * 10000, MAX_MONEY_NUM);	/*¿ÛÇ® */
						utmpshm->mc.prizeSoccer += sum * 10000;
						utmpshm->mc.prizeSoccer = limitValue(utmpshm->mc.prizeSoccer, MAX_POOL_MONEY);
						saveSoccerRecord(buf);	/*´¦Àí²¢±£´æ¸´Ê½Âò×¢¼ÇÂ¼ */
						move(12, 4);
						prints("                                                             ");
						move(12, 4);
						sprintf(genbuf,"ÄúÒ»¹²¹ºÂòÁË%d×¢¡£×£ÄúÖĞ´ó½±£¡", sum);
						prints("%s", genbuf);
						sprintf(letter,
							"Äú¹ºÂòÁËÒ»×¢(¸´Ê½)×ã²Ê¡£×¢ºÅÊÇ£º%s¡£ÇëÍ×ÉÆ±£´æ£¬µ½ÆÚ¶Ò½±¡£",
							buf);
						sprintf(title, "²ÊÆ±ÖĞĞÄ¹ºÂòÆ¾Ö¤");
						mail_buf(letter, currentuser.userid, title);
						pressanykey();
					}
				}
			}
			break;
		case '3':
			nomoney_show_stat("²©²Ê¹«Ë¾¾­ÀíÊÒ");
			whoTakeCharge(2, uident);
			whoTakeCharge2(2, name);
			if (strcmp(currentuser.userid, uident)) {
				move(6, 4);
				prints("ÃØÊé%sÌáÊ¾Äú:¡°¾­Àí%sÍâ³ö¿¼²ìÈ¥ÁË£¬ÓĞÊÂÇëÖ±½Ó¸úËûÁªÏµ¡£¡±",
				    name, uident);
				pressanykey();
				break;
			}
			quitRoom = 0;
			while (!quitRoom) {
				char strTime[15];
				nomoney_show_stat("²©²Ê¹«Ë¾¾­ÀíÊÒ");
				move(t_lines - 1, 0);
				prints
				    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]¿ª½± [2]ĞÂ½¨ [3]Í£Ö¹×ã²ÊÏúÊÛ [4]´ÇÖ° [Q]Àë¿ª\033[m");
				ch = igetkey();
				switch (ch) {
				case '1':
					nomoney_show_stat("²©²Ê¹«Ë¾¾­ÀíÊÒ");
					move(6, 10);
					prints("1.  36Ñ¡7");
					move(7, 10);
					prints("2.  ×ãÇò²ÊÆ±");
					move(8, 10);
					prints("Q.  ÍË³ö");
					move(4, 4);
					prints("ÇëÑ¡ÔñÒª¿ª½±µÄ²ÊÆ±´úºÅ£º");
					move(12, 4);
					ch = igetkey();
					move(t_lines - 5, 15);
					switch (ch) {
					case '1':
						fp = fopen(DIR_MC_TEMP "36_7_start", "r");
						if (fp) {
							fgets(strTime, 15, fp);
							openTime = atoi(strTime);
							fclose(fp);
							if (time(0) >= openTime) //||strcmp(currentuser.userid,"macintosh")==0
								if (open_36_7() == 0) {
									prints("¿ª½±³É¹¦£¡");
									sprintf(genbuf, "%sĞĞÊ¹²ÊÆ±¹ÜÀíÈ¨ÏŞ",currentuser.userid);
									millionairesrec(genbuf, "36Ñ¡7¿ª½±", "");
								}
								else prints("·¢ÉúÒâÍâ´íÎó...");
							else
								prints("¿ª½±Ê±¼ä»¹Ã»ÓĞµ½°¡£¡");

						} else
							prints("Ã»ÓĞ¸Ã²ÊÆ±µÄ¼ÇÂ¼¡£");
						pressanykey();
						break;
					case '2':
						fp = fopen(DIR_MC_TEMP "soccer_start", "r");
						if (fp) {
							fgets(strTime, 15, fp);
							fclose(fp);
							openTime =atoi(strTime);
							if (time(0) >= openTime) //||strcmp(currentuser.userid,"macintosh")==0
							{
								getdata(t_lines - 5, 4,
									"ÇëÊäÈë¶Ò½±ĞòÁĞ(ÎŞĞè - )[°´\033[1;33mENTER\033[m·ÅÆú]: ",
									buf, 55, DOECHO, YEA);
								if (strlen(buf) == 0)
									break;
								if (open_soccer(buf) ==0)	{
									prints("¿ª½±³É¹¦£¡");
									sprintf(genbuf, "%sĞĞÊ¹²ÊÆ±¹ÜÀíÈ¨ÏŞ",currentuser.userid);
									millionairesrec(genbuf, "×ã²Ê¿ª½±", "");
								}
								else prints("·¢ÉúÒâÍâ´íÎó...");

							} else
								prints ("¿ª½±Ê±¼ä»¹Ã»ÓĞµ½°¡£¡");
						} else
							prints ("Ã»ÓĞ¸Ã²ÊÆ±µÄ¼ÇÂ¼¡£");
						pressanykey();
						break;
					case 'q':
					case 'Q':
						break;
					}
					break;
				case '2':
					nomoney_show_stat("²©²Ê¹«Ë¾¾­ÀíÊÒ");
					move(6, 10);
					prints("1. 36Ñ¡7 ");
					move(7, 10);
					prints("2. ×ã²Ê");
					move(8, 10);
					prints("Q. ÍË³ö");
					move(4, 4);
					prints("ÇëÑ¡Ôñ¿ª½±ÖÖÀà»ò²Ù×÷£º");
					ch = igetkey();
					switch (ch) {
					case '1':
						nomoney_show_stat
						    ("²©²Ê¹«Ë¾¾­ÀíÊÒ");
						move(4, 4);
						if (!access(DIR_MC_TEMP "36_7_start",0)) {
							prints("36Ñ¡7²ÊÆ±ÏúÊÛÕıÔÚ»ğÈÈ½øĞĞ¡£");
							pressanykey();
							break;
						}
						prints("ĞÂ½¨36Ñ¡7");
						inputValid = 0;
						while (!inputValid) {
							getdata(8, 4,"²ÊÆ±ÏúÊÛÌìÊı[1-7]: ",buf, 2, DOECHO,YEA);
							if (buf[0] > '0' && buf[0] < '8')
								inputValid = 1;
						}
						time2string(time(0) + (buf[0] - '0') * 86400, genbuf);
						addtofile(DIR_MC_TEMP "36_7_start", genbuf);

						sprintf(genbuf,
							"±¾ÆÚ²ÊÆ±½«ÓÚ %s Ììºó¿ª½±¡£»¶Ó­´ó¼ÒÓ»Ô¾¹ºÂò£¡",
							buf);
						deliverreport
						    ("[¹«¸æ]ĞÂÒ»ÆÚ36Ñ¡7²ÊÆ±¿ªÊ¼ÏúÊÛ", genbuf);

						move(10, 4);
						prints("½¨Á¢³É¹¦£¡Çëµ½Ê±¿ª½±¡£");
						sprintf(genbuf, "ĞÂ½¨36Ñ¡7£¬%sÌìºó¿ª½±¡£",buf);
						sprintf(buf, "%sĞĞÊ¹²ÊÆ±¹ÜÀíÈ¨ÏŞ",currentuser.userid);
						millionairesrec(buf, genbuf, "");
						pressanykey();
						break;
					case '2':
						nomoney_show_stat("²©²Ê¹«Ë¾¾­ÀíÊÒ");
						move(4, 4);
						if (!access(DIR_MC_TEMP "soccer_start",0)) {
							prints("×ãÇò²ÊÆ±ÏúÊÛÕıÔÚ»ğÈÈ½øĞĞ¡£");
							pressanykey();
							break;
						}
						prints("ĞÂ½¨×ã²Ê");
						inputValid = 0;
						while (!inputValid) {
							getdata(8, 4,"²ÊÆ±ÏúÊÛÌìÊı[1-7]: ",buf, 2, DOECHO,YEA);
							if (buf[0] > '0' && buf[0] < '8')
								inputValid = 1;
						}
						time2string(time(0) +(buf[0] - '0') * 86400, genbuf);
						addtofile(DIR_MC_TEMP "soccer_start", genbuf);
						utmpshm->mc.isSoccerSelling = 1;
						sprintf(genbuf,
							"±¾ÆÚ²ÊÆ±½«ÓÚ %s Ììºó¿ª½±¡£»¶Ó­´ó¼ÒÓ»Ô¾¹ºÂò£¡",
							buf);
						deliverreport
						    ("[¹«¸æ]ĞÂÒ»ÆÚ×ãÇò²ÊÆ±¿ªÊ¼ÏúÊÛ", genbuf);

						move(10, 4);
						prints("½¨Á¢³É¹¦£¡Çëµ½Ê±¿ª½±¡£");
						sprintf(genbuf, "ĞÂ½¨×ã²Ê£¬%sÌìºó¿ª½±¡£",buf);
						sprintf(buf, "%sĞĞÊ¹²ÊÆ±¹ÜÀíÈ¨ÏŞ",currentuser.userid);
						millionairesrec(buf, genbuf, "");
						pressanykey();
						break;
					case 'q':
					case 'Q':
						break;
					}
					break;

				case '3':
					nomoney_show_stat("²©²Ê¹«Ë¾¾­ÀíÊÒ");
					move(6, 4);
					if (askyn("ÄúÕæµÄÒªÍ£ÊÛ×ã²ÊÂğ£¿", NA, NA) == YEA) {
						utmpshm->mc.isSoccerSelling = 0;
						deliverreport("[¹«¸æ]±¾ÆÚ×ãÇò²ÊÆ±Í£Ö¹ÏúÊÛ",
							      "Çë¹ã´ó²ÊÃñÄÍĞÄµÈ´ı¿ª½±£¡");
						sprintf(buf, "%sĞĞÊ¹²ÊÆ±¹ÜÀíÈ¨ÏŞ", currentuser.userid);
						millionairesrec(buf, "Í£ÊÛ±¾ÆÚ×ã²Ê", "");
						move(8, 4);
						prints("ÒÑ¾­Í£ÊÛ£¡Çëµ½Ê±¿ª½±¡£");
						pressanykey();
					}
					break;

				case '4':
					nomoney_show_stat("²©²Ê¹«Ë¾¾­ÀíÊÒ");
					move(6, 4);
					if (askyn("ÄúÕæµÄÒª´ÇÖ°Âğ£¿", NA, NA) ==
					    YEA) {
					/*	del_from_file(MC_BOSS_FILE, "lottery");
						sprintf(genbuf,
							"%s Ğû²¼´ÇÈ¥±øÂíÙ¸²©²Ê¹«Ë¾¾­ÀíÖ°Îñ",
							currentuser.userid);
						deliverreport(genbuf,
							      "±øÂíÙ¸½ğÈÚÖĞĞÄ¶ÔÆäÒ»Ö±ÒÔÀ´µÄ¹¤×÷±íÊ¾¸ĞĞ»£¬×£ÒÔºóË³Àû£¡");
						move(8, 4);
						prints
						    ("ºÃ°É£¬¼ÈÈ»ÄãÒâÒÑ¾ö£¬¶­ÊÂ»áÒ²²»±ãÇ¿Áô¡£ÔÙ¼û£¡");
						pressanykey();
						quitRoom = 1;
					*/
						sprintf(genbuf, "%s Òª´ÇÈ¥±øÂíÙ¸²©²Ê¹«Ë¾¾­ÀíÖ°Îñ",
							currentuser.userid);
						mail_buf(genbuf, "millionaires", genbuf);
						move(8, 4);
						prints("ºÃ°É£¬ÒÑ¾­·¢ĞÅ¸æÖª×Ü¹ÜÁË");
						pressanykey();
					}
					break;
				case 'q':
				case 'Q':
					quitRoom = 1;
					break;
				}
			}
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}


struct MC_Jijin{
	char userid[14];
	char name[18];
};

static int
addOrDel_contrb()
{
	char uident[STRLEN], ans[8];
	int count = 0, tag = 0, i, j, fd, x=0;
	char buf[STRLEN], title[STRLEN];
	void *buffer = NULL;
	size_t filesize;
	FILE *fpw;

	struct MC_Jijin JijinTmp, *JijinMem;

	while (1) {
		clear();
		count = get_num_records(MC_JIJIN_CTRL_FILE, sizeof(struct MC_Jijin));
		if (count == 0){
			if ((fd = open(MC_JIJIN_CTRL_FILE, O_CREAT | O_EXCL | O_WRONLY, 0660)) == -1)
				return -1;
			close(fd);
		}
		filesize = sizeof(struct MC_Jijin) * count;
		JijinMem = loadData(MC_JIJIN_CTRL_FILE, buffer, filesize);
		if (JijinMem == (void *) -1)
			return -1;

		prints("Éè¶¨½øÈë¾è¿îÃûµ¥µÄ»ù½ğ: \n");
		j = 0;
		for(i = 0; i<count ;i++){
			if (JijinMem[i].userid[0]==0)
				continue;
			sprintf(buf, "%-12.12s  %s", JijinMem[i].userid, JijinMem[i].name);
			if (j < 15)
				showAt(3+j, 0, buf, 0);
			else if (j <30)
				showAt(3+j, 40, buf, 0);
			j++;
		}
		if (j==0){
			unlink(MC_JIJIN_CTRL_FILE);
			count = 0;
		}

		if (count)
			getdata(1, 0, "(A)Ôö¼Ó (D)É¾³ı (C)¸Ä±ä (E)Àë¿ª [E]: ", ans, 2, DOECHO, YEA);
		else
			getdata(1, 0, "(A)Ôö¼Ó (E)Àë¿ª [E]: ", ans, 2, DOECHO, YEA);

		tag = 0;
		if (*ans == 'A' || *ans == 'a') {
			move(1, 0);
			while (1){
				move(1, 0);
				clrtoeol();
				usercomplete("Ôö¼Óid£º", uident);
				if (*uident == '\0')
					break;
				if (!getuser(uident)) {
					showAt(2, 0, "¸Ãid²»´æÔÚ", 1);
					tag = -1;
					break;
				}
				if (!seek_in_file(DIR_MC "jijin", uident)) {
					showAt(2, 0, "¸Ãid²»ÊÇ»ù½ğ!", 1);
					tag = -1;
					break;
				}
				for(i = 0; i<count ;i++){
					if (!strcmp(JijinMem[i].userid, uident)){
						showAt(2, 0, "¸ÃidÒÑ¾­´æÔÚ", 1);
						tag = -1;
						break;
					}
				}
				if (tag == 0)
					tag = 1;
				break;
			}
			buf[0] = 0;
			memset(&JijinTmp, 0, sizeof(struct MC_Jijin));
			if (tag == 1){
				sprintf(JijinTmp.userid, "%s", uident);
				while (buf[0] == 0)
					getdata(2, 0, "ÇëÊäÈë»ù½ğÃû³Æ: ", buf, 18, DOECHO, YEA);
				sprintf(JijinTmp.name, "%s", buf);
				append_record(MC_JIJIN_CTRL_FILE, &JijinTmp, sizeof(struct MC_Jijin));
				sprintf(title, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ(ÉèÖÃ¾è¿î»ù½ğ)", currentuser.userid);
				sprintf(buf,"%s°Ñ%sÌí¼ÓÎª %s»ù½ğ", currentuser.userid, JijinTmp.userid, JijinTmp.name);
				millionairesrec(title,buf, "");
			}
		} else if ((*ans == 'C' || *ans == 'c')) {
			move(1, 0);
			usercomplete("¸Ä±äÄÄ¸öid: ", uident);
			if (*uident != '\0') {
				for(i = 0; i<count ;i++)
					if (!strcmp(JijinMem[i].userid, uident)){
						tag = 1;
						break;
					}
			}
			buf[0] = 0;
			if (tag == 1){
				while (buf[0] == 0)
					getdata(2, 0, "ÇëÊäÈëĞÂµÄÃû³Æ: ", buf, 18, DOECHO, YEA);
				sprintf(JijinMem[i].name, "%s", buf);
				sprintf(title, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ(ÉèÖÃ¾è¿î»ù½ğ)", currentuser.userid);
				sprintf(buf,"%s¸Ä±ä%sµÄÃû³ÆÎª %s»ù½ğ", currentuser.userid, JijinMem[i].userid, JijinMem[i].name);
				millionairesrec(title,buf, "");
				saveData(JijinMem, filesize);
			} else
				showAt(2, 0, "ÄúÊäÈëµÄid²»ÔÚÁĞ±íÖĞ", 1);
		} else if ((*ans == 'D' || *ans == 'd') && count) {
			move(1, 0);
			usercomplete("É¾³ıid: ", uident);
			if (uident[0] != '\0') {
				for(i = 0; i < count ;i++)
					if (!strcmp(JijinMem[i].userid, uident)){
						tag = 1;
						x = i;
						break;
					}
			}
			if (tag == 1){
				fpw = fopen(MC_JIJIN_CTRL_FILE".tmp", "w");
				if (fpw == 0) {
					showAt(2, 0, "·¢ÉúÒâÍâ´íÎó!", 1);
					return -1;
				}
				sprintf(title, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ(ÉèÖÃ¾è¿î»ù½ğ)", currentuser.userid);
				sprintf(buf,"%sÉ¾³ı%s(%s»ù½ğ)", currentuser.userid, JijinMem[i].userid, JijinMem[i].name);
				millionairesrec(title,buf, "");
				for (i = 0; i < x; i++)
					fwrite(&JijinMem[i], sizeof(struct MC_Jijin), 1, fpw);
				for (i = x+1; i < count; i++)
					fwrite(&JijinMem[i], sizeof(struct MC_Jijin), 1, fpw);
				fclose(fpw);
				unlink(MC_JIJIN_CTRL_FILE);
				rename(MC_JIJIN_CTRL_FILE".tmp", MC_JIJIN_CTRL_FILE);
				showAt(2, 0, "É¾³ı³É¹¦", 1);
			}else
				showAt(2, 0, "ÄúÊäÈëµÄid²»ÔÚÁĞ±íÖĞ", 1);
		} else
			break;
	}
	clear();
	return 1;
}


static int
money_sackOrAppoint(int type) //type1Ö°Î» 2ÃØÊé
{
	int pos, i=0 , j;
	char buf[100], letter[100], report[100], uident[IDLEN + 1], boss[IDLEN + 1];
	const char feaStr[][20] =
	    { "bank", "lottery", "gambling", "gang", "beggar", "stock", "shop",
		"police","killer","marriage","office",""
	};
	const char feaStr2[][20] =
	    { "ÒøĞĞ", "²ÊÆ±", "¶Ä³¡", "ºÚ°ï", "Ø¤°ï", "¹ÉÊĞ", "ÉÌ³¡",
		"¾¯Êğ","É±ÊÖ","½ÌÌÃ","ÖĞĞÄ", ""
	};

	clear();
	if (type==1)
		showAt(2, 4, "Ä¿Ç°±øÂíÙ¸½ğÈÚÖĞĞÄ¸÷Ö°Î»Çé¿ö£º", 0);
	if (type==2)
		showAt(2, 4, "Ä¿Ç°±øÂíÙ¸½ğÈÚÖĞĞÄ¸÷ÃØÊéÖ°Î»Çé¿ö£º", 0);

	while (feaStr[i][0]){
		if (type == 1)
			whoTakeCharge(i+1, boss);
		else
			whoTakeCharge2(i+1, boss);
		sprintf(buf, "%d.%s: %s", i+1, feaStr2[i], boss);
		showAt(i+5, 4, buf, 0);
		i++;
	}

	getdata(16, 4, "ÇëÑ¡ÔñÖ°Îñ? ", buf, 3, DOECHO, YEA);
	pos = atoi(buf);
	if (pos > 11 || pos < 1)
		return 0;

	getdata(16, 4, "ÇëÑ¡Ôñ:  1.ÈÎÃü  2.ÃâÖ°? ", buf, 2, DOECHO, YEA);
	j = atoi(buf);
	if (j > 2 || j < 1)
		return 0;

	if (type == 1)
		whoTakeCharge(pos, boss);
	else
		whoTakeCharge2(pos, boss);
	if (j==1){
		if (boss[0] != '\0') {	//Èç¹û¸ÃÖ°Î»·Ç¿Õ
			prints("%sÒÑ¾­¸ºÔğ¸ÃÖ°Î»¡£", boss);
			pressanykey();
			return 0;
		}
		move(16, 4);
		usercomplete("ÈÎÃüË­£¿", uident);
		move(17, 4);
		if (uident[0] == '\0')
			return 0;
		if (!searchuser(uident)) {
			prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
			pressanykey();
			return 0;
		}
		sprintf(genbuf, "È·¶¨ÈÎÃü %s Ö°Î» %d%s Âğ£¿", uident, pos, (type==1)?"":"ÃØÊé");
		if (askyn(genbuf, NA, NA) == YEA) {
			if (type==1){
				sackOrAppoint(pos, uident, 0, letter);
				savestrvalue(MC_BOSS_FILE, feaStr[pos - 1], uident);
			}else{
				sackOrAppoint2(pos, uident, 0, letter);
				savestrvalue(MC_ASS_FILE, feaStr[pos - 1], uident);
			}
			deliverreport(letter,
				      "½÷ÍûÆäÄÜÁ®½à·î¹«£¬²»ÒÔÈ¨Ä±Ë½Àû£¬Îª±øÂíÙ¸½ğÈÚÊÂÒµµÄ·¢Õ¹¾Ï¹ª¾¡´á¡£");
			mail_buf(letter, uident, letter);
			sprintf(genbuf, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ", currentuser.userid);
			sprintf(buf,"%sÈÎÃü%s¸ºÔğ%dÖ°Î»%s", currentuser.userid, uident, pos, (type==1)?"":"ÃØÊé");
			millionairesrec(genbuf, buf, "");
			move(18, 4);
			prints("ÈÎÃü³É¹¦¡£");
			pressanykey();
		}
	}else if (j==2){
		if (boss[0] == '\0') {	//Èç¹û¸ÃÖ°Î»Îª¿Õ
			prints("Ä¿Ç°²¢ÎŞÈË¸ºÔğ¸ÃÖ°Î»¡£");
			pressanykey();
			return 0;
		}
		getdata(17, 4, "ÃâÈ¥Ô­Òò:", genbuf, 50, DOECHO, YEA);
		sprintf(report, "ÃâÈ¥Ô­Òò£º%s", genbuf);
		move(17, 4);
		sprintf(genbuf, "È·¶¨ÃâÈ¥ %s µÄ%sÖ°Î»Âğ£¿", boss, (type==1)?"":"ÃØÊé");
		if (askyn(genbuf, NA, NA) == YEA) {
			if (type==1){
				sackOrAppoint(pos, boss, 1, letter);
				del_from_file(MC_BOSS_FILE, (char *) feaStr[pos - 1]);
			}else{
				sackOrAppoint2(pos, boss, 1, letter);
				del_from_file(MC_ASS_FILE, (char *) feaStr[pos - 1]);
			}
			deliverreport(letter, report);
			mail_buf(letter, boss, letter);
			sprintf(genbuf, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ", currentuser.userid);
			sprintf(buf,"%sÃâÈ¥%sµÄ%d%sÖ°Î»", currentuser.userid, boss, pos, (type==1)?"":"ÃØÊé");
			millionairesrec(genbuf, buf, "");
			move(18, 4);
			prints("ÃâÖ°³É¹¦¡£");
			pressanykey();
		}
	}
	return 1;
}


static int //¹ÜÀíÏµÍ³  ¹ÉÆ±ÏµÍ³
money_admin()
{
	int ch, i, j, quit = 0;
	char buf[100], letter[100], uident[IDLEN + 1];
	char stockboard[STRLEN][MAX_STOCK_NUM];
	FILE *fp1;
	int count;

	if (!seek_in_file(MC_ADMIN_FILE, currentuser.userid)
	    && !(currentuser.userlevel & PERM_SPECIAL5) && strcmp(currentuser.userid, "macintosh")) {
		return 0;
	}
	clear();
	while (!quit) {
		clear();
		nomoney_show_stat("±øÂíÙ¸½ğÈÚÖĞĞÄ¹ÜÀí");
		move(5, 4);
		prints("ÕâÀï¸ºÔğ±øÂíÙ¸½ğÈÚÖĞĞÄµÄÈËÊÂ¹ÜÀí¡£");
		move(7, 8);
		prints("A. ÈÎÃü½ğÈÚÖĞĞÄ×Ü¹Ü             B. ÃâÈ¥½ğÈÚÖĞĞÄ×Ü¹Ü");
		move(8, 8);
		prints("C. ÁĞ³ö×Ü¹ÜÃûµ¥           ");
		move(9, 8);
		prints("E. ÈÎÃâÖ°Î»                     F. ÈÎÃâÃØÊé");
		move(10, 8);
		prints("I. ÉèÖÃÉÏ°ñÃñ¼ä»ù½ğ");
		move(11, 8);
		prints("J. ÈÎÃüÃûÈËÌÃ³ÉÔ±               K. È¡ÏûÃûÈËÌÃ×Ê¸ñ");
		move(12, 8);
		prints("L. ÈÎÃü²èÓÑ                     M. È¡Ïû²èÓÑ×Ê¸ñ");
		move(13, 8);
		prints("N. ÈÎÃüÌú¹«¼¦                   O. È¡ÏûÌú¹«¼¦");
		move(14, 8);
		prints("Y. ÈÎÃü»ù½ğid                   Z. È¡Ïû»ù½ğid");
		move(15, 8);
		prints("R. ÁĞ³öÃûÈËÌÃ³ÉÔ±               S. ÁĞ³ö²èÓÑÃûµ¥");
		move(16, 8);
		prints("T. ÁĞ³öÌú¹«¼¦Ãûµ¥               U. ÁĞ³ö»ù½ğidÃûµ¥");
		move(17, 8);
		prints("P. ¹ÉÊĞ³õÊ¼»¯");
		move(19, 8);
		prints("X. ºÚÃûµ¥²Ù×÷                   0. ¿ª¹Ø½ğÈÚÖĞĞÄ");
		move(20, 8);
		prints("1. ¸Ä±ä¸öÈËÏÖ½ğ                 2. ¸Ä±ä¸öÈË´æ¿î");
		move(22, 8);
		prints("G. ´ÇÖ°                         Q. ÍË³ö");


		ch = igetkey();
		switch (ch) {
		case 'e':
		case 'E':
			money_sackOrAppoint(1);
			break;

		case 'f':
		case 'F':
			money_sackOrAppoint(2);//ÃØÊé
			break;

		case 'a':
		case 'A':
			clear();
			move(15, 4);
			usercomplete("ÊÚÓèË­½ğÈÚÖĞĞÄ×Ü¹ÜÈ¨ÏŞ£¿", uident);
			move(16, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressanykey();
				break;
			}
			if (!seek_in_file(MC_ADMIN_FILE, uident)) {
				if (askyn("È·¶¨Âğ£¿", NA, NA) == NA) {
					break;
				}
				addtofile(MC_ADMIN_FILE, uident);
				move(17, 4);
				prints("ÈÎÃü³É¹¦!");
				sprintf(genbuf,
					"[¹«¸æ]ÊÚÓè %s ±øÂíÙ¸½ğÈÚÖĞĞÄ¹ÜÀíÈ¨ÏŞ",
					uident);
				deliverreport(genbuf,
					      "½÷ÍûÆäÄÜÁ®½à·î¹«£¬²»ÒÔÈ¨Ä±Ë½Àû£¬Îª±øÂíÙ¸½ğÈÚÊÂÒµµÄ·¢Õ¹¾Ï¹ª¾¡´á¡£");
				sprintf(genbuf,
					"%s ÓÉ %s ÊÚÓè±øÂíÙ¸½ğÈÚÖĞĞÄ¹ÜÀíÈ¨ÏŞ",
					uident, currentuser.userid);
				mail_buf(genbuf, uident, genbuf);
				//add by macintosh for system record
				sprintf(genbuf, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ", currentuser.userid);
				sprintf(buf,"%sÈÎÃü%sÎª½ğÈÚÖĞĞÄ×Ü¹Ü", currentuser.userid, uident);
				millionairesrec(genbuf, buf, "");
			} else {
				prints("¸ÃIDÒÑ¾­¾ßÓĞ½ğÈÚÖĞĞÄ¹ÜÀíÈ¨ÏŞ");
			}
			pressanykey();
			break;
		case 'c':
		case 'C':
			clear();
			move(1, 0);
			prints("Ä¿Ç°¾ßÓĞ¹ÜÀíÈ¨ÏŞµÄIDÁĞ±í£º");
			listfilecontent(MC_ADMIN_FILE);
			pressanykey();
			break;
		case 'b':
		case 'B':
			clear();
			move(15, 4);
			usercomplete("È¡ÏûË­µÄ½ğÈÚÖĞĞÄ×Ü¹ÜÈ¨ÏŞ£¿", uident);
			move(16, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressanykey();
				break;
			}
			if (seek_in_file(MC_ADMIN_FILE, uident)) {
				getdata(16, 4, "È¡ÏûÔ­Òò£º", buf, 50,
					DOECHO, YEA);
				move(17, 4);
				if (askyn("È·¶¨Âğ£¿", NA, NA) == NA) {
					pressanykey();
					break;
				}
				del_from_file(MC_ADMIN_FILE, uident);
				move(18, 4);
				prints("È¡Ïû³É¹¦!");
				sprintf(genbuf,
					"[¹«¸æ]È¡Ïû %s µÄ±øÂíÙ¸½ğÈÚÖĞĞÄ¹ÜÀíÈ¨ÏŞ",
					uident);
				sprintf(letter, "È¡ÏûÔ­Òò£º %s", buf);
				deliverreport(genbuf, letter);
				sprintf(genbuf,
					"%s ±» %s È¡Ïû±øÂíÙ¸½ğÈÚÖĞĞÄ¹ÜÀíÈ¨ÏŞ",
					uident, currentuser.userid);
				mail_buf(genbuf, uident, genbuf);
				sprintf(genbuf, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ", currentuser.userid);
				sprintf(buf,"%sÈ¡Ïû%sµÄ½ğÈÚÖĞĞÄ×Ü¹ÜÈ¨ÏŞ", currentuser.userid, uident);
				millionairesrec(genbuf, buf, "");
			} else {
				prints("¸ÃIDÃ»ÓĞ´ËÈ¨ÏŞ¡£");
			}
			pressanykey();
			break;

		case 'g':
		case 'G':
			clear();
			move(15, 4);
			if (!seek_in_file(MC_ADMIN_FILE, currentuser.userid)) {
				break;
			}
			if (askyn("ÄúÕæµÄÒª´ÇÖ°Âğ£¿", NA, NA) == YEA) {
				del_from_file(MC_ADMIN_FILE,
					      currentuser.userid);
				sprintf(genbuf,
					"%s Ğû²¼´ÇÈ¥±øÂíÙ¸½ğÈÚÖĞĞÄ×Ü¹ÜÖ°Îñ",
					currentuser.userid);
				deliverreport(genbuf,
					      "±øÂíÙ¸½ğÈÚÖĞĞÄ¶ÔÆäÒ»Ö±ÒÔÀ´µÄ¹¤×÷±íÊ¾¸ĞĞ»£¬×£ÒÔºóË³Àû£¡");
				sprintf(genbuf, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ", currentuser.userid);
				sprintf(buf,"%s´ÇÈ¥±øÂíÙ¸½ğÈÚÖĞĞÄ×Ü¹ÜÖ°Îñ", currentuser.userid);
				millionairesrec(genbuf, buf, "");
				move(16, 4);
				prints
				    ("ºÃ°É£¬¼ÈÈ»ÄãÒâÒÑ¾ö£¬½ğÈÚÖĞĞÄÒ²²»±ãÇ¿Áô¡£ÔÙ¼û£¡");
				quit = 1;
				pressanykey();
			}
			break;
			case 'j':
			case 'J':
				clear();
				move(15, 4);
				usercomplete("ÈÎÃüË­½øÃûÈËÌÃ£¿", uident);
				move(16, 4);
				if (uident[0] == '\0')
					break;
				if (!searchuser(uident)) {
					prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
					pressanykey();
					break;
				}
				if (seek_in_file
				    (DIR_MC "mingren", uident)) {
					prints("¸ÃIDÒÑ¾­ÊÇÃûÈËÁË¡£");
					pressanykey();
					break;
				}
				if (askyn("È·¶¨Âğ£¿", NA, NA) == YEA) {
					addtofile(DIR_MC "mingren",
						  uident);
					sprintf(genbuf,
						"¹§Ï²%s½øÈë±øÂíÙ¸½ğÈÚÖĞĞÄÃûÈËÌÃ",
						uident);
					deliverreport(genbuf,
				      "±øÂíÙ¸½ğÈÚÖĞĞÄ¶ÔÆäÒ»Ö±ÒÔÀ´µÄ¹¤×÷±íÊ¾¸ĞĞ»£¬×£ÒÔºóË³Àû£¡");
					mail_buf
					    ("¸ĞĞ»ÄãÎªÁË´ó¸»ÎÌÓÎÏ·µÄ¸¶³ö",
					     uident, genbuf);
					sprintf(genbuf, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ", currentuser.userid);
					sprintf(buf,"%sÊÚÓè%s»ÆÂí¹Ó", currentuser.userid, uident);
					millionairesrec(genbuf, buf, "");

					move(17, 4);
					prints("ÈÎÃü³É¹¦¡£");
					pressanykey();
				}
				break;
		case 'k':
		case 'K':
			clear();
			move(12, 4);
			usercomplete("½â³ıÄÄÎ»£¿", uident);
			move(13, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressanykey();
				break;
			}
			if (!seek_in_file
			    (DIR_MC "mingren", uident)) {
				prints
				    ("¸ÃID²»ÊÇ±øÂíÙ¸ÃûÈË¡£");
				pressanykey();
				break;
			}
			if (askyn("È·¶¨Âğ£¿", NA, NA) == YEA) {
				del_from_file(DIR_MC
					      "mingren",
					      uident);
				sprintf(genbuf,
					"%s ÖØ³ö½­ºşÁË",
					uident);
				deliverreport(genbuf,
			      "½­ºşÓÖÒªÓĞÒ»³¡ÑªÓêĞÈ·çÁË");
				sprintf(genbuf, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ", currentuser.userid);
				sprintf(buf,"%s½â³ı%s»ÆÂí¼×", currentuser.userid, uident);
				millionairesrec(genbuf, buf, "");
				move(14, 4);
				prints("½âÖ°³É¹¦¡£");
				pressanykey();
			}
			break;
		case 'l':
		case 'L':
			clear();
			move(15, 4);
			usercomplete("ÈÎÃüË­Îª±øÂíÙ¸½ğÈÚÖĞĞÄ²èÓÑ£¿", uident);
			move(16, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressanykey();
				break;
			}
			if (seek_in_file
			    (DIR_MC "chayou", uident)) {
				prints("¸ÃIDÒÑ¾­ÊÇ²èÓÑÁË¡£");
				pressanykey();
				break;
			}
			if (askyn("È·¶¨Âğ£¿", NA, NA) == YEA) {
				addtofile(DIR_MC "chayou",
					  uident);
				sprintf(genbuf,
					"¹§Ï²%s³ÉÎª±øÂíÙ¸½ğÈÚÖĞĞÄ²èÓÑ",
					uident);
				deliverreport(genbuf,
			      "´ó¸»ÎÌËæÊ±¹§ºòÄúÀ´ºÈ²è×ö¿Í£¡");
				mail_buf
				    ("´ó¸»ÎÌËæÊ±¹§ºòÄúÀ´ºÈ²è×ö¿Í£¡",
				     uident, genbuf);
				sprintf(genbuf, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ", currentuser.userid);
				sprintf(buf,"%sÈÎÃü%sÎª²èÓÑ", currentuser.userid, uident);
				millionairesrec(genbuf, buf, "");

				move(17, 4);
				prints("ÈÎÃü³É¹¦¡£");
				pressanykey();
			}
			break;
		case 'm':
		case 'M':
			clear();
			move(12, 4);
			usercomplete("½â³ıÄÄÎ»£¿", uident);
			move(13, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressanykey();
				break;
			}
			if (!seek_in_file
			    (DIR_MC "chayou", uident)) {
				prints
				    ("¸ÃID²»ÊÇ±øÂíÙ¸½ğÈÚÖĞĞÄ²èÓÑ¡£");
				pressanykey();
				break;
			}
			if (askyn("È·¶¨Âğ£¿", NA, NA) == YEA) {
				del_from_file(DIR_MC
					      "chayou",
					      uident);
				sprintf(genbuf,
					"%s ÖØ³ö½­ºşÁË",
					uident);
				deliverreport(genbuf,
			      "¸ĞĞ»ÄúÒ»Ö±ÒÔÀ´¶Ô´ó¸»ÎÌµÄ¹Ø×¢¡£");
				sprintf(genbuf, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ", currentuser.userid);
				sprintf(buf,"%sÈ¡Ïû%sµÄ²èÓÑÉí·İ", currentuser.userid, uident);
				millionairesrec(genbuf, buf, "");
				move(14, 4);
				prints("½âÖ°³É¹¦¡£");
				pressanykey();
			}
			break;
	       case 'n':
		case 'N':
			clear();
			move(15, 4);
			usercomplete("ÈÎÃüË­ÎªÌú¹«¼¦£¿", uident);
			move(16, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressanykey();
				break;
			}
			if (seek_in_file
			    (DIR_MC "gongji", uident)) {
				prints("¸ÃIDÒÑ¾­ÊÇÌú¹«¼¦ÁË¡£");
				pressanykey();
				break;
			}
			if (askyn("È·¶¨Âğ£¿", NA, NA) == YEA) {
				addtofile(DIR_MC "gongji",
					  uident);
				sprintf(genbuf,
					"¹§Ï²%s»ñµÃÌú¹«¼¦³ÆºÅ",
					uident);
				 deliverreport(genbuf,
			      "±øÂíÙ¸½ğÈÚÖĞĞÄ¶ÔÆäÒ»Ã«²»°ÎµÄĞĞÎª±íÊ¾½±Àø£¡");
				 //deliverreport(genbuf,
			      //"±øÂíÙ¸½ğÈÚÖĞĞÄ¶ÔÆäÒ»¹áµÄ¼è¿àÆÓËØ£¬ÇÚ¼ó½ÚÔ¼±íÊ¾ÔŞÉÍ£¡");
				 mail_buf
				    ("»ñµÃÌú¹«¼¦³ÆºÅ",
				     uident, genbuf);
				sprintf(genbuf, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ", currentuser.userid);
				sprintf(buf,"%sÈÎÃü%sÎªÌú¹«¼¦", currentuser.userid, uident);
				millionairesrec(genbuf, buf, "");

				move(17, 4);
				prints("ÈÎÃü³É¹¦¡£");
				pressanykey();
			}
			break;
		case 'o':
		case 'O':
			clear();
			move(12, 4);
			usercomplete("½â³ıÄÄÎ»£¿", uident);
			move(13, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressanykey();
				break;
			}
			if (!seek_in_file
			    (DIR_MC "gongji", uident)) {
				prints
				    ("¸ÃID²»ÊÇÌú¹«¼¦¡£");
				pressanykey();
				break;
			}
			if (askyn("È·¶¨Âğ£¿", NA, NA) == YEA) {
				del_from_file(DIR_MC
					      "gongji",
					      uident);
				sprintf(genbuf,
					"%s ¾ö¶¨»¨Ç®ÏúÔÖÁË",
					uident);
				deliverreport(genbuf, "´ÓÌú¹«¼¦ÉíÉÏÄÜÕ¥³öÓÍË®À´¡£À÷º¦À÷º¦");
				sprintf(genbuf, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ", currentuser.userid);
				sprintf(buf,"%sÈ¡Ïû%sµÄÌú¹«¼¦³ÆºÅ", currentuser.userid, uident);
				millionairesrec(genbuf, buf, "");
				move(14, 4);
				prints("½âÖ°³É¹¦¡£");
				pressanykey();
			}
			break;

		case 'y':
		case 'Y':
			clear();
			move(13, 4);
			usercomplete("ÈÎÃüË­Îª»ù½ğid£¿", uident);
			move(14, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressanykey();
				break;
			}
			if (seek_in_file
			    (DIR_MC "jijin", uident)) {
				prints("¸ÃIDÒÑ¾­ÊÇ»ù½ğIDÁË¡£");
				pressanykey();
				break;
			}
			getdata(14, 4, "»ù½ğÃû³Æ£º", buf, 50, DOECHO, YEA);
			sprintf(genbuf, "[¹«¸æ]³ÉÁ¢%s»ù½ğ%s", buf, uident);
			getdata(15, 4, "Ô­Òò£º", buf, 50, DOECHO, YEA);
			sprintf(letter, "³ÉÁ¢Ô­Òò£º%s\nÏ£Íû»ù½ğ¹ÜÀíÕßÖÒÓÚÖ°ÊØ£¬½¨ÉèÁ®½à¸ßĞ§µÄ»ù½ğÌåÏµ¡£", buf);
			move(16, 4);
			if (askyn("È·¶¨Âğ£¿", NA, NA) == NA)
				break;
			addtofile(DIR_MC "jijin",uident);
			if (!seek_in_file(DIR_MC "mingren", uident))
				addtofile(DIR_MC "mingren",uident);
			//»ù½ğidÊÇ¸øÓèÌØÊâµÄ»ÆÂí¹Ó
			 deliverreport(genbuf, letter);
			 mail_buf (letter, uident, genbuf);
			sprintf(genbuf, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ", currentuser.userid);
			sprintf(buf,"%sÈÎÃü%sÎª»ù½ğID", currentuser.userid, uident);
			millionairesrec(genbuf, buf, "");
			move(17, 4);
			prints("ÈÎÃü³É¹¦¡£");
			pressanykey();
			break;

		case 'z':
		case 'Z':
			clear();
			move(12, 4);
			usercomplete("½â³ıÄÄÎ»£¿", uident);
			move(13, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressanykey();
				break;
			}
			if (!seek_in_file(DIR_MC "jijin", uident)) {
				prints
				    ("¸ÃID²»ÊÇ»ù½ğID¡£");
				pressanykey();
				break;
			}
			getdata(15, 4, "Ô­Òò£º", buf, 50, DOECHO, YEA);
			sprintf(letter, "³·ÏúÔ­Òò£º%s", buf);
			move(16, 4);
			if (askyn("È·¶¨Âğ£¿", NA, NA) == NA)
				break;
			del_from_file(DIR_MC"jijin", uident);
			del_from_file(DIR_MC"mingren", uident);
			//Ò»²¢È¡Ïû»ÆÂí¹Ó
			sprintf(genbuf, "[¹«¸æ]³·Ïú»ù½ğ%s", uident);
			 deliverreport(genbuf, letter);
			 mail_buf (letter, uident, genbuf);
			sprintf(genbuf, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ", currentuser.userid);
			sprintf(buf,"%s³·Ïú»ù½ğ%s", currentuser.userid, uident);
			millionairesrec(genbuf, buf, "");
			move(17, 4);
			prints("½â³ı³É¹¦¡£");
			pressanykey();
			break;

		case 'p':
		case 'P':
			clear();
			fp1 = fopen( MC_STOCK_BOARDS, "r" );
			count = listfilecontent(MC_STOCK_BOARDS);
			clear();
			for (j = 0; j < count; j++) {
				fscanf(fp1, "%s", stockboard[j]);
			}
			fclose(fp1);

			move(12, 4);
			if (askyn("È·¶¨Òª³õÊ¼»¯¹ÉÊĞÂğ£¿", NA, NA) == YEA)
			{
				for (i = 0; i < numboards; i++)
					for (j = 0; j < count; j++)
						if (!strcmp(bcache[i].header.filename, stockboard[j]))
						{
//									stock_price[j] = utmpshm->ave_score / 100 + bcache[i].score / 20;
							if (bcache[i].score > 10000)
									bcache[i].stocknum = bcache[i].score * 2000;
								else
									bcache[i].stocknum = bcache[i].score * 1000;
							if (bcache[i].stocknum < 50000)
								bcache[i].stocknum = 50000;
						}
				sprintf(genbuf, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ", currentuser.userid);
				sprintf(buf,"%s³õÊ¼»¯¹ÉÊĞ", currentuser.userid);
				millionairesrec(genbuf, buf, "");
				move(14, 4);
				prints("¹ÉÊĞ³õÊ¼»¯³É¹¦¡£");
				pressanykey();
			}
			break;

		case 'r':
		case 'R':
                        clear();
                        move(1, 0);
                        prints("Ä¿Ç°ÃûÈËÌÃµÄIDÁĞ±í£º");
                        listfilecontent(DIR_MC "mingren");
                        pressanykey();
                        break;

		case 's':
              case 'S':
                        clear();
                        move(1, 0);
                        prints("Ä¿Ç°²èÓÑµÄIDÁĞ±í£º");
                        listfilecontent(DIR_MC "chayou");
                        pressanykey();
                        break;

                case 't':
                case 'T':
                        clear();
                        move(1, 0);
                        prints("Ä¿Ç°Ìú¹«¼¦µÄIDÁĞ±í£º");
                        listfilecontent(DIR_MC "gongji");
                        pressanykey();
                        break;

		  case 'u':
                case 'U':
                        clear();
                        move(1, 0);
                        prints("Ä¿Ç°»ù½ğIDÁĞ±í£º");
                        listfilecontent(DIR_MC "jijin");
                        pressanykey();
                        break;

		case 'X':
		case 'x':
			money_deny();
			break;

		int num=0;
		case '1':
			clear();
			move(12, 4);
			usercomplete("¸ü¸ÄË­µÄÏÖ½ğÊı¶î£¿", uident);
			move(13, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressanykey();
				break;
			}
			prints("%sÄ¿Ç°ÓĞÏÖ½ğ%d±øÂíÙ¸±Ò¡£", uident,
				loadValue(uident, MONEY_NAME, MAX_MONEY_NUM));
			getdata(14, 4, "¸ÄÎª¶àÉÙ?", genbuf, 10, DOECHO, YEA);
			num = atoi(genbuf);
			sprintf(buf, "È·¶¨Òª¸ÄÎª%dÂğ£¿", num);
			move(15, 4);
			if (askyn(buf, NA, NA) == YEA) {
				saveValue(uident ,MONEY_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
				saveValue(uident ,MONEY_NAME, num, MAX_MONEY_NUM);
				sprintf(genbuf, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ", currentuser.userid);
				sprintf(buf,"¸ü¸Ä%sÏÖ½ğÊı¶îÎª%d", uident, num);
				millionairesrec(genbuf, buf, "");
				move(17, 4);
				prints("ĞŞ¸Ä³É¹¦¡£");
				pressanykey();
			}
			break;
		case '2':
			clear();
			move(12, 4);
			usercomplete("¸ü¸ÄË­µÄ´æ¿îÊı¶î£¿", uident);
			move(13, 4);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressanykey();
				break;
			}
			prints("%sÄ¿Ç°ÓĞ´æ¿î%d±øÂíÙ¸±Ò¡£", uident,
				loadValue(uident, CREDIT_NAME, MAX_MONEY_NUM));
			getdata(14, 4, "¸ÄÎª¶àÉÙ?", genbuf, 10, DOECHO, YEA);
			num = atoi(genbuf);
			sprintf(buf, "È·¶¨Òª¸ÄÎª%dÂğ£¿", num);
			move(15, 4);
			if (askyn(buf, NA, NA) == YEA) {
				saveValue(uident, CREDIT_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
				saveValue(uident, CREDIT_NAME, num, MAX_MONEY_NUM);
				sprintf(genbuf, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ", currentuser.userid);
				sprintf(buf,"¸ü¸Ä%s´æ¿îÊı¶îÎª%d", uident, num);
				millionairesrec(genbuf, buf, "");
				move(17, 4);
				prints("ĞŞ¸Ä³É¹¦¡£");
				pressanykey();
			}
			break;

		case '0':
			clear();
			move(6, 4);
			sprintf(buf, "È·¶¨Òª%s½ğÈÚÖĞĞÄÂğ£¿",
				(utmpshm->mc.isMCclosed)?"¿ªÆô":"¹Ø±Õ");
			if (askyn(buf, NA, NA) == YEA)
				utmpshm->mc.isMCclosed = (utmpshm->mc.isMCclosed)?0:1;
			move(9, 4);
			prints("ĞŞ¸Ä³É¹¦¡£");
			pressanykey();
			break;

		case 'i':
		case 'I':
			addOrDel_contrb();
			break;

		case 'Q':
		case 'q':
			quit = 1;
			break;
		}
	}
	return 0;
}

static int//²ÊÆ±36Ñ¡7
valid367Bet(char *buf)
{
	int i, j;
	int temp[7];
	int slot = 0;

	if (strlen(buf) != 20) {	/*  ³¤¶È±ØĞëÎª20 */
		return 0;
	}
	for (i = 0; i < 20; i++) {	/*  »ù±¾¸ñÊ½±ØĞëÕıÈ·   */
		if ((i % 3 == 2) && buf[i] != '-') {
			return 0;
		}
		if ((i % 3 != 2) && !(buf[i] >= '0' && buf[i] <= '9')) {
			return 0;
		}
		if (i % 3 == 0) {
			temp[slot] = (buf[i] - '0') * 10 + (buf[i + 1] - '0');
			if (temp[slot] > 36) {
				return 0;
			}
			slot++;
		}
	}
	for (i = 0; i < 7; i++) {	/* Êı×ÖÎŞÖØ¸´ */
		for (j = 0; j < 7; j++) {
			if (temp[j] == temp[i] && i != j) {
				return 0;
			}
		}
	}
	return 1;
}

static int//²ÊÆ±36Ñ¡7
make367Prize(char *bet, char *prizeSeq)
{
	int count = 0;
	int i, j;
	int len = strlen(bet);

	if (strlen(bet) != strlen(prizeSeq)) {
		return 0;
	}
	for (i = 0; i + 1 < len; i = i + 3) {
		for (j = 0; j + 1 < len; j = j + 3) {
			if (bet[i] == prizeSeq[j]
			    && bet[i + 1] == prizeSeq[j + 1]) {
				count++;
			}
		}
	}
	return count;
}

static void//²ÊÆ±36Ñ¡7
make367Seq(char *prizeSeq)
{
	int i, j;
	int num;
	int temp[7];
	int slot = 0;
	int success;

	srandom(time(0));
	for (i = 0; i < 7; i++) {
		do {		/*  Êı×Ö²»ÄÜÏàÍ¬  */
			success = 1;
			num = 1 + random() % 36;
			for (j = 0; j <= slot; j++) {
				if (num == temp[j]) {
					success = 0;
					break;
				}
			}
			if (success) {
				temp[slot++] = num;
			}
		} while (!success);
		prizeSeq[3 * i] = (char) (num / 10 + '0');
		prizeSeq[3 * i + 1] = (char) (num % 10 + '0');
		if (i != 6) {
			prizeSeq[3 * i + 2] = '-';
		} else {
			prizeSeq[3 * i + 2] = '\0';
		}
	}

	sprintf(genbuf, "ĞòÁĞÊÇ£º  %s  ¡£ÄúÖĞ½±ÁËÂğ£¿", prizeSeq);
	deliverreport("[¹«¸æ]±¾ÆÚ36Ñ¡7²ÊÆ±Ò¡½±½á¹û", genbuf);
}

static int/*²ÊÆ±26Ñ¡7 */
open_36_7(void)
{
	FILE *fp;
	char line[MAX_RECORD_LINE];
	char prizeSeq[MAX_BET_LENGTH];
	char *bet;
	char *userid;
	int prizeType;
	int totalMoney, remainMoney;
	int num_bp = 0, num_1p = 0, num_2p = 0, num_3p = 0, num_cp = 0;

	make367Seq(prizeSeq);	//²úÉúĞòÁĞ

	fp = fopen(DIR_MC_TEMP "36_7_list", "r");
	if (!fp) {
		return -1;
	}
	while (fgets(line, MAX_RECORD_LINE, fp)) {
		userid = strtok(line, " ");
		bet = strtok(NULL, "\n");
		if (!userid || !bet) {
			continue;
		}
		/*   ---------------------¼ÆËã½±Àø----------------------- */
		prizeType = make367Prize(bet, prizeSeq);
		switch (prizeType) {
		case 7:
			addtofile(DIR_MC_TEMP "36_7_bp", userid);
			num_bp++;
			break;
		case 6:
			addtofile(DIR_MC_TEMP "36_7_1p", userid);
			num_1p++;
			break;
		case 5:
			addtofile(DIR_MC_TEMP "36_7_2p", userid);
			num_2p++;
			break;
		case 4:
			addtofile(DIR_MC_TEMP "36_7_3p", userid);
			num_3p++;
			break;
		case 3:
			addtofile(DIR_MC_TEMP "36_7_cp", userid);
			num_cp++;
			break;
		default:
			break;
		}
	}			/* end of while */
	fclose(fp);

	/*  ------------------------ ·¢½± --------------------- */
	totalMoney = utmpshm->mc.prize367 + PRIZE_PER;
	remainMoney = totalMoney;
	if (num_bp > 0) {
		int per_bp = (BIG_PRIZE * totalMoney) / num_bp;
		char buf[1024];
		char title[80];

		remainMoney -= BIG_PRIZE * totalMoney;

		fp = fopen(DIR_MC_TEMP "36_7_bp", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_bp, MAX_MONEY_NUM);
			sprintf(genbuf,
				"ÄúµÃµ½ÁË %d ±øÂíÙ¸±ÒµÄ½±½ğ¡£¹§Ï²£¡Ï£ÍûÏÂ´Î»¹ÓĞºÃÔË¡«¡«¡«",
				per_bp);
			mail_buf(genbuf, userid, "¹§Ï²Äú»ñµÃ36Ñ¡7ÌØµÈ½±£¡");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 1024-1, fp);
		sprintf(title, "±¾ÆÚ36Ñ¡7ÌØµÈ½±Ãûµ¥£¨Ã¿×¢½±½ğ%d±øÂíÙ¸±Ò£©",
			per_bp);
		deliverreport(title, buf);
		fclose(fp);
	}

	if (num_1p > 0) {
		int per_1p = (I_PRIZE * totalMoney) / num_1p;
		char buf[1024];
		char title[80];

		remainMoney -= I_PRIZE * totalMoney;

		fp = fopen(DIR_MC_TEMP "36_7_1p", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_1p, MAX_MONEY_NUM);
			sprintf(genbuf,
				"ÄúµÃµ½ÁË %d ±øÂíÙ¸±ÒµÄ½±½ğ¡£¹§Ï²£¡Ï£ÍûÏÂ´Î»¹ÓĞºÃÔË¡«¡«¡«",
				per_1p);
			mail_buf(genbuf, userid, "¹§Ï²Äú»ñµÃ36Ñ¡7Ò»µÈ½±£¡");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 1024-1, fp);
		sprintf(title, "±¾ÆÚ36Ñ¡7Ò»µÈ½±Ãûµ¥£¨Ã¿×¢½±½ğ%d±øÂíÙ¸±Ò£©",
			per_1p);
		deliverreport(title, buf);
		fclose(fp);
	}

	if (num_2p > 0) {
		int per_2p = (II_PRIZE * totalMoney) / num_2p;
		char buf[1024];
		char title[80];

		remainMoney -= II_PRIZE * totalMoney;

		fp = fopen(DIR_MC_TEMP "36_7_2p", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_2p, MAX_MONEY_NUM);
			sprintf(genbuf,
				"ÄúµÃµ½ÁË %d ±øÂíÙ¸±ÒµÄ½±½ğ¡£¹§Ï²£¡Ï£ÍûÏÂ´Î»¹ÓĞºÃÔË¡«¡«¡«",
				per_2p);
			mail_buf(genbuf, userid, "¹§Ï²Äú»ñµÃ36Ñ¡7¶şµÈ½±£¡");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 1024-1, fp);
		sprintf(title, "±¾ÆÚ36Ñ¡7¶şµÈ½±Ãûµ¥£¨Ã¿×¢½±½ğ%d±øÂíÙ¸±Ò£©",
			per_2p);
		deliverreport(title, buf);
		fclose(fp);
	}
	if (num_3p > 0) {
		int per_3p = (III_PRIZE * totalMoney) / num_3p;
		char buf[2048];
		char title[80];

		remainMoney -= III_PRIZE * totalMoney;

		fp = fopen(DIR_MC_TEMP "36_7_3p", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_3p, MAX_MONEY_NUM);
			sprintf(genbuf,
				"ÄúµÃµ½ÁË %d ±øÂíÙ¸±ÒµÄ½±½ğ¡£¹§Ï²£¡Ï£ÍûÏÂ´Î»¹ÓĞºÃÔË¡«¡«¡«",
				per_3p);
			mail_buf(genbuf, userid, "¹§Ï²Äú»ñµÃ36Ñ¡7ÈıµÈ½±£¡");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 2048-1, fp);
		sprintf(title, "±¾ÆÚ36Ñ¡7ÈıµÈ½±Ãûµ¥£¨Ã¿×¢½±½ğ%d±øÂíÙ¸±Ò£©",
			per_3p);
		deliverreport(title, buf);
		fclose(fp);

	}
	if (num_cp > 0) {
		int per_cp = CMFT_PRIZE;
		char buf[2048];
		char title[80];

		remainMoney -= CMFT_PRIZE * num_cp;

		fp = fopen(DIR_MC_TEMP "36_7_cp", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_cp, MAX_MONEY_NUM);
			sprintf(genbuf,
				"ÄúµÃµ½ÁË %d ±øÂíÙ¸±ÒµÄ½±½ğ¡£¹§Ï²£¡Ï£ÍûÏÂ´Î»¹ÓĞºÃÔË¡«¡«¡«",
				per_cp);
			mail_buf(genbuf, userid, "¹§Ï²Äú»ñµÃ36Ñ¡7°²Î¿½±£¡");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 2048-1, fp);
		sprintf(title, "±¾ÆÚ36Ñ¡7°²Î¿½±Ãûµ¥£¨Ã¿×¢½±½ğ%d±øÂíÙ¸±Ò£©",
			CMFT_PRIZE);
		deliverreport(title, buf);
		fclose(fp);
	}
	remainMoney = limitValue(remainMoney, MAX_POOL_MONEY);
	utmpshm->mc.prize367 = remainMoney;
	remove(DIR_MC_TEMP "36_7_list");
	remove(DIR_MC_TEMP "36_7_bp");
	remove(DIR_MC_TEMP "36_7_1p");
	remove(DIR_MC_TEMP "36_7_2p");
	remove(DIR_MC_TEMP "36_7_3p");
	remove(DIR_MC_TEMP "36_7_cp");
	remove(DIR_MC_TEMP "36_7_start");
	return 0;
}

static int /*²ÊÆ±--×ã²Ê*/
computeSum(char *complexBet)
{				/*¼ÆËã¸´Ê½×¢µÄÊıÁ¿ */
	int i;
	int len;
	int countNum = 0;
	int total = 1;

	len = strlen(complexBet);

	for (i = 0; i < len; i++) {
		if (complexBet[i] == '-') {
			total *= countNum;
			countNum = 0;
		} else {
			countNum++;
		}
	}
	total *= countNum;	/*×îºóÒ»¸öµ¥Ôª */
	return total;
}

static void/*²ÊÆ±--×ã²Ê*/
saveSoccerRecord(char *complexBet)
{				/*±£´æ¸´Ê½×¢Îªµ¥×¢ */
	int i, j;
	int len;
	int simple = 1;
	int meet = 0;
	int count = 0;
	int firstDivEnd, firstDivStart;
	char buf[100];

	len = strlen(complexBet);
	firstDivEnd = len;

	for (i = 0; 2 * i + 1 < len; i++) {
		if (complexBet[2 * i + 1] != '-') {
			simple = 0;
			break;
		}
	}
	if (simple) {		/*¼òµ¥±ê×¼ĞÎÊ½£¬Ö±½Ó´òÓ¡ */
		for (i = 0, j = 0; i < len; i++) {
			if (complexBet[i] != '-') {
				genbuf[j++] = complexBet[i];
			}
		}
		genbuf[j] = '\0';
		sprintf(buf, "%s %s", currentuser.userid, genbuf);
		addtofile(DIR_MC_TEMP "soccer_list", buf);
	} else {
		for (i = 0; i < len; i++) {	/*Ñ°ÕÒµÚÒ»¸ö¸´Ê½µ¥Ôª */
			if (complexBet[i] == '-') {
				if (count > 1 && !meet) {
					firstDivEnd = i;
					break;
				} else {
					count = 0;
				}
			} else {
				count++;
			}
		}
		firstDivStart = firstDivEnd - count;
		firstDivEnd--;

		for (i = 0; i < count; i++) {	/*¶ÔÃ¿Ò»¸öÒª²ğ·ÖµÄµ¥ÔªµÄÔªËØ */
			int slot = 0;
			char *temp = malloc(len * sizeof (char));

			/*µÃµ½Ç°ÃæµÄ²¿·Ö */
			if (firstDivStart != 0) {
				for (j = 0; j < firstDivStart; j++, slot++) {
					temp[slot] = complexBet[j];
				}
			}
			temp[slot] = complexBet[firstDivStart + i];
			slot++;
			/*µÃµ½ºóÃæµÄ²¿·Ö */
			for (j = firstDivEnd + 1; j < len; j++, slot++) {
				temp[slot] = complexBet[j];
			}
			temp[slot] = '\0';

			/*¶ÔÃ¿Ò»¸ö²ğ·Ö£¬½øĞĞµİ¹éµ÷ÓÃ */
			saveSoccerRecord(temp);
		}

	}
}

static int /*²ÊÆ±--×ã²Ê*/
validSoccerBet(char *buf)
{
	int count = 0;
	int meetSeperator = 1;
	int i;
	int first = 0, second = 0;

	if (strlen(buf) == 0) {
		return 0;
	}
	for (i = 0; i < strlen(buf); i++) {
		if (buf[i] == '-') {
			if (meetSeperator == 1) {	/*Èç¹ûÁ¬ĞøÓöµ½-£¬¿Ï¶¨²»ÕıÈ· */
				return 0;
			}
			count = 0;
			meetSeperator = 1;
		} else {
			if (buf[i] != '3' && buf[i] != '1' && buf[i] != '0') {	/*²»ÊÇ310£¬¿Ï¶¨²»¶Ô */
				return 0;
			}
			count++;
			if (count > 3) {
				return 0;
			}
			if (count == 1) {
				first = buf[i];
			} else if (count == 2) {
				if (buf[i] == first) {	/*ÖØºÏ */
					return 0;
				}
				second = buf[i];
			} else if (count == 3) {
				if (buf[i] == first || buf[i] == second) {	/*ÖØºÏ */
					return 0;
				}
			}
			meetSeperator = 0;
		}
	}
	if (buf[strlen(buf) - 1] == '-') {
		return 0;
	}
	return 1;
}

static int /*²ÊÆ±--×ã²Ê*/
makeSoccerPrize(char *bet, char *prizeSeq)
{
	int diff = 0;
	int i;
	int n1 = strlen(bet);
	int n2 = strlen(prizeSeq);

	if (n1 != n2) {
		return 10;	/*²»ÖĞ½± */
	}
	for (i = 0; i < n1; i++) {
		if (bet[i] != prizeSeq[i]) {
			diff++;
		}
	}
	return diff;
}

static int /*²ÊÆ±--×ã²Ê*/
open_soccer(char *prizeSeq)
{
	FILE *fp;
	char line[MAX_RECORD_LINE];
	char *bet;
	char *userid;
	int prizeType;
	int totalMoney, remainMoney;
	int num_bp = 0, num_1p = 0, num_2p = 0, num_3p = 0, num_cp = 0;

	fp = fopen(DIR_MC_TEMP "soccer_list", "r");

	if (!fp) {
		return -1;
	}
	sprintf(genbuf, "ĞòÁĞÊÇ£º%s¡£ÄúÖĞ½±ÁËÂğ£¿", prizeSeq);
	deliverreport("[¹«¸æ]±¾ÆÚ×ã²Ê½á¹û", genbuf);
	while (fgets(line, MAX_RECORD_LINE, fp)) {
		userid = strtok(line, " ");
		bet = strtok(NULL, "\n");
		if (!userid || !bet) {
			continue;
		}
		/*   ---------------------¼ÆËã½±Àø----------------------- */
		prizeType = makeSoccerPrize(bet, prizeSeq);
		switch (prizeType) {
		case 0:	/*ÍêÈ«ÏàÍ¬ */
			addtofile(DIR_MC_TEMP "soccer_bp", userid);
			num_bp++;
			break;
		case 1:	/*ÓĞÒ»¸ö²»Í¬ */
			addtofile(DIR_MC_TEMP "soccer_1p", userid);
			num_1p++;
			break;
		case 2:	/*ÓĞ¶ş¸ö²»Í¬ */
			addtofile(DIR_MC_TEMP "soccer_2p", userid);
			num_2p++;
			break;
		case 3:	/*ÓĞÈı¸ö²»Í¬ */
			addtofile(DIR_MC_TEMP "soccer_3p", userid);
			num_3p++;
			break;
		case 4:	/*ÓĞËÄ¸ö²»Í¬ */
			addtofile(DIR_MC_TEMP "soccer_cp", userid);
			num_cp++;
			break;
		default:
			break;
		}
	}			/* end of while */
	fclose(fp);
	/*  ------------------------ ·¢½± --------------------- */
	totalMoney = utmpshm->mc.prizeSoccer + PRIZE_PER;
	remainMoney = totalMoney;
	if (num_bp > 0) {
		int per_bp = (BIG_PRIZE * totalMoney) / num_bp;
		char buf[1024];
		char title[80];

		remainMoney -= BIG_PRIZE * totalMoney;

		fp = fopen(DIR_MC_TEMP "soccer_bp", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_bp, MAX_MONEY_NUM);
			sprintf(genbuf,
				"ÄúµÃµ½ÁË %d ±øÂíÙ¸±ÒµÄ½±½ğ¡£¹§Ï²£¡Ï£ÍûÏÂ´Î»¹ÓĞºÃÔË¡«¡«¡«",
				per_bp);
			mail_buf(genbuf, userid, "¹§Ï²Äú»ñµÃ×ãÇò²ÊÆ±ÌØµÈ½±£¡");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 1024-1, fp);
		sprintf(title, "±¾ÆÚ×ã²ÊÌØµÈ½±Ãûµ¥£¨Ã¿×¢½±½ğ%d±øÂíÙ¸±Ò£©",
			per_bp);
		deliverreport(title, buf);
		fclose(fp);
	}

	if (num_1p > 0) {
		int per_1p = (I_PRIZE * totalMoney) / num_1p;
		char buf[1024];
		char title[80];

		remainMoney -= I_PRIZE * totalMoney;

		fp = fopen(DIR_MC_TEMP "soccer_1p", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_1p, MAX_MONEY_NUM);
			sprintf(genbuf,
				"ÄúµÃµ½ÁË %d ±øÂíÙ¸±ÒµÄ½±½ğ¡£¹§Ï²£¡Ï£ÍûÏÂ´Î»¹ÓĞºÃÔË¡«¡«¡«",
				per_1p);
			mail_buf(genbuf, userid, "¹§Ï²Äú»ñµÃ×ãÇò²ÊÆ±Ò»µÈ½±£¡");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 1024-1, fp);
		sprintf(title, "±¾ÆÚ×ã²ÊÒ»µÈ½±Ãûµ¥£¨Ã¿×¢½±½ğ%d±øÂíÙ¸±Ò£©",
			per_1p);
		deliverreport(title, buf);
		fclose(fp);
	}

	if (num_2p > 0) {
		int per_2p = (II_PRIZE * totalMoney) / num_2p;
		char buf[2048];
		char title[80];

		remainMoney -= II_PRIZE * totalMoney;

		fp = fopen(DIR_MC_TEMP "soccer_2p", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_2p, MAX_MONEY_NUM);
			sprintf(genbuf,
				"ÄúµÃµ½ÁË %d ±øÂíÙ¸±ÒµÄ½±½ğ¡£¹§Ï²£¡Ï£ÍûÏÂ´Î»¹ÓĞºÃÔË¡«¡«¡«",
				per_2p);
			mail_buf(genbuf, userid, "¹§Ï²Äú»ñµÃ×ãÇò²ÊÆ±¶şµÈ½±£¡");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 2048-1, fp);
		sprintf(title, "±¾ÆÚ×ã²Ê¶şµÈ½±Ãûµ¥£¨Ã¿×¢½±½ğ%d±øÂíÙ¸±Ò£©",
			per_2p);
		deliverreport(title, buf);
		fclose(fp);
	}
	/*if (num_3p > 0) {
		int per_3p = (III_PRIZE * totalMoney) / num_3p;
		char buf[2048];
		char title[80];

		remainMoney -= III_PRIZE * totalMoney;

		fp = fopen(DIR_MC_TEMP "soccer_3p", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_3p, MAX_MONEY_NUM);
			sprintf(genbuf,
				"ÄúµÃµ½ÁË %d ±øÂíÙ¸±ÒµÄ½±½ğ¡£¹§Ï²£¡Ï£ÍûÏÂ´Î»¹ÓĞºÃÔË¡«¡«¡«",
				per_3p);
			mail_buf(genbuf, userid, "¹§Ï²Äú»ñµÃ×ãÇò²ÊÆ±ÈıµÈ½±£¡");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 2048, fp);
		sprintf(title, "±¾ÆÚ×ã²ÊÈıµÈ½±Ãûµ¥£¨Ã¿×¢½±½ğ%d±øÂíÙ¸±Ò£©",
			per_3p);
		deliverreport(title, buf);
		fclose(fp);

	}*/
	if (num_cp > 0) {
		int per_cp = CMFT_PRIZE;
		char buf[2048];
		char title[80];

		remainMoney -= CMFT_PRIZE * num_cp;

		fp = fopen(DIR_MC_TEMP "soccer_cp", "r");
		if (!fp) {
			return -1;
		}
		while (fgets(line, MAX_RECORD_LINE, fp)) {
			userid = strtok(line, "\n");
			if (!userid) {
				continue;
			}
			saveValue(userid, MONEY_NAME, per_cp, MAX_MONEY_NUM);
			sprintf(genbuf,
				"ÄúµÃµ½ÁË %d ±øÂíÙ¸±ÒµÄ½±½ğ¡£¹§Ï²£¡Ï£ÍûÏÂ´Î»¹ÓĞºÃÔË¡«¡«¡«",
				per_cp);
			mail_buf(genbuf, userid, "¹§Ï²Äú»ñµÃ×ãÇò²ÊÆ±°²Î¿½±£¡");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 2048-1, fp);
		sprintf(title, "±¾ÆÚ×ã²Ê°²Î¿½±Ãûµ¥£¨Ã¿×¢½±½ğ%d±øÂíÙ¸±Ò£©",
			CMFT_PRIZE);
		deliverreport(title, buf);
		fclose(fp);
	}
	remainMoney = limitValue(remainMoney, MAX_POOL_MONEY);
	utmpshm->mc.prizeSoccer = remainMoney;
	remove(DIR_MC_TEMP "soccer_list");
	remove(DIR_MC_TEMP "soccer_bp");
	remove(DIR_MC_TEMP "soccer_1p");
	remove(DIR_MC_TEMP "soccer_2p");
	remove(DIR_MC_TEMP "soccer_3p");
	remove(DIR_MC_TEMP "soccer_cp");
	remove(DIR_MC_TEMP "soccer_start");
	return 0;
}


static int/*ÉÌ³¡--±£ïÚ*/
money_check_guard()
{
	int money, guard;
	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	guard = loadValue(currentuser.userid, "guard", 8);
	if (guard > 0) {
		saveValue(currentuser.userid, "guard", -guard, 50);
		move(9, 4);
		if (random() % 2 == 0) {
			prints("ÄãµÄ±£ïÚÀëÄã¶øÈ¥,²¢Ë³ÊÖÄÃÁËÄãÁ½³ÉµÄÏÖ½ğ.");
			saveValue(currentuser.userid, MONEY_NAME, -money / 5,
				  MAX_MONEY_NUM);
		} else {
			prints
			    ("ÄãµÄ±£ïÚÒ»°ô×ÓÇÃÔÎÁËÄã,ÄÃ×ßÁËÄãÉíÉÏÒ»°ëµÄÇ®£¬ÅÜÂ·ÁË¡£");
			saveValue(currentuser.userid, MONEY_NAME, -money / 2,
				  MAX_MONEY_NUM);
			pressanykey();
			Q_Goodbye();
		}
		return 1;
	}
	return 0;
}

static int /*¶Ä²©--÷»±¦*/
money_dice()
{
	int quit = 0;
	int ch, num = 0, money;
	int target;
	int t1, t2, t3;
	int win;
	int isVIP;
	char slow[IDLEN + 1];
	char title[STRLEN], buf[256];

	isVIP = seek_in_file(DIR_MC "gamble_VIP", currentuser.userid);
       //isVIP=1;
	while (!quit) {
		clear();
		if (isVIP) {
			money_show_stat("±øÂíÙ¸¶Ä³¡÷»±¦ÌüVIPÊÒ");
		} else {
			money_show_stat("±øÂíÙ¸¶Ä³¡÷»±¦Ìü");
		}
		move(4, 4);
		prints
		    ("\033[1;31m¶àÂò¶à×¬£¬ÉÙÂòÉÙÅâ£¬Âò¶¨ÀëÊÖ£¬Ô¸¶Ä·şÊä\033[m");
		move(5, 4);
		prints("·Ö´óĞ¡Á½ÃÅ£¬4-10µãÊÇĞ¡£¬11-17µãÎª´ó¡£");
		move(6, 4);
		prints("ÈôÑºĞ¡¿ªĞ¡£¬¿ÉÄÃÒ»±¶²Ê½ğ£¬Ñº´óµÄ¾ÍÈ«¹é×¯¼Ò¡£");
		move(7, 4);
		prints("×¯¼ÒÒªÊÇÒ¡³öÈ«÷»£¨Èı¸ö÷»×ÓµãÊıÒ»Ñù£©ÔòÍ¨³Ô´óĞ¡¼Ò¡£");
		move(8, 4);
		if (isVIP) {
			prints("×îĞ¡Ñ¹ 100000±øÂíÙ¸±Ò,ÉÏÏŞ 10000000 ±øÂíÙ¸±Ò¡£");
		} else {
			prints
			    ("×îĞ¡Ñ¹ 1000 ±øÂíÙ¸±Ò,ÉÏÏŞ 500000 ±øÂíÙ¸±Ò¡£ÒªÍæ´óµÄÇë½øVIPÊÒ¡£");
		}
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]ÏÂ×¢ [Q]Àë¿ª                                                   \033[m");
		win = 0;
		ch = igetkey();
		switch (ch) {
		case '1':
			if (isVIP) {
				getdata(9, 4, "ÄúÑ¹¶àÉÙ±øÂíÙ¸±Ò£¿[100000]", genbuf,
					9, DOECHO, YEA);
			} else {
				getdata(9, 4, "ÄúÑ¹¶àÉÙ±øÂíÙ¸±Ò£¿[1000]", genbuf,
					7, DOECHO, YEA);
			}
			num = atoi(genbuf);
			if (!genbuf[0]){
				if (!isVIP)
					num = 1000;
				else
					num = 100000;
			}
			if (isVIP && num < 100000) {
				move(11, 4);
				prints("ÕâÀïÊÇVIPÊÒ£¬Ñ¹×¢ÓĞÏÂÏŞ¡£");
				pressanykey();
				break;
			}
			if (!isVIP && num > 500000) {
				move(11, 4);
				prints("ÒªÍæµÄ´óµÄ£¬Çë½øVIPÊÒ¡£");
				pressanykey();
				break;
			}
			if (num < 1000) {
				move(11, 4);
				prints("ÓĞÃ»ÓĞÇ®°¡£¿ÄÇÃ´µãÇ®ÎÒÃÇ²»´øÍæµÄ¡£");
				pressanykey();
				break;
			}
			if (num > 10000000) {
				move(11,4);
				prints("³¬¹ıÁË×î´ó¶Ä×¢£¬ÇëÖØĞÂÏÂ×¢¡£");
				pressanykey();
				break;
			}
			getdata(10, 4, "ÄúÑ¹´ó(L)»¹ÊÇĞ¡(S)£¿[L]", genbuf, 3,
				DOECHO, YEA);
			if (genbuf[0] == 'S' || genbuf[0] == 's')
				target = 1;
			else
				target = 0;
			sprintf(genbuf,
				"Âò¶¨ÀëÊÖ£¬ÄúÂòÁË \033[1;31m%d\033[m ±øÂíÙ¸±ÒµÄ \033[1;31m%s\033[m£¬È·¶¨Ã´£¿",
				num, target ? "Ğ¡" : "´ó");
			move(11, 4);
			if (askyn(genbuf, YEA, NA) == YEA) {
				money =
				    loadValue(currentuser.userid, MONEY_NAME,
					      MAX_MONEY_NUM);
				if (money < num) {
					move(12, 4);
					prints("È¥È¥È¥£¬Ã»ÄÇÃ´¶àÇ®µ·Ê²Ã´ÂÒ£¡      \n");
					pressanykey();
					break;
				}
				//srandom(time(0));
				t1 = random() % 6 + 1;
				t2 = random() % 6 + 1;
				t3 = random() % 6 + 1;
				move(12, 4);
				if ((t1 == t2) && (t2 == t3)) {
					if (num > 2000000)
						utmpshm->mc.prize777 += 1000000;
					else
						utmpshm->mc.prize777 += num * 50 / 100;
					if (utmpshm->mc.prize777 > MAX_MONEY_NUM)
						utmpshm->mc.prize777 = MAX_MONEY_NUM;
					sprintf(genbuf, "\033[1;32m×¯¼ÒÍ¨É±£¡\033[m");
				} else if (t1 + t2 + t3 < 11) {
					sprintf(genbuf,
						"%d µã£¬\033[1;32mĞ¡\033[m",
						t1 + t2 + t3);
					if (target == 1)
						win = 1;
				} else if (t1 + t2 + t3 > 10) {
					sprintf(genbuf,
						"%d µã£¬\033[1;32m´ó\033[m",
						t1 + t2 + t3);
					if (target == 0)
						win = 1;
				}
				prints("¿ªÁË¿ªÁË£¬%d %d %d£¬%s", t1, t2, t3, genbuf);
				move(13, 4);
				if (win) {
					prints("¹§Ï²Äú£¬ÔÙÀ´Ò»°Ñ°É£¡");
					saveValue(currentuser.userid,
						  MONEY_NAME, num,
						  MAX_MONEY_NUM);
					whoTakeCharge(3, slow);//slowaction
                     		saveValue(slow,
						  MONEY_NAME, -num,
						  MAX_MONEY_NUM);

					if (num >= RUMOR_MONEY && random() % 2) {
						int rumor = makeRumor(num);
						sprintf(genbuf,
							"ÓĞÈËÄ¿»÷ %s ÔÚ±øÂíÙ¸¶Ä³¡Ò»°ÑÓ®ÁË %d µÄ±øÂíÙ¸±Ò£¡",
							currentuser.userid,
							rumor);
						deliverreport
						    ("[Ò¥ÑÔ]À´×Ô±øÂíÙ¸¶Ä³¡µÄÏûÏ¢", genbuf);
					}
					sprintf(title, "%s²ÎÓë¶Ä²©(÷»±¦)(Ó®)", currentuser.userid);
					sprintf(buf, "%sÔÚ÷»±¦Ó®ÁË%d±øÂíÙ¸±Ò", currentuser.userid, num);
					millionairesrec(title, buf, "¶Ä²©÷»±¦");
				} else {
					prints("Ã»ÓĞ¹ØÏµ£¬ÏÈÊäºóÓ®...");
					saveValue(currentuser.userid,
						  MONEY_NAME, -num,
						  MAX_MONEY_NUM);
					whoTakeCharge(3, slow);//slowaction
                    			saveValue(slow,
						MONEY_NAME, +num,
						MAX_MONEY_NUM);
					sprintf(title, "%s²ÎÓë¶Ä²©(÷»±¦)(Êä)", currentuser.userid);
					sprintf(buf, "%sÔÚ÷»±¦ÊäÁË%d±øÂíÙ¸±Ò", currentuser.userid, num);
					millionairesrec(title, buf, "¶Ä²©÷»±¦");
				}
				pressanykey();
			}
			break;
		case 'Q':
		case 'q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

static int /*ºÚ°ï*/
money_robber()
{
	int quit = 0, guard_num = 0;
	int ch, x, y, z, ch2;
	int num, money, r, ra, id, count = 0, rob,credit;
	int freeTime;
	int zhuannum=20;
	int currentTime = time(0);
	char uident[IDLEN + 1], buf[200], title[40];
	double mathtmp;
	srandom(time(0));
	char letter1[] = "ÏŞÄã°ëĞ¡Ê±ÄÚ¸øÎÒ¼ÄÇ®£¬²»È»ÓĞÄãºÃ¿´£¡\n";
	char letter2[] =
	    "¿ì¸øÎÒ¼ÄÇ®£¬·ñÔòĞ¡ĞÄÄãµÄÄÔ´ü°¤°å×©¡£\nÎÒ»á¼Ç¹Ò×ÅÄãµÄ°²È«µÄ£¬ºÙºÙ...";
	char letter3[] = "¿ì¸øÎÒ¼ÄÇ®£¬·ñÔòĞ¡ĞÄÎÒ°ÑÄãµÄÇ®È«²¿ÇÀ×ß£¡";
	while (!quit) {
		clear();
		money_show_stat("±³ÒõÏï");
		move(4, 4);
		prints
		    ("Á½ÄêÇ°µÄ±øÂíÙ¸ºÚ°ïÎŞ¶ñ²»×÷£¬ÃûÔëÒ»Ê±£¬²»¹ı×î½ü¾¯²ìÑÏ´ò£¬»î¶¯ÓĞËùÊÕÁ²¡£");
		move(5, 4);
		prints("Ò»¸öºÚÒÂÈËĞ¡ÉùËµ£º¡°Òª°å×©Ã´£¿ÅÄÈËºÜÌÛµÄ¡£¡±");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]ÅÄ×© [2]ÍµÇÔ [3]ÀÕË÷ [4]ÇÀÈË [5]ºÚ°ï°ïÖ÷ [Q]Àë¿ª\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			clear();

		       if(!Allclubtest(currentuser.userid)){
			   	move(5, 4);
				prints("    \033[1;32m  ÆÕÍ¨ÊĞÃñ²»ÒªÈÇÊÂ\033[m");
				pressanykey();
				break;
			}
			if (seek_in_file(DIR_MC "chayou", currentuser.userid)){
				move(5, 4);
				prints("    \033[1;32m  ²èÓÑ²»ÒªÈÇÊÂ\033[m");
				pressanykey();
				break;
			}
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)) {
				move(5, 4);
				prints("    \033[1;32m  ²»ÒªÈÇÊÂ\033[m");
				pressanykey();
				break;
			}
			r = random() % 40;
			if (r < 1)
				money_police();
			money_show_stat("ºÚ°ï°å×©Éú²ú»ùµØ");
			move(4, 4);
			prints("ÕâÀïµÄ°å×©ÖÊµØÓÅÁ¼£¬ÄÃÈ¥ÅÄÈËÒ»¶¨Í´¿ì¡£");
			move(5, 4);
			prints("Ò»¿é°å×© 1000 ±øÂíÙ¸±Ò¡£");
			move(6, 4);
			if (currentuser.dietime > 0) {
				prints("ÄãÒÑ¾­ËÀÁË°¡£¡×¥¹í°¡£¡");
				pressanykey();
				Q_Goodbye();
				break;
			}
			usercomplete("ÄãÒªÅÄË­:", uident);
			if (uident[0] == '\0')
				break;
			freeTime = loadValue(currentuser.userid, "freeTime", 2000000000);
	       	if (currentTime < freeTime){
				pressreturn();
				break;
			}
			if (!(id = getuser(uident))) {
				move(7, 4);
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressreturn();
				break;
			}
			if (lookupuser.dietime > 0) {
				move(7, 4);
				prints("ËÀÈËÄãÒ²²»·Å¹ı£¬Ì«ºİÁË°É£¿");
				pressreturn();
				break;
			}
			if ((slowclubtest("Beggar", currentuser.userid)
				&& slowclubtest("Beggar", uident)) ||
				(slowclubtest("Rober", currentuser.userid)
				&& slowclubtest("Rober", uident)) ||
				(slowclubtest("Police", currentuser.userid)
				&& slowclubtest("Police", uident)) ||
				(slowclubtest("killer", currentuser.userid)
				&& slowclubtest("killer", uident)))
			{
				move(7, 4);
				prints("¶¼ÊÇ×Ô¼ÒĞÖµÜ...");
				pressreturn();
				break;
			}
			getdata(7, 4, "ÄãÒªÅÄ¼¸¿é£¿ [0]", genbuf, 4,
				DOECHO, YEA);
			if (genbuf[0] == '\0')
				break;
			count = atoi(genbuf);
			if (count < 1) {
				move(8, 4);
				prints("Ã»ÓĞ°å×©ÄãÄÃÊ²Ã´ÅÄ£¿");
				pressanykey();
				break;
			}
			if (currentuser.dietime > 0) {
				prints("ÄãÒÑ¾­ËÀÁË°¡£¡×¥¹í°¡£¡");
				pressanykey();
				Q_Goodbye();
				break;
			}
			move(8, 4);
			num = count * 1000;
			sprintf(genbuf, "×Ü¹²ĞèÒª %d ±øÂíÙ¸±Ò¡£", num);
			if (askyn(genbuf, NA, NA) == YEA) {
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				if (money < num) {
					move(9, 4);
					prints("ÄúµÄÇ®²»¹»...");
					pressanykey();
					break;
				}
				if (money_check_guard()) {
					pressanykey();
					break;
				}
				if(seek_in_file(DIR_MC "mingren", uident)) {
					prints ("      ËûÓĞ»ÆÂí¹Ó£¬Äã»¹ÊÇËãÁË°É\n");
					pressanykey();
					break;
				}

				if (seek_in_file(DIR_MC "killer", currentuser.userid))
					zhuannum=40;

				saveValue(currentuser.userid, "last_rob",
					  -2000000000, 2000000000);
                		saveValue(currentuser.userid, "last_rob",
					  time(0), 2000000000);
				saveValue(currentuser.userid, MONEY_NAME,
					-num,  MAX_MONEY_NUM);

				saveValue("BMYRober", MONEY_NAME,
					+num/2, MAX_MONEY_NUM);

				prints
				    ("        ¾­¹ı¼¸ÌìµÄÍµ¿úºÍ¸ú×Ù£¬Äã·¢ÏÖÃ¿ÌìÔçÉÏ7µã10·Ö%s»áÂ·¹ıÆ§¾²µÄ\n",
				     uident);
				prints
				    ("    ¶«»¨Ô°±ß¡£½ñÌìÄãÄÃ×ÅÂòÀ´±øÂíÙ¸°å×©£¬×¼±¸ĞĞ¶¯ÁË¡£\n");
				prints
				    ("        ÅÄÈË°å×©£¬¿ÉÒÔÈÃÆä×¡Ôº»¨Ç®ÖÎÉË£¬ºÙºÙ...\n");
				prints
				    ("        µ±È»£¬ÄãÒ²¿ÉÄÜÔâµ½·´»÷£¬ÉõÖÁÖÂËÀ£¡\n");
				if (askyn("    ·Ï»°ÉÙËµ£¬Äã»¹ÏëÅÄÃ´£¿", YEA, NA) == NA) {
					move(15, 0);
					prints
					    ("            °¦£¬×îºó¹ØÍ·Äãº¦ÅÂÁË£¬ËùÒÔ²»ÅÄÁË¡£\n");
					pressanykey();
					break;
				} else {
					if(!seek_in_file(DIR_MC "gongji", uident))
						saveValue(currentuser.userid, "rob", 1, 50);
					if (currentuser.dietime > 0) {
						prints("ÄãÒÑ¾­ËÀÁË°¡£¡×¥¹í°¡£¡");
						pressanykey();
						Q_Goodbye();
						break;
					}
					sleep(5);
					x = countexp(&currentuser);
					y = countexp(&lookupuser);
					r = random() % 2;
					if (r == 0)
						z = x;
					else
						z = y;
					r = random() % 100;
					num = 1000 + random() % 2000;
					move(16, 4);
					if (r < 100 * z / (x + x + y + y) +zhuannum+ count)	//ÅÄÈË³É¹¦
					{
						guard_num = loadValue(uident, "guard", 8);
						if (guard_num > 0) {
							saveValue(uident, "guard", -1, 50);
							prints("Äã¸ÉµôÁËËûÒ»¸ö±£ïÚ");
							pressanykey();
							break;
						}

						prints
						    ("       ÄãÕâ»µµ°£¬±³ºóÍµÏ®£¬ÔÒÖĞ%sµÄĞ¡ÄÔ´ü¹Ï¡£\n",
						     uident);
						money = loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
						if (money == 0) {
							if(!Allclubtest(lookupuser.userid) || seek_in_file(DIR_MC "chayou", lookupuser.userid)){
								showAt(17, 4, "Äã¶¼ÅÄµ½ÈË¼ÒÃ»Ç®ÖÎÉËÁË...»ıµãÒõµÂ°É£¡\n", 0);
								sprintf(buf,
									"Äã±»%sÅÄÁË°å×©£¬ÄãÃ»Ç®ÖÎÉË£¬Ö»ÄÜÒ§ÑÀÈÌÍ´...",
									currentuser.userid);
							}else{
								saveValue(uident, MONEY_NAME, -money, MAX_MONEY_NUM);
								move(17, 4);
								prints ("       ÄãÅÄÁË%s°å×ª£¬ËûËÀÁË¡£", uident);
								sprintf(genbuf, "%s½øĞĞºÚ°ï»î¶¯(ÅÄ×©)", currentuser.userid);
								sprintf(buf,"%sÅÄËÀÁË%s ", currentuser.userid, uident);
								millionairesrec(genbuf, buf, "ºÚ°ï»î¶¯");
								lookupuser.dietime = lookupuser.stay + 999 * 60;
								substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
								if (seek_in_file(DIR_MC "killer", currentuser.userid)){
									if (random()%3 == 0){
										sprintf(genbuf, "Äã±»%sÓÃ°å×©ÔÒËÀÁË£¬ºÃ²Ò",
											currentuser.userid);
										mail_buf(genbuf, uident, "ÌæÌìĞĞµÀ");}
                                  						sprintf(genbuf,
											"±¾Õ¾ÈËÊ¿%sÓÚ10·ÖÖÓÇ°ÔÚÍ­ÂàÍåµÄ\nÒ»ÆğÇ¹»÷ÊÂ¼şÖĞÒûµ¯ÉíÍö\n¾¯·½Í¸Â¶´ËÈËÓĞ°ï»á±³¾°\n\n"
									 		"Ä¿Ç°±¾Õ¾¼¤½ø×éÖ¯É±ÊÖÌì¿ÕĞû²¼¶Ô´ËÊÂ¸ºÔğ£¬\nÓĞ¹ØÊÂ¼şµÄ½øÒ»²½±¨µÀÇë¹Ø×¢±¾°æĞÂÎÅ", uident);
                                   					deliverreport("[ĞÂÎÅ]Í­ÂàÍå·¢ÉúÒ»ÆğÇ¹»÷ÊÂ¼ş", genbuf);
								}
								else if (slowclubtest("Beggar", currentuser.userid)){
									sprintf(genbuf,
										"±¾¸ÛÈËÊ¿%sÓÚ10·ÖÖÓÇ°ÔÚ¼âÉ³¾×µÄ\nÒ»Æğ±©Á¦³åÍ»ÖĞÉËÖØ²»ÖÎ\n¾¯·½³Æ´ËÈËÓĞ°ï»á±³¾°\n\n"
									 	"¾İÏûÏ¢ÁéÍ¨ÈËÊ¿Í¸Â¶£¬´ËÊÂ¼şÓë½üÆÚ\nµÄØ¤°ï»î¶¯ÓĞ¹Ø", uident);
                                   				deliverreport("[ĞÂÎÅ]¼âÉ³¾×·¢ÉúÒ»Æğ±©Á¦ÊÂ¼ş",genbuf);
								   	sprintf(genbuf,
										"Äã±»Ø¤°ïµÜ×Ó%sÓÃ°å×©ÔÒËÀÁË£¬ºÃ²Ò", currentuser.userid);
									mail_buf(genbuf, uident, "ÄãËÀÁË");
								}
								else if (slowclubtest("Rober",currentuser.userid)){
									sprintf(genbuf,
										"±¾¸ÛÈËÊ¿%sÓÚ10·ÖÖÓÇ°ÔÚ°ÄÃÅµÄ\nÒ»ÆğºÚ°ïĞµ¶·ÖĞÉ¥Ãü\n¾¯·½»³ÒÉ´ËÈËÓëºÚÉç»áÓĞ¹ı½Ú\n\n"
									 	"¾İÒ»Î»²»Ô¸Í¸Â¶ĞÕÃûµÄ¾¯Êğ¹ÙÔ±Í¸Â¶\nÕâ´ÎÊÂ¼ş¿ÉÄÜºÍºÚ°ïÑ°³ğÓĞ¹Ø\n¾¯·½±íÊ¾Ò»¶¨´ò»÷·¸×ï£¬Î¬»¤ÖÎ°²", uident);
                                   				deliverreport("[ĞÂÎÅ]°ÄÃÅ·¢ÉúÒ»Æğ°ï»á³åÍ»", genbuf);
									sprintf(genbuf,"Äã±»%sÓÃ°å×©ÔÒËÀÁË£¬ºÃ²Ò", currentuser.userid);
									mail_buf(genbuf, uident,"ÄãËÀÁË");
								}
								else if (slowclubtest("killer",currentuser.userid)){
									sprintf(genbuf,
									"ÄãÔÚºÍºÚ°ïµÄ³åÍ»ÖĞ±»%sÓÃ°å×©ÔÒËÀÁË£¬ºÃ²Ò", currentuser.userid);
									mail_buf(genbuf, uident,"ÌæÌìĞĞµÀ");
                                  					sprintf(genbuf,
										"±¾Õ¾ÈËÊ¿%sÓÚ10·ÖÖÓÇ°ÔÚ¾ÅÁúµÄ\nÒ»ÆğÇ¹»÷ÊÂ¼şÖĞÒûµ¯ÉíÍö\n¾¯·½Í¸Â¶´ËÈËÓĞ°ï»á±³¾°\n\n"
									 	"¾¯·½»³ÒÉËÀÕßÓëÉ±ÊÖÓĞË½ÈË¶÷Ô¹£¬\nÓĞ¹ØÊÂ¼şµÄ½øÒ»²½±¨µÀÇë¹Ø×¢±¾°æĞÂÎÅ", uident);
                                   				deliverreport("[ĞÂÎÅ]¾ÅÁú·¢ÉúÒ»ÆğÇ¹»÷ÊÂ¼ş", genbuf);
							     }
								else{
									sprintf(genbuf, "Äã±»%sÓÃ°å×©ÔÒËÀÁË£¬ºÃ²Ò", currentuser.userid);
									mail_buf(genbuf, uident, "ÄãËÀÁË");
								}
								//saveValue(lookupuser.userid, MONEY_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
								pressanykey();
							}
						} else {
							saveValue(uident, MONEY_NAME, -num, MAX_MONEY_NUM);
							sprintf(buf,
								"¹ş¹ş£¬%s»¨ÁË%dÔªÖÎÉË£¬ÏÖÔÚ³öÔºÁË¡£Ğ¡ĞÄ±¨¸´Äã£¡\n",
								uident, num);
							move(17, 4);
							prints("%s", buf);
							sprintf(buf,
								"Äã±»%sÅÄÁË°å×©£¬»¨ÁË%d±øÂíÙ¸±ÒÖÎÉË£¬ÎØÎØÎØÎØ...",
								currentuser.userid, num);
						}
					} else {
						prints
						    ("      ºÜ²»ĞÒ£¬ÄãÃ»ÓĞÅÄÖĞ¡£·´¶ø±»ÔÒÖĞĞ¡ÄÔ´ü¹Ï...");

						money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
						num *= 3;
						if (money < num) {
							saveValue (currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
							showAt(17, 4, "ÄãÏÊÑªÖ±Á÷£¬¿ÉÊÇÇ®²»¹»ÖÎÁÆ£¬±»Ò½ÔºÈÓÁË³öÀ´¡£", 0);
							showAt(18, 4, "×îºóÉËÊÆ¶ñ»¯£¬ÄãËÀÁË...", 0);
							sprintf(genbuf, "%s½øĞĞºÚ°ï»î¶¯(ÅÄ×©)", currentuser.userid);
							sprintf(buf,"%sÅÄ%s, ×Ô¼º¹ÒÁË, ¹Ï ", currentuser.userid, uident);
							millionairesrec(genbuf, buf, "ºÚ°ï»î¶¯");
							set_safe_record();
							currentuser.dietime = currentuser.stay + (num - money);
							substitute_record (PASSFILE, &currentuser, sizeof(currentuser), usernum);
							saveValue(currentuser.userid, MONEY_NAME,  -MAX_MONEY_NUM,  MAX_MONEY_NUM);
							saveValue(currentuser.userid, CREDIT_NAME,  -MAX_MONEY_NUM,  MAX_MONEY_NUM);
							pressanykey();
							Q_Goodbye();
						} else {
							saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
							move(17, 4);
							sprintf(buf, "Äã»¨ÁË%d±øÂíÙ¸±Ò²ÅÖÎºÃÁËÉË£¬¿´ÄãÏÂ´Î»¹ÅÄÈË²»¡£",
								num);
							prints("%s", buf);
						}
					}
				}
				pressanykey();
			}
			break;
		case '2':
			clear();
			if(!Allclubtest(currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  ÆÕÍ¨ÊĞÃñ²»ÒªÈÇÊÂ\033[m", 1);
				break;
			}
			if (seek_in_file(DIR_MC "chayou", currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  ²èÓÑ²»ÒªÈÇÊÂ\033[m", 1);
				break;
			}
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  ²»ÒªÈÇÊÂ\033[m", 1);
				break;
			}
			move(6, 4);
			usercomplete("ÍµË­£¿", uident);
			if (uident[0] == '\0')
				break;
			/*if(!Allclubtest(uident)){
				prints("    \033[1;32m  ²»Òª²ĞÉ±ÎŞ¹¼£¡\033[m");
				pressanykey();
				break;
			}*/
			freeTime = loadValue(currentuser.userid, "freeTime", 2000000000);
			if (currentTime < freeTime){
				pressreturn();
				break;
			}
			if (!getuser(uident)) {
				showAt(7, 4, "´íÎóµÄÊ¹ÓÃÕß´úºÅ...", 2);
				break;
			}
			if(seek_in_file(DIR_MC "mingren", uident)){
				showAt (7, 4, "      ËûÓĞ»ÆÂí¹Ó£¬Äã»¹ÊÇËãÁË°É\n", 1);
				break;
			}
			if (lookupuser.dietime > 0) {
				showAt(7, 4, "ËÀÈËÄãÒ²²»·Å¹ı£¬Ì«ºİÁË°É£¿", 1);
				break;
			}
			if(strcmp(lookupuser.userid,"BMYpolice")==0||strcmp(lookupuser.userid,"BMYbeg")==0||
				strcmp(lookupuser.userid,"BMYRober")==0||strcmp(lookupuser.userid,"BMYboss")==0||
                		strcmp(lookupuser.userid,"BMYKillersky")==0){
				showAt(7, 4, "Õâ¸öÈËÊÇÎÒÇ×Æİ£¬²»ĞíÇÀ", 2);
				break;
			}
			credit = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);
			if(credit<2000){
				showAt(7, 4, "±£Ö¤½ğ¶¼Ã»ÓĞ£¬»¹ÊÇ²»ÒªÍµÁË!", 2);
				break;
			}

			money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);

			if (currentuser.stay < 86400) {
				showAt(7, 4, "Ğ¡º¢×Ó¼Ò±ğÑ§»µÁË!", 2);
				break;
			}
			getdata(7, 4, "ÇëÊäÈëÄãµÄÃÜÂë: ", buf, PASSLEN, NOECHO, YEA);
			if (*buf == '\0'
			    || !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
				showAt(8, 4, "ºÜ±§Ç¸, ÄúÊäÈëµÄÃÜÂë²»ÕıÈ·¡£", 2);
				break;
			}
			  saveValue(currentuser.userid, "last_rob", -2000000000, 2000000000);
			  saveValue(currentuser.userid, "last_rob", time(0), 2000000000);
			showAt(9, 4,
				"\033[1;5;31m¾¯¸æ\033[0;1;31m£º Ğ¡ĞÄ°¡£¬×î½ü¾¯ÊğÔÚÑÏ´òÅ¶£¡", 0);
			move(10, 4);
			if (askyn("ÕæµÄÒªÍµÃ´£¿", NA, NA) == NA)
				break;
			set_safe_record();
			if (currentuser.dietime > 0) {
				showAt(11, 4, "ÄãÒÑ¾­ËÀÁË°¡£¡×¥¹í°¡£¡", 1);
				Q_Goodbye();
				break;
			}
			if (money_check_guard()) {
				pressanykey();
				break;
			}
			//currentuser.stay -= 3600 * 1;
			//substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
			r = random() % 100;
			x = countexp(&currentuser);
			y = countexp(&lookupuser);
			if(random() % x *0.7<random() % y)
			{
			//saveValue(currentuser.userid, CREDIT_NAME, -2000, MAX_MONEY_NUM);
			//saveValue("police", MONEY_NAME, +2000, MAX_MONEY_NUM);
			}

			if (NULL == t_search(uident, NA, 1))
				ra = 1;
			else
				ra = 10;
			if (r < 100 * x / (x + x + y + y) / ra) {
				guard_num = loadValue(uident, "guard", 8);
				if (guard_num > 0) {
					if (loadValue(uident, MONEY_NAME, MAX_MONEY_NUM) > guard_num * 1000000) {
						if (random() % 2 == 0)
							saveValue(uident, "guard", -2, 50);
						else
							saveValue(uident, "guard", -1, 50);
					} else {
							saveValue(uident, "guard", -1, 50);
					}
					showAt(11, 4, "Äã¸ÉµôÁËËûÒ»¸ö±£ïÚ", 1);
					break;
				}
				if (random() % 2 == 0) {
					money = loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
					r = random() % 50;
					money = money / 100 * r;
					saveValue(uident, MONEY_NAME, -money, MAX_MONEY_NUM);
					saveValue(currentuser.userid, MONEY_NAME, money,
						  MAX_MONEY_NUM);
					move(11, 4);
					prints
					    ("\033[1;31m%s\033[m µÄÇ®°üÃ»·ÅºÃ£¬Äã°ÑÊÖÉì½øÈ¥£¬Ãşµ½ÁË %d ±øÂíÙ¸±ÒÏÖ½ğ£¬¿ìÅÜ°É...",
					     uident, money);
					sprintf(title, "%s½øĞĞºÚ°ï»î¶¯(ÍµÇÔ)", currentuser.userid);
					sprintf(buf,"%sÍµÁË%s %d±øÂíÙ¸±Ò", currentuser.userid, uident, money);
					if (money != 0)
						millionairesrec(title, buf, "ºÚ°ï»î¶¯");
					sprintf(buf,
						"%s ³ÃÄú²»×¢ÒâµÄÊ±ºòÍµÁËÄú %d ±øÂíÙ¸±Ò¡£",
						currentuser.userid, money);
					sprintf(title, "¶Ô²»Æğ£¬Äú±»ÍµÇÔ");
					if(Allclubtest(uident)||loadValue(uident, "mail", 8))
					mail_buf(buf, uident, title);
					pressanykey();
					break;
				} else {
					money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
					r = random() % 70;
					money = money / 100 * r;
					saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
					saveValue(uident, MONEY_NAME, money, MAX_MONEY_NUM);
					move(11, 4);
					prints
					    ("\033[1;31mÄãÈ¥Ãş %s µÄÇ®°ü,ÑÛ¿´ÒÑ¾­µÃÊÖÁË,ËûºöÈ»×ª¹ıÉíÀ´·¢ÏÖÁËÄã",
					     uident);
					move(12, 4);
					prints
					    ("\033[1;31m°¦Ñ½Ñ½,ÄãÒ»ã¶Éñ,²»½öÃ»Íµµ½ËûµÄÇ®°ü,·´¶ø±»ËûÃş×ßÁË %d ±øÂíÙ¸±Ò¡£",
					     money);
					sprintf(title, "%s½øĞĞºÚ°ï»î¶¯(ÍµÇÔ)", currentuser.userid);
					sprintf(buf,"%sÍµ%s, ·´±»ÇÀÁË%d±øÂíÙ¸±Ò", currentuser.userid, uident, money);
					if (money != 0)
						millionairesrec(title, buf, "ºÚ°ï»î¶¯");
					sprintf(title, "ÄúÓöµ½Ğ¡Íµ");
					sprintf(buf,
						"%s Ïë³ÃÄú²»×¢ÒâÍµÄúµÄÇ®°ü,½á¹ûÈÃÄã·¢ÏÖÁË¡£Äã·´ÇÀÁËËû %d ±øÂíÙ¸±Ò¡£Õâ°Ñ×¬·­ÁË,^_^",
						currentuser.userid, money);
					if(Allclubtest(uident)||loadValue(uident, "mail", 8))
					mail_buf(buf, uident, title);
					pressanykey();
					break;
				}

			} else if (r < 90) {
				money = loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
				rob = loadValue(currentuser.userid, "rob", 50);
				move(11, 4);
				if (rob > 20) {
					saveValue(currentuser.userid, "rob", -rob/2, 50);
					prints
					    ("°¡£¡ÓĞ¾¯²ì£¬ÄãÔÚÌÓÅÜµÄÊ±ºòÖ»ÌıÒ»ÉùÇ¹Ïì...");
					set_safe_record();
					if (money / 200 < 3600)
						currentuser.dietime = currentuser.stay + 1000*60;
					else if (money < 10000000){
						mathtmp = (double)(money)/10000;
						mathtmp = 686.3455879296685 + 4.0492760356525315 * mathtmp + 0.004264378376417802 * mathtmp * mathtmp;//ÎÒÄâºÏµÄ¶ş´Îº¯Êı
						currentuser.dietime = currentuser.stay + (int)(mathtmp * 60);//+(money / 200)
					}
					else{
						mathtmp = 9 + (double)(currentuser.lastlogin)/(double)(currentuser.stay + currentuser.lastlogin);
						currentuser.dietime = currentuser.stay +(int) (1000*mathtmp*60);
					}
					substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
					saveValue(currentuser.userid, MONEY_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
					pressanykey();
					Q_Goodbye();
				} else {
					if (askyn
					    ("±»¾¯²ì·¢ÏÖÁË,ÄãÒªÌÓÅÜÃ´?", YEA, NA) == NA) {
						saveValue(currentuser.userid, "rob", 1, 50);
						move(12, 4);
						if (askyn ("¾¯²ìÎÊÄã»°,Äã×¼±¸Ì¹°×´Ó¿íÃ´?", YEA, NA) == YEA) {
							money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
							saveValue(currentuser.userid, MONEY_NAME, -money * 50 /100, MAX_MONEY_NUM);
							sprintf(title, "%s½øĞĞºÚ°ï»î¶¯(ÍµÇÔ)", currentuser.userid);
							sprintf(buf,"%sÍµ%s, ±»¾¯²ìÃ»ÊÕ%d±øÂíÙ¸±Ò", currentuser.userid, uident, money/2);
							if (money != 0)
								millionairesrec(title, buf, "ºÚ°ï»î¶¯");
							showAt
							    (13, 4, "Äã±»´øµ½¾¯²ì¾Ö,ÔÚÃ»ÊÕÁËÉíÉÏËùÓĞµÄÇ®Ö®ºó,»¹Òª¸øÄãÑµ»°Ò»·¬¡£", 0);
							showAt
							    (14, 4, "ÏÖÔÚÊÇ¾¯²ì¸øÄãµÄ15ÃëÖÓÑµ»°Ê±¼ä£¬ÀÏÀÏÊµÊµÌı×Å°É¡£", 1);
							sleep(15);
							money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
							sprintf(genbuf,
								"³öÁË¾¯²ì¾Ö,Äã¸ßĞËµÄ´ÓĞ¬ÀïÌÍ³ö²ØÆğÀ´µÄ%d±øÂíÙ¸±Ò¡£ÎØÎØ,Ò»¹É³ô½ÅÑ¾×ÓÎ¶...",
								money);
							showAt(15, 4, genbuf, 1);
						} else {
							money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
							if (random() % 2 == 0) {
								saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
								sprintf(title, "%s½øĞĞºÚ°ï»î¶¯(ÍµÇÔ)", currentuser.userid);
								sprintf(buf,"%sÍµ%s, ±»¾¯²ìÃ»ÊÕ%d±øÂíÙ¸±Ò(È«²¿)", currentuser.userid, uident, money);
								if (money != 0)
									millionairesrec(title, buf, "ºÚ°ï»î¶¯");
								showAt
								    (13, 4, "¾¯²ìÎÊ»°Äã»¹²»ÀÏÊµ,ËûÒ»Å­Ö®ÏÂÒ»°Ñ¶á¹ıÄãµÄÇ®°ü,Ñï³¤¶øÈ¥¡£", 0);
								showAt
								    (14, 4, "Äã×øÔÚµØÉÏ´ó¿Ş:\"¾¯·ËÒ»¼Ò°¡!ÎÒµÄÇ®,ÎÒµÄÇ®...\"", 1);
							} else {
								showAt
								    (13, 4, "¾¯²ìÎÊ»°Ê±Äã°Ù°ãµÖÀµ,µ½×îºóËûÒ²ÄÃÄãÃ»°ì·¨,Ö»ºÃ°ÑÄã·ÅÁË.", 0);
								showAt
								    (14, 4, "¹ş¹ş! ¿¹¾Ü´ÓÑÏ,»Ø¼Ò¹ıÄê", 1);
							}
						}
					} else {
						move(12, 4);
						if (random() % 2 == 0) {
							saveValue(currentuser.userid, "rob", 5, 50);
							prints("ÄãÃ»ÃüµØÌÓÅÜ,¿ÉÏ§,Ç®°ü¶ªÔÚÁËÂ·ÉÏ...");
							money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
							saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
							sprintf(title, "%s½øĞĞºÚ°ï»î¶¯(ÍµÇÔ)", currentuser.userid);
							sprintf(buf,"%sÍµ%s, ÌÓÅÜÖĞËğÊ§%d±øÂíÙ¸±Ò(È«²¿)", currentuser.userid, uident, money);
							if (money != 0)
								millionairesrec(title, buf, "ºÚ°ï»î¶¯");
							pressanykey();
						} else {
							saveValue(currentuser.userid, "rob", -rob/2, 50);
							prints
							    ("°¡£¡ÄãÔÚÌÓÅÜµÄÊ±ºòÖ»ÌıÒ»ÉùÇ¹Ïì...");
							set_safe_record();
							if (money / 200 < 3600)
								currentuser.dietime = currentuser.stay + 1000*60;
							else if (money < 10000000){
								mathtmp = (double)(money)/10000;
								mathtmp = 686.3455879296685 + 4.0492760356525315 * mathtmp + 0.004264378376417802 * mathtmp * mathtmp;//ÎÒÄâºÏµÄ¶ş´Îº¯Êı
								currentuser.dietime = currentuser.stay + (int)(mathtmp * 60);//+(money / 200)
							}
							else{
								mathtmp = 9 + (double)(currentuser.lastlogin)/(double)(currentuser.stay + currentuser.lastlogin);
								currentuser.dietime = currentuser.stay +(int) (1000*mathtmp*60);
							}
							substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
							pressanykey();
							saveValue(currentuser.userid, MONEY_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
							sprintf(title, "%s½øĞĞºÚ°ï»î¶¯(ÍµÇÔ)", currentuser.userid);
							sprintf(buf,"%sÍµ%s, ±»»÷±Ğ, ËğÊ§%d±øÂíÙ¸±Ò(È«²¿)", currentuser.userid, uident, money);
							if (money != 0)
								millionairesrec(title, buf, "ºÚ°ï»î¶¯");
							Q_Goodbye();
						}
					}
				}
				break;
			} else {
				move(11, 4);
				prints
				    ("\033[1;31m%s\033[m °ÑÇ®°ü¿´µÃ½ô½ôµÄ£¬Äã¼Ù×°²»Ğ¡ĞÄ×²ÁËËûÒ»ÏÂ,¿ÉÒ»·ÖÇ®¶¼Ã»Íµµ½¡£",
				     uident);
				pressanykey();
				break;
			}
			break;
		case '3':
			clear();
			money_show_stat("±øÂíÙ¸ºÚ°ïÑø¸ë³¡");
			showAt
			    (4, 4, "ºÚ°ïÎªÄãÌá¹©ÀÕË÷ĞÅ¼ş·¢ËÍÒµÎñ,Ã¿´ÎÊÕ·ÑÊÓÇéĞÎ¶ø¶¨¡£", 0);
			if (currentuser.dietime > 0) {
				showAt(5, 4, "ÄãÒÑ¾­ËÀÁË°¡£¡×¥¹í°¡£¡", 1);
				Q_Goodbye();
				break;
			}
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
				clear();
				showAt(5, 4, "    \033[1;32m  ²»ÒªÈÇÊÂ\033[m", 1);
				break;
			}
			usercomplete("ÄãÒªÀÕË÷Ë­:", uident);
			if (uident[0] == '\0')
				break;
			if (!(id = getuser(uident))) {
				showAt(7, 4, "´íÎóµÄÊ¹ÓÃÕß´úºÅ...", 2);
				break;
			}
			if (lookupuser.dietime > 0) {
				showAt(7, 4, "¹íÄãÒ²¸ÒÀÕË÷°¡...", 1);
				break;
			}
			move(8, 4);
			sprintf(genbuf, "È·¶¨ÒªÀÕË÷Ã´?");
			if (askyn(genbuf, NA, NA) == YEA) {
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				if (money < 100) {
					showAt(9, 4, "ÄúµÄÇ®²»¹»¡£", 1);
					break;
				}
				if (money < 1000) {
					saveValue(currentuser.userid, MONEY_NAME, -100, MAX_MONEY_NUM);
					sprintf(title, "%s¹ÜÄãÒª¼¸Ç§¿é±øÂíÙ¸±Ò", currentuser.userid);
					mail_buf(letter1, uident, title);
				} else if (money < 100000) {
					saveValue(currentuser.userid, MONEY_NAME, -1000, MAX_MONEY_NUM);
					sprintf(title, "%s¹ÜÄãÒª¼¸Íò¿é±øÂíÙ¸±Ò", currentuser.userid);
					mail_buf(letter2, uident, title);
				} else if (money < 10000000) {
					saveValue(currentuser.userid, MONEY_NAME, -100000, MAX_MONEY_NUM);
					sprintf(title, "%s¹ÜÄãÒªÒ»°ÙÍò±øÂíÙ¸±Ò", currentuser.userid);
					mail_buf(letter3, uident, title);
				} else {
					saveValue(currentuser.userid, MONEY_NAME, -500000, MAX_MONEY_NUM);
					sprintf(title, "%s¹ÜÄãÒªÒ»Ç§Íò±øÂíÙ¸±Ò", currentuser.userid);
					mail_buf(letter3, uident, title);
				}
				showAt(10, 4, "ĞÅ·¢³öÈ¥ÁË£¬»ØÈ¥µÈÏûÏ¢°É¡£", 1);
			}
			break;
		case '4':
			clear();
			if (seek_in_file(DIR_MC "chayou", currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  ²èÓÑ²»ÒªÈÇÊÂ\033[m", 1);
				break;
			}
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  ²»ÒªÈÇÊÂ\033[m", 1);
				break;
			}
			move(6, 4);
			usercomplete("ÇÀË­£¿", uident);
			if (uident[0] == '\0')
				break;
			/*if(!Allclubtest(uident)){
				prints("    \033[1;32m  ²»Òª²ĞÉ±ÎŞ¹¼£¡\033[m");
				pressanykey();
				break;
			}*/
			freeTime = loadValue(currentuser.userid, "freeTime", 2000000000);
			if (currentTime < freeTime){
				pressreturn();
				break;
			}
			if (!getuser(uident)) {
				showAt(7, 4, "´íÎóµÄÊ¹ÓÃÕß´úºÅ...", 2);
				break;
			}
			if(seek_in_file(DIR_MC "mingren", uident)){
				showAt(7, 4, "      ËûÓĞ»ÆÂí¹Ó£¬Äã»¹ÊÇËãÁË°É\n", 1);
				break;
			}
			if (lookupuser.dietime > 0) {
				showAt(7, 4, "ËÀÈËÄãÒ²²»·Å¹ı£¬Ì«ºİÁË°É£¿", 1);
				break;
			}
			if(strcmp(lookupuser.userid,"BMYpolice")==0||strcmp(lookupuser.userid,"BMYbeg")==0||
				strcmp(lookupuser.userid,"BMYRober")==0||strcmp(lookupuser.userid,"BMYboss")==0||
                		strcmp(lookupuser.userid,"BMYKillersky")==0){
				showAt(7, 4, "Õâ¸öÈËÊÇÎÒÇ×Æİ£¬²»ĞíÇÀ", 2);
				break;
			}
			money = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);

			if (currentuser.stay < 3600 + 86400) {
				showAt(7, 4, "Ğ¡º¢×Ó¼Ò²»ÒªÑ§»µÁË!", 2);
				break;
			}
			if (!clubtest("Rober")) {
				showAt(7, 4, "ÔõÃ´¿´ÄãÒ²²»ÏñÊÇ×÷¼é·¸¿ÆµÄÈË°¡£¡", 2);
				break;
			}
			getdata(7, 4, "ÇëÊäÈëÄãµÄÃÜÂë: ", buf, PASSLEN, NOECHO, YEA);
			if (*buf == '\0'
			    || !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
				showAt(8, 4, "ºÜ±§Ç¸, ÄúÊäÈëµÄÃÜÂë²»ÕıÈ·¡£", 2);
				break;
			}
			saveValue(currentuser.userid, "last_rob", -2000000000, 2000000000);
			saveValue(currentuser.userid, "last_rob", time(0), 2000000000);
			showAt(9, 4,
				"\033[1;5;31m¾¯¸æ\033[0;1;31m£º Ğ¡ĞÄ°¡£¬×î½ü¾¯ÊğÔÚÑÏ´òÅ¶£¡", 0);
			move(10, 4);
			if (askyn("ÕæµÄÒªÇÀÃ´£¿", NA, NA) == NA)
				break;
			set_safe_record();
			if (currentuser.dietime > 0) {
				showAt(11, 4, "ÄãÒÑ¾­ËÀÁË°¡£¡×¥¹í°¡£¡", 1);
				Q_Goodbye();
				break;
			}
			if (money_check_guard()) {
				pressanykey();
				break;
			}
			if (lookupuser.dietime > 0) {
				showAt(11, 4, "ÈË¶¼ËÀÁË,ÈÃËû°²Ï¢°É.", 1);
				break;
			}
			//currentuser.stay -= 3600 * 1;
			//substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
			r = random() % 100;
			x = countexp(&currentuser);
			y = countexp(&lookupuser);
			if (NULL == t_search(uident, NA, 1))
				ra = 1;//ra = 10;
			else
				ra = 2;
			if (r < 100 * x / (x + x + y + y) / ra) {
				guard_num = loadValue(uident, "guard", 8);
				if (guard_num > 0) {
					if (loadValue(uident, MONEY_NAME, MAX_MONEY_NUM) > guard_num * 1000000)
						saveValue(uident, "guard", -guard_num, 50);
					else
						saveValue(uident, "guard", -1, 50);
					prints
					    ("Äã¸ÉµôÁËËûÒ»¸ö±£ïÚ");
					pressanykey();
					break;
				}

				if (random() % 2 == 0) {
					money = loadValue(uident, CREDIT_NAME, MAX_MONEY_NUM);
					r = random() % 50;
					money = money / 100 * r;
					saveValue(uident, CREDIT_NAME, -money, MAX_MONEY_NUM);
					saveValue(currentuser.userid, MONEY_NAME, money, MAX_MONEY_NUM);
					move(11, 4);
					prints
					    ("\033[1;31m%s\033[m µÄÃÅÃ»Ëø£¬ÄãÁïÁË½øÈ¥,ÕÒ³ö´æÕÛ, »»µÃ %d ±øÂíÙ¸±ÒÏÖ½ğ£¬¿ìÅÜ°É¡£",
					     uident, money);
					sprintf(buf,
						"%s ³ÃÄú²»×¢ÒâµÄÊ±ºòÄÃÁËÄã¼ÒµÄ´æÕÛ,µÈÄã·¢ÏÖ¹ÒÊ§µÄÊ±ºòÒÑ¾­ËğÊ§ÁË %d ±øÂíÙ¸±Ò¡£",
						currentuser.userid, money);
					sprintf(title, "¶Ô²»Æğ£¬Äú±»ÇÀ½Ù");
					if(Allclubtest(uident)||loadValue(uident, "mail", 8))
					mail_buf(buf, uident, title);
					sprintf(title, "%s½øĞĞºÚ°ï»î¶¯(ÇÀ½Ù)", currentuser.userid);
					sprintf(buf,"%sÇÀ%s  %d±øÂíÙ¸±Ò", currentuser.userid, uident, money);
					millionairesrec(title, buf, "ºÚ°ï»î¶¯");
					pressanykey();
					break;
				} else {
					money = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);
					r = random() % 70;
					money = money / 100 * r;
					saveValue(currentuser.userid, CREDIT_NAME, -money, MAX_MONEY_NUM);
					saveValue(uident, CREDIT_NAME, money, MAX_MONEY_NUM);
					move(11, 4);
					prints
					    ("\033[1;31mÄãÁï½øÁË %s µÄÃÅ,ÕıµÃÒâÄØ,Ì§ÑÛ¿´¼ûºÚ¶´¶´µÄÇ¹¿Ú¶Ô×ÅÄã...",
					     uident);
					move(12, 4);
					prints
					    ("\033[1;31m°¦Ñ½Ñ½,Ã»Ïëµ½ËûÔÚ¼Ò,Äã±»ÆÈË½ÁË,´Ó´æÕÛÀïÈ¡³ö %d ±øÂíÙ¸±Ò¸øËû¡£",
					     money);
					sprintf(title, "ÄúÔâÓöÇÀ½Ù");
					sprintf(buf,
						"%s ÏëÇÀÄãµÄÇ®,½á¹ûÈÃÄã·¢ÏÖÁË,ÄãÀÕË÷ÁËËû %d ±øÂíÙ¸±Ò,ËÍÉÏÃÅµÄ·ÊÈâ°¡¡£",
						currentuser.userid, money);
					if(Allclubtest(uident)||loadValue(uident, "mail", 8))
					mail_buf(buf, uident, title);
					sprintf(title, "%s½øĞĞºÚ°ï»î¶¯(ÇÀ½Ù)", currentuser.userid);
					sprintf(buf,"%sÇÀ%s , ·´±»ÀÕË÷%d±øÂíÙ¸±Ò", currentuser.userid, uident, money);
					millionairesrec(title, buf, "ºÚ°ï»î¶¯");
					pressanykey();
					break;
				}

			} else if (r < 90) {
				money = loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
				rob = loadValue(currentuser.userid, "rob", 50);
				move(11, 4);
				if (rob > 20) {
					saveValue(currentuser.userid, "rob", -rob/2, 50);
					prints
					    ("°¡£¡ÓĞ¾¯²ì£¬ÄãÔÚÌÓÅÜµÄÊ±ºòÖ»ÌıÒ»ÉùÇ¹Ïì...");
					set_safe_record();
					if (money / 200 < 3600)
						currentuser.dietime = currentuser.stay + 1000*60;
					else if (money < 10000000){
						mathtmp = (double)(money)/10000;
						mathtmp = 686.3455879296685 + 4.0492760356525315 * mathtmp + 0.004264378376417802 * mathtmp * mathtmp;//ÎÒÄâºÏµÄ¶ş´Îº¯Êı
						currentuser.dietime = currentuser.stay + (int)(mathtmp * 60);//+(money / 200)
					}else{
						mathtmp = 9 + (double)(currentuser.lastlogin)/(double)(currentuser.stay + currentuser.lastlogin);
						currentuser.dietime = currentuser.stay +(int) (1000*mathtmp*60);
					}
					substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
					saveValue(currentuser.userid, MONEY_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
					pressanykey();
					Q_Goodbye();
				} else {
					if (askyn("±»¾¯²ì·¢ÏÖÁË,ÄãÒªÌÓÅÜÃ´?", YEA, NA) == NA) {
						saveValue(currentuser.userid, "rob", 1, 50);
						money = loadValue(currentuser.userid, MONEY_NAME,  MAX_MONEY_NUM);
						saveValue(currentuser.userid, MONEY_NAME, -money * 50 / 100, MAX_MONEY_NUM);
						sprintf(title, "%s½øĞĞºÚ°ï»î¶¯(ÇÀ½Ù)", currentuser.userid);
						sprintf(buf,"%sÇÀ%s ±»¾¯²ìÃ»ÊÕ%d±øÂíÙ¸±Ò", currentuser.userid, uident, money/2);
						millionairesrec(title, buf, "ºÚ°ï»î¶¯");
						showAt
						    (12, 4, "Äã±»´øµ½¾¯²ì¾Ö,ÔÚÃ»ÊÕÁËÉíÉÏËùÓĞµÄÇ®Ö®ºó,ÏÖÔÚµÈ¾¯²ì¸øÄãÑµ»°", 0);
						showAt
						    (13, 4, "ÏÖÔÚÊÇ¾¯²ì¸øÄãµÄ15ÃëÖÓÑµ»°Ê±¼ä£¬Ó²×ÅÍ·Æ¤Ìı°É¡£", 1);
						sleep(15);
						money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
						sprintf(genbuf,
							"³öÁË¾¯²ì¾ÖÁË,Äã¸ßĞËµÄ´ÓĞ¬ÀïÌÍ³ö²ØÆğÀ´µÄ%d±øÂíÙ¸±Ò¡£ÎØÎØ,Ò»¹É³ô½ÅÑ¾×ÓÎ¶...",
							money);
						showAt(14, 4, genbuf, 1);
					} else {
						move(12, 4);
						if (random() % 2 == 0) {
							saveValue(currentuser.userid, "rob", 5, 50);
							prints
							    ("ÌÓÅÜ³É¹¦,¿ÉÏ§,ÄãµÄÇ®°ü¶ªÔÚÁËÂ·ÉÏ...");
							saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
							sprintf(title, "%s½øĞĞºÚ°ï»î¶¯(ÇÀ½Ù)", currentuser.userid);
							sprintf(buf,"%sÇÀ%s, ÌÓÅÜËğÊ§%d±øÂíÙ¸±Ò(È«²¿)", currentuser.userid, uident, money);
							millionairesrec(title, buf, "ºÚ°ï»î¶¯");
							pressanykey();
						} else {
							saveValue(currentuser.userid, "rob", -rob/2, 50);
							prints("°¡£¡ÄãÔÚÌÓÅÜµÄÊ±ºòÖ»ÌıÒ»ÉùÇ¹Ïì...");
							set_safe_record();
							if (money / 200 < 3600)
								currentuser.dietime = currentuser.stay + 1000*60;
							else if (money < 10000000){
								mathtmp = (double)(money)/10000;
								mathtmp = 686.3455879296685 + 4.0492760356525315 * mathtmp + 0.004264378376417802 * mathtmp * mathtmp;//ÎÒÄâºÏµÄ¶ş´Îº¯Êı
								currentuser.dietime = currentuser.stay + (int)(mathtmp * 60);//+(money / 200)
							}else{
								mathtmp = 9 + (double)(currentuser.lastlogin)/(double)(currentuser.stay + currentuser.lastlogin);
								currentuser.dietime = currentuser.stay +(int) (1000*mathtmp*60);
							}
							substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
							pressanykey();
							saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
							sprintf(title, "%s½øĞĞºÚ°ï»î¶¯(ÇÀ½Ù)", currentuser.userid);
							sprintf(buf,"%sÇÀ%s, ±»»÷±Ğ, ËğÊ§%d±øÂíÙ¸±Ò(È«²¿)", currentuser.userid, uident, money);
							millionairesrec(title, buf, "ºÚ°ï»î¶¯");
							Q_Goodbye();
						}
					}
				}
				break;
			} else {
				move(11, 4);
				prints
				    ("\033[1;31m%s\033[m ¼ÒµÄÃÅËøµÄ½ô½ôµÄ£¬Äã¼Ù×°Â·¹ı,¿´¿´ÎŞ·¨µÃÊÖ,Ö»ºÃÀë¿ª¡£",
				     uident);
				pressanykey();
				break;
			}
			break;
			case '5':
				nomoney_show_stat("ºÚ°ï°ïÖ÷°ì¹«ÊÒ");
				whoTakeCharge2(4, buf);
				whoTakeCharge(4, uident);
				if (strcmp(currentuser.userid, uident)) {
					move(6, 4);
					prints
				  	  ("ÃØÊé%sÀ¹×¡ÁËÄã,ËµµÀ:¡°ÀÏ´ó%sÏÖÔÚºÜÃ¦,Ã»Ê±¼ä½Ó´ıÄã¡£¡±", buf,uident);
					move(8,4);
					if(!slowclubtest("Rober",currentuser.userid)){
					if (askyn("ÄãÊÇÏë¼ÓÈëºÚ°ïÂğ£¿", NA, NA) == YEA) {
						sprintf(genbuf, "%s Òª¼ÓÈëºÚ°ï", currentuser.userid);
						mail_buf(genbuf, "BMYRober", genbuf);
						move(14, 4);
						prints("ºÃÁË£¬ÎÒ»áÍ¨ÖªÀÏ´óµÄ");
					}}
					pressanykey();
					break;
				} else {
					move(6, 4);
					prints("ÇëÑ¡Ôñ²Ù×÷´úºÅ:");
					move(7, 6);
					prints("5. ´ÇÖ°                      6. ÍË³ö");
					ch2 = igetkey();
					switch (ch2) {
					case '5':
						move(12, 4);
						if (askyn("ÄúÕæµÄÒª´ÇÖ°Âğ£¿", NA,NA) == YEA) {
						/*	del_from_file(MC_BOSS_FILE,"gang");
							sprintf(genbuf, "%s Ğû²¼´ÇÈ¥ºÚ°ï°ïÖ÷Ö°Îñ", currentuser.userid);
							deliverreport(genbuf,
							      "±øÂíÙ¸½ğÈÚÖĞĞÄ¶ÔÆäÒ»Ö±ÒÔÀ´µÄ¹¤×÷±íÊ¾¸ĞĞ»£¬×£ÒÔºóË³Àû£¡");
							move(14, 4);
							prints
							    ("ºÃ°É£¬¼ÈÈ»ÄãÒâÒÑ¾ö£¬ÖĞĞÄÒ²Ö»ÓĞÅú×¼¡£");
							quit = 1;
							pressanykey();
						*/
						sprintf(genbuf, "%s Òª´ÇÈ¥ºÚ°ï°ïÖ÷Ö°Îñ",
							currentuser.userid);
						mail_buf(genbuf, "millionaires", genbuf);
						move(14, 4);
						prints("ºÃ°É£¬ÒÑ¾­·¢ĞÅ¸æÖª×Ü¹ÜÁË");
						pressanykey();
						}
						break;
					}
				}
				break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

static int/*Ø¤°ï*/
money_beggar()
{
	int ch,ch2;
	int quit = 0;
	char uident[IDLEN + 1], buf[STRLEN], title[40];
	int money, credit, num;
	int id;
	while (!quit) {
		money_show_stat("Ø¤°ï×Ü¶æ");
		move(4, 4);
		prints
		    ("Ø¤°ï×Ô¹ÅÌìÏÂµÚÒ»´ó°ï£¬²»¹ıÄ¿Ç°¾­¼Ã»¹Ëã¾°Æø£¬×öÆòØ¤µÄÈËÒ²²»¶àÀ²¡£");
		move(5, 4);
		prints
		    ("Ò»¸öÆòØ¤×ß¹ıÀ´ÎÊµÀ£º¡°Òª´òÌıÏûÏ¢Ã´£¿Ø¤°ïÌìÉÏµØÏÂÎŞËù²»Öª£¬ÎŞËù²»Ïş¡£¡±");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]´òÌ½ [2]ÉÕÇ® [3]¸ú×Ù [4]ÆòÌÖ [5]Ø¤°ï°ïÖ÷ [Q]Àë¿ª\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			move(6, 4);
			usercomplete("²éË­µÄ¼Òµ×£¿", uident);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				move(7, 4);
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressreturn();
				break;
			}
			money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);

			if (money < 1000) {
				showAt(7, 4, "°¡£¬ÄãÖ»´øÁËÕâÃ´µãÇ®Âğ£¿", 0);
				showAt(8, 4, "ÄÇÆòØ¤½Ó¹ıÇ®×ªÉí¾Í×ßÁË£¬ÔÙÒ²Ã»ÁËÏÂÎÄ¡£", 1);
				break;
			}
			saveValue(currentuser.userid, MONEY_NAME, -1000, MAX_MONEY_NUM);
			saveValue("BMYbeg", MONEY_NAME, 500, MAX_MONEY_NUM);
			saveValue("millionaires", MONEY_NAME, 500, MAX_MONEY_NUM);
			money = loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
			if (money >= 100)
				money = makeRumor(money);

			credit = loadValue(uident, CREDIT_NAME, MAX_MONEY_NUM);
			if (credit >= 100)
				credit = makeRumor(credit);

			move(7, 4);
			prints
			    ("\033[1;31m%s\033[m ´óÔ¼ÓĞ \033[1;31m%d\033[m ±øÂíÙ¸±ÒµÄÏÖ½ğ£¬ÒÔ¼° \033[1;31m%d\033[m ±øÂíÙ¸±ÒµÄ´æ¿î¡£",
			     uident, money, credit);
			pressanykey();
			break;
		case '2':
			clear();
			money_show_stat("Ø¤°ïÉñÃí");
			move(4, 4);
			prints
			    ("ÉÕÇ®×îĞ¡½ğ¶î 1000 ±øÂíÙ¸±Ò¡£¿ÉÂòÍ¨Ú¤¼ä¹ÜÊÂ£¬ÈÃËÀÕß¸´»î¡£");
			move(5, 4);
			usercomplete("¸øË­ÉÕ£¿", uident);
			if (uident[0] == '\0')
				break;
			if (!(id = getuser(uident))) {
				move(6, 4);
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressreturn();
				break;
			}
			if (!seek_in_file(MC_ADMIN_FILE, currentuser.userid) &&
				((lookupuser.dietime- lookupuser.stay) > 10000*60) ) {//5000->10000
				showAt(6, 4, "×ÔÉ±µÄÈËÔõÃ´¸´»î£¿ËÀÁË¾Í·ÅĞÄµÄÈ¥°É£¡£¡£¡", 1);
				break;
			}
			getdata(6, 4, "Äú´òËãÉÕ¶àÉÙ±øÂíÙ¸±Ò£¿[0]", genbuf, 10, DOECHO, YEA);
			num = atoi(genbuf);
			if (num < 1000) {
				showAt(7, 4, "ÄÇÃ´µãÇ®£¬ÔõÃ´»ßÂ¸Ú¤¼ä¹ÜÊÂ°¡£¿", 1);
				break;
			}
			move(7, 4);
			sprintf(genbuf, "ÄúÈ·ÈÏ¸ø %s ÉÕ %d ±øÂíÙ¸±Ò£¿", uident, num);
			if (askyn(genbuf, NA, NA) == YEA) {
				money = loadValue(currentuser.userid, MONEY_NAME,  MAX_MONEY_NUM);
				if (money < num) {
					showAt(8, 4, "ÄúµÄÇ®²»¹»", 1);
					break;
				}
				saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
				saveValue("millionaires", MONEY_NAME, +num/2, MAX_MONEY_NUM);
				saveValue("BMYboss", MONEY_NAME, +num/2, MAX_MONEY_NUM);
				if (lookupuser.dietime == 2 || lookupuser.dietime == 0) {
					showAt(8, 4, "°¡£¡²»ÊÇËÀ¹í£¬°×ÉÕÁË...", 1);
					break;
				}
				if (seek_in_file(MC_ADMIN_FILE, currentuser.userid) &&
					((lookupuser.dietime- lookupuser.stay) > 5000*60)){
					sprintf(title,"%sĞĞÊ¹ÉÕÇ®ÌØÈ¨", currentuser.userid);
					sprintf(buf,"%s¸ø%sÉÕÁË%d(/60=%d)±øÂíÙ¸±Ò",
						currentuser.userid, uident, num, num / 60);
					millionairesrec(title, buf, "");
				}else{
					sprintf(title,"%s¸ø%sÉÕÇ®", currentuser.userid, uident);
					sprintf(buf,"%s¸ø%sÉÕÁË%d(/60=%d)±øÂíÙ¸±Ò",
						currentuser.userid, uident, num, num / 60);
					millionairesrec(title, buf, "ÉÕÇ®");
				}
				if (lookupuser.dietime > lookupuser.stay)
					lookupuser.dietime -= num;
				if (lookupuser.dietime <= lookupuser.stay)
					lookupuser.dietime = 2;
				substitute_record(PASSFILE, &lookupuser, sizeof (lookupuser), id);
				showAt(8, 4, "ÉÕÍêÁË£¬×ß°É¡£", 1);
				sprintf(title,
					"ÄúµÄÅóÓÑ %s ¸øÄúËÍÇ®À´ÁË",
					currentuser.userid);
				sprintf(buf,
					"ÄúµÄÅóÓÑ %s ¸øÄúÉÕÁËµãÇ®£¬ÄúµÄËÀÆÚËõ¶ÌÁË%d·ÖÖÓ",
					currentuser.userid, num / 60);
				mail_buf(buf, uident, title);
				pressanykey();
			}
			break;
		case '3':
			move(6, 4);
			usercomplete("Òª¸ú×ÙË­£¿", uident);
			if (uident[0] == '\0')
				break;
			if (!getuser(uident)) {
				showAt(7, 4, "´íÎóµÄÊ¹ÓÃÕß´úºÅ...", 2);
				break;
			}
			money =  loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
			if (money < 1000) {
				showAt(7, 4, "ÄãÉíÉÏÕâµãÇ®»¹²»¹»ÅÜÂ··Ñ°¡¡£", 0);
				showAt(8, 4, "ÄÇÆòØ¤½Ó¹ıÇ®×ªÉí¾Í×ßÁË£¬ÔÙÒ²Ã»ÁËÏÂÎÄ¡£", 1);
				break;
			}
			saveValue(currentuser.userid, MONEY_NAME, -1000, MAX_MONEY_NUM);
			saveValue("BMYbeg", MONEY_NAME, 500, MAX_MONEY_NUM);
			saveValue("millionaires", MONEY_NAME, 500, MAX_MONEY_NUM);
			move(7, 4);
			prints("¼¸Ììºó£¬ÄãÊÕµ½Ø¤°ïµÄÏûÏ¢Ëµ£º");
			move(8, 4);
			prints
			    ("\033[1;31m%s\033[m ÓĞ \033[1;31m%s\033[m µÄµØÎ»£¬ÒÔ¼° \033[1;31m%s\033[m Ò»°ãµÄ²ÅÒÕ¡£",
			     uident, charexp(countexp(&lookupuser)), cperf(countperf(&lookupuser)));
			pressanykey();
			break;
		case '4':
			clear();
			if (seek_in_file(DIR_MC "chayou", currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  ²èÓÑ²»ÒªÈÇÊÂ\033[m", 1);
				break;
			}
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  ²»ÒªÈÇÊÂ\033[m", 1);
				break;
			}
			money_show_stat("±øÂíÙ¸Ğ¡Çø");
			showAt(4, 4, "ÕâÀïÊÇ±øÂíÙ¸µÄ¸»ÈËÇø£¬ÆòÌÖµÄºÃµØ·½¡£", 0);
			move(6, 4);
			usercomplete("ÏòË­ÆòÌÖ£¿", uident);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				showAt(7, 4, "´íÎóµÄÊ¹ÓÃÕß´úºÅ...", 2);
				break;
			}
			if (!getuser(uident)) {
				showAt(7, 4, "´íÎóµÄÊ¹ÓÃÕß´úºÅ...", 2);
				break;
			}
			/*if(!Allclubtest(uident)){
				prints("    \033[1;32m  ²»Òª²ĞÉ±ÎŞ¹¼£¡\033[m");
				pressanykey();
				break;
			}*/
			if(seek_in_file(DIR_MC "mingren", uident)){
				showAt(7, 4, "      ËûÓĞ»ÆÂí¹Ó£¬Äã»¹ÊÇËãÁË°É\n", 1);
				break;
			}
			if (!clubtest("Beggar")) {
				showAt(7, 4, "ÔõÃ´¿´ÄãÒ²²»ÏñÊÇØ¤°ïµÄ°¡£¡", 1);
				break;
			}
			if (lookupuser.dietime>0) {
				showAt(7, 4, "ÈË¶¼ËÀÁË,ÈÃËû°²Ï¢°É.", 1);
				break;
			}
			money = loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
			credit = loadValue(uident, CREDIT_NAME, MAX_MONEY_NUM);
			int flag;
			if (money/2>credit/9){
				num=random() % (money/2);
		    		flag=1;
			}else{
				num=credit/9;
			if(num>money)
				num=random() % (num/2);
		    		flag=0;
			}
		   	if(num>500000)
				num=random() % 500000;
			/*if (money >= 100 || credit >= 100) {
				prints("Äã»¹ÓĞÇ®£¬¸ÉÂïÆòÌÖ£¿");
				pressanykey();
				break;
			}*/
			if (loadValue(currentuser.userid, "begtime", 2000000000) >=12) {
				if(time(0) > 24*3600 + loadValue(currentuser.userid, "last_beg", 2000000000)){
				  	saveValue(currentuser.userid, "begtime", -12, 2000000000);
					saveValue(currentuser.userid, "last_beg", time(0), 2000000000);
					saveValue(currentuser.userid, "begtime", +1, 2000000000);
				}else
					prints("%sÅ­²»¿É¶ô£¬³åÄãÂîµÀ£º¡°³ôÒª·¹µÄ£¬·³ËÀÁË£¬»¹²»¿ì¹ö£¡¡±", uident);
					pressanykey();
					break;
				}
				saveValue(currentuser.userid, "begtime", +1, 2000000000);
				if (!t_search(uident, NA, 1)) {
					if (random() % 5 == 0) {
						prints("Äã¶Ô×Å%s¿Şº°µÀ£º¡°¿ÉÁ¯¿ÉÁ¯ÎÒ°É£¬»¹ÓĞÎÒµÄĞ¡Ç¿£¡ÎØÎØÎØ...¡±",
							 uident);
						//num = (random() % (1 + 100))*10000 + 500000;
						if(flag==1)
							saveValue(uident, MONEY_NAME, -num, MAX_MONEY_NUM);
						else
							saveValue(uident, CREDIT_NAME, -num, MAX_MONEY_NUM);
						saveValue(currentuser.userid, MONEY_NAME, num, MAX_MONEY_NUM);

						sprintf(title, "%s²ÎÓëØ¤°ï»î¶¯", currentuser.userid);
						sprintf(buf, "%sÆòÌÖÁË%s %d±øÂíÙ¸±Ò", currentuser.userid, uident, num);
						if (num != 0)
							millionairesrec(title, buf, "Ø¤°ï»î¶¯");

						move(8, 4);
						prints
							("%sÑÛÈ¦¶ÙÊ±ºìÁË£¬¸Ï½ô´ÓÉíÉÏÄÃ³ö %d ±øÂíÙ¸±Ò¸øÄã¡£",
							 uident, num);
						sprintf(genbuf,
							"ÄãÒ»Ê±ºÃĞÄ£¬¸øÁË%s %d±øÂíÙ¸±Ò£¬¹ıºóÏëÏëÕæ²»ÊÇ×ÌÎ¶¡£",
							currentuser.userid, num);
						if(Allclubtest(uident)||loadValue(uident, "mail", 8))
						mail_buf(genbuf, uident, "ÄãÓöµ½½Ğ»¨×Ó");
						pressanykey();
					} else {
						prints("Äã¶Ô×Å%s¿ŞÆüµÀ£º¡°¹ÙÈË£¬ÎÒÒª£¡¡±", uident);
						move(8, 4);
						prints("%sÒ»½Å°ÑÄãõßÁË³öÀ´¡£", uident);
						pressanykey();
					}
				/*prints("%s²»ÔÚ¼Ò£¬ÄãÇÃÁË°ëÌìÃÅÒ²Ã»ÈËÓ¦¡£",
				       uident);
				pressanykey();
				break;*/
			}

			else {
				int begmoney= loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
				if (seek_in_file(DIR_MC "gongji", uident)){
					if(random() % 3 == 0){
						saveValue(uident, MONEY_NAME, -begmoney, MAX_MONEY_NUM);
						saveValue(currentuser.userid, MONEY_NAME, begmoney, MAX_MONEY_NUM);

						sprintf(title, "%s²ÎÓëØ¤°ï»î¶¯", currentuser.userid);
						sprintf(buf, "%sÆòÌÖÁË%s %d±øÂíÙ¸±Ò", currentuser.userid, uident, begmoney);
						if (begmoney != 0)
							millionairesrec(title, buf, "Ø¤°ï»î¶¯");

						prints
							("%sÑÛÈ¦¶ÙÊ±ºìÁË£¬¸Ï½ô´ÓÉíÉÏÄÃ³öËùÓĞµÄ±øÂíÙ¸±ÒÒ»¹² %d ¸øÄã¡£",
							 uident, num);
						sprintf(genbuf,
							"ÄãÒ»Ê±ºÃĞÄ£¬¸øÁË%s %d±øÂíÙ¸±Ò£¬¹ıºóÏëÏëÕæ²»ÊÇ×ÌÎ¶¡£",
							currentuser.userid, num);
						if(Allclubtest(uident)||loadValue(uident, "mail", 8))
						mail_buf(genbuf, uident, "ÄãÓöµ½½Ğ»¨×Ó");
						pressanykey();
					}
			  	}

				if (random() % 3 == 0) {
					prints
					    ("Äã¶Ô×Å%s¿Şº°µÀ£º¡°¿ÉÁ¯¿ÉÁ¯ÎÒ°É£¬»¹ÓĞÎÒµÄĞ¡Ç¿£¡ÎØÎØÎØ...¡±",
					     uident);
					//num = (random() % (1 + 100))*10000 + 500000;
					if(flag==1)
						saveValue(uident, MONEY_NAME, -num, MAX_MONEY_NUM);
					else
						saveValue(uident, CREDIT_NAME, -num, MAX_MONEY_NUM);
					saveValue(currentuser.userid, MONEY_NAME, num, MAX_MONEY_NUM);

					sprintf(title, "%s²ÎÓëØ¤°ï»î¶¯", currentuser.userid);
					sprintf(buf, "%sÆòÌÖÁË%s %d±øÂíÙ¸±Ò", currentuser.userid, uident, num);
					if (num != 0)
						millionairesrec(title, buf, "Ø¤°ï»î¶¯");

					move(8, 4);
					prints
					    ("%sÑÛÈ¦¶ÙÊ±ºìÁË£¬¸Ï½ô´ÓÉíÉÏÄÃ³ö %d ±øÂíÙ¸±Ò¸øÄã¡£",
					     uident, num);
					sprintf(genbuf,
						"ÄãÒ»Ê±ºÃĞÄ£¬¸øÁË%s %d±øÂíÙ¸±Ò£¬¹ıºóÏëÏëÕæ²»ÊÇ×ÌÎ¶¡£",
						currentuser.userid, num);
					if(Allclubtest(uident)||loadValue(uident, "mail", 8))
					mail_buf(genbuf, uident, "ÄãÓöµ½½Ğ»¨×Ó");
					pressanykey();
				} else {
					prints("Äã¶Ô×Å%s¿ŞÆüµÀ£º¡°¹ÙÈË£¬ÎÒÒª£¡¡±", uident);
					move(8, 4);
					prints("%sÒ»½Å°ÑÄãõßÁË³öÀ´¡£", uident);
					pressanykey();
				}
			}
			break;
			case '5':
				nomoney_show_stat("Ø¤°ï°ïÖ÷°ì¹«ÊÒ");
				whoTakeCharge2(5, buf);
				whoTakeCharge(5, uident);
				if (strcmp(currentuser.userid, uident)) {
					move(6, 4);
					prints
				  	  ("ÃØÊé%sÀ¹×¡ÁËÄã,ËµµÀ:¡°ÀÏ´ó%sÏÖÔÚºÜÃ¦,Ã»Ê±¼ä½Ó´ıÄã¡£¡±", buf,uident);
					move(8,4);
					if(!slowclubtest("Beggar",currentuser.userid)){
					if (askyn("ÄãÊÇÏë¼ÓÈëØ¤°ïÂğ£¿", NA, NA) == YEA) {
						sprintf(genbuf, "%s Òª¼ÓÈëØ¤°ï", currentuser.userid);
						mail_buf(genbuf, "BMYbeg", genbuf);
						move(14, 4);
						prints("ºÃÁË£¬ÎÒ»áÍ¨ÖªÀÏ´óµÄ");
					}}
					pressanykey();
					break;
				} else {
					move(6, 4);
					prints("ÇëÑ¡Ôñ²Ù×÷´úºÅ:");
					move(7, 6);
					prints("5. ´ÇÖ°                      6. ÍË³ö");
					ch2 = igetkey();
					switch (ch2) {
					case '5':
						move(12, 4);
						if (askyn("ÄúÕæµÄÒª´ÇÖ°Âğ£¿", NA,NA) == YEA) {
						/*	del_from_file(MC_BOSS_FILE,"beggar");
							sprintf(genbuf, "%s Ğû²¼´ÇÈ¥Ø¤°ï°ïÖ÷Ö°Îñ", currentuser.userid);
							deliverreport(genbuf,
							      "±øÂíÙ¸½ğÈÚÖĞĞÄ¶ÔÆäÒ»Ö±ÒÔÀ´µÄ¹¤×÷±íÊ¾¸ĞĞ»£¬×£ÒÔºóË³Àû£¡");
							move(14, 4);
							prints
							    ("ºÃ°É£¬¼ÈÈ»ÄãÒâÒÑ¾ö£¬ÖĞĞÄÒ²Ö»ÓĞÅú×¼¡£");
							quit = 1;
							pressanykey();
						*/
						sprintf(genbuf, "%s Òª´ÇÈ¥Ø¤°ï°ïÖ÷Ö°Îñ",
							currentuser.userid);
						mail_buf(genbuf, "millionaires", genbuf);
						move(14, 4);
						prints("ºÃ°É£¬ÒÑ¾­·¢ĞÅ¸æÖª×Ü¹ÜÁË");
						pressanykey();
						}
						break;
					}
				}
				break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

static int/*É±ÊÖrewrite by macintosh 20051204*/
money_killer()
{
	int ch,ch2;
	int guard_num;
	int robTimes;
	int x,y;
	int quit = 0;
	int quit2=0;
	int count=0;
	int freeTime;
	int currentTime = time(0);
	char uident[IDLEN + 1], name[IDLEN + 1], buf[STRLEN];
	int money,num;
	int id;
	char c4_price[10];
	int price;
	while (!quit) {
		quit2=0;
		nomoney_show_stat("É±ÊÖÌì¿Õ");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]¹ÍÓ¶É±ÊÖ [2]¾ü»ğ [3]É±ÊÖ°ïÖ÷ [Q]Àë¿ª\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			if (seek_in_file(DIR_MC "chayou", currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  ²èÓÑ²»ÒªÈÇÊÂ\033[m", 1);
				break;
			}
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
				showAt(5, 4, "    \033[1;32m  »ÆÂí¹Ó²»ÒªÈÇÊÂ\033[m", 1);
				break;
			}
			money_show_stat("É±ÊÖÖ®¼Ò");
			readstrvalue(MC_PRICE_FILE, "c4_price", c4_price, 10);
			price = atoi(c4_price);
			if (price==0)
				price=300000;
			move(4, 4);
			prints("ÎÒÃÇÕâÀïÉ±Ò»´Î %d ±øÂíÙ¸±Ò¡£", price);
			move(5, 4);
			prints("Ã¿¸öÈËÃ¿ÌìÖ»ÄÜÉ±Ò»´Î£¬Ã¿´Î×î¶à¿ÉÒÔÂòÉ±ËûÈı´Î¡£");
			if (currentuser.dietime > 0) {
				showAt(7, 4, "ÄãÒÑ¾­ËÀÁË°¡£¡×¥¹í°¡£¡", 1);
				Q_Goodbye();
				break;
			}
			move(6, 4);
			usercomplete("ÄãÒªÉ±Ë­:", uident);
			if (uident[0] == '\0')
				break;
			if (!(id = getuser(uident))) {
				showAt(7, 4, "´íÎóµÄÊ¹ÓÃÕß´úºÅ...", 2);
				break;
			}
			if (lookupuser.dietime > 0) {
				showAt(7, 4, "ËÀÈËÄãÒ²²»·Å¹ı£¬Ì«ºİÁË°É£¿", 1);
				break;
			}
			if(seek_in_file(DIR_MC "mingren", uident)){
				showAt(7, 4, "ËûÓĞ»ÆÂí¹Ó£¬Äã»¹ÊÇËãÁË°É", 1);
				break;
			}
		       if(!Allclubtest(uident)){
			   	showAt(7, 4, "É±ÊÖ²»É±ÎŞ¹¼°ÙĞÕ...", 1);
				break;
			}
 			getdata(7, 4, "ÄãÒªÉ±¼¸´Î£¿ [1-3]", genbuf, 2, DOECHO, YEA);
			if (genbuf[0] == '\0')
				break;
			count = atoi(genbuf);
			if (count < 1) {
				showAt(8, 4, "ºİ²»ÏÂĞÄ¶¯ÊÖÁË£¿", 1);
				break;
			}
			if (count > 3) {
				move(8, 4);
				sprintf(genbuf, "ÒªÉ±%d´Î£¿É±ÊÖ×î¶àÉ±3´Î£¬ËûÒª°ÑÇ®Ë½ÍÌÁË", count);
				if (askyn(genbuf, NA, NA) == NA){
					showAt(9, 4, "ÖØĞÂÏëÏë°É£¿", 1);
					break;
				}
			}
			move(8, 4);
			num = count * price;
			sprintf(genbuf, "×Ü¹²ĞèÒª %d ±øÂíÙ¸±Ò¡£", num);
			if (askyn(genbuf, NA, NA) == YEA) {
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				if (money < num) {
					showAt(9, 4, "ÄãµÄÇ®²»¹»...", 1);
					break;
				}
				saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
				saveValue("BMYKillersky", MONEY_NAME, num, MAX_MONEY_NUM);
				sprintf(buf,"%s»¨ÁË%d±øÂíÙ¸±ÒÒªÉ±%s%d´Î",currentuser.userid,num,uident, count);
				mail_buf(buf, "BMYKillersky","[ÈÎÎñ]É±ÊÖÓĞÉúÒâÁË");

				if (seek_in_file(DIR_MC "killerlist", uident)){
					FILE *fp;
					char *ptr;
					int count2=0;
					fp = fopen(DIR_MC "killerlist","r");
					while (fgets(buf,sizeof(buf),fp)) {
						ptr= strstr(buf,uident);
						if(ptr){
							count2 = atoi(ptr+strlen(uident)+1);
							break;
						}
					}
					fclose(fp);
					if (count2+count>3)
						count2 = 3;
					else
						count2 += count;
					del_from_file(DIR_MC "killerlist", uident);
					sprintf(buf, "%s\t%d",uident, count2);
					addtofile(DIR_MC "killerlist",buf);
				}else{
					sprintf(buf, "%s\t%d",uident, (count>3)?3:count);
					addtofile(DIR_MC "killerlist",buf);
				}
				showAt(10, 4, "ÄúÒÑ¾­³É¹¦¹ºÂòÁËÕâ¸öÈËµÄÈËÍ·£¬Çë¾²ºò¼ÑÒô", 1);
			}
			break;

		case '2':
			while (!quit2) {
			nomoney_show_stat("µØÏÂ¾ü»ğ½»Ò×ÊĞ³¡");
			move(t_lines - 1, 0);
		     	prints
			    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]c4  [Q]Àë¿ª\033[m");
			ch2 = igetkey();
			switch (ch2) {
			case 'q':
			case 'Q':
				quit2 = 1;
			  	break;
			case '1':
				if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
					showAt(5, 4, "    \033[1;32m  ²»ÒªÈÇÊÂ\033[m", 1);
					break;
				}
				if (!seek_in_file(DIR_MC "killer", currentuser.userid)
					||!slowclubtest("killer", currentuser.userid)){
					showAt(7, 4, "\033[1;31mÒªÆ´ÃüÈ¥ÕÒÉ±ÊÖ\033[m", 1);
					break;
				}
				if (loadValue(currentuser.userid, "guard", 8) > 0) {
					showAt(7, 4, "Äã×Ü²»ÄÜ´ø×ÅĞÖµÜÒ»ÆğËÀ°É£¬^_^", 1);
					break;
				}
				showAt(4, 4,"\033[1;35mÄã¾ö¶¨·¢¶¯×ÔÉ±Ê½¹¥»÷¡£\033[m", 0);
                		money =loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				if (money <10000) {
					showAt(9, 4, "ÄúµÄÇ®²»¹»...", 1);
					break;
				}
				if (currentuser.dietime > 0) {
					showAt(9, 4, "ÄãÒÑ¾­ËÀÁË°¡£¡×¥¹í°¡£¡", 1);
					Q_Goodbye();
					break;
				}
				usercomplete("ÄãÒªÕ¨Ë­:", uident);
				if (uident[0] == '\0')
					break;
				if (!(id = getuser(uident))) {
					showAt(7, 4,"´íÎóµÄÊ¹ÓÃÕß´úºÅ...", 1);
					break;
				}
				if (lookupuser.dietime > 0) {
					showAt(7, 4,"ËÀÈËÄãÒ²²»·Å¹ı£¬Ì«ºİÁË°É£¿", 1);
					break;
				}
				if (seek_in_file(DIR_MC "mingren", uident)){
					showAt(7, 4, "      ËûÓĞ»ÆÂí¹Ó£¬Äã»¹ÊÇËãÁË°É\n", 1);
					break;
				}
				if (!Allclubtest(uident)){
					showAt(7, 4, "    \033[1;32m  ²»Òª²ĞÉ±ÎŞ¹¼£¡\033[m", 1);
					break;
				}				guard_num =loadValue(uident, "guard", 8);
				if (guard_num > 0) {
					showAt(7, 4, "¶Ô·½ÓĞ±£ïÚ»¤Éí,Äã»¹ÊÇËãÁË°É...", 1);
					break;
				}

				freeTime = loadValue(currentuser.userid, "freeTime", 2000000000);
				if (currentTime < freeTime){
					pressreturn();
					break;
				}
				saveValue(currentuser.userid, MONEY_NAME, -100000, MAX_MONEY_NUM);
				move(6, 4);
				prints
				    ("  \n\033[1;35m  Äã±§ÆğÕ¨Ò©°ü£¬´óº°Ò»Éù´òµ¹Ğ¡ÈÕ±¾,Ïò%s³åÁË¹ıÈ¥\033[m\n", uident);
				sprintf(genbuf, "±¾¸ÛÈËÊ¿%sÓÚ10·ÖÖÓÇ°ÔÚ¾ÅÁúµÄ\nÒ»Æğ×ÔÉ±Ê½¹¥»÷ÖĞÉíÍö\n¾¯·½»³ÒÉ´ËÈËÓĞ°ï»á±³¾°\n\n"
							"¾İÒ»Î»²»Ô¸Í¸Â¶ĞÕÃûµÄ¾¯Êğ¹ÙÔ±Í¸Â¶\nÕâ´ÎÊÂ¼ş¿ÉÄÜÊÇÖ°ÒµÉ±ÊÖËùÎª", uident);
				x = countexp(&currentuser);
				y = countexp(&lookupuser);
				robTimes = loadValue(currentuser.userid, "rob", 50);
				saveValue(currentuser.userid, "rob", -robTimes, 50);
				if(random()/x>(random()/y)/3||(random() % 3==0)){
					lookupuser.dietime = lookupuser.stay + 4500 * 60;
					substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
					deliverreport("[ĞÂÎÅ]±¾Õ¾·¢Éú×ÔÉ±¹¥»÷",genbuf);
					mail_buf_slow(uident,	 "Äã¹ÒÁË","ÓĞÈË¶ÔÄã·¢¶¯ÁË×ÔÉ±Ê½¹¥»÷¡£","BMYKillersky");
					sprintf(buf,"¶Ô %s ·¢¶¯ÁË×ÔÉ±Ê½¹¥»÷",uident);
					mail_buf(buf, "BMYKillersky","ÈÎÎñÍê³É");
					if (seek_in_file(DIR_MC "killerlist", uident)){
						FILE *fp;
						char *ptr;
						int count2=0;
						fp = fopen(DIR_MC "killerlist","r");
						while (fgets(buf,sizeof(buf),fp)) {
							ptr = strstr(buf,uident);
							if(ptr){
								count2 = atoi(ptr+strlen(uident)+1);
								break;
							}
						}
						fclose(fp);
						del_from_file(DIR_MC "killerlist", uident);
						if (count2==2 || count2==3){
							sprintf(buf, "%s\t%d",uident, count2-1);
							addtofile(DIR_MC "killerlist",buf);
						}
					}
				}
				set_safe_record();
				currentuser.dietime = currentuser.stay +1000 * 60;
				substitute_record (PASSFILE, &currentuser, sizeof(currentuser), usernum);
				pressanykey();
				Q_Goodbye();
				}
			limit_cpu();
			}
			break;

			case '3':
				nomoney_show_stat("É±ÊÖ°ïÖ÷°ì¹«ÊÒ");
				whoTakeCharge2(9, name);
				whoTakeCharge(9, uident);
				if (strcmp(currentuser.userid, uident)) {
					move(6, 4);
					prints
				  	  ("ÃØÊé%sÀ¹×¡ÁËÄã,ËµµÀ:¡°ÀÏ´ó%sÏÖÔÚºÜÃ¦,Ã»Ê±¼ä½Ó´ıÄã¡£¡±", name,uident);
					move(8,4);
					if (!seek_in_file(DIR_MC "killer", currentuser.userid) &&
						!slowclubtest("killer",currentuser.userid)){
					if (askyn("ÄãÊÇÏë³ÉÎªÉ±ÊÖÂğ£¿", NA, NA) == YEA) {
						sprintf(genbuf, "%s Òª¼ÓÈëÉ±ÊÖ", currentuser.userid);
						mail_buf(genbuf, "BMYKillersky", genbuf);
						move(14, 4);
						prints("ºÃÁË£¬ÎÒ»áÍ¨ÖªÀÏ´óµÄ");
					}}
					pressanykey();
					break;
				} else {
					move(6, 4);
					prints("ÇëÑ¡Ôñ²Ù×÷´úºÅ:");
					move(7, 6);
					prints("1. ÈÎÃüÉ±ÊÖ                  2. ½âÖ°É±ÊÖ");
					move(8, 6);
					prints("3. É±ÊÖÃûµ¥                  4. ÈÎÎñÃûµ¥");
					move(9, 6);
					prints("5. ´ÇÖ°                      6. c4¶¨¼Û");
					move(10, 6);
					prints("7. ÍË³ö");
					ch2 = igetkey();
					switch (ch2) {
					case '1':
						move(12, 4);
						usercomplete("ÈÎÃüË­ÎªÉ±ÊÖ£¿", uident);
						move(13, 4);
						if (uident[0] == '\0')
							break;
						if (!searchuser(uident)) {
							prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
							pressanykey();
							break;
							}
						if (seek_in_file(DIR_MC "killer", uident)) {
							prints("¸ÃIDÒÑ¾­ÊÇÉ±ÊÖÁË¡£");
							pressanykey();
							break;
							}
						if (askyn("È·¶¨Âğ£¿", NA, NA) == YEA) {
							addtofile(DIR_MC "killer", uident);
							sprintf(genbuf, "%s ÈÎÃüÄãÎªÉ±ÊÖ",currentuser.userid);
							mail_buf("Ï£ÍûÄã²»¹¼¸º´ó¼ÒµÄÏ£Íû£¬Íê³ÉÈÎÎñ£¡",uident, genbuf);
							move(14, 4);
							prints("ÈÎÃü³É¹¦¡£");
							sprintf(genbuf, "%sĞĞÊ¹É±ÊÖ¹ÜÀíÈ¨ÏŞ",currentuser.userid);
							sprintf(buf, "ÈÎÃü%sÎªÉ±ÊÖ", uident);
							millionairesrec(genbuf, buf, "BMYKillersky");
							pressanykey();
							}
						break;
					case '2':
						move(12, 4);
						usercomplete("½âÖ°ÄÄÎ»É±ÊÖ£¿", uident);
						move(13, 4);
						if (uident[0] == '\0')
							break;
						if (!searchuser(uident)) {
							prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
							pressanykey();
							break;
						}
						if (!seek_in_file(DIR_MC "killer", uident)) {
							prints("¸ÃID²»ÊÇÉ±ÊÖ¡£");
							pressanykey();
							break;
							}
						if (askyn("È·¶¨Âğ£¿", NA, NA) == YEA) {
							del_from_file(DIR_MC "killer", uident);
							sprintf(genbuf, "%s ½â³ıÄãµÄÉ±ÊÖÖ°Îñ", currentuser.userid);
							mail_buf("¸ĞĞ»ÄãÍê³ÉÈÎÎñ¡£", uident, genbuf);
							move(14, 4);
							prints("½âÖ°³É¹¦¡£");
							sprintf(genbuf, "%sĞĞÊ¹É±ÊÖ¹ÜÀíÈ¨ÏŞ",currentuser.userid);
							sprintf(buf, "½â³ı%sµÄÉ±ÊÖÖ°Îñ", uident);
							millionairesrec(genbuf, buf, "BMYKillersky");
							pressanykey();
							}
						break;
					case '3':
						clear();
						move(1, 0);
						prints("Ä¿Ç°±øÂíÙ¸É±ÊÖÃûµ¥£º");
						listfilecontent(DIR_MC "killer");
						pressanykey();
						break;
					case '4':
						clear();
						move(1, 0);
						prints("Ä¿Ç°±øÂíÙ¸×·É±Ãûµ¥£º");
						move(2, 0);
						prints("Ä¿±êID\t´ÎÊı");
						listfilecontent(DIR_MC "killerlist");
						pressanykey();
						break;
					case '5':
						move(12, 4);
						if (askyn("ÄúÕæµÄÒª´ÇÖ°Âğ£¿", NA,NA) == YEA) {
						/*	del_from_file(MC_BOSS_FILE,"killer");
							sprintf(genbuf, "%s Ğû²¼´ÇÈ¥É±ÊÖ°ïÖ÷Ö°Îñ", currentuser.userid);
							deliverreport(genbuf,
							      "±øÂíÙ¸½ğÈÚÖĞĞÄ¶ÔÆäÒ»Ö±ÒÔÀ´µÄ¹¤×÷±íÊ¾¸ĞĞ»£¬×£ÒÔºóË³Àû£¡");
							move(14, 4);
							prints
							    ("ºÃ°É£¬¼ÈÈ»ÄãÒâÒÑ¾ö£¬ÖĞĞÄÒ²Ö»ÓĞÅú×¼¡£");
							quit = 1;
							pressanykey();
						*/
							sprintf(genbuf, "%s Òª´ÇÈ¥É±ÊÖ°ïÖ÷Ö°Îñ",
								currentuser.userid);
							mail_buf(genbuf, "millionaires", genbuf);
							move(14, 4);
							prints("ºÃ°É£¬ÒÑ¾­·¢ĞÅ¸æÖª×Ü¹ÜÁË");
							pressanykey();
						}
						break;
					case '6':
						move(12, 4);
						readstrvalue(MC_PRICE_FILE, "c4_price", c4_price, 10);
						price = atoi(c4_price);
						prints("ÏÖÔÚµÄ¼Û¸ñÊÇ%d", price ? price : 300000);
						getdata(13, 4, "Éè¶¨ĞÂµÄ¼Û¸ñ: ", buf, 10, DOECHO, YEA);
						move(14, 4);
						sprintf(genbuf, "ĞÂµÄ¼Û¸ñÊÇ %d£¬È·¶¨Âğ£¿", atoi(buf));
						if (askyn(genbuf, NA, NA) == YEA) {
							if (atoi(buf)>MAX_MONEY_NUM){
								move(15, 4);
								prints("²»ÒªÌ«ºİÁË...");
								pressanykey();
								sprintf(buf, "%d", MAX_MONEY_NUM);
							}
							savestrvalue(MC_PRICE_FILE, "c4_price", buf);
							move(15, 4);
							prints("ÉèÖÃÍê±Ï¡£    ");
							sprintf(genbuf, "ÉèÖÃc4¼Û¸ñÎª%s¡£", buf);
							sprintf(buf, "%sĞĞÊ¹É±ÊÖ¹ÜÀíÈ¨ÏŞ", currentuser.userid);
							millionairesrec(buf, genbuf, "BMYKillersky");
							pressanykey();
						}
						break;
					}
				}
				break;

			case 'q':
			case 'Q':
				quit = 1;
				break;
		}
		limit_cpu();
	}
	return 0;
}

static int
money_postoffice()
{
	int ch2, slownum=0;

	nomoney_show_stat("´ó¸»ÎÌÓÊ¼şÉèÖÃ");
	slownum=loadValue(currentuser.userid, "mail", 8);
	move(6, 4);
	if(Allclubtest(currentuser.userid)){
		prints("°ïÅÉÈËÊ¿¾Í²»Òª¹ÜÕâÃ´¶àÁË!");
		pressanykey();
		return 0;
	}
	if (slownum==0){
		prints("ÄúÉĞÎ´¿ªÍ¨±øÂíÙ¸ÓÊ¾ÖÓÊ¼ş·şÎñ£¬²»ÄÜÊÕµ½¸÷´ó°ïÅÉ¸øÄúµÄĞÅ¼ş¡£");
		move(t_lines - 1, 0);
		prints("\033[1;44m Ñ¡µ¥ \033[1;46m [1]¿ªÍ¨·şÎñ [Q]Àë¿ª\033[m");
	}
	else{
		prints("ÄúÒÑ¾­ÆôÓÃÁË±øÂíÙ¸ÓÊ¾ÖµÄÓÊ¼ş·şÎñ£¬ÎÒÃÇ½«ÔÚµÚÒ»Ê±¼ä½«¸÷´ó°ïÅÉ");
		move(7, 4);
		prints("¸øÄúµÄĞÅ¼şµİ¸øÄú¡£");
		move(t_lines - 1, 0);
		prints("\033[1;44m Ñ¡µ¥ \033[1;46m [1]È¡Ïû·şÎñ [Q]Àë¿ª\033[m");
	}
	ch2 = igetkey();
	switch (ch2) {
	  	case '1':
	    		if(slownum==0){
				saveValue(currentuser.userid, "mail", 1, 50);
				nomoney_show_stat("´ó¸»ÎÌÓÊ¼şÉèÖÃ");
				move(6, 4);
				prints("»¶Ó­Ê¹ÓÃ±øÂíÙ¸ÓÊ¾ÖÓÊ¼ş·şÎñÏµÍ³£¬ÎÒÃÇ½«ÔÚµÚÒ»Ê±¼ä½«¸÷´ó°ïÅÉ");
				move(7, 4);
				prints("¸øÄúµÄĞÅ¼şµİµ½ÄúµÄĞÅÏä¡£ÔÙ¼û¡£");
	    		}else{
				saveValue(currentuser.userid, "mail", -slownum, 50);
				nomoney_show_stat("´ó¸»ÎÌÓÊ¼şÉèÖÃ");
				move(6,4);
				prints("»¶Ó­ÄúÏÂ´Î¼ÌĞøÊ¹ÓÃ±¾ÓÊ¾ÖµÄ¸÷Ïî·şÎñ£¬Ğ»Ğ»ÄúµÄ¹â¹Ë£¬ÔÙ¼û¡£");
	    		}
			pressanykey();
			break;

		case 'q':
		case 'Q':
	    	 	break;
		}
	return 0;
}


static int /*ÉÌ³¡rewrite by macintosh 20051204*/
money_shop()
{
	int ch, money, num, ch2;
	int guard_num;
	char uident[IDLEN + 1], ticket_price[10], buf[STRLEN];
	int quit = 0, quit2= 0, price=0;

	while (!quit) {
		quit2=0;
		nomoney_show_stat("±øÂíÙ¸ÉÌ³¡");
		move(6, 4);
		prints("±øÂíÙ¸ÉÌ³¡×î½üÉúÒâºì»ğ£¬´ó¼Ò¾¡ĞË£¡");
		move(t_lines - 1, 0);
		prints
	    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]¹Í±£ïÚ [2]ÀñÆ·µê [3]¾­ÀíÊÒ [4]ÓÊ¾Ö [6]»ğ³µÆ±¼Û¼ÆËã [Q]Àë¿ª\033[m");
		   // ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]¹Í±£ïÚ [2]ºØ¿¨ [4]¾­ÀíÊÒ [5]hell²Î¹Û [Q]Àë¿ª\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			nomoney_show_stat("±øÂíÙ¸±£ïÚ¹«Ë¾");
			move(4, 4);
			prints
			    ("±øÂíÙ¸±£ïÚ¹«Ë¾¶ÔÓĞĞèÒªµÄÈËÊ¿Ìá¹©±£ïÚÒµÎñ,¼Û¸ñÊÓÇé¿ö¶ø¶¨¡£");
			move(5, 4);
			prints
			    ("µ«ÊÇ±»±£»¤¶ÔÏóÒ»µ©Îª¶ñ,±£ïÚ×Ô¶¯Àë¿ª,²¢¿ÉÄÜ»á¶Ô¹ÍÖ÷½øĞĞºÚ³ÔºÚÅ¶£¡");
			move(7, 4);
			sprintf(genbuf, "ÄãÈ·¶¨Òª¹Í±£ïÚÃ´?");
			if (askyn(genbuf, NA, NA) == YEA) {
				money =
				    loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				move(8, 4);
				if (money < 10000) {
					prints
					    ("Äã»¹ÊÇÊ¡Ê¡°É£¬Ã»ÈË»á´òÄãÖ÷ÒâµÄ¡£¾ÍÄÇÃ´µãÇ®...");
					pressanykey();
					break;
				}
				guard_num =(countexp(&currentuser) / 1000) + 1 >
				    8 ? 8 : (countexp(&currentuser) / 1000) + 1;
				prints
				    ("°´ÕÕÄúÄ¿Ç°µÄÉí·İµØÎ»£¬¹ÍÓ¶%d¸ö±£ïÚ¾Í¹»ÁË¡£",
				     guard_num);
				saveValue(currentuser.userid, MONEY_NAME, -money / 20, MAX_MONEY_NUM);
				move(9, 4);
				if (loadValue(currentuser.userid, "rob", 50) > 0) {
					prints
					    ("ºÙºÙ£¬ÄãÓĞ°¸µ×£¡ÄîÔÚÊÕÁËÄãÇ®µÄ·İÉÏ£¬¸Ï½ôÅÜÂ·°É...");
					pressanykey();
					break;
				}
				if (loadValue(currentuser.userid, "guard", 8) > 0) {
					prints
					    ("ÄãÒÑ¾­ÓĞ±£ïÚÁË¡£Ç®ÎÒÃÇÊÕÏÂ£¬±£ïÚ²»ÄÜÔÙ¸øÁË£¬^_^");
					pressanykey();
				} else {
					saveValue(currentuser.userid, "guard", guard_num, 50);
					prints
					    ("¹ÍÓ¶±£ïÚ³É¹¦,Äã¿ÉÒÔÓĞÒ»¶ÎÊ±¼ä°²ÏíÌ«Æ½ÁË¡£");
					pressanykey();
				}
			}
			break;

		case '2':
			while (!quit2) {
				nomoney_show_stat("±øÂíÙ¸ÀñÆ·µê");
				move(6, 4);
				//prints ("»¶Ó­¹âÁÙ±øÂíÙ¸ÀñÆ·µê£¡");
				prints("±¾µêASCII×÷Æ·¾ù·Ç±¾µêÖÆ×÷£¬²¿·Ö×÷Æ·ÓÉÓÚÖÖÖÖÔ­Òò£¬Î´ÄÜ±êÃ÷×÷Õß¡£\n"
					"    Èç×÷Æ·´´×÷Õß¶ÔÆä×÷Æ·ÓÃÓÚ±¾µê³ÖÓĞÒìÒé£¬ÇëÓë±¾Õ¾´ó¸»ÎÌ×Ü¹ÜÁªÏµ¡£\n"
					"    ±¾Õ¾½«¼°Ê±¸ù¾İ×÷ÕßÒâÔ¸×÷³öµ÷Õû¡£\n\n"
					"                                                   \33[1;32m±øÂíÙ¸ÀñÆ·µê\033[0m\n");
				move(t_lines - 1, 0);
				prints
				    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]ÏÊ»¨ [2]ºØ¿¨ [Q]Àë¿ª\033[m");
				ch2 = igetkey();
				switch (ch2) {
					case 'q':
					case 'Q':
						quit2=1;
						break;
					case '1':
						shop_present(1, "ÏÊ»¨", NULL);
						break;
					case '2':
						shop_present(2, "ºØ¿¨", NULL);
						break;
					}
				limit_cpu();
				}
			break;

		case '3':
			nomoney_show_stat("±øÂíÙ¸ÉÌ³¡¾­ÀíÊÒ");
			whoTakeCharge(7, uident);
			char name[20];
			whoTakeCharge2(7, name);
			if (strcmp(currentuser.userid, uident)) {
				move(6, 4);
				prints
				    ("Öµ°àÃØÊé%s½Ğ×¡ÁËÄã£¬ËµµÀ:¡°¾­Àí%sÕıÔÚ¿ª»á£¬ÓĞÊ²Ã´ÊÂ¸úÎÒËµÒ²ĞĞ¡£¡±",
				     name,uident);
				pressanykey();
				break;
			} else {
					move(6, 4);
					prints("ÇëÑ¡Ôñ²Ù×÷´úºÅ:");
					move(9, 6);
					prints("5. ´ÇÖ°                      6. ËãÆ±¼Û¶¨¼Û");
					move(10, 6);
					prints("7. ÍË³ö");
					ch2 = igetkey();
					switch (ch2) {
					case '5':
						move(12, 4);
						if (askyn("ÄúÕæµÄÒª´ÇÖ°Âğ£¿", NA,NA) == YEA) {
							sprintf(genbuf, "%s Òª´ÇÈ¥ÉÌ³¡¾­ÀíÖ°Îñ",
								currentuser.userid);
							mail_buf(genbuf, "millionaires", genbuf);
							move(14, 4);
							prints("ºÃ°É£¬ÒÑ¾­·¢ĞÅ¸æÖª×Ü¹ÜÁË");
							pressanykey();
						}
						break;
					case '6':
						move(12, 4);
						readstrvalue(MC_PRICE_FILE, "ticket_price", ticket_price, 10);
						price = atoi(ticket_price);
						prints("ÏÖÔÚµÄ¼Û¸ñÊÇ%d", price);
						getdata(13, 4, "Éè¶¨ĞÂµÄ¼Û¸ñ: ", ticket_price, 10, DOECHO, YEA);
						move(14, 4);
						sprintf(genbuf, "ĞÂµÄ¼Û¸ñÊÇ %d£¬È·¶¨Âğ£¿", atoi(ticket_price));
						if (askyn(genbuf, NA, NA) == YEA) {
							if (atoi(ticket_price)>MAX_MONEY_NUM){
								move(15, 4);
								prints("²»ÒªÌ«ºİÁË...");
								pressanykey();
								sprintf(ticket_price, "%d", MAX_MONEY_NUM);
							}
							savestrvalue(MC_PRICE_FILE, "ticket_price", ticket_price);
							move(15, 4);
							prints("ÉèÖÃÍê±Ï¡£    ");
							sprintf(genbuf, "ÉèÖÃËãÆ±¼Û¼Û¸ñÎª%s¡£", ticket_price);
							sprintf(buf, "%sĞĞÊ¹ÉÌ³¡¾­Àí¹ÜÀíÈ¨ÏŞ", currentuser.userid);
							millionairesrec(buf, genbuf, "");
							pressanykey();
						}
						break;
					}
				}
				break;

		/*case '5':
			 sprintf(genbuf, "ÕæµÄÒªÈ¥hell");
			move(11, 4);
			if (askyn(genbuf, NA, NA) == YEA){
				set_safe_record();
				currentuser.dietime = currentuser.stay + 1;
				substitute_record(PASSFILE,&currentuser,sizeof(currentuser),usernum);
				pressanykey();
				Q_Goodbye();
			}
			break;
		*/

		case '6':
			money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
			readstrvalue(MC_PRICE_FILE, "ticket_price", ticket_price, 10);
			num = atoi(ticket_price);
			clear();
			move(5, 4);
			if (askyn("±¾·şÎñÊÕ·Ñ£¬È·¶¨ÒªËãÂğ? ", YEA, NA) == YEA){
				if (money < num) {
					move(9, 4);
					prints("¶Ô²»Æğ£¬ÄúµÄ½ğ¶î²»×ã¡£");
					pressanykey();
					break;
				}else{
					saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
					//calc_ticket_price();
				}
			}
			break;

		case '4':
			money_postoffice();
			break;

		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}


static int/*¹ÉÆ±ÏµÍ³*/
money_stock()
{
//      moneycenter_welcome();
	int quit = 0;
	char ch;

	while (!quit) {
		clear();
		money_show_stat("±øÂíÙ¸¹ÉÊĞ");

		if (utmpshm->ave_score == 0) {
			clear();
			move(7, 10);
			prints("\033[1;31m±øÂíÙ¸¹ÉÊĞ½ñÌìĞİÊĞ\033[0m");
			pressanykey();
			return 0;
		}


		move(4, 4);
		prints("ÇëÈ·ÈÏÄãÒÑ¾­ÔÚ"MC_BOARD"°æÔÄ¶Á¹ı±øÂíÙ¸¹ÉÊĞ¹æÔò¡£");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]´«Í³°å¿é [2]ÍÆ¼ö°å¿é [3]Ö¤¼à»áÖ÷Ï¯°ì¹«ÊÒ [Q]Àë¿ª   \033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			money_stock_board();
			break;
		case '2':
			clear();
			move(7, 10);
			prints("\033[1;32mÉĞÎ´¿ª·Å\033[0m");
			pressanykey();
			break;
		case '3':
			stockboards();
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}
	return 0;
}

static int/*¹ÉÆ±ÏµÍ³*/
money_stock_board()
{
	char stockname[STRLEN][MAX_STOCK_NUM];
	char stockboard[STRLEN][MAX_STOCK_NUM];
	int ch, i, j, quit = 0, money, count, count1;
	int stock_num[MAX_STOCK_NUM], addto_num[MAX_STOCK_NUM],
	    stock_board[MAX_STOCK_NUM];
	int stock_price[MAX_STOCK_NUM];
	int total_money = 0, temp_sum = 0, total_sum = 0;
	char slow[IDLEN + 1];
	char uident[IDLEN + 1];
	char title[80];
	char buf[200];
	int getnum=0;
	FILE *fp1;

	fp1 = fopen( MC_STOCK_BOARDS, "r" );
	count1= count = listfilecontent(MC_STOCK_BOARDS);
	clear();
	if (count==0){
		move(7, 10);
		prints("\033[1;32m±øÂíÙ¸¹ÉÊĞÉĞÎ´¿ªÅÌ\033[0m");
		pressanykey();
		return 0;
	}
	for (j = 0; j < count; j++)
		fscanf(fp1, "%s", stockboard[j]);
	fclose(fp1);
	for (j = 0; j < count; j++)
		sprintf(stockname[j], "St_%s", stockboard[j]);

	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	clear();
	//count = MAX_STOCK_NUM;

	bzero(&stock_price, sizeof (stock_price));
	bzero(&stock_num, sizeof (stock_num));
	bzero(&addto_num, sizeof (addto_num));
	for (i = 0; i < numboards; i++) {
		for (j = 0; j < count; j++) {
			if (!strcmp(bcache[i].header.filename, stockboard[j])) {
				stock_price[j] = utmpshm->ave_score / 100 + bcache[i].score / 20;
				if (bcache[i].stocknum <= 0) {
					if (bcache[i].score > 10000)
						bcache[i].stocknum = bcache[i].score * 2000;
					else
						bcache[i].stocknum = bcache[i].score * 1000;
				}
				stock_board[j] = i;
				count1--;
				break;
			}
		}
		if (count1 == 0)
			break;
	}//¼ÆËã¹É¼Û
	for (i = 0; i < count; i++) {
		stock_num[i] =
		    loadValue(currentuser.userid, stockname[i], 1000000);
	}//Í³¼Æ×Ô¼ºµÄÊıÁ¿
	//for (i = 0; i < MAX_STOCK_NUM; i++)
	 i=0;
	while(!quit){
		money =
		    loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
		persenal_stock_info(stock_num, stock_price, money, stockboard,
				    stock_board);
		move(t_lines - 1, 0);
		prints("\033[1;44m Ñ¡µ¥ \033[1;46m [B]¹ºÂò [S]³öÊÛ [C]×ªÈÃ [Q]Àë¿ª\033[m");
		ch = igetkey();
		switch (ch){
		case 'B':
		case 'b':
			total_money = 0;
			if (stop_buy()) {
				clear();
				move(7, 10);
				prints("\033[1;31m±øÂíÙ¸¹ÉÊĞÉĞÎ´¿ªÅÌ\033[0m");
				pressanykey();
				break;
			}
			getdata(t_lines - 1, 0, "ÄúÑ¡ÔñÄÄÖ§¹ÉÆ±?[0]", genbuf, 7,
				DOECHO, YEA);
			getnum=atoi(genbuf);
			if(getnum<0||getnum>count-1)
				break; //·Ç·¨ÊäÈë
			else
				i=getnum;
			if (seek_in_file(MC_STOCK_STOPBUY, stockboard[i])){
				move(t_lines - 2, 0);
				prints("±¾Ö§¹ÉÆ±ÒÑ±»ÔİÍ£½»Ò×!");
				pressanykey();
				break;
			}
			getdata(t_lines - 1, 0, "ÄúÒªÂò¶àÉÙ¹É?[0]", genbuf, 7,
				DOECHO, YEA);

			addto_num[i] = atoi(genbuf);
			if (!genbuf[0])
				addto_num[i] = 0;
			//addto_num[i] = abs(addto_num[i]);
			if (addto_num[i] <= 0){
				move(t_lines - 2, 0);
				prints("µ½µ×ÊÇÒªÂò»¹ÊÇÂô...");
				pressanykey();
				break;
			}
			stock_num[i] =
			    loadValue(currentuser.userid, stockname[i], 1000000);
			if (stock_num[i] >= 1000000) {
				move(t_lines - 2, 0);
				prints("ÄãÒÑ¾­ÓĞºÜ¶à¹ÉÆ±ÁË,²»ÒªÔÙÂòÁË");
				pressanykey();
				break;
			}
			if (bcache[stock_board[i]].stocknum <= 50000) {
				move(t_lines - 2, 0);
				prints("¶Ô²»Æğ,´Ë¹ÉÄ¿Ç°Ã»ÓĞ¿ÉÒÔ³öÊÛµÄ¹ÉÆ±!");
				pressanykey();
				break;
			}
			if (stock_num[i] + addto_num[i] > 1000000){
				addto_num[i] = 1000000 - stock_num[i];
				move(t_lines - 2, 0);
				prints("¶Ô²»Æğ,ÄãÒÑ¾­ÓĞºÜ¶à¹ÉÆ±ÁË!");
				pressanykey();
			}
			if (bcache[stock_board[i]].stocknum - addto_num[i] < 50000){
				addto_num[i] = bcache[stock_board[i]].stocknum - 50000;
				move(t_lines - 2, 0);
				prints("¶Ô²»Æğ,´Ë¹ÉÄ¿Ç°Ã»ÓĞÄÇÃ´¶à¹ÉÆ±³öÊÛ!");
				pressanykey();
			}
			move(t_lines - 2, 0);
			sprintf(genbuf, "È·¶¨¹ºÂò %d ¹É %s Âğ£¿",
				addto_num[i], stockname[i]);
			if (askyn(genbuf, NA, NA) == YEA) {
				temp_sum = addto_num[i] * stock_price[i];
				total_money += temp_sum;
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				if (money - temp_sum < 0) {
					total_money -= addto_num[i] * stock_price[i];
					addto_num[i] = 0;
					move(t_lines - 2, 0);
					prints("Äãµ±Ç°µÄ±øÂíÙ¸±Ò²»¹»Íê³É´ËÏî²Ù×÷!");
					pressanykey();
					break;
				}
				bcache[stock_board[i]].stocknum -= addto_num[i];
				saveValue(currentuser.userid, MONEY_NAME, -temp_sum, MAX_MONEY_NUM);
				stock_num[i] += addto_num[i];
				saveValue(currentuser.userid, stockname[i],
					  addto_num[i], 1000000);
				if (addto_num[i]>0){
					sprintf(genbuf, "%s½øĞĞ¹ÉÆ±½»Ò×(ÂòÈë)", currentuser.userid);
					sprintf(buf,"%s¹ºÂòÁË%d¹É%s¹ÉÆ±(Ã¿¹É%d±øÂíÙ¸±Ò)£¬»¨·Ñ%d±øÂíÙ¸±Ò\n",
						currentuser.userid, addto_num[i], stockname[i], stock_price[i], temp_sum);
					millionairesrec(genbuf, buf, "¹ÉÆ±½»Ò×");
					sprintf(buf,"Äú¹ºÂòÁË%d¹É%s¹ÉÆ±£¬³É½»¼Û%d±øÂíÙ¸±ÒÃ¿¹É£¬»¨·Ñ%d±øÂíÙ¸±Ò¡£\n",
						addto_num[i], stockname[i], stock_price[i], temp_sum);
					sprintf(title,"¹ÉÆ±¹ºÂòÆ¾Ö¤");
					mail_buf(buf, currentuser.userid, title);
					total_sum -= temp_sum;
					sprintf(genbuf, "Äã»¨µôÁË%d±øÂíÙ¸±Ò", temp_sum);
					move(t_lines - 2, 0);
					clrtoeol();
					prints("%s", genbuf);
					pressanykey();
				}
			}
			sleep(1);
			break;
		case 'S':
		case 's':
			total_money= 0;
			if (stop_buy()) {
				clear();
				move(7, 10);
				prints("\033[1;31m±øÂíÙ¸¹ÉÊĞÉĞÎ´¿ªÅÌ\033[0m");
				pressanykey();
				break;
			}
			getdata(t_lines - 1, 0, "ÄúÑ¡ÔñÄÄÖ§¹ÉÆ±?[0]", genbuf, 7,
				DOECHO, YEA);
			getnum=atoi(genbuf);
			if(getnum<0||getnum>count-1)
				break; //·Ç·¨ÊäÈë
			else
				i=getnum;
			if (seek_in_file(MC_STOCK_STOPBUY, stockboard[i])){
				move(t_lines - 2, 0);
				prints("±¾Ö§¹ÉÆ±ÒÑ±»ÔİÍ£½»Ò×!");
				pressanykey();
				break;
			}

			getdata(t_lines - 1, 0, "ÄúÒªÂô¶àÉÙ¹É?[0]", genbuf, 7,
				DOECHO, YEA);
			stock_num[i] =
			    loadValue(currentuser.userid, stockname[i], 1000000);
			addto_num[i] = atoi(genbuf);
			if (!genbuf[0])
				addto_num[i] = 0;
			//addto_num[i] = abs(addto_num[i]);
			if (addto_num[i] <= 0){
				move(t_lines - 2, 0);
				prints("µ½µ×ÊÇÒªÂò»¹ÊÇÂô...");
				pressanykey();
				break;
			}
			/*
			if (stock_num[i] - addto_num[i] < 0)
				addto_num[i] = stock_num[i];
			*/
			if (stock_num[i] < addto_num[i]) {
				move(t_lines - 2, 0);
				prints
				    ("ÄãÃ»ÓĞÕâÃ´¶à¹ÉÆ±°¡...ÊÇÄã·¸ÔÎ»¹ÊÇÎÒ·¸ÔÎ?");
				pressanykey();
				break;
			}
			move(t_lines - 2, 0);
			sprintf(genbuf, "È·¶¨³öÊÛ %d ¹É %s Âğ£¿",
				addto_num[i], stockname[i]);
			if (askyn(genbuf, NA, NA) == YEA) {
				addto_num[i] *= -1;
				temp_sum = addto_num[i] * stock_price[i];
				stock_num[i] += addto_num[i];
				saveValue(currentuser.userid, MONEY_NAME, temp_sum/100-temp_sum,
									  MAX_MONEY_NUM);
				whoTakeCharge(6, slow);//slowaction
	                     saveValue(slow, MONEY_NAME, -temp_sum/100, MAX_MONEY_NUM);
				saveValue(currentuser.userid, stockname[i],
					  addto_num[i], 1000000);
				total_money += temp_sum-temp_sum/100;
				bcache[stock_board[i]].stocknum -= addto_num[i];
				temp_sum = bcache[stock_board[i]].score;
				if (temp_sum > 10000) {
					if (bcache[stock_board[i]].stocknum > temp_sum * 2000)
						bcache[stock_board[i]].stocknum = temp_sum * 2000;
				} else {
					if (bcache[stock_board[i]].stocknum > temp_sum * 1000)
						bcache[stock_board[i]].stocknum = temp_sum * 1000;
				}
				sprintf(genbuf, "%s½øĞĞ¹ÉÆ±½»Ò×(Âô³ö)", currentuser.userid);
				sprintf(buf,"%sÂô³öÁË%d¹É%s¹ÉÆ±(Ã¿¹É%d±øÂíÙ¸±Ò)£¬»ñµÃ%d±øÂíÙ¸±Ò\n",
					currentuser.userid, -addto_num[i], stockname[i], stock_price[i], -total_money);
				millionairesrec(genbuf, buf, "¹ÉÆ±½»Ò×");
				total_sum -= total_money;
				sprintf(genbuf, "¿Û³ıÊÖĞø·ÑºóÄãÄÃ»ØÁË%d±øÂíÙ¸±Ò", (-1) * total_money);
				move(t_lines - 2, 0);
				clrtoeol();
				prints("%s", genbuf);
				pressanykey();
			}
			sleep(1);
			break;
		case 'c':
		case 'C':
			/*if (stop_buy()) {
				clear();
				move(7, 10);
				prints("\033[1;31m±øÂíÙ¸¹ÉÊĞÉĞÎ´¿ªÅÌ\033[0m");
				pressanykey();
				break;
			}*/
			move(t_lines - 1, 0);
       		usercomplete("×ªÈÃ¹ÉÆ±¸øË­£¿", uident);
			if (uident[0] == '\0')
				return 0;
			if (!getuser(uident)) {
				move(t_lines - 2, 0);
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressreturn();
				return 0;
			}
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
				if (seek_in_file(DIR_MC "jijin", currentuser.userid));
				else if (!seek_in_file(DIR_MC "mingren", uident)) {
					clear();
					move(t_lines - 2, 0);
					prints
					    ("¶Ô²»Æğ£¬Ö¤¼à»á²»ÔÊĞí»ÆÂí¹ÓÏòÍâ×ªÈÃ¹ÉÆ±¡£");
					pressreturn();
					break;
				}
			}
			getdata(t_lines - 1, 0, "ÄúÑ¡ÔñÄÄÖ§¹ÉÆ±?[0]", genbuf, 7,
				DOECHO, YEA);
			getnum=atoi(genbuf);
			if(getnum<0||getnum>count-1)
				break; //·Ç·¨ÊäÈë
			else
				i=getnum;
			getdata(t_lines - 1, 0, "ÄúÒª×ªÈÃ¶àÉÙ¹É?[0]", genbuf, 7,
				DOECHO, YEA);
			stock_num[i] =
			    loadValue(currentuser.userid, stockname[i], 1000000);
			addto_num[i] = atoi(genbuf);
			if (addto_num[i] < 0){
				move(t_lines - 2, 0);
				prints("Ïë×ªÈÃ¸ºµÄ£¿ĞÑĞÑ...");
				pressanykey();
				break;
			}
			if (addto_num[i] == 0){
				pressanykey();
				break;
			}
			if (stock_num[i] < addto_num[i]) {
				move(t_lines - 2, 0);
				prints
				    ("ÄãÃ»ÓĞÕâÃ´¶à¹ÉÆ±°¡...ÊÇÄã·¸ÔÎ»¹ÊÇÎÒ·¸ÔÎ?");
				pressanykey();
				break;
			}

			sprintf(genbuf, "È·¶¨×ªÕË¸ø %s %d %sÂğ£¿",
				uident, addto_num[i], stockname[i]);
			if (askyn(genbuf, NA, NA) == YEA){
				saveValue(currentuser.userid, stockname[i],
					-addto_num[i], 1000000);
				saveValue(uident, stockname[i],
					addto_num[i], 1000000);
				sprintf(genbuf, "ÏòÄã×ªÈÃÁË%d¹É¹ÉÆ±",addto_num[i]);
	            		sprintf(title, "ÄúµÄÅóÓÑ¸øÄúËÍ%s¹ÉÆ±À´ÁË", stockname[i]);
				mail_buf(genbuf, uident, title);
				sprintf(genbuf, "%s½øĞĞ¹ÉÆ±½»Ò×(×ªÈÃ)", currentuser.userid);
				sprintf(buf,"%sÏò%s×ªÈÃÁË%d¹É%s¹ÉÆ±(Ã¿¹É¼ÛÖµ%d±øÂíÙ¸±Ò)",
					currentuser.userid, uident, addto_num[i], stockname[i], stock_price[i]);
				millionairesrec(genbuf, buf, "¹ÉÆ±½»Ò×");
				move(t_lines - 2, 0);
				clrtoeol();
				prints("×ªÈÃ³É¹¦", genbuf);
				pressanykey();
			}
			sleep(1);
			//quit=1;
			break;

		case 'Q':
		case 'q':
			quit = 1;
			break;
		default:
			break;
		}
		//if (quit)
		//	return 0;
		limit_cpu();
	}
	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	persenal_stock_info(stock_num, stock_price,
			    money, stockboard, stock_board);
	move(t_lines - 2, 0);
	clrtobot();
	if (total_sum > 0)
		sprintf(genbuf, "Õâ´Î½»Ò×ÖĞÄãÄÃ»Ø%d±øÂíÙ¸±Ò", total_sum);
	else if (total_sum < 0)
		sprintf(genbuf, "Õâ´Î½»Ò×ÖĞÄã»¨µôÁË%d±øÂíÙ¸±Ò", -total_sum);
	else
		sprintf(genbuf, "ÄãÕâ´Î½»Ò×ÖĞÃ»ÓĞÊ¹ÓÃµ½ÏÖ½ğ");
	prints("%s", genbuf);
	pressanykey();
	return 0;
}

static void /*ÏÔÊ¾money*/
money_show_stat(char *position)
{
	int money, credit;
	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	credit = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);
	clear();
	move(2, 0);
	prints("ÄúÉíÉÏ´ø×Å \033[1;31m%d\033[m ±øÂíÙ¸±Ò£¬", money);
	prints("´æ¿î \033[1;31m%d\033[m ±øÂíÙ¸±Ò¡£µ±Ç°Î»ÖÃ \033[1;33m%s\033[m",
	       credit, position);
	move(3, 0);
	prints
	    ("\033[1m--------------------------------------------------------------------------------\033[m");
}

static void /*ÏÔÊ¾µ±Ç°Î»ÖÃ*/
nomoney_show_stat(char *position)
{
	clear();
	move(2, 0);
	prints
	    ("\033[1;32m»¶Ó­¹âÁÙ±øÂíÙ¸½ğÈÚÖĞĞÄ£¬µ±Ç°Î»ÖÃÊÇ\033[0m \033[1;33m%s\033[0m",
	     position);
	move(3, 0);
	prints
	    ("\033[1m--------------------------------------------------------------------------------\033[m");
}

static int /*¶Ä³¡´óÌü*/
money_gamble()
{
	int ch;
	int quit = 0;
	char uident[IDLEN + 1];
	char buf[STRLEN];
	clear();
	while (!quit) {
		clear();
		money_show_stat("±øÂíÙ¸¶Ä³¡´óÌü");
		move(6, 4);
		prints("±øÂíÙ¸¶Ä³¡×î½üÉúÒâºì»ğ£¬´ó¼Ò¾¡ĞË°¡");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]÷»±¦ [2]777 [3]²ÂÊı×Ö [4]½ğÆË¿ËËó¹ş [5]¶íÂŞË¹ÂÖÅÌ [6]¾­ÀíÊÒ [Q]Àë¿ª\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			money_dice();
			break;
		case '2':
			money_777();
			break;
		case '3':
			//guess_number();
			russian_gun();
			break;
		case '4':
			p_gp();
			break;
		case '5':
			russian_gun();
			break;
		case '6':
			clear();
		    char name[20];
            whoTakeCharge2(3, name);
		    nomoney_show_stat("±øÂíÙ¸¶Ä³¡¾­ÀíÊÒ");
			whoTakeCharge(3, uident);
			if (strcmp(currentuser.userid, uident)) {
				move(6, 4);
				prints
				    ("ÃØÊé%s³åÄãºÈµÀ:¡°ËÀ¶Ä¹í£¬ÓÖÊä¹âÀ²£¿£¡ÀÏ°å%s²»»áÔÙ½èÇ®¸øÄãÁË¡£¡±",
				     name,uident);
				pressanykey();
				break;
			} else {
				move(6, 4);
				prints("ÇëÑ¡Ôñ²Ù×÷´úºÅ:");
				move(7, 6);
				prints
				    ("1. ·¢·ÅVIP¿¨                  2. ÊÕ»ØVIP¿¨");
				move(8, 6);
				prints
				    ("3. VIP¿Í»§                    4. ·¢ÑûÇëº¯");
				move(9, 6);
				prints("5. ½ğÅèÏ´ÊÖ                   6. ÍË³ö");
				ch = igetkey();
				switch (ch) {
				case '1':
					move(12, 4);
					usercomplete("ÏòË­·¢·ÅVIP¿¨£¿", uident);
					move(13, 4);
					if (uident[0] == '\0')
						break;
					if (!searchuser(uident)) {
						prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
						pressanykey();
						break;
					}
					if (seek_in_file
					    (DIR_MC "gamble_VIP", uident)) {
						prints
						    ("¸Ã¿Í»§ÒÑ¾­ÓµÓĞ¶Ä³¡VIP¿¨¡£");
						pressanykey();
						break;
					}
					if (askyn("È·¶¨Âğ£¿", NA, NA) == YEA) {
						addtofile(DIR_MC "gamble_VIP",
							  uident);
						sprintf(genbuf,
							"%s ÏòÄã·¢·Å±øÂíÙ¸¶Ä³¡VIP¿¨",
							currentuser.userid);
						mail_buf
						    ("×ğ¾´µÄ¿Í»§£º »¶Ó­¶à¶à¹âÁÙ±øÂíÙ¸¶Ä³¡£¬¹§×£·¢²Æ!",
						     uident, genbuf);
						move(14, 4);
						prints("·¢·ÅÍê³É¡£");
						sprintf(buf, "¸ø%s·¢·Å¶Ä³¡VIP¿¨",uident);
						sprintf(genbuf, "%sĞĞÊ¹¶Ä³¡¹ÜÀíÈ¨ÏŞ",currentuser.userid);
						millionairesrec(genbuf, buf, "BMYboss");
						pressanykey();
					}
					break;
				case '2':
					move(12, 4);
					usercomplete("ÊÕ»ØË­µÄVIP¿¨£¿", uident);
					move(13, 4);
					if (uident[0] == '\0')
						break;
					if (!searchuser(uident)) {
						prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
						pressanykey();
						break;
					}
					if (!seek_in_file
					    (DIR_MC "gamble_VIP", uident)) {
						prints("¸Ã¿Í»§Ã»ÓĞ¶Ä³¡VIP¿¨¡£");
						pressanykey();
						break;
					}
					if (askyn("È·¶¨Âğ£¿", NA, NA) == YEA) {
						del_from_file(DIR_MC
							      "gamble_VIP",
							      uident);
						sprintf(genbuf,
							"%s ÊÕ»ØÁËÄãµÄ±øÂíÙ¸¶Ä³¡VIP¿¨",
							currentuser.userid);
						mail_buf
						    ("Çî¹í£¬Ã»Ç®ÁË»¹VIP°¡£¿ÏÂ±²×Ó°É£¡",
						     uident, genbuf);
						move(14, 4);
						prints("¿¨ÒÑÊÕ»Ø¡£");
						sprintf(buf, "ÊÕ»Ø%sµÄ¶Ä³¡VIP¿¨",uident);
						sprintf(genbuf, "%sĞĞÊ¹¶Ä³¡¹ÜÀíÈ¨ÏŞ",currentuser.userid);
						millionairesrec(genbuf, buf, "BMYboss");
						pressanykey();
					}
					break;
				case '3':
					clear();
					move(1, 0);
					prints("Ä¿Ç°ÓµÓĞ¶Ä³¡VIP¿¨µÄ¿Í»§£º");
					listfilecontent(DIR_MC "gamble_VIP");
					pressanykey();
					break;
				case '4':
					move(12, 4);
					/*if (time(0) <
					    3600 +
					    loadValue(currentuser.userid,
						      "last_invitation",
						      2000000000)) {
						prints("¸ã³öÌ«¶àÈËÃü²»ºÃ°É£¿");
						pressanykey();
						break;
					}*/
					usercomplete("¸øË­·¢ÑûÇëº¯£¿", uident);
					move(13, 4);
					if (uident[0] == '\0')
						break;
					if (!searchuser(uident)) {
						prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
						pressanykey();
						break;
					}
					if (loadValue(uident, "invitation", 1)) {
						prints("ÒÑ¾­·¢¹ıÁË¡£");
						pressanykey();
						break;
					}
					if (askyn("È·¶¨Âğ£¿", NA, NA) == YEA) {
						saveValue(uident, "invitation",
							  1, 1);
						saveValue(currentuser.userid,
							  "last_invitation",
							  -2000000000,
							  2000000000);
						saveValue(currentuser.userid,
							  "last_invitation",
							  time(0), 2000000000);
						sprintf(genbuf,
							"%s ¸øÄú·¢À´ÁË¶Ä³¡ÑûÇëº¯",
							currentuser.userid);
						mail_buf
						    ("Äú½«ÓĞ»ú»á»ñµÃ20Íò´ó½±£¡µ«ÊÇ£¬ÄúÓĞ¸ü´óµÄ»ú»áÎª´ËËÍÃü¡££­£­ĞÄÌø¾¡ÔÚ±øÂíÙ¸¶Ä³¡¶íÂŞË¹ÂÖÅÌ¶Ä£¡",
						     uident, genbuf);
						move(14, 4);
						prints("ÑûÇëº¯·¢³öÈ¥ÁË¡£");
						sprintf(buf, "¸ø%s·¢·Å¶Ä³¡ÑûÇëº¯",uident);
						sprintf(genbuf, "%sĞĞÊ¹¶Ä³¡¹ÜÀíÈ¨ÏŞ",currentuser.userid);
						millionairesrec(genbuf, buf, "BMYboss");
						pressanykey();
					}
					break;
				case '5':
					move(12, 4);
					if (askyn
					    ("ÄúÕæµÄÒª½ğÅèÏ´ÊÖÂğ£¿", NA,
					     NA) == YEA) {
					/*	del_from_file(MC_BOSS_FILE,
							      "gambling");
						sprintf(genbuf,
							"%s Ğû²¼´ÇÈ¥±øÂíÙ¸¶Ä³¡¾­ÀíÖ°Îñ",
							currentuser.userid);
						deliverreport(genbuf,
							      "±øÂíÙ¸½ğÈÚÖĞĞÄ¶ÔÆäÒ»Ö±ÒÔÀ´µÄ¹¤×÷±íÊ¾¸ĞĞ»£¬×£ÒÔºóË³Àû£¡");
						move(14, 4);
						prints
						    ("ºÃ°É£¬¼ÈÈ»ÄãÒâÒÑ¾ö£¬µÜĞÖÃÇÖ»ÓĞËµÔÙ¼ûÁË£¡");
						quit = 1;
					*/
						sprintf(genbuf, "%s Òª´ÇÈ¥¶Ä³¡¾­ÀíÖ°Îñ",
							currentuser.userid);
						mail_buf(genbuf, "millionaires", genbuf);
						move(14, 4);
						prints("ºÃ°É£¬ÒÑ¾­·¢ĞÅ¸æÖª×Ü¹ÜÁË");
						pressanykey();
					}
					break;
				case 'q':
				case 'Q':
					break;
				}
			}
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}
	return 0;
}

static int/*¶Ä²©-- 777*/
money_777()
{
	int ch;
	int quit = 0;
	int bid;
	int money;
	int t1, t2, t3, winrate, r;
	char n[9] = "-R-B-6-7";
	char title[STRLEN], buf[256];

	clear();
	srandom(time(0));
	while (!quit) {
		if (utmpshm->mc.prize777 <= 0)
			utmpshm->mc.prize777 = 30000;
		bid = 0;
		clear();
		money_show_stat("±øÂíÙ¸¶Ä³¡777");
		move(6, 4);
		prints("--R 1:2    -RR 1:3    RR- 1:3    -BB 1:5    BB- 1:5");
		move(7, 4);
		prints("RRR 1:10   BBB 1:20   666 1:40   677 1:60   --- 1:1");
		move(8, 4);
		prints
		    ("         777 1:80 ÇÒÓĞ»ú»áÓ®µÃµ±Ç°ÀÛ»ı»ù½ğµÄÒ»°ë         ");
		move(9, 4);
		prints("±øÂíÙ¸Ä¿Ç°ÀÛ»ı½±½ğÊı: %d£¬ÏëÓ®´ó½±Ã´£¿Ñ¹ 100 ¿é¾ÍĞĞÅ¶¡£",
		       utmpshm->mc.prize777);
		r = random() % 40;
		if (r < 1)
			money_police();

		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1] Ñ¹30 [2] Ñ¹100 [Q]Àë¿ª                                          \033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			bid = 30;
			break;
		case '2':
			bid = 100;
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		if (quit)
			break;
		if (bid == 0)
			continue;
		money =
		    loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
		if (money < bid) {
			move(11, 4);
			prints("Ã»Ç®¾Í±ğ¶ÄÁË...");
			pressanykey();
			continue;
		}
		saveValue(currentuser.userid, MONEY_NAME, -bid, MAX_MONEY_NUM);
		t1 = random() % 8;
		t2 = random() % 8;
		t3 = random() % 8;
		move(11, 20);
		prints("%c", n[t1]);
		refresh();
		sleep(1);
		move(11, 22);
		prints("%c", n[t2]);
		refresh();
		sleep(1);
		move(11, 24);
		prints("%c", n[t3]);
		refresh();
		sleep(1);
		winrate = calc777(t1, t2, t3);
		if (winrate <= 0) {
			utmpshm->mc.prize777 += bid * 80 / 100;
			if (utmpshm->mc.prize777 >= MAX_MONEY_NUM)
				utmpshm->mc.prize777 = MAX_MONEY_NUM;

			sprintf(title, "%s²ÎÓë¶Ä²©(777)(Êä)", currentuser.userid);
			sprintf(buf, "%sÔÚ777 ÊäÁË%d±øÂíÙ¸±Ò", currentuser.userid, bid);
			millionairesrec(title, buf, "¶Ä²©777");

			move(12, 4);
			prints
			    ("ÊäÁË£¬¶Ä×¢°Ù·ÖÖ®°ËÊ®¹öÈë±øÂíÙ¸ÀÛ»ı»ù½ğ£¬Ôì¸£ËûÈËµÈÓÚÔì¸£×Ô¼º¡£");
			limit_cpu();
			pressanykey();
			continue;
		}
		if (winrate > 0) {
			saveValue(currentuser.userid, MONEY_NAME, bid * winrate,
				  MAX_MONEY_NUM);
			move(12, 4);
			prints("ÄúÓ®ÁË %d Ôª", bid * (winrate - 1));
			utmpshm->mc.prize777 -= bid * (winrate - 1);

			sprintf(title, "%s²ÎÓë¶Ä²©(777)(Ó®)", currentuser.userid);
			sprintf(buf, "%sÔÚ777 Ó®ÁË%d±øÂíÙ¸±Ò", currentuser.userid, bid * (winrate - 1));
			millionairesrec(title, buf, "¶Ä²©777");
		}
		if (winrate == 81 && bid == 100) {
			saveValue(currentuser.userid, MONEY_NAME,
				  utmpshm->mc.prize777 / 2, MAX_MONEY_NUM);
			utmpshm->mc.prize777 /= 2;
			move(13, 4);
			prints("¹§Ï²Äú»ñµÃ±øÂíÙ¸´ó½±£¡");
			sprintf(title, "%s²ÎÓë¶Ä²©(777)(Ó®³ÉÂíÁË)", currentuser.userid);
			sprintf(buf, "%sÔÚ777 Ó®ÁË%d±øÂíÙ¸±Ò", currentuser.userid, utmpshm->mc.prize777 / 2);
			millionairesrec(title, buf, "¶Ä²©777");
		}
		limit_cpu();
		pressanykey();
	}
	return 0;
}

static int/*¶Ä²©--777*/
calc777(int t1, int t2, int t3)
{
	if ((t1 % 2 == 0) && (t2 % 2 == 0) && (t3 % 2 == 0))
		return 2;
	if ((t1 % 2 == 0) && (t2 % 2 == 0) && (t3 == 1))
		return 3;
	if ((t1 % 2 == 0) && (t2 == 1) && (t3 == 1))
		return 4;
	if ((t1 == 1) && (t2 == 1) && (t3 % 2 == 0))
		return 4;
	if ((t1 % 2 == 0) && (t2 == 3) && (t3 == 3))
		return 6;
	if ((t1 == 3) && (t2 == 3) && (t3 % 2 == 0))
		return 6;
	if ((t1 == 1) && (t2 == 1) && (t3 == 1))
		return 11;
	if ((t1 == 3) && (t2 == 3) && (t3 == 3))
		return 21;
	if ((t1 == 5) && (t2 == 5) && (t3 == 5))
		return 41;
	if ((t1 == 5) && (t2 == 7) && (t3 == 7))
		return 61;
	if ((t1 == 7) && (t2 == 7) && (t3 == 7))
		return 81;
	return 0;
}

static int/*¶Ä²©--²ÂÊı×Ö*/
guess_number()
{
	int quit = 0;
	int ch, num, money;
	int a, b, c;
	int win;
	int count;
	char ans[5] = "";
	int bet[7] = { 0, 100, 50, 20, 5, 3, 1 };
	char title[STRLEN], buf[256];

	srandom(time(0));
	while (!quit) {
		clear();
		money_show_stat("±øÂíÙ¸Á¼ÃñÉú²ÆÖ®Â·...");
		move(4, 4);
		prints("\033[1;31m¿ª¶¯ÄÔ½î×¬Ç®°¡~~~\033[m");
		move(5, 4);
		//prints("×îĞ¡Ñ¹ 100 ±øÂíÙ¸±Ò£¬ÉÏÏŞ999");
		prints("Ò»´Î 100 ±øÂíÙ¸±Ò.");
		move(6, 4);
		prints("mAnB±íÊ¾ÓĞm¸öÊı×Ö²Â¶ÔÇÒÎ»ÖÃÒ²¶Ô,n¸öÊı×Ö²Â¶Ôµ«Î»ÖÃ²»¶Ô");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]ÏÂ×¢ [Q]Àë¿ª                                                 \033[m");
		if (random() % 40 < 1)
			money_police();
		ch = igetkey();
		switch (ch) {
		case '1':
			win = 0;

			getdata(8, 4, "ÄúÑ¹¶àÉÙ±øÂíÙ¸±Ò£¿[100]", genbuf, 5,
			   DOECHO, YEA);
			   num = atoi(genbuf);
			   if (!genbuf[0])
			   num = 100;
			   if (num < 100) {
			   move(9, 4);
			   prints("ÓĞÃ»ÓĞÇ®°¡£¿ÄÇÃ´µãÇ®ÎÒÃÇ²»´øÍæµÄ");
			   pressanykey();
			   break;
			   }
			//num = 100;
			sprintf(genbuf,
				"ÄúÑ¹ÁË \033[1;31m%d\033[m ±øÂíÙ¸±Ò£¬È·¶¨Ã´£¿",
				num);
			move(9, 4);
			if (askyn(genbuf, YEA, NA) == YEA) {
				money =
				    loadValue(currentuser.userid, MONEY_NAME,
					      MAX_MONEY_NUM);
				if (money < num) {
					move(11, 4);
					prints("È¥È¥È¥£¬Ã»ÄÇÃ´¶àÇ®µ·Ê²Ã´ÂÒ         \n");
					pressanykey();
					break;
				}
				//if (num > 999)
					//num = 999;
				saveValue(currentuser.userid, MONEY_NAME, -num,
					  MAX_MONEY_NUM);
				do {
					itoa(random() % 10000, ans);
					for (a = 0; a < 3; a++)
						for (b = a + 1; b < 4; b++)
							if (ans[a] == ans[b])
								ans[0] = 0;
				} while (!ans[0]);
				for (count = 1; count < 7; count++) {
					do {
						move(10, 4);
						prints
						    ("ÇëÊäÈëËÄ¸ö²»ÖØ¸´µÄÊı×Ö");
						getdata(11, 4, "Çë²Â[q - ÍË³ö] ¡ú ", genbuf, 5, DOECHO, YEA);
						if (genbuf[0] == 'q' || genbuf[0] == 'Q') {
							utmpshm->mc.prize777 += num;
							if (utmpshm->mc.prize777 > MAX_MONEY_NUM)
								utmpshm->mc.prize777 = MAX_MONEY_NUM;
							move(12, 4);
							prints("byebye!");
							pressanykey();
							quit = 1;
							return 0;
						}
						c = atoi(genbuf);
						itoa(c, genbuf);
						for (a = 0; a < 3; a++)
							for (b = a + 1; b < 4; b++)
								if (genbuf[a] == genbuf[b])
									genbuf[0] = 0;
						if (!genbuf[0]) {
							move(12, 4);
							prints ("ÊäÈëÊı×ÖÓĞÎÊÌâ!!");
							pressanykey();
							move(12, 4);
							prints ("                ");
						}
					} while (!genbuf[0]);
					move(count + 13, 0);
					prints("  µÚ %2d ´Î£º %s  ->  %dA %dB ",
					       count, genbuf, an(genbuf, ans),
					       bn(genbuf, ans));
					if (an(genbuf, ans) == 4)
						break;
					sleep(1);
				}

				move(12, 4);
				if (count > 6) {
					sprintf(genbuf,
						"ÄãÊäÁËßÏ£¡ÕıÈ·´ğ°¸ÊÇ %s£¬ÏÂ´ÎÔÙ¼ÓÓÍ°É!!",
						ans);
					sprintf(genbuf,
						"\033[1;31m¿ÉÁ¯Ã»²Âµ½£¬ÊäÁË %d Ôª£¡\033[m",
						num);
					//utmpshm->mc.prize777 += num;

					sprintf(title, "%s²ÎÓë¶Ä²©(²ÂÊı×Ö)(Êä)", currentuser.userid);
					sprintf(buf, "%sÔÚ²ÂÊı×ÖÊäÁË%d±øÂíÙ¸±Ò", currentuser.userid, num);
					millionairesrec(title, buf, "¶Ä²©²ÂÊı×Ö");

					if (utmpshm->mc.prize777 > MAX_MONEY_NUM)
						utmpshm->mc.prize777 = MAX_MONEY_NUM;
				} else {
					int oldmoney = num;
					num *= bet[count];
					if (num - oldmoney > 0) {
						sprintf(genbuf,
							"¹§Ï²£¡×Ü¹²²ÂÁË %d ´Î£¬¾»×¬½±½ğ %d Ôª",
							count, num);
						num += oldmoney;
						saveValue(currentuser.userid,
							  MONEY_NAME, num,
							  MAX_MONEY_NUM);
						win = 1;

						sprintf(title, "%s²ÎÓë¶Ä²©(²ÂÊı×Ö)(Ó®)", currentuser.userid);
						sprintf(buf, "%sÔÚ²ÂÊı×ÖÓ®ÁË%d±øÂíÙ¸±Ò", currentuser.userid, num);
						millionairesrec(title, buf, "¶Ä²©²ÂÊı×Ö");

					} else if (num - oldmoney == 0) {
						sprintf(genbuf,
							"°¦¡«¡«×Ü¹²²ÂÁË %d ´Î£¬Ã»ÊäÃ»Ó®£¡",
							count);
						saveValue(currentuser.userid,
							  MONEY_NAME, num,
							  MAX_MONEY_NUM);
					} else {
						utmpshm->mc.prize777 +=
						    oldmoney;
						if (utmpshm->mc.prize777 > MAX_MONEY_NUM)
							utmpshm->mc.prize777 = MAX_MONEY_NUM;

						sprintf(genbuf,
							"°¡¡«¡«×Ü¹²²ÂÁË %d ´Î£¬ÅâÇ® %d Ôª£¡",
							count,
							oldmoney - money);
					}
				}
				prints("½á¹û: %s", genbuf);
				move(13, 4);
				pressanykey();
			}
			break;
		case 'Q':
		case 'q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

static int
an(a, b)
char *a, *b;
{
	int i, k = 0;
	for (i = 0; i < 4; i++)
		if (*(a + i) == *(b + i))
			k++;
	return k;
}

static int
bn(a, b)
char *a, *b;
{
	int i, j, k = 0;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			if (*(a + i) == *(b + j))
				k++;
	return (k - an(a, b));
}

static void
itoa(i, a)
int i;
char *a;
{
	int j, k, l = 1000;

	for (j = 3; j > 0; j--) {
		k = i - (i % l);
		i -= k;
		k = k / l + 48;
		a[3 - j] = k;
		l /= 10;
	}
	a[3] = i + 48;
	a[4] = 0;

}

static int/*¾¯Êğ--¾¯²ìÁÙ¼ì*/
money_police()
{
	char ch;
	char buf[200], title[STRLEN];
	int money = 0, quit = 1, check_num;
	//int mode = 0, color;
	move(t_lines - 1, 0);
	check_num = 97 + random() % 26;
	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
	if (random() % 4 > 0) {
		sprintf
		    (buf,
		     "\033[1;41m ÏµÍ³ÁÙ¼ì \033[1;46m ÇëÊäÈë×Ö·û:%c        \033[m",
		     check_num);
	}
	//else if (random() % 3 == 0)
	else {
		check_num = 0;
		sprintf(buf,
			"\033[1;41m ÏµÍ³ÁÙ¼ì \033[1;46m ÇëÊäÈëÄãµÄID(×¢Òâ´óĞ¡Ğ´!):        \033[m");
	}
	/*else {
		mode = 1;
		color = (random() % 7) + 31;
		snprintf(buf, 200,
			 "\033[1;41m ÏµÍ³ÁÙ¼ì \033[1;46m \033[1;%dmÎÄ×ÖÑÕÉ«\033[1;37m:[\033[1;31mºì\033[1;32mÂÌ\033[1;33m»Æ\033[1;34mÀ¶\033[1;35m×Ï\033[1;36mÇà\033[1;37m°×]\033[m",
			 color);
		getdata(t_lines - 1, 4, buf, genbuf, 5, DOECHO, YEA);
		if (color == 31 && (!strcmp("ºì", genbuf) || !strcmp("red", genbuf)))
			quit = 0;
		if (color == 32 && (!strcmp("ÂÌ", genbuf) || !strcmp("green", genbuf)))
			quit = 0;
		if (color == 33 && (!strcmp("»Æ", genbuf) || !strcmp("yellow", genbuf)))
			quit = 0;
		if (color == 34 && (!strcmp("À¶", genbuf) || !strcmp("blue", genbuf)))
			quit = 0;
		if (color == 35 && (!strcmp("×Ï", genbuf) || !strcmp("purple", genbuf)))
			quit = 0;
		if (color == 36 && (!strcmp("Çà", genbuf) || !strcmp("cyan", genbuf)))
			quit = 0;
		if (color == 37 && (!strcmp("°×", genbuf) || !strcmp("white", genbuf)))
			quit = 0;
			quit = 0;
	}
	*/
	//if (mode == 0) {
		getdata(t_lines - 1, 4, buf, genbuf, 13, DOECHO, YEA);
		if (check_num > 0) {
			ch = genbuf[0];
			if (ch == check_num)
				quit = 0;
			else
				quit = 1;
		} else {
			if (strcmp(genbuf, currentuser.userid))
				quit = 1;
			else
				quit = 0;
		}
	//}
	if (quit) {
		move(t_lines - 2, 4);
		prints("ÏµÍ³ÈÏÎªÄãÓĞ×÷±×ÏÓÒÉ£¬·£µôÉíÉÏËùÓĞÏÖ½ğ,Õæ²Ò°¡~~~");
		saveValue("millionaires", MONEY_NAME, money, MAX_MONEY_NUM);
		sprintf(title, "%s±»ÏµÍ³ÁÙ¼ì", currentuser.userid);
		sprintf(buf, "ÏµÍ³ÁÙ¼ì, %s±»·£µôËùÓĞÏÖ½ğ%d±øÂíÙ¸±Ò", currentuser.userid, money);
		millionairesrec(title, buf, "ÏµÍ³ÁÙ¼ì");
		pressanykey();
		Q_Goodbye();
	} else {
		saveValue(currentuser.userid, MONEY_NAME, money, MAX_MONEY_NUM);
		move(t_lines - 2, 4);
		sprintf(buf, "Äã¾ÍÊÇ´óÃû¶¦¶¦µÄ%s°¡,¼ÌĞø¼ÌĞø...",
			currentuser.userid);
		prints("%s", buf);
		pressanykey();

	}
	return 0;
}

static void/*¸öÈË¹ÉÆ±ÏµÍ³*/
persenal_stock_info(int stock_num[MAX_STOCK_NUM],
		    int stock_price[MAX_STOCK_NUM], int money,
		    char stockboard[STRLEN][MAX_STOCK_NUM], int stock_board[MAX_STOCK_NUM])
{
	int i, count;
	count = listfilecontent(MC_STOCK_BOARDS);
	clear();
	move(0, 4);
	prints("±øÂíÙ¸¹ÉÊĞÊÔÓªÒµ...ÒÔÏÂÊÇÄãµÄ¸÷¹É³ÖÓĞÊı,Ã¿¹É¹ºÂòÉÏÏŞ1000000¹É");
	move(1, 4);
	sprintf(genbuf, "Ä¿Ç°ÄãµÄ±øÂíÙ¸±Ò½ğ¶îÎª%d", money);
	prints("%s", genbuf);
	for (i = 0; i < count; i++) {
		move(3 + i, 0);
		/*sprintf(genbuf,
			"±àºÅ:%2d Stock%c¼ÛÇ®:%d\t³ÖÓĞÁ¿:%d\t°æÃû:%-10s ÏÖÓĞ¹ÉÆ±Êı:%d",i,
			65 + i, stock_price[i], stock_num[i], stockboard[i],
			bcache[stock_board[i]].stocknum);
		*/
		sprintf(genbuf,
			"±àºÅ:%2d ¼ÛÇ®:%-5d ³ÖÓĞÁ¿:%-7d °æÃû:%-18s ÏÖÓĞ¹ÉÆ±Êı:%d",
 			i, stock_price[i], stock_num[i], stockboard[i], bcache[stock_board[i]].stocknum);
		if (seek_in_file(MC_STOCK_STOPBUY, stockboard[i]))
			prints("\033[1;30m%s\033[m", genbuf);
		else
			prints("%s", genbuf);
	}
}
/*
static void//¸öÈË¹ÉÆ±ÏµÍ³
persenal_stock_info2(int stock_num[MAX_STOCK_NUM2],
		    int stock_price[MAX_STOCK_NUM2], int money,
		    char *stockboard[], int stock_board[MAX_STOCK_NUM2])
{
	int i;
	clear();
	move(0, 4);
	prints("±øÂíÙ¸¹ÉÊĞÊÔÓªÒµ...ÒÔÏÂÊÇÄãµÄ¸÷¹É³ÖÓĞÊı,Ã¿¹É¹ºÂòÉÏÏŞ50,000¹É");
	move(1, 4);
	sprintf(genbuf, "Ä¿Ç°ÄãµÄ±øÂíÙ¸±Ò½ğ¶îÎª%d", money);
	prints("%s", genbuf);
	for (i = 0; i < MAX_STOCK_NUM2; i++) {
		move(3 + i, 0);
		sprintf(genbuf,
			"±àºÅ:%2d Stock%c¼ÛÇ®:%d\t³ÖÓĞÁ¿:%d\t°æÃû:%-10sÏÖÓĞ¹ÉÆ±Êı:%d",i,
			65 + i, stock_price[i], stock_num[i], stockboard[i],
			bcache[stock_board[i]].stocknum);
		prints("%s", genbuf);
	}
}*/

/*-------------Å×Æú¾ÉµÄºØ¿¨ÏµÍ³-------macintosh 20051203------*/
/*static int
shop_card_show(char *card[][2], int group)
{
	int key, i, j, x = 0, y = 0;
	int global_x = 0, local_x = 0, limit = 0, base = 0;
	y = 1;
	clear();
	move(5, 4);
	prints("±¾µêºØ¿¨¾ù·Ç±¾µêÖÆ×÷£¬²¿·ÖºØ¿¨ÓÉÓÚÖÖÖÖÔ­Òò£¬Î´ÄÜ±êÃ÷×÷Õß¡£\n");
	move(6, 4);
	prints("ÈçºØ¿¨´´×÷Õß¶ÔÆä×÷Æ·ÓÃÓÚ±¾µê³ÖÓĞÒìÒé£¬ÇëÓë±¾Õ¾´ó¸»ÎÌ×Ü¹ÜÁªÏµ¡£\n");
	move(7, 4);
	prints("±¾Õ¾½«¼°Ê±¸ù¾İ×÷ÕßÒâÔ¸×÷³öµ÷Õû¡£\n");
	move(9, 20);
	prints("±øÂíÙ¸ºØ¿¨µê \n");
	pressanykey();
	while (y) {
		clear();
		nomoney_show_stat("±øÂíÙ¸ºØ¿¨ÉÌµê");

		if (y == 1) {
			for (i = 0; i < group; i++) {
				move(5 + i, 4);
				if (i == x)
					sprintf(genbuf,
						"\033[1;41m[+]> %s\033[0m",
						card[i][0]);
				else
					sprintf(genbuf,
						"\033[1;43m[+]  %s\033[0m",
						card[i][0]);
				prints("%s", genbuf);
			}
		} else if (y == 2) {
			i = 0;
			for (j = 0; j < group; j++) {
				if (6 + 2 + local_x > 22) {
					limit = 1;
					base = 8 + local_x - 22;
				} else
					limit = 0;
				if (j == global_x) {
					if (!limit) {
						move(5 + j + i, 4);
						sprintf(genbuf,
							"\033[1;44m[-] %s\033[0m",
							card[j][0]);
						prints("%s", genbuf);
					}
					for (i = 0; i < atoi(card[x][1]); i++) {
						if (!limit) {
							if (6 + j + i > 23)
								continue;
							else {
								move(6 + j + i,
								     8);
								if (i ==
								    local_x)
									sprintf
									    (genbuf,
									     "\033[1;41m>|--%s%d\033[0m",
									     card
									     [j]
									     [0],
									     i +
									     1);
								else
									sprintf
									    (genbuf,
									     "\033[1;42m |--%s%d\033[0m",
									     card
									     [j]
									     [0],
									     i +
									     1);
								prints("%s",
								       genbuf);
							}
						} else {
							// base = 8+local_x-22;
							// local_x = 15; base = 1;
							if ((i - base) > 4
							    && (i - base) <
							    24) {
								move(i - base,
								     8);
								if (i ==
								    local_x)
									sprintf
									    (genbuf,
									     "\033[1;41m>|--%s%d\033[0m",
									     card
									     [j]
									     [0],
									     i +
									     1);
								else
									sprintf
									    (genbuf,
									     "\033[1;42m |--%s%d\033[0m",
									     card
									     [j]
									     [0],
									     i +
									     1);
								prints("%s",
								       genbuf);
							} else
								continue;
						}
					}
				} else {
					if (!limit) {
						if ((5 + j + i) < 24
						    && (5 + j + i) > 4) {
							move(5 + j + i, 4);
							sprintf(genbuf,
								"\033[1;43m[+] %s\033[0m",
								card[j][0]);
							prints("%s", genbuf);
						}
					}
				}
			}
		}
		move(t_lines - 1, 4);
		prints
		    ("\033[1;45m·½Ïò¼ü²Ù×÷£¬×ó¼ü·µ»ØÉÏÒ»²ã£¬ÓÒ¼ü½øÈë£¬ÉÏÏÂ¼üÑ¡Ôñ \033[0m");
		key = egetch();
		switch (key) {
		case KEY_LEFT:
		case 'q':
		case 'Q':
		case 'e':
		case 'E':
			y--;
			global_x = x;
			break;
		case KEY_RIGHT:
		case '\n':
		case '\r':
			if (y == 2) {
				buy_card(card[global_x][0], local_x + 1);
			}
			if (y < 2) {
				y++;
				local_x = 0;
			}
			global_x = x;
			break;
		case KEY_UP:
			if (y == 2) {
				local_x--;
				if (local_x < 0)
					local_x = atoi(card[x][1]) - 1;
			} else {
				x--;
				if (x < 0)
					x = group - 1;

			}
			break;
		case KEY_DOWN:
			if (y == 2) {
				local_x++;
				if (local_x >= atoi(card[x][1]))
					local_x = 0;
			} else {
				x++;
				if (x > group - 1)
					x = 0;

			}
			break;
		}
		limit_cpu();
		if (y == 0)
			break;
	}
	move(t_lines - 2, 5);
	prints("»¶Ó­ÄúÔÙÀ´!");
	pressanykey();
	return 0;
}*/

/*----Å×Æú¾ÉµÄºØ¿¨ÏµÍ³(¹ºÂòºØ¿¨)-----macintosh 20051203-----*/
/*static int
buy_card(char *cardname, int cardnumber)
{
	char card_name[20], filepath[80], uident[IDLEN + 1];
	char note[3][STRLEN], tmpname[STRLEN];
	int money, i;
	bzero(card_name, sizeof (card_name));
	sprintf(card_name, "%s%d", cardname, cardnumber);
	sprintf(filepath, "0Announce/game/cardshop/%s/%d", cardname,
		cardnumber);
	ansimore2(filepath, 0, 0, 25);
	move(2, 0);
	prints("Preview....");
	move(8, 20);
	prints("Preview....");
	move(14, 40);
	prints("Preview....");
	//clear();
	move(t_lines - 2, 4);
	sprintf(genbuf, "ÄãÈ·¶¨ÒªÂòºØ¿¨%sÃ´?", card_name);
	if (askyn(genbuf, YEA, NA) != YEA)
		return 0;
	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	if (money < 1000) {
		move(t_lines - 2, 4);
		prints("ÄãµÄÇ®²»¹»°¡~~~");
		pressanykey();
		return 0;
	}

	saveValue(currentuser.userid, MONEY_NAME, -1000, MAX_MONEY_NUM);
	move(0, 0);
	clrtobot();
	usercomplete("Òª°ÑÕâ¿¨Æ¬ËÍ¸øË­? ", uident);
	if (uident[0] == '\0') {
		move(t_lines - 2, 4);
		clrtobot();
		prints("¾ÓÈ»²»Ğ´µØÖ·£¬ÄãÂòµÄ¿¨Æ¬¶ªÊ§ÔÚÓÊ¼ÄÍ¾ÖĞ...");
		pressanykey();
		return 0;
	}

	if (!getuser(uident)) {
		move(t_lines - 2, 4);
		clrtobot();
		prints("Ã»ÓĞÕâ¸öÈË°¡£¬ÄãÂòµÄ¿¨Æ¬±»ÓÊµİÔ±Ë½ÍÌÁË...");
		pressanykey();
		return 0;
	}
	move(0, 0);
	clear();
	prints("ÓĞ»°ÒªÔÚ¿¨Æ¬ÀïËµÂğ£¿");
	bzero(note, sizeof (note));
	for (i = 0; i < 3; i++) {
		getdata(1 + i, 0, ": ", note[i], STRLEN - 1, DOECHO, NA);
		if (note[i][0] == '\0')
			break;
	}
	sprintf(tmpname, "bbstmpfs/tmp/card.%s.%d", currentuser.userid,
		uinfo.pid);
	copyfile(filepath, tmpname);
	if (i > 0) {
		int j;
		FILE *fp = fopen(tmpname, "a");
		fprintf(fp, "\nÒÔÏÂÊÇ %s µÄ¸½ÑÔ:\n", currentuser.userid);
		for (j = 0; j < i; j++)
			fprintf(fp, "%s", note[j]);
		fclose(fp);
	}

	move(t_lines - 2, 4);
	clrtobot();
	prints("ÄãµÄºØ¿¨ÒÑ¾­·¢³öÈ¥ÁË");
	mail_file(tmpname, uident, "ÎÒ´Ó±øÂíÙ¸ÉÌµêÀïÌô¸øÄãµÄºØ¿¨£¬Äã¿´¿´ÖĞÒâÃ´?");
	unlink(tmpname);
	pressanykey();
	return 0;
}*/

//add by happybird for ÏÊ»¨µê£¬ºØ¿¨µê
//ÏÔÊ¾»¶Ó­»­Ãæ
//Õâ¸öÒª¶ÁÎÄ¼ş¿ÉÄÜ»áÔì³ÉËğºÄ£¬Ğ¡ĞÄÊ¹ÓÃ
int show_welcome(char *filepath,int startline,int endline)
{
	FILE *fp;
	char buf[400];
	int linecount=0;

	fp=fopen(filepath,"r");
	if(!fp){
		move(startline,10);
		prints("»¶Ó­ÄúµÄµ½À´!");
		return 0;
	}
	linecount=0;
	while(!feof(fp)){
		if(fgets(buf,400,fp)){
			move(startline+linecount,0);
			prints("%s",buf);
			linecount++;
		}
		if(linecount >endline-startline) break;
	}
	fclose(fp);
	return 1;
}


#define PRESENT_DIR "0Announce/groups/GROUP_0/" MC_BOARD "/system/present"
//#define FLOWER_WELCOME	MY_BBS_HOME FLOWER_DIR "/welcome"
#define PATHLEN 1000

//ÀñÆ·µê£¬ÏÊ»¨ºØ¿¨¶şºÏÒ»£¬macintosh@bmy 20051204
static int
shop_present(int order, char *kind, char *touserid)
{
	char ok_filename[PATHLEN];
	char ok_title[STRLEN];
	int price_per=0;
	char *ptr1,*ptr2;
	//char filepath[256];
	//void *buffer_me = NULL;
	char buf[STRLEN];
	int ma;

	sprintf(buf, "±øÂíÙ¸ÀñÆ·µê%s¹ñÌ¨", kind);
	nomoney_show_stat(buf);
	sprintf(buf, "%s%d%s", PRESENT_DIR, order, "/welcome");
	show_welcome(buf,6,20);
	pressanykey();

	DIR *dp;
//	struct dirent *dirp;
	char dirNameBuffer[10][PATHLEN], dirTitleBuffer[10][STRLEN];
	char fileNameBuffer[10][PATHLEN],  fileTitleBuffer[10][STRLEN];
	char dirpath[PATHLEN], filepath[PATHLEN], dir[PATHLEN], indexpath[PATHLEN], title[STRLEN];
	int numDir=0, numFile=0, dirIndex, cardIndex, m;
	int HIDE=0;
	FILE *fp;

	sprintf(buf, "±øÂíÙ¸ÀñÆ·µê%s¹ñÌ¨", kind);
	nomoney_show_stat(buf);
	move(4,4);
	sprintf(dir, "%s%d/", PRESENT_DIR, order);
	sprintf(indexpath, "%s.Names", dir);
	prints("±¾µê³öÊÛÈçÏÂÖÖÀàµÄ%s: ", kind);
	if ((dp = opendir(dir)) == NULL)
		return -1;

	fp=fopen(indexpath, "r");
	if(fp!=0) {
		while(fgets(buf, STRLEN, fp)>0 && numDir<10) {
			if(!strncmp(buf, "Name=", 5)) {
				sprintf(title, "%s", buf+5);
				if(strstr(title + 38,"(BM: SYSOPS)") ||
					strstr(title + 38,"(BM: BMS)")||
					!strncmp(title, "<HIDE>",6))
					HIDE=1;
				else
					HIDE=0;
				title[38]=0;
				fgets(buf, STRLEN, fp);
				if(!strncmp("Path=~/", buf, 6)) {
					if(HIDE) continue;
					snprintf(dirpath, PATHLEN,  "%s%s", dir, buf+7);
					for(m=0; m<strlen(dirpath); m++) if (dirpath[m]<27) dirpath[m]=0;
					if (!file_isdir(dirpath))
						continue;
					for(m=0; m<strlen(title); m++) if(title[m]==' ') title[m]=0;
					strncpy(dirNameBuffer[numDir], dirpath, PATHLEN);
					strncpy(dirTitleBuffer[numDir], title, STRLEN);
					move(6 + numDir, 8);
					prints("%d. %s", numDir, title);
					numDir++;
				}
			}
		}
		fclose(fp);
	}

/*
	for (numDir = 0; (dirp = readdir(dp)) != NULL && numDir < 10; ) {
		snprintf(dirpath, 255,  "%s%s", dir, dirp->d_name);
		if (!file_isdir(dirpath) || dirp->d_name[0] == '.')
			continue;
		move(6 + numDir, 8);
		prints("%d. %s", numDir, dirp->d_name);
		strncpy(dirNameBuffer[numDir], dirp->d_name, 31);
		dirNameBuffer[numDir][31] = '\0';
		numDir++;
	}
*/
	while(1) {
		getdata(16, 4, "ÇëÑ¡ÔñÀàĞÍ:", buf, 3, DOECHO, YEA);
		if (buf[0] == '\0')
			return 0;
		dirIndex = atoi(buf);
		if (dirIndex >= 0 && dirIndex < numDir)
			break;
	}

	sprintf(buf, "±øÂíÙ¸ÀñÆ·µê%s¹ñÌ¨", kind);
	nomoney_show_stat(buf);
	move(4,4);
	snprintf(dirpath, PATHLEN, "%s", dirNameBuffer[dirIndex]);
	if ((dp = opendir(dirpath)) == NULL)
		return -1;
	//prints("±¾µê³öÊÛÈçÏÂÖÖÀàµÄ%s: ", kind);

	sprintf(indexpath, "%s/.Names", dirpath);
	fp=fopen(indexpath, "r");
	if(fp!=0) {
		while(fgets(buf, STRLEN, fp)>0 && numFile<10) {
			if(!strncmp(buf, "Name=", 5)) {
				sprintf(title, "%s", buf+5);
				if(strstr(title + 38,"(BM: SYSOPS)") ||
					strstr(title + 38,"(BM: BMS)")||
					!strncmp(title, "<HIDE>",6))
					HIDE=1;
				else
					HIDE=0;
				title[38]=0;
				fgets(buf, STRLEN, fp);
				if(!strncmp("Path=~/", buf, 6)) {
					if(HIDE) continue;
					snprintf(filepath, PATHLEN,  "%s/%s", dirpath, buf+7);
					for(m=0; m<strlen(filepath); m++) if (filepath[m]<27) filepath[m]=0;
					if (!file_isfile(filepath))
						continue;
					//for(m=0; m<strlen(title); m++) if(title[m]==' ') title[m]=0;
					strncpy(fileNameBuffer[numFile], filepath, PATHLEN);
					strncpy(fileTitleBuffer[numFile], title, STRLEN);
					move(6 + numFile, 8);
					prints("%d. %s", numFile, title);
					numFile++;
				}
			}
		}
		fclose(fp);
	}

	move(4,4);
	prints("±¾µê %s Àà%s¹²ÓĞ %d ÖÖ: ", dirTitleBuffer[dirIndex], kind, numFile);
/*
	for (numFile = 0; (dirp = readdir(dp)) != NULL; ) {
		snprintf(filepath, PATHLEN, "%s/%s", dirpath, dirp->d_name);
		if(file_isfile(filepath) && dirp->d_name[0] != '.')
			numFile++;
	}
*/
	move(17, 4);
	while(1) {
		getdata(18, 4, "ÇëÑ¡ÔñÒªÔ¤ÀÀµÄ±àºÅ[ENTER·ÅÆú]: ",
			buf, 3, DOECHO, YEA);
		if(buf[0] == '\0')
			return 0;
		cardIndex = atoi(buf);
		if (cardIndex >= 0 && cardIndex <= numFile)
			break;
	}

	sprintf(buf, "%s¹ñÌ¨%s Àà%sÕ¹Ê¾", kind, dirTitleBuffer[dirIndex], fileTitleBuffer[cardIndex]);
	nomoney_show_stat(buf);
	//show_welcome(fileNameBuffer[cardIndex], 5, 20);
	ansimore2(fileNameBuffer[cardIndex], 1, 5, 20);

	limit_cpu();

	strncpy(ok_filename, fileNameBuffer[cardIndex], PATHLEN);
	strncpy(ok_title, fileTitleBuffer[cardIndex], STRLEN);
	if(!ok_filename[0])  return 0;

	sprintf(buf, "ÀñÆ·µê%sÊÕÒøÌ¨", kind);
	money_show_stat(buf);
	//ok_title= Ãµ¹å»¨1(Ö¦)   ¼Û:100bmyb
	ptr1= strstr(ok_title,"¼Û:");
	if(!ptr1){
		move(7,10);
		prints("My God! ±¾ÉÌÆ·»¹Ã»ÓĞ¶¨¼Û£¬¸Ï¿ìÈ¥¸æËßÀñÆ·µêÀÏ°å°É");
		pressanykey();
		return 0;
	}
	ptr1+=3;
	if(!ptr1){
		move(7,10);
		prints("My God! ±¾ÉÌÆ·»¹Ã»ÓĞ¶¨¼Û£¬¸Ï¿ìÈ¥¸æËßÀñÆ·µêÀÏ°å°É");
		pressanykey();
		return 0;
	}
	ptr2= strstr(ptr1,"bmyb");
	if(!ptr2){
		move(7,10);
		prints("My God! ±¾ÉÌÆ·¶¨¼ÛÓĞÎÊÌâ£¬¸Ï¿ìÈ¥¸æËßÀñÆ·µêÀÏ°å°É");
		pressanykey();
		return 0;
	}
	*ptr2='\0';
	price_per = atoi(ptr1);
	*ptr2='b';
	if(price_per<0){
			move(7,10);
			prints("My God! ±¾ÉÌÆ·¶¨¼ÛÓĞÎÊÌâ£¬¸Ï¿ìÈ¥¸æËßÀñÆ·µêÀÏ°å°É");
			pressanykey();
			return 0;
	}else if(price_per == 0){
			move(7,10);
			prints("ºÙºÙ! ±¾ÉÌÆ·Ãâ·ÑÔùËÍ£¬ÒÔºóÒª¶à¶àÖ§³Ö±¾µêà¸");
			pressanykey();
	}

	ptr1-=4;
	while(*ptr1==' ') ptr1--;
	*(ptr1+1) = '\0';

	move(8,10);
	sprintf(genbuf, "ÄãÈ·¶¨Òª¸¶Ç®¹ºÂò%sÂğ",ok_title);
	if (askyn(genbuf, YEA, NA) == NA)
		return 0;
	ma =
		buy_present(order, kind, ok_title, ok_filename, price_per, touserid);
	if (ma==9) return 9;
	return 1;
}


static int
buy_present(int order, char *kind, char *cardname, char *filepath, int price_per,char *touserid)
{
	int i;
	int inputNum=1;
	char uident[IDLEN + 1], note[3][STRLEN], tmpname[STRLEN];
	int price;
	char buf[200];
	char *ptr1,*ptr2;
	char unit[STRLEN];

	clear();
	ansimore2(filepath, 0, 0, 25);
	move(t_lines - 2, 0);
	sprintf(genbuf, "ÇëÊäÈëÒª¹ºÂòµÄÊıÁ¿[%d]: ",inputNum);
	while(1) {
		inputNum = 1;
		getdata(15, 4, genbuf, buf, 8, DOECHO, YEA);
		if(buf[0] == '\0' || (inputNum = atoi(buf)) >= 1)
			break;
	}
	price = price_per*inputNum;
	//¼Ó¸öÏŞÖÆ
	if (price < 0 || price > MAX_MONEY_NUM){
		move(t_lines - 2, 4);
		prints("´ó×Ú»õÎïÇëÌáÇ°Ô¤Ô¼...");
		pressanykey();
		return 0;
	}

	//cardname= Ãµ¹å»¨1(Ö¦)   ¼Û:100bmyb
	ptr1= strstr(cardname,"(");
	ptr1++;
	if(!ptr1)
		sprintf(buf,"%s","·İ");
	else{
		ptr2=strstr(ptr1,")");
		if(!ptr2)
			sprintf(buf,"%s","·İ");
		else{
			*ptr2='\0';
			strncpy(buf, ptr1, STRLEN);
			}
		if (!strlen(buf))
			sprintf(buf,"%s","·İ");
		ptr1--;
		*ptr1='\0';
		}
	sprintf(unit,"%s",buf);
	sprintf(genbuf, "ÄãÈ·¶¨Òª»¨·Ñ%d±øÂíÙ¸±Ò¹ºÂò%d%s%sÂğ",price,inputNum,unit,cardname);
	if (askyn(genbuf, YEA, NA) == NA)
		return 0;
	if (loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM) < price) {
		move(t_lines - 2, 4);
		prints("ÄãµÄÇ®²»¹»°¡~~~");
		pressanykey();
		return 0;
	}
	saveValue(currentuser.userid, MONEY_NAME, -price, MAX_MONEY_NUM);
	saveValue("millionaires", MONEY_NAME, price, MAX_MONEY_NUM);
	clear();
	if(touserid && touserid[0]){
		strncpy(uident,touserid,IDLEN);
	}else{
		move(0, 0);
		sprintf(buf, "Òª°Ñ%sËÍ¸øË­? ", kind);
		usercomplete(buf, uident);
		if (uident[0] == '\0') {
			move(t_lines - 2, 4);
			clrtobot();
			sprintf(buf, "¾ÓÈ»²»Ğ´µØÖ·£¬ÄãÂòµÄ%s¶ªÊ§ÔÚÓÊ¼ÄÍ¾ÖĞ...", kind);
			prints(buf);
			pressanykey();
			return 0;
			}
		if (!getuser(uident)) {
			move(t_lines - 2, 4);
			clrtobot();
			sprintf(buf, "Ã»ÓĞÕâ¸öÈË°¡£¬ÄãÂòµÄ%s±»ÓÊµİÔ±Ë½ÍÌÁË...", kind);
			prints(buf);
			pressanykey();
			return 0;
			}
		}
	move(2, 0);
	prints("»¹ÓĞÊ²Ã´»°Òª¸½ÉÏÂğ£¿[¿ÉÒÔĞ´3ĞĞà¸]");
	bzero(note, sizeof (note));
	for (i = 0; i < 3; i++) {
		getdata(3 + i, 0, ": ", note[i], STRLEN - 1, DOECHO, NA);
		if (note[i][0] == '\0')
			break;
	}
	sprintf(tmpname, "bbstmpfs/tmp/present.%s.%d", currentuser.userid,
		uinfo.pid);
	copyfile(filepath, tmpname);
	if (i > 0) {
		int j;
		FILE *fp = fopen(tmpname, "a");
		fprintf(fp, "\nÒÔÏÂÊÇ %s µÄ¸½ÑÔ:\n", currentuser.userid);
		for (j = 0; j < i; j++)
			fprintf(fp, "%s", note[j]);
		fclose(fp);
	}
	sprintf(buf,"ËÍÄã%d%s%s£¬Ï²»¶Âğ£¿",inputNum,unit,cardname);
	if (mail_file(tmpname, uident, buf) >= 0) {
		move(8,0);
		sprintf(buf,"ÄãµÄ%sÒÑ¾­·¢³öÈ¥ÁË",kind);
		prints(buf);
		pressanykey();
		return 9; //for marry
	} else {
		move(8,0);
		prints("·¢ËÍÊ§°Ü£¬¶Ô·½ÓÊÏä³¬Èİ");
		pressanykey();
	}
	unlink(tmpname);
	return 0;
}


/* write by dsyan               */
/* 87/10/24                     */
/* Ìì³¤µØ¾Ã Forever.twbbs.org   */

//char card[52], mycard[5], cpucard[5], sty[100], now;
char *card, *mycard, *cpucard, *sty;
int now;
static int
forq(a, b)
char *a, *b;
{
	char c = (*a) % 13;
	char d = (*b) % 13;
	if (!c)
		c = 13;
	if (!d)
		d = 13;
	if (c == 1)
		c = 14;
	if (d == 1)
		d = 14;
	if (c == d)
		return *a - *b;
	return c - d;
}

static void/*¶Ä²©--Ëó¹ş*/
p_gp()
{
	char genbuf[200], hold[5];
	int quit = 0;
	int num, i, j, k, tmp, x, xx, doub, gw = 0, cont = 0, money = 0;
//      int game_times = 0;
	char ans[5] = "", ch = ' ';
	char c1[52], mycard1[5], cpucard1[5], sty1[100];
	char title[STRLEN], buf[256];
	card = c1;
	mycard = mycard1;
	cpucard = cpucard1;
	sty = sty1;
	srandom(time(0));
	num = 0;
	while (!quit) {
		clear();
		nomoney_show_stat("½ğÆË¿ËËó¹ş");
		move(4, 4);
		prints("\033[1;31m¿ª¶¯ÄÔ½î×¬Ç®°¡~~~\033[m");
		move(5, 4);
		prints("Ò»´ÎÑ¹ 100 ±øÂíÙ¸±Ò");
		move(6, 4);
		prints("´óĞ¡:");
		move(7, 4);
		prints
		    ("Í¬»¨Ë³£¾ÌúÖ¦£¾ºùÂ«£¾Í¬»¨£¾Ë³×Ó£¾ÈıÌõ£¾ÍÃÅß£¾µ¥Åß£¾µ¥ÕÅ");
		move(8, 4);
		prints("ÌØÊâ¼Ó·Ö£º");
		move(9, 4);
		prints("Í¬»¨Ë³  £±£µ±¶");
		move(10, 4);
		prints("Ìú¡¡Ö¦  £±£°±¶");
		move(11, 4);
		prints("ºù¡¡Â«¡¡¡¡£µ±¶");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]ÏÂ×¢ [Q]Àë¿ª                                                \033[m");
		if (random() % 40 < 1)
			money_police();
		if (!cont)
			ch = igetkey();
		switch (ch) {
		case '1':
			clear();
			money_show_stat("½ğÆË¿ËËó¹ş");
			if (!cont) {
				ans[0] = 0;
				move(6, 4);
				//if (askyn("ÄúÈ·¶¨ÒªÑ¹100±øÂíÙ¸±ÒÃ´?", YEA, NA) == NA)
					//break;
				getdata(8, 4, "ÄúÑ¹¶àÉÙ±øÂíÙ¸±Ò£¿[100-9999]", genbuf, 5,
					DOECHO, YEA);
				num = atoi(genbuf);
				if (!genbuf[0])
				num = 999;
				if (num < 100) {
				   move(9, 4);
				   prints("ÓĞÃ»ÓĞÇ®°¡£¿ÄÇÃ´µãÇ®ÎÒÃÇ²»´øÍæµÄ");
				   pressanykey();
				   break;
				}
				money = loadValue(currentuser.userid,
					MONEY_NAME, MAX_MONEY_NUM);
				if (money < num) {
					move(8, 4);
					prints("È¥È¥È¥£¬Ã»ÄÇÃ´¶àÇ®µ·Ê²Ã´ÂÒ£¡      \n");
					pressanykey();
					break;
				}
				//num = 100;
				saveValue(currentuser.userid, MONEY_NAME, -num,
						  MAX_MONEY_NUM);
			}
			clear();
			money_show_stat("½ğÆË¿ËËó¹ş");
			move(21, 0);
			if (cont > 0)
				prints
				    ("\033[33;1m (¡û)(¡ú)¸Ä±äÑ¡ÅÆ  (SPACE)¸Ä±ä»»ÅÆ  (Enter)È·¶¨\033[m");
			else
				prints
				    ("\033[33;1m (¡û)(¡ú)¸Ä±äÑ¡ÅÆ  (d)Double  (SPACE)¸Ä±ä»»ÅÆ  (Enter)È·¶¨\033[m");
			move(22, 0);
			prints("µ±Ç°ÏÂ×¢½ğ¶î: %d ±øÂíÙ¸±Ò", num);
			for (i = 0; i < 52; i++)
				card[i] = i;	/* 0~51 ..ºÚ½Ü¿ËÊÇ 1~52 */

			for (i = 0; i < 1000; i++) {
				j = random() % 52;
				k = random() % 52;
				tmp = card[j];
				card[j] = card[k];
				card[k] = tmp;
			}

			now = doub = 0;
			for (i = 0; i < 5; i++) {
				mycard[i] = card[now++];
				hold[i] = 1;
			}
			qsort(mycard, 5, sizeof (char), (void *) forq);

			for (i = 0; i < 5; i++)
				show_card(0, mycard[i], i);

			x = xx = tmp = 0;
			while (tmp != '\r' && tmp != '\n' && tmp != 'k') {
				for (i = 0; i < 5; i++) {
					move(16, i * 4 + 2);
					outs(hold[i] < 0 ? "±£" : "  ");
					move(17, i * 4 + 2);
					outs(hold[i] < 0 ? "Áô" : "  ");
				}
				move(8, xx * 4 + 2);
				outs("  ");
				move(8, x * 4 + 2);
				outs("¡ı");
				move(t_lines - 1, 0);
				xx = x;

				tmp = egetch();
				switch (tmp) {
#ifdef GP_DEBUG
				case KEY_UP:
					getdata(21, 0, "°ÑÅÆ»»³É> ", genbuf, 3,
						DOECHO, YEA);
					mycard[x] = atoi(genbuf);
					qsort(mycard, 5, sizeof (char), forq);
					for (i = 0; i < 5; i++)
						show_card(0, mycard[i], i);
					break;
#endif
				case KEY_LEFT:
				case 'l':
					x = x ? x - 1 : 4;
					break;
				case KEY_RIGHT:
				case 'r':
					x = (x == 4) ? 0 : x + 1;
					break;
				case ' ':
					hold[x] *= -1;
					break;
				case 'd':
					if (!cont && !doub
					    && loadValue(currentuser.userid,
							 MONEY_NAME,
							 MAX_MONEY_NUM) >=
					    num) {
						doub++;
						move(21, 0);
						clrtoeol();
						prints
						    ("\033[33;1m (¡û)(¡ú)¸Ä±äÑ¡ÅÆ  (SPACE)¸Ä±ä»»ÅÆ  (Enter)È·¶¨\033[m");
						saveValue(currentuser.userid,
							  MONEY_NAME, -num,
							  MAX_MONEY_NUM);
						num *= 2;
						move(22, 0);
						prints("µ±Ç°ÏÂ×¢½ğ¶î %d ±øÂíÙ¸±Ò",
						       num);
						//show_money(bet, NULL, NA);
					}
					break;
				}
			}

			for (i = 0; i < 5; i++)
				if (hold[i] == 1)
					mycard[i] = card[now++];
			qsort(mycard, 5, sizeof (char), (void *) forq);
			for (i = 0; i < 5; i++)
				show_card(0, mycard[i], i);
			move(11, x * 4 + 2);
			outs("  ");
			money_cpu();
#ifdef GP_DEBUG
			for (x = 0; x < 5; x++) {
				getdata(21, 0, "°ÑÅÆ»»³É> ", genbuf, 3, DOECHO,
					YEA);
				cpucard[x] = atoi(genbuf);
			}
			qsort(cpucard, 5, sizeof (char), forq);
			for (i = 0; i < 5; i++)
				show_card(1, cpucard[i], i);
#endif
			i = gp_win();

			if (i < 0) {
				saveValue(currentuser.userid, MONEY_NAME,
					  num * 2, MAX_MONEY_NUM);

				sprintf(title, "%s²ÎÓë¶Ä²©(Ëó¹ş)(Ó®)", currentuser.userid);
				sprintf(buf, "%sÔÚËó¹şÓ®ÁË%d±øÂíÙ¸±Ò", currentuser.userid, num);
				millionairesrec(title, buf, "¶Ä²©Ëó¹ş");

				sprintf(genbuf,
					"ÍÛ!ºÃ°ôà¸!!! ¾»×¬ %d Ôª...  :D", num);
				prints("%s", genbuf);
				if (cont > 0)
					sprintf(genbuf,
						"Á¬Ê¤ %d ´Î, Ó®ÁË %d Ôª",
						cont + 1, num);
				else
					sprintf(genbuf, "Ó®ÁË %d Ôª", num);
				num = (num > 50000 ? 100000 : num * 2);
				gw = 1;
			} else if (i > 1000) {
				switch (i) {
				case 1001:
					doub = 15;
					break;
				case 1002:
					doub = 10;
					break;
				case 1003:
					doub = 5;
					break;
				}
				saveValue(currentuser.userid, MONEY_NAME,
					  num * 2 * doub, MAX_MONEY_NUM);

				sprintf(title, "%s²ÎÓë¶Ä²©(Ëó¹ş)(Ó®)", currentuser.userid);
				sprintf(buf, "%sÔÚËó¹şÓ®ÁË%d±øÂíÙ¸±Ò", currentuser.userid, num * 2 * doub - num);
				millionairesrec(title, buf, "¶Ä²©Ëó¹ş");

				sprintf(genbuf, "ÍÛ!ºÃ°ôà¸!!!¾»×¬ %d Ôª...  :D",
					num * 2 * doub - num);
				prints("%s", genbuf);
				if (cont > 0)
					sprintf(genbuf,
						"Á¬Ê¤ %d ´Î, Ó®ÁË %d Ôª",
						cont + 1, num * doub);
				else
					sprintf(genbuf, "Ó®ÁË %d Ôª",
						num * doub);
				num = (num > 5000 ? 10000 : num * 2 * doub);
				gw = 1;
				num = (num >= 10000 ? 10000 : num);
			} else {
				prints("ÊäÁË...:~~~");

				sprintf(title, "%s²ÎÓë¶Ä²©(Ëó¹ş)(Êä)", currentuser.userid);
				sprintf(buf, "%sÔÚËó¹şÊäÁË%d±øÂíÙ¸±Ò", currentuser.userid, num);
				millionairesrec(title, buf, "¶Ä²©Ëó¹ş");

				if (cont > 1)
					sprintf(genbuf,
						"ÖĞÖ¹ %d Á¬Ê¤, ÊäÁË %d Ôª",
						cont, num);
				else
					sprintf(genbuf, "ÊäÁË %d Ôª", num);
				cont = 0;
				num = 0;
				pressanykey();
			}

			if (gw == 1) {
				gw = 0;
				getdata(21, 0, "ÄúÒª°Ñ½±½ğ¼ÌĞøÑ¹×¢Âğ (y/n)?",
					ans, 2, DOECHO, YEA);
				if (ans[0] == 'y' || ans[0] == 'Y') {
					saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);	/* added by soff */
					cont++;
				} else {
					cont = 0;
					num = 0;
				}
			}
			break;
		case 'Q':
		case 'q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
}
static /*¶Ä²©--Ëó¹ş--ÏÔÊ¾ÆË¿ËÅÆ*/
void show_card(isDealer, c, x)
int isDealer, c, x;
{
	int beginL;
	char *suit[4] = { "£Ã", "£Ä", "£È", "£Ó" };
	char *num[13] = { "£Ë", "£Á", "£²", "£³", "£´", "£µ", "£¶",
		"£·", "£¸", "£¹", "10", "£Ê", "£Ñ"
	};

	beginL = (isDealer) ? 4 : 12;
	move(beginL, x * 4);
	outs("¨q©¤©¤©¤¨r");
	move(beginL + 1, x * 4);
	prints("©¦%2s    ©¦", num[c % 13]);
	move(beginL + 2, x * 4);
	prints("©¦%2s    ©¦", suit[c / 13]);	/* ¡ûÕâÀï¸úºÚ½Ü¿Ë */
#ifdef GP_DEBUG
	move(beginL + 3, x * 4);
	prints("©¦%2d    ©¦", c);	/* ÓĞµã²»Í¬à¸!! */
#else
	move(beginL + 3, x * 4);
	outs("©¦      ©¦");	/* ÓĞµã²»Í¬à¸!! */
#endif
	move(beginL + 4, x * 4);
	outs("©¦      ©¦");
	move(beginL + 5, x * 4);
	outs("©¦      ©¦");
	move(beginL + 6, x * 4);
	outs("¨t©¤©¤©¤¨s");
}

static/*¶Ä²©--Ëó¹ş*/
void money_cpu()
{
	char hold[5];
	int i, j;
	char p[13], q[5], r[4];
	char a[5], b[5];

	for (i = 0; i < 5; i++) {
		cpucard[i] = card[now++];
		hold[i] = 0;
	}
	qsort(cpucard, 5, sizeof (char), (void *) forq);
	for (i = 0; i < 5; i++)
		show_card(1, cpucard[i], i);

	money_suoha_tran(a, b, cpucard);
	money_suoha_check(p, q, r, cpucard);

	for (i = 0; i < 13; i++)
		if (p[i] > 1)
			for (j = 0; j < 5; j++)
				if (i == cpucard[j] % 13)
					hold[j] = -1;

	for (i = 0; i < 5; i++) {
		if (a[i] == 13 || a[i] == 1)
			hold[i] = -1;
		move(8, i * 4 + 2);
		outs(hold[i] < 0 ? "±£" : "  ");
		move(9, i * 4 + 2);
		outs(hold[i] < 0 ? "Áô" : "  ");
	}
	move(11, 25);
	prints("\033[44;37mµçÄÔ»»ÅÆÇ°...\033[40m");
	pressanykey();
	move(11, 0);
	clrtoeol();

	for (i = 0; i < 5; i++)
		if (!hold[i])
			cpucard[i] = card[now++];
	qsort(cpucard, 5, sizeof (char), (void *) forq);
	for (i = 0; i < 5; i++)
		show_card(1, cpucard[i], i);
}

static/*¶Ä²©-Ëó¹ş*/
int gp_win()
{
	int my, cpu, ret = 0;
	char myx, myy, cpux, cpuy;

	my = complex(mycard, &myx, &myy);
	cpu = complex(cpucard, &cpux, &cpuy);
	show_style(my, cpu);

	if (my != cpu)
		ret = my - cpu;
	else if (myx == 1 && cpux != 1)
		ret = -1;
	else if (myx != 1 && cpux == 1)
		ret = 1;
	else if (myx != cpux)
		ret = cpux - myx;
	else if (myy != cpuy)
		ret = cpuy - myy;

	if (ret < 0)
		switch (my) {
		case 1:
			ret = 1001;
			break;
		case 2:
			ret = 1002;
			break;
		case 3:
			ret = 1003;
			break;
		}

	return ret;
}

//Í¬»¨Ë³¡¢ÌúÖ¦¡¢ºù¡¢Í¬»¨¡¢Ë³¡¢ÈıÌõ¡¢ÍÃÅß¡¢Åß¡¢Ò»Ö»
static
    int
complex(cc, x, y)
char *cc, *x, *y;
{
	char p[13], q[5], r[4];
	char a[5], b[5], c[5], d[5];
	int i, j, k;

	money_suoha_tran(a, b, cc);
	money_suoha_check(p, q, r, cc);

	/* Í¬»¨Ë³ */
	if ((a[0] == a[1] - 1 && a[1] == a[2] - 1 && a[2] == a[3] - 1
	     && a[3] == a[4] - 1) && (b[0] == b[1] && b[1] == b[2]
				      && b[2] == b[3] && b[3] == b[4])) {
		*x = a[4];
		*y = b[4];
		return 1;
	}
	if (a[4] == 1 && a[0] == 2 && a[1] == 3 && a[2] == 4 && a[3] == 5 &&
	    (b[0] == b[1] && b[1] == b[2] && b[2] == b[3] && b[3] == b[4])) {
		*x = a[3];
		*y = b[4];
		return 1;
	}
	if (a[4] == 1 && a[0] == 10 && a[1] == 11 && a[2] == 12 && a[3] == 13 &&
	    (b[0] == b[1] && b[1] == b[2] && b[2] == b[3] && b[3] == b[4])) {
		*x = 1;
		*y = b[4];
		return 1;
	}
	/*ÌúÖ¦  */
	if (q[4] == 1) {
		for (i = 0; i < 13; i++)
			if (p[i] == 4)
				*x = i ? i : 13;
		return 2;
	}
	/* ºùÂ« */
	if (q[3] == 1 && q[2] == 1) {
		for (i = 0; i < 13; i++)
			if (p[i] == 3)
				*x = i ? i : 13;
		return 3;
	}
	/* Í¬»¨ */
	for (i = 0; i < 4; i++)
		if (r[i] == 5) {
			*x = i;
			return 4;
		}
	/* Ë³×Ó */
	memcpy(c, a, 5);
	memcpy(d, b, 5);
	for (i = 0; i < 4; i++)
		for (j = i; j < 5; j++)
			if (c[i] > c[j]) {
				k = c[i];
				c[i] = c[j];
				c[j] = k;
				k = d[i];
				d[i] = d[j];
				d[j] = k;
			}
	if (10 == c[1] && c[1] == c[2] - 1 && c[2] == c[3] - 1
	    && c[3] == c[4] - 1 && c[0] == 1) {
		*x = 1;
		*y = d[0];
		return 5;
	}
	if (c[0] == c[1] - 1 && c[1] == c[2] - 1 && c[2] == c[3] - 1
	    && c[3] == c[4] - 1) {
		*x = c[4];
		*y = d[4];
		return 5;
	}
	/* ÈıÌõ */
	if (q[3] == 1)
		for (i = 0; i < 13; i++)
			if (p[i] == 3) {
				*x = i ? i : 13;
				return 6;
			}
	/* ÍÃÅß */
	if (q[2] == 2) {
		for (*x = 0, i = 0; i < 13; i++)
			if (p[i] == 2) {
				if ((i > 1 ? i : i + 13) > (*x == 1 ? 14 : *x)) {
					*x = i ? i : 13;
					*y = 0;
					for (j = 0; j < 5; j++)
						if (a[j] == i && b[j] > *y)
							*y = b[j];
				}
			}
		return 7;
	}
	/* Åß */
	if (q[2] == 1)
		for (i = 0; i < 13; i++)
			if (p[i] == 2) {
				*x = i ? i : 13;
				*y = 0;
				for (j = 0; j < 5; j++)
					if (a[j] == i && b[j] > *y)
						*y = b[j];
				return 8;
			}
	/* Ò»ÕÅ */
	*x = 0;
	*y = 0;
	for (i = 0; i < 5; i++)
		if ((a[i] = a[i] ? a[i] : 13 > *x || a[i] == 1) && *x != 1) {
			*x = a[i];
			*y = b[i];
		}
	return 9;
}

/* a ÊÇµãÊı .. b ÊÇ»¨É« */
static void/*¶Ä²©--Ëó¹ş*/
money_suoha_tran(a, b, c)
char *a, *b, *c;
{
	int i;
	for (i = 0; i < 5; i++) {
		a[i] = c[i] % 13;
		if (!a[i])
			a[i] = 13;
	}

	for (i = 0; i < 5; i++)
		b[i] = c[i] / 13;
}

static void/*¶Ä²©--Ëó¹ş*/
money_suoha_check(p, q, r, cc)
char *p, *q, *r, *cc;
{
	int i;

	for (i = 0; i < 13; i++)
		p[i] = 0;
	for (i = 0; i < 5; i++)
		q[i] = 0;
	for (i = 0; i < 4; i++)
		r[i] = 0;

	for (i = 0; i < 5; i++)
		p[cc[i] % 13]++;

	for (i = 0; i < 13; i++)
		q[(int) p[i]]++;

	for (i = 0; i < 5; i++)
		r[cc[i] / 13]++;
}

//Í¬»¨Ë³¡¢ÌúÖ¦¡¢ºù¡¢Í¬»¨¡¢Ë³¡¢ÈıÌõ¡¢ÍÃÅß¡¢Åß¡¢Ò»Ö»
static void/*¶Ä²©--Ëó¹ş*/
show_style(my, cpu)
char my, cpu;
{
	char *style[9] = { "Í¬»¨Ë³", "ÌúÖ¦", "ºùÂ«", "Í¬»¨", "Ë³×Ó",
		"ÈıÌõ", "ÍÃÅß", "µ¥Åß", "Ò»ÕÅ"
	};
	move(5, 26);
	prints("\033[41;37;1m%s\033[m", style[cpu - 1]);
	move(15, 26);
	prints("\033[41;37;1m%s\033[m", style[my - 1]);
	sprintf(sty, "ÎÒµÄÅÆ\033[44;1m%s\033[m..µçÄÔµÄÅÆ\033[44;1m%s\033[m",
		style[my - 1], style[cpu - 1]);
}

static void/*¶Ä²©--ÂÖÅÌ*/
russian_gun()
{

	int i;
	int line;
	int first;
	char uident[IDLEN + 1];
	char title[STRLEN], buf[256];

	clear();
	money_show_stat("±øÂíÙ¸¶Ä³¡´óÌü");
	////slowaction
	if (currentuser.stay < 86400) {
		move(7, 4);
		prints
		    ("Ğ¡º¢×ÓÀ´Æ´Ê²Ã´Ãü£¬ÕÒÄãÃÇ¼Ò´óÈËÀ´¡£\n¹Ô£¬¸øÄãÒ»¿é±øÂíÙ¸±ÒÂòÌÇ³Ô");
		pressanykey();
		return;
	}
	//slowaction

	move(6, 4);
	if (!loadValue(currentuser.userid, "invitation", 1)) {
		prints
		    ("Öµ°àÃØÊéÉÏÏÂ´òÁ¿ÁËÄã°ëÉÎ£¬ËµµÀ£º¡°ÕâÀïÃ»ÓĞÕâÖÖ¶Ä·¨£¬Äã×ß°É¡£¡±");
		pressanykey();
	} else {
		saveValue(currentuser.userid, "invitation", -1, 1);
		whoTakeCharge(3, uident);
		prints
		    ("Öµ°à¾­Àí¿´ÍêÄãµİ¹ıµÄÑûÇëº¯£¬ÓÖËÄÏÂ¿´ÁË¿´£¬²ÅËµµÀ£º¡°ÇëËæÎÒÀ´¡£¡±");
		pressanykey();
		clear();
		money_show_stat("±øÂíÙ¸¶Ä³¡ÃÜÊÒ");
		move(4, 4);
		prints
		    ("ÕâÀïÊÇÒ»¼ä²»´óµÄÃÜÊÒ£¬ºÜ¾²£¬¾²µÃ¿ÉÅÂ¡£¿ÕÆøÖĞËÆºõÓĞÑªĞÈµÄÎ¶µÀ...");
		move(6, 4);
		prints
		    ("%s¾Í×øÔÚÃæÇ°£¬Î¢Ğ¦µÀ£º¡°Äã¸ÒÀ´¸°Ô¼£¬ËãÄãÓĞµ¨Á¿£¡Çë×ø¡£¡±",
		     uident);
		pressanykey();
		move(8, 4);
		prints("Ò»°Ñ×óÂÖÊÖÇ¹ÈÓµ½ÁË×ÀÉÏ...Ò»¸öÃÉÃæÄĞ×Ó×øµ½ÁËÄãÃæÇ°...");
		move(10, 4);
		if (askyn("ÄãÖªµÀ¶íÂŞË¹ÂÖÅÌ¶ÄµÄ¹æÔòÂğ£¿", NA, NA) == NA) {
			move(12, 4);
			prints
			    ("%sÌ¾ÁË¿ÚÆø£¬ËµµÀ£º¡°ËãÁË£¬±ğËÀÁËÁ¬ÔõÃ´»ØÊÂ¶¼²»ÖªµÀ¡£Äã×ß°É£¡¡±",
			     uident);
			pressanykey();
			return;
		}
		move(12, 4);
		if (askyn
		    ("ºÃ£¡ÄÇ¾Í¿ªÊ¼°É£¬×£ÄãºÃÔË¡£ÄãÊÇ¿Í£¬ÄãÒªÏÈÀ´Âğ£¿", NA,
		     NA) == YEA) {
			first = 1;
		} else {
			first = 0;
		}
		clear();
		money_show_stat("±øÂíÙ¸¶Ä³¡ÃÜÊÒ");
		set_safe_record();
		currentuser.dietime = currentuser.stay + 4444 * 60;
		substitute_record(PASSFILE,
				  &currentuser, sizeof (currentuser), usernum);
		for (i = 0, line = 6; i < 6; i += 2) {
			srandom(time(0));
			move(line++, 4);
			if (first) {
				prints
				    ("ÄãÄÃÆğ×óÂÖÊÖÇ¹£¬¶Ô×¼×Ô¼ºµÄÌ«ÑôÑ¨£¬Ò§Ò§ÑÀ¿Û¶¯ÁË°â»ú...");
			} else {
				prints
				    ("ÃÉÃæÄĞ×ÓÄÃÆğ×óÂÖÊÖÇ¹£¬¶Ô×¼×Ô¼ºµÄÌ«ÑôÑ¨£¬¿Û¶¯ÁË°â»ú...");
			}
			pressanykey();
			if (random() % (6 - i)) {
				move(line++, 4);
				if (first) {
					prints
					    ("\033[1;33mßÇßÕ£¡\033[mÒ»ÉùÏì¹ı£¬Äã¾ª»êÎ´¶¨Ö®Óà·¢ÏÖ×Ô¼º»¹»î×Å...");
				} else {
					prints
					    ("\033[1;33mßÇßÕ£¡\033[mÒ»ÉùÏì¹ı£¬ÃÉÃæÄĞ×ÓºÁ·¢ÎŞËğ...");
				}
				move(line++, 4);
				if (5 - i == 1 && first) {
					if (first) {
						move(line++, 4);
						prints
						    ("ÃÉÃæÄĞ×Ó¾øÍûµÄ²ü¶¶×Å£¬ÓÃ°§ÇóµÄÑÛÉñ¿´×Å%s¡£",
						     uident);
						move(line++, 4);
						prints
						    ("\033[1;31mÅé£¡Ò»Éù¾ŞÏì£¬ÃÉÃæÄĞ×ÓÑªÁ÷ÂúµØ...\033[m");
						pressanykey();
						move(line++, 4);
						prints
						    ("%s²ÁÁË²Á»¹ÔÚÃ°ÑÌµÄÊÖÇ¹£¬ÓÖ·Å½øÁË¿Ú´üÀï¡£",
						     uident);
						break;
					} else {
						prints
						    ("ÃÉÃæÄĞ×ÓµÃÒâµÄÄüĞ¦×Å£¬°ÑÇ¹¿Ú¶Ô×¼ÁËÄã...");
						prints
						    ("\033[1;31mÅé£¡Ò»Éù¾ŞÏì£¬ÄãÖ»¾õµÃÒâÊ¶Ë²¼äÄ£ºı...\033[m");
						pressanykey();
						Q_Goodbye();
					}
				}

				if (first) {
					prints("ÏÖÔÚÂÖµ½ÃÉÃæÄĞ×Ó...");
				} else {
					prints
					    ("ÏÖÔÚÂÖµ½ÄãÁË...ÄãµÄĞÄÔà\033[5;31mÅéÅé\033[mÌøµÃÀ÷º¦...");
				}
				pressanykey();
				if (random() % (5 - i)) {
					move(line++, 4);
					if (first) {
						prints
						    ("\033[1;33mßÇßÕ£¡\033[mÒ»ÉùÏì¹ı£¬ÃÉÃæÄĞ×ÓºÁ·¢ÎŞËğ...");
					} else {
						prints
						    ("\033[1;33mßÇßÕ£¡\033[mÒ»ÉùÏì¹ı£¬Äã¾ª»êÎ´¶¨Ö®Óà·¢ÏÖ×Ô¼º»¹»î×Å...");
					}
				} else {
					move(line++, 4);
					if (first) {
						prints
						    ("\033[1;31mÅé£¡Ò»Éù¾ŞÏì£¬ÃÉÃæÄĞ×ÓÑªÁ÷ÂúµØ...\033[m");
						break;
					} else {
						prints
						    ("\033[1;31mÅé£¡Ò»Éù¾ŞÏì£¬ÄãÖ»¾õµÃÒâÊ¶Ë²¼äÄ£ºı...\033[m");
						pressanykey();
						Q_Goodbye();
					}
				}
			} else {
				move(line++, 4);
				if (first) {
					prints
					    ("\033[1;31mÅé£¡Ò»Éù¾ŞÏì£¬ÄãÖ»¾õµÃÒâÊ¶Ë²¼äÄ£ºı...\033[m");
					pressanykey();
					Q_Goodbye();
				} else {
					prints
					    ("\033[1;31mÅé£¡Ò»Éù¾ŞÏì£¬ÃÉÃæÄĞ×ÓÑªÁ÷ÂúµØ...\033[m");
					break;
				}
			}
		}
		move(line++, 4);
		set_safe_record();
		currentuser.dietime = 0;
		substitute_record(PASSFILE,
				  &currentuser, sizeof (currentuser), usernum);
		prints
		    ("Ò»ÇĞ¶¼½áÊøÁË...ÄãÒ»¿ÌÒ²²»Ô¸ÁôÔÚÕâ¿Ö²ÀµÄµØ·½£¬¾¡¹ÜÄãµÃµ½ÁË200000 ±øÂíÙ¸±Ò¡£");
		saveValue(currentuser.userid, MONEY_NAME,200000,
			  MAX_MONEY_NUM);

		sprintf(title, "%s²ÎÓë¶Ä²©(ÂÖÅÌ)", currentuser.userid);
		sprintf(buf, "%sÔÚÂÖÅÌÓ®ÁË%d±øÂíÙ¸±Ò", currentuser.userid, 200000);
		millionairesrec(title, buf, "¶Ä²©ÂÖÅÌ");

		pressanykey();
	}
}

static void
policereport(char *str)
{
	FILE *se;
	char fname[STRLEN], title[STRLEN];

	sprintf(fname, "bbstmpfs/tmp/police.%s.%05d", currentuser.userid,
		uinfo.pid);
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "%s", "¡¾´ËÆªÎÄÕÂÓÉ±øÂíÙ¸´ó¸»ÎÌ×Ô¶¯ÕÅÌùÏµÍ³·¢±í¡¿\n\n");
		fprintf(se, "%s", str);
		fclose(se);
		sprintf(title,"[±¨¸æ]%s", str);
		postfile(fname, "Police", title, 2);
		unlink(fname);
	}
}


static int/*¾¯Êğ*/
money_cop()
{
	int ch;
	int quit = 0;
	char uident[IDLEN + 1];
	char buf[200], title[STRLEN];
	int robTimes;
	int seized;
	int die = 0;
	int id;
	int escTime;
	int money=0;

	while (!quit) {
		clear();
		nomoney_show_stat("±øÂíÙ¸¾¯Êğ");
		move(8, 16);
		prints("´ò»÷·¸×ï£¬Î¬³ÖÖÎ°²£¡");
		move(t_lines - 1, 0);
		prints
		    ("\033[1;44m Ñ¡µ¥ \033[1;46m [1]±¨°¸ [2]×ÔÊ× [3]Í¨¼©°ñ [4]ĞÌ¾¯¶Ó [5]Êğ³¤°ì¹«ÊÒ [Q]Àë¿ª\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			clear();
			nomoney_show_stat("±øÂíÙ¸¾¯Êğ½Ó´ıÌü");
			move(4, 4);
			prints("Èç¹ûÄúÔâÓöÇÀ½Ù»òÍµÇÔ£¬Èç¹ûÄúÓĞÈÎºÎ·¸×ïÏÓÒÉÈËµÄÏßË÷£¬ÇëÏò¾¯·½±¨¸æ¡£\n    ÕıÈ·¾Ù±¨ÓĞ½±£¬·Ì°ùËûÈËÊÜ·£");
			money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
			if(money<5000)
			{
				break;
				return 0;
			}
			move(6, 4);
			prints("¾¯ÃñºÏ×÷£¬¹²´´°²¶¨´óºÃ¾ÖÃæ£¡");
			move(7, 4);
			usercomplete("¾Ù±¨Ë­£¿", uident);
			move(8, 4);
			if (uident[0] == '\0')
				break;
			if (!getuser(uident)) {
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressreturn();
				break;
			}
			if (lookupuser.dietime > 0) {
				prints("ÈË¶¼ËÀÁË£¬¾¯²ìÒ²Ã»°ì·¨...");
				//saveValue(currentuser.userid, MONEY_NAME,-2000, MAX_MONEY_NUM);
				pressreturn();
				break;
			}
			if (loadValue(uident, "freeTime", 2000000000) > 0) {
				prints("Õâ¸öÈËÒÑ¾­±»¾¯Êğ¼à½ûÁË¡£");
				//saveValue(currentuser.userid, MONEY_NAME, -2000, MAX_MONEY_NUM);
				pressanykey();
				break;
			}
			if (loadValue(uident, "rob", 50) == 0) {
				prints
				    ("Õâ¸öÈË×î½üºÜ°²·Ö°¡£¡Äã²»Òª·Ì°ù±ğÈËÅ¶£¡");
				saveValue(currentuser.userid, MONEY_NAME, -2000, MAX_MONEY_NUM);
				pressanykey();
				break;
			}
			if (seek_in_file(DIR_MC "criminals_list", uident)) {
				prints
				    ("´ËÈËÒÑ¾­±»¾¯ÊğÍ¨¼©ÁË£¬²»¹ı¾¯ÊğÈÔÈ»¶ÔÄã±íÊ¾¸ĞĞ»¡£");
				saveValue(currentuser.userid, MONEY_NAME, -2000, MAX_MONEY_NUM);
				pressanykey();
				break;
			}
			getdata(8, 4, "¼òÊö°¸Çé£º", genbuf, 40, DOECHO, YEA);
			if (genbuf[0] == '\0')
				break;
			move(9, 4);
			if (askyn("\033[1;33mÄãÏò¾¯·½Ìá¹©µÄÉÏÊöĞÅÏ¢ÕæÊµÂğ£¿\033[0m", NA, NA) == NA)
				break;
			saveValue(currentuser.userid, MONEY_NAME, +2000, MAX_MONEY_NUM);
			strcpy(buf, uident);
			strcat(buf, "\t");
			strcat(buf, genbuf);
			addtofile(DIR_MC "criminals_list", buf);
			move(10, 4);
			prints
			    ("¾¯·½·Ç³£¸ĞĞ»ÄúÌá¹©µÄÏßË÷£¬ÎÒÃÇ½«¾¡Á¦¾¡¿ìÆÆ°¸¡£");
			pressanykey();
			sprintf(buf, "ID: %s\n°¸Çé: %s", uident, genbuf);
			sprintf(genbuf, "%s±¨°¸",currentuser.userid);
			millionairesrec(genbuf, buf, "");
			break;
		case '2':
			clear();
			nomoney_show_stat("±øÂíÙ¸¾¯Êğ½Ó´ıÌü");
			move(4, 4);
			prints("Ì¹°×´Ó¿í£¬¿¹¾Ü´ÓÑÏ¡£");
			move(5, 4);
			prints("Ö÷¶¯½»´ú×Ô¼ºµÄ×ïĞĞ,½«¼õÇáÒ»°ëµÄ´¦·£¡£");
			move(7, 4);
			robTimes = loadValue(currentuser.userid, "rob", 50);
			if (robTimes == 0) {
				prints("ÄãÓĞ²¡°¡£¿Ã»ÊÂÅÜÀ´ÈÏ×ï...");
				pressanykey();
				break;
			}
			if (time(0) <12*3600 + loadValue(currentuser.userid, "last_rob", 2000000000)) {
				prints("ÕâÃ´¿ì¾ÍÀ´ÈÏ×ï£¬²»ĞĞ");
				pressanykey();
				break;
			}
			sprintf(genbuf,
				"\033[1;31mÄãµÄÂÉÊ¦ÌáĞÑÄã,Èç¹ûÈÏ×ïÄã½«±»´¦ÒÔ%dÌì¼à½û¡£»¹ÒªÈÏ×ïÂğ?\033[0m",
				robTimes / 2 + 1);
			move(8, 4);
			if (askyn(genbuf, NA, NA) == YEA) {
				move(9, 4);
				if (loadValue(currentuser.userid, "freeTime", 2000000000) > 0) {
					prints
					    ("ÄãÒÑ¾­±»¼à½ûÁË£¬ÏëÈÏ×ïÒ²À´²»¼°ÁË¡£");
					pressanykey();
					Q_Goodbye();
				}
				prints("ĞüÑÂÀÕÂí,»¹À´µÃ¼°¡£ºÃºÃ¸ÄÔì°É£¡");
				saveValue(currentuser.userid, "freeTime",
					  time(0) + 86400 * (robTimes / 2 + 1), 2000000000);
				saveValue(currentuser.userid, "rob", -robTimes, 50);
				del_from_file(DIR_MC "criminals_list", currentuser.userid);
				pressanykey();
				Q_Goodbye();
			} else {
				move(9, 4);
				prints("¶ãµÃÁË³õÒ»£¬¶ã²»¹ıÊ®Îå¡£ºÃ×ÔÎªÖª°É£¡");
				pressanykey();
			}
			break;
		case '3':
			clear();
			move(1, 0);
			prints("±øÂíÙ¸¾¯Êğµ±Ç°Í¨¼©µÄ·¸×ïÏÓÒÉÈË:");
			listfilecontent(DIR_MC "criminals_list");
			pressanykey();
			break;
		case '4':
			clear();
			nomoney_show_stat("ĞÌ¾¯¶Ó");
			move(6, 4);
			money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
			if (seek_in_file(DIR_MC "mingren", currentuser.userid))
			{
				move(5, 4);
				prints("    \033[1;32m  ²»ÒªÈÇÊÂ\033[m");
				pressanykey();
				break;
			}
			if (!seek_in_file(DIR_MC "policemen", currentuser.userid)||money<5000) {
				prints
				    ("ÕâÀï²»ÊÇ¹«Ô°£¡ÓĞÊÂµ½½Ó´ıÌüÈ¥,±ğµ½´¦ÂÒ´³£¡\n");
                		prints
				    ("²»ÄÃÇ®¾ÍÈ¥×¥ÈË£¬³öÁËÊÂÁ¬Ò½Ò©·Ñ¶¼Ã»ÓĞ\n");
				pressanykey();
				break;
			}
			usercomplete("½ñ´ÎĞĞ¶¯µÄÄ¿±ê·¸×ïÏÓÒÉÈËÊÇ:", uident);
			move(7, 4);
			if (uident[0] == '\0')
				break;
			if (!(id = getuser(uident))) {
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressreturn();
				break;
			}
			if(ifinprison(lookupuser.userid))
			{prints("¶¼ÒÑ¾­ÔÚ¼àÓüÁË£¬»¹Òª×¥ËûÈ¥ÄÇÀï¡£¡£¡£¡£");
				pressreturn();
				break;
			}
			if (lookupuser.dietime > 0) {
				prints("ÈË¶¼ËÀÁË£¬¾¯²ìÒ²Ã»°ì·¨...");
				pressreturn();
				break;
			}
			if (loadValue(uident, "freeTime", 2000000000) > 0) {
				prints("Õâ¸öÈËÒÑ¾­±»¾¯Êğ¼à½ûÁË¡£");
				pressanykey();
				break;
			}
			if (time(0) < 5*60 + loadValue(currentuser.userid, "last_catch", 2000000000)) {
				prints("×¥ÈË²»ÓÃÕâÃ´»ı¼«°É");
				pressanykey();
				break;
			}
			robTimes = loadValue(uident, "rob", 50);
			if (robTimes == 0) {
				prints("Õâ¸öÈË×î½üºÜ°²·Ö°¡£¡»á²»»áÊÇ¸ã´íÁË?\n");
                		prints("Ëû¸æÄãÃÇÀÄÓÃÖ°È¨£¬ÄãÅâÁËËû5000µÄ¾«ÉñËğÊ§·Ñ");
                		saveValue(currentuser.userid, MONEY_NAME, -5000, MAX_MONEY_NUM);
                		saveValue(uident, MONEY_NAME, +5000, MAX_MONEY_NUM);
				pressanykey();
				break;
			}
			escTime = loadValue(uident, "escTime", 2000000000);
			if (escTime > 0 && time(0) < escTime + 3600) {
				prints
				    ("¸Ã·¸×ïÏÓÒÉÈË¸Õ¸ÕÌÓÍÑ,Ò»Ê±°ë»á¶ù¿ÖÅÂ»¹ÕÒ²»µ½¡£");
				pressanykey();
				break;
			}
			move(8, 4);
			if (askyn("×¼±¸ºÃÁËÂğ?", NA, NA) == YEA) {
				saveValue(currentuser.userid, "last_catch", -2000000000, 2000000000);
				saveValue(currentuser.userid, "last_catch", time(0), 2000000000);
				move(10, 4);
				prints
				    ("\033[1;33m¸ù¾İÏßÈËÌá¹©µÄÏûÏ¢,ÄãÖÕÓÚÕÒµ½ÁË%s²ØÄäµÄµØ·½¡£\033[0m",
				     uident);
				move(11, 4);
				seized = 0;
				srandom(time(0));
				if (askyn("\033[5;31mÒªÆÆÃÅ¶øÈëÃ´?\033[0m", NA, NA) == YEA) {
					move(12, 4);
					prints
					    ("\033[1;31mÄã°Î³öÊÖÇ¹£¬Ò»½Å½«ÃÅõß¿ª£¬³åÁË½øÈ¥£¬º°µÀ£º¡°¾¯²ì£¡¡±\033[0m");
					move(13, 4);
					if (random() % 10 == 0) {
						prints
						    ("\033[1;32mÀïÃæ¿ÕÎŞÒ»ÈË£¬´°»§ÊÇ´ò¿ªµÄ¡£¿´À´%s¸Õ¸ÕÌø´°¶øÌÓ¡£\033[0m",
						     uident);
						move(14, 4);
						prints
						    ("ÄãÖ»ºÃ°ÃÄÕ¶ø·µ¡£´óºÃµÄ»ú»á°¡£¡");
						saveValue(uident, "escTime", -2000000000, 2000000000);
						saveValue(uident, "escTime", time(0), 2000000000);
						pressanykey();
						sprintf(buf,"%sÌÓÍÑ",uident);
						policereport(buf);
						sprintf(title, "%s²ÎÓë×¥ÈË", currentuser.userid);
						millionairesrec(title, buf, "¾¯Êğ»î¶¯");
						break;
					} else {
						if (robTimes < 3 && random() % 10) {
							prints("\033[1;32m%sÒ»¿´µ½Äã¶ÙÊ±ÏÅÉµÁË,¹Ô¹Ô¾ÙÆğÁËË«ÊÖ¡£\033[0m",
							     uident);
                           				sprintf(genbuf,
									"±øÂíÙ¸¾¯ÊğÔÚ½ñÌìµÄ×¥²¶ĞĞ¶¯ÖĞ×¥»ñÒ»Ãû´ÂºÅ%sµÄ·ËÍ½\n¾¯·½Í¸Â¶×¥²¶¹ı³Ì·Ç³£Ë³Àû\n\n"
									 "¾¯ÊğÏ£Íû²»Á¼·Ö×ÓÒıÒÔÎª½ä£¬\n ±¾Õ¾¾ÓÃñ¸ß¶ÈÔŞÑï¾¯ÊğÖ°Ô±ÎªÃñ³ıº¦ ", uident);
                                   		deliverreport("[ĞÂÎÅ]±øÂíÙ¸¾¯ÊğÇÜ»ñÒ»Ãû·ËÍ½",genbuf);
							//saveValue(currentuser.userid, MONEY_NAME, robTimes*80000*0.3, MAX_MONEY_NUM);
							move(14, 4);
							seized = 1;
						} else if (robTimes >= 3 && robTimes < 6 && random() % 5) {
							prints("\033[1;32m%sÒ»¿´µ½Äã¾ÍÒªÌø´°ÌÓÅÜ£¬µ«ÄãÑÛÃ÷ÊÖ¿ì£¬Ò»Ç¹»÷ÖĞÆäĞ¡ÍÈ¡£\033[0m",
							     uident);
							sprintf(genbuf,
									"±øÂíÙ¸¾¯ÊğÔÚ½ñÌìµÄ×¥²¶ĞĞ¶¯ÖĞ×¥»ñÒ»Ãû´ÂºÅ%sµÄ·ËÍ½\n¾¯·½Í¸Â¶´ËÈËÔÚÓë¾¯²ìµÄÇ¹Õ½ÖĞ¸ºÉË\n\n"
									 "¾¯ÊğÏ£Íû·¸×ï·Ö×Ó²»Òª¾Ü²¶£¬\n ÒÔÃâÔì³É²»±ØÒªµÄÉËÍö ", uident);
	                               		deliverreport("[ĞÂÎÅ]±øÂíÙ¸¾¯ÊğÇÜ»ñÒ»Ãû·ËÍ½",genbuf);
							//saveValue(currentuser.userid, MONEY_NAME, robTimes*80000*0.3, MAX_MONEY_NUM);
							move(14, 4);
							seized = 1;
						} else if (robTimes >= 6 && robTimes < 8 && random() % 3) {
							prints("\033[1;32m%sÏòÄãÃÍÆË¹ıÀ´£¬ÄãÀ´²»¼°¿ªÇ¹£¬Ö»ºÃºÍÆäÅ¤³ÉÒ»ÍÅ...\033[0m",
							     uident);
							pressanykey();
							move(14, 4);
							prints("\033[1;32m¾­¹ıÒ»·¬²«¶·£¬ÄãÖÕÓÚÖÆ·şÁË%s¡£²»¹ıÄãÒ²ÀÛµÃ¹»Çº£¬»¹±»Ò§ÁËÒ»¿Ú¡£\033[0m",
							     uident);
							sprintf(genbuf,
								"±øÂíÙ¸¾¯ÊğÔÚ½ñÌìµÄ×¥²¶ĞĞ¶¯ÖĞ×¥»ñÒ»Ãû´ÂºÅ%sµÄ·ËÍ½\n¾¯·½Í¸Â¶ÓĞ¾¯Ô±ÔÚÇ¹Õ½ÖĞ¸ºÉË\n\n"
								 "¾¯ÊğÏ£Íû·¸×ï·Ö×Ó²»Òª¾Ü²¶£¬\n ÒÔÃâÔì³É²»±ØÒªµÄÉËÍö ", uident);
                         			      deliverreport("[ĞÂÎÅ]±øÂíÙ¸¾¯ÊğÇÜ»ñÒ»Ãû·ËÍ½",genbuf);
							//saveValue(currentuser.userid, MONEY_NAME, robTimes*80000*0.3, MAX_MONEY_NUM);
							move(15, 4);
							seized = 1;
						} else if (robTimes >= 8 && random() % 2) {
							prints("\033[5;32mÔ­À´%sÒ²ÓĞÇ¹£¡ÄãÃÇÍ¬Ê±Ãé×¼ÁË¶Ô·½£¡\033[0m",
							     uident);
							pressanykey();
							move(14, 4);
							prints("\033[1;35mÇ¹ÉùÏì¹ı£¬%sÍ´¿àµÄÎæ×¡ÁËÊÖÍó£¬ÏÊÑªÖ±Á÷¡£Äã°²È»ÎŞí¦£¬ÇìĞÒ°¡£¡\033[0m",
							     uident);
							//saveValue(currentuser.userid, MONEY_NAME, robTimes*80000*0.3, MAX_MONEY_NUM);
							move(15, 4);
							seized = 1;
						}
						if (seized) {
							prints("Äã½«%sÑº»ØÁË¾¯Êğ,Õâ¸ö»µµ°±»´¦ÒÔ%dÌì¼à½û¡£ÄãÓÖÁ¢ÁËÒ»¹¦£¡",
							     uident, robTimes);
							saveValue(uident, "rob", -robTimes, 50);
							saveValue(uident, "freeTime", time(0) + 86400 * robTimes, 2000000000);
							sprintf(genbuf,
								"Äã±»±øÂíÙ¸¾¯Êğ×¥»ñ£¬²¢´¦ÒÔ%dÌìµÄ¼à½û¡£",
								robTimes);
							mail_buf_slow(uident, "Äã±»¾¯²ì´ş²¶", genbuf, "BMY_FBI");
							del_from_file(DIR_MC "criminals_list", uident);
							sprintf(buf, "%s\t%d", uident, robTimes);
							addtofile(DIR_MC "imprison_list", buf);
							pressanykey();
							sprintf(buf,"×¥»ñ%s£¬²¢¼à½û%dÌì", uident, robTimes);
							policereport(buf);
							sprintf(title, "%s²ÎÓë×¥ÈË", currentuser.userid);
							millionairesrec(title, buf, "¾¯Êğ»î¶¯");
							break;
						} else {
							saveValue(uident, "escTime", -2000000000, 2000000000);
							saveValue(uident, "escTime", time(0), 2000000000);
						}
						if (random() % 20) {
							prints
							    ("\033[5;32mÔ­À´%sÒ²ÓĞÇ¹£¡ÄãÃÇÍ¬Ê±Ãé×¼ÁË¶Ô·½£¡\033[0m",
							     uident);
							move(14, 4);
							if (askyn("\033[1;31mÊÇ·ñ½ô¼±¶ã±Ü£¿", NA, NA) == YEA) {
								move(15, 4);
								if (random() %3) {
									prints("ÄãÒ»¸öºóÑö£¬×Óµ¯´ø×Å·çÉù´ÓÄãÃæÃÅ·É¹ı¡£");
									move(16, 4);
									prints("%s³Ã»úÌÓ×ßÁË£¬Äã²»ÖªµÀÊÇ¸Ã°ÃÄÕ»¹ÊÇÇìĞÒ¡£",
									     uident);
									pressanykey();
									sprintf(buf,"%sÌÓ×ß", uident);
									policereport(buf);
									sprintf(title, "%s²ÎÓë×¥ÈË", currentuser.userid);
									millionairesrec(title, buf, "¾¯Êğ»î¶¯");
									break;
								} else {
									prints("ÄãÏë¶ã±Ü£¬µ«ÊÇÒÑ¾­À´²»¼°ÁË¡£ÄãÖ»¾õµÃĞØ¿ÚÒ»¹ÉÈÈÑªÅçÁË³öÀ´...");
									move(16,4);
									//saveValue(currentuser.userid, MONEY_NAME, 50000, MAX_MONEY_NUM);
									prints("\033[1;31mÄã×³ÁÒÎşÉüÁË¡£\033[0m");
									die = 1;
								}
							}
							else {
								move(15, 4);
								prints("\033[1;31mÏÁÂ·Ïà·êÓÂÕßÊ¤£¡ÄãºÁ²»ÓÌÔ¥µÄ¿ªÇ¹ÁË£¡\033[0m");
								move(16, 4);
								if (random() % 3) {
									prints("\033[1;35mÇ¹ÉùÏì¹ı£¬%s±»»÷ÖĞÍ·²¿£¬µ±³¡ËÀÍö¡£\033[0m",
									     uident);
									move(17, 4);
									prints("ÄãºİºİµÄÌßÁËÒ»½Å%sµÄÊ¬Ìå£¬Í¬Ê±°µ×ÔÇìĞÒ½ñÌì×ßÔË¡£",
									     uident);
 									//saveValue(currentuser.userid, MONEY_NAME, 50000, MAX_MONEY_NUM);
									saveValue(uident, "rob", -robTimes/2, 50);
									lookupuser.dietime = lookupuser.stay + 999*60;
									substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
									sprintf(genbuf,
										"±øÂíÙ¸¾¯ÊğÔÚ½ñÌìµÄ×¥²¶ĞĞ¶¯ÖĞ»÷±ĞÒ»Ãû´ÂºÅ%sµÄ·ËÍ½\n¾¯·½Í¸Â¶´ËÈËÓĞ³ÖÇ¹¾Ü²¶ĞĞÎª\n\n"
										 "¾¯ÊğÏ£Íû²»Á¼·Ö×ÓÒıÒÔÎª½ä£¬\n ±¾Õ¾¾ÓÃñ¸ß¶ÈÔŞÑï¾¯ÊğÖ°Ô±ÎªÃñ³ıº¦ ", uident);
                                   				deliverreport("[ĞÂÎÅ]±øÂíÙ¸¾¯Êğ»÷±ĞÒ»Ãû·ËÍ½",genbuf);
									mail_buf_slow(uident, "Äã±»¾¯²ì»÷±Ğ","ÄãÔÚµÖ¿¹¾¯²ì×¥²¶µÄ¹ı³ÌÖĞ£¬±»Ò»Ç¹»÷ÖĞÍ·²¿ËÀÍö¡£ÉÆ¶ñÖÕÓĞ±¨°¡£¡","BMY_FBI");
									del_from_file(DIR_MC "criminals_list", uident);
									pressanykey();
									sprintf(buf,"»÷±Ğ%s", uident);
									policereport(buf);
									sprintf(title, "%s²ÎÓë×¥ÈË", currentuser.userid);
									millionairesrec(title, buf, "¾¯Êğ»î¶¯");
									break;
								} else {
									prints("Ç¹ÉùÏì¹ı£¬ÄãÖ»¾õµÃĞØ¿ÚÒ»¹ÉÈÈÑªÅçÁË³öÀ´...");
									move(17, 4);
									prints("\033[1;31mÄã×³ÁÒÎşÉüÁË¡£\033[0m");
									die = 1;
								}
							}
						} else {
							prints
							    ("\033[5;32mÔ­À´%sÉí²ØÊÖÀ×£¬Ò»¼ûÌÓÅÜÎŞÍû£¬%sÖ»ºÃÒı±¬ÊÖÀ×ºÍÄãÍ¬¹éÓÚ¾¡£¡\033[0m",
							     uident, uident);
							move(14, 4);
							prints
							    ("\033[1;31mÄã×³ÁÒÎşÉüÁË¡£\033[0m");
							die = 1;
							saveValue(uident, "rob", -robTimes / 2,50);
							lookupuser.dietime = lookupuser.stay + 999 * 60;
							substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
							sprintf(genbuf,
									"±øÂíÙ¸¾¯ÊğÔÚ½ñÌìµÄ×¥²¶ĞĞ¶¯ÖĞ»÷±ĞÒ»Ãû´ÂºÅ%sµÄ·ËÍ½\n¾¯·½Í¸Â¶ÓĞ¾¯Ô±ÔÚÇ¹Õ½ÖĞÖĞµ¯ÉËÊÆÑÏÖØ\n\n"
									 "¾¯Êğ±íÊ¾Ò»¶¨È«Á¦ÇÀ¾È£¬\n  ", uident);
							deliverreport("[ĞÂÎÅ]±øÂíÙ¸¾¯Êğ»÷±ĞÒ»Ãû·ËÍ½",genbuf);
							mail_buf_slow(uident,
								 "Äã±»¾¯²ì´ş²¶","ÄãÔÚµÖ¿¹¾¯²ì×¥²¶µÄ¹ı³ÌÖĞ£¬Òı±¬ÉíÉÏµÄÊÖÀ×£¬Óë¾¯²ìÍ¬¹éÓÚ¾¡¡£","BMY_FBI");
							del_from_file(DIR_MC "criminals_list", uident);
						}
						if (die) {
							set_safe_record();
							saveValue(uident, "rob", -robTimes/2, 50);
							currentuser.dietime = currentuser.stay + 999 * 60;
							substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
							set_safe_record();
							sprintf(buf,
								"Ò»Ãû¾¯Êğ¾¯Ô±ÔÚ½ñÌìµÄ×¥²¶ĞĞ¶¯ÖĞ²»ĞÒÑ³Ö°¡£±øÂíÙ¸¾¯Êğ½÷ÏòÓ¢ĞÛÖÂÒÔ×î¸ßµÄ¾´Òâ£¬"
							"\n²¢·¢ÊÄ½«ÑÏ³Í×ï·¸¡£¾¯ÊğÃ»ÓĞÍ¸Â¶¾¯Ô±µÄÕæÊµĞÕÃû");
							deliverreport("[ĞÂÎÅ]±øÂíÙ¸¾¯ÊğÒ»Ãû¾¯²ìÑ³Ö°", buf);
							pressanykey();
							sprintf(buf,"%sÔÚ×¥²¶%sÊ±Ó¢ÓÂÎşÉüÁË", currentuser.userid, uident);
							policereport(buf);
							sprintf(title, "%s²ÎÓë×¥ÈË", currentuser.userid);
							millionairesrec(title, buf, "¾¯Êğ»î¶¯");
							Q_Goodbye();
						}
					}
				} else {
					move(12, 4);
					prints
					    ("Äã¾ö¶¨»¹ÊÇÏÈ²»Òª´ò²İ¾ªÉßµÄºÃ...");
					pressanykey();
				}
			}
			break;
		case '5':
			clear();
			nomoney_show_stat("Êğ³¤°ì¹«ÊÒ");
			char name[20];
			whoTakeCharge2(8, name);
			whoTakeCharge(8, uident);
			if (strcmp(currentuser.userid, uident)) {
				move(6, 4);
				prints
				    ("¾¯»¨%sÀ¹×¡ÁËÄã,ËµµÀ:¡°Êğ³¤%sÏÖÔÚºÜÃ¦,Ã»Ê±¼ä½Ó´ıÄã¡£¡±",
				     name,uident);
				move(8, 4);
				if (!seek_in_file(DIR_MC "policemen", currentuser.userid)
					&& !slowclubtest("Police",currentuser.userid)){
				if (askyn("ÄãÊÇÏë¼ÓÈë¾¯ÊğÂğ£¿", NA, NA) == YEA) {
						sprintf(genbuf, "%s Òª¼ÓÈë¾¯Êğ", currentuser.userid);
						mail_buf(genbuf, "BMYpolice", genbuf);
						move(14, 4);
						prints("ºÃµÄ£¬ÎÒ»áÍ¨ÖªÊğ³¤µÄ¡£");
					}}
				pressanykey();
				break;
			} else {
				move(6, 4);
				prints("ÇëÑ¡Ôñ²Ù×÷´úºÅ:");
				move(7, 6);
				prints
				    ("1. ÈÎÃü¾¯Ô±                  2. ½âÖ°¾¯Ô±");
				move(8, 6);
				prints
				    ("3. ¾¯Ô±Ãûµ¥                  4. ¼à½ûÃûµ¥");
				move(9, 6);
				prints("5. ´ÇÖ°                      6. ÍË³ö");
				ch = igetkey();
				switch (ch) {
				case '1':
					move(12, 4);
					usercomplete("ÈÎÃüË­Îª¾¯Ô±£¿", uident);
					move(13, 4);
					if (uident[0] == '\0')
						break;
					if (!searchuser(uident)) {
						prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
						pressanykey();
						break;
					}
					if (seek_in_file
					    (DIR_MC "policemen", uident)) {
						prints("¸ÃIDÒÑ¾­ÊÇ¾¯Ô±ÁË¡£");
						pressanykey();
						break;
					}
					if (askyn("È·¶¨Âğ£¿", NA, NA) == YEA) {
						addtofile(DIR_MC "policemen",
							  uident);
						sprintf(genbuf,
							"%s ÈÎÃüÄãÎª±øÂíÙ¸¾¯Êğ¾¯Ô±",
							currentuser.userid);
						mail_buf
						    ("¾¯ÊğÏ£ÍûÄã²»Î·Ç¿±©£¬´ò»÷·¸×ï£¬¹«ÕıÎŞË½£¬²»ÅÂÎşÉü£¡",
						     uident, genbuf);
						move(14, 4);
						prints("ÈÎÃü³É¹¦¡£");
						sprintf(genbuf, "%sĞĞÊ¹¾¯Êğ¹ÜÀíÈ¨ÏŞ",currentuser.userid);
						sprintf(buf, "ÈÎÃü%sÎª¾¯Êğ¾¯Ô±", uident);
						millionairesrec(genbuf, buf, "BMYpolice");
						pressanykey();
					}
					break;
				case '2':
					move(12, 4);
					usercomplete("½âÖ°ÄÄÎ»¾¯Ô±£¿", uident);
					move(13, 4);
					if (uident[0] == '\0')
						break;
					if (!searchuser(uident)) {
						prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
						pressanykey();
						break;
					}
					if (!seek_in_file
					    (DIR_MC "policemen", uident)) {
						prints
						    ("¸ÃID²»ÊÇ±øÂíÙ¸¾¯Êğ¾¯Ô±¡£");
						pressanykey();
						break;
					}
					if (askyn("È·¶¨Âğ£¿", NA, NA) == YEA) {
						del_from_file(DIR_MC
							      "policemen",
							      uident);
						sprintf(genbuf,
							"%s ½â³ıÄãµÄ±øÂíÙ¸¾¯Êğ¾¯Ô±Ö°Îñ",
							currentuser.userid);
						mail_buf
						    ("¸ĞĞ»ÄãÒ»Ö±ÒÔÀ´µÄ¹¤×÷£¬²¢Ï£ÍûÄã×÷ÎªÊĞÃñ¼ÌĞøÎªÎ¬»¤ÖÎ°²¶ø¾¡ÒåÎñ¡£",
						     uident, genbuf);
						move(14, 4);
						prints("½âÖ°³É¹¦¡£");
						sprintf(genbuf, "%sĞĞÊ¹¾¯Êğ¹ÜÀíÈ¨ÏŞ",currentuser.userid);
						sprintf(buf, "½â³ı%sµÄ¾¯Êğ¾¯Ô±Éí·İ", uident);
						millionairesrec(genbuf, buf, "BMYpolice");
						pressanykey();
					}
					break;
				case '3':
					clear();
					move(1, 0);
					prints("Ä¿Ç°±øÂíÙ¸¾¯Êğ¾¯Ô±Ãûµ¥£º");
					listfilecontent(DIR_MC "policemen");
					pressanykey();
					break;
				case '4':
					clear();
					move(1, 0);
					prints("Ä¿Ç°±øÂíÙ¸¾¯Êğ¼à½û×ï·¸Ãûµ¥£º");
					move(2, 0);
					prints("×ï·¸ID\t¼à½ûÌìÊı");
					listfilecontent(DIR_MC "imprison_list");
					pressanykey();
					break;
				case '5':
					move(12, 4);
					if (askyn
					    ("ÄúÕæµÄÒª´ÇÖ°Âğ£¿", NA, NA) == YEA) {
					/*	del_from_file(MC_BOSS_FILE, "police");
						sprintf(genbuf,
							"%s Ğû²¼´ÇÈ¥±øÂíÙ¸¾¯ÊğÊğ³¤Ö°Îñ",
							currentuser.userid);
						deliverreport(genbuf,
							      "±øÂíÙ¸½ğÈÚÖĞĞÄ¶ÔÆäÒ»Ö±ÒÔÀ´µÄ¹¤×÷±íÊ¾¸ĞĞ»£¬×£ÒÔºóË³Àû£¡");
						move(14, 4);
						prints
						    ("ºÃ°É£¬¼ÈÈ»ÄãÒâÒÑ¾ö£¬¾¯ÊğÒ²Ö»ÓĞÅú×¼¡£");
						quit = 1;
						pressanykey();
					*/
						sprintf(genbuf, "%s Òª´ÇÈ¥±øÂíÙ¸¾¯ÊğÊğ³¤Ö°Îñ",
							currentuser.userid);
						mail_buf(genbuf, "millionaires", genbuf);
						move(14, 4);
						prints("ºÃ°É£¬ÒÑ¾­·¢ĞÅ¸æÖª×Ü¹ÜÁË");
						pressanykey();
					}
					break;
				}
		case '6':
				break;
			}
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}


static int //slowaction
/*¼ì²éÈ¨ÏŞ*/
Allclubtest(char *id)
{  if (slowclubtest("Beggar",id))
        return 1;
    else if (slowclubtest("Rober",id))
         return 1;
        else if (slowclubtest("Police",id))
             return 1;
			  else if (slowclubtest("killer",id))
				   return 1;
              else  return 0;
}

static int  //slowaction
slowclubtest(char *board,char *id)
{
	char buf[256];
	sprintf(buf, "boards/%s/club_users", board);
	return seek_in_file(buf, id);
}

//¹ÉÆ±¿ªÅÌ
static int
stop_buy()
{
	FILE *f_fp;
	char fname[125];
	sprintf(fname,"%s/stopbuy", DIR_MC);
	f_fp=fopen(fname,"r");
	if(f_fp!=NULL)
	{
		fclose(f_fp);
		return 1;
	}
		return 0;
}
/*int
mail_buf_slow(char *userid, char *title, char *content, char *sender)
{
        FILE *fp;
        char buf[256], dir[256];
        struct fileheader header;
        int t;
	int now;
        bzero(&header, sizeof (header));
        fh_setowner(&header, sender, 0);
        sprintf(buf, "mail/%c/%s/", mytoupper(userid[0]), userid);
	if (!file_isdir(buf))
		return -1;
	now = time(NULL);
        t = trycreatefile(buf, "M.%d.A", now, 100);
        if (t < 0)
                return -1;
        header.filetime = t;
        ytht_strsncpy(header.title, title, sizeof (header.title));
        fp = fopen(buf, "w");
        if (fp == 0)
                return -2;
	fprintf(fp, "%s", content);
        fclose(fp);
        setmailfile(dir, userid, ".DIR");
        append_record(dir, &header, sizeof (header));
        return 0;
}
///slowaction to help bm
*/

//½á»é
static int
money_marry()
{
	int n, ch, quit = 0;
	size_t filesize=0;
	void *buffer = NULL;
	struct MC_Marry *marryMem,mm;
	char note[3][STRLEN];
	char buf[STRLEN];
	int i,j,k;
	int flag = 1;
	int freshflag = 1;

	if (!file_exist(MC_MARRY_RECORDS)){
		int fd;
		void *ptr = NULL;
		filesize = sizeof(struct MC_Marry);
		bzero(&mm, filesize);
		ptr = &mm;
		mkdir(DIR_MC_MARRY, 0770);
		if ((fd = open(MC_MARRY_RECORDS, O_CREAT | O_EXCL | O_WRONLY, 0660)) == -1)
			return -1;
		write(fd, ptr, filesize);
		close(fd);
		}
	n = get_num_records(MC_MARRY_RECORDS, sizeof(struct MC_Marry));
	if (n < 0)
		return 0;
/*	if(n<100){
		n=100;		//Ò»´ÎÔØÈëÒ»°ÙÌõ
		truncate(MC_MARRY_RECORDS,100*sizeof(struct MC_Marry));
	}
*/
	filesize = sizeof(struct MC_Marry) * n;
	//¼ÓÔØĞÅÏ¢
	marryMem = loadData(MC_MARRY_RECORDS, buffer, filesize);
	if (marryMem == (void *) -1)
                return -1;
	//´¦Àí¸÷ÖÖ»éÒö×´Ì¬±ä»¯
	marry_refresh(marryMem,n);
	//²é¿´ÊÇ·ñÓĞÇó»éÉêÇë
	for(j=0; j<n; j++){
		if(marryMem[j].enable == 0) continue;
		if(marryMem[j].status != MAR_COURT) continue;
		if(!strcmp(marryMem[j].bride, currentuser.userid)){
			break;
		}
	}
	//±ÜÃâ½ÓÊÜ¶à¸öÈËÇó»é
	for(i=0;i<n;i++){
		if(!strcmp(marryMem[i].bride, currentuser.userid)){
			if(marryMem[i].status == MAR_MARRIED
					|| marryMem[i].status == MAR_MARRYING){
				marryMem[j].status =  MAR_COURT_FAIL;
				marryMem[j].enable = 0;
				sprintf(genbuf, "%s ÒÑ¾­½ÓÊÜÁË±ğÈËµÄÇó»é\n",marryMem[j].bride);
				strcat(genbuf,"\n±ğ»ÒĞÄ£¬ÈıÌõÍÈµÄ¸òó¡²»¶à¼û£¬Á½ÌõÍÈµÄ¹ÃÄï»¹²»ÓĞµÄÊÇ~~");
				sprintf(buf, "¶Ô²»Æğ£¬%s ²»ÄÜ½ÓÊÜÄúµÄÇó»é", marryMem[j].bride);
				mail_buf(genbuf, marryMem[j].bridegroom, buf);
				j=n;
			}
		}
	}

	if(j<n){
		money_show_stat("±øÂíÙ¸½ÌÌÃ");
		move(5, 4);
		flag = 1;
		sprintf(buf, "ĞÒ¸£µÄÈË¶ù£¬ÄúÊÇ·ñ½ÓÊÜ \033[1;33m%s\033[m µÄÇó»é£¿",marryMem[j].bridegroom);
		if (askyn(buf, NA, NA) == NA) {
			move(6, 4);
			prints("ÇĞ£¬ËûËãÄÇ¸ù´Ğ~~");
			flag = 0;
			marryMem[j].enable = 0;
		}else{
			move(6, 4);
			prints("*^^*£¬ÖÕÓÚµÈµ½ÕâÒ»Ìì~~");
			marryMem[j].enable = 1;
			marryMem[j].marry_t = time(NULL) + 24*60*60;		//»éÀñÒ»Ììºó¾ÙĞĞ
			marryMem[j].status = MAR_MARRYING;
			flag = 1;
		}

		move(7, 4);
		prints("ÄúÒª¶ÔËûËµĞ©Ê²Ã´Âğ£¿[¿ÉÒÔĞ´3ĞĞà¸]");
		bzero(note, sizeof (note));
		for (i = 0; i < 3; i++) {
			getdata(8 + i, 0, ": ", note[i], STRLEN - 1, DOECHO, NA);
			if (note[i][0] == '\0')
				break;
		}
		sprintf(genbuf, "%s %sÁËÄúµÄÇó»é\n",marryMem[j].bride, flag?"½ÓÊÜ":"¾Ü¾ø");
		if (i > 0) {
			sprintf(buf, "\033[1;33m%s\033[m%sµÄËµ:\n", marryMem[j].bride, flag?"Ğß´ğ´ğ":"ÀäÀä");
			strcat(genbuf,buf);
			for (k = 0; k < i; k++){
				strcat(genbuf,note[k]);
				strcat(genbuf,"\n");
			}
		}
		if(flag)
			strcat(genbuf,"\n±ğÕ¾×ÅÉµÀÖÀ²£¬¿ìÈ¥±øÂíÙ¸½ÌÌÃ×¼±¸»éÀñ°É!");
		else strcat(genbuf,"\n±ğ»ÒĞÄ£¬ÈıÌõÍÈµÄ¸òó¡²»¶à¼û£¬Á½ÌõÍÈµÄ¹ÃÄï»¹²»ÓĞµÄÊÇ~~");

		sprintf(buf, "[%s]%s %sÁËÄúµÄÇó»é",flag?"¹§Ï²":"Í¨Öª", marryMem[j].bride, flag?"½ÓÊÜ":"¾Ü¾ø");
		mail_buf(genbuf, marryMem[j].bridegroom, buf);
		if (flag){
			if (i > 0) {
			sprintf(genbuf, "\033[1;33m%s\033[mĞß´ğ´ğµÄËµ:\n", marryMem[j].bride);
				for (k = 0; k < i; k++){
					strcat(genbuf,note[k]);
					strcat(genbuf,"\n");
				}
			}
			sprintf(buf,"[ºÅÍâ]%s½ÓÊÜÁË%sµÄÇó»é",marryMem[j].bride,marryMem[j].bridegroom);
			if (note[0][0] == '\0')
				deliverreport(buf,"\n");
			else
	 			deliverreport(buf, genbuf);
		}
		move(13, 4);
		prints("ÎÒÃÇÒÑ¾­·¢ĞÅÍ¨ÖªÁË¶Ô·½");
		pressanykey();
		}


	while (!quit) {
		clear();
		money_show_stat("±øÂíÙ¸½ÌÌÃ");
		if(freshflag){
			show_welcome(MC_MAEEY_SET,4,22);
			freshflag =0;
		}
		//move(6, 4);
		//prints("»¶Ó­Äú×ß½ø»éÒöµÄÎ§³Ç");
		move(t_lines - 1, 0);
		prints("\033[1;44m Ñ¡µ¥ \033[1;46m [1]²Î¼Ó»éÀñ [2]Çó»é [3]×¼±¸»éÀñ [4]Àë»é [5]µÇ¼Ç±í [6]»éÒö¹ÜÀí°ì [Q]Àë¿ª\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			freshflag = 1;
			marry_attend(marryMem, n);
			break;
		case '2':
			freshflag = 1;
			marry_court(marryMem, n);
			break;
		case '3':
			freshflag = 1;
			marry_perpare(marryMem, n);
			break;
		case '4':
			freshflag = 1;
			marry_divorce();
			break;
		case '5':
			freshflag = 1;
			marry_recordlist(marryMem, n);
			break;
		case '6':
			freshflag = 1;
			marry_admin(marryMem, n);
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	saveData(marryMem, filesize);
	return 0;
}

/*add by macintosh@BMY 2006.10*/
static int
marry_admin(struct MC_Marry *marryMem, int n)
{
	int offset, ch, quit = 0;
	int count, count2, count3, no=0;
	char uident[IDLEN + 1], uident2[IDLEN + 1], buf[2048], title[STRLEN], ans[8];
	char jhdate[30], lhdate[30], lhz[2048];
	size_t filesize;
	//struct MC_Marry *marryMem;
	struct MC_Marry *mm;
	void *buffer = NULL;
	time_t local_now_t = time(NULL);

	nomoney_show_stat("±øÂíÙ¸»éÒö¹ÜÀí°ì¹«ÊÒ");
	whoTakeCharge2(10, uident2);
	whoTakeCharge(10, uident);

	if (!seek_in_file(MC_ADMIN_FILE, currentuser.userid)
		&& !seek_in_file(MC_MARRYADMIN_FILE, currentuser.userid)
		&& strcmp(currentuser.userid, uident)) {
		move(6, 4);
		prints
		  ("ÃØÊé%sÀ¹×¡ÁËÄã,ËµµÀ:¡°Ö÷ÈÎÃÇÏÖÔÚÕıÃ¦×Å´òÂé½«£¬Ã»Ê±¼ä½Ó´ı!¡±", uident2);
		pressanykey();
		return 0;
	}

	while (!quit) {
		nomoney_show_stat("±øÂíÙ¸»éÒö¹ÜÀí°ì¹«ÊÒ");
		move(t_lines - 2, 0);
		prints("\033[1;44m Ñ¡ \033[1;46m [1]²éÑ¯»éÒö×´¿ö [2]°ìÀíÀë»é [3]ÉèÖÃ¹ÜÀíÈËÔ± [4]·¢ËÍÀë»éÍ¨ÖªÊé             \n"
			   "\033[1;44m µ¥ \033[1;46m [5]Ç¿ÖÆ½â³ı»éÔ¼ [Q]Àë¿ª                                               ");
		ch = igetkey();
		switch (ch) {
		case '1':
			clear();
			move(6, 4);
			usercomplete("²éË­µÄÇé¿ö£¿", uident);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				move(7, 4);
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressreturn();
				break;
			}
			marry_query_records(uident);
			break;
		case '2':
			clear();
			move(4, 4);
			prints("\033[1;31;5mÊäÈëIDÊ±Çë×¢Òâ´óĞ¡Ğ´\033[m");
			getdata(6, 4, "ÇëÊäÈëÅ®·½ID: ", uident, 13, DOECHO, YEA);
			getdata(7, 4, "ÇëÊäÈëÄĞ·½ID: ", uident2, 13, DOECHO, YEA);
			/*
			move(6, 4);
			usercomplete("ÇëÊäÈëÅ®·½ID: ", uident);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				move(7, 4);
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressreturn();
				break;
			}
			move(7, 4);
			usercomplete("ÇëÊäÈëÄĞ·½ID: ", uident2);
			if (uident2[0] == '\0')
				break;
			if (!searchuser(uident2)) {
				move(8, 4);
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressreturn();
				break;
			}
			*/
			if (askyn("È·¶¨Âğ£¿", NA, NA) == YEA){
				if (!file_exist(MC_MARRY_RECORDS_ALL)){
					clear();
					move(9, 4);
					prints("Ã»ÓĞÈÎºÎ¼ÇÂ¼!");
					pressanykey();
					break;
				}
				n = get_num_records(MC_MARRY_RECORDS_ALL, sizeof(struct MC_Marry));
				if (n <= 0)
					break;
				filesize = sizeof(struct MC_Marry) * n;
				marryMem = loadData(MC_MARRY_RECORDS_ALL, buffer, filesize);
				if (marryMem == (void *) -1)
					break;
				count = 0;
				for(offset = 0; ;offset++){
					if (offset >= n || offset < 0)
						break;
					mm = &marryMem[offset];
					if(mm->enable==0)
						continue;
					if(!mm->bride[0] || !mm->bridegroom[0])
						continue;
					if(strcmp(mm->bride, uident))
						continue;
					if(strcmp(mm->bridegroom, uident2))
						continue;
					if(mm->status==MAR_MARRIED){
						sprintf(buf, "½á»éÊ±¼äÎª%s£¬È·¶¨Âğ£¿",
							get_simple_date_str(&mm->marry_t));
						if (askyn(buf, NA, NA) == YEA){
							mm->status=MAR_DIVORCE;
							mm->divorce_t=time(NULL);
							count++;
							sprintf(jhdate, "%s", get_simple_date_str(&mm->marry_t));
							jhdate[10]=0;
							sprintf(lhdate, "%s", get_simple_date_str(&mm->divorce_t));
							lhdate[10]=0;
							no=offset;
						}
					}
				}
				move(12, 4);
				if (count>0){
					saveData(marryMem, filesize);
					sprintf(title, "%sºÍ%sÀÍÑà·Ö·É", uident, uident2);
					sprintf(buf,"¡¡¡¡ËäÈ»´ó¸»ÎÌ»éÒö¹ÜÀí°ì¹«ÊÒÖ÷ÈÎ%s¶à´Îµ÷½â£¬"
						"µ«ÊÇ%s£¨Å®·½£©ºÍ%s£¨ÄĞ·½£©µÄ°®ÇéÒÑ¾­×ßµ½¾¡Í·£¬"
						"Õ÷Ñ¯Ë«·½Òâ¼ûºó£¬´ó¸»ÎÌ»éÒö¹ÜÀí°ì¹«ÊÒ¾ö¶¨Åú×¼"
						"¶şÈËÀë»é£¬Ô¸¶şÈË½ñºóÉú»îË³Àû¡£\n",
						currentuser.userid, uident, uident2);
					deliverreport(title, buf);
					sprintf(title, "[¹«¸æ]%sºÍ%sÀë»é", uident, uident2);
					sprintf(lhz,
							"[0m               [47m                                                [40m \n"
							"               [47m  [41m[1;32m¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î[47m  [40m \n"
							"               [47m  [41m¡î                                        ¡î[47m  [40m \n"
							"               [47m  [41m¡î               [37mÀë »é Ö¤                 [32m¡î[47m  [40m \n"
							"               [47m  [41m¡î                                        ¡î[47m  [40m \n"
							"               [47m  [41m¡î                    [34m[[37m»é×Ö[34m]µÚ [37m%5.5d [34mºÅ   [32m¡î[47m  [40m \n"
							"               [47m  [41m¡î   [37m³ÖÖ¤ÈË                               [32m¡î[47m  [40m \n"
							"               [47m  [41m¡î   [4;37m%-12.12s[0;1;41m£¨Å®£©[4m%-12.12s[0;1;41m£¨ÄĞ£© [32m¡î[47m  [40m \n"
							"               [47m  [41m¡î   [37m½á»éÈÕÆÚ£º[4m%s[0;41m                 [1;32m¡î[47m  [40m \n"
							"               [47m  [41m¡î      [37mÉêÇëÀë»é£¬¾­Éó²é·ûºÏ±øÂíÙ¸´ó¸»ÎÌ  [32m¡î[47m  [40m \n"
							"               [47m  [41m¡î   [37m¹ØÓÚÀë»éµÄ¹æ¶¨£¬×¼ÓèµÇ¼Ç£¬·¢¸ø´ËÖ¤¡£ [32m¡î[47m  [40m \n"
							"               [47m  [41m¡î          [37m·¢Ö¤»ú¹Ø ´ó¸»ÎÌ»éÒö¹ÜÀí°ì¹«ÊÒ [32m¡î[47m  [40m \n"
							"               [47m  [41m¡î          [37m·¢Ö¤ÈÕÆÚ %s           [32m¡î[47m  [40m \n"
							"               [47m  [41m¡î                                        ¡î[47m  [40m \n"
							"               [47m  [41m¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î¡î[47m  [40m \n"
							"               [47m                                                [40m \n"
							"                                                                [m\n",
							no, uident, uident2, jhdate, lhdate);
					deliverreport(title, lhz);
					sprintf(title,"»éÒö¹ÜÀí°ì¹«ÊÒÖ÷ÈÎ%s°ìÀíÀë»éÒµÎñ",
						currentuser.userid);
					millionairesrec(title, buf, "Marriage");
					sprintf(buf,"´ó¸»ÎÌ»éÒö¹ÜÀí°ì¹«ÊÒÍ¬ÒâÄúÓë%sµÄÀë»éÒªÇó£¬Ô¸Äú½ñºóÉú»îË³Àû¡£\n", uident);
					mail_buf_slow(uident2, "´ó¸»ÎÌ»éÒö¹ÜÀí°ì¹«ÊÒÍ¬ÒâÄúµÄÀë»éÒªÇó", buf,"XJTU-XANET");
					sprintf(buf,"´ó¸»ÎÌ»éÒö¹ÜÀí°ì¹«ÊÒÍ¬ÒâÄúÓë%sµÄÀë»éÒªÇó£¬Ô¸Äú½ñºóÉú»îË³Àû¡£\n", uident2);
					mail_buf_slow(uident, "´ó¸»ÎÌ»éÒö¹ÜÀí°ì¹«ÊÒÍ¬ÒâÄúµÄÀë»éÒªÇó", buf,"XJTU-XANET");
					prints("Íê³É²Ù×÷!");

					//ÔÙ´ÎÈ·ÈÏÊÇ²»ÊÇÒÑ¾­½á»é£¬¾ö¶¨ÊÇ·ñ´ÓÒÑ½á»éÃûµ¥É¾³ı
					n = get_num_records(MC_MARRY_RECORDS_ALL, sizeof(struct MC_Marry));
					if (n <= 0)
						break;
					filesize = sizeof(struct MC_Marry) * n;
					marryMem = loadData(MC_MARRY_RECORDS_ALL, buffer, filesize);
					if (marryMem == (void *) -1)
						break;
					count2 = 0;
					count3 = 0;
					for(offset = 0; ;offset++){
						if (offset >= n || offset < 0)
							break;
						mm = &marryMem[offset];
						if(mm->enable==0)
							continue;
						if(!mm->bride[0] || !mm->bridegroom[0])
							continue;
						if(!strcmp(mm->bride, uident))
							if(mm->status==MAR_MARRIED)
								count2++;
						if(!strcmp(mm->bridegroom, uident))
							if(mm->status==MAR_MARRIED)
								count2++;
						if(!strcmp(mm->bride, uident2))
							if(mm->status==MAR_MARRIED)
								count3++;
						if(!strcmp(mm->bridegroom, uident2))
							if(mm->status==MAR_MARRIED)
								count3++;
					}

					if (count2==0){
						if (seek_in_file(MC_MARRIED_LIST, uident))
							del_from_file(MC_MARRIED_LIST, uident);
					} else {
						if (!seek_in_file(MC_MARRIED_LIST, uident))
							addtofile(MC_MARRIED_LIST, uident);
					}
					if (count3==0){
						if (seek_in_file(MC_MARRIED_LIST, uident2))
							del_from_file(MC_MARRIED_LIST, uident2);
					} else {
						if (!seek_in_file(MC_MARRIED_LIST, uident2))
							addtofile(MC_MARRIED_LIST, uident2);
					}
				} else
					prints("Ã»ÓĞÕÒµ½ÈÎºÎÏà¹Ø¼ÇÂ¼!");
				pressreturn();
			}
			break;

		case '3':
			clear();
			if (!seek_in_file(MC_ADMIN_FILE, currentuser.userid)) {
				move(6, 4);
				prints("×Ü¹Ü²Å¿ÉÒÔ²Ù×÷Ó´");
				pressanykey();
				break;
			}

			while (1) {
				clear();
				prints("Éè¶¨»éÒö¹ÜÀí°ì¹«ÊÒÈËÔ±\n");
				count = listfilecontent(MC_MARRYADMIN_FILE);
				if (count)
					getdata(1, 0, "(A)Ôö¼Ó (D)É¾³ı (E)Àë¿ª [E]: ", ans, 7, DOECHO, YEA);
				else
					getdata(1, 0, "(A)Ôö¼Ó  (E)Àë¿ª[E]: ", ans, 7, DOECHO, YEA);
				if (*ans == 'A' || *ans == 'a') {
					move(1, 0);
					usercomplete("Ôö¼ÓÈËÔ±: ", uident);
					if (*uident != '\0') {
						if (seek_in_file(MC_MARRYADMIN_FILE, uident)) {
							move(2, 0);
							prints("ÊäÈëµÄID ÒÑ¾­´æÔÚ!");
							pressreturn();
							break;
						}
						move(4, 0);
						if (askyn("ÕæµÄÒªÌí¼ÓÃ´?", NA, NA) == YEA){
							addtofile(MC_MARRYADMIN_FILE, uident);
							sprintf(title, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ(»éÒö)", currentuser.userid);
							sprintf(buf, "Ìí¼Ó%sÎª»éÒö¹ÜÀíÈËÔ±", uident);
							millionairesrec(title, buf, "Marriage");
							//deliverreport(titlebuf, repbuf);
							//mail_buf(repbuf, uident, titlebuf);
						}
					}
				} else if ((*ans == 'D' || *ans == 'd') && count) {
					move(1, 0);
					namecomplete("É¾³ıÈËÔ±: ", uident);
					move(1, 0);
					clrtoeol();
					if (uident[0] != '\0') {
						if (!seek_in_file(MC_MARRYADMIN_FILE, uident)) {
							move(2, 0);
							prints("ÊäÈëµÄID ²»´æÔÚ!");
							pressreturn();
							break;
						}
						move(4, 0);
						if (askyn("ÕæµÄÒªÉ¾³ıÃ´?", NA, NA)==YEA){
							del_from_file(MC_MARRYADMIN_FILE, uident);
							sprintf(title, "%sĞĞÊ¹¹ÜÀíÈ¨ÏŞ(»éÒö)", currentuser.userid);
							sprintf(buf, "È¡Ïû%sµÄ»éÒö¹ÜÀíÖ°Îñ", uident);
							millionairesrec(title, buf, "Marriage");
							//deliverreport(titlebuf, repbuf);
							//mail_buf(repbuf, uident, titlebuf);
						}
					}
				}  else
					break;
			}
			clear();
			break;

		case '4':
			clear();
			move(6, 4);
			usercomplete("ÇëÊäÈëÊÕĞÅ·½ID: ", uident);
			if (uident[0] == '\0')
				break;
			if (!searchuser(uident)) {
				move(7, 4);
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressreturn();
				break;
			}
			move(7, 4);
			usercomplete("ÇëÊäÈëÌá³ö·½ID: ", uident2);
			if (uident2[0] == '\0')
				break;
			if (!searchuser(uident2)) {
				move(8, 4);
				prints("´íÎóµÄÊ¹ÓÃÕß´úºÅ...");
				pressreturn();
				break;
			}

			if (askyn("È·¶¨Âğ£¿", NA, NA) == YEA){
				sprintf(lhdate, "%s", get_simple_date_str(&local_now_t));
				lhdate[10]=0;
				sprintf(buf,"×ğ¾´µÄ%s£º\n"
					"¡¡¡¡±¾»éÒö¹ÜÀíÖĞĞÄÊÜÀíÔ­¸æ%s"
					"ËßÄãÀë»é¾À·×Ò»°¸£¬ÏÖÒÀ´ó¸»ÎÌ"
					"¹ØÓÚÀë»éµÄ¹æ¶¨ÏòÄãËÍ´ï¡£×Ô±¾"
					"Í¨Öª·¢³öÖ®ÈÕÆğ¾­¹ı6ÈÕ¼´ÊÓÎªËÍ´ï¡£"
					"ÇëÄãÈÏÕæÔÄ¶Á±øÂíÙ¸´ó¸»ÎÌ¹ØÓÚÀë»éµÄ"
					"Ïà¹Ø¹æ¶¨£¬²¢ÔÚ6ÈÕÄÚ×ö³ö´ğ¸´£¬¹æ¶¨ÍâµÄ"
					"²Æ²ú·Ö¸î¼°ËğÊ§Åâ³¥ÇëÓëÔ­¸æÁªÏµ£¬"
					"¶¨ÓÚµÚ7ÈÕ£¨Óö½Ú¼ÙÈÕË³ÑÓ£©ÉóºË´ËËßËÏ"
					"ÇëÇó£¬ÓâÆÚ£¨ÒÔ30ÈÕÎªÏŞ£©½«ÒÀÕÕÓĞ¹Ø¹æ¶¨"
					"ÅĞ¾ö¡£\n\n%80.80s\n%80.80s\n",
					uident, uident2, " ±øÂíÙ¸´ó¸»ÎÌ»éÒö¹ÜÀíÖĞĞÄ", lhdate);
				mail_buf_slow(uident, "Àë»éÍ¨ÖªÊé", buf, "Marriage");

				sprintf(title, "%sËß%sÀë»é¾À·×Ò»°¸¿ªÍ¥ÉóÀí", uident2, uident);
				sprintf(buf,
					"±øÂíÙ¸»éÒö°ì¹«ÊÒÊÜÀí%sÀë»éÇëÇó£¬"
					"ÒÑÏò%s·¢³öÁËÀë»éÍ¨ÖªÊé¡£", uident2, uident);
				deliverreport(title, buf);

				sprintf(title,"»éÒö¹ÜÀí°ì¹«ÊÒÖ÷ÈÎ%s·¢ËÍÀë»éÍ¨ÖªÊé",
					currentuser.userid);
				millionairesrec(title, buf, "Marriage");
				prints("Íê³É²Ù×÷!");
				pressanykey();
			}
			break;

		case '5':
			clear();
			showAt(2, 4, "\033[1;31m´Ë¹¦ÄÜÉ÷ÓÃ! \033[m", 0);
			showAt(4, 4, "\033[1;32mÊäÈëIDÊ±Çë×¢Òâ´óĞ¡Ğ´\033[m", 0);
			getdata(6, 4, "ÇëÊäÈëÅ®·½ID: ", uident, 13, DOECHO, YEA);
			getdata(7, 4, "ÇëÊäÈëÄĞ·½ID: ", uident2, 13, DOECHO, YEA);
			if (askyn("È·¶¨Âğ£¿", NA, NA) == YEA){
				/*if (!file_exist(MC_MARRY_RECORDS_ALL)){
					clear();
					move(9, 4);
					prints("Ã»ÓĞÈÎºÎ¼ÇÂ¼!");
					pressanykey();
					break;
				}
				n = get_num_records(MC_MARRY_RECORDS_ALL, sizeof(struct MC_Marry));
				if (n <= 0)
					break;
				filesize = sizeof(struct MC_Marry) * n;
				marryMem = loadData(MC_MARRY_RECORDS_ALL, buffer, filesize);
				if (marryMem == (void *) -1)
					break;
				*/
				count = 0;
				for(offset = 0; ;offset++){
					if (offset >= n || offset < 0)
						break;
					mm = &marryMem[offset];
					if(mm->enable==0)
						continue;
					if(!mm->bride[0] || !mm->bridegroom[0])
						continue;
					if(strcmp(mm->bride, uident))
						continue;
					if(strcmp(mm->bridegroom, uident2))
						continue;
					sprintf(buf, "¶©»éÊ±¼äÎª%s£¬È·¶¨Âğ£¿",
						get_simple_date_str(&mm->court_t));
					if (askyn(buf, NA, NA) == YEA){
						mm->enable=0;
						mm->status = MAR_COURT_FAIL;
						count++;
					}

				}
				move(12, 4);
				if (count>0){
					saveData(marryMem, filesize);
					sprintf(title,"»éÒö¹ÜÀí°ì¹«ÊÒÖ÷ÈÎ%sÇ¿ÖÆ½â³ı»éÔ¼",
						currentuser.userid);
					sprintf(buf,"Ç¿ÖÆ½â³ı%sÓë%sµÄ»éÔ¼",
						uident, uident2);
					millionairesrec(title, buf, "Marriage");
					prints("Íê³É²Ù×÷!");

					//ÔÙ´ÎÈ·ÈÏÊÇ²»ÊÇÒÑ¾­½á»é£¬¾ö¶¨ÊÇ·ñ´ÓÒÑ½á»éÃûµ¥É¾³ı
					n = get_num_records(MC_MARRY_RECORDS_ALL, sizeof(struct MC_Marry));
					if (n <= 0)
						break;
					filesize = sizeof(struct MC_Marry) * n;
					marryMem = loadData(MC_MARRY_RECORDS_ALL, buffer, filesize);
					if (marryMem == (void *) -1)
						break;
					count2 = 0;
					count3 = 0;
					for(offset = 0; ;offset++){
						if (offset >= n || offset < 0)
							break;
						mm = &marryMem[offset];
						if(mm->enable==0)
							continue;
						if(!mm->bride[0] || !mm->bridegroom[0])
							continue;
						if(!strcmp(mm->bride, uident))
							if(mm->status==MAR_MARRIED)
								count2++;
						if(!strcmp(mm->bridegroom, uident))
							if(mm->status==MAR_MARRIED)
								count2++;
						if(!strcmp(mm->bride, uident2))
							if(mm->status==MAR_MARRIED)
								count3++;
						if(!strcmp(mm->bridegroom, uident2))
							if(mm->status==MAR_MARRIED)
								count3++;
					}

					if (count2==0){
						if (seek_in_file(MC_MARRIED_LIST, uident))
							del_from_file(MC_MARRIED_LIST, uident);
					} else {
						if (!seek_in_file(MC_MARRIED_LIST, uident))
							addtofile(MC_MARRIED_LIST, uident);
					}
					if (count3==0){
						if (seek_in_file(MC_MARRIED_LIST, uident2))
							del_from_file(MC_MARRIED_LIST, uident2);
					} else {
						if (!seek_in_file(MC_MARRIED_LIST, uident2))
							addtofile(MC_MARRIED_LIST, uident2);
					}
				} else
					prints("Ã»ÓĞÕÒµ½ÈÎºÎÏà¹Ø¼ÇÂ¼!");
				pressreturn();
			}
			break;

		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

static int
marry_recordlist(struct MC_Marry *marryMem, int n)
{
	int ch, quit = 0;
	while (!quit) {
		nomoney_show_stat("±øÂíÙ¸½ÌÌÃµµ°¸¹İ");
		move(8, 16);
		prints(" Çó»éµÄ£¬ÒÑ»éµÄ£¬Àë»éµÄ...È«ÔÚÕâ¼Ç×ÅÄØ£¬¿´°É");
		move(t_lines - 1, 0);
		prints("\033[1;44m Ñ¡µ¥ \033[1;46m [1]»éÊÂµÇ¼Ç±í [2]×´¿ö¼ÇÂ¼±í [3]¸öÈË²éÑ¯ [Q]Àë¿ª\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			marry_active_records(marryMem, n);
			break;
		case '2':
			marry_all_records();
			break;
		case '3':
			marry_query_records(currentuser.userid);
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

//²éÑ¯»éÒö×´¿ö
/*add by macintosh@BMY 2006.10*/
static int
marry_query_records(char *id)
{
	int i,j;
	char buf[STRLEN];
	int offset;
	int pages;
	int count;
	struct MC_Marry *mm;
	char timestr[STRLEN];
	struct MC_Marry *marryMem;
	void *buffer = NULL;
	size_t filesize;
	time_t local_now_t;
	int n;

	if (!file_exist(MC_MARRY_RECORDS_ALL)){
		clear();
		move(6, 4);
		prints("Ã»ÓĞÈÎºÎ¼ÇÂ¼!");
		pressanykey();
		return 0;
	}
	n = get_num_records(MC_MARRY_RECORDS_ALL, sizeof(struct MC_Marry));
	if (n <= 0)
		return 0;
	filesize = sizeof(struct MC_Marry) * n;
	//¼ÓÔØĞÅÏ¢
	marryMem = loadData(MC_MARRY_RECORDS_ALL, buffer, filesize);
	if (marryMem == (void *) -1)
		return -1;
	money_show_stat("±øÂíÙ¸½ÌÌÃµµ°¸¹İ");
	move(5, 0);
	prints("                       \033[1;31m¸öÈË»éÒöÇé¿ö²éÑ¯½á¹û (%s)\033[m         ", id);
	move(6, 0);
	sprintf(buf,"%-6.6s %-20.20s %-10.10s %-10.10s %-16.16s %-4.4s %-6.6s","±àºÅ","Ö÷Ìâ","ĞÂÄï","ĞÂÀÉ","Çó/½á/»éÀñÊ±¼ä","µ½·Ã","×´Ì¬");
	prints(buf);
	move(7, 0);
	prints("--------------------------------------------------------------------------------------");
	pages = n / 10 + 1;
	for(i = 0; ;i++) {	//iÓÃÓÚ¿ØÖÆÒ³Êı
		local_now_t = time(NULL);
		for(j=0; j<10; j++){
				move(8 + j , 0);
				clrtoeol();
		}
		count = 0;
		for(j = 0; count < 10; j++) {	//Ã¿ÆÁÏÔÊ¾×î¶à10
			offset = i * 10 + j;
			move(8 + count , 0);
			if (offset >= n || offset < 0) {
				//clrtoeol();
				//continue;
				break;
			}
			mm = &marryMem[offset];
			if(mm->enable==0) continue;
			if(!mm->bride[0] || !mm->bridegroom[0]) continue;
			if(strcmp(mm->bride, id)
				&& strcmp(mm->bridegroom, id))
				continue;
			count++;
			switch(mm->status){
				case MAR_COURT:
				case MAR_COURT_FAIL:
					strcpy(timestr,get_simple_date_str(&mm->court_t));
					break;
				case MAR_MARRIED:
				case MAR_MARRYING:
					strcpy(timestr,get_simple_date_str(&mm->marry_t));
					break;
				case MAR_DIVORCE:
					strcpy(timestr,get_simple_date_str(&mm->divorce_t));
					break;
				default:
					strcpy(timestr,get_simple_date_str(&mm->marry_t));
			}
			sprintf(buf, "[%4d] %-20.20s %-10.10s %-10.10s %-16.16s %4d \033[1;%dm%-6.6s\033[m",
				offset,mm->subject,mm->bride,mm->bridegroom,timestr,mm->visitcount,
				(mm->status ==MAR_MARRYING)?32:37,marry_status[mm->status]);
			prints("%s", buf);
			}
		if ((offset >= n ) && (count <= 0)){
			move(9, 0);
			prints("Ã»ÓĞÕÒµ½ÈÎºÎÏà¹Ø¼ÇÂ¼!");
			pressreturn();
			return 0;
		} else {
			getdata(19, 4, "[B]Ç°Ò³ [C]ÏÂÒ³ [Q]ÍË³ö: [C]", buf, 2, DOECHO, YEA);
			if (toupper(buf[0]) == 'Q')
				return 0;
			if (toupper(buf[0]) == 'B')
				i = (i == 0) ? (i-1) : (i-2);
			else
				if (offset < n)
					i = (i == pages -1) ? (i-1) : i;
				else i--;
		}
	}
	return 1;
}
//×´¿ö¼ÇÂ¼±í
//°üÀ¨½á»é£¬Àë»é£¬Çó»éÊ§°Ü
static int
marry_all_records()
{
	int i,j;
	char buf[STRLEN];
	int offset;
	int pages;
	int count;
	struct MC_Marry *mm;
	char timestr[STRLEN];
	struct MC_Marry *marryMem;
	void *buffer = NULL;
	size_t filesize;
	time_t local_now_t;
	int n;

	if (!file_exist(MC_MARRY_RECORDS_ALL)){
		clear();
		move(6, 4);
		prints("ß×£¿ÔõÃ´Ã»ÓĞ¼ÇÂ¼£¬ÄÑµÃ»¹´ÓÎ´ÓĞ¹Ø»éÊÂ»î¶¯£¿£¡");
		pressanykey();
		return 0;
	}
	n = get_num_records(MC_MARRY_RECORDS_ALL, sizeof(struct MC_Marry));
	if (n <= 0)
		return 0;
	filesize = sizeof(struct MC_Marry) * n;
	//¼ÓÔØĞÅÏ¢
	marryMem = loadData(MC_MARRY_RECORDS_ALL, buffer, filesize);
	if (marryMem == (void *) -1)
		return -1;
	money_show_stat("±øÂíÙ¸½ÌÌÃµµ°¸¹İ");
	move(5, 4);
	prints("                             \033[1;31m½ÌÌÃ»éÊÂ×´¿ö¼ÇÂ¼±í\033[m         ");
	move(6, 0);
	sprintf(buf,"%-6.6s %-20.20s %-10.10s %-10.10s %-16.16s %-4.4s %-6.6s","±àºÅ","Ö÷Ìâ","ĞÂÄï","ĞÂÀÉ","Çó/½á/»éÀñÊ±¼ä","µ½·Ã","×´Ì¬");
	prints(buf);
	move(7, 0);
	prints("--------------------------------------------------------------------------------------");
	pages = n / 10 + 1;
	for(i = 0; ;i++) {	//iÓÃÓÚ¿ØÖÆÒ³Êı
		local_now_t = time(NULL);
		for(j=0; j<10; j++){
				move(8 + j , 0);
				clrtoeol();
		}
		count = 0;
		for(j = 0; count < 10; j++) {	//Ã¿ÆÁÏÔÊ¾×î¶à10
			offset = i * 10 + j;
			move(8 + count , 0);
			if (offset >= n || offset < 0) {
				//clrtoeol();
				//continue;
				break;
			}
			mm = &marryMem[offset];
			if(mm->enable==0) continue;
			if(!mm->bride[0] || !mm->bridegroom[0]) continue;
			count++;
			switch(mm->status){
				case MAR_COURT:
				case MAR_COURT_FAIL:
					strcpy(timestr,get_simple_date_str(&mm->court_t));
					break;
				case MAR_MARRIED:
				case MAR_MARRYING:
					strcpy(timestr,get_simple_date_str(&mm->marry_t));
					break;
				case MAR_DIVORCE:
					strcpy(timestr,get_simple_date_str(&mm->divorce_t));
					break;
				default:
					strcpy(timestr,get_simple_date_str(&mm->marry_t));
			}
			sprintf(buf, "[%4d] %-20.20s %-10.10s %-10.10s %-16.16s %4d \033[1;%dm%-6.6s\033[m",
				offset,mm->subject,mm->bride,mm->bridegroom,timestr,mm->visitcount,
				(mm->status ==MAR_MARRYING)?32:37,marry_status[mm->status]);
			prints("%s", buf);
			//offset++;
		}
		getdata(19, 4, "[B]Ç°Ò³ [C]ÏÂÒ³ [Q]ÍË³ö: [C]", buf, 2, DOECHO, YEA);
		if (toupper(buf[0]) == 'Q')
			return 0;
		if (toupper(buf[0]) == 'B')
			i = (i == 0) ? (i-1) : (i-2);
		else
			if (offset < n)
				i = (i == pages -1) ? (i-1) : i;
			else i--;
	}
	return 1;
}

//Çó»é½á»éµÇ¼Ç±í
//»éÊÂµÇ¼Ç±í
static int
marry_active_records(struct MC_Marry *marryMem, int n)
{
	int i,j;
	char buf[STRLEN];
	int offset=0;
	int pages;
	int count;
	struct MC_Marry *mm;
	char timestr[STRLEN];
	time_t local_now_t;

	money_show_stat("±øÂíÙ¸½ÌÌÃµµ°¸¹İ");
	move(5, 4);
	prints("                             \033[1;31m½ÌÌÃ»éÊÂµÇ¼Ç±í\033[m         ");
	move(6, 0);
	sprintf(buf,"%-6.6s %-20.20s %-10.10s %-10.10s %-16.16s %-4.4s %-6.6s","±àºÅ","Ö÷Ìâ","ĞÂÄï","ĞÂÀÉ","Çó»é/»éÀñÊ±¼ä","µ½·Ã","×´Ì¬");
	prints(buf);
	move(7, 0);
	prints("--------------------------------------------------------------------------------------");
	pages = n / 10 + 1;
	for(i = 0; ;i++) {	//iÓÃÓÚ¿ØÖÆÒ³Êı
		local_now_t = time(NULL);
		count = 0;
		for(j=0;j<10;j++) {
				move(8 + j , 0);
				clrtoeol();
		}
		for(j = 0; count < 10; j++) {	//Ã¿ÆÁÏÔÊ¾×î¶à10
			offset = i * 10 + j;
			move(8 + count , 0);
			if (offset >= n || offset < 0) {
				//clrtoeol();
				break;
				//continue;
			}
			mm = &marryMem[offset];
			if(mm->enable==0) continue;
			if(!mm->bride[0] || !mm->bridegroom[0]) continue;
			count++;
			switch(mm->status){
				case MAR_COURT:
				case MAR_COURT_FAIL:
					strcpy(timestr,get_simple_date_str(&mm->court_t));
					break;
				case MAR_MARRIED:
				case MAR_MARRYING:
					strcpy(timestr,get_simple_date_str(&mm->marry_t));
					break;
				case MAR_DIVORCE:
					strcpy(timestr,get_simple_date_str(&mm->divorce_t));
					break;
				default:
					strcpy(timestr,get_simple_date_str(&mm->marry_t));
			}
			sprintf(buf, "[%4d] %-20.20s %-10.10s %-10.10s %-16.16s %4d \033[1;%dm%-6.6s\033[m",
				offset,mm->subject,mm->bride,mm->bridegroom,timestr,mm->visitcount,
				(mm->status ==MAR_MARRYING)?32:37,
				(mm->status ==MAR_MARRYING)?((mm->marry_t > time(NULL))?"³ï±¸ÖĞ":"»éÀñÖĞ"):(marry_status[mm->status]));
			prints("%s", buf);
			//offset++;
		}
		getdata(19, 4, "[B]Ç°Ò³ [C]ÏÂÒ³ [Q]ÍË³ö: [C]", buf, 2, DOECHO, YEA);
		if (toupper(buf[0]) == 'Q')
			return 0;
		if (toupper(buf[0]) == 'B')
			i = (i == 0) ? (i-1) : (i-2);
		else
			if (offset < n)
				i = (i == pages -1) ? (i-1) : i;
			else i--;
	}
	return 1;
}

//±éÀú½á»é±í£¬´¦Àí¸÷ÖÖÇé¿ö±àºÅ
static int
marry_refresh(struct MC_Marry *marryMem, int n)
{
	int i;
	char buf[400];
	char filetmp[STRLEN];
	char invpath[STRLEN];
	char setpath[STRLEN];
	char visitpath[STRLEN];
	FILE *fp, *fp2;
	struct MC_Marry *mm;
	time_t local_now_t= time(NULL);

	for(i=0;i<n;i++){
		if(marryMem[i].status==MAR_COURT && (marryMem[i].enable==0||(local_now_t-marryMem[i].court_t)>7*24*60*60)){
			//Çó»éÊ§°Ü
			mm = &marryMem[i];
			mm->status = MAR_COURT_FAIL;
			mm->enable = 0;
			append_record(MC_MARRY_RECORDS_ALL, mm, sizeof(struct MC_Marry));	//×ªÈë¼ÇÂ¼
		}
		/*else if(marryMem[i].status==MAR_MARRIED){
			//ÒÑ½á»é£¬×ªÈë¼ÇÂ¼±í£¬ÕâÖÖÇé¿öÊÇÎªÁË´Ë´ÎĞŞ¸Ä£¬Ò»°ã²»»á³öÏÖ
			mm = &marryMem[i];
			mm->status = MAR_MARRIED;
			append_record(MC_MARRY_RECORDS_ALL, mm, sizeof(struct MC_Marry));	//×ªÈë¼ÇÂ¼
			mm->enable = 0;	//ÔÚÕâ±ßactive±íÖĞ×÷·Ï
			if (!seek_in_file(MC_MARRIED_LIST, mm->bride))
				addtofile(MC_MARRIED_LIST, mm->bride);
			if (!seek_in_file(MC_MARRIED_LIST, mm->bridegroom))
				addtofile(MC_MARRIED_LIST, mm->bridegroom);
		}
		*/
		else if(marryMem[i].status==MAR_MARRYING
					&& marryMem[i].unused[0]!='d'
					&& !(marryMem[i].marry_t > local_now_t)){
			mm = &marryMem[i];
			mm->unused[0]='d';
			sprintf(filetmp, MY_BBS_HOME "/bbstmpfs/tmp/%s.%d",
				currentuser.userid, getpid());
			fp = fopen(filetmp,"w");
			if(!fp) continue;
			fprintf(fp,"     \033[1;31m%s\033[mºÍ\033[1;32m%s\033[mµÄ»éÀñÕıÊ½¿ªÊ¼£¬»¶Ó­´ó¼Ò¹âÁÙ\n\n"
					"     ÈÃÎÒÃÇ¹²Í¬×£¸£ËûÃÇ°É£¡\n\n",mm->bride,mm->bridegroom);
			fclose(fp);
			sprintf(buf,"[¹«¸æ]%sºÍ%sµÄ»éÀñÕıÊ½¿ªÊ¼£¡",mm->bride,mm->bridegroom);
			postfile(filetmp, MC_BOARD, buf , 1);
		}else if(marryMem[i].status==MAR_MARRYING && local_now_t-marryMem[i].marry_t >4*60*60){
			//»éÀñ4Ğ¡Ê±ºó½áÊø
			mm = &marryMem[i];
			mm->status = MAR_MARRIED;
			append_record(MC_MARRY_RECORDS_ALL, mm, sizeof(struct MC_Marry));	//×ªÈë¼ÇÂ¼
			mm->enable = 0;	//ÔÚÕâ±ßactive±íÖĞ×÷·Ï
			if (!seek_in_file(MC_MARRIED_LIST, mm->bride))
				addtofile(MC_MARRIED_LIST, mm->bride);
			if (!seek_in_file(MC_MARRIED_LIST, mm->bridegroom))
				addtofile(MC_MARRIED_LIST, mm->bridegroom);
			sprintf(invpath,"%s/M.%d.A",DIR_MC_MARRY,mm->invitationfile);
			sprintf(setpath,"%s/M.%d.A",DIR_MC_MARRY,mm->setfile);
			sprintf(visitpath,"%s/M.%d.A",DIR_MC_MARRY, mm->visitfile);
			sprintf(filetmp, MY_BBS_HOME "/bbstmpfs/tmp/%s.%d",
				currentuser.userid, getpid());
			fp = fopen(filetmp,"w");
			if(!fp) continue;
			fprintf(fp,"     \033[1;31m%s\033[mºÍ\033[1;32m%s\033[mµÄ»éÀñµ½´Ë½áÊø£¬¸ĞĞ»´ó¼ÒµÄ¹âÁÙ£¬"
					"ÈÃÎÒÃÇ¹²Í¬×£¸£ËûÃÇĞÒ¸£ÌğÃÛµÄ»éºóÉú»î¡£\n\n",mm->bride,mm->bridegroom);
			fprintf(fp,"    \033[1;36mÒÔÏÂÊÇÕâ´Î»éÀñµÄÇé¿ö¼ÇÂ¼ºÍÍ³¼Æ\033[m\n\n");
			fprintf(fp,"»éÀñÊ±¼ä: %s\n",get_date_str(&mm->marry_t));
			fprintf(fp,"ËùÊÕÀñ½ğ: \033[1;31m%d\033[m ±øÂíÙ¸±Ò\n",mm->giftmoney);
			fprintf(fp,"µ½·ÃÈË´Î: \033[1;31m%d\033[m\n",mm->visitcount);
			fp2= fopen(visitpath,"r");
			if(fp2){
				while(!feof(fp2)){
					if(fgets(buf,sizeof(buf),fp2) == NULL) break;
					fprintf(fp,"%s",buf);
				}
				fclose(fp2);
			}
			fprintf(fp,"\n\033[1mÇë¼í: \033[m\n");
			fp2= fopen(invpath,"r");
			if(fp2){
				while(!feof(fp2)){
					if(fgets(buf,sizeof(buf),fp2) == NULL) break;
					fprintf(fp,"%s",buf);
				}
				fclose(fp2);
			}
			fprintf(fp,"\n\n\033[1m»éÀñ²¼¾°: \033[m\n");
			fp2= fopen(setpath,"r");
			if(fp2){
				while(!feof(fp2)){
					if(fgets(buf,sizeof(buf),fp2) == NULL) break;
					fprintf(fp,"%s",buf);
				}
				fclose(fp2);
			}
			fclose(fp);
			sprintf(buf,"[¹§Ï²]%sºÍ%s´ó»éÒÑ³É",mm->bride,mm->bridegroom);
			postfile(filetmp, MC_BOARD, buf , 1);
		}
	}
	return 1;
}

static int
marry_givemoney(struct MC_Marry *mm)
{
	char uident[IDLEN + 1];
//	void *buffer = NULL;
	int i;
	char note[3][STRLEN];
	char buf[STRLEN];
	time_t local_now_t = time(NULL);
	int num;

	move(4,4);
	if(mm->marry_t > local_now_t){
			prints("»éÀñ»¹Î´¿ªÊ¼,ÇëÉÔºóÔÙÀ´");
			pressanykey();
			return 0;
	}
	else prints("»éÀñ½øĞĞÖĞ£¬ËÍÀñµÄºÃÊ±»ú");

	if(!strcmp(mm->bride, currentuser.userid) || !strcmp(mm->bridegroom, currentuser.userid)){
		move(7 ,4);
		prints("¹ş¹ş£¬¸ø×Ô¼ÒÈËËÍÇ®¾Í²»ÓÃÍ¨¹ıÒøĞĞÁË°É...");
		pressanykey();
		return 0;
	}

	move(5,4);
	if(local_now_t%2==1){	//ĞÂÀÉĞÂÄï¸÷Ò»°ëµÄ»ú»áÊÜÀñ
		strncpy(uident,mm->bride,IDLEN);
		prints("Àñ½ğ½«ËÍµ½ĞÂÄï\033[1;31m%s\033[mµÄÑü°ü",uident);
	}else{
		strncpy(uident,mm->bridegroom,IDLEN);
		prints("Àñ½ğ½«ËÍµ½ĞÂÀÉ\033[1;32m%s\033[mµÄÑü°ü",uident);
	}

	getdata(6, 4, "×ªÕÊ¶àÉÙ±øÂíÙ¸±Ò£¿[100000]", buf, 10, DOECHO, YEA);
	num = atoi(buf);
	if (buf[0]=='\0')
		num=100000;
	if (num<100000) {
		move(7, 4);
		prints("ÈË¼ÒĞÂ»é´óÏ²ÄØ£¬ÕâÃ´µãÇ®ÄãÒ²ºÃÒâË¼ÄÃ³öÊÖ£¬Ğ¡Æø£¬ºßºß:(");
		pressanykey();
		return 0;
		}
	if (num>MAX_MONEY_NUM)
		num=MAX_MONEY_NUM;
	move(7, 4);
	snprintf(buf, STRLEN - 1, "È·¶¨×ªÕÊ %d ±øÂíÙ¸±ÒÂğ£¿", num);
	if (askyn(buf, NA, NA) == NA)
       	return 0;
	if (loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM) < num) {
		move(8, 4);
		prints("¸øÄãÃÇËÍÀñÀ´À²...ÌÍÁË°ëÌì¿Ú´ü...°¡£¿£¡¾ÓÈ»Ã»´øÇ®£¿");
		pressanykey();
		return 0;
	}

	move(7, 4);
	prints("ÓĞ»°ÒªËµÂğ£¿[¿ÉÒÔĞ´3ĞĞà¸]");
	bzero(note, sizeof (note));
	for (i = 0; i < 3; i++) {
		getdata(8 + i, 0, ": ", note[i], STRLEN - 1, DOECHO, NA);
		if (note[i][0] == '\0')
			break;
	}

	saveValue(uident, MONEY_NAME, num, MAX_MONEY_NUM);
	saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
	mm->giftmoney += num;
	sleep(1);

	sprintf(genbuf, "\033[1;32m%s\033[m¸øÄúËÍÀñ \033[1;31m%d\033[m ±øÂíÙ¸±Ò ½á»éÀñ½ğ¡£\n\n",currentuser.userid,num);
	if (i > 0) {
		int j;
		sprintf(buf, "¸½ÑÔ:\n");
		strcat(genbuf,buf);
		for (j = 0; j < i; j++){
			strcat(genbuf,note[j]);
			strcat(genbuf,"\n");
		}
	}

	sprintf(buf, "[¹§ºØĞÂ»é]%s¸øÄúËÍºØÀñÀ´À²", currentuser.userid);
	mail_buf(genbuf,uident, buf);
	sprintf(buf, "[¹§ºØĞÂ»é]%s¹§ºØ%sºÍ%sĞÂ»é´óÏ²", currentuser.userid, mm->bride, mm->bridegroom);
	sprintf(genbuf, "ËÍ\033[1;31mºì°ü\033[mÒ»¸ö\n\n¹§×£ĞÂÀÉĞÂÄï½á»é´óÏ²£¬°ÙÄêºÃºÏ£¬ÔçÉú¹ó×Ó:)");
	deliverreport(buf, genbuf);

	sprintf(buf, "%s²Î¼Ó%sºÍ%sµÄ»éÀñ(ËÍºì°ü)", currentuser.userid, mm->bride, mm->bridegroom);
	sprintf(genbuf, "%s¸ø%sËÍºì°ü (%d±øÂíÙ¸±Ò)",  currentuser.userid, uident, num);
	millionairesrec(buf, genbuf, "²Î¼Ó»éÀñ");

	move(14 ,4);
	prints("Àñ½ğÒÑËÍ´ï¡£");
	pressanykey();
	return 0;
}

//¾«¼òµÄÈÕÆÚ±í
char *get_simple_date_str(time_t *tt)
{
    struct tm *tm;
	static char timestr[200];
	if(tt==0) return "------";
	tm = localtime(tt);
	sprintf(timestr,"%02d/%02d/%02d %02d:%02d",
		tm->tm_year+1900, tm->tm_mon+1,tm->tm_mday, tm->tm_hour, tm->tm_min);
	//prints(timestr);
	return timestr;
}

//²Î¼Ó»éÀñ
static int
marry_attend(struct MC_Marry *marryMem, int n)
{
	int ch, quit = 0;
	int i,j;
	char buf[STRLEN];
	int offset;
	int pages;
	int count;
	struct MC_Marry *mm;
	int index;
	time_t local_now_t;
	int freshflag=1;
	char uident[IDLEN + 1];
	char visitfile[STRLEN];
	char filepath[STRLEN];
	time_t t;

	money_show_stat("±øÂíÙ¸½ÌÌÃ");
	move(5, 4);
	prints("                             \033[1;31m½ÌÌÃ»éÀñµÇ¼Ç±í\033[m         ");
	move(6, 0);
	sprintf(buf,"%-6.6s %-20.20s %-10.10s %-10.10s %-16.16s %-4.4s %-6.6s","±àºÅ","Ö÷Ìâ","ĞÂÄï","ĞÂÀÉ","»éÀñÊ±¼ä","µ½·Ã","×´Ì¬");
	prints(buf);
	move(7, 0);
	prints("--------------------------------------------------------------------------------------");
	pages = n / 10 + 1;
	for(i = 0; ;i++) {	//iÓÃÓÚ¿ØÖÆÒ³Êı
		local_now_t = time(NULL);
		for(j=0;j<10;j++) {
				move(8 + j , 0);
				clrtoeol();
		}
		count = 0;
		for(j = 0; count < 10; j++) {	//Ã¿ÆÁÏÔÊ¾×î¶à10Ö§¹ÉÆ±
			offset = i * 10 + j;
			move(8 + count , 0);
			if (offset >= n || offset < 0) {
				//clrtoeol();
				break;
			}
			mm = &marryMem[offset];
			if(mm->status!=MAR_MARRYING) continue;
			if(mm->enable==0) continue;
			if(!mm->bride[0] || !mm->bridegroom[0]) continue;
			count++;
			sprintf(buf, "[%4d] %-20.20s %-10.10s %-10.10s %-16.16s %4d \033[1;%dm%-6.6s\033[m",
				offset,mm->subject,mm->bride,mm->bridegroom,get_simple_date_str(&mm->marry_t),mm->visitcount,
				(mm->marry_t > local_now_t)?37:32,(mm->marry_t > local_now_t)?"³ï±¸ÖĞ":"½øĞĞÖĞ");
			prints("%s", buf);
			//offset++;
		}
		getdata(19, 4, "[B]Ç°Ò³ [C]ÏÂÒ³ [S]Ñ¡Ôñ [Q]ÍË³ö: [C]", buf, 2, DOECHO, YEA);
		if (toupper(buf[0]) == 'Q')
			return 0;
		if (toupper(buf[0]) == 'S')
			break;
		if (toupper(buf[0]) == 'B')
			i = (i == 0) ? (i-1) : (i-2);
		else
			i = (i == pages -1) ? (i-1) : i;
	}

	while(1) {
		getdata(t_lines-5, 4, "ÇëÑ¡ÔñÄúÒª²Î¼ÓµÄ»éÀñ±àºÅ[ENTER·ÅÆú]:", buf, 3, DOECHO, YEA);
		if (buf[0] == '\0')
			return 0;
		index = atoi(buf);
		if (index >= 0 && index < n && marryMem[index].status == MAR_MARRYING)
			break;
	}
	mm = &marryMem[index];
	mm->visitcount++;	//µ½·Ã¼ÇÂ¼
	local_now_t = time(NULL);
	strncpy(visitfile,DIR_MC_MARRY,STRLEN-1);
	if(mm->visitfile==0){
		t = trycreatefile(visitfile, "M.%d.A", local_now_t, 100);
		if (t < 0)
			return -1;
		mm->visitfile = t;
	}else sprintf(visitfile,"%s/M.%d.A",DIR_MC_MARRY, mm->visitfile);
	if(!seek_in_file(visitfile, currentuser.userid))
		addtofile(visitfile, currentuser.userid);

	while (!quit) {
		money_show_stat("±øÂíÙ¸½ÌÌÃ");
		if(freshflag){
			sprintf(filepath,"%s/M.%d.A",DIR_MC_MARRY,mm->setfile);
			show_welcome(filepath,4,22);
			freshflag =0;
		}
		move(4, 10);
		local_now_t = time(NULL);
		if(mm->marry_t > local_now_t)
			prints("»éÀñ»¹Î´¿ªÊ¼");
		else prints("»éÀñ½øĞĞÖĞ...");
		move(t_lines - 1, 0);
		prints("\033[1;44m Ñ¡µ¥ \033[1;46m [1]ËÍÀñ½ğ [2]ËÍÏÊ»¨ [3]ËÍºØ¿¨ [Q]Àë¿ª\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			freshflag = 1;
			if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
				move(5, 4);
				prints("    \033[1;32m  »ÆÂí¹Ó°¡£¬¹«¹«ËµÄãÃÇÈ¥ËÍÖíÍ·°É \033[m");
				pressanykey();
				break;
			}
			marry_givemoney(mm);
			break;
		case '2':
			freshflag = 1;
			move(5,4);
			if(mm->marry_t > local_now_t){
				prints("»éÀñ»¹Î´¿ªÊ¼,ÇëÉÔºóÔÙÀ´");
				pressanykey();
				break;
			}
			if(!strcmp(mm->bride, currentuser.userid) || !strcmp(mm->bridegroom, currentuser.userid)){
				move(7 ,4);
				prints("¹ş¹ş£¬¸ø×Ô¼ÒÈËËÍ¶«Î÷¾Í²»ÓÃÕâÃ´Âé·³ÁË°É...");
				pressanykey();
				break;
			}
			if(local_now_t%2==1){	//ĞÂÀÉĞÂÄï¸÷Ò»°ëµÄ»ú»áÊÜÀñ
				strncpy(uident,mm->bride,IDLEN);
				prints("ÏÊ»¨½«ËÍµ½ĞÂÄï\033[1;31m%s\033[mµÄÊÖÖĞ",uident);
			}else{
				strncpy(uident,mm->bridegroom,IDLEN);
				prints("ÏÊ»¨½«ËÍµ½ĞÂÀÉ\033[1;32m%s\033[mµÄÊÖÖĞ",uident);
			}
			pressanykey();
			if (shop_present(1, "ÏÊ»¨",uident) == 9) {
				sprintf(buf, "[¹§ºØĞÂ»é]%s¹§ºØ%sºÍ%sĞÂ»é´óÏ²", currentuser.userid, mm->bride, mm->bridegroom);
				sprintf(genbuf, "ËÍ\033[1;31mÏÊ»¨\033[mÒ»Êø\n\n¹§×£ĞÂÀÉĞÂÄï½á»é´óÏ²£¬°ÙÄêºÃºÏ£¬ÔçÉú¹ó×Ó:)");
				deliverreport(buf, genbuf);
			}
			break;
		case '3':
			freshflag = 1;
			move(5,4);
			if(mm->marry_t > local_now_t){
				prints("»éÀñ»¹Î´¿ªÊ¼,ÇëÉÔºóÔÙÀ´");
				pressanykey();
				break;
			}
			if(!strcmp(mm->bride, currentuser.userid) || !strcmp(mm->bridegroom, currentuser.userid)){
				move(7 ,4);
				prints("¹ş¹ş£¬¸ø×Ô¼ÒÈËËÍ¶«Î÷¾Í²»ÓÃÕâÃ´Âé·³ÁË°É...");
				pressanykey();
				break;
			}
			if(local_now_t%2==1){	//ĞÂÀÉĞÂÄï¸÷Ò»°ëµÄ»ú»áÊÜÀñ
				strncpy(uident,mm->bride,IDLEN);
				prints("ºØ¿¨½«ËÍµ½ĞÂÄï\033[1;31m%s\033[mµÄÊÖÖĞ",uident);
			}else{
				strncpy(uident,mm->bridegroom,IDLEN);
				prints("ºØ¿¨½«ËÍµ½ĞÂÀÉ\033[1;32m%s\033[mµÄÊÖÖĞ",uident);
			}
			pressanykey();
			if(shop_present(2, "ºØ¿¨",uident) == 9) {
				sprintf(buf, "[¹§ºØĞÂ»é]%s¹§ºØ%sºÍ%sĞÂ»é´óÏ²", currentuser.userid, mm->bride, mm->bridegroom);
				sprintf(genbuf, "ËÍ\033[1;32mºØ¿¨\033[mÒ»ÕÅ\n\n¹§×£ĞÂÀÉĞÂÄï½á»é´óÏ²£¬°ÙÄêºÃºÏ£¬ÔçÉú¹ó×Ó:)");
				deliverreport(buf, genbuf);
			}
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

static int PutMarryRecord(struct MC_Marry *marryMem, int n, struct MC_Marry *new_mm)
{
	int i, slot = -1;

	for(i = 0; i < n; i++) {
		if(marryMem[i].enable == 0 && slot == -1)	//·Åµ½µÚÒ»¸ö¿ÕÎ»
			slot = i;
	}
	if(slot >= 0) {
		memcpy(&marryMem[slot], new_mm, sizeof(struct MC_Marry));
	}else{
		append_record(MC_MARRY_RECORDS, new_mm, sizeof(struct MC_Marry));
	}
	return slot;
}

//Çó»é
static int
marry_court(struct MC_Marry *marryMem, int n)
{
	char note[3][STRLEN];
	char buf[STRLEN];
	struct MC_Marry mm;
	int i;
	char uident[IDLEN+2];

	money_show_stat("½ÌÌÃµÇ¼Ç´¦");
	if (seek_in_file(MC_MARRIED_LIST, currentuser.userid)){
		move(5, 4);
		prints("ÄãÒÑ¾­½á»éÁË°¡£¬Ğ¡ĞÄ¸æÄãÖØ»é×ï£¡");
		pressanykey();
		return 0;
	}
	for(i=0;i<n;i++){
		if(!strcmp(marryMem[i].bride,currentuser.userid) || !strcmp(marryMem[i].bridegroom,currentuser.userid) ){
			if( marryMem[i].status == MAR_COURT ){
				move(5, 4);
				prints("Î¹£¡ÄãÕıÇó×Å»éÄØ£¬ÕâÃ´²»×¨Ò»£¬ÈÃmmÔõÃ´ÏàĞÅÄã");
				pressanykey();
				return 0;
			}
			/*
			else if(marryMem[i].status == MAR_MARRIED){
				move(5, 4);
				prints("ÄãÒÑ¾­½á»éÁË°¡£¬Ğ¡ĞÄ¸æÄãÖØ»é×ï£¡");
				pressanykey();
				return 0;
			}
			*/
			else if(marryMem[i].status == MAR_MARRYING){
				move(5, 4);
				prints("ÓĞÃ»¸ã´í£¬»éÀñÕıÔÚ¾ÙĞĞÄØ£¬ÓÖÒªÇó»é£¬ÄÔ´üÃ»·¢ÉÕ°É~~");
				pressanykey();
				return 0;
			}
		}
	}

	move(5,4);
	prints("»éÒö·ÇÍ¬¶ùÏ·£¬±¾Õ¾²»Ìá³«Àë»é£¬ÇëÉ÷ÖØ¿¼ÂÇ£¡£¡");
	move(6,4);
	if (askyn("ÄúÏÂ¶¨¾õĞÄÒªÇó»éÁËÂğ£¿", NA, NA) == NA) {
		move(7, 4);
		prints("°¦£¬»¹ÊÇÔÙµÈµÈ°É....");
		pressanykey();
		return 0;
	}

	money_show_stat("½ÌÌÃµÇ¼Ç´¦");
	move(5, 4);
	usercomplete("ÄÄÎ»mmÕâÃ´ĞÒ¸££¿", uident);
	if (uident[0] == '\0')
		return 0;
	if(!getuser(uident)) {
		move(6, 4);
		prints("Ã»ÓĞÕâÃ´¸ömm°¡....");
              pressanykey();
              return 0;
	}
	if(!strcmp(uident, currentuser.userid)){
		move(6, 4);
		prints("Î¹£¬ĞÑĞÑ°É£¬ÔÙ×ÔÁµÒ²²»ÄÜÏò×Ô¼ºÇó»é°¡£¡");
		pressanykey();
		return 0;
	}
	if (seek_in_file(MC_MARRIED_LIST, uident)){
		move(6, 4);
		prints("ÈË¼ÒÒÑ¾­½á»éÁËÑ½£¬µ±µÚÈıÕß²»ºÃµÄ£¡");
		pressanykey();
		return 0;
	}
	for(i=0;i<n;i++){
		if(!strcmp(marryMem[i].bride,uident) || !strcmp(marryMem[i].bridegroom,uident) ){
			if( marryMem[i].status == MAR_COURT && !strcmp(marryMem[i].bridegroom,uident)){
				move(5, 4);
				prints("ÄãËÀĞÄ°É£¬ÈË¼ÒÒÑ¾­Ïò±ğÈËÇó»éÁË....");
				pressanykey();
				return 0;
			}
			/*
			else if(marryMem[i].status == MAR_MARRIED){
				move(5, 4);
				prints("ÈË¼ÒÒÑ¾­½á»éÁËÑ½£¬µ±µÚÈıÕß²»ºÃµÄ£¡");
				pressanykey();
				return 0;
			}
			*/
			else if(marryMem[i].status == MAR_MARRYING){
				move(5, 4);
				prints("ÓĞÃ»¸ã´í£¬ÈË¼ÒÕı½á»éÄØ£¬µ·Ê²Ã´ÂÒ°¡~~");
				pressanykey();
				return 0;
			}
		}
	}


	move(7, 4);
	prints("ÓĞ»°Ïë¶ÔmmËµÂğ£¿[¿ÉÒÔĞ´3ĞĞà¸]");
	bzero(note, sizeof (note));
	for (i = 0; i < 3; i++) {
		getdata(8 + i, 0, ": ", note[i], STRLEN - 1, DOECHO, NA);
		if (note[i][0] == '\0')
			break;
	}

	sprintf(genbuf, "         \033[1;31mÇó»é\033[m\n\n");
	if (i > 0) {
		int j;
		sprintf(buf, "\033[1;33m%s\033[mÎÂÇéµÄËµ:\n", currentuser.userid);
		strcat(genbuf,buf);
		for (j = 0; j < i; j++){
			strcat(genbuf,note[j]);
			strcat(genbuf,"\n");
		}
	}
	strcat(genbuf,"\n´ó¼ÒÆëÉù: ¼Ş¸øËû°É£¬¼Ş¸øËû°É~~");

	move(11, 4);
	sprintf(buf,"½á»é¿É²»ÊÇĞ¡ÊÂ£¬ÒªÏëºÃÁËÅ¶£¬ÄúÏÂ¶¨¾õĞÄÏò%sÇó»éÁËÂğ?",uident);
	if (askyn(buf, YEA, NA) == NA) {
		move(12, 4);
		prints("°¦£¬¿ÉºŞ½ôÒª¹ØÍ·ÎÒÔõÃ´¾ÍÃ»Õâ¸öÓÂÆø~~");
		pressanykey();
		return 0;
	}

	memset(&mm,0,sizeof(struct MC_Marry));
	mm.enable = 1;
	strcpy(mm.bride ,uident);
	strcpy(mm.bridegroom, currentuser.userid);
	mm.status = MAR_COURT;
	mm.giftmoney = 0;
	mm.attendmen = 0;
	mm.court_t = time(NULL);
	mm.marry_t = 0;
	mm.divorce_t = 0;
	strcpy(mm.subject, "×·Öğ°®ÇéµÄ³¯Ñô");
	mm.setfile = 0;
	mm.invitationfile = 0;

	PutMarryRecord(marryMem, n, &mm);

	sprintf(buf,"[ºÅÍâ]%sÏò%sÉîÇéµÄÇó»é",currentuser.userid,uident);
 	deliverreport(buf, genbuf);
	sprintf(buf, "[¹§Ï²]%sÉîÇéµÄÏòÄúÇó»é", currentuser.userid);
	mail_buf(genbuf, uident, buf);
	move(13, 4);
	prints("¹§Ï²Äú£¬ÄúµÄÅ¨ÒâÉîÇéÒÑËÍµ½%sÊÖÖĞ£¬µÈ´ıºÃÏûÏ¢°É~~",uident);
	pressanykey();
	return 0;
}


char *get_date_str(time_t *tt)
{
    struct tm *tm;
	static char timestr[200];
	tm = localtime(tt);
	sprintf(timestr,"%04dÄê\033[1;33m%02d\033[mÔÂ\033[1;33m%02d\033[mÈÕ \033[1;33m%02d\033[mÊ±:\033[1;33m%02d\033[m·Ö",
		tm->tm_year+1900, tm->tm_mon+1,tm->tm_mday, tm->tm_hour, tm->tm_min);
	//prints(timestr);
	return timestr;
}

static int
marry_selectday(struct MC_Marry *mm)
{
	int ch, quit = 0;
	time_t local_now_t = time(NULL);
	//mm->marry_t = local_now_t;
	if(mm->marry_t < local_now_t){
		move(5,4);
		prints("»éÀñÒÑ¾­¿ªÊ¼...");
		return 0;
	}
	while (!quit) {
		money_show_stat("±øÂíÙ¸½ÌÌÃ");
		local_now_t = time(NULL);
		//ÏŞÖÆÔÚÒ»ÄêÄÚ£¬10·ÖÖÓºó¾ÙĞĞ
		if(mm->marry_t ==0) mm->marry_t = local_now_t + 600;
		if(mm->marry_t - local_now_t <600) mm->marry_t = local_now_t + 600;
		if(mm->marry_t - local_now_t >365*30*24*60*60 ) mm->marry_t = local_now_t + 365*30*24*60*60;
		move(6, 4);
		prints("Á¼³½¼ªÈÕ:  " );
		prints(get_date_str(&mm->marry_t));
		move(10, 4);
		prints("°´¼üµ÷Õû: ab[+-ÔÂ] cd[+-ÈÕ] ef[+-Ê±] gh[+-]·Ö [Q]½áÊø");

		ch = igetkey();
		switch (ch) {
		case 'a':
		case 'A':	//ÔÂ
			mm->marry_t += 30*24*60*60;
			break;
		case 'b':
		case 'B':
			mm->marry_t -= 30*24*60*60;
			break;
		case 'c':	//ÈÕ
		case 'C':
			mm->marry_t += 24*60*60;
			break;
		case 'd':
		case 'D':
			mm->marry_t -= 24*60*60;
			break;
		case 'e':	//Ê±
		case 'E':
			mm->marry_t += 60*60;
			break;
		case 'f':
		case 'F':
			mm->marry_t -= 60*60;
			break;
		case 'g':
		case 'G':
			mm->marry_t += 60;
			break;
		case 'h':
		case 'H':
			mm->marry_t -= 60;
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}
	return 0;
}

static int
marry_editinvitation(struct MC_Marry *mm)
{
	FILE *oldfp,*newfp;
	char buf[400];
	time_t t;
	char filepath[STRLEN];
	char attach_path[STRLEN];
	char edittmp[STRLEN];
	time_t local_now_t= time(NULL);

	strncpy(filepath,DIR_MC_MARRY,STRLEN-1);
	if(mm->invitationfile == 0){
		t = trycreatefile(filepath, "M.%d.A", local_now_t, 100);
		if (t < 0)
			return -1;
		mm->invitationfile = t;
		oldfp = fopen(MC_MAEEY_INVITATION,"r");	//³õÊ¼Ê¹ÓÃÄ¬ÈÏÎÄ¼ş
		if(oldfp){
			newfp = fopen(filepath,"w");
			if(newfp){
				while(!feof(oldfp)){
					if(fgets(buf,sizeof(buf),oldfp) == NULL)
						break;
					char *s;
					int i;
					while (1) {
						s = strstr(buf, "$bridegroom");
						if (s == 0)
							break;
						for (i = 0; i < 11; i++)
							s[i] = 32;
						for (i = 0; i < strlen(mm->bridegroom); i++)
							s[i] = mm->bridegroom[i];
						}
					while (1) {
						s = strstr(buf, "$bride");
						if (s == 0)
							break;
						for (i = 0; i < 6; i++)
							s[i] = 32;
						for (i = 0; i < strlen(mm->bride); i++)
							s[i] = mm->bride[i];
						}
					while (1) {
						s = strstr(buf, "$marrytime");
						if (s == 0)
							break;
						for (i = 0; i < strlen(get_date_str(&mm->marry_t)); i++)
							s[i] =get_date_str(&mm->marry_t)[i];
						}
					fprintf(newfp,"%s",buf);
				}
				fclose(oldfp);
			}
			fclose(newfp);
		}
	}else
	sprintf(filepath,"%s/M.%d.A",DIR_MC_MARRY,mm->invitationfile);

	if (dashl(filepath) || !dashf(filepath))
				return -1;
	sprintf(edittmp, MY_BBS_HOME "/bbstmpfs/tmp/%s.%d",
		currentuser.userid, getpid());
	copyfile_attach(filepath, edittmp);
	if (vedit(edittmp, 0, YEA) < 0) {
		unlink(edittmp);
		clear();
		do_delay(-1);	/* by ylsdd */
		return -1;
	}
	snprintf(attach_path, sizeof (attach_path),
		 PATHUSERATTACH "/%s", currentuser.userid);
	clearpath(attach_path);
	decode_attach(filepath, attach_path);
	insertattachments_byfile(filepath, edittmp,
					 currentuser.userid);
	unlink(edittmp);
	return 1;
}


static int
marry_editset(struct MC_Marry *mm)
{
	FILE *oldfp,*newfp;
	char buf[400];
	time_t t;
	char filepath[STRLEN];
	char attach_path[STRLEN];
	char edittmp[STRLEN];
	time_t local_now_t= time(NULL);

	strncpy(filepath,DIR_MC_MARRY,STRLEN-1);
	if(mm->setfile == 0){
		t = trycreatefile(filepath, "M.%d.A", local_now_t, 100);
		if (t < 0)
			return -1;
		mm->setfile = t;
		oldfp = fopen(MC_MAEEY_SET,"r");	//³õÊ¼Ê¹ÓÃÄ¬ÈÏÎÄ¼ş
		if(oldfp){
			newfp = fopen(filepath,"w");
			if(newfp){
				while(!feof(oldfp)){
					if(fgets(buf,sizeof(buf),oldfp) == NULL)
						break;
					char *s;
					int i;
					while (1) {
						s = strstr(buf, "$bridegroom");
						if (s == 0)
							break;
						for (i = 0; i < 11; i++)
							s[i] = 32;
						for (i = 0; i < strlen(mm->bridegroom); i++)
							s[i] = mm->bridegroom[i];
						}
					while (1) {
						s = strstr(buf, "$bride");
						if (s == 0)
							break;
						for (i = 0; i < 6; i++)
							s[i] = 32;
						for (i = 0; i < strlen(mm->bride); i++)
							s[i] = mm->bride[i];
						}
					while (1) {
						s = strstr(buf, "$marrytime");
						if (s == 0)
							break;
						for (i = 0; i < strlen(get_date_str(&mm->marry_t)); i++)
							s[i] = get_date_str(&mm->marry_t)[i];
						}
					fprintf(newfp,"%s",buf);
					}/*by macintosh 20051203*/
				fclose(oldfp);
			}
			fclose(newfp);
		}
	}else
	sprintf(filepath,"%s/M.%d.A",DIR_MC_MARRY,mm->setfile);

	if (dashl(filepath) || !dashf(filepath))
				return -1;
	sprintf(edittmp, MY_BBS_HOME "/bbstmpfs/tmp/%s.%d",
		currentuser.userid, getpid());
	copyfile_attach(filepath, edittmp);
	if (vedit(edittmp, 0, YEA) < 0) {
		unlink(edittmp);
		clear();
		do_delay(-1);	/* by ylsdd */
		return -1;
	}
	snprintf(attach_path, sizeof (attach_path),
		 PATHUSERATTACH "/%s", currentuser.userid);
	clearpath(attach_path);
	decode_attach(filepath, attach_path);
	insertattachments_byfile(filepath, edittmp,
					 currentuser.userid);
	unlink(edittmp);

	return 1;
}


//×¼±¸»éÀñ
static int
marry_perpare(struct MC_Marry *marryMem, int n)
{
	int ch, quit = 0;
	int i;
	struct MC_Marry *mm;
	char buf[STRLEN];
	char filepath[STRLEN];
	char title[STRLEN];
	char uident[IDLEN+2];
	int freshflag = 1;

	clear();
	for(i=0; i<n; i++){
		if(marryMem[i].enable == 0) continue;
		if(marryMem[i].status != MAR_MARRYING) continue;
		if(!strcmp(marryMem[i].bride, currentuser.userid) || !strcmp(marryMem[i].bridegroom, currentuser.userid)){
			mm = &marryMem[i];
			break;
		}
	}
	if(i>=n){
		prints("½ÌÌÃÃ»ÓĞÄúµÄ»éÀñµÇ¼Ç°¡£¬ÄúÇó»éÁËÂğ£¿Ëı´ğÓ¦ÁËÂğ£¿");
		pressanykey();
		return 0;
	}
	while (!quit) {
		money_show_stat("±øÂíÙ¸½ÌÌÃ");
		if(freshflag){
			//sprintf(filepath,"%s/M.%d.A",DIR_MC_MARRY,mm->setfile);
			//show_welcome(filepath,4,22);
			freshflag =0;
		}
		move(5, 4);
		prints(mm->subject);
		move(6, 4);
		prints("ĞÂÄï:\033[1;31m%s\033[m ĞÂÀÉ:\033[1;32m%s\033[m ",mm->bride, mm->bridegroom);
		move(7, 4);
		prints("Ã»Ïëµ½½á´Î»éÕâÃ´²»ÈİÒ×£¬Ã¦µÄÔÎÍ·×ªÏò£¬\n    ²»¹ıÏëÏë»éºóµÄĞÒ¸£Éú»î£¬ºÙºÙ£¬ĞÄÀïÄÇ¸öÃÀ°¡~~");
		move(t_lines - 1, 0);
		prints("\033[1;44m Ñ¡µ¥ \033[1;46m [1]Ñ¡¼ªÈÕ [2]Ğ´Çë¼í [3]·¢ÇëÌù [4]¹«¸æÌìÏÂ [5]ÉèÖÃÖ÷Ìâ [6]²¼ÖÃ½ÌÌÃ [Q]Àë¿ª\033[m");
		ch = igetkey();
		switch (ch) {
		case '1':
			freshflag = 1;
			marry_selectday(mm);
			break;
		case '2':
			freshflag = 1;
			marry_editinvitation(mm);
			break;
		case '3':
			if (HAS_PERM(PERM_DENYMAIL)) {
				move(5, 4);
				prints("Äú±»½ûÖ¹·¢ĞÅ");
				pressanykey();
				break;
			}
			freshflag = 1;
			clear();
			move(5, 4);
			if (askyn("Òª·¢Çë¼í¸øËùÓĞºÃÓÑÂğ£¿", YEA, NA) == NA) {
				move(6, 4);
				usercomplete("·¢Çë¼í¸øÄÄÎ»£¿", uident);
				if (uident[0] == '\0')
					break;
				if(!searchuser(uident)) {
					move(7, 4);
	                		prints("Ãû×Ö¼Ç´íÁË°É...");
					pressanykey();
	                		break;
					}
				if(!strcmp(uident,currentuser.userid)){
					move(10, 4);
					prints("Î¹£¬¸ã´íÁË°É£¬¸ø×Ô¼º·¢Çë¼í°¡");
					pressanykey();
					break;
				}
				sprintf(filepath,"%s/M.%d.A",DIR_MC_MARRY,mm->invitationfile);
				sprintf(title,"%sÌ¨Æô,%sÓë%sµÄ»éÀñÇë¼í",uident,mm->bride,mm->bridegroom);
				mail_file(filepath,uident,title);
			}else {
				sprintf(filepath,"%s/M.%d.A",DIR_MC_MARRY,mm->invitationfile);
				for (i = 0; i  < uinfo.fnum; i++) {
					move(6, 4);
					clrtoeol();
					getuserid(uident, uinfo.friend[i]);
					if (!getuser(uident)) {
						prints("%sÕâ¸öÊ¹ÓÃÕß´úºÅÊÇ´íÎóµÄ.\n",uident);
						pressanykey();
						continue;
					} else if (!(lookupuser.userlevel & PERM_READMAIL)) {
						prints("ÎŞ·¨ËÍĞÅ¸ø [1m%s[m\n", lookupuser.userid);
						pressanykey();
						continue;
 					} else if (!strcmp(uident, currentuser.userid)) {
						prints("×Ô¼º¾Í²»Òª¸ø×Ô¼º·¢Çë¼í°É\n");
						pressanykey();
						continue;
					}
					sprintf(title,"%sÌ¨Æô,%sÓë%sµÄ»éÀñÇë¼í",uident,mm->bride,mm->bridegroom);
					mail_file(filepath,uident,title);
				}
			}
			move(11, 4);
			prints("Çë¼íÒÑ·¢ËÍ");
			pressanykey();
			break;
		case '4':		//°æÃæ¹«¸æ
			freshflag = 1;
			move(9, 4);
			if(mm->invitationfile == 0){
				prints("»¹Ã»Ğ´ºÃÇë¼íÄØ");
				pressanykey();
				break;
			}
			sprintf(buf,"ÄúÈ·¶¨Òª·¢½á»éÇë¼íµ½¡°´ó¸»ÎÌ¡±°æÂğ£¿");
			if (askyn(buf, YEA, NA) == NA) {
				move(10, 4);
				prints("Âı×Å£¬Çë¼í»¹ÒªÔÙ¸Ä¸Ä~~");
				pressanykey();
				break;
			}
			sprintf(filepath,"%s/M.%d.A",DIR_MC_MARRY,mm->invitationfile);
			sprintf(title,"[Çë¼í]¾´ÇëÄúãØµÚ²Î¼Ó%sºÍ%sµÄ½á»éµäÀñ",mm->bride,mm->bridegroom);
			move(12, 4);
			if (mm->enable<3){
				postfile(filepath, MC_BOARD, title ,1);
				mm->enable++;
				prints("ÄúµÄ»éÊÂÒÑ¹«¸æÌìÏÂ£¬¹§Ï²À²~~");
			}else
				prints("ÇëÌùÒ²²»Òª×Ü·¢Ñ½£¬Á½´Î¾ÍºÃÁË~~");
			pressanykey();
			break;
		case '5':
			freshflag = 1;
			buf[0] = 0;
			getdata(9, 0, "ÇëÊäÈë»éÀñÖ÷Ìâ[×î¶à28ºº×Ö]: ", buf, 56, DOECHO, NA);
			if(buf[0]){
				strncpy(mm->subject,buf,58);
			}
			break;
		case '6':
			freshflag = 1;
			marry_editset(mm);
			break;
		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
		limit_cpu();
	}//end of while...
	return 0;
}

//Àë»é
static int
marry_divorce()
{
	clear();
	move(10, 4);
	//prints("°¥Ñ½£¬º¢×Ó¶¼ÕâÃ´´óÁË»¹ÓĞÊ²Ã´Ïë²»¿ªµÄ£¬¿ì»ØÈ¥ºÃºÃ¹ıÈÕ×Ó°É~~");
	prints("½á»é×ÔÓÉ£¬Àë»é×ÔÔ¸£¬½»×ã³¡µØ·Ñ£¬Ìì¸ßÈÎÄã·É");
	move(12, 4);
	prints("ÈôÏëÀë»é£¬ÇëÓë´ó¸»ÎÌ»éÒö¹ÜÀí°ì¹«ÊÒÁªÏµ½â¾ö");
	pressanykey();
	return 0;
}

//ºÚÃûµ¥
static int
money_deny()
{
	char uident[STRLEN];
	char ans[8];
	char msgbuf[256];
	int count;

	while (1) {
		clear();
		prints("Éè¶¨ºÚÃûµ¥\n");
		count = listfilecontent(MC_DENY_FILE);
		if (count)
			getdata(1, 0, "(A)Ôö¼Ó (D)É¾³ı (C)¸Ä±ä or (E)Àë¿ª [E]: ", ans,
				7, DOECHO, YEA);
		else
			getdata(1, 0, "(A)Ôö¼Ó or (E)Àë¿ª [E]: ", ans, 7,
				DOECHO, YEA);
		if (*ans == 'A' || *ans == 'a') {
			move(1, 0);
			usercomplete("°ÑË­¼ÓÈëºÚÃûµ¥: ", uident);
			if (*uident != '\0')
				if (mc_addtodeny(uident, msgbuf, 0 ) == 1)
					mc_denynotice(1, uident, msgbuf);
		} else if ((*ans == 'C' || *ans == 'c')) {
			move(1, 0);
			usercomplete("¸Ä±äË­µÄ·â½ûÊ±¼ä»òËµÃ÷: ", uident);
			if (*uident != '\0')
				if (mc_addtodeny(uident, msgbuf, 1) == 1)
					mc_denynotice(3, uident, msgbuf);
		} else if ((*ans == 'D' || *ans == 'd') && count) {
			move(1, 0);
			namecomplete("´ÓºÚÃûµ¥ÖĞÉ¾³ıË­: ", uident);
			move(1, 0);
			clrtoeol();
			if (uident[0] != '\0')
				if (del_from_file(MC_DENY_FILE, uident))
					mc_denynotice(2, uident, msgbuf);
		} else
			break;
	}
	clear();
	return 1;
}



static int
mc_addtodeny(char *uident, char *msg, int ischange)
{
	char buf[50], strtosave[256];
	char buf2[50];
	int day;
	time_t nowtime;
	char ans[8];
	int seek;

	seek = seek_in_file(MC_DENY_FILE, uident);
	if ((ischange && !seek) || (!ischange && seek)) {
		move(2, 0);
		prints("ÊäÈëµÄID²»¶Ô!");
		pressreturn();
		return -1;
	}
	buf[0] = 0;
	move(2, 0);
	prints("·â½û¶ÔÏó£º%s", uident);
	while (strlen(buf) < 4)
		getdata(3, 0, "ÊäÈëËµÃ÷(ÖÁÉÙÁ½×Ö): ", buf, 40, DOECHO, YEA);

	do {
		getdata(4, 0, "ÊäÈëÌìÊı(0-ÊÖ¶¯½â·â): ", buf2, 4, DOECHO, YEA);
		day = atoi(buf2);
	} while (day < 0);

	nowtime = time(NULL);
	if (day) {
		struct tm *tmtime;
		time_t undenytime = nowtime + day * 24 * 60 * 60;
		tmtime = gmtime(&undenytime);
		sprintf(strtosave, "%-12s %-40s %2dÔÂ%2dÈÕ½â \x1b[%ldm", uident,
			buf, tmtime->tm_mon + 1, tmtime->tm_mday,
			(long int) undenytime);
		sprintf(msg,
			"¾İ´ó¸»ÎÌĞÂÎÅ·¢ÑÔÈË½ñÈÕÍ¸Â¶£¬%s ÒòÎª"
			" \033[1m%s\033[m Ô­Òò±»×Ü¹Ü %s ½ûÖ¹½øÈë´ó¸»ÎÌÓÎÏ·"
			" %d Ìì£¬Ï£ÍûËùÓĞ´ó¸»ÎÌÈËÊ¿ÒıÒÔÎª½ä£¬"
			"¹²Í¬´´½¨ºÍĞ³´ó¸»ÎÌ£¡",
			uident, buf, currentuser.userid, day);
	} else {
		sprintf(strtosave, "%-12s %-35s ÊÖ¶¯½â·â", uident, buf);
		sprintf(msg, "¾İ´ó¸»ÎÌĞÂÎÅ·¢ÑÔÈË½ñÈÕÍ¸Â¶£¬%s ÒòÎª"
			" \033[1m%s \033[mÔ­Òò±»×Ü¹Ü %s ÓÀ¾Ã½ûÖ¹½øÈë´ó¸»ÎÌÓÎÏ·£¬"
			"Ï£ÍûËùÓĞ´ó¸»ÎÌÈËÊ¿ÒıÒÔÎª½ä£¬¹²Í¬´´½¨ºÍĞ³´ó¸»ÎÌ£¡",
			uident, buf, currentuser.userid);
	}
	if (ischange)
		getdata(5, 0, "ÕæµÄÒª¸Ä±äÃ´?[Y/N]: ", ans, 7, DOECHO, YEA);
	else
		getdata(5, 0, "ÕæµÄÒª·âÃ´?[Y/N]: ", ans, 7, DOECHO, YEA);
	if ((*ans != 'Y') && (*ans != 'y'))
		return -1;
	if (ischange)
		del_from_file(MC_DENY_FILE, uident);
	return addtofile(MC_DENY_FILE, strtosave);
}


static int
mc_denynotice(int action, char *user, char *msgbuf)
{
	char repbuf[STRLEN];
	char repuser[IDLEN + 1];
	strcpy(repuser, user);
	switch (action) {
	case 1:
		sprintf(repbuf,
			"[ºÅÍâ]%s±»ÁĞÈë´ó¸»ÎÌºÚÃûµ¥", repuser);
		deliverreport(repbuf, msgbuf);
		sprintf(repbuf,
			"%s±»%sÁĞÈë´ó¸»ÎÌºÚÃûµ¥",
			user, currentuser.userid);
		mail_buf(msgbuf, user, repbuf);
		millionairesrec(repbuf, msgbuf,"");
		break;
	case 3:
		sprintf(repbuf,
			"%s¸Ä±ä%s´ó¸»ÎÌºÚÃûµ¥µÄÊ±¼ä»òËµÃ÷",
			currentuser.userid, user);
		millionairesrec(repbuf, msgbuf,"");
		mail_buf(msgbuf, user, repbuf);
		break;
	case 2:
		sprintf(repbuf,
			"»Ö¸´ %s ½øÈë´ó¸»ÎÌÓÎÏ·µÄÈ¨Àû",
			repuser);
		snprintf(msgbuf, 256, "%s %s\n"
			 "ÇëÀí½â´ó¸»ÎÌ×Ü¹Ü¹¤×÷£¬Ğ»Ğ»£¡\n",
			 currentuser.userid, repbuf);
		deliverreport(repbuf, msgbuf);
		millionairesrec(repbuf, msgbuf,"");
		mail_buf(msgbuf, user, repbuf);
		break;
	}
	return 0;
}

static int
mc_autoundeny()
{
	char *ptr, buf[STRLEN];
	int undenytime;
	if (!seek_in_file(MC_DENY_FILE, currentuser.userid))
		return 0;
	readstrvalue(MC_DENY_FILE, currentuser.userid, buf, STRLEN);
	ptr=strchr(buf, 0x1b);
	if (ptr)
		memmove(buf, ptr+2, sizeof(buf));
	else return 0;
	undenytime=atoi(buf);
	if (undenytime > time(0))
		return 0;
	if (del_from_file(MC_DENY_FILE, currentuser.userid)) {
		sprintf(buf,
			"»Ö¸´ %s ½øÈë´ó¸»ÎÌÓÎÏ·µÄÈ¨Àû",
			currentuser.userid);
		//deliverreport(buf, "ÇëÀí½â´ó¸»ÎÌ×Ü¹Ü¹¤×÷£¬Ğ»Ğ»£¡\n");
		millionairesrec(buf, "ÏµÍ³×Ô¶¯½â·â\n","");
		mail_buf("ÇëÀí½â´ó¸»ÎÌ×Ü¹Ü¹¤×÷£¬Ğ»Ğ»£¡\n", currentuser.userid, buf);
	}
	return 1;
}

static int
addstockboard(char *sbname, char *fname)
{
	int i;
	int seek;

	if ((i = getbnum(sbname)) == 0){
		move(3, 0);
		prints("´íÎó£¬²»´æÔÚµÄ°æÃæ");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	seek = seek_in_file(fname, sbname);
	if (seek) {
		move(3, 0);
		prints("ÊäÈëµÄ°æÃæÒÑ¾­´æÔÚ!");
		pressreturn();
		return 0;
	}
	move(3, 0);
	if (askyn("ÕæµÄÒªÌí¼ÓÂğ£¿", NA, YEA) == NA) {
		pressanykey();
		return 0;
	}
	return addtofile(fname, sbname);

}

static int
delstockboard(char *sbname, char *fname)
{
	int i, seek;
	if ((i = getbnum(sbname)) == 0){
		move(3, 0);
		prints("´íÎó£¬²»´æÔÚµÄ°æÃæ");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	seek = seek_in_file(fname, sbname);
	if (!seek) {
		move(3, 0);
		prints("ÊäÈëµÄ°æÃæ²»ÔÚÁĞ±íÖĞ!");
		pressreturn();
		return 0;
	}
	move(3, 0);
	if (askyn("ÕæµÄÒªÉ¾³ıÂğ£¿", NA, NA)==NA){
		pressanykey();
		return 0;
	}
	return del_from_file(fname, sbname);
}

static int
stockboards()
{
	char uident[STRLEN];
	char ans[8], repbuf[200], buf[200], titlebuf[STRLEN], bname[STRLEN],  bpath[STRLEN];
	int count, ch2;
	struct stat st;
	FILE *f_fp;

	nomoney_show_stat("Ö¤¼à»áÖ÷Ï¯°ì¹«ÊÒ");
	whoTakeCharge2(6, buf);
	whoTakeCharge(6, uident);
	if (strcmp(currentuser.userid, uident)) {
		move(6, 4);
		prints
	  	  ("ÃØÊé%sÀ¹×¡ÁËÄã,ËµµÀ:¡°Ö÷Ï¯%sÏÖÔÚºÜÃ¦,Ã»Ê±¼ä½Ó´ıÄã¡£¡±", buf,uident);
		pressanykey();
		return 0;
	} else {
		move(6, 4);
		prints("ÇëÑ¡Ôñ²Ù×÷´úºÅ:");
		move(7, 6);
		prints("1. Éè¶¨ÉÏÊĞ°æÃæÃûµ¥          2. ÔİÍ£/»Ö¸´Ä³¹É½»Ò×");
		move(8, 6);
		prints("3. ÔİÍ£È«²¿½»Ò×              4. »Ö¸´È«²¿½»Ò×");
		move(9, 6);
		prints("5. ´ÇÖ°                      6. ÍË³ö");
		ch2 = igetkey();
		switch (ch2) {
		case '1':
			ansimore(MC_STOCK_BOARDS, YEA);
			while (1) {
				clear();
				prints("Éè¶¨ÉÏÊĞ°æÃæÃûµ¥\n");
				count = listfilecontent(MC_STOCK_BOARDS);
				if (count)
					getdata(1, 0, "(A)Ôö¼Ó (D)É¾³ı (E)Àë¿ª [E]: ",
						ans, 7, DOECHO, YEA);
				else
					getdata(1, 0, "(A)Ôö¼Ó  (E)Àë¿ª [E]: ", ans, 7,
						DOECHO, YEA);
				if (*ans == 'A' || *ans == 'a') {
					move(1, 0);
					make_blist();
					namecomplete("Ôö¼Ó°æÃæ: ", bname);
					setbpath(bpath, bname);
					if ((*bname == '\0') || (stat(bpath, &st) == -1)) {
						move(2, 0);
						prints("²»ÕıÈ·µÄÌÖÂÛÇø.\n");
						pressreturn();
						break;
					}
					if (!(st.st_mode & S_IFDIR)) {
						move(2, 0);
						prints("²»ÕıÈ·µÄÌÖÂÛÇø.\n");
						pressreturn();
						break;
					}
					if (bname[0] != '\0' && bname[0] != '\n' && bname[0] != '\r') {
						if (addstockboard(bname, MC_STOCK_BOARDS)) {
							sprintf(repbuf, "[¹«¸æ]%s°æÉÏÊĞ", bname);
							sprintf(buf,
								"¾­¹ı°æÖ÷ÉêÇë£¬´ó¸»ÎÌÖ¤¼à»áÍ¨¹ı£¬"
								"Åú×¼%s°æÃæÉÏÊĞ£¬ÊÔÔËÓªÆÚÒ»¸öÔÂ£¬"
								"Íû¹ã´ó¹ÉÃñ×¢Òâ¡£"
								"ÈçÓĞÏëÉÏÊĞ²¢·ûºÏÌõ¼şµÄ°æÃæ£¬"
								"»¶Ó­°´ÕÕÏà¹ØÁ÷³ÌÉêÇëÉÏÊĞ¡£\n",
								bname);
							deliverreport(repbuf, buf);
							sprintf(titlebuf, "%sĞĞÊ¹¹ÉÊĞ¹ÜÀíÈ¨ÏŞ", currentuser.userid);
							sprintf(repbuf, "Ìí¼ÓÉÏÊĞ°æÃæ: %s°æ", bname);
							millionairesrec(titlebuf, repbuf, "");
						}
					}
				} else if ((*ans == 'D' || *ans == 'd') && count) {
					move(1, 0);
					namecomplete("É¾³ı°æÃæ: ", bname);
					move(1, 0);
					clrtoeol();
					if (bname[0] != '\0' && bname[0] != '\n' && bname[0] != '\r') {
						if (delstockboard(bname, MC_STOCK_BOARDS)) {
							getdata(6, 0, "È¡ÏûÔ­Òò£º", buf, 50, DOECHO, YEA);
							/*move(7, 0);
							if (askyn("È·¶¨Âğ£¿", NA, NA) == NA) {
								addtofile(MC_STOCK_BOARDS, bname);
								pressanykey();
								break;
							}*/
							sprintf(repbuf, "Ô­Òò£º%s", buf);
							sprintf(titlebuf, "[¹«¸æ]%s°æÍËÊĞ", bname);
							deliverreport(titlebuf, repbuf);
							sprintf(titlebuf, "%sĞĞÊ¹¹ÉÊĞ¹ÜÀíÈ¨ÏŞ", currentuser.userid);
							sprintf(repbuf, "È¡ÏûÉÏÊĞ°æÃæ: %s°æ\n\nÈ¡ÏûÔ­Òò£º%s\n", bname, buf);
							millionairesrec(titlebuf, repbuf, "");
						}
					}
				} else
				break;
			}
			break;

		case '2':
			//ansimore(MC_STOCK_STOPBUY, YEA);
			while (1) {
				clear();
				prints("±»ÔİÍ£½»Ò×µÄ°æÃæÃûµ¥\n");
				count = listfilecontent(MC_STOCK_STOPBUY);
				if (count)
					getdata(1, 0, "(A)Ôö¼Ó (D)É¾³ı (E)Àë¿ª [E]: ",
						ans, 7, DOECHO, YEA);
				else
					getdata(1, 0, "(A)Ôö¼Ó  (E)Àë¿ª [E]: ", ans, 7,
						DOECHO, YEA);
				if (*ans == 'A' || *ans == 'a') {
					move(1, 0);
					make_blist();
					namecomplete("ÔİÍ£ÄÄ°æ½»Ò×: ", bname);
					setbpath(bpath, bname);
					if ((*bname == '\0') || (stat(bpath, &st) == -1)) {
						move(2, 0);
						prints("²»ÕıÈ·µÄÌÖÂÛÇø.\n");
						pressreturn();
						break;
					}
					if (!(st.st_mode & S_IFDIR)) {
						move(2, 0);
						prints("²»ÕıÈ·µÄÌÖÂÛÇø.\n");
						pressreturn();
						break;
					}
					if (!seek_in_file(MC_STOCK_BOARDS, bname)){
						move(2, 0);
						prints("ÄúÑ¡ÔñµÄ°æÃæÃ»ÓĞÉÏÊĞ\n");
						pressreturn();
						break;
					}

					if (bname[0] != '\0' && bname[0] != '\n' && bname[0] != '\r') {
						if (addstockboard(bname, MC_STOCK_STOPBUY)) {
							getdata(6, 0, "ÔİÍ£Ô­Òò£º", buf, 50, DOECHO, YEA);
							move(7, 0);
							if (askyn("È·¶¨Âğ£¿", NA, NA) == NA) {
								pressanykey();
								break;
							}
							sprintf(repbuf, "ÔİÍ£Ô­Òò£º%s", buf);
							sprintf(titlebuf, "[¹«¸æ]%s°æ¹ÉÆ±Í£ÅÆ", bname);
							deliverreport(titlebuf, repbuf);
							sprintf(titlebuf, "%sĞĞÊ¹¹ÉÊĞ¹ÜÀíÈ¨ÏŞ", currentuser.userid);
							sprintf(repbuf, "ÔİÍ£%s°æ¹ÉÆ±½»Ò×\n\nÔ­Òò£º%s\n", bname, buf);
							millionairesrec(titlebuf, repbuf, "");
						}
					}
				} else if ((*ans == 'D' || *ans == 'd') && count) {
					move(1, 0);
					namecomplete("Òª»Ö¸´½»Ò×µÄ°æÃæ: ", bname);
					move(1, 0);
					clrtoeol();
					if (bname[0] != '\0' && bname[0] != '\n' && bname[0] != '\r') {
						if (delstockboard(bname, MC_STOCK_STOPBUY)) {
							getdata(6, 0, "»Ö¸´Ô­Òò£º", buf, 50, DOECHO, YEA);
							/*move(7, 0);
							if (askyn("È·¶¨Âğ£¿", NA, NA) == NA) {
								pressanykey();
								break;
							}*/
							sprintf(repbuf, "»Ö¸´Ô­Òò£º%s", buf);
							sprintf(titlebuf, "[¹«¸æ]%s°æ¹ÉÆ±¸´ÅÆ", bname);
							deliverreport(titlebuf, repbuf);
							sprintf(titlebuf, "%sĞĞÊ¹¹ÉÊĞ¹ÜÀíÈ¨ÏŞ", currentuser.userid);
							sprintf(repbuf, "»Ö¸´%s°æ¹ÉÆ±½»Ò×\n\nÔ­Òò£º%s\n", bname, buf);
							millionairesrec(titlebuf, repbuf, "");
						}
					}
				} else
				break;
			}
			break;

		case '3':
		//	utmpshm->ave_score = 0;
			sprintf(buf,"%s/stopbuy",DIR_MC);
			if (file_exist(buf)){
				clear();
				move(6, 4);
				prints("ÒÑ¾­Í£ÅÌ");
				pressreturn();
				break;
			}

			f_fp=fopen(buf,"w");
			if(f_fp!=NULL){
				fclose(f_fp);
				//sprintf(repbuf, "Ô­Òò£º%s", buf);
				//sprintf(titlebuf, "[¹«¸æ]±øÂíÙ¸¹ÉÊĞÍ£ÅÌ");
				deliverreport("[¹«¸æ]±øÂíÙ¸¹ÉÊĞÍ£ÅÌ", "");
				sprintf(titlebuf, "%sĞĞÊ¹¹ÉÊĞ¹ÜÀíÈ¨ÏŞ", currentuser.userid);
				//sprintf(repbuf, "ÔİÍ£È«²¿½»Ò×\n\nÔ­Òò£º%s\n", bname, buf);
				millionairesrec(titlebuf, "ÔİÍ£È«²¿½»Ò×", "");

				clear();
				move(6, 4);
				prints("²Ù×÷³É¹¦!");
				pressanykey();
			}else{
				clear();
				move(6, 4);
				prints("·¢Éú´íÎó");
				pressreturn();
			}
			break;

		case '4':
			sprintf(buf,"%s/stopbuy",DIR_MC);
			if (!file_exist(buf)){
				clear();
				move(6, 4);
				prints("Ã»ÓĞÍ£ÅÌ°¡");
				pressreturn();
				break;
			}
			remove(buf);
			deliverreport("[¹«¸æ]±øÂíÙ¸¹ÉÊĞÖØĞÂ¿ªÅÌ", "");
			sprintf(titlebuf, "%sĞĞÊ¹¹ÉÊĞ¹ÜÀíÈ¨ÏŞ", currentuser.userid);
			millionairesrec(titlebuf, "»Ö¸´È«²¿½»Ò×", "");

			clear();
			move(6, 4);
			prints("²Ù×÷³É¹¦!");
			pressanykey();
			break;

		case '5':
			move(12, 4);
			if (askyn("ÄúÕæµÄÒª´ÇÖ°Âğ£¿", NA,NA) == YEA) {
			sprintf(genbuf, "%s Òª´ÇÈ¥Ö¤¼à»áÖ÷Ï¯Ö°Îñ",
				currentuser.userid);
			mail_buf(genbuf, "millionaires", genbuf);
			move(14, 4);
			prints("ºÃ°É£¬ÒÑ¾­·¢ĞÅ¸æÖª×Ü¹ÜÁË");
			pressanykey();
			}
			break;
		}
	}
	clear();
	return FULLUPDATE;
}

/* »ğ³µÆ±Æ±¼Û¼ÆËã³ÌĞò by macintosh 2006.12.28 */
/* 2007.10.26ĞŞ¸Ä*/

struct ticket_info {
	char CheCi[6];
	char ShiFa[11];
	char ZhongDao[11];
	//ÒÔÉÏÊÇÆ±ÃæĞÅÏ¢
	int LiCheng;
	int LiCheng2;
	//Í¨Æ±Ç°Ò»¶ÎµÄÀï³Ì
	char PiaoZhong;
	//Ñ§²Ğº¢ÍÅÓÅ
	char XiBie;
	//ÈíÓ²
	char JiaKuai;
	//ÆÕ¿ìÌØ¿ì
	char KongTiao;
	//¿Õµ÷
	char WoPu;
	//Ã»ÓĞ£¬ÉÏÖĞÏÂ
	char DongChe;
	//¶¯³µ×éÒ»µÈ×ù£¬¶şµÈ×ù
	char DaoDi;
	//Í¨Æ±µ½µ×ÀàĞÍ
	float ShangFu;
	//ÉÏ¸¡
	float ZaiFu;
	//ÔÙ¸¡
} myTicket;

struct TrainInfo{
	char CheCi[6];
	char KongTiao;
	float ShangFu;
	float ZaiFu;
};

//Ä¬ÈÏ¿ìËÙÌØ¿ìÓĞ¿Õµ÷¡¢ÆÕ¿ìÎŞ¿Õµ÷£¬ÒÔÏÂ½öÁĞ³öÀıÍâ³µ´Î
//ÒÔÏÂ½öÁĞ³öÎ÷°²Õ¾µ½·¢³µ´Î£¬º¬¸´³µ´Î
struct TrainInfo XianTrain[]= {
	{"T193", 2, 0.3, 0.0},{"T194", 2, 0.3, 0.0},{"T191", 2, 0.3, 0.0},{"T192", 2, 0.3, 0.0},
	{"T197", 2, 0.3, 0.0},{"T198", 2, 0.3, 0.0},
	{"K5", 2, 0.4, 0.0},	{"K6", 2, 0.4, 0.0},
	{"K165", 2, 0.4, 0.0},{"K166", 2, 0.4, 0.0},
	{"K173", 0, 0.0, 0.0},{"K174", 0, 0.0, 0.0},{"K171", 0, 0.0, 0.0},{"K172", 0, 0.0, 0.0},
	{"K241", 2, 0.4, 0.0},{"K242", 2, 0.4, 0.0},{"K243", 2, 0.4, 0.0},{"K244", 2, 0.4, 0.0},
	{"K245", 2, 0.4, 0.0},{"K246", 2, 0.4, 0.0},{"K247", 2, 0.4, 0.0},{"K248", 2, 0.4, 0.0},
	{"K317", 2, 0.4, 0.0},{"K318", 2, 0.4, 0.0},{"K315", 2, 0.4, 0.0},{"K316", 2, 0.4, 0.0},
	{"K361", 2, 0.4, 0.0},{"K362", 2, 0.4, 0.0},{"K360", 2, 0.4, 0.0},{"K359", 2, 0.4, 0.0},
	{"K385", 2, 0.4, 0.0},{"K386", 2, 0.4, 0.0},{"K387", 2, 0.4, 0.0},{"K388", 2, 0.4, 0.0},
	{"K419", 2, 0.4, 0.0},{"K420", 2, 0.4, 0.0},{"K417", 2, 0.4, 0.0},{"K418", 2, 0.4, 0.0},
	{"K447", 2, 0.4, 0.0},{"K448", 2, 0.4, 0.0},{"K446", 2, 0.4, 0.0},{"K445", 2, 0.4, 0.0},
	{"K467", 2, 0.3, 0.0},{"K468", 2, 0.3, 0.0},{"K466", 2, 0.3, 0.0},{"K465", 2, 0.3, 0.0},
	{"K543", 2, 0.4, 0.0},{"K544", 2, 0.4, 0.0},{"K542", 2, 0.4, 0.0},{"K541", 2, 0.4, 0.0},
	{"K595", 2, 0.4, 0.0},{"K596", 2, 0.4, 0.0},{"K594", 2, 0.4, 0.0},{"K593", 2, 0.4, 0.0},
	{"K617", 2, 0.4, 0.0},{"K618", 2, 0.4, 0.0},
	{"K621", 2, 0.3, 0.0},{"K622", 2, 0.3, 0.0},{"K623", 2, 0.3, 0.0},{"K624", 2, 0.3, 0.0},
	{"1131", 2, 0.5, 0.0},{"1132", 2, 0.5, 0.0},{"1130", 2, 0.5, 0.0},{"1129", 2, 0.5, 0.0},
	{"1158", 2, 0.5, 0.0},{"1159", 2, 0.5, 0.0},{"1157", 2, 0.5, 0.0},{"1160", 2, 0.5, 0.0},
	{"1353", 2, 0.5, 0.0},{"1354", 2, 0.5, 0.0},{"1352", 2, 0.5, 0.0},{"1351", 2, 0.5, 0.0},
	{"1363", 2, 0.3, 0.0},{"1364", 2, 0.3, 0.0},
	{"1433", 2, 0.3, 0.0},{"1434", 2, 0.3, 0.0},{"1432", 2, 0.3, 0.0},{"1431", 2, 0.3, 0.0},
	{"N373", 0, 0.0, 0.0},{"N374", 0, 0.0, 0.0},
	{"N375", 2, 0.5, -0.15},	{"N376", 2, 0.5, -0.15},	{"N376", 2, 0.5, -0.15},	{"N378", 2, 0.5, -0.15},
	{"N359", 2, 0.4, 0.0},{"N360", 2, 0.4, 0.0},{"N357", 2, 0.4, 0.0},{"N358", 2, 0.4, 0.0},
	{"4901", 2, 0.4, 0.0},{"4902", 2, 0.4, 0.0},{"4903", 2, 0.4, 0.0},{"4904", 2, 0.4, 0.0},
	{"4909", 2, 0.4, 0.0},{"4910", 2, 0.4, 0.0},{"4908", 2, 0.4, 0.0},{"4907", 2, 0.4, 0.0},
	{"4911", 2, 0.3, 0.0},{"4912", 2, 0.3, 0.0},
	{"4915", 2, 0.5, -0.3},{"4916", 2, 0.5, -0.3},{"4917", 2, 0.5, -0.3},{"4918", 2, 0.5, -0.3},

	{"A351", 1, 0.0, 0.0},{"A352", 1, 0.0, 0.0},
};

//ËÄÉáÎåÈë
static float
Round(float num)
{
	num = (float)(int) (num + 0.5);
	return num;
}


//¼ÆËãÓ²Ï¯»ù±¾Æ±
static float
calc_basic_price(int LiCheng, int flag)
{
	int mininum, distance = 0, order = 0, i, j;
	float rate=0, basic_price=0;
	const float BASIC = 0.05861;

	if (LiCheng <= 0)
		return 0;

	//ÆğÂëÀï³Ì
	switch (flag){
		case 2:
			mininum = 100;//¼Ó¿ìÆ±
			break;
		case 3:
			mininum = 400;//ÎÔÆÌÆ±
			break;
		default:
			mininum = 0;//20
			break;
	}
	if (LiCheng <= mininum)
		distance = mininum;
	else
		//¼ÆËã²Î¼ÓÔËËãµÄÀï³Ì
		for (i = 4600, j = 100; j > 0; j -= 10){
			if (LiCheng > i){
				order = (LiCheng - i) / j;
				if ((LiCheng - i) % j == 0)
					order--;
				distance = i + order * j + j/2;
				break;
			}else
				i = i-(j/10-1)*100;
			//´¦ÀíÀï³Ì<=200µÄÇé¿ö
			if (j == 20)
				i = 0;
		}

	//Æ±¼ÛµİÔ¶µİ¼õÇø¶Î
	const int qd[7] = {0, 200, 500, 1000, 1500, 2500, 99999};

	for (i = 0; i < 6; i++){
		if (distance > qd[i]){
			rate = BASIC * (1 - 0.1 * i);
			basic_price += rate * (min(qd[i+1], distance) - qd[i]);
		} else
			break;
	}
	return basic_price;
}


static float
show_ticket()
{
	float JiBenPiao, BaoXian, KePiao, KuaiPiao, KongPiao, WoPiao, QuanJia, JiJin, CheZhan, KePiao2;
	int YouXiaoQi=2, i;
	float jk1=0, jk2=0;
	char printbuf[1024], ZTKN[5], printbuf2[128];

	if (myTicket.DongChe > 0){
		switch (myTicket.DongChe){
			case 3:
				QuanJia = 0.2805 * myTicket.LiCheng * 1.1 * 0.75;
				break;
			case 2:
				QuanJia = 0.3366 * myTicket.LiCheng * 1.1;
				break;
			case 1:
			default:
				QuanJia = 0.2805 * myTicket.LiCheng  * 1.1;
				break;
		}
		QuanJia = ceil(QuanJia);

		sprintf(printbuf, "\033[1m%s%s\033[0m¶¯³µ×éÁĞ³µÆ±¼ÛĞÅÏ¢£¨Àï³Ì%d¹«Àï£©",
			myTicket.CheCi,  myTicket.CheCi[0]?"´Î":"",
			myTicket.LiCheng);
		showAt(5, 6, printbuf, 0);
		showAt(9, 6, "×¢Òâ: ¶¯³µ×éÆ±¼Û½ö¹©²Î¿¼£¬¾ßÌåÆ±¼Û²Î¼û³µÕ¾¹«¸æ¡£", 0);

		sprintf(printbuf, "£¤ %.2f Ôª\t\t\t  ¶¯³µ×é%sµÈ×ù",
			QuanJia,
			(myTicket.DongChe == 2)?"Ò»":"¶ş");
		showAt(16, 6, printbuf, 0);

		if (myTicket.DongChe == 3)
			showAt(16, 24, "(Ñ§)", 0);
		showAt(18, 6, "ºÍ Ğ³ ºÅ", 0);

		return QuanJia;
	}

	JiBenPiao = calc_basic_price(myTicket.LiCheng, 0);

	//¿ÍÆ±¼ÆËã
	//Í¨Æ±
	if (myTicket.DaoDi > 0){
		JiBenPiao = calc_basic_price(myTicket.LiCheng2, 0);
		BaoXian = 0.02 * JiBenPiao;
		BaoXian = ceil(BaoXian * 10); //Ö±½Ó½øÎ»µ½0.1Ôª
		BaoXian = BaoXian/10;
		//ÈíÏ¯
		if (myTicket.XiBie == 1)
			KePiao2 = Round(JiBenPiao * 2 + BaoXian);
		else
			KePiao2 = Round(JiBenPiao + BaoXian);
		if (KePiao2 < 1)
			KePiao2 = 1;
		KePiao2 = Round(KePiao2 * (1 + myTicket.ShangFu)) -Round(JiBenPiao + BaoXian);
		//È«³Ì
		JiBenPiao = calc_basic_price(myTicket.LiCheng, 0);
		BaoXian = 0.02 * JiBenPiao;
		BaoXian = ceil(BaoXian * 10); //Ö±½Ó½øÎ»µ½0.1Ôª
		BaoXian = BaoXian/10;
		KePiao = Round(JiBenPiao + BaoXian);
		KePiao += KePiao2;
	}
	else{
		KePiao = JiBenPiao;
		//±£ÏÕ·Ñ¼ÆËã
		BaoXian = 0.02 * JiBenPiao;
		BaoXian = ceil(BaoXian * 10); //Ö±½Ó½øÎ»µ½0.1Ôª
		BaoXian = BaoXian/10;
		if (myTicket.XiBie == 1)
			KePiao *= 2;
		KePiao += BaoXian;
		KePiao = Round(KePiao);
		if (KePiao < 1)
			KePiao = 1;
		KePiao *= (1 + myTicket.ShangFu);//ÉÏ¸¡
		KePiao = Round(KePiao);
	}

	//¼Ó¿ìÆ±¼ÆËã
	KuaiPiao = 0;
	//ÏÈËãÇ°°ë²¿·Ö(·ÇÍ¨Æ±ËãÈ«³Ì)
	JiBenPiao = calc_basic_price(myTicket.LiCheng2, 2);
	if (myTicket.JiaKuai > 0)
		KuaiPiao = 0.2 * JiBenPiao;
	KuaiPiao = Round(KuaiPiao);
	KuaiPiao *= (1 + myTicket.ShangFu);//ÉÏ¸¡
	KuaiPiao = Round(KuaiPiao);
	if (myTicket.JiaKuai > 1)
		KuaiPiao *= 2;

	//Í¨Æ±¼ÆËãÇ°³ÌÏàÓ¦µ½µ×µÈ¼¶µÄ¼Ó¿ìÆ±jk1
	if (myTicket.DaoDi > 0){
		if (myTicket.DaoDi == 2){
			if (myTicket.JiaKuai < 1)//¿÷Ç®£¬ÆÕ¿ÍÁĞ³µÆÕ¿ìµ½µ×
				jk1 = 0;
			else{
				jk1 = 0.2 * JiBenPiao;
				jk1 = Round(jk1);
			}
		}
		if (myTicket.DaoDi == 3){
			if (myTicket.JiaKuai < 1)//¿÷Ç®£¬ÆÕ¿ÍÁĞ³µÌØ¿ìµ½µ×
				jk1 = 0;
			else if (myTicket.JiaKuai < 2){//¿÷Ç®£¬ÆÕ¿ìÁĞ³µÌØ¿ìµ½µ×
					jk1 = 0.2 * JiBenPiao;
					jk1 = Round(jk1);
			}else{
				jk1 = 0.2 * JiBenPiao;
				jk1 = Round(jk1);
				jk1 *= 2;
			}
		}
	}

	//Í¨Æ±È«³Ìµ½µ×¼Ó¿ìÆ±jk2
	if (myTicket.DaoDi > 1){
		JiBenPiao = calc_basic_price(myTicket.LiCheng, 2);
		jk2 = 0.2 * JiBenPiao;
		jk2 = Round(jk2);
	}
	if (myTicket.DaoDi > 2)
		jk2 *= 2;

	KuaiPiao = KuaiPiao - jk1 + jk2;


	//¿Õµ÷Æ±¼ÆËã
	KongPiao = 0;
	if (myTicket.KongTiao > 0){
		JiBenPiao = calc_basic_price(myTicket.LiCheng2, 4);
		KongPiao = 0.25 * JiBenPiao;
		KongPiao = Round(KongPiao);
		if (KongPiao < 1)
			KongPiao = 1; //¿Õµ÷Æ±²»×ã1Ôª°´1ÔªÊÕ
		KongPiao *= (1 + myTicket.ShangFu);//ÉÏ¸¡
		KongPiao = Round(KongPiao);
	}

	//ÎÔÆÌÆ±¼ÆËã
	WoPiao = 0;
	JiBenPiao = calc_basic_price(myTicket.LiCheng2, 3);
	if (myTicket.XiBie == 0){//Ó²ÎÔ
		if (myTicket.WoPu == 1)
			WoPiao = 1.1 * JiBenPiao;
		else if (myTicket.WoPu == 2)
			WoPiao = 1.2 * JiBenPiao;
		else if (myTicket.WoPu == 3)
			WoPiao = 1.3 * JiBenPiao;
	} else {//ÈíÎÔ
		if (myTicket.WoPu == 1)
			WoPiao = 1.75 * JiBenPiao;
		else if (myTicket.WoPu > 1)
			WoPiao = 1.95 * JiBenPiao;
	}
	WoPiao = Round(WoPiao);
	WoPiao *= (1 + myTicket.ShangFu);//ÉÏ¸¡
	WoPiao = Round(WoPiao);

	//ÔÙÉÏ¸¡
	KePiao = Round((1 + myTicket.ZaiFu) * KePiao);
	KuaiPiao = Round((1 + myTicket.ZaiFu) * KuaiPiao);
	KongPiao = Round((1 + myTicket.ZaiFu) * KongPiao);
	WoPiao = Round((1 + myTicket.ZaiFu) * WoPiao);

	//Ñ§ÉúÆ±
	if (myTicket.PiaoZhong == 1 && myTicket.XiBie == 0){
		KePiao *= 0.5;
		KuaiPiao *= 0.5;
		KongPiao *= 0.5;
	}

	//Ğ¡º¢Æ±
	if (myTicket.PiaoZhong == 2){
		KePiao *= 0.5;
		KuaiPiao *= 0.5;
		KongPiao *= 0.5;
	}

	//²ĞÆ±
	if (myTicket.PiaoZhong == 3){
		KePiao *= 0.5;
		KuaiPiao *= 0.5;
		KongPiao *= 0.5;
		WoPiao *= 0.5;
	}

	//Ğ¡º¢µ¥¶ÀÊ¹ÓÃÎÔÆÌ
	if (myTicket.PiaoZhong == 4){
		KePiao = 0;
		KuaiPiao = 0;
		KongPiao *= 0.5;
	}

	//ÎÔÆÌ¶©Æ±·Ñ
	if (WoPiao > 0)
		WoPiao += 10;

	//¿ÍÆ±ĞÅÏ¢»¯·¢Õ¹»ù½ğ
	if (KePiao + KuaiPiao + KongPiao + WoPiao > 5)
		JiJin = 1;
	else
		JiJin = 0.5;

	//³µÕ¾¿Õµ÷·Ñ
	if (myTicket.LiCheng >= 200)
		CheZhan = 1;
	else
		CheZhan = 0;
	//ÈíÏ¯²»ÊÕ¿Õµ÷·Ñ
	if (myTicket.XiBie)
		CheZhan = 0;

	if (myTicket.LiCheng <= 0)
		KePiao = KuaiPiao = KongPiao = WoPiao = 0;

	QuanJia = KePiao + KuaiPiao + KongPiao + WoPiao + JiJin + CheZhan;

	sprintf(printbuf, "¿ÍÆ±Æ±¼Û£º\t%.2f Ôª", KePiao);
	showAt(7, 6, printbuf, 0);
	sprintf(printbuf, "ÎÔÆÌÆ±Æ±¼Û£º\t%.2f Ôª", WoPiao);
	showAt(7, 44, printbuf, 0);
	sprintf(printbuf, "\033[1;30mÒâÍâÉËº¦±£ÏÕ£º\t%.2f Ôª", BaoXian);
	showAt(8, 6, printbuf, 0);
	sprintf(printbuf, "ÎÔÆÌ¶©Æ±·Ñ£º\t%.2f Ôª\033[m", (WoPiao > 0) ? 10.0 : 0.0);
	showAt(8, 44, printbuf, 0);
	sprintf(printbuf, "¼Ó¿ìÆ±Æ±¼Û£º\t%.2f Ôª", KuaiPiao);
	showAt(9, 6, printbuf, 0);
	sprintf(printbuf, "¿Õµ÷Æ±Æ±¼Û£º\t%.2f Ôª", KongPiao);
	showAt(9, 44, printbuf, 0);
	sprintf(printbuf, "¿ÍÆ±ĞÅÏ¢»¯»ù½ğ£º\t%.2f Ôª", JiJin);
	showAt(10, 6, printbuf, 0);
	sprintf(printbuf, "³µÕ¾¿Õµ÷·Ñ£º\t%.2f Ôª\033[m", CheZhan);
	showAt(10, 44, printbuf, 0);

	switch (myTicket.CheCi[0]){
		case 'Z':
			strcpy(ZTKN, "Ö±´ï");
			break;
		case 'K':
		case 'N':
			strcpy(ZTKN, "¿ìËÙ");
			break;
		case 'T':
		default:
			strcpy(ZTKN, "ÌØ¿ì");
			break;
	}
	sprintf(printbuf, "£¤ %.2f Ôª\t\t\t  %s%s%s%s%s",
		QuanJia,
		(myTicket.KongTiao == 2)?"ĞÂ":"",
		(myTicket.KongTiao > 0)?"¿Õµ÷":"",
		(myTicket.XiBie)?"Èí×ù":"Ó²×ù",
		(myTicket.JiaKuai>0)?((myTicket.JiaKuai>1)?ZTKN:"ÆÕ¿ì"):"ÆÕ¿Í",
		(myTicket.WoPu>0)?"ÎÔ":"");
	showAt(16, 6, printbuf, 0);

	if (myTicket.DaoDi > 0){
		sprintf(printbuf, "(%sÖÁµ½Õ¾)",
			(myTicket.DaoDi > 1)?((myTicket.DaoDi > 2)?"ÌØ¿ì":"ÆÕ¿ì"):"ÆÕ¿Í");
		showAt(16, 60, printbuf, 0);
		showAt(17, 42, "ÖÁ»»³ËÕ¾", 0);
	}

	if (myTicket.DaoDi > 0)
		sprintf(printbuf2, "£¬ÖĞ×ªÇ°%d¹«Àï", myTicket.LiCheng2);
	else
		printbuf2[0] = 0;
	sprintf(printbuf, "\033[1m%s%s\033[0m%sÁĞ³µÆ±¼ÛĞÅÏ¢£¨Àï³Ì%d¹«Àï%s£©",
		myTicket.CheCi,  myTicket.CheCi[0]?"´Î":"",
		(myTicket.JiaKuai>0)?((myTicket.JiaKuai>1)?ZTKN:"ÆÕ¿ì"):"ÆÕÍ¨",
		myTicket.LiCheng,
		printbuf2);
	showAt(5, 6, printbuf, 0);

	YouXiaoQi = 2;
	i = 500;
	while (myTicket.LiCheng > i){
		YouXiaoQi ++;
		i += 1000;
	}
	sprintf(printbuf, "ÔÚ %d ÈÕÄÚµ½ÓĞĞ§", YouXiaoQi);
	showAt(18, 6, printbuf, 0);

	switch (myTicket.PiaoZhong){
		case 1:
			showAt(16, 24, "(Ñ§)", 0);
			break;
		case 2:
		case 4:
			showAt(16, 24, "(º¢)", 0);
			break;
		case 3:
			showAt(16, 24, "(²Ğ)", 0);
			break;
		default:
			break;
	}

	if (myTicket.ShangFu > 0 && myTicket.ShangFu < 0.5 && myTicket.KongTiao == 2)
		showAt(17, 24, "(ÕÛ)", 0);

	return QuanJia;
}
/*
static int
calc_ticket_price()
{
	int ch, quit = 0, temp, i;
	char buf[STRLEN], ZTK=0;
	float tempf;

	bzero(&myTicket, sizeof (struct ticket_info));
	clear();
	while (!quit) {
		nomoney_show_stat("»ğ³µÆ±Æ±¼Û¼ÆËã");
		show_ticket();
		showAt(t_lines - 2, 0, "\033[1;44m Éè \033[1;46m [0]³µ´Î [1]Àï³Ì [2]Æ±ÖÖ [3]Ï¯±ğ [4]µÈ¼¶ [5]¿Õµ÷ [6]ÉÏ¸¡ÂÊ [7]ÎÔÆÌ [8]ÔÙ¸¡ÂÊ          \033[m", 0);
		if (myTicket.DaoDi == 0)
			showAt(t_lines - 1, 0, "\033[1;44m ÖÃ \033[1;46m [A]Í¨Æ± [D]¶¯³µ×éÆ±¼Û [H]°ïÖú [Q]Àë¿ª                                                          \033[m", 0);
		else
			showAt(t_lines - 1, 0, "\033[1;44m ÖÃ \033[1;46m [A]Í¨Æ± [B]ÖĞ×ªÇ°Àï³Ì [H]°ïÖú [Q]Àë¿ª                                             \033[m", 0);

		ch = igetkey();
		switch (ch) {
		case '0':
			getdata(t_lines-3, 0, "ÇëÊäÈë³µ´Î: ", buf, 5, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			bzero(&myTicket, sizeof (struct ticket_info));
			sprintf(myTicket.CheCi, "%s", buf);
			if (isalpha(buf[0])){
				ZTK = toupper(buf[0]);
				myTicket.CheCi[0] = ZTK;
			}else
				ZTK = 0;
			temp = atoi(buf);

			if (ZTK=='Z' || ZTK=='T' || ZTK=='K' || ZTK=='N'){
				myTicket.JiaKuai = 2;
				myTicket.KongTiao = 2;
				myTicket.ShangFu = 0.5;
			}else if (ZTK=='D')
				myTicket.DongChe = 1;
			else if (temp < 6000)
				myTicket.JiaKuai = 1;
			else
				myTicket.JiaKuai = 0;

			i = 0;
			while (XianTrain[i].CheCi[0] != '\0'){
				if (!strcmp(XianTrain[i].CheCi, myTicket.CheCi)){
					myTicket.KongTiao = XianTrain[i].KongTiao;
					myTicket.ShangFu = XianTrain[i].ShangFu;
					myTicket.ZaiFu = XianTrain[i].ZaiFu;
					break;
				}
				i++;
			}
			break;

		case '1':
			getdata(t_lines-3, 0, "ÇëÊäÈëÀï³Ì: ", buf, 5, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.LiCheng = (temp > 0) ? temp : 0;
			myTicket.LiCheng2 = myTicket.LiCheng;
			break;

		case '2':
			showAt(t_lines-4, 0, "0.È«¼Û 1.Ñ§ÉúÆ± 2.Ğ¡º¢Æ± 3.ÉË²Ğ¾üÈËÆ± 4.Ğ¡º¢µ¥¶ÀÊ¹ÓÃÎÔÆÌ", 0);
			getdata(t_lines-3, 0, "ÇëÊäÈëÆ±ÖÖ: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.PiaoZhong = (temp > 0 && temp < 5) ? temp : 0;
			break;

		case '3':
			showAt(t_lines-4, 0, "0.Ó²Ï¯   1.ÈíÏ¯", 0);
			getdata(t_lines-3, 0, "ÇëÊäÈëÏ¯±ğ: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.XiBie = (temp > 0 && temp < 2) ? temp : 0;
			break;

		case '4':
			showAt(t_lines-4, 0, "0.ÆÕ¿Í   1.ÆÕ¿ì   2.¿ìËÙ/ÌØ¿ì/Ö±ÌØ", 0);
			getdata(t_lines-3, 0, "ÇëÊäÈëµÈ¼¶: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.JiaKuai = (temp > 0 && temp < 3) ? temp : 0;
			break;

		case '5':
			showAt(t_lines-4, 0, "0.ÎŞ¿Õµ÷   1.ÆÕÍ¨ÓĞ¿Õµ÷   2.ĞÂĞÍÓĞ¿Õµ÷", 0);
			getdata(t_lines-3, 0, "ÇëÊäÈëµÈ¼¶: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.KongTiao = (temp > 0 && temp < 3) ? temp : 0;
			if (myTicket.KongTiao == 1)
				myTicket.ShangFu = 0.0;
			else if (myTicket.KongTiao == 2)
				myTicket.ShangFu = 0.5;
			break;

		case '6':
			getdata(t_lines-3, 0, "ÇëÊäÈëÉÏ¸¡ÂÊ: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			tempf = atof(buf);
			if (tempf <= 0)
				tempf = 0;
			else if (tempf >= 2.99)
				tempf = 2.99;
			myTicket.ShangFu = tempf;
			break;

		case '7':
			if (myTicket.XiBie == 0)
				showAt(t_lines-4, 0, "0.È¡Ïû   1.ÉÏÆÌ  2.ÖĞÆÌ  3.ÏÂÆÌ", 0);
			else
				showAt(t_lines-4, 0, "0.È¡Ïû   1.ÉÏÆÌ  2.ÏÂÆÌ", 0);
			getdata(t_lines-3, 0, "ÇëÊäÈëÎÔÆÌÖÖÀà: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.WoPu= (temp > 0 && temp < 4) ? temp : 0;
			break;

		case '8':
			getdata(t_lines-3, 0, "ÇëÊäÈëÔÙ¸¡ÂÊ(¿ÉÎª¸ºÖµ): ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			tempf = atof(buf);
			if (tempf < -0.5)
				tempf = -0.5;
			else if (tempf > 0.3)
				tempf = 0.3;
			myTicket.ZaiFu = tempf;
			break;


		case 'A':
		case 'a':
			if (myTicket.DongChe > 0){
				myTicket.DaoDi = 0;
				showAt(t_lines-4, 0, "\033[1;5;31m¶¯³µ×éÁĞ³µ³µÆ±×îÔ¶Ö»·¢ÊÛÖÁ±¾´ÎÁĞ³µÖÕµãÕ¾¡£\033[m", 0);
				pressreturn();
				break;
			}	 //¶¯³µ×é²»´òÍ¨Æ±
			showAt(t_lines-4, 0, "0.È¡Ïû   1.ÆÕ¿Íµ½µ×  2.ÆÕ¿ìµ½µ×  3.ÌØ¿ìµ½µ×", 0);
			getdata(t_lines-3, 0, "ÇëÊäÈëÍ¨Æ±ÀàĞÍ: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.DaoDi= (temp > 0 && temp < 4) ? temp : 0;
			break;

		case 'B':
		case 'b':
			if (myTicket.DaoDi == 0)
				break;
			getdata(t_lines-3, 0, "ÇëÊäÈëÖĞ×ªÇ°µÄÀï³Ì: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.LiCheng2 = (temp > 0) ? temp : 0;
			if (myTicket.LiCheng2 > myTicket.LiCheng)
				myTicket.LiCheng2 = myTicket.LiCheng;
			break;

		case 'd':
		case 'D':
			showAt(t_lines-4, 0, "0.È¡Ïû   1.¶şµÈ×ù   2.Ò»µÈ×ù   3.¶şµÈ×ùÑ§ÉúÆ±", 0);
			getdata(t_lines-3, 0, "ÇëÊäÈë: ", buf, 6, DOECHO, YEA);
			if (buf[0] == '\0' || buf[0] == '\n')
				break;
			temp = atoi(buf);
			myTicket.DongChe = (temp > 0 && temp < 4) ? temp : 0;
			if (myTicket.DongChe > 0)
				myTicket.DaoDi = 0; //¶¯³µ×é²»´òÍ¨Æ±
			break;

		case 'h':
		case 'H':
			clear();
			showAt(5, 4, "Ö±´ï¡¢ÌØ¿ì¡¢¿ìËÙÁĞ³µÄ¬ÈÏÎªĞÂ¿Õµ÷ÁĞ³µ£¬ÈçĞèĞŞ¸ÄÇë°´5¡£", 0);
			showAt(7, 4, "ĞÂ¿Õµ÷ÁĞ³µÉÏ¸¡ÂÊÎª0.5£»Ò»¡¢¶şµµÕÛ¿Û·Ö±ğÎª0.4¡¢0.3£¬ÈçĞèĞŞ¸ÄÇë°´6¡£", 0);
			showAt(9, 4, "\033[1mÓĞ³ÌĞò·½ÃæÒÉÎÊÇëµ½\033[32m"MC_BOARD"\033[37m°æ×ÉÑ¯!\033[m", 0);
			showAt(11, 4, "\033[1mÓĞÌúÂ·Æ±¼Û·½ÃæÒÉÎÊÇëµ½\033[32mtraffic\033[37m°æ×ÉÑ¯!\033[m", 0);
			showAt(13, 4, "\033[1;32m¸ĞĞ»ÄúµÄÊ¹ÓÃ! »¶Ó­ÄúÏÂ´ÎÔÙÀ´!\033[m", 1);
			break;

		case 'q':
		case 'Q':
			quit = 1;
	    	 	break;
		}
	}
	return 0;
}
*/
/* ¾è¿î by macintosh  */

static int
loadContributions(char *cname, char *user)
{
	char value[20];
	char path[256];
	sprintf(path, DIR_CONTRIBUTIONS"%s", cname);
	if (readstrvalue(path, user, value, 20) != 0)
		return 0;
	else
		return limitValue(atoi(value), sizeof(int));
}  //¶ÁÈ¡¸÷»ù½ğ¾è¿îÊıÖµ

static int
saveContributions(char *cname, char *user, int valueToAdd)
{
	int valueInt;
	char value[20], path[256];

	sprintf(path, DIR_CONTRIBUTIONS"%s", cname);
	if (readstrvalue(path, user, value, 20) != 0)
		valueInt = 0;
	else
		valueInt = limitValue(atoi(value), MAX_CTRBT_NUM);
	valueInt += valueToAdd;
	valueInt = limitValue(valueInt, MAX_CTRBT_NUM);
	snprintf(value, 20, "%d", valueInt);
	savestrvalue(path, user, value);
	return 0;
}  //±£´æ¾è¿îÊıÖµ


static void
doContributions(struct MC_Jijin *clist)
{
	int money, i=0, num=0, num2, total_num, old_num ;
	float transfer_rate;
	char title[80], buf[512];

	clear();
	sprintf(buf, "No. %-12.12s  %16.16s  %s", "»ù½ğID", "»ù½ğÃû³Æ", "ÀÛ¼Æ¾è¿î");
	showAt(5, 2, buf, 0);
	while (clist[i].userid[0]!= 0){
		sprintf(buf, "ctr_%s", clist[i].userid);
		old_num = loadValue(currentuser.userid, buf, MAX_CTRBT_NUM);
		sprintf(buf, "%2d  %-12.12s  %17.17s  %d", i+1, clist[i].userid, clist[i].name, old_num);
		showAt(7+i, 2, buf, 0);
		i++;
	}
	sprintf(title, "ÇëÑ¡Ôñ¾è¿î¶ÔÏó[1-%d]: ", i);
	getdata(t_lines-6, 2, title, buf, 3, DOECHO, YEA);
	if (buf[0] == '\0' || buf[0] == '\n')
		return;
	num = atoi(buf);
	if (num > i || num < 1){
		showAt(t_lines-4, 2, "¿¼ÂÇºÃÁËÔÙ¾è°É...", 1);
		return;
	}
	num --;

	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	getdata(t_lines-5, 2, "ÇëÊäÈëÏÖ½ğ½ğ¶î[ÏÂÏŞ1000]: ", buf, 10, DOECHO, YEA);
	num2 = atoi(buf);
	if (num2 < 1000){
		showAt(t_lines-4, 2, "1000¶¼Ã»ÓĞ°¡...", 1);
		return;
	}
	if (num2 > money || num2 <= 0){
		showAt(t_lines-4, 2, "¶Ô²»Æğ, ÄúÏÖ½ğ½ğ¶î²»×ã", 1);
		return;
	}
	transfer_rate = utmpshm->mc.transfer_rate / 10000.0;
	sprintf(buf,
		" ÊÖĞø·Ñ %.2f£¥£¨×î¸ßÊÕÈ¡ 100000 ±øÂíÙ¸±Ò£¬²»×ã1°´1ÊÕÈ¡¡££©",
		transfer_rate * 100);
	showAt(t_lines-4, 2, buf, 0);
	move(t_lines-3, 2);
	sprintf(buf, "È·¶¨¸ø %s »ù½ğ£¨%s£©¾è%d ±øÂíÙ¸±ÒÂğ£¿", clist[num].name, clist[num].userid, num2);

	if (askyn(buf, YEA, NA) == YEA) {
		if (num2 * transfer_rate >= 100000) {
			total_num = num2 + 100000;
		} else {
			total_num = num2 * (1.0 + transfer_rate);
		}
		if (total_num - num2 < 1)
			total_num +=1;
		if (money < total_num) {
			move(t_lines-2, 4);
			prints("ÄúµÄÏÖ½ğ²»¹»£¬¼ÓÊÖĞø·Ñ¹²Ğè %d ±øÂíÙ¸±Ò", total_num);
			pressanykey();
			return;
		}
		saveValue(currentuser.userid, MONEY_NAME, -total_num, MAX_MONEY_NUM);
		saveValue(clist[num].userid, MONEY_NAME, num2, MAX_MONEY_NUM);
		sprintf(title, "[Í¨Öª] %s ¸ø%s»ù½ğ¾è¿î", currentuser.userid, clist[num].name);
		sprintf(buf,
			"%s Í¨¹ı±øÂíÙ¸¾è¿î°ì¹«ÊÒÏòÄú¾èÔù %d ±øÂíÙ¸±Ò£¬Çë²éÊÕ¡£",
			currentuser.userid, num2);
		mail_buf(buf, clist[num].userid, title);

		sprintf(buf, "ctr_%s", clist[num].userid);
		saveValue(currentuser.userid, buf, num2, MAX_CTRBT_NUM);
		saveContributions(clist[num].userid, currentuser.userid, num2);

		sprintf(title, "[¹«¸æ] %s»ù½ğ£¨%s£©ÊÕµ½¾è¿î", clist[num].name, clist[num].userid);
		sprintf(buf,"¸ĞĞ»%s¶Ô%s»ù½ğµÄ´óÁ¦Ö§³Ö£¬±øÂíÙ¸´ó¸»ÎÌ´ú±íÈ«ÌåÄÉË°ÈËÏòÆä±íÊ¾¸ĞĞ»£¡", currentuser.userid, clist[num].name);
		deliverreport(title, buf);

		sprintf(genbuf, "%s½øĞĞ¾è¿î", currentuser.userid);
		sprintf(buf,"%s¾è¿î¸ø%s»ù½ğ£¨%s£© %d±øÂíÙ¸±Ò", currentuser.userid, clist[num].name, clist[num].userid, num2);
		millionairesrec(genbuf, buf, "¾è¿î");
		showAt(t_lines-2, 4, "¾è¿î³É¹¦£¬¸ĞĞ»Äã¶Ô±øÂíÙ¸´ó¸»ÎÌµÄÖ§³Ö¡£", 1);
	}
	return;
}

static int
money_contributions()
{
	int ch, money, money2, quit = 0, count = 0;
	void *buffer = NULL;
	size_t filesize;
	char title[STRLEN], buf[256];

	struct MC_Jijin clist1[]= {
		{"millionaires", "´ó¸»ÎÌ»ù½ğ"},
		{"BMYbeg", "Ø¤°ï»ù½ğ"},
		{"BMYRober", "ºÚ°ï»ù½ğ"},
		{"BMYpolice", "¾¯Êğ»ù½ğ"},
		{"BMYKillersky", "É±ÊÖ»ù½ğ"},
		{"", ""}
	};
	struct MC_Jijin *clist2;

	while (!quit) {
		nomoney_show_stat("´ó¸»ÎÌ¾è¿î°ì¹«ÊÒ");
		showAt(6, 4, "Ï×³öÒ»·İ°®ĞÄ", 0);
		showAt(t_lines - 1, 0,
			"\033[1;44m Ñ¡µ¥ \033[1;46m [1]°ïÅÉ»ù½ğ [2]Ãñ¼ä»ù½ğ [3]´ÈÉÆ¼ÒÅÅĞĞ°ñ [4]¾èÏ×È«²¿²Æ²ú [Q]Àë¿ª             \033[m", 0);
		ch = igetkey();
		switch (ch) {
		case '1':
			doContributions(clist1);
			break;

		case '2':
			count = get_num_records(MC_JIJIN_CTRL_FILE, sizeof(struct MC_Jijin));
			filesize = sizeof(struct MC_Jijin) * count;
			clist2 = loadData(MC_JIJIN_CTRL_FILE, buffer, filesize);
			if (clist2 == (void *) -1)
				break;
			doContributions(clist2);
			break;

		case '3':
			clear();
			showAt(4, 4, "\033[1;32mÔŞÎŞ\033[m", 1);
			break;

		case '4':
			showAt(5, 0,
				"[1;32mÄúÈ·¶¨Ç®·ÅÔÚ¿Ú´üÉÕÊÖ£¬´æÔÚÒøĞĞÉÕĞÄ£¬×¼±¸¿´ÆÆºì³¾ËÄ´ó½Ô¿ÕÅ×¿ªÈ«²¿[m\n"
				"[1;32mÉí¼ÒÉÏÉ½µ±ºÍÉĞÃ´£¿[m\n"
				"[1;32mÄúµÄÈ«²¿²Æ²ú½«Áô¸ømillionaires×÷ÎªÌê¶È·Ñ£¬×Ê½ğ½«ÓÃÓÚ½¨ÉèÏ£ÍûĞ¡Ñ§[m\n"
				"[1;32mºÍÔ®Öú°¬×Ì²¡»¼Õß£¬ÒÔ¼°×ÊÖúÌ¹É£ÄáÑÇ¡¢ÔŞ±ÈÑÇµÈ¹úÆ¶Ãñ[m\n"
				"[1;31m×¢Òâ£º¹«¹«Ö»¸ºÔğÌê¶È²»¸ºÔğ»¹Ë×£¡£¡[m\n"
				"[1;33mÇ®²»ÊÇÍòÄÜµÄ£¬Ã»ÓĞÇ®È´ÍòÍò²»ÄÜ£¬ÈıË¼¶øºóĞĞ°¡£¡[m\n"
				, 0);
			move(12, 0);
			if (askyn("È·¶¨¾èÏ×È«²¿²Æ²úÂğ? ", NA, NA) == YEA) {
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				money2 = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);

				if (money + money2 == 0){
					showAt(15, 0, "Ã»Ç®¾Í²»ÓÃÀ´´ÕÈÈÄÖÁË~", 1);
					break;
				}

				saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
				saveValue("millionaires", MONEY_NAME, money, MAX_MONEY_NUM);
				saveValue(currentuser.userid, "ctr_millionaires", money, MAX_CTRBT_NUM);
				saveValue(currentuser.userid, CREDIT_NAME, -money2, MAX_MONEY_NUM);
				saveValue("millionaires", CREDIT_NAME, money2, MAX_MONEY_NUM);
				saveValue(currentuser.userid, "ctr_millionaires", money2, MAX_CTRBT_NUM);

				sprintf(title, "%s¾èÏ×È«²¿²Æ²ú", currentuser.userid);
				sprintf(buf, "%s¾èÏ×È«²¿²Æ²ú:\nÏÖ½ğ%d±øÂíÙ¸±Ò\n´æ¿î%d±øÂíÙ¸±Ò", currentuser.userid, money, money2);
				millionairesrec(title, buf, "¾è¿î");

				sprintf(title, "[¹«¸æ] ´ó¸»ÎÌ»ù½ğÊÕµ½À´×Ô%sµÄ¾è¿î", currentuser.userid);
				sprintf(buf,"¸ĞĞ»%sÏò±øÂíÙ¸´ó¸»ÎÌ¾èÏ×ÆäÈ«²¿²Æ²ú£¬´ó¸»ÎÌÏòÆä±íÊ¾×î³ç¸ßµÄ¾´Òâ£¡\n"
						"²¢×£Ô¸Æä½ñºóĞŞĞĞË³Àû£¡", currentuser.userid);
				deliverreport(title, buf);

				showAt(15, 0, "Íê³É!", 1);
			}
			break;

		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}
	return 0;
}


static int
money_office()
{
	int ch, quit = 0;
	char uident[IDLEN + 1];

	while (!quit) {
		nomoney_show_stat("±øÂíÙ¸´ó¸»ÎÌ¹ÜÀíÖĞĞÄ");
		showAt(6, 4, "´ó¸»ÎÌ¹ÜÀíÖĞĞÄ»¶Ó­Äã£¡", 0);
		showAt(t_lines - 1, 0,
			"\033[1;44m Ñ¡µ¥ \033[1;46m [1]¾è¿î°ì¹«ÊÒ [2]ĞÅ·Ã°ì¹«ÊÒ [3]¼àÓü [4]ÓÊÕş¾Ö [5]×Ü¹Ü°ì¹«ÊÒ [Q]Àë¿ª       \033[m", 0);
		ch = igetkey();
		switch (ch) {
		case '1':
			money_contributions();
			break;

		case '2':
			if (!HAS_PERM(PERM_POST))
				break;
			move(6, 4);
			if (askyn("È·¶¨Òª·¢ĞÅÂğ? ", NA, NA) == YEA)
				m_send("millionaires");
			break;

		case '3':
			showAt(6, 4, "¿´ÄãÌ½Í·Ì½ÄÔâ«â«ËöËöµÄÑù×Ó£¬´òËã½ÙÓü£¿´ø¸ö°ô°ôÌÇ¾Íµ±ÊÇAK-47£¿\n"
						"Æ¤Ñ÷ÁË°É£¿Ğ¡ĞÄµç¾¯¹÷£¡", 1);
			break;

		case '4':
			money_postoffice();
			break;

		case '5':
			nomoney_show_stat("´ó¸»ÎÌ×Ü¹Ü°ì¹«ÊÒ");
			whoTakeCharge2(11, uident);
			if (strcmp(currentuser.userid, uident)) {
				move(6, 4);
				prints
				    ("Öµ°àÃØÊé%s½Ğ×¡ÁËÄã£¬ËµµÀ:¡°¹«¹«ÃÇÕıÔÚ¿ª»á£¬ÇëÏÈËÄ´¦×ª×ª°É¡£¡±",
				     uident);
				pressanykey();
				break;
			}
			break;

		case 'q':
		case 'Q':
			quit = 1;
			break;
		}
	}
	return 0;
}

