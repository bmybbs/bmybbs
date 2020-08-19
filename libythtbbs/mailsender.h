#ifndef BMYBBS_MAILSENDER_H
#define BMYBBS_MAILSENDER_H

/**
 * 状态码
 */
enum mail_sender_code {
	MAIL_SENDER_SUCCESS = 0,
	/* enum_status_code */
	MAIL_SENDER_WRONG_EMAIL = 20,
	MAIL_SENDER_CONFIG_NOT_EXIST = 21,
	MAIL_SENDER_CONFIG_ERROR = 22,
	/* not a valid status code */
	MAIL_SENDER__LAST
};

/**
 * 发送电子邮件
 * @param mail_to         收件人邮箱
 * @param mail_to_name    收件人名字（建议使用 bmy id）
 * @param mail_subject    邮件标题（utf-8 编码）
 * @param mail_body       邮件正文（utf-8 编码）
 * @return 状态码 @ref mail_sender_code
 */
enum mail_sender_code send_mail(const char *mail_to, const char *mail_to_name, const char *mail_subject, const char *mail_body);
#endif //BMYBBS_MAILSENDER_H
