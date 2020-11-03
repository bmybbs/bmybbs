#include "bbslib.h"

int
bbssecfly_main()
{
	struct boardmem *(data[MAXBOARD]), *x;
	int i, total = 0;
	const struct sectree *sec;

	sec = getsectree("?");
/*	if (secstr[0] != '*') {
		    if (cache_header
		    (max(thisversion, file_time(MY_BBS_HOME "/wwwtmp")), 120))
			return 0;
	}
*/
	html_header(1);
	check_msg();
	//printf("<style type=text/css>A {color: #0000f0}</style>");
	changemode(SELECT);

	printf("<script src=\"/inc/tog.js\"></script></head><body leftmargin=0 topmargin=0>\n");
	show_banner();

	//show boards
	printf("%s", "<table width=75% border=0 cellpadding=0 cellspacing=0>\n");
	show_sec(&sectree);

	//show right top header
	printf("</table>\n<br></td>\n");
	printf("</body></html>");
	return 0;


}

