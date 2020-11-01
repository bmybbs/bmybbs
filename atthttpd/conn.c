#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ght_hash_table.h>

#include <time.h>

#include "config.h"
#include "conn.h"
#include "mimetype.h"

enum {
	AWAITING_REQUEST,
	SENDING_HEADER,
	SENDING_DATA,
	SENDING_CACHE_HEADER,
	SENDING_CACHE_HEADER_304,
	SENDING_ERROR_HEADER
};

typedef struct conn_t {
	int state;
	int sd;
	struct mmapfile mf;
	int pos;
	int wsize;
	int size;
	char *head;
	int hlen;
	int hoff;
	char *cache_header;
	int chlen;
	int choff;
	time_t lasttime;
} conn_t;

conn_t *vector[MAX_CONNECTIONS];
time_t now_t;
size_t num_connections;

void
conn_init(void)
{
	size_t i;

	for (i = 0; i < MAX_CONNECTIONS; i++)
		vector[i] = NULL;

	num_connections = 0;
}

void
get_load(load)
double load[];
{
#if defined(LINUX)
	FILE *fp;
	fp = fopen("/proc/loadavg", "r");
	if (!fp)
		load[0] = load[1] = load[2] = 0;
	else {
		float av[3];
		fscanf(fp, "%g %g %g", av, av + 1, av + 2);
		fclose(fp);
		load[0] = av[0];
		load[1] = av[1];
		load[2] = av[2];
	}
#elif defined(BSD44)
	getloadavg(load, 3);
#else
	struct statstime rs;
	rstat("localhost", &rs);
	load[0] = rs.avenrun[0] / (double) (1 << 8);
	load[1] = rs.avenrun[1] / (double) (1 << 8);
	load[2] = rs.avenrun[2] / (double) (1 << 8);
#endif
}

static int
l_write(int fd, const void *buf, size_t size)
{
	static int lastcounter = 0;
	int nowcounter;
	static int bufcounter;
	static int load_refresh = 0;
	static double cpu_load[3];
	int ret;
	nowcounter = time(0);
	if (nowcounter - load_refresh > 10) {
		load_refresh = nowcounter;
		get_load(cpu_load);
		if (cpu_load[0] > 2)
			cpu_load[0] = 2;
	}
	if (lastcounter == nowcounter) {
		if (bufcounter >= 1024 * 1024 * 2 * (2.5 - cpu_load[0])) {
			sleep(1);
			nowcounter = time(0);
			bufcounter = 0;
		}
	} else {
		// time clocked, clear bufcounter
		bufcounter = 0;
	}
	lastcounter = nowcounter;
	ret = write(fd, buf, size);
	bufcounter += ret;
	return ret;
}

static conn_t *
mkconn(int sd)
{
	conn_t *c = (conn_t *) malloc(sizeof (conn_t));
	if (c == NULL)
		return NULL;
	bzero(c, sizeof (*c));
	c->state = AWAITING_REQUEST;
	c->sd = sd;
	c->lasttime = now_t;
	c->mf.ptr = NULL;
	num_connections++;
	return c;
}

static conn_t *
rmconn(conn_t * c)
{
	mmapfile(NULL, &c->mf);
	close(c->sd);
	free(c);
	num_connections--;
	return NULL;
}

int
conn_insert(int sd)
{
	conn_t **v = vector;
	size_t i;

	for (i = 0; i < MAX_CONNECTIONS; i++)
		if (v[i] == NULL) {
			if ((v[i] = mkconn(sd)) == NULL)
				return -1;
			return 0;
		}
	close(sd);
	return -1;
}

