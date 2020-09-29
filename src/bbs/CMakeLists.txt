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
	regular.c
	1984.c
	moneycenter.c
	editboard.c
	power_select.c
	tmpl.c
	identify.c
)

add_definitions(-DBBSMAIN -DBACK_DELETE_RANGE)

add_executable(bbs ${SRC})
target_link_libraries(bbs ythtbbs ytht m)
