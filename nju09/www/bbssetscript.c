#include "bbslib.h"

int
bbssetscript_main()
{
	html_header(1);
	if (!loginok)
		return 0;
	w_info->doc_mode=0;
	return 0;
}
