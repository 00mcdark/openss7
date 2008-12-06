# vim: ft=config sw=4 noet nocin nosi com=b\:#,b\:dnl,b\:***,b\:@%\:@ fo+=tcqlorn
# =============================================================================
# BEGINNING OF SEPARATE COPYRIGHT MATERIAL
# =============================================================================
# 
# @(#) $RCSfile: acinclude.m4,v $ $Name:  $($Revision: 0.9.2.60 $) $Date: 2008-12-06 12:58:12 $
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
# Last Modified $Date: 2008-12-06 12:58:12 $ by $Author: brian $
#
# =============================================================================

m4_include([m4/openss7.m4])
m4_include([m4/dist.m4])
m4_include([m4/pr.m4])
m4_include([m4/init.m4])
m4_include([m4/kernel.m4])
m4_include([m4/devfs.m4])
dnl m4_include([m4/genksyms.m4])
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
m4_include([m4/sigtran.m4])
m4_include([m4/voip.m4])
dnl m4_include([m4/doxy.m4])

# =============================================================================
# AC_OS7
# -----------------------------------------------------------------------------
AC_DEFUN([AC_OS7], [dnl
    _OPENSS7_PACKAGE([OpenSS7], [OpenSS7 Master Package])
    _OS7_OPTIONS
    _AUTOPR
    _MAN_CONVERSION
    _PUBLIC_RELEASE
    _INIT_SCRIPTS
    _RPM_SPEC
    _DEB_DPKG
dnl AC_CONFIG_FILES([debian/openss7-core.postinst
dnl		     debian/openss7-core.postrm
dnl		     debian/openss7-core.preinst
dnl		     debian/openss7-core.prerm
dnl		     debian/openss7-devel.preinst
dnl		     debian/openss7-dev.postinst
dnl		     debian/openss7-dev.preinst
dnl		     debian/openss7-dev.prerm
dnl		     debian/openss7-doc.postinst
dnl		     debian/openss7-doc.preinst
dnl		     debian/openss7-doc.prerm
dnl		     debian/openss7-init.postinst
dnl		     debian/openss7-init.postrm
dnl		     debian/openss7-init.preinst
dnl		     debian/openss7-init.prerm
dnl		     debian/openss7-lib.preinst
dnl		     debian/openss7-source.preinst
dnl		     debian/openss7-util.preinst
dnl		     src/util/modutils/openss7
dnl		     src/include/sys/openss7/version.h])
    _LDCONFIG
    USER_CPPFLAGS="$CPPFLAGS"
    USER_CFLAGS="$CFLAGS"
    USER_LDFLAGS="$LDFLAGS"
    _OS7_SETUP
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+ }"'-imacros ${top_builddir}/config.h'
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+ }"'-imacros ${top_builddir}/${STRCONF_CONFIG}'
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+ }"'-I${top_srcdir}'
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${SIGTRAN_CPPFLAGS:+ }}${SIGTRAN_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${ATM_CPPFLAGS:+ }}${ATM_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${ISDN_CPPFLAGS:+ }}${ISDN_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${ISO_CPPFLAGS:+ }}${ISO_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${X25_CPPFLAGS:+ }}${X25_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${CHAN_CPPFLAGS:+ }}${CHAN_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${SCTP_CPPFLAGS:+ }}${SCTP_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${INET_CPPFLAGS:+ }}${INET_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${SOCK_CPPFLAGS:+ }}${SOCK_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${NSL_CPPFLAGS:+ }}${NSL_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${XTI_CPPFLAGS:+ }}${XTI_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${XNS_CPPFLAGS:+ }}${XNS_CPPFLAGS}"
    PKG_INCLUDES="${PKG_INCLUDES}${PKG_INCLUDES:+${SS7_CPPFLAGS:+ }}${SS7_CPPFLAGS}"
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
    PKG_MANPATH="${SS7_MANPATH:+${SS7_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${XNS_MANPATH:+${XNS_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${XTI_MANPATH:+${XTI_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${NSL_MANPATH:+${NSL_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${SOCK_MANPATH:+${SOCK_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${INET_MANPATH:+${INET_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${SCTP_MANPATH:+${SCTP_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${CHAN_MANPATH:+${CHAN_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${X25_MANPATH:+${X25_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${ISO_MANPATH:+${ISO_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${ISDN_MANPATH:+${ISDN_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${ATM_MANPATH:+${ATM_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH="${SIGTRAN_MANPATH:+${SIGTRAN_MANPATH}${PKG_MANPATH:+:}}${PKG_MANPATH}"
    PKG_MANPATH='$(top_builddir)/doc/man'"${PKG_MANPATH:+:}${PKG_MANPATH}"
    AC_SUBST([PKG_MANPATH])dnl
    CPPFLAGS=
    CFLAGS=
    _OS7_OUTPUT
dnl _AUTOTEST
])# AC_OS7
# =============================================================================

