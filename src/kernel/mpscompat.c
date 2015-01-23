/*****************************************************************************

 @(#) File: src/kernel/mpscompat.c

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2015  Monavacon Limited <http://www.monavacon.com/>
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

 *****************************************************************************/

static char const ident[] = "File: " __FILE__ "  Version: " PACKAGE_ENVR "  Date: " PACKAGE_DATE;

/* 
 *  This is my solution for those who don't want to inline GPL'ed functions or who don't use
 *  optimizations when compiling or specifies -fnoinline-functions or something of the like.  This
 *  file implements all of the extern inlines from the header files by just including the header
 *  files with the functions declared 'inline' instead of 'extern inline'.
 *
 *  There are implemented here in a separate object, out of the way of the modules that don't use
 *  them.
 */

#define __MPS_EXTERN_INLINE __inline__ streamscall __unlikely
#define __MPS_EXTERN streamscall

#define _MPS_SOURCE

#include "sys/os7/compat.h"

#include <linux/types.h>	/* for ptrdiff_t */
#include <linux/ctype.h>	/* for isdigit */
#include <asm/div64.h>		/* for do_div */

#define MPSCOMP_DESCRIP		"Mentat Portable STREAMS Compatibility for Linux Fast-STREAMS"
#define MPSCOMP_EXTRA		"UNIX SYSTEM V RELEASE 4.2 FAST STREAMS FOR LINUX"
#define MPSCOMP_COPYRIGHT	"Copyright (c) 2008-2015  Monavacon Limited.  All Rights Reserved."
#define MPSCOMP_REVISION	"OpenSS7 File: " __FILE__ "  Version: " PACKAGE_ENVR "  Date: " PACKAGE_DATE
#define MPSCOMP_DEVICE		"Mentat Portable STREAMS Compatibility"
#define MPSCOMP_CONTACT		"Brian Bidulock <bidulock@openss7.org>"
#define MPSCOMP_LICENSE		"GPL"
#define MPSCOMP_BANNER		MPSCOMP_DESCRIP		"\n" \
				MPSCOMP_EXTRA		"\n" \
				MPSCOMP_COPYRIGHT	"\n" \
				MPSCOMP_REVISION	"\n" \
				MPSCOMP_DEVICE		"\n" \
				MPSCOMP_CONTACT		"\n"
#define MPSCOMP_SPLASH		MPSCOMP_DEVICE		" - " \
				MPSCOMP_REVISION	"\n"

#ifdef CONFIG_STREAMS_COMPAT_MPS_MODULE
MODULE_AUTHOR(MPSCOMP_CONTACT);
MODULE_DESCRIPTION(MPSCOMP_DESCRIP);
MODULE_SUPPORTED_DEVICE(MPSCOMP_DEVICE);
MODULE_LICENSE(MPSCOMP_LICENSE);
#if defined MODULE_ALIAS
MODULE_ALIAS("streams-mpscompat");
#endif
#ifdef MODULE_VERSION
MODULE_VERSION(PACKAGE_ENVR);
#endif
#endif

/**
 * mi_bcmp: - bcmp() replacement
 * @s1: first byte string
 * @s2: second byte string
 * @len: length to compare
 */
__MPS_EXTERN_INLINE int mi_bcmp(const void *s1, const void *s2, size_t len);

EXPORT_SYMBOL(mi_bcmp);

/**
 * mi_alloc: - kmem_alloc() replacement
 * @size: size of memory extent to allocate
 * @pri: priority
 */
__MPS_EXTERN_INLINE void *mi_alloc(size_t size, unsigned int pri);

EXPORT_SYMBOL(mi_alloc);	/* mps/ddi.h */

/**
 * mi_alloc_sleep: - kmem_alloc() replacement
 * @size: size of memory extent to allocate
 * @pri: priority
 */
__MPS_EXTERN_INLINE void *mi_alloc_sleep(size_t size, unsigned int pri);

EXPORT_SYMBOL(mi_alloc_sleep);	/* mps/ddi.h */

/**
 * mi_zalloc: - kmem_zalloc() replacement
 * @size: size of memory extent to allocate and zero
 */
__MPS_EXTERN_INLINE caddr_t mi_zalloc(size_t size);

EXPORT_SYMBOL(mi_zalloc);	/* mps/ddi.h */

/*
 * mi_zalloc_sleep: - kmem_zalloc() replacement
 * @size: size of memory extent to allocate and zero
 */
__MPS_EXTERN_INLINE caddr_t mi_zalloc_sleep(size_t size);

EXPORT_SYMBOL(mi_zalloc_sleep);	/* mps/ddi.h */

/**
 * mi_free: - kmem_free() replacement
 * @ptr: pointer to memory extent to free
 */
__MPS_EXTERN_INLINE void mi_free(void *ptr);

EXPORT_SYMBOL(mi_free);		/* mps/ddi.h */

/*
 *  =========================================================================
 *
 *  Module or driver open and close helper functions.
 *
 *  =========================================================================
 */
struct mi_comm {
	struct mi_comm **mi_prev;	/* must be first */
	struct mi_comm **mi_head;
	struct mi_comm *mi_next;
	size_t mi_size;			/* size of this structure plus private data */
	unsigned short mi_mid;		/* module identifier */
	unsigned short mi_sid;		/* stream identifier */
	bcid_t mi_bcid;			/* queue bufcall */
	spinlock_t mi_lock;		/* structure lock */
	long mi_users;			/* users */
	long mi_flags;			/* wait flags */
	queue_t *mi_q;			/* attached read queue */
	atomic_t mi_refs;		/* references to structure */
	struct mi_comm *mi_waiter;	/* waiter on lock */
	wait_queue_head_t mi_waitq;	/* processes waiting on lock */
	kmem_cachep_t mi_cachep;	/* cache pointer */
	union {
		dev_t dev;		/* device number (or NODEV for modules) */
		int index;		/* link index */
	} mi_u;
	char mi_ptr[0];			/* followed by private data */
};

#define MI_WAIT_LOCKED_BIT	0
#define MI_WAIT_RQ_BIT		1
#define MI_WAIT_WQ_BIT		2
#define MI_WAIT_SLEEP_BIT	3

#define MI_WAIT_LOCKED		(1<<MI_WAIT_LOCKED_BIT)
#define MI_WAIT_RQ		(1<<MI_WAIT_RQ_BIT)
#define MI_WAIT_WQ		(1<<MI_WAIT_WQ_BIT)
#define MI_WAIT_SLEEP		(1<<MI_WAIT_SLEEP_BIT)

#define mi_dev mi_u.dev
#define mi_index mi_u.index

#define mi_to_ptr(mi) ((mi) ? (mi)->mi_ptr : NULL)
#define ptr_to_mi(ptr) ((ptr) ? (struct mi_comm *)(ptr) - 1 : NULL)

static void
mi_grab(struct mi_comm *mi)
{
	assert(mi);
	assert(atomic_read(&mi->mi_refs) > 0);
	atomic_inc(&mi->mi_refs);
}

/**
 * mi_open_grab: - grab a reference to an open structure
 * @ptr: a pointer to the user portion of the structure
 *
 * Grabs a reference to the open structure referenced by @ptr and returns a pointer to the user
 * portion of the structure.  If the passed in pointer is invalid or NULL, the function returns NULL.
 */
__MPS_EXTERN caddr_t
mi_open_grab(caddr_t ptr)
{
	assert(ptr);
	if (ptr)
		mi_grab(ptr_to_mi(ptr));
	return (ptr);
}

EXPORT_SYMBOL_GPL(mi_open_grab);

noinline void mi_unbufcall(struct mi_comm *mi);
noinline void mi_wakepriv(struct mi_comm *mi);

static struct mi_comm *
mi_put(struct mi_comm *mi)
{
	assert(mi);
	assert(atomic_read(&mi->mi_refs) > 0);
	if (atomic_dec_and_test(&mi->mi_refs)) {
		if (mi->mi_q != NULL)
			swerr();	// mi_detach(mi->mi_q, ptr);
		if (mi->mi_head != NULL)
			swerr();	// mi_close_unlink((caddr_t *)mi->mi_head, ptr);
		if (mi->mi_bcid != 0)
			mi_unbufcall(mi);
		if (mi->mi_waiter != NULL)
			mi_wakepriv(mi);
		if (mi->mi_cachep)
			kmem_cache_free(mi->mi_cachep, mi);
		else
			kmem_free(mi, mi->mi_size);
		mi = NULL;
	}
	return (mi);
}

/**
 * mi_close_put: - release a reference to an open structure
 * @ptr: a pointer to the user portion of the structure
 *
 * Release a reference from the open structure referenced by @ptr and returns a pointer to the user
 * portion of the structure.  If the reference is the last reference released, the open structure is
 * detached, unlinked and deallocated as necessary.  It the passed pointer is invalid or NULL, the
 * function simply returns NULL.
 */
__MPS_EXTERN caddr_t
mi_close_put(caddr_t ptr)
{
	if (ptr)
		ptr = mi_to_ptr(mi_put(ptr_to_mi(ptr)));
	return (ptr);
}

EXPORT_SYMBOL_GPL(mi_close_put);

/**
 * mi_open_size: - obtain size of open structure
 * @size: size of user portion
 * 
 * Calculates and returns the size of the structure necessary for the user to allocate to use the
 * mi_open procedures with the structure.  The allocated structure must be initialized using
 * mi_open_obj.
 */
__MPS_EXTERN size_t
mi_open_size(size_t size)
{
	return (size + sizeof(struct mi_comm));
}

EXPORT_SYMBOL_GPL(mi_open_size);

/**
 * mi_open_obj: - initialize open structure
 * @obj: pointer to allocated memory extent
 * @size: size of user portion (zero for cache allocated)
 * @cachep: memory cache pointer (NULL for kmem allocated)
 *
 * Initializes a user allocated structure for use by the mi_open routines and returns a pointer to
 * the user portion.  The structure allocated should be of the size returned by mi_open_size().  The
 * pointer returned points to the user portion of the structure.  The object pointer can be obtained
 * using the mi_close_obj() function.
 *
 * Note that this function will return NULL if passed NULL.
 */
__MPS_EXTERN caddr_t
mi_open_obj(void *obj, size_t size, kmem_cachep_t cachep)
{
	struct mi_comm *mi;

	if ((mi = (typeof(mi)) obj)) {
		bzero(mi, sizeof(*mi));
		mi->mi_prev = mi->mi_head = &mi->mi_next;
		mi->mi_size = size;
		mi->mi_cachep = cachep;
		spin_lock_init(&mi->mi_lock);
		atomic_set(&mi->mi_refs, 1);
		init_waitqueue_head(&mi->mi_waitq);
	}
	return (mi_to_ptr(mi));
}

EXPORT_SYMBOL_GPL(mi_open_obj);

/**
 * mi_close_obj: - obtain an pointer to the object to free
 * @ptr: a pointer to the user portion of the structure
 *
 * Returns a pointer to the memory extent to free when deallocating a user allocated structure for
 * use with mi_open routines.  The purpose of these functions are to permit the user to provide
 * their own allocation schemes (such as memory caches).
 * 
 */
__MPS_EXTERN void *
mi_close_obj(caddr_t ptr)
{
	return ((caddr_t) ptr_to_mi(ptr));
}

EXPORT_SYMBOL_GPL(mi_close_obj);

/**
 * mi_close_size: - obtain size of close structure
 * @ptr: a pointer to the user portion of the structure
 *
 * Calculates and returns the size of the structure necessary for the user to deallocate.
 */
__MPS_EXTERN size_t
mi_close_size(caddr_t ptr)
{
	struct mi_comm *mi = ptr_to_mi(ptr);

	return (mi_open_size(mi->mi_size));
}

EXPORT_SYMBOL_GPL(mi_close_size);

__MPS_EXTERN caddr_t
mi_open_alloc_cache(kmem_cachep_t cachep, int flag)
{
	struct mi_comm *mi;

	if ((mi = kmem_cache_alloc(cachep, flag)))
		return mi_open_obj(mi, 0, cachep);
	return (NULL);
}

EXPORT_SYMBOL_GPL(mi_open_alloc_cache);

/**
 * mi_close_free_cache: - free a private structure
 * @cachep: memory cache from which structure was allocated
 * @ptr: private structure to free
 *
 * This function does not actually free the structure but removes a reference from the structure.
 * When the last reference is removed, the structure will be freed.
 */
__MPS_EXTERN void
mi_close_free_cache(kmem_cachep_t cachep, caddr_t ptr)
{
	(void) cachep;
	mi_close_put(ptr);
}

EXPORT_SYMBOL_GPL(mi_close_free_cache);

static caddr_t
mi_open_alloc_flag(size_t size, int flag)
{
	struct mi_comm *mi;

	if ((mi = kmem_zalloc(mi_open_size(size), flag)))
		return mi_open_obj(mi, size, NULL);
	return (NULL);
}

/*
 *  MI_OPEN_ALLOC
 *  -------------------------------------------------------------------------
 */
__MPS_EXTERN caddr_t
mi_open_alloc(size_t size)
{
	return mi_open_alloc_flag(size, KM_NOSLEEP);
}

EXPORT_SYMBOL(mi_open_alloc);	/* mps/ddi.h */

/*
 *  MI_OPEN_ALLOC_SLEEP
 *  -------------------------------------------------------------------------
 */
__MPS_EXTERN caddr_t
mi_open_alloc_sleep(size_t size)
{
	return mi_open_alloc_flag(size, KM_SLEEP);
}

EXPORT_SYMBOL(mi_open_alloc_sleep);	/* mps/ddi.h */

