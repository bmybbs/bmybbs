#ifndef __MGREP_H
#define __MGREP_H

#define MAXPAT		256
#define MAXLINE		1024
#define MAXSYM		256
#define MAXMEMBER1	4096
#define MAXPATFILE	2600		/*pattern文件的最大长度 */
#define BLOCKSIZE	8192		/*用于预读的数据大小 */
#define MAXHASH		512			/*pattern使用的hash表大小 */
#define mm			511			/*用于hash值的取模运算 */
#define max_num		200			/*最大的pattern个数 */
#define W_DELIM		128
#define L_DELIM		10

struct pat_list {
	int index;
	int next;
/*    struct pat_list *next;*/
};
struct pattern_image {
	int LONG;
	int SHORT;
	size_t p_size;
	unsigned char SHIFT1[MAXMEMBER1];
	unsigned char tr[MAXSYM];
	unsigned char tr1[MAXSYM];
	unsigned int HASH[MAXHASH];
	unsigned char buf[MAXPATFILE + BLOCKSIZE];
	unsigned char pat_spool[MAXPATFILE + 2 * max_num + MAXPAT];
	unsigned long patt[max_num];	/*用于指向pat_spool的偏移 */
	unsigned char pat_len[max_num];
	struct pat_list hashtable[max_num];
};

int ytht_mgrep_releasepf(struct pattern_image *patt_img);
int ytht_mgrep_prepf(int fp, struct pattern_image **ppatt_img, size_t *patt_image_len);
int ytht_mgrep_mgrep_str(char *text, int num, struct pattern_image *patt_img);


#endif
