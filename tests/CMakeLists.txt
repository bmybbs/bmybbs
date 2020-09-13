add_executable(bmybbs.test
	main.c
	libythtbbs/check_libidentify.c
	libythtbbs/check_mailsender.c
	libythtbbs/check_notification.c
	libythtbbs/check_captcha.c
	libbmy/check_iphash.c
)
target_link_libraries(bmybbs.test check ythtbbs bmy ytht)
