#include "ythtlib.h"
#include <sys/ipc.h>
#include <sys/shm.h>

void *
try_get_shm(int key, int size, int flag)
{
	int id;
	void *ptr;
	id = shmget(key, size, flag);
	if (id < 0)
		return 0;
	ptr = shmat(id, NULL, 0);
	if (ptr == (void *) -1)
		return 0;
	else
		return ptr;
}

void *
get_shm(int key, int size)
{
	void *ptr;
	ptr = try_get_shm(key, size, IPC_CREAT | IPC_EXCL | 0600);
	if (ptr != NULL)
		memset(ptr, 0, size);
	else
		ptr = try_get_shm(key, size, 0);
	return ptr;
}
