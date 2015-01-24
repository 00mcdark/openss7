/*****************************************************************************

 @(#) src/include/sys/svr3/ddi.h

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

#ifndef __SYS_SVR3_DDI_H__
#define __SYS_SVR3_DDI_H__

#ifndef __KERNEL__
#error "Do not use kernel headers for user space programs"
#endif				/* __KERNEL__ */

#ifndef __EXTERN_INLINE
#define __EXTERN_INLINE extern __inline__
#endif

#ifndef __SVR3_EXTERN_INLINE
#define __SVR3_EXTERN_INLINE __EXTERN_INLINE streamscall
#endif				/* __SVR3_EXTERN_INLINE */

#ifndef __SVR3_EXTERN
#define __SVR3_EXTERN extern streamscall
#endif

#ifndef _SVR3_SOURCE
#warning "_SVR3_SOURCE not defined but SVR3 ddi.h included"
#endif

#if defined(CONFIG_STREAMS_COMPAT_SVR3) || defined(CONFIG_STREAMS_COMPAT_SVR3_MODULE)

#ifndef dev_t
#define dev_t __streams_dev_t
#endif

__SVR3_EXTERN_INLINE mblk_t *
alloc_proto(size_t psize, size_t dsize, int type, uint bpri)
{
	mblk_t *mp = NULL, *dp = NULL;

	if (psize && !(mp = allocb(psize, bpri)))
		goto enobufs;
	if (dsize && !(dp = allocb(dsize, bpri)))
		goto enobufs;
	if (mp) {
		mp->b_datap->db_type = type;
		mp->b_wptr = mp->b_rptr + psize;
		bzero(mp->b_rptr, psize);
		mp->b_cont = dp;
	}
	if (dp) {
		dp->b_datap->db_type = M_DATA;
		dp->b_wptr = dp->b_rptr + dsize;
		bzero(dp->b_rptr, dsize);
	}
	return (mp ? mp : dp);
      enobufs:
	if (mp)
		freemsg(mp);
	if (dp)
		freemsg(dp);
	return (NULL);
}

/* these two are included here because we need the STREAMS version of dev_t */
__SVR3_EXTERN_INLINE major_t
emajor(dev_t dev)
{
	return (getmajor(dev) + MAJOR(getminor(dev)));
}
__SVR3_EXTERN_INLINE minor_t
eminor(dev_t dev)
{
	return (MINOR(getminor(dev)));
}

#else
#ifdef _SVR3_SOURCE
#warning "_SVR3_SOURCE defined but not CONFIG_STREAMS_COMPAT_SVR3"
#endif
#endif				/* CONFIG_STREAMS_COMPAT_SVR3 */

#endif				/* __SYS_SVR3_DDI_H__ */