/**
 * mi_first_ptr: - get the first pointer from a head list
 * @mi_head: head list
 *
 * OpenSolaris uses a head structure, we don't, primarily because the documentation for mi_open_comm
 * for both AIX and MacOT says that we use a "static pointer" initialized to NULL.
 */
__MPS_EXTERN caddr_t
mi_first_ptr(caddr_t *mi_head)
{
	struct mi_comm *mi = *(struct mi_comm **) mi_head;

	return (mi_to_ptr(mi));
}

EXPORT_SYMBOL(mi_first_ptr);	/* mps/ddi.h */

/**
 * mi_first_dev_ptr: - get first device pointer from a head list
 * @mi_head: head list
 *
 * OpenSolaris tracks modules and drivers on the same list.  We don't, primarily because AIX and
 * MacOT don't document it that way.  If you have a STREAMS driver that can also be pushed as a
 * module, use two separate lists.
 */
__MPS_EXTERN caddr_t
mi_first_dev_ptr(caddr_t *mi_head)
{
	struct mi_comm *mi = *(struct mi_comm **) mi_head;

	for (; mi && getmajor(mi->mi_dev) != 0; mi = mi->mi_next) ;
	return (mi_to_ptr(mi));
}

EXPORT_SYMBOL(mi_first_dev_ptr);	/* mps/ddi.h */

/**
 * mi_next_ptr: - get the next pointer in head list
 * @ptr: previous pointer
 */
__MPS_EXTERN caddr_t
mi_next_ptr(caddr_t ptr)
{
	struct mi_comm *mi = ptr_to_mi(ptr);

	if (mi)
		mi = mi->mi_next;
	return (mi_to_ptr(mi));
}

EXPORT_SYMBOL(mi_next_ptr);	/* mps/ddi.h, aix/ddi.h */

/**
 * mi_next_dev_ptr: - get the next device pointer in head list
 * @mi_head: - head list
 * @ptr: previous pointer
 *
 * OpenSolaris tracks modules and drivers on the same list.  We don't, primarily because AIX and
 * MacOT don't document it that way.  If you have a STREAMS driver that can also be pushed as a
 * module, use two separate lists.
 */
__MPS_EXTERN caddr_t
mi_next_dev_ptr(caddr_t *mi_head, caddr_t ptr)
{
	struct mi_comm *mi = ptr_to_mi(ptr);

	for (mi = mi ? mi->mi_next : NULL; mi && getmajor(mi->mi_dev) != 0; mi = mi->mi_next) ;
	return (mi_to_ptr(mi));
}

EXPORT_SYMBOL(mi_next_dev_ptr);	/* mps/ddi.h */

/**
 * mi_prev_ptr: - get the previous element in head list
 * @ptr: current pointer
 *
 * Linux Fast-STREAMS embellishment.
 */
__MPS_EXTERN caddr_t
mi_prev_ptr(caddr_t ptr)
{
	struct mi_comm *mi = ptr_to_mi(ptr);

	if (mi && mi->mi_prev != &mi->mi_next && mi->mi_prev != mi->mi_head)
		return ((caddr_t) (((struct mi_comm *) mi->mi_prev) + 1));
	return (NULL);
}

EXPORT_SYMBOL(mi_prev_ptr);	/* mps/ddi.h, aix/ddi.h */

#if	defined DEFINE_SPINLOCK
static DEFINE_SPINLOCK(mi_list_lock);
#elif	defined __SPIN_LOCK_UNLOCKED
static spinlock_t mi_list_lock = __SPIN_LOCK_UNLOCKED(mi_list_lock);
#elif	defined SPIN_LOCK_UNLOCKED
static spinlock_t mi_list_lock = SPIN_LOCK_UNLOCKED;
#else
#error cannot initialize spin locks
#endif


/**
 * mi_open_link: - link a private structure into head list
 * @mi_head: head list
 * @ptr: pointer to private structure
 * @devp: device pointer from qi_qopen()
 * @flag: open flags from qi_qopen()
 * @sflag: STREAMS flag from qi_qopen()
 * @credp: credentials pointer from qi_qopen()
 */
__MPS_EXTERN int
mi_open_link(caddr_t *mi_head, caddr_t ptr, dev_t *devp, int flag, int sflag, cred_t *credp)
{
	struct mi_comm *mi, **mip;
	major_t cmajor = devp ? getmajor(*devp) : 0;
	minor_t cminor = devp ? getminor(*devp) : 0;

	if (mi_head == NULL || (mi = ptr_to_mi(ptr)) == NULL)
		return (EINVAL);

	switch (sflag) {
	case CLONEOPEN:
		/* first clone minor (above 5 per AIX docs, above 10 per MacOT docs), but the
		   caller can start wherever they want above that */
#define MI_OPEN_COMM_CLONE_MINOR 10
		if (cminor <= MI_OPEN_COMM_CLONE_MINOR)
			cminor = MI_OPEN_COMM_CLONE_MINOR + 1;
		/* fall through */
	case DRVOPEN:
		/* devp must be specified for DRVOPEN or CLONEOPEN */
		if (devp == NULL)
			return (EINVAL);
		break;
	case MODOPEN:
		cmajor = 0;
		/* caller can specify a preferred instance number */
		if (cminor == 0)
			cminor = 1;
		break;
	default:
		/* invalid sflag */
		return (EINVAL);
	}

	mi_grab(mi);
	spin_lock(&mi_list_lock);
	for (mip = (struct mi_comm **) mi_head; *mip; mip = &(*mip)->mi_next) {
		if (cmajor != (*mip)->mi_mid)
			break;
		if (cmajor == (*mip)->mi_mid) {
			if (cminor < (*mip)->mi_sid)
				break;
			if (cminor > (*mip)->mi_sid)
				continue;
			if (cminor == (*mip)->mi_sid) {
				if (sflag == DRVOPEN) {
					/* conflicting device number */
					spin_unlock(&mi_list_lock);
					mi_put(mi);
					return (ENXIO);
				}
				if (getminor(makedevice(0, ++cminor)) == 0) {
					/* no minor device numbers left */
					spin_unlock(&mi_list_lock);
					mi_put(mi);
					return (EAGAIN);
				}
				continue;
			}
		}
	}

	mi->mi_dev = makedevice(cmajor, cminor);
	mi->mi_mid = cmajor;
	mi->mi_sid = cminor;
	if ((mi->mi_next = *mip))
		mi->mi_next->mi_prev = &mi;
	mi->mi_prev = mip;
	*mip = mi;
	mi->mi_head = (struct mi_comm **) mi_head;
	spin_unlock(&mi_list_lock);
	if (sflag == CLONEOPEN)
		/* must return unique device number */
		*devp = makedevice(cmajor, cminor);
	return (0);
}

EXPORT_SYMBOL(mi_open_link);	/* mps/ddi.h */

/**
 * mi_open_detached: - detached open
 * @mi_head: private structure head
 * @size: size of private structure to allocate
 * @devp: device pointer from qi_qopen()
 */
__MPS_EXTERN_INLINE caddr_t mi_open_detached(caddr_t *mi_head, size_t size, dev_t *devp);

EXPORT_SYMBOL(mi_open_detached);	/* mps/ddi.h */

/**
 * mi_attach: - attach a private structure to a queue
 * @q: queue to which to attach
 * @ptr: private structure
 */
__MPS_EXTERN void
mi_attach(queue_t *q, caddr_t ptr)
{
	struct mi_comm *mi = ptr_to_mi(ptr);
	unsigned long flags;

	mi_grab(mi);
	/* Protect dereference of mi->mi_q and resetting mi_flags. */
	spin_lock_irqsave(&mi->mi_lock, flags);
	mi->mi_mid = q->q_qinfo->qi_minfo->mi_idnum;
	mi->mi_q = q;
	q->q_ptr = WR(q)->q_ptr = ptr;
	spin_unlock_irqrestore(&mi->mi_lock, flags);
}

EXPORT_SYMBOL(mi_attach);	/* mps/ddi.h, mac/ddi.h */

/**
 * mi_open_comm: - perform qi_qopen() housekeeping
 * @mi_head: private structure list head
 * @size: size of private structure to allocate
 * @q: queue pointer from qi_qopen()
 * @devp: device pointer from qi_qopen()
 * @flag: open flags from qi_qopen()
 * @sflag: STREAMS flag from qi_qopen()
 * @credp: credentials pointer from qi_qopen()
 */
__MPS_EXTERN_INLINE int mi_open_comm(caddr_t *mi_head, size_t size, queue_t *q, dev_t *devp,
				     int flag, int sflag, cred_t *credp);
EXPORT_SYMBOL(mi_open_comm);	/* mps/ddi.h, aix/ddi.h */

/**
 * mi_close_unlink: - unlink a private structure
 * @mi_head: private structure list head
 * @ptr: private structure to unlink
 */
__MPS_EXTERN void
mi_close_unlink(caddr_t *mi_head, caddr_t ptr)
{
	struct mi_comm *mi = ptr_to_mi(ptr);

	if (mi) {
		spin_lock(&mi_list_lock);
		if ((*mi->mi_prev = mi->mi_next))
			mi->mi_next->mi_prev = mi->mi_prev;
		mi->mi_next = NULL;
		mi->mi_prev = &mi->mi_next;
		mi->mi_head = NULL;
		spin_unlock(&mi_list_lock);
		mi_put(mi);
	}
}

EXPORT_SYMBOL(mi_close_unlink);	/* mps/ddi.h */

/**
 * mi_close_free: - free a private structure
 * @ptr: private structure to free
 *
 * This function does not actually free the structure but removes a reference from the structure.
 * When the last reference is removed, the structure will be freed.
 */
__MPS_EXTERN void
mi_close_free(caddr_t ptr)
{
	mi_close_put(ptr);
}

EXPORT_SYMBOL(mi_close_free);	/* mps/ddi.h */

/**
 * mi_detach: - detach a private structure from a queue pair
 * @q: read queue of queue pair
 * @ptr: private structure to detach
 *
 * Normally one should perform qprocsoff(9) before calling this function or another housekeeping
 * function that calls this one.
 */
__MPS_EXTERN void
mi_detach(queue_t *q, caddr_t ptr)
{
	struct mi_comm *mi = ptr_to_mi(ptr);

	if (mi) {
		unsigned long flags;

		/* Protect dereference of mi->mi_q and resetting mi_flags. */
		spin_lock_irqsave(&mi->mi_lock, flags);
		mi->mi_flags = 0;
		mi->mi_q = NULL;
		q->q_ptr = WR(q)->q_ptr = NULL;
		spin_unlock_irqrestore(&mi->mi_lock, flags);
		mi_put(mi);
	} else {
		q->q_ptr = WR(q)->q_ptr = NULL;
	}
}

EXPORT_SYMBOL(mi_detach);	/* mps/ddi.h */

/**
 * mi_close_detached: - close a detached private structure
 * @mi_head: private structure list head
 * @ptr: private structure to unlink and free
 */
__MPS_EXTERN_INLINE void mi_close_detached(caddr_t *mi_head, caddr_t ptr);

EXPORT_SYMBOL(mi_close_detached);	/* mps/ddi.h */

/**
 * mi_close_comm: - perform common qi_qclose() housekeeping
 * @mi_head: private structure list head
 * @q: read queue of queue pair
 */
__MPS_EXTERN_INLINE int mi_close_comm(caddr_t *mi_head, queue_t *q);

EXPORT_SYMBOL(mi_close_comm);	/* mps/ddi.h, aix/ddi.h */

/**
 * mi_open_detached_cache: - detached open
 * @mi_head: private structure head
 * @cachep: cache pointer for appropriately sized private structure cache
 * @devp: device pointer from qi_qopen()
 */
__MPS_EXTERN_INLINE caddr_t mi_open_detached_cache(caddr_t *mi_head, kmem_cachep_t cachep,
						   dev_t *devp);
EXPORT_SYMBOL_GPL(mi_open_detached_cache);

/**
 * mi_open_comm_cache: - perform qi_qopen() housekeeping
 * @mi_head: private structure list head
 * @cachep: cache pointer for appropriately sized private structure cache
 * @q: queue pointer from qi_qopen()
 * @devp: device pointer from qi_qopen()
 * @flag: open flags from qi_qopen()
 * @sflag: STREAMS flag from qi_qopen()
 * @credp: credentials pointer from qi_qopen()
 */
__MPS_EXTERN_INLINE int mi_open_comm_cache(caddr_t *mi_head, kmem_cachep_t cachep, queue_t *q,
					   dev_t *devp, int flag, int sflag, cred_t *credp);
EXPORT_SYMBOL_GPL(mi_open_comm_cache);

/**
 * mi_close_detached_cache: - close a detached cached private structure
 * @mi_head: private structure list head
 * @cachep: cache pointer for private structure cache
 * @ptr: private structure to unlink and free
 */
__MPS_EXTERN_INLINE void mi_close_detached_cache(caddr_t *mi_head, kmem_cachep_t cachep,
						 caddr_t ptr);
EXPORT_SYMBOL_GPL(mi_close_detached_cache);

/**
 * mi_close_comm_cache: - perform common qi_qclose() housekeeping
 * @mi_head: private structure list head
 * @cachep: cache pointer for private structure cache
 * @q: read queue of queue pair
 */
__MPS_EXTERN_INLINE int mi_close_comm_cache(caddr_t *mi_head, kmem_cachep_t cachep, queue_t *q);

EXPORT_SYMBOL_GPL(mi_close_comm_cache);

struct mi_iocblk {
	caddr_t mi_caddr;		/* uaddr of TRANSPARENT ioctl, NULL for I_STR */
	caddr_t mi_uaddr;		/* uaddr of last copyin operation, NULL for implicit */
	short mi_dir;			/* direction of operation MI_COPY_IN or MI_COPY_OUT */
	short mi_cnt;			/* operation count, starting from 1 for each direction */
	int mi_rval;			/* return value */
};

