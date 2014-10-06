# vim: ft=config sw=4 noet nocin nosi com=b\:#,b\:dnl,b\:***,b\:@%\:@ fo+=tcqlorn
# =============================================================================
# BEGINNING OF SEPARATE COPYRIGHT MATERIAL
# =============================================================================
# 
# @(#) $RCSfile$ $Name$($Revision$) $Date$
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
# Last Modified $Date$ by $Author$
#
# =============================================================================

dnl DAST_CHECK_BOOL
dnl Check for bool support. Defines bool, true, and false.

AH_TEMPLATE([HAVE_POSIX_THREAD], [])
AH_TEMPLATE([_REENTRANT], [])
AH_TEMPLATE([ssize_t], [Define to "int" if <sys/types.h> does not define.])
AH_TEMPLATE([bool])
AH_TEMPLATE([true])
AH_TEMPLATE([false])

AC_DEFUN([DAST_CHECK_BOOL], [dnl
    AC_CHECK_SIZEOF([bool])
    if test "$ac_cv_sizeof_bool" = 0 ; then
        AC_DEFINE([bool], [int])
    fi
    AC_CACHE_CHECK([if true is defined], [ac_cv_have_true], [dnl
        AC_LANG_PUSH([C++])
        AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM(
                [[]],
                [[unsigned int i = true]])],
            [ac_cv_have_true=yes],
            [ac_cv_have_true=no])
        AC_LANG_POP([])
    ])
    if test "$ac_cv_have_true" != yes ; then
        AC_DEFINE([true],  [1])
        AC_DEFINE([false], [0])
    fi
])

dnl ===================================================================
dnl DAST_REPLACE_TYPE( type, sizeof )
dnl Check for the type as AC_CHECK_TYPE does. Define HAVE_<type>
dnl if type exists; don't define <type> to anything if it doesn't exist.
dnl Useful if there is no well-defined default type, such as int32_t

AC_DEFUN([DAST_REPLACE_TYPE], [dnl
    AC_CACHE_CHECK([for $1], [ac_cv_type_$1], [dnl
        AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM(
                [[$ac_includes_default]],
                [[$1 foo]])],
            [ac_cv_type_$1=yes],
            [ac_cv_type_$1=no])
        if test $ac_cv_type_$1 != yes ; then
            if test "$ac_cv_sizeof_char" = $2; then
                ac_cv_type_$1="char"
            elif test "$ac_cv_sizeof_short" = $2; then
                ac_cv_type_$1="short"
            elif test "$ac_cv_sizeof_int" = $2; then
                ac_cv_type_$1="int"
            elif test "$ac_cv_sizeof_long" = $2; then
                ac_cv_type_$1="long"
            elif test "$ac_cv_sizeof_long_long" = $2; then
                ac_cv_type_$1="long long"
            fi
        fi
    ])
    if test "$ac_cv_type_$1" != no; then
        if test "$ac_cv_type_$1" != yes; then
            AC_DEFINE_UNQUOTED([$1], [$ac_cv_type_$1])
        fi
        AC_DEFINE_UNQUOTED([HAVE_`echo $1 | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`],[])
    fi
])

AC_DEFUN([DAST_REPLACE_TYPE_UNSIGNED], [dnl
    AC_CACHE_CHECK([for $1], [ac_cv_type_$1], [dnl
        AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM(
                [[$ac_includes_default]],
                [[$1 foo]])],
            [ac_cv_type_$1=yes],
            [ac_cv_type_$1=no])
        if test $ac_cv_type_$1 != yes ; then
            if test "$ac_cv_sizeof_unsigned_char" = $2; then
                ac_cv_type_$1="unsigned char"
            elif test "$ac_cv_sizeof_unsigned_short" = $2; then
                ac_cv_type_$1="unsigned short"
            elif test "$ac_cv_sizeof_unsigned_int" = $2; then
                ac_cv_type_$1="unsigned int"
            elif test "$ac_cv_sizeof_unsigned_long" = $2; then
                ac_cv_type_$1="unsigned long"
            elif test "$ac_cv_sizeof_unsigned_long_long" = $2; then
                ac_cv_type_$1="unsigned long long"
            fi
        fi
    ])
    if test "$ac_cv_type_$1" != no; then
        if test "$ac_cv_type_$1" != yes; then
            AC_DEFINE_UNQUOTED([$1], [$ac_cv_type_$1])
        fi
        AC_DEFINE_UNQUOTED([HAVE_`echo $1 | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`],[])
    fi
])

dnl DAST_CHECK_ARG
dnl Check for the 3rd arguement to accept

AC_DEFUN([DAST_ACCEPT_ARG], [dnl
    if test -z "$ac_cv_accept_arg" ; then
        AC_LANG_PUSH([C++])
        AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM(
                [[$ac_includes_default
#include <sys/socket.h>]],
                [[$1 length;
accept( 0, 0, &length );]])],
            [ac_cv_accept_arg=$1])
        AC_LANG_POP([])
    fi
])

dnl Configure paths for Web100.  Based off of Owen Taylor's gtk.m4 from gtk+-1.2.10
dnl DAST_PATH_WEB100([EXACT-VERSION [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for Web100, and define WEB100_CFLAGS and WEB100_LIBS
dnl
AC_DEFUN([DAST_PATH_WEB100], [dnl
    web100_success=""
    AC_PATH_PROG([WEB100_CONFIG], [web100-config], [no])
    AC_MSG_CHECKING([for Web100])
    if test "$WEB100_CONFIG" != "no"; then
        WEB100_CFLAGS=`$WEB100_CONFIG --cflags`
        WEB100_LIBS=`$WEB100_CONFIG --libs`
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
        echo "*** The web100-config script installed by Web100 could not be found"
        echo "*** If Web100 was installed in PREFIX, make sure PREFIX/bin is in"
        echo "*** your path, or set the WEB100_CONFIG environment variable to the"
        echo "*** full path to web100-config"
        web100_success="no"
    fi
    if test x$web100_success = x; then
        if test "x$1" != "x"; then
            AC_MSG_CHECKING([for Web100 - version $1])
            WEB100_VERSION=`$WEB100_CONFIG --version`
            if test "$WEB100_VERSION" = "$1"; then
                AC_MSG_RESULT([yes])
            else
                AC_MSG_RESULT([no])
                echo "*** The requested ($1) and installed ($WEB100_VERSION) versions"
                echo "*** of Web100 do not match."
                web100_success="no"
            fi
        fi
    fi
    if test x$web100_success = x; then
        web100_success="yes"
    fi
    if test x$web100_success = xyes; then
        m4_if([$2], [], [:], [$2])
    else
        WEB100_CFLAGS=""
        WEB100_LIBS=""
        m4_if([$3], [], [:], [$3])
    fi
    AC_SUBST(WEB100_CFLAGS)dnl
    AC_SUBST(WEB100_LIBS)dnl
])

# =============================================================================
#
# $Log$
# =============================================================================
# 
# Copyright (c) 2001-2008  OpenSS7 Corporation <http://www.openss7.com/>
# Copyright (c) 1997-2000  Brian F. G. Bidulock <bidulock@openss7.org>
# 
# =============================================================================
# ENDING OF SEPARATE COPYRIGHT MATERIAL vim: ft=config sw=4 et
# =============================================================================
