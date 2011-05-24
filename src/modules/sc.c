/*****************************************************************************

 @(#) $RCSfile: sc.c,v $ $Name:  $($Revision: 1.1.2.3 $) $Date: 2010-11-28 14:22:05 $

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2011  Monavacon Limited <http://www.monavacon.com/>
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

 Last Modified $Date: 2010-11-28 14:22:05 $ by $Author: brian $

 -----------------------------------------------------------------------------

 $Log: sc.c,v $
 Revision 1.1.2.3  2010-11-28 14:22:05  brian
 - remove #ident, protect _XOPEN_SOURCE

 Revision 1.1.2.2  2009-06-29 07:35:46  brian
 - SVR 4.2 => SVR 4.2 MP

 Revision 1.1.2.1  2009-06-21 11:40:33  brian
 - added files to new distro

 *****************************************************************************/

static char const ident[] = "$RCSfile: sc.c,v $ $Name:  $($Revision: 1.1.2.3 $) $Date: 2010-11-28 14:22:05 $";

/* 
 *  This is SC, a STREAMS Configuration module for Linux Fast-STREAMS.  This
 *  is an auxilliary module to the SAD (STREAMS Administrative Driver) and can
 *  be pushed over that module or over the NULS (Null STREAM) driver.
 */

#ifdef NEED_LINUX_AUTOCONF_H
#include <linux/autoconf.h>
#endif
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>

#include <sys/kmem.h>
#include <sys/cmn_err.h>
#include <sys/stream.h>
#include <sys/strconf.h>
#include <sys/strsubr.h>
#include <sys/ddi.h>

#include <sys/sc.h>

#include "sys/config.h"
#include "src/kernel/strlookup.h"

#define SC_DESCRIP	"UNIX SYSTEM V RELEASE 4.2 FAST STREAMS FOR LINUX"
#define SC_COPYRIGHT	"Copyright (c) 2008-2011  Monavacon Limited.  All Rights Reserved."
#define SC_REVISION	"LfS $RCSfile: sc.c,v $ $Name:  $($Revision: 1.1.2.3 $) $Date: 2010-11-28 14:22:05 $"
#define SC_DEVICE	"SVR 4.2 MP STREAMS STREAMS Configuration Module (SC)"
#define SC_CONTACT	"Brian Bidulock <bidulock@openss7.org>"
#define SC_LICENSE	"GPL"
#define SC_BANNER	SC_DESCRIP	"\n" \
			SC_COPYRIGHT	"\n" \
			SC_REVISION	"\n" \
			SC_DEVICE	"\n" \
			SC_CONTACT	"\n"
#define SC_SPLASH	SC_DEVICE	" - " \
			SC_REVISION	"\n"

#ifdef CONFIG_STREAMS_SC_MODULE
MODULE_AUTHOR(SC_CONTACT);
MODULE_DESCRIPTION(SC_DESCRIP);
MODULE_SUPPORTED_DEVICE(SC_DEVICE);
MODULE_LICENSE(SC_LICENSE);
#ifdef MODULE_VERSION
MODULE_VERSION(__stringify(PACKAGE_RPMEPOCH) ":" PACKAGE_VERSION "." PACKAGE_RELEASE
	       PACKAGE_PATCHLEVEL "-" PACKAGE_RPMRELEASE PACKAGE_RPMEXTRA2);
#endif				/* MODULE_VERSION */
#endif				/* CONFIG_STREAMS_SC_MODULE */

#ifndef CONFIG_STREAMS_SC_NAME
#error CONFIG_STREAMS_SC_NAME must be defined.
#endif
#ifndef CONFIG_STREAMS_SC_MODID
#error CONFIG_STREAMS_SC_MODID must be defined.
#endif

#ifdef MODULE
#ifdef MODULE_ALIAS
MODULE_ALIAS("streams-sc");
#endif				/* MODULE_ALIAS */
#endif				/* MODULE */

#ifndef CONFIG_STREAMS_SC_MODULE
static
#endif
modID_t modid = CONFIG_STREAMS_SC_MODID;

#ifdef CONFIG_STREAMS_SC_MODULE
#ifndef module_param
MODULE_PARM(modid, "h");
#else
module_param(modid, ushort, 0444);
#endif
MODULE_PARM_DESC(modid, "Module ID for SC.");
#endif				/* CONFIG_STREAMS_SC_MODULE */

#ifdef MODULE
#ifdef MODULE_ALIAS
MODULE_ALIAS("streams-modid-" __stringify(CONFIG_STREAMS_SC_MODID));
MODULE_ALIAS("streams-module-sc");
#endif				/* MODULE_ALIAS */
#endif				/* MODULE */

static struct module_info sc_minfo = {
	.mi_idnum = CONFIG_STREAMS_SC_MODID,
	.mi_idname = CONFIG_STREAMS_SC_NAME,
	.mi_minpsz = STRMINPSZ,
	.mi_maxpsz = STRMAXPSZ,
	.mi_hiwat = STRHIGH,
	.mi_lowat = STRLOW,
};

static struct module_stat sc_rstat __attribute__ ((__aligned__(SMP_CACHE_BYTES)));
static struct module_stat sc_wstat __attribute__ ((__aligned__(SMP_CACHE_BYTES)));

