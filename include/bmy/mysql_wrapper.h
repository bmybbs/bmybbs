#ifndef BMY_MYSQL_WRAPPER
#define BMY_MYSQL_WRAPPER
#include <mysql/mysql.h>

enum MYSQL_CHARSET_BMY {
	MYSQL_CHARSET_GBK,
	MYSQL_CHARSET_UTF8
};

enum MYSQL_STATUS {
	MYSQL_CANNOT_CONNECT_TO_MYSQL        = -1,
	MYSQL_CANNOT_INIT_STMT               = -2,
	MYSQL_CANNOT_BIND_PARAMS             = -3,
	MYSQL_CANNOT_BIND_RESULT_COLS        = -4,
	MYSQL_CANNOT_EXEC_STMT               = -5,
	MYSQL_CANNOT_STORE_RESULT            = -6,
	MYSQL_OK                             = 0
};

typedef void (*BMY_MYSQL_WRAPPER_CALLBACK)(MYSQL_STMT *stmt, MYSQL_BIND *result_cols, void *result_set);

/**
 * 使用 mysql prepared statement 进行 sql 查询的函数封装
 *
 * 执行查询后结果集会全部缓存在客户端，因此需要留意查询的规模。函数内部处理了建立连接、查询结束后释放资源。结果集的使用由回调函数处理，并返回回调函数的执行结果。
 * @param sqlbuf 查询的 sql 语句（以 prepared statement 形式写入）
 * @param charset 使用的字符集
 * @param params 绑定的参数，不存在输入 NULL
 * @param result_cols 绑定的返回值，不存在输入 NULL，会被传入 callback
 * @param result_set 用于存放返回结果的指针，会被传入 callback
 * @param callback 处理查询结果的回调函数
 * @return 返回 mysql 调用状态
 */
int execute_prep_stmt(const char* sqlbuf, enum MYSQL_CHARSET_BMY charset, MYSQL_BIND *params, MYSQL_BIND *result_cols, void *result_set, BMY_MYSQL_WRAPPER_CALLBACK callback);

#endif
