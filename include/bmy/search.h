#ifndef BMY_SEARCH_H
#define BMY_SEARCH_H
#include <stddef.h>
#include "bmy/article.h"

/**
 * @brief 版面文章搜索
 * @param board 版面名称，实现内部不做版面是否存在的校验
 * @param whattosearch 搜索使用的关键字
 * @param search_size 返回搜索结果的数量
 * @return fileheader_utf 数组，标题采用 utf8 编码，使用完成需调用 free(3)
 */
struct fileheader_utf *bmy_search_board(const char *board, const char *whattosearch, size_t *search_size);
#endif

