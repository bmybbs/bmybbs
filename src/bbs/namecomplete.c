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

#include "bbs.h"

extern int can_R_endline;

struct word {
	char *word;
	struct word *next;
};

struct word *toplev = NULL, *current = NULL;

static void FreeNameList(void);
static int NumInList(register struct word *list);
static struct word *GetSubList(register char *tag, register struct word *list);
static void ClearSubList(struct word *list);
static int MaxLen(struct word *list, int count);
static size_t MaxCommonStr(char str[STRLEN], struct word *list, size_t n);
static int UserMaxLen(char cwlist[][IDLEN + 1], int cwnum, int morenum, int count);
static int UserSubArray(char cwbuf[][IDLEN + 1], char cwlist[][IDLEN + 1], int cwnum, int key, int pos);

static void
FreeNameList()
{
	struct word *p, *temp;

	for (p = toplev; p != NULL; p = temp) {
		temp = p->next;
		free(p->word);
		free(p);
	}
}

void
CreateNameList()
{
	if (toplev)
		FreeNameList();
	toplev = NULL;
	current = NULL;
}

void
AddNameList(name)
char *name;
{
	struct word *node;

	node = (struct word *) malloc(sizeof (struct word));
	node->next = NULL;
	node->word = (char *) malloc(strlen(name) + 1);
	strcpy(node->word, name);
	if (toplev == NULL) {
		toplev = node;
		current = node;
	} else {
		current->next = node;
		current = node;
	}
}

static int
NumInList(list)
register struct word *list;
{
	register int i;

	for (i = 0; list != NULL; i++, list = list->next)
		/*Null Statement */ ;
	return i;
}

void
ApplyToNameList(fptr)
int ((*fptr) ());
{
	struct word *p;

	for (p = toplev; p != NULL; p = p->next)
		(*fptr) (p->word);
}

int
chkstr(otag, tag, name)
char *otag, *tag, *name;
{
	char ch, *oname = name;

	while (*tag != '\0') {
		ch = *name++;
		if (*tag != (char) toupper(ch))
			return 0;
		tag++;
	}

	if (*tag != '\0' && *name == '\0')
		strcpy(otag, oname);

	return 1;
}

static struct word *
GetSubList(tag, list)
register char *tag;
register struct word *list;
{
	struct word *wlist, *wcurr;
	char tagbuf[STRLEN];
	int n;

	wlist = NULL;
	wcurr = NULL;
	for (n = 0; tag[n] != '\0'; n++) {
		tagbuf[n] = toupper(tag[n]);
	}
	tagbuf[n] = '\0';
	while (list != NULL) {
		if (chkstr(tag, tagbuf, list->word)) {
			register struct word *node;

			node = (struct word *) malloc(sizeof (struct word));
			node->word = list->word;
			node->next = NULL;
			if (wlist)
				wcurr->next = node;
			else
				wlist = node;
			wcurr = node;
		}
		list = list->next;
	}
	return wlist;
}

static void
ClearSubList(list)
struct word *list;
{
	struct word *tmp_list;

	while (list) {
		tmp_list = list->next;
		free(list);
		list = tmp_list;
	}
}

static int
MaxLen(list, count)
struct word *list;
int count;
{
	int len = strlen(list->word);

	while (list != NULL && count) {
		int t = strlen(list->word);
		if (t > len)
			len = t;
		list = list->next;
		count--;
	}
	return len;
}

static size_t
MaxCommonStr(str, list, n)
char str[STRLEN];
struct word *list;
size_t n;
{
	size_t len;
	strcpy(str, list->word);
	len = strlen(str);
	list = list->next;
	while (list != NULL && len > n) {
		while (strncasecmp(str, list->word, len) != 0 && len > 0)
			len--;
		str[len] = '\0';
		if (strlen(list->word) == len)
			strcpy(str, list->word);
		list = list->next;
	}
	return len;
}

#define NUMLINES (t_lines - 4)

