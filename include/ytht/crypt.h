/* crypt.c */
#ifndef __CRYPT_H
#define __CRYPT_H

/**
 * @brief 创建密码
 * @warning 多线程不安全
 * @param pw 明文密码
 * @return 密文密码
 */
char *ytht_crypt_genpasswd(char *pw);

/**
 * @brief 实际的加密函数
 * 在 ytht_crypt 中检查了是否存在其他加密函数，本函数是缺省项，也是当前实际使用的加密函数。
 * @warning 多线程不安全
 * @param buf 密码缓冲区
 * @param salt 盐
 * @return 密文密码
 */
char *ytht_crypt_crypt1(const char *buf, const char *salt);

/**
 * @brief 检查密码
 * 多线程安全
 * @param pw_crypted 密码明文
 * @param pw_try     密码密文
 * @return
 *     1             通过
 *     0             错误
 */
int ytht_crypt_checkpasswd(const char *pw_crypted, const char *pw_try);
#endif