/*
 *  =========================================================================
 *
 *  Locking helper function.
 *
 *  =========================================================================
 */

noinline fastcall void
mi_wakeold(struct mi_comm *mi)
{
	unsigned long flags;

	/* Protect dereference of mi->mi_q and resetting mi_flags. */
	spin_lock_irqsave(&mi->mi_lock, flags);
	{
		assert(atomic_read(&mi->mi_refs) > 1);
		assert(atomic_read(&mi->mi_refs) < 10);
		if (test_and_clear_bit(MI_WAIT_SLEEP_BIT, &mi->mi_flags))
			if (waitqueue_active(&mi->mi_waitq))
				wake_up_all(&mi->mi_waitq);
		if (test_and_clear_bit(MI_WAIT_RQ_BIT, &mi->mi_flags) && mi->mi_q)
			qenable(RD(mi->mi_q));
		if (test_and_clear_bit(MI_WAIT_WQ_BIT, &mi->mi_flags) && mi->mi_q)
			qenable(WR(mi->mi_q));
	}
	spin_unlock_irqrestore(&mi->mi_lock, flags);
	mi_put(mi);
}

noinline void
mi_wakepriv(struct mi_comm *mi)
{
	unsigned long flags;
	struct mi_comm *mo;

	spin_lock_irqsave(&mi->mi_lock, flags);
	if ((mo = XCHG(&mi->mi_waiter, NULL)) == mi)
		mo = NULL;
	spin_unlock_irqrestore(&mi->mi_lock, flags);
	if (unlikely(mo != NULL))
		mi_wakeold(mo);
}

noinline fastcall struct mi_comm *
__mi_waitpriv(struct mi_comm *mi, queue_t *q, int bit)
{
	struct mi_comm *mw;

	if ((mw = q ? ptr_to_mi(q->q_ptr) : NULL)) {
		/* do not wake old waiter or install new waiter if old same as new */
		assert(atomic_read(&mw->mi_refs) > 1);
		assert(atomic_read(&mw->mi_refs) < 10);
		if (mi->mi_waiter == mw) {
			set_bit(bit, &mw->mi_flags);
			mw = NULL;
		} else {
			mi_grab(mw);
			set_bit(bit, &mw->mi_flags);
			mw = XCHG(&mi->mi_waiter, mw);
		}
	}
	return (mw);
}

static inline fastcall int
__mi_acquire(struct mi_comm *mi, queue_t *q, int bit)
{
	struct mi_comm *mo = NULL;
	unsigned long flags;
	int result;

	assert(mi);
	spin_lock_irqsave(&mi->mi_lock, flags);
	if (unlikely(!(result = !mi->mi_users++))) {
		mo = __mi_waitpriv(mi, q, bit);
		--mi->mi_users;
	}
	spin_unlock_irqrestore(&mi->mi_lock, flags);
	if (unlikely(mo != NULL))
		mi_wakeold(mo);
	return (result);

}

/**
 * mi_acquire: - try to lock a private structure
 * @ptr: pointer to private structure to lock
 * @q: active queue pair associated with locker
 *
 * Returns a pointer to the locked private structure or NULL if the private structure could not be
 * locked or does not exist.
 */
__MPS_EXTERN caddr_t
mi_acquire(caddr_t ptr, queue_t *q)
{
	struct mi_comm *mi;
	int bit;

	mi = ptr_to_mi(ptr);
	bit = q ? ((q->q_flag & QREADR) ? MI_WAIT_RQ_BIT : MI_WAIT_WQ_BIT) : 0;
	return (likely(__mi_acquire(mi, q, bit)) ? ptr : NULL);
}

EXPORT_SYMBOL_GPL(mi_acquire);

/**
 * mi_acquire_sleep: - lock a private structure or wait
 * @ptrw: pointer to private structure of locker
 * @ptrp: pointer to pointer to private structure to lock
 * @lockp: pointer to an irq locked lock protecting *ptrp
 * @flagsp: pointer to saved flags word
 *
 * Returns a pointer to the locked private structure or NULL if the private structure could not be
 * locked because a signal was caught or does not exist.  The caller can test the condition for
 * returning NULL by checking *ptrp.  When a lock pointer is passed, the lock must be write locked
 * before the call and the function returns with the lock write locked.  If a flags pointer is
 * provided, irq write locks will be performed.  The lock must protect the *ptrp dereference.
 *
 * This function will sleep until *ptrp is successfully locked, *ptrp becomes NULL, or the waiting
 * process is interrupted by a signal.
 *
 * This function is typically used to acquire a lock on a lower multiplex private structure from the
 * open or close routine of a upper multiplex private structure.
 */
__MPS_EXTERN caddr_t
mi_acquire_sleep(caddr_t ptrw, caddr_t *ptrp, rwlock_t *lockp, unsigned long *flagsp)
{
	caddr_t ptr = *ptrp;
	struct mi_comm *mi = ptr_to_mi(ptr);
	struct mi_comm *mw = ptr_to_mi(ptrw);

	if (unlikely(!__mi_acquire(mi, mw->mi_q, MI_WAIT_SLEEP_BIT))) {
		DECLARE_WAITQUEUE(wait, current);
		add_wait_queue(&mw->mi_waitq, &wait);
		for (;;) {
			if (unlikely(signal_pending(current))) {
				ptr = NULL;
				break;
			}
			set_current_state(TASK_INTERRUPTIBLE);
			if (likely(__mi_acquire(mi, mw->mi_q, MI_WAIT_SLEEP_BIT)))
				break;
			if (lockp) {
				if (flagsp)
					write_unlock_irqrestore(lockp, *flagsp);
				else
					write_unlock(lockp);
			}
			schedule();
			if (lockp) {
				if (flagsp)
					write_lock_irqsave(lockp, *flagsp);
				else
					write_lock(lockp);
			}
			if (unlikely((ptr = *ptrp) == NULL))
				break;
			mi = ptr_to_mi(ptr);
		}
		set_current_state(TASK_RUNNING);
		remove_wait_queue(&mw->mi_waitq, &wait);
	}
	return (ptr);
}

EXPORT_SYMBOL_GPL(mi_acquire_sleep);

/**
 * mi_acquire_sleep_nosignal: - lock a private structure or wait
 * @ptrw: pointer to private structure of locker
 * @ptrp: pointer to pointer to private structure to lock
 * @lockp: pointer to an irq locked lock protecting *ptrp
 * @flagsp: pointer to saved flags word
 *
 * Just as mi_acquire_sleep(), except will not be woken by a signal.
 */
__MPS_EXTERN caddr_t
mi_acquire_sleep_nosignal(caddr_t ptrw, caddr_t *ptrp, rwlock_t *lockp, unsigned long *flagsp)
{
	caddr_t ptr = *ptrp;
	struct mi_comm *mi = ptr_to_mi(ptr);
	struct mi_comm *mw = ptr_to_mi(ptrw);

	if (unlikely(!__mi_acquire(mi, mw->mi_q, MI_WAIT_SLEEP_BIT))) {
		DECLARE_WAITQUEUE(wait, current);
		add_wait_queue(&mw->mi_waitq, &wait);
		for (;;) {
			set_current_state(TASK_UNINTERRUPTIBLE);
			if (likely(__mi_acquire(mi, mw->mi_q, MI_WAIT_SLEEP_BIT)))
				break;
			if (lockp) {
				if (flagsp)
					write_unlock_irqrestore(lockp, *flagsp);
				else
					write_unlock(lockp);
			}
			schedule();
			if (lockp) {
				if (flagsp)
					write_lock_irqsave(lockp, *flagsp);
				else
					write_lock(lockp);
			}
			if (unlikely((ptr = *ptrp) == NULL))
				break;
			mi = ptr_to_mi(ptr);
		}
		set_current_state(TASK_RUNNING);
		remove_wait_queue(&mw->mi_waitq, &wait);
	}
	return (ptr);
}

EXPORT_SYMBOL_GPL(mi_acquire_sleep_nosignal);

static inline fastcall void
__mi_release(struct mi_comm *mi)
{
	unsigned long flags;
	struct mi_comm *mo;

	spin_lock_irqsave(&mi->mi_lock, flags);
	__assert(mi->mi_users == 1);
	mi->mi_users = 0;
	mo = XCHG(&mi->mi_waiter, NULL);
	spin_unlock_irqrestore(&mi->mi_lock, flags);
	if (unlikely(mo != NULL))
		mi_wakeold(mo);
}

__MPS_EXTERN void
mi_release(caddr_t ptr)
{
	__mi_release(ptr_to_mi(ptr));
}

EXPORT_SYMBOL_GPL(mi_release);

/**
 * mi_unlock: - unlock a private structure
 * @ptr: private structure to unlock
 *
 * Unlock a private structure and wake waiters and schedule blocked queues.
 */
__MPS_EXTERN void
mi_unlock(caddr_t ptr)
{
	return mi_release(ptr);
}

EXPORT_SYMBOL_GPL(mi_unlock);

__MPS_EXTERN caddr_t
mi_trylock(queue_t *q)
{
	return mi_acquire((caddr_t) q->q_ptr, q);
}

EXPORT_SYMBOL_GPL(mi_trylock);

__MPS_EXTERN caddr_t
mi_sleeplock(queue_t *q)
{
	caddr_t ptr = (caddr_t) q->q_ptr;

	ptr = mi_acquire_sleep(ptr, &ptr, NULL, NULL);
	return (ptr);
}

EXPORT_SYMBOL_GPL(mi_sleeplock);

/*
 *  =========================================================================
 *
 *  Buffer call helper function.
 *
 *  =========================================================================
 */
static fastcall void
mi_qenable(long data)
{
	struct mi_comm *mi = (struct mi_comm *) data;

	if (likely(xchg(&mi->mi_bcid, 0)))
		mi_wakeold(mi);
}

noinline void
mi_rebufcall(struct mi_comm *mi, bcid_t bcid)
{
	if ((bcid = xchg(&mi->mi_bcid, bcid))) {
		unbufcall(bcid);
		mi_put(mi);
	}
}

noinline void
mi_unbufcall(struct mi_comm *mi)
{
	mi_rebufcall(mi, 0);
}

__MPS_EXTERN void
mi_bufcall(queue_t *q, int size, int priority)
{
	struct mi_comm *mi;

	if (q && (mi = ptr_to_mi(q->q_ptr))) {
		bcid_t bid;

		assert(atomic_read(&mi->mi_refs) > 1);
		assert(atomic_read(&mi->mi_refs) < 10);
		mi_grab(mi);
		set_bit((q->q_flag & QREADR) ? MI_WAIT_RQ_BIT : MI_WAIT_WQ_BIT, &mi->mi_flags);
		if ((bid = bufcall(size, priority, &mi_qenable, (long) mi))) {
			mi_rebufcall(mi, bid);
			return;
		}
		mi_wakeold(mi);
	}
}

EXPORT_SYMBOL(mi_bufcall);

__MPS_EXTERN_INLINE void mi_esbbcall(queue_t *q, int priority);

EXPORT_SYMBOL(mi_esbbcall);

/*
 *  =========================================================================
 *
 *  Message block allocation helper functions.
 *
 *  =========================================================================
 */

__MPS_EXTERN mblk_t *
mi_reuse_proto(mblk_t *mp, size_t size, int keep_on_error)
{
	if (unlikely(mp == NULL || (size > FASTBUF && mp->b_datap->db_lim - mp->b_rptr < size)
		     || mp->b_datap->db_ref > 1 || mp->b_datap->db_frtnp != NULL)) {
		/* can't reuse this message block (or no message block to begin with) */
		if (mp && !keep_on_error)
			freemsg(xchg(&mp, NULL));
		mp = NULL;
	} else
		/* simply resize it - leave everything else intact */
		mp->b_wptr = mp->b_rptr + size;
	return (mp);
}

EXPORT_SYMBOL(mi_reuse_proto);

__MPS_EXTERN mblk_t *
mi_reallocb(mblk_t *mp, size_t size)
{
	if (unlikely(mp == NULL ||
		     (size > FASTBUF && mp->b_datap->db_lim - mp->b_datap->db_base < size) ||
		     mp->b_datap->db_ref > 1 || mp->b_datap->db_frtnp != NULL)) {
		/* can't reuse this message block (or no message block to begin with) */
		if (mp)
			freemsg(xchg(&mp, NULL));
		mp = allocb(size, BPRI_MED);
	} else {
		/* prepare existing message block for reuse as though just allocated */
		mp->b_next = mp->b_prev = NULL;
		if (mp->b_cont)
			freemsg(xchg(&mp->b_cont, NULL));
		mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
		mp->b_band = 0;
		mp->b_flag = 0;
		mp->b_datap->db_type = M_DATA;
	}
	return (mp);
}

EXPORT_SYMBOL(mi_reallocb);

__MPS_EXTERN_INLINE mblk_t *mi_allocb(queue_t *q, size_t size, int priority);

EXPORT_SYMBOL(mi_allocb);	/* mps/stream.h */

__MPS_EXTERN_INLINE mblk_t *mi_esballoc(queue_t *q, unsigned char *base,
		size_t size, uint priority, frtn_t *freeinfo);

EXPORT_SYMBOL(mi_esballoc);	/* mps/stream.h */

__MPS_EXTERN_INLINE mblk_t *mi_copyb(queue_t *q, mblk_t *bp);

EXPORT_SYMBOL(mi_copyb);	/* mps/stream.h */

