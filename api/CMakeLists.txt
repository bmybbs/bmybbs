add_library(apilib STATIC apilib.c)

add_executable(bmyapi
	main.c
	api_error.c
	api_template.c
	api_user.c
	api_article.c
	api_board.c
	api_meta.c
	api_attach.c
	api_mail.c
	api_notification.c
	api_subscription.c
	api_oauth.c
	api_search.c
)

target_link_libraries(bmyapi apilib ythtbbs ytht onion json-c hiredis)

add_executable(bmybbs.api.test
	tests/main.c
	tests/apilib.test.c
)

target_link_libraries(bmybbs.api.test check apilib ythtbbs ytht onion json-c)
