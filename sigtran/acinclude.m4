# vim: ft=config sw=4 noet nocin nosi com=b\:#,b\:dnl,b\:***,b\:@%\:@ fo+=tcqlorn
# =============================================================================
# BEGINNING OF SEPARATE COPYRIGHT MATERIAL
# =============================================================================
# 
# @(#) $RCSfile: acinclude.m4,v $ $Name:  $($Revision: 0.9.2.20 $) $Date: 2009-02-18 20:18:41 $
#
# -----------------------------------------------------------------------------
#
# Copyright (c) 2001-2008  OpenSS7 Corporation <http://www.openss7.com/>
# Copyright (c) 1997-2001  Brian F. G. Bidulock <bidulock@openss7.org>
#
# All Rights Reserved.
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Affero General Public License as published by the Free
# Software Foundation; version 3 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more
# details.
#
# You should have received a copy of the GNU Affero General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>, or write to
# the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# -----------------------------------------------------------------------------
#
# U.S. GOVERNMENT RESTRICTED RIGHTS.  If you are licensing this Software on
# behalf of the U.S. Government ("Government"), the following provisions apply
# to you.  If the Software is supplied by the Department of Defense ("DoD"), it
# is classified as "Commercial Computer Software" under paragraph 252.227-7014
# of the DoD Supplement to the Federal Acquisition Regulations ("DFARS") (or any
# successor regulations) and the Government is acquiring only the license rights
# granted herein (the license rights customarily provided to non-Government
# users).  If the Software is supplied to any unit or agency of the Government
# other than DoD, it is classified as "Restricted Computer Software" and the
# Government's rights in the Software are defined in paragraph 52.227-19 of the
# Federal Acquisition Regulations ("FAR") (or any successor regulations) or, in
# the cases of NASA, in paragraph 18.52.227-86 of the NASA Supplement to the FAR
# (or any successor regulations).
#
# -----------------------------------------------------------------------------
#
# Commercial licensing and support of this software is available from OpenSS7
# Corporation at a fee.  See http://www.openss7.com/
#
# -----------------------------------------------------------------------------
#
# Last Modified $Date: 2009-02-18 20:18:41 $ by $Author: brian $
#
# =============================================================================

m4_include([m4/openss7.m4])
m4_include([m4/dist.m4])
m4_include([m4/pr.m4])
m4_include([m4/init.m4])
m4_include([m4/kernel.m4])
m4_include([m4/devfs.m4])
m4_include([m4/genksyms.m4])
m4_include([m4/man.m4])
m4_include([m4/public.m4])
m4_include([m4/rpm.m4])
m4_include([m4/deb.m4])
m4_include([m4/libraries.m4])
m4_include([m4/autotest.m4])
m4_include([m4/strconf.m4])
m4_include([m4/streams.m4])
m4_include([m4/strcomp.m4])
m4_include([m4/xns.m4])
m4_include([m4/xti.m4])
m4_include([m4/nsl.m4])
m4_include([m4/sock.m4])
m4_include([m4/inet.m4])
m4_include([m4/sctp.m4])
m4_include([m4/chan.m4])
m4_include([m4/x25.m4])
m4_include([m4/iso.m4])
m4_include([m4/isdn.m4])
m4_include([m4/atm.m4])
m4_include([m4/ss7.m4])
m4_include([m4/doxy.m4])
m4_include([m4/lib32.m4])
m4_include([m4/perl.m4])
m4_include([m4/snmp.m4])
m4_include([m4/agent.m4])