__MPS_EXTERN_INLINE mblk_t *mi_dupb(queue_t *q, mblk_t *bp);

EXPORT_SYMBOL(mi_dupb);		/* mps/stream.h */

__MPS_EXTERN_INLINE mblk_t *mi_copymsg(queue_t *q, mblk_t *mp);

EXPORT_SYMBOL(mi_copymsg);	/* mps/stream.h */

__MPS_EXTERN_INLINE mblk_t *mi_dupmsg(queue_t *q, mblk_t *mp);

EXPORT_SYMBOL(mi_dupmsg);	/* mps/stream.h */

__MPS_EXTERN_INLINE mblk_t *mi_msgpullup(queue_t *q, mblk_t *mp, ssize_t length);

EXPORT_SYMBOL(mi_msgpullup);	/* mps/stream.h */

/*
 *  =========================================================================
 *
 *  Copyin and copyout M_IOCTL helper functions.
 *
 *  =========================================================================
 */

#define MI_COPY_IN			1
#define MI_COPY_OUT			2
#define MI_COPY_CASE(_dir, _cnt)	(((_cnt)<<2|_dir))

#if 0
#define MI_COPY_DIRECTION(_mp)		(*(int *)&(_mp)->b_cont->b_next)
#define MI_COPY_COUNT(_mp)		(*(int *)&(_mp)->b_cont->b_prev)
#else
#define MI_COPY_DIRECTION(_mp)		(((struct mi_iocblk *)(_mp)->b_cont->b_rptr)->mi_dir)
#define MI_COPY_COUNT(_mp)		(((struct mi_iocblk *)(_mp)->b_cont->b_rptr)->mi_cnt)
#define MI_COPY_UADDR(_mp)		(((struct mi_iocblk *)(_mp)->b_cont->b_rptr)->mi_uaddr)
#endif

#define MI_COPY_STATE(_mp)		MI_COPY_CASE(MI_COPY_DIRECTION(_mp), MI_COPY_COUNT(_mp))

/**
 * mi_copy_done: - complete an input-output control operation
 * @q: write queue upon which message was received
 * @mp: the M_IOCTL/M_IOCDATA message
 * @err: non-zero error to return, or zero for success
 *
 * mi_copy_done() is used to complete an input-output control operation where there is no data to be
 * copied out (neither implicit nor explicit).
 *
 * Almost all versions of LiS have a pretty nasty bug with regard to M_IOCDATA.  According to SVR 4
 * STREAMS Programmer's Guide and all major UNIX implementations of STREAMS, when M_IOCDATA returns
 * an error (cp_rval != NULL), the module or driver is supposed to clean up and abort the M_IOCTL.
 * The Stream head has already returned an error to the user and does not expect an M_IOCACK or
 * M_IOCNAK.  LiS makes the error of sleeping again and expecting a negative acknowledgement, so,
 * for LiS (and not Linux Fast-STREAMS) we return an M_IOCNAK with what will be the ultimate error
 * code.  Modules and drivers ported to LiS that don't know about this will hang until timeout or
 * signal (SIGKILL, SIGTERM), possibly indefinitely when an M_IOCDATA error is sent.  Even if LiS is
 * fixed, this code will still work, it will just send an extraneous M_IOCNAK to the Stream head
 * that will be discarded because no M_IOCTL will be in progress anymore.
 */
__MPS_EXTERN void
mi_copy_done(queue_t *q, mblk_t *mp, int err)
{
	union ioctypes *ioc;
	struct mi_iocblk *mi;
	int rval;

	dassert(mp);
	dassert(q);
	dassert(mp->b_wptr >= mp->b_rptr + sizeof(*ioc));

	ioc = (typeof(ioc)) mp->b_rptr;
	rval = ioc->iocblk.ioc_rval;
	switch (mp->b_datap->db_type) {
	case M_IOCDATA:
		if (ioc->copyresp.cp_private) {
			mi = (typeof(mi)) ioc->copyresp.cp_private->b_rptr;
			rval = mi->mi_rval;
			freemsg(XCHG(&ioc->copyresp.cp_private, NULL));
		}
		if (ioc->copyresp.cp_rval == (caddr_t) 0)
			break;
		/* SVR 4 SPG says don't return ACK/NAK on M_IOCDATA error */
		freemsg(mp);
		return;
	case M_IOCTL:
		break;
	default:
		swerr();
		freemsg(mp);
		return;
	}
	mp->b_datap->db_type = err ? M_IOCNAK : M_IOCACK;
	ioc->iocblk.ioc_error = (err < 0) ? -err : err;
	ioc->iocblk.ioc_rval = err ? -1 : rval;
	ioc->iocblk.ioc_count = 0;
	qreply(q, mp);
}

EXPORT_SYMBOL(mi_copy_done);	/* mps/ddi.h */

/**
 * mi_copyin: - perform explicit or implicit copyin() operation
 * @q: write queue on which message was received
 * @mp: the M_IOCTL or M_IOCDATA message
 * @uaddr: NULL for first copyin operation, or user address for subsequent operation
 * @len: length of data to copy in
 */
__MPS_EXTERN void
mi_copyin(queue_t *q, mblk_t *mp, caddr_t uaddr, size_t len)
{
	union ioctypes *ioc = (typeof(ioc)) mp->b_rptr;
	struct mi_iocblk *mi;
	mblk_t *db, *bp, *dp;

	switch (mp->b_datap->db_type) {
	case M_IOCTL:
		/* Note that uaddr is ignored on the first operation, but should be specified as
		   NULL according to the documentation anyway. */
		if (ioc->iocblk.ioc_count == TRANSPARENT) {
			/* for transparent ioctl perform a copy in */
			if (!(bp = XCHG(&mp->b_cont, NULL)))
				return __ctrace(mi_copy_done(q, mp, EPROTO));
			mi = (typeof(mi)) bp->b_rptr;
			mp->b_datap->db_type = M_COPYIN;
			ioc->copyreq.cq_private = bp;
			ioc->copyreq.cq_size = len;
			ioc->copyreq.cq_flag = 0;
			ioc->copyreq.cq_addr = mi->mi_caddr;
			mi->mi_uaddr = mi->mi_caddr;
			mi->mi_cnt = 1;
			mi->mi_dir = MI_COPY_IN;
			if (mp->b_cont)
				freemsg(XCHG(&mp->b_cont, NULL));
			qreply(q, mp);
			return;
		}
		if (!(db = mp->b_cont))
			return __ctrace(mi_copy_done(q, mp, EPROTO));
		/* for a non-transparent ioctl the first copyin is already performed for us, fake
		   out an M_IOCDATA response.  However, only place the requested data from the
		   implicit copyin into the data block.  Save the rest for a subsequent
		   mi_copyin_n() operation. */

		/* If the amount copied in implicitly is less than that requested in the
		   input-output control operation, then it is the user's fault and EINVAL is
		   returned. */
		if (msgdsize(db) < len)
			return mi_copy_done(q, mp, EINVAL);
#if 0
		if (!pullupmsg(db, -1) || !(dp = copyb(db)))
			return mi_copy_done(q, mp, ENOSR);
#else
		if (!(dp = copyb(db)))
			return mi_copy_done(q, mp, ENOSR);
#endif
		if (!(bp = allocb(sizeof(*mi), BPRI_MED))) {
			freeb(dp);
			return mi_copy_done(q, mp, ENOSR);
		}
		dp->b_wptr = dp->b_rptr + len;
		bp->b_cont = db;    /* save original */
		mp->b_cont = dp;
		mi = (typeof(mi)) bp->b_rptr;
		mp->b_datap->db_type = M_IOCDATA;
		ioc->copyresp.cp_private = bp;
		ioc->copyresp.cp_rval = (caddr_t) 0;
		mi->mi_caddr = NULL;
		mi->mi_uaddr = NULL;
		mi->mi_cnt = 1;
		mi->mi_dir = MI_COPY_IN;
		mi->mi_rval = 0;
		putq(q, mp);
		/* There are two clues to the fact that an implicit copyin was performed to
		   subsequent operations: mi->mi_uaddr == NULL and cp_private->b_cont != NULL while 
		   mi->mi_dir == MI_COPY_IN.  This fact is used by mi_copyin_n() to determine
		   whether to issue another M_COPYIN or whether to get the data from the buffer
		   saved at cp_private->b_cont.  When the operation switches over to copyout, this
		   copied in buffer will be discarded. */
		return;
	case M_IOCDATA:
		/* for a subsequent M_IOCDATA, we should already have our private message block
		   stacked up against the private pointer */
		if (!(bp = XCHG(&ioc->copyresp.cp_private, NULL)))
			return __ctrace(mi_copy_done(q, mp, EPROTO));
		mi = (typeof(mi)) bp->b_rptr;
		mp->b_datap->db_type = M_COPYIN;
		ioc->copyreq.cq_private = bp;
		ioc->copyreq.cq_size = len;
		ioc->copyreq.cq_flag = 0;
		ioc->copyreq.cq_addr = uaddr;
		/* leave mi_caddr untouched for implicit copyout operation */
		mi->mi_uaddr = uaddr;
		mi->mi_cnt += 1;
		mi->mi_dir = MI_COPY_IN;
		if (mp->b_cont)
			freemsg(XCHG(&mp->b_cont, NULL));
		if (bp->b_cont)
			freemsg(XCHG(&bp->b_cont, NULL));
		qreply(q, mp);
		return;
	default:
		return __ctrace(mi_copy_done(q, mp, EPROTO));
	}
}

EXPORT_SYMBOL(mi_copyin);	/* mps/ddi.h */

/**
 * mi_copyin_n: - perform subsequent copyin operation
 * @q: write queue on which message was received
 * @mp: the M_IOCDATA message
 * @offset: offset into original memory extent
 * @len: length of data to copy in
 *
 * Note that this function can only be used on M_IOCDATA blocks, and that M_IOCDATA block must
 * contain our private data and the direction of the previous operation must have been an
 * MI_COPY_IN.  That is, mi_copyin() must be called before this function.
 *
 * When the mi_copyin() was the first mi_copyin() operation on an I_STR input-output control, the
 * implicit copyin buffer was attached as cp_private->b_cont.  The subsequent mi_copyin_n()
 * operation will take from that buffer.  An explicit mi_copyin() can subsequently be performed,
 * even for an I_STR input-output control, in which case, a subsequent mi_copyin_n() operation will
 * always issue an M_COPYIN resulting in an M_IOCDATA.
 */
__MPS_EXTERN void
mi_copyin_n(queue_t *q, mblk_t *mp, size_t offset, size_t len)
{
	union ioctypes *ioc = (typeof(ioc)) mp->b_rptr;
	struct mi_iocblk *mi;
	mblk_t *bp, *dp;

	if (mp->b_datap->db_type != M_IOCDATA || !(bp = ioc->copyresp.cp_private))
		return __ctrace(mi_copy_done(q, mp, EPROTO));
	mi = (typeof(mi)) bp->b_rptr;
	if (mi->mi_dir != MI_COPY_IN)
		return __ctrace(mi_copy_done(q, mp, EPROTO));
	if (mi->mi_uaddr != NULL) {
		/* last operation was an explicit copyin */
		mp->b_datap->db_type = M_COPYIN;
		ioc->copyreq.cq_private = bp;
		ioc->copyreq.cq_size = len;
		ioc->copyreq.cq_flag = 0;
		ioc->copyreq.cq_addr = mi->mi_uaddr + offset;
		mi->mi_cnt++;
		if (mp->b_cont)
			freemsg(XCHG(&mp->b_cont, NULL));
		qreply(q, mp);
	} else {
		/* last operation was an implicit copyin */
		if ((dp = bp->b_cont) == NULL)
			return __ctrace(mi_copy_done(q, mp, EPROTO));
		/* If the implicit copyin buffer was to small to satisfy the request, then it was
		   the user's fault in supplying a buffer that was too small, so return EINVAL. */
		if (msgdsize(dp) < offset + len)
			return mi_copy_done(q, mp, EINVAL);
		if (!(dp = copyb(dp)))
			return mi_copy_done(q, mp, ENOSR);
		dp->b_rptr += offset;
		dp->b_wptr = dp->b_rptr + len;
		ioc->copyresp.cp_rval = (caddr_t) 0;
		if (mp->b_cont)
			freemsg(XCHG(&mp->b_cont, NULL));
		mp->b_cont = dp;
		mi->mi_cnt++;
		putq(q, mp);
	}
}

EXPORT_SYMBOL(mi_copyin_n);	/* mps/ddi.h */

/**
 * mi_copyout_alloc: - allocate a message block for a copyout operation
 * @q: write queue on which messagew as received
 * @mp: the M_IOCTL or M_IOCDATA message
 * @uaddr: explicit user address (or NULL for implicit address) to copy out to
 * @len: length of data to copy out
 * @free_on_error: when non-zero close input-output operation automatically on error
 */
