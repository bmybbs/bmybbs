add_executable(printSecLastUpdate printSecLastUpdate.c ../common/www.c)
target_link_libraries(printSecLastUpdate ythtbbs ytht pcre ${LIBXML2_LIBRARIES})
