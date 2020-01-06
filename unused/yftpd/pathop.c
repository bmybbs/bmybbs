#include <sys/mman.h>
#include "../include/bbs.h"
#include "login.h"
#include "logging.h"
#include "options.h"
#include "string.h"
#include "pathop.h"

struct quota *quota, *quotaf;

char partpath[PATH_MAX];

struct boardmem *
search_board(char *board)
{
	int i;
	static struct boardmem b;
	static struct BCACHE *shm_bcache = NULL;
	if (shm_bcache == NULL)
		shm_bcache =
		    (struct BCACHE *) get_old_shm(BCACHE_SHMKEY,
						  sizeof (struct BCACHE));
	if (shm_bcache == NULL)
		return NULL;
	for (i = 0; i < shm_bcache->number && i < MAXBOARD; i++) {
		if (!strcmp(shm_bcache->bcache[i].header.filename, board)) {
			b = shm_bcache->bcache[i];
			return &b;
		}
	}
	return NULL;
}

int
has_readperm()
{
	char *ptr, board[30];
	struct boardmem *bptr;
	if (HAS_PERM(PERM_SYSOP))
		return 1;
	//if it is not concerning any board, it is readable for everyone.
	if (strncmp(partpath, "/boards/", 8))
		return 1;
	if ((ptr = strchr(partpath + 8, '/')) == NULL) {
		if (strlen(partpath + 8) >= sizeof (board))
			return 0;
		strcpy(board, partpath + 8);
	} else {
		if (ptr - (partpath + 8) >= sizeof (board))
			return 0;
		strncpy(board, partpath + 8, ptr - (partpath + 8));
		board[ptr - (partpath + 8)] = 0;
	}
	if ((bptr = search_board(board)) == NULL)
		return 0;
	if (!hasreadperm
	    (bptr->header.clubnum, bptr->header.flag,
	     bptr->header.level)) return 0;
	return 1;
}

int
has_writeperm()
{
	char *ptr, board[30];
	struct boardmem *bptr;
	if (HAS_PERM(PERM_SYSOP))
		return 1;
	if (!HAS_PERM(PERM_BOARDS) || strncmp(partpath, "/boards/", 8)
	    || (ptr = strchr(partpath + 8, '/')) == NULL)
		return 0;
	if (ptr - (partpath + 8) >= sizeof (board))
		return 0;
	strncpy(board, partpath + 8, ptr - (partpath + 8));
	board[ptr - (partpath + 8)] = 0;
	if ((bptr = search_board(board)) == NULL)
		return 0;
	if (!hasreadperm
	    (bptr->header.clubnum, bptr->header.flag,
	     bptr->header.level)) return 0;
	if (HAS_PERM(PERM_BLEVELS))
		return 1;
	if (!HAS_PERM(PERM_BOARDS))
		return 0;
	if (!chk_BM(&currentuser, &(bptr->header), 0))
		return 0;
	return 1;
}

int
is_incoming()
{
	return (!isanonymous && has_readperm()
		&& strstr(partpath, "/incoming/") != NULL);
}

char *
contractpath(char *path)
{
	char lpath[PATH_MAX], *retv, *p2, *ptr = path;
	lpath[0] = 0;
	while ((retv = strsep(&ptr, "/")) != NULL) {
		if (!strcmp(retv, "..")) {
			p2 = strrchr(lpath, '/');
			if (p2 == NULL)
				lpath[0] = 0;
			else
				*p2 = 0;
		} else if (strcmp(retv, ".") && retv[0]) {
			strcat(lpath, "/");
			strcat(lpath, retv);
		}
	}
	if (lpath[0] != '/')
		strcpy(path, "/");
	strcat(path, lpath);
	return path;
}

char *
constructpath(const char *fname)
{
	static char fullpath[PATH_MAX];
	char cwd[PATH_MAX];
	int len = strlen(MY_FTP_ROOT);

	//construct a relative path, beginning with '/'
	if (*fname == '/') {
		if (strlen(fname) >= sizeof (partpath))
			return NULL;
		strcpy(partpath, fname);
	} else {
		if (getcwd(cwd, sizeof (cwd)) == NULL)
			return NULL;
		if (strlen(cwd) + strlen(fname) + 1 >= sizeof (partpath))
			return NULL;
		if (strncmp(cwd, MY_FTP_ROOT, len))
			exit(0);	// Should not happen.
		partpath[0] = 0;
		switch (cwd[len]) {
		case 0:
			break;
		case '/':
			if (cwd[len] != '/')
				strcat(partpath, "/");
			strcat(partpath, cwd + len);
			break;
		default:
			exit(0);	//Should not happen.
		}
		strcat(partpath, "/");
		strcat(partpath, fname);
	}
	contractpath(partpath);
	if (strlen(MY_FTP_ROOT) + strlen(partpath) >= sizeof (fullpath))
		return NULL;
	strcpy(fullpath, MY_FTP_ROOT);
	strcat(fullpath, partpath);
	//yftpd_log("construct path: \n\t%s\n\t%s\n\t%s\n\t%s\n", cwd, fname,
	//        partpath, fullpath);
	return fullpath;
}

int
my_open(const char *filename, int flages, mode_t mode)
{
	char *fullpath = constructpath(filename);
	if (!fullpath)
		return -1;
	if (flages & O_WRONLY || flages & O_RDWR) {
		if (!has_writeperm()) {
			if (!is_incoming())
				return -1;
			flages |= O_EXCL;
		}
	} else if (!has_readperm())
		return -1;
	return open(fullpath, flages, mode);
}

