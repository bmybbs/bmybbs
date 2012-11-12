/*

random.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Sat Mar  4 14:55:57 1995 ylo

Cryptographically strong random number generation.

*/

/*
 * $Id: randoms.c,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: randoms.c,v $
 * Revision 1.1.1.1  2009-03-04 06:33:27  bmybbs
 * bmysrc
 *
 * Revision 1.1.1.1  2002/10/01 09:42:06  clearboy
 * update on 20051031
 * by clearboy 
 * for transfering the source codes from main site to the experimental site 
 * for the first time.
 *
 *
 * Revision 1.1.1.1  2002/10/01 09:42:06  ylsdd
 * 水木底sshbbsd导入
 * 然后慢慢改吧
 *
 * Revision 1.3  2002/08/04 11:39:43  kcn
 * format c
 *
 * Revision 1.2  2002/08/04 11:08:48  kcn
 * format C
 *
 * Revision 1.1.1.1  2002/04/27 05:47:26  kxn
 * no message
 *
 * Revision 1.1  2001/07/04 06:07:11  bbsdev
 * bbs sshd
 *
 * Revision 1.4  1998/05/23 20:23:13  kivinen
 * 	Changed uint32 to word32.
 *
 * Revision 1.3  1998/04/17  00:39:38  kivinen
 * 	Removed sys/resource.h including (it is already included in
 * 	the includes.h).
 *
 * Revision 1.2  1997/03/26 05:35:54  kivinen
 * 	Added HAVE_NO_TZ_IN_GETTIMEOFDAY support.
 *
 * Revision 1.1.1.1  1996/02/18 21:38:11  ylo
 * 	Imported ssh-1.2.13.
 *
 * Revision 1.6  1995/10/02  01:25:24  ylo
 * 	Added a cast to avoid compiler warning; also minor change in
 * 	noise collection.
 *
 * Revision 1.5  1995/09/21  17:12:23  ylo
 * 	Don't use the second argument of gettimeofday.
 *
 * Revision 1.4  1995/09/13  11:59:07  ylo
 * 	Large modifications to make this work on machines without 32
 * 	bit integer type (Cray).
 *
 * Revision 1.3  1995/08/29  22:22:50  ylo
 * 	Removed extra '&'.
 *
 * Revision 1.2  1995/07/13  01:29:20  ylo
 * 	Removed "Last modified" header.
 * 	Added cvs log.
 *
 * $Endlog$
 */

#include "includes.h"
#include "randoms.h"
#include "getput.h"
#include "userfile.h"

#ifdef HAVE_GETRUSAGE
#ifdef HAVE_RUSAGE_H
#include <sys/rusage.h>
#endif                          /* HAVE_RUSAGE_H */
#endif                          /* HAVE_GETRUSAGE */

#ifdef HAVE_TIMES
#include <sys/times.h>
#endif                          /* HAVE_TIMES */

/* Initializes the random number generator, loads any random information
   from the given file, and acquires as much environmental noise as it
   can to initialize the random number generator.  More noise can be
   acquired later by calling random_add_noise + random_stir, or by
   calling random_get_environmental_noise again later when the environmental
   situation has changed.  All I/O will be done with the given uid using
   userfile. */

void random_initialize(RandomState * state, uid_t uid, const char *filename)
{
    char buf[8192];
    int bytes;
    UserFile uf;

    state->add_position = 0;
    state->next_available_byte = sizeof(state->stir_key);
    state->last_dev_random_usage = 0;

    /* This isn't strictly necessary, but will keep programs like 3rd degree or
       purify silent. */
    memset(state->state, 0, sizeof(state->state));

    /* Get noise from the file. */
    random_add_noise(state, filename, strlen(filename));        /* Use the path. */
    uf = userfile_open(uid, filename, O_RDONLY, 0);
    if (uf != NULL) {
        state->state[0] += (int) uf;
        bytes = userfile_read(uf, buf, sizeof(buf));
        userfile_close(uf);
        if (bytes > 0)
            random_add_noise(state, buf, bytes);
        memset(buf, 0, sizeof(buf));
    } else {
        /* Get all possible noise since we have no seed. */
        random_acquire_environmental_noise(state, uid);
        random_save(state, uid, filename);
    }

    /* Get easily available noise from the environment. */
    random_acquire_light_environmental_noise(state);
}

