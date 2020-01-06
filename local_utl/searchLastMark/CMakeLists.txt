add_executable(searchLastMark searchLastMark.c)
target_link_libraries(searchLastMark ythtbbs ytht pcre ${LIBXML2_LIBRARIES})