# =============================================================================
# _OS7_OPTIONS
# -----------------------------------------------------------------------------
AC_DEFUN([_OS7_OPTIONS], [dnl
    AC_ARG_WITH([ALL],
		AS_HELP_STRING([--with-ALL],
			       [include all packages in master pack @<:@disabled@:>@]),
		[with_ALL="${withval:-yes}"],
		[with_ALL=''])
    AC_ARG_WITH([SCTP],
		AS_HELP_STRING([--without-SCTP],
			       [do not include SCTP in master pack @<:@detected@:>@]),
		[with_SCTP="${withval:-yes}"],
		[with_SCTP="${with_ALL}"])
    AC_CACHE_CHECK([for sub-package sctp],[os7_cv_sctp_dir],[dnl
	    os7_cv_sctp_dir=''
	    for dir in $srcdir/sctp $srcdir/sctp-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_sctp_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_sctp_dir:-no}" = :no ; then
	with_SCTP='no'
    else
	if test :"${os7_cv_sctp_dir:-no}" != :sctp ; then
	    if test -e $srcdir/sctp ; then
		if test -L $srcdir/sctp ; then
		    if test "$(readlink $srcdir/sctp)" != "$os7_cv_sctp_dir" ; then
			rm -f $srcdir/sctp
			( cd $srcdir ; ln -sf $os7_cv_sctp_dir sctp )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_sctp_dir sctp )
	    fi
	fi
    fi
    AC_ARG_WITH([IPERF],
		AS_HELP_STRING([--without-IPERF],
			       [do not include IPERF in master pack @<:@detected@:>@]),
		[with_IPERF="${withval:-yes}"],
		[with_IPERF="${with_ALL}"])
    AC_CACHE_CHECK([for sub-package iperf],[os7_cv_iperf_dir],[dnl
	    os7_cv_iperf_dir=''
	    for dir in $srcdir/iperf $srcdir/iperf-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_iperf_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_iperf_dir:-no}" = :no ; then
	with_IPERF='no'
    else
	if test :"${os7_cv_iperf_dir:-no}" != :iperf ; then
	    if test -e $srcdir/iperf ; then
		if test -L $srcdir/iperf ; then
		    if test "$(readlink $srcdir/iperf)" != "$os7_cv_iperf_dir" ; then
			rm -f $srcdir/iperf
			( cd $srcdir ; ln -sf $os7_cv_iperf_dir iperf )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_iperf_dir iperf )
	    fi
	fi
    fi
    AC_ARG_WITH([LIS],
		AS_HELP_STRING([--with-LIS],
			       [include LIS in master pack @<:@detected@:>@]),
		[with_LIS="${withval:-yes}"],
		[with_LIS='no'])
    AC_CACHE_CHECK([for sub-package LiS],[os7_cv_LiS_dir],[dnl
	    os7_cv_LiS_dir=''
	    for dir in $srcdir/LiS $srcdir/LiS-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_LiS_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_LiS_dir:-no}" = :no ; then
	with_LIS='no'
    else
	if test :"${os7_cv_LiS_dir:-no}" != :LiS ; then
	    if test -e $srcdir/LiS ; then
		if test -L $srcdir/LiS ; then
		    if test "$(readlink $srcdir/LiS)" != "$os7_cv_LiS_dir" ; then
			rm -f $srcdir/LiS
			( cd $srcdir ; ln -sf $os7_cv_LiS_dir LiS )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_LiS_dir LiS )
	    fi
	fi
    fi
    AC_ARG_WITH([STREAMS],
		AS_HELP_STRING([--without-STREAMS],
			       [do not include STREAMS in master pack @<:@included@:>@]),
		[with_STREAMS="${withval:-yes}"],
		[with_STREAMS='yes'])
    AC_CACHE_CHECK([for sub-package streams],[os7_cv_streams_dir],[dnl
	    os7_cv_streams_dir=''
	    for dir in $srcdir/streams $srcdir/streams-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_streams_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_streams_dir:-no}" = :no ; then
	with_STREAMS='no'
    else
	if test :"${os7_cv_streams_dir:-no}" != :streams ; then
	    if test -e $srcdir/streams ; then
		if test -L $srcdir/streams ; then
		    if test "$(readlink $srcdir/streams)" != "$os7_cv_streams_dir" ; then
			rm -f $srcdir/streams
			( cd $srcdir ; ln -sf $os7_cv_streams_dir streams )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_streams_dir streams )
	    fi
	fi
    fi
    AC_ARG_WITH([STRCOMPAT],
		AS_HELP_STRING([--without-STRCOMPAT],
			       [do not include STRCOMPAT in master pack @<:@included@:>@]),
		[with_STRCOMPAT="${withval:-yes}"],
		[with_STRCOMPAT='yes'])
    AC_CACHE_CHECK([for sub-package strcompat],[os7_cv_strcompat_dir],[dnl
	    os7_cv_strcompat_dir=''
	    for dir in $srcdir/strcompat $srcdir/strcompat-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_strcompat_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_strcompat_dir:-no}" = :no ; then
	with_STRCOMPAT='no'
    else
	if test :"${os7_cv_strcompat_dir:-no}" != :strcompat ; then
	    if test -e $srcdir/strcompat ; then
		if test -L $srcdir/strcompat ; then
		    if test "$(readlink $srcdir/strcompat)" != "$os7_cv_strcompat_dir" ; then
			rm -f $srcdir/strcompat
			( cd $srcdir ; ln -sf $os7_cv_strcompat_dir strcompat )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_strcompat_dir strcompat )
	    fi
	fi
    fi
    AC_ARG_WITH([STRUTIL],
		AS_HELP_STRING([--with-STRUTIL],
			       [include STRUTIL in master pack @<:@detected@:>@]),
		[with_STRUTIL="${withval:-yes}"],
		[with_STRUTIL='no'])
    AC_CACHE_CHECK([for sub-package strutil],[os7_cv_strutil_dir],[dnl
	    os7_cv_strutil_dir=''
	    for dir in $srcdir/strutil $srcdir/strutil-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_strutil_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_strutil_dir:-no}" = :no ; then
	with_STRUTIL='no'
    else
	if test :"${os7_cv_strutil_dir:-no}" != :strutil ; then
	    if test -e $srcdir/strutil ; then
		if test -L $srcdir/strutil ; then
		    if test "$(readlink $srcdir/strutil)" != "$os7_cv_strutil_dir" ; then
			rm -f $srcdir/strutil
			( cd $srcdir ; ln -sf $os7_cv_strutil_dir strutil )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_strutil_dir strutil )
	    fi
	fi
    fi
    AC_ARG_WITH([STRBCM],
		AS_HELP_STRING([--with-STRBCM],
			       [include STRBCM in master pack @<:@detected@:>@]),
		[with_STRBCM="${withval:-yes}"],
		[with_STRBCM='no'])
    AC_CACHE_CHECK([for sub-package strbcm],[os7_cv_strbcm_dir],[dnl
	    os7_cv_strbcm_dir=''
	    for dir in $srcdir/strbcm $srcdir/strbcm-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_strbcm_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_strbcm_dir:-no}" = :no ; then
	with_STRBCM='no'
    else
	if test :"${os7_cv_strbcm_dir:-no}" != :strbcm ; then
	    if test -e $srcdir/strbcm ; then
		if test -L $srcdir/strbcm ; then
		    if test "$(readlink $srcdir/strbcm)" != "$os7_cv_strbcm_dir" ; then
			rm -f $srcdir/strbcm
			( cd $srcdir ; ln -sf $os7_cv_strbcm_dir strbcm )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_strbcm_dir strbcm )
	    fi
	fi
    fi
    AC_ARG_WITH([STRTTY],
		AS_HELP_STRING([--without-STRTTY],
			       [do not include STRTTY in master pack @<:@included@:>@]),
		[with_STRTTY="${withval:-yes}"],
		[with_STRTTY='yes'])
    AC_CACHE_CHECK([for sub-package strtty],[os7_cv_strtty_dir],[dnl
	    os7_cv_strtty_dir=''
	    for dir in $srcdir/strtty $srcdir/strtty-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_strtty_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_strtty_dir:-no}" = :no ; then
	with_STRTTY='no'
    else
	if test :"${os7_cv_strtty_dir:-no}" != :strtty ; then
	    if test -e $srcdir/strtty ; then
		if test -L $srcdir/strtty ; then
		    if test "$(readlink $srcdir/strtty)" != "$os7_cv_strtty_dir" ; then
			rm -f $srcdir/strtty
			( cd $srcdir ; ln -sf $os7_cv_strtty_dir strtty )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_strtty_dir strtty )
	    fi
	fi
    fi
    AC_ARG_WITH([STRXNS],
		AS_HELP_STRING([--without-STRXNS],
			       [do not include STRXNS in master pack @<:@included@:>@]),
		[with_STRXNS="${withval:-yes}"],
		[with_STRXNS='yes'])
    AC_CACHE_CHECK([for sub-package strxns],[os7_cv_strxns_dir],[dnl
	    os7_cv_strxns_dir=''
	    for dir in $srcdir/strxns $srcdir/strxns-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_strxns_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_strxns_dir:-no}" = :no ; then
	with_STRXNS='no'
    else
	if test :"${os7_cv_strxns_dir:-no}" != :strxns ; then
	    if test -e $srcdir/strxns ; then
		if test -L $srcdir/strxns ; then
		    if test "$(readlink $srcdir/strxns)" != "$os7_cv_strxns_dir" ; then
			rm -f $srcdir/strxns
			( cd $srcdir ; ln -sf $os7_cv_strxns_dir strxns )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_strxns_dir strxns )
	    fi
	fi
    fi
    AC_ARG_WITH([STRXNET],
		AS_HELP_STRING([--without-STRXNET],
			       [do not include STRXNET in master pack @<:@included@:>@]),
		[with_STRXNET="${withval:-yes}"],
		[with_STRXNET='yes'])
    AC_CACHE_CHECK([for sub-package strxnet],[os7_cv_strxnet_dir],[dnl
	    os7_cv_strxnet_dir=''
	    for dir in $srcdir/strxnet $srcdir/strxnet-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_strxnet_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_strxnet_dir:-no}" = :no ; then
	with_STRXNET='no'
    else
	if test :"${os7_cv_strxnet_dir:-no}" != :strxnet ; then
	    if test -e $srcdir/strxnet ; then
		if test -L $srcdir/strxnet ; then
		    if test "$(readlink $srcdir/strxnet)" != "$os7_cv_strxnet_dir" ; then
			rm -f $srcdir/strxnet
			( cd $srcdir ; ln -sf $os7_cv_strxnet_dir strxnet )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_strxnet_dir strxnet )
	    fi
	fi
    fi
    AC_ARG_WITH([STRNSL],
		AS_HELP_STRING([--without-STRNSL],
			       [do not include STRNSL in master pack @<:@included@:>@]),
		[with_STRNSL="${withval:-yes}"],
		[with_STRNSL='yes'])
    AC_CACHE_CHECK([for sub-package strnsl],[os7_cv_strnsl_dir],[dnl
	    os7_cv_strnsl_dir=''
	    for dir in $srcdir/strnsl $srcdir/strnsl-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_strnsl_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_strnsl_dir:-no}" = :no ; then
	with_STRNSL='no'
    else
	if test :"${os7_cv_strnsl_dir:-no}" != :strnsl ; then
	    if test -e $srcdir/strnsl ; then
		if test -L $srcdir/strnsl ; then
		    if test "$(readlink $srcdir/strnsl)" != "$os7_cv_strnsl_dir" ; then
			rm -f $srcdir/strnsl
			( cd $srcdir ; ln -sf $os7_cv_strnsl_dir strnsl )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_strnsl_dir strnsl )
	    fi
	fi
    fi
    AC_ARG_WITH([STRSOCK],
		AS_HELP_STRING([--without-STRSOCK],
			       [do not include STRSOCK in master pack @<:@included@:>@]),
		[with_STRSOCK="${withval:-yes}"],
		[with_STRSOCK='yes'])
    AC_CACHE_CHECK([for sub-package strsock],[os7_cv_strsock_dir],[dnl
	    os7_cv_strsock_dir=''
	    for dir in $srcdir/strsock $srcdir/strsock-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_strsock_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_strsock_dir:-no}" = :no ; then
	with_STRSOCK='no'
    else
	if test :"${os7_cv_strsock_dir:-no}" != :strsock ; then
	    if test -e $srcdir/strsock ; then
		if test -L $srcdir/strsock ; then
		    if test "$(readlink $srcdir/strsock)" != "$os7_cv_strsock_dir" ; then
			rm -f $srcdir/strsock
			( cd $srcdir ; ln -sf $os7_cv_strsock_dir strsock )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_strsock_dir strsock )
	    fi
	fi
    fi
    AC_ARG_WITH([STRINET],
		AS_HELP_STRING([--without-STRINET],
			       [do not include STRINET in master pack @<:@included@:>@]),
		[with_STRINET="${withval:-yes}"],
		[with_STRINET='yes'])
    AC_CACHE_CHECK([for sub-package strinet],[os7_cv_strinet_dir],[dnl
	    for dir in $srcdir/strinet $srcdir/strinet-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_strinet_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_strinet_dir:-no}" = :no ; then
	with_STRINET='no'
    else
	if test :"${os7_cv_strinet_dir:-no}" != :strinet ; then
	    if test -e $srcdir/strinet ; then
		if test -L $srcdir/strinet ; then
		    if test "$(readlink $srcdir/strinet)" != "$os7_cv_strinet_dir" ; then
			rm -f $srcdir/strinet
			( cd $srcdir ; ln -sf $os7_cv_strinet_dir strinet )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_strinet_dir strinet )
	    fi
	fi
    fi
    AC_ARG_WITH([STRSCTP],
		AS_HELP_STRING([--without-STRSCTP],
			       [do not include STRSCTP in master pack @<:@included@:>@]),
		[with_STRSCTP="${withval:-yes}"],
		[with_STRSCTP='yes'])
    AC_CACHE_CHECK([for sub-package strsctp],[os7_cv_strsctp_dir],[dnl
	    os7_cv_strsctp_dir=''
	    for dir in $srcdir/strsctp $srcdir/strsctp-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_strsctp_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_strsctp_dir:-no}" = :no ; then
	with_STRSCTP='no'
    else
	if test :"${os7_cv_strsctp_dir:-no}" != :strsctp ; then
	    if test -e $srcdir/strsctp ; then
		if test -L $srcdir/strsctp ; then
		    if test "$(readlink $srcdir/strsctp)" != "$os7_cv_strsctp_dir" ; then
			rm -f $srcdir/strsctp
			( cd $srcdir ; ln -sf $os7_cv_strsctp_dir strsctp )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_strsctp_dir strsctp )
	    fi
	fi
    fi
    AC_ARG_WITH([STRCHAN],
		AS_HELP_STRING([--without-STRCHAN],
			       [do not include STRCHAN in master pack @<:@included@:>@]),
		[with_STRCHAN="${withval:-yes}"],
		[with_STRCHAN='yes'])
    AC_CACHE_CHECK([for sub-package strchan],[os7_cv_strchan_dir],[dnl
	    for dir in $srcdir/strchan $srcdir/strchan-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_strchan_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_strchan_dir:-no}" = :no ; then
	with_STRCHAN='no'
    else
	if test :"${os7_cv_strchan_dir:-no}" != :strchan ; then
	    if test -e $srcdir/strchan ; then
		if test -L $srcdir/strchan ; then
		    if test "$(readlink $srcdir/strchan)" != "$os7_cv_strchan_dir" ; then
			rm -f $srcdir/strchan
			( cd $srcdir ; ln -sf $os7_cv_strchan_dir strchan )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_strchan_dir strchan )
	    fi
	fi
    fi
    AC_ARG_WITH([STRX25],
		AS_HELP_STRING([--without-STRX25],
			       [do not include STRX25 in master pack @<:@included@:>@]),
		[with_STRX25="${withval:-yes}"],
		[with_STRX25='yes'])
    AC_CACHE_CHECK([for sub-package strx25],[os7_cv_strx25_dir],[dnl
	    os7_cv_strx25_dir=''
	    for dir in $srcdir/strx25 $srcdir/strx25-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_strx25_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_strx25_dir:-no}" = :no ; then
	with_STRX25='no'
    else
	if test :"${os7_cv_strx25_dir:-no}" != :strx25 ; then
	    if test -e $srcdir/strx25 ; then
		if test -L $srcdir/strx25 ; then
		    if test "$(readlink $srcdir/strx25)" != "$os7_cv_strx25_dir" ; then
			rm -f $srcdir/strx25
			( cd $srcdir ; ln -sf $os7_cv_strx25_dir strx25 )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_strx25_dir strx25 )
	    fi
	fi
    fi
    AC_ARG_WITH([STRISO],
		AS_HELP_STRING([--without-STRISO],
			       [do not include STRISO in master pack @<:@included@:>@]),
		[with_STRISO="${withval:-yes}"],
		[with_STRISO='yes'])
    AC_CACHE_CHECK([for sub-package striso],[os7_cv_striso_dir],[dnl
	    os7_cv_striso_dir=''
	    for dir in $srcdir/striso $srcdir/striso-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_striso_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_striso_dir:-no}" = :no ; then
	with_STRISO='no'
    else
	if test :"${os7_cv_striso_dir:-no}" != :striso ; then
	    if test -e $srcdir/striso ; then
		if test -L $srcdir/striso ; then
		    if test "$(readlink $srcdir/striso)" != "$os7_cv_striso_dir" ; then
			rm -f $srcdir/striso
			( cd $srcdir ; ln -sf $os7_cv_striso_dir striso )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_striso_dir striso )
	    fi
	fi
    fi
    AC_ARG_WITH([NETPERF],
		AS_HELP_STRING([--without-NETPERF],
			       [do not include NETPERF in master pack @<:@included@:>@]),
		[with_NETPERF="${withval:-yes}"],
		[with_NETPERF='yes'])
    AC_CACHE_CHECK([for sub-package netperf],[os7_cv_netperf_dir],[dnl
	    os7_cv_netperf_dir=''
	    for dir in $srcdir/netperf $srcdir/netperf-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_netperf_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_netperf_dir:-no}" = :no ; then
	with_NETPERF='no'
    else
	if test :"${os7_cv_netperf_dir:-no}" != :netperf ; then
	    if test -e $srcdir/netperf ; then
		if test -L $srcdir/netperf ; then
		    if test "$(readlink $srcdir/netperf)" != "$os7_cv_netperf_dir" ; then
			rm -f $srcdir/netperf
			( cd $srcdir ; ln -sf $os7_cv_netperf_dir netperf )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_netperf_dir netperf )
	    fi
	fi
    fi
    AC_ARG_WITH([STRISDN],
		AS_HELP_STRING([--without-STRISDN],
			       [do not include STRISDN in master pack @<:@included@:>@]),
		[with_STRISDN="${withval:-yes}"],
		[with_STRISDN='yes'])
    AC_CACHE_CHECK([for sub-package strisdn],[os7_cv_strisdn_dir],[dnl
	    os7_cv_strisdn_dir=''
	    for dir in $srcdir/strisdn $srcdir/strisdn-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_strisdn_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_strisdn_dir:-no}" = :no ; then
	with_STRISDN='no'
    else
	if test :"${os7_cv_strisdn_dir:-no}" != :strisdn ; then
	    if test -e $srcdir/strisdn ; then
		if test -L $srcdir/strisdn ; then
		    if test "$(readlink $srcdir/strisdn)" != "$os7_cv_strisdn_dir" ; then
			rm -f $srcdir/strisdn
			( cd $srcdir ; ln -sf $os7_cv_strisdn_dir strisdn )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_strisdn_dir strisdn )
	    fi
	fi
    fi
    AC_ARG_WITH([STRATM],
		AS_HELP_STRING([--without-STRATM],
			       [do not include STRATM in master pack @<:@included@:>@]),
		[with_STRATM="${withval:-yes}"],
		[with_STRATM='yes'])
    AC_CACHE_CHECK([for sub-package stratm],[os7_cv_stratm_dir],[dnl
	    os7_cv_stratm_dir=''
	    for dir in $srcdir/stratm $srcdir/stratm-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_stratm_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_stratm_dir:-no}" = :no ; then
	with_STRATM='no'
    else
	if test :"${os7_cv_stratm_dir:-no}" != :stratm ; then
	    if test -e $srcdir/stratm ; then
		if test -L $srcdir/stratm ; then
		    if test "$(readlink $srcdir/stratm)" != "$os7_cv_stratm_dir" ; then
			rm -f $srcdir/stratm
			( cd $srcdir ; ln -sf $os7_cv_stratm_dir stratm )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_stratm_dir stratm )
	    fi
	fi
    fi
    AC_ARG_WITH([STACKS],
		AS_HELP_STRING([--without-STACKS],
			       [do not include STACKS in master pack @<:@included@:>@]),
		[with_STACKS="${withval:-yes}"],
		[with_STACKS='yes'])
    AC_CACHE_CHECK([for sub-package strss7],[os7_cv_strss7_dir],[dnl
	    os7_cv_strss7_dir=''
	    for dir in $srcdir/stacks $srcdir/strss7 $srcdir/strss7-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_strss7_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_strss7_dir:-no}" = :no ; then
	with_STACKS='no'
    else
	if test :"${os7_cv_strss7_dir:-no}" != :stacks ; then
	    if test -e $srcdir/stacks ; then
		if test -L $srcdir/stacks ; then
		    if test "$(readlink $srcdir/stacks)" != "$os7_cv_strss7_dir" ; then
			rm -f $srcdir/stacks
			( cd $srcdir ; ln -sf $os7_cv_strss7_dir stacks )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_strss7_dir stacks )
	    fi
	fi
    fi
    AC_ARG_WITH([SIGTRAN],
		AS_HELP_STRING([--without-SIGTRAN],
			       [do not include SIGTRAN in master pack @<:@included@:>@]),
		[with_SIGTRAN="${withval:-yes}"],
		[with_SIGTRAN='yes'])
    AC_CACHE_CHECK([for sub-package sigtran],[os7_cv_sigtran_dir],[dnl
	    os7_cv_sigtran_dir=''
	    for dir in $srcdir/sigtran $srcdir/sigtran-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_sigtran_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_sigtran_dir:-no}" = :no ; then
	with_SIGTRAN='no'
    else
	if test :"${os7_cv_sigtran_dir:-no}" != :sigtran ; then
	    if test -e $srcdir/sigtran ; then
		if test -L $srcdir/sigtran ; then
		    if test "$(readlink $srcdir/sigtran)" != "$os7_cv_sigtran_dir" ; then
			rm -f $srcdir/sigtran
			( cd $srcdir ; ln -sf $os7_cv_sigtran_dir sigtran )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_sigtran_dir sigtran )
	    fi
	fi
    fi
    AC_ARG_WITH([STRVOIP],
		AS_HELP_STRING([--without-STRVOIP],
			       [do not include STRVOIP in master pack @<:@included@:>@]),
		[with_STRVOIP="${withval:-yes}"],
		[with_STRVOIP='yes'])
    AC_CACHE_CHECK([for sub-package strvoip],[os7_cv_strvoip_dir],[dnl
	    os7_cv_strvoip_dir=''
	    for dir in $srcdir/strvoip $srcdir/strvoip-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_strvoip_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_strvoip_dir:-no}" = :no ; then
	with_STRVOIP='no'
    else
	if test :"${os7_cv_strvoip_dir:-no}" != :strvoip ; then
	    if test -e $srcdir/strvoip ; then
		if test -L $srcdir/strvoip ; then
		    if test "$(readlink $srcdir/strvoip)" != "$os7_cv_strvoip_dir" ; then
			rm -f $srcdir/strvoip
			( cd $srcdir ; ln -sf $os7_cv_strvoip_dir strvoip )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_strvoip_dir strvoip )
	    fi
	fi
    fi
    AC_ARG_WITH([OSR61],
		AS_HELP_STRING([--without-OSR61],
			       [do not include OSR61 in master pack @<:@included@:>@]),
		[with_OSR61="${withval:-yes}"],
		[with_OSR61='yes'])
    AC_CACHE_CHECK([for sub-package osr61],[os7_cv_osr61_dir],[dnl
	    os7_cv_osr61_dir=''
	    for dir in $srcdir/osr61 $srcdir/osr61-* ; do
		if test -d "$dir" -a -e "$dir/configure.ac" -a -r "$dir/configure.ac" ; then
		    os7_cv_osr61_dir="$(basename $dir)"
		fi
	    done
	])
    if test :"${os7_cv_osr61_dir:-no}" = :no ; then
	with_OSR61='no'
    else
	if test :"${os7_cv_osr61_dir:-no}" != :osr61 ; then
	    if test -e $srcdir/osr61 ; then
		if test -L $srcdir/osr61 ; then
		    if test "$(readlink $srcdir/osr61)" != "$os7_cv_osr61_dir" ; then
			rm -f $srcdir/osr61
			( cd $srcdir ; ln -sf $os7_cv_osr61_dir osr61 )
		    fi
		fi
	    else
		( cd $srcdir ; ln -sf $os7_cv_osr61_dir osr61 )
	    fi
	fi
    fi
])# _OS7_OPTIONS
# =============================================================================

