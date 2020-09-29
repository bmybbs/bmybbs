#ifndef _TEMPLATE_H
#define _TEMPLATE_H
#include "config.h"

struct s_content {
	char text[50];
	size_t length;
};

struct s_template {
	char title[50];
	char authorid[IDLEN];
	char title_prefix[20];
	int content_num;
	char filename[STRLEN];
	int flag;
	int version;
	char unused[16];
	char title_tmpl[STRLEN];
};

struct a_template {
	struct s_template * tmpl;
	struct s_content * cont;
};

int m_template(void);
int choose_tmpl(void);
#endif
