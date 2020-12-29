#ifndef LOCAL_UTL_IMPORT_FEED_DB_H
#define LOCAL_UTL_IMPORT_FEED_DB_H
#include <mysql/mysql.h>

#if MYSQL_VERSION_ID < 80000
typedef my_bool MYBOOL;
#else
typedef bool MYBOOL;
#endif

MYSQL *my_connect_mysql(MYSQL *s);
void mysql_error_stmt(MYSQL_STMT *stmt);
#endif

