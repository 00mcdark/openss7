/*****************************************************************************

 @(#) $RCSfile: sfx.c,v $ $Name:  $($Revision: 1.1.2.5 $) $Date: 2011-09-02 08:46:36 $

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2013  Monavacon Limited <http://www.monavacon.com/>
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

 Last Modified $Date: 2011-09-02 08:46:36 $ by $Author: brian $

 -----------------------------------------------------------------------------

 $Log: sfx.c,v $
 Revision 1.1.2.5  2011-09-02 08:46:36  brian
 - sync up lots of repo and build changes from git

 Revision 1.1.2.4  2011-05-31 09:46:06  brian
 - new distros

 Revision 1.1.2.3  2010-11-28 14:21:36  brian
 - remove #ident, protect _XOPEN_SOURCE

 Revision 1.1.2.2  2009-06-29 07:35:44  brian
 - SVR 4.2 => SVR 4.2 MP

 Revision 1.1.2.1  2009-06-21 11:20:53  brian
 - added files to new distro

 *****************************************************************************/

static char const ident[] = "$RCSfile: sfx.c,v $ $Name:  $($Revision: 1.1.2.5 $) $Date: 2011-09-02 08:46:36 $";

#ifdef NEED_LINUX_AUTOCONF_H
#include NEED_LINUX_AUTOCONF_H
#endif
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>

#include <sys/stream.h>
#include <sys/strconf.h>
#include <sys/strsubr.h>
#include <sys/ddi.h>

#include "sys/config.h"
#include "src/kernel/strsched.h"
#include "src/kernel/strsad.h"	/* for autopush */
#include "src/modules/sth.h"
#include "src/drivers/fifo.h"	/* for fifo stuff */

#define SFX_DESCRIP	"SVR 4.2 STREAMS-based FIFOs Driver"
#define SFX_EXTRA	"Part of UNIX SYSTEM V RELEASE 4.2 FAST STREAMS FOR LINUX"
#define SFX_COPYRIGHT	"Copyright (c) 2008-2013  Monavacon Limited.  All Rights Reserved."
#define SFX_REVISION	"LfS $RCSfile: sfx.c,v $ $Name:  $($Revision: 1.1.2.5 $) $Date: 2011-09-02 08:46:36 $"
#define SFX_DEVICE	"SVR 4.2 MP STREAMS-based FIFOs"
#define SFX_CONTACT	"Brian Bidulock <bidulock@openss7.org>"
#define SFX_LICENSE	"GPL"
#define SFX_BANNER	SFX_DESCRIP	"\n" \
			SFX_EXTRA	"\n" \
			SFX_COPYRIGHT	"\n" \
			SFX_REVISION	"\n" \
			SFX_DEVICE	"\n" \
			SFX_CONTACT	"\n"
#define SFX_SPLASH	SFX_DESCRIP	" - " \
			SFX_COPYRIGHT	"\n"

#ifdef CONFIG_STREAMS_SFX_MODULE
MODULE_AUTHOR(SFX_CONTACT);
MODULE_DESCRIPTION(SFX_DESCRIP);
MODULE_SUPPORTED_DEVICE(SFX_DEVICE);
MODULE_LICENSE(SFX_LICENSE);
#ifdef MODULE_VERSION
MODULE_VERSION(PACKAGE_ENVR);
#endif				/* MODULE_VERSION */
#endif				/* CONFIG_STREAMS_SFX_MODULE */

#ifndef CONFIG_STREAMS_SFX_NAME
//#define CONFIG_STREAMS_SFX_NAME "sfx"
#error "CONFIG_STREAMS_SFX_NAME must be defined."
#endif
#ifndef CONFIG_STREAMS_SFX_MODID
//#define CONFIG_STREAMS_SFX_MODID 9
#error "CONFIG_STREAMS_SFX_MODID must be defined."
#endif
#ifndef CONFIG_STREAMS_SFX_MAJOR
//#define CONFIG_STREAMS_SFX_MAJOR 0
#error "CONFIG_STREAMS_SFX_MAJOR must be defined."
#endif

#ifdef MODULE
#ifdef MODULE_ALIAS
MODULE_ALIAS("streams-sfx");
#endif				/* MODULE_ALIAS */
#endif				/* MODULE */

#ifndef CONFIG_STREAMS_SFX_MODULE
static
#endif
modID_t modid = CONFIG_STREAMS_SFX_MODID;

