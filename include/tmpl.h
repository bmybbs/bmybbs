#ifndef _TEMPLATE_H
#define _TEMPLATE_H
#include <stddef.h>
#include "config.h"

#define TEMPLATE_DIR ".tmpl"
#define MAX_TEMPLATE 20
#define MAX_CONTENT 20
#define TMPL_BM_FLAG 0x1
#define MAX_CONTENT_LENGTH 555
#define TMPL_NOW_VERSION 1

struct s_content {
	char text[50];
	size_t length;
};

struct s_template {
	char title[50];
	char authorid[IDLEN + 1];
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
