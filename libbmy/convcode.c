#include <string.h>
#include <stdlib.h>
#include <iconv.h>

int code_convert(const char *from_charset, const char *to_charset, const char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
	iconv_t cd;
	size_t rc;
	char *tmp = strdup(inbuf);
	char *tmp_p = tmp;

	if (tmp == NULL)
		return -1;

	cd = iconv_open(to_charset, from_charset);
	if (cd == 0) {
		free(tmp);
		return -1;
	}

	memset(outbuf, 0, outlen);
	rc = iconv(cd, &tmp_p, &inlen, &outbuf, &outlen);
	free(tmp);
	iconv_close(cd);

	return (rc == (size_t) -1) ? -1 : 0;
}

//UNICODE码转为GBK码
int u2g(const char *inbuf,size_t inlen,char *outbuf,size_t outlen) {
	return code_convert("utf-8","gbk",inbuf,inlen,outbuf,outlen);
}
//GBK码转为UNICODE码
int g2u(const char *inbuf,size_t inlen,char *outbuf,size_t outlen) {
	return code_convert("gbk","utf-8",inbuf,inlen,outbuf,outlen);
}

int is_utf_special_byte(unsigned char c){
	unsigned special_byte = 0X02;	//binary 00000010
	if(c>>6==special_byte)
		return 1;
	else
		return 0;
}

//判断是否为UNICODE编码
int is_utf(const char * inbuf, size_t inlen) {
	unsigned one_byte   = 0X00; 	//binary 00000000
	unsigned two_byte   = 0X06; 	//binary 00000110
	unsigned three_byte = 0X0E; 	//binary 00001110
	unsigned four_byte  = 0X1E; 	//binary 00011110
	unsigned five_byte  = 0X3E; 	//binary 00111110
	unsigned six_byte   = 0X7E; 	//binary 01111110

	unsigned int i;
	unsigned int c;

	unsigned char k = 0;
	unsigned char m = 0;
	unsigned char n = 0;
	unsigned char p = 0;
	unsigned char q = 0;

	for (i=0;i<inlen;){
		c=(unsigned char)inbuf[i];
		if(c>>7==one_byte){
			i++;
			continue;
		} else if(c>>5==two_byte){
			k = (unsigned char)inbuf[i+1];
			if(is_utf_special_byte(k)){
				return 1;
			}
		} else if(c>>4==three_byte){
			m = (unsigned char)inbuf[i+1];
			n = (unsigned char)inbuf[i+2];
			if(is_utf_special_byte(m) && is_utf_special_byte(n)){
				return 1;
			}
		} else if(c>>3==four_byte){
			k = (unsigned char)inbuf[i+1];
			m = (unsigned char)inbuf[i+2];
			n = (unsigned char)inbuf[i+3];
			if(is_utf_special_byte(k)
					&& is_utf_special_byte(m)
					&& is_utf_special_byte(n)){
				return 1;
			}
		} else if(c>>2 == five_byte){
			k = (unsigned char)inbuf[i+1];
			m = (unsigned char)inbuf[i+2];
			n = (unsigned char)inbuf[i+3];
			p = (unsigned char)inbuf[i+4];
			if(is_utf_special_byte(k)
					&& is_utf_special_byte(m)
					&& is_utf_special_byte(n)
					&& is_utf_special_byte(p)){
				return 1;
			}
		} else if(c>>1==six_byte){
			k = (unsigned char)inbuf[i+1];
			m = (unsigned char)inbuf[i+2];
			n = (unsigned char)inbuf[i+3];
			p = (unsigned char)inbuf[i+4];
			q = (unsigned char)inbuf[i+5];
			if ( is_utf_special_byte(k)
					&& is_utf_special_byte(m)
					&& is_utf_special_byte(n)
					&& is_utf_special_byte(p)
					&& is_utf_special_byte(q) ) {
				return 1;
			}
		}
	}
	return 0;
}

