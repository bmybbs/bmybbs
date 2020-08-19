add_library (ytht SHARED
	uudecode.c
	uuencode.c
	strhash.c
	common.c
	numbyte.c
	strop.c
	fileop.c
	strlib.c
	shmop.c
	named_socket.c
	crypt.c
	limitcpu.c
	timeop.c
	smth_filter.c
	mgrep.c
	smtp.c
)

target_link_libraries(ytht ssl crypto)
