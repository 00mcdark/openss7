# vim: ft=config sw=4 noet nocin nosi com=b\:#,b\:dnl,b\:***,b\:@%\:@ fo+=tcqlorn
# =============================================================================
# BEGINNING OF SEPARATE COPYRIGHT MATERIAL
# =============================================================================
# 
# @(#) $RCSfile: isdn.m4,v $ $Name: OpenSS7-0_9_2 $($Revision: 0.9.2.23 $) $Date: 2008-10-30 11:36:15 $
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
# Last Modified $Date: 2008-10-30 11:36:15 $ by $Author: brian $
#
# =============================================================================

# -----------------------------------------------------------------------------
# This file provides some common macros for finding a STREAMS ISDN
# release and necessary include directories and other configuration for
# compiling kernel modules to run with the STREAMS ISDN package.
# -----------------------------------------------------------------------------
# Interesting enough, there is no need to have strisdn loaded on the build
# machine to compile modules.  Only the proper header files are required.
# -----------------------------------------------------------------------------

# =============================================================================
# _ISDN
# -----------------------------------------------------------------------------
# Check for the existence of ISDN header files, particularly isdn/sys/isdni.h
# ISDN header files are required for building the ISDN interface.
# Without ISDN header files, the ISDN interface will not be built.
# -----------------------------------------------------------------------------
AC_DEFUN([_ISDN], [dnl
    _ISDN_CHECK
    if test :"${isdn_cv_includes:-no}" = :no ; then
	AC_MSG_ERROR([
*** 
*** Configure could not find the STREAMS ISDN include directories.  If
*** you wish to use the STREAMS ISDN package you will need to specify
*** the location of the STREAMS ISDN (strisdn) include directories with
*** the --with-isdn=@<:@DIRECTORY@:>@ option to ./configure and try again.
***
*** Perhaps you just forgot to load the STREAMS ISDN package?  The
*** STREAMS strisdn package is available from The OpenSS7 Project
*** download page at http://www.openss7.org/ and comes in a tarball
*** named something like "strisdn-0.9.2.4.tar.gz".
*** ])
    fi
])# _ISDN
# =============================================================================

# =============================================================================
# _ISDN_CHECK
# -----------------------------------------------------------------------------
AC_DEFUN([_ISDN_CHECK], [dnl
    AC_REQUIRE([_LINUX_STREAMS])dnl
    _ISDN_OPTIONS
    _ISDN_SETUP
dnl
dnl Skip kernel checks if not configuring for the kernel (i.e. no _LINUX_KERNEL)
dnl as we do for netperf.
dnl
    m4_ifdef([_LINUX_KERNEL], [_ISDN_KERNEL])
    _ISDN_USER
    _ISDN_OUTPUT
    AC_SUBST([ISDN_CPPFLAGS])dnl
    AC_SUBST([ISDN_MODFLAGS])dnl
    AC_SUBST([ISDN_LDADD])dnl
    AC_SUBST([ISDN_LDADD32])dnl
    AC_SUBST([ISDN_LDFLAGS])dnl
    AC_SUBST([ISDN_LDFLAGS32])dnl
    AC_SUBST([ISDN_MODMAP])dnl
    AC_SUBST([ISDN_SYMVER])dnl
    AC_SUBST([ISDN_MANPATH])dnl
    AC_SUBST([ISDN_VERSION])dnl
])# _ISDN_CHECK
# =============================================================================

# =============================================================================
# _ISDN_OPTIONS
# -----------------------------------------------------------------------------
# allow the user to specify the header file location
# -----------------------------------------------------------------------------
AC_DEFUN([_ISDN_OPTIONS], [dnl
    AC_ARG_WITH([isdn],
	AS_HELP_STRING([--with-isdn=HEADERS],
	    [specify the ISDN header file directory.  @<:@default=INCLUDEDIR/strisdn@:>@]),
	[with_isdn="$withval" ; for s in ${!isdn_cv_*} ; do eval "unset $s" ; done],
	[with_isdn=''])
])# _ISDN_OPTIONS
# =============================================================================

