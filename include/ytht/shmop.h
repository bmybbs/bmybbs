#ifndef BMYBBS_SHMOP_H
#define BMYBBS_SHMOP_H
void *try_get_shm(int key, size_t size, int flag);
void *get_shm(int key, int size);

#define get_old_shm(x,y) try_get_shm(x,y,0)
#endif //BMYBBS_SHMOP_H
