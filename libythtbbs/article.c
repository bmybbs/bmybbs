#include <fcntl.h>
#include <errno.h>
#include "ythtbbs.h"

// ÎªÁË±à¼­ html/xml/xhtml ÎÄ¼şÒıÈë¿â libxml2 by IronBlood 20130805
#include <sys/file.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/xpath.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>   // ±à¼­xmlÄÚÈİ
#include <libxml/xmlsave.h>

// ÎªÁË´¦Àí @id ÒıÈë pcre ¿â by IronBlood 20140624
#include <pcre.h>

static int is_article_link_in_file(char *boardname, int thread, char *filename);
static int update_article_link_in_file(char *boardname, int oldthread, int newfiletime, char *newtitle, char *filename);

char *
fh2fname(struct fileheader *fh)
{
	static char s[16];
	sprintf(s, "M.%d.A", fh->filetime);
	if (fh->accessed & FH_ISDIGEST)
		s[0] = 'G';
	if (fh->accessed & FILE_ISTOP1)  //add by wjbta
	        s[0] = 'T';   
	return s;
}

char *
bknh2bknname(struct bknheader *bknh)
{
	static char s[16];
	sprintf(s, "B.%d", bknh->filetime);
	return s;
}

char *
fh2owner(struct fileheader *fh)
{
	if (!fh->owner[0])
		return "Anonymous";
	else
		return fh->owner;
}

char *
fh2realauthor(struct fileheader *fh)
{
	if (!fh->owner[0])
		return fh->owner + 1;
	else
		return fh->owner;
}

int
fh2modifytime(struct fileheader *fh)
{
	if (fh->edittime)
		return fh->edittime;
	else
		return fh->filetime;
}

void
fh_setowner(struct fileheader *fh, char *owner, int anony)
{
	char *ptr;
	bzero(fh->owner, sizeof (fh->owner));
	if (anony) {
		fh->owner[0] = 0;
		strsncpy(fh->owner + 1, owner, sizeof (fh->owner) - 1);
		return;
	}
	if (!*owner) {
		*fh->owner = 0;
		return;
	}
	strsncpy(fh->owner, owner, sizeof (fh->owner) - 1);
	if (!strchr(owner, '@') && !strchr(owner, '.')) {
		ptr = strchr(fh->owner, ' ');
		if (ptr)
			*ptr = 0;
		return;
	}
	ptr = fh->owner + 1;
	while (*ptr) {
		if (*ptr == '@' || *ptr == '.' || *ptr == ' ') {
			*ptr = '.';
			*(ptr + 1) = 0;
			return;
		}
		ptr++;
	}
	*--ptr = '.';
	return;
}

int
change_dir(char *direct, struct fileheader *fileinfo,
	   void (*func(void *, void *)), int ent, int digestmode, int mode)
{
	int i, newent;
	int fd;
	int size = sizeof (struct fileheader);
	struct fileheader xfh, newfileinfo;
	struct flock ldata;
	if (digestmode != 0)
		return -1;
	if ((fd = open(direct, O_RDWR, 0660)) == -1)
		return -2;

	if (mode)
		memcpy(&newfileinfo, fileinfo, size);
	newent = ent;
	for (i = newent; i > 0; i--) {
		if (lseek(fd, size * (i - 1), SEEK_SET) == -1) {
			i = 0;
			break;

		}
		if (read(fd, &xfh, size) != size) {
			i = 0;
			break;
		}
		if (fileinfo->filetime == xfh.filetime) {
			newent = i;
			break;
		}
	}
	if (!i) {
		close(fd);
		return -3;
	}
	memcpy(fileinfo, &xfh, size);
	ldata.l_type = F_WRLCK;
	ldata.l_whence = 0;
	ldata.l_len = size;
	ldata.l_start = size * (newent - 1);
	if (fcntl(fd, F_SETLKW, &ldata) == -1) {
		errlog("reclock error %d", errno);
		close(fd);
		return -4;
	}
	(*func) (fileinfo, &newfileinfo);
	if (lseek(fd, size * (newent - 1), SEEK_SET) == -1) {
		errlog("subrec seek err %d", errno);
		close(fd);
		return -5;
	}
	if (write(fd, fileinfo, size) != size) {
		errlog("subrec write err %d", errno);
		close(fd);
		return -6;
	}
	ldata.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &ldata);
	close(fd);
	return 0;
}