# =============================================================================
# _ISDN_SETUP
# -----------------------------------------------------------------------------
AC_DEFUN([_ISDN_SETUP], [dnl
    _ISDN_CHECK_HEADERS
    for isdn_include in $isdn_cv_includes ; do
	ISDN_CPPFLAGS="${ISDN_CPPFLAGS}${ISDN_CPPFLAGS:+ }-I${isdn_include}"
    done
    if test :"${isdn_cv_config:-no}" != :no ; then
	ISDN_CPPFLAGS="${ISDN_CPPFLAGS}${ISDN_CPPFLAGS:+ }-include ${isdn_cv_config}"
    fi
    if test :"${isdn_cv_modversions:-no}" != :no ; then
	ISDN_MODFLAGS="${ISDN_MODFLAGS}${ISDN_MODFLAGS:+ }-include ${isdn_cv_modversions}"
    fi
])# _ISDN_SETUP
# =============================================================================

# =============================================================================
# _ISDN_CHECK_HEADERS
# -----------------------------------------------------------------------------
AC_DEFUN([_ISDN_CHECK_HEADERS], [dnl
    # Test for the existence of Linux Fast-STREAMS ISDN header files.  The
    # package normally requires ISDN header files to compile.
    AC_CACHE_CHECK([for isdn include directory], [isdn_cv_includes], [dnl
	isdn_what="sys/isdni.h"
	if test :"${with_isdn:-no}" != :no -a :"${with_isdn:-no}" != :yes ; then
	    # First thing to do is to take user specified director(ies)
	    AC_MSG_RESULT([(searching $with_isdn)])
	    for isdn_dir in $with_isdn ; do
		if test -d "$isdn_dir" ; then
		    AC_MSG_CHECKING([for isdn include directory... $isdn_dir])
		    if test -r "$isdn_dir/$isdn_what" ; then
			isdn_cv_includes="$with_isdn"
			AC_MSG_RESULT([yes])
			break
		    fi
		    AC_MSG_RESULT([no])
		fi
	    done
	    if test :"${isdn_cv_includes:-no}" = :no ; then
		AC_MSG_WARN([
***
*** You have specified include directories using:
***
***	    --with-isdn="$with_isdn"
***
*** however, $isdn_what does not exist in any of the specified
*** directories.  Configure will continue to search other known
*** directories.
*** ])
	    fi
	    AC_MSG_CHECKING([for isdn include directory])
	fi
	if test :"${isdn_cv_includes:-no}" = :no ; then
	    # The next place to look is under the master source and build
	    # directory, if any.
	    AC_MSG_RESULT([(searching $os7_cv_master_srcdir $os7_cv_master_builddir)])
	    isdn_bld="${os7_cv_master_builddir:+$os7_cv_master_builddir/strisdn/src/include}"
	    isdn_inc="${os7_cv_master_builddir:+$os7_cv_master_builddir/strisdn/include}"
	    isdn_dir="${os7_cv_master_srcdir:+$os7_cv_master_srcdir/strisdn/src/include}"
	    if test -d "$isdn_dir" ; then
		AC_MSG_CHECKING([for isdn include directory... $isdn_dir $isdn_bld])
		if test -r "$isdn_dir/$isdn_what" ; then
		    isdn_cv_includes="$isdn_inc $isdn_bld $isdn_dir"
		    isdn_cv_ldadd= # "$os7_cv_master_builddir/strisdn/libisdn.la"
		    isdn_cv_ldadd32= # "$os7_cv_master_builddir/strisdn/lib32/libisdn.la"
		    isdn_cv_modversions="$os7_cv_master_builddir/strisdn/include/sys/strisdn/modversions.h"
		    isdn_cv_modmap="$os7_cv_master_builddir/strisdn/Modules.map"
		    isdn_cv_symver="$os7_cv_master_builddir/strisdn/Module.symvers"
		    isdn_cv_manpath="$os7_cv_master_builddir/strisdn/doc/man"
		    AC_MSG_RESULT([yes])
		else
		    AC_MSG_RESULT([no])
		fi
	    fi
	    AC_MSG_CHECKING([for isdn include directory])
	fi
	if test :"${isdn_cv_includes:-no}" = :no ; then
	    # The next place to look now is for a peer package being built under
	    # the same top directory, and then the higher level directory.
	    isdn_here=`pwd`
	    AC_MSG_RESULT([(searching from $isdn_here)])
	    for isdn_dir in \
		$srcdir/strisdn*/src/include \
		$srcdir/../strisdn*/src/include \
		../_build/$srcdir/../../strisdn*/src/include \
		../_build/$srcdir/../../../strisdn*/src/include
	    do
		if test -d "$isdn_dir" ; then
		    isdn_bld=`echo $isdn_dir | sed -e "s|^$srcdir/|$isdn_here/|;"'s|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
		    isdn_inc=`echo $isdn_bld/../../include |sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
		    isdn_dir=`(cd $isdn_dir; pwd)`
		    AC_MSG_CHECKING([for isdn include directory... $isdn_dir $isdn_bld])
		    if test -d "$isdn_bld" -a -r "$isdn_dir/$isdn_what" ; then
			isdn_cv_includes="$isdn_inc $isdn_bld $isdn_dir"
			isdn_cv_ldadd= # `echo "$isdn_bld/../../libisdn.la" |sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
			isdn_cv_ldadd32= # `echo "$isdn_bld/../../lib32/libisdn.la" |sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
			isdn_cv_modversions=`echo "$isdn_inc/sys/strisdn/modversions.h" | sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
			isdn_cv_modmap=`echo "$isdn_bld/../../Modules.map" |sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
			isdn_cv_symver=`echo "$isdn_bld/../../Module.symvers" |sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
			isdn_cv_manpath=`echo "$isdn_bld/../../doc/man" |sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
			AC_MSG_RESULT([yes])
			break
		    fi
		    AC_MSG_RESULT([no])
		fi
	    done
	    AC_MSG_CHECKING([for isdn include directory])
	fi
	if test :"${isdn_cv_includes:-no}" = :no ; then
	    # ISDN header files are normally found in the strisdn package.
	    case "$streams_cv_package" in
		(LiS)
		    eval "isdn_search_path=\"
			${DESTDIR}${includedir}/strisdn
			${DESTDIR}${rootdir}${oldincludedir}/strisdn
			${DESTDIR}${rootdir}/usr/include/strisdn
			${DESTDIR}${rootdir}/usr/local/include/strisdn
			${DESTDIR}${rootdir}/usr/src/strisdn/src/include
			${DESTDIR}${oldincludedir}/strisdn
			${DESTDIR}/usr/include/strisdn
			${DESTDIR}/usr/local/include/strisdn
			${DESTDIR}/usr/src/strisdn/src/include\""
		    ;;
		(LfS)
		    eval "isdn_search_path=\"
			${DESTDIR}${includedir}/strisdn
			${DESTDIR}${rootdir}${oldincludedir}/strisdn
			${DESTDIR}${rootdir}/usr/include/strisdn
			${DESTDIR}${rootdir}/usr/local/include/strisdn
			${DESTDIR}${rootdir}/usr/src/strisdn/src/include
			${DESTDIR}${includedir}/streams
			${DESTDIR}${rootdir}${oldincludedir}/streams
			${DESTDIR}${rootdir}/usr/include/streams
			${DESTDIR}${rootdir}/usr/local/include/streams
			${DESTDIR}${rootdir}/usr/src/streams/include
			${DESTDIR}${oldincludedir}/strisdn
			${DESTDIR}/usr/include/strisdn
			${DESTDIR}/usr/local/include/strisdn
			${DESTDIR}/usr/src/strisdn/src/include
			${DESTDIR}${oldincludedir}/streams
			${DESTDIR}/usr/include/streams
			${DESTDIR}/usr/local/include/streams
			${DESTDIR}/usr/src/streams/include\""
		    ;;
	    esac
	    isdn_search_path=`echo "$isdn_search_path" | sed -e 's|\<NONE\>||g;s|//|/|g'`
	    isdn_cv_includes=
	    AC_MSG_RESULT([(searching)])
	    for isdn_dir in $isdn_search_path ; do
		if test -d "$isdn_dir" ; then
		    AC_MSG_CHECKING([for isdn include directory... $isdn_dir])
		    if test -r "$isdn_dir/$isdn_what" ; then
			isdn_cv_includes="$isdn_dir"
			#isdn_cv_ldadd=
			#isdn_cv_ldadd32=
			#isdn_cv_modmap=
			#isdn_cv_symver=
			#isdn_cv_manpath=
			AC_MSG_RESULT([yes])
			break
		    fi
		    AC_MSG_RESULT([no])
		fi
	    done
	    AC_MSG_CHECKING([for isdn include directory])
	fi
    ])
    AC_CACHE_CHECK([for isdn ldadd native], [isdn_cv_ldadd], [dnl
	isdn_what="libisdn.la"
	isdn_cv_ldadd=
	for isdn_dir in $isdn_cv_includes ; do
	    if test -f "$isdn_dir/../../$isdn_what" ; then
		isdn_cv_ldadd=`echo "$isdn_dir/../../$isdn_what" | sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
		break
	    fi
	done
	if test -z "$isdn_cv_ldadd" ; then
	    eval "isdn_search_path=\"
		${DESTDIR}${rootdir}${libdir}
		${DESTDIR}${libdir}\""
	    isdn_search_path=`echo "$isdn_search_path" | sed -e 's|\<NONE\>|'$ac_default_prefix'|g;s|//|/|g'`
	    AC_MSG_RESULT([(searching)])
	    for isdn_dir in $isdn_search_path ; do
		if test -d "$isdn_dir" ; then
		    AC_MSG_CHECKING([for isdn ldadd native... $isdn_dir])
		    if test -r "$isdn_dir/$isdn_what" ; then
			isdn_cv_ldadd="$isdn_dir/$isdn_what"
			isdn_cv_ldflags=
			AC_MSG_RESULT([yes])
			break
		    fi
		    AC_MSG_RESULT([no])
		fi
	    done
	    AC_MSG_CHECKING([for isdn ldadd native])
	fi
    ])
    AC_CACHE_CHECK([for isdn ldflags], [isdn_cv_ldflags], [dnl
	isdn_cv_ldflags=
	if test -z "$isdn_cv_ldadd" ; then
	    isdn_cv_ldflags= # '-L$(DESTDIR)$(rootdir)$(libdir) -lisdn'
	else
	    isdn_cv_ldflags= # "-L$(dirname $isdn_cv_ldadd)/.libs/"
	fi
    ])
    AC_CACHE_CHECK([for isdn ldadd 32-bit], [isdn_cv_ldadd32], [dnl
	isdn_what="libisdn.la"
	isdn_cv_ldadd32=
	for isdn_dir in $isdn_cv_includes ; do
	    if test -f "$isdn_dir/../../lib32/$isdn_what" ; then
		isdn_cv_ldadd32=`echo "$isdn_dir/../../lib32/$isdn_what" | sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
		break
	    fi
	done
	if test -z "$isdn_cv_ldadd32" ; then
	    eval "isdn_search_path=\"
		${DESTDIR}${rootdir}${lib32dir}
		${DESTDIR}${lib32dir}\""
	    isdn_search_path=`echo "$isdn_search_path" | sed -e 's|\<NONE\>|'$ac_default_prefix'|g;s|//|/|g'`
	    AC_MSG_RESULT([(searching)])
	    for isdn_dir in $isdn_search_path ; do
		if test -d "$isdn_dir" ; then
		    AC_MSG_CHECKING([for isdn ldadd 32-bit... $isdn_dir])
		    if test -r "$isdn_dir/$isdn_what" ; then
			isdn_cv_ldadd32="$isdn_dir/$isdn_what"
			isdn_cv_ldflags32=
			AC_MSG_RESULT([yes])
			break
		    fi
		    AC_MSG_RESULT([no])
		fi
	    done
	    AC_MSG_CHECKING([for isdn ldadd 32-bit])
	fi
    ])
    AC_CACHE_CHECK([for isdn ldflags 32-bit], [isdn_cv_ldflags32], [dnl
	isdn_cv_ldflags32=
	if test -z "$isdn_cv_ldadd32" ; then
	    isdn_cv_ldflags32= # '-L$(DESTDIR)$(rootdir)$(lib32dir) -lisdn'
	else
	    isdn_cv_ldflags32= # "-L$(dirname $isdn_cv_ldadd32)/.libs/"
	fi
    ])
    AC_CACHE_CHECK([for isdn modmap], [isdn_cv_modmap], [dnl
	isdn_cv_modmap=
	for isdn_dir in $isdn_cv_includes ; do
	    if test -f "$isdn_dir/../../Modules.map" ; then
		isdn_cv_modmap=`echo "$isdn_dir/../../Modules.map" | sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
		break
	    fi
	done
    ])
    AC_CACHE_CHECK([for isdn symver], [isdn_cv_symver], [dnl
	isdn_cv_symver=
	for isdn_dir in $isdn_cv_includes ; do
	    if test -f "$isdn_dir/../../Module.symvers" ; then
		isdn_cv_symver=`echo "$isdn_dir/../../Module.symvers" | sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
		break
	    fi
	done
    ])
    AC_CACHE_CHECK([for isdn manpath], [isdn_cv_manpath], [dnl
	isdn_cv_manpath=
	for isdn_dir in $isdn_cv_includes ; do
	    if test -d "$isdn_dir/../../doc/man" ; then
		isdn_cv_manpath=`echo "$isdn_dir/../../doc/man" | sed -e 's|/[[^/]][[^/]]*/\.\./|/|g;s|/[[^/]][[^/]]*/\.\./|/|g;s|/\./|/|g;s|//|/|g'`
		break
	    fi
	done
    ])
    AC_CACHE_CHECK([for isdn version], [isdn_cv_version], [dnl
	isdn_cv_version=
	if test -z "$isdn_cv_version" ; then
	    isdn_what="sys/strisdn/version.h"
	    isdn_file=
	    if test -n "$isdn_cv_includes" ; then
		for isdn_dir in $isdn_cv_includes ; do
		    # old place for version
		    if test -f "$isdn_dir/$isdn_what" ; then
			isdn_file="$isdn_dir/$isdn_what"
			break
		    fi
		    # new place for version
		    if test -n "$linux_cv_k_release" ; then
dnl		    if linux_cv_k_release is not defined (no _LINUX_KERNEL) then
dnl		    this will just not be set
			if test -f "$isdn_dir/$linux_cv_k_release/$target_cpu/$isdn_what" ; then
			    isdn_file="$isdn_dir/$linux_cv_k_release/$target_cpu/$isdn_what"
			    break
			fi
		    fi
		done
	    fi
	    if test :${isdn_file:-no} != :no ; then
		isdn_cv_version=`grep '#define.*\<STRISDN_VERSION\>' $isdn_file 2>/dev/null | sed -e 's|^[^"]*"||;s|".*$||'`
	    fi
	fi
	if test -z "$isdn_cv_version" ; then
	    isdn_epoch=
	    isdn_version=
	    isdn_package=
	    isdn_release=
	    isdn_patchlevel=
	    if test -n "$isdn_cv_includes" ; then
		for isdn_dir in $isdn_cv_includes ; do
		    if test -z "$isdn_epoch" -a -s "$isdn_dir/../.rpmepoch" ; then
			isdn_epoch=`cat $isdn_dir/../.rpmepoch`
		    fi
		    if test -z "$isdn_epoch" -a -s "$isdn_dir/../../.rpmepoch" ; then
			isdn_epoch=`cat $isdn_dir/../../.rpmepoch`
		    fi
		    if test -z "$isdn_version" -a -s "$isdn_dir/../.version" ; then
			isdn_version=`cat $isdn_dir/../.version`
		    fi
		    if test -z "$isdn_version" -a -s "$isdn_dir/../../.version" ; then
			isdn_version=`cat $isdn_dir/../../.version`
		    fi
		    if test -z "$isdn_version" ; then
			if test -z "$isdn_version" -a -s "$isdn_dir/../configure" ; then
			    isdn_version=`grep '^PACKAGE_VERSION=' $isdn_dir/../configure | head -1 | sed -e "s,^[[^']]*',,;s,'.*[$],,"`
			fi
			if test -z "$isdn_version" -a -s "$isdn_dir/../../configure" ; then
			    isdn_version=`grep '^PACKAGE_VERSION=' $isdn_dir/../../configure | head -1 | sed -e "s,^[[^']]*',,;s,'.*[$],,"`
			fi
			if test -z "$isdn_package" -a -s "$isdn_dir/../.pkgrelease" ; then
			    isdn_package=`cat $isdn_dir/../.pkgrelease`
			fi
			if test -z "$isdn_package" -a -s "$isdn_dir/../../.pkgrelease" ; then
			    isdn_package=`cat $isdn_dir/../../.pkgrelease`
			fi
			if test -z "$isdn_patchlevel" -a -s "$isdn_dir/../.pkgpatchlevel" ; then
			    isdn_patchlevel=`cat $isdn_dir/../.pkgpatchlevel`
			fi
			if test -z "$isdn_patchlevel" -a -s "$isdn_dir/../../.pkgpatchlevel" ; then
			    isdn_patchlevel=`cat $isdn_dir/../../.pkgpatchlevel`
			fi
			if test -n "$isdn_version" -a -n "$isdn_package" ; then
			    isdn_version="$isdn_version.$isdn_package${isdn_patchlevel:+.$isdn_patchlevel}"
			else
			    isdn_version=
			fi
		    fi
		    if test -z "$isdn_release" -a -s "$isdn_dir/../.rpmrelease" ; then
			isdn_release=`cat $isdn_dir/../.rpmrelease`
		    fi
		    if test -z "$isdn_release" -a -s "$isdn_dir/../../.rpmrelease" ; then
			isdn_release=`cat $isdn_dir/../../.rpmrelease`
		    fi
		done
	    fi
	    if test -n "$isdn_epoch" -a -n "$isdn_version" -a -n "$isdn_release" ; then
		isdn_cv_version="$isdn_epoch:$isdn_version-$isdn_release"
	    fi
	fi
    ])
    isdn_what="sys/strisdn/config.h"
    AC_CACHE_CHECK([for isdn $isdn_what], [isdn_cv_config], [dnl
	isdn_cv_config=
	if test -n "$isdn_cv_includes" ; then
	    AC_MSG_RESULT([(searching $isdn_cv_includes)])
	    for isdn_dir in $isdn_cv_includes ; do
		# old place for config
		AC_MSG_CHECKING([for isdn $isdn_what... $isdn_dir])
		if test -f "$isdn_dir/$isdn_what" ; then
		    isdn_cv_config="$isdn_dir/$isdn_what"
		    AC_MSG_RESULT([yes])
		    break
		fi
		AC_MSG_RESULT([no])
		# new place for config
		if test -n "$linux_cv_k_release" ; then
dnl		    if linux_cv_k_release is not defined (no _LINUX_KERNEL) then
dnl		    this will just not be set
		    AC_MSG_CHECKING([for isdn $isdn_what... $isdn_dir/$linux_cv_k_release/$target_cpu])
		    if test -f "$isdn_dir/$linux_cv_k_release/$target_cpu/$isdn_what" ; then
			isdn_cv_config="$isdn_dir/$linux_cv_k_release/$target_cpu/$isdn_what"
			AC_MSG_RESULT([yes])
			break
		    fi
		    AC_MSG_RESULT([no])
		fi
	    done
	    AC_MSG_CHECKING([for isdn $isdn_what])
	fi
    ])
    isdn_what="sys/strisdn/modversions.h"
    AC_CACHE_CHECK([for isdn $isdn_what], [isdn_cv_modversions], [dnl
	isdn_cv_modversions=
dnl	if linux_cv_k_ko_modules is not defined (no _LINUX_KERNEL) then we
dnl	assume normal objects
	if test :"${linux_cv_k_ko_modules:-no}" = :no ; then
	    if test -n "$isdn_cv_includes" ; then
		AC_MSG_RESULT([(searching $isdn_cv_includes)])
		for isdn_dir in $isdn_cv_includes ; do
		    # old place for modversions
		    AC_MSG_CHECKING([for isdn $isdn_what... $isdn_dir])
		    if test -f "$isdn_dir/$isdn_what" ; then
			isdn_cv_modversions="$isdn_dir/$isdn_what"
			AC_MSG_RESULT([yes])
			break
		    fi
		    AC_MSG_RESULT([no])
		    # new place for modversions
		    if test -n "$linux_cv_k_release" ; then
dnl			if linux_cv_k_release is not defined (no _LINUX_KERNEL)
dnl			then this will just not be set
			AC_MSG_CHECKING([for isdn $isdn_what... $isdn_dir/$linux_cv_k_release/$target_cpu])
			if test -f "$isdn_dir/$linux_cv_k_release/$target_cpu/$isdn_what" ; then
			    isdn_cv_includes="$isdn_dir/$linux_cv_k_release/$target_cpu $isdn_cv_includes"
			    isdn_cv_modversions="$isdn_dir/$linux_cv_k_release/$target_cpu/$isdn_what"
			    AC_MSG_RESULT([yes])
			    break
			fi
			AC_MSG_RESULT([no])
		    fi
		done
		AC_MSG_CHECKING([for isdn $isdn_what])
	    fi
	fi
    ])
    AC_MSG_CHECKING([for isdn added configure arguments])
dnl Older rpms (particularly those used by SuSE) are too stupid to handle --with
dnl and --without rpmopt syntax, so convert to the equivalent --define syntax.
dnl Also, I don't know that even rpm 4.2 handles --with xxx=yyy properly, so we
dnl use defines.
    if test -z "$with_isdn" ; then
	if test :"${isdn_cv_includes:-no}" = :no ; then
	    PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_with_isdn --with-isdn\""
	    PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--with-isdn'"
	    AC_MSG_RESULT([--with-isdn])
	else
	    PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_isdn --without-isdn\""
	    PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-isdn'"
	    AC_MSG_RESULT([--without-isdn])
	fi
    else
	AC_MSG_RESULT([--with-isdn="$with_isdn"])
    fi
])# _ISDN_CHECK_HEADERS
# =============================================================================

