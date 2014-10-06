# vim: ft=config sw=4 noet nocin nosi com=b\:#,b\:dnl,b\:***,b\:@%\:@ fo+=tcqlorn
# =============================================================================
# BEGINNING OF SEPARATE COPYRIGHT MATERIAL
# =============================================================================
# 
# @(#) $RCSfile: pr.m4,v $ $Name:  $($Revision: 1.1.2.3 $) $Date: 2011-02-28 19:51:30 $
#
# -----------------------------------------------------------------------------
#
# Copyright (c) 2008-2011  Monavacon Limited <http://www.monavacon.com/>
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
# Last Modified $Date: 2011-02-28 19:51:30 $ by $Author: brian $
#
# =============================================================================

# =============================================================================
# _AUTOPR
# -----------------------------------------------------------------------------
AC_DEFUN([_AUTOPR], [dnl
    AC_REQUIRE([_DISTRO])
    AC_MSG_NOTICE([+---------------------------+])
    AC_MSG_NOTICE([| Problem Report Generation |])
    AC_MSG_NOTICE([+---------------------------+])
    _AUTOPR_SETUP
    _AUTOPR_OUTPUT
])# _AUTOPR
# =============================================================================

# =============================================================================
# _AUTOPR_SETUP
# -----------------------------------------------------------------------------
AC_DEFUN([_AUTOPR_SETUP], [dnl
    AC_CACHE_CHECK([for send-pr distribution], [ap_cv_distribution], [dnl
	case "$dist_cv_host_flavor" in
	    (centos)	 ap_cv_distribution="COS$dist_cv_host_release"	    ;;
	    (lineox)	 ap_cv_distribution="LEL$dist_cv_host_release"	    ;;
	    (whitebox)	 ap_cv_distribution="WBEL$dist_cv_host_release"	    ;;
	    (fedora)	 ap_cv_distribution="FC$dist_cv_host_release"	    ;;
	    (redhat)
		case $dist_cv_host_release in
		    (6.2|7.[[0-3]]|8.0|9)
				ap_cv_distribution="RH$dist_cv_host_release" ;;
		    (2|2.?)	ap_cv_distribution="RHAS2" ;;
		    (3|3.?)	ap_cv_distribution="RHEL3" ;;
		    (4|4.?)	ap_cv_distribution="RHEL4" ;;
		    (5|5.?)	ap_cv_distribution="RHEL5" ;;
		    (*)		ap_cv_distribution="RH$dist_cv_host_release" ;;
		esac ;;
	    (rhel)
		case $dist_cv_host_release in
		    (2|2.?)	ap_cv_distribution="RHAS2" ;;
		    (3|3.?)	ap_cv_distribution="RHEL3" ;;
		    (4|4.?)	ap_cv_distribution="RHEL4" ;;
		    (5|5.?)	ap_cv_distribution="RHEL5" ;;
		    (6|6.?)	ap_cv_distribution="RHEL6" ;;
		    (*)		ap_cv_distribution="RHEL$dist_cv_host_release" ;;
	        esac ;;
	    (mandrake)	 ap_cv_distribution="MDK$dist_cv_host_release"	    ;;
	    (mandriva)	 ap_cv_distribution="MDV$dist_cv_host_release"	    ;;
	    (manbo)	 ap_cv_distribution="MNB$dist_cv_host_release"	    ;;
	    (suse)
	        case $dist_cv_host_release in
		    (6.2|7.[[0-3]]|8.[[0-3]]|9.[[0-3]])
		    		ap_cv_distribution="SuSE$dist_cv_host_release"	;;
		    (8|9|10)	ap_cv_distribution="SLES$dist_cv_host_release"	;;
		    (*)		ap_cv_distribution="SuSE$dist_cv_host_release"	;;
		esac ;;
	    (sles)	 ap_cv_distribution="SLES$dist_cv_host_release"	    ;;
	    (sled)	 ap_cv_distribution="SLED$dist_cv_host_release"	    ;;
	    (sle)	 ap_cv_distribution="SLE$dist_cv_host_release"	    ;;
	    (debian)	 ap_cv_distribution="Debian$dist_cv_host_release"   ;;
	    (ubuntu)	 ap_cv_distribution="Ubuntu$dist_cv_host_release"   ;;
	    (montavista) ap_cv_distribution="MontaVista"		    ;;
	    (bluecat)	 ap_cv_distribution="BlueCat"			    ;;
	    (yellowdog)	 ap_cv_distribution="YellowDog"			    ;;
	    (denx)	 ap_cv_distribution="ELDK"			    ;;
	    (*)		 ap_cv_distribution='other'			    ;;
	esac
	])
    AP_DISTRIBUTION="$ap_cv_distribution"
    AC_SUBST([AP_DISTRIBUTION])
    AC_MSG_CHECKING([for send-pr script])
    AP_SCRIPT="$ac_aux_dir/send-pr"
    AC_MSG_RESULT([${AP_SCRIPT}])
    AC_SUBST([AP_SCRIPT])
    AC_MSG_CHECKING([for send-pr command])
    AP_AUTOPR="$SHELL $AP_SCRIPT"
    AC_MSG_RESULT([${AP_AUTOPR}])
    AC_SUBST([AP_AUTOPR])
    AC_MSG_CHECKING([for send-pr config])
    AP_CONFIG="scripts/send-pr.config"
    AC_MSG_RESULT([${AP_CONFIG}])
    AC_SUBST([AP_CONFIG])
    AC_CONFIG_FILES([scripts/send-pr.config])
])# _AUTOPR_SETUP
# =============================================================================

# =============================================================================
# _AUTOPR_OUTPUT
# -----------------------------------------------------------------------------
AC_DEFUN([_AUTOPR_OUTPUT], [dnl
])# _AUTOPR_OUTPUT
# =============================================================================

# =============================================================================
# _AUTOPR_
# -----------------------------------------------------------------------------
AC_DEFUN([_AUTOPR_], [dnl
])# _AUTOPR_
# =============================================================================

# =============================================================================
#
# $Log: pr.m4,v $
# Revision 1.1.2.3  2011-02-28 19:51:30  brian
# - better repository build
#
# Revision 1.1.2.2  2011-02-07 04:48:32  brian
# - updated configure and build scripts
#
# Revision 1.1.2.1  2009-06-21 11:06:05  brian
# - added files to new distro
#
# Revision 0.9.2.6  2008-04-28 09:41:03  brian
# - updated headers for release
#
# Revision 0.9.2.5  2007/08/12 19:05:31  brian
# - rearrange and update headers
#
# Revision 0.9.2.4  2006/08/16 07:30:47  brian
# - changes for SLES 10
#
# Revision 0.9.2.3  2006/06/29 23:38:09  brian
# - more variations for Ubuntu, which in most ways is much like Debian.
#
# Revision 0.9.2.2  2006/03/23 05:41:18  brian
# - renamed auto-pr to send-pr
#
# Revision 0.9.2.1  2006/03/21 13:23:45  brian
# - added problem reports
#
# =============================================================================
# 
# Copyright (c) 2008-2011  Monavacon Limited <http://www.monavacon.com/>
# Copyright (c) 2001-2008  OpenSS7 Corporation <http://www.openss7.com/>
# Copyright (c) 1997-2000  Brian F. G. Bidulock <bidulock@openss7.org>
# 
# =============================================================================
# ENDING OF SEPARATE COPYRIGHT MATERIAL
# =============================================================================
# vim: ft=config sw=4 noet nocin nosi com=b\:#,b\:dnl,b\:***,b\:@%\:@ fo+=tcqlorn
