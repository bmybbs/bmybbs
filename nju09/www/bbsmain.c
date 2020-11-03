#include "bbslib.h"
int looponce = 0;

#include "njuapi.h"

/* BBSLIB.c */
extern int cookie_parse();
extern int url_parse();
extern int http_parm_init();

struct cgi_applet applets[] = {
//      {bbsusr_main, {"bbsusr", NULL}},
	{bbsrss_main, {"bbsrss", "rss", NULL}, 0L, 0L, 0},
	{bbstop10_main, {"bbstop10", NULL}, 0L, 0L, 0},
	{bbsdoc_main, {"bbsdoc", "doc", NULL}, 0L, 0L, 0},
	{bbscon_main, {"bbscon", "con", NULL}, 0L, 0L, 0},
	{bbsbrdadd_main, {"bbsbrdadd", "brdadd", NULL}, 0L, 0L, 0},
	{bbsboa_main, {"bbsboa", "boa", NULL}, 0L, 0L, 0},
	{bbsall_main, {"bbsall", NULL}, 0L, 0L, 0},
	{bbsanc_main, {"bbsanc", "anc", NULL}, 0L, 0L, 0},
	{bbs0an_main, {"bbs0an", "0an", NULL}, 0L, 0L, 0},
	{bbslogout_main, {"bbslogout", NULL}, 0L, 0L, 0},
	{bbsleft_main, {"bbsleft", NULL}, 0L, 0L, 0},
	{bbslogin_main, {"bbslogin", NULL}, 0L, 0L, 0},
	{bbsbadlogins_main, {"bbsbadlogins", NULL}, 0L, 0L, 0},
	{bbsqry_main, {"bbsqry", "qry", NULL}, 0L, 0L, 0},
	{bbsnot_main, {"bbsnot", "not", NULL}, 0L, 0L, 0},
	{bbsfind_main, {"bbsfind", NULL}, 0L, 0L, 0},
	{bbsfadd_main, {"bbsfadd", NULL}, 0L, 0L, 0},
	{bbsfdel_main, {"bbsfdel", NULL}, 0L, 0L, 0},
	{bbsfall_main, {"bbsfall", NULL}, 0L, 0L, 0},
	{bbsbadd_main, {"bbsbadd", NULL}, 0L, 0L, 0},	// 添加黑名单
	{bbsbdel_main, {"bbsbdel", NULL}, 0L, 0L, 0},	// 删除黑名单
	{bbsball_main, {"bbsball", NULL}, 0L, 0L, 0},	// 查看所有黑名单
	{bbsfriend_main, {"bbsfriend", NULL}, 0L, 0L, 0},
	{bbsfoot_main, {"bbsfoot", NULL}, 0L, 0L, 0},
	{bbsform_main, {"bbsform", NULL}, 0L, 0L, 0},
	{bbspwd_main, {"bbspwd", NULL}, 0L, 0L, 0},
	{bbsplan_main, {"bbsplan", NULL}, 0L, 0L, 0},
	{bbsinfo_main, {"bbsinfo", NULL}, 0L, 0L, 0},
	{bbsmybrd_main, {"bbsmybrd", NULL}, 0L, 0L, 0},
	{bbssig_main, {"bbssig", NULL}, 0L, 0L, 0},
	{bbspst_main, {"bbspst", "pst", NULL}, 0L, 0L, 0},
	{bbsgcon_main, {"bbsgcon", "gcon", NULL}, 0L, 0L, 0},
	{bbsgdoc_main, {"bbsgdoc", "gdoc", NULL}, 0L, 0L, 0},
	{bbsmmdoc_main, {"bbsmmdoc", "mmdoc", NULL}, 0L, 0L, 0},	//add by macintosh 050516
	{bbsdel_main, {"bbsdel", "del", NULL}, 0L, 0L, 0},
	{bbsdelmail_main, {"bbsdelmail", NULL}, 0L, 0L, 0},
	{bbsmailcon_main, {"bbsmailcon", NULL}, 0L, 0L, 0},
	{bbsmail_main, {"bbsmail", "mail", NULL}, 0L, 0L, 0},
	{bbsdelmsg_main, {"bbsdelmsg", NULL}, 0L, 0L, 0},
	{bbssnd_main, {"bbssnd", NULL}, 0L, 0L, 0},
//      {bbsalluser_main, {"bbsalluser", NULL}},
	{bbsnotepad_main, {"bbsnotepad", NULL}, 0L, 0L, 0},
	{bbsmsg_main, {"bbsmsg", NULL}, 0L, 0L, 0},
	{bbssendmsg_main, {"bbssendmsg", NULL}, 0L, 0L, 0},
	{bbsreg_main, {"bbsreg", NULL}, 0L, 0L, 0},
	{bbsscanreg_main, {"bbsscanreg", NULL}, 0L, 0L, 0},
	{bbsmailmsg_main, {"bbsmailmsg", NULL}, 0L, 0L, 0},
	{bbssndmail_main, {"bbssndmail", NULL}, 0L, 0L, 0},
	{bbsnewmail_main, {"bbsnewmail", NULL}, 0L, 0L, 0},
	{bbspstmail_main, {"bbspstmail", "pstmail", NULL}, 0L, 0L, 0},
	{bbsgetmsg_main, {"bbsgetmsg", NULL}, 0L, 0L, 0},
	//{bbschat_main, {"bbschat", NULL}},
	{bbscloak_main, {"bbscloak", NULL}, 0L, 0L, 0},
	{bbsmdoc_main, {"bbsmdoc", "mdoc", NULL}, 0L, 0L, 0},
	{bbsnick_main, {"bbsnick", NULL}, 0L, 0L, 0},
	{bbstfind_main, {"bbstfind", "tfind", NULL}, 0L, 0L, 0},
	{bbsadl_main, {"bbsadl", NULL}, 0L, 0L, 0},
	{bbstcon_main, {"bbstcon", "tcon", NULL}, 0L, 0L, 0},
	{bbstdoc_main, {"bbstdoc", "tdoc", NULL}, 0L, 0L, 0},
	{bbsdoreg_main, {"bbsdoreg", NULL}, 0L, 0L, 0},
	{bbsmywww_main, {"bbsmywww", NULL}, 0L, 0L, 0},
	{bbsccc_main, {"bbsccc", "ccc", NULL}, 0L, 0L, 0},
	{bbsufind_main, {"bbsufind", NULL}, 0L, 0L, 0},
	{bbsclear_main, {"bbsclear", "clear", NULL}, 0L, 0L, 0},
	{bbsstat_main, {"bbsstat", NULL}, 0L, 0L, 0},
	{bbsedit_main, {"bbsedit", "edit", NULL}, 0L, 0L, 0},
	{bbsman_main, {"bbsman", "man", NULL}, 0L, 0L, 0},
	{bbsparm_main, {"bbsparm", NULL}, 0L, 0L, 0},
	{bbsfwd_main, {"bbsfwd", "fwd", NULL}, 0L, 0L, 0},
	{bbsmnote_main, {"bbsmnote", NULL}, 0L, 0L, 0},
	{bbsdenyall_main, {"bbsdenyall", NULL}, 0L, 0L, 0},
	{bbsdenydel_main, {"bbsdenydel", NULL}, 0L, 0L, 0},
	{bbsdenyadd_main, {"bbsdenyadd", NULL}, 0L, 0L, 0},
	{bbstopb10_main, {"bbstopb10", NULL}, 0L, 0L, 0},
	{bbsbfind_main, {"bbsbfind", "bfind", NULL}, 0L, 0L, 0},
	{bbsx_main, {"bbsx", NULL}, 0L, 0L, 0},
	{bbseva_main, {"bbseva", "eva", NULL}, 0L, 0L, 0},
	{bbsvote_main, {"bbsvote", "vote", NULL}, 0L, 0L, 0},
	{bbsshownav_main, {"bbsshownav", NULL}, 0L, 0L, 0},
	{bbsbkndoc_main, {"bbsbkndoc", "bkndoc", NULL}, 0L, 0L, 0},
	{bbsbknsel_main, {"bbsbknsel", "bknsel", NULL}, 0L, 0L, 0},
	{bbsbkncon_main, {"bbsbkncon", "bkncon", NULL}, 0L, 0L, 0},
	{bbshome_main, {"bbshome", "home", NULL}, 0L, 0L, 0},
	{bbsindex_main, {"bbsindex", NULL}, 0L, 0L, 0},
	{bbslform_main, {"bbslform", NULL}, 0L, 0L, 0},
	{bbst_main, {"bbst", NULL}, 0L, 0L, 0},
	{bbslt_main, {"bbslt", NULL}, 0L, 0L, 0},
	{bbsdt_main, {"bbsdt", NULL}, 0L, 0L, 0},
	{regreq_main, {"regreq", NULL}, 0L, 0L, 0},
	{bbsselstyle_main, {"bbsselstyle", NULL}, 0L, 0L, 0},
	{bbscon1_main, {"bbscon1", "c1", NULL}, 0L, 0L, 0},
	{bbsattach_main, {"attach", NULL}, 0L, 0L, 0},
	{bbskick_main, {"kick", NULL}, 0L, 0L, 0},
	{bbsshowfile_main, {"bbshowfile", "showfile", NULL}, 0L, 0L, 0},
	{bbsincon_main, {"boards", NULL}, 0L, 0L, 0},
	{bbssetscript_main, {"setscript",NULL}, 0L, 0L, 0},
	{bbsucss_main,{"bbsucss",NULL}, 0L, 0L, 0},
	{bbsdefcss_main,{"bbsdefcss",NULL}, 0L, 0L, 0},
	{bbscccmail_main, {"bbscccmail", NULL}, 0L, 0L, 0},
	{bbsfwdmail_main, {"bbsfwdmail", NULL}, 0L, 0L, 0},
	{bbssecfly_main, {"bbssecfly", NULL}, 0L, 0L, 0},
	{bbstmpl_main, {"bbstmpl", NULL}, 0L, 0L, 0},
	{bbssbs_main, {"bbssbs", NULL}, 0L, 0L, 0},
	{bbseditmail_main, {"bbseditmail", NULL}, 0L, 0L, 0},
	{apiqry_main, {"apiqry", NULL}, 0L, 0L, 0},
	{bbsresetpass_main, {"bbsresetpass", NULL}, 0L, 0L, 0},
	{bbsfindacc_main, {"bbsfindacc", NULL}, 0L, 0L, 0},
	{bbsnotify_main, {"bbsnotify", NULL}, 0L, 0L, 0},
	{bbsdelnotify_main, {"bbsdelnotify", NULL}, 0L, 0L, 0},
//	{bbschangestyle_main, {"bbschangestyle", "changestyle", NULL}},
	{NULL, {NULL}, 0L, 0L, 0}
};