# =============================================================================
# _ISDN_KERNEL
# -----------------------------------------------------------------------------
AC_DEFUN([_ISDN_KERNEL], [dnl
])# _ISDN_KERNEL
# =============================================================================

# =============================================================================
# _ISDN_OUTPUT
# -----------------------------------------------------------------------------
AC_DEFUN([_ISDN_OUTPUT], [dnl
    _ISDN_DEFINES
])# _ISDN_OUTPUT
# =============================================================================

# =============================================================================
# _ISDN_DEFINES
# -----------------------------------------------------------------------------
AC_DEFUN([_ISDN_DEFINES], [dnl
    if test :"${isdn_cv_modversions:-no}" != :no ; then
	AC_DEFINE_UNQUOTED([HAVE_SYS_ISDN_MODVERSIONS_H], [1], [Define when
	    the STREAMS ISDN release supports module versions such as
	    the OpenSS7 autoconf releases.])
    fi
    ISDN_CPPFLAGS="${ISDN_CPPFLAGS:+ }${ISDN_CPPFLAGS}"
    ISDN_LDADD="$isdn_cv_ldadd"
    ISDN_LDADD32="$isdn_cv_ldadd32"
    ISDN_LDFLAGS="$isdn_cv_ldflags"
    ISDN_LDFLAGS32="$isdn_cv_ldflags32"
    ISDN_MODMAP="$isdn_cv_modmap"
    ISDN_SYMVER="$isdn_cv_symver"
    ISDN_MANPATH="$isdn_cv_manpath"
    ISDN_VERSION="$isdn_cv_version"
    MODPOST_INPUTS="${MODPOST_INPUTS}${ISDN_SYMVER:+${MODPOST_INPUTS:+ }${ISDN_SYMVER}}"
    AC_DEFINE_UNQUOTED([_XOPEN_SOURCE], [600], [dnl
	Define for SuSv3.  This is necessary for LiS and LfS and strisdn for
	that matter.
    ])
])# _ISDN_DEFINES
# =============================================================================

