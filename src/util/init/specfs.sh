#!/bin/sh
#
# @(#) $RCSfile: specfs.sh,v $ $Name:  $($Revision: 1.1.2.2 $) $Date: 2011-01-12 00:19:32 $
# Copyright (c) 2001-2011  OpenSS7 Corporation <http://www.openss7.com>
# Copyright (c) 1997-2000  Brian F. G. Bidulock <bidulock@openss7.org>
# All Rights Reserved.
#
# Distributed by OpenSS7 Corporation.  See the bottom of this script for copying
# permissions.
#
# These are arguments to update-rc.d ala chkconfig and lsb.  They are recognized
# by openss7 install_initd and remove_initd scripts.  Each line specifies
# arguments to add and remove links after the the name argument:
#
# specfs:	mount and unmount specfs
# update-rc.d:	start 35 S .
# config:	/etc/default/specfs
# probe:	false
# hide:		false
# license:	GPL
# description:	The SPECFS is a special shadow filesystem used for Linux \
#		Fast-STREAMS.  The purpose of this init script is to detect \
#		whether the specfs is supported for the running kernel, and if \
#		it is, to configure and load the kernel module associated with \
#		the special filesystem.
#
# LSB init script conventions
#
### BEGIN INIT INFO
# Provides: specfs
# Required-Start: $local_fs
# Required-Stop: $local_fs
# Default-Start: 2 3 4 5
# Default-Stop: 0 1 6
# X-UnitedLinux-Default-Enabled: yes
# Short-Description: mount and unmount specfs
# License: GPL
# Description:	The SPECFS is a special shadow filesystem used for Linux
#	Fast-STREAMS.  The purpose of this init script is to detect whether the
#	specfs is supported for the running kernel, and if it is, to configure
#	and load the kernel module associated with the special filesystem.
### END INIT INFO

PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin
name='specfs'
config="/etc/default/$name"
desc="the STREAMS special shadow filesystem"

[ -e /proc/modules ] || exit 0
[ -e /proc/filesystems ] || exit 0

# Specify defaults

[ -n "$SPECFS_MOUNTPOINT" ] || SPECFS_MOUNTPOINT="/dev/streams"
[ -n "$SPECFS_UID"        ] || SPECFS_UID=
[ -n "$SPECFS_GID"        ] || SPECFS_GID=
[ -n "$SPECFS_MODE"       ] || SPECFS_MODE=
[ -n "$SPECFS_OPTIONS"    ] || SPECFS_OPTIONS=

# Source config file
for file in $config ; do
    [ -f $file ] && . $file
done

RETVAL=0

umask 077

case :$VERBOSE in
    :no|:NO|:false|:FALSE|:0|:)
	redir='>/dev/null 2>&1'
	;;
    *)
	redir=
	;;
esac

build_options() {
    # Build up the options string
    [ -n "$SPECFS_UID" ] && \
	SPECFS_OPTIONS="${SPECFS_OPTIONS:--o }${SPECFS_OPTIONS:+,}uid=${SPECFS_UID}"
    [ -n "$SPECFS_GID" ] && \
	SPECFS_OPTIONS="${SPECFS_OPTIONS:--o }${SPECFS_OPTIONS:+,}gid=${SPECFS_GID}"
    [ -n "$SPECFS_MODE" ] && \
	SPECFS_OPTIONS="${SPECFS_OPTIONS:--o }${SPECFS_OPTIONS:+,}mode=${SPECFS_MODE}"
    [ "$1" = remount ] && \
	SPECFS_OPTIONS="${SPECFS_OPTIONS:--o }${SPECFS_OPTIONS:+,}remount"
}

start() {
    if [ -n "$SPECFS_MOUNTPOINT" ] ; then
	if ! grep -qc '[[:space:]]specfs\>' /proc/filesystems ; then
	    echo -n "Loading SPECFS kernel modules: "
	    eval "modprobe -q -- specfs $redir"
	    RETVAL=$? ; if [ $RETVAL -ne 0 ] ; then echo "(failed.)" ; return $RETVAL ; fi
	    echo "specfs."
	fi
	if [ ! -d "$SPECFS_MOUNTPOINT" ] ; then
	    echo -n "Creating SPECFS mount point: "
	    eval "mkdir -p -- \"$SPECFS_MOUNTPOINT\" $redir"
	    RETVAL=$? ; if [ $RETVAL -ne 0 ] ; then echo "(failed.)" ; return $RETVAL ; fi
	    echo "$SPECFS_MOUNTPOINT."
	fi
	if ! mount | grep " on $SPECFS_MOUNTPOINT type specfs" >/dev/null 2>&1 ; then
	    build_options
	    echo -n "Mounting SPECFS filesystem: "
	    eval "mount -t specfs ${SPECFS_OPTIONS} -- specfs \"$SPECFS_MOUNTPOINT\" $redir"
	    RETVAL=$? ; if [ $RETVAL -ne 0 ] ; then echo "(failed.)" ; return $RETVAL ; fi
	    echo "$SPECFS_MOUNTPOINT."
	fi
    fi
    return 0
}

