/*****************************************************************************

 @(#) File: src/kernel/hpuxcompat.c

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2019  Monavacon Limited <http://www.monavacon.com/>
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

static char const ident[] = "src/kernel/hpuxcompat.c (" PACKAGE_ENVR ") " PACKAGE_DATE;

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

#define __HPUX_EXTERN_INLINE __inline__ streamscall __unlikely
#define __HPUX_EXTERN streamscall

#define _HPUX_SOURCE

#include "sys/os7/compat.h"

#define HPUXCOMP_DESCRIP	"HP-UX 11i v2 Compatibility for Linux Fast-STREAMS"
#define HPUXCOMP_EXTRA		"Part of UNIX SYSTEM V RELEASE 4.2 FAST STREAMS FOR LINUX"
#define HPUXCOMP_COPYRIGHT	"Copyright (c) 2008-2019  Monavacon Limited.  All Rights Reserved."
#define HPUXCOMP_REVISION	"OpenSS7 src/kernel/hpuxcompat.c (" PACKAGE_ENVR ") " PACKAGE_DATE
#define HPUXCOMP_DEVICE		"HP-UX 11i v2 Compatibility"
#define HPUXCOMP_CONTACT	"Brian Bidulock <bidulock@openss7.org>"
#define HPUXCOMP_LICENSE	"GPL"
#define HPUXCOMP_BANNER		HPUXCOMP_DESCRIP	"\n" \
				HPUXCOMP_EXTRA		"\n" \
				HPUXCOMP_COPYRIGHT	"\n" \
				HPUXCOMP_REVISION	"\n" \
				HPUXCOMP_DEVICE		"\n" \
				HPUXCOMP_CONTACT	"\n"
#define HPUXCOMP_SPLASH		HPUXCOMP_DEVICE		" - " \
				HPUXCOMP_REVISION	"\n"

#ifdef CONFIG_STREAMS_COMPAT_HPUX_MODULE
MODULE_AUTHOR(HPUXCOMP_CONTACT);
MODULE_DESCRIPTION(HPUXCOMP_DESCRIP);
MODULE_SUPPORTED_DEVICE(HPUXCOMP_DEVICE);
MODULE_LICENSE(HPUXCOMP_LICENSE);
#if defined MODULE_ALIAS
MODULE_ALIAS("streams-hpuxcompat");
#endif
#ifdef MODULE_VERSION
MODULE_VERSION(PACKAGE_ENVR);
#endif
#endif

#if	defined DEFINE_SPINLOCK
static DEFINE_SPINLOCK(sleep_lock);
#elif	defined __SPIN_LOCK_UNLOCKED
static lock_t sleep_lock = __SPIN_LOCK_UNLOCKED(sleep_lock);
#elif	defined SPIN_LOCK_UNLOCKED
static lock_t sleep_lock = SPIN_LOCK_UNLOCKED;
#else
#error cannot initialize spin locks
#endif

/**
 *  streams_get_sleep_lock: - acquire the global sleep lock
 *  @event:	the event which will be later passed to sleep
 *
 *  streams_get_sleep_lock() provides access to a global spinlock_t that may be used
 *  by all threads entering a wait queue to avoid race conditions between
 *  threads entering the wait queue.
 *
 *  Return Value:streams_get_sleep_lock() returns a pointer to the global sleep lock.
 */
__HPUX_EXTERN lock_t *
streams_get_sleep_lock(caddr_t event)
{
	(void) event;
	return &sleep_lock;
}

EXPORT_SYMBOL(streams_get_sleep_lock);	/* hpux/ddi.h */

__HPUX_EXTERN lock_t *get_sleep_lock(caddr_t event)
    __attribute__ ((alias(__stringify(streams_get_sleep_lock))));

EXPORT_SYMBOL(get_sleep_lock);	/* hpux/ddi.h */

