#ifndef BMYBBS_CAPTCHA_H
#define BMYBBS_CAPTCHA_H

enum CAPTCHA_FILE_TYPE {
	CAPTCHA_FILE_REGISTER         = 0,
	CAPTCHA_FILE_RESET            = 1,
};

enum CAPTCHA_STATUS_CODE {
	CAPTCHA_USED                  = 1,
	CAPTCHA_ALLOW_TO_REGEN        = 2,
	CAPTCHA_TIMEOUT               = 3,
	CAPTCHA_NOT_ALLOW_TO_REGEN    = 4,
	CAPTCHA_FILE_ERROR            = -1,
	CAPTCHA_WRONG                 = -2,
	CAPTCHA_NO_CAP                = -3,
	CAPTCHA_OK                    = 0
};

struct BMYCaptcha {
	char value[6];
	long long timestamp;
	time_t create_time;
};

int unlink_captcha(const char *userid, enum CAPTCHA_FILE_TYPE file_type);

int check_captcha_status(const char *userid, enum CAPTCHA_FILE_TYPE file_type);

int gen_captcha_for_user(const char *userid, struct BMYCaptcha *captcha, enum CAPTCHA_FILE_TYPE file_type);

int verify_captcha_for_user(const char *userid, const char *code, enum CAPTCHA_FILE_TYPE file_type);

void gen_captcha_url(char *buf, size_t buf_size, long long timestamp);
#endif //BMYBBS_CAPTCHA_H
