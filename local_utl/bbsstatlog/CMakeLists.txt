add_executable(bbsstatlog bbsstatlog.c)
target_link_libraries(bbsstatlog ythtbbs ytht pcre ${LIBXML2_LIBRARIES})