int
namecomplete(prompt, data)
char *prompt, *data;
{
	char *temp;
	int ch;
	int count = 0;
	int clearbot = NA;
	struct word *cwlist, *morelist;
	int x, y;
	int origx, origy;
	if (prompt != NULL) {
		prints("%s", prompt);
		clrtoeol();
	}
	temp = data;

	if (toplev == NULL)
		AddNameList("");
	cwlist = GetSubList("", toplev);
	morelist = NULL;
	getyx(&y, &x);
	getyx(&origy, &origx);
	while (1) {
		can_R_endline = 1;
		ch = igetkey();
		can_R_endline = 0;
		if (ch == EOF)
			break;
		if (ch == '\n' || ch == '\r') {
			struct word *wordptr = cwlist;
			*temp = '\0';
			prints("\n");
			if (NumInList(cwlist) == 1) {
				strcpy(data, cwlist->word);
				break;
			}
			//if(!strcasecmp(data,cwlist->word))
			//    strcpy(data,cwlist->word) ;
			while (wordptr != NULL) {
				if (strcasecmp(data, wordptr->word) == 0) {
					strcpy(data, wordptr->word);
					break;
				}
				wordptr = wordptr->next;
			}
			if (wordptr == NULL)
				data[0] = 0;
			ClearSubList(cwlist);
			break;
		}
		if (ch == ' ') {
			int col, len;
			char str[STRLEN];
			if (count < 2)
				continue;
			if (NumInList(cwlist) == 1) {
				strcpy(data, cwlist->word);
				move(y, x);
				prints("%s", data + count);
				count = strlen(data);
				temp = data + count;
				getyx(&y, &x);
				continue;
			}
			//×Ô¶¯²¹Æë, ´úÂëÌí¼Ó¿ªÊ¼, by ecnegrevid
			if (MaxCommonStr(str, cwlist, strlen(data)) > strlen(data)) {
				struct word *node;
				strcpy(data, str);
				node = GetSubList(data, cwlist);
				ClearSubList(cwlist);
				cwlist = node;
				move(y, x);
				prints("%s", data + count);
				count = strlen(data);
				temp = data + count;
				getyx(&y, &x);
			}
			//×Ô¶¯²¹ÆëÌí¼Ó½áÊø
			clearbot = YEA;
			col = 0;
			if (!morelist)
				morelist = cwlist;
			len = MaxLen(morelist, NUMLINES);
			move(origy + 1, 0);
			clrtobot();
//			standout();
			printdash(" ÁÐ±í ");
//			standend();
			while (len + col < 80) {
				int i;
				for (i = NUMLINES;
				     (morelist) && (i > origy - 1); i--) {
					move(origy + 2 + (NUMLINES - i), col);
					prints("%s", morelist->word);
					morelist = morelist->next;
				}
				col += len + 2;
				if (!morelist)
					break;
				len = MaxLen(morelist, NUMLINES);
			}
			if (morelist) {
				move(t_lines - 1, 0);
				prints
				    ("[1;44m-- »¹ÓÐ --                                                                     [m");
			}
			move(y, x);
			continue;
		}
		if (ch == '\177' || ch == '\010') {
			if (temp == data)
				continue;
			temp--;
			count--;
			*temp = '\0';
			ClearSubList(cwlist);
			cwlist = GetSubList(data, toplev);
			morelist = NULL;
			x--;
			move(y, x);
			outc(' ');
			move(y, x);
			continue;
		}
		if(ch == '#' && count==0){
			*temp = '\0';
			ClearSubList(cwlist);
			return ( ch );
		}//add by macintosh 07.05.28
		if (count < STRLEN) {
			struct word *node;

			*temp++ = ch;
			count++;
			*temp = '\0';
			node = GetSubList(data, cwlist);
			if (node == NULL) {
				temp--;
				*temp = '\0';
				count--;
				continue;
			}
			ClearSubList(cwlist);
			cwlist = node;
			morelist = NULL;
			move(y, x);
			outc(ch);
			x++;
		}
	}
	if (ch == EOF)
		longjmp(byebye, -1);
	prints("\n");
	if (clearbot) {
		move(2, 0);
		clrtobot();
	}
	if (*data) {
		move(origy, origx);
		prints("%s\n", data);
	}
	return 0;
}

static int
UserMaxLen(cwlist, cwnum, morenum, count)
char cwlist[][IDLEN + 1];
int cwnum, morenum, count;
{
	int len, max = 0;

	while (count-- > 0 && morenum < cwnum) {
		len = strlen(cwlist[morenum++]);
		if (len > max)
			max = len;
	}
	return max;
}

