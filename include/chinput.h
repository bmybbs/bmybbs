
struct HZS {
   char p[2];
   char h[120][2];
   short f[120]; //一个汉字，一个字频
};

struct WORDS {
   char p[100][2];
   char h[100][4];
   short f[100];  //两个韵母，两个汉字，一个词频
};

struct LWORDS {
   char p[100][6+6];
   char h[100][12];
   short f[100];
};

struct PINYINARRAY {
   short i[24]; //给s做的index, 按照声母
   struct HZS s[406];  //406个拼音
   struct WORDS w[24][24]; //23个声母外加一个零声母
   struct LWORDS l[100];  //24*24*24个声母组合映射到1～99 s1*s2*s3%100
};