# =============================================================================
# _ISDN_USER
# -----------------------------------------------------------------------------
AC_DEFUN([_ISDN_USER], [dnl
])# _ISDN_USER
# =============================================================================

# =============================================================================
# _ISDN_
# -----------------------------------------------------------------------------
AC_DEFUN([_ISDN_], [dnl
])# _ISDN_
# =============================================================================

# =============================================================================
#
# $Log: isdn.m4,v $
# Revision 0.9.2.23  2008-10-30 11:36:15  brian
# - corrections to build
#
# Revision 0.9.2.22  2008-10-26 12:17:18  brian
# - update package discovery macros
#
# Revision 0.9.2.21  2008-09-28 19:10:58  brian
# - quotation corrections
#
# Revision 0.9.2.20  2008-09-28 18:42:57  brian
# - corrections
#
# Revision 0.9.2.19  2008-09-28 17:48:29  brian
# - more version number corrections
#
# Revision 0.9.2.18  2008-09-28 16:50:56  brian
# - parsing correction and addition of patchlevel
#
# Revision 0.9.2.17  2008-04-28 09:41:03  brian
# - updated headers for release
#
# Revision 0.9.2.16  2007/08/12 19:05:30  brian
# - rearrange and update headers
#
# Revision 0.9.2.15  2007/03/07 10:12:59  brian
# - more corrections
#
# Revision 0.9.2.14  2007/03/07 09:24:08  brian
# - further corrections
#
# Revision 0.9.2.13  2007/03/07 07:29:21  brian
# - search harder for versions
#
# Revision 0.9.2.12  2007/03/06 23:39:54  brian
# - more corrections
#
# Revision 0.9.2.11  2007/03/06 23:13:56  brian
# - master build correction
#
# Revision 0.9.2.10  2007/03/04 23:26:40  brian
# - corrected modversions directory
#
# Revision 0.9.2.9  2007/03/04 23:14:42  brian
# - better search for modversions
#
# Revision 0.9.2.8  2007/03/01 07:17:25  brian
# - updating common build process
#
# Revision 0.9.2.7  2007/03/01 06:38:15  brian
# - updates to common build process
#
# Revision 0.9.2.6  2007/03/01 01:45:15  brian
# - updating build process
#
# Revision 0.9.2.5  2007/03/01 00:10:18  brian
# - update to build process for 2.4 kernels
#
# Revision 0.9.2.4  2007/02/28 11:51:31  brian
# - make sure build directory exists
#
# Revision 0.9.2.3  2006/12/28 08:32:31  brian
# - use cache names for master src and build directories
#
# Revision 0.9.2.2  2006/10/16 11:44:46  brian
# - change search file
#
# Revision 0.9.2.1  2006/10/16 08:28:02  brian
# - added new package discovery macros
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
