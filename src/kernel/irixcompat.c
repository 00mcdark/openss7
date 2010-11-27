/*****************************************************************************

 @(#) $RCSfile: irixcompat.c,v $ $Name:  $($Revision: 1.1.2.2 $) $Date: 2009-07-21 11:06:16 $

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2010  Monavacon Limited <http://www.monavacon.com/>
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

 Last Modified $Date: 2009-07-21 11:06:16 $ by $Author: brian $

 -----------------------------------------------------------------------------

 $Log: irixcompat.c,v $
 Revision 1.1.2.2  2009-07-21 11:06:16  brian
 - changes from release build

 Revision 1.1.2.1  2009-06-21 11:37:16  brian
 - added files to new distro

 *****************************************************************************/

static char const ident[] = "$RCSfile: irixcompat.c,v $ $Name:  $($Revision: 1.1.2.2 $) $Date: 2009-07-21 11:06:16 $";

/* 
 *  This is my solution for those who don't want to inline GPL'ed functions or
 *  who don't use optimizations when compiling or specifies
 *  -fnoinline-functions or something of the like.  This file implements all
 *  of the extern inlines from the header files by just including the header
 *  files with the functions declared 'inline' instead of 'extern inline'.
 *
 *  There are implemented here in a separate object, out of the way of the
 *  modules that don't use them.
 */

#define __IRIX_EXTERN_INLINE __inline__ streamscall __unlikely
#define __IRIX_EXTERN streamscall

#define _IRIX_SOURCE

#include "sys/os7/compat.h"

#define IRIXCOMP_DESCRIP	"UNIX SYSTEM V RELEASE 4.2 FAST STREAMS FOR LINUX"
#define IRIXCOMP_COPYRIGHT	"Copyright (c) 2008-2010  Monavacon Limited.  All Rights Reserved."
#define IRIXCOMP_REVISION	"LfS $RCSfile: irixcompat.c,v $ $Name:  $($Revision: 1.1.2.2 $) $Date: 2009-07-21 11:06:16 $"
#define IRIXCOMP_DEVICE		"IRIX 6.5.17 Compatibility"
#define IRIXCOMP_CONTACT	"Brian Bidulock <bidulock@openss7.org>"
#define IRIXCOMP_LICENSE	"GPL"
#define IRIXCOMP_BANNER		IRIXCOMP_DESCRIP	"\n" \
				IRIXCOMP_COPYRIGHT	"\n" \
				IRIXCOMP_REVISION	"\n" \
				IRIXCOMP_DEVICE		"\n" \
				IRIXCOMP_CONTACT	"\n"
#define IRIXCOMP_SPLASH		IRIXCOMP_DEVICE		" - " \
				IRIXCOMP_REVISION	"\n"

#ifdef CONFIG_STREAMS_COMPAT_IRIX_MODULE
MODULE_AUTHOR(IRIXCOMP_CONTACT);
MODULE_DESCRIPTION(IRIXCOMP_DESCRIP);
MODULE_SUPPORTED_DEVICE(IRIXCOMP_DEVICE);
MODULE_LICENSE(IRIXCOMP_LICENSE);
#if defined MODULE_ALIAS
MODULE_ALIAS("streams-irixcompat");
#endif
#ifdef MODULE_VERSION
MODULE_VERSION(__stringify(PACKAGE_RPMEPOCH) ":" PACKAGE_VERSION "." PACKAGE_RELEASE
	       PACKAGE_PATCHLEVEL "-" PACKAGE_RPMRELEASE PACKAGE_RPMEXTRA2);
#endif
#endif

#ifdef CONFIG_STREAMS_COMPAT_IRIX_MODULE
static
#endif
int __init
irixcomp_init(void)
{
#ifdef CONFIG_STREAMS_COMPAT_IRIX_MODULE
	printk(KERN_INFO IRIXCOMP_BANNER);
#else
	printk(KERN_INFO IRIXCOMP_SPLASH);
#endif
	return (0);
}

#ifdef CONFIG_STREAMS_COMPAT_IRIX_MODULE
static
#endif
void __exit
irixcomp_exit(void)
{
	return;
}

#ifdef HAVE_ICMN_ERR_EXPORT
#undef icmn_err
#define icmn_err icmn_err_
#endif
__IRIX_EXTERN_INLINE void icmn_err(int err_lvl, const char *fmt, va_list args);

EXPORT_SYMBOL(icmn_err);	/* irix/ddi.h */
#ifdef HAVE_ICMN_ERR_EXPORT
#undef icmn_err
#endif
/* gcc 3.4.3 can't handle inlining with variable argument list */
__IRIX_EXTERN void
cmn_err_tag(int sequence, int err_lvl, const char *fmt, ... /* args */ )
{
	va_list args;

	va_start(args, fmt);
	icmn_err(err_lvl, fmt, args);
	va_end(args);
	return;
}

EXPORT_SYMBOL(cmn_err_tag);	/* irix/ddi.h */

#ifdef CONFIG_STREAMS_COMPAT_IRIX_MODULE
module_init(irixcomp_init);
module_exit(irixcomp_exit);
#endif