#include <sys/times.h>
char *cginame = NULL;
static unsigned int rt = 0;

static void cgi_time(struct cgi_applet *a) {
	static struct rusage r0, r1;
	getrusage(RUSAGE_SELF, &r1);

	if (a == NULL) {
		r0 = r1;
		return;
	}
	a->count++;
	a->utime += 1000. * (r1.ru_utime.tv_sec - r0.ru_utime.tv_sec) + (r1.ru_utime.tv_usec - r0.ru_utime.tv_usec) / 1000.;
	a->stime += 1000. * (r1.ru_stime.tv_sec - r0.ru_stime.tv_sec) + (r1.ru_stime.tv_usec - r0.ru_stime.tv_usec) / 1000.;
	r0 = r1;
}

void wantquit(int signal) {
	if (incgiloop)
		incgiloop = 0;
	else
		exit(6);
}

struct cgi_applet *get_cgi_applet(char *needcgi) {
	struct cgi_applet *a;
	int i;
	a = applets;
	while (a->main != NULL) {
		for (i = 0; a->name[i] != NULL; i++)
			if (!strcmp(needcgi, a->name[i]))
				return a;
		a++;
	}
	return NULL;
}

int nologin = 1;


int main(int argc, char *argv[]) {
	struct cgi_applet *a = NULL;
	struct rlimit rl;
	int i;
	seteuid(BBSUID);
	setuid(BBSUID);
	setgid(BBSGID);
	cgi_time(NULL);
	rl.rlim_cur = 20 * 1024 * 1024;
	rl.rlim_max = 40 * 1024 * 1024;
	setrlimit(RLIMIT_CORE, &rl);
	thispid = getpid();
	now_t = time(NULL);
	srand(now_t * 2 + thispid);
	html_header(0);
	if (geteuid() != BBSUID)
		http_fatal("uid error.");
	chdir(BBSHOME);
	shm_init();
	if (ummap())
		http_fatal("mmap error.");
	signal(SIGTERM, wantquit);
	if (access("NOLOGIN", F_OK))
		nologin = 0;
	while (looponce-- >= 0) {
		cginame = NULL;
		incgiloop = 1;
		if (setjmp(cgi_start)) {
			cgi_time(a);
			if (!incgiloop || rt++ > 40000) {
				exit(2);
			}
			incgiloop = 0;
			continue;
		}
		html_header(0);
		now_t = time(NULL);
		ytht_strsncpy(fromhost, getsenv("REMOTE_ADDR"), BMY_IPV6_LEN); //ipv6 by leoncom
		fromhost[BMY_IPV6_LEN - 1] = '\0';
		inet_pton(PF_INET6,fromhost,&from_addr);

		cookie_parse();
		if (url_parse())
			http_fatal("%s 没有实现的功能!", getsenv("SCRIPT_URL"));
		http_parm_init();
		a = get_cgi_applet(needcgi);
		if (a != NULL) {
			cginame = a->name[0];
			(*(a->main)) ();
			cgi_time(a);
			if (!incgiloop) {
				exit(4);
			}
			incgiloop = 0;
			continue;
		}
		http_fatal("%s 没有实现的功能!", getsenv("SCRIPT_URL"));
		incgiloop = 0;
	}
	munmap(ummap_ptr, ummap_size);
	exit(5);
}

