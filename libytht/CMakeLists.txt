add_library (ytht SHARED
	uudecode.c
	uuencode.c
	common.c
	numbyte.c
	fileop.c
	strlib.c
	shmop.c
	crypt.c
	limitcpu.c
	timeop.c
	smth_filter.c
	mgrep.c
	random.c
	newtrace.c
)

target_link_libraries(ytht)
