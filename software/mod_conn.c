/* -----------------------------------------------------------------------
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * 4. The names "Apache Server" and "Apache Group" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache"
 *    nor may "Apache" appear in their names without prior written
 *    permission of the Apache Group.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the Apache Group
 *    for use in the Apache HTTP server project (http://www.apache.org/)."
 *
 * THIS SOFTWARE IS PROVIDED BY THE APACHE GROUP ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE APACHE GROUP OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 */

// -------------------------------------------------------------------
// mod_conn.c
//   Control the maximum connection number from the same IP address
//
// Copyight (C) Holly Lee Dec 1999 in Shanghai
// -------------------------------------------------------------------

#include "httpd.h"
#include "http_config.h"
#include "http_request.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#define  MAX_IP_SLOTS   256
#define  IP_ADDR_LEN     16

#define  LOCK_FILE      "/tmp/mod_conn.lck"

// #define __DEBUG__

// -------------------------------------------------------------------
// Type definition
// -------------------------------------------------------------------
struct conn_dir_conf {
       int maxClientsPerHost;
};

struct conn_ip {
       char ip[IP_ADDR_LEN];
       int  count;
};
       
struct conn_ip_pool {
       struct conn_ip ip_pool[MAX_IP_SLOTS];
};

int initialized = 0;
struct conn_ip_pool * ipPool = 0;          
int lockfd = 0;

extern int ap_daemons_limit; // MaxClients

module MODULE_VAR_EXPORT conn_module;