stop() {
    if mount | grep " on [^[:space:]]* type specfs" >/dev/null 2>&1 ; then
	echo -n "Unmounting SPECFS filesystem: "
	mount | grep " on [^[:space:]]* type specfs" 2>/dev/null | \
	while read line ; do
	    set $line
	    echo -n "$3 "
	    eval "umount -- \"$3\" $redir"
	    if [ $? -ne 0 ] ; then echo -n "(failed) " ; continue ; fi
	done
	echo "."
    fi
    if [ -n "$SPECFS_MOUNTPOINT" -a -d "$SPECFS_MOUNTPOINT" ] ; then
	echo -n "Removing SPECFS mount point: "
	eval "rmdir -- \"$SPECFS_MOUNTPOINT\" $redir"
	if [ $? -ne 0 ] ; then echo -n "(failed) " ; else echo "$SPECFS_MOUNTPOINT." ; fi
    fi
    if grep -qc '[[:space:]]specfs\>' /proc/filesystems ; then
	    echo -n "Removing SPECFS kernel modules: specfs "
	    eval "modprobe -r -q -- specfs $redir" || :
	    if grep -qc '[[:space:]]specfs\>' /proc/filesystems ; then
		echo "(failed.)"
		return 1
	    else
		echo "."
	    fi
    fi
    return 0
}

restart() {
    stop
    start
    return $?
}

reload() {
    [ -n "$SPECFS_MOUNTPOINT" ] || return 0
    echo -n "Remounting SPECFS filesystem: "
    if mount | grep ' on [^[:space:]]* type specfs' 1>/dev/null 2>&1 ; then
	build_options remount
	eval "mount -t specfs ${SPECFS_OPTIONS} -- specfs \"$SPECFS_MOUNTPOINT\" $redir"
	RETVAL=$? ; if [ $RETVAL -ne 0 ] ; then echo "(failed.)" ; return $RETVAL ; fi
	echo "$SPECFS_MOUNTPOINT."
	return 0
    else
	echo "(failed)."
	return 1
    fi
}

show() {
    echo "$name.sh: show: not yet implemented." >&2
    return 1
}

usage() {
    echo "Usage: /etc/init.d/$name.sh (start|stop|restart|reload|force-reload|show)" >&2
    return 1
}

case "$1" in
    (start|stop|restart|reload|show)
	$1 || RETVAL=$?
	;;
    (force-reload)
	reload || RETVAL=$?
	;;
    (*)
	usage || RETVAL=$?
	;;
esac

[ "${0##*/}" = "$name.sh" ] && exit $RETVAL

# =============================================================================
# 
# @(#) $RCSfile: specfs.sh,v $ $Name:  $($Revision: 1.1.2.2 $) $Date: 2011-01-12 00:19:32 $
#
# -----------------------------------------------------------------------------
#
# Copyright (c) 2001-2011  OpenSS7 Corporation <http://www.openss7.com/>
# Copyright (c) 1997-2000  Brian F. G. Bidulock <bidulock@openss7.org>
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
# Last Modified $Date: 2011-01-12 00:19:32 $ by $Author: brian $
#
# -----------------------------------------------------------------------------
#
# $Log: specfs.sh,v $
# Revision 1.1.2.2  2011-01-12 00:19:32  brian
# - modprobe no longer accepts k option
#
# Revision 1.1.2.1  2009-06-21 11:47:57  brian
# - added files to new distro
#
# Revision 0.9.2.15  2008-10-21 22:42:15  brian
# - handle verbose better in debian initscripts
#
# Revision 0.9.2.14  2008-04-28 12:54:11  brian
# - update file headers for release
#
# Revision 0.9.2.13  2007/08/13 22:46:39  brian
# - GPLv3 header updates
#
# =============================================================================

# vim: ft=sh sw=4 tw=80
