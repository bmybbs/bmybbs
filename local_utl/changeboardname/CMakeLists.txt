add_executable(changeboardname changeboardname.c ../common/record.c ../common/sysrecord.c)
target_link_libraries(changeboardname ythtbbs ytht pcre ${LIBXML2_LIBRARIES})
