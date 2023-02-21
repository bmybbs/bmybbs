CC		= gcc
BSRCPTH	= /home/bmybbs
FLAGS 	= -O -Wall -g -D_GNU_SOURCE -I$(BSRCPTH)/include -I$(BSRCPTH)/ythtlib -I$(BSRCPTH)/libythtbbs `xml2-config --cflags`
BBSLIBS	= -L/home/bbs/bin -lythtbbs -lytht -lmysqlclient -lxml2 -ljson-c -lpcre -lm -lhiredis
ONILIBS = -lonion -pthread -lrt

PROGNAME = bmyapi
CFILES	:= main.c api_error.c api_template.c api_user.c \
		   apilib.c api_article.c api_board.c api_brc.c \
		   api_meta.c api_attach.c api_mail.c api_notification.c
		   
COBJS	:= $(CFILES:.c=.o)
.c.o	:; $(CC) -c $*.c $(FLAGS)

all: $(PROGNAME)

$(PROGNAME): $(COBJS)
	$(CC) -o $@ $^ $(BBSLIBS) $(ONILIBS)
	
clean:
	rm -rf $(COBJS) $(PROGNAME)
