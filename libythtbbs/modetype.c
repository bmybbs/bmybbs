/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu
    Firebird Bulletin Board System
    Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
                        Peng Piaw Foong, ppfoong@csie.ncu.edu.tw

    Copyright (C) 1999, KCN,Zhou Lin, kcn@cic.tsinghua.edu.cn
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version. 

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include "ythtbbs/modes.h"

char *
ModeType(int mode)
{
	switch (mode) {
	case IDLE:
		return "";
	case NEW:
		return "新站友注册";
	case LOGIN:
		return "进入本站";
	case DIGEST:
		return "浏览精华区";
	case MMENU:
		return "主选单";
	case ADMIN:
		return "管理者选单";
	case SELECT:
		return "选择讨论区";
	case READBRD:
		return "览遍天下";
	case READNEW:
		return "览新文章";
	case READING:
		return "品味文章";
	case POSTING:
		return "文豪挥笔";
	case MAIL:
		return "处理信笺";
	case SMAIL:
		return "寄语信鸽";
	case RMAIL:
		return "阅览信笺";
	case TMENU:
		return "聊天选单";
	case LUSERS:
		return "环顾四方";
	case FRIEND:
		return "寻找好友";
	case MONITOR:
		return "探视民情";
	case QUERY:
		return "查询网友";
	case TALK:
		return "聊天";
	case PAGE:
		return "呼叫";
	case CHAT1:
		return "聊天室中";
	case CHAT2:
		return "夜猫子客栈";
	case CHAT3:
		return "版主会议室";
	case CHAT4:
		return "站务会议室";
	case IRCCHAT:
		return "会谈IRC";
	case LAUSERS:
		return "探视网友";
	case XMENU:
		return "系统资讯";
	case VOTING:
		return "投票";
	case BBSNET:
		return "BBSNET";
	case EDITWELC:
		return "编辑Welc";
	case EDITUFILE:
		return "编辑个人档";
	case EDITSFILE:
		return "编修系统档";
	case ZAP:
		return "订阅讨论区";
	case GAME:
		return "脑力激荡";
	case SYSINFO:
		return "检查系统";
	case ARCHIE:
		return "ARCHIE";
	case DICT:
		return "翻查字典";
	case LOCKSCREEN:
		return "屏幕锁定";
	case NOTEPAD:
		return "留言板";
	case GMENU:
		return "工具箱";
	case MSG:
		return "讯息中";
	case USERDEF:
		return "自订参数";
	case EDIT:
		return "修改文章";
	case OFFLINE:
		return "自杀中..";
	case EDITANN:
		return "编修精华";
	case WWW:
		return "悠游 WWW";
	case HYTELNET:
		return "Hytelnet";
	case CCUGOPHER:
		return "他站精华";
	case LOOKMSGS:
		return "察看讯息";
	case WFRIEND:
		return "寻人名册";
	case FIVE:
		return "五子棋 VS";
	case PAGE_FIVE:
		return "邀请下棋";
	case WORKER:
		return "推箱子";
	case TETRIS:
		return "俄罗斯方块";
	case WINMINE:
		return "扫雷";
	case WINMINE2:
		return "感应式扫雷";
	case TT:
		return "打字练习";
	case ADDRESSBOOK:
		return "察看通讯录";
	case SELBACKNUMBER:
		return "选择过刊";
	case BACKNUMBER:
		return "浏览过刊";
	case RECITE:
		return "背单词";
	case CHESS:
		return "棋牌中心";
	case QKMJ:
		return "打麻将";
	case NCCE:
		return "科技词典";
	case GOODWISH:
		return "送祝福";
	case M_2NDHAND:
		return "跳蚤市场";
	case DO1984:
		return "研究文章";
	case MONEY:
		return "专心挣钱";
	case QUICKCALC:
		return "处理数字";
	case FREEIP:
		return "查询IP";
	case USERDF1:
		return "吃饭去了";
	case USERDF2:
		return "和MM聊天";
	case USERDF3:
		return "别来烦我";
	case USERDF4:
		return "荧幕锁定";
	default:
		return "去了哪里!?";
	}
}

char *
ModeColor(int mode)
{
	switch (mode) {
	case POSTING:
	case PAGE_FIVE:
		return "\x1b[1;32m";
	case CHAT1:
	case CHAT2:
	case CHAT3:
	case CHAT4:
	case BBSNET:
		return "\x1b[1;33m";
	case USERDF1:
	case USERDF2:
	case USERDF3:
	case USERDF4:
		return "\x1b[1;34m";
	default:
		return "";
	}
}
