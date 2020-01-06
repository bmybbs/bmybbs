add_executable(searchDIR searchDIR.c)
target_link_libraries(searchDIR ythtbbs ytht pcre ${LIBXML2_LIBRARIES})
