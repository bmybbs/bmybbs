/*  

   socketpair() emulator for System V Release 3

   socketpair driver included here ignores all the standard 
   socketpair() parameters except the file descriptors.

   s_pipe() is by W. Richard Stevens from the book titled
   "Unix Network Programming"  ISBN 0-13-949876-1  1990

*/

#include "includes.h"
#include <sys/stream.h>         /* defines queue_t */
#include <stropts.h>            /* defines strfdinsert */

#define SPX_DEVICE  "/dev/spx"


int socketpair(int a_family, int s_type, int s_protocol, int s_fd)
{
    return s_pipe(s_fd);
}


int s_pipe(int fd[2])
{
    struct strfdinsert ins;
    queue_t *pointer;

    /* 
     * Open the stream clone device "/dev/spx" twice,
     * obtaining two file descriptors.
     */

    if ((fd[0] = open(SPX_DEVICE, O_RDWR)) < 0)
        return (-1);

    if ((fd[1] = open(SPX_DEVICE, O_RDWR)) < 0) {
        close(fd[0]);
        return (-1);
    }

    /*
     * Now link these two streams together with an I_FDINSERT ioctl.
     */

    ins.ctlbuf.buf = (char *) &pointer; /* no ctl info, just the ptr */
    ins.ctlbuf.maxlen = sizeof(queue_t *);
    ins.ctlbuf.len = sizeof(queue_t *);

    ins.databuf.buf = (char *) 0;       /* no data to send */
    ins.databuf.maxlen = 0;
    ins.databuf.len = -1;       /* magic! must be -1 not 0 for stream pipe */

    ins.fildes = fd[1];         /* the fd to connect with fd[0] */
    ins.flags = 0;              /* nonpriority message */
    ins.offset = 0;             /* offset of pointer in control buffer */

    if (ioctl(fd[0], I_FDINSERT, (char *) &ins) < 0) {
        close(fd[0]);
        close(fd[1]);
        return (-1);
    }

    return (0);                 /* tutto posto */

}
