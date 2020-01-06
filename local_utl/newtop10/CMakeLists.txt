add_executable(newtop10 newtop10.c ../common/sysrecord.c)
target_link_libraries(newtop10 ythtbbs ytht ghthash pcre ${LIBXML2_LIBRARIES})
