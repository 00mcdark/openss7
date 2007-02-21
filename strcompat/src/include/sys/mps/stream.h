/*****************************************************************************

 @(#) $Id: stream.h,v 0.9.2.17 2007/02/21 01:09:16 brian Exp $

 -----------------------------------------------------------------------------

 Copyright (c) 2001-2006  OpenSS7 Corporation <http://www.openss7.com/>
 Copyright (c) 1997-2001  Brian F. G. Bidulock <bidulock@openss7.org>

 All Rights Reserved.

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free Software
 Foundation; version 2 of the License.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 details.

 You should have received a copy of the GNU General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 675 Mass
 Ave, Cambridge, MA 02139, USA.

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

 Last Modified $Date: 2007/02/21 01:09:16 $ by $Author: brian $

 -----------------------------------------------------------------------------

 $Log: stream.h,v $
 Revision 0.9.2.17  2007/02/21 01:09:16  brian
 - updating mtp.c driver, better mi_open allocators

 Revision 0.9.2.16  2007/02/14 14:09:15  brian
 - broad changes updating support for SS7 MTP and M3UA

 Revision 0.9.2.15  2007/01/28 01:09:56  brian
 - updated test programs and working up m2ua-as driver

 Revision 0.9.2.14  2006/12/19 11:47:57  brian
 - better mi_bufcall implementation

 Revision 0.9.2.13  2006/12/19 00:51:53  brian
 - corrections to mi_open/close functions

 Revision 0.9.2.12  2006/12/08 05:08:14  brian
 - some rework resulting from testing and inspection

 Revision 0.9.2.11  2006/11/03 10:39:21  brian
 - updated headers, correction to mi_timer_expiry type

 Revision 0.9.2.10  2006/07/24 09:00:57  brian
 - results of udp2 optimizations

 Revision 0.9.2.9  2006/06/22 13:11:25  brian
 - more optmization tweaks and fixes

 Revision 0.9.2.8  2006/05/23 10:44:04  brian
 - mark normal inline functions for unlikely text section

 Revision 0.9.2.7  2005/12/28 09:51:48  brian
 - remove warnings on FC4 compile

 Revision 0.9.2.6  2005/07/22 06:06:50  brian
 - working up streams/src/kernel/strsched.h

 Revision 0.9.2.5  2005/07/18 15:52:10  brian
 - implemented mps mpprintf functions

 Revision 0.9.2.4  2005/07/18 12:25:40  brian
 - standard indentation

 Revision 0.9.2.3  2005/07/15 23:09:13  brian
 - checking in for sync

 Revision 0.9.2.2  2005/07/14 22:03:54  brian
 - updates for check pass and header splitting

 Revision 0.9.2.1  2005/07/12 13:54:43  brian
 - changes for os7 compatibility and check pass

 *****************************************************************************/

#ifndef __SYS_MPS_STREAM_H__
#define __SYS_MPS_STREAM_H__

#ident "@(#) $RCSfile: stream.h,v $ $Name:  $($Revision: 0.9.2.17 $) Copyright (c) 2001-2006 OpenSS7 Corporation."

#ifndef __SYS_STREAM_H__
#warning "Do not include sys/mps/stream.h directly, include sys/stream.h instead."
#endif

#ifndef __KERNEL__
#error "Do not include kernel header files in user space programs."
#endif

#ifndef __EXTERN_INLINE
#define __EXTERN_INLINE extern __inline__
#endif

#ifndef __MPS_EXTERN_INLINE
#define __MPS_EXTERN_INLINE __EXTERN_INLINE streamscall
#endif

#ifndef _MPS_SOURCE
#warning "_MPS_SOURCE not defined but MPS stream.h included."
#endif

#include <sys/strcompat/config.h>

#if defined CONFIG_STREAMS_COMPAT_MPS || defined CONFIG_STREAMS_COMPAT_MPS_MODULE

#ifndef dev_t
#define dev_t __streams_dev_t
#endif

/*
 *  Module or driver open and close helper functions.
 */
extern void *mi_close_obj(caddr_t ptr);
extern size_t mi_open_size(size_t size);
extern size_t mi_close_size(caddr_t ptr);
extern caddr_t mi_open_obj(void *obj, size_t size);
extern caddr_t mi_open_alloc(size_t size);
extern caddr_t mi_open_alloc_sleep(size_t size);
extern caddr_t mi_open_alloc_cache(kmem_cache_t *cachep, int flag);
extern caddr_t mi_first_ptr(caddr_t *mi_head);
extern caddr_t mi_first_dev_ptr(caddr_t *mi_head);
extern caddr_t mi_next_ptr(caddr_t ptr);
extern caddr_t mi_next_dev_ptr(caddr_t *mi_head, caddr_t ptr);
extern caddr_t mi_prev_ptr(caddr_t ptr);
extern int mi_open_link(caddr_t *mi_head, caddr_t ptr, dev_t *devp, int flag, int sflag,
			cred_t *credp);
extern void mi_close_free(caddr_t ptr);
extern void mi_close_free_cachep(kmem_cache_t *cachep, caddr_t ptr);

__MPS_EXTERN_INLINE caddr_t
mi_open_detached(caddr_t *mi_head, size_t size, dev_t *devp)
{
	caddr_t ptr;
	int err;

	if (!(ptr = mi_open_alloc_sleep(size)))
		return (NULL);
	if (!(err = mi_open_link(mi_head, ptr, devp, 0, DRVOPEN, NULL)))
		return (ptr);
	mi_close_free(ptr);
	return (NULL);
}

extern void mi_attach(queue_t *q, caddr_t ptr);