#define SWITCH_FLAG(x, y) if(x & y) {x &= ~y;} else { x |= y;}

void
DIR_do_mark(struct fileheader *fileinfo, struct fileheader *newfileinfo)
{
	SWITCH_FLAG(fileinfo->accessed, FH_MARKED);
}

void
DIR_do_digest(struct fileheader *fileinfo, struct fileheader *newfileinfo)
{
	SWITCH_FLAG(fileinfo->accessed, FH_DIGEST);
}

void
DIR_do_underline(struct fileheader *fileinfo, struct fileheader *newfileinfo)
{
	SWITCH_FLAG(fileinfo->accessed, FH_NOREPLY);
}

void DIR_do_water(struct fileheader *fileinfo, struct fileheader *newfileinfo)
{
	SWITCH_FLAG(fileinfo->accessed, FH_ISWATER);
}

void
DIR_do_allcanre(struct fileheader *fileinfo, struct fileheader *newfileinfo)
{
	SWITCH_FLAG(fileinfo->accessed, FH_ALLREPLY);
}

void
DIR_do_attach(struct fileheader *fileinfo, struct fileheader *newfileinfo)
{
	fileinfo->accessed |= FH_ATTACHED;
}

void
DIR_clear_dangerous(struct fileheader *fileinfo, struct fileheader *newfileinfo)
{
	fileinfo->accessed &= ~FH_DANGEROUS;
}

void
DIR_do_dangerous(struct fileheader *fileinfo, struct fileheader *newfileinfo)
{
	fileinfo->accessed |= FH_DANGEROUS;
}

void
DIR_do_markdel(struct fileheader *fileinfo, struct fileheader *newfileinfo)
{
	if(fileinfo->accessed & FH_MINUSDEL)			//add by mintbaggio@BMY 040321 for minus-postnums delete
		fileinfo->accessed &= ~FH_MINUSDEL;
	SWITCH_FLAG(fileinfo->accessed, FH_DEL);
}

void
DIR_do_mark_minus_del(struct fileheader *fileinfo, struct fileheader *newfileinfo)
{								//add by mintbaggio@BMY 040321 for minus-postnums delete
	if(fileinfo->accessed & FH_DEL)
		fileinfo->accessed &= ~FH_DEL;
        SWITCH_FLAG(fileinfo->accessed, FH_MINUSDEL);
}

void
DIR_do_edit(struct fileheader *fileinfo, struct fileheader *newfileinfo)
{
	fileinfo->sizebyte = newfileinfo->sizebyte;
	fileinfo->edittime = newfileinfo->edittime;
}

void
DIR_do_changetitle(struct fileheader *fileinfo, struct fileheader *newfileinfo)
{
	strncpy(fileinfo->title, newfileinfo->title, 60);
	fileinfo->title[59] = 0;
}

void
DIR_do_evaluate(struct fileheader *fileinfo, struct fileheader *newfileinfo)
{
	fileinfo->staravg50 = newfileinfo->staravg50;
	fileinfo->hasvoted = newfileinfo->hasvoted;
}

void
DIR_do_spec(struct fileheader *fileinfo, struct fileheader *newfileinfo)
{
	SWITCH_FLAG(fileinfo->accessed, FH_SPEC);
}

void
DIR_do_import(struct fileheader *fileinfo, struct fileheader *newfileinfo)
{
	if (newfileinfo->accessed & FH_ANNOUNCE)
		fileinfo->accessed |= FH_ANNOUNCE;
}

void
DIR_do_suremarkdel(struct fileheader *fileinfo, struct fileheader *newfileinfo)
{
	fileinfo->accessed |= FH_DEL;
}

