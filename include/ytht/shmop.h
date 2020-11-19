#ifndef BMYBBS_SHMOP_H
#define BMYBBS_SHMOP_H
#include <stddef.h>
#include <sys/types.h>

/**
 * @brief 基于 shmget(2) 和 shmat(2) 的封装
 * @param key
 * @param size
 * @param flag
 * @return
 *   0      - 加载失败，可能是已创建
 *   其他值 - 位于内存中的地址
 */
void *try_get_shm(key_t key, size_t size, int flag);

/**
 * @brief 创建或者加载共享存储
 *
 * 属于 try_get_shm 的调用，首先创建，若创建失败则意味着
 * 共享存储已存在，再次调用 try_get_shm 加载到进程内。
 *
 * 本操作是原子级的，但是后续访问共享内存空间需要考虑锁。
 * @param key  键
 * @param size 大小
 * @return     地址指针
 */
void *get_shm(key_t key, size_t size);

#define get_old_shm(x,y) try_get_shm(x,y,0)
#endif //BMYBBS_SHMOP_H