static size_t
sc_mlist_copy(long major, struct streamtab *st, caddr_t _mlist, const int reset, const uint flag)
{
	struct module_info *info;
	struct module_stat *stat;

#ifdef __LP64__
	if (flag == IOC_ILP32) {
		struct sc_mlist32 *mlist = (typeof(mlist)) _mlist;

		if ((mlist->major = major) != -1) {
			if (st->st_wrinit && (info = st->st_wrinit->qi_minfo)) {
				strncpy(mlist->name, info->mi_idname, FMNAMESZ + 1);
				mlist->mi[0].index = 0x8;
				mlist->mi[0].mi_idnum = info->mi_idnum;
				strncpy(mlist->mi[0].mi_idname, info->mi_idname, FMNAMESZ + 1);
				mlist->mi[0].mi_minpsz = info->mi_minpsz;
				mlist->mi[0].mi_maxpsz = info->mi_maxpsz;
				mlist->mi[0].mi_hiwat = info->mi_hiwat;
				mlist->mi[0].mi_lowat = info->mi_lowat;
				if ((stat = st->st_wrinit->qi_mstat)) {
					mlist->ms[0].index = 0x8;
					mlist->ms[0].ms_pcnt = stat->ms_pcnt;
					mlist->ms[0].ms_scnt = stat->ms_scnt;
					mlist->ms[0].ms_ocnt = stat->ms_ocnt;
					mlist->ms[0].ms_ccnt = stat->ms_ccnt;
					mlist->ms[0].ms_acnt = stat->ms_acnt;
					mlist->ms[0].ms_flags = stat->ms_flags;
					mlist->ms[0].ms_xsize = stat->ms_xsize;
				}
			}
			if (st->st_rdinit && (info = st->st_rdinit->qi_minfo)) {
				if (info == st->st_wrinit->qi_minfo)
					mlist->mi[0].index |= 0x4;
				else
					mlist->mi[1].index = 0x4;
				mlist->mi[1].mi_idnum = info->mi_idnum;
				strncpy(mlist->mi[1].mi_idname, info->mi_idname, FMNAMESZ + 1);
				mlist->mi[1].mi_minpsz = info->mi_minpsz;
				mlist->mi[1].mi_maxpsz = info->mi_maxpsz;
				mlist->mi[1].mi_hiwat = info->mi_hiwat;
				mlist->mi[1].mi_lowat = info->mi_lowat;
				if ((stat = st->st_rdinit->qi_mstat)) {
					if (stat == st->st_wrinit->qi_mstat)
						mlist->ms[0].index |= 0x4;
					else
						mlist->ms[1].index = 0x4;
					mlist->ms[1].ms_pcnt = stat->ms_pcnt;
					mlist->ms[1].ms_scnt = stat->ms_scnt;
					mlist->ms[1].ms_ocnt = stat->ms_ocnt;
					mlist->ms[1].ms_ccnt = stat->ms_ccnt;
					mlist->ms[1].ms_acnt = stat->ms_acnt;
					mlist->ms[1].ms_flags = stat->ms_flags;
					mlist->ms[1].ms_xsize = stat->ms_xsize;
				}
			}
			if (st->st_muxwinit && (info = st->st_muxwinit->qi_minfo)) {
				if (info == st->st_wrinit->qi_minfo)
					mlist->mi[0].index |= 0x2;
				else if (info == st->st_rdinit->qi_minfo)
					mlist->mi[1].index |= 0x2;
				else
					mlist->mi[2].index = 0x2;
				mlist->mi[2].mi_idnum = info->mi_idnum;
				strncpy(mlist->mi[2].mi_idname, info->mi_idname, FMNAMESZ + 1);
				mlist->mi[2].mi_minpsz = info->mi_minpsz;
				mlist->mi[2].mi_maxpsz = info->mi_maxpsz;
				mlist->mi[2].mi_hiwat = info->mi_hiwat;
				mlist->mi[2].mi_lowat = info->mi_lowat;
				if ((stat = st->st_muxwinit->qi_mstat)) {
					if (stat == st->st_wrinit->qi_mstat)
						mlist->ms[0].index |= 0x2;
					else if (stat == st->st_rdinit->qi_mstat)
						mlist->ms[1].index |= 0x2;
					else
						mlist->ms[2].index = 0x2;
					mlist->ms[2].ms_pcnt = stat->ms_pcnt;
					mlist->ms[2].ms_scnt = stat->ms_scnt;
					mlist->ms[2].ms_ocnt = stat->ms_ocnt;
					mlist->ms[2].ms_ccnt = stat->ms_ccnt;
					mlist->ms[2].ms_acnt = stat->ms_acnt;
					mlist->ms[2].ms_flags = stat->ms_flags;
					mlist->ms[2].ms_xsize = stat->ms_xsize;
				}
			}
			if (st->st_muxrinit && (info = st->st_muxrinit->qi_minfo)) {
				if (info == st->st_wrinit->qi_minfo)
					mlist->mi[0].index |= 0x1;
				else if (info == st->st_rdinit->qi_minfo)
					mlist->mi[1].index |= 0x1;
				else if (info == st->st_muxwinit->qi_minfo)
					mlist->mi[2].index |= 0x1;
				else
					mlist->mi[3].index = 0x1;
				mlist->mi[3].mi_idnum = info->mi_idnum;
				strncpy(mlist->mi[3].mi_idname, info->mi_idname, FMNAMESZ + 1);
				mlist->mi[3].mi_minpsz = info->mi_minpsz;
				mlist->mi[3].mi_maxpsz = info->mi_maxpsz;
				mlist->mi[3].mi_hiwat = info->mi_hiwat;
				mlist->mi[3].mi_lowat = info->mi_lowat;
				if ((stat = st->st_muxrinit->qi_mstat)) {
					if (stat == st->st_wrinit->qi_mstat)
						mlist->ms[0].index |= 0x1;
					else if (stat == st->st_rdinit->qi_mstat)
						mlist->ms[1].index |= 0x1;
					else if (stat == st->st_muxwinit->qi_mstat)
						mlist->ms[2].index |= 0x1;
					else
						mlist->ms[3].index = 0x1;
					mlist->ms[3].ms_pcnt = stat->ms_pcnt;
					mlist->ms[3].ms_scnt = stat->ms_scnt;
					mlist->ms[3].ms_ocnt = stat->ms_ocnt;
					mlist->ms[3].ms_ccnt = stat->ms_ccnt;
					mlist->ms[3].ms_acnt = stat->ms_acnt;
					mlist->ms[3].ms_flags = stat->ms_flags;
					mlist->ms[3].ms_xsize = stat->ms_xsize;
				}
			}
			if (reset) {
				if (st->st_wrinit && (stat = st->st_wrinit->qi_mstat)) {
					stat->ms_pcnt = 0;
					stat->ms_scnt = 0;
					stat->ms_ocnt = 0;
					stat->ms_ccnt = 0;
					stat->ms_acnt = 0;
					stat->ms_flags = 0;
				}
				if (st->st_rdinit && (stat = st->st_rdinit->qi_mstat)) {
					stat->ms_pcnt = 0;
					stat->ms_scnt = 0;
					stat->ms_ocnt = 0;
					stat->ms_ccnt = 0;
					stat->ms_acnt = 0;
					stat->ms_flags = 0;
				}
				if (st->st_muxwinit && (stat = st->st_muxwinit->qi_mstat)) {
					stat->ms_pcnt = 0;
					stat->ms_scnt = 0;
					stat->ms_ocnt = 0;
					stat->ms_ccnt = 0;
					stat->ms_acnt = 0;
					stat->ms_flags = 0;
				}
				if (st->st_muxrinit && (stat = st->st_muxrinit->qi_mstat)) {
					stat->ms_pcnt = 0;
					stat->ms_scnt = 0;
					stat->ms_ocnt = 0;
					stat->ms_ccnt = 0;
					stat->ms_acnt = 0;
					stat->ms_flags = 0;
				}
			}
		}
		return sizeof(struct sc_mlist32);
	} else
#endif				/* __LP64__ */
	{
		struct sc_mlist *mlist = (typeof(mlist)) _mlist;

		if ((mlist->major = major) != -1) {
			if (st->st_wrinit && (info = st->st_wrinit->qi_minfo)) {
				strncpy(mlist->name, info->mi_idname, FMNAMESZ + 1);
				mlist->mi[0].index = 0x8;
				mlist->mi[0].mi_idnum = info->mi_idnum;
				strncpy(mlist->mi[0].mi_idname, info->mi_idname, FMNAMESZ + 1);
				mlist->mi[0].mi_minpsz = info->mi_minpsz;
				mlist->mi[0].mi_maxpsz = info->mi_maxpsz;
				mlist->mi[0].mi_hiwat = info->mi_hiwat;
				mlist->mi[0].mi_lowat = info->mi_lowat;
				if ((stat = st->st_wrinit->qi_mstat)) {
					mlist->ms[0].index = 0x8;
					mlist->ms[0].ms_pcnt = stat->ms_pcnt;
					mlist->ms[0].ms_scnt = stat->ms_scnt;
					mlist->ms[0].ms_ocnt = stat->ms_ocnt;
					mlist->ms[0].ms_ccnt = stat->ms_ccnt;
					mlist->ms[0].ms_acnt = stat->ms_acnt;
					mlist->ms[0].ms_flags = stat->ms_flags;
					mlist->ms[0].ms_xsize = stat->ms_xsize;
				}
			}
			if (st->st_rdinit && (info = st->st_rdinit->qi_minfo)) {
				if (info == st->st_wrinit->qi_minfo)
					mlist->mi[0].index |= 0x4;
				else
					mlist->mi[1].index = 0x4;
				mlist->mi[1].mi_idnum = info->mi_idnum;
				strncpy(mlist->mi[1].mi_idname, info->mi_idname, FMNAMESZ + 1);
				mlist->mi[1].mi_minpsz = info->mi_minpsz;
				mlist->mi[1].mi_maxpsz = info->mi_maxpsz;
				mlist->mi[1].mi_hiwat = info->mi_hiwat;
				mlist->mi[1].mi_lowat = info->mi_lowat;
				if ((stat = st->st_rdinit->qi_mstat)) {
					if (stat == st->st_wrinit->qi_mstat)
						mlist->ms[0].index |= 0x4;
					else
						mlist->ms[1].index = 0x4;
					mlist->ms[1].ms_pcnt = stat->ms_pcnt;
					mlist->ms[1].ms_scnt = stat->ms_scnt;
					mlist->ms[1].ms_ocnt = stat->ms_ocnt;
					mlist->ms[1].ms_ccnt = stat->ms_ccnt;
					mlist->ms[1].ms_acnt = stat->ms_acnt;
					mlist->ms[1].ms_flags = stat->ms_flags;
					mlist->ms[1].ms_xsize = stat->ms_xsize;
				}
			}
			if (st->st_muxwinit && (info = st->st_muxwinit->qi_minfo)) {
				if (info == st->st_wrinit->qi_minfo)
					mlist->mi[0].index |= 0x2;
				else if (info == st->st_rdinit->qi_minfo)
					mlist->mi[1].index |= 0x2;
				else
					mlist->mi[2].index = 0x2;
				mlist->mi[2].mi_idnum = info->mi_idnum;
				strncpy(mlist->mi[2].mi_idname, info->mi_idname, FMNAMESZ + 1);
				mlist->mi[2].mi_minpsz = info->mi_minpsz;
				mlist->mi[2].mi_maxpsz = info->mi_maxpsz;
				mlist->mi[2].mi_hiwat = info->mi_hiwat;
				mlist->mi[2].mi_lowat = info->mi_lowat;
				if ((stat = st->st_muxwinit->qi_mstat)) {
					if (stat == st->st_wrinit->qi_mstat)
						mlist->ms[0].index |= 0x2;
					else if (stat == st->st_rdinit->qi_mstat)
						mlist->ms[1].index |= 0x2;
					else
						mlist->ms[2].index = 0x2;
					mlist->ms[2].ms_pcnt = stat->ms_pcnt;
					mlist->ms[2].ms_scnt = stat->ms_scnt;
					mlist->ms[2].ms_ocnt = stat->ms_ocnt;
					mlist->ms[2].ms_ccnt = stat->ms_ccnt;
					mlist->ms[2].ms_acnt = stat->ms_acnt;
					mlist->ms[2].ms_flags = stat->ms_flags;
					mlist->ms[2].ms_xsize = stat->ms_xsize;
				}
			}
			if (st->st_muxrinit && (info = st->st_muxrinit->qi_minfo)) {
				if (info == st->st_wrinit->qi_minfo)
					mlist->mi[0].index |= 0x1;
				else if (info == st->st_rdinit->qi_minfo)
					mlist->mi[1].index |= 0x1;
				else if (info == st->st_muxwinit->qi_minfo)
					mlist->mi[2].index |= 0x1;
				else
					mlist->mi[3].index = 0x1;
				mlist->mi[3].mi_idnum = info->mi_idnum;
				strncpy(mlist->mi[3].mi_idname, info->mi_idname, FMNAMESZ + 1);
				mlist->mi[3].mi_minpsz = info->mi_minpsz;
				mlist->mi[3].mi_maxpsz = info->mi_maxpsz;
				mlist->mi[3].mi_hiwat = info->mi_hiwat;
				mlist->mi[3].mi_lowat = info->mi_lowat;
				if ((stat = st->st_muxrinit->qi_mstat)) {
					if (stat == st->st_wrinit->qi_mstat)
						mlist->ms[0].index |= 0x1;
					else if (stat == st->st_rdinit->qi_mstat)
						mlist->ms[1].index |= 0x1;
					else if (stat == st->st_muxwinit->qi_mstat)
						mlist->ms[2].index |= 0x1;
					else
						mlist->ms[3].index = 0x1;
					mlist->ms[3].ms_pcnt = stat->ms_pcnt;
					mlist->ms[3].ms_scnt = stat->ms_scnt;
					mlist->ms[3].ms_ocnt = stat->ms_ocnt;
					mlist->ms[3].ms_ccnt = stat->ms_ccnt;
					mlist->ms[3].ms_acnt = stat->ms_acnt;
					mlist->ms[3].ms_flags = stat->ms_flags;
					mlist->ms[3].ms_xsize = stat->ms_xsize;
				}
			}
			if (reset) {
				if (st->st_wrinit && (stat = st->st_wrinit->qi_mstat)) {
					stat->ms_pcnt = 0;
					stat->ms_scnt = 0;
					stat->ms_ocnt = 0;
					stat->ms_ccnt = 0;
					stat->ms_acnt = 0;
					stat->ms_flags = 0;
				}
				if (st->st_rdinit && (stat = st->st_rdinit->qi_mstat)) {
					stat->ms_pcnt = 0;
					stat->ms_scnt = 0;
					stat->ms_ocnt = 0;
					stat->ms_ccnt = 0;
					stat->ms_acnt = 0;
					stat->ms_flags = 0;
				}
				if (st->st_muxwinit && (stat = st->st_muxwinit->qi_mstat)) {
					stat->ms_pcnt = 0;
					stat->ms_scnt = 0;
					stat->ms_ocnt = 0;
					stat->ms_ccnt = 0;
					stat->ms_acnt = 0;
					stat->ms_flags = 0;
				}
				if (st->st_muxrinit && (stat = st->st_muxrinit->qi_mstat)) {
					stat->ms_pcnt = 0;
					stat->ms_scnt = 0;
					stat->ms_ocnt = 0;
					stat->ms_ccnt = 0;
					stat->ms_acnt = 0;
					stat->ms_flags = 0;
				}
			}
		}
		return sizeof(struct sc_mlist);
	}
}