__MPS_EXTERN mblk_t *
mi_copyout_alloc(queue_t *q, mblk_t *mp, caddr_t uaddr, size_t len, int free_on_error)
{
	union ioctypes *ioc = (typeof(ioc)) mp->b_rptr;
	struct mi_iocblk *mi;
	mblk_t *db, *bp;

	switch (mp->b_datap->db_type) {
	case M_IOCTL:
		if (ioc->iocblk.ioc_count == TRANSPARENT) {
			bp = XCHG(&mp->b_cont, NULL);
		} else {
			bp = allocb(sizeof(*mi), BPRI_MED);
			if (likely(bp != NULL))
				bzero(bp->b_rptr, sizeof(*mi));
		}
		if (!bp) {
			if (free_on_error)
				mi_copy_done(q, mp, ENOSR);
			return (NULL);
		}
		bp->b_wptr = bp->b_rptr + sizeof(*mi);
		mi = (typeof(mi)) bp->b_rptr;
		mi->mi_cnt = 0;
		mi->mi_dir = MI_COPY_OUT;
		mp->b_datap->db_type = M_IOCDATA;	/* for next pass */
		ioc->copyresp.cp_private = bp;
		ioc->copyresp.cp_rval = 0;
		break;
	case M_IOCDATA:
		if (!(bp = ioc->copyresp.cp_private)) {
			if (free_on_error)
				__ctrace(mi_copy_done(q, mp, EPROTO));
			return (NULL);
		}
		mi = (typeof(mi)) bp->b_rptr;
		if (mi->mi_dir == MI_COPY_IN) {
			if (bp->b_cont)
				freemsg(XCHG(&bp->b_cont, NULL));
			mi->mi_dir = MI_COPY_OUT;
		}
		break;
	default:
		if (free_on_error)
			__ctrace(mi_copy_done(q, mp, EPROTO));
		return (NULL);
	}
	if (mp->b_cont)
		freemsg(XCHG(&mp->b_cont, NULL));
	if (!(db = allocb(len, BPRI_MED))) {
		if (free_on_error)
			mi_copy_done(q, mp, ENOSR);
		return (db);
	}
	db->b_wptr = db->b_rptr + len;
	linkb(bp, db);
	db->b_next = (mblk_t *) (uaddr ? : mi->mi_caddr);
	return (db);
}

EXPORT_SYMBOL(mi_copyout_alloc);	/* mps/ddi.h */

/**
 * mi_copyout: - perform a pending copyout operation and possible close input-output control
 * @q: write queue on which message was received
 * @mp: the M_IOCTL or M_IOCDATA message
 */
__MPS_EXTERN void
mi_copyout(queue_t *q, mblk_t *mp)
{
	union ioctypes *ioc = (typeof(ioc)) mp->b_rptr;
	struct mi_iocblk *mi;
	mblk_t *bp, *db;
	caddr_t uaddr;

#if 1
	if (mp->b_datap->db_type != M_IOCDATA)
		return __ctrace(mi_copy_done(q, mp, EPROTO));
#if 0
	if (!mp->b_cont)
		return __ctrace(mi_copy_done(q, mp, EPROTO));
#endif
	if (!(bp = ioc->copyresp.cp_private))
		return __ctrace(mi_copy_done(q, mp, EPROTO));
#else
	if (mp->b_datap->db_type != M_IOCDATA || !mp->b_cont || !(bp = ioc->copyresp.cp_private))
		return __ctrace(mi_copy_done(q, mp, EPROTO));
#endif
#if 0
	/* This is for LiS that puts an error code in the cp_rval and expects an M_IOCNAK. */
	if (ioc->copyresp.cp_rval)
		return mi_copy_done(q, mp, (int) (long) ioc->copyresp.cp_rval);
#endif
	mi = (typeof(mi)) bp->b_rptr;
	if (!(db = bp->b_cont) || mi->mi_dir == MI_COPY_IN)
		return mi_copy_done(q, mp, 0);
	bp->b_cont = XCHG(&db->b_cont, NULL);
	uaddr = (caddr_t) XCHG(&db->b_next, NULL);
	if (uaddr == NULL)
		uaddr = mi->mi_caddr;
	if (uaddr != NULL) {
		/* explicit copyout operation */
		mp->b_datap->db_type = M_COPYOUT;
		ioc->copyreq.cq_private = bp;
		ioc->copyreq.cq_size = msgdsize(db);
		ioc->copyreq.cq_flag = 0;
		ioc->copyreq.cq_addr = uaddr;
		mi->mi_cnt++;
		mi->mi_dir = MI_COPY_OUT;
	} else {
		/* implicit copyout operation */
		if (ioc->copyresp.cp_private)	/* won't be back */
			freemsg(XCHG(&ioc->copyresp.cp_private, NULL));
		mp->b_datap->db_type = M_IOCACK;
		ioc->iocblk.ioc_error = 0;
		ioc->iocblk.ioc_count = msgdsize(db);
	}
	if (mp->b_cont)
		freemsg(XCHG(&mp->b_cont, NULL));
	mp->b_cont = db;
	qreply(q, mp);
	return;
}

EXPORT_SYMBOL(mi_copyout);	/* mps/ddi.h */

/**
 * mi_copy_state: - determine the state of an input-output control operation
 * @q: write queue on which message was received
 * @mp: the M_IOCDATA message
 * @mpp: message pointer to set do copied in data
 *
 * mi_copy_state() is only used on M_IOCDATA messages.  It checks the return code from the previous
 * copyin or copyout operation and returns an M_IOCNAK or aborts if an error occurred.  If data was
 * copied in, mpp is set to point to the data block containing the data.  The user must not free
 * this data block: it remains attached to the M_IOCDATA message.  If data was copied out, the
 * pointer pointed to by mpp is untouched.  The function returns -1 if an error occurred (in which
 * case the passed in message block is used to pass an M_IOCNAK upstream).  If no error occured, it
 * returns the state of the input-output control operation.
 */
__MPS_EXTERN int
mi_copy_state(queue_t *q, mblk_t *mp, mblk_t **mpp)
{
	union ioctypes *ioc = (typeof(ioc)) mp->b_rptr;
	struct mi_iocblk *mi;
	mblk_t *bp, *db = NULL;
	int err = EPROTO;

	if (mp->b_datap->db_type != M_IOCDATA || !(db = mp->b_cont)
	    || !(bp = ioc->copyresp.cp_private)) {
		__swerr();
		goto error;
	}
#if 0
	/* This is for LiS that puts an error code in the cp_rval and expects an M_IOCNAK. */
	if ((err = (int) (long) ioc->copyresp.cp_rval))
		goto error;
#endif
	switch (mp->b_datap->db_type) {
	case M_IOCDATA:
#if 0
		err = ENOMEM;
		if (!pullupmsg(db, -1))
			goto error;
#endif
		mi = (typeof(mi)) bp->b_rptr;
		if (mi->mi_dir == MI_COPY_IN)
			*mpp = db;
		return MI_COPY_CASE(mi->mi_dir, mi->mi_cnt);
	case M_IOCTL:
		__swerr();
		goto error;
	}
      error:
	mi_copy_done(q, mp, err);
	return (-1);
}

EXPORT_SYMBOL(mi_copy_state);	/* mps/ddi.h */

/**
 * mi_copy_set_rval: - set the input-output control operation return value
 * @mp: the M_IOCTL or M_IOCDATA message
 * @rval: the return value to set
 *
 * mi_copy_set_rval() is used to set the return value to something other than zero before the last
 * function in the input-output control operation is called.  The last functions called would be
 * either mi_copyout() or mi_copy_done().
 */
__MPS_EXTERN void
mi_copy_set_rval(mblk_t *mp, int rval)
{
	union ioctypes *ioc = (typeof(ioc)) mp->b_rptr;
	struct mi_iocblk *mi;
	mblk_t *bp;

	switch (mp->b_datap->db_type) {
	case M_IOCDATA:
		if ((bp = ioc->copyresp.cp_private)) {
			mi = (typeof(mi)) bp->b_rptr;
			mi->mi_rval = rval;
			break;
		}
		/* fall through */
	case M_IOCTL:
		ioc->iocblk.ioc_rval = rval;
		break;
	}
	return;
}

EXPORT_SYMBOL(mi_copy_set_rval);

/*
 *  =========================================================================
 *
 *  Timeout helper functions
 *
 *  =========================================================================
 */

/*
 * States are as follows:
 *
 * TB_IDLE:		The timer is idle and has either never been set or has been cancelled at
 *			some point and found not to have been expired yet.  The timer is not on any
 *			queue.
 *
 * TB_ACTIVE:		The timer has been armed and the clock is running.  There are two transient
 *			situations here: tb->tb_tid == 0, where the timer has fired but the timeout
 *			routine has not yet placed the timer on a mesage queue (but will shortly
 *			unless the state is changed); and, tb->tb_tid != 0, where the timer has not
 *			yet fired.  In this state the timer is not on any queue (yet), and changing
 *			the state can prevent the timer from being placed on any queue.
 *
 * TB_EXPIRED:		The timer has fired, and the timer message has been placed on the message
 *			queue.  There is one transient situation where the timer has been removed
 *			from the queue but has not yet been passed to mi_timer_valid().  In this
 *			state, the timer is normally on the target queue (but may exist in the
 *			transient state).  If the message is removed from the target queue should it
 *			be found there, the message will not be processed by the queue service
 *			procedure.
 *
 * TB_TRANSIENT:	The timer has been taken off of the target queue and passed through
 *			mi_timer_valid() and the timeout was found to be valid.  The timer can only
 *			exist on the queue in this state under the condition that the timer was
 *			returned to the queue after mi_timer_valid() was called.  If the timer is
 *			removed from the target queue, it can be guaranteed that the timeout
 *			function will never run for this timer.  Otherwise, it is not possible to
 *			determine whether the timeout function has run or not.  I wish the designers
 *			of this interface had the foresight to call a function, say, mi_timer_done()
 *			once the timeout function was complete.
 */
#define TB_ZOMBIE	-2	/* tb is a zombie, waiting to be freed, but may be on queue */
#define TB_CANCELLED	-1	/* tb has been cancelled, but may be on queue */
#define TB_IDLE		 0	/* tb is idle, not on queue */
#define TB_ACTIVE	 1	/* tb has timer actively running, but is not on queue */
#define TB_EXPIRED	 2	/* tb has expired and may be on queue */
#define TB_RESCHEDULED	 3	/* tb is being rescheduled, but may be on queue */
#define TB_TRANSIENT	 4	/* tb is in a transient state, i.e. it has fired */

typedef struct mi_timeb {
	spinlock_t tb_lock;
	long tb_state;			/* the state of the timer */
	toid_t tb_tid;			/* the timeout id when a timeout is running */
	clock_t tb_time;		/* the time of expiry */
	queue_t *tb_q;			/* where to queue the timer message on timeout */
} tblk_t;

#undef mi_timer_alloc
#undef mi_timer

/*
 * mi_timer_stop() or mi_timer_cancel() stops a running timer.  If the timer is on the target queue,
 * it is removed.
 *
 * mi_timer() stops a running timer and starts a new timer.  The timer is not removed from any queue
 * upon which it exists.
 *
 * mi_timer_expiry() checks for timer cancellation: if cancelled, does nothing.  If not cancelled,
 * the timer is removed from to target queue if it is on the queue and queued against the target
 * queue again.
 *
 * mi_timer_move() removes the timer from the old target queue if it exists there and places it on
 * the new target queue if it existed on the old target queue.
 */

/**
 * mi_timer_remove: - attempt to remove timer from queue
 * @q: the queue to which mp would be queued
 * @mp: the message to remove.
 *
 * Returns a pointer to the removed message when the message was removed from the queue or NULL if
 * the message was not on the queue (and therefore not removed).  The return value, therefore,
 * represents the removed message.
 */
STATIC mblk_t *
mi_timer_remove(queue_t *q, mblk_t *mp)
{
	unsigned long pl;
	mblk_t *b = mp;

	if (q != NULL) {
		pl = freezestr(q);
		for (b = q->q_first; b && b != mp; b = b->b_next) ;
		if (b)
			rmvq(q, b);
		unfreezestr(q, pl);
	}
	return (b);
}

STATIC void
mi_timer_putq(queue_t *q, mblk_t *mp)
{
	unsigned long pl;
	mblk_t *b = mp;

	if (q != NULL) {
		pl = freezestr(q);
		for (b = q->q_first; b && b != mp; b = b->b_next) ;
		if (b)
			rmvq(q, b);
		insq(q, NULL, mp);
		unfreezestr(q, pl);
	}
	return;
}

/**
 * mi_timer_expiry: - process timer expiry
 * @data: pointer to timer message
 *
 * When the timeout expires (and was not cancelled by swaping tb_tid to zero), if the timer is in
 * the TB_ACTIVE state, move it to the TB_IDLE state and place it on the queue.  If the timer is
 * already in the TB_IDLE state.
 * 
 */
static fastcall void
mi_timer_expiry(caddr_t data)
{
	mblk_t *mp = (typeof(mp)) data;
	tblk_t *tb = (typeof(tb)) mp->b_datap->db_base;
	unsigned long flags;

	spin_lock_irqsave(&tb->tb_lock, flags);
	if (XCHG(&tb->tb_tid, 0)) {
		if (tb->tb_state == TB_ACTIVE) {
			tb->tb_state = TB_EXPIRED;
			mp->b_datap->db_type = M_PCSIG;
		}
		mi_timer_putq(tb->tb_q, mp);
	}
	spin_unlock_irqrestore(&tb->tb_lock, flags);
}

/*
 *  MacOT variety
 */

/**
 * mi_timer_alloc: - allocate a timer message (MacOT variety)
 * @q: queue onto which to put message upon expiry
 * @size: size of message block to allocate
 *
 * Allocates and returns a timer message block for use in other helper functions.  Note that we
 * allocate a duplicate message block as protection against being freed.
 */
__MPS_EXTERN mblk_t *
mi_timer_alloc_MAC(queue_t *q, size_t size)
{
	mblk_t *mp;
	tblk_t *tb;

	if ((mp = allocb(sizeof(*tb) + size, BPRI_HI))) {
		mp->b_datap->db_type = M_PCSIG;
		tb = (typeof(tb)) mp->b_rptr;
		mp->b_rptr = (typeof(mp->b_rptr)) (tb + 1);
		mp->b_wptr = mp->b_rptr + size;
#ifdef SPIN_LOCK_UNLOCKED
		tb->tb_lock = SPIN_LOCK_UNLOCKED;
#elif defined spin_lock_init
		spin_lock_init(&tb->tb_lock);
#else
#error cannot initialize spin locks
#endif
		tb->tb_state = TB_IDLE;
		tb->tb_tid = (toid_t) (0);
		tb->tb_time = (clock_t) (0);
		tb->tb_q = q;
	}
	return (mp);
}

