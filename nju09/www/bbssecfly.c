#include "bbslib.h"

// bbsboa
extern void show_banner();
extern void show_sec(const struct sectree *sec);

int bbssecfly_main() {
	html_header(1);
	check_msg();
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

