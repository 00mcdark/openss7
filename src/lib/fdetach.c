/*****************************************************************************

 @(#) $RCSfile$ $Name$($Revision$) $Date$

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2009  Monavacon Limited <http://www.monavacon.com/>
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

 -----------------------------------------------------------------------------

 Last Modified $Date$ by $Author$

 -----------------------------------------------------------------------------

 $Log$
 *****************************************************************************/

#ident "@(#) $RCSfile$ $Name$($Revision$) $Date$"

static char const ident[] = "$RCSfile$ $Name$($Revision$) $Date$";

/* This file can be processed with doxygen(1). */

#include "streams.h"

/** @weakgroup strcalls STREAMS System Calls
  * @{ */

/** @file
  * STREAMS Library fdetach() implementation file. */

#define DUMMY_STREAM "/dev/streams/clone/nuls"
#define DUMMY_MODE   O_RDWR|O_NONBLOCK

/** @brief Detach a path from a Stream.
  * @param path the path in the filesystem from which to detach.
  * @version STREAMS_1.0
  */
int __unlikely
__streams_fdetach(const char *path)
{
	int ffd, error = 0;

	if ((ffd = open(DUMMY_STREAM, DUMMY_MODE)) < 0)
		return -1;
	if (ioctl(ffd, I_FDETACH, path) < 0)
		error = errno;
	close(ffd);
	if (error) {
		errno = error;
		return -1;
	}
	return 0;
}

/** @brief Detach a path from a Stream.
  * @param path the path in the filesystem from which to detach.
  * @version STREAMS_1.0 fdetach()
  *
  * fdetach() cannot contain a thread cancellation point (SUS/XOPEN/POSIX).  We
  * must protect from asyncrhonous cancellation between the open(), ioctl() and
  * close() operations.  */
int __unlikely
__streams_fdetach_r(const char *path)
{
	int oldtype, ret;

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldtype);
	ret = __streams_fdetach(path);
	pthread_setcanceltype(oldtype, NULL);
	return (ret);
}

/** @fn int fdetach(const char *path)
  * @brief Detach a path from a Stream.
  * @param path the path in the filesystem from which to detach.
  * @version STREAMS_1.0 __streams_fdetach_r()
  */
__asm__(".symver __streams_fdetach_r,fdetach@@STREAMS_1.0");

/** @} */

// vim: com=srO\:/**,mb\:*,ex\:*/,srO\:/*,mb\:*,ex\:*/,b\:TRANS