# =============================================================================
# AC_SIGTRAN
# -----------------------------------------------------------------------------
AC_DEFUN([AC_SIGTRAN], [dnl
    _OPENSS7_PACKAGE([SIGTRAN], [OpenSS7 STREAMS SIGTRAN])
    _SIGTRAN_OPTIONS
    _AUTOPR
    _MAN_CONVERSION
    _PUBLIC_RELEASE
    _INIT_SCRIPTS
    _RPM_SPEC
    _DEB_DPKG
    AC_CONFIG_FILES([debian/sigtran-core.postinst
		     debian/sigtran-core.postrm
		     debian/sigtran-core.preinst
		     debian/sigtran-core.prerm
		     debian/sigtran-devel.preinst
		     debian/sigtran-dev.postinst
		     debian/sigtran-dev.preinst
		     debian/sigtran-dev.prerm
		     debian/sigtran-doc.postinst
		     debian/sigtran-doc.preinst
		     debian/sigtran-doc.prerm
		     debian/sigtran-init.postinst
		     debian/sigtran-init.postrm
		     debian/sigtran-init.preinst
		     debian/sigtran-init.prerm
		     debian/sigtran-lib.preinst
		     debian/sigtran-source.preinst
		     debian/sigtran-util.preinst
		     src/util/modutils/sigtran
		     src/util/sysconfig/sigtran
		     src/include/sys/sigtran/version.h
		     Module.mkvars])
    _LDCONFIG
    USER_CPPFLAGS="$CPPFLAGS"
    USER_CFLAGS="$CFLAGS"
    USER_LDFLAGS="$LDFLAGS"
    _SIGTRAN_SETUP
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+ }"'-imacros ${top_builddir}/config.h'
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+ }"'-imacros ${top_builddir}/${STRCONF_CONFIG}'
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+ }"'-I${top_srcdir}'
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${SS7_CPPFLAGS:+ }}${SS7_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${ATM_CPPFLAGS:+ }}${ATM_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${ISDN_CPPFLAGS:+ }}${ISDN_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${ISO_CPPFLAGS:+ }}${ISO_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${X25_CPPFLAGS:+ }}${X25_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${CHAN_CPPFLAGS:+ }}${CHAN_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${SCTP_CPPFLAGS:+ }}${SCTP_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${XTI_CPPFLAGS:+ }}${XTI_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${INET_CPPFLAGS:+ }}${INET_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${SOCK_CPPFLAGS:+ }}${SOCK_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${NSL_CPPFLAGS:+ }}${NSL_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${XNS_CPPFLAGS:+ }}${XNS_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${STRCOMP_CPPFLAGS:+ }}${STRCOMP_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${STREAMS_CPPFLAGS:+ }}${STREAMS_CPPFLAGS}"
    if test :${linux_cv_k_ko_modules:-no} = :no ; then
	PKG_MODFLAGS='$(STREAMS_MODFLAGS) $(STRCOMP_MODFLAGS)'
dnl	if echo "$KERNEL_MODFLAGS" | grep 'modversions\.h' >/dev/null 2>&1 ; then
dnl	    PKG_MODFLAGS="${PKG_MODFLAGS}${PKG_MODFLAGS:+ }"'-include ${top_builddir}/${MODVERSIONS_H}'
dnl	    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+ }"'-I${top_builddir}/include'
dnl	fi
    fi
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+ }"'-I${top_builddir}/src/include -I${top_srcdir}/src/include'
dnl Just check config.log if you want to see these...
dnl AC_MSG_NOTICE([final user    CPPFLAGS  = $USER_CPPFLAGS])
dnl AC_MSG_NOTICE([final user    CFLAGS    = $USER_CFLAGS])
dnl AC_MSG_NOTICE([final user    LDFLAGS   = $USER_LDFLAGS])
dnl AC_MSG_NOTICE([final package INCLUDES  = $PKG_INCLUDES])
dnl AC_MSG_NOTICE([final package MODFLAGS  = $PKG_MODFLAGS])
dnl AC_MSG_NOTICE([final kernel  MODFLAGS  = $KERNEL_MODFLAGS])
dnl AC_MSG_NOTICE([final kernel  NOVERSION = $KERNEL_NOVERSION])
dnl AC_MSG_NOTICE([final kernel  CPPFLAGS  = $KERNEL_CPPFLAGS])
dnl AC_MSG_NOTICE([final kernel  CFLAGS    = $KERNEL_CFLAGS])
dnl AC_MSG_NOTICE([final kernel  LDFLAGS   = $KERNEL_LDFLAGS])
dnl AC_MSG_NOTICE([final streams CPPFLAGS  = $STREAMS_CPPFLAGS])
dnl AC_MSG_NOTICE([final streams MODFLAGS  = $STREAMS_MODFLAGS])
    AC_SUBST([USER_CPPFLAGS])dnl
    AC_SUBST([USER_CFLAGS])dnl
    AC_SUBST([USER_LDFLAGS])dnl
    AC_SUBST([PKG_INCLUDES])dnl
    AC_SUBST([PKG_MODFLAGS])dnl
    PKG_MANPATH='$(mandir)'"${PKG_MANPATH:+:}${PKG_MANPATH}"
    PKG_MANPATH="${STREAMS_MANPATH:+${STREAMS_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${STRCOMP_MANPATH:+${STRCOMP_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${XNS_MANPATH:+${XNS_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${NSL_MANPATH:+${NSL_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${SOCK_MANPATH:+${SOCK_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${INET_MANPATH:+${INET_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${XTI_MANPATH:+${XTI_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${SCTP_MANPATH:+${SCTP_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${CHAN_MANPATH:+${CHAN_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${X25_MANPATH:+${X25_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${ISO_MANPATH:+${ISO_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${ISDN_MANPATH:+${ISDN_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${ATM_MANPATH:+${ATM_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${SS7_MANPATH:+${SS7_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH='$(top_builddir)/doc/man'"${PKG_MANPATH:+:}${PKG_MANPATH}"
    AC_SUBST([PKG_MANPATH])dnl
    CPPFLAGS=
    CFLAGS=
    _SIGTRAN_OUTPUT
    _AGENT
    _AUTOTEST
    _DOXY
])# AC_SIGTRAN
# =============================================================================