# =============================================================================
# _OS7_SETUP
# -----------------------------------------------------------------------------
AC_DEFUN([_OS7_SETUP], [dnl
    _LINUX_KERNEL
    _LINUX_DEVFS
dnl _GENKSYMS
    _OS7_CONFIG_KERNEL
dnl
dnl Must have these directories defined before the locator functions are called.
dnl
    AC_CACHE_CHECK([for master srcdir],[os7_cv_master_srcdir],[dnl
	os7_cv_master_srcdir=`(cd $srcdir; pwd)`
    ])
    AC_CACHE_CHECK([for master builddir],[os7_cv_master_builddir],[dnl
	os7_cv_master_builddir=`pwd`
    ])
    master_srcdir="$os7_cv_master_srcdir"
    master_builddir="$os7_cv_master_builddir"
    AC_SUBST([master_srcdir])
    AC_SUBST([master_builddir])
dnl
dnl we have symbolic links in the master tarball, remove the h from chof
dnl
    am__tar='${AMTAR} cf - "$$tardir"'
dnl
dnl LiS or LFS.
dnl
    with_lis='yes'
    with_lfs='yes'
    if test :"${with_LIS:-yes}" = :no ; then
	with_lis='no'
    fi
    if test :"${with_STREAMS:-yes}" = :no ; then
	with_lfs='no'
    fi
dnl
dnl Make this darn specific
dnl
    if test :"${with_lis}" = :yes ; then
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_with_lis --with-lis\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--with-lis'"
	ac_configure_args="$ac_configure_args --with-lis"
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_lis --without-lis\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-lis'"
	ac_configure_args="$ac_configure_args --without-lis"
    fi
    if test :"${with_lfs}" = :yes ; then
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_with_lfs --with-lfs\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--with-lfs'"
	ac_configure_args="$ac_configure_args --with-lfs"
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_lfs --without-lfs\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-lfs'"
	ac_configure_args="$ac_configure_args --without-lfs"
    fi
    if test :"${with_LIS:-yes}" != :no -o :"${with_STREAMS:-yes}" != :no ; then
	: _LINUX_STREAMS
    fi
dnl
dnl Make this darn specific too
dnl
    if test :"${with_STRCOMPAT:-yes}" != :no ; then
	: _STRCOMP
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_compat --without-compat\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-compat'"
	ac_configure_args="$ac_configure_args --without-compat"
    fi
    if test :"${with_STRUTIL:-yes}" != :no ; then
	:
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_util --without-util\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-util'"
	ac_configure_args="$ac_configure_args --without-util"
    fi
    if test :"${with_STRBCM:-yes}" != :no ; then
	:
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_bcm --without-bcm\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-bcm'"
	ac_configure_args="$ac_configure_args --without-bcm"
    fi
    if test :"${with_STRTTY:-yes}" != :no ; then
	:
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_tty --without-tty\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-tty'"
	ac_configure_args="$ac_configure_args --without-tty"
    fi
    if test :"${with_STRXNS:-yes}" != :no ; then
	: _XNS
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_xns --without-xns\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-xns'"
	ac_configure_args="$ac_configure_args --without-xns"
    fi
    if test :"${with_STRXNET:-yes}" != :no ; then
	: _XTI
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_xti --without-xti\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-xti'"
	ac_configure_args="$ac_configure_args --without-xti"
    fi
    if test :"${with_STRNSL:-yes}" != :no ; then
	: _NSL
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_nsl --without-nsl\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-nsl'"
	ac_configure_args="$ac_configure_args --without-nsl"
    fi
    if test :"${with_STRSOCK:-yes}" != :no ; then
	: _SOCK
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_sock --without-sock\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-sock'"
	ac_configure_args="$ac_configure_args --without-sock"
    fi
    if test :"${with_STRINET:-yes}" != :no ; then
	: _INET
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_inet --without-inet\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-inet'"
	ac_configure_args="$ac_configure_args --without-inet"
    fi
    if test :"${with_STRSCTP:-yes}" != :no ; then
	: _SCTP
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_sctp --without-sctp\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-sctp'"
	ac_configure_args="$ac_configure_args --without-sctp"
    fi
    if test :"${with_STRCHAN:-yes}" != :no ; then
	: _CHAN
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_chan --without-chan\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-chan'"
	ac_configure_args="$ac_configure_args --without-chan"
    fi
    if test :"${with_STRX25:-yes}" != :no ; then
	: _X25
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_x25 --without-x25\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-x25'"
	ac_configure_args="$ac_configure_args --without-x25"
    fi
    if test :"${with_STRISO:-yes}" != :no ; then
	: _ISO
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_iso --without-iso\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-iso'"
	ac_configure_args="$ac_configure_args --without-iso"
    fi
    if test :"${with_STRISDN:-yes}" != :no ; then
	: _ISDN
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_isdn --without-isdn\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-isdn'"
	ac_configure_args="$ac_configure_args --without-isdn"
    fi
    if test :"${with_STRATM:-yes}" != :no ; then
	: _ATM
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_atm --without-atm\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-atm'"
	ac_configure_args="$ac_configure_args --without-atm"
    fi
    if test :"${with_STACKS:-yes}" != :no ; then
	: _SS7
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_ss7 --without-ss7\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-ss7'"
	ac_configure_args="$ac_configure_args --without-ss7"
    fi
    if test :"${with_SIGTRAN:-yes}" != :no ; then
	: _SIGTRAN
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_sigtran --without-sigtran\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-sigtran'"
	ac_configure_args="$ac_configure_args --without-sigtran"
    fi
    if test :"${with_STRVOIP:-yes}" != :no ; then
	: _VOIP
    else
	PACKAGE_RPMOPTIONS="${PACKAGE_RPMOPTIONS}${PACKAGE_RPMOPTIONS:+ }--define \"_without_voip --without-voip\""
	PACKAGE_DEBOPTIONS="${PACKAGE_DEBOPTIONS}${PACKAGE_DEBOPTIONS:+ }'--without-voip'"
	ac_configure_args="$ac_configure_args --without-voip"
    fi
])# _OS7_SETUP
# =============================================================================

