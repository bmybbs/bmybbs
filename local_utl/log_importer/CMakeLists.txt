pkg_check_modules(LIBPQ REQUIRED libpq)

add_executable(log_importer
	main.c
	importer.c
	db.c
	log_parser.c
	log_tokenizer.c
)

add_executable(log_importer_tokenizer.test
	test_log_tokenizer.c
	log_tokenizer.c
)

add_executable(log_importer_parser.test
	test_log_parser.c
	log_parser.c
	log_tokenizer.c
)

target_link_libraries(log_importer_tokenizer.test PRIVATE ${CHECK_LIBRARIES})
target_link_libraries(log_importer_parser.test PRIVATE bmy ${CHECK_LIBRARIES})
target_include_directories(log_importer PRIVATE ${LIBPQ_INCLUDE_DIRS})
target_link_libraries(log_importer PRIVATE bmy)
