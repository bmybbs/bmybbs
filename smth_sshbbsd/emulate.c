#include "includes.h"
#include "ssh.h"
#include "emulate.h"

unsigned long emulation_information = 0;

/* check_emulation: take the remote party's version number as
   arguments and return our possibly modified version number back
   (relevant only for clients).

   Return values:
   EMULATE_VERSION_OK means we can work together

   EMULATE_VERSION_TOO_OLD if the other party has too old version
   which we cannot emulate,

   EMULATE_MAJOR_VERSION_MISMATCH if the other party has different
   major version and thus will probably not understand anything we
   say, and

   EMULATE_VERSION_NEWER if the other party has never code than we
   have.

   */

int check_emulation(int remote_major, int remote_minor, int *return_major, int *return_minor)
{
    if (return_major)
        *return_major = PROTOCOL_MAJOR;
    if (return_minor) {
        if (remote_minor < PROTOCOL_MINOR)
            *return_minor = remote_minor;
        else
            *return_minor = PROTOCOL_MINOR;
    }

    if (remote_major < PROTOCOL_MAJOR)
        return EMULATE_MAJOR_VERSION_MISMATCH;

    if (remote_major == 1 && remote_minor == 0)
        return EMULATE_VERSION_TOO_OLD; /* We no longer support 1.0. */

    if (remote_major == 1 && remote_minor <= 3) {
        debug("Old channel code will be emulated.");
        emulation_information |= EMULATE_OLD_CHANNEL_CODE;
    }

    if (remote_major == 1 && remote_minor <= 4) {
        emulation_information |= EMULATE_OLD_AGENT_BUG;
        debug("Agent forwarding disabled (remote protocol too old)");
    }

    if (remote_major > PROTOCOL_MAJOR || (remote_major == PROTOCOL_MAJOR && remote_minor > PROTOCOL_MINOR)) {
        /* The remote software is newer than we. If we are the client,
           no matter - the server will decide. If we are the server, we
           cannot emulate a newer client and decide to stop. */
        return EMULATE_VERSION_NEWER;
    }

    return EMULATE_VERSION_OK;
}
