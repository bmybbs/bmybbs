BASEPATH = ../..
include $(BASEPATH)/Makefile.Base
#LIBS = $(LIBS) -lcheck

BUILD_TEST_NOTIFICATION: check_notification.c
	$(CC) $(CFLAGS) -o check_notification check_notification.c $(LDFLAGS) $(BBSLIB) $(LIBS) -lcheck