void
DIR_do_top(struct fileheader *fileinfo, struct fileheader *newfileinfo)
{
	SWITCH_FLAG(fileinfo->accessed, FILE_TOP1);
}

int
outgo_post(struct fileheader *fh, char *board, char *id, char *name)
{
	FILE *foo;
	if (fh->accessed & FH_INND)
		if ((foo = fopen("inndlog/out.bntp", "a"))) {
			fprintf(foo, "%s\t%s\t%s\t%s\t%s\n", board,
				fh2fname(fh), id, name, fh->title);
			fclose(foo);
		}
	return 0;
}

/* modifying by ylsdd 
 * unlink action is taked within cancelpost if in mail
 * else this item is added to the file '.DELETED' under
 * the board's directory, the filename is not changed.
 * Unlike the fb code which moves the file to the deleted
 * board.
 *             */
void
cancelpost(char *board, char *userid, struct fileheader *fh, int owned)
{
	struct fileheader postfile;
	FILE *fin;
	int digestmode, len;
	char buf[256], from[STRLEN], *ptr;
	time_t now_t;
	postfile = *fh;
	postfile.accessed &= ~(FH_MARKED | FH_SPEC | FH_DIGEST);
	postfile.deltime = time(&now_t) / (3600 * 24) % 100;
	sprintf(buf, "%-32.32s - %s", fh->title, userid);
	strsncpy(postfile.title, buf, sizeof (postfile.title));
	digestmode = (owned) ? 5 : 4;
	if (5 == digestmode)
		sprintf(buf, MY_BBS_HOME "/boards/%s/.JUNK", board);
	else
		sprintf(buf, MY_BBS_HOME "/boards/%s/.DELETED", board);
	append_record(buf, &postfile, sizeof (postfile));
	if (strrchr(fh->owner, '.'))
		return;
	if ((fh->accessed & FH_INND) && fh->filetime > now_t - 14 * 86400) {
		sprintf(buf, MY_BBS_HOME "/boards/%s/%s", board, fh2fname(fh));
		from[0] = '\0';
		if ((fin = fopen(buf, "r")) != NULL) {
			while (fgets(buf, sizeof (buf), fin) != NULL) {
				len = strlen(buf) - 1;
				buf[len] = '\0';
				if (len <= 8)
					break;
				if (strncmp(buf, "·¢ĞÅÈË: ", 8))
					continue;
				if ((ptr = strrchr(buf, ')')) != NULL) {
					*ptr = '\0';
					if ((ptr = strrchr(buf, '('))
					    != NULL) {
						strcpy(from, ptr + 1);
						break;
					}
				}
			}
			fclose(fin);
		}
		sprintf(buf, "%s\t%s\t%s\t%s\t%s\n",
			board, fh2fname(fh), fh->owner, from, fh->title);
		if ((fin = fopen("inndlog/cancel.bntp", "a")) != NULL) {
			fputs(buf, fin);
			fclose(fin);
		}
	}
}

int
cmp_title(char *title, struct fileheader *fh1)
{
	char *p1;
	if (!strncasecmp(fh1->title, "Re:", 3))
		p1 = fh1->title + 4;
	else
		p1 = fh1->title;
	return (!strncmp(p1, title, 45));
}

int
fh_find_thread(struct fileheader *fh, char *board)
{
	char direct[255];
	char *p;
	int i;
	int start;
	struct mmapfile mf = { ptr:NULL };
	struct fileheader *buf1;
	char *title = fh->title;
	int size = sizeof(struct fileheader);
	if (fh->thread != 0)
		return 0;
	fh->thread = fh->filetime;
	sprintf(direct, MY_BBS_HOME "/boards/%s/.DIR", board);
        if (mmapfile(direct, &mf) < 0)
                return -1;
	if (!strncasecmp(title, "Re:", 3))
		p = title + 4;
	else
		p = title;
	start = mf.size / size;
	for (i = start, buf1 = (struct fileheader *)(mf.ptr + size * (start - 1)); i > start - 100 && i>0; i--, buf1 = (struct fileheader *)(mf.ptr + size * (i - 1)))
		//½öÔÚ×î½ü100ÆªÄÚ³¢ÊÔËÑË÷Í¬Ö÷Ìâ
		if (cmp_title(p, buf1)) {
			if (buf1->thread != 0) {
				fh->thread = buf1->thread;
				break;
			}
		}
	mmapfile(NULL, &mf);
	return 0;
}

