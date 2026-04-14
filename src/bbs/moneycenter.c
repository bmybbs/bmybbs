#include <sys/mman.h>	// for mmap
#include <math.h>
#include "bbs.h"
#include "bbs_global_vars.h"
#include "config.h"
#include "stuff.h"
#include "smth_screen.h"
#include "maintain.h"
#include "namecomplete.h"
#include "main.h"
#include "talk.h"
#include "bbsinc.h"
#include "mail.h"
#include "bm.h"
#include "io.h"
#include "record.h"
#include "more.h"
#include "xyz.h"
#include "boards.h"
#include "edit.h"
#include "bbs-internal.h"
#include "bcache.h"

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
#define MC_STOCK_BOARDS  DIR_MC "stockboards" //上市版面名单
#define MC_STOCK_STOPBUY  DIR_MC "st_stopbuy" //暂停交易的版面名单
#define DIR_CONTRIBUTIONS  DIR_MC "contributions/" //各版面募捐名单存放目录
#define MC_JIJIN_CTRL_FILE  DIR_MC "jijin_ctrl" //出现在捐款中的基金id

#define MAX_RECORD_LINE 100	//记录文件行最大长度
#define MAX_BET_LENGTH  80	//赌注输入最大长度

//货币存储名称
#define MONEY_NAME	"bmy_money"
#define CREDIT_NAME	"bmy_credit"
#define LEND_NAME       "lend_money"
#define INTEREST_NAME   "interest"

//各种金额限制
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
/*修改MAX_STOCK_NUM 要记得再money_stock_board函数中 给stock_name加上相应的股票名称*/

//婚礼数据结构及定义
/*
	结婚程序是这样的:
	男方选择"求婚"向女方求婚,这时登记结婚记录，状态为 MAR_COURT，一星期后自动失败
	女方去教堂，自动提示是否接受求婚？
	如果不接受，结婚记录enable设为0,宣告失败
	如果接受，结婚记录状态设为 MAR_MARRYING 结婚中,结婚日期默认设为1天后
	然后男女方都可以到教堂准备婚礼，包括设置结婚日期，写请柬，发请柬，布置背景等
	此时可以在参加婚礼时的婚礼等级表中看到这条记录，时间一到，婚礼自动开始
	朋友们可以来参见他们的婚礼了，参见婚礼时可以送礼金，送花，送贺卡
	婚礼在四小时后自动结束，发信到金融中心版面
	在MC_MARRY_RECORDS(100条记录)中保存求婚COURT和在婚MARRYING的记录
	在刷新记录时把求婚失败和结婚成功的记录转到MC_MARRY_RECORDS_ALL
*/
#define DIR_MC_MARRY			MY_BBS_HOME"/etc/moneyCenter/marry"
#define MC_MARRY_RECORDS        MY_BBS_HOME"/etc/moneyCenter/marryrecords"
#define MC_MARRY_RECORDS_ALL	MY_BBS_HOME"/etc/moneyCenter/marryrecords_all"
#define MC_MARRIED_LIST 	MY_BBS_HOME"/etc/moneyCenter/marriedlist"
//默认邀请函
#define MC_MAEEY_INVITATION		MY_BBS_HOME"/0Announce/groups/GROUP_0/" MC_BOARD "/system/welcome/invitation"
//默认婚礼布景
#define MC_MAEEY_SET			MY_BBS_HOME"/0Announce/groups/GROUP_0/" MC_BOARD "/system/welcome/frontpage"
#define MAR_COURT		1	//求婚
#define MAR_MARRIED	2	//已婚
#define MAR_MARRYING	3	//结婚中		//婚礼在marry_t四小时后结束
#define MAR_DIVORCE		4	//离婚
#define MAR_COURT_FAIL	5	//求婚失败

struct moneyCenter {
	int ave_score;
	int prize777;
	int prize367;
	int prizeSoccer;
	unsigned char transfer_rate;
	unsigned char deposit_rate;
	unsigned char lend_rate;
	unsigned char isSalaryTime:1,isSoccerSelling:1,isMCclosed:1,unused:5;
};

struct MC_Marry{
	int enable;					//是否有效
	char bride[14];				//新娘
	char bridegroom[14];		//新郎
	int status;					//婚姻状况MAR_...
	int giftmoney;				//礼金
	int attendmen;				//参见人数
	time_t court_t;				//求婚时间
	time_t marry_t;				//结婚时间
	time_t divorce_t;			//离婚时间
	char subject[60];			//主题限30汉字
	time_t setfile;			//婚礼布置的显示文件	时间值
	time_t invitationfile;		//请柬文件	时间值
	time_t visitfile;			//到访人员存储文件
	int visitcount;			//参加人数
	char unused[18];
}; // 150 bytes

#define MC_SHMKEY 38899
static struct moneyCenter *mc;
static char marry_status[][20] = {"未知","求婚","已婚","婚礼中","已离婚","求婚失败",""};
static int multex=0;

static void mc_shm_init(void);
static void *loadData(char *filepath, void *buffer, size_t filesize);
static void saveData(void *buffer, size_t filesize);
static long loadValue(char *user, char *valueName, long sup);
static int saveValue(char *user, char *valueName, long valueToAdd, long sup);
static int show_welcome(char *filepath,int startline,int endline);
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
//static int guess_number(void);
//static int an(char *a, char *b);
//static int bn(char *a, char *b);
//static void itoa(int i, char *a);
static void time2string(time_t num, char *str);
static int money_police(void);
static void persenal_stock_info(int stock_num[15], int stock_price[15],
		int money, char stockboard[STRLEN][MAX_STOCK_NUM],
		int stock_board[15]);
/*static void persenal_stock_info2(int stock_num[15], int stock_price[15],
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
//结婚
static int money_marry();
static int PutMarryRecord(struct MC_Marry *marryMem, int n, struct MC_Marry *new_mm);
static int marry_attend(struct MC_Marry *marryMem, int n);
static int marry_court(struct MC_Marry *marryMem, int n);
static int marry_perpare(struct MC_Marry *marryMem, int n);
static int marry_divorce();
static char *get_date_str(time_t *tt);
static char *get_simple_date_str(time_t *tt);
static int marry_refresh(struct MC_Marry *marryMem, int n);
static int marry_recordlist(struct MC_Marry *marryMem, int n);
static int marry_all_records();
static int marry_active_records(struct MC_Marry *marryMem, int n);
static int marry_query_records(char *id);
static int marry_admin(struct MC_Marry *marryMem, int n);
//黑名单
static int money_deny();
static int mc_addtodeny(char *uident, char *msg, int ischange);
static int mc_denynotice(int action, char *user, char *msgbuf);
static int mc_autoundeny(void);
static int addstockboard(char *sbname, char *fname);
static int delstockboard(char *sbname, char *fname);
static int stockboards();
//static int calc_ticket_price();

static int money_office();

static void showAt(int line, int col, char *str, int flag) {
	move(line, col);
	prints_nofmt(str);
	if (flag == 1)
		pressanykey();
	else if (flag == 2)
		pressreturn();
}

//moneycenter进入界面
int moneycenter(const char *s) {
	(void) s;
	int ch;
	int quit = 0;
	modify_user_mode(MONEY);
	strcpy(currboard, MC_BOARD);
	if (!file_exist(DIR_MC"MoneyValues"))
		mkdir(DIR_MC"MoneyValues", 0770);
	if (!file_exist(DIR_CONTRIBUTIONS))
		mkdir(DIR_CONTRIBUTIONS, 0770);

	mc_shm_init();
	if (mc == NULL) {
		// 系统故障
		clear();
		move(6, 4);
		prints_nofmt("\033[1;31m金融中心系统故障\033[m");
		pressanykey();
		return 0;
	}

	if (!seek_in_file(MC_ADMIN_FILE, currentuser.userid)
			&& !(currentuser.userlevel & PERM_SYSOP)
			&& strcmp(currentuser.userid, "macintosh"))
		if (mc->isMCclosed){
			clear();
			move(6, 4);
			prints_nofmt("\033[1;31m兵马俑金融中心今天休息\033[m");
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
		nomoney_show_stat("兵马俑金融中心");
		move(6, 4);
		prints_nofmt("兵马俑金融中心经过改造，焕然一新，欢迎到处看看！");
		move(8, 4);
		prints_nofmt("学习大富翁游戏规则请去本站系统区millionaires版。");
		move(t_lines - 2, 0);
		prints_nofmt( "\033[1;44m 选 \033[1;46m [1]银行 [2]彩票 [3]赌场 [4]黑帮 [5]丐帮 [6]股市 [7]商场 [8]警署            \n"
				"\033[1;44m 单 \033[1;46m [9]杀手 [C]教堂 [A]大富翁管理中心 [Q]离开                                    ");
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
			money_admin();	//隐藏，且权限检查
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
}  // moneycenter进入界面

static void moneycenter_welcome() {
	clear();
	move(4, 4);
	prints_nofmt("兵马俑金融中心门口立着一块大牌子：");
	move(6, 4);
	prints_nofmt("\033[1;31m打击投机倒把\033[m \033[1;31m稳定金融秩序\033[m");
	move(8, 4);
	prints_nofmt("\033[1;33m金融中心焕然一新，快来捧场！\033[0m");
	pressanykey();
} // 欢迎界面

static void moneycenter_byebye() {
	clear();
	saveValue(currentuser.userid, "multex", -9,9);
	move(5, 14);
	prints_nofmt("\033[1;32m欢迎再次光临金融中心，您的富有是我们的荣幸。\033[m");
	pressanykey();
} //离开界面

//added by macintosh 20051202
int millionairesrec(char *title, char *str, char *owner) {
	struct fileheader postfile;
	char filepath[STRLEN], fname[STRLEN];
	char buf[256];
	time_t now;
	FILE *inf, *of;

	now = time(0);
	snprintf(fname, sizeof fname, "tmp/deliver.millionairesrec.%ld", now);
	if ((inf = fopen(fname, "w")) == NULL)
		return -1;
	fprintf(inf, "%s", str);
	fclose(inf);

	//postfile(fname, owner, "millionairesrec", title);
	memset(&postfile, 0, sizeof (postfile));
	sprintf(filepath, MY_BBS_HOME "/boards/%s/", "millionairesrec");
	now = trycreatefile(filepath, "M.%ld.A", now, 100);
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
	fprintf(of, "发信人: %s (兵马俑大富翁系统记录), 信区: millionairesrec\n", owner[0] ? owner : "millionaires");
	fprintf(of, "标  题: %s\n", title);
	fprintf(of, "发信站: 兵马俑BBS (%24.24s), 本站(bbs.xjtu.edu.cn)\n\n", ctime(&now));
	fprintf(of, "【此篇文章由兵马俑大富翁自动张贴系统发表】\n\n");
	while (fgets(buf, 256, inf) != NULL)
		fprintf(of, "%s", buf);
	fprintf(of, "\n\n");
	fprintf(of, "最近访问IP: %s\n\n", currentuser.lasthost);
	fclose(inf);
	fclose(of);

	sprintf(buf, MY_BBS_HOME "/boards/%s/%s", "millionairesrec", DOT_DIR);
	if (append_record(buf, &postfile, sizeof (postfile)) == -1) {
		//errlog("Posting '%s' on '%s': append_record failed!", postfile.title, "millionairesrec");
		return 0;
	}
	ythtbbs_cache_Board_updatelastpost("millionairesrec");
	unlink(fname);
	return 1;
}//用于系统记录区发文

static int limitValue(int value, int sup) {
	if (value > sup)
		return sup;
	if (value < 0)
		return 0;
	return value;
}

static int savemoneyvalue(char *userid, char *key, char *value) {
	char path[256];
	sprintf(path, DIR_MC"MoneyValues/%s", userid);
	return savestrvalue(path, key, value);
}

static int readmoneyvalue(char *userid, char *key, char *value, int size) {
	char path[256];
	sprintf(path, DIR_MC"MoneyValues/%s", userid);
	return readstrvalue(path, key, value, size);
}

static long loadValue(char *user, char *valueName, long sup) {
	char value[20];
	if (readmoneyvalue(user, valueName, value, 20) != 0)
		return 0;
	else
		return limitValue(atol(value), sup);
}  //读取相关数值

static int saveValue(char *user, char *valueName, long valueToAdd, long sup) {
	long valueInt;
	int retv;
	char value[20];
	valueInt = loadValue(user, valueName, sup);
	valueInt += valueToAdd;
	valueInt = limitValue(valueInt, sup);
	snprintf(value, 20, "%ld", valueInt);
	if ((retv = savemoneyvalue(user, valueName, value)) != 0) {
		errlog("save %s %s %s retv=%d err=%s", currentuser.userid, valueName, value, retv, strerror(errno));
	}
	return 0;
}  //保存相关数值

static void *loadData(char *filepath, void *buffer, size_t filesize) {
	int fd;

	if ((fd = open(filepath, O_RDWR, 0660)) == -1)
		return (void *)-1;
	buffer = mmap(0, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd , 0);
	close(fd);
	return buffer;
}

static void saveData(void *buffer, size_t filesize) {
	if (buffer != NULL)
		munmap(buffer, filesize);
}

// 管理职务系统
static void whoTakeCharge(int pos, char *boss) {
	const char feaStr[][20] = {
		"bank", "lottery", "gambling", "gang", "beggar", "stock", "shop",
		"police","killer","marriage","office"
	};
	if (readstrvalue(MC_BOSS_FILE, feaStr[pos - 1], boss, IDLEN + 1) != 0)
		*boss = '\0';
}

//slowaction 秘书系统
static void whoTakeCharge2(int pos, char *boss) {
	const char feaStr[][20] = {
		"bank", "lottery", "gambling", "gang", "beggar", "stock", "shop",
		"police","killer","marriage","office"
	};
	if (readstrvalue(MC_ASS_FILE, feaStr[pos - 1], boss, IDLEN + 1) != 0)
		*boss = '\0';
}

//检查进入权限
static int check_allow_in() {
	long backTime;
	long freeTime;
	time_t currentTime = time(0);
	int num,money;
	int robTimes;

	mc_autoundeny();
	if (seek_in_file(MC_DENY_FILE, currentuser.userid)){
		clear();
		move(10, 10);
		prints_nofmt("您已经被列入不受大富翁欢迎者名单，抱歉\n");
		pressanykey();
		return 0;
	}

	/* 避免多窗口*/
	if (multex && ythtbbs_cache_UserTable_count_telnet(usernum) > 1) {
		clear();
		move(10, 10);
		prints_nofmt("您已经在金融中心里啦!\n");
		pressanykey();
		return 0;
	}
	set_safe_record();
	if (currentuser.dietime > 0) {
		clear();
		move(10, 10);
		prints_nofmt("您已不在人间，我们不带鬼玩的~~\n");
		pressanykey();
		return 0;
	}

	/* 犯罪被监禁 */
	freeTime = loadValue(currentuser.userid, "freeTime", 2000000000);
	if (currentTime < freeTime) {
		clear();
		move(10, 10);
		if((freeTime - currentTime) / 86400>50)
			saveValue(currentuser.userid, "freeTime", time(0) + 86400 *1, 2000000000);
		prints("你已经被兵马俑警署监禁了。还有%d天的监禁。\n", (freeTime - currentTime) / 86400);
		num=((freeTime - currentTime) / 86400)*25000+25000;
		sprintf(genbuf, "是否要求保释，保释金%d",num);
		if (askyn(genbuf, NA, NA) == YEA) {
			money =	loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
			if (money < num) {
				move(8, 4);
				prints ("您的钱不够，共需 %d 兵马俑币", num);
				pressanykey();
				return 0;
			}
			else {
				saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
				saveValue("BMYpolice", MONEY_NAME, +num, MAX_MONEY_NUM);
				move(8, 4);
				prints_nofmt("保释成功");
				robTimes = loadValue(currentuser.userid, "rob", 50);
				saveValue(currentuser.userid, "rob", -robTimes, 50);
				saveValue(currentuser.userid, "freeTime", -2000000000, 2000000000);
				ytht_del_from_file(DIR_MC "imprison_list", currentuser.userid, true);
			}
		}else{
			pressanykey();
			return 0;
		}
	} else
		if (currentTime > freeTime && freeTime > 0) {
			clear();
			move(10, 10);
			prints_nofmt("监禁期满，恭喜你重新获得自由！");
			saveValue(currentuser.userid, "freeTime", -2000000000, 2000000000);
			ytht_del_from_file(DIR_MC "imprison_list", currentuser.userid, true);
			pressanykey();
		}

	/* 欠款不还 */
	int total_num, lendMoney;
	backTime = loadValue(currentuser.userid, "back_time", 2000000000);
	if((backTime - time(0)) / 3600>5000)
		saveValue(currentuser.userid, "back_time", time(0) + 1* 86400, 2000000000);
	lendMoney = loadValue(currentuser.userid, LEND_NAME, MAX_MONEY_NUM);
	if (backTime < 0 || lendMoney < 0 ) {
		saveValue(currentuser.userid, LEND_NAME, -lendMoney, MAX_MONEY_NUM);
		saveValue(currentuser.userid, "lend_time", -2000000000, 2000000000);
		saveValue(currentuser.userid, "back_time", -2000000000, 2000000000);
	}
	if (currentTime > backTime && backTime > 0) {
		clear();
		move(10, 10);
		if (askyn("你欠银行的贷款到期了，赶紧还吧？", YEA, NA) == YEA) {
			total_num = lendMoney + makeInterest(lendMoney, "lend_time", mc->lend_rate/10000.0);
			money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
			if (money < total_num) {
				move(11, 10);
				prints_nofmt("你的钱不够偿还贷款。");
				pressanykey();
				return 0;
			}
			saveValue(currentuser.userid, MONEY_NAME, -total_num, MAX_MONEY_NUM);
			saveValue(currentuser.userid, LEND_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
			saveValue(currentuser.userid, "lend_time", -2000000000, 2000000000);
			saveValue(currentuser.userid, "back_time", -2000000000, 2000000000);
			ytht_del_from_file(DIR_MC "special_lend", currentuser.userid, true);
			move(12, 10);
			prints_nofmt("好了，你现在无债一身轻啦。");
			pressanykey();
			return 1;
		}
		return 0;
	}

	/* 其它不让进的情况待续 */

	return 1;
}

//计算利息
static int makeInterest(int basicMoney, char *valueName, float rate) {
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

static int makeRumor(int num) {
	if (random() % 3) {
		num += (random() % num) / 5;
	} else {
		num -= (random() % num) / 5;
	}
	return limitValue(num, MAX_MONEY_NUM);
}

static void time2string(time_t num, char *str) {
	int i;
	for (i = 0; num > 0; i++, num /= 10) {
		str[9 - i] = num % 10 + '0';
	}
	str[10] = '\0';
}

//计算是否到领取薪水的时候
static int newSalary() {
	char lastSalaryTime[20];
	return 1;//暂时作废

	if (!readstrvalue(DIR_MC "etc_time", "salary_time", lastSalaryTime, 20)) {
		if (time(0) < atoi(lastSalaryTime) + 30 * 86400)
			return 0;
		return 1;
	}
	return 0;
}

//计算薪水
static int makeSalary() {
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

//职务任免系统
static void sackOrAppoint(int pos, char *boss, int msgType, char *msg) {

	char head[10];
	char in[10];
	char end[10];
	char posDesc[][40] = {
		"兵马俑银行行长", "兵马俑博彩公司经理", "兵马俑赌场经理",
		"兵马俑黑帮帮主", "兵马俑丐帮帮主", "兵马俑证监会主席",
		"兵马俑商场经理", "兵马俑警署署长","兵马俑杀手帮主",
		"兵马俑婚姻管理办公室主任", "兵马俑大富翁管理中心主任"
	};
	if (msgType == 0) {
		strcpy(head, "任命");
		strcpy(in, "为");
		strcpy(end, "");
	} else {
		strcpy(head, "免去");
		strcpy(in, "的");
		strcpy(end, "职务");
	}
	sprintf(msg, "%s %s %s%s%s", head, boss, in, posDesc[pos - 1], end);

}

//秘书任免系统
static void sackOrAppoint2(int pos, char *boss, int msgType, char *msg) {
	char head[10];
	char in[10];
	char end[10];
	char posDesc[][60] = {
		"兵马俑银行行长秘书", "兵马俑博彩公司经理秘书", "兵马俑赌场经理秘书",
		"兵马俑黑帮帮主秘书", "兵马俑丐帮帮主秘书", "兵马俑证监会主席秘书",
		"兵马俑商场经理秘书", "兵马俑警署署长秘书","兵马俑杀手帮主秘书",
		"兵马俑婚姻管理办公室主任秘书", "兵马俑大富翁管理中心秘书"
	};
	if (msgType == 0) {
		strcpy(head, "任命");
		strcpy(in, "为");
		strcpy(end, "");
	} else {
		strcpy(head, "免去");
		strcpy(in, "的");
		strcpy(end, "职务");
	}
	sprintf(msg, "%s %s %s%s%s", head, boss, in, posDesc[pos - 1], end);

}

//银行系统
static int money_bank() {
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
		money_show_stat("兵马俑银行");
		move(8, 16);
		prints_nofmt("兵马俑银行欢迎您的光临！");
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1]换钱 [2]转账 [3]储蓄 [4]贷款 [5]工资 [6]行长办公室 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
			case '1':
				clear();
				money_show_stat("兵马俑银行兑换窗口");
				convert_rate = 100;
				move(4, 4);
				prints("您可以通过变卖文章数获得兵马俑币。今天的汇率是 1:%d", convert_rate);
				move(5, 4);
				prints_nofmt("\033[1;31m注意:文章数一旦变卖概不退还!\033[0m");
				getdata(6, 4, "您要变卖多少文章数？[0]: ", genbuf, 7, DOECHO, YEA);
				num = atoi(genbuf);
				if (num <= 0) {
					break;
				}
				move(7, 4);
				sprintf(genbuf, "确定要变卖 %d 文章数，换取 %d 兵马俑币吗？", num, num * convert_rate);
				if (askyn(genbuf, NA, NA) == YEA) {
					set_safe_record();
					if (currentuser.numposts < (unsigned) num /* safe */) {
						move(8, 4);
						prints_nofmt("您没有那么多文章数...");
						pressanykey();
						break;
					}
					currentuser.numposts -= num;
					substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
					saveValue(currentuser.userid, MONEY_NAME, num * convert_rate, MAX_MONEY_NUM);
					move(8, 4);
					prints("交易成功，这里是您的 %d 兵马俑币。", num * convert_rate);
					sprintf(genbuf, "%s进行银行交易(卖文章)", currentuser.userid);
					sprintf(buf, "%s卖掉%d文章数, 换得 %d兵马俑币", currentuser.userid, num, num * convert_rate);
					millionairesrec(genbuf, buf, "银行交易");
					pressanykey();
				}
				break;
			case '2':
				money_show_stat("兵马俑银行转账窗口");
				move(4, 4);
				transfer_rate = mc->transfer_rate / 10000.0;
				sprintf(genbuf, "最小转账金额 1000 兵马俑币。手续费 %.2f％（最高收取 100000 兵马俑币）", transfer_rate * 100);
				prints_nofmt(genbuf);
				move(5, 4);
				usercomplete("转账给谁？", uident);
				if (uident[0] == '\0')
					break;
				if (!getuser(uident)) {
					move(6, 4);
					prints_nofmt("错误的使用者代号...");
					pressreturn();
					break;
				}
				if (lookupuser.dietime > 0) {
					move(6, 4);
					prints_nofmt("阳间的钱只有烧才能给死人...");
					pressreturn();
					break;
				}
				if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
					if (seek_in_file(DIR_MC "jijin", currentuser.userid));
					else if (!seek_in_file(DIR_MC "mingren", uident)) {
						move(6, 4);
						prints_nofmt("对不起，银行不允许黄马褂向外转帐。");
						pressreturn();
						break;
					}
				}
				getdata(6, 4, "转账多少兵马俑币？[0]", buf, 10, DOECHO, YEA);
				if (buf[0] == '\0') {
					break;
				}
				num = atoi(buf);
				if (num < 1000) {
					move(7, 4);
					prints_nofmt("对不起，未达到最小交易金额。");
					pressanykey();
					break;
				}
				if (currentuser.stay < 86400) {
					move(7, 4);
					prints_nofmt("对不起，银行不向未成年人提供转帐业务。");
					pressanykey();
					break;
				}
				move(7, 4);
				sprintf(genbuf, "确定转账给 %s %d 兵马俑币吗？", uident, num);
				if (askyn(genbuf, NA, NA) == YEA) {
					money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
					if (num * transfer_rate >= 100000) {
						total_num = num + 100000;
					} else {
						total_num = num * (1.0 + transfer_rate);
					}
					if (money < total_num) {
						move(8, 4);
						prints("您的钱不够，加手续费此次交易共需 %d 兵马俑币", total_num);
						pressanykey();
						break;
					}
					saveValue(currentuser.userid, MONEY_NAME, -total_num, MAX_MONEY_NUM);
					saveValue(uident, MONEY_NAME, num, MAX_MONEY_NUM);

					char notebuf[512];
					char note[3][STRLEN];
					int i, j;
					move(9, 0);
					prints_nofmt("还有什么话要附上吗？[可以写3行]");
					bzero(note, sizeof (note));
					for (i = 0; i < 3; i++) {
						getdata(10 + i, 0, ": ", note[i], STRLEN - 1, DOECHO, NA);
						if (note[i][0] == '\0')
							break;
					}
					move(15, 4);
					prints_nofmt("转帐成功，我们已经通知了您的朋友。");
					sprintf(title, "您的朋友 %s 给您送钱来了", currentuser.userid);
					sprintf(notebuf,
							"您的朋友 %s 通过兵马俑银行给您转帐了 %d 兵马俑币，请查收。"
							"\n\n以下是 %s 的附言:\n",
							currentuser.userid, num, currentuser.userid);
					for (j = 0; j < i; j++){
						strcat(notebuf, note[j]);
						strcat(notebuf, "\n");
					}
					mail_buf(notebuf, uident, title);
					if (seek_in_file(DIR_MC "mingren", currentuser.userid))
					{
						sprintf(title, "%s 向 %s 转帐", currentuser.userid, uident);
						sprintf(buf, " %s 通过兵马俑银行向 %s 转帐了 %d 兵马俑币", currentuser.userid, uident, num);
						mail_buf(buf, "millionaires", title);
					}
					if (num >= RUMOR_MONEY && random() % 2) {
						sprintf(genbuf, "据说 %s 收到了一笔 %d 兵马俑币的转帐！", uident, makeRumor(num));
						deliverreport("[谣言]来自兵马俑银行的消息", genbuf);
					}
					sprintf(genbuf, "%s进行银行交易(转账)", currentuser.userid);
					sprintf(buf,"%s转帐给%s %d兵马俑币", currentuser.userid, uident, num);
					millionairesrec(genbuf, buf, "银行交易");
					pressanykey();
				}
				break;
			case '3':
				clear();
				money_show_stat("兵马俑银行储蓄窗口");
				move(4, 4);
				deposit_rate = mc->deposit_rate / 10000.0;
				sprintf(genbuf,
						"最小存取款金额 1000 兵马俑币。存款利率（日）为 %.2f％",
						deposit_rate * 100);
				prints_nofmt(genbuf);
				move(t_lines - 1, 0);
				prints_nofmt("\033[1;44m 选单 \033[1;46m [1]存款 [2]取款 [Q]离开\033[m");
				ch = igetkey();
				switch (ch) {
					case '1':
						getdata(6, 4, "您要存多少兵马俑币?[0]", buf, 10, DOECHO, YEA);
						if (buf[0] == '\0') {
							break;
						}
						num = atoi(buf);
						if (num < 1000) {
							move(7, 4);
							prints_nofmt("对不起，未达到最小交易金额。");
							pressanykey();
							break;
						}
						move(7, 4);
						sprintf(genbuf, "确定存 %d 兵马俑币吗?", num);
						if (askyn(genbuf, NA, NA) == NA) {
							break;
						}
						money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
						credit = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);
						if (money < num) {
							move(8, 4);
							prints_nofmt("您没有这么多钱可以存。");
							pressanykey();
							break;
						}
						if (credit + num > MAX_MONEY_NUM) {
							move(8, 4);
							prints_nofmt("空守着这么多存款做什么呢？");
							pressanykey();
							break;
						}
						/* 扣钱 */
						saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
						/* 计算原先存款的利息 */
						saveValue(currentuser.userid, INTEREST_NAME, makeInterest(credit, "deposit_time", deposit_rate), MAX_MONEY_NUM);
						/* 存款 */
						saveValue(currentuser.userid, CREDIT_NAME, num, MAX_MONEY_NUM);
						saveValue(currentuser.userid, "deposit_time", -2000000000, 2000000000);
						/* 新的存款开始时间 */
						saveValue(currentuser.userid, "deposit_time", time(0), 2000000000);
						move(8, 4);
						prints("交易成功，您现在存有 %d 兵马俑币，利息共计 %d 兵马俑币。",
								loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM),
								loadValue(currentuser.userid, INTEREST_NAME, MAX_MONEY_NUM));
						if (num >= RUMOR_MONEY && random() % 2) {
							sprintf(genbuf,
									"有人目击 %s 在兵马俑银行存入了 %d 的兵马俑币！",
									currentuser.userid,
									makeRumor(num));
							deliverreport("[谣言]来自兵马俑银行的消息", genbuf);
						}
						pressanykey();
						break;
					case '2':
						getdata(6, 4, "您要取多少兵马俑币?[0]", buf, 10, DOECHO, YEA);
						if (buf[0] == '\0') {
							break;
						}
						num = atoi(buf);
						if (num < 1000) {
							move(7, 4);
							prints_nofmt("对不起，未达到最小交易金额。");
							pressanykey();
							break;
						}
						move(7, 4);
						sprintf(genbuf, "确定取 %d 兵马俑币吗?", num);
						if (askyn(genbuf, NA, NA) == NA) {
							break;
						}
						credit = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);
						if (num > credit) {
							move(8, 4);
							prints_nofmt("您没有那么多存款。");
							pressanykey();
							break;
						}
						withdrawAll = 0;
						total_num = num;
						if (num == credit) {
							move(8, 4);
							sprintf(genbuf,	"是否一并取出 %ld 兵马俑币的存款利息？",
									loadValue(currentuser.userid, INTEREST_NAME, MAX_MONEY_NUM)
									+ makeInterest(num, "deposit_time", deposit_rate));
							if (askyn(genbuf, NA, NA) == YEA) {
								/* 存款加利息 */
								total_num = num
									+ makeInterest(num, "deposit_time", deposit_rate)
									+ loadValue(currentuser.userid, INTEREST_NAME, MAX_MONEY_NUM);
								withdrawAll = 1;
							}
						}

						credit = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);
						if (num > credit) {
							move(9, 4);
							prints_nofmt("您没有那么多存款。");
							pressanykey();
							break;
						}
						/* 减去取款 */
						saveValue(currentuser.userid, CREDIT_NAME, -num, MAX_MONEY_NUM);
						/* 取得现金 */
						saveValue(currentuser.userid, MONEY_NAME, total_num, MAX_MONEY_NUM);
						/* 计算所取的钱的利息 */
						if (withdrawAll) {
							saveValue(currentuser.userid, INTEREST_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
						} else {
							saveValue(currentuser.userid, INTEREST_NAME, makeInterest(num, "deposit_time", deposit_rate), MAX_MONEY_NUM);
						}
						move(8, 4);
						prints("交易成功，您现在存有 %d 兵马俑币，存款利息共计 %d 兵马俑币。",
							loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM),
							loadValue(currentuser.userid, INTEREST_NAME, MAX_MONEY_NUM));
						pressanykey();
						break;
					case 'Q':
					case 'q':
						break;
				}
				break;
			case '4':
				clear();
				money_show_stat("兵马俑银行贷款窗口");
				move(4, 4);
				lend_rate = mc->lend_rate / 10000.0;
				sprintf(genbuf,
						"最小交易金额 1000 兵马俑币。贷款利率（日）为 %.2f％",
						lend_rate * 100);
				prints_nofmt(genbuf);
				move(5, 4);
				lendMoney = loadValue(currentuser.userid, LEND_NAME, MAX_MONEY_NUM);
				total_num = lendMoney + makeInterest(lendMoney, "lend_time", lend_rate);
				lendTime = loadValue(currentuser.userid, "lend_time", 2000000000);
				if (lendTime > 0) {
					sprintf(genbuf,
							"您贷款 %d 兵马俑币，当前本息共计 %d 兵马俑币，距到期 %ld 小时。",
							lendMoney, total_num,
							(loadValue(currentuser.userid, "back_time", 2000000000) - time(0)) / 3600);
				} else {
					sprintf(genbuf, "您目前没有贷款。");
				}
				prints_nofmt(genbuf);
				move(t_lines - 1, 0);
				prints_nofmt("\033[1;44m 选单 \033[1;46m [1]贷款 [2]还贷 [Q]离开\033[m");
				ch = igetkey();
				switch (ch) {
					case '1':
						move(6, 4);
						sprintf(genbuf,
								"按照银行的规定，您目前最多可以申请贷款 %d 兵马俑币。",
								countexp(&currentuser) * 100);
						prints_nofmt(genbuf);
						getdata(7, 4, "您要贷款多少兵马俑币?[0]", buf,
								10, DOECHO, YEA);
						if (buf[0] == '\0') {
							break;
						}
						num = atoi(buf);

						if (lendMoney > 0) {
							move(8, 4);
							prints_nofmt("请先还清贷款。");
							pressanykey();
							break;
						}
						if (num < 1000) {
							move(8, 4);
							prints_nofmt("对不起，未达到最小交易金额。");
							pressanykey();
							break;
						}
						if (num > countexp(&currentuser) * 100) {
							move(8, 4);
							prints_nofmt("对不起，您要求贷款的金额超过银行规定。");
							pressanykey();
							break;
						}
						inputValid = 0;
						while (!inputValid) {
							getdata(8, 4, "您要贷款多少天？[3-30]: ", buf, 3, DOECHO, YEA);
							if (atoi(buf) > 2 && atoi(buf) < 31) {
								inputValid = 1;
							}
						}
						saveValue(currentuser.userid, MONEY_NAME, num, MAX_MONEY_NUM);
						saveValue(currentuser.userid, LEND_NAME, num, MAX_MONEY_NUM);
						saveValue(currentuser.userid, "lend_time", time(0), 2000000000);
						saveValue(currentuser.userid, "back_time", time(0) + atoi(buf) * 86400, 2000000000);
						move(9, 4);
						prints_nofmt("您的贷款手续已经完成。请到期还款。");
						sprintf(genbuf, "%s进行银行交易(贷款)", currentuser.userid);
						sprintf(buf, "%s贷款%d兵马俑币%d天", currentuser.userid, num, atoi(buf));
						millionairesrec(genbuf, buf, "银行交易");
						pressanykey();
						break;
					case '2':
						move(6, 4);
						backTime = loadValue(currentuser.userid, "back_time", 2000000000);
						if((backTime - time(0)) / 3600>5000||(backTime - time(0)) / 3600<-30)
							saveValue(currentuser.userid, "back_time", time(0) + 1* 86400, 2000000000);
						lendTime = loadValue(currentuser.userid, "lend_time", 2000000000);
						if (lendTime == 0) {
							prints_nofmt("您记错了吧？没有找到您的贷款记录啊。");
							pressanykey();
							break;
						}
						if (time(0) - lendTime < 86400) {
							prints_nofmt("对不起，银行不接受未产生利息的还贷。");
							pressanykey();
							break;
						}
						if (askyn("您要现在偿还贷款吗？", NA, NA) == YEA) {
							money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
							move(7, 4);
							if (money < total_num) {
								prints_nofmt ("对不起，您的钱不够偿还贷款。");
								pressanykey();
								break;
							}
							saveValue(currentuser.userid, MONEY_NAME, -total_num, MAX_MONEY_NUM);
							saveValue(currentuser.userid, LEND_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
							saveValue(currentuser.userid, "lend_time", -2000000000, 2000000000);
							saveValue(currentuser.userid, "back_time", -2000000000, 2000000000);
							ytht_del_from_file(DIR_MC "special_lend", currentuser.userid, true);
							sprintf(genbuf, "%s进行银行交易(还贷)", currentuser.userid);
							sprintf(buf,"%s偿还贷款本利共%d兵马俑币", currentuser.userid, total_num);
							millionairesrec(genbuf, buf, "银行交易");
							prints_nofmt("您的贷款已经还清。银行乐见并铭记您的诚信。");
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
				money_show_stat("兵马俑银行工资代办窗口");
				salary = makeSalary();
				if (salary == 0) {
					move(10, 10);
					prints_nofmt("对不起，您不是本站公务员，没有工资。");
					pressanykey();
					break;
				}
				if (mc->isSalaryTime == 0) {
					move(10, 10);
					prints_nofmt("对不起，银行还没有收到工资划款。请过几天再来。");
					pressanykey();
					break;
				}
				if (seek_in_file(DIR_MC "salary_list", currentuser.userid)) {
					move(10, 10);
					prints_nofmt("您已经领过本月工资。还是勤奋工作吧！");
					pressanykey();
					break;
				}
				move(6, 4);
				sprintf(genbuf, "您本月的工资 %d 兵马俑币已经划到银行。现在领取吗？",	salary);
				if (askyn(genbuf, NA, NA) == YEA) {
					saveValue(currentuser.userid, MONEY_NAME, salary, MAX_MONEY_NUM);
					ytht_add_to_file(DIR_MC "salary_list", currentuser.userid);
					move(8, 4);
					prints_nofmt("这里是您的工资。感谢您为兵马俑付出的工作!");
					pressanykey();
					break;
				}
				break;
			case '6':
				clear();
				money_show_stat("兵马俑银行行长办公室");
				move(6, 4);
				char name[20];
				whoTakeCharge2(1, name);
				whoTakeCharge(1, uident);
				if (strcmp(currentuser.userid, uident)) {
					prints("秘书%s拦住了你，柔声说道:“行长%s正在里面办公，请勿打扰。”", name,uident);
					pressanykey();
					break;
				} else {
					prints_nofmt("请选择操作代号:");
					move(7, 6);
					prints_nofmt("1. 调整存款利率                  2. 调整贷款利率");
					move(8, 6);
					prints_nofmt("3. 调整转帐费率                  4. 审批贷款");
					move(9, 6);
					prints_nofmt("5. 调查用户帐户                  6. 特别贷款名单");
					move(10, 6);
					prints_nofmt("7. 发放工资                      8. 特殊拨款");
					move(11,6);
					prints_nofmt("9. 辞职                          Q. 退出");
					ch = igetkey();
					switch (ch) {
						case '1':
							getdata(12, 4, "设定新的存款利率[10-250]: ", buf, 4, DOECHO, YEA);
							if (atoi(buf) < 10 || atoi(buf) > 250) {
								break;
							}
							move(14, 4);
							sprintf(genbuf, "新的存款利率是 %.2f％，确定吗？", atoi(buf) / 100.0);
							if (askyn(genbuf, NA, NA) == YEA) {
								savestrvalue(MC_RATE_FILE, "deposit_rate", buf);
								mc->deposit_rate = atoi(buf);
								move(15, 4);
								prints_nofmt("设置完毕。");
								sprintf(genbuf, "新的存款利率为 %.2f％ 。", mc->deposit_rate / 100.0);
								deliverreport("[公告]兵马俑银行调整存款利率", genbuf);
								sprintf(genbuf, "%s行使管理权限",currentuser.userid);
								sprintf(buf, "设置新的存款利率为 %.2f％ 。", mc->deposit_rate / 100.0);
								millionairesrec(genbuf, buf, "");
								pressanykey();
							}
							break;
						case '2':
							getdata(12, 4, "设定新的贷款利率[10-250]: ", buf, 4, DOECHO, YEA);
							if (atoi(buf) < 10 || atoi(buf) > 250) {
								break;
							}
							move(14, 4);
							sprintf(genbuf, "新的贷款利率是 %.2f％，确定吗？", atoi(buf) / 100.0);
							if (askyn(genbuf, NA, NA) == YEA) {
								savestrvalue(MC_RATE_FILE, "lend_rate", buf);
								mc->lend_rate = atoi(buf);
								move(15, 4);
								prints_nofmt("设置完毕。");
								sprintf(genbuf, "新的贷款利率为 %.2f％ 。", mc->lend_rate / 100.0);
								deliverreport("[公告]兵马俑银行调整贷款利率", genbuf);
								sprintf(genbuf, "%s行使银行管理权限",currentuser.userid);
								sprintf(buf, "设置新的贷款利率为 %.2f％ 。", mc->lend_rate / 100.0);
								millionairesrec(genbuf, buf, "");
								pressanykey();
							}
							break;
						case '3':
							getdata(12, 4, "设定新的转帐费率[10-250]: ", buf, 4, DOECHO, YEA);
							if (atoi(buf) < 10 || atoi(buf) > 250) {
								break;
							}
							move(14, 4);
							sprintf(genbuf, "新的转帐费率是 %.2f％，确定吗？", atoi(buf) / 100.0);
							if (askyn(genbuf, NA, NA) == YEA) {
								savestrvalue(MC_RATE_FILE, "transfer_rate", buf);
								mc->transfer_rate = atoi(buf);
								move(15, 4);
								prints_nofmt("设置完毕。");
								sprintf(genbuf, "新的转帐费率为 %.2f％ 。", mc->transfer_rate / 100.0);
								deliverreport("[公告]兵马俑银行调整转帐费率", genbuf);
								sprintf(genbuf, "%s行使银行管理权限",currentuser.userid);
								sprintf(buf, "设置新的转帐费率为 %.2f％ 。", mc->transfer_rate / 100.0);
								millionairesrec(genbuf, buf, "");
								pressanykey();
							}
							break;
						case '4':
							move(12, 4);
							usercomplete("向谁提供特别贷款？", uident);
							if (uident[0] == '\0')
								break;
							if (!getuser(uident)) {
								move(13, 4);
								prints_nofmt("错误的使用者代号...");
								pressreturn();
								break;
							}
							if (lookupuser.dietime > 0) {
								move(13, 4);
								prints_nofmt("这个是死人...");
								pressreturn();
								break;
							}
							if (loadValue(uident, "lend_time", 2000000000) > 0) {
								move(13, 4);
								prints_nofmt("该客户已经贷款，不宜追加贷款。");
								pressanykey();
								break;
							}
							getdata(13, 4, "贷款金额[0]: ", buf, 10, DOECHO, YEA);
							if (buf[0] == '\0')
								break;

							if (atoi(buf) < 100000) {
								move(14, 4);
								prints_nofmt("这么点钱，营业厅就可以办理。");
								pressanykey();
								break;
							}
							if (atoi(buf) > 100000000) {
								move(14, 4);
								prints_nofmt("如此数额巨大的贷款，恐怕董事会不会同意的。");
								pressanykey();
								break;
							}
							num = atoi(buf);
							getdata(14, 4, "贷款天数[3-30]: ", buf, 3, DOECHO, YEA);
							if (atoi(buf) < 1 || atoi(buf) > 30)
								break;
							move(15, 4);
							if (askyn("确定向该客户提供贷款吗？", NA, NA) == NA)
								break;
							time_t t = time(0) + 86400 * atoi(buf);
							char local_buf[STRLEN * 2], local_buf_lg[STRLEN * 4];
							sprintf(local_buf, "%s\t%s", uident, ctime(&t));
							ytht_add_to_file(DIR_MC "special_lend", genbuf);
							saveValue(uident, MONEY_NAME, num, MAX_MONEY_NUM);
							saveValue(uident, LEND_NAME, num, MAX_MONEY_NUM);
							saveValue(uident, "lend_time", time(0), 2000000000);
							saveValue(uident, "back_time", time(0) + atoi(buf) * 86400, 2000000000);
							sprintf(local_buf, "贷款金额 %d 兵马俑币，请务必于 %s 天内偿还贷款。", num, buf);
							mail_buf(local_buf, uident, "兵马俑银行行长同意了您的贷款申请");
							move(16, 4);
							prints_nofmt("贷款审批完成。请确保客户及时还款。");
							sprintf(local_buf_lg, "给%s特别贷款，%s",uident,local_buf);
							sprintf(local_buf, "%s行使银行管理权限",currentuser.userid);
							millionairesrec(local_buf, local_buf_lg, "");
							pressanykey();
							break;
						case '5':
							move(12, 4);
							usercomplete("调查谁的帐户：", uident);
							if (uident[0] == '\0')
								break;
							if (!getuser(uident)) {
								move(13, 4);
								prints_nofmt("错误的使用者代号...");
								pressreturn();
								break;
							}
							move(14, 4);
							sprintf(genbuf,
									"该客户有现金%ld 兵马俑币，存款 %ld 兵马俑币,贷款 %ld 兵马俑币。",
									loadValue(uident, MONEY_NAME, MAX_MONEY_NUM),
									loadValue(uident, CREDIT_NAME, MAX_MONEY_NUM),
									loadValue(uident, LEND_NAME, MAX_MONEY_NUM));
							prints_nofmt(genbuf);
							pressanykey();
							break;
						case '6':
							clear();
							move(1, 0);
							prints_nofmt("目前银行的特别贷款情况：");
							move(2, 0);
							prints_nofmt("客户ID\t还款时间");
							listfilecontent(DIR_MC "special_lend");
							pressanykey();
							break;
						case '7':
							move(12, 4);
							if (newSalary()) {
								if (askyn("确定发放本月工资吗？", NA, NA) == YEA) {
									time2string(time(0), genbuf);
									if (savestrvalue(DIR_MC "etc_time", "salary_time", genbuf)){
										move(14, 4);
										prints_nofmt("错误!不能写文件!");
										pressanykey();
										break;
									}
									strcpy(currboard, "sysop");
									deliverreport("[公告]本站公务员领取本月工资", "请于7天内到兵马俑银行领取，过期视为放弃。\n");
									strcpy(currboard, MC_BOARD);
									remove(DIR_MC "salary_list");
									mc->isSalaryTime = 1;
									move(14, 4);
									prints_nofmt("操作完成。");
									sprintf(genbuf, "%s行使银行管理权限", currentuser.userid);
									millionairesrec(genbuf, "发放工资", "");
									pressanykey();
								}
							} else {
								prints_nofmt("还未到发放时间。");
								pressanykey();
							}
							break;
						case '8':
							move(12, 4);
							usercomplete("向谁提供特别拨款？", uident);
							if (uident[0] == '\0')
								break;
							if (!getuser(uident)) {
								move(13, 4);
								prints_nofmt("错误的使用者代号...");
								pressreturn();
								break;
							}
							if (lookupuser.dietime > 0) {
								move(13, 4);
								prints_nofmt("这个是死人...");
								pressreturn();
								break;
							}
							getdata(13, 4,"拨款金额[0]: ", buf, 10, DOECHO, YEA);
							if (buf[0] == '\0') {
								break;
							}
							if (atoi(buf) < 100000) {
								move(14, 4);
								prints_nofmt("这么点钱，营业厅就可以办理。");
								pressanykey();
								break;
							}
							if (atoi(buf) > 100000000) {
								move(14, 4);
								prints_nofmt("如此数额巨大的贷款，恐怕董事会不会同意的。");
								pressanykey();
								break;
							}
							num = atoi(buf);
							getdata(15, 4, "拨款原因：", buf, 50, DOECHO, YEA);
							sprintf(letter, "拨款用于自身的建设发展，望其按照规定使用，不得进行违法乱纪活动！\n\n拨款原因：%s", buf);
							move(16, 4);
							if (askyn("确定向该客户拨款吗？", NA, NA) == NA)
								break;
							saveValue(uident, MONEY_NAME, num, MAX_MONEY_NUM);
							sprintf(genbuf,"授予%s %d兵马俑币援助拨款",uident, num);
							deliverreport(genbuf, letter);
							mail_buf(genbuf, uident, "兵马俑银行行长同意了您的拨款申请");
							move(17, 4);
							prints_nofmt("拨款完成。");
							sprintf(buf, "给%s特别拨款%d兵马俑币",uident, num);
							sprintf(genbuf, "%s行使银行管理权限", currentuser.userid);
							millionairesrec(genbuf, buf, "");
							pressanykey();
							break;
						case '9':
							move(12, 4);
							if (askyn("您真的要辞职吗？", NA, NA) == YEA) {
								/*	ytht_del_from_file(MC_BOSS_FILE, "bank");
									sprintf(genbuf,
									"%s 宣布辞去兵马俑银行行长职务",
									currentuser.userid);
									deliverreport(genbuf,
									"兵马俑金融中心对其一直以来的工作表示感谢，祝以后顺利！");
									move(14, 4);
									prints
									("好吧，既然你意已决，董事会也不便强留。再见！");
									quit = 1;
									*/
								sprintf(genbuf, "%s 要辞去兵马俑银行行长职务", currentuser.userid);
								mail_buf(genbuf, "millionaires", genbuf);
								move(14, 4);
								prints_nofmt("好吧，已经发信告知总管了");
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

//彩票系统
static int money_lottery() {
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
		nomoney_show_stat("兵马俑彩票中心");
		move(6, 4);
		prints_nofmt("彩票中心隆重开张，欢迎大家踊跃购买彩票～～");
		move(8, 4);
		prints_nofmt("彩票规则请到millionaires版查询。");
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1]36选7 [2]足彩 [3]经理室 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
			case '1':
				nomoney_show_stat("36选7");
				if (access(DIR_MC_TEMP "36_7_start", 0)) {
					move(6, 4);
					prints_nofmt("抱歉！新一期的36选7彩票还未开始销售。");
					pressanykey();
					break;
				}
				move(5, 4);
				prints_nofmt("数字间用-隔开，例如 08-13-01-25-34-17-18");
				move(7, 4);
				sprintf(genbuf,
						"当前奖金池累积奖金：\033[1;31m%d\033[m   固定奖金：\033[1;31m%d\033[m",
						mc->prize367, PRIZE_PER);
				prints_nofmt(genbuf);
				move(9, 4);
				sprintf(genbuf, "每注 10000 兵马俑币。确定买注吗?");
				if (askyn(genbuf, NA, NA) == YEA) {
					money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
					move(10, 4);
					if (money < 10000) {
						prints_nofmt("没有钱就别捣乱，一边去！下一个！");
						pressanykey();
						break;
					}

					saveValue(currentuser.userid, MONEY_NAME, -10000, MAX_MONEY_NUM);	//扣钱
					mc->prize367 += 10000;
					mc->prize367 = limitValue(mc->prize367, MAX_POOL_MONEY);
					inputValid = 0;
					while (!inputValid) {
						getdata(10, 4, "请填写买注单: ", buf, 21, DOECHO, YEA);	/* 2×7＋6＋1＝21 */

						if (!valid367Bet(buf)) {	// 检验下注的合理性
							move(11, 4);
							prints_nofmt("对不起，您的下注单填写好像有问题耶。请重填一次。");
							pressanykey();
						} else {
							sprintf(genbuf, "%s %s", currentuser.userid, buf);
							ytht_add_to_file(DIR_MC_TEMP "36_7_list", genbuf);
							move(11, 4);
							prints_nofmt("                                                             ");
							move(11, 4);
							prints_nofmt("购买成功。祝您中大奖！");
							inputValid = 1;
							sprintf(letter, "您购买了一注36选7。注号是：%s。请妥善保存，到期兑奖。", buf);
							sprintf(title, "彩票中心购买凭证");
							mail_buf(letter, currentuser.userid, title);
							pressanykey();
						}

					}

				}
				break;
			case '2':
				nomoney_show_stat("足彩");
				move(6, 4);
				if (access(DIR_MC_TEMP "soccer_start", 0)) {
					prints_nofmt("抱歉！新一期的足球彩票还未开始销售。");
					pressanykey();
					break;
				}
				if (mc->isSoccerSelling == 0) {
					prints_nofmt("抱歉！销售期已经结束，请等待开奖。");
					pressanykey();
					break;
				}
				move(4, 4);
				prints_nofmt("足彩所猜比赛请见millionaires版公告文章。");
				move(5, 4);
				prints_nofmt("主场胜为3，主场平为1，主场负为0。各场比赛结果用-隔开，支持复式买注。");
				move(6, 4);
				prints_nofmt("例如猜6场比赛时，一个接受的买注范例为： 1-310-1-10-3-0");
				move(8, 4);
				sprintf(genbuf,
						"当前奖金池累计奖金：\033[1;31m%d\033[m  固定奖金：\033[1;31m%d\033[m",
						mc->prizeSoccer, PRIZE_PER);
				prints_nofmt(genbuf);
				move(10, 4);
				sprintf(genbuf, "每注10000兵马俑币。确定买注吗?");
				if (askyn(genbuf, NA, NA) == YEA) {
					money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
					move(11, 4);
					if (money < 10000) {
						prints_nofmt("没有钱就别捣乱，一边去！下一个！");
						pressanykey();
						break;
					}

					inputValid = 0;
					while (!inputValid) {
						int sum;
						getdata(11, 4, "请填写买注单: ", buf, 55, DOECHO, YEA);
						if (!validSoccerBet(buf)) {	/* 检验下注的合理性 */
							move(12, 4);
							prints_nofmt("对不起，您的下注单填写好像有问题耶。请重填一次。");
							pressanykey();
						} else {
							int money;
							inputValid = 1;
							sum = computeSum(buf);	/*计算复式买注的注数 */
							money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
							if (sum > money / 10000) {
								move(12, 4);
								prints_nofmt("                                                     ");
								move(12, 4);
								sprintf(genbuf, "您的钱不够买 %d 注。再斟酌一下吧！", sum);
								prints_nofmt(genbuf);
								pressanykey();
								break;
							}
							saveValue(currentuser.userid, MONEY_NAME, -sum * 10000, MAX_MONEY_NUM);	/*扣钱 */
							mc->prizeSoccer += sum * 10000;
							mc->prizeSoccer = limitValue(mc->prizeSoccer, MAX_POOL_MONEY);
							saveSoccerRecord(buf);	/*处理并保存复式买注记录 */
							move(12, 4);
							prints_nofmt("                                                             ");
							move(12, 4);
							sprintf(genbuf,"您一共购买了%d注。祝您中大奖！", sum);
							prints_nofmt(genbuf);
							sprintf(letter, "您购买了一注(复式)足彩。注号是：%s。请妥善保存，到期兑奖。", buf);
							sprintf(title, "彩票中心购买凭证");
							mail_buf(letter, currentuser.userid, title);
							pressanykey();
						}
					}
				}
				break;
			case '3':
				nomoney_show_stat("博彩公司经理室");
				whoTakeCharge(2, uident);
				whoTakeCharge2(2, name);
				if (strcmp(currentuser.userid, uident)) {
					move(6, 4);
					prints("秘书%s提示您:“经理%s外出考察去了，有事请直接跟他联系。”", name, uident);
					pressanykey();
					break;
				}
				quitRoom = 0;
				while (!quitRoom) {
					char strTime[15];
					nomoney_show_stat("博彩公司经理室");
					move(t_lines - 1, 0);
					prints_nofmt("\033[1;44m 选单 \033[1;46m [1]开奖 [2]新建 [3]停止足彩销售 [4]辞职 [Q]离开\033[m");
					ch = igetkey();
					switch (ch) {
						case '1':
							nomoney_show_stat("博彩公司经理室");
							move(6, 10);
							prints_nofmt("1.  36选7");
							move(7, 10);
							prints_nofmt("2.  足球彩票");
							move(8, 10);
							prints_nofmt("Q.  退出");
							move(4, 4);
							prints_nofmt("请选择要开奖的彩票代号：");
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
												prints_nofmt("开奖成功！");
												sprintf(genbuf, "%s行使彩票管理权限",currentuser.userid);
												millionairesrec(genbuf, "36选7开奖", "");
											}
											else prints_nofmt("发生意外错误...");
										else
											prints_nofmt("开奖时间还没有到啊！");

									} else
										prints_nofmt("没有该彩票的记录。");
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
											getdata(t_lines - 5, 4, "请输入兑奖序列(无需 - )[按\033[1;33mENTER\033[m放弃]: ", buf, 55, DOECHO, YEA);
											if (strlen(buf) == 0)
												break;
											if (open_soccer(buf) ==0)	{
												prints_nofmt("开奖成功！");
												sprintf(genbuf, "%s行使彩票管理权限",currentuser.userid);
												millionairesrec(genbuf, "足彩开奖", "");
											}
											else prints_nofmt("发生意外错误...");

										} else
											prints_nofmt ("开奖时间还没有到啊！");
									} else
										prints_nofmt ("没有该彩票的记录。");
									pressanykey();
									break;
								case 'q':
								case 'Q':
									break;
							}
							break;
						case '2':
							nomoney_show_stat("博彩公司经理室");
							move(6, 10);
							prints_nofmt("1. 36选7 ");
							move(7, 10);
							prints_nofmt("2. 足彩");
							move(8, 10);
							prints_nofmt("Q. 退出");
							move(4, 4);
							prints_nofmt("请选择开奖种类或操作：");
							ch = igetkey();
							switch (ch) {
								case '1':
									nomoney_show_stat("博彩公司经理室");
									move(4, 4);
									if (!access(DIR_MC_TEMP "36_7_start",0)) {
										prints_nofmt("36选7彩票销售正在火热进行。");
										pressanykey();
										break;
									}
									prints_nofmt("新建36选7");
									inputValid = 0;
									while (!inputValid) {
										getdata(8, 4,"彩票销售天数[1-7]: ",buf, 2, DOECHO,YEA);
										if (buf[0] > '0' && buf[0] < '8')
											inputValid = 1;
									}
									time2string(time(0) + (buf[0] - '0') * 86400, genbuf);
									ytht_add_to_file(DIR_MC_TEMP "36_7_start", genbuf);

									sprintf(genbuf, "本期彩票将于 %s 天后开奖。欢迎大家踊跃购买！", buf);
									deliverreport("[公告]新一期36选7彩票开始销售", genbuf);

									move(10, 4);
									prints_nofmt("建立成功！请到时开奖。");
									sprintf(genbuf, "新建36选7，%s天后开奖。",buf);
									sprintf(buf, "%s行使彩票管理权限",currentuser.userid);
									millionairesrec(buf, genbuf, "");
									pressanykey();
									break;
								case '2':
									nomoney_show_stat("博彩公司经理室");
									move(4, 4);
									if (!access(DIR_MC_TEMP "soccer_start",0)) {
										prints_nofmt("足球彩票销售正在火热进行。");
										pressanykey();
										break;
									}
									prints_nofmt("新建足彩");
									inputValid = 0;
									while (!inputValid) {
										getdata(8, 4,"彩票销售天数[1-7]: ",buf, 2, DOECHO,YEA);
										if (buf[0] > '0' && buf[0] < '8')
											inputValid = 1;
									}
									time2string(time(0) +(buf[0] - '0') * 86400, genbuf);
									ytht_add_to_file(DIR_MC_TEMP "soccer_start", genbuf);
									mc->isSoccerSelling = 1;
									sprintf(genbuf, "本期彩票将于 %s 天后开奖。欢迎大家踊跃购买！", buf);
									deliverreport("[公告]新一期足球彩票开始销售", genbuf);

									move(10, 4);
									prints_nofmt("建立成功！请到时开奖。");
									sprintf(genbuf, "新建足彩，%s天后开奖。",buf);
									sprintf(buf, "%s行使彩票管理权限",currentuser.userid);
									millionairesrec(buf, genbuf, "");
									pressanykey();
									break;
								case 'q':
								case 'Q':
									break;
							}
							break;

						case '3':
							nomoney_show_stat("博彩公司经理室");
							move(6, 4);
							if (askyn("您真的要停售足彩吗？", NA, NA) == YEA) {
								mc->isSoccerSelling = 0;
								deliverreport("[公告]本期足球彩票停止销售", "请广大彩民耐心等待开奖！");
								sprintf(buf, "%s行使彩票管理权限", currentuser.userid);
								millionairesrec(buf, "停售本期足彩", "");
								move(8, 4);
								prints_nofmt("已经停售！请到时开奖。");
								pressanykey();
							}
							break;

						case '4':
							nomoney_show_stat("博彩公司经理室");
							move(6, 4);
							if (askyn("您真的要辞职吗？", NA, NA) == YEA) {
								/*	ytht_del_from_file(MC_BOSS_FILE, "lottery");
									sprintf(genbuf,
									"%s 宣布辞去兵马俑博彩公司经理职务",
									currentuser.userid);
									deliverreport(genbuf,
									"兵马俑金融中心对其一直以来的工作表示感谢，祝以后顺利！");
									move(8, 4);
									prints
									("好吧，既然你意已决，董事会也不便强留。再见！");
									pressanykey();
									quitRoom = 1;
									*/
								sprintf(genbuf, "%s 要辞去兵马俑博彩公司经理职务", currentuser.userid);
								mail_buf(genbuf, "millionaires", genbuf);
								move(8, 4);
								prints_nofmt("好吧，已经发信告知总管了");
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

static int addOrDel_contrb() {
	char uident[IDLEN + 1], ans[8];
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

		prints_nofmt("设定进入捐款名单的基金: \n");
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
			getdata(1, 0, "(A)增加 (D)删除 (C)改变 (E)离开 [E]: ", ans, 2, DOECHO, YEA);
		else
			getdata(1, 0, "(A)增加 (E)离开 [E]: ", ans, 2, DOECHO, YEA);

		tag = 0;
		if (*ans == 'A' || *ans == 'a') {
			move(1, 0);
			while (1){
				move(1, 0);
				clrtoeol();
				usercomplete("增加id：", uident);
				if (*uident == '\0')
					break;
				if (!getuser(uident)) {
					showAt(2, 0, "该id不存在", 1);
					tag = -1;
					break;
				}
				if (!seek_in_file(DIR_MC "jijin", uident)) {
					showAt(2, 0, "该id不是基金!", 1);
					tag = -1;
					break;
				}
				for(i = 0; i<count ;i++){
					if (!strcmp(JijinMem[i].userid, uident)){
						showAt(2, 0, "该id已经存在", 1);
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
				ytht_strsncpy(JijinTmp.userid, uident, sizeof(JijinTmp.userid));
				while (buf[0] == 0)
					getdata(2, 0, "请输入基金名称: ", buf, 18, DOECHO, YEA);
				ytht_strsncpy(JijinTmp.name, buf, sizeof(JijinTmp.name));
				append_record(MC_JIJIN_CTRL_FILE, &JijinTmp, sizeof(struct MC_Jijin));
				sprintf(title, "%s行使管理权限(设置捐款基金)", currentuser.userid);
				sprintf(buf,"%s把%s添加为 %s基金", currentuser.userid, JijinTmp.userid, JijinTmp.name);
				millionairesrec(title,buf, "");
			}
		} else if ((*ans == 'C' || *ans == 'c')) {
			move(1, 0);
			usercomplete("改变哪个id: ", uident);
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
					getdata(2, 0, "请输入新的名称: ", buf, 18, DOECHO, YEA);
				ytht_strsncpy(JijinMem[i].name, buf, sizeof JijinMem[i].name);
				sprintf(title, "%s行使管理权限(设置捐款基金)", currentuser.userid);
				sprintf(buf,"%s改变%s的名称为 %s基金", currentuser.userid, JijinMem[i].userid, JijinMem[i].name);
				millionairesrec(title,buf, "");
				saveData(JijinMem, filesize);
			} else
				showAt(2, 0, "您输入的id不在列表中", 1);
		} else if ((*ans == 'D' || *ans == 'd') && count) {
			move(1, 0);
			usercomplete("删除id: ", uident);
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
					showAt(2, 0, "发生意外错误!", 1);
					return -1;
				}
				sprintf(title, "%s行使管理权限(设置捐款基金)", currentuser.userid);
				sprintf(buf,"%s删除%s(%s基金)", currentuser.userid, JijinMem[i].userid, JijinMem[i].name);
				millionairesrec(title,buf, "");
				for (i = 0; i < x; i++)
					fwrite(&JijinMem[i], sizeof(struct MC_Jijin), 1, fpw);
				for (i = x+1; i < count; i++)
					fwrite(&JijinMem[i], sizeof(struct MC_Jijin), 1, fpw);
				fclose(fpw);
				unlink(MC_JIJIN_CTRL_FILE);
				rename(MC_JIJIN_CTRL_FILE".tmp", MC_JIJIN_CTRL_FILE);
				showAt(2, 0, "删除成功", 1);
			}else
				showAt(2, 0, "您输入的id不在列表中", 1);
		} else
			break;
	}
	clear();
	return 1;
}

//type1职位 2秘书
static int money_sackOrAppoint(int type) {
	int pos, i=0 , j;
	char buf[100], letter[100], report[100], uident[IDLEN + 1], boss[IDLEN + 1];
	const char feaStr[][20] = {
		"bank", "lottery", "gambling", "gang", "beggar", "stock", "shop",
		"police","killer","marriage","office",""
	};
	const char feaStr2[][20] = {
		"银行", "彩票", "赌场", "黑帮", "丐帮", "股市", "商场",
		"警署","杀手","教堂","中心", ""
	};

	clear();
	if (type==1)
		showAt(2, 4, "目前兵马俑金融中心各职位情况：", 0);
	if (type==2)
		showAt(2, 4, "目前兵马俑金融中心各秘书职位情况：", 0);

	while (feaStr[i][0]){
		if (type == 1)
			whoTakeCharge(i+1, boss);
		else
			whoTakeCharge2(i+1, boss);
		sprintf(buf, "%d.%s: %s", i+1, feaStr2[i], boss);
		showAt(i+5, 4, buf, 0);
		i++;
	}

	getdata(16, 4, "请选择职务? ", buf, 3, DOECHO, YEA);
	pos = atoi(buf);
	if (pos > 11 || pos < 1)
		return 0;

	getdata(16, 4, "请选择:  1.任命  2.免职? ", buf, 2, DOECHO, YEA);
	j = atoi(buf);
	if (j > 2 || j < 1)
		return 0;

	if (type == 1)
		whoTakeCharge(pos, boss);
	else
		whoTakeCharge2(pos, boss);
	if (j==1){
		if (boss[0] != '\0') {	//如果该职位非空
			prints("%s已经负责该职位。", boss);
			pressanykey();
			return 0;
		}
		move(16, 4);
		usercomplete("任命谁？", uident);
		move(17, 4);
		if (uident[0] == '\0')
			return 0;
		if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
			prints_nofmt("错误的使用者代号...");
			pressanykey();
			return 0;
		}
		sprintf(genbuf, "确定任命 %s 职位 %d%s 吗？", uident, pos, (type==1)?"":"秘书");
		if (askyn(genbuf, NA, NA) == YEA) {
			if (type==1){
				sackOrAppoint(pos, uident, 0, letter);
				savestrvalue(MC_BOSS_FILE, feaStr[pos - 1], uident);
			}else{
				sackOrAppoint2(pos, uident, 0, letter);
				savestrvalue(MC_ASS_FILE, feaStr[pos - 1], uident);
			}
			deliverreport(letter, "谨望其能廉洁奉公，不以权谋私利，为兵马俑金融事业的发展鞠躬尽瘁。");
			mail_buf(letter, uident, letter);
			sprintf(genbuf, "%s行使管理权限", currentuser.userid);
			sprintf(buf,"%s任命%s负责%d职位%s", currentuser.userid, uident, pos, (type==1)?"":"秘书");
			millionairesrec(genbuf, buf, "");
			move(18, 4);
			prints_nofmt("任命成功。");
			pressanykey();
		}
	}else if (j==2){
		if (boss[0] == '\0') {	//如果该职位为空
			prints_nofmt("目前并无人负责该职位。");
			pressanykey();
			return 0;
		}
		char local_buf[STRLEN];
		getdata(17, 4, "免去原因:", local_buf, 50, DOECHO, YEA);
		sprintf(report, "免去原因：%s", local_buf);
		move(17, 4);
		sprintf(genbuf, "确定免去 %s 的%s职位吗？", boss, (type==1)?"":"秘书");
		if (askyn(genbuf, NA, NA) == YEA) {
			if (type==1){
				sackOrAppoint(pos, boss, 1, letter);
				ytht_del_from_file(MC_BOSS_FILE, (char *) feaStr[pos - 1], true);
			}else{
				sackOrAppoint2(pos, boss, 1, letter);
				ytht_del_from_file(MC_ASS_FILE, (char *) feaStr[pos - 1], true);
			}
			deliverreport(letter, report);
			mail_buf(letter, boss, letter);
			sprintf(genbuf, "%s行使管理权限", currentuser.userid);
			sprintf(buf,"%s免去%s的%d%s职位", currentuser.userid, boss, pos, (type==1)?"":"秘书");
			millionairesrec(genbuf, buf, "");
			move(18, 4);
			prints_nofmt("免职成功。");
			pressanykey();
		}
	}
	return 1;
}

static int init_stock_v(struct boardmem *board, int curr_idx, va_list ap) {
	(void) curr_idx;
	const char *name = va_arg(ap, const char *);

	if (!strcmp(board->header.filename, name)) {
		board->stocknum = board->score * ((board->score > 10000) ? 2000 : 1000);

		if (board->stocknum < 50000)
			board->stocknum = 50000;
	}
	return 0;
}

//管理系统  股票系统
static int money_admin() {
	int ch, /* i, */ j, quit = 0;
	char buf[100], letter[100], uident[IDLEN + 1];
	char stockboard[STRLEN][MAX_STOCK_NUM];
	FILE *fp1;
	int count;
	int num=0;

	if (!seek_in_file(MC_ADMIN_FILE, currentuser.userid)
			&& !(currentuser.userlevel & PERM_SPECIAL5) && strcmp(currentuser.userid, "macintosh")) {
		return 0;
	}
	clear();
	while (!quit) {
		clear();
		nomoney_show_stat("兵马俑金融中心管理");
		move(5, 4);
		prints_nofmt("这里负责兵马俑金融中心的人事管理。");
		move(7, 8);
		prints_nofmt("A. 任命金融中心总管             B. 免去金融中心总管");
		move(8, 8);
		prints_nofmt("C. 列出总管名单           ");
		move(9, 8);
		prints_nofmt("E. 任免职位                     F. 任免秘书");
		move(10, 8);
		prints_nofmt("I. 设置上榜民间基金");
		move(11, 8);
		prints_nofmt("J. 任命名人堂成员               K. 取消名人堂资格");
		move(12, 8);
		prints_nofmt("L. 任命茶友                     M. 取消茶友资格");
		move(13, 8);
		prints_nofmt("N. 任命铁公鸡                   O. 取消铁公鸡");
		move(14, 8);
		prints_nofmt("Y. 任命基金id                   Z. 取消基金id");
		move(15, 8);
		prints_nofmt("R. 列出名人堂成员               S. 列出茶友名单");
		move(16, 8);
		prints_nofmt("T. 列出铁公鸡名单               U. 列出基金id名单");
		move(17, 8);
		prints_nofmt("P. 股市初始化");
		move(19, 8);
		prints_nofmt("X. 黑名单操作                   0. 开关金融中心");
		move(20, 8);
		prints_nofmt("1. 改变个人现金                 2. 改变个人存款");
		move(22, 8);
		prints_nofmt("G. 辞职                         Q. 退出");


		ch = igetkey();
		switch (ch) {
			case 'e':
			case 'E':
				money_sackOrAppoint(1);
				break;

			case 'f':
			case 'F':
				money_sackOrAppoint(2);//秘书
				break;

			case 'a':
			case 'A':
				clear();
				move(15, 4);
				usercomplete("授予谁金融中心总管权限？", uident);
				move(16, 4);
				if (uident[0] == '\0')
					break;
				if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
					prints_nofmt("错误的使用者代号...");
					pressanykey();
					break;
				}
				if (!seek_in_file(MC_ADMIN_FILE, uident)) {
					if (askyn("确定吗？", NA, NA) == NA) {
						break;
					}
					ytht_add_to_file(MC_ADMIN_FILE, uident);
					move(17, 4);
					prints_nofmt("任命成功!");
					sprintf(genbuf, "[公告]授予 %s 兵马俑金融中心管理权限", uident);
					deliverreport(genbuf, "谨望其能廉洁奉公，不以权谋私利，为兵马俑金融事业的发展鞠躬尽瘁。");
					sprintf(genbuf, "%s 由 %s 授予兵马俑金融中心管理权限", uident, currentuser.userid);
					mail_buf(genbuf, uident, genbuf);
					//add by macintosh for system record
					sprintf(genbuf, "%s行使管理权限", currentuser.userid);
					sprintf(buf,"%s任命%s为金融中心总管", currentuser.userid, uident);
					millionairesrec(genbuf, buf, "");
				} else {
					prints_nofmt("该ID已经具有金融中心管理权限");
				}
				pressanykey();
				break;
			case 'c':
			case 'C':
				clear();
				move(1, 0);
				prints_nofmt("目前具有管理权限的ID列表：");
				listfilecontent(MC_ADMIN_FILE);
				pressanykey();
				break;
			case 'b':
			case 'B':
				clear();
				move(15, 4);
				usercomplete("取消谁的金融中心总管权限？", uident);
				move(16, 4);
				if (uident[0] == '\0')
					break;
				if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
					prints_nofmt("错误的使用者代号...");
					pressanykey();
					break;
				}
				if (seek_in_file(MC_ADMIN_FILE, uident)) {
					getdata(16, 4, "取消原因：", buf, 50, DOECHO, YEA);
					move(17, 4);
					if (askyn("确定吗？", NA, NA) == NA) {
						pressanykey();
						break;
					}
					ytht_del_from_file(MC_ADMIN_FILE, uident, true);
					move(18, 4);
					prints_nofmt("取消成功!");
					char local_letter[STRLEN * 2];
					sprintf(genbuf, "[公告]取消 %s 的兵马俑金融中心管理权限", uident);
					sprintf(local_letter, "取消原因： %s", buf);
					deliverreport(genbuf, local_letter);
					sprintf(genbuf, "%s 被 %s 取消兵马俑金融中心管理权限", uident, currentuser.userid);
					mail_buf(genbuf, uident, genbuf);
					sprintf(genbuf, "%s行使管理权限", currentuser.userid);
					sprintf(buf,"%s取消%s的金融中心总管权限", currentuser.userid, uident);
					millionairesrec(genbuf, buf, "");
				} else {
					prints_nofmt("该ID没有此权限。");
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
				if (askyn("您真的要辞职吗？", NA, NA) == YEA) {
					ytht_del_from_file(MC_ADMIN_FILE, currentuser.userid, true);
					sprintf(genbuf, "%s 宣布辞去兵马俑金融中心总管职务", currentuser.userid);
					deliverreport(genbuf, "兵马俑金融中心对其一直以来的工作表示感谢，祝以后顺利！");
					sprintf(genbuf, "%s行使管理权限", currentuser.userid);
					sprintf(buf,"%s辞去兵马俑金融中心总管职务", currentuser.userid);
					millionairesrec(genbuf, buf, "");
					move(16, 4);
					prints_nofmt("好吧，既然你意已决，金融中心也不便强留。再见！");
					quit = 1;
					pressanykey();
				}
				break;
			case 'j':
			case 'J':
				clear();
				move(15, 4);
				usercomplete("任命谁进名人堂？", uident);
				move(16, 4);
				if (uident[0] == '\0')
					break;
				if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
					prints_nofmt("错误的使用者代号...");
					pressanykey();
					break;
				}
				if (seek_in_file(DIR_MC "mingren", uident)) {
					prints_nofmt("该ID已经是名人了。");
					pressanykey();
					break;
				}
				if (askyn("确定吗？", NA, NA) == YEA) {
					ytht_add_to_file(DIR_MC "mingren", uident);
					sprintf(genbuf, "恭喜%s进入兵马俑金融中心名人堂", uident);
					deliverreport(genbuf, "兵马俑金融中心对其一直以来的工作表示感谢，祝以后顺利！");
					mail_buf("感谢你为了大富翁游戏的付出", uident, genbuf);
					sprintf(genbuf, "%s行使管理权限", currentuser.userid);
					sprintf(buf,"%s授予%s黄马褂", currentuser.userid, uident);
					millionairesrec(genbuf, buf, "");

					move(17, 4);
					prints_nofmt("任命成功。");
					pressanykey();
				}
				break;
			case 'k':
			case 'K':
				clear();
				move(12, 4);
				usercomplete("解除哪位？", uident);
				move(13, 4);
				if (uident[0] == '\0')
					break;
				if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
					prints_nofmt("错误的使用者代号...");
					pressanykey();
					break;
				}
				if (!seek_in_file(DIR_MC "mingren", uident)) {
					prints_nofmt("该ID不是兵马俑名人。");
					pressanykey();
					break;
				}
				if (askyn("确定吗？", NA, NA) == YEA) {
					ytht_del_from_file(DIR_MC "mingren", uident, true);
					sprintf(genbuf, "%s 重出江湖了", uident);
					deliverreport(genbuf, "江湖又要有一场血雨腥风了");
					sprintf(genbuf, "%s行使管理权限", currentuser.userid);
					sprintf(buf,"%s解除%s黄马甲", currentuser.userid, uident);
					millionairesrec(genbuf, buf, "");
					move(14, 4);
					prints_nofmt("解职成功。");
					pressanykey();
				}
				break;
			case 'l':
			case 'L':
				clear();
				move(15, 4);
				usercomplete("任命谁为兵马俑金融中心茶友？", uident);
				move(16, 4);
				if (uident[0] == '\0')
					break;
				if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
					prints_nofmt("错误的使用者代号...");
					pressanykey();
					break;
				}
				if (seek_in_file(DIR_MC "chayou", uident)) {
					prints_nofmt("该ID已经是茶友了。");
					pressanykey();
					break;
				}
				if (askyn("确定吗？", NA, NA) == YEA) {
					ytht_add_to_file(DIR_MC "chayou", uident);
					sprintf(genbuf, "恭喜%s成为兵马俑金融中心茶友", uident);
					deliverreport(genbuf, "大富翁随时恭候您来喝茶做客！");
					mail_buf("大富翁随时恭候您来喝茶做客！", uident, genbuf);
					sprintf(genbuf, "%s行使管理权限", currentuser.userid);
					sprintf(buf,"%s任命%s为茶友", currentuser.userid, uident);
					millionairesrec(genbuf, buf, "");

					move(17, 4);
					prints_nofmt("任命成功。");
					pressanykey();
				}
				break;
			case 'm':
			case 'M':
				clear();
				move(12, 4);
				usercomplete("解除哪位？", uident);
				move(13, 4);
				if (uident[0] == '\0')
					break;
				if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
					prints_nofmt("错误的使用者代号...");
					pressanykey();
					break;
				}
				if (!seek_in_file(DIR_MC "chayou", uident)) {
					prints_nofmt("该ID不是兵马俑金融中心茶友。");
					pressanykey();
					break;
				}
				if (askyn("确定吗？", NA, NA) == YEA) {
					ytht_del_from_file(DIR_MC "chayou", uident, true);
					sprintf(genbuf, "%s 重出江湖了", uident);
					deliverreport(genbuf, "感谢您一直以来对大富翁的关注。");
					sprintf(genbuf, "%s行使管理权限", currentuser.userid);
					sprintf(buf,"%s取消%s的茶友身份", currentuser.userid, uident);
					millionairesrec(genbuf, buf, "");
					move(14, 4);
					prints_nofmt("解职成功。");
					pressanykey();
				}
				break;
			case 'n':
			case 'N':
				clear();
				move(15, 4);
				usercomplete("任命谁为铁公鸡？", uident);
				move(16, 4);
				if (uident[0] == '\0')
					break;
				if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
					prints_nofmt("错误的使用者代号...");
					pressanykey();
					break;
				}
				if (seek_in_file(DIR_MC "gongji", uident)) {
					prints_nofmt("该ID已经是铁公鸡了。");
					pressanykey();
					break;
				}
				if (askyn("确定吗？", NA, NA) == YEA) {
					ytht_add_to_file(DIR_MC "gongji", uident);
					sprintf(genbuf, "恭喜%s获得铁公鸡称号", uident);
					deliverreport(genbuf, "兵马俑金融中心对其一毛不拔的行为表示奖励！");
					//deliverreport(genbuf,
					//"兵马俑金融中心对其一贯的艰苦朴素，勤俭节约表示赞赏！");
					mail_buf("获得铁公鸡称号", uident, genbuf);
					sprintf(genbuf, "%s行使管理权限", currentuser.userid);
					sprintf(buf,"%s任命%s为铁公鸡", currentuser.userid, uident);
					millionairesrec(genbuf, buf, "");

					move(17, 4);
					prints_nofmt("任命成功。");
					pressanykey();
				}
				break;
			case 'o':
			case 'O':
				clear();
				move(12, 4);
				usercomplete("解除哪位？", uident);
				move(13, 4);
				if (uident[0] == '\0')
					break;
				if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
					prints_nofmt("错误的使用者代号...");
					pressanykey();
					break;
				}
				if (!seek_in_file(DIR_MC "gongji", uident)) {
					prints_nofmt("该ID不是铁公鸡。");
					pressanykey();
					break;
				}
				if (askyn("确定吗？", NA, NA) == YEA) {
					ytht_del_from_file(DIR_MC "gongji", uident, true);
					sprintf(genbuf, "%s 决定花钱销灾了", uident);
					deliverreport(genbuf, "从铁公鸡身上能榨出油水来。厉害厉害");
					sprintf(genbuf, "%s行使管理权限", currentuser.userid);
					sprintf(buf,"%s取消%s的铁公鸡称号", currentuser.userid, uident);
					millionairesrec(genbuf, buf, "");
					move(14, 4);
					prints_nofmt("解职成功。");
					pressanykey();
				}
				break;

			case 'y':
			case 'Y':
				clear();
				move(13, 4);
				usercomplete("任命谁为基金id？", uident);
				move(14, 4);
				if (uident[0] == '\0')
					break;
				if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
					prints_nofmt("错误的使用者代号...");
					pressanykey();
					break;
				}
				if (seek_in_file(DIR_MC "jijin", uident)) {
					prints_nofmt("该ID已经是基金ID了。");
					pressanykey();
					break;
				}
				char local_buf1[STRLEN * 4];
				getdata(14, 4, "基金名称：", buf, 50, DOECHO, YEA);
				sprintf(genbuf, "[公告]成立%s基金%s", buf, uident);
				getdata(15, 4, "原因：", buf, 50, DOECHO, YEA);
				sprintf(local_buf1, "成立原因：%s\n希望基金管理者忠于职守，建设廉洁高效的基金体系。", buf);
				move(16, 4);
				if (askyn("确定吗？", NA, NA) == NA)
					break;
				ytht_add_to_file(DIR_MC "jijin",uident);
				if (!seek_in_file(DIR_MC "mingren", uident))
					ytht_add_to_file(DIR_MC "mingren",uident);
				//基金id是给予特殊的黄马褂
				deliverreport(genbuf, local_buf1);
				mail_buf (local_buf1, uident, genbuf);
				sprintf(genbuf, "%s行使管理权限", currentuser.userid);
				sprintf(buf,"%s任命%s为基金ID", currentuser.userid, uident);
				millionairesrec(genbuf, buf, "");
				move(17, 4);
				prints_nofmt("任命成功。");
				pressanykey();
				break;

			case 'z':
			case 'Z':
				clear();
				move(12, 4);
				usercomplete("解除哪位？", uident);
				move(13, 4);
				if (uident[0] == '\0')
					break;
				if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
					prints_nofmt("错误的使用者代号...");
					pressanykey();
					break;
				}
				if (!seek_in_file(DIR_MC "jijin", uident)) {
					prints_nofmt("该ID不是基金ID。");
					pressanykey();
					break;
				}
				char local_buf[STRLEN];
				getdata(15, 4, "原因：", local_buf, 50, DOECHO, YEA);
				sprintf(letter, "撤销原因：%s", local_buf);
				move(16, 4);
				if (askyn("确定吗？", NA, NA) == NA)
					break;
				ytht_del_from_file(DIR_MC"jijin", uident, true);
				ytht_del_from_file(DIR_MC"mingren", uident, true);
				//一并取消黄马褂
				sprintf(local_buf, "[公告]撤销基金%s", uident);
				deliverreport(local_buf, letter);
				mail_buf(letter, uident, local_buf);
				sprintf(local_buf, "%s行使管理权限", currentuser.userid);
				sprintf(buf,"%s撤销基金%s", currentuser.userid, uident);
				millionairesrec(local_buf, buf, "");
				move(17, 4);
				prints_nofmt("解除成功。");
				pressanykey();
				break;

			case 'p':
			case 'P':
				clear();
				fp1 = fopen(MC_STOCK_BOARDS, "r" );
				if (fp1) {
					count = listfilecontent(MC_STOCK_BOARDS);
					clear();
					for (j = 0; j < count; j++) {
						fscanf(fp1, "%s", stockboard[j]);
					}
					fclose(fp1);
				}

				move(12, 4);
				if (askyn("确定要初始化股市吗？", NA, NA) == YEA) {
					for (j = 0; j < count; j++) {
						ythtbbs_cache_Board_foreach_v(init_stock_v, stockboard[j]);
					}
					sprintf(genbuf, "%s行使管理权限", currentuser.userid);
					sprintf(buf,"%s初始化股市", currentuser.userid);
					millionairesrec(genbuf, buf, "");
					move(14, 4);
					prints_nofmt("股市初始化成功。");
					pressanykey();
				}
				break;

			case 'r':
			case 'R':
				clear();
				move(1, 0);
				prints_nofmt("目前名人堂的ID列表：");
				listfilecontent(DIR_MC "mingren");
				pressanykey();
				break;

			case 's':
			case 'S':
				clear();
				move(1, 0);
				prints_nofmt("目前茶友的ID列表：");
				listfilecontent(DIR_MC "chayou");
				pressanykey();
				break;

			case 't':
			case 'T':
				clear();
				move(1, 0);
				prints_nofmt("目前铁公鸡的ID列表：");
				listfilecontent(DIR_MC "gongji");
				pressanykey();
				break;

			case 'u':
			case 'U':
				clear();
				move(1, 0);
				prints_nofmt("目前基金ID列表：");
				listfilecontent(DIR_MC "jijin");
				pressanykey();
				break;

			case 'X':
			case 'x':
				money_deny();
				break;

			case '1':
				clear();
				move(12, 4);
				usercomplete("更改谁的现金数额？", uident);
				move(13, 4);
				if (uident[0] == '\0')
					break;
				if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
					prints_nofmt("错误的使用者代号...");
					pressanykey();
					break;
				}
				prints("%s目前有现金%d兵马俑币。", uident, loadValue(uident, MONEY_NAME, MAX_MONEY_NUM));
				getdata(14, 4, "改为多少?", genbuf, 10, DOECHO, YEA);
				num = atoi(genbuf);
				sprintf(buf, "确定要改为%d吗？", num);
				move(15, 4);
				if (askyn(buf, NA, NA) == YEA) {
					saveValue(uident ,MONEY_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
					saveValue(uident ,MONEY_NAME, num, MAX_MONEY_NUM);
					sprintf(genbuf, "%s行使管理权限", currentuser.userid);
					sprintf(buf,"更改%s现金数额为%d", uident, num);
					millionairesrec(genbuf, buf, "");
					move(17, 4);
					prints_nofmt("修改成功。");
					pressanykey();
				}
				break;
			case '2':
				clear();
				move(12, 4);
				usercomplete("更改谁的存款数额？", uident);
				move(13, 4);
				if (uident[0] == '\0')
					break;
				if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
					prints_nofmt("错误的使用者代号...");
					pressanykey();
					break;
				}
				prints("%s目前有存款%d兵马俑币。", uident, loadValue(uident, CREDIT_NAME, MAX_MONEY_NUM));
				getdata(14, 4, "改为多少?", genbuf, 10, DOECHO, YEA);
				num = atoi(genbuf);
				sprintf(buf, "确定要改为%d吗？", num);
				move(15, 4);
				if (askyn(buf, NA, NA) == YEA) {
					saveValue(uident, CREDIT_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
					saveValue(uident, CREDIT_NAME, num, MAX_MONEY_NUM);
					sprintf(genbuf, "%s行使管理权限", currentuser.userid);
					sprintf(buf,"更改%s存款数额为%d", uident, num);
					millionairesrec(genbuf, buf, "");
					move(17, 4);
					prints_nofmt("修改成功。");
					pressanykey();
				}
				break;

			case '0':
				clear();
				move(6, 4);
				sprintf(buf, "确定要%s金融中心吗？", (mc->isMCclosed) ? "开启" : "关闭");
				if (askyn(buf, NA, NA) == YEA)
					mc->isMCclosed = (mc->isMCclosed)?0:1;
				move(9, 4);
				prints_nofmt("修改成功。");
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

static int//彩票36选7
valid367Bet(char *buf)
{
	int i, j;
	int temp[7];
	int slot = 0;

	if (strlen(buf) != 20) {	/*  长度必须为20 */
		return 0;
	}
	for (i = 0; i < 20; i++) {	/*  基本格式必须正确   */
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
	for (i = 0; i < 7; i++) {	/* 数字无重复 */
		for (j = 0; j < 7; j++) {
			if (temp[j] == temp[i] && i != j) {
				return 0;
			}
		}
	}
	return 1;
}

//彩票36选7
static int make367Prize(char *bet, char *prizeSeq) {
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

//彩票36选7
static void make367Seq(char *prizeSeq) {
	int i, j;
	int num;
	int temp[7];
	int slot = 0;
	int success;

	memset(temp, 0, sizeof temp);

	srandom(time(0));
	for (i = 0; i < 7; i++) {
		do {		/*  数字不能相同  */
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

	sprintf(genbuf, "序列是：  %s  。您中奖了吗？", prizeSeq);
	deliverreport("[公告]本期36选7彩票摇奖结果", genbuf);
}

/*彩票26选7 */
static int open_36_7(void) {
	FILE *fp;
	char line[MAX_RECORD_LINE];
	char prizeSeq[MAX_BET_LENGTH];
	char *bet;
	char *userid;
	int prizeType;
	int totalMoney, remainMoney;
	int num_bp = 0, num_1p = 0, num_2p = 0, num_3p = 0, num_cp = 0;

	make367Seq(prizeSeq);	//产生序列

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
		/*   ---------------------计算奖励----------------------- */
		prizeType = make367Prize(bet, prizeSeq);
		switch (prizeType) {
			case 7:
				ytht_add_to_file(DIR_MC_TEMP "36_7_bp", userid);
				num_bp++;
				break;
			case 6:
				ytht_add_to_file(DIR_MC_TEMP "36_7_1p", userid);
				num_1p++;
				break;
			case 5:
				ytht_add_to_file(DIR_MC_TEMP "36_7_2p", userid);
				num_2p++;
				break;
			case 4:
				ytht_add_to_file(DIR_MC_TEMP "36_7_3p", userid);
				num_3p++;
				break;
			case 3:
				ytht_add_to_file(DIR_MC_TEMP "36_7_cp", userid);
				num_cp++;
				break;
			default:
				break;
		}
	}			/* end of while */
	fclose(fp);

	/*  ------------------------ 发奖 --------------------- */
	totalMoney = mc->prize367 + PRIZE_PER;
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
			sprintf(genbuf, "您得到了 %d 兵马俑币的奖金。恭喜！希望下次还有好运～～～", per_bp);
			mail_buf(genbuf, userid, "恭喜您获得36选7特等奖！");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 1024-1, fp);
		sprintf(title, "本期36选7特等奖名单（每注奖金%d兵马俑币）", per_bp);
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
			sprintf(genbuf, "您得到了 %d 兵马俑币的奖金。恭喜！希望下次还有好运～～～", per_1p);
			mail_buf(genbuf, userid, "恭喜您获得36选7一等奖！");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 1024-1, fp);
		sprintf(title, "本期36选7一等奖名单（每注奖金%d兵马俑币）", per_1p);
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
			sprintf(genbuf, "您得到了 %d 兵马俑币的奖金。恭喜！希望下次还有好运～～～", per_2p);
			mail_buf(genbuf, userid, "恭喜您获得36选7二等奖！");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 1024-1, fp);
		sprintf(title, "本期36选7二等奖名单（每注奖金%d兵马俑币）", per_2p);
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
			sprintf(genbuf, "您得到了 %d 兵马俑币的奖金。恭喜！希望下次还有好运～～～", per_3p);
			mail_buf(genbuf, userid, "恭喜您获得36选7三等奖！");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 2048-1, fp);
		sprintf(title, "本期36选7三等奖名单（每注奖金%d兵马俑币）", per_3p);
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
			sprintf(genbuf, "您得到了 %d 兵马俑币的奖金。恭喜！希望下次还有好运～～～", per_cp);
			mail_buf(genbuf, userid, "恭喜您获得36选7安慰奖！");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 2048-1, fp);
		sprintf(title, "本期36选7安慰奖名单（每注奖金%d兵马俑币）", CMFT_PRIZE);
		deliverreport(title, buf);
		fclose(fp);
	}
	remainMoney = limitValue(remainMoney, MAX_POOL_MONEY);
	mc->prize367 = remainMoney;
	remove(DIR_MC_TEMP "36_7_list");
	remove(DIR_MC_TEMP "36_7_bp");
	remove(DIR_MC_TEMP "36_7_1p");
	remove(DIR_MC_TEMP "36_7_2p");
	remove(DIR_MC_TEMP "36_7_3p");
	remove(DIR_MC_TEMP "36_7_cp");
	remove(DIR_MC_TEMP "36_7_start");
	return 0;
}

/*彩票--足彩*//*计算复式注的数量 */
static int computeSum(char *complexBet) {
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
	total *= countNum;	/*最后一个单元 */
	return total;
}

/*彩票--足彩*//*保存复式注为单注 */
static void saveSoccerRecord(char *complexBet) {
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
	if (simple) {		/*简单标准形式，直接打印 */
		for (i = 0, j = 0; i < len; i++) {
			if (complexBet[i] != '-') {
				genbuf[j++] = complexBet[i];
			}
		}
		genbuf[j] = '\0';
		snprintf(buf, sizeof buf, "%s %s", currentuser.userid, genbuf);
		ytht_add_to_file(DIR_MC_TEMP "soccer_list", buf);
	} else {
		for (i = 0; i < len; i++) {	/*寻找第一个复式单元 */
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

		for (i = 0; i < count; i++) {	/*对每一个要拆分的单元的元素 */
			int slot = 0;
			char *temp = malloc(len * sizeof (char));

			/*得到前面的部分 */
			if (firstDivStart != 0) {
				for (j = 0; j < firstDivStart; j++, slot++) {
					temp[slot] = complexBet[j];
				}
			}
			temp[slot] = complexBet[firstDivStart + i];
			slot++;
			/*得到后面的部分 */
			for (j = firstDivEnd + 1; j < len; j++, slot++) {
				temp[slot] = complexBet[j];
			}
			temp[slot] = '\0';

			/*对每一个拆分，进行递归调用 */
			saveSoccerRecord(temp);
		}

	}
}

/*彩票--足彩*/
static int validSoccerBet(char *buf) {
	int count = 0;
	int meetSeperator = 1;
	size_t i;
	int first = 0, second = 0;

	if (strlen(buf) == 0) {
		return 0;
	}
	for (i = 0; i < strlen(buf); i++) {
		if (buf[i] == '-') {
			if (meetSeperator == 1) {	/*如果连续遇到-，肯定不正确 */
				return 0;
			}
			count = 0;
			meetSeperator = 1;
		} else {
			if (buf[i] != '3' && buf[i] != '1' && buf[i] != '0') {	/*不是310，肯定不对 */
				return 0;
			}
			count++;
			if (count > 3) {
				return 0;
			}
			if (count == 1) {
				first = buf[i];
			} else if (count == 2) {
				if (buf[i] == first) {	/*重合 */
					return 0;
				}
				second = buf[i];
			} else if (count == 3) {
				if (buf[i] == first || buf[i] == second) {	/*重合 */
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
/*彩票--足彩*/
static int makeSoccerPrize(char *bet, char *prizeSeq) {
	int diff = 0;
	int i;
	int n1 = strlen(bet);
	int n2 = strlen(prizeSeq);

	if (n1 != n2) {
		return 10;	/*不中奖 */
	}
	for (i = 0; i < n1; i++) {
		if (bet[i] != prizeSeq[i]) {
			diff++;
		}
	}
	return diff;
}

/*彩票--足彩*/
static int open_soccer(char *prizeSeq) {
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
	sprintf(genbuf, "序列是：%s。您中奖了吗？", prizeSeq);
	deliverreport("[公告]本期足彩结果", genbuf);
	while (fgets(line, MAX_RECORD_LINE, fp)) {
		userid = strtok(line, " ");
		bet = strtok(NULL, "\n");
		if (!userid || !bet) {
			continue;
		}
		/*   ---------------------计算奖励----------------------- */
		prizeType = makeSoccerPrize(bet, prizeSeq);
		switch (prizeType) {
			case 0:	/*完全相同 */
				ytht_add_to_file(DIR_MC_TEMP "soccer_bp", userid);
				num_bp++;
				break;
			case 1:	/*有一个不同 */
				ytht_add_to_file(DIR_MC_TEMP "soccer_1p", userid);
				num_1p++;
				break;
			case 2:	/*有二个不同 */
				ytht_add_to_file(DIR_MC_TEMP "soccer_2p", userid);
				num_2p++;
				break;
			case 3:	/*有三个不同 */
				ytht_add_to_file(DIR_MC_TEMP "soccer_3p", userid);
				num_3p++;
				break;
			case 4:	/*有四个不同 */
				ytht_add_to_file(DIR_MC_TEMP "soccer_cp", userid);
				num_cp++;
				break;
			default:
				break;
		}
	}			/* end of while */
	fclose(fp);
	/*  ------------------------ 发奖 --------------------- */
	totalMoney = mc->prizeSoccer + PRIZE_PER;
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
			sprintf(genbuf, "您得到了 %d 兵马俑币的奖金。恭喜！希望下次还有好运～～～", per_bp);
			mail_buf(genbuf, userid, "恭喜您获得足球彩票特等奖！");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 1024-1, fp);
		sprintf(title, "本期足彩特等奖名单（每注奖金%d兵马俑币）", per_bp);
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
			sprintf(genbuf, "您得到了 %d 兵马俑币的奖金。恭喜！希望下次还有好运～～～", per_1p);
			mail_buf(genbuf, userid, "恭喜您获得足球彩票一等奖！");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 1024-1, fp);
		sprintf(title, "本期足彩一等奖名单（每注奖金%d兵马俑币）", per_1p);
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
			sprintf(genbuf, "您得到了 %d 兵马俑币的奖金。恭喜！希望下次还有好运～～～", per_2p);
			mail_buf(genbuf, userid, "恭喜您获得足球彩票二等奖！");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 2048-1, fp);
		sprintf(title, "本期足彩二等奖名单（每注奖金%d兵马俑币）", per_2p);
		deliverreport(title, buf);
		fclose(fp);
	}
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
			sprintf(genbuf, "您得到了 %d 兵马俑币的奖金。恭喜！希望下次还有好运～～～", per_cp);
			mail_buf(genbuf, userid, "恭喜您获得足球彩票安慰奖！");
		}
		fseek(fp, 0, SEEK_SET);
		fread(buf, sizeof (char), 2048-1, fp);
		sprintf(title, "本期足彩安慰奖名单（每注奖金%d兵马俑币）", CMFT_PRIZE);
		deliverreport(title, buf);
		fclose(fp);
	}
	remainMoney = limitValue(remainMoney, MAX_POOL_MONEY);
	mc->prizeSoccer = remainMoney;
	remove(DIR_MC_TEMP "soccer_list");
	remove(DIR_MC_TEMP "soccer_bp");
	remove(DIR_MC_TEMP "soccer_1p");
	remove(DIR_MC_TEMP "soccer_2p");
	remove(DIR_MC_TEMP "soccer_3p");
	remove(DIR_MC_TEMP "soccer_cp");
	remove(DIR_MC_TEMP "soccer_start");
	return 0;
}

/*商场--保镖*/
static int money_check_guard() {
	int money, guard;
	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	guard = loadValue(currentuser.userid, "guard", 8);
	if (guard > 0) {
		saveValue(currentuser.userid, "guard", -guard, 50);
		move(9, 4);
		if (random() % 2 == 0) {
			prints_nofmt("你的保镖离你而去,并顺手拿了你两成的现金.");
			saveValue(currentuser.userid, MONEY_NAME, -money / 5, MAX_MONEY_NUM);
		} else {
			prints_nofmt("你的保镖一棒子敲晕了你,拿走了你身上一半的钱，跑路了。");
			saveValue(currentuser.userid, MONEY_NAME, -money / 2, MAX_MONEY_NUM);
			pressanykey();
			Q_Goodbye(0, NULL, NULL);
		}
		return 1;
	}
	return 0;
}

/*赌博--骰宝*/
static int money_dice() {
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
			money_show_stat("兵马俑赌场骰宝厅VIP室");
		} else {
			money_show_stat("兵马俑赌场骰宝厅");
		}
		move(4, 4);
		prints_nofmt("\033[1;31m多买多赚，少买少赔，买定离手，愿赌服输\033[m");
		move(5, 4);
		prints_nofmt("分大小两门，4-10点是小，11-17点为大。");
		move(6, 4);
		prints_nofmt("若押小开小，可拿一倍彩金，押大的就全归庄家。");
		move(7, 4);
		prints_nofmt("庄家要是摇出全骰（三个骰子点数一样）则通吃大小家。");
		move(8, 4);
		if (isVIP) {
			prints_nofmt("最小压 100000兵马俑币,上限 10000000 兵马俑币。");
		} else {
			prints_nofmt("最小压 1000 兵马俑币,上限 500000 兵马俑币。要玩大的请进VIP室。");
		}
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1]下注 [Q]离开                                                   \033[m");
		win = 0;
		ch = igetkey();
		switch (ch) {
			case '1':
				if (isVIP) {
					getdata(9, 4, "您压多少兵马俑币？[100000]", genbuf, 9, DOECHO, YEA);
				} else {
					getdata(9, 4, "您压多少兵马俑币？[1000]", genbuf, 7, DOECHO, YEA);
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
					prints_nofmt("这里是VIP室，压注有下限。");
					pressanykey();
					break;
				}
				if (!isVIP && num > 500000) {
					move(11, 4);
					prints_nofmt("要玩的大的，请进VIP室。");
					pressanykey();
					break;
				}
				if (num < 1000) {
					move(11, 4);
					prints_nofmt("有没有钱啊？那么点钱我们不带玩的。");
					pressanykey();
					break;
				}
				if (num > 10000000) {
					move(11,4);
					prints_nofmt("超过了最大赌注，请重新下注。");
					pressanykey();
					break;
				}
				getdata(10, 4, "您压大(L)还是小(S)？[L]", genbuf, 3, DOECHO, YEA);
				if (genbuf[0] == 'S' || genbuf[0] == 's')
					target = 1;
				else
					target = 0;
				sprintf(genbuf, "买定离手，您买了 \033[1;31m%d\033[m 兵马俑币的 \033[1;31m%s\033[m，确定么？", num, target ? "小" : "大");
				move(11, 4);
				if (askyn(genbuf, YEA, NA) == YEA) {
					money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
					if (money < num) {
						move(12, 4);
						prints_nofmt("去去去，没那么多钱捣什么乱！      \n");
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
							mc->prize777 += 1000000;
						else
							mc->prize777 += num * 50 / 100;
						if (mc->prize777 > MAX_MONEY_NUM)
							mc->prize777 = MAX_MONEY_NUM;
						sprintf(genbuf, "\033[1;32m庄家通杀！\033[m");
					} else if (t1 + t2 + t3 < 11) {
						sprintf(genbuf, "%d 点，\033[1;32m小\033[m", t1 + t2 + t3);
						if (target == 1)
							win = 1;
					} else if (t1 + t2 + t3 > 10) {
						sprintf(genbuf, "%d 点，\033[1;32m大\033[m", t1 + t2 + t3);
						if (target == 0)
							win = 1;
					}
					prints("开了开了，%d %d %d，%s", t1, t2, t3, genbuf);
					move(13, 4);
					if (win) {
						prints_nofmt("恭喜您，再来一把吧！");
						saveValue(currentuser.userid, MONEY_NAME, num, MAX_MONEY_NUM);
						whoTakeCharge(3, slow);//slowaction
						saveValue(slow, MONEY_NAME, -num, MAX_MONEY_NUM);

						if (num >= RUMOR_MONEY && random() % 2) {
							int rumor = makeRumor(num);
							sprintf(genbuf, "有人目击 %s 在兵马俑赌场一把赢了 %d 的兵马俑币！", currentuser.userid, rumor);
							deliverreport("[谣言]来自兵马俑赌场的消息", genbuf);
						}
						sprintf(title, "%s参与赌博(骰宝)(赢)", currentuser.userid);
						sprintf(buf, "%s在骰宝赢了%d兵马俑币", currentuser.userid, num);
						millionairesrec(title, buf, "赌博骰宝");
					} else {
						prints_nofmt("没有关系，先输后赢...");
						saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
						whoTakeCharge(3, slow);//slowaction
						saveValue(slow, MONEY_NAME, +num, MAX_MONEY_NUM);
						sprintf(title, "%s参与赌博(骰宝)(输)", currentuser.userid);
						sprintf(buf, "%s在骰宝输了%d兵马俑币", currentuser.userid, num);
						millionairesrec(title, buf, "赌博骰宝");
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

/*黑帮*/
static int money_robber() {
	int quit = 0, guard_num = 0;
	int ch, x, y, z, ch2;
	int num, money, r, ra, id, count = 0, rob,credit;
	int freeTime;
	int zhuannum=20;
	time_t currentTime = time(0);
	char uident[IDLEN + 1], buf[200], title[40];
	double mathtmp;
	srandom(time(0));
	char letter1[] = "限你半小时内给我寄钱，不然有你好看！\n";
	char letter2[] = "快给我寄钱，否则小心你的脑袋挨板砖。\n我会记挂着你的安全的，嘿嘿...";
	char letter3[] = "快给我寄钱，否则小心我把你的钱全部抢走！";
	while (!quit) {
		clear();
		money_show_stat("背阴巷");
		move(4, 4);
		prints_nofmt("两年前的兵马俑黑帮无恶不作，名噪一时，不过最近警察严打，活动有所收敛。");
		move(5, 4);
		prints_nofmt("一个黑衣人小声说：“要板砖么？拍人很疼的。”");
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1]拍砖 [2]偷窃 [3]勒索 [4]抢人 [5]黑帮帮主 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
			case '1':
				clear();

				if(!Allclubtest(currentuser.userid)){
					move(5, 4);
					prints_nofmt("    \033[1;32m  普通市民不要惹事\033[m");
					pressanykey();
					break;
				}
				if (seek_in_file(DIR_MC "chayou", currentuser.userid)){
					move(5, 4);
					prints_nofmt("    \033[1;32m  茶友不要惹事\033[m");
					pressanykey();
					break;
				}
				if (seek_in_file(DIR_MC "mingren", currentuser.userid)) {
					move(5, 4);
					prints_nofmt("    \033[1;32m  不要惹事\033[m");
					pressanykey();
					break;
				}
				r = random() % 40;
				if (r < 1)
					money_police();
				money_show_stat("黑帮板砖生产基地");
				move(4, 4);
				prints_nofmt("这里的板砖质地优良，拿去拍人一定痛快。");
				move(5, 4);
				prints_nofmt("一块板砖 1000 兵马俑币。");
				move(6, 4);
				if (currentuser.dietime > 0) {
					prints_nofmt("你已经死了啊！抓鬼啊！");
					pressanykey();
					Q_Goodbye(0, NULL, NULL);
					break;
				}
				usercomplete("你要拍谁:", uident);
				if (uident[0] == '\0')
					break;
				freeTime = loadValue(currentuser.userid, "freeTime", 2000000000);
				if (currentTime < freeTime){
					pressreturn();
					break;
				}
				if (!(id = getuser(uident))) {
					move(7, 4);
					prints_nofmt("错误的使用者代号...");
					pressreturn();
					break;
				}
				if (lookupuser.dietime > 0) {
					move(7, 4);
					prints_nofmt("死人你也不放过，太狠了吧？");
					pressreturn();
					break;
				}
				if ((slowclubtest("Beggar", currentuser.userid) && slowclubtest("Beggar", uident))
						|| (slowclubtest("Rober", currentuser.userid) && slowclubtest("Rober", uident))
						|| (slowclubtest("Police", currentuser.userid) && slowclubtest("Police", uident))
						|| (slowclubtest("killer", currentuser.userid) && slowclubtest("killer", uident))) {
					move(7, 4);
					prints_nofmt("都是自家兄弟...");
					pressreturn();
					break;
				}
				getdata(7, 4, "你要拍几块？ [0]", genbuf, 4, DOECHO, YEA);
				if (genbuf[0] == '\0')
					break;
				count = atoi(genbuf);
				if (count < 1) {
					move(8, 4);
					prints_nofmt("没有板砖你拿什么拍？");
					pressanykey();
					break;
				}
				if (currentuser.dietime > 0) {
					prints_nofmt("你已经死了啊！抓鬼啊！");
					pressanykey();
					Q_Goodbye(0, NULL, NULL);
					break;
				}
				move(8, 4);
				num = count * 1000;
				sprintf(genbuf, "总共需要 %d 兵马俑币。", num);
				if (askyn(genbuf, NA, NA) == YEA) {
					money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
					if (money < num) {
						move(9, 4);
						prints_nofmt("您的钱不够...");
						pressanykey();
						break;
					}
					if (money_check_guard()) {
						pressanykey();
						break;
					}
					if(seek_in_file(DIR_MC "mingren", uident)) {
						prints ("      他有黄马褂，你还是算了吧\n");
						pressanykey();
						break;
					}

					if (seek_in_file(DIR_MC "killer", currentuser.userid))
						zhuannum=40;

					saveValue(currentuser.userid, "last_rob", -2000000000, 2000000000);
					saveValue(currentuser.userid, "last_rob", time(0), 2000000000);
					saveValue(currentuser.userid, MONEY_NAME, -num,  MAX_MONEY_NUM);

					saveValue("BMYRober", MONEY_NAME, +num/2, MAX_MONEY_NUM);

					prints("        经过几天的偷窥和跟踪，你发现每天早上7点10分%s会路过僻静的\n", uident);
					prints_nofmt("    东花园边。今天你拿着买来兵马俑板砖，准备行动了。\n");
					prints_nofmt("        拍人板砖，可以让其住院花钱治伤，嘿嘿...\n");
					prints_nofmt("        当然，你也可能遭到反击，甚至致死！\n");
					if (askyn("    废话少说，你还想拍么？", YEA, NA) == NA) {
						move(15, 0);
						prints_nofmt("            唉，最后关头你害怕了，所以不拍了。\n");
						pressanykey();
						break;
					} else {
						if(!seek_in_file(DIR_MC "gongji", uident))
							saveValue(currentuser.userid, "rob", 1, 50);
						if (currentuser.dietime > 0) {
							prints_nofmt("你已经死了啊！抓鬼啊！");
							pressanykey();
							Q_Goodbye(0, NULL, NULL);
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
						if (r < 100 * z / (x + x + y + y) +zhuannum+ count)	//拍人成功
						{
							guard_num = loadValue(uident, "guard", 8);
							if (guard_num > 0) {
								saveValue(uident, "guard", -1, 50);
								prints_nofmt("你干掉了他一个保镖");
								pressanykey();
								break;
							}

							prints("       你这坏蛋，背后偷袭，砸中%s的小脑袋瓜。\n", uident);
							money = loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
							if (money == 0) {
								if(!Allclubtest(lookupuser.userid) || seek_in_file(DIR_MC "chayou", lookupuser.userid)){
									showAt(17, 4, "你都拍到人家没钱治伤了...积点阴德吧！\n", 0);
									sprintf(buf, "你被%s拍了板砖，你没钱治伤，只能咬牙忍痛...", currentuser.userid);
								}else{
									saveValue(uident, MONEY_NAME, -money, MAX_MONEY_NUM);
									move(17, 4);
									prints ("       你拍了%s板转，他死了。", uident);
									sprintf(genbuf, "%s进行黑帮活动(拍砖)", currentuser.userid);
									sprintf(buf,"%s拍死了%s ", currentuser.userid, uident);
									millionairesrec(genbuf, buf, "黑帮活动");
									lookupuser.dietime = lookupuser.stay + 999 * 60;
									substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
									if (seek_in_file(DIR_MC "killer", currentuser.userid)){
										if (random()%3 == 0){
											sprintf(genbuf, "你被%s用板砖砸死了，好惨", currentuser.userid);
											mail_buf(genbuf, uident, "替天行道");}
										sprintf(genbuf,
												"本站人士%s于10分钟前在铜锣湾的\n一起枪击事件中饮弹身亡\n警方透露此人有帮会背景\n\n"
												"目前本站激进组织杀手天空宣布对此事负责，\n有关事件的进一步报道请关注本版新闻", uident);
										deliverreport("[新闻]铜锣湾发生一起枪击事件", genbuf);
									} else if (slowclubtest("Beggar", currentuser.userid)){
										sprintf(genbuf,
												"本港人士%s于10分钟前在尖沙咀的\n一起暴力冲突中伤重不治\n警方称此人有帮会背景\n\n"
												"据消息灵通人士透露，此事件与近期\n的丐帮活动有关", uident);
										deliverreport("[新闻]尖沙咀发生一起暴力事件",genbuf);
										sprintf(genbuf,
												"你被丐帮弟子%s用板砖砸死了，好惨", currentuser.userid);
										mail_buf(genbuf, uident, "你死了");
									} else if (slowclubtest("Rober",currentuser.userid)){
										sprintf(genbuf,
												"本港人士%s于10分钟前在澳门的\n一起黑帮械斗中丧命\n警方怀疑此人与黑社会有过节\n\n"
												"据一位不愿透露姓名的警署官员透露\n这次事件可能和黑帮寻仇有关\n警方表示一定打击犯罪，维护治安", uident);
										deliverreport("[新闻]澳门发生一起帮会冲突", genbuf);
										sprintf(genbuf,"你被%s用板砖砸死了，好惨", currentuser.userid);
										mail_buf(genbuf, uident,"你死了");
									} else if (slowclubtest("killer",currentuser.userid)){
										sprintf(genbuf, "你在和黑帮的冲突中被%s用板砖砸死了，好惨", currentuser.userid);
										mail_buf(genbuf, uident,"替天行道");
										sprintf(genbuf,
												"本站人士%s于10分钟前在九龙的\n一起枪击事件中饮弹身亡\n警方透露此人有帮会背景\n\n"
												"警方怀疑死者与杀手有私人恩怨，\n有关事件的进一步报道请关注本版新闻", uident);
										deliverreport("[新闻]九龙发生一起枪击事件", genbuf);
									}
									else{
										sprintf(genbuf, "你被%s用板砖砸死了，好惨", currentuser.userid);
										mail_buf(genbuf, uident, "你死了");
									}
									//saveValue(lookupuser.userid, MONEY_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
									pressanykey();
								}
							} else {
								saveValue(uident, MONEY_NAME, -num, MAX_MONEY_NUM);
								sprintf(buf, "哈哈，%s花了%d元治伤，现在出院了。小心报复你！\n", uident, num);
								move(17, 4);
								prints_nofmt(buf);
								sprintf(buf, "你被%s拍了板砖，花了%d兵马俑币治伤，呜呜呜呜...", currentuser.userid, num);
							}
						} else {
							prints_nofmt("      很不幸，你没有拍中。反而被砸中小脑袋瓜...");

							money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
							num *= 3;
							if (money < num) {
								saveValue (currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
								showAt(17, 4, "你鲜血直流，可是钱不够治疗，被医院扔了出来。", 0);
								showAt(18, 4, "最后伤势恶化，你死了...", 0);
								sprintf(genbuf, "%s进行黑帮活动(拍砖)", currentuser.userid);
								sprintf(buf,"%s拍%s, 自己挂了, 瓜 ", currentuser.userid, uident);
								millionairesrec(genbuf, buf, "黑帮活动");
								set_safe_record();
								currentuser.dietime = currentuser.stay + (num - money);
								substitute_record (PASSFILE, &currentuser, sizeof(currentuser), usernum);
								saveValue(currentuser.userid, MONEY_NAME,  -MAX_MONEY_NUM,  MAX_MONEY_NUM);
								saveValue(currentuser.userid, CREDIT_NAME,  -MAX_MONEY_NUM,  MAX_MONEY_NUM);
								pressanykey();
								Q_Goodbye(0, NULL, NULL);
							} else {
								saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
								move(17, 4);
								sprintf(buf, "你花了%d兵马俑币才治好了伤，看你下次还拍人不。", num);
								prints_nofmt(buf);
							}
						}
					}
					pressanykey();
				}
				break;
			case '2':
				clear();
				if(!Allclubtest(currentuser.userid)){
					showAt(5, 4, "    \033[1;32m  普通市民不要惹事\033[m", 1);
					break;
				}
				if (seek_in_file(DIR_MC "chayou", currentuser.userid)){
					showAt(5, 4, "    \033[1;32m  茶友不要惹事\033[m", 1);
					break;
				}
				if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
					showAt(5, 4, "    \033[1;32m  不要惹事\033[m", 1);
					break;
				}
				move(6, 4);
				usercomplete("偷谁？", uident);
				if (uident[0] == '\0')
					break;
				freeTime = loadValue(currentuser.userid, "freeTime", 2000000000);
				if (currentTime < freeTime){
					pressreturn();
					break;
				}
				if (!getuser(uident)) {
					showAt(7, 4, "错误的使用者代号...", 2);
					break;
				}
				if(seek_in_file(DIR_MC "mingren", uident)){
					showAt(7, 4, "      他有黄马褂，你还是算了吧\n", 1);
					break;
				}
				if (lookupuser.dietime > 0) {
					showAt(7, 4, "死人你也不放过，太狠了吧？", 1);
					break;
				}
				if(strcmp(lookupuser.userid,"BMYpolice")==0||strcmp(lookupuser.userid,"BMYbeg")==0||
						strcmp(lookupuser.userid,"BMYRober")==0||strcmp(lookupuser.userid,"BMYboss")==0||
						strcmp(lookupuser.userid,"BMYKillersky")==0){
					showAt(7, 4, "这个人是我亲戚，不许抢", 2);
					break;
				}
				credit = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);
				if(credit<2000){
					showAt(7, 4, "保证金都没有，还是不要偷了!", 2);
					break;
				}

				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);

				if (currentuser.stay < 86400) {
					showAt(7, 4, "小孩子家别学坏了!", 2);
					break;
				}
				getdata(7, 4, "请输入你的密码: ", buf, PASSLEN, NOECHO, YEA);
				if (*buf == '\0'
						|| !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
					showAt(8, 4, "很抱歉, 您输入的密码不正确。", 2);
					break;
				}
				saveValue(currentuser.userid, "last_rob", -2000000000, 2000000000);
				saveValue(currentuser.userid, "last_rob", time(0), 2000000000);
				showAt(9, 4, "\033[1;5;31m警告\033[0;1;31m： 小心啊，最近警署在严打哦！", 0);
				move(10, 4);
				if (askyn("真的要偷么？", NA, NA) == NA)
					break;
				set_safe_record();
				if (currentuser.dietime > 0) {
					showAt(11, 4, "你已经死了啊！抓鬼啊！", 1);
					Q_Goodbye(0, NULL, NULL);
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
						showAt(11, 4, "你干掉了他一个保镖", 1);
						break;
					}
					if (random() % 2 == 0) {
						money = loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
						r = random() % 50;
						money = money / 100 * r;
						saveValue(uident, MONEY_NAME, -money, MAX_MONEY_NUM);
						saveValue(currentuser.userid, MONEY_NAME, money, MAX_MONEY_NUM);
						move(11, 4);
						prints("\033[1;31m%s\033[m 的钱包没放好，你把手伸进去，摸到了 %d 兵马俑币现金，快跑吧...", uident, money);
						sprintf(title, "%s进行黑帮活动(偷窃)", currentuser.userid);
						sprintf(buf,"%s偷了%s %d兵马俑币", currentuser.userid, uident, money);
						if (money != 0)
							millionairesrec(title, buf, "黑帮活动");
						sprintf(buf, "%s 趁您不注意的时候偷了您 %d 兵马俑币。", currentuser.userid, money);
						sprintf(title, "对不起，您被偷窃");
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
						prints("\033[1;31m你去摸 %s 的钱包,眼看已经得手了,他忽然转过身来发现了你", uident);
						move(12, 4);
						prints("\033[1;31m唉呀呀,你一愣神,不仅没偷到他的钱包,反而被他摸走了 %d 兵马俑币。", money);
						sprintf(title, "%s进行黑帮活动(偷窃)", currentuser.userid);
						sprintf(buf,"%s偷%s, 反被抢了%d兵马俑币", currentuser.userid, uident, money);
						if (money != 0)
							millionairesrec(title, buf, "黑帮活动");
						sprintf(title, "您遇到小偷");
						sprintf(buf,
								"%s 想趁您不注意偷您的钱包,结果让你发现了。你反抢了他 %d 兵马俑币。这把赚翻了,^_^",
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
						prints_nofmt("啊！有警察，你在逃跑的时候只听一声枪响...");
						set_safe_record();
						if (money / 200 < 3600)
							currentuser.dietime = currentuser.stay + 1000*60;
						else if (money < 10000000){
							mathtmp = (double)(money)/10000;
							mathtmp = 686.3455879296685 + 4.0492760356525315 * mathtmp + 0.004264378376417802 * mathtmp * mathtmp;//我拟合的二次函数
							currentuser.dietime = currentuser.stay + (int)(mathtmp * 60);//+(money / 200)
						} else{
							mathtmp = 9 + (double)(currentuser.lastlogin)/(double)(currentuser.stay + currentuser.lastlogin);
							currentuser.dietime = currentuser.stay +(int) (1000*mathtmp*60);
						}
						substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
						saveValue(currentuser.userid, MONEY_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
						pressanykey();
						Q_Goodbye(0, NULL, NULL);
					} else {
						if (askyn("被警察发现了,你要逃跑么?", YEA, NA) == NA) {
							saveValue(currentuser.userid, "rob", 1, 50);
							move(12, 4);
							if (askyn ("警察问你话,你准备坦白从宽么?", YEA, NA) == YEA) {
								money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
								saveValue(currentuser.userid, MONEY_NAME, -money * 50 /100, MAX_MONEY_NUM);
								sprintf(title, "%s进行黑帮活动(偷窃)", currentuser.userid);
								sprintf(buf,"%s偷%s, 被警察没收%d兵马俑币", currentuser.userid, uident, money/2);
								if (money != 0)
									millionairesrec(title, buf, "黑帮活动");
								showAt(13, 4, "你被带到警察局,在没收了身上所有的钱之后,还要给你训话一番。", 0);
								showAt(14, 4, "现在是警察给你的15秒钟训话时间，老老实实听着吧。", 1);
								sleep(15);
								money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
								sprintf(genbuf, "出了警察局,你高兴的从鞋里掏出藏起来的%d兵马俑币。呜呜,一股臭脚丫子味...", money);
								showAt(15, 4, genbuf, 1);
							} else {
								money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
								if (random() % 2 == 0) {
									saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
									sprintf(title, "%s进行黑帮活动(偷窃)", currentuser.userid);
									sprintf(buf,"%s偷%s, 被警察没收%d兵马俑币(全部)", currentuser.userid, uident, money);
									if (money != 0)
										millionairesrec(title, buf, "黑帮活动");
									showAt(13, 4, "警察问话你还不老实,他一怒之下一把夺过你的钱包,扬长而去。", 0);
									showAt(14, 4, "你坐在地上大哭:\"警匪一家啊!我的钱,我的钱...\"", 1);
								} else {
									showAt(13, 4, "警察问话时你百般抵赖,到最后他也拿你没办法,只好把你放了.", 0);
									showAt(14, 4, "哈哈! 抗拒从严,回家过年", 1);
								}
							}
						} else {
							move(12, 4);
							if (random() % 2 == 0) {
								saveValue(currentuser.userid, "rob", 5, 50);
								prints_nofmt("你没命地逃跑,可惜,钱包丢在了路上...");
								money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
								saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
								sprintf(title, "%s进行黑帮活动(偷窃)", currentuser.userid);
								sprintf(buf,"%s偷%s, 逃跑中损失%d兵马俑币(全部)", currentuser.userid, uident, money);
								if (money != 0)
									millionairesrec(title, buf, "黑帮活动");
								pressanykey();
							} else {
								saveValue(currentuser.userid, "rob", -rob/2, 50);
								prints_nofmt("啊！你在逃跑的时候只听一声枪响...");
								set_safe_record();
								if (money / 200 < 3600)
									currentuser.dietime = currentuser.stay + 1000*60;
								else if (money < 10000000){
									mathtmp = (double)(money)/10000;
									mathtmp = 686.3455879296685 + 4.0492760356525315 * mathtmp + 0.004264378376417802 * mathtmp * mathtmp;//我拟合的二次函数
									currentuser.dietime = currentuser.stay + (int)(mathtmp * 60);//+(money / 200)
								} else{
									mathtmp = 9 + (double)(currentuser.lastlogin)/(double)(currentuser.stay + currentuser.lastlogin);
									currentuser.dietime = currentuser.stay +(int) (1000*mathtmp*60);
								}
								substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
								pressanykey();
								saveValue(currentuser.userid, MONEY_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
								sprintf(title, "%s进行黑帮活动(偷窃)", currentuser.userid);
								sprintf(buf,"%s偷%s, 被击毙, 损失%d兵马俑币(全部)", currentuser.userid, uident, money);
								if (money != 0)
									millionairesrec(title, buf, "黑帮活动");
								Q_Goodbye(0, NULL, NULL);
							}
						}
					}
					break;
				} else {
					move(11, 4);
					prints("\033[1;31m%s\033[m 把钱包看得紧紧的，你假装不小心撞了他一下,可一分钱都没偷到。", uident);
					pressanykey();
					break;
				}
				break;
			case '3':
				clear();
				money_show_stat("兵马俑黑帮养鸽场");
				showAt(4, 4, "黑帮为你提供勒索信件发送业务,每次收费视情形而定。", 0);
				if (currentuser.dietime > 0) {
					showAt(5, 4, "你已经死了啊！抓鬼啊！", 1);
					Q_Goodbye(0, NULL, NULL);
					break;
				}
				if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
					clear();
					showAt(5, 4, "    \033[1;32m  不要惹事\033[m", 1);
					break;
				}
				usercomplete("你要勒索谁:", uident);
				if (uident[0] == '\0')
					break;
				if (!(id = getuser(uident))) {
					showAt(7, 4, "错误的使用者代号...", 2);
					break;
				}
				if (lookupuser.dietime > 0) {
					showAt(7, 4, "鬼你也敢勒索啊...", 1);
					break;
				}
				move(8, 4);
				sprintf(genbuf, "确定要勒索么?");
				if (askyn(genbuf, NA, NA) == YEA) {
					money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
					if (money < 100) {
						showAt(9, 4, "您的钱不够。", 1);
						break;
					}
					if (money < 1000) {
						saveValue(currentuser.userid, MONEY_NAME, -100, MAX_MONEY_NUM);
						sprintf(title, "%s管你要几千块兵马俑币", currentuser.userid);
						mail_buf(letter1, uident, title);
					} else if (money < 100000) {
						saveValue(currentuser.userid, MONEY_NAME, -1000, MAX_MONEY_NUM);
						sprintf(title, "%s管你要几万块兵马俑币", currentuser.userid);
						mail_buf(letter2, uident, title);
					} else if (money < 10000000) {
						saveValue(currentuser.userid, MONEY_NAME, -100000, MAX_MONEY_NUM);
						sprintf(title, "%s管你要一百万兵马俑币", currentuser.userid);
						mail_buf(letter3, uident, title);
					} else {
						saveValue(currentuser.userid, MONEY_NAME, -500000, MAX_MONEY_NUM);
						sprintf(title, "%s管你要一千万兵马俑币", currentuser.userid);
						mail_buf(letter3, uident, title);
					}
					showAt(10, 4, "信发出去了，回去等消息吧。", 1);
				}
				break;
			case '4':
				clear();
				if (seek_in_file(DIR_MC "chayou", currentuser.userid)){
					showAt(5, 4, "    \033[1;32m  茶友不要惹事\033[m", 1);
					break;
				}
				if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
					showAt(5, 4, "    \033[1;32m  不要惹事\033[m", 1);
					break;
				}
				move(6, 4);
				usercomplete("抢谁？", uident);
				if (uident[0] == '\0')
					break;
				freeTime = loadValue(currentuser.userid, "freeTime", 2000000000);
				if (currentTime < freeTime){
					pressreturn();
					break;
				}
				if (!getuser(uident)) {
					showAt(7, 4, "错误的使用者代号...", 2);
					break;
				}
				if(seek_in_file(DIR_MC "mingren", uident)){
					showAt(7, 4, "      他有黄马褂，你还是算了吧\n", 1);
					break;
				}
				if (lookupuser.dietime > 0) {
					showAt(7, 4, "死人你也不放过，太狠了吧？", 1);
					break;
				}
				if(strcmp(lookupuser.userid,"BMYpolice")==0||strcmp(lookupuser.userid,"BMYbeg")==0||
						strcmp(lookupuser.userid,"BMYRober")==0||strcmp(lookupuser.userid,"BMYboss")==0||
						strcmp(lookupuser.userid,"BMYKillersky")==0){
					showAt(7, 4, "这个人是我亲戚，不许抢", 2);
					break;
				}
				money = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);

				if (currentuser.stay < 3600 + 86400) {
					showAt(7, 4, "小孩子家不要学坏了!", 2);
					break;
				}
				if (!clubtest("Rober")) {
					showAt(7, 4, "怎么看你也不像是作奸犯科的人啊！", 2);
					break;
				}
				getdata(7, 4, "请输入你的密码: ", buf, PASSLEN, NOECHO, YEA);
				if (*buf == '\0' || !ytht_crypt_checkpasswd(currentuser.passwd, buf)) {
					showAt(8, 4, "很抱歉, 您输入的密码不正确。", 2);
					break;
				}
				saveValue(currentuser.userid, "last_rob", -2000000000, 2000000000);
				saveValue(currentuser.userid, "last_rob", time(0), 2000000000);
				showAt(9, 4, "\033[1;5;31m警告\033[0;1;31m： 小心啊，最近警署在严打哦！", 0);
				move(10, 4);
				if (askyn("真的要抢么？", NA, NA) == NA)
					break;
				set_safe_record();
				if (currentuser.dietime > 0) {
					showAt(11, 4, "你已经死了啊！抓鬼啊！", 1);
					Q_Goodbye(0, NULL, NULL);
					break;
				}
				if (money_check_guard()) {
					pressanykey();
					break;
				}
				if (lookupuser.dietime > 0) {
					showAt(11, 4, "人都死了,让他安息吧.", 1);
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
						prints_nofmt("你干掉了他一个保镖");
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
						prints("\033[1;31m%s\033[m 的门没锁，你溜了进去,找出存折, 换得 %d 兵马俑币现金，快跑吧。", uident, money);
						sprintf(buf,
								"%s 趁您不注意的时候拿了你家的存折,等你发现挂失的时候已经损失了 %d 兵马俑币。",
								currentuser.userid, money);
						sprintf(title, "对不起，您被抢劫");
						if(Allclubtest(uident)||loadValue(uident, "mail", 8))
							mail_buf(buf, uident, title);
						sprintf(title, "%s进行黑帮活动(抢劫)", currentuser.userid);
						sprintf(buf,"%s抢%s  %d兵马俑币", currentuser.userid, uident, money);
						millionairesrec(title, buf, "黑帮活动");
						pressanykey();
						break;
					} else {
						money = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);
						r = random() % 70;
						money = money / 100 * r;
						saveValue(currentuser.userid, CREDIT_NAME, -money, MAX_MONEY_NUM);
						saveValue(uident, CREDIT_NAME, money, MAX_MONEY_NUM);
						move(11, 4);
						prints("\033[1;31m你溜进了 %s 的门,正得意呢,抬眼看见黑洞洞的枪口对着你...", uident);
						move(12, 4);
						prints("\033[1;31m唉呀呀,没想到他在家,你被迫私了,从存折里取出 %d 兵马俑币给他。", money);
						sprintf(title, "您遭遇抢劫");
						sprintf(buf, "%s 想抢你的钱,结果让你发现了,你勒索了他 %d 兵马俑币,送上门的肥肉啊。", currentuser.userid, money);
						if(Allclubtest(uident)||loadValue(uident, "mail", 8))
							mail_buf(buf, uident, title);
						sprintf(title, "%s进行黑帮活动(抢劫)", currentuser.userid);
						sprintf(buf,"%s抢%s , 反被勒索%d兵马俑币", currentuser.userid, uident, money);
						millionairesrec(title, buf, "黑帮活动");
						pressanykey();
						break;
					}

				} else if (r < 90) {
					money = loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
					rob = loadValue(currentuser.userid, "rob", 50);
					move(11, 4);
					if (rob > 20) {
						saveValue(currentuser.userid, "rob", -rob/2, 50);
						prints_nofmt("啊！有警察，你在逃跑的时候只听一声枪响...");
						set_safe_record();
						if (money / 200 < 3600)
							currentuser.dietime = currentuser.stay + 1000*60;
						else if (money < 10000000){
							mathtmp = (double)(money)/10000;
							mathtmp = 686.3455879296685 + 4.0492760356525315 * mathtmp + 0.004264378376417802 * mathtmp * mathtmp;//我拟合的二次函数
							currentuser.dietime = currentuser.stay + (int)(mathtmp * 60);//+(money / 200)
						}else{
							mathtmp = 9 + (double)(currentuser.lastlogin)/(double)(currentuser.stay + currentuser.lastlogin);
							currentuser.dietime = currentuser.stay +(int) (1000*mathtmp*60);
						}
						substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
						saveValue(currentuser.userid, MONEY_NAME, -MAX_MONEY_NUM, MAX_MONEY_NUM);
						pressanykey();
						Q_Goodbye(0, NULL, NULL);
					} else {
						if (askyn("被警察发现了,你要逃跑么?", YEA, NA) == NA) {
							saveValue(currentuser.userid, "rob", 1, 50);
							money = loadValue(currentuser.userid, MONEY_NAME,  MAX_MONEY_NUM);
							saveValue(currentuser.userid, MONEY_NAME, -money * 50 / 100, MAX_MONEY_NUM);
							sprintf(title, "%s进行黑帮活动(抢劫)", currentuser.userid);
							sprintf(buf,"%s抢%s 被警察没收%d兵马俑币", currentuser.userid, uident, money/2);
							millionairesrec(title, buf, "黑帮活动");
							showAt(12, 4, "你被带到警察局,在没收了身上所有的钱之后,现在等警察给你训话", 0);
							showAt(13, 4, "现在是警察给你的15秒钟训话时间，硬着头皮听吧。", 1);
							sleep(15);
							money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
							sprintf(genbuf,
									"出了警察局了,你高兴的从鞋里掏出藏起来的%d兵马俑币。呜呜,一股臭脚丫子味...",
									money);
							showAt(14, 4, genbuf, 1);
						} else {
							move(12, 4);
							if (random() % 2 == 0) {
								saveValue(currentuser.userid, "rob", 5, 50);
								prints_nofmt("逃跑成功,可惜,你的钱包丢在了路上...");
								saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
								sprintf(title, "%s进行黑帮活动(抢劫)", currentuser.userid);
								sprintf(buf,"%s抢%s, 逃跑损失%d兵马俑币(全部)", currentuser.userid, uident, money);
								millionairesrec(title, buf, "黑帮活动");
								pressanykey();
							} else {
								saveValue(currentuser.userid, "rob", -rob/2, 50);
								prints_nofmt("啊！你在逃跑的时候只听一声枪响...");
								set_safe_record();
								if (money / 200 < 3600)
									currentuser.dietime = currentuser.stay + 1000*60;
								else if (money < 10000000){
									mathtmp = (double)(money)/10000;
									mathtmp = 686.3455879296685 + 4.0492760356525315 * mathtmp + 0.004264378376417802 * mathtmp * mathtmp;//我拟合的二次函数
									currentuser.dietime = currentuser.stay + (int)(mathtmp * 60);//+(money / 200)
								}else{
									mathtmp = 9 + (double)(currentuser.lastlogin)/(double)(currentuser.stay + currentuser.lastlogin);
									currentuser.dietime = currentuser.stay +(int) (1000*mathtmp*60);
								}
								substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
								pressanykey();
								saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
								sprintf(title, "%s进行黑帮活动(抢劫)", currentuser.userid);
								sprintf(buf,"%s抢%s, 被击毙, 损失%d兵马俑币(全部)", currentuser.userid, uident, money);
								millionairesrec(title, buf, "黑帮活动");
								Q_Goodbye(0, NULL, NULL);
							}
						}
					}
					break;
				} else {
					move(11, 4);
					prints("\033[1;31m%s\033[m 家的门锁的紧紧的，你假装路过,看看无法得手,只好离开。", uident);
					pressanykey();
					break;
				}
				break;
			case '5':
				nomoney_show_stat("黑帮帮主办公室");
				whoTakeCharge2(4, buf);
				whoTakeCharge(4, uident);
				if (strcmp(currentuser.userid, uident)) {
					move(6, 4);
					prints("秘书%s拦住了你,说道:“老大%s现在很忙,没时间接待你。”", buf,uident);
					move(8,4);
					if(!slowclubtest("Rober",currentuser.userid)){
						if (askyn("你是想加入黑帮吗？", NA, NA) == YEA) {
							sprintf(genbuf, "%s 要加入黑帮", currentuser.userid);
							mail_buf(genbuf, "BMYRober", genbuf);
							move(14, 4);
							prints_nofmt("好了，我会通知老大的");
						}
					}
					pressanykey();
					break;
				} else {
					move(6, 4);
					prints_nofmt("请选择操作代号:");
					move(7, 6);
					prints_nofmt("5. 辞职                      6. 退出");
					ch2 = igetkey();
					switch (ch2) {
						case '5':
							move(12, 4);
							if (askyn("您真的要辞职吗？", NA,NA) == YEA) {
								/*	ytht_del_from_file(MC_BOSS_FILE,"gang");
									sprintf(genbuf, "%s 宣布辞去黑帮帮主职务", currentuser.userid);
									deliverreport(genbuf,
									"兵马俑金融中心对其一直以来的工作表示感谢，祝以后顺利！");
									move(14, 4);
									prints
									("好吧，既然你意已决，中心也只有批准。");
									quit = 1;
									pressanykey();
									*/
								sprintf(genbuf, "%s 要辞去黑帮帮主职务", currentuser.userid);
								mail_buf(genbuf, "millionaires", genbuf);
								move(14, 4);
								prints_nofmt("好吧，已经发信告知总管了");
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

/*丐帮*/
static int money_beggar() {
	int ch,ch2;
	int quit = 0;
	char uident[IDLEN + 1], buf[STRLEN], title[40];
	int money, credit, num;
	int id;
	while (!quit) {
		money_show_stat("丐帮总舵");
		move(4, 4);
		prints_nofmt("丐帮自古天下第一大帮，不过目前经济还算景气，做乞丐的人也不多啦。");
		move(5, 4);
		prints_nofmt("一个乞丐走过来问道：“要打听消息么？丐帮天上地下无所不知，无所不晓。”");
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1]打探 [2]烧钱 [3]跟踪 [4]乞讨 [5]丐帮帮主 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
			case '1':
				move(6, 4);
				usercomplete("查谁的家底？", uident);
				if (uident[0] == '\0')
					break;
				if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
					move(7, 4);
					prints_nofmt("错误的使用者代号...");
					pressreturn();
					break;
				}
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);

				if (money < 1000) {
					showAt(7, 4, "啊，你只带了这么点钱吗？", 0);
					showAt(8, 4, "那乞丐接过钱转身就走了，再也没了下文。", 1);
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
				prints("\033[1;31m%s\033[m 大约有 \033[1;31m%d\033[m 兵马俑币的现金，以及 \033[1;31m%d\033[m 兵马俑币的存款。", uident, money, credit);
				pressanykey();
				break;
			case '2':
				clear();
				money_show_stat("丐帮神庙");
				move(4, 4);
				prints_nofmt("烧钱最小金额 1000 兵马俑币。可买通冥间管事，让死者复活。");
				move(5, 4);
				usercomplete("给谁烧？", uident);
				if (uident[0] == '\0')
					break;
				if (!(id = getuser(uident))) {
					move(6, 4);
					prints_nofmt("错误的使用者代号...");
					pressreturn();
					break;
				}
				if (!seek_in_file(MC_ADMIN_FILE, currentuser.userid) &&
						((lookupuser.dietime- lookupuser.stay) > 10000*60) ) {//5000->10000
					showAt(6, 4, "自杀的人怎么复活？死了就放心的去吧！！！", 1);
					break;
				}
				getdata(6, 4, "您打算烧多少兵马俑币？[0]", genbuf, 10, DOECHO, YEA);
				num = atoi(genbuf);
				if (num < 1000) {
					showAt(7, 4, "那么点钱，怎么贿赂冥间管事啊？", 1);
					break;
				}
				move(7, 4);
				sprintf(genbuf, "您确认给 %s 烧 %d 兵马俑币？", uident, num);
				if (askyn(genbuf, NA, NA) == YEA) {
					money = loadValue(currentuser.userid, MONEY_NAME,  MAX_MONEY_NUM);
					if (money < num) {
						showAt(8, 4, "您的钱不够", 1);
						break;
					}
					saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
					saveValue("millionaires", MONEY_NAME, +num/2, MAX_MONEY_NUM);
					saveValue("BMYboss", MONEY_NAME, +num/2, MAX_MONEY_NUM);
					if (lookupuser.dietime == 2 || lookupuser.dietime == 0) {
						showAt(8, 4, "啊！不是死鬼，白烧了...", 1);
						break;
					}
					if (seek_in_file(MC_ADMIN_FILE, currentuser.userid) &&
							((lookupuser.dietime- lookupuser.stay) > 5000*60)){
						sprintf(title,"%s行使烧钱特权", currentuser.userid);
						sprintf(buf,"%s给%s烧了%d(/60=%d)兵马俑币", currentuser.userid, uident, num, num / 60);
						millionairesrec(title, buf, "");
					}else{
						sprintf(title,"%s给%s烧钱", currentuser.userid, uident);
						sprintf(buf,"%s给%s烧了%d(/60=%d)兵马俑币", currentuser.userid, uident, num, num / 60);
						millionairesrec(title, buf, "烧钱");
					}
					if (lookupuser.dietime > lookupuser.stay)
						lookupuser.dietime -= num;
					if (lookupuser.dietime <= lookupuser.stay)
						lookupuser.dietime = 2;
					substitute_record(PASSFILE, &lookupuser, sizeof (lookupuser), id);
					showAt(8, 4, "烧完了，走吧。", 1);
					sprintf(title, "您的朋友 %s 给您送钱来了", currentuser.userid);
					sprintf(buf, "您的朋友 %s 给您烧了点钱，您的死期缩短了%d分钟", currentuser.userid, num / 60);
					mail_buf(buf, uident, title);
					pressanykey();
				}
				break;
			case '3':
				move(6, 4);
				usercomplete("要跟踪谁？", uident);
				if (uident[0] == '\0')
					break;
				if (!getuser(uident)) {
					showAt(7, 4, "错误的使用者代号...", 2);
					break;
				}
				money =  loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				if (money < 1000) {
					showAt(7, 4, "你身上这点钱还不够跑路费啊。", 0);
					showAt(8, 4, "那乞丐接过钱转身就走了，再也没了下文。", 1);
					break;
				}
				saveValue(currentuser.userid, MONEY_NAME, -1000, MAX_MONEY_NUM);
				saveValue("BMYbeg", MONEY_NAME, 500, MAX_MONEY_NUM);
				saveValue("millionaires", MONEY_NAME, 500, MAX_MONEY_NUM);
				move(7, 4);
				prints_nofmt("几天后，你收到丐帮的消息说：");
				move(8, 4);
				prints("\033[1;31m%s\033[m 有 \033[1;31m%s\033[m 的地位，以及 \033[1;31m%s\033[m 一般的才艺。",
					uident, charexp(countexp(&lookupuser)), cperf(countperf(&lookupuser)));
				pressanykey();
				break;
			case '4':
				clear();
				if (seek_in_file(DIR_MC "chayou", currentuser.userid)){
					showAt(5, 4, "    \033[1;32m  茶友不要惹事\033[m", 1);
					break;
				}
				if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
					showAt(5, 4, "    \033[1;32m  不要惹事\033[m", 1);
					break;
				}
				money_show_stat("兵马俑小区");
				showAt(4, 4, "这里是兵马俑的富人区，乞讨的好地方。", 0);
				move(6, 4);
				usercomplete("向谁乞讨？", uident);
				if (uident[0] == '\0')
					break;
				if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
					showAt(7, 4, "错误的使用者代号...", 2);
					break;
				}
				if (!getuser(uident)) {
					showAt(7, 4, "错误的使用者代号...", 2);
					break;
				}
				if(seek_in_file(DIR_MC "mingren", uident)){
					showAt(7, 4, "      他有黄马褂，你还是算了吧\n", 1);
					break;
				}
				if (!clubtest("Beggar")) {
					showAt(7, 4, "怎么看你也不像是丐帮的啊！", 1);
					break;
				}
				if (lookupuser.dietime>0) {
					showAt(7, 4, "人都死了,让他安息吧.", 1);
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
				if (loadValue(currentuser.userid, "begtime", 2000000000) >=12) {
					if(time(0) > 24*3600 + loadValue(currentuser.userid, "last_beg", 2000000000)){
						saveValue(currentuser.userid, "begtime", -12, 2000000000);
						saveValue(currentuser.userid, "last_beg", time(0), 2000000000);
						saveValue(currentuser.userid, "begtime", +1, 2000000000);
					}else
						prints("%s怒不可遏，冲你骂道：“臭要饭的，烦死了，还不快滚！”", uident);
					pressanykey();
					break;
				}
				saveValue(currentuser.userid, "begtime", +1, 2000000000);
				if (!t_search(uident, NA, 1)) {
					if (random() % 5 == 0) {
						prints("你对着%s哭喊道：“可怜可怜我吧，还有我的小强！呜呜呜...”", uident);
						//num = (random() % (1 + 100))*10000 + 500000;
						if(flag==1)
							saveValue(uident, MONEY_NAME, -num, MAX_MONEY_NUM);
						else
							saveValue(uident, CREDIT_NAME, -num, MAX_MONEY_NUM);
						saveValue(currentuser.userid, MONEY_NAME, num, MAX_MONEY_NUM);

						sprintf(title, "%s参与丐帮活动", currentuser.userid);
						sprintf(buf, "%s乞讨了%s %d兵马俑币", currentuser.userid, uident, num);
						if (num != 0)
							millionairesrec(title, buf, "丐帮活动");

						move(8, 4);
						prints("%s眼圈顿时红了，赶紧从身上拿出 %d 兵马俑币给你。", uident, num);
						sprintf(genbuf, "你一时好心，给了%s %d兵马俑币，过后想想真不是滋味。", currentuser.userid, num);
						if(Allclubtest(uident)||loadValue(uident, "mail", 8))
							mail_buf(genbuf, uident, "你遇到叫花子");
						pressanykey();
					} else {
						prints("你对着%s哭泣道：“官人，我要！”", uident);
						move(8, 4);
						prints("%s一脚把你踹了出来。", uident);
						pressanykey();
					}
				}

				else {
					int begmoney= loadValue(uident, MONEY_NAME, MAX_MONEY_NUM);
					if (seek_in_file(DIR_MC "gongji", uident)){
						if(random() % 3 == 0){
							saveValue(uident, MONEY_NAME, -begmoney, MAX_MONEY_NUM);
							saveValue(currentuser.userid, MONEY_NAME, begmoney, MAX_MONEY_NUM);

							sprintf(title, "%s参与丐帮活动", currentuser.userid);
							sprintf(buf, "%s乞讨了%s %d兵马俑币", currentuser.userid, uident, begmoney);
							if (begmoney != 0)
								millionairesrec(title, buf, "丐帮活动");

							prints("%s眼圈顿时红了，赶紧从身上拿出所有的兵马俑币一共 %d 给你。", uident, num);
							sprintf(genbuf, "你一时好心，给了%s %d兵马俑币，过后想想真不是滋味。", currentuser.userid, num);
							if(Allclubtest(uident)||loadValue(uident, "mail", 8))
								mail_buf(genbuf, uident, "你遇到叫花子");
							pressanykey();
						}
					}

					if (random() % 3 == 0) {
						prints("你对着%s哭喊道：“可怜可怜我吧，还有我的小强！呜呜呜...”", uident);
						//num = (random() % (1 + 100))*10000 + 500000;
						if(flag==1)
							saveValue(uident, MONEY_NAME, -num, MAX_MONEY_NUM);
						else
							saveValue(uident, CREDIT_NAME, -num, MAX_MONEY_NUM);
						saveValue(currentuser.userid, MONEY_NAME, num, MAX_MONEY_NUM);

						sprintf(title, "%s参与丐帮活动", currentuser.userid);
						sprintf(buf, "%s乞讨了%s %d兵马俑币", currentuser.userid, uident, num);
						if (num != 0)
							millionairesrec(title, buf, "丐帮活动");

						move(8, 4);
						prints("%s眼圈顿时红了，赶紧从身上拿出 %d 兵马俑币给你。", uident, num);
						sprintf(genbuf, "你一时好心，给了%s %d兵马俑币，过后想想真不是滋味。", currentuser.userid, num);
						if(Allclubtest(uident)||loadValue(uident, "mail", 8))
							mail_buf(genbuf, uident, "你遇到叫花子");
						pressanykey();
					} else {
						prints("你对着%s哭泣道：“官人，我要！”", uident);
						move(8, 4);
						prints("%s一脚把你踹了出来。", uident);
						pressanykey();
					}
				}
				break;
			case '5':
				nomoney_show_stat("丐帮帮主办公室");
				whoTakeCharge2(5, buf);
				whoTakeCharge(5, uident);
				if (strcmp(currentuser.userid, uident)) {
					move(6, 4);
					prints
						("秘书%s拦住了你,说道:“老大%s现在很忙,没时间接待你。”", buf,uident);
					move(8,4);
					if(!slowclubtest("Beggar",currentuser.userid)){
						if (askyn("你是想加入丐帮吗？", NA, NA) == YEA) {
							sprintf(genbuf, "%s 要加入丐帮", currentuser.userid);
							mail_buf(genbuf, "BMYbeg", genbuf);
							move(14, 4);
							prints_nofmt("好了，我会通知老大的");
						}}
					pressanykey();
					break;
				} else {
					move(6, 4);
					prints_nofmt("请选择操作代号:");
					move(7, 6);
					prints_nofmt("5. 辞职                      6. 退出");
					ch2 = igetkey();
					switch (ch2) {
						case '5':
							move(12, 4);
							if (askyn("您真的要辞职吗？", NA,NA) == YEA) {
								/*	ytht_del_from_file(MC_BOSS_FILE,"beggar");
									sprintf(genbuf, "%s 宣布辞去丐帮帮主职务", currentuser.userid);
									deliverreport(genbuf,
									"兵马俑金融中心对其一直以来的工作表示感谢，祝以后顺利！");
									move(14, 4);
									prints
									("好吧，既然你意已决，中心也只有批准。");
									quit = 1;
									pressanykey();
									*/
								sprintf(genbuf, "%s 要辞去丐帮帮主职务", currentuser.userid);
								mail_buf(genbuf, "millionaires", genbuf);
								move(14, 4);
								prints_nofmt("好吧，已经发信告知总管了");
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

/*杀手rewrite by macintosh 20051204*/
static int money_killer() {
	int ch,ch2;
	int guard_num;
	int robTimes;
	int x,y;
	int quit = 0;
	int quit2=0;
	int count=0;
	time_t freeTime;
	time_t currentTime = time(0);
	char uident[IDLEN + 1], name[IDLEN + 1], buf[STRLEN];
	int money,num;
	int id;
	char c4_price[10];
	int price;
	while (!quit) {
		quit2=0;
		nomoney_show_stat("杀手天空");
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1]雇佣杀手 [2]军火 [3]杀手帮主 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
			case '1':
				if (seek_in_file(DIR_MC "chayou", currentuser.userid)){
					showAt(5, 4, "    \033[1;32m  茶友不要惹事\033[m", 1);
					break;
				}
				if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
					showAt(5, 4, "    \033[1;32m  黄马褂不要惹事\033[m", 1);
					break;
				}
				money_show_stat("杀手之家");
				readstrvalue(MC_PRICE_FILE, "c4_price", c4_price, 10);
				price = atoi(c4_price);
				if (price==0)
					price=300000;
				move(4, 4);
				prints("我们这里杀一次 %d 兵马俑币。", price);
				move(5, 4);
				prints_nofmt("每个人每天只能杀一次，每次最多可以买杀他三次。");
				if (currentuser.dietime > 0) {
					showAt(7, 4, "你已经死了啊！抓鬼啊！", 1);
					Q_Goodbye(0, NULL, NULL);
					break;
				}
				move(6, 4);
				usercomplete("你要杀谁:", uident);
				if (uident[0] == '\0')
					break;
				if (!(id = getuser(uident))) {
					showAt(7, 4, "错误的使用者代号...", 2);
					break;
				}
				if (lookupuser.dietime > 0) {
					showAt(7, 4, "死人你也不放过，太狠了吧？", 1);
					break;
				}
				if(seek_in_file(DIR_MC "mingren", uident)){
					showAt(7, 4, "他有黄马褂，你还是算了吧", 1);
					break;
				}
				if(!Allclubtest(uident)){
					showAt(7, 4, "杀手不杀无辜百姓...", 1);
					break;
				}
				getdata(7, 4, "你要杀几次？ [1-3]", genbuf, 2, DOECHO, YEA);
				if (genbuf[0] == '\0')
					break;
				count = atoi(genbuf);
				if (count < 1) {
					showAt(8, 4, "狠不下心动手了？", 1);
					break;
				}
				if (count > 3) {
					move(8, 4);
					sprintf(genbuf, "要杀%d次？杀手最多杀3次，他要把钱私吞了", count);
					if (askyn(genbuf, NA, NA) == NA){
						showAt(9, 4, "重新想想吧？", 1);
						break;
					}
				}
				move(8, 4);
				num = count * price;
				sprintf(genbuf, "总共需要 %d 兵马俑币。", num);
				if (askyn(genbuf, NA, NA) == YEA) {
					money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
					if (money < num) {
						showAt(9, 4, "你的钱不够...", 1);
						break;
					}
					saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
					saveValue("BMYKillersky", MONEY_NAME, num, MAX_MONEY_NUM);
					sprintf(buf,"%s花了%d兵马俑币要杀%s%d次",currentuser.userid,num,uident, count);
					mail_buf(buf, "BMYKillersky","[任务]杀手有生意了");

					if (seek_in_file(DIR_MC "killerlist", uident)){
						FILE *fp;
						char *ptr;
						int count2=0;
						fp = fopen(DIR_MC "killerlist","r");
						if (fp) {
							while (fgets(buf,sizeof(buf),fp)) {
								ptr= strstr(buf,uident);
								if(ptr){
									count2 = atoi(ptr+strlen(uident)+1);
									break;
								}
							}
							fclose(fp);
						}
						if (count2+count>3)
							count2 = 3;
						else
							count2 += count;
						ytht_del_from_file(DIR_MC "killerlist", uident, true);
						sprintf(buf, "%s\t%d",uident, count2);
						ytht_add_to_file(DIR_MC "killerlist",buf);
					}else{
						sprintf(buf, "%s\t%d",uident, (count>3)?3:count);
						ytht_add_to_file(DIR_MC "killerlist",buf);
					}
					showAt(10, 4, "您已经成功购买了这个人的人头，请静候佳音", 1);
				}
				break;

			case '2':
				while (!quit2) {
					nomoney_show_stat("地下军火交易市场");
					move(t_lines - 1, 0);
					prints
						("\033[1;44m 选单 \033[1;46m [1]c4  [Q]离开\033[m");
					ch2 = igetkey();
					switch (ch2) {
						case 'q':
						case 'Q':
							quit2 = 1;
							break;
						case '1':
							if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
								showAt(5, 4, "    \033[1;32m  不要惹事\033[m", 1);
								break;
							}
							if (!seek_in_file(DIR_MC "killer", currentuser.userid)
									||!slowclubtest("killer", currentuser.userid)){
								showAt(7, 4, "\033[1;31m要拼命去找杀手\033[m", 1);
								break;
							}
							if (loadValue(currentuser.userid, "guard", 8) > 0) {
								showAt(7, 4, "你总不能带着兄弟一起死吧，^_^", 1);
								break;
							}
							showAt(4, 4,"\033[1;35m你决定发动自杀式攻击。\033[m", 0);
							money =loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
							if (money <10000) {
								showAt(9, 4, "您的钱不够...", 1);
								break;
							}
							if (currentuser.dietime > 0) {
								showAt(9, 4, "你已经死了啊！抓鬼啊！", 1);
								Q_Goodbye(0, NULL, NULL);
								break;
							}
							usercomplete("你要炸谁:", uident);
							if (uident[0] == '\0')
								break;
							if (!(id = getuser(uident))) {
								showAt(7, 4,"错误的使用者代号...", 1);
								break;
							}
							if (lookupuser.dietime > 0) {
								showAt(7, 4,"死人你也不放过，太狠了吧？", 1);
								break;
							}
							if (seek_in_file(DIR_MC "mingren", uident)){
								showAt(7, 4, "      他有黄马褂，你还是算了吧\n", 1);
								break;
							}
							if (!Allclubtest(uident)){
								showAt(7, 4, "    \033[1;32m  不要残杀无辜！\033[m", 1);
								break;
							}

							guard_num =loadValue(uident, "guard", 8);
							if (guard_num > 0) {
								showAt(7, 4, "对方有保镖护身,你还是算了吧...", 1);
								break;
							}

							freeTime = loadValue(currentuser.userid, "freeTime", 2000000000);
							if (currentTime < freeTime){
								pressreturn();
								break;
							}
							saveValue(currentuser.userid, MONEY_NAME, -100000, MAX_MONEY_NUM);
							move(6, 4);
							prints("  \n\033[1;35m  你抱起炸药包，大喊一声打倒小日本,向%s冲了过去\033[m\n", uident);
							sprintf(genbuf, "本港人士%s于10分钟前在九龙的\n一起自杀式攻击中身亡\n警方怀疑此人有帮会背景\n\n"
									"据一位不愿透露姓名的警署官员透露\n这次事件可能是职业杀手所为", uident);
							x = countexp(&currentuser);
							y = countexp(&lookupuser);
							robTimes = loadValue(currentuser.userid, "rob", 50);
							saveValue(currentuser.userid, "rob", -robTimes, 50);
							if(random()/x>(random()/y)/3||(random() % 3==0)){
								lookupuser.dietime = lookupuser.stay + 4500 * 60;
								substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
								deliverreport("[新闻]本站发生自杀攻击",genbuf);
								mail_buf_slow(uident,	 "你挂了","有人对你发动了自杀式攻击。","BMYKillersky");
								sprintf(buf,"对 %s 发动了自杀式攻击",uident);
								mail_buf(buf, "BMYKillersky","任务完成");
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
									ytht_del_from_file(DIR_MC "killerlist", uident, true);
									if (count2==2 || count2==3){
										sprintf(buf, "%s\t%d",uident, count2-1);
										ytht_add_to_file(DIR_MC "killerlist",buf);
									}
								}
							}
							set_safe_record();
							currentuser.dietime = currentuser.stay +1000 * 60;
							substitute_record (PASSFILE, &currentuser, sizeof(currentuser), usernum);
							pressanykey();
							Q_Goodbye(0, NULL, NULL);
					}
					limit_cpu();
				}
				break;

			case '3':
				nomoney_show_stat("杀手帮主办公室");
				whoTakeCharge2(9, name);
				whoTakeCharge(9, uident);
				if (strcmp(currentuser.userid, uident)) {
					move(6, 4);
					prints("秘书%s拦住了你,说道:“老大%s现在很忙,没时间接待你。”", name,uident);
					move(8,4);
					if (!seek_in_file(DIR_MC "killer", currentuser.userid) &&
							!slowclubtest("killer",currentuser.userid)){
						if (askyn("你是想成为杀手吗？", NA, NA) == YEA) {
							sprintf(genbuf, "%s 要加入杀手", currentuser.userid);
							mail_buf(genbuf, "BMYKillersky", genbuf);
							move(14, 4);
							prints_nofmt("好了，我会通知老大的");
						}
					}
					pressanykey();
					break;
				} else {
					move(6, 4);
					prints_nofmt("请选择操作代号:");
					move(7, 6);
					prints_nofmt("1. 任命杀手                  2. 解职杀手");
					move(8, 6);
					prints_nofmt("3. 杀手名单                  4. 任务名单");
					move(9, 6);
					prints_nofmt("5. 辞职                      6. c4定价");
					move(10, 6);
					prints_nofmt("7. 退出");
					ch2 = igetkey();
					switch (ch2) {
						case '1':
							move(12, 4);
							usercomplete("任命谁为杀手？", uident);
							move(13, 4);
							if (uident[0] == '\0')
								break;
							if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
								prints_nofmt("错误的使用者代号...");
								pressanykey();
								break;
							}
							if (seek_in_file(DIR_MC "killer", uident)) {
								prints_nofmt("该ID已经是杀手了。");
								pressanykey();
								break;
							}
							if (askyn("确定吗？", NA, NA) == YEA) {
								ytht_add_to_file(DIR_MC "killer", uident);
								sprintf(genbuf, "%s 任命你为杀手",currentuser.userid);
								mail_buf("希望你不辜负大家的希望，完成任务！",uident, genbuf);
								move(14, 4);
								prints_nofmt("任命成功。");
								sprintf(genbuf, "%s行使杀手管理权限",currentuser.userid);
								sprintf(buf, "任命%s为杀手", uident);
								millionairesrec(genbuf, buf, "BMYKillersky");
								pressanykey();
							}
							break;
						case '2':
							move(12, 4);
							usercomplete("解职哪位杀手？", uident);
							move(13, 4);
							if (uident[0] == '\0')
								break;
							if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
								prints_nofmt("错误的使用者代号...");
								pressanykey();
								break;
							}
							if (!seek_in_file(DIR_MC "killer", uident)) {
								prints_nofmt("该ID不是杀手。");
								pressanykey();
								break;
							}
							if (askyn("确定吗？", NA, NA) == YEA) {
								ytht_del_from_file(DIR_MC "killer", uident, true);
								sprintf(genbuf, "%s 解除你的杀手职务", currentuser.userid);
								mail_buf("感谢你完成任务。", uident, genbuf);
								move(14, 4);
								prints_nofmt("解职成功。");
								sprintf(genbuf, "%s行使杀手管理权限",currentuser.userid);
								sprintf(buf, "解除%s的杀手职务", uident);
								millionairesrec(genbuf, buf, "BMYKillersky");
								pressanykey();
							}
							break;
						case '3':
							clear();
							move(1, 0);
							prints_nofmt("目前兵马俑杀手名单：");
							listfilecontent(DIR_MC "killer");
							pressanykey();
							break;
						case '4':
							clear();
							move(1, 0);
							prints_nofmt("目前兵马俑追杀名单：");
							move(2, 0);
							prints_nofmt("目标ID\t次数");
							listfilecontent(DIR_MC "killerlist");
							pressanykey();
							break;
						case '5':
							move(12, 4);
							if (askyn("您真的要辞职吗？", NA,NA) == YEA) {
								/*	ytht_del_from_file(MC_BOSS_FILE,"killer");
									sprintf(genbuf, "%s 宣布辞去杀手帮主职务", currentuser.userid);
									deliverreport(genbuf,
									"兵马俑金融中心对其一直以来的工作表示感谢，祝以后顺利！");
									move(14, 4);
									prints
									("好吧，既然你意已决，中心也只有批准。");
									quit = 1;
									pressanykey();
									*/
								sprintf(genbuf, "%s 要辞去杀手帮主职务", currentuser.userid);
								mail_buf(genbuf, "millionaires", genbuf);
								move(14, 4);
								prints_nofmt("好吧，已经发信告知总管了");
								pressanykey();
							}
							break;
						case '6':
							move(12, 4);
							readstrvalue(MC_PRICE_FILE, "c4_price", c4_price, 10);
							price = atoi(c4_price);
							prints("现在的价格是%d", price ? price : 300000);
							getdata(13, 4, "设定新的价格: ", buf, 10, DOECHO, YEA);
							move(14, 4);
							sprintf(genbuf, "新的价格是 %d，确定吗？", atoi(buf));
							if (askyn(genbuf, NA, NA) == YEA) {
								if (atoi(buf)>MAX_MONEY_NUM){
									move(15, 4);
									prints_nofmt("不要太狠了...");
									pressanykey();
									sprintf(buf, "%d", MAX_MONEY_NUM);
								}
								savestrvalue(MC_PRICE_FILE, "c4_price", buf);
								move(15, 4);
								prints_nofmt("设置完毕。    ");
								sprintf(genbuf, "设置c4价格为%s。", buf);
								sprintf(buf, "%s行使杀手管理权限", currentuser.userid);
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

static int money_postoffice() {
	int ch2, slownum=0;

	nomoney_show_stat("大富翁邮件设置");
	slownum=loadValue(currentuser.userid, "mail", 8);
	move(6, 4);
	if(Allclubtest(currentuser.userid)){
		prints_nofmt("帮派人士就不要管这么多了!");
		pressanykey();
		return 0;
	}
	if (slownum==0){
		prints_nofmt("您尚未开通兵马俑邮局邮件服务，不能收到各大帮派给您的信件。");
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1]开通服务 [Q]离开\033[m");
	}
	else{
		prints_nofmt("您已经启用了兵马俑邮局的邮件服务，我们将在第一时间将各大帮派");
		move(7, 4);
		prints_nofmt("给您的信件递给您。");
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1]取消服务 [Q]离开\033[m");
	}
	ch2 = igetkey();
	switch (ch2) {
		case '1':
			if(slownum==0){
				saveValue(currentuser.userid, "mail", 1, 50);
				nomoney_show_stat("大富翁邮件设置");
				move(6, 4);
				prints_nofmt("欢迎使用兵马俑邮局邮件服务系统，我们将在第一时间将各大帮派");
				move(7, 4);
				prints_nofmt("给您的信件递到您的信箱。再见。");
			}else{
				saveValue(currentuser.userid, "mail", -slownum, 50);
				nomoney_show_stat("大富翁邮件设置");
				move(6,4);
				prints_nofmt("欢迎您下次继续使用本邮局的各项服务，谢谢您的光顾，再见。");
			}
			pressanykey();
			break;

		case 'q':
		case 'Q':
			break;
	}
	return 0;
}

/*商场rewrite by macintosh 20051204*/
static int money_shop() {
	int ch, money, num, ch2;
	int guard_num;
	char uident[IDLEN + 1], ticket_price[10], buf[STRLEN];
	int quit = 0, quit2= 0, price=0;

	while (!quit) {
		quit2=0;
		nomoney_show_stat("兵马俑商场");
		move(6, 4);
		prints_nofmt("兵马俑商场最近生意红火，大家尽兴！");
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1]雇保镖 [2]礼品店 [3]经理室 [4]邮局 [6]火车票价计算 [Q]离开\033[m");
		// ("\033[1;44m 选单 \033[1;46m [1]雇保镖 [2]贺卡 [4]经理室 [5]hell参观 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
			case '1':
				nomoney_show_stat("兵马俑保镖公司");
				move(4, 4);
				prints_nofmt("兵马俑保镖公司对有需要的人士提供保镖业务,价格视情况而定。");
				move(5, 4);
				prints_nofmt("但是被保护对象一旦为恶,保镖自动离开,并可能会对雇主进行黑吃黑哦！");
				move(7, 4);
				sprintf(genbuf, "你确定要雇保镖么?");
				if (askyn(genbuf, NA, NA) == YEA) {
					money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
					move(8, 4);
					if (money < 10000) {
						prints_nofmt("你还是省省吧，没人会打你主意的。就那么点钱...");
						pressanykey();
						break;
					}
					guard_num =(countexp(&currentuser) / 1000) + 1 > 8 ? 8 : (countexp(&currentuser) / 1000) + 1;
					prints("按照您目前的身份地位，雇佣%d个保镖就够了。", guard_num);
					saveValue(currentuser.userid, MONEY_NAME, -money / 20, MAX_MONEY_NUM);
					move(9, 4);
					if (loadValue(currentuser.userid, "rob", 50) > 0) {
						prints_nofmt("嘿嘿，你有案底！念在收了你钱的份上，赶紧跑路吧...");
						pressanykey();
						break;
					}
					if (loadValue(currentuser.userid, "guard", 8) > 0) {
						prints_nofmt("你已经有保镖了。钱我们收下，保镖不能再给了，^_^");
						pressanykey();
					} else {
						saveValue(currentuser.userid, "guard", guard_num, 50);
						prints_nofmt("雇佣保镖成功,你可以有一段时间安享太平了。");
						pressanykey();
					}
				}
				break;

			case '2':
				while (!quit2) {
					nomoney_show_stat("兵马俑礼品店");
					move(6, 4);
					//prints ("欢迎光临兵马俑礼品店！");
					prints_nofmt("本店ASCII作品均非本店制作，部分作品由于种种原因，未能标明作者。\n"
							"    如作品创作者对其作品用于本店持有异议，请与本站大富翁总管联系。\n"
							"    本站将及时根据作者意愿作出调整。\n\n"
							"                                                   \33[1;32m兵马俑礼品店\033[0m\n");
					move(t_lines - 1, 0);
					prints_nofmt("\033[1;44m 选单 \033[1;46m [1]鲜花 [2]贺卡 [Q]离开\033[m");
					ch2 = igetkey();
					switch (ch2) {
						case 'q':
						case 'Q':
							quit2=1;
							break;
						case '1':
							shop_present(1, "鲜花", NULL);
							break;
						case '2':
							shop_present(2, "贺卡", NULL);
							break;
					}
					limit_cpu();
				}
				break;

			case '3':
				nomoney_show_stat("兵马俑商场经理室");
				whoTakeCharge(7, uident);
				char name[20];
				whoTakeCharge2(7, name);
				if (strcmp(currentuser.userid, uident)) {
					move(6, 4);
					prints("值班秘书%s叫住了你，说道:“经理%s正在开会，有什么事跟我说也行。”", name,uident);
					pressanykey();
					break;
				} else {
					move(6, 4);
					prints_nofmt("请选择操作代号:");
					move(9, 6);
					prints_nofmt("5. 辞职                      6. 算票价定价");
					move(10, 6);
					prints_nofmt("7. 退出");
					ch2 = igetkey();
					switch (ch2) {
						case '5':
							move(12, 4);
							if (askyn("您真的要辞职吗？", NA,NA) == YEA) {
								sprintf(genbuf, "%s 要辞去商场经理职务", currentuser.userid);
								mail_buf(genbuf, "millionaires", genbuf);
								move(14, 4);
								prints_nofmt("好吧，已经发信告知总管了");
								pressanykey();
							}
							break;
						case '6':
							move(12, 4);
							readstrvalue(MC_PRICE_FILE, "ticket_price", ticket_price, 10);
							price = atoi(ticket_price);
							prints("现在的价格是%d", price);
							getdata(13, 4, "设定新的价格: ", ticket_price, 10, DOECHO, YEA);
							move(14, 4);
							sprintf(genbuf, "新的价格是 %d，确定吗？", atoi(ticket_price));
							if (askyn(genbuf, NA, NA) == YEA) {
								if (atoi(ticket_price)>MAX_MONEY_NUM){
									move(15, 4);
									prints_nofmt("不要太狠了...");
									pressanykey();
									sprintf(ticket_price, "%d", MAX_MONEY_NUM);
								}
								savestrvalue(MC_PRICE_FILE, "ticket_price", ticket_price);
								move(15, 4);
								prints_nofmt("设置完毕。    ");
								sprintf(genbuf, "设置算票价价格为%s。", ticket_price);
								sprintf(buf, "%s行使商场经理管理权限", currentuser.userid);
								millionairesrec(buf, genbuf, "");
								pressanykey();
							}
							break;
					}
				}
				break;

			case '6':
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				readstrvalue(MC_PRICE_FILE, "ticket_price", ticket_price, 10);
				num = atoi(ticket_price);
				clear();
				move(5, 4);
				if (askyn("本服务收费，确定要算吗? ", YEA, NA) == YEA){
					if (money < num) {
						move(9, 4);
						prints_nofmt("对不起，您的金额不足。");
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

/*股票系统*/
static int money_stock() {
	//      moneycenter_welcome();
	int quit = 0;
	char ch;

	while (!quit) {
		clear();
		money_show_stat("兵马俑股市");

		if (ythtbbs_cache_utmp_get_ave_score() == 0) {
			clear();
			move(7, 10);
			prints_nofmt("\033[1;31m兵马俑股市今天休市\033[0m");
			pressanykey();
			return 0;
		}

		move(4, 4);
		prints_nofmt("请确认你已经在"MC_BOARD"版阅读过兵马俑股市规则。");
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1]传统板块 [2]推荐板块 [3]证监会主席办公室 [Q]离开   \033[m");
		ch = igetkey();
		switch (ch) {
			case '1':
				money_stock_board();
				break;
			case '2':
				clear();
				move(7, 10);
				prints_nofmt("\033[1;32m尚未开放\033[0m");
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

static int update_stock_v(struct boardmem *board, int curr_idx, va_list ap) {
	const char *name = va_arg(ap, const char *);
	int *price = va_arg(ap, int*);
	int *sboard = va_arg(ap, int*);
	int *count1 = va_arg(ap, int*);

	if (*count1 == 0) {
		return QUIT;
	}
	if (!strcmp(board->header.filename, name)) {
		*price = ythtbbs_cache_utmp_get_ave_score() / 100 + board->score / 20;

		if (board->stocknum <= 0) {
			board->stocknum = board->score * ((board->score > 10000) ? 2000 : 1000);

			if (board->stocknum < 50000) {
				board->stocknum = 50000;
			}
		}

		*sboard = curr_idx;
		*count1 = *count1 - 1;
		return QUIT;
	}
	return 0;
}

/*股票系统*/
static int money_stock_board() {
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

	count1= count = listfilecontent(MC_STOCK_BOARDS);
	clear();
	if (count==0){
		move(7, 10);
		prints_nofmt("\033[1;32m兵马俑股市尚未开盘\033[0m");
		pressanykey();
		return 0;
	}

	if ((fp1 = fopen( MC_STOCK_BOARDS, "r" )) != NULL) {
		for (j = 0; j < count; j++) {
			fscanf(fp1, "%s", stockboard[j]);
			sprintf(stockname[j], "St_%s", stockboard[j]);
		}
		fclose(fp1);
	} else {
		move(7, 10);
		prints_nofmt("\033[1;32m系统错误\033[0m");
		pressanykey();
		return 0;
	}

	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	clear();
	//count = MAX_STOCK_NUM;

	bzero(&stock_price, sizeof (stock_price));
	bzero(&stock_num, sizeof (stock_num));
	bzero(&addto_num, sizeof (addto_num));
	for (j = 0; j < count; j++) {
		ythtbbs_cache_Board_foreach_v(update_stock_v, stockboard[j], &stock_price[j], &stock_board[j], &count1);
		if (count1 == 0)
			break;
	}//计算股价
	for (i = 0; i < count; i++) {
		stock_num[i] = loadValue(currentuser.userid, stockname[i], 1000000);
	}//统计自己的数量
	//for (i = 0; i < MAX_STOCK_NUM; i++)
	i=0;
	while(!quit){
		money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
		persenal_stock_info(stock_num, stock_price, money, stockboard, stock_board);
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [B]购买 [S]出售 [C]转让 [Q]离开\033[m");
		ch = igetkey();
		switch (ch){
			case 'B':
			case 'b':
				total_money = 0;
				if (stop_buy()) {
					clear();
					move(7, 10);
					prints_nofmt("\033[1;31m兵马俑股市尚未开盘\033[0m");
					pressanykey();
					break;
				}
				getdata(t_lines - 1, 0, "您选择哪支股票?[0]", genbuf, 7, DOECHO, YEA);
				getnum=atoi(genbuf);
				if(getnum<0||getnum>count-1)
					break; //非法输入
				else
					i=getnum;
				if (seek_in_file(MC_STOCK_STOPBUY, stockboard[i])){
					move(t_lines - 2, 0);
					prints_nofmt("本支股票已被暂停交易!");
					pressanykey();
					break;
				}
				getdata(t_lines - 1, 0, "您要买多少股?[0]", genbuf, 7, DOECHO, YEA);

				addto_num[i] = atoi(genbuf);
				if (!genbuf[0])
					addto_num[i] = 0;
				//addto_num[i] = abs(addto_num[i]);
				if (addto_num[i] <= 0){
					move(t_lines - 2, 0);
					prints_nofmt("到底是要买还是卖...");
					pressanykey();
					break;
				}
				stock_num[i] = loadValue(currentuser.userid, stockname[i], 1000000);
				if (stock_num[i] >= 1000000) {
					move(t_lines - 2, 0);
					prints_nofmt("你已经有很多股票了,不要再买了");
					pressanykey();
					break;
				}
				if (ythtbbs_cache_Board_get_board_by_idx(stock_board[i])->stocknum <= 50000) {
					move(t_lines - 2, 0);
					prints_nofmt("对不起,此股目前没有可以出售的股票!");
					pressanykey();
					break;
				}
				if (stock_num[i] + addto_num[i] > 1000000){
					addto_num[i] = 1000000 - stock_num[i];
					move(t_lines - 2, 0);
					prints_nofmt("对不起,你已经有很多股票了!");
					pressanykey();
				}
				if (ythtbbs_cache_Board_get_board_by_idx(stock_board[i])->stocknum - addto_num[i] < 50000){
					addto_num[i] = ythtbbs_cache_Board_get_board_by_idx(stock_board[i])->stocknum - 50000;
					move(t_lines - 2, 0);
					prints_nofmt("对不起,此股目前没有那么多股票出售!");
					pressanykey();
				}
				move(t_lines - 2, 0);
				sprintf(genbuf, "确定购买 %d 股 %s 吗？", addto_num[i], stockname[i]);
				if (askyn(genbuf, NA, NA) == YEA) {
					temp_sum = addto_num[i] * stock_price[i];
					total_money += temp_sum;
					money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
					if (money - temp_sum < 0) {
						total_money -= addto_num[i] * stock_price[i];
						addto_num[i] = 0;
						move(t_lines - 2, 0);
						prints_nofmt("你当前的兵马俑币不够完成此项操作!");
						pressanykey();
						break;
					}
					ythtbbs_cache_Board_get_board_by_idx(stock_board[i])->stocknum -= addto_num[i];
					saveValue(currentuser.userid, MONEY_NAME, -temp_sum, MAX_MONEY_NUM);
					stock_num[i] += addto_num[i];
					saveValue(currentuser.userid, stockname[i], addto_num[i], 1000000);
					if (addto_num[i]>0){
						sprintf(genbuf, "%s进行股票交易(买入)", currentuser.userid);
						sprintf(buf,"%s购买了%d股%s股票(每股%d兵马俑币)，花费%d兵马俑币\n",
								currentuser.userid, addto_num[i], stockname[i], stock_price[i], temp_sum);
						millionairesrec(genbuf, buf, "股票交易");
						sprintf(buf,"您购买了%d股%s股票，成交价%d兵马俑币每股，花费%d兵马俑币。\n",
								addto_num[i], stockname[i], stock_price[i], temp_sum);
						sprintf(title,"股票购买凭证");
						mail_buf(buf, currentuser.userid, title);
						total_sum -= temp_sum;
						sprintf(genbuf, "你花掉了%d兵马俑币", temp_sum);
						move(t_lines - 2, 0);
						clrtoeol();
						prints_nofmt(genbuf);
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
					prints_nofmt("\033[1;31m兵马俑股市尚未开盘\033[0m");
					pressanykey();
					break;
				}
				getdata(t_lines - 1, 0, "您选择哪支股票?[0]", genbuf, 7, DOECHO, YEA);
				getnum=atoi(genbuf);
				if(getnum<0||getnum>count-1)
					break; //非法输入
				else
					i=getnum;
				if (seek_in_file(MC_STOCK_STOPBUY, stockboard[i])){
					move(t_lines - 2, 0);
					prints_nofmt("本支股票已被暂停交易!");
					pressanykey();
					break;
				}

				getdata(t_lines - 1, 0, "您要卖多少股?[0]", genbuf, 7, DOECHO, YEA);
				stock_num[i] = loadValue(currentuser.userid, stockname[i], 1000000);
				addto_num[i] = atoi(genbuf);
				if (!genbuf[0])
					addto_num[i] = 0;
				//addto_num[i] = abs(addto_num[i]);
				if (addto_num[i] <= 0){
					move(t_lines - 2, 0);
					prints_nofmt("到底是要买还是卖...");
					pressanykey();
					break;
				}
				if (stock_num[i] < addto_num[i]) {
					move(t_lines - 2, 0);
					prints_nofmt("你没有这么多股票啊...是你犯晕还是我犯晕?");
					pressanykey();
					break;
				}
				move(t_lines - 2, 0);
				sprintf(genbuf, "确定出售 %d 股 %s 吗？", addto_num[i], stockname[i]);
				if (askyn(genbuf, NA, NA) == YEA) {
					addto_num[i] *= -1;
					temp_sum = addto_num[i] * stock_price[i];
					stock_num[i] += addto_num[i];
					saveValue(currentuser.userid, MONEY_NAME, temp_sum/100-temp_sum, MAX_MONEY_NUM);
					whoTakeCharge(6, slow);//slowaction
					saveValue(slow, MONEY_NAME, -temp_sum/100, MAX_MONEY_NUM);
					saveValue(currentuser.userid, stockname[i], addto_num[i], 1000000);
					total_money += temp_sum-temp_sum/100;
					ythtbbs_cache_Board_get_board_by_idx(stock_board[i])->stocknum -= addto_num[i];
					temp_sum = ythtbbs_cache_Board_get_board_by_idx(stock_board[i])->score;
					if (temp_sum > 10000) {
						if (ythtbbs_cache_Board_get_board_by_idx(stock_board[i])->stocknum > temp_sum * 2000)
							ythtbbs_cache_Board_get_board_by_idx(stock_board[i])->stocknum = temp_sum * 2000;
					} else {
						if (ythtbbs_cache_Board_get_board_by_idx(stock_board[i])->stocknum > temp_sum * 1000)
							ythtbbs_cache_Board_get_board_by_idx(stock_board[i])->stocknum = temp_sum * 1000;
					}
					sprintf(genbuf, "%s进行股票交易(卖出)", currentuser.userid);
					sprintf(buf,"%s卖出了%d股%s股票(每股%d兵马俑币)，获得%d兵马俑币\n",
							currentuser.userid, -addto_num[i], stockname[i], stock_price[i], -total_money);
					millionairesrec(genbuf, buf, "股票交易");
					total_sum -= total_money;
					sprintf(genbuf, "扣除手续费后你拿回了%d兵马俑币", (-1) * total_money);
					move(t_lines - 2, 0);
					clrtoeol();
					prints_nofmt(genbuf);
					pressanykey();
				}
				sleep(1);
				break;
			case 'c':
			case 'C':
				move(t_lines - 1, 0);
				usercomplete("转让股票给谁？", uident);
				if (uident[0] == '\0')
					return 0;
				if (!getuser(uident)) {
					move(t_lines - 2, 0);
					prints_nofmt("错误的使用者代号...");
					pressreturn();
					return 0;
				}
				if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
					if (seek_in_file(DIR_MC "jijin", currentuser.userid));
					else if (!seek_in_file(DIR_MC "mingren", uident)) {
						clear();
						move(t_lines - 2, 0);
						prints_nofmt("对不起，证监会不允许黄马褂向外转让股票。");
						pressreturn();
						break;
					}
				}
				getdata(t_lines - 1, 0, "您选择哪支股票?[0]", genbuf, 7, DOECHO, YEA);
				getnum=atoi(genbuf);
				if(getnum<0||getnum>count-1)
					break; //非法输入
				else
					i=getnum;
				getdata(t_lines - 1, 0, "您要转让多少股?[0]", genbuf, 7, DOECHO, YEA);
				stock_num[i] = loadValue(currentuser.userid, stockname[i], 1000000);
				addto_num[i] = atoi(genbuf);
				if (addto_num[i] < 0){
					move(t_lines - 2, 0);
					prints_nofmt("想转让负的？醒醒...");
					pressanykey();
					break;
				}
				if (addto_num[i] == 0){
					pressanykey();
					break;
				}
				if (stock_num[i] < addto_num[i]) {
					move(t_lines - 2, 0);
					prints_nofmt("你没有这么多股票啊...是你犯晕还是我犯晕?");
					pressanykey();
					break;
				}

				sprintf(genbuf, "确定转账给 %s %d %s吗？", uident, addto_num[i], stockname[i]);
				if (askyn(genbuf, NA, NA) == YEA){
					saveValue(currentuser.userid, stockname[i], -addto_num[i], 1000000);
					saveValue(uident, stockname[i], addto_num[i], 1000000);
					sprintf(genbuf, "向你转让了%d股股票",addto_num[i]);
					sprintf(title, "您的朋友给您送%s股票来了", stockname[i]);
					mail_buf(genbuf, uident, title);
					sprintf(genbuf, "%s进行股票交易(转让)", currentuser.userid);
					sprintf(buf,"%s向%s转让了%d股%s股票(每股价值%d兵马俑币)",
							currentuser.userid, uident, addto_num[i], stockname[i], stock_price[i]);
					millionairesrec(genbuf, buf, "股票交易");
					move(t_lines - 2, 0);
					clrtoeol();
					prints_nofmt("转让成功");
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
	persenal_stock_info(stock_num, stock_price, money, stockboard, stock_board);
	move(t_lines - 2, 0);
	clrtobot();
	if (total_sum > 0)
		sprintf(genbuf, "这次交易中你拿回%d兵马俑币", total_sum);
	else if (total_sum < 0)
		sprintf(genbuf, "这次交易中你花掉了%d兵马俑币", -total_sum);
	else
		sprintf(genbuf, "你这次交易中没有使用到现金");
	prints_nofmt(genbuf);
	pressanykey();
	return 0;
}

/*显示money*/
static void money_show_stat(char *position) {
	int money, credit;
	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	credit = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);
	clear();
	move(2, 0);
	prints("您身上带着 \033[1;31m%d\033[m 兵马俑币，", money);
	prints("存款 \033[1;31m%d\033[m 兵马俑币。当前位置 \033[1;33m%s\033[m", credit, position);
	move(3, 0);
	prints_nofmt("\033[1m--------------------------------------------------------------------------------\033[m");
}

/*显示当前位置*/
static void nomoney_show_stat(char *position) {
	clear();
	move(2, 0);
	prints("\033[1;32m欢迎光临兵马俑金融中心，当前位置是\033[0m \033[1;33m%s\033[0m", position);
	move(3, 0);
	prints_nofmt("\033[1m--------------------------------------------------------------------------------\033[m");
}

/*赌场大厅*/
static int money_gamble() {
	int ch;
	int quit = 0;
	char uident[IDLEN + 1];
	char buf[STRLEN];
	clear();
	while (!quit) {
		clear();
		money_show_stat("兵马俑赌场大厅");
		move(6, 4);
		prints_nofmt("兵马俑赌场最近生意红火，大家尽兴啊");
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1]骰宝 [2]777 [3]猜数字 [4]金扑克梭哈 [5]俄罗斯轮盘 [6]经理室 [Q]离开\033[m");
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
				nomoney_show_stat("兵马俑赌场经理室");
				whoTakeCharge(3, uident);
				if (strcmp(currentuser.userid, uident)) {
					move(6, 4);
					prints("秘书%s冲你喝道:“死赌鬼，又输光啦？！老板%s不会再借钱给你了。”", name,uident);
					pressanykey();
					break;
				} else {
					move(6, 4);
					prints_nofmt("请选择操作代号:");
					move(7, 6);
					prints_nofmt("1. 发放VIP卡                  2. 收回VIP卡");
					move(8, 6);
					prints_nofmt("3. VIP客户                    4. 发邀请函");
					move(9, 6);
					prints_nofmt("5. 金盆洗手                   6. 退出");
					ch = igetkey();
					switch (ch) {
						case '1':
							move(12, 4);
							usercomplete("向谁发放VIP卡？", uident);
							move(13, 4);
							if (uident[0] == '\0')
								break;
							if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
								prints_nofmt("错误的使用者代号...");
								pressanykey();
								break;
							}
							if (seek_in_file(DIR_MC "gamble_VIP", uident)) {
								prints_nofmt("该客户已经拥有赌场VIP卡。");
								pressanykey();
								break;
							}
							if (askyn("确定吗？", NA, NA) == YEA) {
								ytht_add_to_file(DIR_MC "gamble_VIP", uident);
								sprintf(genbuf, "%s 向你发放兵马俑赌场VIP卡", currentuser.userid);
								mail_buf("尊敬的客户： 欢迎多多光临兵马俑赌场，恭祝发财!", uident, genbuf);
								move(14, 4);
								prints_nofmt("发放完成。");
								sprintf(buf, "给%s发放赌场VIP卡",uident);
								sprintf(genbuf, "%s行使赌场管理权限",currentuser.userid);
								millionairesrec(genbuf, buf, "BMYboss");
								pressanykey();
							}
							break;
						case '2':
							move(12, 4);
							usercomplete("收回谁的VIP卡？", uident);
							move(13, 4);
							if (uident[0] == '\0')
								break;
							if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
								prints_nofmt("错误的使用者代号...");
								pressanykey();
								break;
							}
							if (!seek_in_file (DIR_MC "gamble_VIP", uident)) {
								prints_nofmt("该客户没有赌场VIP卡。");
								pressanykey();
								break;
							}
							if (askyn("确定吗？", NA, NA) == YEA) {
								ytht_del_from_file(DIR_MC "gamble_VIP", uident, true);
								sprintf(genbuf, "%s 收回了你的兵马俑赌场VIP卡", currentuser.userid);
								mail_buf("穷鬼，没钱了还VIP啊？下辈子吧！", uident, genbuf);
								move(14, 4);
								prints_nofmt("卡已收回。");
								sprintf(buf, "收回%s的赌场VIP卡",uident);
								sprintf(genbuf, "%s行使赌场管理权限",currentuser.userid);
								millionairesrec(genbuf, buf, "BMYboss");
								pressanykey();
							}
							break;
						case '3':
							clear();
							move(1, 0);
							prints_nofmt("目前拥有赌场VIP卡的客户：");
							listfilecontent(DIR_MC "gamble_VIP");
							pressanykey();
							break;
						case '4':
							move(12, 4);
							usercomplete("给谁发邀请函？", uident);
							move(13, 4);
							if (uident[0] == '\0')
								break;
							if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
								prints_nofmt("错误的使用者代号...");
								pressanykey();
								break;
							}
							if (loadValue(uident, "invitation", 1)) {
								prints_nofmt("已经发过了。");
								pressanykey();
								break;
							}
							if (askyn("确定吗？", NA, NA) == YEA) {
								saveValue(uident, "invitation", 1, 1);
								saveValue(currentuser.userid, "last_invitation", -2000000000, 2000000000);
								saveValue(currentuser.userid, "last_invitation", time(0), 2000000000);
								sprintf(genbuf, "%s 给您发来了赌场邀请函", currentuser.userid);
								mail_buf("您将有机会获得20万大奖！但是，您有更大的机会为此送命。－－心跳尽在兵马俑赌场俄罗斯轮盘赌！", uident, genbuf);
								move(14, 4);
								prints_nofmt("邀请函发出去了。");
								sprintf(buf, "给%s发放赌场邀请函",uident);
								sprintf(genbuf, "%s行使赌场管理权限",currentuser.userid);
								millionairesrec(genbuf, buf, "BMYboss");
								pressanykey();
							}
							break;
						case '5':
							move(12, 4);
							if (askyn ("您真的要金盆洗手吗？", NA, NA) == YEA) {
								/*	ytht_del_from_file(MC_BOSS_FILE,
									"gambling");
									sprintf(genbuf,
									"%s 宣布辞去兵马俑赌场经理职务",
									currentuser.userid);
									deliverreport(genbuf,
									"兵马俑金融中心对其一直以来的工作表示感谢，祝以后顺利！");
									move(14, 4);
									prints
									("好吧，既然你意已决，弟兄们只有说再见了！");
									quit = 1;
									*/
								sprintf(genbuf, "%s 要辞去赌场经理职务", currentuser.userid);
								mail_buf(genbuf, "millionaires", genbuf);
								move(14, 4);
								prints_nofmt("好吧，已经发信告知总管了");
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

/*赌博-- 777*/
static int money_777() {
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
		if (mc->prize777 <= 0)
			mc->prize777 = 30000;
		bid = 0;
		clear();
		money_show_stat("兵马俑赌场777");
		move(6, 4);
		prints_nofmt("--R 1:2    -RR 1:3    RR- 1:3    -BB 1:5    BB- 1:5");
		move(7, 4);
		prints_nofmt("RRR 1:10   BBB 1:20   666 1:40   677 1:60   --- 1:1");
		move(8, 4);
		prints_nofmt("         777 1:80 且有机会赢得当前累积基金的一半         ");
		move(9, 4);
		prints("兵马俑目前累积奖金数: %d，想赢大奖么？压 100 块就行哦。", mc->prize777);
		r = random() % 40;
		if (r < 1)
			money_police();

		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1] 压30 [2] 压100 [Q]离开                                          \033[m");
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
			prints_nofmt("没钱就别赌了...");
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
			mc->prize777 += bid * 80 / 100;
			if (mc->prize777 >= MAX_MONEY_NUM)
				mc->prize777 = MAX_MONEY_NUM;

			sprintf(title, "%s参与赌博(777)(输)", currentuser.userid);
			sprintf(buf, "%s在777 输了%d兵马俑币", currentuser.userid, bid);
			millionairesrec(title, buf, "赌博777");

			move(12, 4);
			prints_nofmt("输了，赌注百分之八十滚入兵马俑累积基金，造福他人等于造福自己。");
			limit_cpu();
			pressanykey();
			continue;
		}
		if (winrate > 0) {
			saveValue(currentuser.userid, MONEY_NAME, bid * winrate, MAX_MONEY_NUM);
			move(12, 4);
			prints("您赢了 %d 元", bid * (winrate - 1));
			mc->prize777 -= bid * (winrate - 1);

			sprintf(title, "%s参与赌博(777)(赢)", currentuser.userid);
			sprintf(buf, "%s在777 赢了%d兵马俑币", currentuser.userid, bid * (winrate - 1));
			millionairesrec(title, buf, "赌博777");
		}
		if (winrate == 81 && bid == 100) {
			saveValue(currentuser.userid, MONEY_NAME, mc->prize777 / 2, MAX_MONEY_NUM);
			mc->prize777 /= 2;
			move(13, 4);
			prints_nofmt("恭喜您获得兵马俑大奖！");
			sprintf(title, "%s参与赌博(777)(赢成马了)", currentuser.userid);
			sprintf(buf, "%s在777 赢了%d兵马俑币", currentuser.userid, mc->prize777 / 2);
			millionairesrec(title, buf, "赌博777");
		}
		limit_cpu();
		pressanykey();
	}
	return 0;
}

/*赌博--777*/
static int calc777(int t1, int t2, int t3) {
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

/*赌博--猜数字*/
#if 0
static int guess_number() {
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
		money_show_stat("兵马俑良民生财之路...");
		move(4, 4);
		prints("\033[1;31m开动脑筋赚钱啊~~~\033[m");
		move(5, 4);
		//prints("最小压 100 兵马俑币，上限999");
		prints("一次 100 兵马俑币.");
		move(6, 4);
		prints("mAnB表示有m个数字猜对且位置也对,n个数字猜对但位置不对");
		move(t_lines - 1, 0);
		prints("\033[1;44m 选单 \033[1;46m [1]下注 [Q]离开                                                 \033[m");
		if (random() % 40 < 1)
			money_police();
		ch = igetkey();
		switch (ch) {
			case '1':
				win = 0;

				getdata(8, 4, "您压多少兵马俑币？[100]", genbuf, 5,
						DOECHO, YEA);
				num = atoi(genbuf);
				if (!genbuf[0])
					num = 100;
				if (num < 100) {
					move(9, 4);
					prints("有没有钱啊？那么点钱我们不带玩的");
					pressanykey();
					break;
				}
				//num = 100;
				sprintf(genbuf, "您压了 \033[1;31m%d\033[m 兵马俑币，确定么？", num);
				move(9, 4);
				if (askyn(genbuf, YEA, NA) == YEA) {
					money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
					if (money < num) {
						move(11, 4);
						prints("去去去，没那么多钱捣什么乱         \n");
						pressanykey();
						break;
					}
					//if (num > 999)
					//num = 999;
					saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
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
							prints("请输入四个不重复的数字");
							getdata(11, 4, "请猜[q - 退出] → ", genbuf, 5, DOECHO, YEA);
							if (genbuf[0] == 'q' || genbuf[0] == 'Q') {
								mc->prize777 += num;
								if (mc->prize777 > MAX_MONEY_NUM)
									mc->prize777 = MAX_MONEY_NUM;
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
								prints ("输入数字有问题!!");
								pressanykey();
								move(12, 4);
								prints ("                ");
							}
						} while (!genbuf[0]);
						move(count + 13, 0);
						prints("  第 %2d 次： %s  ->  %dA %dB ", count, genbuf, an(genbuf, ans), bn(genbuf, ans));
						if (an(genbuf, ans) == 4)
							break;
						sleep(1);
					}

					move(12, 4);
					if (count > 6) {
						sprintf(genbuf, "你输了呦！正确答案是 %s，下次再加油吧!!", ans);
						sprintf(genbuf, "\033[1;31m可怜没猜到，输了 %d 元！\033[m", num);
						//mc->prize777 += num;

						sprintf(title, "%s参与赌博(猜数字)(输)", currentuser.userid);
						sprintf(buf, "%s在猜数字输了%d兵马俑币", currentuser.userid, num);
						millionairesrec(title, buf, "赌博猜数字");

						if (mc->prize777 > MAX_MONEY_NUM)
							mc->prize777 = MAX_MONEY_NUM;
					} else {
						int oldmoney = num;
						num *= bet[count];
						if (num - oldmoney > 0) {
							sprintf(genbuf, "恭喜！总共猜了 %d 次，净赚奖金 %d 元", count, num);
							num += oldmoney;
							saveValue(currentuser.userid, MONEY_NAME, num, MAX_MONEY_NUM);
							win = 1;

							sprintf(title, "%s参与赌博(猜数字)(赢)", currentuser.userid);
							sprintf(buf, "%s在猜数字赢了%d兵马俑币", currentuser.userid, num);
							millionairesrec(title, buf, "赌博猜数字");

						} else if (num - oldmoney == 0) {
							sprintf(genbuf, "唉～～总共猜了 %d 次，没输没赢！", count);
							saveValue(currentuser.userid, MONEY_NAME, num, MAX_MONEY_NUM);
						} else {
							mc->prize777 += oldmoney;
							if (mc->prize777 > MAX_MONEY_NUM)
								mc->prize777 = MAX_MONEY_NUM;

							sprintf(genbuf, "啊～～总共猜了 %d 次，赔钱 %d 元！", count, oldmoney - money);
						}
					}
					prints("结果: %s", genbuf);
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
#endif

#if 0
static int an(char *a, char *b) {
	int i, k = 0;
	for (i = 0; i < 4; i++)
		if (*(a + i) == *(b + i))
			k++;
	return k;
}

static int bn(char *a, char *b) {
	int i, j, k = 0;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			if (*(a + i) == *(b + j))
				k++;
	return (k - an(a, b));
}

static void itoa(int i, char *a) {
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
#endif

/*警署--警察临检*/
static int money_police() {
	char ch;
	char buf[200], title[STRLEN];
	int money = 0, quit = 1, check_num;
	//int mode = 0, color;
	move(t_lines - 1, 0);
	check_num = 97 + random() % 26;
	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
	if (random() % 4 > 0) {
		sprintf(buf, "\033[1;41m 系统临检 \033[1;46m 请输入字符:%c        \033[m", check_num);
	}
	//else if (random() % 3 == 0)
	else {
		check_num = 0;
		sprintf(buf, "\033[1;41m 系统临检 \033[1;46m 请输入你的ID(注意大小写!):        \033[m");
	}
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
		prints_nofmt("系统认为你有作弊嫌疑，罚掉身上所有现金,真惨啊~~~");
		saveValue("millionaires", MONEY_NAME, money, MAX_MONEY_NUM);
		sprintf(title, "%s被系统临检", currentuser.userid);
		sprintf(buf, "系统临检, %s被罚掉所有现金%d兵马俑币", currentuser.userid, money);
		millionairesrec(title, buf, "系统临检");
		pressanykey();
		Q_Goodbye(0, NULL, NULL);
	} else {
		saveValue(currentuser.userid, MONEY_NAME, money, MAX_MONEY_NUM);
		move(t_lines - 2, 4);
		sprintf(buf, "你就是大名鼎鼎的%s啊,继续继续...", currentuser.userid);
		prints_nofmt(buf);
		pressanykey();

	}
	return 0;
}

/*个人股票系统*/
/*
int stock_num[MAX_STOCK_NUM],
int stock_price[MAX_STOCK_NUM],
int money,
char stockboard[STRLEN][MAX_STOCK_NUM],
int stock_board[MAX_STOCK_NUM]
*/
static void persenal_stock_info(int stock_num[15], int stock_price[15], int money, char stockboard[STRLEN][MAX_STOCK_NUM], int stock_board[15]) {
	int i, count;
	count = listfilecontent(MC_STOCK_BOARDS);
	clear();
	move(0, 4);
	prints_nofmt("兵马俑股市试营业...以下是你的各股持有数,每股购买上限1000000股");
	move(1, 4);
	sprintf(genbuf, "目前你的兵马俑币金额为%d", money);
	prints_nofmt(genbuf);
	for (i = 0; i < count; i++) {
		move(3 + i, 0);
		sprintf(genbuf, "编号:%2d 价钱:%-5d 持有量:%-7d 版名:%-18s 现有股票数:%d",
				i, stock_price[i], stock_num[i], stockboard[i], ythtbbs_cache_Board_get_board_by_idx(stock_board[i])->stocknum);
		if (seek_in_file(MC_STOCK_STOPBUY, stockboard[i]))
			prints("\033[1;30m%s\033[m", genbuf);
		else
			prints_nofmt(genbuf);
	}
}

//add by happybird for 鲜花店，贺卡店
//显示欢迎画面
//这个要读文件可能会造成损耗，小心使用
static int show_welcome(char *filepath,int startline,int endline)
{
	FILE *fp;
	char buf[400];
	int linecount=0;

	fp=fopen(filepath,"r");
	if(!fp){
		move(startline,10);
		prints_nofmt("欢迎您的到来!");
		return 0;
	}
	linecount=0;
	while(!feof(fp)){
		if(fgets(buf,400,fp)){
			move(startline+linecount,0);
			prints_nofmt(buf);
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

//礼品店，鲜花贺卡二合一，macintosh@bmy 20051204
static int shop_present(int order, char *kind, char *touserid) {
	char ok_filename[PATHLEN];
	char ok_title[STRLEN];
	int price_per=0;
	char *ptr1,*ptr2;
	//char filepath[256];
	//void *buffer_me = NULL;
	char buf[STRLEN];
	int ma;

	sprintf(buf, "兵马俑礼品店%s柜台", kind);
	nomoney_show_stat(buf);
	sprintf(buf, "%s%d%s", PRESENT_DIR, order, "/welcome");
	show_welcome(buf,6,20);
	pressanykey();

	DIR *dp;
	//	struct dirent *dirp;
	char dirNameBuffer[10][PATHLEN], dirTitleBuffer[10][STRLEN];
	char fileNameBuffer[10][PATHLEN],  fileTitleBuffer[10][STRLEN];
	char dirpath[PATHLEN], filepath[PATHLEN + STRLEN], dir[STRLEN * 4], indexpath[PATHLEN + STRLEN], title[STRLEN];
	int numDir=0, numFile=0, dirIndex, cardIndex;
	size_t m;
	int HIDE=0;
	FILE *fp;

	sprintf(buf, "兵马俑礼品店%s柜台", kind);
	nomoney_show_stat(buf);
	move(4,4);
	sprintf(dir, "%s%d/", PRESENT_DIR, order);
	sprintf(indexpath, "%s.Names", dir);
	prints("本店出售如下种类的%s: ", kind);
	if ((dp = opendir(dir)) == NULL)
		return -1;
	closedir(dp);

	fp=fopen(indexpath, "r");
	if(fp!=0) {
		while (fgets(buf, STRLEN, fp) != NULL && numDir < 10) {
			if(!strncmp(buf, "Name=", 5)) {
				sprintf(title, "%s", buf+5);
				if(strstr(title + 38,"(BM: SYSOPS)") || strstr(title + 38,"(BM: BMS)")|| !strncmp(title, "<HIDE>",6))
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
					ytht_strsncpy(dirNameBuffer[numDir], dirpath, PATHLEN);
					ytht_strsncpy(dirTitleBuffer[numDir], title, STRLEN);
					move(6 + numDir, 8);
					prints("%d. %s", numDir, title);
					numDir++;
				}
			}
		}
		fclose(fp);
	}

	while(1) {
		getdata(16, 4, "请选择类型:", buf, 3, DOECHO, YEA);
		if (buf[0] == '\0')
			return 0;
		dirIndex = atoi(buf);
		if (dirIndex >= 0 && dirIndex < numDir)
			break;
	}

	sprintf(buf, "兵马俑礼品店%s柜台", kind);
	nomoney_show_stat(buf);
	move(4,4);
	ytht_strsncpy(dirpath, dirNameBuffer[dirIndex], sizeof(dirpath));
	if ((dp = opendir(dirpath)) == NULL)
		return -1;
	closedir(dp);
	//prints("本店出售如下种类的%s: ", kind);

	sprintf(indexpath, "%s/.Names", dirpath);
	fp=fopen(indexpath, "r");
	if(fp!=0) {
		while (fgets(buf, STRLEN, fp) != NULL && numFile < 10) {
			if(!strncmp(buf, "Name=", 5)) {
				sprintf(title, "%s", buf+5);
				if(strstr(title + 38,"(BM: SYSOPS)") || strstr(title + 38,"(BM: BMS)")|| !strncmp(title, "<HIDE>",6))
					HIDE=1;
				else
					HIDE=0;
				title[38]=0;
				fgets(buf, STRLEN, fp);
				if(!strncmp("Path=~/", buf, 6)) {
					if(HIDE) continue;
					snprintf(filepath, sizeof(filepath), "%s/%s", dirpath, buf+7);
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
	prints("本店 %s 类%s共有 %d 种: ", dirTitleBuffer[dirIndex], kind, numFile);
	move(17, 4);
	while(1) {
		getdata(18, 4, "请选择要预览的编号[ENTER放弃]: ", buf, 3, DOECHO, YEA);
		if(buf[0] == '\0')
			return 0;
		cardIndex = atoi(buf);
		if (cardIndex >= 0 && cardIndex <= numFile)
			break;
	}

	char local_buf[STRLEN * 4];
	sprintf(local_buf, "%s柜台%s 类%s展示", kind, dirTitleBuffer[dirIndex], fileTitleBuffer[cardIndex]);
	nomoney_show_stat(local_buf);
	//show_welcome(fileNameBuffer[cardIndex], 5, 20);
	ansimore2(fileNameBuffer[cardIndex], 1, 5, 20);

	limit_cpu();

	ytht_strsncpy(ok_filename, fileNameBuffer[cardIndex], PATHLEN);
	ytht_strsncpy(ok_title, fileTitleBuffer[cardIndex], STRLEN);
	if(!ok_filename[0])  return 0;

	sprintf(buf, "礼品店%s收银台", kind);
	money_show_stat(buf);
	//ok_title= 玫瑰花1(枝)   价:100bmyb
	ptr1= strstr(ok_title,"价:");
	if(!ptr1){
		move(7,10);
		prints_nofmt("My God! 本商品还没有定价，赶快去告诉礼品店老板吧");
		pressanykey();
		return 0;
	}
	ptr1+=3;
	if(!ptr1){
		move(7,10);
		prints_nofmt("My God! 本商品还没有定价，赶快去告诉礼品店老板吧");
		pressanykey();
		return 0;
	}
	ptr2= strstr(ptr1,"bmyb");
	if(!ptr2){
		move(7,10);
		prints_nofmt("My God! 本商品定价有问题，赶快去告诉礼品店老板吧");
		pressanykey();
		return 0;
	}
	*ptr2='\0';
	price_per = atoi(ptr1);
	*ptr2='b';
	if(price_per<0){
		move(7,10);
		prints_nofmt("My God! 本商品定价有问题，赶快去告诉礼品店老板吧");
		pressanykey();
		return 0;
	}else if(price_per == 0){
		move(7,10);
		prints_nofmt("嘿嘿! 本商品免费赠送，以后要多多支持本店喔");
		pressanykey();
	}

	ptr1-=4;
	while(*ptr1==' ') ptr1--;
	*(ptr1+1) = '\0';

	move(8,10);
	sprintf(genbuf, "你确定要付钱购买%s吗",ok_title);
	if (askyn(genbuf, YEA, NA) == NA)
		return 0;
	ma = buy_present(order, kind, ok_title, ok_filename, price_per, touserid);
	if (ma==9) return 9;
	return 1;
}

static int buy_present(int order, char *kind, char *cardname, char *filepath, int price_per,char *touserid) {
	(void) order;
	int i;
	int inputNum=1;
	char uident[IDLEN + 1], note[3][STRLEN], tmpname[STRLEN];
	int price;
	char buf[STRLEN];
	char *ptr1,*ptr2;
	char unit[STRLEN];

	clear();
	ansimore2(filepath, 0, 0, 25);
	move(t_lines - 2, 0);
	sprintf(genbuf, "请输入要购买的数量[%d]: ",inputNum);
	while(1) {
		inputNum = 1;
		getdata(15, 4, genbuf, buf, 8, DOECHO, YEA);
		if(buf[0] == '\0' || (inputNum = atoi(buf)) >= 1)
			break;
	}
	price = price_per*inputNum;
	//加个限制
	if (price < 0 || price > MAX_MONEY_NUM){
		move(t_lines - 2, 4);
		prints_nofmt("大宗货物请提前预约...");
		pressanykey();
		return 0;
	}

	//cardname= 玫瑰花1(枝)   价:100bmyb
	if ((ptr1 = strstr(cardname,"(")) == NULL)
		sprintf(buf,"%s","份");
	else{
		ptr1++;
		ptr2=strstr(ptr1,")");
		if(!ptr2)
			sprintf(buf,"%s","份");
		else{
			*ptr2='\0';
			ytht_strsncpy(buf, ptr1, STRLEN);
		}
		if (!strlen(buf))
			sprintf(buf,"%s","份");
		ptr1--;
		*ptr1='\0';
	}
	sprintf(unit,"%s",buf);
	sprintf(genbuf, "你确定要花费%d兵马俑币购买%d%s%s吗",price,inputNum,unit,cardname);
	if (askyn(genbuf, YEA, NA) == NA)
		return 0;
	if (loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM) < price) {
		move(t_lines - 2, 4);
		prints_nofmt("你的钱不够啊~~~");
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
		sprintf(buf, "要把%s送给谁? ", kind);
		usercomplete(buf, uident);
		if (uident[0] == '\0') {
			move(t_lines - 2, 4);
			clrtobot();
			snprintf(buf, sizeof buf, "居然不写地址，你买的%s丢失在邮寄途中...", kind);
			prints_nofmt(buf);
			pressanykey();
			return 0;
		}
		if (!getuser(uident)) {
			move(t_lines - 2, 4);
			clrtobot();
			snprintf(buf, sizeof buf, "没有这个人啊，你买的%s被邮递员私吞了...", kind);
			prints_nofmt(buf);
			pressanykey();
			return 0;
		}
	}
	move(2, 0);
	prints_nofmt("还有什么话要附上吗？[可以写3行喔]");
	bzero(note, sizeof (note));
	for (i = 0; i < 3; i++) {
		getdata(3 + i, 0, ": ", note[i], STRLEN - 1, DOECHO, NA);
		if (note[i][0] == '\0')
			break;
	}
	sprintf(tmpname, "bbstmpfs/tmp/present.%s.%d", currentuser.userid, uinfo.pid);
	copyfile(filepath, tmpname);
	if (i > 0) {
		int j;
		FILE *fp;
		
		if ((fp = fopen(tmpname, "a")) != NULL) {
			fprintf(fp, "\n以下是 %s 的附言:\n", currentuser.userid);
			for (j = 0; j < i; j++)
				fprintf(fp, "%s", note[j]);
			fclose(fp);
		}
	}
	char local_buf[STRLEN * 4];
	sprintf(local_buf,"送你%d%s%s，喜欢吗？",inputNum,unit,cardname);
	if (mail_file(tmpname, uident, local_buf) >= 0) {
		move(8,0);
		sprintf(local_buf,"你的%s已经发出去了",kind);
		prints_nofmt(local_buf);
		pressanykey();
		return 9; //for marry
	} else {
		move(8,0);
		prints_nofmt("发送失败，对方邮箱超容");
		pressanykey();
	}
	unlink(tmpname);
	return 0;
}


/* write by dsyan               */
/* 87/10/24                     */
/* 天长地久 Forever.twbbs.org   */

//char card[52], mycard[5], cpucard[5], sty[100], now;
char *card, *mycard, *cpucard, *sty;
int now;
static int forq(char *a, char *b) {
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

/*赌博--梭哈*/
static void p_gp() {
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
		nomoney_show_stat("金扑克梭哈");
		move(4, 4);
		prints_nofmt("\033[1;31m开动脑筋赚钱啊~~~\033[m");
		move(5, 4);
		prints_nofmt("一次压 100 兵马俑币");
		move(6, 4);
		prints_nofmt("大小:");
		move(7, 4);
		prints_nofmt("同花顺＞铁枝＞葫芦＞同花＞顺子＞三条＞兔胚＞单胚＞单张");
		move(8, 4);
		prints_nofmt("特殊加分：");
		move(9, 4);
		prints_nofmt("同花顺  １５倍");
		move(10, 4);
		prints_nofmt("铁　枝  １０倍");
		move(11, 4);
		prints_nofmt("葫　芦　　５倍");
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1]下注 [Q]离开                                                \033[m");
		if (random() % 40 < 1)
			money_police();
		if (!cont)
			ch = igetkey();
		switch (ch) {
			case '1':
				clear();
				money_show_stat("金扑克梭哈");
				if (!cont) {
					ans[0] = 0;
					move(6, 4);
					//if (askyn("您确定要压100兵马俑币么?", YEA, NA) == NA)
					//break;
					getdata(8, 4, "您压多少兵马俑币？[100-9999]", genbuf, 5, DOECHO, YEA);
					num = atoi(genbuf);
					if (!genbuf[0])
						num = 999;
					if (num < 100) {
						move(9, 4);
						prints_nofmt("有没有钱啊？那么点钱我们不带玩的");
						pressanykey();
						break;
					}
					money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
					if (money < num) {
						move(8, 4);
						prints_nofmt("去去去，没那么多钱捣什么乱！      \n");
						pressanykey();
						break;
					}
					//num = 100;
					saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
				}
				clear();
				money_show_stat("金扑克梭哈");
				move(21, 0);
				if (cont > 0)
					prints_nofmt("\033[33;1m (←)(→)改变选牌  (SPACE)改变换牌  (Enter)确定\033[m");
				else
					prints_nofmt("\033[33;1m (←)(→)改变选牌  (d)Double  (SPACE)改变换牌  (Enter)确定\033[m");
				move(22, 0);
				prints("当前下注金额: %d 兵马俑币", num);
				for (i = 0; i < 52; i++)
					card[i] = i;	/* 0~51 ..黑杰克是 1~52 */

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
						outs(hold[i] < 0 ? "保" : "  ");
						move(17, i * 4 + 2);
						outs(hold[i] < 0 ? "留" : "  ");
					}
					move(8, xx * 4 + 2);
					outs("  ");
					move(8, x * 4 + 2);
					outs("↓");
					move(t_lines - 1, 0);
					xx = x;

					tmp = egetch();
					switch (tmp) {
#ifdef GP_DEBUG
						case KEY_UP:
							getdata(21, 0, "把牌换成> ", genbuf, 3, DOECHO, YEA);
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
							if (!cont && !doub && loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM) >= num) {
								doub++;
								move(21, 0);
								clrtoeol();
								prints_nofmt("\033[33;1m (←)(→)改变选牌  (SPACE)改变换牌  (Enter)确定\033[m");
								saveValue(currentuser.userid, MONEY_NAME, -num, MAX_MONEY_NUM);
								num *= 2;
								move(22, 0);
								prints("当前下注金额 %d 兵马俑币", num);
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
					getdata(21, 0, "把牌换成> ", genbuf, 3, DOECHO, YEA);
					cpucard[x] = atoi(genbuf);
				}
				qsort(cpucard, 5, sizeof (char), forq);
				for (i = 0; i < 5; i++)
					show_card(1, cpucard[i], i);
#endif
				i = gp_win();

				if (i < 0) {
					saveValue(currentuser.userid, MONEY_NAME, num * 2, MAX_MONEY_NUM);

					sprintf(title, "%s参与赌博(梭哈)(赢)", currentuser.userid);
					sprintf(buf, "%s在梭哈赢了%d兵马俑币", currentuser.userid, num);
					millionairesrec(title, buf, "赌博梭哈");

					sprintf(genbuf, "哇!好棒喔!!! 净赚 %d 元...  :D", num);
					prints_nofmt(genbuf);
					if (cont > 0)
						sprintf(genbuf, "连胜 %d 次, 赢了 %d 元", cont + 1, num);
					else
						sprintf(genbuf, "赢了 %d 元", num);
					num = (num > 50000 ? 100000 : num * 2);
					gw = 1;
				} else if (i > 1000) {
					switch (i) {
						case 1001: doub = 15; break;
						case 1002: doub = 10; break;
						case 1003: doub = 5; break;
					}
					saveValue(currentuser.userid, MONEY_NAME, num * 2 * doub, MAX_MONEY_NUM);

					sprintf(title, "%s参与赌博(梭哈)(赢)", currentuser.userid);
					sprintf(buf, "%s在梭哈赢了%d兵马俑币", currentuser.userid, num * 2 * doub - num);
					millionairesrec(title, buf, "赌博梭哈");

					sprintf(genbuf, "哇!好棒喔!!!净赚 %d 元...  :D", num * 2 * doub - num);
					prints_nofmt(genbuf);
					if (cont > 0)
						sprintf(genbuf, "连胜 %d 次, 赢了 %d 元", cont + 1, num * doub);
					else
						sprintf(genbuf, "赢了 %d 元", num * doub);
					num = (num > 5000 ? 10000 : num * 2 * doub);
					gw = 1;
					num = (num >= 10000 ? 10000 : num);
				} else {
					prints_nofmt("输了...:~~~");

					sprintf(title, "%s参与赌博(梭哈)(输)", currentuser.userid);
					sprintf(buf, "%s在梭哈输了%d兵马俑币", currentuser.userid, num);
					millionairesrec(title, buf, "赌博梭哈");

					if (cont > 1)
						sprintf(genbuf, "中止 %d 连胜, 输了 %d 元", cont, num);
					else
						sprintf(genbuf, "输了 %d 元", num);
					cont = 0;
					num = 0;
					pressanykey();
				}

				if (gw == 1) {
					gw = 0;
					getdata(21, 0, "您要把奖金继续压注吗 (y/n)?", ans, 2, DOECHO, YEA);
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

/*赌博--梭哈--显示扑克牌*/
static void show_card(int isDealer, int c, int x) {
	int beginL;
	char *suit[4] = { "Ｃ", "Ｄ", "Ｈ", "Ｓ" };
	char *num[13] = { "Ｋ", "Ａ", "２", "３", "４", "５", "６",
		"７", "８", "９", "10", "Ｊ", "Ｑ"
	};

	beginL = (isDealer) ? 4 : 12;
	move(beginL, x * 4);
	outs("╭───╮");
	move(beginL + 1, x * 4);
	prints("│%2s    │", num[c % 13]);
	move(beginL + 2, x * 4);
	prints("│%2s    │", suit[c / 13]);	/* ←这里跟黑杰克 */
#ifdef GP_DEBUG
	move(beginL + 3, x * 4);
	prints("│%2d    │", c);	/* 有点不同喔!! */
#else
	move(beginL + 3, x * 4);
	outs("│      │");	/* 有点不同喔!! */
#endif
	move(beginL + 4, x * 4);
	outs("│      │");
	move(beginL + 5, x * 4);
	outs("│      │");
	move(beginL + 6, x * 4);
	outs("╰───╯");
}

/*赌博--梭哈*/
static void money_cpu() {
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
		outs(hold[i] < 0 ? "保" : "  ");
		move(9, i * 4 + 2);
		outs(hold[i] < 0 ? "留" : "  ");
	}
	move(11, 25);
	prints_nofmt("\033[44;37m电脑换牌前...\033[40m");
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

/*赌博-梭哈*/
static int gp_win() {
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
			case 1: ret = 1001; break;
			case 2: ret = 1002; break;
			case 3: ret = 1003; break;
		}

	return ret;
}

//同花顺、铁枝、葫、同花、顺、三条、兔胚、胚、一只
static int complex(char *cc, char *x, char *y) {
	char p[13], q[5], r[4];
	char a[5], b[5], c[5], d[5];
	int i, j, k;

	money_suoha_tran(a, b, cc);
	money_suoha_check(p, q, r, cc);

	/* 同花顺 */
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
	/*铁枝  */
	if (q[4] == 1) {
		for (i = 0; i < 13; i++)
			if (p[i] == 4)
				*x = i ? i : 13;
		return 2;
	}
	/* 葫芦 */
	if (q[3] == 1 && q[2] == 1) {
		for (i = 0; i < 13; i++)
			if (p[i] == 3)
				*x = i ? i : 13;
		return 3;
	}
	/* 同花 */
	for (i = 0; i < 4; i++)
		if (r[i] == 5) {
			*x = i;
			return 4;
		}
	/* 顺子 */
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
	/* 三条 */
	if (q[3] == 1)
		for (i = 0; i < 13; i++)
			if (p[i] == 3) {
				*x = i ? i : 13;
				return 6;
			}
	/* 兔胚 */
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
	/* 胚 */
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
	/* 一张 */
	*x = 0;
	*y = 0;
	for (i = 0; i < 5; i++)
		if ((a[i] = a[i] ? a[i] : 13 > *x || a[i] == 1) && *x != 1) {
			*x = a[i];
			*y = b[i];
		}
	return 9;
}

/* a 是点数 .. b 是花色 *//*赌博--梭哈*/
static void money_suoha_tran(char *a, char *b, char *c) {
	int i;
	for (i = 0; i < 5; i++) {
		a[i] = c[i] % 13;
		if (!a[i])
			a[i] = 13;
	}

	for (i = 0; i < 5; i++)
		b[i] = c[i] / 13;
}

/*赌博--梭哈*/
static void money_suoha_check(char *p, char *q, char *r, char *cc) {
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

//同花顺、铁枝、葫、同花、顺、三条、兔胚、胚、一只
/*赌博--梭哈*/
static void show_style(int my, int cpu) {
	char *style[9] = {
		"同花顺", "铁枝", "葫芦", "同花", "顺子",
		"三条", "兔胚", "单胚", "一张"
	};
	move(5, 26);
	prints("\033[41;37;1m%s\033[m", style[cpu - 1]);
	move(15, 26);
	prints("\033[41;37;1m%s\033[m", style[my - 1]);
	sprintf(sty, "我的牌\033[44;1m%s\033[m..电脑的牌\033[44;1m%s\033[m", style[my - 1], style[cpu - 1]);
}

/*赌博--轮盘*/
static void russian_gun() {
	int i;
	int line;
	int first;
	char uident[IDLEN + 1];
	char title[STRLEN], buf[256];

	clear();
	money_show_stat("兵马俑赌场大厅");
	////slowaction
	if (currentuser.stay < 86400) {
		move(7, 4);
		prints_nofmt("小孩子来拼什么命，找你们家大人来。\n乖，给你一块兵马俑币买糖吃");
		pressanykey();
		return;
	}
	//slowaction

	move(6, 4);
	if (!loadValue(currentuser.userid, "invitation", 1)) {
		prints_nofmt("值班秘书上下打量了你半晌，说道：“这里没有这种赌法，你走吧。”");
		pressanykey();
	} else {
		saveValue(currentuser.userid, "invitation", -1, 1);
		whoTakeCharge(3, uident);
		prints_nofmt("值班经理看完你递过的邀请函，又四下看了看，才说道：“请随我来。”");
		pressanykey();
		clear();
		money_show_stat("兵马俑赌场密室");
		move(4, 4);
		prints_nofmt("这里是一间不大的密室，很静，静得可怕。空气中似乎有血腥的味道...");
		move(6, 4);
		prints("%s就坐在面前，微笑道：“你敢来赴约，算你有胆量！请坐。”", uident);
		pressanykey();
		move(8, 4);
		prints_nofmt("一把左轮手枪扔到了桌上...一个蒙面男子坐到了你面前...");
		move(10, 4);
		if (askyn("你知道俄罗斯轮盘赌的规则吗？", NA, NA) == NA) {
			move(12, 4);
			prints("%s叹了口气，说道：“算了，别死了连怎么回事都不知道。你走吧！”", uident);
			pressanykey();
			return;
		}
		move(12, 4);
		if (askyn("好！那就开始吧，祝你好运。你是客，你要先来吗？", NA, NA) == YEA) {
			first = 1;
		} else {
			first = 0;
		}
		clear();
		money_show_stat("兵马俑赌场密室");
		set_safe_record();
		currentuser.dietime = currentuser.stay + 4444 * 60;
		substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
		for (i = 0, line = 6; i < 6; i += 2) {
			srandom(time(0));
			move(line++, 4);
			if (first) {
				prints_nofmt("你拿起左轮手枪，对准自己的太阳穴，咬咬牙扣动了扳机...");
			} else {
				prints_nofmt("蒙面男子拿起左轮手枪，对准自己的太阳穴，扣动了扳机...");
			}
			pressanykey();
			if (random() % (6 - i)) {
				move(line++, 4);
				if (first) {
					prints_nofmt("\033[1;33m咔哒！\033[m一声响过，你惊魂未定之余发现自己还活着...");
				} else {
					prints_nofmt("\033[1;33m咔哒！\033[m一声响过，蒙面男子毫发无损...");
				}
				move(line++, 4);
				if (5 - i == 1 && first) {
					if (first) {
						move(line++, 4);
						prints("蒙面男子绝望的颤抖着，用哀求的眼神看着%s。", uident);
						move(line++, 4);
						prints_nofmt("\033[1;31m砰！一声巨响，蒙面男子血流满地...\033[m");
						pressanykey();
						move(line++, 4);
						prints("%s擦了擦还在冒烟的手枪，又放进了口袋里。", uident);
						break;
					} else {
						prints_nofmt("蒙面男子得意的狞笑着，把枪口对准了你...");
						prints_nofmt("\033[1;31m砰！一声巨响，你只觉得意识瞬间模糊...\033[m");
						pressanykey();
						Q_Goodbye(0, NULL, NULL);
					}
				}

				if (first) {
					prints_nofmt("现在轮到蒙面男子...");
				} else {
					prints_nofmt("现在轮到你了...你的心脏\033[5;31m砰砰\033[m跳得厉害...");
				}
				pressanykey();
				if (random() % (5 - i)) {
					move(line++, 4);
					if (first) {
						prints_nofmt("\033[1;33m咔哒！\033[m一声响过，蒙面男子毫发无损...");
					} else {
						prints_nofmt("\033[1;33m咔哒！\033[m一声响过，你惊魂未定之余发现自己还活着...");
					}
				} else {
					move(line++, 4);
					if (first) {
						prints_nofmt("\033[1;31m砰！一声巨响，蒙面男子血流满地...\033[m");
						break;
					} else {
						prints_nofmt("\033[1;31m砰！一声巨响，你只觉得意识瞬间模糊...\033[m");
						pressanykey();
						Q_Goodbye(0, NULL, NULL);
					}
				}
			} else {
				move(line++, 4);
				if (first) {
					prints_nofmt("\033[1;31m砰！一声巨响，你只觉得意识瞬间模糊...\033[m");
					pressanykey();
					Q_Goodbye(0, NULL, NULL);
				} else {
					prints_nofmt("\033[1;31m砰！一声巨响，蒙面男子血流满地...\033[m");
					break;
				}
			}
		}
		move(line++, 4);
		set_safe_record();
		currentuser.dietime = 0;
		substitute_record(PASSFILE, &currentuser, sizeof (currentuser), usernum);
		prints_nofmt("一切都结束了...你一刻也不愿留在这恐怖的地方，尽管你得到了200000 兵马俑币。");
		saveValue(currentuser.userid, MONEY_NAME,200000, MAX_MONEY_NUM);

		sprintf(title, "%s参与赌博(轮盘)", currentuser.userid);
		sprintf(buf, "%s在轮盘赢了%d兵马俑币", currentuser.userid, 200000);
		millionairesrec(title, buf, "赌博轮盘");

		pressanykey();
	}
}

static void policereport(char *str) {
	FILE *se;
	char fname[STRLEN], title[STRLEN];

	sprintf(fname, "bbstmpfs/tmp/police.%s.%05d", currentuser.userid, uinfo.pid);
	if ((se = fopen(fname, "w")) != NULL) {
		fprintf(se, "%s", "【此篇文章由兵马俑大富翁自动张贴系统发表】\n\n");
		fprintf(se, "%s", str);
		fclose(se);
		sprintf(title,"[报告]%s", str);
		postfile(fname, "Police", title, 2);
		unlink(fname);
	}
}

/*警署*/
static int money_cop() {
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
		nomoney_show_stat("兵马俑警署");
		move(8, 16);
		prints_nofmt("打击犯罪，维持治安！");
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1]报案 [2]自首 [3]通缉榜 [4]刑警队 [5]署长办公室 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
			case '1':
				clear();
				nomoney_show_stat("兵马俑警署接待厅");
				move(4, 4);
				prints_nofmt("如果您遭遇抢劫或偷窃，如果您有任何犯罪嫌疑人的线索，请向警方报告。\n    正确举报有奖，诽谤他人受罚");
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				if(money<5000)
				{
					break;
					return 0;
				}
				move(6, 4);
				prints_nofmt("警民合作，共创安定大好局面！");
				move(7, 4);
				usercomplete("举报谁？", uident);
				move(8, 4);
				if (uident[0] == '\0')
					break;
				if (!getuser(uident)) {
					prints_nofmt("错误的使用者代号...");
					pressreturn();
					break;
				}
				if (lookupuser.dietime > 0) {
					prints_nofmt("人都死了，警察也没办法...");
					//saveValue(currentuser.userid, MONEY_NAME,-2000, MAX_MONEY_NUM);
					pressreturn();
					break;
				}
				if (loadValue(uident, "freeTime", 2000000000) > 0) {
					prints_nofmt("这个人已经被警署监禁了。");
					//saveValue(currentuser.userid, MONEY_NAME, -2000, MAX_MONEY_NUM);
					pressanykey();
					break;
				}
				if (loadValue(uident, "rob", 50) == 0) {
					prints_nofmt("这个人最近很安分啊！你不要诽谤别人哦！");
					saveValue(currentuser.userid, MONEY_NAME, -2000, MAX_MONEY_NUM);
					pressanykey();
					break;
				}
				if (seek_in_file(DIR_MC "criminals_list", uident)) {
					prints_nofmt("此人已经被警署通缉了，不过警署仍然对你表示感谢。");
					saveValue(currentuser.userid, MONEY_NAME, -2000, MAX_MONEY_NUM);
					pressanykey();
					break;
				}
				char local_buf[STRLEN];
				getdata(8, 4, "简述案情：", local_buf, 40, DOECHO, YEA);
				if (local_buf[0] == '\0')
					break;
				move(9, 4);
				if (askyn("\033[1;33m你向警方提供的上述信息真实吗？\033[0m", NA, NA) == NA)
					break;
				saveValue(currentuser.userid, MONEY_NAME, +2000, MAX_MONEY_NUM);
				strcpy(buf, uident);
				strcat(buf, "\t");
				strcat(buf, local_buf);
				ytht_add_to_file(DIR_MC "criminals_list", buf);
				move(10, 4);
				prints_nofmt("警方非常感谢您提供的线索，我们将尽力尽快破案。");
				pressanykey();
				sprintf(buf, "ID: %s\n案情: %s", uident, local_buf);
				sprintf(local_buf, "%s报案",currentuser.userid);
				millionairesrec(local_buf, buf, "");
				break;
			case '2':
				clear();
				nomoney_show_stat("兵马俑警署接待厅");
				move(4, 4);
				prints_nofmt("坦白从宽，抗拒从严。");
				move(5, 4);
				prints_nofmt("主动交代自己的罪行,将减轻一半的处罚。");
				move(7, 4);
				robTimes = loadValue(currentuser.userid, "rob", 50);
				if (robTimes == 0) {
					prints_nofmt("你有病啊？没事跑来认罪...");
					pressanykey();
					break;
				}
				if (time(0) <12*3600 + loadValue(currentuser.userid, "last_rob", 2000000000)) {
					prints_nofmt("这么快就来认罪，不行");
					pressanykey();
					break;
				}
				sprintf(genbuf, "\033[1;31m你的律师提醒你,如果认罪你将被处以%d天监禁。还要认罪吗?\033[0m", robTimes / 2 + 1);
				move(8, 4);
				if (askyn(genbuf, NA, NA) == YEA) {
					move(9, 4);
					if (loadValue(currentuser.userid, "freeTime", 2000000000) > 0) {
						prints_nofmt("你已经被监禁了，想认罪也来不及了。");
						pressanykey();
						Q_Goodbye(0, NULL, NULL);
					}
					prints_nofmt("悬崖勒马,还来得及。好好改造吧！");
					saveValue(currentuser.userid, "freeTime", time(0) + 86400 * (robTimes / 2 + 1), 2000000000);
					saveValue(currentuser.userid, "rob", -robTimes, 50);
					ytht_del_from_file(DIR_MC "criminals_list", currentuser.userid, true);
					pressanykey();
					Q_Goodbye(0, NULL, NULL);
				} else {
					move(9, 4);
					prints_nofmt("躲得了初一，躲不过十五。好自为知吧！");
					pressanykey();
				}
				break;
			case '3':
				clear();
				move(1, 0);
				prints_nofmt("兵马俑警署当前通缉的犯罪嫌疑人:");
				listfilecontent(DIR_MC "criminals_list");
				pressanykey();
				break;
			case '4':
				clear();
				nomoney_show_stat("刑警队");
				move(6, 4);
				money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
				if (seek_in_file(DIR_MC "mingren", currentuser.userid)) {
					move(5, 4);
					prints_nofmt("    \033[1;32m  不要惹事\033[m");
					pressanykey();
					break;
				}
				if (!seek_in_file(DIR_MC "policemen", currentuser.userid)||money<5000) {
					prints_nofmt("这里不是公园！有事到接待厅去,别到处乱闯！\n");
					prints_nofmt("不拿钱就去抓人，出了事连医药费都没有\n");
					pressanykey();
					break;
				}
				usercomplete("今次行动的目标犯罪嫌疑人是:", uident);
				move(7, 4);
				if (uident[0] == '\0')
					break;
				if (!(id = getuser(uident))) {
					prints_nofmt("错误的使用者代号...");
					pressreturn();
					break;
				}
				if(ifinprison(lookupuser.userid))
				{prints_nofmt("都已经在监狱了，还要抓他去那里。。。。");
					pressreturn();
					break;
				}
				if (lookupuser.dietime > 0) {
					prints_nofmt("人都死了，警察也没办法...");
					pressreturn();
					break;
				}
				if (loadValue(uident, "freeTime", 2000000000) > 0) {
					prints_nofmt("这个人已经被警署监禁了。");
					pressanykey();
					break;
				}
				if (time(0) < 5*60 + loadValue(currentuser.userid, "last_catch", 2000000000)) {
					prints_nofmt("抓人不用这么积极吧");
					pressanykey();
					break;
				}
				robTimes = loadValue(uident, "rob", 50);
				if (robTimes == 0) {
					prints_nofmt("这个人最近很安分啊！会不会是搞错了?\n");
					prints_nofmt("他告你们滥用职权，你赔了他5000的精神损失费");
					saveValue(currentuser.userid, MONEY_NAME, -5000, MAX_MONEY_NUM);
					saveValue(uident, MONEY_NAME, +5000, MAX_MONEY_NUM);
					pressanykey();
					break;
				}
				escTime = loadValue(uident, "escTime", 2000000000);
				if (escTime > 0 && time(0) < escTime + 3600) {
					prints_nofmt("该犯罪嫌疑人刚刚逃脱,一时半会儿恐怕还找不到。");
					pressanykey();
					break;
				}
				move(8, 4);
				if (askyn("准备好了吗?", NA, NA) == YEA) {
					saveValue(currentuser.userid, "last_catch", -2000000000, 2000000000);
					saveValue(currentuser.userid, "last_catch", time(0), 2000000000);
					move(10, 4);
					prints("\033[1;33m根据线人提供的消息,你终于找到了%s藏匿的地方。\033[0m", uident);
					move(11, 4);
					seized = 0;
					srandom(time(0));
					if (askyn("\033[5;31m要破门而入么?\033[0m", NA, NA) == YEA) {
						move(12, 4);
						prints_nofmt("\033[1;31m你拔出手枪，一脚将门踹开，冲了进去，喊道：“警察！”\033[0m");
						move(13, 4);
						if (random() % 10 == 0) {
							prints("\033[1;32m里面空无一人，窗户是打开的。看来%s刚刚跳窗而逃。\033[0m", uident);
							move(14, 4);
							prints_nofmt("你只好懊恼而返。大好的机会啊！");
							saveValue(uident, "escTime", -2000000000, 2000000000);
							saveValue(uident, "escTime", time(0), 2000000000);
							pressanykey();
							sprintf(buf,"%s逃脱",uident);
							policereport(buf);
							sprintf(title, "%s参与抓人", currentuser.userid);
							millionairesrec(title, buf, "警署活动");
							break;
						} else {
							if (robTimes < 3 && random() % 10) {
								prints("\033[1;32m%s一看到你顿时吓傻了,乖乖举起了双手。\033[0m", uident);
								sprintf(genbuf,
										"兵马俑警署在今天的抓捕行动中抓获一名绰号%s的匪徒\n警方透露抓捕过程非常顺利\n\n"
										"警署希望不良分子引以为戒，\n 本站居民高度赞扬警署职员为民除害 ", uident);
								deliverreport("[新闻]兵马俑警署擒获一名匪徒",genbuf);
								//saveValue(currentuser.userid, MONEY_NAME, robTimes*80000*0.3, MAX_MONEY_NUM);
								move(14, 4);
								seized = 1;
							} else if (robTimes >= 3 && robTimes < 6 && random() % 5) {
								prints("\033[1;32m%s一看到你就要跳窗逃跑，但你眼明手快，一枪击中其小腿。\033[0m", uident);
								sprintf(genbuf,
										"兵马俑警署在今天的抓捕行动中抓获一名绰号%s的匪徒\n警方透露此人在与警察的枪战中负伤\n\n"
										"警署希望犯罪分子不要拒捕，\n 以免造成不必要的伤亡 ", uident);
								deliverreport("[新闻]兵马俑警署擒获一名匪徒",genbuf);
								//saveValue(currentuser.userid, MONEY_NAME, robTimes*80000*0.3, MAX_MONEY_NUM);
								move(14, 4);
								seized = 1;
							} else if (robTimes >= 6 && robTimes < 8 && random() % 3) {
								prints("\033[1;32m%s向你猛扑过来，你来不及开枪，只好和其扭成一团...\033[0m", uident);
								pressanykey();
								move(14, 4);
								prints("\033[1;32m经过一番搏斗，你终于制服了%s。不过你也累得够呛，还被咬了一口。\033[0m", uident);
								sprintf(genbuf,
										"兵马俑警署在今天的抓捕行动中抓获一名绰号%s的匪徒\n警方透露有警员在枪战中负伤\n\n"
										"警署希望犯罪分子不要拒捕，\n 以免造成不必要的伤亡 ", uident);
								deliverreport("[新闻]兵马俑警署擒获一名匪徒",genbuf);
								//saveValue(currentuser.userid, MONEY_NAME, robTimes*80000*0.3, MAX_MONEY_NUM);
								move(15, 4);
								seized = 1;
							} else if (robTimes >= 8 && random() % 2) {
								prints("\033[5;32m原来%s也有枪！你们同时瞄准了对方！\033[0m", uident);
								pressanykey();
								move(14, 4);
								prints("\033[1;35m枪声响过，%s痛苦的捂住了手腕，鲜血直流。你安然无恙，庆幸啊！\033[0m", uident);
								//saveValue(currentuser.userid, MONEY_NAME, robTimes*80000*0.3, MAX_MONEY_NUM);
								move(15, 4);
								seized = 1;
							}
							if (seized) {
								prints("你将%s押回了警署,这个坏蛋被处以%d天监禁。你又立了一功！", uident, robTimes);
								saveValue(uident, "rob", -robTimes, 50);
								saveValue(uident, "freeTime", time(0) + 86400 * robTimes, 2000000000);
								sprintf(genbuf, "你被兵马俑警署抓获，并处以%d天的监禁。", robTimes);
								mail_buf_slow(uident, "你被警察逮捕", genbuf, "BMY_FBI");
								ytht_del_from_file(DIR_MC "criminals_list", uident, true);
								sprintf(buf, "%s\t%d", uident, robTimes);
								ytht_add_to_file(DIR_MC "imprison_list", buf);
								pressanykey();
								sprintf(buf,"抓获%s，并监禁%d天", uident, robTimes);
								policereport(buf);
								sprintf(title, "%s参与抓人", currentuser.userid);
								millionairesrec(title, buf, "警署活动");
								break;
							} else {
								saveValue(uident, "escTime", -2000000000, 2000000000);
								saveValue(uident, "escTime", time(0), 2000000000);
							}
							if (random() % 20) {
								prints("\033[5;32m原来%s也有枪！你们同时瞄准了对方！\033[0m", uident);
								move(14, 4);
								if (askyn("\033[1;31m是否紧急躲避？", NA, NA) == YEA) {
									move(15, 4);
									if (random() %3) {
										prints_nofmt("你一个后仰，子弹带着风声从你面门飞过。");
										move(16, 4);
										prints("%s趁机逃走了，你不知道是该懊恼还是庆幸。", uident);
										pressanykey();
										sprintf(buf,"%s逃走", uident);
										policereport(buf);
										sprintf(title, "%s参与抓人", currentuser.userid);
										millionairesrec(title, buf, "警署活动");
										break;
									} else {
										prints_nofmt("你想躲避，但是已经来不及了。你只觉得胸口一股热血喷了出来...");
										move(16,4);
										//saveValue(currentuser.userid, MONEY_NAME, 50000, MAX_MONEY_NUM);
										prints_nofmt("\033[1;31m你壮烈牺牲了。\033[0m");
										die = 1;
									}
								}
								else {
									move(15, 4);
									prints_nofmt("\033[1;31m狭路相逢勇者胜！你毫不犹豫的开枪了！\033[0m");
									move(16, 4);
									if (random() % 3) {
										prints("\033[1;35m枪声响过，%s被击中头部，当场死亡。\033[0m", uident);
										move(17, 4);
										prints("你狠狠的踢了一脚%s的尸体，同时暗自庆幸今天走运。", uident);
										//saveValue(currentuser.userid, MONEY_NAME, 50000, MAX_MONEY_NUM);
										saveValue(uident, "rob", -robTimes/2, 50);
										lookupuser.dietime = lookupuser.stay + 999*60;
										substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
										sprintf(genbuf,
												"兵马俑警署在今天的抓捕行动中击毙一名绰号%s的匪徒\n警方透露此人有持枪拒捕行为\n\n"
												"警署希望不良分子引以为戒，\n 本站居民高度赞扬警署职员为民除害 ", uident);
										deliverreport("[新闻]兵马俑警署击毙一名匪徒",genbuf);
										mail_buf_slow(uident, "你被警察击毙","你在抵抗警察抓捕的过程中，被一枪击中头部死亡。善恶终有报啊！","BMY_FBI");
										ytht_del_from_file(DIR_MC "criminals_list", uident, true);
										pressanykey();
										sprintf(buf,"击毙%s", uident);
										policereport(buf);
										sprintf(title, "%s参与抓人", currentuser.userid);
										millionairesrec(title, buf, "警署活动");
										break;
									} else {
										prints_nofmt("枪声响过，你只觉得胸口一股热血喷了出来...");
										move(17, 4);
										prints_nofmt("\033[1;31m你壮烈牺牲了。\033[0m");
										die = 1;
									}
								}
							} else {
								prints("\033[5;32m原来%s身藏手雷，一见逃跑无望，%s只好引爆手雷和你同归于尽！\033[0m", uident, uident);
								move(14, 4);
								prints ("\033[1;31m你壮烈牺牲了。\033[0m");
								die = 1;
								saveValue(uident, "rob", -robTimes / 2,50);
								lookupuser.dietime = lookupuser.stay + 999 * 60;
								substitute_record(PASSFILE, &lookupuser, sizeof(lookupuser), id);
								sprintf(genbuf,
										"兵马俑警署在今天的抓捕行动中击毙一名绰号%s的匪徒\n警方透露有警员在枪战中中弹伤势严重\n\n"
										"警署表示一定全力抢救，\n  ", uident);
								deliverreport("[新闻]兵马俑警署击毙一名匪徒",genbuf);
								mail_buf_slow(uident, "你被警察逮捕","你在抵抗警察抓捕的过程中，引爆身上的手雷，与警察同归于尽。","BMY_FBI");
								ytht_del_from_file(DIR_MC "criminals_list", uident, true);
							}
							if (die) {
								set_safe_record();
								saveValue(uident, "rob", -robTimes/2, 50);
								currentuser.dietime = currentuser.stay + 999 * 60;
								substitute_record(PASSFILE, &currentuser, sizeof(currentuser), usernum);
								set_safe_record();
								sprintf(buf,
										"一名警署警员在今天的抓捕行动中不幸殉职。兵马俑警署谨向英雄致以最高的敬意，"
										"\n并发誓将严惩罪犯。警署没有透露警员的真实姓名");
								deliverreport("[新闻]兵马俑警署一名警察殉职", buf);
								pressanykey();
								sprintf(buf,"%s在抓捕%s时英勇牺牲了", currentuser.userid, uident);
								policereport(buf);
								sprintf(title, "%s参与抓人", currentuser.userid);
								millionairesrec(title, buf, "警署活动");
								Q_Goodbye(0, NULL, NULL);
							}
						}
					} else {
						move(12, 4);
						prints_nofmt("你决定还是先不要打草惊蛇的好...");
						pressanykey();
					}
				}
				break;
			case '5':
				clear();
				nomoney_show_stat("署长办公室");
				char name[20];
				whoTakeCharge2(8, name);
				whoTakeCharge(8, uident);
				if (strcmp(currentuser.userid, uident)) {
					move(6, 4);
					prints("警花%s拦住了你,说道:“署长%s现在很忙,没时间接待你。”", name,uident);
					move(8, 4);
					if (!seek_in_file(DIR_MC "policemen", currentuser.userid) && !slowclubtest("Police",currentuser.userid)){
						if (askyn("你是想加入警署吗？", NA, NA) == YEA) {
							sprintf(genbuf, "%s 要加入警署", currentuser.userid);
							mail_buf(genbuf, "BMYpolice", genbuf);
							move(14, 4);
							prints_nofmt("好的，我会通知署长的。");
						}
					}
					pressanykey();
					break;
				} else {
					move(6, 4);
					prints_nofmt("请选择操作代号:");
					move(7, 6);
					prints_nofmt("1. 任命警员                  2. 解职警员");
					move(8, 6);
					prints_nofmt("3. 警员名单                  4. 监禁名单");
					move(9, 6);
					prints_nofmt("5. 辞职                      6. 退出");
					ch = igetkey();
					switch (ch) {
						case '1':
							move(12, 4);
							usercomplete("任命谁为警员？", uident);
							move(13, 4);
							if (uident[0] == '\0')
								break;
							if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
								prints_nofmt("错误的使用者代号...");
								pressanykey();
								break;
							}
							if (seek_in_file (DIR_MC "policemen", uident)) {
								prints_nofmt("该ID已经是警员了。");
								pressanykey();
								break;
							}
							if (askyn("确定吗？", NA, NA) == YEA) {
								ytht_add_to_file(DIR_MC "policemen", uident);
								sprintf(genbuf, "%s 任命你为兵马俑警署警员", currentuser.userid);
								mail_buf("警署希望你不畏强暴，打击犯罪，公正无私，不怕牺牲！", uident, genbuf);
								move(14, 4);
								prints_nofmt("任命成功。");
								sprintf(genbuf, "%s行使警署管理权限",currentuser.userid);
								sprintf(buf, "任命%s为警署警员", uident);
								millionairesrec(genbuf, buf, "BMYpolice");
								pressanykey();
							}
							break;
						case '2':
							move(12, 4);
							usercomplete("解职哪位警员？", uident);
							move(13, 4);
							if (uident[0] == '\0')
								break;
							if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
								prints_nofmt("错误的使用者代号...");
								pressanykey();
								break;
							}
							if (!seek_in_file(DIR_MC "policemen", uident)) {
								prints_nofmt("该ID不是兵马俑警署警员。");
								pressanykey();
								break;
							}
							if (askyn("确定吗？", NA, NA) == YEA) {
								ytht_del_from_file(DIR_MC "policemen", uident, true);
								sprintf(genbuf, "%s 解除你的兵马俑警署警员职务", currentuser.userid);
								mail_buf("感谢你一直以来的工作，并希望你作为市民继续为维护治安而尽义务。", uident, genbuf);
								move(14, 4);
								prints_nofmt("解职成功。");
								sprintf(genbuf, "%s行使警署管理权限",currentuser.userid);
								sprintf(buf, "解除%s的警署警员身份", uident);
								millionairesrec(genbuf, buf, "BMYpolice");
								pressanykey();
							}
							break;
						case '3':
							clear();
							move(1, 0);
							prints_nofmt("目前兵马俑警署警员名单：");
							listfilecontent(DIR_MC "policemen");
							pressanykey();
							break;
						case '4':
							clear();
							move(1, 0);
							prints_nofmt("目前兵马俑警署监禁罪犯名单：");
							move(2, 0);
							prints_nofmt("罪犯ID\t监禁天数");
							listfilecontent(DIR_MC "imprison_list");
							pressanykey();
							break;
						case '5':
							move(12, 4);
							if (askyn("您真的要辞职吗？", NA, NA) == YEA) {
								/*	ytht_del_from_file(MC_BOSS_FILE, "police");
									sprintf(genbuf,
									"%s 宣布辞去兵马俑警署署长职务",
									currentuser.userid);
									deliverreport(genbuf,
									"兵马俑金融中心对其一直以来的工作表示感谢，祝以后顺利！");
									move(14, 4);
									prints
									("好吧，既然你意已决，警署也只有批准。");
									quit = 1;
									pressanykey();
									*/
								sprintf(genbuf, "%s 要辞去兵马俑警署署长职务", currentuser.userid);
								mail_buf(genbuf, "millionaires", genbuf);
								move(14, 4);
								prints_nofmt("好吧，已经发信告知总管了");
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

//slowaction
/*检查权限*/
static int Allclubtest(char *id) {
	if (slowclubtest("Beggar",id))
	return 1;
	else if (slowclubtest("Rober",id))
		return 1;
	else if (slowclubtest("Police",id))
		return 1;
	else if (slowclubtest("killer",id))
		return 1;
	else  return 0;
}

//slowaction
static int slowclubtest(char *board,char *id) {
	char buf[256];
	sprintf(buf, "boards/%s/club_users", board);
	return seek_in_file(buf, id);
}

//股票开盘
static int stop_buy() {
	FILE *f_fp;
	char fname[125];
	sprintf(fname,"%s/stopbuy", DIR_MC);
	f_fp=fopen(fname,"r");
	if(f_fp!=NULL) {
		fclose(f_fp);
		return 1;
	}
	return 0;
}

//结婚
static int money_marry() {
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
		n=100;		//一次载入一百条
		truncate(MC_MARRY_RECORDS,100*sizeof(struct MC_Marry));
		}
		*/
	filesize = sizeof(struct MC_Marry) * n;
	//加载信息
	marryMem = loadData(MC_MARRY_RECORDS, buffer, filesize);
	if (marryMem == (void *) -1)
		return -1;
	//处理各种婚姻状态变化
	marry_refresh(marryMem,n);
	//查看是否有求婚申请
	for(j=0; j<n; j++){
		if(marryMem[j].enable == 0) continue;
		if(marryMem[j].status != MAR_COURT) continue;
		if(!strcmp(marryMem[j].bride, currentuser.userid)){
			break;
		}
	}
	//避免接受多个人求婚
	for(i=0;i<n;i++){
		if(!strcmp(marryMem[i].bride, currentuser.userid)){
			if(marryMem[i].status == MAR_MARRIED || marryMem[i].status == MAR_MARRYING){
				marryMem[j].status =  MAR_COURT_FAIL;
				marryMem[j].enable = 0;
				sprintf(genbuf, "%s 已经接受了别人的求婚\n",marryMem[j].bride);
				strcat(genbuf,"\n别灰心，三条腿的蛤蟆不多见，两条腿的姑娘还不有的是~~");
				sprintf(buf, "对不起，%s 不能接受您的求婚", marryMem[j].bride);
				mail_buf(genbuf, marryMem[j].bridegroom, buf);
				j=n;
			}
		}
	}

	if(j<n){
		money_show_stat("兵马俑教堂");
		move(5, 4);
		flag = 1;
		sprintf(buf, "幸福的人儿，您是否接受 \033[1;33m%s\033[m 的求婚？",marryMem[j].bridegroom);
		if (askyn(buf, NA, NA) == NA) {
			move(6, 4);
			prints_nofmt("切，他算那根葱~~");
			flag = 0;
			marryMem[j].enable = 0;
		}else{
			move(6, 4);
			prints_nofmt("*^^*，终于等到这一天~~");
			marryMem[j].enable = 1;
			marryMem[j].marry_t = time(NULL) + 24*60*60;		//婚礼一天后举行
			marryMem[j].status = MAR_MARRYING;
			flag = 1;
		}

		move(7, 4);
		prints_nofmt("您要对他说些什么吗？[可以写3行喔]");
		bzero(note, sizeof (note));
		for (i = 0; i < 3; i++) {
			getdata(8 + i, 0, ": ", note[i], STRLEN - 1, DOECHO, NA);
			if (note[i][0] == '\0')
				break;
		}
		sprintf(genbuf, "%s %s了您的求婚\n",marryMem[j].bride, flag?"接受":"拒绝");
		if (i > 0) {
			sprintf(buf, "\033[1;33m%s\033[m%s的说:\n", marryMem[j].bride, flag?"羞答答":"冷冷");
			strcat(genbuf,buf);
			for (k = 0; k < i; k++){
				strcat(genbuf,note[k]);
				strcat(genbuf,"\n");
			}
		}
		if(flag)
			strcat(genbuf,"\n别站着傻乐啦，快去兵马俑教堂准备婚礼吧!");
		else strcat(genbuf,"\n别灰心，三条腿的蛤蟆不多见，两条腿的姑娘还不有的是~~");

		sprintf(buf, "[%s]%s %s了您的求婚",flag?"恭喜":"通知", marryMem[j].bride, flag?"接受":"拒绝");
		mail_buf(genbuf, marryMem[j].bridegroom, buf);
		if (flag){
			if (i > 0) {
				sprintf(genbuf, "\033[1;33m%s\033[m羞答答的说:\n", marryMem[j].bride);
				for (k = 0; k < i; k++){
					strcat(genbuf,note[k]);
					strcat(genbuf,"\n");
				}
			}
			sprintf(buf,"[号外]%s接受了%s的求婚",marryMem[j].bride,marryMem[j].bridegroom);
			if (note[0][0] == '\0')
				deliverreport(buf,"\n");
			else
				deliverreport(buf, genbuf);
		}
		move(13, 4);
		prints_nofmt("我们已经发信通知了对方");
		pressanykey();
	}


	while (!quit) {
		clear();
		money_show_stat("兵马俑教堂");
		if(freshflag){
			show_welcome(MC_MAEEY_SET,4,22);
			freshflag =0;
		}
		//move(6, 4);
		//prints("欢迎您走进婚姻的围城");
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1]参加婚礼 [2]求婚 [3]准备婚礼 [4]离婚 [5]登记表 [6]婚姻管理办 [Q]离开\033[m");
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
static int marry_admin(struct MC_Marry *marryMem, int n) {
	int offset, ch, quit = 0;
	int count, count2, count3, no=0;
	char uident[IDLEN + 1], uident2[IDLEN + 1], buf[2048], title[STRLEN], ans[8];
	char jhdate[30], lhdate[30], lhz[2048];
	size_t filesize;
	//struct MC_Marry *marryMem;
	struct MC_Marry *mm;
	void *buffer = NULL;
	time_t local_now_t = time(NULL);

	nomoney_show_stat("兵马俑婚姻管理办公室");
	whoTakeCharge2(10, uident2);
	whoTakeCharge(10, uident);

	if (!seek_in_file(MC_ADMIN_FILE, currentuser.userid)
			&& !seek_in_file(MC_MARRYADMIN_FILE, currentuser.userid)
			&& strcmp(currentuser.userid, uident)) {
		move(6, 4);
		prints("秘书%s拦住了你,说道:“主任们现在正忙着打麻将，没时间接待!”", uident2);
		pressanykey();
		return 0;
	}

	while (!quit) {
		nomoney_show_stat("兵马俑婚姻管理办公室");
		move(t_lines - 2, 0);
		prints_nofmt( "\033[1;44m 选 \033[1;46m [1]查询婚姻状况 [2]办理离婚 [3]设置管理人员 [4]发送离婚通知书             \n"
				"\033[1;44m 单 \033[1;46m [5]强制解除婚约 [Q]离开                                               ");
		ch = igetkey();
		switch (ch) {
			case '1':
				clear();
				move(6, 4);
				usercomplete("查谁的情况？", uident);
				if (uident[0] == '\0')
					break;
				if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
					move(7, 4);
					prints_nofmt("错误的使用者代号...");
					pressreturn();
					break;
				}
				marry_query_records(uident);
				break;
			case '2':
				clear();
				move(4, 4);
				prints_nofmt("\033[1;31;5m输入ID时请注意大小写\033[m");
				getdata(6, 4, "请输入女方ID: ", uident, 13, DOECHO, YEA);
				getdata(7, 4, "请输入男方ID: ", uident2, 13, DOECHO, YEA);
				if (askyn("确定吗？", NA, NA) == YEA){
					if (!file_exist(MC_MARRY_RECORDS_ALL)){
						clear();
						move(9, 4);
						prints_nofmt("没有任何记录!");
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
							sprintf(buf, "结婚时间为%s，确定吗？", get_simple_date_str(&mm->marry_t));
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
						sprintf(title, "%s和%s劳燕分飞", uident, uident2);
						sprintf(buf,"　　虽然大富翁婚姻管理办公室主任%s多次调解，"
								"但是%s（女方）和%s（男方）的爱情已经走到尽头，"
								"征询双方意见后，大富翁婚姻管理办公室决定批准"
								"二人离婚，愿二人今后生活顺利。\n",
								currentuser.userid, uident, uident2);
						deliverreport(title, buf);
						sprintf(title, "[公告]%s和%s离婚", uident, uident2);
						sprintf(lhz,
								"\033[0m               \033[47m                                                \033[40m \n"
								"               \033[47m  \033[41m\033[1;32m☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆\033[47m  \033[40m \n"
								"               \033[47m  \033[41m☆                                        ☆\033[47m  \033[40m \n"
								"               \033[47m  \033[41m☆               \033[37m离 婚 证                 \033[32m☆\033[47m  \033[40m \n"
								"               \033[47m  \033[41m☆                                        ☆\033[47m  \033[40m \n"
								"               \033[47m  \033[41m☆                    \033[34m[\033[37m婚字\033[34m]第 \033[37m%5.5d \033[34m号   \033[32m☆\033[47m  \033[40m \n"
								"               \033[47m  \033[41m☆   \033[37m持证人                               \033[32m☆\033[47m  \033[40m \n"
								"               \033[47m  \033[41m☆   \033[4;37m%-12.12s\033[0;1;41m（女）\033[4m%-12.12s\033[0;1;41m（男） \033[32m☆\033[47m  \033[40m \n"
								"               \033[47m  \033[41m☆   \033[37m结婚日期：\033[4m%s\033[0;41m                 \033[1;32m☆\033[47m  \033[40m \n"
								"               \033[47m  \033[41m☆      \033[37m申请离婚，经审查符合兵马俑大富翁  \033[32m☆\033[47m  \033[40m \n"
								"               \033[47m  \033[41m☆   \033[37m关于离婚的规定，准予登记，发给此证。 \033[32m☆\033[47m  \033[40m \n"
								"               \033[47m  \033[41m☆          \033[37m发证机关 大富翁婚姻管理办公室 \033[32m☆\033[47m  \033[40m \n"
								"               \033[47m  \033[41m☆          \033[37m发证日期 %s           \033[32m☆\033[47m  \033[40m \n"
								"               \033[47m  \033[41m☆                                        ☆\033[47m  \033[40m \n"
								"               \033[47m  \033[41m☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆\033[47m  \033[40m \n"
								"               \033[47m                                                \033[40m \n"
								"                                                                \033[m\n",
								no, uident, uident2, jhdate, lhdate);
						deliverreport(title, lhz);
						sprintf(title,"婚姻管理办公室主任%s办理离婚业务", currentuser.userid);
						millionairesrec(title, buf, "Marriage");
						sprintf(buf,"大富翁婚姻管理办公室同意您与%s的离婚要求，愿您今后生活顺利。\n", uident);
						mail_buf_slow(uident2, "大富翁婚姻管理办公室同意您的离婚要求", buf,"XJTU-XANET");
						sprintf(buf,"大富翁婚姻管理办公室同意您与%s的离婚要求，愿您今后生活顺利。\n", uident2);
						mail_buf_slow(uident, "大富翁婚姻管理办公室同意您的离婚要求", buf,"XJTU-XANET");
						prints_nofmt("完成操作!");

						//再次确认是不是已经结婚，决定是否从已结婚名单删除
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
								ytht_del_from_file(MC_MARRIED_LIST, uident, true);
						} else {
							if (!seek_in_file(MC_MARRIED_LIST, uident))
								ytht_add_to_file(MC_MARRIED_LIST, uident);
						}
						if (count3==0){
							if (seek_in_file(MC_MARRIED_LIST, uident2))
								ytht_del_from_file(MC_MARRIED_LIST, uident2, true);
						} else {
							if (!seek_in_file(MC_MARRIED_LIST, uident2))
								ytht_add_to_file(MC_MARRIED_LIST, uident2);
						}
					} else
						prints_nofmt("没有找到任何相关记录!");
					pressreturn();
				}
				break;

			case '3':
				clear();
				if (!seek_in_file(MC_ADMIN_FILE, currentuser.userid)) {
					move(6, 4);
					prints_nofmt("总管才可以操作哟");
					pressanykey();
					break;
				}

				while (1) {
					clear();
					prints_nofmt("设定婚姻管理办公室人员\n");
					count = listfilecontent(MC_MARRYADMIN_FILE);
					if (count)
						getdata(1, 0, "(A)增加 (D)删除 (E)离开 [E]: ", ans, 7, DOECHO, YEA);
					else
						getdata(1, 0, "(A)增加  (E)离开[E]: ", ans, 7, DOECHO, YEA);
					if (*ans == 'A' || *ans == 'a') {
						move(1, 0);
						usercomplete("增加人员: ", uident);
						if (*uident != '\0') {
							if (seek_in_file(MC_MARRYADMIN_FILE, uident)) {
								move(2, 0);
								prints_nofmt("输入的ID 已经存在!");
								pressreturn();
								break;
							}
							move(4, 0);
							if (askyn("真的要添加么?", NA, NA) == YEA){
								ytht_add_to_file(MC_MARRYADMIN_FILE, uident);
								sprintf(title, "%s行使管理权限(婚姻)", currentuser.userid);
								sprintf(buf, "添加%s为婚姻管理人员", uident);
								millionairesrec(title, buf, "Marriage");
								//deliverreport(titlebuf, repbuf);
								//mail_buf(repbuf, uident, titlebuf);
							}
						}
					} else if ((*ans == 'D' || *ans == 'd') && count) {
						move(1, 0);
						namecomplete("删除人员: ", uident);
						move(1, 0);
						clrtoeol();
						if (uident[0] != '\0') {
							if (!seek_in_file(MC_MARRYADMIN_FILE, uident)) {
								move(2, 0);
								prints_nofmt("输入的ID 不存在!");
								pressreturn();
								break;
							}
							move(4, 0);
							if (askyn("真的要删除么?", NA, NA)==YEA){
								ytht_del_from_file(MC_MARRYADMIN_FILE, uident, true);
								sprintf(title, "%s行使管理权限(婚姻)", currentuser.userid);
								sprintf(buf, "取消%s的婚姻管理职务", uident);
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
				usercomplete("请输入收信方ID: ", uident);
				if (uident[0] == '\0')
					break;
				if (!ythtbbs_cache_UserTable_search_usernum(uident)) {
					move(7, 4);
					prints_nofmt("错误的使用者代号...");
					pressreturn();
					break;
				}
				move(7, 4);
				usercomplete("请输入提出方ID: ", uident2);
				if (uident2[0] == '\0')
					break;
				if (!ythtbbs_cache_UserTable_search_usernum(uident2)) {
					move(8, 4);
					prints_nofmt("错误的使用者代号...");
					pressreturn();
					break;
				}

				if (askyn("确定吗？", NA, NA) == YEA){
					sprintf(lhdate, "%s", get_simple_date_str(&local_now_t));
					lhdate[10]=0;
					sprintf(buf,"尊敬的%s：\n"
							"　　本婚姻管理中心受理原告%s"
							"诉你离婚纠纷一案，现依大富翁"
							"关于离婚的规定向你送达。自本"
							"通知发出之日起经过6日即视为送达。"
							"请你认真阅读兵马俑大富翁关于离婚的"
							"相关规定，并在6日内做出答复，规定外的"
							"财产分割及损失赔偿请与原告联系，"
							"定于第7日（遇节假日顺延）审核此诉讼"
							"请求，逾期（以30日为限）将依照有关规定"
							"判决。\n\n%80.80s\n%80.80s\n",
							uident, uident2, " 兵马俑大富翁婚姻管理中心", lhdate);
					mail_buf_slow(uident, "离婚通知书", buf, "Marriage");

					sprintf(title, "%s诉%s离婚纠纷一案开庭审理", uident2, uident);
					sprintf(buf,
							"兵马俑婚姻办公室受理%s离婚请求，"
							"已向%s发出了离婚通知书。", uident2, uident);
					deliverreport(title, buf);

					sprintf(title,"婚姻管理办公室主任%s发送离婚通知书", currentuser.userid);
					millionairesrec(title, buf, "Marriage");
					prints_nofmt("完成操作!");
					pressanykey();
				}
				break;

			case '5':
				clear();
				showAt(2, 4, "\033[1;31m此功能慎用! \033[m", 0);
				showAt(4, 4, "\033[1;32m输入ID时请注意大小写\033[m", 0);
				getdata(6, 4, "请输入女方ID: ", uident, 13, DOECHO, YEA);
				getdata(7, 4, "请输入男方ID: ", uident2, 13, DOECHO, YEA);
				if (askyn("确定吗？", NA, NA) == YEA){
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
						sprintf(buf, "订婚时间为%s，确定吗？", get_simple_date_str(&mm->court_t));
						if (askyn(buf, NA, NA) == YEA){
							mm->enable=0;
							mm->status = MAR_COURT_FAIL;
							count++;
						}

					}
					move(12, 4);
					if (count>0){
						saveData(marryMem, filesize);
						sprintf(title,"婚姻管理办公室主任%s强制解除婚约", currentuser.userid);
						sprintf(buf,"强制解除%s与%s的婚约", uident, uident2);
						millionairesrec(title, buf, "Marriage");
						prints_nofmt("完成操作!");

						//再次确认是不是已经结婚，决定是否从已结婚名单删除
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
								ytht_del_from_file(MC_MARRIED_LIST, uident, true);
						} else {
							if (!seek_in_file(MC_MARRIED_LIST, uident))
								ytht_add_to_file(MC_MARRIED_LIST, uident);
						}
						if (count3==0){
							if (seek_in_file(MC_MARRIED_LIST, uident2))
								ytht_del_from_file(MC_MARRIED_LIST, uident2, true);
						} else {
							if (!seek_in_file(MC_MARRIED_LIST, uident2))
								ytht_add_to_file(MC_MARRIED_LIST, uident2);
						}
					} else
						prints_nofmt("没有找到任何相关记录!");
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

static int marry_recordlist(struct MC_Marry *marryMem, int n) {
	int ch, quit = 0;
	while (!quit) {
		nomoney_show_stat("兵马俑教堂档案馆");
		move(8, 16);
		prints_nofmt(" 求婚的，已婚的，离婚的...全在这记着呢，看吧");
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1]婚事登记表 [2]状况记录表 [3]个人查询 [Q]离开\033[m");
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

//查询婚姻状况
/*add by macintosh@BMY 2006.10*/
static int marry_query_records(char *id) {
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
	//time_t local_now_t;
	int n;

	if (!file_exist(MC_MARRY_RECORDS_ALL)){
		clear();
		move(6, 4);
		prints_nofmt("没有任何记录!");
		pressanykey();
		return 0;
	}
	n = get_num_records(MC_MARRY_RECORDS_ALL, sizeof(struct MC_Marry));
	if (n <= 0)
		return 0;
	filesize = sizeof(struct MC_Marry) * n;
	//加载信息
	marryMem = loadData(MC_MARRY_RECORDS_ALL, buffer, filesize);
	if (marryMem == (void *) -1)
		return -1;
	money_show_stat("兵马俑教堂档案馆");
	move(5, 0);
	prints("                       \033[1;31m个人婚姻情况查询结果 (%s)\033[m         ", id);
	move(6, 0);
	snprintf(buf, sizeof buf,"%-6.6s %-20.20s %-10.10s %-10.10s %-16.16s %-4.4s %-6.6s","编号","主题","新娘","新郎","求/结/婚礼时间","到访","状态");
	prints_nofmt(buf);
	move(7, 0);
	prints_nofmt("--------------------------------------------------------------------------------------");
	pages = n / 10 + 1;
	for(i = 0; ;i++) {	//i用于控制页数
		//local_now_t = time(NULL);
		for(j=0; j<10; j++){
			move(8 + j , 0);
			clrtoeol();
		}
		count = 0;
		for(j = 0; count < 10; j++) {	//每屏显示最多10
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
			if(strcmp(mm->bride, id) && strcmp(mm->bridegroom, id))
				continue;
			count++;
			switch(mm->status){
				case MAR_COURT:
				case MAR_COURT_FAIL:
					ytht_strsncpy(timestr,get_simple_date_str(&mm->court_t), sizeof(timestr));
					break;
				case MAR_MARRIED:
				case MAR_MARRYING:
					ytht_strsncpy(timestr,get_simple_date_str(&mm->marry_t), sizeof(timestr));
					break;
				case MAR_DIVORCE:
					ytht_strsncpy(timestr,get_simple_date_str(&mm->divorce_t), sizeof(timestr));
					break;
				default:
					ytht_strsncpy(timestr,get_simple_date_str(&mm->marry_t), sizeof(timestr));
			}
			char local_buf[STRLEN * 2];
			snprintf(local_buf, sizeof(local_buf), "[%4d] %-20.20s %-10.10s %-10.10s %-16.16s %4d \033[1;%dm%-6.6s\033[m",
					offset,mm->subject,mm->bride,mm->bridegroom,timestr,mm->visitcount,
					(mm->status ==MAR_MARRYING)?32:37,marry_status[mm->status]);
			prints_nofmt(local_buf);
		}
		if ((offset >= n ) && (count <= 0)){
			move(9, 0);
			prints_nofmt("没有找到任何相关记录!");
			pressreturn();
			return 0;
		} else {
			getdata(19, 4, "[B]前页 [C]下页 [Q]退出: [C]", buf, 2, DOECHO, YEA);
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
//状况记录表
//包括结婚，离婚，求婚失败
static int marry_all_records() {
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
	//time_t local_now_t;
	int n;

	if (!file_exist(MC_MARRY_RECORDS_ALL)){
		clear();
		move(6, 4);
		prints_nofmt("咦？怎么没有记录，难得还从未有关婚事活动？！");
		pressanykey();
		return 0;
	}
	n = get_num_records(MC_MARRY_RECORDS_ALL, sizeof(struct MC_Marry));
	if (n <= 0)
		return 0;
	filesize = sizeof(struct MC_Marry) * n;
	//加载信息
	marryMem = loadData(MC_MARRY_RECORDS_ALL, buffer, filesize);
	if (marryMem == (void *) -1)
		return -1;
	money_show_stat("兵马俑教堂档案馆");
	move(5, 4);
	prints_nofmt("                             \033[1;31m教堂婚事状况记录表\033[m         ");
	move(6, 0);
	snprintf(buf,sizeof buf,"%-6.6s %-20.20s %-10.10s %-10.10s %-16.16s %-4.4s %-6.6s","编号","主题","新娘","新郎","求/结/婚礼时间","到访","状态");
	prints_nofmt(buf);
	move(7, 0);
	prints_nofmt("--------------------------------------------------------------------------------------");
	pages = n / 10 + 1;
	for(i = 0; ;i++) {	//i用于控制页数
		//local_now_t = time(NULL);
		for(j=0; j<10; j++){
			move(8 + j , 0);
			clrtoeol();
		}
		count = 0;
		for(j = 0; count < 10; j++) {	//每屏显示最多10
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
					ytht_strsncpy(timestr,get_simple_date_str(&mm->court_t), sizeof(timestr));
					break;
				case MAR_MARRIED:
				case MAR_MARRYING:
					ytht_strsncpy(timestr,get_simple_date_str(&mm->marry_t), sizeof(timestr));
					break;
				case MAR_DIVORCE:
					ytht_strsncpy(timestr,get_simple_date_str(&mm->divorce_t), sizeof(timestr));
					break;
				default:
					ytht_strsncpy(timestr,get_simple_date_str(&mm->marry_t), sizeof(timestr));
			}
			char local_buf[STRLEN * 2];
			snprintf(local_buf, sizeof(local_buf), "[%4d] %-20.20s %-10.10s %-10.10s %-16.16s %4d \033[1;%dm%-6.6s\033[m",
					offset,mm->subject,mm->bride,mm->bridegroom,timestr,mm->visitcount,
					(mm->status ==MAR_MARRYING)?32:37,marry_status[mm->status]);
			prints_nofmt(local_buf);
			//offset++;
		}
		getdata(19, 4, "[B]前页 [C]下页 [Q]退出: [C]", buf, 2, DOECHO, YEA);
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

//求婚结婚登记表
//婚事登记表
static int marry_active_records(struct MC_Marry *marryMem, int n) {
	int i,j;
	char buf[STRLEN];
	int offset=0;
	int pages;
	int count;
	struct MC_Marry *mm;
	char timestr[STRLEN];
	//time_t local_now_t;

	money_show_stat("兵马俑教堂档案馆");
	move(5, 4);
	prints_nofmt("                             \033[1;31m教堂婚事登记表\033[m         ");
	move(6, 0);
	snprintf(buf, sizeof buf,"%-6.6s %-20.20s %-10.10s %-10.10s %-16.16s %-4.4s %-6.6s","编号","主题","新娘","新郎","求婚/婚礼时间","到访","状态");
	prints_nofmt(buf);
	move(7, 0);
	prints_nofmt("--------------------------------------------------------------------------------------");
	pages = n / 10 + 1;
	for(i = 0; ;i++) {	//i用于控制页数
		//local_now_t = time(NULL);
		count = 0;
		for(j=0;j<10;j++) {
			move(8 + j , 0);
			clrtoeol();
		}
		for(j = 0; count < 10; j++) {	//每屏显示最多10
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
					ytht_strsncpy(timestr,get_simple_date_str(&mm->court_t), sizeof(timestr));
					break;
				case MAR_MARRIED:
				case MAR_MARRYING:
					ytht_strsncpy(timestr,get_simple_date_str(&mm->marry_t), sizeof(timestr));
					break;
				case MAR_DIVORCE:
					ytht_strsncpy(timestr,get_simple_date_str(&mm->divorce_t), sizeof(timestr));
					break;
				default:
					ytht_strsncpy(timestr,get_simple_date_str(&mm->marry_t), sizeof(timestr));
			}
			char local_buf[STRLEN * 2];
			snprintf(local_buf, sizeof(local_buf), "[%4d] %-20.20s %-10.10s %-10.10s %-16.16s %4d \033[1;%dm%-6.6s\033[m",
					offset,mm->subject,mm->bride,mm->bridegroom,timestr,mm->visitcount,
					(mm->status ==MAR_MARRYING)?32:37,
					(mm->status ==MAR_MARRYING)?((mm->marry_t > time(NULL))?"筹备中":"婚礼中"):(marry_status[mm->status]));
			prints_nofmt(local_buf);
			//offset++;
		}
		getdata(19, 4, "[B]前页 [C]下页 [Q]退出: [C]", buf, 2, DOECHO, YEA);
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

//遍历结婚表，处理各种情况编号
static int marry_refresh(struct MC_Marry *marryMem, int n) {
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
			//求婚失败
			mm = &marryMem[i];
			mm->status = MAR_COURT_FAIL;
			mm->enable = 0;
			append_record(MC_MARRY_RECORDS_ALL, mm, sizeof(struct MC_Marry));	//转入记录
		}
		/*else if(marryMem[i].status==MAR_MARRIED){
		//已结婚，转入记录表，这种情况是为了此次修改，一般不会出现
		mm = &marryMem[i];
		mm->status = MAR_MARRIED;
		append_record(MC_MARRY_RECORDS_ALL, mm, sizeof(struct MC_Marry));	//转入记录
		mm->enable = 0;	//在这边active表中作废
		if (!seek_in_file(MC_MARRIED_LIST, mm->bride))
		ytht_add_to_file(MC_MARRIED_LIST, mm->bride);
		if (!seek_in_file(MC_MARRIED_LIST, mm->bridegroom))
		ytht_add_to_file(MC_MARRIED_LIST, mm->bridegroom);
		}
		*/
		else if(marryMem[i].status==MAR_MARRYING
				&& marryMem[i].unused[0]!='d'
				&& !(marryMem[i].marry_t > local_now_t)){
			mm = &marryMem[i];
			mm->unused[0]='d';
			sprintf(filetmp, MY_BBS_HOME "/bbstmpfs/tmp/%s.%d", currentuser.userid, getpid());
			fp = fopen(filetmp,"w");
			if(!fp) continue;
			fprintf(fp,"     \033[1;31m%s\033[m和\033[1;32m%s\033[m的婚礼正式开始，欢迎大家光临\n\n"
					"     让我们共同祝福他们吧！\n\n",mm->bride,mm->bridegroom);
			fclose(fp);
			sprintf(buf,"[公告]%s和%s的婚礼正式开始！",mm->bride,mm->bridegroom);
			postfile(filetmp, MC_BOARD, buf , 1);
		}else if(marryMem[i].status==MAR_MARRYING && local_now_t-marryMem[i].marry_t >4*60*60){
			//婚礼4小时后结束
			mm = &marryMem[i];
			mm->status = MAR_MARRIED;
			append_record(MC_MARRY_RECORDS_ALL, mm, sizeof(struct MC_Marry));	//转入记录
			mm->enable = 0;	//在这边active表中作废
			if (!seek_in_file(MC_MARRIED_LIST, mm->bride))
				ytht_add_to_file(MC_MARRIED_LIST, mm->bride);
			if (!seek_in_file(MC_MARRIED_LIST, mm->bridegroom))
				ytht_add_to_file(MC_MARRIED_LIST, mm->bridegroom);
			sprintf(invpath,"%s/M.%ld.A",DIR_MC_MARRY,mm->invitationfile);
			sprintf(setpath,"%s/M.%ld.A",DIR_MC_MARRY,mm->setfile);
			sprintf(visitpath,"%s/M.%ld.A",DIR_MC_MARRY, mm->visitfile);
			sprintf(filetmp, MY_BBS_HOME "/bbstmpfs/tmp/%s.%d", currentuser.userid, getpid());
			fp = fopen(filetmp,"w");
			if(!fp) continue;
			fprintf(fp,"     \033[1;31m%s\033[m和\033[1;32m%s\033[m的婚礼到此结束，感谢大家的光临，"
					"让我们共同祝福他们幸福甜蜜的婚后生活。\n\n",mm->bride,mm->bridegroom);
			fprintf(fp,"    \033[1;36m以下是这次婚礼的情况记录和统计\033[m\n\n");
			fprintf(fp,"婚礼时间: %s\n",get_date_str(&mm->marry_t));
			fprintf(fp,"所收礼金: \033[1;31m%d\033[m 兵马俑币\n",mm->giftmoney);
			fprintf(fp,"到访人次: \033[1;31m%d\033[m\n",mm->visitcount);
			fp2= fopen(visitpath,"r");
			if(fp2){
				while(!feof(fp2)){
					if(fgets(buf,sizeof(buf),fp2) == NULL) break;
					fprintf(fp,"%s",buf);
				}
				fclose(fp2);
			}
			fprintf(fp,"\n\033[1m请柬: \033[m\n");
			fp2= fopen(invpath,"r");
			if(fp2){
				while(!feof(fp2)){
					if(fgets(buf,sizeof(buf),fp2) == NULL) break;
					fprintf(fp,"%s",buf);
				}
				fclose(fp2);
			}
			fprintf(fp,"\n\n\033[1m婚礼布景: \033[m\n");
			fp2= fopen(setpath,"r");
			if(fp2){
				while(!feof(fp2)){
					if(fgets(buf,sizeof(buf),fp2) == NULL) break;
					fprintf(fp,"%s",buf);
				}
				fclose(fp2);
			}
			fclose(fp);
			sprintf(buf,"[恭喜]%s和%s大婚已成",mm->bride,mm->bridegroom);
			postfile(filetmp, MC_BOARD, buf , 1);
		}
	}
	return 1;
}

static int marry_givemoney(struct MC_Marry *mm) {
	char uident[IDLEN + 1];
	//	void *buffer = NULL;
	int i;
	char note[3][STRLEN];
	char buf[STRLEN];
	time_t local_now_t = time(NULL);
	int num;

	move(4,4);
	if(mm->marry_t > local_now_t){
		prints_nofmt("婚礼还未开始,请稍后再来");
		pressanykey();
		return 0;
	}
	else prints_nofmt("婚礼进行中，送礼的好时机");

	if(!strcmp(mm->bride, currentuser.userid) || !strcmp(mm->bridegroom, currentuser.userid)){
		move(7 ,4);
		prints_nofmt("哈哈，给自家人送钱就不用通过银行了吧...");
		pressanykey();
		return 0;
	}

	move(5,4);
	if(local_now_t%2==1){	//新郎新娘各一半的机会受礼
		strncpy(uident,mm->bride,IDLEN);
		prints("礼金将送到新娘\033[1;31m%s\033[m的腰包",uident);
	}else{
		strncpy(uident,mm->bridegroom,IDLEN);
		prints("礼金将送到新郎\033[1;32m%s\033[m的腰包",uident);
	}

	getdata(6, 4, "转帐多少兵马俑币？[100000]", buf, 10, DOECHO, YEA);
	num = atoi(buf);
	if (buf[0]=='\0')
		num=100000;
	if (num<100000) {
		move(7, 4);
		prints_nofmt("人家新婚大喜呢，这么点钱你也好意思拿出手，小气，哼哼:(");
		pressanykey();
		return 0;
	}
	if (num>MAX_MONEY_NUM)
		num=MAX_MONEY_NUM;
	move(7, 4);
	snprintf(buf, STRLEN - 1, "确定转帐 %d 兵马俑币吗？", num);
	if (askyn(buf, NA, NA) == NA)
		return 0;
	if (loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM) < num) {
		move(8, 4);
		prints_nofmt("给你们送礼来啦...掏了半天口袋...啊？！居然没带钱？");
		pressanykey();
		return 0;
	}

	move(7, 4);
	prints_nofmt("有话要说吗？[可以写3行喔]");
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

	sprintf(genbuf, "\033[1;32m%s\033[m给您送礼 \033[1;31m%d\033[m 兵马俑币 结婚礼金。\n\n",currentuser.userid,num);
	if (i > 0) {
		int j;
		sprintf(buf, "附言:\n");
		strcat(genbuf,buf);
		for (j = 0; j < i; j++){
			strcat(genbuf,note[j]);
			strcat(genbuf,"\n");
		}
	}

	sprintf(buf, "[恭贺新婚]%s给您送贺礼来啦", currentuser.userid);
	mail_buf(genbuf,uident, buf);
	sprintf(buf, "[恭贺新婚]%s恭贺%s和%s新婚大喜", currentuser.userid, mm->bride, mm->bridegroom);
	sprintf(genbuf, "送\033[1;31m红包\033[m一个\n\n恭祝新郎新娘结婚大喜，百年好合，早生贵子:)");
	deliverreport(buf, genbuf);

	sprintf(buf, "%s参加%s和%s的婚礼(送红包)", currentuser.userid, mm->bride, mm->bridegroom);
	sprintf(genbuf, "%s给%s送红包 (%d兵马俑币)",  currentuser.userid, uident, num);
	millionairesrec(buf, genbuf, "参加婚礼");

	move(14 ,4);
	prints_nofmt("礼金已送达。");
	pressanykey();
	return 0;
}

//精简的日期表
static char *get_simple_date_str(time_t *tt) {
	struct tm *tm;
	static char timestr[200];
	if(tt==0) return "------";
	tm = localtime(tt);
	sprintf(timestr,"%02d/%02d/%02d %02d:%02d",
			tm->tm_year+1900, tm->tm_mon+1,tm->tm_mday, tm->tm_hour, tm->tm_min);
	//prints(timestr);
	return timestr;
}

//参加婚礼
static int marry_attend(struct MC_Marry *marryMem, int n) {
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

	money_show_stat("兵马俑教堂");
	move(5, 4);
	prints_nofmt("                             \033[1;31m教堂婚礼登记表\033[m         ");
	move(6, 0);
	snprintf(buf, sizeof buf,"%-6.6s %-20.20s %-10.10s %-10.10s %-16.16s %-4.4s %-6.6s","编号","主题","新娘","新郎","婚礼时间","到访","状态");
	prints_nofmt(buf);
	move(7, 0);
	prints_nofmt("--------------------------------------------------------------------------------------");
	pages = n / 10 + 1;
	for(i = 0; ;i++) {	//i用于控制页数
		local_now_t = time(NULL);
		for(j=0;j<10;j++) {
			move(8 + j , 0);
			clrtoeol();
		}
		count = 0;
		for(j = 0; count < 10; j++) {	//每屏显示最多10支股票
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
			char local_buf[STRLEN * 2];
			snprintf(local_buf, sizeof(local_buf), "[%4d] %-20.20s %-10.10s %-10.10s %-16.16s %4d \033[1;%dm%-6.6s\033[m",
					offset,mm->subject,mm->bride,mm->bridegroom,get_simple_date_str(&mm->marry_t),mm->visitcount,
					(mm->marry_t > local_now_t)?37:32,(mm->marry_t > local_now_t)?"筹备中":"进行中");
			prints_nofmt(local_buf);
			//offset++;
		}
		getdata(19, 4, "[B]前页 [C]下页 [S]选择 [Q]退出: [C]", buf, 2, DOECHO, YEA);
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
		getdata(t_lines-5, 4, "请选择您要参加的婚礼编号[ENTER放弃]:", buf, 3, DOECHO, YEA);
		if (buf[0] == '\0')
			return 0;
		index = atoi(buf);
		if (index >= 0 && index < n && marryMem[index].status == MAR_MARRYING)
			break;
	}
	mm = &marryMem[index];
	mm->visitcount++;	//到访记录
	local_now_t = time(NULL);
	strncpy(visitfile,DIR_MC_MARRY,STRLEN-1);
	if(mm->visitfile==0){
		t = trycreatefile(visitfile, "M.%ld.A", local_now_t, 100);
		if (t < 0)
			return -1;
		mm->visitfile = t;
	}else sprintf(visitfile,"%s/M.%ld.A",DIR_MC_MARRY, mm->visitfile);
	if(!seek_in_file(visitfile, currentuser.userid))
		ytht_add_to_file(visitfile, currentuser.userid);

	while (!quit) {
		money_show_stat("兵马俑教堂");
		if(freshflag){
			sprintf(filepath,"%s/M.%ld.A",DIR_MC_MARRY,mm->setfile);
			show_welcome(filepath,4,22);
			freshflag =0;
		}
		move(4, 10);
		local_now_t = time(NULL);
		if(mm->marry_t > local_now_t)
			prints_nofmt("婚礼还未开始");
		else prints_nofmt("婚礼进行中...");
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1]送礼金 [2]送鲜花 [3]送贺卡 [Q]离开\033[m");
		ch = igetkey();
		switch (ch) {
			case '1':
				freshflag = 1;
				if (seek_in_file(DIR_MC "mingren", currentuser.userid)){
					move(5, 4);
					prints_nofmt("    \033[1;32m  黄马褂啊，公公说你们去送猪头吧 \033[m");
					pressanykey();
					break;
				}
				marry_givemoney(mm);
				break;
			case '2':
				freshflag = 1;
				move(5,4);
				if(mm->marry_t > local_now_t){
					prints_nofmt("婚礼还未开始,请稍后再来");
					pressanykey();
					break;
				}
				if(!strcmp(mm->bride, currentuser.userid) || !strcmp(mm->bridegroom, currentuser.userid)){
					move(7 ,4);
					prints_nofmt("哈哈，给自家人送东西就不用这么麻烦了吧...");
					pressanykey();
					break;
				}
				if(local_now_t%2==1){	//新郎新娘各一半的机会受礼
					strncpy(uident,mm->bride,IDLEN);
					prints("鲜花将送到新娘\033[1;31m%s\033[m的手中",uident);
				}else{
					strncpy(uident,mm->bridegroom,IDLEN);
					prints("鲜花将送到新郎\033[1;32m%s\033[m的手中",uident);
				}
				pressanykey();
				if (shop_present(1, "鲜花",uident) == 9) {
					sprintf(buf, "[恭贺新婚]%s恭贺%s和%s新婚大喜", currentuser.userid, mm->bride, mm->bridegroom);
					sprintf(genbuf, "送\033[1;31m鲜花\033[m一束\n\n恭祝新郎新娘结婚大喜，百年好合，早生贵子:)");
					deliverreport(buf, genbuf);
				}
				break;
			case '3':
				freshflag = 1;
				move(5,4);
				if(mm->marry_t > local_now_t){
					prints_nofmt("婚礼还未开始,请稍后再来");
					pressanykey();
					break;
				}
				if(!strcmp(mm->bride, currentuser.userid) || !strcmp(mm->bridegroom, currentuser.userid)){
					move(7 ,4);
					prints_nofmt("哈哈，给自家人送东西就不用这么麻烦了吧...");
					pressanykey();
					break;
				}
				if(local_now_t%2==1){	//新郎新娘各一半的机会受礼
					strncpy(uident,mm->bride,IDLEN);
					prints("贺卡将送到新娘\033[1;31m%s\033[m的手中",uident);
				}else{
					strncpy(uident,mm->bridegroom,IDLEN);
					prints("贺卡将送到新郎\033[1;32m%s\033[m的手中",uident);
				}
				pressanykey();
				if(shop_present(2, "贺卡",uident) == 9) {
					sprintf(buf, "[恭贺新婚]%s恭贺%s和%s新婚大喜", currentuser.userid, mm->bride, mm->bridegroom);
					sprintf(genbuf, "送\033[1;32m贺卡\033[m一张\n\n恭祝新郎新娘结婚大喜，百年好合，早生贵子:)");
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

static int PutMarryRecord(struct MC_Marry *marryMem, int n, struct MC_Marry *new_mm) {
	int i, slot = -1;

	for(i = 0; i < n; i++) {
		if(marryMem[i].enable == 0 && slot == -1)	//放到第一个空位
			slot = i;
	}
	if(slot >= 0) {
		memcpy(&marryMem[slot], new_mm, sizeof(struct MC_Marry));
	}else{
		append_record(MC_MARRY_RECORDS, new_mm, sizeof(struct MC_Marry));
	}
	return slot;
}

//求婚
static int marry_court(struct MC_Marry *marryMem, int n) {
	char note[3][STRLEN];
	char buf[STRLEN];
	struct MC_Marry mm;
	int i;
	char uident[IDLEN+2];

	money_show_stat("教堂登记处");
	if (seek_in_file(MC_MARRIED_LIST, currentuser.userid)){
		move(5, 4);
		prints_nofmt("你已经结婚了啊，小心告你重婚罪！");
		pressanykey();
		return 0;
	}
	for(i=0;i<n;i++){
		if(!strcmp(marryMem[i].bride,currentuser.userid) || !strcmp(marryMem[i].bridegroom,currentuser.userid) ){
			if( marryMem[i].status == MAR_COURT ){
				move(5, 4);
				prints_nofmt("喂！你正求着婚呢，这么不专一，让mm怎么相信你");
				pressanykey();
				return 0;
			} else if(marryMem[i].status == MAR_MARRYING){
				move(5, 4);
				prints_nofmt("有没搞错，婚礼正在举行呢，又要求婚，脑袋没发烧吧~~");
				pressanykey();
				return 0;
			}
		}
	}

	move(5,4);
	prints_nofmt("婚姻非同儿戏，本站不提倡离婚，请慎重考虑！！");
	move(6,4);
	if (askyn("您下定觉心要求婚了吗？", NA, NA) == NA) {
		move(7, 4);
		prints_nofmt("唉，还是再等等吧....");
		pressanykey();
		return 0;
	}

	money_show_stat("教堂登记处");
	move(5, 4);
	usercomplete("哪位mm这么幸福？", uident);
	if (uident[0] == '\0')
		return 0;
	if(!getuser(uident)) {
		move(6, 4);
		prints_nofmt("没有这么个mm啊....");
		pressanykey();
		return 0;
	}
	if(!strcmp(uident, currentuser.userid)){
		move(6, 4);
		prints_nofmt("喂，醒醒吧，再自恋也不能向自己求婚啊！");
		pressanykey();
		return 0;
	}
	if (seek_in_file(MC_MARRIED_LIST, uident)){
		move(6, 4);
		prints_nofmt("人家已经结婚了呀，当第三者不好的！");
		pressanykey();
		return 0;
	}
	for(i=0;i<n;i++){
		if(!strcmp(marryMem[i].bride,uident) || !strcmp(marryMem[i].bridegroom,uident) ){
			if( marryMem[i].status == MAR_COURT && !strcmp(marryMem[i].bridegroom,uident)){
				move(5, 4);
				prints_nofmt("你死心吧，人家已经向别人求婚了....");
				pressanykey();
				return 0;
			} else if(marryMem[i].status == MAR_MARRYING){
				move(5, 4);
				prints_nofmt("有没搞错，人家正结婚呢，捣什么乱啊~~");
				pressanykey();
				return 0;
			}
		}
	}


	move(7, 4);
	prints_nofmt("有话想对mm说吗？[可以写3行喔]");
	bzero(note, sizeof (note));
	for (i = 0; i < 3; i++) {
		getdata(8 + i, 0, ": ", note[i], STRLEN - 1, DOECHO, NA);
		if (note[i][0] == '\0')
			break;
	}

	sprintf(genbuf, "         \033[1;31m求婚\033[m\n\n");
	if (i > 0) {
		int j;
		sprintf(buf, "\033[1;33m%s\033[m温情的说:\n", currentuser.userid);
		strcat(genbuf,buf);
		for (j = 0; j < i; j++){
			strcat(genbuf,note[j]);
			strcat(genbuf,"\n");
		}
	}
	strcat(genbuf,"\n大家齐声: 嫁给他吧，嫁给他吧~~");

	move(11, 4);
	sprintf(buf,"结婚可不是小事，要想好了哦，您下定觉心向%s求婚了吗?",uident);
	if (askyn(buf, YEA, NA) == NA) {
		move(12, 4);
		prints_nofmt("唉，可恨紧要关头我怎么就没这个勇气~~");
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
	ytht_strsncpy(mm.subject, "追逐爱情的朝阳", sizeof mm.subject);
	mm.setfile = 0;
	mm.invitationfile = 0;

	PutMarryRecord(marryMem, n, &mm);

	sprintf(buf,"[号外]%s向%s深情的求婚",currentuser.userid,uident);
	deliverreport(buf, genbuf);
	sprintf(buf, "[恭喜]%s深情的向您求婚", currentuser.userid);
	mail_buf(genbuf, uident, buf);
	move(13, 4);
	prints("恭喜您，您的浓意深情已送到%s手中，等待好消息吧~~",uident);
	pressanykey();
	return 0;
}


static char *get_date_str(time_t *tt) {
	struct tm *tm;
	static char timestr[200];
	tm = localtime(tt);
	sprintf(timestr,"%04d年\033[1;33m%02d\033[m月\033[1;33m%02d\033[m日 \033[1;33m%02d\033[m时:\033[1;33m%02d\033[m分",
			tm->tm_year+1900, tm->tm_mon+1,tm->tm_mday, tm->tm_hour, tm->tm_min);
	//prints(timestr);
	return timestr;
}

static int marry_selectday(struct MC_Marry *mm) {
	int ch, quit = 0;
	time_t local_now_t = time(NULL);
	//mm->marry_t = local_now_t;
	if(mm->marry_t < local_now_t){
		move(5,4);
		prints_nofmt("婚礼已经开始...");
		return 0;
	}
	while (!quit) {
		money_show_stat("兵马俑教堂");
		local_now_t = time(NULL);
		//限制在一年内，10分钟后举行
		if(mm->marry_t ==0) mm->marry_t = local_now_t + 600;
		if(mm->marry_t - local_now_t <600) mm->marry_t = local_now_t + 600;
		if(mm->marry_t - local_now_t >365*30*24*60*60 ) mm->marry_t = local_now_t + 365*30*24*60*60;
		move(6, 4);
		prints_nofmt("良辰吉日:  " );
		prints_nofmt(get_date_str(&mm->marry_t));
		move(10, 4);
		prints_nofmt("按键调整: ab[+-月] cd[+-日] ef[+-时] gh[+-]分 [Q]结束");

		ch = igetkey();
		switch (ch) {
			case 'a':
			case 'A':	//月
				mm->marry_t += 30*24*60*60;
				break;
			case 'b':
			case 'B':
				mm->marry_t -= 30*24*60*60;
				break;
			case 'c':	//日
			case 'C':
				mm->marry_t += 24*60*60;
				break;
			case 'd':
			case 'D':
				mm->marry_t -= 24*60*60;
				break;
			case 'e':	//时
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

static int marry_editinvitation(struct MC_Marry *mm) {
	FILE *oldfp,*newfp;
	char buf[400];
	time_t t;
	char filepath[STRLEN];
	char attach_path[STRLEN];
	char edittmp[STRLEN];
	time_t local_now_t= time(NULL);

	strncpy(filepath,DIR_MC_MARRY,STRLEN-1);
	if(mm->invitationfile == 0){
		t = trycreatefile(filepath, "M.%ld.A", local_now_t, 100);
		if (t < 0)
			return -1;
		mm->invitationfile = t;
		oldfp = fopen(MC_MAEEY_INVITATION,"r");	//初始使用默认文件
		if(oldfp){
			newfp = fopen(filepath,"w");
			if(newfp){
				while(!feof(oldfp)){
					if(fgets(buf,sizeof(buf),oldfp) == NULL)
						break;
					char *s;
					size_t i;
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
				fclose(newfp);
			}
			fclose(oldfp);
		}
	}else
		sprintf(filepath,"%s/M.%ld.A",DIR_MC_MARRY,mm->invitationfile);

	if (dashl(filepath) || !dashf(filepath))
		return -1;
	sprintf(edittmp, MY_BBS_HOME "/bbstmpfs/tmp/%s.%d", currentuser.userid, getpid());
	copyfile_attach(filepath, edittmp);
	if (vedit(edittmp, 0, YEA) < 0) {
		unlink(edittmp);
		clear();
		do_delay(-1);	/* by ylsdd */
		return -1;
	}
	snprintf(attach_path, sizeof (attach_path), PATHUSERATTACH "/%s", currentuser.userid);
	clearpath(attach_path);
	decode_attach(filepath, attach_path);
	insertattachments_byfile(filepath, edittmp, currentuser.userid);
	unlink(edittmp);
	return 1;
}


static int marry_editset(struct MC_Marry *mm) {
	FILE *oldfp,*newfp;
	char buf[400];
	time_t t;
	char filepath[STRLEN];
	char attach_path[STRLEN];
	char edittmp[STRLEN];
	time_t local_now_t= time(NULL);

	strncpy(filepath,DIR_MC_MARRY,STRLEN-1);
	if(mm->setfile == 0){
		t = trycreatefile(filepath, "M.%ld.A", local_now_t, 100);
		if (t < 0)
			return -1;
		mm->setfile = t;
		oldfp = fopen(MC_MAEEY_SET,"r");	//初始使用默认文件
		if(oldfp){
			newfp = fopen(filepath,"w");
			if(newfp){
				while(!feof(oldfp)){
					if(fgets(buf,sizeof(buf),oldfp) == NULL)
						break;
					char *s;
					size_t i;
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
				fclose(newfp);
			}
			fclose(oldfp);
		}
	}else
		sprintf(filepath,"%s/M.%ld.A",DIR_MC_MARRY,mm->setfile);

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
	snprintf(attach_path, sizeof (attach_path), PATHUSERATTACH "/%s", currentuser.userid);
	clearpath(attach_path);
	decode_attach(filepath, attach_path);
	insertattachments_byfile(filepath, edittmp, currentuser.userid);
	unlink(edittmp);

	return 1;
}


//准备婚礼
static int marry_perpare(struct MC_Marry *marryMem, int n) {
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
		prints_nofmt("教堂没有您的婚礼登记啊，您求婚了吗？她答应了吗？");
		pressanykey();
		return 0;
	}
	while (!quit) {
		money_show_stat("兵马俑教堂");
		if(freshflag){
			//sprintf(filepath,"%s/M.%d.A",DIR_MC_MARRY,mm->setfile);
			//show_welcome(filepath,4,22);
			freshflag =0;
		}
		move(5, 4);
		mm->subject[sizeof mm->subject - 1] = '\0';
		prints_nofmt(mm->subject);
		move(6, 4);
		prints("新娘:\033[1;31m%s\033[m 新郎:\033[1;32m%s\033[m ",mm->bride, mm->bridegroom);
		move(7, 4);
		prints_nofmt("没想到结次婚这么不容易，忙的晕头转向，\n    不过想想婚后的幸福生活，嘿嘿，心里那个美啊~~");
		move(t_lines - 1, 0);
		prints_nofmt("\033[1;44m 选单 \033[1;46m [1]选吉日 [2]写请柬 [3]发请贴 [4]公告天下 [5]设置主题 [6]布置教堂 [Q]离开\033[m");
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
				if (HAS_PERM(PERM_DENYMAIL, currentuser)) {
					move(5, 4);
					prints_nofmt("您被禁止发信");
					pressanykey();
					break;
				}
				freshflag = 1;
				clear();
				move(5, 4);
				if (askyn("要发请柬给所有好友吗？", YEA, NA) == NA) {
					move(6, 4);
					usercomplete("发请柬给哪位？", uident);
					if (uident[0] == '\0')
						break;
					if(!ythtbbs_cache_UserTable_search_usernum(uident)) {
						move(7, 4);
						prints_nofmt("名字记错了吧...");
						pressanykey();
						break;
					}
					if(!strcmp(uident,currentuser.userid)){
						move(10, 4);
						prints_nofmt("喂，搞错了吧，给自己发请柬啊");
						pressanykey();
						break;
					}
					sprintf(filepath,"%s/M.%ld.A",DIR_MC_MARRY,mm->invitationfile);
					sprintf(title,"%s台启,%s与%s的婚礼请柬",uident,mm->bride,mm->bridegroom);
					mail_file(filepath,uident,title);
				}else {
					sprintf(filepath,"%s/M.%ld.A",DIR_MC_MARRY,mm->invitationfile);
					for (i = 0; i  < uinfo.fnum; i++) {
						move(6, 4);
						clrtoeol();
						ythtbbs_cache_UserTable_getuserid(uinfo.friend[i], uident, sizeof(uident));
						if (!getuser(uident)) {
							prints("%s这个使用者代号是错误的.\n",uident);
							pressanykey();
							continue;
						} else if (!(lookupuser.userlevel & PERM_READMAIL)) {
							prints("无法送信给 \033[1m%s\033[m\n", lookupuser.userid);
							pressanykey();
							continue;
						} else if (!strcmp(uident, currentuser.userid)) {
							prints_nofmt("自己就不要给自己发请柬吧\n");
							pressanykey();
							continue;
						}
						sprintf(title,"%s台启,%s与%s的婚礼请柬",uident,mm->bride,mm->bridegroom);
						mail_file(filepath,uident,title);
					}
				}
				move(11, 4);
				prints_nofmt("请柬已发送");
				pressanykey();
				break;
			case '4':		//版面公告
				freshflag = 1;
				move(9, 4);
				if(mm->invitationfile == 0){
					prints_nofmt("还没写好请柬呢");
					pressanykey();
					break;
				}
				sprintf(buf,"您确定要发结婚请柬到“大富翁”版吗？");
				if (askyn(buf, YEA, NA) == NA) {
					move(10, 4);
					prints_nofmt("慢着，请柬还要再改改~~");
					pressanykey();
					break;
				}
				sprintf(filepath,"%s/M.%ld.A",DIR_MC_MARRY,mm->invitationfile);
				sprintf(title,"[请柬]敬请您阖第参加%s和%s的结婚典礼",mm->bride,mm->bridegroom);
				move(12, 4);
				if (mm->enable<3){
					postfile(filepath, MC_BOARD, title ,1);
					mm->enable++;
					prints_nofmt("您的婚事已公告天下，恭喜啦~~");
				}else
					prints_nofmt("请贴也不要总发呀，两次就好了~~");
				pressanykey();
				break;
			case '5':
				freshflag = 1;
				buf[0] = 0;
				getdata(9, 0, "请输入婚礼主题[最多28汉字]: ", buf, 56, DOECHO, NA);
				if(buf[0]){
					ytht_strsncpy(mm->subject, buf, sizeof mm->subject);
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

//离婚
static int marry_divorce() {
	clear();
	move(10, 4);
	//prints("哎呀，孩子都这么大了还有什么想不开的，快回去好好过日子吧~~");
	prints_nofmt("结婚自由，离婚自愿，交足场地费，天高任你飞");
	move(12, 4);
	prints_nofmt("若想离婚，请与大富翁婚姻管理办公室联系解决");
	pressanykey();
	return 0;
}

//黑名单
static int money_deny() {
	char uident[STRLEN];
	char ans[8];
	char msgbuf[256];
	int count;

	while (1) {
		clear();
		prints_nofmt("设定黑名单\n");
		count = listfilecontent(MC_DENY_FILE);
		if (count)
			getdata(1, 0, "(A)增加 (D)删除 (C)改变 or (E)离开 [E]: ", ans, 7, DOECHO, YEA);
		else
			getdata(1, 0, "(A)增加 or (E)离开 [E]: ", ans, 7, DOECHO, YEA);
		if (*ans == 'A' || *ans == 'a') {
			move(1, 0);
			usercomplete("把谁加入黑名单: ", uident);
			if (*uident != '\0')
				if (mc_addtodeny(uident, msgbuf, 0 ) == 1)
					mc_denynotice(1, uident, msgbuf);
		} else if ((*ans == 'C' || *ans == 'c')) {
			move(1, 0);
			usercomplete("改变谁的封禁时间或说明: ", uident);
			if (*uident != '\0')
				if (mc_addtodeny(uident, msgbuf, 1) == 1)
					mc_denynotice(3, uident, msgbuf);
		} else if ((*ans == 'D' || *ans == 'd') && count) {
			move(1, 0);
			namecomplete("从黑名单中删除谁: ", uident);
			move(1, 0);
			clrtoeol();
			if (uident[0] != '\0')
				if (ytht_del_from_file(MC_DENY_FILE, uident, true))
					mc_denynotice(2, uident, msgbuf);
		} else
			break;
	}
	clear();
	return 1;
}

static int mc_addtodeny(char *uident, char *msg, int ischange) {
	char buf[50], strtosave[256];
	char buf2[50];
	int day;
	time_t nowtime;
	char ans[8];
	int seek;

	seek = seek_in_file(MC_DENY_FILE, uident);
	if ((ischange && !seek) || (!ischange && seek)) {
		move(2, 0);
		prints_nofmt("输入的ID不对!");
		pressreturn();
		return -1;
	}
	buf[0] = 0;
	move(2, 0);
	prints("封禁对象：%s", uident);
	while (strlen(buf) < 4)
		getdata(3, 0, "输入说明(至少两字): ", buf, 40, DOECHO, YEA);

	do {
		getdata(4, 0, "输入天数(0-手动解封): ", buf2, 4, DOECHO, YEA);
		day = atoi(buf2);
	} while (day < 0);

	nowtime = time(NULL);
	if (day) {
		struct tm *tmtime;
		time_t undenytime = nowtime + day * 24 * 60 * 60;
		tmtime = gmtime(&undenytime);
		sprintf(strtosave, "%-12s %-40s %2d月%2d日解 \x1b[%ldm", uident,
				buf, tmtime->tm_mon + 1, tmtime->tm_mday,
				(long int) undenytime);
		sprintf(msg,
				"据大富翁新闻发言人今日透露，%s 因为"
				" \033[1m%s\033[m 原因被总管 %s 禁止进入大富翁游戏"
				" %d 天，希望所有大富翁人士引以为戒，"
				"共同创建和谐大富翁！",
				uident, buf, currentuser.userid, day);
	} else {
		sprintf(strtosave, "%-12s %-35s 手动解封", uident, buf);
		sprintf(msg, "据大富翁新闻发言人今日透露，%s 因为"
				" \033[1m%s \033[m原因被总管 %s 永久禁止进入大富翁游戏，"
				"希望所有大富翁人士引以为戒，共同创建和谐大富翁！",
				uident, buf, currentuser.userid);
	}
	if (ischange)
		getdata(5, 0, "真的要改变么?[Y/N]: ", ans, 7, DOECHO, YEA);
	else
		getdata(5, 0, "真的要封么?[Y/N]: ", ans, 7, DOECHO, YEA);
	if ((*ans != 'Y') && (*ans != 'y'))
		return -1;
	if (ischange)
		ytht_del_from_file(MC_DENY_FILE, uident, true);
	return ytht_add_to_file(MC_DENY_FILE, strtosave);
}


static int mc_denynotice(int action, char *user, char *msgbuf) {
	char repbuf[STRLEN];
	char repuser[IDLEN + 1];
	ytht_strsncpy(repuser, user, sizeof repuser);
	switch (action) {
		case 1:
			sprintf(repbuf, "[号外]%s被列入大富翁黑名单", repuser);
			deliverreport(repbuf, msgbuf);
			sprintf(repbuf, "%s被%s列入大富翁黑名单", user, currentuser.userid);
			mail_buf(msgbuf, user, repbuf);
			millionairesrec(repbuf, msgbuf,"");
			break;
		case 3:
			sprintf(repbuf, "%s改变%s大富翁黑名单的时间或说明", currentuser.userid, user);
			millionairesrec(repbuf, msgbuf,"");
			mail_buf(msgbuf, user, repbuf);
			break;
		case 2:
			sprintf(repbuf, "恢复 %s 进入大富翁游戏的权利", repuser);
			snprintf(msgbuf, 256, "%s %s\n" "请理解大富翁总管工作，谢谢！\n", currentuser.userid, repbuf);
			deliverreport(repbuf, msgbuf);
			millionairesrec(repbuf, msgbuf,"");
			mail_buf(msgbuf, user, repbuf);
			break;
	}
	return 0;
}

static int mc_autoundeny() {
	char *ptr, buf[STRLEN];
	int undenytime;
	if (!seek_in_file(MC_DENY_FILE, currentuser.userid))
		return 0;
	readstrvalue(MC_DENY_FILE, currentuser.userid, buf, STRLEN);
	ptr=strchr(buf, 0x1b);
	if (ptr)
		ytht_strsncpy(buf, ptr+2, sizeof(buf));
	else return 0;
	undenytime=atoi(buf);
	if (undenytime > time(0))
		return 0;
	if (ytht_del_from_file(MC_DENY_FILE, currentuser.userid, true)) {
		sprintf(buf, "恢复 %s 进入大富翁游戏的权利", currentuser.userid);
		//deliverreport(buf, "请理解大富翁总管工作，谢谢！\n");
		millionairesrec(buf, "系统自动解封\n","");
		mail_buf("请理解大富翁总管工作，谢谢！\n", currentuser.userid, buf);
	}
	return 1;
}

static int addstockboard(char *sbname, char *fname) {
	int i;
	int seek;

	if ((i = getbnum(sbname)) == 0){
		move(3, 0);
		prints_nofmt("错误，不存在的版面");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	seek = seek_in_file(fname, sbname);
	if (seek) {
		move(3, 0);
		prints_nofmt("输入的版面已经存在!");
		pressreturn();
		return 0;
	}
	move(3, 0);
	if (askyn("真的要添加吗？", NA, YEA) == NA) {
		pressanykey();
		return 0;
	}
	return ytht_add_to_file(fname, sbname);

}

static int delstockboard(char *sbname, char *fname) {
	int i, seek;
	if ((i = getbnum(sbname)) == 0){
		move(3, 0);
		prints_nofmt("错误，不存在的版面");
		clrtoeol();
		pressreturn();
		clear();
		return 0;
	}
	seek = seek_in_file(fname, sbname);
	if (!seek) {
		move(3, 0);
		prints_nofmt("输入的版面不在列表中!");
		pressreturn();
		return 0;
	}
	move(3, 0);
	if (askyn("真的要删除吗？", NA, NA)==NA){
		pressanykey();
		return 0;
	}
	return ytht_del_from_file(fname, sbname, true);
}

static int stockboards() {
	char uident[STRLEN];
	char ans[8], repbuf[200], buf[200], titlebuf[STRLEN], bname[24 /* see boardheader */],  bpath[STRLEN], reasonbuf[50];
	int count, ch2;
	struct stat st;
	FILE *f_fp;

	nomoney_show_stat("证监会主席办公室");
	whoTakeCharge2(6, buf);
	whoTakeCharge(6, uident);
	if (strcmp(currentuser.userid, uident)) {
		move(6, 4);
		prints("秘书%s拦住了你,说道:“主席%s现在很忙,没时间接待你。”", buf,uident);
		pressanykey();
		return 0;
	} else {
		move(6, 4);
		prints_nofmt("请选择操作代号:");
		move(7, 6);
		prints_nofmt("1. 设定上市版面名单          2. 暂停/恢复某股交易");
		move(8, 6);
		prints_nofmt("3. 暂停全部交易              4. 恢复全部交易");
		move(9, 6);
		prints_nofmt("5. 辞职                      6. 退出");
		ch2 = igetkey();
		switch (ch2) {
			case '1':
				ansimore(MC_STOCK_BOARDS, YEA);
				while (1) {
					clear();
					prints_nofmt("设定上市版面名单\n");
					count = listfilecontent(MC_STOCK_BOARDS);
					if (count)
						getdata(1, 0, "(A)增加 (D)删除 (E)离开 [E]: ", ans, 7, DOECHO, YEA);
					else
						getdata(1, 0, "(A)增加  (E)离开 [E]: ", ans, 7, DOECHO, YEA);
					if (*ans == 'A' || *ans == 'a') {
						move(1, 0);
						make_blist();
						namecomplete("增加版面: ", bname);
						setbpath(bpath, sizeof(bpath), bname);
						if ((*bname == '\0') || (stat(bpath, &st) == -1)) {
							move(2, 0);
							prints_nofmt("不正确的讨论区.\n");
							pressreturn();
							break;
						}
						if (!(st.st_mode & S_IFDIR)) {
							move(2, 0);
							prints_nofmt("不正确的讨论区.\n");
							pressreturn();
							break;
						}
						if (bname[0] != '\0' && bname[0] != '\n' && bname[0] != '\r') {
							if (addstockboard(bname, MC_STOCK_BOARDS)) {
								sprintf(repbuf, "[公告]%s版上市", bname);
								sprintf(buf,
										"经过版主申请，大富翁证监会通过，"
										"批准%s版面上市，试运营期一个月，"
										"望广大股民注意。"
										"如有想上市并符合条件的版面，"
										"欢迎按照相关流程申请上市。\n",
										bname);
								deliverreport(repbuf, buf);
								sprintf(titlebuf, "%s行使股市管理权限", currentuser.userid);
								sprintf(repbuf, "添加上市版面: %s版", bname);
								millionairesrec(titlebuf, repbuf, "");
							}
						}
					} else if ((*ans == 'D' || *ans == 'd') && count) {
						move(1, 0);
						namecomplete("删除版面: ", bname);
						move(1, 0);
						clrtoeol();
						if (bname[0] != '\0' && bname[0] != '\n' && bname[0] != '\r') {
							if (delstockboard(bname, MC_STOCK_BOARDS)) {
								getdata(6, 0, "取消原因：", reasonbuf, 50, DOECHO, YEA);
								snprintf(repbuf, sizeof repbuf, "原因：%s", reasonbuf);
								snprintf(titlebuf, sizeof titlebuf, "[公告]%s版退市", bname);
								deliverreport(titlebuf, repbuf);
								snprintf(titlebuf, sizeof titlebuf, "%s行使股市管理权限", currentuser.userid);
								snprintf(repbuf, sizeof repbuf, "取消上市版面: %s版\n\n取消原因：%s\n", bname, reasonbuf);
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
					prints_nofmt("被暂停交易的版面名单\n");
					count = listfilecontent(MC_STOCK_STOPBUY);
					if (count)
						getdata(1, 0, "(A)增加 (D)删除 (E)离开 [E]: ", ans, 7, DOECHO, YEA);
					else
						getdata(1, 0, "(A)增加  (E)离开 [E]: ", ans, 7, DOECHO, YEA);
					if (*ans == 'A' || *ans == 'a') {
						move(1, 0);
						make_blist();
						namecomplete("暂停哪版交易: ", bname);
						setbpath(bpath, sizeof(bpath), bname);
						if ((*bname == '\0') || (stat(bpath, &st) == -1)) {
							move(2, 0);
							prints_nofmt("不正确的讨论区.\n");
							pressreturn();
							break;
						}
						if (!(st.st_mode & S_IFDIR)) {
							move(2, 0);
							prints_nofmt("不正确的讨论区.\n");
							pressreturn();
							break;
						}
						if (!seek_in_file(MC_STOCK_BOARDS, bname)){
							move(2, 0);
							prints_nofmt("您选择的版面没有上市\n");
							pressreturn();
							break;
						}

						if (bname[0] != '\0' && bname[0] != '\n' && bname[0] != '\r') {
							if (addstockboard(bname, MC_STOCK_STOPBUY)) {
								getdata(6, 0, "暂停原因：", reasonbuf, 50, DOECHO, YEA);
								move(7, 0);
								if (askyn("确定吗？", NA, NA) == NA) {
									pressanykey();
									break;
								}
								snprintf(repbuf, sizeof repbuf, "暂停原因：%s", reasonbuf);
								snprintf(titlebuf, sizeof titlebuf, "[公告]%s版股票停牌", bname);
								deliverreport(titlebuf, repbuf);
								snprintf(titlebuf, sizeof titlebuf, "%s行使股市管理权限", currentuser.userid);
								snprintf(repbuf, sizeof repbuf, "暂停%s版股票交易\n\n原因：%s\n", bname, reasonbuf);
								millionairesrec(titlebuf, repbuf, "");
							}
						}
					} else if ((*ans == 'D' || *ans == 'd') && count) {
						move(1, 0);
						namecomplete("要恢复交易的版面: ", bname);
						move(1, 0);
						clrtoeol();
						if (bname[0] != '\0' && bname[0] != '\n' && bname[0] != '\r') {
							if (delstockboard(bname, MC_STOCK_STOPBUY)) {
								getdata(6, 0, "恢复原因：", reasonbuf, 50, DOECHO, YEA);
								snprintf(repbuf, sizeof repbuf, "恢复原因：%s", reasonbuf);
								snprintf(titlebuf, sizeof titlebuf, "[公告]%s版股票复牌", bname);
								deliverreport(titlebuf, repbuf);
								snprintf(titlebuf, sizeof titlebuf, "%s行使股市管理权限", currentuser.userid);
								snprintf(repbuf, sizeof repbuf, "恢复%s版股票交易\n\n原因：%s\n", bname, reasonbuf);
								millionairesrec(titlebuf, repbuf, "");
							}
						}
					} else
						break;
				}
				break;

			case '3':
				// ythtbbs_cache_utmp_set_ave_score(0);
				sprintf(buf,"%s/stopbuy",DIR_MC);
				if (file_exist(buf)){
					clear();
					move(6, 4);
					prints_nofmt("已经停盘");
					pressreturn();
					break;
				}

				f_fp=fopen(buf,"w");
				if(f_fp!=NULL){
					fclose(f_fp);
					//sprintf(repbuf, "原因：%s", buf);
					//sprintf(titlebuf, "[公告]兵马俑股市停盘");
					deliverreport("[公告]兵马俑股市停盘", "");
					sprintf(titlebuf, "%s行使股市管理权限", currentuser.userid);
					//sprintf(repbuf, "暂停全部交易\n\n原因：%s\n", bname, buf);
					millionairesrec(titlebuf, "暂停全部交易", "");

					clear();
					move(6, 4);
					prints_nofmt("操作成功!");
					pressanykey();
				}else{
					clear();
					move(6, 4);
					prints_nofmt("发生错误");
					pressreturn();
				}
				break;

			case '4':
				sprintf(buf,"%s/stopbuy",DIR_MC);
				if (!file_exist(buf)){
					clear();
					move(6, 4);
					prints_nofmt("没有停盘啊");
					pressreturn();
					break;
				}
				remove(buf);
				deliverreport("[公告]兵马俑股市重新开盘", "");
				sprintf(titlebuf, "%s行使股市管理权限", currentuser.userid);
				millionairesrec(titlebuf, "恢复全部交易", "");

				clear();
				move(6, 4);
				prints_nofmt("操作成功!");
				pressanykey();
				break;

			case '5':
				move(12, 4);
				if (askyn("您真的要辞职吗？", NA,NA) == YEA) {
					sprintf(genbuf, "%s 要辞去证监会主席职务", currentuser.userid);
					mail_buf(genbuf, "millionaires", genbuf);
					move(14, 4);
					prints_nofmt("好吧，已经发信告知总管了");
					pressanykey();
				}
				break;
		}
	}
	clear();
	return FULLUPDATE;
}

/* 火车票票价计算程序 by macintosh 2006.12.28 */
/* 2007.10.26修改*/

struct ticket_info {
	char CheCi[6];
	char ShiFa[11];
	char ZhongDao[11];
	//以上是票面信息
	int LiCheng;
	int LiCheng2;
	//通票前一段的里程
	char PiaoZhong;
	//学残孩团优
	char XiBie;
	//软硬
	char JiaKuai;
	//普快特快
	char KongTiao;
	//空调
	char WoPu;
	//没有，上中下
	char DongChe;
	//动车组一等座，二等座
	char DaoDi;
	//通票到底类型
	float ShangFu;
	//上浮
	float ZaiFu;
	//再浮
} myTicket;

struct TrainInfo{
	char CheCi[6];
	char KongTiao;
	float ShangFu;
	float ZaiFu;
};

//默认快速特快有空调、普快无空调，以下仅列出例外车次
//以下仅列出西安站到发车次，含复车次
struct TrainInfo XianTrain[]= {
	{"T193", 2, 0.3, 0.0},{"T194", 2, 0.3, 0.0},{"T191", 2, 0.3, 0.0},{"T192", 2, 0.3, 0.0},
	{"T197", 2, 0.3, 0.0},{"T198", 2, 0.3, 0.0}, {"K5", 2, 0.4, 0.0},{"K6", 2, 0.4, 0.0},
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
	{"N375", 2, 0.5, -0.15},{"N376", 2, 0.5, -0.15},{"N376", 2, 0.5, -0.15},{"N378", 2, 0.5, -0.15},
	{"N359", 2, 0.4, 0.0},{"N360", 2, 0.4, 0.0},{"N357", 2, 0.4, 0.0},{"N358", 2, 0.4, 0.0},
	{"4901", 2, 0.4, 0.0},{"4902", 2, 0.4, 0.0},{"4903", 2, 0.4, 0.0},{"4904", 2, 0.4, 0.0},
	{"4909", 2, 0.4, 0.0},{"4910", 2, 0.4, 0.0},{"4908", 2, 0.4, 0.0},{"4907", 2, 0.4, 0.0},
	{"4911", 2, 0.3, 0.0},{"4912", 2, 0.3, 0.0},
	{"4915", 2, 0.5, -0.3},{"4916", 2, 0.5, -0.3},{"4917", 2, 0.5, -0.3},{"4918", 2, 0.5, -0.3},
	{"A351", 1, 0.0, 0.0},{"A352", 1, 0.0, 0.0},
};

//四舍五入
#if 0
static float Round(float num) {
	num = (float)(int) (num + 0.5);
	return num;
}
#endif

#if 0
//计算硬席基本票
static float calc_basic_price(int LiCheng, int flag) {
	int mininum, distance = 0, order = 0, i, j;
	float rate=0, basic_price=0;
	const float BASIC = 0.05861;

	if (LiCheng <= 0)
		return 0;

	//起码里程
	switch (flag){
		case 2:
			mininum = 100;//加快票
			break;
		case 3:
			mininum = 400;//卧铺票
			break;
		default:
			mininum = 0;//20
			break;
	}
	if (LiCheng <= mininum)
		distance = mininum;
	else
		//计算参加运算的里程
		for (i = 4600, j = 100; j > 0; j -= 10){
			if (LiCheng > i){
				order = (LiCheng - i) / j;
				if ((LiCheng - i) % j == 0)
					order--;
				distance = i + order * j + j/2;
				break;
			}else
				i = i-(j/10-1)*100;
			//处理里程<=200的情况
			if (j == 20)
				i = 0;
		}

	//票价递远递减区段
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
#endif

#if 0
static float show_ticket() {
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

		sprintf(printbuf, "\033[1m%s%s\033[0m动车组列车票价信息（里程%d公里）",
				myTicket.CheCi,  myTicket.CheCi[0]?"次":"",
				myTicket.LiCheng);
		showAt(5, 6, printbuf, 0);
		showAt(9, 6, "注意: 动车组票价仅供参考，具体票价参见车站公告。", 0);

		sprintf(printbuf, "￥ %.2f 元\t\t\t  动车组%s等座",
				QuanJia,
				(myTicket.DongChe == 2)?"一":"二");
		showAt(16, 6, printbuf, 0);

		if (myTicket.DongChe == 3)
			showAt(16, 24, "(学)", 0);
		showAt(18, 6, "和 谐 号", 0);

		return QuanJia;
	}

	JiBenPiao = calc_basic_price(myTicket.LiCheng, 0);

	//客票计算
	//通票
	if (myTicket.DaoDi > 0){
		JiBenPiao = calc_basic_price(myTicket.LiCheng2, 0);
		BaoXian = 0.02 * JiBenPiao;
		BaoXian = ceil(BaoXian * 10); //直接进位到0.1元
		BaoXian = BaoXian/10;
		//软席
		if (myTicket.XiBie == 1)
			KePiao2 = Round(JiBenPiao * 2 + BaoXian);
		else
			KePiao2 = Round(JiBenPiao + BaoXian);
		if (KePiao2 < 1)
			KePiao2 = 1;
		KePiao2 = Round(KePiao2 * (1 + myTicket.ShangFu)) -Round(JiBenPiao + BaoXian);
		//全程
		JiBenPiao = calc_basic_price(myTicket.LiCheng, 0);
		BaoXian = 0.02 * JiBenPiao;
		BaoXian = ceil(BaoXian * 10); //直接进位到0.1元
		BaoXian = BaoXian/10;
		KePiao = Round(JiBenPiao + BaoXian);
		KePiao += KePiao2;
	}
	else{
		KePiao = JiBenPiao;
		//保险费计算
		BaoXian = 0.02 * JiBenPiao;
		BaoXian = ceil(BaoXian * 10); //直接进位到0.1元
		BaoXian = BaoXian/10;
		if (myTicket.XiBie == 1)
			KePiao *= 2;
		KePiao += BaoXian;
		KePiao = Round(KePiao);
		if (KePiao < 1)
			KePiao = 1;
		KePiao *= (1 + myTicket.ShangFu);//上浮
		KePiao = Round(KePiao);
	}

	//加快票计算
	KuaiPiao = 0;
	//先算前半部分(非通票算全程)
	JiBenPiao = calc_basic_price(myTicket.LiCheng2, 2);
	if (myTicket.JiaKuai > 0)
		KuaiPiao = 0.2 * JiBenPiao;
	KuaiPiao = Round(KuaiPiao);
	KuaiPiao *= (1 + myTicket.ShangFu);//上浮
	KuaiPiao = Round(KuaiPiao);
	if (myTicket.JiaKuai > 1)
		KuaiPiao *= 2;

	//通票计算前程相应到底等级的加快票jk1
	if (myTicket.DaoDi > 0){
		if (myTicket.DaoDi == 2){
			if (myTicket.JiaKuai < 1)//亏钱，普客列车普快到底
				jk1 = 0;
			else{
				jk1 = 0.2 * JiBenPiao;
				jk1 = Round(jk1);
			}
		}
		if (myTicket.DaoDi == 3){
			if (myTicket.JiaKuai < 1)//亏钱，普客列车特快到底
				jk1 = 0;
			else if (myTicket.JiaKuai < 2){//亏钱，普快列车特快到底
				jk1 = 0.2 * JiBenPiao;
				jk1 = Round(jk1);
			}else{
				jk1 = 0.2 * JiBenPiao;
				jk1 = Round(jk1);
				jk1 *= 2;
			}
		}
	}

	//通票全程到底加快票jk2
	if (myTicket.DaoDi > 1){
		JiBenPiao = calc_basic_price(myTicket.LiCheng, 2);
		jk2 = 0.2 * JiBenPiao;
		jk2 = Round(jk2);
	}
	if (myTicket.DaoDi > 2)
		jk2 *= 2;

	KuaiPiao = KuaiPiao - jk1 + jk2;


	//空调票计算
	KongPiao = 0;
	if (myTicket.KongTiao > 0){
		JiBenPiao = calc_basic_price(myTicket.LiCheng2, 4);
		KongPiao = 0.25 * JiBenPiao;
		KongPiao = Round(KongPiao);
		if (KongPiao < 1)
			KongPiao = 1; //空调票不足1元按1元收
		KongPiao *= (1 + myTicket.ShangFu);//上浮
		KongPiao = Round(KongPiao);
	}

	//卧铺票计算
	WoPiao = 0;
	JiBenPiao = calc_basic_price(myTicket.LiCheng2, 3);
	if (myTicket.XiBie == 0){//硬卧
		if (myTicket.WoPu == 1)
			WoPiao = 1.1 * JiBenPiao;
		else if (myTicket.WoPu == 2)
			WoPiao = 1.2 * JiBenPiao;
		else if (myTicket.WoPu == 3)
			WoPiao = 1.3 * JiBenPiao;
	} else {//软卧
		if (myTicket.WoPu == 1)
			WoPiao = 1.75 * JiBenPiao;
		else if (myTicket.WoPu > 1)
			WoPiao = 1.95 * JiBenPiao;
	}
	WoPiao = Round(WoPiao);
	WoPiao *= (1 + myTicket.ShangFu);//上浮
	WoPiao = Round(WoPiao);

	//再上浮
	KePiao = Round((1 + myTicket.ZaiFu) * KePiao);
	KuaiPiao = Round((1 + myTicket.ZaiFu) * KuaiPiao);
	KongPiao = Round((1 + myTicket.ZaiFu) * KongPiao);
	WoPiao = Round((1 + myTicket.ZaiFu) * WoPiao);

	//学生票
	if (myTicket.PiaoZhong == 1 && myTicket.XiBie == 0){
		KePiao *= 0.5;
		KuaiPiao *= 0.5;
		KongPiao *= 0.5;
	}

	//小孩票
	if (myTicket.PiaoZhong == 2){
		KePiao *= 0.5;
		KuaiPiao *= 0.5;
		KongPiao *= 0.5;
	}

	//残票
	if (myTicket.PiaoZhong == 3){
		KePiao *= 0.5;
		KuaiPiao *= 0.5;
		KongPiao *= 0.5;
		WoPiao *= 0.5;
	}

	//小孩单独使用卧铺
	if (myTicket.PiaoZhong == 4){
		KePiao = 0;
		KuaiPiao = 0;
		KongPiao *= 0.5;
	}

	//卧铺订票费
	if (WoPiao > 0)
		WoPiao += 10;

	//客票信息化发展基金
	if (KePiao + KuaiPiao + KongPiao + WoPiao > 5)
		JiJin = 1;
	else
		JiJin = 0.5;

	//车站空调费
	if (myTicket.LiCheng >= 200)
		CheZhan = 1;
	else
		CheZhan = 0;
	//软席不收空调费
	if (myTicket.XiBie)
		CheZhan = 0;

	if (myTicket.LiCheng <= 0)
		KePiao = KuaiPiao = KongPiao = WoPiao = 0;

	QuanJia = KePiao + KuaiPiao + KongPiao + WoPiao + JiJin + CheZhan;

	sprintf(printbuf, "客票票价：\t%.2f 元", KePiao);
	showAt(7, 6, printbuf, 0);
	sprintf(printbuf, "卧铺票票价：\t%.2f 元", WoPiao);
	showAt(7, 44, printbuf, 0);
	sprintf(printbuf, "\033[1;30m意外伤害保险：\t%.2f 元", BaoXian);
	showAt(8, 6, printbuf, 0);
	sprintf(printbuf, "卧铺订票费：\t%.2f 元\033[m", (WoPiao > 0) ? 10.0 : 0.0);
	showAt(8, 44, printbuf, 0);
	sprintf(printbuf, "加快票票价：\t%.2f 元", KuaiPiao);
	showAt(9, 6, printbuf, 0);
	sprintf(printbuf, "空调票票价：\t%.2f 元", KongPiao);
	showAt(9, 44, printbuf, 0);
	sprintf(printbuf, "客票信息化基金：\t%.2f 元", JiJin);
	showAt(10, 6, printbuf, 0);
	sprintf(printbuf, "车站空调费：\t%.2f 元\033[m", CheZhan);
	showAt(10, 44, printbuf, 0);

	switch (myTicket.CheCi[0]){
		case 'Z':
			strcpy(ZTKN, "直达");
			break;
		case 'K':
		case 'N':
			strcpy(ZTKN, "快速");
			break;
		case 'T':
		default:
			strcpy(ZTKN, "特快");
			break;
	}
	sprintf(printbuf, "￥ %.2f 元\t\t\t  %s%s%s%s%s",
			QuanJia,
			(myTicket.KongTiao == 2)?"新":"",
			(myTicket.KongTiao > 0)?"空调":"",
			(myTicket.XiBie)?"软座":"硬座",
			(myTicket.JiaKuai>0)?((myTicket.JiaKuai>1)?ZTKN:"普快"):"普客",
			(myTicket.WoPu>0)?"卧":"");
	showAt(16, 6, printbuf, 0);

	if (myTicket.DaoDi > 0){
		sprintf(printbuf, "(%s至到站)",
				(myTicket.DaoDi > 1)?((myTicket.DaoDi > 2)?"特快":"普快"):"普客");
		showAt(16, 60, printbuf, 0);
		showAt(17, 42, "至换乘站", 0);
	}

	if (myTicket.DaoDi > 0)
		sprintf(printbuf2, "，中转前%d公里", myTicket.LiCheng2);
	else
		printbuf2[0] = 0;
	sprintf(printbuf, "\033[1m%s%s\033[0m%s列车票价信息（里程%d公里%s）",
			myTicket.CheCi,  myTicket.CheCi[0]?"次":"",
			(myTicket.JiaKuai>0)?((myTicket.JiaKuai>1)?ZTKN:"普快"):"普通",
			myTicket.LiCheng,
			printbuf2);
	showAt(5, 6, printbuf, 0);

	YouXiaoQi = 2;
	i = 500;
	while (myTicket.LiCheng > i){
		YouXiaoQi ++;
		i += 1000;
	}
	sprintf(printbuf, "在 %d 日内到有效", YouXiaoQi);
	showAt(18, 6, printbuf, 0);

	switch (myTicket.PiaoZhong){
		case 1:
			showAt(16, 24, "(学)", 0);
			break;
		case 2:
		case 4:
			showAt(16, 24, "(孩)", 0);
			break;
		case 3:
			showAt(16, 24, "(残)", 0);
			break;
		default:
			break;
	}

	if (myTicket.ShangFu > 0 && myTicket.ShangFu < 0.5 && myTicket.KongTiao == 2)
		showAt(17, 24, "(折)", 0);

	return QuanJia;
}
#endif

#if 0
/* 捐款 by macintosh  */
static int loadContributions(char *cname, char *user) {
	char value[20];
	char path[256];
	sprintf(path, DIR_CONTRIBUTIONS"%s", cname);
	if (readstrvalue(path, user, value, 20) != 0)
		return 0;
	else
		return limitValue(atoi(value), sizeof(int));
}  //读取各基金捐款数值
#endif

static int saveContributions(char *cname, char *user, int valueToAdd) {
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
}  //保存捐款数值


static void doContributions(struct MC_Jijin *clist) {
	int money, i=0, num=0, num2, total_num, old_num ;
	float transfer_rate;
	char title[80], buf[512];

	clear();
	sprintf(buf, "No. %-12.12s  %16.16s  %s", "基金ID", "基金名称", "累计捐款");
	showAt(5, 2, buf, 0);
	while (clist[i].userid[0]!= 0){
		sprintf(buf, "ctr_%s", clist[i].userid);
		old_num = loadValue(currentuser.userid, buf, MAX_CTRBT_NUM);
		sprintf(buf, "%2d  %-12.12s  %17.17s  %d", i+1, clist[i].userid, clist[i].name, old_num);
		showAt(7+i, 2, buf, 0);
		i++;
	}
	sprintf(title, "请选择捐款对象[1-%d]: ", i);
	getdata(t_lines-6, 2, title, buf, 3, DOECHO, YEA);
	if (buf[0] == '\0' || buf[0] == '\n')
		return;
	num = atoi(buf);
	if (num > i || num < 1){
		showAt(t_lines-4, 2, "考虑好了再捐吧...", 1);
		return;
	}
	num --;

	money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
	getdata(t_lines-5, 2, "请输入现金金额[下限1000]: ", buf, 10, DOECHO, YEA);
	num2 = atoi(buf);
	if (num2 < 1000){
		showAt(t_lines-4, 2, "1000都没有啊...", 1);
		return;
	}
	if (num2 > money || num2 <= 0){
		showAt(t_lines-4, 2, "对不起, 您现金金额不足", 1);
		return;
	}
	transfer_rate = mc->transfer_rate / 10000.0;
	sprintf(buf, " 手续费 %.2f％（最高收取 100000 兵马俑币，不足1按1收取。）", transfer_rate * 100);
	showAt(t_lines-4, 2, buf, 0);
	move(t_lines-3, 2);
	sprintf(buf, "确定给 %s 基金（%s）捐%d 兵马俑币吗？", clist[num].name, clist[num].userid, num2);

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
			prints("您的现金不够，加手续费共需 %d 兵马俑币", total_num);
			pressanykey();
			return;
		}
		saveValue(currentuser.userid, MONEY_NAME, -total_num, MAX_MONEY_NUM);
		saveValue(clist[num].userid, MONEY_NAME, num2, MAX_MONEY_NUM);
		sprintf(title, "[通知] %s 给%s基金捐款", currentuser.userid, clist[num].name);
		sprintf(buf,
				"%s 通过兵马俑捐款办公室向您捐赠 %d 兵马俑币，请查收。",
				currentuser.userid, num2);
		mail_buf(buf, clist[num].userid, title);

		sprintf(buf, "ctr_%s", clist[num].userid);
		saveValue(currentuser.userid, buf, num2, MAX_CTRBT_NUM);
		saveContributions(clist[num].userid, currentuser.userid, num2);

		sprintf(title, "[公告] %s基金（%s）收到捐款", clist[num].name, clist[num].userid);
		sprintf(buf,"感谢%s对%s基金的大力支持，兵马俑大富翁代表全体纳税人向其表示感谢！", currentuser.userid, clist[num].name);
		deliverreport(title, buf);

		sprintf(genbuf, "%s进行捐款", currentuser.userid);
		sprintf(buf,"%s捐款给%s基金（%s） %d兵马俑币", currentuser.userid, clist[num].name, clist[num].userid, num2);
		millionairesrec(genbuf, buf, "捐款");
		showAt(t_lines-2, 4, "捐款成功，感谢你对兵马俑大富翁的支持。", 1);
	}
	return;
}

static int money_contributions() {
	int ch, money, money2, quit = 0, count = 0;
	void *buffer = NULL;
	size_t filesize;
	char title[STRLEN], buf[256];

	struct MC_Jijin clist1[]= {
		{"millionaires", "大富翁基金"},
		{"BMYbeg", "丐帮基金"},
		{"BMYRober", "黑帮基金"},
		{"BMYpolice", "警署基金"},
		{"BMYKillersky", "杀手基金"},
		{"", ""}
	};
	struct MC_Jijin *clist2;

	while (!quit) {
		nomoney_show_stat("大富翁捐款办公室");
		showAt(6, 4, "献出一份爱心", 0);
		showAt(t_lines - 1, 0,
				"\033[1;44m 选单 \033[1;46m [1]帮派基金 [2]民间基金 [3]慈善家排行榜 [4]捐献全部财产 [Q]离开             \033[m", 0);
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
				showAt(4, 4, "\033[1;32m赞无\033[m", 1);
				break;

			case '4':
				showAt(5, 0,
						"\033[1;32m您确定钱放在口袋烧手，存在银行烧心，准备看破红尘四大皆空抛开全部\033[m\n"
						"\033[1;32m身家上山当和尚么？\033[m\n"
						"\033[1;32m您的全部财产将留给millionaires作为剃度费，资金将用于建设希望小学\033[m\n"
						"\033[1;32m和援助艾滋病患者，以及资助坦桑尼亚、赞比亚等国贫民\033[m\n"
						"\033[1;31m注意：公公只负责剃度不负责还俗！！\033[m\n"
						"\033[1;33m钱不是万能的，没有钱却万万不能，三思而后行啊！\033[m\n"
						, 0);
				move(12, 0);
				if (askyn("确定捐献全部财产吗? ", NA, NA) == YEA) {
					money = loadValue(currentuser.userid, MONEY_NAME, MAX_MONEY_NUM);
					money2 = loadValue(currentuser.userid, CREDIT_NAME, MAX_MONEY_NUM);

					if (money + money2 == 0){
						showAt(15, 0, "没钱就不用来凑热闹了~", 1);
						break;
					}

					saveValue(currentuser.userid, MONEY_NAME, -money, MAX_MONEY_NUM);
					saveValue("millionaires", MONEY_NAME, money, MAX_MONEY_NUM);
					saveValue(currentuser.userid, "ctr_millionaires", money, MAX_CTRBT_NUM);
					saveValue(currentuser.userid, CREDIT_NAME, -money2, MAX_MONEY_NUM);
					saveValue("millionaires", CREDIT_NAME, money2, MAX_MONEY_NUM);
					saveValue(currentuser.userid, "ctr_millionaires", money2, MAX_CTRBT_NUM);

					sprintf(title, "%s捐献全部财产", currentuser.userid);
					sprintf(buf, "%s捐献全部财产:\n现金%d兵马俑币\n存款%d兵马俑币", currentuser.userid, money, money2);
					millionairesrec(title, buf, "捐款");

					sprintf(title, "[公告] 大富翁基金收到来自%s的捐款", currentuser.userid);
					sprintf(buf,"感谢%s向兵马俑大富翁捐献其全部财产，大富翁向其表示最崇高的敬意！\n" "并祝愿其今后修行顺利！", currentuser.userid);
					deliverreport(title, buf);

					showAt(15, 0, "完成!", 1);
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


static int money_office() {
	int ch, quit = 0;
	char uident[IDLEN + 1];

	while (!quit) {
		nomoney_show_stat("兵马俑大富翁管理中心");
		showAt(6, 4, "大富翁管理中心欢迎你！", 0);
		showAt(t_lines - 1, 0, "\033[1;44m 选单 \033[1;46m [1]捐款办公室 [2]信访办公室 [3]监狱 [4]邮政局 [5]总管办公室 [Q]离开       \033[m", 0);
		ch = igetkey();
		switch (ch) {
			case '1':
				money_contributions();
				break;

			case '2':
				if (!HAS_PERM(PERM_POST, currentuser))
					break;
				move(6, 4);
				if (askyn("确定要发信吗? ", NA, NA) == YEA)
					m_send("millionaires");
				break;

			case '3':
				showAt(6, 4, "看你探头探脑猥猥琐琐的样子，打算劫狱？带个棒棒糖就当是AK-47？\n" "皮痒了吧？小心电警棍！", 1);
				break;

			case '4':
				money_postoffice();
				break;

			case '5':
				nomoney_show_stat("大富翁总管办公室");
				whoTakeCharge2(11, uident);
				if (strcmp(currentuser.userid, uident)) {
					move(6, 4);
					prints("值班秘书%s叫住了你，说道:“公公们正在开会，请先四处转转吧。”", uident);
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

static void mc_shm_init() {
	char local_buf[512];
	if (mc == NULL) {
		mc = get_shm(MC_SHMKEY, sizeof(*mc));
	}

	if (mc != NULL) {
		// 转账手续费
		if (mc->transfer_rate == 0) {
			readstrvalue(MC_RATE_FILE, "transfer_rate", local_buf, sizeof(local_buf));
			mc->transfer_rate = atoi(local_buf);
		}
		// 存款利率
		if (mc->deposit_rate == 0) {
			readstrvalue(MC_RATE_FILE, "deposit_rate", local_buf, sizeof(local_buf));
			mc->deposit_rate = atoi(local_buf);
		}
		// 贷款利率
		if (mc->lend_rate == 0) {
			readstrvalue(MC_RATE_FILE, "lend_rate", local_buf, sizeof(local_buf));
			mc->lend_rate = atoi(local_buf);
		}
	}
}

