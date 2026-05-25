pkg_check_modules(LIBPQ REQUIRED libpq)

add_library(bmy SHARED
	mysql_wrapper.c
	smtp.c
	iphash.c
	cookie.c
	wechat.c
	convcode.c
	article.c
	board.c
	user.c
	subscription.c
	algorithms.c
	2fa.c
	search.c
	logging.c
	pg_wrapper.c
)

target_link_libraries(bmy ytht mysqlclient ssl crypto curl json-c hiredis ${LIBPQ_LIBRARIES})

