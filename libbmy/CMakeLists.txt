add_library(bmy SHARED
	mysql_wrapper.c
)

target_link_libraries(bmy ytht mysqlclient)