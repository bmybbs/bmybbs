#include <string.h>
#include <sys/file.h>
#include "bmy/smtp.h"
#include "config.h"
#include "ytht/fileop.h"
#include "ytht/msg.h"
#include "ythtbbs/mailsender.h"
#include "ythtbbs/misc.h"

const char *MAIL_CONFIG_FILE = MY_BBS_HOME "/etc/smtpconfig";
const int SMTP_FLAG_NO_DEBUG = 0;

/**
 * To check where a character is *illegal*
 * @param c
 * @return 1 if illegal
 */
static int
illegal_email_chars(char c) {
	// https://stackoverflow.com/a/2049510/803378
	// special characters !#$%&'*+-/=?^_`{|}~
	if(!(c >= 'A' && c <= 'Z')
			&& !(c >= 'a' && c <= 'z')
			&& !(c >= '0' && c <= '9')
			&& c != '.' && c != '-'
			&& c != '_')
		return 1;
	return 0;
}

enum mail_sender_code
check_mail_to_address(const char *mail_to) {
	int i;
	char *at, *ptr;
	const char * MAIL_PATTERNS[] = {
		"@xjtu.edu.cn",
		"@mail.xjtu.edu.cn",
		"@stu.xjtu.edu.cn"
	};

	if (strlen(mail_to) <= strlen(MAIL_PATTERNS[0]))
		return MAIL_SENDER_WRONG_EMAIL;

	at = strrchr(mail_to, '@');
	if (at == NULL || at == mail_to)
		return MAIL_SENDER_WRONG_EMAIL;

	for(ptr = (char *) mail_to; ptr != at; ptr++) {
		if (illegal_email_chars(*ptr) == 1)
			return MAIL_SENDER_WRONG_EMAIL;
	}

	for (i=0; i<3; i++) {
		if (!strcmp(at, MAIL_PATTERNS[i]))
			return MAIL_SENDER_SUCCESS;
	}

	return MAIL_SENDER_WRONG_EMAIL;
}

enum mail_sender_code
send_mail(const char *mail_to, const char *mail_to_name, const char *mail_subject, const char *mail_body) {
	char mail_server[32];
	char mail_port[8];
	char security[32]; // "TLS" / "STARTTLS" / "NONE"

	char mail_user[32];
	char mail_name[32];
	char mail_pass[32];

	int  mail_security;
	int  rc, rc1, rc2, rc3, rc4, rc5, rc6, rc7;
	char log[160];
	int  cfg_fd;
	FILE *cfg_fp;
	struct smtp *smtp;

	rc = check_mail_to_address(mail_to);
	if (rc != MAIL_SENDER_SUCCESS)
		return rc;

	cfg_fp = fopen(MAIL_CONFIG_FILE, "r");
	if (!cfg_fp)
		return MAIL_SENDER_CONFIG_NOT_EXIST;

	cfg_fd = fileno(cfg_fp);
	flock(cfg_fd, LOCK_SH);

	readstrvalue_fp(cfg_fp, "MAIL_SERVER", mail_server, sizeof(mail_server));
	readstrvalue_fp(cfg_fp, "MAIL_PORT", mail_port, sizeof(mail_port));
	readstrvalue_fp(cfg_fp, "SECURITY", security, sizeof(security));

	readstrvalue_fp(cfg_fp, "MAIL_USER", mail_user, sizeof(mail_user));
	readstrvalue_fp(cfg_fp, "MAIL_NAME", mail_name, sizeof(mail_name));
	readstrvalue_fp(cfg_fp, "MAIL_PASS", mail_pass, sizeof(mail_pass));

	flock(cfg_fd, LOCK_UN);
	fclose(cfg_fp);

	if (!strcmp(security, "TLS"))
		mail_security = SMTP_SECURITY_TLS;
	else if (!strcmp(security, "STARTTLS"))
		mail_security = SMTP_SECURITY_STARTTLS;
	else
		mail_security = SMTP_SECURITY_NONE;

	rc1 = smtp_open(mail_server, mail_port, mail_security, SMTP_FLAG_NO_DEBUG, NULL, &smtp);

	rc2 = smtp_auth(smtp, SMTP_AUTH_PLAIN, mail_user, mail_pass);

	rc3 = smtp_address_add(smtp, SMTP_ADDRESS_FROM, mail_user, mail_name);
	rc4 = smtp_address_add(smtp, SMTP_ADDRESS_TO, mail_to, mail_to_name);

	rc5 = smtp_header_add(smtp, "Subject", mail_subject);
	rc6 = smtp_mail(smtp, mail_body);

	rc7 = smtp_close(smtp);

	snprintf(log, sizeof(log), "[mail] %s send to %s smtp-status %d-%d-%d-%d-%d-%d-%d",
		mail_to_name, mail_to, rc1, rc2, rc3, rc4, rc5, rc6, rc7);
	newtrace(log);

	return rc7;
}

