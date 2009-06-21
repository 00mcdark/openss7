# vim: ft=config sw=4 noet nocin nosi com=b\:#,b\:dnl,b\:***,b\:@%\:@ fo+=tcqlorn
# =============================================================================
# BEGINNING OF SEPARATE COPYRIGHT MATERIAL
# =============================================================================
# 
# @(#) $RCSfile: nsl.m4,v $ $Name:  $($Revision: 1.1.2.1 $) $Date: 2009-06-21 11:06:05 $
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
# Last Modified $Date: 2009-06-21 11:06:05 $ by $Author: brian $
#
# =============================================================================

# -----------------------------------------------------------------------------
# This file provides some common macros for finding a STREAMS NSL
# release and necessary include directories and other configuration for
# compiling kernel modules to run with the STREAMS NSL package.
# -----------------------------------------------------------------------------
# Interesting enough, there is no need to have strnsl loaded on the build
# machine to compile modules.  Only the proper header files are required.
# -----------------------------------------------------------------------------

# =============================================================================
# _NSL
# -----------------------------------------------------------------------------
# Check for the existence of NSL header files, particularly sys/netconfig.h.
# NSL header files are required for building the TPI interface for NSL.
# Without NSL header files, the TPI interface to NSL will not be built.
# -----------------------------------------------------------------------------
AC_DEFUN([_NSL], [dnl
    _NSL_CHECK
    if test :"${nsl_cv_includes:-no}" = :no ; then
	AC_MSG_ERROR([
*** 
*** Configure could not find the STREAMS NSL include directories.  If
*** you wish to use the STREAMS NSL package you will need to specify
*** the location of the STREAMS NSL (strnsl) include directories with
*** the --with-nsl=@<:@DIRECTORY@:>@ option to ./configure and try again.
***
*** Perhaps you just forgot to load the STREAMS NSL package?  The
*** STREAMS strnsl package is available from The OpenSS7 Project
*** download page at http://www.openss7.org/ and comes in a tarball
*** named something like "strnsl-0.9.2.1.tar.gz".
*** ])
    fi
])# _NSL
# =============================================================================

# =============================================================================
# _NSL_CHECK
# -----------------------------------------------------------------------------
AC_DEFUN([_NSL_CHECK], [dnl
    AC_REQUIRE([_LINUX_STREAMS])dnl
    _NSL_OPTIONS
    _NSL_SETUP
dnl
dnl Skip kernel checks if not configuring for the kernel (i.e. no _LINUX_KERNEL)
dnl as we do for netperf.
dnl
    m4_ifdef([_LINUX_KERNEL], [_NSL_KERNEL])
    _NSL_USER
    _NSL_OUTPUT
    AC_SUBST([NSL_CPPFLAGS])dnl
    AC_SUBST([NSL_MODFLAGS])dnl
    AC_SUBST([NSL_LDADD])dnl
    AC_SUBST([NSL_LDADD32])dnl
    AC_SUBST([NSL_LDFLAGS])dnl
    AC_SUBST([NSL_LDFLAGS32])dnl
    AC_SUBST([NSL_MODMAP])dnl
    AC_SUBST([NSL_SYMVER])dnl
    AC_SUBST([NSL_MANPATH])dnl
    AC_SUBST([NSL_VERSION])dnl
])# _NSL_CHECK
# =============================================================================

# =============================================================================
# _NSL_OPTIONS
# -----------------------------------------------------------------------------
# allow the user to specify the header file location
# -----------------------------------------------------------------------------
AC_DEFUN([_NSL_OPTIONS], [dnl
    AC_ARG_WITH([nsl],
	AS_HELP_STRING([--with-nsl=HEADERS],
	    [specify the NSL header file directory.  @<:@default=INCLUDEDIR/strnsl@:>@]),
	[with_nsl="$withval" ; for s in ${!nsl_cv_*} ; do eval "unset $s" ; done],
	[with_nsl=''])
])# _NSL_OPTIONS
# =============================================================================

