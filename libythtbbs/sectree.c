#include <stdio.h>
#include "sectree.h"
const struct sectree sectree;
static const struct sectree sectreeA;
static const struct sectree sectreeAA;
static const struct sectree sectreeB;
static const struct sectree sectreeBA;
static const struct sectree sectreeBB;
static const struct sectree sectreeC;
static const struct sectree sectreeD;
static const struct sectree sectreeE;
static const struct sectree sectreeF;
static const struct sectree sectreeG;
static const struct sectree sectreeH;
static const struct sectree sectreeI;
static const struct sectree sectreeIA;
static const struct sectree sectreeJ;
static const struct sectree sectreeJA;
static const struct sectree sectreeK;
static const struct sectree sectreeL;
static const struct sectree sectreeLA;
static const struct sectree sectreeM;
static const struct sectree sectreeN;
static const struct sectree sectreeO;
/*------ sectree -------*/
const struct sectree sectree = {
	parent: NULL,
	title: "兵马俑BBS",
	basestr: "",
	seccodes: "0123456789GNHAC",
	introstr: "0123456789GNHCA",
	des: "",
	nsubsec: 15,
	subsec: {
		&sectreeA,
		&sectreeB,
		&sectreeC,
		&sectreeD,
		&sectreeE,
		&sectreeF,
		&sectreeG,
		&sectreeH,
		&sectreeI,
		&sectreeJ,
		&sectreeK,
		&sectreeL,
		&sectreeM,
		&sectreeN,
		&sectreeO,
	}
};
/*------ sectreeA -------*/
static const struct sectree sectreeA = {
	parent: &sectree,
	title: "本站系统",
	basestr: "0",
	seccodes: "BC",
	introstr: "BC",
	des: "[本站]",
	nsubsec: 1,
	subsec: {
		&sectreeAA,
	}
};
/*------ sectreeAA -------*/
static const struct sectree sectreeAA = {
	parent: &sectreeA,
	title: "版务讨论",
	basestr: "0B",
	seccodes: "",
	introstr: "",
	des: "",
	nsubsec: 0,
	subsec: {
	}
};
static const struct sectree sectreeAC = {
	parent: &sectreeA,
	title: "已关版面",
	basestr: "0C",
	seccodes: "",
	introstr: "",
	des: "",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeB -------*/
static const struct sectree sectreeB = {
	parent: &sectree,
	title: "交通大学",
	basestr: "1",
	seccodes: "AB",
	introstr: "ABC",
	des: "[院系]",
	nsubsec: 2,
	subsec: {
		&sectreeBA,
		&sectreeBB,
	}
};
/*------ sectreeBA -------*/
static const struct sectree sectreeBA = {
	parent: &sectreeB,
	title: "院系",
	basestr: "1A",
	seccodes: "",
	introstr: "",
	des: "",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeBB -------*/
static const struct sectree sectreeBB = {
	parent: &sectreeB,
	title: "社团群体",
	basestr: "1B",
	seccodes: "",
	introstr: "",
	des: "",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeC -------*/
static const struct sectree sectreeC = {
	parent: &sectree,
	title: "开发技术",
	basestr: "2",
	seccodes: "",
	introstr: "",
	des: "[编程][开发]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeD -------*/
static const struct sectree sectreeD = {
	parent: &sectree,
	title: "电脑应用",
	basestr: "3",
	seccodes: "",
	introstr: "",
	des: "[电脑][应用]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeE -------*/
static const struct sectree sectreeE = {
	parent: &sectree,
	title: "学术科学",
	basestr: "4",
	seccodes: "",
	introstr: "",
	des: "[学术][科学]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeF -------*/
static const struct sectree sectreeF = {
	parent: &sectree,
	title: "社会科学",
	basestr: "5",
	seccodes: "",
	introstr: "",
	des: "[人文][社会]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeG -------*/
static const struct sectree sectreeG = {
	parent: &sectree,
	title: "文学艺术",
	basestr: "6",
	seccodes: "",
	introstr: "",
	des: "[文学][艺术]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeH -------*/
static const struct sectree sectreeH = {
	parent: &sectree,
	title: "知性感性",
	basestr: "7",
	seccodes: "",
	introstr: "",
	des: "[闲聊][感性]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeI -------*/
static const struct sectree sectreeI = {
	parent: &sectree,
	title: "体育运动",
	basestr: "8",
	seccodes: "A",
	introstr: "A",
	des: "[体育]",
	nsubsec: 1,
	subsec: {
		&sectreeIA,
	}
};
/*------ sectreeIA -------*/
static const struct sectree sectreeIA = {
	parent: &sectreeI,
	title: "足球俱乐部",
	basestr: "8A",
	seccodes: "",
	introstr: "",
	des: "",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeJ -------*/
static const struct sectree sectreeJ = {
	parent: &sectree,
	title: "休闲音乐",
	basestr: "9",
	seccodes: "A",
	introstr: "A",
	des: "[休闲][音乐]",
	nsubsec: 1,
	subsec: {
		&sectreeJA,
	}
};
/*------ sectreeJA -------*/
static const struct sectree sectreeJA = {
	parent: &sectreeJ,
	title: "明星俱乐部",
	basestr: "9A",
	seccodes: "",
	introstr: "",
	des: "",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeK -------*/
static const struct sectree sectreeK = {
	parent: &sectree,
	title: "游戏天地",
	basestr: "G",
	seccodes: "",
	introstr: "",
	des: "[游戏]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeL -------*/
static const struct sectree sectreeL = {
	parent: &sectree,
	title: "新闻信息",
	basestr: "N",
	seccodes: "B",
	introstr: "B",
	des: "[新闻][信息]",
	nsubsec: 1,
	subsec: {
		&sectreeLA,
	}
};
/*------ sectreeLA -------*/
static const struct sectree sectreeLA = {
	parent: &sectreeL,
	title: "交易市场",
	basestr: "NB",
	seccodes: "",
	introstr: "",
	des: "",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeM -------*/
static const struct sectree sectreeM = {
	parent: &sectree,
	title: "乡音乡情",
	basestr: "H",
	seccodes: "",
	introstr: "",
	des: "[陕西][其他]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeN -------*/
static const struct sectree sectreeN = {
	parent: &sectree,
	title: "校务信息",
	basestr: "A",
	seccodes: "",
	introstr: "",
	des: "[校务][信息]",
	nsubsec: 0,
	subsec: {
	}
};
/*------ sectreeO -------*/
static const struct sectree sectreeO = {
	parent: &sectree,
	title: "俱乐部区",
	basestr: "C",
	seccodes: "",
	introstr: "",
	des: "     [俱乐部]",
	nsubsec: 0,
	subsec: {
	}
};
