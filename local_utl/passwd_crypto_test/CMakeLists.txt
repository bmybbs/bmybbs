add_library(passwd_crypto_test_common OBJECT common.c)
target_include_directories(passwd_crypto_test_common PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

add_executable(generator generator.c $<TARGET_OBJECTS:passwd_crypto_test_common>)
target_include_directories(generator PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../include
)
target_link_libraries(generator ytht)

add_executable(validator_in_tree validator_in_tree.c $<TARGET_OBJECTS:passwd_crypto_test_common>)
target_include_directories(validator_in_tree PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../include
)
target_link_libraries(validator_in_tree ytht)

add_executable(validator_libcrypt validator_libcrypt.c $<TARGET_OBJECTS:passwd_crypto_test_common>)
target_include_directories(validator_libcrypt PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../include
)
target_link_libraries(validator_libcrypt crypt)
