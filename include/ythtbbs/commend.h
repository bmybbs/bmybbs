#ifndef BMYBBS_COMMEND_H
#define BMYBBS_COMMEND_H
#include <time.h>
#include "config.h"

#define COMMENDFILE      MY_BBS_HOME"/.COMMEND"
#define COMMENDFILE2     MY_BBS_HOME"/.COMMEND2"
/**
 * @brief front page commend
 * modify by mintbaggio 040326
 */
struct commend{
	char board[24];         ///< the board that the article be commened is in
	char userid[IDLEN+2];   ///< the author of the article be commended
	char com_user[IDLEN+2]; ///< the user who commend this article
	char title[80];         ///< the title of the article be commended
	char filename[80];      ///< the filename of the article be commended
	time_t time;            ///< the time when the com_user commend this article
	unsigned int accessed;
};
#endif

