add_definitions(-DBACK_DELETE_RANGE)
add_executable(thread
	thread.c
	#../bbs/record.c
)

target_link_libraries(thread ythtbbs ytht)
