/*****************************************************************************

 @(#) $RCSfile: sscop_ip.c,v $ $Name:  $($Revision: 1.1.2.2 $) $Date: 2010-11-28 14:21:36 $

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2010  Monavacon Limited <http://www.monavacon.com/>
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

 Last Modified $Date: 2010-11-28 14:21:36 $ by $Author: brian $

 -----------------------------------------------------------------------------

 $Log: sscop_ip.c,v $
 Revision 1.1.2.2  2010-11-28 14:21:36  brian
 - remove #ident, protect _XOPEN_SOURCE

 Revision 1.1.2.1  2009-06-21 11:20:54  brian
 - added files to new distro

 *****************************************************************************/

static char const ident[] = "$RCSfile: sscop_ip.c,v $ $Name:  $($Revision: 1.1.2.2 $) $Date: 2010-11-28 14:21:36 $";

/*
 *  This driver provides the functionality of SSCOP-MCE/IP as specified in
 *  Q.2111.  It uses only the IP network provider mode of SSCOP-MCE.
 */

#include <sys/os7/compat.h>

#define SSCOP_DESCRIP	"SSCOP-MCE/IP STREAMS DRIVER."
#define SSCOP_REVISION	"OpenSS7 $RCSfile: sscop_ip.c,v $ $Name:  $ ($Revision: 1.1.2.2 $) $Date: 2010-11-28 14:21:36 $"
#define SSCOP_COPYRIGHT	"Copyright (c) 2008-2010  Monavacon Limited.  All Rights Reserved."
#define SSCOP_DEVICE	"Part of the OpenSS7 Stack for Linux Fast-STREAMS."
#define SSCOP_CONTACT	"Brian Bidulock <bidulock@openss7.org>"
#define SSCOP_LICENSE	"GPL"
#define SSCOP_BANNER	SSCOP_DESCRIP	"\n"	\
			SSCOP_REVISION	"\n"	\
			SSCOP_COPYRIGHT	"\n"	\
			SSCOP_DEVICE	"\n"	\
			SSCOP_CONTACT	"\n"	\
#define SSCOP_SPLASH	SSCOP_DEVICE	" - "	\
			SSCOP_REVISION	"\n"

#ifdef LINUX
MODULE_AUTHOR(SSCOP_CONTACT);
MODULE_DESCRIPTION(SSCOP_DESCRIP);
MODULE_SUPPORTED_DEVICE(SSCOP_DEVICE);
#ifdef MODULE_LICENSE
MODULE_LICENSE(SSCOP_LICENSE);
#endif
#if defined MODULE_ALIAS
MODULE_ALIAS("streams-sscop_ip");
#endif
#ifdef MODULE_VERSION
MODULE_VERSION(__stringify(PACKAGE_RPMEPOCH) ":" PACKAGE_VERSION "." PACKAGE_RELEASE
	       PACKAGE_PATCHLEVEL "-" PACKAGE_RPMRELEASE PACKAGE_RPMEXTRA2);
#endif
#endif				/* LINUX */

#ifdef SSCOP_DEBUG
static int sscop_debug = SSCOP_DEBUG;
#else
static int sscop_debug = 2;
#endif

#define SSCOP_MCE_DRV_ID	CONFIG_STREAMS_SSCOP_MCE_MODID
#define SSCOP_MCE_DRV_NAME	CONFIG_STREAMS_SSCOP_MCE_NAME
#define SSCOP_MCE_CMAJORS	CONFIG_STREAMS_SSCOP_MCE_NMAJORS
#define SSCOP_MCE_CMAJOR_0	CONFIG_STREAMS_SSCOP_MCE_MAJOR

#ifndef SSCOP_MCE_CMAJOR_0
#define SSCOP_MCE_CMAJOR_0 201
#endif
#define SSCOP_MCE_NMINOR 255

/*
 *  =========================================================================
 *
 *  STREAMS Definitions
 *
 *  =========================================================================
 */

static struct module_info sscop_minfo = {
	0,				/* Module Id number */
	"sscop-mce/ip",			/* Module name */
	1,				/* Min packet size accepted */
	512,				/* Max packet size accepted */
	8 * 512,			/* Hi water mark *//* XXX */
	1 * 512				/* Lo water mark *//* XXX */
};

static int sscop_open(queue_t *, dev_t *, int, int, cred_t *);
static int sscop_close(queue_t *, int, cred_t *);
static int sscop_rput(queue_t *, mblk_t *);
static int sscop_rsrv(queue_t *);
static int sscop_wput(queue_t *, mblk_t *);
static int sscop_wsrv(queue_t *);

