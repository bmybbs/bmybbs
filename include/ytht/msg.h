#ifndef YTHT_MSG_H
#define YTHT_MSG_H

struct mymsgbuf {
	long int mtype;
	char mtext[1];
};

void newtrace(const char *s);
#endif

