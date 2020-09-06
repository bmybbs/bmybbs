#ifdef CAN_EXEC
#include "bbs.h"

void
queue_init(queue_tl * q)
{
	q->rptr = 0;
	q->wptr = 0;
}

int
queue_free_size(queue_tl * q)
{
	if (q->rptr <= q->wptr)
		return (q->rptr + sizeof (q->buf) - 1) - q->wptr;
	return q->rptr - 1 - q->wptr;
}

int
queue_data_size(queue_tl * q)
{
	if (q->wptr < q->rptr)
		return (q->wptr + sizeof (q->buf)) - q->rptr;
	return q->wptr - q->rptr;
}

int
queue_read(queue_tl * q, char *buf, int size)
{
	int i;
	for (i = 0; i < size; i++) {
		if (q->rptr == q->wptr)
			break;
		buf[i] = q->buf[q->rptr];
		q->rptr = (q->rptr + 1) % sizeof (q->buf);
	}
	return i;
}

int
queue_write_char(queue_tl * q, char ch)
{
	if ((q->wptr + 1) % sizeof (q->buf) == q->rptr)
		return 0;
	q->buf[q->wptr] = ch;
	q->wptr = (q->wptr + 1) % sizeof (q->buf);
	return 1;
}

int
queue_write(queue_tl * q, const char *buf, int size)
{
	int i;
	for (i = 0; i < size; i++) {
		if ((q->wptr + 1) % sizeof (q->buf) == q->rptr)
			break;
		q->buf[q->wptr] = buf[i];
		q->wptr = (q->wptr + 1) % sizeof (q->buf);
	}
	return i;
}

int
queue_send(queue_tl * q, int socket)
{
	int count, ret;

	count = 0;
	ret = 0;
	if (q->rptr == q->wptr)
		return 0;	// nothing to send
	if (q->rptr > q->wptr) {
		ret =
		    write(socket, &q->buf[q->rptr], sizeof (q->buf) - q->rptr);
		if (ret <= 0)
			return ret - 1;
		count += ret;
		q->rptr = (q->rptr + ret) % sizeof (q->buf);
		if (q->rptr != 0)
			return count;
	}
	ret = write(socket, &q->buf[q->rptr], q->wptr - q->rptr);
	if (ret <= 0)
		return ret - 1;
	count += ret;
	q->rptr = (q->rptr + ret) % sizeof (q->buf);
	return count;
}

int
queue_recv(queue_tl * q, int socket)
{
	int count, ret;
	if ((q->wptr + 1) % sizeof (q->buf) == q->rptr)
		return 0;	// queue full
	if (q->wptr > q->rptr) {
		count = sizeof (q->buf) - q->wptr;
		if (q->rptr == 0)
			count--;
		ret = read(socket, &q->buf[q->wptr], count);
		if (ret <= 0)
			return ret - 1;
		q->wptr = (q->wptr + ret) % sizeof (q->buf);
		return ret;
	}
	count = q->rptr - q->wptr - 1;
	ret = read(socket, &q->buf[q->wptr], count);
	if (ret <= 0)
		return ret - 1;
	q->wptr = (q->wptr + ret) % sizeof (q->buf);
	return ret;
}

#endif
