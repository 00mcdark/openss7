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


#include <sys/os7/compat.h>

#include "sua.h"
#include "sua_data.h"

/*
 *  =========================================================================
 *
 *  SCCP/NPI/TPI --> SUA (Upstream Primitives received from downstream)
 *
 *  =========================================================================
 */

/* from sua_nsp.c */
extern int nsp_r_data(queue_t *, mblk_t *);
extern int nsp_r_proto(queue_t *, mblk_t *);
extern int nsp_r_ctl(queue_t *, mblk_t *);
extern int nsp_r_error(queue_t *, mblk_t *);
extern int nsp_r_flush(queue_t *, mblk_t *);
extern int nsp_r_hangup(queue_t *, mblk_t *);

/* from sua_tsp.c */
extern int tsp_r_data(queue_t *, mblk_t *);
extern int tsp_r_proto(queue_t *, mblk_t *);
extern int tsp_r_ctl(queue_t *, mblk_t *);
extern int tsp_r_error(queue_t *, mblk_t *);
extern int tsp_r_flush(queue_t *, mblk_t *);
extern int tsp_r_hangup(queue_t *, mblk_t *);

/* from sua_ss7.c */
extern int ss7_r_data(queue_t *, mblk_t *);
extern int ss7_r_proto(queue_t *, mblk_t *);
extern int ss7_r_ctl(queue_t *, mblk_t *);
extern int ss7_r_error(queue_t *, mblk_t *);
extern int ss7_r_flush(queue_t *, mblk_t *);
extern int ss7_r_hangup(queue_t *, mblk_t *);

/*
 *  =========================================================================
 *
 *  QUEUE PUT and SERVICE routines
 *
 *  =========================================================================
 */

static inline int
sua_recover(queue_t *q, mblk_t *mp, int err)
{
	switch (err) {
	case -EBUSY:
	case -EAGAIN:
	case -ENOMEM:
	case -ENOBUFS:
		putq(q, mp);
		return (0);
	}
	freemsg(mp);
	return (err);
}
static inline int
sua_reservice(queue_t *q, mblk_t *mp, int err)
{
	if (mp->b_datap->db_type < QPCTL)
		switch (err) {
		case -EBUSY:
		case -EAGAIN:
		case -ENOMEM:
		case -ENOBUFS:
			putbq(q, mp);
			return (0);
		}
	freemsg(mp);
	return (err);
}

/*
 *  READ PUT and SERVICE (Message from below SCCP/NPI/TPI --> SUA)
 *  -------------------------------------------------------------------------
 */
static inline int
sua_rd(queue_t *q, mblk_t *mp)
{
	switch (Q_TYPE(q)) {
	case Q_TYPE_NPI:
		switch (mp->b_datap->db_type) {
		case M_DATA:
			return nsp_r_data(q, mp);
		case M_PROTO:
		case M_PCPROTO:
			return nsp_r_proto(q, mp);
		case M_CTL:
			return nsp_r_ctl(q, mp);
		case M_ERROR:
			return nsp_r_error(q, mp);
		case M_FLUSH:
			return nsp_r_flush(q, mp);
		case M_HANGUP:
			return nsp_r_hangup(q, mp);
		}
		return (-EOPNOTSUPP);
	case Q_TYPE_TPI:
		switch (mp->b_datap->db_type) {
		case M_DATA:
			return tsp_r_data(q, mp);
		case M_PROTO:
		case M_PCPROTO:
			return tsp_r_proto(q, mp);
		case M_CTL:
			return tsp_r_ctl(q, mp);
		case M_ERROR:
			return tsp_r_error(q, mp);
		case M_FLUSH:
			return tsp_r_flush(q, mp);
		case M_HANGUP:
			return tsp_r_hangup(q, mp);
		}
		return (-EOPNOTSUPP);
	case Q_TYPE_SS7:
		switch (mp->b_datap->db_type) {
		case M_DATA:
			return ss7_r_data(q, mp);
		case M_PROTO:
		case M_PCPROTO:
			return ss7_r_proto(q, mp);
		case M_CTL:
			return ss7_r_ctl(q, mp);
		case M_ERROR:
			return ss7_r_error(q, mp);
		case M_FLUSH:
			return ss7_r_flush(q, mp);
		case M_HANGUP:
			return ss7_r_hangup(q, mp);
		}
		return (-EOPNOTSUPP);
	}
	return (-EFAULT);
}

int
sua_l_rput(queue_t *q, mblk_t *mp)
{
	int err;

	if (mp->b_datap->db_type < QPCTL && (q->q_count || !canputnext(q))) {
		putq(q, mp);
		return (0);
	}
	if ((err = sua_rd(q, mp)))
		return (sua_recover(q, mp, err));
	return (0);
}

int
sua_l_rsrv(queue_t *q)
{
	mblk_t *mp;

	while ((mp = getq(q))) {
		int err;

		if (!(err = sua_rd(q, mp)))
			continue;
		if (mp->b_datap->db_type < QPCTL)
			return (sua_reservice(q, mp, err));
		freemsg(mp);
		return (err);
	}
	return (0);
}
