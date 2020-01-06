static char *(replacestr[][2]) = { {
"pku", "bbsboa?secstr=1"}, {
"wenxue", "bbsboa?secstr=Y"}, {
NULL, NULL}};

char *
bbsred(char *command)
{
	static char buf[256];
	char path[128];
	struct userec *x;
	int clen;
	int i;
	clen = strlen(command);
	if (!clen || isaword(specname, command)) {
		strcpy(buf, "bbsboa?secstr=?");
		return buf;
	}
	for (i = 0; replacestr[i][0]; i++) {
		if (!strcasecmp(command, replacestr[i][0]))
			return replacestr[i][1];
	}
	if (clen <= IDLEN) {
		x = getuser(command);
		if (x) {
			snprintf(path, 128,
				 "/groups/GROUP_0/PersonalCorpus/%c/%s",
				 mytoupper(x->userid[0]), x->userid);
			snprintf(buf, 256, MY_BBS_HOME "/0Announce%s", path);
			if (file_exist(buf)) {
				snprintf(buf, 256, "bbs0an?path=%s", path);
				return buf;
			}
		}
	}
	if (getboard(command))
		snprintf(buf, 256, "%s%s", showByDefMode(), command);
	else
		strcpy(buf, "bbsboa?secstr=?");
	return buf;
}