/**
 *  streams_put: - deferred call to a STREAMS module qi_putp() procedure.
 *  @func:  put function (often the put() function)
 *  @q:	    queue against which to defer the call
 *  @mp:    message block to pass to the callback function
 *  @priv:  private data to pass to the callback function (often @q)
 *
 *  streams_put() will defer the function @func until it can synchronize with @q.  Once the @q has
 *  been syncrhonized, the STREAMS scheduler will call the callback function @func with arguments
 *  @priv and @mp.  streams_put() is closely related to qwrite() below.
 *
 *  Notices: @func will be called by the STREAMS executive on the same CPU as the CPU that called
 *  streams_put().  @func is guarateed not to run until the caller exits or preempts.
 *
 *  Usage: streams_put() is intended to be called from contexts outside of the STREAMS scheduler
 *  (e.g. interrupt service routines) where @func is intended to run under the STREAMS scheduler.
 *
 *  Examples: streams_put((void *)&put, q, mp, q) will effect the put() STREAMS utility, but always
 *  guaranteed to be executed within the STREAMS scheduler.
 */
__HPUX_EXTERN void
streams_put(streams_put_t func, queue_t *q, mblk_t *mp, void *priv)
{
	extern void __strfunc(void (*func) (void *, mblk_t *), queue_t *q, mblk_t *mp, void *arg);

	__strfunc(func, q, mp, priv);
}

EXPORT_SYMBOL(streams_put);	/* hpux/ddi.h */

__HPUX_EXTERN int
str_install_HPUX(struct stream_inst *inst)
{
	switch (inst->inst_flags & STR_TYPE_MASK) {
	case STR_IS_DEVICE:
	{
		struct cdevsw *cdev;
		int err;

		if (!(cdev = kmem_zalloc(sizeof(*cdev), KM_NOSLEEP)))
			return (ENOMEM);
		cdev->d_name = inst->name;
		cdev->d_str = &inst->inst_str_tab;
		/* build flags */
		cdev->d_flag = 0;
		if (inst->inst_flags & STR_QSAFETY) {
			cdev->d_flag |= D_SAFE;
		}
		switch ((cdev->d_sqlvl = inst->inst_sync_level)) {
		case SQLVL_NOP:
			cdev->d_flag |= D_MP;
			break;
			/* Note that HPUX does not allow more than one thread in _any_ open or
			   close procedure.  We signal this to the registration function by
			   setting D_MTOCEXCL without setting D_MTOUTPERIM when synchronization is
			   beneath a module and by setting D_MTOCEXCL without setting D_MTOUTPERIM
			   when sycncrhonization is at the module or above. */
		case SQLVL_QUEUE:
			cdev->d_flag |= D_MTPERQ | D_MTOCEXCL;
			break;
		case SQLVL_DEFAULT:
			/* Note that the default level for HPUX is queue pair instead of module
			   like other implementations. */
			cdev->d_sqlvl = SQLVL_QUEUEPAIR;
		case SQLVL_QUEUEPAIR:
			cdev->d_flag |= D_MTQPAIR | D_MTOCEXCL;
			break;
		case SQLVL_MODULE:
			cdev->d_flag |= D_MTPERMOD | D_MTOCEXCL;
			break;
		case SQLVL_ELSEWHERE:
			cdev->d_flag |= D_MTOCEXCL;
			cdev->d_sqinfo = inst->inst_sync_info;
			break;
		case SQLVL_GLOBAL:
			/* Note that the HPUX SQLVL_GLOBAL only guarantees one thread within
			   modules also set to SQLVL_GLOBAL in stark contrast to descriptions for
			   AIX.  This is really SQLVL_ELSEWHERE with a well-known common barrier. */
			cdev->d_flag |= D_MTOCEXCL;
			cdev->d_sqinfo = "global";
			cdev->d_sqlvl = SQLVL_ELSEWHERE;
			break;
		default:
			kmem_free(cdev, sizeof(*cdev));
			return (EINVAL);
		}
		if ((err = register_strdev(cdev, inst->inst_major)) < 0) {
			kmem_free(cdev, sizeof(*cdev));
			return (-err);
		}
		inst->inst_major = err;
		return (0);
	}
	case STR_IS_MODULE:
	{
		struct fmodsw *fmod;
		int err;

		if (!(fmod = kmem_zalloc(sizeof(*fmod), KM_NOSLEEP)))
			return (ENOMEM);
		fmod->f_name = inst->name;
		fmod->f_str = &inst->inst_str_tab;
		/* build flags */
		fmod->f_flag = 0;
		if (inst->inst_flags & STR_QSAFETY) {
			fmod->f_flag |= D_SAFE;
		}
		switch ((fmod->f_sqlvl = inst->inst_sync_level)) {
		case SQLVL_NOP:
			fmod->f_flag |= D_MP;
			break;
			/* Note that HPUX does not allow more than one thread in _any_ open or
			   close procedure.  We signal this to the registration function by
			   setting D_MTOCEXCL without setting D_MTOUTPERIM when synchronization is
			   beneath a module and by setting D_MTOCEXCL without setting D_MTOUTPERIM
			   when sycncrhonization is at the module or above. */
		case SQLVL_QUEUE:
			fmod->f_flag |= D_MTPERQ | D_MTOCEXCL;
			break;
		case SQLVL_DEFAULT:
			/* Note that the default level for HPUX is queue pair instead of module
			   like other implementations. */
			fmod->f_sqlvl = SQLVL_QUEUEPAIR;
		case SQLVL_QUEUEPAIR:
			fmod->f_flag |= D_MTQPAIR | D_MTOCEXCL;
			break;
		case SQLVL_MODULE:
			fmod->f_flag |= D_MTPERMOD | D_MTOCEXCL;
			break;
		case SQLVL_ELSEWHERE:
			fmod->f_flag |= D_MTOCEXCL;
			fmod->f_sqinfo = inst->inst_sync_info;
			break;
		case SQLVL_GLOBAL:
			/* Note that the HPUX SQLVL_GLOBAL only guarantees one thread within
			   modules also set to SQLVL_GLOBAL in stark contrast to descriptions for
			   AIX.  This is really SQLVL_ELSEWHERE with a well-known common barrier. */
			fmod->f_flag |= D_MTOCEXCL;
			fmod->f_sqinfo = "global";
			fmod->f_sqlvl = SQLVL_ELSEWHERE;
			break;
		default:
			kmem_free(fmod, sizeof(*fmod));
			return (EINVAL);
		}
		if ((err = register_strmod(fmod)) < 0) {
			kmem_free(fmod, sizeof(*fmod));
			return (-err);
		}
		inst->inst_major = err;
		return (0);
	}
	default:
		return (EINVAL);
	}
}

