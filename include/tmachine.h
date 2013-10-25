#ifndef _M_TELNET_H
#define _M_TELNET_H


extern char          term_type[64];
extern int           term_cols;
extern int           term_lines;
extern int           max_timeout;
extern unsigned char options[256];
extern unsigned int  mstate;

extern char          remotehost[64];
extern char          user_name[64];
extern char          exec_param[256];


#define  QUEUE_BUF_SIZE   512

typedef struct queue_tlag {
	int  rptr;
	int  wptr;
	char buf[QUEUE_BUF_SIZE];
} queue_tl;

void queue_init(queue_tl* q);
int  queue_free_size(queue_tl* q);
int  queue_data_size(queue_tl* q);
int  queue_read(queue_tl* q,char *buf,int size);
int  queue_write(queue_tl* q,const char *buf,int size);
int  queue_recv(queue_tl* q,int socket);
int  queue_send(queue_tl* q,int socket);
int  queue_write_char(queue_tl* q,char ch);


int nload(const char *file);
int nsave(const char *file);
int nread(int net,void *pbuf,int size);
int nwrite(int net,const void *pbuf,int size);
int tmachine_init(int net);


#endif // _M_TELNET_H