void random_xor_noise(RandomState * state, unsigned int i, word32 value)
{
    value ^= GET_32BIT(state->state + 4 * i);
    PUT_32BIT(state->state + 4 * i, value);
}

/* Acquires as much environmental noise as it can.  This is probably quite
   sufficient on a unix machine, but might be grossly inadequate on a
   single-user PC or a Macintosh. 

   We test the elapsed real time after each command, and abort if we have
   consumed over 30 seconds.  */

void random_acquire_environmental_noise(RandomState * state, uid_t uid)
{
    time_t start_time;

    /* Record the start time. */
    start_time = time(NULL);

    /* Run these first so that other statistics accumulate from these.  We stop
       collecting more noise when we have spent 30 seconds real time; on a large
       system a single executed command is probably enough, whereas on small
       systems we must use all possible noise sources. */
    random_get_noise_from_command(state, uid, "ps laxww 2>/dev/null");
    if (time(NULL) - start_time < 30)
        random_get_noise_from_command(state, uid, "ps -al 2>/dev/null");
    if (time(NULL) - start_time < 30)
        random_get_noise_from_command(state, uid, "ls -alni /tmp/. 2>/dev/null");
    if (time(NULL) - start_time < 30)
        random_get_noise_from_command(state, uid, "w 2>/dev/null");
    if (time(NULL) - start_time < 30)
        random_get_noise_from_command(state, uid, "netstat -s 2>/dev/null");
    if (time(NULL) - start_time < 30)
        random_get_noise_from_command(state, uid, "netstat -an 2>/dev/null");
    if (time(NULL) - start_time < 30)
        random_get_noise_from_command(state, uid, "netstat -in 2>/dev/null");

    /* Get other easily available noise. */
    random_acquire_light_environmental_noise(state);
}

/* Acquires easily available environmental noise. */

