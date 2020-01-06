add_executable(fixdir fixdir.c)
target_link_libraries(fixdir ythtbbs ytht pcre ${LIBXML2_LIBRARIES})
