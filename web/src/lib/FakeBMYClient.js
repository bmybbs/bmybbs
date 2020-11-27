/**
 * 便于开发调试创建的类，用于模拟接口返回数据
 */
export const BMYClient = {
	get_announce() {
		return new Promise((resolve) => {
			resolve({
				errcode: 0,
				articlelist: [
					{ title: "本站开设“西部创新港”版", author: "SYSOP", board: "sysop", aid: 1600156339, comments: 7 },
					{ title: "学校各职能部门在创新港办公地点", author: "ylqxzyyn", board: "InnovationHarbour", aid: 1600156407, comments: 2 },
					{ title: "关于学生事务大厅延长开放时间的通知", author: "ylqxzyyn", board: "XJTUnews", aid: 1600735641, comments: 3 },
					{ title: "[程序组公告] 欢迎试用新的身份验证方式确认账号", author: "IronBlood", board: "XJTUnews", aid: 1599907642, comments: 1 },
				]
			});
		});
	},
	get_commend() {
		return new Promise((resolve) => {
			resolve({
				errcode: 0,
				articlelist: [
					{ title: "秋日", author: "eledy", board: "bloom", aid: 1600073310, comments: 3 },
					{ title: "绿萝", author: "eledy", board: "bloom", aid: 1598517189, comments: 5 },
					{ title: "关于火车", author: "casablanca", board: "bloom", aid: 1589374392, comments: 1 },
					{ title: "题目大概先叫毕业10年总结吧", author: "lalaarere", board: "XJTUnews", aid: 1602825853, comments: 74 },
					{ title: "变", author: "casablanca", board: "bloom", aid: 1589547244, comments: 3 },
				]
			});
		});
	},
	get_boards_by_section(sec_id) {
		return new Promise((resolve) => {
			let boards;
			switch(sec_id) {
			case "0":
				boards = [
					{ name: "ArtDesign", title: "系统美工", articles: 3455 },
					{ name: "sysop", title: "站长工作室", articles: 10741 },
					{ name: "BMY_Dev", title: "程序组工作站", articles: 1476 },
					{ name: "committe", title: "纪律委员会", articles: 3088 },
				];
				break;
			case "1":
				boards = [
					{ name: "XJTUnews", title: "说说咱交大", articles: 12984 },
					{ name: "doctor", title: "交大博士", articles: 3882 },
					{ name: "XJTUei", title: "电子与信息学部", articles: 8720 },
					{ name: "XJTUeec", title: "电气工程学院", articles: 10926 },
				];
				break;
			case "2":
				boards = [
					{ name: "C_Cpp", title: "C/C++程序设计", articles: 2220 },
					{ name: "Java", title: "Java技术", articles: 2081 },
					{ name: "database", title: "数据库技术", articles: 587 },
					{ name: "WebDev", title: "Web开发", articles: 514 },
				];
				break;
			case "3":
				boards = [
					{ name: "Security", title: "安全技术", articles: 1615 },
					{ name: "Hardware", title: "硬件乐园", articles: 13175 },
					{ name: "Apple", title: "苹果园", articles: 3924 },
					{ name: "Software", title: "软件应用", articles: 1250 },
				];
				break;
			case "4":
				boards = [
					{ name: "thesis", title: "论文", articles: 1407 },
					{ name: "Nature", title: "奇趣大自然", articles: 4836 },
					{ name: "Robot", title: "机器人", articles: 4317 },
					{ name: "FEA", title: "有限元分析", articles: 832 },
				];
				break;
			case "5":
				boards = [
					{ name: "newlife", title: "工作一族", articles: 56505 },
					{ name: "RealEstate", title: "房里房外", articles: 9345 },
					{ name: "history", title: "历史", articles: 3078 },
					{ name: "overseas", title: "海外交大人", articles: 664 },
				];
				break;
			case "6":
				boards = [
					{ name: "WormLife", title: "网友与网事", articles: 3612 },
					{ name: "Photography", title: "摄影版", articles: 8573 },
					{ name: "bloom", title: "千千阙歌", articles: 7094 },
					{ name: "Art", title: "艺术版", articles: 13050 },
				];
				break;
			case "7":
				boards = [
					{ name: "PieBridge", title: "执子之手", articles: 25083 },
					{ name: "Joke", title: "笑话", articles: 36442 },
					{ name: "water", title: "水库", articles: 17560 },
					{ name: "Single", title: "单身男女", articles: 45964 },
				];
				break;
			case "8":
				boards = [
					{ name: "football", title: "足球天下", articles: 18282 },
					{ name: "Badminton", title: "羽毛球", articles: 7199 },
					{ name: "Cycling", title: "骑行天下", articles: 32371 },
					{ name: "Fitness", title: "健身", articles: 2200 },
				];
				break;
			case "9":
				boards = [
					{ name: "PerPhoto", title: "个人写真", articles: 5867 },
					{ name: "Picture", title: "贴图版", articles: 30276 },
					{ name: "Tea", title: "茶文化", articles: 3129 },
					{ name: "Movie", title: "电影", articles: 13395 },
				];
				break;
			case "G":
				boards = [
					{ name: "Cstrike", title: "反恐精英", articles: 2493 },
					{ name: "Game", title: "计算机游戏", articles: 7041 },
					{ name: "Diablo", title: "暗黑破坏神", articles: 3174 },
					{ name: "D2Trade", title: "暗黑交易", articles: 222 },
				];
				break;
			case "N":
				boards = [
					{ name: "SecondHand", title: "二手货市场", articles: 75211 },
					{ name: "TempHouse", title: "租房", articles: 9266 },
					{ name: "job", title: "找工作", articles: 21116 },
					{ name: "selfhelping", title: "勤工助学", articles: 12203 },
				];
				break;
			case "H":
				boards = [
					{ name: "Hunan", title: "潇湘神韵", articles: 3195 },
					{ name: "Shanxi", title: "三晋大地", articles: 7345 },
					{ name: "Liaoning", title: "辽沈大地", articles: 6756 },
					{ name: "Tianjin", title: "海河儿女", articles: 490 },
				];
				break;
			case "A":
				boards = [
					{ name: "XJTUjyzx", title: "就业中心", articles: 1756 },
					{ name: "XJTUnic", title: "网络中心", articles: 12344 },
					{ name: "XJTUjjc", title: "基建处", articles: 264 },
					{ name: "XJTUwyc", title: "物业处", articles: 2923 },
				];
				break;
			case "C":
				boards = [
					{ name: "JOBinfo", title: "工作信息", articles: 1618 },
					{ name: "Milan", title: "米兰", articles: 3767 },
					{ name: "TiaoZhan", title: "挑战网", articles: 2789 },
					{ name: "Acfiorentina", title: "罗马", articles: 1094 },
				];
				break;
			default:
				boards = [];
				break;
			} /* end of switch-cases */

			resolve({
				errcode: 0,
				boards
			});
		});
	},
};

