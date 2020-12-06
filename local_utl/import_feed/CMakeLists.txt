add_executable(import_feed main.c test_board.c import_board.c import_user common.c db.c)
target_link_libraries(import_feed ythtbbs)