EXPORT_SYMBOL(mi_timer_alloc_MAC);

/**
 * mi_timer: - start or restart a timer (MacOT variety)
 * @mp: timer message
 * @msec: milliseconds to expiry
 *
 * Start or restart a timer.
 */
__MPS_EXTERN void
mi_timer_MAC(mblk_t *mp, clock_t msec)
{
	mi_timer_ticks(mp, drv_msectohz(msec));
}

EXPORT_SYMBOL(mi_timer_MAC);

__MPS_EXTERN int
mi_timer_requeue(mblk_t *mp)
{
	tblk_t *tb = (typeof(tb)) mp->b_datap->db_base;
	unsigned long flags;
	int rval;

	spin_lock_irqsave(&tb->tb_lock, flags);
	switch (__builtin_expect(tb->tb_state, TB_IDLE)) {
	case TB_RESCHEDULED:
		/* Timer was rescheduled while in user's hands. */
		if (tb->tb_time > jiffies) {
			tb->tb_state = TB_ACTIVE;
			tb->tb_tid = timeout(mi_timer_expiry, (caddr_t) mp, tb->tb_time - jiffies);
			rval = (0);
			if (tb->tb_tid)
				break;
			tb->tb_state = TB_RESCHEDULED;
			mp->b_datap->db_type = M_PCSIG;
			mi_timer_putq(tb->tb_q, mp);
			break;
		}
		/* Timer has already expired. */
		/* fall through */
	case TB_EXPIRED:
		/* Timer expired, mi_timer_valid not called yet. */
	case TB_TRANSIENT:
		/* Timer is transient. */
	case TB_IDLE:
		tb->tb_state = TB_EXPIRED;
		mp->b_datap->db_type = M_SIG;
		rval = (1);	/* caller must putq or putbq back */
		break;
	case TB_CANCELLED:
		/* Timer was cancelled while in user's hands. */
		tb->tb_state = TB_IDLE;
		rval = (0);
		break;
	default:
	case TB_ACTIVE:
		/* Timers in the active state are never in the user's hands. */
		swerr();
		rval = (0);
		break;
	}
	spin_unlock_irqrestore(&tb->tb_lock, flags);
	return (rval);
}

EXPORT_SYMBOL(mi_timer_requeue);

/**
 * mi_timer_cancel: - cancel a timer message (MacOT variety)
 * @mp: the timer message
 *
 * This function attempts to cancel execution of the timer's timeout function.  When it can be
 * guaranteed that the timer is not already on a queue and not about to be queued, true (1) is
 * returned.  When the queue state is unknown or the message is unusable, false (0) is returned.
 * This return value is for use by mi_timer_q_switch() only.  mi_timer_cancel() can be called in any
 * context at any point in time.
 */
__MPS_EXTERN int
mi_timer_cancel(mblk_t *mp)
{
	tblk_t *tb = (typeof(tb)) mp->b_datap->db_base;
	unsigned long flags;
	toid_t tid;
	int rval;

	if ((tid = xchg(&tb->tb_tid, 0)))
		untimeout(tid);

	spin_lock_irqsave(&tb->tb_lock, flags);
	switch (__builtin_expect(tb->tb_state, TB_ACTIVE)) {
	default:
		swerr();
	case TB_EXPIRED:
		/* Expired, could be on queue. */
	case TB_TRANSIENT:
		/* Transient, could be on queue. */
	case TB_RESCHEDULED:
		/* Recheduled, could be on queue. */
	case TB_CANCELLED:
		/* Cancelled, could be on queue. */
		tb->tb_state = TB_CANCELLED;
		rval = (0);
		break;
	case TB_ACTIVE:
		/* Active, not on queue. */
		if (tid == (toid_t) 0) {
			tb->tb_state = TB_CANCELLED;
			rval = (0);
			break;
		}
	case TB_IDLE:
		/* Idle, not on queue */
		tb->tb_state = TB_IDLE;
		rval = (1);
		break;
	case TB_ZOMBIE:
		/* Zombie, could be on queue. */
		rval = (0);
		break;
	}
	spin_unlock_irqrestore(&tb->tb_lock, flags);
	return (rval);
}

EXPORT_SYMBOL(mi_timer_cancel);

/**
 * mi_timer_q_switch: - switch target queues for a timer (MacOT variety)
 * @mp: existing timer message
 * @q: queue for new timer
 * @new_mp: new timer message to use if necessary
 *
 * Returns the timer message that was used.
 */
__MPS_EXTERN mblk_t *
mi_timer_q_switch(mblk_t *mp, queue_t *q, mblk_t *new_mp)
{
	tblk_t *tb = (typeof(tb)) mp->b_datap->db_base;

	if (mi_timer_cancel(mp)) {
		/* not on queue, can be rescheduled on new queue */
		mi_timer_move(q, mp);
		mi_timer_ticks(mp, tb->tb_time - jiffies);
		return (mp);
	} else {
		/* possibly on queue, use new timer block */
		mi_timer_move(q, new_mp);
		mi_timer_ticks(new_mp, tb->tb_time - jiffies);
		return (new_mp);
	}
}

EXPORT_SYMBOL(mi_timer_q_switch);

/*
 *  OpenSolaris variety
 */

/**
 * mi_timer_alloc_SUN: - allocate a timer (OpenSolaris variety)
 * @size: size of the timer message to allocate
 *
 * Solaris style mi_timer_alloc() does not assign a queue at the time of allocation.
 */
__MPS_EXTERN mblk_t *
mi_timer_alloc_SUN(size_t size)
{
	return mi_timer_alloc_MAC(NULL, size);
}

EXPORT_SYMBOL(mi_timer_alloc_SUN);

/**
 *  mi_timer_ticks: - start a timer -- OpenSS7 variety
 *  @mp: the timer message
 *  @ticks: the time in HZ
 */
__MPS_EXTERN void
mi_timer_ticks(mblk_t *mp, clock_t ticks)
{
	tblk_t *tb = (typeof(tb)) mp->b_datap->db_base;
	unsigned long flags;
	toid_t tid;

	if ((tid = xchg(&tb->tb_tid, 0)))
		untimeout(tid);

	spin_lock_irqsave(&tb->tb_lock, flags);
	tb->tb_time = jiffies + ticks;
	switch (__builtin_expect(tb->tb_state, TB_IDLE)) {
	case TB_EXPIRED:
		/* Timer has expired, can be rescheduled and may be on queue. */
	case TB_TRANSIENT:
		/* Timer is transient, can be rescheduled and may be on queue. */
	case TB_CANCELLED:
		/* Timer was cancelled, can be rescheduled and may be on queue. */
	case TB_RESCHEDULED:
		/* Timer was rescheduled, can be rescheduled and may be on queue. */
		tb->tb_time = jiffies + ticks;
		tb->tb_state = TB_RESCHEDULED;
		break;
	case TB_ACTIVE:
		/* Timer is actively running, can be rescheduled and is not on queue. */
		if (tid == (toid_t) 0) {
			tb->tb_time = jiffies + ticks;
			tb->tb_state = TB_RESCHEDULED;
			break;
		}
	case TB_IDLE:
		/* Timer is idle (not running), can be rescheduled and is not on queue. */
		if (ticks <= 0)
			ticks = 2;
		tb->tb_state = TB_ACTIVE;
		tb->tb_tid = timeout(mi_timer_expiry, (caddr_t) mp, ticks);
		if (tb->tb_tid)
			break;
		tb->tb_time = jiffies + ticks;
		tb->tb_state = TB_RESCHEDULED;
		mi_timer_putq(tb->tb_q, mp);
		break;
	case TB_ZOMBIE:
		/* Timer is a zombie waiting to be freed, and may be on queue. */
		break;
	default:
		swerr();
		break;
	}
	spin_unlock_irqrestore(&tb->tb_lock, flags);
}

EXPORT_SYMBOL(mi_timer_ticks);

/**
 *  mi_timer_SUN: - start a timer -- OpenSolaris variety
 *  @q: queue to which to ultimately timer message on expiry
 *  @mp: the timer message
 *  @msec: the time in milliseconds
 *
 *  Accepts several special values for @msec.  When msec is (clock_t)(-1), then stop the timer.
 *  When msec is (clock_t)(-2) move the timer to the new @q specified.  When msec is (clock_t)(0),
 *  requeue the message.
 */
__MPS_EXTERN void
mi_timer_SUN(queue_t *q, mblk_t *mp, clock_t msec)
{
	tblk_t *tb = (typeof(tb)) mp->b_datap->db_base;

	if (msec >= 0) {
		mi_timer_move(q, mp);
		mi_timer_ticks(mp, drv_msectohz(msec));
	} else {
		switch (msec) {
		case ((clock_t) (-1)):
			mi_timer_stop(mp);
			break;
		case ((clock_t) (-2)):
			mi_timer_move(q, mp);
			break;
		default:
			swerr();
			break;
		}
	}
}

EXPORT_SYMBOL(mi_timer_SUN);

/**
 * mi_timer_stop: - stop a timer (OpenSolaris variety)
 * @mp: timer message to stop
 *
 * We want mi_timer_stop() to be way faster than mi_timer_cancel() which is plodding and always
 * attempts to remove the message from the queue if it finds it there.
 */
__MPS_EXTERN void
mi_timer_stop(mblk_t *mp)
{
	mi_timer_cancel(mp);
}

EXPORT_SYMBOL(mi_timer_stop);

/**
 * mi_timer_move: - move a timer message to different queue (OpenSolaris variety)
 * @q: queue to which to move the message
 * @mp: the timer message
 *
 * Moves the timer message, @mp, from the currently active target queue to the specified target
 * queue, @q.  If the timer is in some temporary state and was sitting on the old target queue, it
 * is removed from that queue and its disposition made permanent, which may involve being requeued
 * to the new target queue.
 */
__MPS_EXTERN void
mi_timer_move(queue_t *q, mblk_t *mp)
{
	tblk_t *tb = (typeof(tb)) mp->b_datap->db_base;
	unsigned long flags;
	queue_t *oldq;

	spin_lock_irqsave(&tb->tb_lock, flags);
	if ((oldq = XCHG(&tb->tb_q, q)) != q) {
		switch (tb->tb_state) {
		default:
			/* Do no know, illegal state. */
			swerr();
		case TB_TRANSIENT:
			/* Timer is transient, could be on a queue. */
		case TB_EXPIRED:
			/* Timer is expired, could be on a queue. */
		case TB_RESCHEDULED:
			/* Timer is recheduled, could be on a queue. */
		case TB_CANCELLED:
			/* Timer is cancelled, could be on a queue. */
		case TB_ZOMBIE:
			/* Timer is zombie waiting to be free, could be on a queue */
			if (mi_timer_remove(oldq, mp)) {
				/* was queued, cannot be in user's hands */
				/* do similar to mi_timer_valid here */
				switch (tb->tb_state) {
				case TB_RESCHEDULED:
					if (tb->tb_time > jiffies) {
						tb->tb_state = TB_ACTIVE;
						tb->tb_tid =
						    timeout(mi_timer_expiry, (caddr_t) mp,
							    tb->tb_time - jiffies);
						if (tb->tb_tid)
							break;
						tb->tb_state = TB_RESCHEDULED;
						mi_timer_putq(tb->tb_q, mp);
						break;
					}
					/* fall through */
				case TB_TRANSIENT:
				case TB_EXPIRED:
					tb->tb_state = TB_EXPIRED;
					mi_timer_putq(tb->tb_q, mp);
					break;
				case TB_CANCELLED:
					tb->tb_state = TB_IDLE;
					break;
				case TB_ZOMBIE:
					tb->tb_state = TB_IDLE;
					spin_unlock_irqrestore(&tb->tb_lock, flags);
					freemsg(mp);
					return;
				}
				break;
			}
			/* fall through */
		case TB_ACTIVE:
			/* Timer is running and is not on any queue. */
		case TB_IDLE:
			/* Timer is idle and is not on any queue. */
			break;
		}
	}
	spin_unlock_irqrestore(&tb->tb_lock, flags);
}

EXPORT_SYMBOL(mi_timer_move);

/*
 *  Common
 */

/**
 * mi_timer_valid: - check validity of timer message (Common)
 * @mp: timer message
 *
 * See mi_timer(9) for user documentation.  Slight variation on a theme here.  We allow the message
 * to be requeued and processed later.  (MacOT documentation says that this function must not be
 * called twice in a row on the same message.)  The user may mark the message type to M_SIG when
 * valid is returned so that the message will be requeued as a normal priority message.  When
 * mi_timer_valid() is called for the second time on the message, this function returns true again if
 * no other action has been taken on the timer while it was waiting on queue.  If any other action
 * was taken, the second pass will return false.
 *
 * Timer message states are as follows (for messages taken off the queue):
 *
 * TB_ZOMBIE:					- freed timer
 * TB_CANCELLED:				- cancelled timer
 * TB_IDLE:					- fired timer processed again
 * TB_ACTIVE:	    tb_tid == (toid_t) 0	- timer fired
 * TB_ACTIVE:	    tb_tid != (toid_t) 0	- timer running, will be requeued on expiry
 * TB_RESCHEDULED:				- timer needs rescheduling
 * TB_TRANSIENT					- timer in transient state (fired)
 *
 * In implementation it is just easier if messages are not simply placed onto queues after having
 * mi_timer_valid() called on them.  Try mi_timer_MAC(tp, 0) or mi_timer_SUN(q, tp, 0) as a more
 * portable way of accomplishing this.  These functions have been adjusted to simply queue the
 * message immediately when the timeout is zero.  On other implementation it should have a similar
 * effect (even if timeout is called with a zero argument).
 */