# =============================================================================
# _SIGTRAN_OPTIONS
# -----------------------------------------------------------------------------
AC_DEFUN([_SIGTRAN_OPTIONS], [dnl
    _SIGTRAN_CHECK_SIGTRAN
    _SIGTRAN_CHECK_SCTP
])# _SIGTRAN_OPTIONS
# =============================================================================

# =============================================================================
# _SIGTRAN_CHECK_SIGTRAN
# -----------------------------------------------------------------------------
AC_DEFUN([_SIGTRAN_CHECK_SIGTRAN], [dnl
])# _SIGTRAN_CHECK_SIGTRAN
# =============================================================================

# =============================================================================
# _SIGTRAN_CHECK_SCTP
# -----------------------------------------------------------------------------
AC_DEFUN([_SIGTRAN_CHECK_SCTP], [dnl
])# _SIGTRAN_CHECK_SCTP
# =============================================================================

# =============================================================================
# _SIGTRAN_OTHER_SCTP
# -----------------------------------------------------------------------------
AC_DEFUN([_SIGTRAN_OTHER_SCTP], [dnl
])# _SIGTRAN_OTHER_SCTP
# =============================================================================

# =============================================================================
# _SIGTRAN_SETUP
# -----------------------------------------------------------------------------
AC_DEFUN([_SIGTRAN_SETUP], [dnl
    _LINUX_KERNEL
    _LINUX_DEVFS
    _GENKSYMS
    _LINUX_STREAMS
    _STRCOMP
    _XNS
    _XTI
    _NSL
    _SOCK
    _INET
    _SCTP
    _CHAN
    _X25
    _ISO
    _ISDN
    _ATM
    _SS7
    # here we have our flags set and can perform preprocessor and compiler
    # checks on the kernel
    _SIGTRAN_OTHER_SCTP
    _SIGTRAN_SETUP_MODULE
    _SIGTRAN_CONFIG_KERNEL
])# _SIGTRAN_SETUP
# =============================================================================

# =============================================================================
# _SIGTRAN_SETUP_MODULE
# -----------------------------------------------------------------------------
AC_DEFUN([_SIGTRAN_SETUP_MODULE], [dnl
    AC_REQUIRE([_LINUX_KERNEL])dnl
    if test :"${linux_cv_k_linkage:-loadable}" = :loadable ; then
	AC_DEFINE_UNQUOTED([SIGTRAN_CONFIG_MODULE], [], [When defined, SIGTRAN is
			    being compiled as a loadable kernel module.])
    else
	AC_DEFINE_UNQUOTED([SIGTRAN_CONFIG], [], [When defined, SIGTRAN is being
			    compiled as a kernel linkable object.])
    fi
    AM_CONDITIONAL([SIGTRAN_CONFIG_MODULE], [test :${linux_cv_k_linkage:-loadable} = :loadable])
    AM_CONDITIONAL([SIGTRAN_CONFIG], [test :${linux_cv_k_linkage:-loadable} = :linkable])
])# _SIGTRAN_SETUP_MODULE
# =============================================================================