void random_acquire_light_environmental_noise(RandomState * state)
{
    int f;
    char buf[32];
    int len;

    /* Stir first to make all bits depend on all other bits (some of
       them not revealed to callers). */
    random_stir(state);

    /* About every five minutes, mix in some noise from /dev/random. */
    if (time(NULL) - state->last_dev_random_usage > 5 * 60) {
        state->last_dev_random_usage = time(NULL);

        /* If /dev/random is available, read some data from there in non-blocking
           mode and mix it into the pool. */
        f = open("/dev/random", O_RDONLY);
        if (f >= 0) {
            /* Set the descriptor into non-blocking mode. */
#if defined(O_NONBLOCK) && !defined(O_NONBLOCK_BROKEN)
            fcntl(f, F_SETFL, O_NONBLOCK);
#else                           /* O_NONBLOCK && !O_NONBLOCK_BROKEN */
            fcntl(f, F_SETFL, O_NDELAY);
#endif                          /* O_NONBLOCK && !O_NONBLOCK_BROKEN */
            len = read(f, buf, sizeof(buf));
            close(f);
            if (len > 0)
                random_add_noise(state, buf, len);
        }
    }

    /* Get miscellaneous noise from various system parameters and statistics. */
    random_xor_noise(state, (unsigned int) (state->state[0] + 256 * state->state[1]) % (RANDOM_STATE_BYTES / 4), (word32) time(NULL));

#ifdef HAVE_GETTIMEOFDAY
    {
        struct timeval tv;

#ifdef HAVE_NO_TZ_IN_GETTIMEOFDAY
        gettimeofday(&tv);
#else
        gettimeofday(&tv, NULL);
#endif
        random_xor_noise(state, 0, (word32) tv.tv_usec);
        random_xor_noise(state, 1, (word32) tv.tv_sec);
#ifdef HAVE_CLOCK
        random_xor_noise(state, 3, (word32) clock());
#endif                          /* HAVE_CLOCK */
    }
#endif                          /* HAVE_GETTIMEOFDAY */
#ifdef HAVE_TIMES
    {
        struct tms tm;

        random_xor_noise(state, 2, (word32) times(&tm));
        random_xor_noise(state, 4, (word32) (tm.tms_utime ^ (tm.tms_stime << 8) ^ (tm.tms_cutime << 16) ^ (tm.tms_cstime << 24)));
    }
#endif                          /* HAVE_TIMES */
#ifdef HAVE_GETRUSAGE
    {
        struct rusage ru, cru;

        getrusage(RUSAGE_SELF, &ru);
        getrusage(RUSAGE_CHILDREN, &cru);
        random_xor_noise(state, 0, (word32) (ru.ru_utime.tv_usec + cru.ru_utime.tv_usec));
        random_xor_noise(state, 2, (word32) (ru.ru_stime.tv_usec + cru.ru_stime.tv_usec));
        random_xor_noise(state, 5, (word32) (ru.ru_maxrss + cru.ru_maxrss));
        random_xor_noise(state, 6, (word32) (ru.ru_ixrss + cru.ru_ixrss));
        random_xor_noise(state, 7, (word32) (ru.ru_idrss + cru.ru_idrss));
        random_xor_noise(state, 8, (word32) (ru.ru_minflt + cru.ru_minflt));
        random_xor_noise(state, 9, (word32) (ru.ru_majflt + cru.ru_majflt));
        random_xor_noise(state, 10, (word32) (ru.ru_nswap + cru.ru_nswap));
        random_xor_noise(state, 11, (word32) (ru.ru_inblock + cru.ru_inblock));
        random_xor_noise(state, 12, (word32) (ru.ru_oublock + cru.ru_oublock));
        random_xor_noise(state, 13, (word32) ((ru.ru_msgsnd ^ ru.ru_msgrcv ^ ru.ru_nsignals) + (cru.ru_msgsnd ^ cru.ru_msgrcv ^ cru.ru_nsignals)));
        random_xor_noise(state, 14, (word32) (ru.ru_nvcsw + cru.ru_nvcsw));
        random_xor_noise(state, 15, (word32) (ru.ru_nivcsw + cru.ru_nivcsw));
    }
#endif                          /* HAVE_GETRUSAGE */
    random_xor_noise(state, 11, (word32) getpid());
    random_xor_noise(state, 12, (word32) getppid());
    random_xor_noise(state, 10, (word32) getuid());
    random_xor_noise(state, 10, (word32) (getgid() << 16));
#ifdef _POSIX_CHILD_MAX
    random_xor_noise(state, 13, (word32) (_POSIX_CHILD_MAX << 16));
#endif                          /* _POSIX_CHILD_MAX */
#ifdef CLK_TCK
    random_xor_noise(state, 14, (word32) (CLK_TCK << 16));
#endif                          /* CLK_TCK */

    random_stir(state);
}

/* Executes the given command, and processes its output as noise.  The 
   command will be run via userfile with the given uid. */

void random_get_noise_from_command(RandomState * state, uid_t uid, const char *cmd)
{
    char line[1000];
    UserFile uf;

    uf = userfile_popen(uid, cmd, "r");
    if (uf == NULL)
        return;
    while (userfile_gets(line, sizeof(line), uf))
        random_add_noise(state, line, strlen(line));
    userfile_pclose(uf);
    memset(line, 0, sizeof(line));
}

/* Adds the contents of the buffer as noise. */

void random_add_noise(RandomState * state, const void *buf, unsigned int bytes)
{
    unsigned int pos = state->add_position;
    const char *input = buf;

    while (bytes > 0) {
        if (pos >= RANDOM_STATE_BYTES) {
            pos = 0;
            random_stir(state);
        }
        state->state[pos] ^= *input;
        input++;
        bytes--;
        pos++;
    }
    state->add_position = pos;
}

/* Stirs the random pool to consume any newly acquired noise or to get more
   random numbers.

   This works by encrypting the data in the buffer in CFB mode with MD5 as
   the cipher. */

