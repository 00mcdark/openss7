/*****************************************************************************

 @(#) $Id: sgtty.h,v 1.1.2.1 2009-06-21 11:25:38 brian Exp $

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2010  Monavacon Limited <http://www.monavacon.com/>
 Copyright (c) 2001-2008  OpenSS7 Corporation <http://www.openss7.com/>
 Copyright (c) 1997-2001  Brian F. G. Bidulock <bidulock@openss7.org>

 All Rights Reserved.

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU Affero General Public License as published by the Free
 Software Foundation; version 3 of the License.

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

 Last Modified $Date: 2009-06-21 11:25:38 $ by $Author: brian $

 -----------------------------------------------------------------------------

 $Log: sgtty.h,v $
 Revision 1.1.2.1  2009-06-21 11:25:38  brian
 - added files to new distro

 *****************************************************************************/

#ifndef __SYS_SGTTY_H__
#define __SYS_SGTTY_H__

struct sgttyb {
	char sg_ispeed;			/* input speed */
	char sg_ospeed;			/* output speed */
	char sg_erase;			/* erase character */
	char sg_kill;			/* kill character */
	int sg_flags;			/* mode flags */
};

/* Old System V modes */
#define O_HUPCL		0000001
#define O_XTABS		0000002
#define O_LCASE		0000004
#define O_ECHO		0000010
#define O_CRMOD		0000020
#define O_RAW		0000040
#define O_ODDP		0000100
#define O_EVENP		0000200
#define O_ANYP		0000300
#define O_NLDELAY	0001400
#define O_NL1		0000400
#define O_NL2		0001000
#define O_TBDELAY	0002000
#define O_NOAL		0004000
#define O_CRDELAY	0030000
#define O_CR1		0010000
#define O_CR2		0020000
#define O_VTDELAY	0040000
#define O_BSDELAY	0100000

#define TIOCGETP	(('T'<<8)| 8)	/* read struct sgttyb */
#define TIOCSETP	(('T'<<8)| 9)	/* write struct sgttyb */
#define TIOCSETN	(('T'<<8)|10)	/* write struct sgttyb, no flush tty */

#endif				/* __SYS_SGTTY_H__ */