static struct qinit sscop_rinit = {
	sscop_rput,			/* Read put (msg from below) */
	sscop_rsrv,			/* Read queue service */
	sscop_open,			/* Each open */
	sscop_close,			/* Last close */
	NULL,				/* Admin (not used) */
	&sscop_minfo,			/* Information */
	NULL				/* Statistics */
};

static struct qinit sscop_winit = {
	sscop_wput,			/* Write put (msg from above) */
	sscop_wsrv,			/* Write queue service */
	NULL,				/* Each open */
	NULL,				/* Last close */
	NULL,				/* Admin (not used) */
	NULL				/* Statistics */
};

MODULE_STATIC struct streamtab sscop_info = {
	&sscop_rinit,			/* Upper read queue */
	&sscop_winit,			/* Upper write queue */
	NULL,				/* Lower read queue */
	NULL				/* Lower write queue */
};

/*
 *  =========================================================================
 *
 *  SSCOP Private Structures
 *
 *  =========================================================================
 */
#define Q_SSCOP(__q) ((sscop_t *)((__q)->q_ptr))

/*
 *  =========================================================================
 *
 *  SSCOP --> User (Upstream N-Primitives)
 *
 *  =========================================================================
 */
#include "../npi/npi_prov.h"

/*  N_CONN_IND		(11 - Incoming connection indication)		*/
/*  --------------------------------------------------------------------*/
static inline mblk_t *
aa_conn_ind(uint flags, uint seq, uint32_t daddr, uint32_t saddr)
{
	return n_conn_ind(flags, seq, &daddr, sizeof(daddr), &saddr, sizeof(saddr), NULL, 0);
}

/*  N_CONN_CON		(12 - Connection established)			*/
/*  --------------------------------------------------------------------*/
static inline mblk_t *
aa_conn_con(uint flags, uint32_t raddr)
{
	return n_conn_con(flags, &raddr, sizeof(raddr), NULL, 0);
}

/*  N_DISCON_IND	(13 - NC disconnected)				*/
/*  --------------------------------------------------------------------*/
static inline mblk_t *
aa_discon_ind(uint seq, uint orig, uint reason, uint32_t raddr)
{
	return n_discon_ind(seq, orig, reason, &raddr, sizeof(raddr));
}

/*  N_DATA_IND		(14 - Incoming connection-mode data indication)	*/
/*  --------------------------------------------------------------------*/
static inline mblk_t *
aa_data_ind(uint flags, mblk_t *dp)
{
	return n_data_ind(flags, dp);
}

/*  N_EXDATA_IND	(15 - Incoming expedited data indication)	*/
/*  --------------------------------------------------------------------*/
static inline mblk_t *
aa_exdata_ind(mblk_t *dp)
{
	return n_exdata_ind(dp);
}

/*  N_INFO_ACK		(16 - Information Acknowledgement)		*/
/*  --------------------------------------------------------------------*/
static inline mblk_t *
aa_info_ack(queue_t *q)
{
	sscop_t *sp = Q_SSCOP(q);
	mblk_t *mp;
	N_info_ack_t *p;

	if ((mp = allocb(sizeof(*p) + add_len + qos_len + qor_len, BRPI_MED))) {
		mp->b_datap->db_type = M_PCPROTO;
		p = (N_info_ack_t *) mp->b_wptr;
		p->PRIM_type = N_INFO_ACK;
		p->NSDU_size = sp->nsdu;
		p->ENSDU_size = sp->ensdu;
		p->CDATA_size = sp->nsdu;
		p->DDATA_size = sp->nsdu;
		p->ADDR_size = sizeof(uint32_t);
		p->ADDR_length = sizeof(uint32_t);
		p->ADDR_offset = sizeof(*p);
		p->QOS_length = 0;
		p->QOS_offset = 0;
		p->QOS_range_length = 0;
		p->QOS_range_offset = 0;
		p->OPTIONS_flags = REC_CONF_OPT | EX_DATA_OPT;
		p->NIDU_size = sp->nidu;
		p->SERV_type = N_CONS | N_CLNS;
		p->CURRENT_state = sp->state;
		p->PROVIDER_type = N_SNICFP;
		p->NODU_size = sp->nodu;
		p->PROTOID_length = sizeof(uint32_t);
		p->PROTOID_offset = sizeof(*p) + sizeof(uint32_t);
		p->NPI_version = N_CURRENT_VERSION;
		mp->b_wptr += sizeof(*p);
		*((uint32_t *) mp->b_wptr)++ = sp->baddr;
		*((uint32_t *) mp->b_wptr)++ = sp->protid;
	}
	return (mp);
}

