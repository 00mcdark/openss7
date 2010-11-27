/*****************************************************************************

 @(#) $Id: ll_control.h,v 1.1.2.1 2009-06-21 11:25:32 brian Exp $

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

 Last Modified $Date: 2009-06-21 11:25:32 $ by $Author: brian $

 -----------------------------------------------------------------------------

 $Log: ll_control.h,v $
 Revision 1.1.2.1  2009-06-21 11:25:32  brian
 - added files to new distro

 *****************************************************************************/

#ifndef __SYS_SNET_LL_CONTROL_H__
#define __SYS_SNET_LL_CONTROL_H__

#include <sys/types.h>
#include <stdint.h>

struct lapbtune {
	uint16_t N2;
	uint16_t T1;
	uint16_t Tpf;
	uint16_t Trej;
	uint16_t Tbusy;
	uint16_t Tidle;
	uint16_t ack_delay;
	uint16_t notack_max;
	uint16_t tx_window;
	uint16_t tx_probe;
	uint16_t max_I_len;
	uint16_t llconform;
	uint16_t sabm_in_x32;
};

struct lapb_tnioc {
	uint8_t lli_type;		/* LI_LAPBTUNE */
	uint8_t lli_spare[3];
	uint32_t lli_snid;
	struct lapbtune lapb_tune;
};

struct llc2tune {
	uint16_t N2;
	uint16_t T1;
	uint16_t Tpf;
	uint16_t Trej;
	uint16_t Tbusy;
	uint16_t Tidle;
	uint16_t ack_delay;
	uint16_t notack_max;
	uint16_t tx_window;
	uint16_t tx_probe;
	uint16_t max_I_len;
	uint16_t xid_window;
	uint16_t xid_Ndup;
	uint16_t xid_Tdup;
};

struct llc2_tnioc {
	uint8_t lli_type;		/* LI_LLC2TUNE */
	uint8_t lli_spare[3];
	uint32_t lli_snid;
	struct llc2tune llc2_tune;
};

#endif				/* __SYS_SNET_LL_CONTROL_H__ */