__MPS_EXTERN int
mi_timer_valid(mblk_t *mp)
{
	tblk_t *tb = (typeof(tb)) mp->b_datap->db_base;
	unsigned long flags;
	int rval = (0);

	spin_lock_irqsave(&tb->tb_lock, flags);
	switch (__builtin_expect(tb->tb_state, TB_EXPIRED)) {
	case TB_RESCHEDULED:
		/* Timer was rescheduled while in user's hands. */
		if (tb->tb_time > jiffies) {
			tb->tb_state = TB_ACTIVE;
			tb->tb_tid = timeout(mi_timer_expiry, (caddr_t) mp, tb->tb_time - jiffies);
			if (tb->tb_tid)
				break;
			tb->tb_state = TB_RESCHEDULED;
			mi_timer_putq(tb->tb_q, mp);
			break;
		}
		/* Timer has already expired. */
		/* fall through */
	case TB_EXPIRED:
		/* Timer is expired and was queued as the result of a timer timeout */
	case TB_TRANSIENT:
		/* Timer is transient and was requeued by the put or service procedure. */
		tb->tb_state = TB_IDLE;
		mp->b_datap->db_type = M_PCSIG;
		rval = (1);
		break;
	case TB_CANCELLED:
		/* Timer was cancelled while in user's hands. */
	case TB_IDLE:
		/* Timer was cancelled while in the user's hands. */
		tb->tb_state = TB_IDLE;
		rval = (0);
		break;
	case TB_ZOMBIE:
		/* Timer was freed while in user's hands. */
		tb->tb_state = TB_IDLE;
		spin_unlock_irqrestore(&tb->tb_lock, flags);
		freemsg(mp);
		return (0);
	default:
	case TB_ACTIVE:
		/* Timers in the active state are never in the user's hands. */
		swerr();
		break;
	}
	spin_unlock_irqrestore(&tb->tb_lock, flags);
	return (rval);
}

EXPORT_SYMBOL(mi_timer_valid);

/**
 * mi_timer_running: - is the timer running
 * @mp: timer message
 */
__MPS_EXTERN int
mi_timer_running(mblk_t *mp)
{
	tblk_t *tb = (typeof(tb)) mp->b_datap->db_base;
	int state = (volatile int) tb->tb_state;

	return (state == TB_ACTIVE || state == TB_RESCHEDULED);
}

EXPORT_SYMBOL_GPL(mi_timer_running);

/**
 * mi_timer_cond: - start timer if not already running
 * @mp: timer message
 * @msec: milliseconds to expiry
 *
 * Return is true (1) if the timer was restarted, or false (0) if it was just left running.
 */
__MPS_EXTERN int
mi_timer_cond(mblk_t *mp, clock_t ticks)
{
	tblk_t *tb = (typeof(tb)) mp->b_datap->db_base;
	unsigned long flags;
	int rtn;

	spin_lock_irqsave(&tb->tb_lock, flags);
	switch (tb->tb_state) {
	case TB_ACTIVE:
		/* Timer is running. */
	case TB_EXPIRED:
		/* Timer has expired and will be queued. */
	case TB_RESCHEDULED:
		/* Timer is running. */
		rtn = (0);	/* already running */
		break;
	case TB_IDLE:
		/* Timer is idle, not on queue. */
	case TB_TRANSIENT:
		/* Timer is transient, not on queue. */
		rtn = (1);
		if (ticks <= 0)
			ticks = 2;
		tb->tb_state = TB_ACTIVE;
		tb->tb_tid = timeout(mi_timer_expiry, (caddr_t) mp, ticks);
		if (tb->tb_tid)
			break;
		tb->tb_time = jiffies + ticks;
		tb->tb_state = TB_RESCHEDULED;
		mi_timer_putq(tb->tb_q, mp);
		break;
	case TB_CANCELLED:
		/* Timer has been cancelled, but will be queued */
		tb->tb_time = jiffies + ticks;
		tb->tb_state = TB_RESCHEDULED;
		rtn = (1);
		break;
	default:
		swerr();
	case TB_ZOMBIE:
		/* Timer is a zombie, waiting to be freed */
		rtn = (0);
		break;
	}
	spin_unlock_irqrestore(&tb->tb_lock, flags);
	return (rtn);
}

EXPORT_SYMBOL_GPL(mi_timer_cond);

/**
 * mi_timer_remain: - return remaining time on timer
 * @mp: timer message
 *
 * Returns the remaining time (in milliseconds) on a timer if running or rescheduled, zero
 * otherwise.
 */
__MPS_EXTERN unsigned long
mi_timer_remain(mblk_t *mp)
{
	tblk_t *tb = (typeof(tb)) mp->b_datap->db_base;
	unsigned long flags;
	unsigned long rval = 0;

	spin_lock_irqsave(&tb->tb_lock, flags);
	switch (__builtin_expect(tb->tb_state, TB_ACTIVE)) {
	case TB_ACTIVE:
	case TB_RESCHEDULED:
		if (tb->tb_time > jiffies)
			rval = tb->tb_time - jiffies;
		break;
	default:
		rval = 0;
		break;
	}
	spin_unlock_irqrestore(&tb->tb_lock, flags);
	if (rval)
		rval = drv_hztomsec(rval);
	return (rval);
}

EXPORT_SYMBOL_GPL(mi_timer_remain);

/**
 * mi_timer_free: - free a timer message (Common)
 * @mp: timer message to free
 *
 * Cancels and frees (or marks to be freed) a timer message.
 */
__MPS_EXTERN void
mi_timer_free(mblk_t *mp)
{
	tblk_t *tb;
	unsigned long flags;
	toid_t tid;

	if (mp == NULL)
		return;

	tb = (typeof(tb)) mp->b_datap->db_base;

	if ((tid = xchg(&tb->tb_tid, 0)))
		untimeout(tid);

	spin_lock_irqsave(&tb->tb_lock, flags);
	switch (tb->tb_state) {
	case TB_ACTIVE:
		if (!tid)
			/* unsuccessful cancel, will be queued */
			goto zombie;
		/* fall through */
	case TB_IDLE:
		/* not on queue */
		spin_unlock_irqrestore(&tb->tb_lock, flags);
		freemsg(mp);
		return;
	default:
	      zombie:
		tb->tb_state = TB_ZOMBIE;
		break;
	}
	spin_unlock_irqrestore(&tb->tb_lock, flags);
}

EXPORT_SYMBOL(mi_timer_free);

__MPS_EXTERN queue_t *
mi_allocq(struct streamtab *st)
{
	queue_t *q;

	if ((q = allocq()))
		setq(q, st->st_rdinit, st->st_wrinit);
	return (q);
}

EXPORT_SYMBOL(mi_allocq);

__MPS_EXTERN void
mi_freeq(queue_t *q)
{
	freeq(q);
}

EXPORT_SYMBOL(mi_freeq);

__MPS_EXTERN int
mi_strlog(queue_t *q, char level, ushort flags, char *fmt, ...)
{
	int result;
	struct mi_comm *mi;
	modID_t mid = 0;
	minor_t sid = 0;
	va_list args;

	if (q != NULL && (mi = ptr_to_mi(q->q_ptr)) != NULL) {
		mid = mi->mi_mid;
		sid = mi->mi_sid;
	}
	va_start(args, fmt);
	result = vstrlog(mid, sid, level, flags, fmt, args);
	va_end(args);
	return (result);
}

EXPORT_SYMBOL(mi_strlog);

#define _MI_FLAG_ZEROPAD	(1<<0)
#define _MI_FLAG_SIGN		(1<<1)
#define _MI_FLAG_PLUS		(1<<2)
#define _MI_FLAG_SPACE		(1<<3)
#define _MI_FLAG_LEFT		(1<<4)
#define _MI_FLAG_SPECIAL	(1<<5)
#define _MI_FLAG_LARGE		(1<<6)

static int
mi_putc(char c, mblk_t **mpp)
{
	if (unlikely((*mpp)->b_wptr >= (*mpp)->b_datap->db_lim)) {
		mblk_t *mp;

		mp = allocb(256, BPRI_MED);
		(*mpp)->b_cont = mp;
		mpp = &(*mpp)->b_cont;
	}
	if (likely(*mpp != NULL)) {
		(*mpp)->b_wptr[0] = (unsigned char) c;
		(*mpp)->b_wptr++;
		return (1);
	}
	return (0);
}

static int
mi_number(mblk_t **mpp, unsigned long long num, int base, int width, int decimal, int flags)
{
	char sign;
	int i, count = 0, rval = 0;
	char tmp[66];

	if (flags & _MI_FLAG_LEFT)
		flags &= ~_MI_FLAG_ZEROPAD;
	if (base < 2 || base > 16)
		return (count);
	sign = '\0';
	if (flags & _MI_FLAG_SIGN) {
		if ((signed long long) num < 0) {
			sign = '-';
			num = -(signed long long) num;
			width--;
		} else if (flags & _MI_FLAG_PLUS) {
			sign = '+';
			width--;
		} else if (flags & _MI_FLAG_SPACE) {
			sign = ' ';
			width--;
		}
	}
	if (flags & _MI_FLAG_SPECIAL) {
		switch (base) {
		case 16:
			width -= 2;	/* for 0x */
			break;
		case 8:
			width--;	/* for 0 */
			break;
		}
	}
	i = 0;
	if (num == 0)
		tmp[i++] = '0';
	else {
		const char *hexdig =
		    (flags & _MI_FLAG_LARGE) ? "0123456789ABCDEF" : "0123456789abcdef";
		for (; num != 0; tmp[i++] = hexdig[do_div(num, base)]) ;
	}
	if (i > decimal)
		decimal = i;
	width -= decimal;
	if (!(flags & (_MI_FLAG_ZEROPAD | _MI_FLAG_LEFT)))
		while (width-- > 0) {
			if (!mi_putc(' ', mpp))
				goto abort;
			count++;
		}
	if (sign != '\0') {
		if (!mi_putc(sign, mpp))
			goto abort;
		count++;
	}
	if (flags & _MI_FLAG_SPECIAL) {
		switch (base) {
		case 8:
			if (!mi_putc('0', mpp))
				goto abort;
			count++;
			break;
		case 16:
		{
			char xchar = (flags & _MI_FLAG_LARGE) ? 'X' : 'x';

			if (!mi_putc('0', mpp))
				goto abort;
			count++;
			if (!mi_putc(xchar, mpp))
				goto abort;
			count++;
			break;
		}
		}
	}
	if (!(flags & _MI_FLAG_LEFT)) {
		char pad = (flags & _MI_FLAG_ZEROPAD) ? '0' : ' ';

		while (width-- > 0) {
			if (!mi_putc(pad, mpp))
				goto abort;
			count++;
		}
	}
	while (i < decimal--) {
		if (!mi_putc('0', mpp))
			goto abort;
		count++;
	}
	while (i-- > 0) {
		if (!mi_putc(tmp[i], mpp))
			goto abort;
		count++;
	}
	while (width-- > 0) {
		if (!mi_putc(' ', mpp))
			goto abort;
		count++;
	}
	rval = count;
      abort:
	return (rval);
}

