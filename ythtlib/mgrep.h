#ifndef __MGREP_H
#define __MGREP_H

#define MAXPAT  256
#define MAXLINE 1024
#define MAXSYM  256
#define MAXMEMBER1 4096
#define MAXPATFILE 2600		/*pattern�ļ�����󳤶� */
#define BLOCKSIZE  8192		/*����Ԥ�������ݴ�С */
#define MAXHASH    512		/*patternʹ�õ�hash���С */
#define mm 	   511		/*����hashֵ��ȡģ���� */
#define max_num    200		/*����pattern���� */
#define W_DELIM	   128
#define L_DELIM    10

struct pat_list {
	int index;
	int next;
/*    struct pat_list *next;*/
};
struct pattern_image {
	int LONG;
	int SHORT;
	int p_size;
	unsigned char SHIFT1[MAXMEMBER1];
	unsigned char tr[MAXSYM];
	unsigned char tr1[MAXSYM];
	unsigned int HASH[MAXHASH];
	unsigned char buf[MAXPATFILE + BLOCKSIZE];
	unsigned char pat_spool[MAXPATFILE + 2 * max_num + MAXPAT];
	unsigned long patt[max_num];	/*����ָ��pat_spool��ƫ�� */
	unsigned char pat_len[max_num];
	struct pat_list hashtable[max_num];
};

int releasepf(struct pattern_image *patt_img);
int prepf(int fp, struct pattern_image **ppatt_img, size_t *patt_image_len);
int mgrep_str(char *text, int num, struct pattern_image *patt_img);
int mgrep(int fd, struct pattern_image *patt_img);
void monkey1(register unsigned char *text, int start, int end, struct pattern_image *patt_img);
int m_short(unsigned char *text, int start, int end, struct pattern_image *patt_img);
void f_prep(int pat_index, unsigned char *Pattern, struct pattern_image *patt_img);

#endif
