/*****************************************************************************

 @(#) File: src/drivers/m3ua_sgp.c

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2015  Monavacon Limited <http://www.monavacon.com/>
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

 *****************************************************************************/

static char const ident[] = "src/drivers/m3ua_sgp.c (" PACKAGE_ENVR ") " PACKAGE_DATE;

#define __NO_VERSION__

#include <sys/os7/compat.h>

#include "../../include/ss7/mtp.h"
#include "../mtp/mtp_prov.h"

#include "../ua/ua_data.h"
#include "m3ua_data.h"
#include "../ua/ua_msg.h"
#include "m3ua_msg.h"
#include "../ua/ua_asp.h"

/*
 *  =========================================================================
 *
 *  M3UA Peer --> M3UA (SGP) Received Messages
 *
 *  =========================================================================
 */
/*
 *  XFER DATA (Pass along as M_DATA)
 *  -------------------------------------------------------------------------
 */
static int
m3ua_recv_data(sgp_t * sgp, mblk_t *msg)
{
	uint32_t tag, *p = ((uint32_t *) msg->b_rptr) + 4;
	size_t len = UA_SIZE(*p);

	tag = *p++ & UA_TAG_MASK;
	(void) len;
	(void) tag;
	/* 
	 *  FIXME:
	 */
	return (-EFAULT);
}

/*
 *  SNMM
 *  -------------------------------------------------------------------------
 *  Some routines for distributing SNMM messages.
 */
/*
 *  Distribute SNMM primitives to MTP for a range of Routing Contexts.
 */
static int
m3ua_distribute(sgp_t * sgp, mblk_t *mp, parm_t * rc)
{
	int i;
	mblk_t *dp, *ep;

	for (i = 0; i < rc->len >> 2; i++) {
		gp_t *gp;
		uint found = 0;
		uint32_t rcval = ntohl(rc->u.wptr[i]);

		for (gp = Q_SP(q)->gp; gp; gp = gp->as_next) {
			as_t *as = gp->as;

			if (!rc->len || as->list->id == rcval) {
				if (!(dp = dupmsg(mp))) {
					freemsg(mp);	/* free original */
					return (-ENOBUFS);
				}
				putq(as->list->lp->q, dp);
				found = 1;
			}
		}
		if (rc->len && !found) {
			ep = ua_send_err(UA_ECODE_INVALID_ROUTING_CONTEXT,
					 (caddr_t) &rc->u.wptr[i], 4);
			if (!ep)
				return (-ENOBUFS);
			ptrace(("Error: DUNA: invalid Routing Context\n"));
			qreply(q, ep);
		}
	}
	freemsg(mp);		/* free original */
	return (0);
}
static int
m3ua_snmm_parm_check(sgp_t * sgp, mblk_t *msg, m3ua_parms_t * parms)
{
	mblk_t *ep;
	size_t elen = min(40, msgdsize(msg));
	parm_t *rc = &parms->common.rc;
	parm_t *ad = &parms->affect_dest;

	if (!ad->u.wptr) {
		ep = ua_send_err(UA_ECODE_PROTOCOL_ERROR, msg->b_rptr, elen);
		if (!ep)
			return (-ENOBUFS);
		ptrace(("Error: SNMM: missing mandatory parm Affected Desination\n"));
		qreply(q, ep);
		return (-EINVAL);
	}
	if (ad->len < sizeof(uint32_t)) {
		ep = ua_send_err(UA_ECODE_INVALID_PARAMETER_VALUE, ad->u.cptr - 4, ad->len + 4);
		if (!ep)
			return (-ENOBUFS);
		ptrace(("Error: SNMM: parm Affected Desination too short: %d bytes\n", ad->len));
		qreply(q, ep);
		return (-EINVAL);
	}
	if (rc->u.wptr && rc->len < sizeof(uint32_t)) {
		ep = ua_send_err(UA_ECODE_INVALID_PARAMETER_VALUE, (caddr_t) rc->u.wptr - 4,
				 ad->len + 4);
		if (!ep)
			return (-ENOBUFS);
		ptrace(("Error: SNMM: parm Routing Context too short: %d bytes\n", rc->len));
		qreply(q, ep);
		return (-EINVAL);
	}
	return (0);
}
static int
m3ua_not_sg(sgp_t * sgp, mblk_t *msg)
{
	mblk_t *ep;

	if (Q_MODE(q) & Q_MODE_SG) {
		uint elen = min(40, msgdsize(msg));

		ep = ua_send_err(UA_ECODE_UNEXPECTED_MESSAGE, msg->b_rptr, elen);
		if (!ep)
			return (-ENOBUFS);
		qreply(q, ep);
		return (-EINVAL);
	}
	return (0);
}