EXPORT_SYMBOL(str_install_HPUX);

__HPUX_EXTERN int
str_uninstall(struct stream_inst *inst)
{
	switch (inst->inst_flags & STR_TYPE_MASK) {
	case STR_IS_DEVICE:
	{
		struct cdevsw *cdev;
		int err;

		if ((cdev = cdev_str(&inst->inst_str_tab)) == NULL)
			return (ENOENT);
		if ((err = unregister_strdev(cdev, inst->inst_major)) == 0)
			kmem_free(cdev, sizeof(*cdev));
		return (-err);
	}
	case STR_IS_MODULE:
	{
		struct fmodsw *fmod;
		int err;

		if ((fmod = fmod_str(&inst->inst_str_tab)) == NULL)
			return (ENOENT);
		if ((err = unregister_strmod(fmod)) == 0)
			kmem_free(fmod, sizeof(fmod));
		return (-err);
	}
	default:
		return (EINVAL);
	}
}

EXPORT_SYMBOL(str_uninstall);

#ifdef CONFIG_STREAMS_COMPAT_HPUX_MODULE
static int
#else
int __init
#endif
hpuxcomp_init(void)
{
#ifdef CONFIG_STREAMS_COMPAT_HPUX_MODULE
	printk(KERN_INFO HPUXCOMP_BANNER);
#else
	printk(KERN_INFO HPUXCOMP_SPLASH);
#endif
	return (0);
}

#ifdef CONFIG_STREAMS_COMPAT_HPUX_MODULE
static void
#else
void __exit
#endif
hpuxcomp_exit(void)
{
	return;
}

#ifdef CONFIG_STREAMS_COMPAT_HPUX_MODULE
module_init(hpuxcomp_init);
module_exit(hpuxcomp_exit);
#endif
