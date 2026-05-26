#include "bmy/pg_wrapper.h"
#include <libpq-fe.h>

static bool bmy_pg_exec_simple(PGconn *conn, const char *sql);

PGconn *bmy_pg_connect_default(void) {
	return PQconnectdb("");
}

bool bmy_pg_begin(PGconn *conn) {
	return bmy_pg_exec_simple(conn, "BEGIN");
}

bool bmy_pg_commit(PGconn *conn) {
	return bmy_pg_exec_simple(conn, "COMMIT");
}

bool bmy_pg_rollback(PGconn *conn) {
	return bmy_pg_exec_simple(conn, "ROLLBACK");
}

PGresult *bmy_pg_exec_params(PGconn *conn, const char *sql, int n_params, const char *const params[]) {
	return PQexecParams(conn, sql, n_params, NULL, params, NULL, NULL, 0);
}

static bool bmy_pg_exec_simple(PGconn *conn, const char *sql) {
	PGresult *res;
	bool ok;

	res = PQexec(conn, sql);
	ok = PQresultStatus(res) == PGRES_COMMAND_OK;

	PQclear(res);
	return ok;
}
