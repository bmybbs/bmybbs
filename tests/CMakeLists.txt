add_subdirectory(libytht/test_ytht_del_from_file)
add_subdirectory(libbmy/test_join_int_array_to_string)

add_executable(bmybbs.test
	main.c
	libytht/check_timeop.c
	libythtbbs/check_libidentify.c
	libythtbbs/check_mailsender.c
	libythtbbs/check_notification.c
	libythtbbs/check_captcha.c
	libythtbbs/check_session.c
	libbmy/check_iphash.c
	libbmy/check_redis.c
	libbmy/check_cookie.c
	libbmy/check_2fa.c
)
target_link_libraries(bmybbs.test check ythtbbs bmy ytht)
