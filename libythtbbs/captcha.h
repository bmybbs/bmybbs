#ifndef BMYBBS_CAPTCHA_H
#define BMYBBS_CAPTCHA_H

enum CAPTCHA_STATUS_CODE {
	CAPTCHA_USED                  = 1,
	CAPTCHA_ALLOW_TO_REGEN        = 2,
	CAPTCHA_TIMEOUT               = 3,
	CAPTCHA_NOT_ALLOW_TO_REGEN    = 4,
	CAPTCHA_FILE_ERROR            = -1,
	CAPTCHA_OK                    = 0
};

struct BMYCaptcha {
	char value[6];
	long long timestamp;
	time_t create_time;
};

int gen_captcha_for_user(const char *userid, struct BMYCaptcha *captcha);
#endif //BMYBBS_CAPTCHA_H