# =============================================================================
# _OS7_CONFIG_KERNEL
# -----------------------------------------------------------------------------
AC_DEFUN([_OS7_CONFIG_KERNEL], [dnl
])# _OS7_CONFIG_KERNEL
# =============================================================================

# =============================================================================
# _OS7_REQUIRE([package], [version], [build])
# -----------------------------------------------------------------------------
AC_DEFUN([_OS7_REQUIRE], [dnl
    os7_mc='$1'
    os7_uc=`echo "$1" | sed -e 'y,abcdefghijklmnopqrstuvwxyz,ABCDEFGHIJKLMNOPQRSTUVWXYZ,'`
    os7_bc=`echo "$3" | sed -e 'y,abcdefghijklmnopqrstuvwxyz,ABCDEFGHIJKLMNOPQRSTUVWXYZ,'`
    if test :$os7_mc = :strss7 ; then os7_uc='STACKS' ; fi
    os7_with="with_${os7_uc}"
    os7_dir="os7_cv_${os7_mc}_dir"
    os7_srcdir=`(cd $srcdir; pwd)`
    if eval "test :\${$os7_dir:-no} == :no" ; then
	AC_MSG_ERROR([
***
*** Sub-package $3 requires sub-package $1-$2 (or better)
*** to build.  Please download package $1-$2 from
***
***   http://www.openss7.org/tarballs/$1-$2.tar.bz2
***
*** and unpack it in the source directory
*** 
***   $os7_srcdir
***
*** and call configure again, or specificy the configuration
*** option --without-$os7_bc and configure again.
*** ])
    fi
    if eval "test :\${$os7_with:-yes} != :yes" ; then
	AC_MSG_ERROR([
***
*** Sub-package $3 requires sub-package $1-$2 (or better)
*** to build.  Please specify --with-$os7_uc or --without-$os7_bc
*** and configure again.
*** ])
    fi
])# _OS7_REQUIRE
# =============================================================================