/*  N_BIND_ACK		(17 - NS User bound to network address)		*/
/*  --------------------------------------------------------------------*/
static inline mblk_t *
aa_bind_ack(uint cons, uint token, uint32_t baddr, uint32_t protoid)
{
	return n_bind_ack(cons, token, &baddr, sizeof(baddr), &protoid, sizeof(protoid));
}

/*  N_ERROR_ACK		(18 - Error Acknowledgement)			*/
/*  --------------------------------------------------------------------*/
static inline mblk_t *
aa_error_ack(int prim, int err)
{
	return n_error_ack(prim, err);
}

/*  N_OK_ACK		(19 - Success Acknowledgement)			*/
/*  --------------------------------------------------------------------*/
static inline mblk_t *
aa_ok_ack(int prim)
{
	return n_ok_ack(prim);
}

/*  N_UNITDATA_IND	(20 - Connection-less data receive indication)	*/
/*  --------------------------------------------------------------------*/
static inline mblk_t *
aa_unitdata_ind(uint32_t daddr, uint32_t saddr, mblk_t *dp)
{
	return n_unitdata_ind(&daddr, sizeof(daddr), &saddr, sizeof(saddr), dp);
}

/*  N_UDERROR_IND	(21 - UNITDATA Error Indication)		*/
/*  --------------------------------------------------------------------*/
static inline mblk_t *
aa_uderror_ind(uint ecode, uint32_t daddr)
{
	return n_uderror_ind(ecode, &daddr, sizeof(daddr));
}

/*  N_DATACK_IND	(24 - Data acknowledgement indication)		*/
/*  --------------------------------------------------------------------*/
static inline mblk_t *
aa_datack_ind(void)
{
	return n_datack_ind();
}

/*  N_RESET_IND		(26 - Incoing NC reset request indication)	*/
/*  --------------------------------------------------------------------*/
static inline mblk_t *
aa_reset_ind(uint orig, uint reason)
{
	return n_reset_ind(orig, reason);
}

/*  N_RESET_CON		(28 - Reset processing complete)		*/
/*  --------------------------------------------------------------------*/
static inline mblk_t *
aa_reset_con(void)
{
	return n_reset_con();
}

/*  N_RECOVER_IND	(29 - NC Recovery indication)			*/
/*  --------------------------------------------------------------------*/
static inline mblk_t *
aa_recover_ind(void)
{
	return n_recover_ind();
}

/*  N_RETRIEVE_IND	(32 - NC Retrieval indication)			*/
/*  --------------------------------------------------------------------*/
static inline mblk_t *
aa_retrieve_ind(mblk_t *dp)
{
	return n_retrieve_ind(dp);
}

/*  N_RETRIEVE_CON	(33 - NC Retrieval complete confirmation)	*/
/*  --------------------------------------------------------------------*/
static inline mblk_t *
aa_retrieve_con(void)
{
	return n_retrieve_con();
}

/*
 *  =========================================================================
 *
 *  User --> SCCOP (Downstream N-Primitives)
 *
 *  =========================================================================
 */
/*
 *  N_CONN_REQ		(0 - NC request)
 *  -------------------------------------------------------------------------
 */
static int
aa_conn_req(queue_t *q, mblk_t *mp)
{
	(void) q;
	(void) mp;
	return (-EOPNOTSUPP);
}

/*
 *  N_CONN_RES		(1 - Accept prevsious connection indication)
 *  -------------------------------------------------------------------------
 */
static int
aa_conn_res(queue_t *q, mblk_t *mp)
{
	(void) q;
	(void) mp;
	return (-EOPNOTSUPP);
}

/*
 *  N_DISCON_REQ	(2 - NC disconnection request)
 *  -------------------------------------------------------------------------
 */
static int
aa_discon_req(queue_t *q, mblk_t *mp)
{
	(void) q;
	(void) mp;
	return (-EOPNOTSUPP);
}

/*
 *  N_DATA_REQ		(3 - Connection-Mode data transfer request)
 *  -------------------------------------------------------------------------
 */
static int
aa_data_req(queue_t *q, mblk_t *mp)
{
	(void) q;
	(void) mp;
	return (-EOPNOTSUPP);
}

/*
 *  N_EXDATA_REQ	(4 - Expedited data request)
 *  -------------------------------------------------------------------------
 */
static int
aa_exdata_req(queue_t *q, mblk_t *mp)
{
	(void) q;
	(void) mp;
	return (-EOPNOTSUPP);
}

