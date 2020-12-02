#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/file.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "config.h"
#include "ytht/msg.h"
#include "ytht/fileop.h"
#include "bmy/wechat.h"

static const char *BMY_USER_AGENT = "libbmy/1.0";
static const char *WECHAT_CONFIG_FILE = MY_BBS_HOME "/etc/wechatconfig";

struct MemoryStruct {
	char *memory;
	size_t size;
};

// ref https://curl.se/libcurl/c/getinmemory.html
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *) userp;

	char *ptr = realloc(mem->memory, mem->size + realsize + 1);
	if (ptr == NULL) {
		newtrace("[bmy/wechat] WriteMemoryCallback failed to realloc");
		return 0;
	}

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

int bmy_wechat_session_get(const char *code, struct bmy_wechat_session *s) {
	CURL *curl_handle;
	CURLcode res;

	int rc;
	FILE *cfg_fp;
	char  wechat_appid[24];
	char  wechat_secret[40];
	char  url[256];
	char  log_buf[128];

	struct MemoryStruct chunk;
	json_object *jobj;

	if (code == NULL || strlen(code) == 0 || s == NULL || s->openid != NULL || s->session_key != NULL)
		return BMY_WECHAT_ERRCODE_WRONG_PARAM;

	cfg_fp = fopen(WECHAT_CONFIG_FILE, "r");
	if (!cfg_fp)
		return BMY_WECHAT_ERRCODE_NO_CFGFILE;
	readstrvalue_fp(cfg_fp, "appid", wechat_appid, sizeof(wechat_appid));
	readstrvalue_fp(cfg_fp, "secret", wechat_secret, sizeof(wechat_secret));
	fclose(cfg_fp);

	chunk.memory = malloc(1);
	chunk.size = 0;

	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();

	snprintf(url, sizeof(url), "https://api.weixin.qq.com/sns/jscode2session?appid=%s&secret=%s&js_code=%s&grant_type=authorization_code", wechat_appid, wechat_secret, code);
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *) &chunk);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, BMY_USER_AGENT);

	res = curl_easy_perform(curl_handle);
	if (res != CURLE_OK) {
		snprintf(log_buf, sizeof(log_buf), "[bmy/wechat] request session with curl result: %d", res);
		newtrace(log_buf);
		rc = BMY_WECHAT_REQUEST_ERROR;
	} else {
		jobj = json_tokener_parse(chunk.memory);
		rc = json_object_get_int(json_object_object_get(jobj, "errcode"));
		if (rc == BMY_WECHAT_ERRCODE_SUCCESS) {
			s->openid = strdup(json_object_get_string(json_object_object_get(jobj, "openid")));
			s->session_key = strdup(json_object_get_string(json_object_object_get(jobj, "session_key")));
		} else {
			snprintf(log_buf, sizeof(log_buf), "[bmy/wechat] request session errcode[%d] errmsg: %s", rc, json_object_get_string(json_object_object_get(jobj, "errmsg")));
			newtrace(log_buf);
		}
	}

	if (chunk.memory)
		free(chunk.memory);
	return rc;
}

void bmy_wechat_session_free(struct bmy_wechat_session *s) {
	if (s->openid)
		free(s->openid);
	if (s->session_key)
		free(s->session_key);
}

