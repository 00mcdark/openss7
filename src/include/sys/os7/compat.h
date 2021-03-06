/*****************************************************************************

 @(#) src/include/sys/os7/compat.h

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2015  Monavacon Limited <http://www.monavacon.com/>
 Copyright (c) 2001-2008  OpenSS7 Corporation <http://www.openss7.com/>
 Copyright (c) 1997-2001  Brian F. G. Bidulock <bidulock@openss7.org>

 All Rights Reserved.

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU Affero General Public License as published by the Free
 Software Foundation; version 3 of the License.

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

 *****************************************************************************/

#ifndef __LOCAL_COMPAT_H__
#define __LOCAL_COMPAT_H__

#define __OPTIMIZE__ 1

#include <sys/openss7/config.h>

#ifndef HAVE_KTYPE_BOOL
#include <stdbool.h>
#endif

#ifdef LINUX
#ifdef NEED_LINUX_AUTOCONF_H
#include NEED_LINUX_AUTOCONF_H
#endif
#include <linux/version.h>	/* for UTS_RELEASE */
#ifdef HAVE_KINC_LINUX_UTSRELEASE_H
#include <linux/utsrelease.h>	/* for UTS_RELEASE */
#endif
#ifdef HAVE_KINC_GENERATED_UTSRELEASE_H
#include <generated/utsrelease.h>	/* for UTS_RELEASE */
#endif
#include <linux/compiler.h>
#include <linux/module.h>
#include <linux/init.h>
#endif				/* LINUX */

#ifdef LINUX
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#ifdef HAVE_KINC_LINUX_SCHED_SIGNAL_H
#include <linux/sched/signal.h>
#endif
#ifdef HAVE_KINC_ASM_SYSTEM_H
#include <asm/system.h>
#endif
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kdev_t.h>
#ifdef HAVE_KINC_LINUX_SECURITY_H
#include <linux/security.h>
#endif
#ifndef HAVE_KTYPE_IRQRETURN_T
typedef void irqreturn_t;
#endif
#undef IRQ_NONE
#define IRQ_NONE	(0)
#undef IRQ_HANDLED
#define IRQ_HANDLED	(1)
#undef IRQ_RETVAL
#define IRQ_RETVAL(x)	((x) != 0)
#endif				/* LINUX */

#include <asm/byteorder.h>

#ifndef __MOD_INC_USE_COUNT
#undef MOD_INC_USE_COUNT
#define MOD_INC_USE_COUNT
#endif
#ifndef __MOD_DEC_USE_COUNT
#undef MOD_DEC_USE_COUNT
#define MOD_DEC_USE_COUNT
#endif

#ifndef EXPORT_SYMBOL_NOVERS
#define EXPORT_SYMBOL_NOVERS(__sym) EXPORT_SYMBOL(__sym)
#endif

#define _RP streamscall

#define _OS7_SOURCE

#include <sys/stream.h>

#include <sys/cmn_err.h>
#include <sys/kmem.h>
#include <sys/dki.h>
#include <sys/ddi.h>

#include <sys/strconf.h>
#include <sys/strlog.h>

/* minor device number range */
#define NMINORS		((1UL<<16)-1)

#ifdef LINUX
#ifdef MODULE
#define MODULE_STATIC STATIC
#else				/* MODULE */
#define MODULE_STATIC
#endif				/* MODULE */
#else				/* LINUX */
#define MODULE_STATIC STATIC
#endif				/* LINUX */

#undef htonl
#undef htons
#undef ntohl
#undef ntohs

#define htonl(x) ___htonl(x)
#define htons(x) ___htons(x)
#define ntohl(x) ___ntohl(x)
#define ntohs(x) ___ntohs(x)

#endif				/* __LOCAL_COMPAT_H__ */
