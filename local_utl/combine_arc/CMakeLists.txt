add_executable(combine_arc combine_arc.c)
target_link_libraries(combine_arc ythtbbs ytht pcre ${LIBXML2_LIBRARIES})
