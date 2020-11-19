#ifndef BMYBBS_NUMBYTE_H
#define BMYBBS_NUMBYTE_H

/**
 * @brief 将数字转化为字符
 * 使用一个字符的大小粗略表示数值大小
 * @param n 数值
 * @return 0~255 (内部数组索引值)
 */
unsigned char ytht_num2byte(int n);

/**
 * @brief 将字符转化为数值
 * @param c 字符(内部数组索引值)
 * @return 近似的数值
 */
int ytht_byte2num(unsigned char c);
#endif //BMYBBS_NUMBYTE_H
