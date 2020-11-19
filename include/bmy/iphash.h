#ifndef BMYBBS_IPHASH_H
#define BMYBBS_IPHASH_H

/**
 * 依据 IP 计算散列值
 *
 * 函数假定 IP 要么为 IPv4（以 '.' 分隔），要么 IPv6（以 ':' 分隔），IPv4 映射到 IPv6 的
 * 归为后者（理论上计算应该兼容）。对于前者，调用在 nju09 中被注释掉的原先的 hash 算法，
 * 即整个 IPv4 地址作为 32 进制数进行模运算，而后者则迭代求余。
 * @param ip     [char *] IP 字符串
 * @param nhash  [uint32] 模
 * @return
 */
unsigned int bmy_iphash(const char *ip, unsigned int nhash);
#endif //BMYBBS_IPHASH_H
