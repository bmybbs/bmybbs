add_library(bmy SHARED
	mysql_wrapper.c
	smtp.c
	iphash.c
)

target_link_libraries(bmy ytht mysqlclient ssl crypto)