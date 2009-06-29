/*****************************************************************************

 @(#) $RCSfile: pipe.c,v $ $Name:  $($Revision: 1.1.2.2 $) $Date: 2009-06-29 07:35:44 $

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2009  Monavacon Limited <http://www.monavacon.com/>
 Copyright (c) 2001-2008  OpenSS7 Corporation <http://www.openss7.com/>
 Copyright (c) 1997-2001  Brian F. G. Bidulock <bidulock@openss7.org>

 All Rights Reserved.

 This program is free software: you can redistribute it and/or modify it under
 the terms of the GNU Affero General Public License as published by the Free
 Software Foundation, version 3 of the license.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more
 details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>, or
 write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA
 02139, USA.

 -----------------------------------------------------------------------------

 U.S. GOVERNMENT RESTRICTED RIGHTS.  If you are licensing this Software on
 behalf of the U.S. Government ("Government"), the following provisions apply
 to you.  If the Software is supplied by the Department of Defense ("DoD"), it
 is classified as "Commercial Computer Software" under paragraph 252.227-7014
 of the DoD Supplement to the Federal Acquisition Regulations ("DFARS") (or any
 successor regulations) and the Government is acquiring only the license rights
 granted herein (the license rights customarily provided to non-Government
 users).  If the Software is supplied to any unit or agency of the Government
 other than DoD, it is classified as "Restricted Computer Software" and the
 Government's rights in the Software are defined in paragraph 52.227-19 of the
 Federal Acquisition Regulations ("FAR") (or any successor regulations) or, in
 the cases of NASA, in paragraph 18.52.227-86 of the NASA Supplement to the FAR
 (or any successor regulations).

 -----------------------------------------------------------------------------

 Commercial licensing and support of this software is available from OpenSS7
 Corporation at a fee.  See http://www.openss7.com/

 -----------------------------------------------------------------------------

 Last Modified $Date: 2009-06-29 07:35:44 $ by $Author: brian $

 -----------------------------------------------------------------------------

 $Log: pipe.c,v $
 Revision 1.1.2.2  2009-06-29 07:35:44  brian
 - SVR 4.2 => SVR 4.2 MP

 Revision 1.1.2.1  2009-06-21 11:20:50  brian
 - added files to new distro

 *****************************************************************************/

#ident "@(#) $RCSfile: pipe.c,v $ $Name:  $($Revision: 1.1.2.2 $) $Date: 2009-06-29 07:35:44 $"

static char const ident[] = "$RCSfile: pipe.c,v $ $Name:  $($Revision: 1.1.2.2 $) $Date: 2009-06-29 07:35:44 $";

#include <linux/autoconf.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>

#include <sys/stream.h>
#include <sys/strconf.h>
#include <sys/strsubr.h>
#include <sys/ddi.h>

#include "sys/config.h"
#include "src/kernel/strreg.h"
#include "src/kernel/strsched.h"
#include "src/kernel/strsad.h"	/* for autopush */
#include "src/modules/sth.h"
#include "pipe.h"		/* extern verification */

#define PIPE_DESCRIP	"UNIX SYSTEM V RELEASE 4.2 FAST STREAMS FOR LINUX"
#define PIPE_COPYRIGHT	"Copyright (c) 2008-2009  Monavacon Limited.  All Rights Reserved."
#define PIPE_REVISION	"LfS $RCSfile: pipe.c,v $ $Name:  $($Revision: 1.1.2.2 $) $Date: 2009-06-29 07:35:44 $"
#define PIPE_DEVICE	"SVR 4.2 MP STREAMS-based PIPEs"
#define PIPE_CONTACT	"Brian Bidulock <bidulock@openss7.org>"
#define PIPE_LICENSE	"GPL"
#define PIPE_BANNER	PIPE_DESCRIP	"\n" \
			PIPE_COPYRIGHT	"\n" \
			PIPE_REVISION	"\n" \
			PIPE_DEVICE	"\n" \
			PIPE_CONTACT	"\n"
#define PIPE_SPLASH	PIPE_DEVICE	" - " \
			PIPE_REVISION	"\n"

#ifdef CONFIG_STREAMS_PIPE_MODULE
MODULE_AUTHOR(PIPE_CONTACT);
MODULE_DESCRIPTION(PIPE_DESCRIP);
MODULE_SUPPORTED_DEVICE(PIPE_DEVICE);
MODULE_LICENSE(PIPE_LICENSE);
#ifdef MODULE_VERSION
MODULE_VERSION(__stringify(PACKAGE_RPMEPOCH) ":" PACKAGE_VERSION "." PACKAGE_RELEASE
	       PACKAGE_PATCHLEVEL "-" PACKAGE_RPMRELEASE PACKAGE_RPMEXTRA2);
#endif				/* MODULE_VERSION */
#endif				/* CONFIG_STREAMS_PIPE_MODULE */

#ifndef CONFIG_STREAMS_PIPE_NAME
//#define CONFIG_STREAMS_PIPE_NAME "pipe"
#error "CONFIG_STREAMS_PIPE_NAME must be defined."
#endif
#ifndef CONFIG_STREAMS_PIPE_MODID
//#define CONFIG_STREAMS_PIPE_MODID 8
#error "CONFIG_STREAMS_PIPE_MODID must be defined."
#endif
#ifndef CONFIG_STREAMS_PIPE_MAJOR
//#define CONFIG_STREAMS_PIPE_MAJOR 0
#error "CONFIG_STREAMS_PIPE_MAJOR must be defined."
#endif

