#ifndef BMYBBS_ONE_KEY_H
#define BMYBBS_ONE_KEY_H

/**
 * Used to pass commands to the readmenu
 */
struct one_key {
	int key;
	int (*fptr) ();
	char func[33];    ///< add by gluon for self-define menu
};

#endif
