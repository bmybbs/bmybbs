SET(CONFSRCS
	idea.c
)

SET(COMMON_SRCS
	rsa.c
	randoms.c
	buffer.c
	emulate.c
	packet.c
	compress.c
	xmalloc.c
	bufaux.c
	authfile.c
	crc32.c
	rsaglue.c
	cipher.c
	des.c
	match.c
	mpaux.c
	userfile.c
	signals.c
	blowfish.c
	deattack.c
	md5.c
)

SET(BBS_SRCS
	../src/bbs/bbs.c
	../src/bbs/record.c
	../src/bbs/delete.c
	../src/bbs/main.c
	../src/bbs/stuff.c
	../src/bbs/mail.c
	../src/bbs/register.c
	../src/bbs/xyz.c
	../src/bbs/talk.c
	../src/bbs/read.c
	../src/bbs/comm_lists.c
	../src/bbs/namecomplete.c
	../src/bbs/chat.c
	../src/bbs/maintain.c
	../src/bbs/edit.c
	../src/bbs/more.c
	../src/bbs/help.c
	../src/bbs/bcache.c
	../src/bbs/boards.c
	../src/bbs/smth_screen.c
	../src/bbs/io.c
	../src/bbs/term.c
	../src/bbs/userinfo.c
	../src/bbs/vote.c
	../src/bbs/announce.c
	../src/bbs/sendmsg.c
	../src/bbs/bm.c
	../src/bbs/list.c
	../src/bbs/goodbye.c
	../src/bbs/bbsgopher.c
	../src/bbs/fileshm.c
	../src/bbs/postheader.c
	../src/bbs/convcode.c
	../src/bbs/five.c
	../src/bbs/tmachine.c
	../src/bbs/addressbook.c
	../src/bbs/backnumber.c
	../src/bbs/common.c
	../src/bbs/regular.c
	../src/bbs/1984.c
	../src/bbs/moneycenter.c
	../src/bbs/editboard.c
	../src/bbs/power_select.c
	../src/bbs/tmpl.c
	../src/bbs/identify.c
	../src/bbsd/ip.c
)

SET(SSHD_SRCS
	sshd.c
	auth-passwd.c
	log-server.c
	canohost.c
	servconf.c
	tildexpand.c
	serverloop.c
	${COMMON_SRCS}
	${BBS_SRCS}
)

SET(BBSHOME            /home/bbs)
SET(ETCDIR             ${BBSHOME}/etc)
SET(SERVER_CONFIG_FILE ${ETCDIR}/sshd_config)
SET(HOST_KEY_FILE      ${ETCDIR}/ssh_host_key)

add_definitions(-DBBSMAIN -DBACK_DELETE_RANGE -DSSHBBS -DHOST_KEY_FILE=\"${HOST_KEY_FILE}\" -DSERVER_CONFIG_FILE=\"${SERVER_CONFIG_FILE}\" -DETCDIR=\"${ETCDIR}\" -DPIDDIR=\"${BBSHOME}\")
add_compile_options(-include ../src/bbs/proto.h)

add_executable(sshbbsd ${SSHD_SRCS} ${CONFSRCS})
target_link_libraries(sshbbsd ythtbbs ytht m gmp z)
