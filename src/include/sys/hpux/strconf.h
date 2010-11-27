/*****************************************************************************

 @(#) $Id: strconf.h,v 1.1.2.1 2009-06-21 11:26:45 brian Exp $

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2010  Monavacon Limited <http://www.monavacon.com/>
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

 -----------------------------------------------------------------------------

 Last Modified $Date: 2009-06-21 11:26:45 $ by $Author: brian $

 -----------------------------------------------------------------------------

 $Log: strconf.h,v $
 Revision 1.1.2.1  2009-06-21 11:26:45  brian
 - added files to new distro

 *****************************************************************************/

#ifndef __SYS_HPUX_STRCONF_H__
#define __SYS_HPUX_STRCONF_H__

#ifndef __SYS_STRCONF_H__
#warning "Do not include sys/aix/strconf.h directly, include sys/strconf.h instead."
#endif

#ifndef __KERNEL__
#error "Do not use kernel headers for user space programs"
#endif				/* __KERNEL__ */

#ifndef __EXTERN_INLINE
#define __EXTERN_INLINE extern __inline__
#endif

#ifndef __HPUX_EXTERN_INLINE
#define __HPUX_EXTERN_INLINE __EXTERN_INLINE streamscall
#endif				/* __EXTERN_INLINE */

#ifndef __HPUX_EXTERN
#define __HPUX_EXTERN extern streamscall
#endif				/* __AIX_EXTERN_INLINE */

#ifndef _HPUX_SOURCE
#warning "_HPUX_SOURCE not defined but HPUX strconf.h included"
#endif

#include <sys/openss7/config.h>

#if defined(CONFIG_STREAMS_COMPAT_HPUX) || defined(CONFIG_STREAMS_COMPAT_HPUX_MODULE)

typedef struct stream_inst {
	char *name;			/* name of driver or module */
	int inst_major;			/* major number for driver (-1 dynamic or module) */
	struct streamtab inst_str_tab;	/* current streams tab entry */
	unsigned int inst_flags;	/* flags, e.g. STR_SVR4_OPEN, STR_IS_MODULE */
	int inst_sync_level;		/* sync level defined by stream.h */
	char inst_sync_info[FMNAMESZ + 1];	/* elsewhere sync param described in MP */
} streams_info_t;

__HPUX_EXTERN int str_install_HPUX(struct stream_inst *inst);
__HPUX_EXTERN int str_uninstall(struct stream_inst *inst);

#ifndef str_install
#define str_install(__inst) str_install_HPUX(__inst)
#endif

#else
#ifdef _HPUX_SOURCE
#warning "_HPUX_SOURCE defined but not CONFIG_STREAMS_COMPAT_HPUX"
#endif
#endif				/* CONFIG_STREAMS_COMPAT_HPUX */

#endif				/* __SYS_HPUX_STRCONF_H__ */