int
Search_Bin(char *ptr, int key, int start, int end)
{
        // ÔÚÓĞĞò±íÖĞÕÛ°ë²éÕÒÆä¹Ø¼ü×ÖµÈÓÚkeyµÄÊı¾İÔªËØ¡£
        // Èô²éÕÒµ½£¬·µ»ØË÷Òı
	// ·ñÔòÎª´óÓÚkeyµÄ×îĞ¡Êı¾İÔªËØË÷Òım£¬·µ»Ø(-m-1) 
        int low, high, mid;
	struct fileheader *totest;
        low = start;
        high = end;
        while (low <= high) {
                mid = (low + high) / 2;
		totest = (struct fileheader *)(ptr + mid * sizeof(struct fileheader));
                if (key == totest->filetime)
                        return mid; 
                else if (key < totest->filetime)
                        high = mid - 1;
                else
                        low = mid + 1;
        }
        return -(low+1);
}

int
add_edit_mark(char *fname, char *userid, time_t now_t, char *fromhost)
{
        FILE *fp;
        if ((fp = fopen(fname, "a")) == NULL)
                return 0;
        fprintf(fp,
                "\n[1;36m¡ù ĞŞ¸Ä:£®%s ÓÚ %15.15s ĞŞ¸Ä±¾ÎÄ£®[FROM: %-.20s][m",
                userid, ctime(&now_t) + 4, fromhost);
        fclose(fp);
        return 0;
}

/**
 * ¼ì²éÖ÷ÌâÊÇ·ñ´æÔÚÓëËùÔÚÌÖÂÛÇøÈÈÃÅ»°ÌâÖĞ
 * @param boardname °æÃæÃû³Æ
 * @param thread Ö÷Ìâid
 * @return ´æÔÚ·µ»Ø1
 */
int is_article_area_top(char *boardname, int thread) {
	struct boardmem *bm = getboardbyname(boardname);
    if(bm==NULL)
		return 0;

	char area_top_filename[20];
	sprintf(area_top_filename, "etc/Area_Dir/%c", bm->header.secnumber1);

	return is_article_link_in_file(boardname, thread, area_top_filename);
}

/**
 * ÔÚÖ÷ÌâËùÔÚÌÖÂÛÇøÖĞ¸üĞÂÁ´½Ó
 * @param boardname °æÃæÃû³Æ
 * @param oldthread Ô­Ö÷Ìâid
 * @param newfiletime ¶ÔÓ¦ĞÂµÄºÏ¼¯ÎÄÕÂµÄ filetime
 * @param newtitle ¶ÔÓ¦ĞÂµÄºÏ¼¯ÎÄÕÂµÄ±êÌâ£¬e.g. "¡¾ºÏ¼¯¡¿ "
 * @return ¸üĞÂ³É¹¦·µ»Ø1
 */
int update_article_area_top_link(char *boardname, int oldthread, int newfiletime, char *newtitle) {
	struct boardmem *bm = getboardbyname(boardname);
    if(bm==NULL)
		return 0;

	char area_top_filename[20];
	sprintf(area_top_filename, "etc/Area_Dir/%c", bm->header.secnumber1);

	return update_article_link_in_file(boardname, oldthread, newfiletime, newtitle, area_top_filename);
}

/**
 * ÅĞ¶ÏÊÇ·ñÊ®´ó¡£
 * @warning ½öĞèÒª¼ì²éÒ»¸öÎÄ¼ş¡£
 * @param boardname °æÃæÃû³Æ
 * @param thread Ö÷Ìâid
 * @return ´æÔÚ·µ»Ø1
 */