# =============================================================================
# _SIGTRAN_CONFIG_KERNEL
# -----------------------------------------------------------------------------
# Later we should have some checks here for things like OSS, ALS, Zaptel, etc.
# -----------------------------------------------------------------------------
AC_DEFUN([_SIGTRAN_CONFIG_KERNEL], [dnl
    _LINUX_CHECK_HEADERS([linux/namespace.h linux/kdev_t.h linux/statfs.h linux/namei.h \
			  linux/locks.h asm/softirq.h linux/slab.h linux/cdev.h \
			  linux/hardirq.h linux/cpumask.h linux/kref.h linux/security.h \
			  asm/uaccess.h linux/kthread.h linux/compat.h linux/ioctl32.h \
			  asm/ioctl32.h linux/syscalls.h linux/rwsem.h linux/smp_lock.h \
			  linux/devfs_fs_kernel.h linux/compile.h linux/utsrelease.h], [:], [:], [
#include <linux/compiler.h>
#include <linux/autoconf.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#ifdef HAVE_KINC_LINUX_LOCKS_H
#include <linux/locks.h>
#endif
#ifdef HAVE_KINC_LINUX_SLAB_H
#include <linux/slab.h>
#endif
#include <linux/fs.h>
#include <linux/sched.h>
])
    _LINUX_CHECK_TYPES([irqreturn_t, irq_handler_t, bool, kmem_cache_t *,
			uintptr_t, intptr_t, uchar], [:], [:], [
#include <linux/compiler.h>
#include <linux/autoconf.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#ifdef HAVE_KINC_LINUX_LOCKS_H
#include <linux/locks.h>
#endif
#ifdef HAVE_KINC_LINUX_SLAB_H
#include <linux/slab.h>
#endif
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/wait.h>
#ifdef HAVE_KINC_LINUX_KDEV_T_H
#include <linux/kdev_t.h>
#endif
#ifdef HAVE_KINC_LINUX_STATFS_H
#include <linux/statfs.h>
#endif
#ifdef HAVE_KINC_LINUX_NAMESPACE_H
#include <linux/namespace.h>
#endif
#include <linux/interrupt.h>	/* for irqreturn_t */ 
#ifdef HAVE_KINC_LINUX_HARDIRQ_H
#include <linux/hardirq.h>	/* for in_interrupt */
#endif
#ifdef HAVE_KINC_LINUX_KTHREAD_H
#include <linux/kthread.h>
#endif
#include <linux/time.h>		/* for struct timespec */
])
    AH_TEMPLATE([kmem_cachep_t], [This kmem_cache_t is deprecated in recent
	2.6.20 kernels.  When it is deprecated, define this to struct
	kmem_cache *.])
    if test :"${linux_cv_type_kmem_cache_t_p:-no}" = :no ; then
	AC_DEFINE_UNQUOTED([kmem_cachep_t], [struct kmem_cache *])
    else
	AC_DEFINE_UNQUOTED([kmem_cachep_t], [kmem_cache_t *])
    fi
    _LINUX_KERNEL_ENV([dnl
	AC_CACHE_CHECK([for kernel kmem_cache_create with 5 args],
		       [linux_cv_kmem_cache_create_5_args], [dnl
	    AC_COMPILE_IFELSE([
		AC_LANG_PROGRAM([[
#include <linux/compiler.h>
#include <linux/autoconf.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#ifdef HAVE_KINC_LINUX_LOCKS_H
#include <linux/locks.h>
#endif
#ifdef HAVE_KINC_LINUX_SLAB_H
#include <linux/slab.h>
#endif
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/wait.h>
#ifdef HAVE_KINC_LINUX_KDEV_T_H
#include <linux/kdev_t.h>
#endif
#ifdef HAVE_KINC_LINUX_STATFS_H
#include <linux/statfs.h>
#endif
#ifdef HAVE_KINC_LINUX_NAMESPACE_H
#include <linux/namespace.h>
#endif
#include <linux/interrupt.h>	/* for irqreturn_t */ 
#ifdef HAVE_KINC_LINUX_HARDIRQ_H
#include <linux/hardirq.h>	/* for in_interrupt */
#endif
#ifdef HAVE_KINC_LINUX_KTHREAD_H
#include <linux/kthread.h>
#endif
#include <linux/time.h>		/* for struct timespec */]],
		    [[struct kmem_cache *(*my_autoconf_function_pointer)
		      (const char *, size_t, size_t, unsigned long,
		       void (*)(struct kmem_cache *, void *)) =
		       &kmem_cache_create;]]) ],
		[linux_cv_kmem_cache_create_5_args='yes'],
		[linux_cv_kmem_cache_create_5_args='no'])
	    ])
	if test :$linux_cv_kmem_cache_create_5_args = :yes ; then
	    AC_DEFINE([HAVE_KFUNC_KMEM_CACHE_CREATE_5_ARGS], [1], [Define if
		       function kmem_cache_create takes 4 arguments.])
	fi
	AH_VERBATIM([kmem_create_cache],
[/* silly kernel developers */
#ifdef HAVE_KFUNC_KMEM_CACHE_CREATE_5_ARGS
#define kmem_create_cache(a1,a2,a3,a4,a5,a6) kmem_cache_create(a1,a2,a3,a4,a5)
#else
#define kmem_create_cache(a1,a2,a3,a4,a5,a6) kmem_cache_create(a1,a2,a3,a4,a5,a6)
#endif])dnl
    ])
])# _SIGTRAN_CONFIG_KERNEL
# =============================================================================

# =============================================================================
# _SIGTRAN_OUTPUT
# -----------------------------------------------------------------------------
AC_DEFUN([_SIGTRAN_OUTPUT], [dnl
    _SIGTRAN_CONFIG
    _SIGTRAN_STRCONF
])# _SIGTRAN_OUTPUT
# =============================================================================

# =============================================================================
# _SIGTRAN_CONFIG
# -----------------------------------------------------------------------------
AC_DEFUN([_SIGTRAN_CONFIG], [dnl
    pkg_src=`(cd $srcdir ; /bin/pwd)`
    pkg_bld=`(cd . ; /bin/pwd)`
    sigtran_cv_config="${pkg_bld}/src/include/sys/sigtran/config.h"
    sigtran_cv_includes="${pkg_bld}/include ${pkg_bld}/src/include ${pkg_src}/src/include"
    sigtran_cv_ldadd= # "${pkg_bld}/libsigtran.la"
    sigtran_cv_ldflags= # "-L${pkg_bld}/.libs/"
    sigtran_cv_ldadd32= # "${pkg_bld}/lib32/libsigtran.la"
    sigtran_cv_ldflags32= # "-L${pkg_bld}/lib32/.libs/"
    sigtran_cv_manpath="${pkg_bld}/doc/man"
    sigtran_cv_modversions="${pkg_bld}/include/sys/${PACKAGE}/modversions.h"
    sigtran_cv_modmap="${pkg_bld}/Modules.map"
    sigtran_cv_symver="${pkg_bld}/Module.symvers"
    sigtran_cv_version="${PACKAGE_RPMEPOCH}:${VERSION}-${PACKAGE_RPMRELEASE}"
])# _SIGTRAN_CONFIG
# =============================================================================

# =============================================================================
# _SIGTRAN_STRCONF
# -----------------------------------------------------------------------------
AC_DEFUN([_SIGTRAN_STRCONF], [dnl
    AC_REQUIRE([_LINUX_STREAMS])
    strconf_prefix='sigtran'
    AC_CACHE_CHECK([for sigtran major device number base], [sigtran_cv_majbase], [dnl
	if test ${ss7_cv_majbase-0} -gt 15 ; then
	    ((sigtran_cv_majbase=ss7_cv_majbase-15))
	else
	    sigtran_cv_majbase=165
	    if test ${streams_cv_package:-LfS} = LfS ; then
		if test ${linux_cv_minorbits:-8} -gt 8 ; then
		    ((sigtran_cv_majbase+=2000))
		fi
	    fi
	fi
    ])
    AC_CACHE_CHECK([for sigtran module id base], [sigtran_cv_midbase], [dnl
	if test ${ss7_cv_midbase-0} -gt 0 ; then
	    ((sigtran_cv_midbase=ss7_cv_midbase+10))
	else
	    sigtran_cv_midbase=150
	    if test ${streams_cv_package:-LfS} = LfS ; then
		if test ${linux_cv_minorbits:-8} -gt 8 ; then
		    ((sigtran_cv_midbase+=5000))
		fi
	    fi
	fi
    ])
    _STRCONF
    ((sigtran_cv_majlast=sigtran_cv_majbase+10))
    ((sigtran_cv_midlast=sigtran_cv_midbase+10))
])# _SIGTRAN_STRCONF
# =============================================================================

# =============================================================================
# _SIGTRAN_
# -----------------------------------------------------------------------------
AC_DEFUN([_SIGTRAN_], [dnl
])# _SIGTRAN_
# =============================================================================

# =============================================================================
#
# $Log: acinclude.m4,v $
# Revision 0.9.2.20  2009-02-18 20:18:41  brian
# - updated agents
#
# Revision 0.9.2.19  2008-12-07 10:40:16  brian
# - new stratm package
#
# Revision 0.9.2.18  2008-10-31 06:54:51  brian
# - move config files, better strconf handling
#
# Revision 0.9.2.17  2008-10-30 11:36:17  brian
# - corrections to build
#
# Revision 0.9.2.16  2008-10-27 17:32:58  brian
# - corrections to checks
#
# Revision 0.9.2.15  2008-09-10 03:49:16  brian
# - changes to accomodate FC9, SUSE 11.0 and Ubuntu 8.04
#
# Revision 0.9.2.14  2008-08-11 22:27:19  brian
# - added makefile variables for modules to acinclude
#
# Revision 0.9.2.13  2008-05-03 21:22:27  brian
# - updates for release
#
# Revision 0.9.2.12  2008-04-29 01:52:14  brian
# - updated headers for release
#
# Revision 0.9.2.11  2007/08/14 08:33:17  brian
# - GPLv3 header update
#
# Revision 0.9.2.10  2007/03/25 18:58:32  brian
# - changes to support 2.6.20-1.2307.fc5 kernel
#
# Revision 0.9.2.9  2007/03/05 23:01:39  brian
# - checking in release changes
#
# Revision 0.9.2.8  2007/03/04 23:41:30  brian
# - additional include path
#
# Revision 0.9.2.7  2007/03/04 23:29:56  brian
# - corrected modversions directory
#
# Revision 0.9.2.6  2007/03/04 23:14:20  brian
# - better search for modversions
#
# Revision 0.9.2.5  2007/03/02 10:03:35  brian
# - updates to common build process and versions for all exported symbols
#
# Revision 0.9.2.4  2006/12/29 05:50:56  brian
# - changes for successful master build
#
# Revision 0.9.2.3  2006/12/23 13:06:45  brian
# - manual page and other package updates for release
#
# Revision 0.9.2.2  2006/12/18 08:51:34  brian
# - corections from testing, resolve device numbering
#
# Revision 0.9.2.1  2006/10/16 10:42:40  brian
# - added new package files
#
# =============================================================================
# 
# Copyright (c) 2001-2008  OpenSS7 Corporation <http://www.openss7.com/>
# Copyright (c) 1997-2000  Brian F. G. Bidulock <bidulock@openss7.org>
# 
# =============================================================================
# ENDING OF SEPARATE COPYRIGHT MATERIAL
# =============================================================================
# vim: ft=config sw=4 noet nocin nosi com=b\:#,b\:dnl,b\:***,b\:@%\:@ fo+=tcqlorn