// -------------------------------------------------------------------
// Initialize
// -------------------------------------------------------------------
static void conn_init(server_rec * server, pool * p)
{
       // Create new lock file
       lockfd = open(LOCK_FILE, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
       if ( lockfd == -1 ) {
          munmap(ipPool, sizeof(struct conn_ip_pool));
          initialized = 0;
          return;
       }

       // Allocate the memory
       ipPool = ap_palloc(p, sizeof(struct conn_ip_pool));

       // Fill with zero
       memset(ipPool, 0, sizeof(struct conn_ip_pool));

       // Fill the file
       write(lockfd, ipPool, sizeof(struct conn_ip_pool));
       
#ifdef __DEBUG__
{
       FILE * fp = fopen("/dev/tty10", "w");
       fprintf(fp, "creat OK\n");
       fclose(fp);
}
#endif
       // mmap - MAP_SHARED can be inherited in child process
       // map from /dev/zero will be filled with 0
       ipPool = (struct conn_ip_pool *)mmap(0, sizeof(struct conn_ip_pool),
                                            PROT_READ | PROT_WRITE,
                                            MAP_SHARED, lockfd, 0);
       if ( ipPool == (struct conn_ip_pool *)-1 ) {
          initialized = 0;
#ifdef __DEBUG__
{
       FILE * fp = fopen("/dev/tty10", "w");
       fprintf(fp, "mmap: %s, euid: %d\n", sys_errlist[errno], geteuid());
       fclose(fp);
}
#endif

          // close(zerofd);
          return;
       }

       // close(zerofd);

#ifdef __DEBUG__
{
       FILE * fp = fopen("/dev/tty10", "w");
       fprintf(fp, "mmap OK\n");
       fclose(fp);
}
#endif

       initialized = 1;
}

// ------------------------------------------------------------------
// Create Dir Config
// ------------------------------------------------------------------
static void * conn_create_dir_config(pool * p, char * dir)
{
       struct conn_dir_conf * pConnConf =
              (struct conn_dir_conf *)ap_palloc(p, sizeof(struct conn_dir_conf));
              
       pConnConf->maxClientsPerHost = 0;
       
#ifdef __DEBUG__
{
       FILE * fp = fopen("/dev/tty10", "w");
       fprintf(fp, "access_checker: dir = %s\n", dir);
       fclose(fp);
}
#endif
       
       return pConnConf;
}
              
// -------------------------------------------------------------------
// Command Handler
// -------------------------------------------------------------------
static const char * set_max_clients_per_host(cmd_parms *cmd, void *dummy, char *arg)
{
       struct conn_dir_conf * pConnConf = (struct conn_dir_conf *)dummy;
#ifdef __DEBUG__
{
       FILE * fp = fopen("/dev/tty10", "w");
       fprintf(fp, "Command - pConnConf = %x, arg = %s\n", pConnConf, arg);
       fclose(fp);
}
#endif
       pConnConf->maxClientsPerHost = atoi(arg);
       return 0;
}

// -----------------------------------------------------------------
// Find IP address in conn_ip_pool, return index, -1 not found
// -----------------------------------------------------------------
static int _find_ip_slot(char * ip)
{
       int i;
       for ( i = 0; i < MAX_IP_SLOTS; i++ ) {
           if ( strcmp(ip, ipPool->ip_pool[i].ip) == 0 && ipPool->ip_pool[i].count > 0 )
              return i;  // found
       }
       
       return -1;
}

static int _find_empty_slot()
{
       int i;
       for ( i = 0; i < MAX_IP_SLOTS; i++ ) {
           if ( ipPool->ip_pool[i].count <= 0 )
              return i;  // found
       }
       
       return -1;
}

static void _lock()
{
       struct flock lock = {
              F_WRLCK,       // l_type
              SEEK_SET,      // l_whence
              0              // l_len
       };
        
       // Wait the Lock
       fcntl(lockfd, F_SETLKW, &lock);
}

static void _unlock()
{
       struct flock lock = {
              F_UNLCK,       // l_type
              SEEK_SET,      // l_whence
              0              // l_len
       };
        
       // Wait the Lock
       fcntl(lockfd, F_SETLKW, &lock);
}

       
// -----------------------------------------------------------------
// Access Checker
// -----------------------------------------------------------------
static int conn_access_checker(request_rec * r)
{                            
       int slot;
       
       struct conn_dir_conf * pConnConf =
              (struct conn_dir_conf *)ap_get_module_config(r->per_dir_config, &conn_module);
              
#ifdef __DEBUG__
{
       FILE * fp = fopen("/dev/tty10", "w");
       fprintf(fp, "access_checker: initialized = %d\n", initialized);
       fclose(fp);
}
#endif
       if ( initialized == 0 )  return OK;
       
       if ( pConnConf->maxClientsPerHost <= 0 )
          return OK;

       // Lock
       _lock();
       
#ifdef __DEBUG__
{
       FILE * fp = fopen("/dev/tty10", "w");
       fprintf(fp, "access_checker: locked\n");
       fclose(fp);
}
#endif
       // Finding
       slot = _find_ip_slot(r->connection->remote_ip);
       // not found
       if ( slot == -1 ) {
          int newslot = _find_empty_slot();
          // No empty!
          if ( newslot == -1 ) {
             _unlock();
             return FORBIDDEN;
          }
          
#ifdef __DEBUG__
{
       FILE * fp = fopen("/dev/tty10", "w");
       fprintf(fp, "access_checker: get new slot %d\n", newslot);
       fclose(fp);
}
#endif
          // found empty
          strcpy(ipPool->ip_pool[newslot].ip, r->connection->remote_ip);
          ipPool->ip_pool[newslot].count = 1;
          
          _unlock();
          
#ifdef __DEBUG__
{
       FILE * fp = fopen("/dev/tty10", "w");
       fprintf(fp, "access_checker: unlock\n");
       fclose(fp);
}
#endif
          return OK;
       }
       
       // found
       
       // Check internal redirection
#ifdef __DEBUG__
{
       FILE * fp = fopen("/dev/tty10", "w");
       fprintf(fp, "access checker: r = %x, r->main = %x, r->prev = %x, r->next = %x\n",
               r, r->main, r->prev, r->next);

       fclose(fp);
}
#endif
       if ( r->prev != 0 || r->main != 0 )
          return OK;

       // Full.......
       if ( ipPool->ip_pool[slot].count >= pConnConf->maxClientsPerHost ) {
#ifdef __DEBUG__
{
       FILE * fp = fopen("/dev/tty10", "w");
       fprintf(fp, "Declined by too many connections. count = %d, max = %d\n",
               ipPool->ip_pool[slot].count, pConnConf->maxClientsPerHost );
       fclose(fp);
}
#endif
          _unlock(); 
          return FORBIDDEN;
       }

       //
       ipPool->ip_pool[slot].count++;
#ifdef __DEBUG__
{
       FILE * fp = fopen("/dev/tty10", "w");
       fprintf(fp, "Add count - %d\n", ipPool->ip_pool[slot].count);
       fclose(fp);
}
#endif
       
       _unlock();
       
       return OK;
}

// -----------------------------------------------------------------
// Logger - do count--
// -----------------------------------------------------------------
static int conn_logger(request_rec * r)
{
       // Do count-- on logger routine
       int slot;
       
       struct conn_dir_conf * pConnConf = 
              (struct conn_dir_conf *)ap_get_module_config(r->per_dir_config, &conn_module);
              
#ifdef __DEBUG__
{
       FILE * fp = fopen("/dev/tty10", "w");
       fprintf(fp, "access_checker: initialized = %d\n", initialized);
       fclose(fp);
}
#endif
       if ( initialized == 0 )   return DECLINED;
       
       _lock();
       
       // Finding
       slot = _find_ip_slot(r->connection->remote_ip);
       if ( slot == -1 ) {
          _unlock();
          return DECLINED;
       }
       
       ipPool->ip_pool[slot].count--;
#ifdef __DEBUG__
{
       FILE * fp = fopen("/dev/tty10", "w");
       fprintf(fp, "Remove count - %d\n", ipPool->ip_pool[slot].count);
       fclose(fp);
}
#endif
       _unlock();
       
       return OK;
}      

// -----------------------------------------------------------------
// Commands
// -----------------------------------------------------------------
static const command_rec conn_cmds[] = {
       { "MaxClientsPerHost", set_max_clients_per_host, 0, OR_OPTIONS, TAKE1,
         "Set permitted maximum connections from the same client." },
       { 0 }
};

// ---------------------------------------------------------------
// Module
// ---------------------------------------------------------------
module MODULE_VAR_EXPORT conn_module = {
       STANDARD_MODULE_STUFF,             // Standard stuff
       conn_init,                         // init routine
       conn_create_dir_config,            // create dir config
       0,                                 // merge dir config
       0,                                 // create server config
       0,                                 // merge server config
       conn_cmds,                         // struct command_rec * cmds
       0,                                 // struct handler_rec * handlers
       0,                                 // translate handler
       0,                                 // ap_check_user_id
       0,                                 // auth_checker
       conn_access_checker,               // access_checker
       0,                                 // type checker
       0,                                 // fixer_upper
       conn_logger,                       // logger
       0,                                 // header_parser
       0,                                 // child_init
       0,                                 // child_exit
       0                                  // post_read_request
};


