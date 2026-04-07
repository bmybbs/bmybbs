#ifndef BMYBBS_ONE_KEY_H
#define BMYBBS_ONE_KEY_H

typedef int (*OneKeyHandler)(int ent, void *record, char *direct);

/**
 * Used to pass commands to the readmenu
 */
struct one_key {
	int key;
	OneKeyHandler fptr;
	char func[33];    ///< add by gluon for self-define menu
};

#endif