/*
 *  N_INFO_REQ		(5 - Information Request)
 *  -------------------------------------------------------------------------
 */
static int
aa_info_req(queue_t *q, mblk_t *mp)
{
	(void) q;
	(void) mp;
	return (-EOPNOTSUPP);
}

/*
 *  N_BIND_REQ		(6 - Bind a NS user to network address)
 *  -------------------------------------------------------------------------
 */
static int
aa_bind_req(queue_t *q, mblk_t *mp)
{
	(void) q;
	(void) mp;
	return (-EOPNOTSUPP);
}

/*
 *  N_UNBIND_REQ	(7 - Unbind NS user from network address)
 *  -------------------------------------------------------------------------
 */
static int
aa_unbind_req(queue_t *q, mblk_t *mp)
{
	(void) q;
	(void) mp;
	return (-EOPNOTSUPP);
}

/*
 *  N_UNITDATA_REQ	(8 - Connection-less data send request)
 *  -------------------------------------------------------------------------
 */
static int
aa_unitdata_req(queue_t *q, mblk_t *mp)
{
	(void) q;
	(void) mp;
	return (-EOPNOTSUPP);
}

/*
 *  N_OPTMGMT_REQ	(9 - Options Management request)
 *  -------------------------------------------------------------------------
 */
static int
aa_optmgmt_req(queue_t *q, mblk_t *mp)
{
	(void) q;
	(void) mp;
	return (-EOPNOTSUPP);
}

/*
 *  N_DATACK_REQ	(23 - Data acknowledgement request)
 *  -------------------------------------------------------------------------
 */
static int
aa_datack_req(queue_t *q, mblk_t *mp)
{
	(void) q;
	(void) mp;
	return (-EOPNOTSUPP);
}

/*
 *  N_RESET_REQ		(25 - NC reset request)
 *  -------------------------------------------------------------------------
 */
static int
aa_reset_req(queue_t *q, mblk_t *mp)
{
	(void) q;
	(void) mp;
	return (-EOPNOTSUPP);
}

/*
 *  N_RESET_RES		(27 - Reset processing accepted)
 *  -------------------------------------------------------------------------
 */
static int
aa_reset_res(queue_t *q, mblk_t *mp)
{
	(void) q;
	(void) mp;
	return (-EOPNOTSUPP);
}

/*
 *  N_RECOVER_REQ	(30 - NC recovery request)
 *  -------------------------------------------------------------------------
 */
static int
aa_reset_req(queue_t *q, mblk_t *mp)
{
	(void) q;
	(void) mp;
	return (-EOPNOTSUPP);
}

/*
 *  N_RETRIEVE_REQ	(32 - NC retrieval request)
 *  -------------------------------------------------------------------------
 */
static int
aa_retreive_req(queue_t *q, mblk_t *mp)
{
	(void) q;
	(void) mp;
	return (-EOPNOTSUPP);
}

static int (*sscop_ustr_primp[]) (queue_t *q, mblk_t *mp) = {
	aa_conn_req,		/* N_CONN_REQ (0 - NC request) */
	    aa_conn_res,	/* N_CONN_RES (1 - Accept prevsious connection indication) */
	    aa_discon_req,	/* N_DISCON_REQ (2 - NC disconnection request) */
	    aa_data_req,	/* N_DATA_REQ (3 - Connection-Mode data transfer request) */
	    aa_exdata_req,	/* N_EXDATA_REQ (4 - Expedited data request) */
	    aa_info_req,	/* N_INFO_REQ (5 - Information Request) */
	    aa_bind_req,	/* N_BIND_REQ (6 - Bind a NS user to network address) */
	    aa_unbind_req,	/* N_UNBIND_REQ (7 - Unbind NS user from network address) */
	    aa_unitdata_req,	/* N_UNITDATA_REQ (8 - Connection-less data send request) */
	    aa_optmgmt_req,	/* N_OPTMGMT_REQ (9 - Options Management request) */
	    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, aa_datack_req,	/* N_DATACK_REQ 
														   (23 
														   - 
														   Data 
														   acknowledgement 
														   request) 
														 */
	    NULL, aa_reset_req,	/* N_RESET_REQ (25 - NC reset request) */
	    NULL, aa_reset_res,	/* N_RESET_RES (27 - Reset processing accepted) */
	    NULL, NULL, aa_recover_req,	/* N_RECOVER_REQ (30 - NC recovery request) */
	    NULL, aa_retrieve_req	/* N_RETRIEVE_REQ (32 - NC retrieval request) */
NULL, NULL,};

static int
sscop_w_proto(queue_t *q, mblk_t *mp)
{
}
