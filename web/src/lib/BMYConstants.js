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