#ifdef CONFIG_STREAMS_SFX_MODULE
#ifndef module_param
MODULE_PARM(modid, "h");
#else
module_param(modid, ushort, 0444);
#endif
MODULE_PARM_DESC(modid, "Module id number for STREAMS-based FIFOs (0 for allocation).");
#endif				/* CONFIG_STREAMS_SFX_MODULE */

#ifdef MODULE
#ifdef MODULE_ALIAS
MODULE_ALIAS("streams-modid-" __stringify(CONFIG_STREAMS_SFX_MODID));
MODULE_ALIAS("streams-driver-sfx");
#endif				/* MODULE_ALIAS */
#endif				/* MODULE */

#ifndef CONFIG_STREAMS_SFX_MODULE
static
#endif
major_t major = CONFIG_STREAMS_SFX_MAJOR;

#ifdef CONFIG_STREAMS_SFX_MODULE
#ifndef module_param
MODULE_PARM(major, "h");
#else
module_param(major, uint, 0444);
#endif
MODULE_PARM_DESC(major, "Major device number for STREAMS-based FIFOs (0 for allocation).");
#endif				/* CONFIG_STREAMS_SFX_MODULE */

#ifdef MODULE
#ifdef MODULE_ALIAS
MODULE_ALIAS("char-major-" __stringify(CONFIG_STREAMS_CLONE_MAJOR) "-" __stringify(CONFIG_STREAMS_SFX_MAJOR));
MODULE_ALIAS("/dev/sfx");
MODULE_ALIAS("devname:sfx");
MODULE_ALIAS("streams-major-" __stringify(CONFIG_STREAMS_SFX_MAJOR));
MODULE_ALIAS("/dev/streams/sfx");
MODULE_ALIAS("/dev/streams/sfx/*");
MODULE_ALIAS("/dev/streams/clone/sfx");
#endif				/* MODULE_ALIAS */
#endif				/* MODULE */

static struct module_info sfx_minfo = {
	.mi_idnum = CONFIG_STREAMS_SFX_MODID,
	.mi_idname = CONFIG_STREAMS_SFX_NAME,
	.mi_minpsz = STRMINPSZ,
	.mi_maxpsz = STRMAXPSZ,
	.mi_hiwat = STRHIGH,
	.mi_lowat = STRLOW,
};

static struct module_stat sfx_rstat __attribute__ ((__aligned__(SMP_CACHE_BYTES)));
static struct module_stat sfx_wstat __attribute__ ((__aligned__(SMP_CACHE_BYTES)));

static struct qinit sfx_rqinit = {
	.qi_putp = strrput,
	.qi_srvp = strrsrv,
	.qi_qopen = str_open,
	.qi_qclose = str_close,
	.qi_minfo = &sfx_minfo,
	.qi_mstat = &sfx_rstat,
};

static struct qinit sfx_wqinit = {
	.qi_putp = strwput,
	.qi_srvp = strwsrv,
	.qi_minfo = &sfx_minfo,
	.qi_mstat = &sfx_wstat,
};

#ifdef CONFIG_STREAMS_SFX_MODULE
static
#endif
struct streamtab sfxinfo = {
	.st_rdinit = &sfx_rqinit,
	.st_wrinit = &sfx_wqinit,
};

static struct cdevsw sfx_cdev = {
	.d_name = CONFIG_STREAMS_SFX_NAME,
	.d_str = &sfxinfo,
	.d_flag = D_MP,
	.d_fop = &strm_f_ops,
	.d_mode = S_IFIFO | S_IRUGO | S_IWUGO,
	.d_kmod = THIS_MODULE,
};

#ifdef CONFIG_STREAMS_SFX_MODULE
static
#endif
int __init
sfxinit(void)
{
	int err;

#ifdef CONFIG_STREAMS_SFX_MODULE
	printk(KERN_INFO SFX_BANNER);
#else
	printk(KERN_INFO SFX_SPLASH);
#endif
	sfx_minfo.mi_idnum = modid;
	if ((err = register_strdev(&sfx_cdev, major)) < 0)
		return (err);
	if (major == 0 && err > 0)
		major = err;
	return (0);
};

#ifdef CONFIG_STREAMS_SFX_MODULE
static
#endif
void __exit
sfxexit(void)
{
	unregister_strdev(&sfx_cdev, major);
};

#ifdef CONFIG_STREAMS_SFX_MODULE
module_init(sfxinit);
module_exit(sfxexit);
#endif
