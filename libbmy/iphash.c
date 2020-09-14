#include <arpa/inet.h>
#include <string.h>

static unsigned int iphash_v4(const char *ipv4, unsigned int nhash) {
	struct in_addr addr;
	unsigned int result = 0;

	if (inet_aton(ipv4, &addr) == 1) {
		result = (unsigned int)addr.s_addr % nhash;
	}

	return result;
}

static unsigned int iphash_v6(const char *ipv6, unsigned int nhash) {
	struct in6_addr addr;
	unsigned int result = 0, i;
	if (inet_pton(AF_INET6, ipv6, &addr) == 1) {
		for (i=0; i<16; i++){
			result = ((result << 8) + addr.s6_addr[i]) % nhash;
		}
	}
	return result;
}

unsigned int bmy_iphash(const char *ip, unsigned int nhash) {
	return (strchr(ip, ':') != NULL) ? iphash_v6(ip, nhash) : iphash_v4(ip, nhash);
}