static size_t
str_mlist_count(void)
{
	return (cdev_count + fmod_count);
}

/**
 * sc_slist_copy: copy stats for a given index
 *
 * @_slist: pointer to area to fill with stats structure
 * @index: index into stats array
 * @flag: flag indicating 32bit or native model
 *
 * Copies the stats structure for @index into the area pointed to by @_slist
 * and returns the number of bytes to increment @_slist by to point to the
 * next element in the list considering the data model indicated by @flag. 
 */
static size_t
sc_slist_copy(caddr_t _slist, const int index, const uint flag)
{
	struct strinfo *si = &Strinfo[index];

#ifdef __LP64__
	if (flag == IOC_ILP32) {
		struct sc_stat32 *slist = (typeof(slist)) _slist;

		if (index < DYN_SIZE) {
			slist->sc_alloc = atomic_read(&si->si_cnt);
			slist->sc_hiwat = si->si_hwl;
		} else {
			slist->sc_alloc = 0;
			slist->sc_hiwat = 0;
		}
		return (sizeof(struct sc_stat32));
	} else
#endif				/* __LP64__ */
	{
		struct sc_stat *slist = (typeof(slist)) _slist;

		if (index < DYN_SIZE) {
			slist->sc_alloc = atomic_read(&si->si_cnt);
			slist->sc_hiwat = si->si_hwl;
		} else {
			slist->sc_alloc = 0;
			slist->sc_hiwat = 0;
		}
		return (sizeof(struct sc_stat));
	}
}

