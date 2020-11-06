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

	bbsresetpass.c
	bbsfindacc.c
	bbsnotify.c

	bbsdelnotify.c
	bbsbadd.c
	bbsbdel.c
	bbsball.c

	BBSLIB.c
	bbsupdatelastpost.c
	boardrc.c
	deny_users.c
)

add_executable(www ${SRC})
target_link_libraries(www ythtbbs ytht ghthash)