# =============================================================================
# _NSL_SETUP
# -----------------------------------------------------------------------------
AC_DEFUN([_NSL_SETUP], [dnl
    _NSL_CHECK_HEADERS
    for nsl_include in $nsl_cv_includes ; do
	NSL_CPPFLAGS="${NSL_CPPFLAGS}${NSL_CPPFLAGS:+ }-I${nsl_include}"
    done
    if test :"${nsl_cv_config:-no}" != :no ; then
	NSL_CPPFLAGS="${NSL_CPPFLAGS}${NSL_CPPFLAGS:+ }-include ${nsl_cv_config}"
    fi
    if test :"${nsl_cv_modversions:-no}" != :no ; then
	NSL_MODFLAGS="${NSL_MODFLAGS}${NSL_MODFLAGS:+ }-include ${nsl_cv_modversions}"
    fi
])# _NSL_SETUP
# =============================================================================

# =============================================================================
# _NSL_CHECK_HEADERS
# -----------------------------------------------------------------------------
AC_DEFUN([_NSL_CHECK_HEADERS], [dnl
    # Test for the existence of Linux Fast-STREAMS NSL header files.  The
    # package normally requires NSL header files to compile.
    AC_CACHE_CHECK([for nsl include directory], [nsl_cv_includes], [dnl
	nsl_what="netconfig.h"
	if test :"${with_nsl:-no}" != :no -a :"${with_nsl:-no}" != :yes ; then
	    # First thing to do is to take user specified director(ies)
	    AC_MSG_RESULT([(searching $with_nsl)])
	    for nsl_dir in $with_nsl ; do
		if test -d "$nsl_dir" ; then
		    AC_MSG_CHECKING([for nsl include directory... $nsl_dir])
		    if test -r "$nsl_dir/$nsl_what" ; then
			nsl_cv_includes="$with_nsl"
			AC_MSG_RESULT([yes])
			break
		    fi
		    AC_MSG_RESULT([no])
		fi
	    done
	    if test :"${nsl_cv_includes:-no}" = :no ; then
		AC_MSG_WARN([
***
*** You have specified include directories using:
***
***	    --with-nsl="$with_nsl"
***
*** however, $nsl_what does not exist in any of the specified
*** directories.  Configure will continue to search other known
*** directories.
*** ])
	    fi
	    AC_MSG_CHECKING([for nsl include directory])
	fi
	if test :"${nsl_cv_includes:-no}" = :no ; then
	    # The next place to look is under the master source and build
	    # directory, if any.
	    AC_MSG_RESULT([(searching $os7_cv_master_srcdir $os7_cv_master_builddir)])
	    nsl_bld="${os7_cv_master_builddir:+$os7_cv_master_builddir/strnsl/src/include}"
	    nsl_inc="${os7_cv_master_builddir:+$os7_cv_master_builddir/strnsl/include}"
	    nsl_dir="${os7_cv_master_srcdir:+$os7_cv_master_srcdir/strnsl/src/include}"
	    if test -d "$nsl_dir" ; then
		AC_MSG_CHECKING([for nsl include directory... $nsl_dir $nsl_bld])
		if test -r "$nsl_dir/$nsl_what" ; then
		    nsl_cv_includes="$nsl_inc $nsl_bld $nsl_dir"
		    nsl_cv_ldadd="$os7_cv_master_builddir/strnsl/libxnsl.la"
		    nsl_cv_ldadd32="$os7_cv_master_builddir/strnsl/lib32/libxnsl.la"
		    nsl_cv_modversions= # "$os7_cv_master_builddir/strnsl/include/sys/strnsl/modversions.h"
		    nsl_cv_modmap= # "$os7_cv_master_builddir/strnsl/Modules.map"
		    nsl_cv_symver= # "$os7_cv_master_builddir/strnsl/Module.symvers"
		    nsl_cv_manpath="$os7_cv_master_builddir/strnsl/doc/man"
		    AC_MSG_RESULT([yes])
		else
		    AC_MSG_RESULT([no])
		fi
	    fi
	    AC_MSG_CHECKING([for nsl include directory])
	fi
	if test :"${nsl_cv_includes:-no}" = :no ; then
	    # The next place to look now is for a peer package being built under
	    # the same top directory, and then the higher level directory.
	    nsl_here=`pwd`
	    AC_MSG_RESULT([(searching from $nsl_here)])
	    for nsl_dir in \
		$srcdir/strnsl*/src/include \
		$srcdir/../strnsl*/src/include \
		../_build/$srcdir/../../strnsl*/src/include \
		../_build/$srcdir/../../../strnsl*/src/include
	    do
		if test -d "$nsl_dir" ; then
		    nsl_bld=`echo $nsl_dir | sed -e "s|^$srcdir/|$nsl_here/|;"'s|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
		    nsl_inc=`echo $nsl_bld/../../include |sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
		    nsl_dir=`(cd $nsl_dir; pwd)`
		    AC_MSG_CHECKING([for nsl include directory... $nsl_dir $nsl_bld])
		    if test -d "$nsl_bld" -a -r "$nsl_dir/$nsl_what" ; then
			nsl_cv_includes="$nsl_inc $nsl_bld $nsl_dir"
			nsl_cv_ldadd=`echo "$nsl_bld/../../libxnsl.la" |sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
			nsl_cv_ldadd32=`echo "$nsl_bld/../../lib32/libxnsl.la" |sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
			nsl_cv_modversions= # `echo "$nsl_inc/sys/strnsl/modversions.h" | sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
			nsl_cv_modmap= # `echo "$nsl_bld/../../Modules.map" |sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
			nsl_cv_symver= # `echo "$nsl_bld/../../Module.symvers" |sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
			nsl_cv_manpath=`echo "$nsl_bld/../../doc/man" |sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
			AC_MSG_RESULT([yes])
			break
		    fi
		    AC_MSG_RESULT([no])
		fi
	    done
	    AC_MSG_CHECKING([for nsl include directory])
	fi
	if test :"${nsl_cv_includes:-no}" = :no ; then
	    # NSL header files are normally found in the strnsl package now.
	    # They used to be part of the XNET add-on package.
	    case "$streams_cv_package" in
		(LiS)
		    eval "nsl_search_path=\"
			${DESTDIR}${includedir}/strnsl
			${DESTDIR}${rootdir}${oldincludedir}/strnsl
			${DESTDIR}${rootdir}/usr/include/strnsl
			${DESTDIR}${rootdir}/usr/local/include/strnsl
			${DESTDIR}${rootdir}/usr/src/strnsl/src/include
			${DESTDIR}${includedir}/strxnet
			${DESTDIR}${rootdir}${oldincludedir}/strxnet
			${DESTDIR}${rootdir}/usr/include/strxnet
			${DESTDIR}${rootdir}/usr/local/include/strxnet
			${DESTDIR}${rootdir}/usr/src/strxnet/src/include
			${DESTDIR}${includedir}/strxnet
			${DESTDIR}${rootdir}${oldincludedir}/strxnet
			${DESTDIR}${rootdir}/usr/include/strxnet
			${DESTDIR}${rootdir}/usr/local/include/strxnet
			${DESTDIR}${rootdir}/usr/src/strxnet/src/include
			${DESTDIR}${oldincludedir}/strnsl
			${DESTDIR}/usr/include/strnsl
			${DESTDIR}/usr/local/include/strnsl
			${DESTDIR}/usr/src/strnsl/src/include
			${DESTDIR}${oldincludedir}/strxnet
			${DESTDIR}/usr/include/strxnet
			${DESTDIR}/usr/local/include/strxnet
			${DESTDIR}/usr/src/strxnet/src/include
			${DESTDIR}${oldincludedir}/strxnet
			${DESTDIR}/usr/include/strxnet
			${DESTDIR}/usr/local/include/strxnet
			${DESTDIR}/usr/src/strxnet/src/include\""
		    ;;
		(LfS)
		    eval "nsl_search_path=\"
			${DESTDIR}${includedir}/strnsl
			${DESTDIR}${rootdir}${oldincludedir}/strnsl
			${DESTDIR}${rootdir}/usr/include/strnsl
			${DESTDIR}${rootdir}/usr/local/include/strnsl
			${DESTDIR}${rootdir}/usr/src/strnsl/src/include
			${DESTDIR}${includedir}/strxnet
			${DESTDIR}${rootdir}${oldincludedir}/strxnet
			${DESTDIR}${rootdir}/usr/include/strxnet
			${DESTDIR}${rootdir}/usr/local/include/strxnet
			${DESTDIR}${rootdir}/usr/src/strxnet/include
			${DESTDIR}${includedir}/streams
			${DESTDIR}${rootdir}${oldincludedir}/streams
			${DESTDIR}${rootdir}/usr/include/streams
			${DESTDIR}${rootdir}/usr/local/include/streams
			${DESTDIR}${rootdir}/usr/src/streams/include
			${DESTDIR}${oldincludedir}/strnsl
			${DESTDIR}/usr/include/strnsl
			${DESTDIR}/usr/local/include/strnsl
			${DESTDIR}/usr/src/strnsl/src/include
			${DESTDIR}${oldincludedir}/strxnet
			${DESTDIR}/usr/include/strxnet
			${DESTDIR}/usr/local/include/strxnet
			${DESTDIR}/usr/src/strxnet/include
			${DESTDIR}${oldincludedir}/streams
			${DESTDIR}/usr/include/streams
			${DESTDIR}/usr/local/include/streams
			${DESTDIR}/usr/src/streams/include\""
		    ;;
	    esac
	    nsl_search_path=`echo "$nsl_search_path" | sed -e 's|\<NONE\>||g;s|//|/|g'`
	    nsl_cv_includes=
	    AC_MSG_RESULT([(searching)])
	    for nsl_dir in $nsl_search_path ; do
		if test -d "$nsl_dir" ; then
		    AC_MSG_CHECKING([for nsl include directory... $nsl_dir])
		    if test -r "$nsl_dir/$nsl_what" ; then
			nsl_cv_includes="$nsl_dir"
			#nsl_cv_ldadd=
			#nsl_cv_ldadd32=
			#nsl_cv_modmap=
			#nsl_cv_symver=
			#nsl_cv_manpath=
			AC_MSG_RESULT([yes])
			break
		    fi
		    AC_MSG_RESULT([no])
		fi
	    done
	    AC_MSG_CHECKING([for nsl include directory])
	fi
    ])
    AC_CACHE_CHECK([for nsl ldadd native], [nsl_cv_ldadd], [dnl
	nsl_what="libxnsl.la"
	nsl_cv_ldadd=
	for nsl_dir in $nsl_cv_includes ; do
	    if test -f "$nsl_dir/../../$nsl_what" ; then
		nsl_cv_ldadd=`echo "$nsl_dir/../../$nsl_what" | sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
		break
	    fi
	done
	if test -z "$nsl_cv_ldadd" ; then
	    eval "nsl_search_path=\"
		${DESTDIR}${rootdir}${libdir}
		${DESTDIR}${libdir}\""
	    nsl_search_path=`echo "$nsl_search_path" | sed -e 's|\<NONE\>|'$ac_default_prefix'|g;s|//|/|g'`
	    AC_MSG_RESULT([(searching)])
	    for nsl_dir in $nsl_search_path ; do
		if test -d "$nsl_dir" ; then
		    AC_MSG_CHECKING([for nsl ldadd native... $nsl_dir])
		    if test -r "$nsl_dir/$nsl_what" ; then
			nsl_cv_ldadd="$nsl_dir/$nsl_what"
			nsl_cv_ldflags=
			AC_MSG_RESULT([yes])
			break
		    fi
		    AC_MSG_RESULT([no])
		fi
	    done
	    AC_MSG_CHECKING([for nsl ldadd native])
	fi
    ])
    AC_CACHE_CHECK([for nsl ldflags], [nsl_cv_ldflags], [dnl
	nsl_cv_ldflags=
	if test -z "$nsl_cv_ldadd" ; then
	    nsl_cv_ldflags='-L$(DESTDIR)$(rootdir)$(libdir) -lxnsl'
	else
	    nsl_cv_ldflags="-L$(dirname $nsl_cv_ldadd)/.libs/"
	fi
    ])
    AC_CACHE_CHECK([for nsl ldadd 32-bit], [nsl_cv_ldadd32], [dnl
	nsl_what="libxnsl.la"
	nsl_cv_ldadd32=
	for nsl_dir in $nsl_cv_includes ; do
	    if test -f "$nsl_dir/../../lib32/$nsl_what" ; then
		nsl_cv_ldadd32=`echo "$nsl_dir/../../lib32/$nsl_what" | sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
		break
	    fi
	done
	if test -z "$nsl_cv_ldadd32" ; then
	    eval "nsl_search_path=\"
		${DESTDIR}${rootdir}${lib32dir}
		${DESTDIR}${lib32dir}\""
	    nsl_search_path=`echo "$nsl_search_path" | sed -e 's|\<NONE\>|'$ac_default_prefix'|g;s|//|/|g'`
	    AC_MSG_RESULT([(searching)])
	    for nsl_dir in $nsl_search_path ; do
		if test -d "$nsl_dir" ; then
		    AC_MSG_CHECKING([for nsl ldadd 32-bit... $nsl_dir])
		    if test -r "$nsl_dir/$nsl_what" ; then
			nsl_cv_ldadd32="$nsl_dir/$nsl_what"
			nsl_cv_ldflags32=
			AC_MSG_RESULT([yes])
			break
		    fi
		    AC_MSG_RESULT([no])
		fi
	    done
	    AC_MSG_CHECKING([for nsl ldadd 32-bit])
	fi
    ])
    AC_CACHE_CHECK([for nsl ldflags 32-bit], [nsl_cv_ldflags32], [dnl
	nsl_cv_ldflags32=
	if test -z "$nsl_cv_ldadd32" ; then
	    nsl_cv_ldflags32='-L$(DESTDIR)$(rootdir)$(lib32dir) -lxnsl'
	else
	    nsl_cv_ldflags32="-L$(dirname $nsl_cv_ldadd32)/.libs/"
	fi
    ])
    AC_CACHE_CHECK([for nsl modmap], [nsl_cv_modmap], [dnl
	nsl_cv_modmap=
	for nsl_dir in $nsl_cv_includes ; do
	    if test -f "$nsl_dir/../../Modules.map" ; then
		nsl_cv_modmap=`echo "$nsl_dir/../../Modules.map" | sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
		break
	    fi
	done
    ])
    AC_CACHE_CHECK([for nsl symver], [nsl_cv_symver], [dnl
	nsl_cv_symver=
	for nsl_dir in $nsl_cv_includes ; do
	    if test -f "$nsl_dir/../../Module.symvers" ; then
		nsl_cv_symver=`echo "$nsl_dir/../../Module.symvers" | sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
		break
	    fi
	done
    ])
    AC_CACHE_CHECK([for nsl manpath], [nsl_cv_manpath], [dnl
	nsl_cv_manpath=
	for nsl_dir in $nsl_cv_includes ; do
	    if test -d "$nsl_dir/../../doc/man" ; then
		nsl_cv_manpath=`echo "$nsl_dir/../../doc/man" | sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
		break
	    fi
	done
    ])
    AC_CACHE_CHECK([for nsl version], [nsl_cv_version], [dnl
	nsl_cv_version=
	if test -z "$nsl_cv_version" ; then
	    nsl_what="sys/strnsl/version.h"
	    nsl_file=
	    if test -n "$nsl_cv_includes" ; then
		for nsl_dir in $nsl_cv_includes ; do
		    # old place for version
		    if test -f "$nsl_dir/$nsl_what" ; then
			nsl_file="$nsl_dir/$nsl_what"
			break
		    fi
		    # new place for version
		    if test -n "$linux_cv_k_release" ; then
dnl		    if linux_cv_k_release is not defined (no _LINUX_KERNEL) then
dnl		    this will just not be set
			if test -f "$nsl_dir/$linux_cv_k_release/$target_cpu/$nsl_what" ; then
			    nsl_file="$nsl_dir/$linux_cv_k_release/$target_cpu/$nsl_what"
			    break
			fi
		    fi
		done
	    fi
	    if test :${nsl_file:-no} != :no ; then
		nsl_cv_version=`grep '#define.*\<STRNSL_VERSION\>' $nsl_file 2>/dev/null | sed -e 's|^[^"]*"||;s|".*$||'`
	    fi
	fi
	if test -z "$nsl_cv_version" ; then
	    nsl_epoch=
	    nsl_version=
	    nsl_package=
	    nsl_release=
	    nsl_patchlevel=
	    if test -n "$nsl_cv_includes" ; then
		for nsl_dir in $nsl_cv_includes ; do
		    if test -z "$nsl_epoch" -a -s "$nsl_dir/../.rpmepoch" ; then
			nsl_epoch=`cat $nsl_dir/../.rpmepoch`
		    fi
		    if test -z "$nsl_epoch" -a -s "$nsl_dir/../../.rpmepoch" ; then
			nsl_epoch=`cat $nsl_dir/../../.rpmepoch`
		    fi
		    if test -z "$nsl_version" -a -s "$nsl_dir/../.version" ; then
			nsl_version=`cat $nsl_dir/../.version`
		    fi
		    if test -z "$nsl_version" -a -s "$nsl_dir/../../.version" ; then
			nsl_version=`cat $nsl_dir/../../.version`
		    fi
		    if test -z "$nsl_version" ; then
			if test -z "$nsl_version" -a -s "$nsl_dir/../configure" ; then
			    nsl_version=`grep '^PACKAGE_VERSION=' $nsl_dir/../configure | head -1 | sed -e "s,^[[^']]*',,;s,'.*[$],,"`
			fi
			if test -z "$nsl_version" -a -s "$nsl_dir/../../configure" ; then
			    nsl_version=`grep '^PACKAGE_VERSION=' $nsl_dir/../../configure | head -1 | sed -e "s,^[[^']]*',,;s,'.*[$],,"`
			fi
			if test -z "$nsl_package" -a -s "$nsl_dir/../.pkgrelease" ; then
			    nsl_package=`cat $nsl_dir/../.pkgrelease`
			fi
			if test -z "$nsl_package" -a -s "$nsl_dir/../../.pkgrelease" ; then
			    nsl_package=`cat $nsl_dir/../../.pkgrelease`
			fi
			if test -z "$nsl_patchlevel" -a -s "$nsl_dir/../.pkgpatchlevel" ; then
			    nsl_patchlevel=`cat $nsl_dir/../.pkgpatchlevel`
			fi
			if test -z "$nsl_patchlevel" -a -s "$nsl_dir/../../.pkgpatchlevel" ; then
			    nsl_patchlevel=`cat $nsl_dir/../../.pkgpatchlevel`
			fi
			if test -n "$nsl_version" -a -n "$nsl_package" ; then
			    nsl_version="$nsl_version.$nsl_package${nsl_patchlevel:+.$nsl_patchlevel}"
			else
			    nsl_version=
			fi
		    fi
		    if test -z "$nsl_release" -a -s "$nsl_dir/../.rpmrelease" ; then
			nsl_release=`cat $nsl_dir/../.rpmrelease`
		    fi
		    if test -z "$nsl_release" -a -s "$nsl_dir/../../.rpmrelease" ; then
			nsl_release=`cat $nsl_dir/../../.rpmrelease`
		    fi
		done
	    fi
	    if test -n "$nsl_epoch" -a -n "$nsl_version" -a -n "$nsl_release" ; then
		nsl_cv_version="$nsl_epoch:$nsl_version-$nsl_release"
	    fi
	fi
    ])
    nsl_what="sys/strnsl/config.h"
    AC_CACHE_CHECK([for nsl $nsl_what], [nsl_cv_config], [dnl
	nsl_cv_config=
	if test -n "$nsl_cv_includes" ; then
	    AC_MSG_RESULT([(searching $nsl_cv_includes)])
	    for nsl_dir in $nsl_cv_includes ; do
		# old place for config
		AC_MSG_CHECKING([for nsl $nsl_what... $nsl_dir])
		if test -f "$nsl_dir/$nsl_what" ; then
		    nsl_cv_config="$nsl_dir/$nsl_what"
		    AC_MSG_RESULT([yes])
		    break
		fi
		AC_MSG_RESULT([no])
		# new place for config
		if test -n "$linux_cv_k_release" ; then
dnl		    if linux_cv_k_release is not defined (no _LINUX_KERNEL) then
dnl		    this will just not be set
		    AC_MSG_CHECKING([for nsl $nsl_what... $nsl_dir/$linux_cv_k_release/$target_cpu])
		    if test -f "$nsl_dir/$linux_cv_k_release/$target_cpu/$nsl_what" ; then
			nsl_cv_config="$nsl_dir/$linux_cv_k_release/$target_cpu/$nsl_what"
			AC_MSG_RESULT([yes])
			break
		    fi
		    AC_MSG_RESULT([no])
		fi
	    done
	    AC_MSG_CHECKING([for nsl $nsl_what])
	fi
    ])
    nsl_what="sys/strnsl/modversions.h"
    AC_CACHE_CHECK([for nsl $nsl_what], [nsl_cv_modversions], [dnl
	nsl_cv_modversions=
dnl	if linux_cv_k_ko_modules is not defined (no _LINUX_KERNEL) then we
dnl	assume normal objects
	if test :"${linux_cv_k_ko_modules:-no}" = :no ; then
	    if test -n "$nsl_cv_includes" ; then
		AC_MSG_RESULT([(searching $nsl_cv_includes)])
		for nsl_dir in $nsl_cv_includes ; do
		    # old place for modversions
		    AC_MSG_CHECKING([for nsl $nsl_what... $nsl_dir])
		    if test -f "$nsl_dir/$nsl_what" ; then
			nsl_cv_modversions="$nsl_dir/$nsl_what"
			AC_MSG_RESULT([yes])
			break
		    fi
		    AC_MSG_RESULT([no])
		    # new place for modversions
		    if test -n "$linux_cv_k_release" ; then
dnl			if linux_cv_k_release is not defined (no _LINUX_KERNEL)
dnl			then this will just not be set
			AC_MSG_CHECKING([for nsl $nsl_what... $nsl_dir/$linux_cv_k_release/$target_cpu])
			if test -f "$nsl_dir/$linux_cv_k_release/$target_cpu/$nsl_what" ; then
			    nsl_cv_includes="$nsl_dir/$linux_cv_k_release/$target_cpu $nsl_cv_includes"
			    nsl_cv_modversions="$nsl_dir/$linux_cv_k_release/$target_cpu/$nsl_what"
			    AC_MSG_RESULT([yes])
			    break
			fi
			AC_MSG_RESULT([no])
		    fi
		done
		AC_MSG_CHECKING([for nsl $nsl_what])
	    fi
	fi
    ])
    AC_MSG_CHECKING([for nsl added configure arguments])
dnl Older rpms (particularly those used by SuSE) are too stupid to handle --with
dnl and --without rpmopt syntax, so convert to the equivalent --define syntax.
dnl Also, I don't know that even rpm 4.2 handles --with xxx=yyy properly, so we
dnl use defines.
    if test -z "$with_nsl" ; then
	if test :"${nsl_cv_includes:-no}" = :no ; then
	    PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_with_nsl --with-nsl\""
	    PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--with-nsl'"
	    AC_MSG_RESULT([--with-nsl])
	else
	    PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_nsl --without-nsl\""
	    PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-nsl'"
	    AC_MSG_RESULT([--without-nsl])
	fi
    else
	AC_MSG_RESULT([--with-nsl="$with_nsl"])
    fi
])# _NSL_CHECK_HEADERS
# =============================================================================