static size_t
str_slist_count(void)
{
	return (DYN_SIZE);
}

/**
 * sc_tlist_tune: perform tuning for a given index
 *
 * @_tlist: pointer to area to tune from
 * @index: index into tune array
 * @flag: flag indicating 32bit or native model
 *
 * Sets the parameters that are requested to be set by flags and returns all
 * parameters for the requested queue information structures.  Trace levels
 * are not really supported yet.
 *
 */
static size_t
sc_tlist_tune(struct streamtab *st, caddr_t _tlist, const int index, const uint flag)
{
	struct module_info *mi = NULL;

#ifdef __LP64__
	if (flag == IOC_ILP32) {
		struct sc_tune32 *tlist = (typeof(tlist)) _tlist;

		if (tlist->sc_flags & SC_SET_RDQUEUE) {
			if (tlist->sc_flags & SC_SET_LOWERMUX) {
				if (st->st_muxrinit)
					mi = st->st_muxrinit->qi_minfo;
			} else {
				if (st->st_rdinit)
					mi = st->st_rdinit->qi_minfo;
			}
			if (mi) {
				if (tlist->sc_flags & SC_SET_MINPSZ)
					mi->mi_minpsz = tlist->sc_minpsz;
				if (tlist->sc_flags & SC_SET_MAXPSZ)
					mi->mi_maxpsz = tlist->sc_maxpsz;
				if (tlist->sc_flags & SC_SET_HIWAT)
					mi->mi_hiwat = tlist->sc_hiwat;
				if (tlist->sc_flags & SC_SET_LOWAT)
					mi->mi_lowat = tlist->sc_lowat;
				tlist->sc_minpsz = mi->mi_minpsz;
				tlist->sc_maxpsz = mi->mi_maxpsz;
				tlist->sc_hiwat = mi->mi_hiwat;
				tlist->sc_lowat = mi->mi_lowat;
			}
		}
		if (tlist->sc_flags & SC_SET_WRQUEUE) {
			if (tlist->sc_flags & SC_SET_LOWERMUX) {
				if (st->st_muxwinit)
					mi = st->st_muxwinit->qi_minfo;
			} else {
				if (st->st_wrinit)
					mi = st->st_wrinit->qi_minfo;
			}
			if (mi) {
				if (tlist->sc_flags & SC_SET_MINPSZ)
					mi->mi_minpsz = tlist->sc_minpsz;
				if (tlist->sc_flags & SC_SET_MAXPSZ)
					mi->mi_maxpsz = tlist->sc_maxpsz;
				if (tlist->sc_flags & SC_SET_HIWAT)
					mi->mi_hiwat = tlist->sc_hiwat;
				if (tlist->sc_flags & SC_SET_LOWAT)
					mi->mi_lowat = tlist->sc_lowat;
				tlist->sc_minpsz = mi->mi_minpsz;
				tlist->sc_maxpsz = mi->mi_maxpsz;
				tlist->sc_hiwat = mi->mi_hiwat;
				tlist->sc_lowat = mi->mi_lowat;
			}
		}
		if (tlist->sc_flags & SC_SET_TRCLEVEL) {
			/* trace levels not supported yet. */
		}
		return (sizeof(*tlist));
	} else
#endif
	{
		struct sc_tune *tlist = (typeof(tlist)) _tlist;

		if (tlist->sc_flags & SC_SET_RDQUEUE) {
			if (tlist->sc_flags & SC_SET_LOWERMUX) {
				if (st->st_muxrinit)
					mi = st->st_muxrinit->qi_minfo;
			} else {
				if (st->st_rdinit)
					mi = st->st_rdinit->qi_minfo;
			}
			if (mi) {
				if (tlist->sc_flags & SC_SET_MINPSZ)
					mi->mi_minpsz = tlist->sc_minpsz;
				if (tlist->sc_flags & SC_SET_MAXPSZ)
					mi->mi_maxpsz = tlist->sc_maxpsz;
				if (tlist->sc_flags & SC_SET_HIWAT)
					mi->mi_hiwat = tlist->sc_hiwat;
				if (tlist->sc_flags & SC_SET_LOWAT)
					mi->mi_lowat = tlist->sc_lowat;
				tlist->sc_minpsz = mi->mi_minpsz;
				tlist->sc_maxpsz = mi->mi_maxpsz;
				tlist->sc_hiwat = mi->mi_hiwat;
				tlist->sc_lowat = mi->mi_lowat;
			}
		}
		if (tlist->sc_flags & SC_SET_WRQUEUE) {
			if (tlist->sc_flags & SC_SET_LOWERMUX) {
				if (st->st_muxwinit)
					mi = st->st_muxwinit->qi_minfo;
			} else {
				if (st->st_wrinit)
					mi = st->st_wrinit->qi_minfo;
			}
			if (mi) {
				if (tlist->sc_flags & SC_SET_MINPSZ)
					mi->mi_minpsz = tlist->sc_minpsz;
				if (tlist->sc_flags & SC_SET_MAXPSZ)
					mi->mi_maxpsz = tlist->sc_maxpsz;
				if (tlist->sc_flags & SC_SET_HIWAT)
					mi->mi_hiwat = tlist->sc_hiwat;
				if (tlist->sc_flags & SC_SET_LOWAT)
					mi->mi_lowat = tlist->sc_lowat;
				tlist->sc_minpsz = mi->mi_minpsz;
				tlist->sc_maxpsz = mi->mi_maxpsz;
				tlist->sc_hiwat = mi->mi_hiwat;
				tlist->sc_lowat = mi->mi_lowat;
			}
		}
		if (tlist->sc_flags & SC_SET_TRCLEVEL) {
			/* trace levels not supported yet. */
		}
		return (sizeof(*tlist));
	}
}