struct boardmem *
getbcache(char *board)
{
	int i, j;
	static int num = 0;
	char upperstr[STRLEN];
	static ght_hash_table_t *p_table = NULL;
	static time_t uptime = 0;
	static struct BCACHE *shm_bcache = NULL;
	if (shm_bcache == NULL)
		shm_bcache = (struct BCACHE *) get_old_shm(BCACHE_SHMKEY, sizeof (struct BCACHE));
	if (shm_bcache == NULL)
		return NULL;
	if (board[0] == 0)
		return NULL;
	if (p_table && (num != shm_bcache->number || shm_bcache->uptime > uptime)) {
		ght_finalize(p_table);
		p_table = NULL;
	}
	if (p_table == NULL) {
		num = 0;
		p_table = ght_create(MAXBOARD, NULL, 0);
		for (i = 0; i < MAXBOARD && i < shm_bcache->number; i++) {
			num++;
			if (!shm_bcache->bcache[i].header.filename[0])
				continue;
			ytht_strsncpy(upperstr, shm_bcache->bcache[i].header.filename, sizeof(upperstr));
			for (j = 0; upperstr[j]; j++)
				upperstr[j] = toupper(upperstr[j]);
			ght_insert(p_table, &shm_bcache->bcache[i], j, upperstr);
		}
		uptime = now_t;
	}
	ytht_strsncpy(upperstr, board, sizeof(upperstr));
	for (j = 0; upperstr[j]; j++)
		upperstr[j] = toupper(upperstr[j]);
	return ght_get(p_table, j, upperstr);
}

int
testboard(char *board)
{
	struct boardmem *x;
	x = getbcache(board);
	if (x && !x->header.clubnum && !x->header.level)
		return 0;
	return -1;
}

/* getreq - get HTTP request, sanity checks */
static int
write_cache_header(conn_t * c, time_t t, int age, char *old)
{
	time_t oldt = 0;
	char buf[STRLEN];
	char *ptr, *ptr1;
	char *c_header;
	int ret = 0;
	if (!t)
		return 0;
	if (old) {
		struct tm tm;
		ptr = old + 19;
		while (*ptr == ' ')
			ptr++;
		ptr1 = strchr(ptr, ';');
		if (ptr1)
			*ptr1 = 0;
		if (strptime(ptr, "%a, %d %b %Y %H:%M:%S %Z", &tm)) {
			oldt = mktime(&tm) - timezone;
		}
	}
	c_header = malloc(512);
	bzero(c_header, 512);
	if (oldt >= t) {
		sprintf(c_header,
			"HTTP/1.1 304 Not Modified.\r\nCache-Control: max-age=%d\r\n\r\n",
			age);
		c->state = SENDING_CACHE_HEADER_304;
		ret = 1;
		goto END;
	}
	if (strftime(buf, STRLEN, "%a, %d %b %Y %H:%M:%S", gmtime(&t)))
		sprintf(c_header, "Last-Modified: %s GMT\r\n", buf);
	t = now_t + age;
	if (strftime(buf, STRLEN, "%a, %d %b %Y %H:%M:%S", gmtime(&t))) {
		strcat(c_header, "Expires ");
		strcat(c_header, buf);
		strcat(c_header, " GMT\r\n");
	}
	strcat(c_header, "Cache-Control: max-age=");
	sprintf(buf, "%d\r\n\r\n", age);
	strcat(c_header, buf);
	c->state = SENDING_HEADER;
      END:
	c->cache_header = c_header;
	c->chlen = strlen(c->cache_header);
	c->choff = 0;
	return ret;
}

static int
findarticle(char *board, char *filename)
{
	struct mmapfile mf={ptr:NULL};
	char dir[80];
	int index;
	int total;
	sprintf(dir, "boards/%s/.DIR", board);
	MMAP_TRY {
		if (mmapfile(dir, &mf)<0) {
			MMAP_RETURN(0);
		}
		total = mf.size / sizeof(struct fileheader);
		if (total == 0) {
			mmapfile(NULL, &mf);
			MMAP_RETURN(0);
		}
		index = Search_Bin(mf.ptr, atoi(filename+2), 0, total -1);
	}
	MMAP_CATCH {
		index = -1;
	}
	MMAP_END mmapfile(NULL, &mf);
	if (index < 0)
		return 0;
	return 1;
}