static int
mi_vmpprintf(mblk_t *mp, char *fmt, va_list args)
{
	char type;
	int count = 0, rval = -1, flags = 0, width = -1, decimal = -1, base = 10;
	mblk_t **mpp = &mp;

	for (; *fmt; ++fmt) {
		const char *pos;
		unsigned long long num = 0;

		if (*fmt != '%') {
			if (!mi_putc(*fmt, mpp))
				goto abort;
			count++;
			continue;
		}
		pos = fmt;	/* remember position of % */
		/* process flags */
		for (++fmt;; ++fmt) {
			switch (*fmt) {
			case '-':
				flags |= _MI_FLAG_LEFT;
				continue;
			case '+':
				flags |= _MI_FLAG_PLUS;
				continue;
			case ' ':
				flags |= _MI_FLAG_SPACE;
				continue;
			case '#':
				flags |= _MI_FLAG_SPECIAL;
				continue;
			case '0':
				flags |= _MI_FLAG_ZEROPAD;
				continue;
			default:
				break;
			}
		}
		/* get field width */
		if (isdigit(*fmt))
			for (width = 0; isdigit(*fmt); width *= 10, width += (*fmt - '0')) ;
		else if (*fmt == '*') {
			++fmt;
			if ((width = va_arg(args, int)) < 0) {
				width = -width;
				flags |= _MI_FLAG_LEFT;
			}
		}
		/* get the decimal precision */
		if (*fmt == '.') {
			++fmt;
			if (isdigit(*fmt))
				for (decimal = 0; isdigit(*fmt);
				     decimal += 10, decimal += (*fmt - '0')) ;
			else if (*fmt == '*') {
				if ((decimal = va_arg(args, int)) < 0)
					 decimal = 0;
			}
		}
		/* get conversion type */
		type = 'i';
		switch (*fmt) {
		case 'h':
			type = *fmt;
			++fmt;
			if (*fmt == 'h') {
				type = 'c';
				++fmt;
			}
			break;
		case 'l':
			type = *fmt;
			++fmt;
			if (*fmt == 'l') {
				type = 'L';
				++fmt;
			}
			break;
		case 'q':
		case 'L':
		case 'Z':
		case 'z':
		case 't':
			type = *fmt;
			++fmt;
			break;
		}
		switch (*fmt) {
		case 'c':
		{
			char c;

			if (!(flags & _MI_FLAG_LEFT))
				while (--width > 0) {
					if (!mi_putc(' ', mpp))
						goto abort;
					count++;
				}
			c = va_arg(args, int);

			if (!mi_putc(c, mpp))
				goto abort;
			count++;
			while (--width > 0) {
				if (!mi_putc(' ', mpp))
					goto abort;
				count++;
			}
			continue;
		}
		case 's':
		{
			const char *s;
			int i;
			size_t len = 0;
			s = va_arg(args, char *);

			len = strnlen(s, 1024);
			if (!(flags & _MI_FLAG_LEFT))
				while (len < width--) {
					if (!mi_putc(' ', mpp))
						goto abort;
					count++;
				}
			for (i = 0; i < len; ++i, ++s) {
				if (!mi_putc(*s, mpp))
					goto abort;
				count++;
			}
			while (len < width--) {
				if (!mi_putc(' ', mpp))
					goto abort;
				count++;
			}
			continue;
		}
		case '%':
			if (!mi_putc('%', mpp))
				goto abort;
			count++;
			continue;
		case 'p':
		case 'o':
		case 'X':
		case 'x':
		case 'd':
		case 'i':
		case 'u':
			switch (*fmt) {
			case 'p':
				type = 'p';
				if (width == -1) {
					width = 2 * sizeof(void *);
					flags |= _MI_FLAG_ZEROPAD;
				}
				base = 16;
				break;
			case 'o':
				base = 8;
				break;
			case 'X':
				flags |= _MI_FLAG_LARGE;
			case 'x':
				base = 16;
				break;
			case 'd':
			case 'i':
				flags |= _MI_FLAG_SIGN;
			case 'u':
				base = 10;
				break;
			}
			switch (type) {
			case 'c':
				num = va_arg(args, int);

				if (flags & _MI_FLAG_SIGN)
					num = (signed char) num;
				break;
			case 'h':
				num = va_arg(args, int);

				if (flags & _MI_FLAG_SIGN)
					num = (signed short) num;
				break;
			case 'p':
				num = va_arg(args, uintptr_t);
				flags &= ~_MI_FLAG_SIGN;
				break;
			case 'l':
				num = va_arg(args, unsigned long);

				if (flags & _MI_FLAG_SIGN)
					num = (signed long) num;
				break;
			case 'q':
			case 'L':
				num = va_arg(args, unsigned long long);

				if (flags & _MI_FLAG_SIGN)
					num = (signed long long) num;
				break;
			case 'Z':
			case 'z':
				num = va_arg(args, size_t);

				if (flags & _MI_FLAG_SIGN)
					num = (ssize_t) num;
				break;
			case 't':
				num = va_arg(args, ptrdiff_t);
				break;
			case 'i':
			default:
				num = va_arg(args, unsigned int);

				if (flags & _MI_FLAG_SIGN)
					num = (signed int) num;
				break;
			}
			{
				int result;

				if (!(result = mi_number(mpp, num, base, width, decimal, flags)))
					goto abort;
				count += result;
			}
			continue;
		case '\0':
		default:
			/* put origianl '%' */
			if (!mi_putc('%', mpp))
				goto abort;
			count++;
			/* put the bad format characters */
			for (; fmt > pos; pos++, count++)
				if (!mi_putc(*pos, mpp))
					goto abort;
			if (*fmt == '\0')
				break;
			continue;
		}
		break;
	}
	if (!mi_putc('\0', mpp))
		goto abort;
	rval = count;
      abort:
	return (rval);
}

/**
 *  mi_mpprintf: - print a formatted string to a message block
 *  @mp: message block into which to print
 *  @fmt: printf-style format string
 *  @...: arguments to format string
 *
 *  This function is similar to snprintf() with the exception that the message string is printed
 *  into the message block, with new message blocks being created on demand as required, instead of
 *  to a linear buffer of fixed size.  A newline character will always be written to terminate the
 *  string.
 */
__MPS_EXTERN int
mi_mpprintf(mblk_t *mp, char *fmt, ...)
{
	va_list args;
	int count = -1;

	va_start(args, fmt);
	if (mp)
		count = mi_vmpprintf(mp, fmt, args);
	va_end(args);
	return (count);
}

EXPORT_SYMBOL(mi_mpprintf);

/**
 * mi_mpprintf_nr: - printf to a message block removing previous newline
 * @mp: message block to which to print
 * @fmt: printf style format string
 * @...: arguments to format string
 *
 * mi_mpprintf_nr() is identical to mi_mpprintf() with the exception that it strips the trailing
 * newline character from the previous string written to the message block with either mi_mpprintf()
 * or a previous call to mi_mpprintf_nr().
 */
__MPS_EXTERN int
mi_mpprintf_nr(mblk_t *mp, char *fmt, ...)
{
	va_list args;
	int count = -1;

	va_start(args, fmt);
	if (mp) {
		adjmsg(mp, -1);
		count = mi_vmpprintf(mp, fmt, args);
	}
	va_end(args);
	return (count);
}

EXPORT_SYMBOL(mi_mpprintf_nr);

/*
 *  =========================================================================
 *
 *  Stream head helper functions
 *
 *  =========================================================================
 */

/**
 * mi_set_sth_hiwat: - set Stream Head high water mark
 * @q: read queue of queue pair in Stream
 * @size: size of high water mark (in bytes)
 */
__MPS_EXTERN int
mi_set_sth_hiwat(queue_t *q, size_t size)
{
	struct stroptions *so;
	mblk_t *mp;

	dassert(q == RD(q));
	if ((mp = allocb(sizeof(*so), BPRI_MED))) {
		mp->b_datap->db_type = M_SETOPTS;
		mp->b_wptr += sizeof(*so);
		so = (typeof(so)) mp->b_rptr;
		so->so_flags = SO_HIWAT;
		so->so_band = 0;
		so->so_hiwat = size;
		putnext(q, mp);
		return (1);
	}
	return (0);
}

EXPORT_SYMBOL(mi_set_sth_hiwat);

/**
 * mi_set_sth_lowat: - set Stream Head low water mark
 * @q: read queue of queue pair in Stream
 * @size: size of low water mark (in bytes)
 */
__MPS_EXTERN int
mi_set_sth_lowat(queue_t *q, size_t size)
{
	struct stroptions *so;
	mblk_t *mp;

	dassert(q == RD(q));
	if ((mp = allocb(sizeof(*so), BPRI_MED))) {
		mp->b_datap->db_type = M_SETOPTS;
		mp->b_wptr += sizeof(*so);
		so = (typeof(so)) mp->b_rptr;
		so->so_flags = SO_LOWAT;
		so->so_band = 0;
		so->so_lowat = size;
		putnext(q, mp);
		return (1);
	}
	return (0);
}

EXPORT_SYMBOL(mi_set_sth_lowat);

/**
 * mi_set_sth_maxblk: - set Stream Head maximum block size
 * @q: read queue of queue pair in Stream
 * @size: maximum block size (bytes)
 */
__MPS_EXTERN int
mi_set_sth_maxblk(queue_t *q, ssize_t size)
{
#ifdef SO_MAXBLK
	struct stroptions *so;
	mblk_t *mp;

	dassert(q == RD(q));
	if ((mp = allocb(sizeof(*so), BPRI_MED))) {
		mp->b_datap->db_type = M_SETOPTS;
		mp->b_wptr += sizeof(*so);
		so = (typeof(so)) mp->b_rptr;
		so->so_flags = SO_MAXBLK;
		so->so_maxblk = size;
		putnext(q, mp);
		return (1);
	}
#endif
	return (0);
}

EXPORT_SYMBOL(mi_set_sth_maxblk);

/**
 * mi_set_sth_coyopt: - set Stream Head copy options
 * @q: read queue of queue pair in Stream
 * @copyopt: copy options
 */
__MPS_EXTERN int
mi_set_sth_copyopt(queue_t *q, int copyopt)
{
#ifdef SO_COPYOPT
	struct stroptions *so;
	mblk_t *mp;

	dassert(q == RD(q));
	if ((mp = allocb(sizeof(*so), BPRI_MED))) {
		mp->b_datap->db_type = M_SETOPTS;
		mp->b_wptr += sizeof(*so);
		so = (typeof(so)) mp->b_rptr;
		so->so_flags = SO_COPYOPT;
		so->so_copyopt = copyopt;
		putnext(q, mp);
		return (1);
	}
#endif
	return (0);
}

EXPORT_SYMBOL(mi_set_sth_copyopt);

/**
 * mi_set_sth_wroff: - set Stream Head write offset
 * @q: read queue of queue pair in Stream
 * @size: size of write offset (bytes)
 */
__MPS_EXTERN int
mi_set_sth_wroff(queue_t *q, size_t size)
{
	struct stroptions *so;
	mblk_t *mp;

	dassert(q == RD(q));
	if ((mp = allocb(sizeof(*so), BPRI_MED))) {
		mp->b_datap->db_type = M_SETOPTS;
		mp->b_wptr += sizeof(*so);
		so = (typeof(so)) mp->b_rptr;
		so->so_flags = SO_WROFF;
		so->so_wroff = size;
		putnext(q, mp);
		return (1);
	}
	return (0);
}

EXPORT_SYMBOL(mi_set_sth_wroff);

/*
 *  =========================================================================
 *
 *  System wrapper functions
 *
 *  =========================================================================
 */
__MPS_EXTERN int
mi_sprintf(char *buf, char *fmt, ...)
{
	int result;
	va_list args;

	va_start(args, fmt);
	result = vsprintf(buf, fmt, args);
	va_end(args);
	return result;
}

EXPORT_SYMBOL(mi_sprintf);

__MPS_EXTERN int
mi_strcmp(const caddr_t cp1, const caddr_t cp2)
{
	return strcmp(cp1, cp2);
}

EXPORT_SYMBOL(mi_strcmp);

__MPS_EXTERN int
mi_strlen(const caddr_t str)
{
	return strlen(str);
}

EXPORT_SYMBOL(mi_strlen);

__MPS_EXTERN long
mi_strtol(const caddr_t str, caddr_t *ptr, int base)
{
	return simple_strtol(str, ptr, base);
}

EXPORT_SYMBOL(mi_strtol);

/*
 *  =========================================================================
 *
 *  Message block functions
 *
 *  =========================================================================
 */
/*
 *  MI_OFFSET_PARAM
 *  -------------------------------------------------------------------------
 */
__MPS_EXTERN uint8_t *
mi_offset_param(mblk_t *mp, size_t offset, size_t len)
{
	if (mp || len == 0) {
		size_t blen = mp->b_wptr > mp->b_rptr ? mp->b_wptr - mp->b_rptr : 0;

		if (blen >= offset + len)
			return (mp->b_rptr + offset);
	}
	return (NULL);
}

EXPORT_SYMBOL(mi_offset_param);

/*
 *  MI_OFFSET_PARAMC
 *  -------------------------------------------------------------------------
 */
__MPS_EXTERN uint8_t *
mi_offset_paramc(mblk_t *mp, size_t offset, size_t len)
{
	uint8_t *result = NULL;

	for (; mp; mp = mp->b_cont) {
		if (isdatamsg(mp)) {
			if ((result = mi_offset_param(mp, offset, len)))
				break;
			else {
				size_t blen = mp->b_wptr > mp->b_rptr ? mp->b_wptr - mp->b_rptr : 0;

				if (offset < blen)
					/* spans blocks - should do a pullup */
					break;
				offset -= blen;
			}
		}
	}
	return (result);
}

EXPORT_SYMBOL(mi_offset_paramc);

/*
 *  =========================================================================
 *
 *  Some internal functions showing...
 *
 *  =========================================================================
 */
/*
 *  MPS_BECOME_WRITER
 *  -------------------------------------------------------------------------
 */
__MPS_EXTERN void
mps_become_writer(queue_t *q, mblk_t *mp, proc_ptr_t proc)
{
	extern void __strwrit(queue_t *q, mblk_t *mp,
			      void streamscall (*func) (queue_t *, mblk_t *), int perim);
	__strwrit(q, mp, proc, PERIM_INNER | PERIM_OUTER);
}

EXPORT_SYMBOL(mps_become_writer);

/*
 *  MPS_INTR_DISABLE
 *  -------------------------------------------------------------------------
 */
__MPS_EXTERN void
mps_intr_disable(pl_t * plp)
{
	unsigned long flags = *plp;

	local_irq_save(flags);
	*plp = flags;
}

EXPORT_SYMBOL(mps_intr_disable);

/*
 *  MPS_INTR_ENABLE
 *  -------------------------------------------------------------------------
 */
__MPS_EXTERN void
mps_intr_enable(pl_t pl)
{
	unsigned long flags = pl;

	local_irq_restore(flags);
}

EXPORT_SYMBOL(mps_intr_enable);

#ifdef CONFIG_STREAMS_COMPAT_MPS_MODULE
static
#endif
int __init
mpscomp_init(void)
{
#ifdef CONFIG_STREAMS_COMPAT_MPS_MODULE
	printk(KERN_INFO MPSCOMP_BANNER);
#else
	printk(KERN_INFO MPSCOMP_SPLASH);
#endif
	return (0);
}

#ifdef CONFIG_STREAMS_COMPAT_MPS_MODULE
static
#endif
void __exit
mpscomp_exit(void)
{
	return;
}

#ifdef CONFIG_STREAMS_COMPAT_MPS_MODULE
module_init(mpscomp_init);
module_exit(mpscomp_exit);
#endif
