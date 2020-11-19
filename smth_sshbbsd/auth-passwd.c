/*

auth-passwd.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Sat Mar 18 05:11:38 1995 ylo

Password authentication.  This file contains the functions to check whether
the password is valid for the user.

*/

/*
 * $Id: auth-passwd.c,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: auth-passwd.c,v $
 * Revision 1.1.1.1  2009-03-04 06:33:27  bmybbs
 * bmysrc
 *
 * Revision 1.1.1.1  2003/04/18 15:00:21  clearboy
 * update on 20051031
 * by clearboy 
 * for transfering the source codes from main site to the experimental site 
 * for the first time.
 *
 *
 * Revision 1.7  2003/04/18 15:00:21  lepton
 * logattempt统一处理
 *
 * Revision 1.6  2002/10/07 10:04:51  lepton
 * ssh用户登录禁止guest和new
 * cvs: ----------------------------------------------------------------------
 *
 * Revision 1.5  2002/10/05 05:24:25  lepton
 * 1. CHAT_CLOAK -> ARBITRATE 防止歧义
 * 2. 现在 站务 SPECIAL2 仲裁 监察 立法会 都可以在关站的时候进入
 *
 * Revision 1.4  2002/10/02 07:47:16  lepton
 * ssh登录加上判断NOLOGIN文件
 *
 * Revision 1.3  2002/10/01 14:32:16  lepton
 * 1.加入sshbbsd配置文件
 * 2.现在密码验证工作正常
 * 3.可能退出的一些环境清理有问题 以及执行外部程序比如穿梭
 *
 * Revision 1.2  2002/10/01 11:50:26  lepton
 * 现在sshbbsd倒是在我这里能运行了
 * 不过还远没有实用...
 * 需要加入密码验证 外部程序处理 ip显示等等
 * 目前只能演示一下而已
 *
 * Revision 1.1.1.1  2002/10/01 09:42:06  ylsdd
 * 水木底sshbbsd导入
 * 然后慢慢改吧
 *
 * Revision 1.3  2002/08/04 11:39:40  kcn
 * format c
 *
 * Revision 1.2  2002/08/04 11:08:44  kcn
 * format C
 *
 * Revision 1.1.1.1  2002/04/27 05:47:26  kxn
 * no message
 *
 * Revision 1.3  2002/04/25 10:47:37  kxn
 * removed libBBS.a
 * fixed logattempt failure
 * added username display in proctitle
 *
 * Revision 1.2  2001/08/22 07:51:47  bbsdev
 * *** empty log message ***
 *
 * Revision 1.1  2001/07/04 06:07:07  bbsdev
 * bbs sshd
 *
 * Revision 1.23  1999/04/29 07:52:02  tri
 * 	Replaced OSF1/C2 security support with more complete SIA
 *         (Security Integration Architecture) support by Tom Woodburn.
 *
 * Revision 1.22  1999/02/23 07:06:14  tri
 *      Fixed compilation errors.
 *
 * Revision 1.21  1999/02/22 08:13:56  tri
 *      Final fixes for 1.2.27.
 *
 * Revision 1.20  1999/02/21 19:51:52  ylo
 *      Intermediate commit of ssh1.2.27 stuff.
 *      Main change is sprintf -> snprintf; however, there are also
 *      many other changes.
 *
 * Revision 1.19  1998/07/08 01:44:46  kivinen
 *      Added one missing space.
 *
 * Revision 1.18  1998/07/08 00:36:44  kivinen
 *      Changed to use PASSWD_PATH. Better HPUX TCB AUTH support.
 *
 * Revision 1.17  1998/06/11 00:03:38  kivinen
 *      Added username to /bin/password commands.
 *
 * Revision 1.16  1998/05/23  20:19:40  kivinen
 *      Removed #define uint32 rpc_unt32, because md5 uint32 is now
 *      md5_uint32. Changed () -> (void). Changed osf1c2_getprpwent
 *      function to return true/false. Added
 *      forced_empty_passwd_change support.
 *
 * Revision 1.15  1998/05/11  21:27:47  kivinen
 *      Set correct_passwd to contain 255...255 so even if some
 *      function doesn't set it, it cannot contain empty password.
 *
 * Revision 1.14  1998/04/30 01:50:20  kivinen
 *      Added code that will force /bin/passwd command if password is
 *      expired.
 *
 * Revision 1.13  1998/03/27 16:53:09  kivinen
 *      Added aix authenticate function support.
 *
 * Revision 1.12  1998/01/02 06:14:31  kivinen
 *      Fixed kerberos ticket name handling. Added OSF C2 account
 *      locking and expiration support.
 *
 * Revision 1.11  1997/04/17 03:57:05  kivinen
 *      Kept FILE: prefix in kerberos ticket filename as DCE cache
 *      code requires it (patch from Doug Engert <DEEngert@anl.gov>).
 *
 * Revision 1.10  1997/04/05 21:45:25  kivinen
 *      Changed verify_krb_v5_tgt to take *error_code instead of
 *      error_code.
 *
 * Revision 1.9  1997/03/27 03:09:20  kivinen
 *      Added kerberos patches from Glenn Machin.
 *
 * Revision 1.8  1997/03/26 06:59:18  kivinen
 *      Changed uid 0 to UID_ROOT.
 *
 * Revision 1.7  1997/03/19 15:57:21  kivinen
 *      Added SECURE_RPC, SECURE_NFS and NIS_PLUS support from Andy
 *      Polyakov <appro@fy.chalmers.se>.
 *
 * Revision 1.6  1996/10/30 04:22:43  kivinen
 *      Added #ifdef HAVE_SHADOW_H around shadow.h including.
 *
 * Revision 1.5  1996/10/29 22:33:59  kivinen
 *      log -> log_msg.
 *
 * Revision 1.4  1996/10/08 13:50:44  ttsalo
 *      Allow long passwords for HP-UX TCB authentication
 *
 * Revision 1.3  1996/09/08 17:36:51  ttsalo
 *      Patches for HPUX 10.x shadow passwords from
 *      vincent@ucthpx.uct.ac.za (Russell Vincent) merged.
 *
 * Revision 1.2  1996/02/18 21:53:45  ylo
 *      Test for HAVE_ULTRIX_SHADOW_PASSWORDS instead of ultrix
 *      (mips-dec-mach3 has ultrix defined, but does not support the
 *      shadow password stuff).
 *
 * Revision 1.1.1.1  1996/02/18 21:38:12  ylo
 *      Imported ssh-1.2.13.
 *
 * Revision 1.8  1995/09/27  02:10:34  ylo
 *      Added support for SCO unix shadow passwords.
 *
 * Revision 1.7  1995/09/10  22:44:41  ylo
 *      Added OSF/1 C2 extended security stuff.
 *
 * Revision 1.6  1995/08/21  23:20:29  ylo
 *      Fixed a typo.
 *
 * Revision 1.5  1995/08/19  13:15:56  ylo
 *      Changed securid code to initialize itself only once.
 *
 * Revision 1.4  1995/08/18  22:42:51  ylo
 *      Added General Dynamics SecurID support from Donald McKillican
 *      <dmckilli@qc.bell.ca>.
 *
 * Revision 1.3  1995/07/13  01:12:34  ylo
 *      Removed the "Last modified" header.
 *
 * Revision 1.2  1995/07/13  01:09:50  ylo
 *      Added cvs log.
 *
 * $Endlog$
 */

#include "includes.h"
#include "packet.h"
#include "ssh.h"
#include "servconf.h"
#include "xmalloc.h"
#include "bbs.h"

extern struct userec currentuser;
/* Tries to authenticate the user using password.  Returns true if
   authentication succeeds. */
int auth_password(const char *server_user, const char *password)
{
    if (password[0] == '\0'||!strcasecmp(server_user,"guest")||!strcasecmp(server_user,"new"))
        return 0;

    if( !access("NOLOGIN",F_OK) && !HAS_PERM(PERM_SPEC, currentuser))
	return 0;

    if (!ytht_crypt_checkpasswd(currentuser.passwd, password)) {
	time_t t=time(0);
        logattempt(server_user, get_remote_ipaddr(),"SSH", t);
        return 0;
    }
    return 1;
}
