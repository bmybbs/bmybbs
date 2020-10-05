#include "ytht/shmop.h"
#include "ythtbbs/cache.h"
#include "cache-internal.h"

static struct UTMPFILE *shm_utmp;

void ythtbbs_cache_resolve_utmp() {
	if (shm_utmp == NULL) {
		shm_utmp = get_shm(UTMP_SHMKEY, sizeof(*shm_utmp));
		if (shm_utmp == NULL) {
			shm_err(UTMP_SHMKEY);
		}
	}
}

