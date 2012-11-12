#include "bbslib.h"
#if defined(ENABLE_GHTHASH) && defined(ENABLE_FASTCGI)
#include <ght_hash_table.h>
#endif

#ifndef ENABLE_FASTCGI
int looponce = 0;
#define FCGI_Accept() looponce--
#endif

struct cgi_applet applets[] = {
//      {bbsusr_main, {"bbsusr", NULL}},
	{bbsrss_main, {"bbsrss", "rss", NULL}},
	{bbstop10_main, {"bbstop10", NULL}},
	{bbsdoc_main, {"bbsdoc", "doc", NULL}},
	{bbscon_main, {"bbscon", "con", NULL}},
	{bbsbrdadd_main, {"bbsbrdadd", "brdadd", NULL}},
	{bbsboa_main, {"bbsboa", "boa", NULL}},
	{bbsall_main, {"bbsall", NULL}},
	{bbsanc_main, {"bbsanc", "anc", NULL}},
	{bbs0an_main, {"bbs0an", "0an", NULL}},
	{bbslogout_main, {"bbslogout", NULL}},
	{bbsleft_main, {"bbsleft", NULL}},
	{bbslogin_main, {"bbslogin", NULL}},
	{bbsbadlogins_main, {"bbsbadlogins", NULL}},
	{bbsqry_main, {"bbsqry", "qry", NULL}},
	{bbsnot_main, {"bbsnot", "not", NULL}},
	{bbsfind_main, {"bbsfind", NULL}},
	{bbsfadd_main, {"bbsfadd", NULL}},
	{bbsfdel_main, {"bbsfdel", NULL}},
	{bbsfall_main, {"bbsfall", NULL}},
	{bbsfriend_main, {"bbsfriend", NULL}},
	{bbsfoot_main, {"bbsfoot", NULL}},
	{bbsform_main, {"bbsform", NULL}},
	{bbspwd_main, {"bbspwd", NULL}},
	{bbsplan_main, {"bbsplan", NULL}},
	{bbsinfo_main, {"bbsinfo", NULL}},
	{bbsmybrd_main, {"bbsmybrd", NULL}},
	{bbssig_main, {"bbssig", NULL}},
	{bbspst_main, {"bbspst", "pst", NULL}},
	{bbsgcon_main, {"bbsgcon", "gcon", NULL}},
	{bbsgdoc_main, {"bbsgdoc", "gdoc", NULL}},
	{bbsmmdoc_main, {"bbsmmdoc", "mmdoc", NULL}},	//add by macintosh 050516
	{bbsdel_main, {"bbsdel", "del", NULL}},
	{bbsdelmail_main, {"bbsdelmail", NULL}},
	{bbsmailcon_main, {"bbsmailcon", NULL}},
	{bbsmail_main, {"bbsmail", "mail", NULL}},
	{bbsdelmsg_main, {"bbsdelmsg", NULL}},
	{bbssnd_main, {"bbssnd", NULL}},
//      {bbsalluser_main, {"bbsalluser", NULL}},
	{bbsnotepad_main, {"bbsnotepad", NULL}},
	{bbsmsg_main, {"bbsmsg", NULL}},
	{bbssendmsg_main, {"bbssendmsg", NULL}},
	{bbsreg_main, {"bbsreg", NULL}},
	{bbsscanreg_main, {"bbsscanreg", NULL}},
	{bbsmailmsg_main, {"bbsmailmsg", NULL}},
	{bbssndmail_main, {"bbssndmail", NULL}},
	{bbsnewmail_main, {"bbsnewmail", NULL}},
	{bbspstmail_main, {"bbspstmail", "pstmail", NULL}},
	{bbsgetmsg_main, {"bbsgetmsg", NULL}},
	//{bbschat_main, {"bbschat", NULL}},
	{bbscloak_main, {"bbscloak", NULL}},
	{bbsmdoc_main, {"bbsmdoc", "mdoc", NULL}},
	{bbsnick_main, {"bbsnick", NULL}},
	{bbstfind_main, {"bbstfind", "tfind", NULL}},
	{bbsadl_main, {"bbsadl", NULL}},
	{bbstcon_main, {"bbstcon", "tcon", NULL}},
	{bbstdoc_main, {"bbstdoc", "tdoc", NULL}},
	{bbsdoreg_main, {"bbsdoreg", NULL}},
	{bbsmywww_main, {"bbsmywww", NULL}},
	{bbsccc_main, {"bbsccc", "ccc", NULL}},
	{bbsufind_main, {"bbsufind", NULL}},
	{bbsclear_main, {"bbsclear", "clear", NULL}},
	{bbsstat_main, {"bbsstat", NULL}},
	{bbsedit_main, {"bbsedit", "edit", NULL}},
	{bbsman_main, {"bbsman", "man", NULL}},
	{bbsparm_main, {"bbsparm", NULL}},
	{bbsfwd_main, {"bbsfwd", "fwd", NULL}},
	{bbsmnote_main, {"bbsmnote", NULL}},
	{bbsdenyall_main, {"bbsdenyall", NULL}},
	{bbsdenydel_main, {"bbsdenydel", NULL}},
	{bbsdenyadd_main, {"bbsdenyadd", NULL}},
	{bbstopb10_main, {"bbstopb10", NULL}},
	{bbsbfind_main, {"bbsbfind", "bfind", NULL}},
	{bbsx_main, {"bbsx", NULL}},
	{bbseva_main, {"bbseva", "eva", NULL}},
	{bbsvote_main, {"bbsvote", "vote", NULL}},
	{bbsshownav_main, {"bbsshownav", NULL}},
	{bbsbkndoc_main, {"bbsbkndoc", "bkndoc", NULL}},
	{bbsbknsel_main, {"bbsbknsel", "bknsel", NULL}},
	{bbsbkncon_main, {"bbsbkncon", "bkncon", NULL}},
	{bbshome_main, {"bbshome", "home", NULL}},
	{bbsindex_main, {"bbsindex", NULL}},
	{bbssechand_main, {"bbssechand", NULL}},
	{bbsupload_main, {"bbsupload", NULL}},
	{bbslform_main, {"bbslform", NULL}},
	{bbst_main, {"bbst", NULL}},
	{bbslt_main, {"bbslt", NULL}},
	{bbsdt_main, {"bbsdt", NULL}},
	{regreq_main, {"regreq", NULL}},
	{bbsselstyle_main, {"bbsselstyle", NULL}},
	{bbscon1_main, {"bbscon1", "c1", NULL}},
	{bbsattach_main, {"attach", NULL}},
	{bbskick_main, {"kick", NULL}},
	{bbsshowfile_main, {"bbshowfile", "showfile", NULL}},
	{bbsincon_main, {"boards", NULL}},
	{bbssetscript_main, {"setscript",NULL}},
	{bbsucss_main,{"bbsucss",NULL}},
	{bbsdefcss_main,{"bbsdefcss",NULL}},
	{bbscccmail_main, {"bbscccmail", NULL}},
	{bbsfwdmail_main, {"bbsfwdmail", NULL}},
	{bbssecfly_main, {"bbssecfly", NULL}},
	{bbstmpl_main, {"bbstmpl", NULL}},
	{bbssbs_main, {"bbssbs", NULL}},
	{bbseditmail_main, {"bbseditmail", NULL}},
	{apiqry_main, {"apiqry", NULL}},
	{bbsfindpass_main, {"bbsfindpass", NULL}},
	{bbsresetpass_main, {"bbsresetpass", NULL}},
	{bbsfindacc_main, {"bbsfindacc", NULL}},
//	{bbschangestyle_main, {"bbschangestyle", "changestyle", NULL}},
	{NULL}
};

