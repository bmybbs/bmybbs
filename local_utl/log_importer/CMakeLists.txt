add_executable(log_importer_tokenizer.test
	test_log_tokenizer.c
	log_tokenizer.c
)

target_link_libraries(log_importer_tokenizer.test PRIVATE ${CHECK_LIBRARIES})