void random_stir(RandomState * state)
{
    word32 iv[4];
    unsigned int i;

    /* Start IV from last block of random pool. */
    iv[0] = GET_32BIT(state->state);
    iv[1] = GET_32BIT(state->state + 4);
    iv[2] = GET_32BIT(state->state + 8);
    iv[3] = GET_32BIT(state->state + 12);

    /* First CFB pass. */
    for (i = 0; i < RANDOM_STATE_BYTES; i += 16) {
        MD5Transform(iv, state->stir_key);
        iv[0] ^= GET_32BIT(state->state + i);
        PUT_32BIT(state->state + i, iv[0]);
        iv[1] ^= GET_32BIT(state->state + i + 4);
        PUT_32BIT(state->state + i + 4, iv[1]);
        iv[2] ^= GET_32BIT(state->state + i + 8);
        PUT_32BIT(state->state + i + 8, iv[2]);
        iv[3] ^= GET_32BIT(state->state + i + 12);
        PUT_32BIT(state->state + i + 12, iv[3]);
    }

    /* Get new key. */
    memcpy(state->stir_key, state->state, sizeof(state->stir_key));

    /* Second CFB pass. */
    for (i = 0; i < RANDOM_STATE_BYTES; i += 16) {
        MD5Transform(iv, state->stir_key);
        iv[0] ^= GET_32BIT(state->state + i);
        PUT_32BIT(state->state + i, iv[0]);
        iv[1] ^= GET_32BIT(state->state + i + 4);
        PUT_32BIT(state->state + i + 4, iv[1]);
        iv[2] ^= GET_32BIT(state->state + i + 8);
        PUT_32BIT(state->state + i + 8, iv[2]);
        iv[3] ^= GET_32BIT(state->state + i + 12);
        PUT_32BIT(state->state + i + 12, iv[3]);
    }

    memset(iv, 0, sizeof(iv));

    state->add_position = 0;

    /* Some data in the beginning is not returned to aboid giving an observer
       complete knowledge of the contents of our random pool. */
    state->next_available_byte = sizeof(state->stir_key);
}

/* Returns a random byte.  Stirs the random pool if necessary.  Acquires
   new environmental noise approximately every five minutes. */

unsigned int random_get_byte(RandomState * state)
{
    if (state->next_available_byte >= RANDOM_STATE_BYTES) {
        /* Get some easily available noise.  More importantly, this stirs
           the pool. */
        random_acquire_light_environmental_noise(state);
    }
    assert(state->next_available_byte < RANDOM_STATE_BYTES);
    return state->state[state->next_available_byte++];
}

/* Saves random data in a disk file.  This is used to create a file that
   can be used as a random seed on future runs.  Only half of the random
   data in our pool is written to the file to avoid an observer being
   able to deduce the contents of our random pool from the file.
   I/O will be done using the give uid with userfile. */

void random_save(RandomState * state, uid_t uid, const char *filename)
{
    char buf[RANDOM_STATE_BYTES / 2];   /* Save only half of its bits. */
    int i;
    UserFile uf;

    /* Get some environmental noise to make it harder to predict previous
       values from saved bits (besides, we have now probably consumed some
       resources so the noise may be really useful).  This also stirs
       the pool.  We also clear the last /dev/random usage time to take
       noise from there if available. */
    state->last_dev_random_usage = 0;
    random_acquire_light_environmental_noise(state);

    /* Get as many bytes as is half the size of the pool.  I am assuming
       this will get enough randomness for it to be very useful, but will
       not reveal enough to make it possible to determine previous or future
       returns by the generator. */
    for (i = 0; i < sizeof(buf); i++)
        buf[i] = random_get_byte(state);

    /* Again get a little noise and stir it to mix the unrevealed half with 
       those bits that have been saved to a file.  There should be enough 
       unrevealed bits (plus the new noise) to make it infeasible to try to 
       guess future values from the saved bits. */
    random_acquire_light_environmental_noise(state);

    /* Create and write the file.  Failure to create the file is silently
       ignored. */
    uf = userfile_open(uid, filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (uf != NULL) {
        /* Creation successful.  Write data to the file. */
        userfile_write(uf, buf, sizeof(buf));
        userfile_close(uf);
    }
    memset(buf, 0, sizeof(buf));
}

/* Clears the random number generator data structures. */

void random_clear(RandomState * state)
{
    memset(state, 0, sizeof(*state));
}
