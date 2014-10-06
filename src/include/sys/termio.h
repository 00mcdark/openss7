/*****************************************************************************

 @(#) $Id: termio.h,v 1.1.2.2 2010-11-28 14:21:49 brian Exp $

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

 Last Modified $Date: 2010-11-28 14:21:49 $ by $Author: brian $

 -----------------------------------------------------------------------------

 $Log: termio.h,v $
 Revision 1.1.2.2  2010-11-28 14:21:49  brian
 - remove #ident, protect _XOPEN_SOURCE

 Revision 1.1.2.1  2009-06-21 11:25:38  brian
 - added files to new distro

 *****************************************************************************/

#ifndef __SYS_TERMIO_H__
#define __SYS_TERMIO_H__

#include <sys/termios.h>

struct termcb {
	char st_flgs;			/* term flags */
	char st_termt;			/* terminal type */
	char st_crow;			/* gtty only - current row */
	char st_ccol;			/* gtty only - current col */
	char st_vrow;			/* variable row */
	char st_lrow;			/* last row */
};

/* term flags - st_flags */
#define TM_NONE		0000	/* use default flags */
#define TM_SNL		0001	/* special newline flag */
#define TM_ANL		0002	/* auto newline on column 80 */
#define TM_LCF		0004	/* last col of last row special */
#define TM_CECHO	0010	/* echo terminal cursor control */
#define TM_CINVIS	0020	/* do not sent esc seq to user */
#define TM_SET		0200	/* must be on to set/res flags */

/* term types - st_termt */
#define TERM_NONE	0	/* tty */
#define TERM_TEC	1	/* TEC Scope */
#define TERM_V61	2	/* DEC VT61 */
#define TERM_V10	3	/* DEC VT100 */
#define TERM_TEX	4	/* Tektronix 4023 */
#define TERM_D40	5	/* TTY Model 40/1 */
#define TERM_H45	6	/* Hewlett-Packard 45 */
#define TERM_D42	7	/* TTY Model 40/2B */

#define LDIOC	('D'<<8)
#define LDOPEN	(LDIOC|0)
#define LDCLOSE	(LDIOC|1)
#define LDCHG	(LDIOC|2)
#define LDGETT	(LDIOC|8)
#define LDSETT	(LDIOC|9)

#undef NCC
#define NCC	10

#ifndef _LINUX_TERMIOS_H
struct termio {
	unsigned short c_iflag;		/* input modes */
	unsigned short c_oflag;		/* output modes */
	unsigned short c_cflag;		/* control modes */
	unsigned short c_lflag;		/* line discipline modes */
	char c_line;			/* line discipline */
	unsigned char c_cc[NCC];	/* control chars */
};
#endif				/* _LINUX_TERMIOS_H */

#endif				/* __SYS_TERMIO_H__ */