static size_t
str_tlist_count(void)
{
	return (4);
}

/* 
 *  -------------------------------------------------------------------------
 *
 *  PUT routine
 *  
 *  -------------------------------------------------------------------------
 */
static streamscall int
sc_wput(queue_t *q, mblk_t *mp)
{
	union ioctypes *ioc;
	int err = 0, rval = 0, reset = 0;
	mblk_t *dp = mp->b_cont, *cp = NULL;

	switch (mp->b_datap->db_type) {
	case M_FLUSH:
		/* canonical flushing */
		if (mp->b_rptr[0] & FLUSHW) {
			if (mp->b_rptr[0] & FLUSHBAND)
				flushband(q, mp->b_rptr[1], FLUSHDATA);
			else
				flushq(q, FLUSHDATA);
			mp->b_rptr[0] &= ~FLUSHW;
		}
		if (mp->b_rptr[0] & FLUSHR) {
			queue_t *rq = RD(q);

			if (mp->b_rptr[0] & FLUSHBAND)
				flushband(rq, mp->b_rptr[1], FLUSHDATA);
			else
				flushq(rq, FLUSHDATA);
			qreply(q, mp);
			return (0);
		}
		break;
	case M_IOCTL:
	{
		caddr_t uaddr;
		size_t usize;

		_trace();
		ioc = (typeof(ioc)) mp->b_rptr;

		switch (ioc->iocblk.ioc_cmd) {
		case SC_IOC_RESET:
			reset = 1;
		case SC_IOC_LIST:
#ifdef __LP64__
			if (ioc->iocblk.ioc_flag == IOC_ILP32) {
				uaddr = (caddr_t) (unsigned long) (uint32_t) *(unsigned long *)
				    dp->b_rptr;
				usize = sizeof(struct sc_list32);
			} else
#endif
			{
				uaddr = (caddr_t) *(unsigned long *) dp->b_rptr;
				usize = sizeof(struct sc_list);
			}
			/* there is really no reason why a regular user cannot list modules and
			   related information. */
#if 0
			_trace();
			err = -EPERM;
			if (ioc->iocblk.ioc_uid != 0) {
				_ptrace(("Error path taken!\n"));
				goto nak;
			}
#endif
			_trace();
			if (ioc->iocblk.ioc_count == TRANSPARENT) {
				_trace();
				if (uaddr == NULL) {
					rval = str_mlist_count();
					goto ack;
				}
				mp->b_datap->db_type = M_COPYIN;
				ioc->copyreq.cq_addr = uaddr;
				ioc->copyreq.cq_size = usize;
				ioc->copyreq.cq_flag = 0;
				ioc->copyreq.cq_private = (mblk_t *) 0;
				qreply(q, mp);
				return (0);
			}
			_trace();
			/* doesn't support I_STR yet, just TRANSPARENT */
			err = -EINVAL;
			goto nak;
		case SC_IOC_TUNE:
#ifdef __LP64__
			if (ioc->iocblk.ioc_flag == IOC_ILP32) {
				uaddr = (caddr_t) (unsigned long) (uint32_t) *(unsigned long *)
				    dp->b_rptr;
				usize = sizeof(struct sc_tlist32);
			} else
#endif
			{
				uaddr = (caddr_t) *(unsigned long *) dp->b_rptr;
				usize = sizeof(struct sc_tlist);
			}
			if (ioc->iocblk.ioc_count == TRANSPARENT) {
				mp->b_datap->db_type = M_COPYIN;
				ioc->copyreq.cq_addr = uaddr;
				ioc->copyreq.cq_size = usize;
				ioc->copyreq.cq_flag = 0;
				ioc->copyreq.cq_private = (mblk_t *) 0;
				qreply(q, mp);
				return (0);
			}
			/* doesn't support I_STR yet, just TRANSPARENT */
			err = -EINVAL;
			goto nak;
		case SC_IOC_STATS:
#ifdef __LP64__
			if (ioc->iocblk.ioc_flag == IOC_ILP32) {
				uaddr =
				    (caddr_t) (unsigned long) (uint32_t) *(unsigned long *) dp->
				    b_rptr;
				usize = sizeof(struct sc_slist32);
			} else
#endif
			{
				uaddr = (caddr_t) *(unsigned long *) dp->b_rptr;
				usize = sizeof(struct sc_slist);
			}
			if (ioc->iocblk.ioc_count == TRANSPARENT) {
				if (uaddr == NULL) {
					rval = str_slist_count();
					goto ack;
				}
				mp->b_datap->db_type = M_COPYIN;
				ioc->copyreq.cq_addr = uaddr;
				ioc->copyreq.cq_size = usize;
				ioc->copyreq.cq_flag = 0;
				ioc->copyreq.cq_private = (mblk_t *) 0;
				qreply(q, mp);
				return (0);
			}
			/* doesn't support I_STR yet, just TRANSPARENT */
			err = -EINVAL;
			goto nak;
		}
		break;
	}
	case M_IOCDATA:
		ioc = (typeof(ioc)) mp->b_rptr;

		switch (ioc->copyresp.cp_cmd) {
		case SC_IOC_RESET:
			reset = 1;
		case SC_IOC_LIST:
			_trace();
			if (ioc->copyresp.cp_rval != 0) {
				_ptrace(("Aborting ioctl!\n"));
				goto abort;
			}
			_trace();
			if (ioc->copyresp.cp_private == (mblk_t *) 0) {
				int n, count;
				caddr_t uaddr;
				size_t usize;

				n = 0;
				if (!dp || dp->b_wptr == dp->b_rptr) {
					rval = str_mlist_count();
					goto ack;
				}
#ifdef __LP64__
				if (ioc->copyresp.cp_flag == IOC_ILP32) {
					if (dp->b_wptr < dp->b_rptr + sizeof(struct sc_list32)) {
						_ptrace(("Error path taken!\n"));
						err = -EFAULT;
						goto nak;
					} else {
						struct sc_list32 *sclp = (typeof(sclp)) dp->b_rptr;

						count = sclp->sc_nmods;
						uaddr = (caddr_t) (unsigned long) sclp->sc_mlist;
						usize = count * sizeof(struct sc_mlist32);
					}
				} else
#endif
				{
					if (dp->b_wptr < dp->b_rptr + sizeof(struct sc_list)) {
						_ptrace(("Error path taken!\n"));
						err = -EFAULT;
						goto nak;
					} else {
						struct sc_list *sclp = (typeof(sclp)) dp->b_rptr;

						count = sclp->sc_nmods;
						uaddr = (caddr_t) sclp->sc_mlist;
						usize = count * sizeof(struct sc_mlist);
					}
				}
				if (count < 0) {
					_ptrace(("Error path taken!\n"));
					err = -EINVAL;
					goto nak;
				}
				if (count > 100) {
					_ptrace(("Error path taken!\n"));
					err = -ERANGE;
					goto nak;
				}
				if (count == 0) {
					rval = str_mlist_count();
					goto ack;
				}
				if (!(dp = allocb(usize, BPRI_MED))) {
					_ptrace(("Error path taken!\n"));
					err = -ENOSR;
					goto nak;
				}
				dp->b_wptr = dp->b_rptr + usize;
				bzero(dp->b_rptr, usize);
				freemsg(mp->b_cont);
				mp->b_cont = dp;
				{
					struct list_head *pos;
					uint flag = ioc->copyresp.cp_flag;
					caddr_t mlist = (typeof(mlist)) dp->b_rptr;

					_trace();
					if (n < count) {
						_trace();
						/* output all devices */
						read_lock(&cdevsw_lock);
						list_for_each(pos, &cdevsw_list) {
							struct cdevsw *cdev;
							struct streamtab *st;

							if (n >= count)
								break;
							cdev =
							    list_entry(pos, struct cdevsw, d_list);
							st = cdev->d_str;
							mlist += sc_mlist_copy(cdev->d_major, st,
									       mlist, reset, flag);
							n++;
						}
						read_unlock(&cdevsw_lock);
					}
					_trace();
					if (n < count) {
						_trace();
						/* output all modules */
						read_lock(&fmodsw_lock);
						list_for_each(pos, &fmodsw_list) {
							struct fmodsw *fmod;
							struct streamtab *st;

							if (n >= count)
								break;
							fmod =
							    list_entry(pos, struct fmodsw, f_list);
							st = fmod->f_str;
							mlist += sc_mlist_copy(0, st,
									       mlist, reset, flag);
							n++;
						}
						read_unlock(&fmodsw_lock);
					}
					_trace();
					/* zero all excess elements */
					for (; n < count; n++) {
						mlist +=
						    sc_mlist_copy(-1, NULL, mlist, reset, flag);
					}
				}
				_trace();
				mp->b_datap->db_type = M_COPYOUT;
				ioc->copyreq.cq_addr = uaddr;
				ioc->copyreq.cq_size = usize;
				ioc->copyreq.cq_flag = 0;
				ioc->copyreq.cq_private = (mblk_t *) (long) count;
				qreply(q, mp);
				return (0);
			} else {
				_trace();
				/* done */
				rval = (int) (long) ioc->copyresp.cp_private;
				goto ack;
			}
		case SC_IOC_TUNE:
		{
			int n = 0, count;
			caddr_t uaddr;
			size_t usize;

			cp = ioc->copyresp.cp_private;
			ioc->copyresp.cp_private = NULL;
			if (ioc->copyresp.cp_rval != 0) {
				freemsg(cp);
				_ptrace(("Aborting ioctl!\n"));
				goto abort;
			}
			if (cp == NULL) {
				if (!dp || dp->b_wptr == dp->b_rptr) {
					rval = str_tlist_count();
					goto ack;
				}
#ifdef __LP64__
				if (ioc->copyresp.cp_flag == IOC_ILP32) {
					if (dp->b_wptr < dp->b_rptr + sizeof(struct sc_tlist32)) {
						_ptrace(("Error path taken!\n"));
						err = -EFAULT;
						goto nak;
					} else {
						struct sc_tlist32 *sclp = (typeof(sclp)) dp->b_rptr;

						count = sclp->sc_ntune;
						uaddr = (caddr_t) (unsigned long) sclp->sc_tlist;
						usize = count * sizeof(struct sc_tune32);
					}
				} else
#endif				/* __LP64__ */
				{
					if (dp->b_wptr < dp->b_rptr + sizeof(struct sc_tlist)) {
						_ptrace(("Error path taken!\n"));
						err = -EFAULT;
						goto nak;
					} else {
						struct sc_tlist *sclp = (typeof(sclp)) dp->b_rptr;

						count = sclp->sc_ntune;
						uaddr = (caddr_t) (unsigned long) sclp->sc_tlist;
						usize = count * sizeof(struct sc_tune);
					}
				}
				if (count < 0) {
					_ptrace(("Error path taken!\n"));
					err = -EINVAL;
					goto nak;
				}
				if (count > 4) {
					_ptrace(("Error path taken!\n"));
					err = -ERANGE;
					goto nak;
				}
				if (count == 0) {
					rval = str_tlist_count();
					goto ack;
				}
				if (!(dp = allocb(usize, BPRI_MED))) {
					_ptrace(("Error path taken!\n"));
					err = -ENOSR;
					goto nak;
				}
				dp->b_wptr = dp->b_rptr + usize;
				bzero(dp->b_rptr, usize);
				dp = XCHG(&mp->b_cont, dp);

				mp->b_datap->db_type = M_COPYIN;
				ioc->copyreq.cq_addr = uaddr;
				ioc->copyreq.cq_size = usize;
				ioc->copyreq.cq_flag = 0;
				ioc->copyreq.cq_private = dp;
				qreply(q, mp);
				return (0);

			}
			if ((long) cp >= 1 && (long) cp <= 4) {
				/* done */
				rval = (int) (long) cp;
				goto ack;
			}
			{
				uint flag = ioc->copyresp.cp_flag;
				caddr_t tlist = (typeof(tlist)) dp->b_rptr;
				struct fmodsw *fmod = NULL;
				struct cdevsw *cdev = NULL;
				struct streamtab *st = NULL;

#ifdef __LP64__
				if (flag == IOC_ILP32) {
					struct sc_tlist32 *sclp = (typeof(sclp)) cp->b_rptr;

					if (cp->b_wptr < cp->b_rptr + sizeof(*sclp)) {
						_ptrace(("Error path taken!\n"));
						freemsg(cp);
						err = -EFAULT;
						goto nak;
					}
					count = sclp->sc_ntune;
					uaddr = (caddr_t) (unsigned long) sclp->sc_tlist;
					usize = count * sizeof(struct sc_tune32);

					/* First order of business is to find the STREAMS driver or 
					   module streamtab structure. */
					if (sclp->sc_major == 0) {
						if ((fmod = fmod_find(sclp->sc_name)))
							st = fmod->f_str;
						else if ((cdev = cdev_find(sclp->sc_name)))
							st = cdev->d_str;
					} else {
						if ((cdev = sdev_get(sclp->sc_major)))
							st = cdev->d_str;
					}
				} else
#endif
				{
					struct sc_tlist *sclp = (typeof(sclp)) cp->b_rptr;

					if (cp->b_wptr < cp->b_rptr + sizeof(*sclp)) {
						_ptrace(("Error path taken!\n"));
						freemsg(cp);
						err = -EFAULT;
						goto nak;
					}
					count = sclp->sc_ntune;
					uaddr = (caddr_t) (unsigned long) sclp->sc_tlist;
					usize = count * sizeof(*sclp->sc_tlist);

					/* First order of business is to find the STREAMS driver or 
					   module streamtab structure. */
					if (sclp->sc_major == 0) {
						if ((fmod = fmod_find(sclp->sc_name)))
							st = fmod->f_str;
						else if ((cdev = cdev_find(sclp->sc_name)))
							st = cdev->d_str;
					} else {
						if ((cdev = sdev_get(sclp->sc_major)))
							st = cdev->d_str;
					}
				}
				if (st == NULL) {
					_ptrace(("Error path taken!\n"));
					if (fmod != NULL)
						fmod_put(fmod);
					if (cdev != NULL)
						sdev_put(cdev);
					freemsg(cp);
					err = -EINVAL;
					goto nak;
				}
				if (dp->b_wptr < dp->b_rptr + usize) {
					_ptrace(("Error path taken!\n"));
					if (fmod != NULL)
						fmod_put(fmod);
					if (cdev != NULL)
						sdev_put(cdev);
					freemsg(cp);
					err = -EFAULT;
					goto nak;
				}

				for (; n < count; n++)
					tlist += sc_tlist_tune(st, tlist, n, flag);

				rval = count;
				if (fmod != NULL)
					fmod_put(fmod);
				if (cdev != NULL)
					sdev_put(cdev);
				freemsg(cp);
			}
			mp->b_datap->db_type = M_COPYOUT;
			ioc->copyreq.cq_addr = uaddr;
			ioc->copyreq.cq_size = usize;
			ioc->copyreq.cq_flag = 0;
			ioc->copyreq.cq_private = (mblk_t *) (long) count;
			qreply(q, mp);
			return (0);
		}
		case SC_IOC_STATS:
			if (ioc->copyresp.cp_rval != 0) {
				_ptrace(("Aborting ioctl!\n"));
				goto abort;
			}
			if (ioc->copyresp.cp_private == (mblk_t *) 0) {
				int n = 0, count;
				caddr_t uaddr;
				size_t usize;

				if (!dp || dp->b_wptr == dp->b_rptr) {
					rval = str_slist_count();
					goto ack;
				}
#ifdef __LP64__
				if (ioc->copyresp.cp_flag == IOC_ILP32) {
					if (dp->b_wptr < dp->b_rptr + sizeof(struct sc_slist32)) {
						_ptrace(("Error path taken!\n"));
						err = -EFAULT;
						goto nak;
					} else {
						struct sc_slist32 *sclp = (typeof(sclp)) dp->b_rptr;

						count = sclp->sc_nstat;
						uaddr = (caddr_t) (unsigned long) sclp->sc_slist;
						usize = count * sizeof(struct sc_stat32);
					}
				} else
#endif
				{
					if (dp->b_wptr < dp->b_rptr + sizeof(struct sc_slist)) {
						_ptrace(("Error path taken!\n"));
						err = -EFAULT;
						goto nak;
					} else {
						struct sc_slist *sclp = (typeof(sclp)) dp->b_rptr;

						count = sclp->sc_nstat;
						uaddr = (caddr_t) sclp->sc_slist;
						usize = count * sizeof(struct sc_stat);
					}
				}
				if (count < 0) {
					_ptrace(("Error path taken!\n"));
					err = -EINVAL;
					goto nak;
				}
				if (count > 100) {
					_ptrace(("Error path taken!\n"));
					err = -ERANGE;
					goto nak;
				}
				if (count == 0) {
					rval = str_slist_count();
					goto ack;
				}
				if (!(dp = allocb(usize, BPRI_MED))) {
					_ptrace(("Error path taken!\n"));
					err = -ENOSR;
					goto nak;
				}
				dp->b_wptr = dp->b_rptr + usize;
				bzero(dp->b_rptr, usize);
				freemsg(mp->b_cont);
				mp->b_cont = dp;
				{
					uint flag = ioc->copyresp.cp_flag;
					caddr_t slist = (typeof(slist)) dp->b_rptr;

					for (; n < count; n++)
						slist += sc_slist_copy(slist, n, flag);
					rval = str_slist_count();
				}
				mp->b_datap->db_type = M_COPYOUT;
				ioc->copyreq.cq_addr = uaddr;
				ioc->copyreq.cq_size = usize;
				ioc->copyreq.cq_flag = 0;
				ioc->copyreq.cq_private = (mblk_t *) (long) count;
				qreply(q, mp);
				return (0);
			} else {
				/* done */
				rval = (int) (long) ioc->copyresp.cp_private;
				goto ack;
			}
		}
	      nak:
		mp->b_datap->db_type = M_IOCNAK;
		ioc->iocblk.ioc_count = 0;
		ioc->iocblk.ioc_rval = -1;
		ioc->iocblk.ioc_error = -err;
		qreply(q, mp);
		return (0);
	      ack:
		mp->b_datap->db_type = M_IOCACK;
		ioc->iocblk.ioc_count = 0;
		ioc->iocblk.ioc_rval = rval;
		ioc->iocblk.ioc_error = 0;
		qreply(q, mp);
		return (0);
	      abort:
		_ctrace(freemsg(mp));
		return (0);
	}
	putnext(q, mp);
	return (0);
}
static streamscall int
sc_rput(queue_t *q, mblk_t *mp)
{
	putnext(q, mp);
	return (0);
}

