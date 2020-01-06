set(SRC
	bbsmain.c
	bbstop10.c
	bbsdoc.c
	bbscon.c
	bbsbrdadd.c
	bbsboa.c
	bbsall.c
	bbsanc.c
	bbs0an.c

	bbslogout.c
	bbsleft.c
	bbslogin.c
	bbsbadlogins.c
	bbsqry.c
	bbsnot.c
	bbsfind.c

	bbsfadd.c
	bbsfdel.c
	bbsfall.c
	bbsfriend.c
	bbsfoot.c
	bbsform.c
	bbspwd.c
	bbsplan.c
	bbsinfo.c

	bbsmybrd.c
	bbssig.c
	bbspst.c
	bbsgcon.c
	bbsgdoc.c
	bbsmmdoc.c
	bbsdel.c
	bbsdelmail.c
	bbsmailcon.c
	bbsmail.c

	bbsdelmsg.c
	bbssnd.c
	bbsnotepad.c
	bbsmsg.c
	bbssendmsg.c
	bbsreg.c

	bbsmailmsg.c
	bbssndmail.c
	bbsnewmail.c
	bbspstmail.c
	bbsgetmsg.c
	bbscloak.c

	bbsmdoc.c
	bbsnick.c
	bbstfind.c
	bbsadl.c
	bbstcon.c
	bbstdoc.c

	bbsdoreg.c
	bbsmywww.c
	bbsccc.c
	bbsufind.c
	bbsclear.c
	bbsstat.c
	bbsedit.c
	bbsman.c
	bbsparm.c

	bbsfwd.c
	bbsmnote.c
	bbsdenyall.c
	bbsdenydel.c
	bbsdenyadd.c

	bbstopb10.c
	bbsbfind.c
	bbsx.c
	bbseva.c
	bbsvote.c

	bbsshownav.c
	bbsbkndoc.c
	bbsbknsel.c
	bbsbkncon.c

	bbshome.c
	bbsindex.c
	bbssechand.c
	bbsupload.c
	bbslform.c
	regreq.c

	bbsselstyle.c
	bbscon1.c
	bbsattach.c
	bbskick.c

	bbsscanreg.c
	bbsshowfile.c
	bbst.c
	bbsdt.c
	bbslt.c
	bbsincon.c
	bbssetscript.c

	bbscccmail.c
	bbsfwdmail.c
	bbstmpl.c
	bbsrss.c
	bbsucss.c
	bbsdefcss.c
	bbssecfly.c

	bbssbs.c
	bbseditmail.c

	bbsfindpass.c
	bbsresetpass.c
	bbsfindacc.c
	bbsnotify.c

	bbsdelnotify.c
	bbsbadd.c
	bbsbdel.c
	bbsball.c

	BBSLIB.c
)

set (OTHERS
	boardrc.c
	deny_users.c
	bbsupdatelastpost.c
	bbsred.c
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

execute_process(
	COMMAND bash -c "echo \"${SRC}\" | sed 's/;/ /g' > ${CMAKE_CURRENT_SOURCE_DIR}/list.txt"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

if(SHOULD_GENERATE_PROTO)
	execute_process(
		COMMAND bash -c "cproto -DMAKE_PROTO `echo \"${SRC}\" | sed 's/;/ /g'` -I ../../include -I ../../ythtlib -I ../../libythtbbs > proto.h"
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
	message(STATUS "${PROTO_FILE} has been generated successfully")
else(SHOULD_GENERATE_PROTO)
	message(STATUS "there's no need to regenerate ${PROTO_FILE}")
endif(SHOULD_GENERATE_PROTO)

add_executable(www ${SRC})
target_link_libraries(www ythtbbs ytht ghthash pcre mysqlclient ${LIBXML2_LIBRARIES})