int is_article_site_top(char *boardname, int thread) {
	char *site_top_file1 = "wwwtmp/topten";

	return is_article_link_in_file(boardname, thread, site_top_file1);
}

/**
 * ¸üĞÂÊ®´óÁ´½Ó¡£
 * @warning ĞèÒª¸üĞÂËÄ¸öÎÄ¼ş¡£
 * @param boardname °æÃæÃû³Æ
 * @param oldthread Ô­Ö÷Ìâid
 * @param newfiletime ¶ÔÓ¦ĞÂµÄºÏ¼¯ÎÄÕÂµÄ filetime
 * @param newtitle ¶ÔÓ¦ĞÂµÄºÏ¼¯ÎÄÕÂµÄ±êÌâ£¬e.g. "¡¾ºÏ¼¯¡¿ "
 * @return ¸üĞÂ³É¹¦·µ»Ø1
 */
int update_article_site_top_link(char *boardname, int oldthread, int newfiletime, char *newtitle) {
	char *site_top_file1 = "wwwtmp/topten";
	char *site_top_file2 = "wwwtmp/indextopten";
	char *site_top_file3 = "wwwtmp/ctopten";
	char *site_top_file4 = "wwwtmp/cindextopten";

	return update_article_link_in_file(boardname, oldthread, newfiletime, newtitle, site_top_file1)
			&& update_article_link_in_file(boardname, oldthread, newfiletime, newtitle, site_top_file2)
			&& update_article_link_in_file(boardname, oldthread, newfiletime, newtitle, site_top_file3)
			&& update_article_link_in_file(boardname, oldthread, newfiletime, newtitle, site_top_file4);
}

/**
 * ÅĞ¶ÏÎÄÕÂÖ÷ÌâÁ´½ÓÊÇ·ñ´æÔÚÓÚÎÄ¼şÖĞ
 * @param boardname °æÃæÃû³Æ
 * @param thread Ô­Ö÷Ìâid
 * @param filename ĞèÒªÅĞ¶ÏµÄÎÄ¼şÂ·¾¶
 * @return Èç¹ûÎÄ¼şÖĞ´æÔÚ¸ÃÆªÎÄÕÂÖ÷Ìâ£¬Ôò·µ»Ø1£¬²»´æÔÚ·µ»Ø0£¬³ö´í·µ»Ø-1
 */
static int is_article_link_in_file(char *boardname, int thread, char *filename) {
	htmlDocPtr doc = htmlParseFile(filename, "GBK");
	if(doc == NULL) {
		return -1;
	}

	xmlNodePtr pRoot = xmlDocGetRootElement(doc);
	if(pRoot == NULL) {
		xmlFreeDoc(doc);
		return -1;
	}

	char xpath[80];
	sprintf(xpath, "//a[@href='tfind?board=%s&th=%d']", boardname, thread);

	xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
	xmlXPathObjectPtr result = xmlXPathEvalExpression((const xmlChar *)xpath, ctx);

	int r = (result->nodesetval->nodeNr == 1);

	xmlXPathFreeObject(result);
	xmlXPathFreeContext(ctx);
	xmlFreeDoc(doc);

	return r;
}

/**
 * ¸üĞÂÎÄ¼şÖĞÎÄÕÂµÄÁ´½Ó
 * @param boardname °æÃæÃû³Æ
 * @param oldthread Ô­Ö÷Ìâid
 * @param newfiletime ¶ÔÓ¦ĞÂÎÄÕÂµÄ filetime
 * @param newtitle ĞÂÎÄÕÂµÄ±êÌâ£¬e.g. "¡¾ºÏ¼¯¡¿ "
 * @param filename ĞèÒª¸üĞÂµÄÎÄ¼şÂ·¾¶
 * @return ÈôÎÄ¼şÒÑ¸üĞÂ£¬Ôò·µ»Ø1
 */
