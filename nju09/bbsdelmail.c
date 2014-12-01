#include "bbslib.h"

int
bbsdelmail_main()
{
	FILE *fp, *fpw;
	struct fileheader f;
	char path[80], tmppath[80], file[80], *ptr, list[40][20];
	int num, ndelfile = 0;
	html_header(1);
	check_msg();
	if (!loginok || isguest)
		http_fatal("����δ��¼");
	changemode(RMAIL);

	strsncpy(list[ndelfile], getparm("file"), 20);
	if (list[ndelfile][0] == 'M')
		ndelfile++;
	for (num = 0; num < 40 && ndelfile < 40; num++) {
		sprintf(file, "F%d", num);
		strsncpy(list[ndelfile], getparm(file), 20);
		if (list[ndelfile][0] == 'M' || list[ndelfile][0] == 'G')
			ndelfile++;
	}

    int box_type = 0;
    char type_string[20]; 
    strsncpy(type_string, getparm("box_type"), 20);
    if(type_string[0] != 0) {
        box_type = atoi(type_string);
    }
    snprintf(type_string, sizeof(type_string), "box_type=%d", box_type);
    if(box_type == 1) {
        setsentmailfile(path, currentuser.userid, ".DIR");
	    setsentmailfile(tmppath, currentuser.userid, ".DIR.tmp");
    } else {
        setmailfile(path, currentuser.userid, ".DIR");
        setmailfile(tmppath, currentuser.userid, ".DIR.tmp");
    }
	fp = fopen(path, "r");
	if (fp == 0)
		http_fatal("����Ĳ���2");
	fpw = fopen(tmppath, "w");
	if (fpw == 0) {
		fclose(fp);
		http_fatal("�޷�ɾ���ʼ�");
	}
	while (1) {
		if (fread(&f, sizeof (f), 1, fp) <= 0)
			break;
		ptr = fh2fname(&f);
		for (num = 0; num < ndelfile; num++) {
			if (!strcmp(ptr, list[num]))
				break;
		}
		if (num == ndelfile) {
			fwrite(&f, sizeof (f), 1, fpw);
			continue;
		}
		setmailfile(file, currentuser.userid, ptr);
		unlink(file);
	}
	fclose(fp);
	fclose(fpw);
	rename(tmppath, path);
	printf("�ż���ɾ��.<br><a href=bbsmail?%s>�����ż��б�</a>\n"
            , type_string);
	http_quit();
	return 0;
}
