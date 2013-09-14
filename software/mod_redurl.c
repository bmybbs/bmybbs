#define WANT_BASENAME_MATCH
/* ====================================================================
 * Copyright (c) 1996-1999 The Apache Group.  All rights reserved.
 *
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
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Group and was originally based
 * on public domain software written at the National Center for
 * Supercomputing Applications, University of Illinois, Urbana-Champaign.
 * For more information on the Apache Group and the Apache HTTP server
 * project, please see <http://www.apache.org/>.
 *
 */

#include "httpd.h"
#include "http_core.h"
#include "http_config.h"
#include "http_log.h"

#include <iconv.h>

/* mod_url.c - by Won-kyu Park <wkpark@chem.skku.ac.kr>
 * 
 * based mod_speling.c Alexei Kosut <akosut@organic.com> June, 1996
 *
 * Activate it with "CheckURL encoding On"
 */

MODULE_VAR_EXPORT module redurl_module;

typedef struct {
    int enabled;
} urlconfig;

/*
 * Create a configuration specific to this module for a server or directory
 * location, and fill it with the default settings.
 *
 * The API says that in the absence of a merge function, the record for the
 * closest ancestor is used exclusively.  That's what we want, so we don't
 * bother to have such a function.
 */

static void *mkconfig(pool *p)
{
    urlconfig *cfg = ap_pcalloc(p, sizeof(urlconfig));

    cfg->enabled = 0;
    return cfg;
}

/*
 * Respond to a callback to create configuration record for a server or
 * vhost environment.
 */
static void *create_mconfig_for_server(pool *p, server_rec *s)
{
    return mkconfig(p);
}

/*
 * Respond to a callback to create a config record for a specific directory.
 */
static void *create_mconfig_for_directory(pool *p, char *dir)
{
    return mkconfig(p);
}

/*
 * Handler for the CheckURL encoding directive, which is FLAG.
 */
static const char *set_redurl(cmd_parms *cmd, void *mconfig, int arg)
{
    urlconfig *cfg = (urlconfig *) mconfig;

    cfg->enabled = arg;
    return NULL;
}

/*
 * Define the directives specific to this module.  This structure is referenced
 * later by the 'module' structure.
 */
static const command_rec redurl_cmds[] =
{
    { "CheckURL", set_redurl, NULL, OR_OPTIONS, FLAG,
      "whether or not to fix mis-encoded URL requests" },
    { NULL }
};

static int check_redurl(request_rec *r)
{
    urlconfig *cfg;
    char *good, *bad, *postgood, *url;
    int filoc, dotloc, urlen, pglen;
    DIR *dirp;
    struct DIR_TYPE *dir_entry;
    array_header *candidates = NULL;

    cfg = ap_get_module_config(r->per_dir_config, &redurl_module);
    if (!cfg->enabled) {
        return DECLINED;
    }

    /* We only want to worry about GETs */
    if (r->method_number != M_GET) {
        return DECLINED;
    }

    /* We've already got a file of some kind or another */
    if (r->proxyreq || (r->finfo.st_mode != 0)) {
        return DECLINED;
    }

    /* This is a sub request - don't mess with it */
    if (r->main) {
        return DECLINED;
    }

    /*
     * The request should end up looking like this:
     * r->uri: /correct-url/mispelling/more
     * r->filename: /correct-file/mispelling r->path_info: /more
     *
     * So we do this in steps. First break r->filename into two pieces
     */

    filoc = ap_rind(r->filename, '/');
    /*
     * Don't do anything if the request doesn't contain a slash, or
     * requests "/" 
     */
    if (filoc == -1 || strcmp(r->uri, "/") == 0) {
        return DECLINED;
    }

    /* good = /correct-file */
    good = ap_pstrndup(r->pool, r->filename, filoc);
    /* bad = mispelling */
    bad = ap_pstrdup(r->pool, r->filename + filoc + 1);
    /* postgood = mispelling/more */
    postgood = ap_pstrcat(r->pool, bad, r->path_info, NULL);

    urlen = strlen(r->uri);
    pglen = strlen(postgood);

    /* Check to see if the URL pieces add up */
    if (strcmp(postgood, r->uri + (urlen - pglen))) {
        return DECLINED;
    }

    /* url = /correct-url */
    url = ap_pstrndup(r->pool, r->uri, (urlen - pglen));

    /* 시작 */
    ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, r,
		 "Orig URL: %s %s url:%s",
		 r->uri, good, url);

    {
        static iconv_t cd = 0;
//        const char *src = "안녕하세요";
//        const char *src = "휟켝襤8�~T";
        const char *src = r->uri;
        char buf[2048]="\0", *to; /* XXX */
        size_t len,flen, tlen,t;
        if (cd == 0) {
           cd = iconv_open("GB2312", "UTF-8");
        }
        flen = len = strlen(src);
        tlen = 2*flen;
        to= buf;
        // to = malloc(tlen);
        t=iconv(cd, &src, &flen, &to, &tlen);

        tlen=strlen(buf);
        ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, r,
		 "URL: from uri %s to %s(%d->%d):%d",
		 r->uri,buf,len,tlen,t);
       if (t != -1 && t == 0 && len != 0 && tlen != len) {
                  /*    ^^^^^  바로 이 부분입니다.*/
                  /*  t == 0 이어야 euckr로 인코딩 된 url로의 redirect가 발생하는 조건인데..  */
                  /*  아래 조건대로라면 정상  URL일 경우에만 redirect가 발생하는 겁니다..*/
                  /* t != 0 으로 바꾸어서 다시 컴파일 하면 정상 작동합니다. */
	/* t== -1일 경우는 URL이 euckr로 이미 인코딩 된 경우 */
	/* t== 0 일 경우는 정상 */
	/* flen == tlen 인 경우는 URL이 ascii일 경우 */
	char *nuri;

            nuri = ap_pstrcat(r->pool, buf,
			      r->parsed_uri.query ? "?" : "",
			      r->parsed_uri.query ? r->parsed_uri.query : "",
			      NULL);

            ap_table_setn(r->headers_out, "Location",
			  ap_construct_url(r->pool, nuri, r));

            ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, r,
			 "Fixed URL: %s to %s",
			 r->uri, nuri);

            return HTTP_MOVED_PERMANENTLY;
       } else
            return DECLINED;
    } 
    /* 끝 */

    return OK;
}

module MODULE_VAR_EXPORT redurl_module =
{
    STANDARD_MODULE_STUFF,
    NULL,                       /* initializer */
    create_mconfig_for_directory,  /* create per-dir config */
    NULL,                       /* merge per-dir config */
    create_mconfig_for_server,  /* server config */
    NULL,                       /* merge server config */
    redurl_cmds,               /* command table */
    NULL,                       /* handlers */
    NULL,                       /* filename translation */
    NULL,                       /* check_user_id */
    NULL,                       /* check auth */
    NULL,                       /* check access */
    NULL,                       /* type_checker */
    check_redurl,              /* fixups */
    NULL,                       /* logger */
    NULL,                       /* header parser */
    NULL,                       /* child_init */
    NULL,                       /* child_exit */
    NULL                        /* post read-request */
};