/*
 *  RKMM REG REQ
 *  -------------------------------------------------------------------------
 */
static int
m3ua_recv_reg_req(sgp_t * sgp, mblk_t *msg)
{
	(void) q;
	(void) msg;
	return (-EOPNOTSUPP);
}

/*
 *  RKMM DEREG REQ
 *  -------------------------------------------------------------------------
 */
static int
m3ua_recv_dereg_req(sgp_t * sgp, mblk_t *msg)
{
	(void) q;
	(void) msg;
	return (-EOPNOTSUPP);
}

/*
 *  Common defined ASP/SGP management procedures.
 */
extern int (*ua_sgp_mgmt[]) (sgp_t *, mblk_t *);
extern int (*ua_sgp_asps[]) (sgp_t *, mblk_t *);
extern int (*ua_sgp_aspt[]) (sgp_t *, mblk_t *);

static int (*m3ua_xfer[]) (sgp_t *, mblk_t *) = {
	NULL,			/* (reserved) 0x0 */
	    m3ua_recv_data	/* M3UA_XFER_DATA 0x1 */
};
static int (*m3ua_snmm[]) (sgp_t *, mblk_t *) = {
	NULL,			/* (reserved) 0x0 */
	    m3ua_recv_duna,	/* UA_SNMM_DUNA 0x1 */
	    m3ua_recv_dava,	/* UA_SNMM_DAVA 0x2 */
	    m3ua_recv_daud,	/* UA_SNMM_DAUD 0x3 */
	    m3ua_recv_scon,	/* UA_SNMM_SCON 0x4 */
	    m3ua_recv_dupu,	/* UA_SNMM_DUPU 0x5 */
	    m3ua_recv_drst	/* UA_SNMM_DRST 0x6 */
};
static int (*m3ua_rkmm[]) (sgp_t *, mblk_t *) = {
	NULL,			/* (reserved) 0x0 */
	    m3ua_recv_reg_req,	/* UA_RKMM_REG_REQ 0x1 */
	    NULL,		/* UA_RKMM_REG_RSP 0x2 */
	    m3ua_recv_dereg_req,	/* UA_RKMM_DEREG_REQ 0x3 */
	    NULL		/* UA_RKMM_DEREG_RSP 0x4 */
};

static struct msg_class msg_decode[] = {
	{ua_sgp_mgmt, UA_MGMT_LAST},	/* UA_CLASS_MGMT 0x0 */
	{m3ua_xfer, M3UA_XFER_LAST},	/* UA_CLASS_XFER 0x1 */
	{NULL, 0},		/* UA_CLASS_SNMM 0x2 */
	{ua_sgp_asps, UA_ASPS_LAST},	/* UA_CLASS_ASPS 0x3 */
	{ua_sgp_aspt, UA_ASPT_LAST},	/* UA_CLASS_ASPT 0x4 */
	{NULL, 0},		/* UA_CLASS_Q921 0x5 */
	{NULL, 0},		/* UA_CLASS_MAUP 0x6 */
	{NULL, 0},		/* UA_CLASS_CNLS 0x7 */
	{NULL, 0},		/* UA_CLASS_CONS 0x8 */
	{m3ua_rkmm, UA_RKMM_LAST},	/* UA_CLASS_RKMM 0x9 */
	{NULL, 0},		/* UA_CLASS_TDHM 0xa */
	{NULL, 0}		/* UA_CLASS_TCHM 0xb */
};

int
m3ua_sgp_recv_msg(sgp_t * sgp, mblk_t *mp)
{
	return ua_recv_msg(q, mp, msg_decode);
}