int
my_stat(const char *file_name, struct stat *buf)
{
	char *fullpath = constructpath(file_name);
	if (!fullpath)
		return -1;
	if (!has_readperm())
		return -1;
	return stat(fullpath, buf);
}

int
my_lstat(const char *file_name, struct stat *buf)
{
	char *fullpath = constructpath(file_name);
	if (!fullpath)
		return -1;
	if (!has_readperm())
		return -1;
	return lstat(fullpath, buf);
}

FILE *
my_fopen(const char *path, const char *mode)
{
	char *fullpath = constructpath(path);
	if (!fullpath)
		return NULL;
	if (!strcmp(mode, "r") || !strcmp(mode, "rb")) {
		if (!has_readperm())
			return NULL;
	} else if (!has_writeperm())
		return NULL;
	return fopen(fullpath, mode);
}

int
my_rename(const char *oldpath, const char *newpath)
{
	char *fullpath;
	char fulloldpath[PATH_MAX];
	if (!(fullpath = constructpath(oldpath)))
		return -1;
	if (!has_writeperm())
		return -1;
	strcpy(fulloldpath, fullpath);
	if (!(fullpath = constructpath(newpath)))
		return -1;
	if (!has_writeperm())
		return -1;
	return rename(fulloldpath, fullpath);
}

DIR *
my_opendir(const char *name)
{
	char *fullpath = constructpath(name);
	if (!fullpath)
		return NULL;
	if (!has_readperm())
		return NULL;
	return opendir(fullpath);
}

int
my_chdir(const char *path)
{
	char *fullpath = constructpath(path);
	if (!fullpath)
		return -1;
	if (!has_readperm())
		return -1;
	//yftpd_log("chdir %s\n", fullpath);
	return chdir(fullpath);
}

char *
my_getcwd(char *buf, size_t size)
{
	char *retv;
	if (!(retv = getcwd(buf, size)))
		return NULL;
	if (!strcmp(buf, MY_FTP_ROOT)) {
		strcpy(buf, "/");
		return buf;
	}
	memmove(buf, buf + strlen(MY_FTP_ROOT),
		strlen(buf) - strlen(MY_FTP_ROOT) + 1);
	return buf;
}

int
my_unlink(const char *pathname)
{
	char *fullpath;
	struct stat buf;
	int quota_type = my_quota_type(pathname);
	if (quota_type < 0)
		return -1;
	fullpath = constructpath(pathname);
	if (!fullpath)
		return -1;
	if (!has_writeperm())
		return -1;
	if (lstat(fullpath, &buf) < 0)
		return -1;
	if (unlink(fullpath) < 0)
		return -1;
	quota->q[quota_type] += buf.st_size;
	quota->changed++;
	return 0;
}

int
my_mkdir(const char *pathname, mode_t mode)
{
	char *fullpath = constructpath(pathname);
	if (!fullpath)
		return -1;
	if (!has_writeperm() && !is_incoming())
		return -1;
	return mkdir(fullpath, mode);
}

int
my_rmdir(const char *pathname)
{
	char *fullpath = constructpath(pathname);
	if (!fullpath)
		return -1;
	if (!has_writeperm())
		return -1;
	return rmdir(fullpath);
}

int
my_chmod(const char *path, mode_t mode)
{
	char *fullpath = constructpath(path);
	if (!fullpath)
		return -1;
	if (!has_writeperm())
		return -1;
	return chmod(fullpath, mode);
}

int
my_quota_init()
{
	int i, fd;
	struct stat st;
	if ((fd = open(PATH_YFTPQUOTA, O_RDWR | O_CREAT, 0600)) < 0)
		return -1;
	if (fstat(fd, &st) < 0) {
		close(fd);
		return -1;
	}
	if (st.st_size < sizeof (struct quota) * NUM_QUOTA_TYPE) {
		char c = 0;
		lseek(fd, sizeof (struct quota), SEEK_SET);
		write(fd, &c, 1);
	}
	quotaf =
	    mmap(NULL, sizeof (struct quota), PROT_READ | PROT_WRITE,
		 MAP_SHARED, fd, 0);
	close(fd);
	if (quotaf == NULL)
		return -1;
	if (quotaf->hasinit != 1) {
		for (i = 0; i < NUM_QUOTA_TYPE; i++) {
			quotaf->q[i] = 0;
			strcpy(quotaf->d[i], "Î´Ö¸¶¨");
		}
		quotaf->q[0] = 600 * 1024 * 1024;
		strcpy(quotaf->d[0], "ÏµÍ³Ê£Óà´ÅÅÌÅä¶î");
		quotaf->q[1] = 600 * 1024 * 1024;
		strcpy(quotaf->d[1], "°æÃæÊ£Óà´ÅÅÌÅä¶î");
		quotaf->hasinit = 1;
	}
	quotaf->changed = 0;
	i = shmget(YFTP_SHMKEY, sizeof (*quota), IPC_CREAT | S_IRWXU);
	if (i < 0)
		return -1;
	quota = shmat(i, NULL, 0);
	if (quota == NULL)
		return -1;
	memcpy(quota, quotaf, sizeof (struct quota));
	return 0;
}

int
my_quota_type(const char *filename)
{
	char *ptr, *fullpath = constructpath(filename);
	if (!fullpath)
		return -1;
	if (!strncmp(partpath, "/boards/", 8)
	    || (ptr = strchr(partpath + 8, '/')) != NULL)
		return 1;
	return 0;
}
