// gcc -g -Wall -I include -I ythtlib -I libythtbbs trans_bh.c -o trans_bh
// read .BOARDS write .NEWBOARDS
#include "bbs.h"
struct boardheader nb;
struct oldboardheader ob;
/*
struct boardheader {		
	char filename[24];	
	char title[24];
	int clubnum;
	unsigned level;
	unsigned short flag;
	unsigned char secnumber;
	unsigned char type;
	struct bm bms[24];
	char unused[64];
};

struct oldboardheader {
	char filename[STRLEN];
	int stocknum;
	int score;
	int clubnum;
	int total;
	int lastpost;
	char BM[BM_LEN - 1];
	char flag;
	char title[STRLEN];
	unsigned level;
	short inboard;
	unsigned char unused[10];
};
*/
int
main()
{
	char sec[13][5] =
	    { "a", "b", "efg", "hij", "klm", "ns", "op", "qr", "xtu", "y", "w",
		"d",
		"c"
	};
	char secname[13] = "0123456789TYC";
	FILE *fp1, *fp2;
	int i, j, bnum = 0;
	char *ptr;
	time_t now;
	time(&now);
	printf("%d\n", sizeof (struct boardheader));
	printf("%d\n", sizeof (struct oldboardheader));
	fp1 = fopen(".BOARDS", "r");
	fp2 = fopen(".NEWBOARDS", "w");
	while (fread(&ob, sizeof (ob), 1, fp1) != 0) {
		bzero(&nb, sizeof (nb));
		if (!ob.filename[0])
			continue;
		strncpy(nb.filename, ob.filename, 24);
		ptr = strchr(ob.title, ']');
		if (ptr)
			strncpy(nb.title, ptr + 2, 24);
		nb.clubnum = ob.clubnum;
		nb.level = ob.level;
		nb.flag = ob.flag;
		for (i = 0; i < 13; i++) {
			if (strchr(sec[i], ob.title[0])) {
				nb.secnumber1 = secname[i];
				break;
			}
		}
		if (i == 13) {
			exit(-1);
		}
		if (ob.title[1] != '[') {
			for (i = 0; i < 13; i++) {
				if (strchr(sec[i], ob.title[1])) {
					nb.secnumber2 = secname[i];
					break;
				}
			}
		}
		if (i == 13) {
			exit(-1);
		}
		ptr = strchr(ob.title, '[');
		if (ptr)
			strncpy(nb.type, ptr + 1, 4);
		nb.type[4] = 0;
		bnum = 0;
		for (i = 0, j = 0; ob.BM[i] != '\0'; i++) {
			if (ob.BM[i] == ' ') {
				nb.bm[bnum][j] = 0;
				bnum++;
				j = 0;
			} else {
				nb.bm[bnum][j] = ob.BM[i];
				j++;
			}
		}
		for (i = 0; i <= bnum; i++) {
			nb.hiretime[i] = now;
		}
		fwrite(&nb, sizeof (nb), 1, fp2);
	}
	fclose(fp1);
	fclose(fp2);
	return 0;
}
