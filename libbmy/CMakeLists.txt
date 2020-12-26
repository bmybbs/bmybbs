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
)

target_link_libraries(bmy ytht mysqlclient ssl crypto curl json-c)