#include <sys/times.h>
char *cginame = NULL;
static unsigned int rt = 0;
void
logtimeused()
{
	char buf[1024];
	struct cgi_applet *a = applets;
	while (a->main) {
		sprintf(buf, "%s %d %f %f\n", a->name[0], a->count, a->utime,
			a->stime);
		f_append(MY_BBS_HOME "/gprof/cgirtime", buf);
		a++;
	}
}

static void
cgi_time(struct cgi_applet *a)
{
	static struct rusage r0, r1;
	getrusage(RUSAGE_SELF, &r1);

	if (a == NULL) {
		r0 = r1;
		return;
	}
	a->count++;
	a->utime +=
	    1000. * (r1.ru_utime.tv_sec - r0.ru_utime.tv_sec) +
	    (r1.ru_utime.tv_usec - r0.ru_utime.tv_usec) / 1000.;
	a->stime +=
	    1000. * (r1.ru_stime.tv_sec - r0.ru_stime.tv_sec) +
	    (r1.ru_stime.tv_usec - r0.ru_stime.tv_usec) / 1000.;
	r0 = r1;
}

void
wantquit(int signal)
{
	if (incgiloop)
		incgiloop = 0;
	else
		exit(6);
}

#if defined(ENABLE_GHTHASH) && defined(ENABLE_FASTCGI)
struct cgi_applet *
get_cgi_applet(char *needcgi)
{
	static ght_hash_table_t *p_table = NULL;
	struct cgi_applet *a;

	if (p_table == NULL) {
		int i;
		a = applets;
		p_table = ght_create(250, NULL, 0);
		while (a->main != NULL) {
			a->count = 0;
			a->utime = 0;
			a->stime = 0;
			for (i = 0; a->name[i] != NULL; i++)
				ght_insert(p_table, (void *) a,
					   strlen(a->name[i]),
					   (void *) a->name[i]);
			a++;
		}
	}
	if (p_table == NULL)
		return NULL;
	return ght_get(p_table, strlen(needcgi), needcgi);
}
#else
struct cgi_applet *
get_cgi_applet(char *needcgi)
{
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
#endif

int nologin = 1;

FILE *myout;
char *myoutbuf = NULL;
int myoutsize;
FILE oldout;

#if 0
//don't delete this function although it is unused now
//it will perhaps be used in the furture.
void
start_outcache()
{
	if (myoutbuf) {
		free(myoutbuf);
		myoutbuf = NULL;
	}
	myout = open_memstream(&myoutbuf, &myoutsize);
	if (NULL == myout) {
		errlog("can't open mem stream...");
		exit(14);
	}
	oldout = *stdout;
#ifdef ENABLE_FASTCGI
	stdout->stdio_stream = myout;
	stdout->fcgx_stream = 0;
#else
	*stdout = *myout;
#endif
}
#endif
void
no_outcache()
{
	fclose(stdout);
	*stdout = oldout;
	if (myoutbuf) {
		free(myoutbuf);
		myoutbuf = NULL;
	}
}

void
end_outcache()
{
	char *ptr;
	int len;
	fclose(stdout);
	*stdout = oldout;
	if (!myoutbuf)
		return;
	ptr = strstr(myoutbuf, "\n\n");
	if (!ptr) {
		printf("\n\nfaint! 出毛病了...");
		return;
	}
	if (strnstr(myoutbuf, "Content-type: ", ptr - myoutbuf)) {
		len = myoutsize - (ptr - myoutbuf) - 2;
		printf("Content-Length: %d\n", len);
	}
	fputs(myoutbuf, stdout);
	free(myoutbuf);
	myoutbuf = NULL;
}

void
get_att_server()
{
	FILE *fp;
	char *ptr;
	char buf[128];
	unsigned long accel_ip, accel_port, validproxy[MAX_PROXY_NUM];
	int i;
	struct in_addr proxy_ip;
	i = accel_ip = accel_port = 0;
	bzero(validproxy, sizeof (validproxy));
	fp = fopen("ATT_SERVER", "r");
	if (!fp)
		goto END;

	while (fgets(buf, sizeof (buf), fp) && i < MAX_PROXY_NUM + 1) {
		strtok(buf, "\n");
		if (i) {
			if (inet_aton(buf, &proxy_ip)) {
				validproxy[i - 1] = proxy_ip.s_addr;
				i++;
			}
			continue;
		}
		ptr = strchr(buf, ':');
		if (ptr)
			*ptr = 0;
		if (!inet_aton(buf, &proxy_ip))
			continue;
		accel_ip = proxy_ip.s_addr;
		if (ptr)
			accel_port = atoi(ptr + 1);
		else
			accel_port = DEFAULT_PROXY_PORT;
		i++;
	}
	fclose(fp);
      END:
	wwwcache->accel_ip = accel_ip;
	wwwcache->accel_port = accel_port;
	for (i = 0; i < MAX_PROXY_NUM; i++)
		wwwcache->validproxy[i] = validproxy[i];
}

time_t thisversion;

int
main(int argc, char *argv[])
{
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
	wwwcache = get_shm(WWWCACHE_SHMKEY, sizeof (struct WWWCACHE));
	if (NULL == wwwcache)
		exit(0);
	thisversion = file_time(argv[0]);
	if (thisversion > wwwcache->www_version)
		wwwcache->www_version = thisversion;
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
	get_att_server();
	while (FCGI_Accept() >= 0) {
//              start_outcache();
		cginame = NULL;
		incgiloop = 1;
		if (setjmp(cgi_start)) {
//                      end_outcache();
			cgi_time(a);
			if (!incgiloop || wwwcache->www_version > thisversion
			    || rt++ > 40000) {
				logtimeused();
				exit(2);
			}
			incgiloop = 0;
			continue;
		}
		html_header(0);
		now_t = time(NULL);
		via_proxy = 0;
		strsncpy(fromhost, getsenv("REMOTE_ADDR"), 46); //ipv6 by leoncom
		inet_pton(PF_INET6,fromhost,&from_addr);
		//inet_aton(fromhost, &from_addr);
		/*  ipv6 by leoncom 无视validproxy
		for (i = 0; wwwcache->validproxy[i] && i < MAX_PROXY_NUM; i++) {
			if (from_addr.s_addr == wwwcache->validproxy[i]) {
				via_proxy = 1;
				break;
			}
		}
		if (via_proxy) {
			char *ptr, *p;
			int IPLEN = 255;
			ptr = getenv("HTTP_X_FORWARDED_FOR");
			if (!ptr)
				ptr = getsenv("REMOTE_ADDR");
			p = strrchr(ptr, ',');
			if (p != NULL) {
				while (!isdigit(*p) && *p)
					p++;
				if (*p)
					strncpy(fromhost, p, IPLEN);
				else
					strncpy(fromhost, ptr, IPLEN);
			} else
				strncpy(fromhost, ptr, IPLEN);
			fromhost[IPLEN] = 0;
			inet_aton(fromhost, &from_addr);
		}
		*/
		if (url_parse())
			http_fatal("%s 没有实现的功能!", getsenv("SCRIPT_URL"));
		http_parm_init();
		a = get_cgi_applet(needcgi);
		if (a != NULL) {
			cginame = a->name[0];
			//access(getsenv("QUERY_STRING"), F_OK);
			wwwcache->www_visit++;
			(*(a->main)) ();
//                      end_outcache();
			cgi_time(a);
			if (!incgiloop || wwwcache->www_version > thisversion) {
				logtimeused();
				exit(4);
			}
			incgiloop = 0;
			continue;
		}
		http_fatal("%s 没有实现的功能!", getsenv("SCRIPT_URL"));
//              end_outcache();
		incgiloop = 0;
	}
	munmap(ummap_ptr, ummap_size);
	exit(5);


}
