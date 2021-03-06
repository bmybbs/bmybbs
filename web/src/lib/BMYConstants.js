export const ITEMS_PER_PAGE = 20;

export const BMYSECSTRS = [
	{ id: '0', name: "本站系统", icon: "paper-plane" },
	{ id: '1', name: "交通大学", icon: "graduation-cap" },
	{ id: '2', name: "开发技术", icon: "code" },
	{ id: '3', name: "电脑应用", icon: "laptop" },
	{ id: '4', name: "学术科学", icon: "atom" },
	{ id: '5', name: "社会科学", icon: "globe-asia" },
	{ id: '6', name: "文学艺术", icon: "book" },
	{ id: '7', name: "知性感性", icon: "heart" },
	{ id: '8', name: "体育运动", icon: "futbol" },
	{ id: '9', name: "休闲音乐", icon: "music" },
	{ id: 'G', name: "游戏天地", icon: "gamepad" },
	{ id: 'N', name: "新闻信息", icon: "info-circle" },
	{ id: 'H', name: "乡音乡情", icon: "map" },
	{ id: 'A', name: "校务信息", icon: "building" },
	{ id: 'C', name: "俱乐部区", icon: "pizza-slice" }
];

export const BOARD_SORT_MODE = {
	BY_ALPHABET: 1,
	BY_SCORE: 2,
	BY_INBOARD: 3,
};

export const BOARD_ARTICLE_MODE = {
	THREAD_MODE: "thread",
	NORMAL_MODE: "",
};

export const ARTICLE_FLAGS = {
	FH_MARKED: 0x04,
	FH_DIGEST: 0x08,
	FH_NOREPLY: 0x10,
	FH_MATH: 0x04000,
	FH_ISWATER: 0x00100000,
};

export const ANSI_TAGS = {
	ANSI_CLEAR:     "[ESC][0m",
	ANSI_ITALIC:    "[ESC][3m",
	ANSI_UNDERL:    "[ESC][4m",
	ANSI_BLINK:     "[ESC][5m",

	ANSI_FC_BLACK:  "[ESC][1;30m",
	ANSI_FC_RED:    "[ESC][1;31m",
	ANSI_FC_GREEN:  "[ESC][1;32m",
	ANSI_FC_YELLOW: "[ESC][1;33m",
	ANSI_FC_BLUE:   "[ESC][1;34m",
	ANSI_FC_PINK:   "[ESC][1;35m",
	ANSI_FC_CYAN:   "[ESC][1;36m",
	ANSI_FC_WHITE:  "[ESC][1;37m",

	ANSI_BG_BLACK:  "[ESC][1;40m",
	ANSI_BG_RED:    "[ESC][1;41m",
	ANSI_BG_GREEN:  "[ESC][1;42m",
	ANSI_BG_YELLOW: "[ESC][1;43m",
	ANSI_BG_BLUE:   "[ESC][1;44m",
	ANSI_BG_PINK:   "[ESC][1;45m",
	ANSI_BG_CYAN:   "[ESC][1;46m",
	ANSI_BG_WHITE:  "[ESC][1;47m",
};

export const BMY_EC = {
	API_RT_SUCCESSFUL     : 0,
	API_RT_NOEMTSESS      : 1,
	API_RT_CNTLGOUTGST    : 2,
	API_RT_NOTOP10FILE    : 3,
	API_RT_XMLFMTERROR    : 4,
	API_RT_NOSUCHFILE     : 5,
	API_RT_NOCMMNDFILE    : 6,
	API_RT_NOGDBRDFILE    : 7,
	API_RT_CNTMAPBRDIR    : 8,
	API_RT_ATCLINNERR     : 9,
	API_RT_WRONGACTIVE    : 10,
	API_RT_NOBRDTPFILE    : 11,
	API_RT_NOTENGMEM      : 12,
	API_RT_MAILINNERR     : 13,
	API_RT_WRONGPARAM     : 1000,
	API_RT_WRONGSESS      : 1001,
	API_RT_NOTLOGGEDIN    : 1002,
	API_RT_FUNCNOTIMPL    : 1003,
	API_RT_WRONGMETHOD    : 1004,
	API_RT_WXAPIERROR     : 1005,
	API_RT_NOTEMPLATE     : 1100,
	API_RT_NOSUCHUSER     : 100000,
	API_RT_SITEFBDIP      : 100001,
	API_RT_FORBIDDENIP    : 100002,
	API_RT_ERRORPWD       : 100003,
	API_RT_FBDNUSER       : 100004,
	API_RT_INVSESSID      : 100005,
	API_RT_WRONGTOKEN     : 100006,
	API_RT_USEREXSITED    : 100007,
	API_RT_FBDUSERNAME    : 100008,
	API_RT_REACHMAXRCD    : 100009,
	API_RT_ALRDYINRCD     : 100010,
	API_RT_NOTINRCD       : 100011,
	API_RT_HASOPENID      : 100012,
	API_RT_2FA_INTERNAL   : 100013,
	API_RT_2FA_INVALID    : 100014,
	API_RT_NOOPENID       : 100015,
	API_RT_USERLOCKFAIL   : 100016,
	API_RT_NOSUCHBRD      : 110000,
	API_RT_NOBRDRPERM     : 110001,
	API_RT_EMPTYBRD       : 110002,
	API_RT_NOBRDPPERM     : 110003,
	API_RT_FBDGSTPIP      : 110004,
	API_RT_NOSUCHATCL     : 120000,
	API_RT_ATCLDELETED    : 120001,
	API_RT_ATCLNOTITLE    : 120002,
	API_RT_ATCLFBDREPLY   : 120003,
	API_RT_ATCL1984       : 120004,
	API_RT_MAILDIRERR     : 130000,
	API_RT_MAILEMPTY      : 130001,
	API_RT_MAILATTERR     : 130002,
	API_RT_MAILNOPPERM    : 130003,
	API_RT_MAILFULL       : 130004,
	API_RT_INUSERBLIST    : 130005,
	API_RT_ATTINNERR      : 140000,
	API_RT_ATTNOSPACE     : 140001,
	API_RT_ATTTOOBIG      : 140002,
	API_RT_NOMOREFEED     : 150001,
};

export const BMY_FILE_HEADER = {
	FH_MATH: 0x4000,
};

