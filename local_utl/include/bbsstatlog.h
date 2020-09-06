#define  BBSSTATELOGFILE MY_BBS_HOME"/reclog/bbsstat.log"
struct bbsstatlogitem {
        time_t time;
        float load[3];
        int naccount;
        int nonline;
        int ntelnet;
        int nwww;
        int n162105;
        int n162105telnet;
        int nwwwguest;
	int netflow;
        int nouse1[18];
};