__MPS_EXTERN_INLINE int
mi_open_comm(caddr_t *mi_head, size_t size, queue_t *q, dev_t *devp, int flag, int sflag,
	     cred_t *credp)
{
	caddr_t ptr;
	int err;

	if (q->q_ptr != NULL)
		return (0);	/* already open */
	if (sflag == MODOPEN) {
		if (!WR(q)->q_next)
			return (EIO);
	} else {
		if (WR(q)->q_next && SAMESTR(q))
			return (EIO);
	}
	if (!(ptr = mi_open_alloc_sleep(size)))
		return (EAGAIN);
	if (!(err = mi_open_link(mi_head, ptr, devp, flag, sflag, credp))) {
		mi_attach(q, ptr);
		return (0);
	}
	mi_close_free(ptr);
	return (err);
}

extern void mi_close_unlink(caddr_t *mi_head, caddr_t ptr);

extern void mi_detach(queue_t *q, caddr_t ptr);

__MPS_EXTERN_INLINE void
mi_close_detached(caddr_t *mi_head, caddr_t ptr)
{
	mi_close_unlink(mi_head, ptr);
	mi_close_free(ptr);
}

__MPS_EXTERN_INLINE int
mi_close_comm(caddr_t *mi_head, queue_t *q)
{
	caddr_t ptr = q->q_ptr;

	mi_detach(q, ptr);
	mi_close_detached(mi_head, ptr);
	return (0);
}

/*
 *  Timer helper functions.
 */

/* MacOT flavor */
extern mblk_t *mi_timer_alloc_MAC(queue_t *q, size_t size);
extern void mi_timer_MAC(mblk_t *mp, clock_t msec);
extern int mi_timer_cancel(mblk_t *mp);
extern mblk_t *mi_timer_q_switch(mblk_t *mp, queue_t *q, mblk_t *new_mp);

/* OpenSolaris flavor */
extern mblk_t *mi_timer_alloc_SUN(size_t size);
extern void mi_timer_SUN(queue_t *q, mblk_t *mp, clock_t msec);
extern void mi_timer_stop(mblk_t *mp);
extern void mi_timer_move(queue_t *q, mblk_t *mp);

/* common */
extern int mi_timer_valid(mblk_t *mp);
extern void mi_timer_free(mblk_t *mp);
extern unsigned long mi_timer_remain(mblk_t *mp);

/*
 *  Locking helper function.
 */
extern caddr_t mi_trylock(queue_t *q);
extern caddr_t mi_sleeplock(queue_t *q);
extern void mi_unlock(caddr_t ptr);

/*
 *  Buffer call helper function.
 */
extern void mi_bufcall(queue_t *q, int size, int priority);

/*
 *  Message block allocation helper functions.
 */
extern mblk_t *mi_reuse_proto(mblk_t *mp, size_t size, int keep_on_error);
extern mblk_t *mi_reallocb(mblk_t *mp, size_t size);

__MPS_EXTERN_INLINE mblk_t *
mi_allocb(queue_t *q, size_t size, int priority)
{
	mblk_t *mp;

	if (unlikely((mp = allocb(size, priority)) == NULL))
		mi_bufcall(q, size, priority);
	return (mp);
}

/*
 *  M_IOTCL handling helper functions.
 */
#define MI_COPY_IN			1
#define MI_COPY_OUT			2
#define MI_COPY_CASE(_dir, _cnt)	(((_cnt)<<2|_dir))

extern void mi_copyin(queue_t *q, mblk_t *mp, caddr_t uaddr, size_t len);
extern void mi_copyin_n(queue_t *q, mblk_t *mp, size_t offset, size_t len);
extern void mi_copyout(queue_t *q, mblk_t *mp);
extern mblk_t *mi_copyout_alloc(queue_t *q, mblk_t *mp, caddr_t uaddr, size_t len,
				int free_on_error);
extern void mi_copy_done(queue_t *q, mblk_t *mp, int err);
extern int mi_copy_state(queue_t *q, mblk_t *mp, mblk_t **mpp);
extern void mi_copy_set_rval(mblk_t *mp, int rval);

/*
 *  Stream head helper functions.
 */
extern int mi_set_sth_hiwat(queue_t *q, size_t size);
extern int mi_set_sth_lowat(queue_t *q, size_t size);
extern int mi_set_sth_maxblk(queue_t *q, ssize_t size);
extern int mi_set_sth_copyopt(queue_t *q, int copyopt);
extern int mi_set_sth_wroff(queue_t *q, size_t size);

/*
 *  STREAMS wrapper functions.
 */
extern queue_t *mi_allocq(struct streamtab *st);
extern void mi_freeq(queue_t *q);
extern int mi_strlog(queue_t *q, char level, ushort flags, char *fmt, ...)
    __attribute__ ((format(printf, 4, 5)));
extern int mi_mpprintf(mblk_t *mp, char *fmt, ...) __attribute__ ((format(printf, 2, 3)));
extern int mi_mpprintf_nr(mblk_t *mp, char *fmt, ...) __attribute__ ((format(printf, 2, 3)));

/*
 *  Message block functions
 */
extern uint8_t *mi_offset_param(mblk_t *mp, size_t offset, size_t len);
extern uint8_t *mi_offset_paramc(mblk_t *mp, size_t offset, size_t len);

/*
 *  Some internals showing.
 */
typedef void (*proc_ptr_t) (queue_t *, mblk_t *);
extern void mps_become_writer(queue_t *q, mblk_t *mp, proc_ptr_t proc);

#else
#ifdef _MPS_SOURCE
#warning "_MPS_SOURCE defined by not CONFIG_STREAMS_COMPAT_MPS"
#endif
#endif

#endif				/* __SYS_MPS_STREAM_H__ */
