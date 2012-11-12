struct tshirt {
	char id[IDLEN + 1];
	short fm;
	short style;
	short price;
	short size;
	short color;
	int num;
	int dtime;
	char address[250];
	char nouse[16];
};
extern char *stylestr[];
extern char *fmstr[];
extern char *sizestr[];
extern char *pricestr[];
extern char *colorstr[];
	