# =============================================================================
# _NSL_KERNEL
# -----------------------------------------------------------------------------
AC_DEFUN([_NSL_KERNEL], [dnl
])# _NSL_KERNEL
# =============================================================================

# =============================================================================
# _NSL_OUTPUT
# -----------------------------------------------------------------------------
AC_DEFUN([_NSL_OUTPUT], [dnl
    _NSL_DEFINES
])# _NSL_OUTPUT
# =============================================================================

# =============================================================================
# _NSL_DEFINES
# -----------------------------------------------------------------------------
AC_DEFUN([_NSL_DEFINES], [dnl
    if test :"${nsl_cv_modversions:-no}" != :no ; then
	AC_DEFINE_UNQUOTED([HAVE_SYS_NSL_MODVERSIONS_H], [1], [Define when
	    the STREAMS NSL release supports module versions such as
	    the OpenSS7 autoconf releases.])
    fi
    NSL_CPPFLAGS="${NSL_CPPFLAGS:+ }${NSL_CPPFLAGS}"
    NSL_LDADD="$nsl_cv_ldadd"
    NSL_LDADD32="$nsl_cv_ldadd32"
    NSL_LDFLAGS="$nsl_cv_ldflags"
    NSL_LDFLAGS32="$nsl_cv_ldflags32"
    NSL_MODMAP="$nsl_cv_modmap"
    NSL_SYMVER="$nsl_cv_symver"
    NSL_MANPATH="$nsl_cv_manpath"
    NSL_VERSION="$nsl_cv_version"
    MODPOST_INPUTS="${MODPOST_INPUTS}${NSL_SYMVER:+${MODPOST_INPUTS:+ }${NSL_SYMVER}}"
    AC_DEFINE_UNQUOTED([_XOPEN_SOURCE], [600], [dnl
	Define for SuSv3.  This is necessary for LiS and LfS and strnsl for
	that matter.
    ])
])# _NSL_DEFINES
# =============================================================================

