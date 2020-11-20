#ifndef NJU09_CHECK_SERVER_H
#define NJU09_CHECK_SERVER_H
#include <stdbool.h>

#define MAX_URL_LEN 1024

extern bool g_is_nginx;

// 截断 g_url_buf 的查询参数后，等效于 SCRIPT_URL
extern char *g_url;

// 用于存放 REQUEST_URI 环境变量的缓冲区，该值包含
// 获取到的格式为 /foo/bar?p=v 而原 SCRIPT_URL 获取的
// 数据不包含 query string，因此需要额外处理
extern char g_url_buf[MAX_URL_LEN];

/**
 * @brief 检查服务器软件
 * 依据环境变量 SERVER_SOFTWARE 判断，如果字符串以 "nginx/" 开头，则
 * 设置 g_is_nginx 为 true
 */
extern void check_server(void);
#endif