/* 
 *  -------------------------------------------------------------------------
 *
 *  OPEN and CLOSE
 *
 *  -------------------------------------------------------------------------
 */
static streamscall int
sc_open(queue_t *q, dev_t *devp, int oflag, int sflag, cred_t *crp)
{
	if (q->q_ptr)
		return (0);	/* already open */
	if (sflag == MODOPEN && WR(q)->q_next != NULL) {
		/* don't need to be privileged to push the module, just to use it */
#if 0
		if (crp->cr_uid != 0 && crp->cr_ruid != 0)
			return (-EACCES);
#endif
		q->q_ptr = WR(q)->q_ptr = (void *) 1;
		qprocson(q);
		return (0);
	}
	return (-EIO);		/* can't be opened as driver */
}
static streamscall int
sc_close(queue_t *q, int oflag, cred_t *crp)
{
	qprocsoff(q);
	q->q_ptr = WR(q)->q_ptr = NULL;
	return (0);
}

/* 
 *  -------------------------------------------------------------------------
 *
 *  Registration and initialization
 *
 *  -------------------------------------------------------------------------
 */

static struct qinit sc_rqinit = {
	.qi_putp = sc_rput,
	.qi_qopen = sc_open,
	.qi_qclose = sc_close,
	.qi_minfo = &sc_minfo,
	.qi_mstat = &sc_rstat,
};

