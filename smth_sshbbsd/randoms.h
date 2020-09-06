/*

random.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Sat Mar  4 14:49:05 1995 ylo

Cryptographically strong random number generator.

*/

/*
 * $Id: randoms.h,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: randoms.h,v $
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
 * Revision 1.1.1.1  1996/02/18 21:38:10  ylo
 * 	Imported ssh-1.2.13.
 *
 * Revision 1.3  1995/09/13  12:00:02  ylo
 * 	Changes to make this work on Cray.
 *
 * Revision 1.2  1995/07/13  01:29:28  ylo
 * 	Removed "Last modified" header.
 * 	Added cvs log.
 *
 * $Endlog$
 */

#ifndef RANDOM_H
#define RANDOM_H

#include "md5.h"

#define RANDOM_STATE_BITS	8192
#define RANDOM_STATE_BYTES	(RANDOM_STATE_BITS / 8)

/* Structure for the random state. */
typedef struct {
    unsigned char state[RANDOM_STATE_BYTES];    /* Pool of random data. */
    unsigned char stir_key[64]; /* Extra data for next stirring. */
    unsigned int next_available_byte;   /* Index of next available byte. */
    unsigned int add_position;  /* Index to add noise. */
    time_t last_dev_random_usage;       /* Time of last /dev/random usage. */
} RandomState;

/* Initializes the random number generator, loads any random information
   from the given file, and acquires as much environmental noise as it
   can to initialize the random number generator.  More noise can be
   acquired later by calling random_add_noise + random_stir, or by
   calling random_get_environmental_noise again later when the environmental
   situation has changed.  All I/O will be done with the given uid. */
void random_initialize(RandomState * state, uid_t uid, const char *filename);

/* Acquires as much environmental noise as it can.  This is probably quite
   sufficient on a unix machine, but might be grossly inadequate on a
   single-user PC or a Macintosh.  This call random_stir automatically. 
   This call may take many seconds to complete on a busy system. 
   This will perform any commands with the given uid using userfile. */
void random_acquire_environmental_noise(RandomState * state, uid_t uid);

/* Acquires easily available noise from the environment. */
void random_acquire_light_environmental_noise(RandomState * state);

/* Executes the given command, and processes its output as noise.
   random_stir should be called after this.  The command will be called
   with the given uid via userfile. */
void random_get_noise_from_command(RandomState * state, uid_t uid, const char *cmd);

/* Adds the contents of the buffer as noise.  random_stir should be called
   after this. */
void random_add_noise(RandomState * state, const void *buf, unsigned int bytes);

/* Stirs the random pool to consume any newly acquired noise or to get more
   random numbers.  This should be called after adding noise to properly
   mix the noise into the random pool. */
void random_stir(RandomState * state);

/* Returns a random byte.  Stirs the random pool if necessary.  Acquires
   new environmental noise approximately every five minutes. */
unsigned int random_get_byte(RandomState * state);

/* Saves some random bits in the file so that it can be used as a source
   of randomness for later runs.  I/O will be done with the given uid using
   userfile. */
void random_save(RandomState * state, uid_t uid, const char *filename);

/* Zeroes and frees any data structures associated with the random number
   generator. */
void random_clear(RandomState * state);

#endif                          /* RANDOM_H */
