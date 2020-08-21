add_executable(newtop10 newtop10.c ../common/sysrecord.c ../common/www.c)
target_link_libraries(newtop10 ythtbbs ytht ghthash)
