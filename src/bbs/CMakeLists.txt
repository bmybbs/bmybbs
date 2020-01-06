set(SRC
	bbs.c
	record.c
	delete.c
	main.c
	stuff.c
	mail.c
	register.c
	xyz.c
	talk.c
	read.c
	comm_lists.c
	namecomplete.c
	chat.c
	maintain.c
	edit.c
	more.c
	help.c
	bcache.c
	boards.c
	smth_screen.c
	io.c
	term.c
	userinfo.c
	vote.c
	announce.c
	sendmsg.c
	bm.c
	list.c
	goodbye.c
	bbsgopher.c
	fileshm.c
	postheader.c
	convcode.c
	five.c
	tmachine.c
	addressbook.c
	backnumber.c
	common.c
	sec_hand.c
	regular.c
	1984.c
	moneycenter.c
	editboard.c
	power_select.c
	tmpl.c
	logger.c
	identify.c
)

set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/proto.h)
set(SHOULD_GENERATE_PROTO FALSE)

if(NOT EXISTS ${PROTO_FILE})
	set(SHOULD_GENERATE_PROTO TRUE)
else(NOT EXISTS ${PROTO_FILE})
	foreach(file ${SRC})
		if(${CMAKE_CURRENT_SOURCE_DIR}/${file} IS_NEWER_THAN ${PROTO_FILE})
			set(SHOULD_GENERATE_PROTO TRUE)
		endif(${CMAKE_CURRENT_SOURCE_DIR}/${file} IS_NEWER_THAN ${PROTO_FILE})
	endforeach(file)
endif(NOT EXISTS ${PROTO_FILE})

if(SHOULD_GENERATE_PROTO)
	execute_process(
		COMMAND bash -c "echo \"#include \\\"bbs.h\\\"\" > proto.h ; cproto `echo \"${SRC}\" | sed 's/;/ /g'` -DBBSMAIN -DLINUX -DBACK_DELETE_RANGE -I ../../include -I ../../ythtlib -I ../../libythtbbs >> proto.h"
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
	message(STATUS "${PROTO_FILE} has been generated successfully")
else(SHOULD_GENERATE_PROTO)
	message(STATUS "there's no need to regenerate ${PROTO_FILE}")
endif(SHOULD_GENERATE_PROTO)

add_definitions(-DBBSMAIN -DBACK_DELETE_RANGE)
add_compile_options(-include ${PROTO_FILE})

add_executable(bbs ${SRC})
target_link_libraries(bbs ythtbbs ytht ${LIBXML2_LIBRARIES} m mysqlclient pcre)
