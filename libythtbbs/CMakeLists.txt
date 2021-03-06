add_executable(gensectree gensectree.c)
target_link_libraries(gensectree ytht)

set(SECTREE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/sectree.c)
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

if(NOT EXISTS ${SECTREE_FILE})
	execute_process(
		COMMAND touch ${SECTREE_FILE}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
endif(NOT EXISTS ${SECTREE_FILE})

add_custom_target(sectree ALL
	COMMAND bash -c "if [ ! -f ${SECTREE_FILE} -o ! -s ${SECTREE_FILE} -o ${SECLIST_FILE} -nt ${SECTREE_FILE} ]; then ./gensectree >${SECTREE_FILE} ; fi"
	COMMENT "checking ${SECTREE_FILE} whether it should be generated"
	VERBATIM
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(ythtbbs SHARED
	boardrc.c
	misc.c
	record.c
	user.c
	article.c
	modetype.c
	docutil.c
	msg.c
	goodgbid.c
	binaryattach.c
	bbseva.c
	attach.c
	announce.c
	board.c
	sectree.c
	sectreeop.c
	identify.c
	notification.c
	mailsender.c
	captcha.c
	cache/user.c
	cache/utmp.c
	cache/board.c
	session.c
	override.c
	mybrd.c
)

add_dependencies(ythtbbs sectree)
add_dependencies(sectree gensectree)
target_link_libraries(ythtbbs bmy pcre mysqlclient ${LIBXML2_LIBRARIES} hiredis)

