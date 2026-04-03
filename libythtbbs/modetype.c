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
		// 新站友注册
		return "\xD0\xC2\xD5\xBE\xD3\xD1\xD7\xA2\xB2\xE1";
	case LOGIN:
		// 进入本站
		return "\xBD\xF8\xC8\xEB\xB1\xBE\xD5\xBE";
	case DIGEST:
		// 浏览精华区
		return "\xE4\xAF\xC0\xC0\xBE\xAB\xBB\xAA\xC7\xF8";
	case MMENU:
		// 主选单
		return "\xD6\xF7\xD1\xA1\xB5\xA5";
	case ADMIN:
		// 管理者选单
		return "\xB9\xDC\xC0\xED\xD5\xDF\xD1\xA1\xB5\xA5";
	case SELECT:
		// 选择讨论区
		return "\xD1\xA1\xD4\xF1\xCC\xD6\xC2\xDB\xC7\xF8";
	case READBRD:
		// 览遍天下
		return "\xC0\xC0\xB1\xE9\xCC\xEC\xCF\xC2";
	case READNEW:
		// 览新文章
		return "\xC0\xC0\xD0\xC2\xCE\xC4\xD5\xC2";
	case READING:
		// 品味文章
		return "\xC6\xB7\xCE\xB6\xCE\xC4\xD5\xC2";
	case POSTING:
		// 文豪挥笔
		return "\xCE\xC4\xBA\xC0\xBB\xD3\xB1\xCA";
	case MAIL:
		// 处理信笺
		return "\xB4\xA6\xC0\xED\xD0\xC5\xBC\xE3";
	case SMAIL:
		// 寄语信鸽
		return "\xBC\xC4\xD3\xEF\xD0\xC5\xB8\xEB";
	case RMAIL:
		// 阅览信笺
		return "\xD4\xC4\xC0\xC0\xD0\xC5\xBC\xE3";
	case TMENU:
		// 聊天选单
		return "\xC1\xC4\xCC\xEC\xD1\xA1\xB5\xA5";
	case LUSERS:
		// 环顾四方
		return "\xBB\xB7\xB9\xCB\xCB\xC4\xB7\xBD";
	case FRIEND:
		// 寻找好友
		return "\xD1\xB0\xD5\xD2\xBA\xC3\xD3\xD1";
	case MONITOR:
		// 探视民情
		return "\xCC\xBD\xCA\xD3\xC3\xF1\xC7\xE9";
	case QUERY:
		// 查询网友
		return "\xB2\xE9\xD1\xAF\xCD\xF8\xD3\xD1";
	case TALK:
		// 聊天
		return "\xC1\xC4\xCC\xEC";
	case PAGE:
		// 呼叫
		return "\xBA\xF4\xBD\xD0";
	case CHAT1:
		// 聊天室中
		return "\xC1\xC4\xCC\xEC\xCA\xD2\xD6\xD0";
	case CHAT2:
		// 夜猫子客栈
		return "\xD2\xB9\xC3\xA8\xD7\xD3\xBF\xCD\xD5\xBB";
	case CHAT3:
		// 版主会议室
		return "\xB0\xE6\xD6\xF7\xBB\xE1\xD2\xE9\xCA\xD2";
	case CHAT4:
		// 站务会议室
		return "\xD5\xBE\xCE\xF1\xBB\xE1\xD2\xE9\xCA\xD2";
	case IRCCHAT:
		// 会谈IRC
		return "\xBB\xE1\xCC\xB8IRC";
	case LAUSERS:
		// 探视网友
		return "\xCC\xBD\xCA\xD3\xCD\xF8\xD3\xD1";
	case XMENU:
		// 系统资讯
		return "\xCF\xB5\xCD\xB3\xD7\xCA\xD1\xB6";
	case VOTING:
		// 投票
		return "\xCD\xB6\xC6\xB1";
	case BBSNET:
		return "BBSNET";
	case EDITWELC:
		// 编辑Welc
		return "\xB1\xE0\xBC\xADWelc";
	case EDITUFILE:
		// 编辑个人档
		return "\xB1\xE0\xBC\xAD\xB8\xF6\xC8\xCB\xB5\xB5";
	case EDITSFILE:
		// 编修系统档
		return "\xB1\xE0\xD0\xDE\xCF\xB5\xCD\xB3\xB5\xB5";
	case ZAP:
		// 订阅讨论区
		return "\xB6\xA9\xD4\xC4\xCC\xD6\xC2\xDB\xC7\xF8";
	case GAME:
		// 脑力激荡
		return "\xC4\xD4\xC1\xA6\xBC\xA4\xB5\xB4";
	case SYSINFO:
		// 检查系统
		return "\xBC\xEC\xB2\xE9\xCF\xB5\xCD\xB3";
	case ARCHIE:
		return "ARCHIE";
	case DICT:
		// 翻查字典
		return "\xB7\xAD\xB2\xE9\xD7\xD6\xB5\xE4";
	case LOCKSCREEN:
		// 屏幕锁定
		return "\xC6\xC1\xC4\xBB\xCB\xF8\xB6\xA8";
	case NOTEPAD:
		// 留言板
		return "\xC1\xF4\xD1\xD4\xB0\xE5";
	case GMENU:
		// 工具箱
		return "\xB9\xA4\xBE\xDF\xCF\xE4";
	case MSG:
		// 讯息中
		return "\xD1\xB6\xCF\xA2\xD6\xD0";
	case USERDEF:
		// 自订参数
		return "\xD7\xD4\xB6\xA9\xB2\xCE\xCA\xFD";
	case EDIT:
		// 修改文章
		return "\xD0\xDE\xB8\xC4\xCE\xC4\xD5\xC2";
	case OFFLINE:
		// 自杀中..
		return "\xD7\xD4\xC9\xB1\xD6\xD0..";
	case EDITANN:
		// 编修精华
		return "\xB1\xE0\xD0\xDE\xBE\xAB\xBB\xAA";
	case WWW:
		// 悠游 WWW
		return "\xD3\xC6\xD3\xCE WWW";
	case HYTELNET:
		return "Hytelnet";
	case CCUGOPHER:
		// 他站精华
		return "\xCB\xFB\xD5\xBE\xBE\xAB\xBB\xAA";
	case LOOKMSGS:
		// 察看讯息
		return "\xB2\xEC\xBF\xB4\xD1\xB6\xCF\xA2";
	case WFRIEND:
		// 寻人名册
		return "\xD1\xB0\xC8\xCB\xC3\xFB\xB2\xE1";
	case FIVE:
		// 五子棋 VS
		return "\xCE\xE5\xD7\xD3\xC6\xE5 VS";
	case PAGE_FIVE:
		// 邀请下棋
		return "\xD1\xFB\xC7\xEB\xCF\xC2\xC6\xE5";
	case WORKER:
		// 推箱子
		return "\xCD\xC6\xCF\xE4\xD7\xD3";
	case TETRIS:
		// 俄罗斯方块
		return "\xB6\xED\xC2\xDE\xCB\xB9\xB7\xBD\xBF\xE9";
	case WINMINE:
		// 扫雷
		return "\xC9\xA8\xC0\xD7";
	case WINMINE2:
		// 感应式扫雷
		return "\xB8\xD0\xD3\xA6\xCA\xBD\xC9\xA8\xC0\xD7";
	case TT:
		// 打字练习
		return "\xB4\xF2\xD7\xD6\xC1\xB7\xCF\xB0";
	case ADDRESSBOOK:
		// 察看通讯录
		return "\xB2\xEC\xBF\xB4\xCD\xA8\xD1\xB6\xC2\xBC";
	case SELBACKNUMBER:
		// 选择过刊
		return "\xD1\xA1\xD4\xF1\xB9\xFD\xBF\xAF";
	case BACKNUMBER:
		// 浏览过刊
		return "\xE4\xAF\xC0\xC0\xB9\xFD\xBF\xAF";
	case RECITE:
		// 背单词
		return "\xB1\xB3\xB5\xA5\xB4\xCA";
	case CHESS:
		// 棋牌中心
		return "\xC6\xE5\xC5\xC6\xD6\xD0\xD0\xC4";
	case QKMJ:
		// 打麻将
		return "\xB4\xF2\xC2\xE9\xBD\xAB";
	case NCCE:
		// 科技词典
		return "\xBF\xC6\xBC\xBC\xB4\xCA\xB5\xE4";
	case GOODWISH:
		// 送祝福
		return "\xCB\xCD\xD7\xA3\xB8\xA3";
	case M_2NDHAND:
		// 跳蚤市场
		return "\xCC\xF8\xD4\xE9\xCA\xD0\xB3\xA1";
	case DO1984:
		// 研究文章
		return "\xD1\xD0\xBE\xBF\xCE\xC4\xD5\xC2";
	case MONEY:
		// 专心挣钱
		return "\xD7\xA8\xD0\xC4\xD5\xF5\xC7\xAE";
	case QUICKCALC:
		// 处理数字
		return "\xB4\xA6\xC0\xED\xCA\xFD\xD7\xD6";
	case FREEIP:
		// 查询IP
		return "\xB2\xE9\xD1\xAF\x49\x50";
	case USERDF1:
		// 吃饭去了
		return "\xB3\xD4\xB7\xB9\xC8\xA5\xC1\xCB";
	case USERDF2:
		// 和MM聊天
		return "\xBA\xCDMM\xC1\xC4\xCC\xEC";
	case USERDF3:
		// 别来烦我
		return "\xB1\xF0\xC0\xB4\xB7\xB3\xCE\xD2";
	case USERDF4:
		// 荧幕锁定
		return "\xD3\xAB\xC4\xBB\xCB\xF8\xB6\xA8";
	default:
		// 去了哪里!?
		return "\xC8\xA5\xC1\xCB\xC4\xC4\xC0\xEF!?";
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