static int
getreq(conn_t * c)
{
	char buf[MAX_REQUEST_LENGTH], filepath[100], *ptr;
	char *req, *board, *filename, *posstr, *attachname, *param, *IMS;
	int enhanced = 0;
	int retv;

	while ((retv = read(c->sd, buf, sizeof (buf) - 1)) == -1)
		if (errno != EINTR) {
			return -1;
		}

	buf[retv] = 0;
	if (!(ptr = strtok(buf, " \t\r\n"))) {
		c->state = SENDING_ERROR_HEADER;
		c->head = get_error_type(404);
		c->hlen = strlen(c->head);
		c->hoff = 0;
		return 0;
	}
	if (!(ptr = strtok(NULL, " \t\r\n"))) {
		c->state = SENDING_ERROR_HEADER;
		c->head = get_error_type(404);
		c->hlen = strlen(c->head);
		c->hoff = 0;
		return 0;
	}
	req = ptr;
	ptr = strtok(NULL, " \t\r\n");
	if (ptr && strstr(ptr, "HTTP/1."))
		enhanced = 1;
	IMS = NULL;
	if (enhanced) {
		while ((param = strtok(NULL, "\t\r\n"))) {
			if (!strncasecmp(param, "If-Modified-Since:", 18)) {
				IMS = param;
				break;
			}
		}
	}
	while (*req == '/')
		req++;
	board = strtok(req, "/");
	if (!board || testboard(board) < 0) {
		c->state = SENDING_ERROR_HEADER;
		c->head = get_error_type(404);
		c->hlen = strlen(c->head);
		c->hoff = 0;
		return 0;
	}
	filename = strtok(NULL, "/");
	if (!filename
			|| (strncmp(filename, "M.", 2) && strncmp(filename, "G.", 2))
			|| strlen(filename) > 15 || !findarticle(board, filename)) {
		c->state = SENDING_ERROR_HEADER;
		c->head = get_error_type(404);
		c->hlen = strlen(c->head);
		c->hoff = 0;
		return 0;
	}
	sprintf(filepath, "boards/%s/%s", board, filename);
	if (IMS && file_time(filepath) > 0) {
		if (write_cache_header(c, atoi(filename + 2), 86400, IMS) == 1)
			return 0;
	}
	if (mmapfile(filepath, &c->mf) < 0) {
		c->state = SENDING_ERROR_HEADER;
		c->head = get_error_type(404);
		c->hlen = strlen(c->head);
		c->hoff = 0;
		return 0;
	}
	posstr = strtok(NULL, "/");
	if (!posstr) {
		c->state = SENDING_ERROR_HEADER;
		c->head = get_error_type(404);
		c->hlen = strlen(c->head);
		c->hoff = 0;
		return 0;
	}
	c->pos = atoi(posstr);
	if (c->pos < 1 || c->pos >= c->mf.size - 4 || c->mf.ptr[c->pos - 1] != 0) {
		c->state = SENDING_ERROR_HEADER;
		c->head = get_error_type(404);
		c->hlen = strlen(c->head);
		c->hoff = 0;
		return 0;
	}
	c->size = ntohl(*(unsigned int *) (c->mf.ptr + c->pos));
	if (c->pos + 4 + c->size >= c->mf.size) {
		c->state = SENDING_ERROR_HEADER;
		c->head = get_error_type(404);
		c->hlen = strlen(c->head);
		c->hoff = 0;
		return 0;
	}
	attachname = strtok(NULL, "/");
	if (!attachname) {
		c->state = SENDING_ERROR_HEADER;
		c->head = get_error_type(404);
		c->hlen = strlen(c->head);
		c->hoff = 0;
		return 0;
	}
	if (enhanced) {
		write_cache_header(c, atoi(filename + 2), 86400, NULL);
		c->state = SENDING_HEADER;
		c->head = get_mime_type(attachname);
		c->hlen = strlen(c->head);
		c->hoff = 0;
	}
	else
		c->state = SENDING_DATA;
	return 0;
}

	/* sendhdr - send all/more of http header/content-type */

static int
sendhdr(conn_t * c)
{
	int bc;

	while ((bc = l_write(c->sd, c->head + c->hoff, c->hlen - c->hoff)) == -1) if (errno != EINTR) {
		if (errno == EWOULDBLOCK)
			return 0;
		return -1;
	}
	c->hoff += bc;
	if (c->hoff >= c->hlen)
		c->state = SENDING_CACHE_HEADER;

	return 0;
}

static int
sendcachehdr_304(conn_t * c)
{
	int bc;

	while ((bc = l_write(c->sd, c->cache_header + c->choff, c->chlen - c->choff)) == -1) {
		if (errno != EINTR) {
			if (errno == EWOULDBLOCK)
				return 0;
			free(c->cache_header);
			return -1;
		}
	}
	c->choff += bc;
	if (c->choff >= c->chlen) {
		free(c->cache_header);
		return -1;
	}
	return 0;
}

