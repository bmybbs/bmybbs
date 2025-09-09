add_executable(atthttpd main.c sv_core.c conn.c mimetype.c)
target_link_libraries(atthttpd ythtbbs ytht)