# =============================================================================
# _NSL_USER
# -----------------------------------------------------------------------------
AC_DEFUN([_NSL_USER], [dnl
])# _NSL_USER
# =============================================================================

# =============================================================================
# _NSL_
# -----------------------------------------------------------------------------
AC_DEFUN([_NSL_], [dnl
])# _NSL_
# =============================================================================

# =============================================================================
#
# $Log: nsl.m4,v $
# Revision 1.1.2.1  2009-06-21 11:06:05  brian
# - added files to new distro
#
# Revision 0.9.2.30  2008-10-30 11:36:16  brian
# - corrections to build
#
# Revision 0.9.2.29  2008-10-26 12:17:18  brian
# - update package discovery macros
#
# Revision 0.9.2.28  2008-09-28 19:10:58  brian
# - quotation corrections
#
# Revision 0.9.2.27  2008-09-28 18:42:57  brian
# - corrections
#
# Revision 0.9.2.26  2008-09-28 17:48:29  brian
# - more version number corrections
#
# Revision 0.9.2.25  2008-09-28 16:50:56  brian
# - parsing correction and addition of patchlevel
#
# Revision 0.9.2.24  2008-04-28 09:41:03  brian
# - updated headers for release
#
# Revision 0.9.2.23  2007/08/12 19:05:31  brian
# - rearrange and update headers
#
# Revision 0.9.2.22  2007/03/07 10:12:59  brian
# - more corrections
#
# Revision 0.9.2.21  2007/03/07 09:24:08  brian
# - further corrections
#
# Revision 0.9.2.20  2007/03/07 07:29:21  brian
# - search harder for versions
#
# Revision 0.9.2.19  2007/03/06 23:39:54  brian
# - more corrections
#
# Revision 0.9.2.18  2007/03/06 23:13:57  brian
# - master build correction
#
# Revision 0.9.2.17  2007/03/04 23:26:40  brian
# - corrected modversions directory
#
# Revision 0.9.2.16  2007/03/04 23:14:42  brian
# - better search for modversions
#
# Revision 0.9.2.15  2007/03/03 05:42:46  brian
# - better standalong library detection
#
# Revision 0.9.2.14  2007/03/01 07:17:25  brian
# - updating common build process
#
# Revision 0.9.2.13  2007/03/01 06:38:15  brian
# - updates to common build process
#
# Revision 0.9.2.12  2007/03/01 01:45:15  brian
# - updating build process
#
# Revision 0.9.2.11  2007/03/01 00:10:18  brian
# - update to build process for 2.4 kernels
#
# Revision 0.9.2.10  2007/02/28 11:51:32  brian
# - make sure build directory exists
#
# Revision 0.9.2.9  2007/02/22 08:36:38  brian
# - balance parentheses
#
# Revision 0.9.2.8  2006-12-28 08:32:32  brian
# - use cache names for master src and build directories
#
# Revision 0.9.2.7  2006/09/30 07:29:06  brian
# - corrected warning message
# - corrected variable name in xti.m4
# - added iso.m4 to locate striso package
#
# Revision 0.9.2.6  2006/09/29 10:57:46  brian
# - autoconf does not like multiline cache variables
#
# Revision 0.9.2.5  2006/09/29 03:56:53  brian
# - typos
#
# Revision 0.9.2.4  2006/09/29 03:46:16  brian
# - substitute LDFLAGS32
#
# Revision 0.9.2.3  2006/09/29 03:22:38  brian
# - handle flags better
#
# Revision 0.9.2.2  2006/09/27 05:08:41  brian
# - distinguish LDADD from LDFLAGS
#
# Revision 0.9.2.1  2006/09/25 08:38:20  brian
# - added m4 files for locating NSL and SOCK
#
# Revision 0.9.2.35  2006/09/18 00:33:54  brian
# - added libxnsl library and checks for 32bit compatibility libraries
#
# Revision 0.9.2.34  2006/03/14 09:20:47  brian
# - typo
#
# Revision 0.9.2.33  2006/03/14 09:04:11  brian
# - syntax consistency, advanced search
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
