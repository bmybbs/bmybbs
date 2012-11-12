#ifndef __CONFIG_H
#define __CONFIG_H
#include "bbs.h"

/*
 *	SEE content_types.h FOR A LIST OF ALLOWED FILE TYPES!!
 *	IF IT DOES NOT HAVE ANY OF THE LISTED EXTENSIONS, YOU WILL
 *	NOT BE ABLE TO RETRIEVE THE FILE
 */

/**
 ** binding to anything lower than 1024 requires special capabilities
 ** (usually root privileges). my advice is to run it as 'nobody' and
 ** leave it at port 8080. if you need something lower please hack
 ** the source to setuid to something else once you bind to port < 1024
 ** ..or use capabilities if your system has them.
 **/

#define SERVER_PORT	8080


/**
 ** leave it to 0 to bind to all interfaces, otherwise put in the
 ** host or ip address in quotes to bind specifically to that.
 **/

#define SERVER_ADDR	0


/**
 ** self-explanatory. a trailing slash is not required, but your
 ** system probably won't complain if there is one.
 ** im-httpd chdir()'s to this directory on startup
 **/

#define DOCUMENT_ROOT	"/tmp/web"


/**
 ** according to thttpd, some OS's will return an error to listen()
 ** if you provide an extremely large value rather than silently
 ** clamp it to the max value it can handle
 **
 ** it's here so that you can tune it appropriately..
 **/

#define LISTEN_BACKLOG 1024	


/**
 ** max number of concurrent connections. this is a hard limit.
 ** a small data structure is allocated for each connection at startup.
 ** setting this to 100000000 is a bad idea unless you really have that
 ** much memory to throw at this.
 **/

#define MAX_CONNECTIONS 1024	


/**
 ** buffer size
 **
 ** tuning this parameter is a tradeoff. an appropriately large value
 ** in ideal cases can fire the entire file to the client with one
 ** read and one write. in the worst case, you will incur a read of
 ** BLOCK_SIZE bytes only to find that nothing can be written to the
 ** socket. there's a good chance it'll be in the buffer cache the
 ** next time around, but you'll do less potential damage to the
 ** buffer cache with smaller values, conversely incurring more
 ** read/write/select calls
 **
 ** if most of your clients are on a high speed link you could do good
 ** to increase the size. if most of your clients will be on modems,
 ** you should probably leave it alone.
 **
 ** the default value (16384) guesses at a happy medium
 **/

#define BLOCK_SIZE	16384


/* touch these and you'll be sorry */

/**
 **	The HTTP GET request cannot be longer than this
 **/

#define MAX_REQUEST_LENGTH	4096

#endif

