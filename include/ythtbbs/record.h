/* record.c */
#ifndef __RECORD_H
#define __RECORD_H
#include <stddef.h>
#include <stdarg.h>

/**
 * 处理记录的回调函数，以可变参数列表的形式增加适用范围
 */
typedef int (*ythtbbs_record_callback_v)(void *, va_list);

void tmpfilename(char *filename, char *tmpfile, char *deleted);
int safewrite(int fd, void *buf, int size);
int delete_record(char *filename, int size, int id);
int append_record(const char *filename, const void *record, const size_t size);
int new_apply_record(char *filename, int size, int (*fptr) (void *, void *), void *farg);
int new_search_record(char *filename, void *rptr, int size, int (*fptr) (void *, void *), void *farg);
int search_record(char *filename, void *rptr, int size, int (*fptr) (void *, void *), void *farg);
int delete_file(char *filename, int size, int ent, int (*filecheck) (void *));

int get_record(char *filename, void *rptr, int size, int id);

/**
 * 移植自 src/bbs/record.c 以及 local_utl/common/record.c
 * @param filename
 * @param rptr
 * @param size
 * @param id
 * @return
 */
int substitute_record(char *filename, void *rptr, int size, int id);

/**
 * @brief 应用记录
 * 对应于 new_apply_record 以及 src/bbs/record.c::apply_record 的可变参数版本。
 * 内部缓冲使用批量读取的形式。
 * 若读取的记录长度不是 size 的整数倍时，不调用 fptr 直接返回。
 * @param filename
 * @param fptr
 * @param size
 */
int ythtbbs_record_apply_v(char *filename, ythtbbs_record_callback_v fptr, size_t size, ...);

/**
 * @brief 获取记录的条数
 * 参考了 src/bbs/record.c get_num_records() 的实现，但是不兼容，
 * 在 get_num_records() 中额外处理了 ".TOPFILE" 的条数，在后续重
 * 构中调用两次接口。
 */
long ythtbbs_record_count_records(const char *filename, const size_t size);
#endif