static int update_article_link_in_file(char *boardname, int oldthread, int newfiletime, char *newtitle, char *filename) {
	int fd=open(filename, O_RDONLY);
	if(fd == -1)
		return -1;
	flock(fd, LOCK_EX);
	htmlDocPtr doc = htmlParseFile(filename, "GBK");
	if(doc == NULL) {
		flock(fd, LOCK_UN);
		return -1;
	}

	xmlNodePtr pRoot = xmlDocGetRootElement(doc);
	if(pRoot == NULL) {
		xmlFreeDoc(doc);
		flock(fd, LOCK_UN);
		return -1;
	}

	char xpath[80];
	sprintf(xpath, "//a[@href='tfind?board=%s&th=%d']", boardname, oldthread);

	xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
	xmlXPathObjectPtr result = xmlXPathEvalExpression((const xmlChar*)xpath, ctx);

	// ÎÄ¼şÖĞÓĞÇÒ½öÓĞÒ»¸öÁ´½ÓÊ±²Å»áÖ´ĞĞ¸üĞÂÁ´½ÓµÄ²Ù×÷
	int r=0;
	if((r=result->nodesetval->nodeNr) == 1){
		char new_href[80];
		sprintf(new_href, "con?B=%s&F=M.%d.A", boardname, newfiletime); //ĞÂÁ´½Ó²ÉÓÃÒ»°ãÔÄ¶ÁÄ£Ê½

		xmlNodePtr cur = result->nodesetval->nodeTab[0];
		xmlSetProp(cur, (const xmlChar*)"href", (const xmlChar*)new_href); // ¸üĞÂÁ´½Ó
//		xmlNodeSetContent(cur, (const xmlChar*)newtitle); // ¸üĞÂÁ´½ÓÎÄ×Ö

		char newFilename[80];
		sprintf(newFilename, "%s.new", filename);
		htmlSaveFileEnc(newFilename, doc, "GBK");	
		rename(newFilename, filename);
	}

	xmlXPathFreeObject(result);
	xmlXPathFreeContext(ctx);
	xmlFreeDoc(doc);
	flock(fd, LOCK_UN);
	return r;
}


time_t fn2timestamp(char * filename) {
	char num_str[11]={'0'};
	memcpy(num_str, &filename[2], 10);
	return (time_t)atoi(num_str);
}

int parse_mentions(char *content, char *userids[])
{
	const char * MENTION_PATTERN = "@[A-Za-z]{1,12}\\s";
	pcre *re;
	const char *re_error;
	int erroffset, i, j, offsetcount, offsets[3], is_exist;
	const char *match;
	char buf[14];

	re = pcre_compile(MENTION_PATTERN, 0, &re_error, &erroffset, NULL);
	if(re == NULL) {
		return -1;
	}

	i=0;	// ÓÃÓÚ userids[i] Ë÷Òı
	offsetcount = pcre_exec(re, NULL, content, strlen(content), 0, 0, offsets, 3);
	while(offsetcount>0 && i<MAX_MENTION_ID) {
		if(pcre_get_substring(content, offsets, offsetcount, 0, &match) >= 0) {
			if(i==0) { // userids »¹Îª¿ÕµÄÊ±ºò
				strncpy(userids[0], match+1, strlen(match)-2);
				++i;
			} else { // userids ÒÑ¾­´æÔÚ id ÁË£¬ÔòÑ­»·±È½Ï
				is_exist = 0;
				memset(buf, 0, 14);
				strncpy(buf, match+1, strlen(match)-2);
				for(j=0; j<i; ++j) {
					if(!strcasecmp(buf, userids[j]))
						is_exist = 1;
				}
				if(!is_exist) {	// buf ÔÚ userids ÖĞ²»´æÔÚµÄÊ±ºò¿½±´µ½ userids ÖĞ
					strcpy(userids[i++], buf);
				}
			}

			pcre_free_substring(match);
		}

		offsetcount = pcre_exec(re, NULL, content, strlen(content), offsets[1], 0, offsets, 3);
	}

	pcre_free(re);
}
