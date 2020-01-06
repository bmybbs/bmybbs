add_executable(mergeb mergeb.c ../common/record.c)
target_link_libraries(mergeb ythtbbs ytht pcre ${LIBXML2_LIBRARIES})
