/*****************************************************************************

 @(#) $Id$

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2009  Monavacon Limited <http://www.monavacon.com/>
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

 Last Modified $Date$ by $Author$

 -----------------------------------------------------------------------------

 $Log$
 *****************************************************************************/

#ifndef __SYS_TP_H__
#define __SYS_TP_H__

#ident "@(#) $RCSfile$ $Name$($Revision$) Copyright (c) 2008-2009 Monavacon Limited."

struct tp_mgmt {
	t_uscalar_t cm_user;		/* User index and class. */
	t_uscalar_t cm_prov;		/* Provider index. */
	union T_primitives cm_prim;	/* Primitive. */
};

/* Actually, this information is probably embedded in the T_CONN_REQ. */

/* Mask for cm_user field for protocol class. */
#define TP_MGMT_CLASS		0xff000000

/* Protocol classes in the cm_user field. */
#define TP_MGMT_CLASS_0		0x01000000
#define TP_MGMT_CLASS_1		0x02000000
#define TP_MGMT_CLASS_2		0x04000000
#define TP_MGMT_CLASS_3		0x08000000
#define TP_MGMT_CLASS_4_CONS	0x10000000
#define TP_MGMT_CLASS_4_CLNS	0x20000000
#define TP_MGMT_CLTS		0x40000000

#endif				/* __SYS_TP_H__ */
