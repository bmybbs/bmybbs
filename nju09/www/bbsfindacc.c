#include "bbslib.h"
#include "ythtbbs/identify.h"

#ifdef POP_CHECK
// 登陆邮件服务器用的头文件 added by interma@BMY 2005.5.12
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// 邮件服务器上用户名和密码的长度， added by interma@BMY 2005.5.12
#define USER_LEN 20
#define PASS_LEN 20

#endif

/**
 * RETURN:
 * -1 wrong params
 * -2 email not exist
 * -3 not enough memory
 * -4 cannot send mail
 */
void api_bbsfindacc() {
	int rc = 0;
	int domain_idx;
	char email[80];
	struct associated_userid *au;
	char *user   = getparm("user");
	char *domain = getparm("domain");

	if (user == NULL || strlen(user) == 0 || strlen(user) > 20
		|| domain == NULL || strlen(domain) != 1) {
		rc = -1;
		goto OUTPUT;
	}

	domain_idx = domain[0] - '0';
	snprintf(email, 80, "%s@%s", user, MAIL_DOMAINS[domain_idx]);
	str_to_lowercase(email);

	au = get_associated_userid(email);
	if (au == NULL || au->count == 0) {
		rc = -2;
		goto OUTPUT;
	}

	rc = send_findacc_mail(au, email);
	if (rc != 1) {
		goto OUTPUT;
	}

	rc = 0;
OUTPUT:
	if (au != NULL) free_associated_userid(au);
	json_header();
	printf("{ \"status\": %d }", rc);
}

int
bbsfindacc_main()
{
	char *type = getparm("type");

	if (strcmp(type, "1") == 0) {
		api_bbsfindacc();
		return 0;
	}

	http_fatal("面包坏了？");
	return 0;
}