#ifdef MODULE
#ifdef MODULE_ALIAS
MODULE_ALIAS("streams-pipe");
#endif				/* MODULE_ALIAS */
#endif				/* MODULE */

#ifndef CONFIG_STREAMS_PIPE_MODULE
static
#endif
modID_t modid = CONFIG_STREAMS_PIPE_MODID;

#ifdef CONFIG_STREAMS_PIPE_MODULE
#ifndef module_param
MODULE_PARM(modid, "h");
#else
module_param(modid, ushort, 0444);
#endif
MODULE_PARM_DESC(modid, "Module id number for STREAMS-based PIPEs (0 for allocation).");
#endif				/* CONFIG_STREAMS_PIPE_MODULE */

#ifdef MODULE
#ifdef MODULE_ALIAS
MODULE_ALIAS("streams-modid-" __stringify(CONFIG_STREAMS_PIPE_MODID));
MODULE_ALIAS("streams-driver-pipe");
#endif				/* MODULE_ALIAS */
#endif				/* MODULE */

#ifndef CONFIG_STREAMS_PIPE_MODULE
static
#endif
major_t major = CONFIG_STREAMS_PIPE_MAJOR;

#ifdef CONFIG_STREAMS_PIPE_MODULE
#ifndef module_param
MODULE_PARM(major, "h");
#else
module_param(major, uint, 0444);
#endif
MODULE_PARM_DESC(major, "Major device number for STREAMS-based PIPEs (0 for allocation).");
#endif				/* CONFIG_STREAMS_PIPE_MODULE */

#ifdef MODULE
#ifdef MODULE_ALIAS
MODULE_ALIAS("char-major-" __stringify(CONFIG_STREAMS_PIPE_MAJOR) "-*");
MODULE_ALIAS("/dev/pipe");
MODULE_ALIAS("streams-major-" __stringify(CONFIG_STREAMS_PIPE_MAJOR));
MODULE_ALIAS("/dev/streams/pipe");
MODULE_ALIAS("/dev/streams/pipe/*");
MODULE_ALIAS("/dev/streams/clone/pipe");
#endif				/* MODULE_ALIAS */
#endif				/* MODULE */

static struct module_info pipe_minfo = {
	.mi_idnum = CONFIG_STREAMS_PIPE_MODID,
	.mi_idname = CONFIG_STREAMS_PIPE_NAME,
	.mi_minpsz = STRMINPSZ,
	.mi_maxpsz = STRMAXPSZ,
	.mi_hiwat = STRHIGH,
	.mi_lowat = STRLOW,
};

static struct module_stat pipe_rstat __attribute__ ((__aligned__(SMP_CACHE_BYTES)));
static struct module_stat pipe_wstat __attribute__ ((__aligned__(SMP_CACHE_BYTES)));

static struct qinit pipe_rinit = {
	.qi_putp = strrput,
	.qi_srvp = strrsrv,
	.qi_qopen = str_open,
	.qi_qclose = str_close,
	.qi_minfo = &pipe_minfo,
	.qi_mstat = &pipe_rstat,
};

static struct qinit pipe_winit = {
	.qi_putp = strwput,
	.qi_srvp = strwsrv,
	.qi_minfo = &pipe_minfo,
	.qi_mstat = &pipe_wstat,
};

#ifdef CONFIG_STREAMS_PIPE_MODULE
static
#endif
struct streamtab pipeinfo = {
	.st_rdinit = &pipe_rinit,
	.st_wrinit = &pipe_winit,
};

/* 
 *  -------------------------------------------------------------------------
 *
 *  INITIALIZATION
 *
 *  -------------------------------------------------------------------------
 */

static struct cdevsw pipe_cdev = {
	.d_name = CONFIG_STREAMS_PIPE_NAME,
	.d_str = &pipeinfo,
	.d_flag = D_CLONE | D_MP,
	.d_fop = &strm_f_ops,
	.d_mode = S_IFIFO | S_IRUGO | S_IWUGO,
	.d_kmod = THIS_MODULE,
};

#ifdef CONFIG_STREAMS_PIPE_MODULE
static
#endif
int __init
pipeinit(void)
{
	int err;

#ifdef CONFIG_STREAMS_PIPE_MODULE
	printk(KERN_INFO PIPE_BANNER);
#else
	printk(KERN_INFO PIPE_SPLASH);
#endif
	pipe_minfo.mi_idnum = modid;
	if ((err = register_strdev(&pipe_cdev, major)) < 0)
		return (err);
	if (major == 0 && err > 0)
		major = err;
	return (0);
};

#ifdef CONFIG_STREAMS_PIPE_MODULE
static
#endif
void __exit
pipeexit(void)
{
	unregister_strdev(&pipe_cdev, major);
};

#ifdef CONFIG_STREAMS_PIPE_MODULE
module_init(pipeinit);
module_exit(pipeexit);
#endif
