#ifndef BMY_PG_WRAPPER
#define BMY_PG_WRAPPER

#include <stdbool.h>
#include <libpq-fe.h>

/**
 * 建立数据库连接
 */
PGconn *bmy_pg_connect_default(void);

bool bmy_pg_begin(PGconn *conn);
bool bmy_pg_commit(PGconn *conn);
bool bmy_pg_rollback(PGconn *conn);

/**
 * @brief 执行参数化的 SQL 语句
 *
 * @details 调用者必须检验调用状态，并使用 PQclear() 释放返回值。
 */
PGresult *bmy_pg_exec_params(PGconn *conn, const char *sql, int n_params, const char *const params[]);

#endif
