add_executable(gensectree gensectree.c)
target_link_libraries(gensectree ytht)

if(EXISTS ${CMAKE_SOURCE_DIR}/site/seclist.txt)
	set(SECLIST_FILE ${CMAKE_SOURCE_DIR}/site/seclist.txt)
else(EXISTS ${CMAKE_SOURCE_DIR}/site/seclist.txt)
	set(SECLIST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/seclist.txt)
endif(EXISTS ${CMAKE_SOURCE_DIR}/site/seclist.txt)

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/seclist.local OR ${SECLIST_FILE} IS_NEWER_THAN ${CMAKE_CURRENT_BINARY_DIR}/seclist.local)
	execute_process(
		COMMAND cp ${SECLIST_FILE} seclist.local
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
endif(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/seclist.local OR ${SECLIST_FILE} IS_NEWER_THAN ${CMAKE_CURRENT_BINARY_DIR}/seclist.local)

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/sectree.c OR ${SECLIST_FILE} IS_NEWER_THAN ${CMAKE_CURRENT_SOURCE_DIR}/sectree.c)
	execute_process(
		COMMAND ./gensectree
		OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/sectree.c
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
endif(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/sectree.c OR ${SECLIST_FILE} IS_NEWER_THAN ${CMAKE_CURRENT_SOURCE_DIR}/sectree.c)

add_library(ythtbbs STATIC
	boardrc.c
	misc.c
	record.c
	user.c
	article.c
	modetype.c
	docutil.c
	msg.c
	politics.c
	goodgbid.c
	binaryattach.c
	permissions.c
	bbseva.c
	attach.c
	regform.c
	announce.c
	board.c
	sectree.c
	sectreeop.c
	libiden.c
	notification.c
)

