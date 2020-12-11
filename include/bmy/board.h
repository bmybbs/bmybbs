/**
 * 本代码是订阅功能的一部分，用于将必要数据从 ythtbbs 系统同步到数据库中。
 */
#ifndef BMY_BOARD_H
#define BMY_BOARD_H

/**
 * 在数据库中创建版面，并创建对应版面视图
 * 调用存储过程 procedure_insert_board
 * @param boardnum 版面编号，从 1 开始
 * @param name_en 版面英文名
 * @param name_zh_gbk 版面中文名，gbk 编码
 * @param secstr 分区
 */
void bmy_board_create(int boardnum, char *name_en, char *name_zh_gbk, char *secstr);

/**
 * 更新版面英文名、中文名以及分区信息
 * 调用存储过程 procedure_update_board
 * @param boardnum 版面编号，从 1 开始
 * @param name_en 版面英文名
 * @param name_zh_gbk 版面中文名，gbk 编码
 * @param secstr 分区
 */
void bmy_board_rename(int boardnun, char *name_en, char *name_zh_gbk, char *secstr);

#endif

