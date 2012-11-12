#include "bbslib.h"

int
bbschangestyle_main()
{
	char name[STRLEN], p[STRLEN], main_page[STRLEN], *tmp;
	int colorIndex, n = NWWWSTYLE;
	
	colorIndex = atoi(getparm("color"));
	
	get_session_string(name);
	tmp = strchr(name, '/');
	if (NULL != tmp) {
		*tmp = '\0';
	}
		
	if (!strcmp(currentuser.userid,"guest")) {
		n = NWWWSTYLE - 1;
	}
	
	if (colorIndex > -1 && colorIndex < n) {
		addextraparam(name, sizeof(name), 0, colorIndex);
		
		sprintf(p, "/%s%s./", SMAGIC, name);

		print_session_string(p);
		html_header(1);

		sprintf(main_page, "/%s/", SMAGIC);
		redirect(main_page);
	} else {
		http_fatal("错误的配色方案");
	}
	
        http_quit();
        return 0;
}