static int
senderrorhdr(conn_t * c)
{
	int bc;
	while ((bc = l_write(c->sd, c->head + c->hoff, c->hlen - c->hoff)) == -1) {
		if (errno != EINTR) {
			if (errno == EWOULDBLOCK)
				return 0;
			return -1;
		}
	}
	c->hoff += bc;
	if (c->hoff >= c->hlen) {
		return -1;
	}
	return 0;
}

static int
sendcachehdr(conn_t * c)
{
	int bc;
	while ((bc = l_write(c->sd, c->cache_header + c->choff, c->chlen - c->choff)) == -1) {
		if (errno != EINTR) {
			if (errno == EWOULDBLOCK)
				return 0;
			free(c->cache_header);
			return -1;
		}
	}
	c->choff += bc;
	if (c->choff >= c->chlen) {
		free(c->cache_header);
		c->state = SENDING_DATA;
	}
	return 0;
}

static int
senddata(conn_t * c)
{
	int bs, sz = BLOCK_SIZE;
	if (sz > c->size - c->wsize)
		sz = c->size - c->wsize;
	if (sz == 0)
		return -1;
	while ((bs = l_write(c->sd, c->mf.ptr + c->pos + 4 + c->wsize, sz)) == -1) {
		if (errno == EWOULDBLOCK)
			return 0;
		return -1;
	}
	c->wsize += bs;
	return 0;
}

void
conn_upkeep(fd_set * rfs, fd_set * wfs)
{
	int i, n, nconn0 = num_connections;
	conn_t *v;
	now_t = time(NULL);

	for (i = n = 0; i < MAX_CONNECTIONS && n < nconn0; i++) {
		if (vector[i] == NULL)
			continue;
		n++;

		v = vector[i];
		switch (v->state) {
		case AWAITING_REQUEST:
			if (FD_ISSET(v->sd, rfs)) {
				v->lasttime = now_t;
				if (getreq(v) == -1) {
					vector[i] = rmconn(v);
					continue;
				}
			}
			break;
		case SENDING_HEADER:
			if (FD_ISSET(v->sd, wfs)) {
				v->lasttime = now_t;
				if (sendhdr(v) == -1) {
					vector[i] = rmconn(v);
					continue;
				}
			}
			break;
		case SENDING_DATA:
			if (FD_ISSET(v->sd, wfs)) {
				v->lasttime = now_t;
				if (senddata(v) != 0) {
					vector[i] = rmconn(v);
					continue;
				}
			}
			break;
		case SENDING_CACHE_HEADER_304:
			if (FD_ISSET(v->sd, wfs)) {
				v->lasttime = now_t;
				if (sendcachehdr_304(v) == -1) {
					vector[i] = rmconn(v);
					continue;
				}
			}
			break;
		case SENDING_CACHE_HEADER:
			if (FD_ISSET(v->sd, wfs)) {
				v->lasttime = now_t;
				if (sendcachehdr(v) == -1) {
					vector[i] = rmconn(v);
					continue;
				}
			}
			break;
		case SENDING_ERROR_HEADER:
			if (FD_ISSET(v->sd, wfs)) {
				v->lasttime = now_t;
				if (senderrorhdr(v) == -1) {
					vector[i] = rmconn(v);
					continue;
				}
			}
			break;
		}
	}
}

int
conn_cullselect(fd_set * rfs, fd_set * wfs)
{
	int maxfd = -1;
	int i, n, nconn0;
	conn_t *v;

	now_t = time(NULL);
	nconn0 = num_connections;
	for (i = n = 0; i < MAX_CONNECTIONS && n < nconn0; i++) {
		if (vector[i] == NULL)
			continue;
		n++;
		v = vector[i];
		if (now_t - vector[i]->lasttime > 30) {
			vector[i] = rmconn(v);
			continue;
		}
		v = vector[i];
		switch (v->state) {
		case AWAITING_REQUEST:
			if (maxfd < v->sd)
				maxfd = v->sd;
			FD_SET(v->sd, rfs);
			break;
		case SENDING_DATA:
		case SENDING_HEADER:
		case SENDING_CACHE_HEADER:
		case SENDING_CACHE_HEADER_304:
		case SENDING_ERROR_HEADER:
			if (maxfd < v->sd)
				maxfd = v->sd;
			FD_SET(v->sd, wfs);
			break;
		}
	}
	return maxfd;
}