static struct qinit sc_wqinit = {
	.qi_putp = sc_wput,
	.qi_minfo = &sc_minfo,
	.qi_mstat = &sc_wstat,
};

#ifdef CONFIG_STREAMS_SC_MODULE
static
#endif
struct streamtab scinfo = {
	.st_rdinit = &sc_rqinit,
	.st_wrinit = &sc_wqinit,
};

static struct fmodsw sc_fmod = {
	.f_name = CONFIG_STREAMS_SC_NAME,
	.f_str = &scinfo,
	.f_flag = D_MP,
	.f_kmod = THIS_MODULE,
};

static void *sc_opaque;

static void
sc_unregister_ioctl32(void)
{
	if (sc_opaque != NULL)
		unregister_ioctl32(sc_opaque);
}

static int
sc_register_ioctl32(void)
{
	if ((sc_opaque = register_ioctl32(SC_IOC_LIST)) == NULL)
		return (-ENOMEM);
	return (0);
}

#ifdef CONFIG_STREAMS_SC_MODULE
static
#endif
int __init
scinit(void)
{
	int err;

#ifdef CONFIG_STREAMS_SC_MODULE
	printk(KERN_INFO SC_BANNER);
#else
	printk(KERN_INFO SC_SPLASH);
#endif
	sc_minfo.mi_idnum = modid;
	if ((err = sc_register_ioctl32()) < 0)
		return (err);
	if ((err = register_strmod(&sc_fmod)) < 0) {
		sc_unregister_ioctl32();
		return (err);
	}
	if (modid == 0 && err > 0)
		modid = err;
	return (0);
};

#ifdef CONFIG_STREAMS_SC_MODULE
static
#endif
void __exit
scexit(void)
{
	unregister_strmod(&sc_fmod);
	sc_unregister_ioctl32();
};

#ifdef CONFIG_STREAMS_SC_MODULE
module_init(scinit);
module_exit(scexit);
#endif
