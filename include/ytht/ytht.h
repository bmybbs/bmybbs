#ifndef __YTHTLIB_H
#define __YTHTLIB_H

//Copy from Linux 2.4 kernel...
#define min(x,y) ({ \
	const typeof(x) _x = (x);	\
	const typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x < _y ? _x : _y; })

#define max(x,y) ({ \
	const typeof(x) _x = (x);	\
	const typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x > _y ? _x : _y; })
//end
#include "fileop.h"
#include "named_socket.h"
#include "crypt.h"
#include "limitcpu.h"
#include "timeop.h"
#include "common.h"
#include "strop.h"
#include "smth_filter.h"
#include "uudecode.h"
#include "uuencode.h"
#include "numbyte.h"
#include "shmop.h"
#include "strlib.h"

#endif
