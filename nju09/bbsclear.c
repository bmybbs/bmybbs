#include "bbslib.h"

int
bbsclear_main()
{
	char board[80], start[80], buf[256];
	html_header(1);
	check_msg();
	strsncpy(board, getparm("B"), 32);
	if (!board[0])
		strsncpy(board, getparm("board"), 32);
	strsncpy(start, getparm("S"), 32);
	if (!start[0])
		strsncpy(start, getparm("start"), 32);
	if (!getboard(board))
		http_fatal("´íÎóµÄÌÖÂÛÇø");
	changemode(READNEW);
	brc_initial(currentuser.userid, board);
	brc_clear();
	brc_update(currentuser.userid);
	sprintf(buf, "%s%s&S=%s", showByDefMode(), board, start);
	refreshto(buf, 0);
	http_quit();
	return 0;
}