static int
UserSubArray(cwbuf, cwlist, cwnum, key, pos)
char cwbuf[][IDLEN + 1];
char cwlist[][IDLEN + 1];
int cwnum, key, pos;
{
	int key2, num = 0;
	int n, ch;
	key = (char) toupper(key);
	if (key >= 'A' && key <= 'Z') {
		key2 = key - 'A' + 'a';
	} else {
		key2 = key;
	}
	for (n = 0; n < cwnum; n++) {
		ch = cwlist[n][pos];
		if (ch == key || ch == key2) {
			strcpy(cwbuf[num++], cwlist[n]);
		}
	}
	return num;
}

int
usercomplete(prompt, data)
char *prompt, *data;
{
	char *u_namearray();
	char *cwbuf, *cwlist, *temp;
	int cwnum, x, y, origx, origy;
	int clearbot = NA, count = 0, morenum = 0;
	char ch;
	//int ch;

	cwbuf = malloc(MAXUSERS * (IDLEN + 1));
	if (prompt != NULL) {
		prints("%s", prompt);
		clrtoeol();
	}
	temp = data;
	cwlist = u_namearray((void *)cwbuf, &cwnum, "");
	getyx(&y, &x);
	getyx(&origy, &origx);
	while ((ch = igetkey()) != EOF) {
		if (ch == '\n' || ch == '\r') {
			int i;
			char *ptr;

			*temp = '\0';
			prints("\n");
			ptr = cwlist;
			for (i = 0; i < cwnum; i++) {
				if (strncasecmp(data, ptr, IDLEN + 1) == 0)
					strcpy(data, ptr);
				ptr += IDLEN + 1;
			}
/*
                if( cwnum == 1 )
                    strcpy( data, cwlist );
*/
			break;
		} else if (ch == ' ') {
			int col, len;
			
			if (count < 3)
				continue;
			if (cwnum == 1) {
				strcpy(data, cwlist);
				move(y, x);
				prints("%s", data + count);
				count = strlen(data);
				temp = data + count;
				getyx(&y, &x);
				continue;
			}
			clearbot = YEA;
			col = 0;
			len = UserMaxLen((void *)cwlist, cwnum, morenum, NUMLINES);
			move(origy + 1, 0);
			clrtobot();
			printdash(" ËùÓÐÊ¹ÓÃÕßÁÐ±í ");
			while (len + col < 79) {
				int i;
				for (i = 0;
				     morenum < cwnum
				     && i < NUMLINES - origy + 1; i++) {
					move(origy + 2 + i, col);
					prints("%s ",
					       cwlist + (IDLEN +
							 1) * morenum++);
				}
				col += len + 2;
				if (morenum >= cwnum)
					break;
				len =
				    UserMaxLen((void *)cwlist, cwnum, morenum,
					       NUMLINES);
			}
			if (morenum < cwnum) {
				move(t_lines - 1, 0);
				prints
				    ("[1;44m-- »¹ÓÐÊ¹ÓÃÕß --                                                               [m");
			} else {
				morenum = 0;
			}
			move(y, x);
			continue;
		} else if (ch == '\177' || ch == '\010') {
			if (temp == data)
				continue;
			temp--;
			count--;
			*temp = '\0';
			cwlist = u_namearray((void *)cwbuf, &cwnum, data);
			morenum = 0;
			x--;
			move(y, x);
			outc(' ');
			move(y, x);
			continue;
		} else if (count < STRLEN) {
			int n;

			*temp++ = ch;
			*temp = '\0';
			n = UserSubArray((void *)cwbuf, (void *)cwlist, cwnum, ch, count);
			if (n == 0) {
				temp--;
				*temp = '\0';
				continue;
			}
			cwlist = cwbuf;
			count++;
			cwnum = n;
			morenum = 0;
			move(y, x);
			outc(ch);
			x++;
		}
	}
	free(cwbuf);
	if (ch == EOF)
		longjmp(byebye, -1);
	prints("\n");
	if (clearbot) {
		move(origy + 1, 0);
		clrtobot();
	}
	if (*data) {
		move(origy, origx);
		prints("%s\n", data);
	}
	return 0;
}