# =============================================================================
# _OS7_OUTPUT
# -----------------------------------------------------------------------------
AC_DEFUN([_OS7_OUTPUT], [dnl
    if test :${with_SCTP:-auto} = :auto ; then
	if test :${linux_cv_k_ko_modules:-yes} = :yes ; then
	    with_SCTP='no'
	else
	    with_SCTP='yes'
	fi
    fi
    if test :${with_IPERF:-auto} = :auto ; then
	if test :${linux_cv_k_ko_modules:-yes} = :yes ; then
	    with_IPERF='no'
	else
	    with_IPERF='yes'
	fi
    fi
    if test :${with_LIS:-auto} = :auto ; then
	with_LIS='no'
    fi
    if test :${with_SCTP:-yes} = :yes -a :${os7_cv_sctp_dir:-no} != :no ; then
	AC_CONFIG_SUBDIRS([sctp])
    fi
    AM_CONDITIONAL([WITH_SCTP], [test :${with_SCTP:-yes} = :yes -a :${os7_cv_sctp_dir:-no} != :no])dnl
    if test :${with_IPERF:-yes} = :yes -a :${os7_cv_iperf_dir:-no} != :no ; then
	_OS7_REQUIRE([sctp], [0.2.27], [iperf])
	AC_CONFIG_SUBDIRS([iperf])
    fi
    AM_CONDITIONAL([WITH_IPERF], [test :${with_IPERF:-yes} = :yes -a :${os7_cv_iperf_dir:-no} != :no])dnl
    if test :${with_LIS:-yes} = :yes -a :${os7_cv_LiS_dir:-no} != :no ; then
	AC_CONFIG_SUBDIRS([LiS])
    fi
    AM_CONDITIONAL([WITH_LIS], [test :${with_LIS:-yes} = :yes -a :${os7_cv_LiS_dir:-no} != :no])dnl
    if test :${with_STREAMS:-yes} = :yes -a :${os7_cv_streams_dir:-no} != :no ; then
	AC_CONFIG_SUBDIRS([streams])
    fi
    AM_CONDITIONAL([WITH_LFS], [test :${with_STREAMS:-yes} = :yes -a :${os7_cv_streams_dir:-no} != :no])dnl
    if test :${with_STRCOMPAT:-yes} = :yes -a :${os7_cv_strcompat_dir:-no} != :no ; then
	_OS7_REQUIRE([streams], [0.9.2.4], [strcompat])
	AC_CONFIG_SUBDIRS([strcompat])
    fi
    AM_CONDITIONAL([WITH_STRCOMPAT], [test :${with_STRCOMPAT:-yes} = :yes -a :${os7_cv_strcompat_dir:-no} != :no])dnl
    if test :${with_STRUTIL:-yes} = :yes -a :${os7_cv_strutil_dir:-no} != :no ; then
	_OS7_REQUIRE([strcompat], [0.9.2.7], [strutil])
	AC_CONFIG_SUBDIRS([strutil])
    fi
    AM_CONDITIONAL([WITH_STRUTIL], [test :${with_STRUTIL:-yes} = :yes -a :${os7_cv_strutil_dir:-no} != :no])dnl
    if test :${with_STRBCM:-yes} = :yes -a :${os7_cv_strbcm_dir:-no} != :no ; then
	_OS7_REQUIRE([strcompat], [0.9.2.7], [strbcm])
	AC_CONFIG_SUBDIRS([strbcm])
    fi
    AM_CONDITIONAL([WITH_STRBCM], [test :${with_STRBCM:-yes} = :yes -a :${os7_cv_strbcm_dir:-no} != :no])dnl
    if test :${with_STRTTY:-yes} = :yes -a :${os7_cv_strtty_dir:-no} != :no ; then
	_OS7_REQUIRE([strcompat], [0.9.2.7], [strtty])
	AC_CONFIG_SUBDIRS([strtty])
    fi
    AM_CONDITIONAL([WITH_STRTTY], [test :${with_STRTTY:-yes} = :yes -a :${os7_cv_strtty_dir:-no} != :no])dnl
    if test :${with_STRXNS:-yes} = :yes -a :${os7_cv_strxns_dir:-no} != :no ; then
	_OS7_REQUIRE([strcompat], [0.9.2.7], [strxns])
	AC_CONFIG_SUBDIRS([strxns])
    fi
    AM_CONDITIONAL([WITH_STRXNS], [test :${with_STRXNS:-yes} = :yes -a :${os7_cv_strxns_dir:-no} != :no])dnl
    if test :${with_STRXNET:-yes} = :yes -a :${os7_cv_strxnet_dir:-no} != :no ; then
	_OS7_REQUIRE([strxns], [0.9.2.7], [strxnet])
	AC_CONFIG_SUBDIRS([strxnet])
    fi
    AM_CONDITIONAL([WITH_STRXNET], [test :${with_STRXNET:-yes} = :yes -a :${os7_cv_strxnet_dir:-no} != :no])dnl
    if test :${with_STRNSL:-yes} = :yes -a :${os7_cv_strnsl_dir:-no} != :no ; then
	_OS7_REQUIRE([strxnet], [0.9.2.12], [strnsl])
	AC_CONFIG_SUBDIRS([strnsl])
    fi
    AM_CONDITIONAL([WITH_STRNSL], [test :${with_STRNSL:-yes} = :yes -a :${os7_cv_strnsl_dir:-no} != :no])dnl
    if test :${with_STRSOCK:-yes} = :yes -a :${os7_cv_strsock_dir:-no} != :no ; then
	_OS7_REQUIRE([strnsl], [0.9.2.4], [strsock])
	AC_CONFIG_SUBDIRS([strsock])
    fi
    AM_CONDITIONAL([WITH_STRSOCK], [test :${with_STRSOCK:-yes} = :yes -a :${os7_cv_strsock_dir:-no} != :no])dnl
    if test :${with_STRINET:-yes} = :yes -a :${os7_cv_strinet_dir:-no} != :no ; then
	_OS7_REQUIRE([strxnet], [0.9.2.12], [strinet])
	AC_CONFIG_SUBDIRS([strinet])
    fi
    AM_CONDITIONAL([WITH_STRINET], [test :${with_STRINET:-yes} = :yes -a :${os7_cv_strinet_dir:-no} != :no])dnl
    if test :${with_STRSCTP:-yes} = :yes -a :${os7_cv_strsctp_dir:-no} != :no ; then
	_OS7_REQUIRE([strxnet], [0.9.2.12], [strsctp])
	AC_CONFIG_SUBDIRS([strsctp])
    fi
    AM_CONDITIONAL([WITH_STRSCTP], [test :${with_STRSCTP:-yes} = :yes -a :${os7_cv_strsctp_dir:-no} != :no])dnl
    if test :${with_STRCHAN:-yes} = :yes -a :${os7_cv_strchan_dir:-no} != :no ; then
	_OS7_REQUIRE([strxns], [0.9.2.7], [strchan])
	AC_CONFIG_SUBDIRS([strchan])
    fi
    AM_CONDITIONAL([WITH_STRCHAN], [test :${with_STRCHAN:-yes} = :yes -a :${os7_cv_strchan_dir:-no} != :no])dnl
    if test :${with_STRX25:-yes} = :yes -a :${os7_cv_strx25_dir:-no} != :no ; then
	_OS7_REQUIRE([strxnet], [0.9.2.12], [strx25])
	AC_CONFIG_SUBDIRS([strx25])
    fi
    AM_CONDITIONAL([WITH_STRX25], [test :${with_STRX25:-yes} = :yes -a :${os7_cv_strx25_dir:-no} != :no])dnl
    if test :${with_STRISO:-yes} = :yes -a :${os7_cv_striso_dir:-no} != :no ; then
	_OS7_REQUIRE([strxnet], [0.9.2.12], [striso])
	AC_CONFIG_SUBDIRS([striso])
    fi
    AM_CONDITIONAL([WITH_STRISO], [test :${with_STRISO:-yes} = :yes -a :${os7_cv_striso_dir:-no} != :no])dnl
    if test :${with_NETPERF:-yes} = :yes -a :${os7_cv_netperf_dir:-no} != :no ; then
	_OS7_REQUIRE([strxnet], [0.9.2.12], [netperf])
	_OS7_REQUIRE([strinet], [0.9.2.7], [netperf])
	_OS7_REQUIRE([strsctp], [0.9.2.9], [netperf])
	_OS7_REQUIRE([strx25], [0.9.2.1], [netperf])
	_OS7_REQUIRE([striso], [0.9.2.4], [netperf])
	AC_CONFIG_SUBDIRS([netperf])
    fi
    AM_CONDITIONAL([WITH_NETPERF], [test :${with_NETPERF:-yes} = :yes -a :${os7_cv_netperf_dir:-no} != :no])dnl
    if test :${with_STRISDN:-yes} = :yes -a :${os7_cv_strisdn_dir:-no} != :no ; then
	_OS7_REQUIRE([strxnet], [0.9.2.12], [strisdn])
	AC_CONFIG_SUBDIRS([strisdn])
    fi
    AM_CONDITIONAL([WITH_STRISDN], [test :${with_STRISDN:-yes} = :yes -a :${os7_cv_strisdn_dir:-no} != :no])dnl
    if test :${with_STRATM:-yes} = :yes -a :${os7_cv_stratm_dir:-no} != :no ; then
	_OS7_REQUIRE([strchan], [0.9.2.7], [stratm])
	_OS7_REQUIRE([strxns], [0.9.2.7], [stratm])
	_OS7_REQUIRE([strxnet], [0.9.2.12], [stratm])
	_OS7_REQUIRE([strx25], [0.9.2.1], [stratm])
	_OS7_REQUIRE([striso], [0.9.2.4], [stratm])
	_OS7_REQUIRE([strisdn], [0.9.2.4], [stratm])
	AC_CONFIG_SUBDIRS([stratm])
    fi
    AM_CONDITIONAL([WITH_STRATM], [test :${with_STRATM:-yes} = :yes -a :${os7_cv_stratm_dir:-no} != :no])dnl
    if test :${with_STACKS:-yes} = :yes -a :${os7_cv_strss7_dir:-no} != :no ; then
	_OS7_REQUIRE([strxnet], [0.9.2.12], [strss7])
	_OS7_REQUIRE([strinet], [0.9.2.7], [strss7])
	_OS7_REQUIRE([strsctp], [0.9.2.9], [strss7])
	_OS7_REQUIRE([strx25], [0.9.2.1], [strss7])
	_OS7_REQUIRE([striso], [0.9.2.4], [strss7])
	_OS7_REQUIRE([strisdn], [0.9.2.4], [strss7])
	_OS7_REQUIRE([stratm], [0.9.2.1], [strss7])
	AC_CONFIG_SUBDIRS([stacks])
    fi
    AM_CONDITIONAL([WITH_STACKS], [test :${with_STACKS:-yes} = :yes -a :${os7_cv_strss7_dir:-no} != :no])dnl
    if test :${with_SIGTRAN:-yes} = :yes -a :${os7_cv_sigtran_dir:-no} != :no ; then
	_OS7_REQUIRE([strsctp], [0.9.2.9], [sigtran])
	_OS7_REQUIRE([strss7], [0.9a.7], [sigtran])
	AC_CONFIG_SUBDIRS([sigtran])
    fi
    AM_CONDITIONAL([WITH_SIGTRAN], [test :${with_SIGTRAN:-yes} = :yes -a :${os7_cv_sigtran_dir:-no} != :no])dnl
    if test :${with_STRVOIP:-yes} = :yes -a :${os7_cv_strvoip_dir:-no} != :no ; then
	_OS7_REQUIRE([strss7], [0.9a.8], [strvoip])
	AC_CONFIG_SUBDIRS([strvoip])
    fi
    AM_CONDITIONAL([WITH_STRVOIP], [test :${with_STRVOIP:-yes} = :yes -a :${os7_cv_strvoip_dir:-no} != :no])dnl
    if test :${with_OSR61:-yes} = :yes -a :${os7_cv_osr61_dir:-no} != :no ; then
	_OS7_REQUIRE([strxnet], [0.9.2.12], [osr61])
	AC_CONFIG_SUBDIRS([osr61])
    fi
    AM_CONDITIONAL([WITH_OSR61], [test :${with_OSR61:-yes} = :yes -a :${os7_cv_osr61_dir:-no} != :no])dnl
])# _OS7_OUTPUT
# =============================================================================

# =============================================================================
# _OS7_
# -----------------------------------------------------------------------------
AC_DEFUN([_OS7_], [dnl
])# _OS7_
# =============================================================================

# =============================================================================
#
# $Log: acinclude.m4,v $
# Revision 0.9.2.60  2008-12-06 12:58:12  brian
# - updates for stratm package
#
# Revision 0.9.2.59  2008-09-28 17:04:32  brian
# - adjustments
#
# Revision 0.9.2.58  2008-09-23 03:32:42  brian
# - correct STRUTIL and STRBCM defaults
#
# Revision 0.9.2.57  2008-09-22 17:37:22  brian
# - build and release updates
#
# Revision 0.9.2.56  2008-08-02 16:42:17  brian
# - updates for release
#
# Revision 0.9.2.55  2008-05-03 13:23:24  brian
# - added strx25 sub-package and package updates
#
# Revision 0.9.2.54  2008-04-28 10:07:38  brian
# - updated headers for release
#
# Revision 0.9.2.53  2008-04-12 10:06:44  brian
# - updates for autoconf 2.62
#
# Revision 0.9.2.52  2007/08/15 04:53:57  brian
# - GPLv3 updates
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
