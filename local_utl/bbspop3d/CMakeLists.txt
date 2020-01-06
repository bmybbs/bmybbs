add_executable(bbspop3d bbspop3d.c)
target_link_libraries(bbspop3d ythtbbs ytht pcre ${LIBXML2_LIBRARIES})
