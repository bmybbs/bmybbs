#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "ythtlib.h"

int
make_named_socket(char *filename)
{
	struct sockaddr_un name;
	int sock;
	size_t size;

	sock = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (sock < 0)
		return -1;
	name.sun_family = AF_FILE;
	strsncpy(name.sun_path, filename, sizeof (name.sun_path));
	size = (offsetof(struct sockaddr_un, sun_path)
		+ strlen(name.sun_path) + 1);

	if (bind(sock, (struct sockaddr *) &name, size) < 0) {
		close(sock);
		return -1;
	}
	return sock;
}

int
connect_named_socket(char *filename)
{
	struct sockaddr_un name;
	int sock;
	size_t size;

	sock = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (sock < 0)
		return -1;
	name.sun_family = AF_FILE;
	strsncpy(name.sun_path, filename, sizeof (name.sun_path));
	size = (offsetof(struct sockaddr_un, sun_path)
		+ strlen(name.sun_path) + 1);
	if ((connect(sock, (struct sockaddr *) &name, size))) {
		close(sock);
		return -1;
	}
	return sock;
}
