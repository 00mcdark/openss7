# vim: ft=config sw=4 noet nocin nosi com=b\:#,b\:dnl,b\:***,b\:@%\:@ fo+=tcqlorn
# =============================================================================
# BEGINNING OF SEPARATE COPYRIGHT MATERIAL
# =============================================================================
# 
# @(#) $RCSfile: doxy.m4,v $ $Name:  $($Revision: 1.1.2.4 $) $Date: 2009-07-21 11:06:13 $
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
# Last Modified $Date: 2009-07-21 11:06:13 $ by $Author: brian $
#
# =============================================================================

# =============================================================================
# _DOXY_OPTIONS
# -----------------------------------------------------------------------------
# We use doxygen to document user files and kerneldoc to doucment kernel
# files.  Therefore, USER_CPPFLAGS needs to already be defined at this point
# so that we can strip out the include paths.
# -----------------------------------------------------------------------------
AC_DEFUN([_DOXY_OPTIONS], [dnl
    _DOXY_CONFIG_STRIP_FROM_INC_PATH
    _DOXY_CONFIG_INPUT
])# _DOXY_OPTIONS
# =============================================================================

# =============================================================================
# _DOXY_CONFIG_STRIP_FROM_INC_PATH
# -----------------------------------------------------------------------------
# We use doxygen to document user files and kerneldoc to doucment kernel
# files.  Therefore, USER_CPPFLAGS and PKG_INCLUDES needs to already be
# defined at this point so that we can strip out the include paths.
# -----------------------------------------------------------------------------
AC_DEFUN([_DOXY_CONFIG_STRIP_FROM_INC_PATH], [dnl
    top_builddir=`(cd . ; pwd)`
    top_srcdir=`(cd $srcdir ; pwd)`
    for doxy_tmp in $USER_CPPFLAGS $PKG_INCLUDES
    do
	doxy_incs=
	case $doxy_tmp in
	    (-I*)
	    doxy_tmp=`echo $doxy_tmp | sed -e 's|^-I||'`
	    doxy_tmp=`eval "echo \"$doxy_tmp\""`
	    AC_MSG_CHECKING([for doxy include $doxy_tmp])
	    case $doxy_tmp in
		(/*) ;;
		(*)
		    if test -d $doxy_tmp ; then
			doxy_tmp=`(cd $doxy_tmp ; pwd)`
		    else
			AC_MSG_RESULT([no])
			continue
		    fi
		    ;;
	    esac
	    doxy_incs="${doxy_incs:+$doxy_incs }${doxy_tmp}"
	    AC_MSG_RESULT([yes])
	    ;;
	esac
    done
    DOXY_CONFIG_STRIP_FROM_INC_PATH="$doxy_incs"
    AC_SUBST([DOXY_CONFIG_STRIP_FROM_INC_PATH])dnl
    AC_SUBST([doxy_incs])dnl
])# _DOXY_CONFIG_STRIP_FROM_INC_PATH
# =============================================================================

# =============================================================================
# _DOXY_CONFIG_INPUT
# -----------------------------------------------------------------------------
AC_DEFUN([_DOXY_CONFIG_INPUT], [dnl
    doxy_abs_builddir=`(cd . ; pwd)`
    doxy_abs_srcdir=`(cd $srcdir ; pwd)`
    doxy_tmp=`find $doxy_abs_builddir $doxy_abs_srcdir -name '*.h' -o -name '*.c' | xargs grep -l 'doxygen(1)' | sort -u`
    AC_MSG_CHECKING([for doxy input files])
    doxy_inputs=
    doxy_num=0
    for doxy_file in $doxy_tmp ; do
dnl	AC_MSG_CHECKING([for doxy input $doxy_file])
	doxy_inputs="${doxy_inputs:+$doxy_inputs }${doxy_file}"
	((doxy_num++))
dnl	AC_MSG_RESULT([yes])
    done
    doxy_inputs="AC_PACKAGE_NAME.dox${doxy_inputs:+ $doxy_inputs}"
    DOXY_CONFIG_INPUT="$doxy_inputs"
    AC_MSG_RESULT([$doxy_num files])
    AC_SUBST([DOXY_CONFIG_INPUT])dnl
    AC_SUBST([doxy_abs_builddir])dnl
    AC_SUBST([doxy_abs_srcdir])dnl
    AC_SUBST([doxy_inputs])dnl
])# _DOXY_CONFIG_INPUT
# =============================================================================

# =============================================================================
# _DOXY_SETUP
# -----------------------------------------------------------------------------
AC_DEFUN([_DOXY_SETUP], [dnl
    AC_ARG_VAR([DOXYGEN],
	       [doxygen command. @<:@default=doxygen@:>@])
    AC_PATH_PROG([DOXYGEN], [doxygen], [],
		 [$PATH:/usr/local/bin:/usr/bin])
    if test :"${DOXYGEN:-no}" = :no ; then
	DOXYGEN="${am_missing2_run}doxygen"
	AC_MSG_WARN([Could not find doxygen program in PATH.])
    fi
    AM_CONDITIONAL([HAVE_DOXYGEN], [test ":${ac_cv_path_DOXYGEN:-no}" != :no])dnl
])# _DOXY_SETUP
# =============================================================================

# =============================================================================
# _DOXY_OUTPUT_CONFIG_COMMANDS
# -----------------------------------------------------------------------------
AC_DEFUN([_DOXY_OUTPUT_CONFIG_COMMANDS], [dnl
    ${DOXYGEN:-doxygen} scripts/Doxyfile
])# _DOXY_OUTPUT_CONFIG_COMMANDS
# =============================================================================

# =============================================================================
# _DOXY_OUTPUT
# -----------------------------------------------------------------------------
AC_DEFUN([_DOXY_OUTPUT], [dnl
    AC_CONFIG_FILES([scripts/Doxyfile] AC_PACKAGE_NAME.dox)
    if test -d $srcdir/doc ; then
	doxydir="$srcdir/doc"
    else
	doxydir="$srcdir"
    fi
    AC_SUBST([doxydir])
dnl    AC_CONFIG_COMMANDS([doxygen],
dnl	[_DOXY_OUTPUT_CONFIG_COMMANDS], [dnl
dnldoxydir="doc"
dnlDOXYGEN="$DOXYGEN"
dnl    ])
])# _DOXY_OUTPUT
# =============================================================================

# =============================================================================
# _DOXY
# -----------------------------------------------------------------------------
AC_DEFUN([_DOXY], [dnl
    _DOXY_OPTIONS
    _DOXY_SETUP
    _DOXY_OUTPUT
])# _DOXY
# =============================================================================

# =============================================================================
# _DOXY_
# -----------------------------------------------------------------------------
AC_DEFUN([_DOXY_], [dnl
])# _DOXY_
# =============================================================================


# =============================================================================
#
# $Log: doxy.m4,v $
# Revision 1.1.2.4  2009-07-21 11:06:13  brian
# - changes from release build
#
# Revision 1.1.2.3  2009-07-04 03:51:33  brian
# - updates for release
#
# Revision 1.1.2.2  2009-06-29 07:35:38  brian
# - improvements to build process
#
# Revision 1.1.2.1  2009-06-21 11:06:04  brian
# - added files to new distro
#
# Revision 0.9.2.19  2008-10-27 12:23:34  brian
# - suppress warning on each iteration and cache results
#
# Revision 0.9.2.18  2008/09/21 07:40:45  brian
# - add defaults to environment variables
#
# Revision 0.9.2.17  2008-04-28 09:41:03  brian
# - updated headers for release
#
# Revision 0.9.2.16  2007/10/17 20:00:27  brian
# - slightly different path checks
#
# Revision 0.9.2.15  2007/08/12 19:05:30  brian
# - rearrange and update headers
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
