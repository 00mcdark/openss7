/*****************************************************************************

 @(#) $RCSfile: sccp.c,v $ $Name:  $($Revision: 1.1.2.1 $) $Date: 2009-06-21 11:20:51 $

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

 Last Modified $Date: 2009-06-21 11:20:51 $ by $Author: brian $

 -----------------------------------------------------------------------------

 $Log: sccp.c,v $
 Revision 1.1.2.1  2009-06-21 11:20:51  brian
 - added files to new distro

 *****************************************************************************/

#ident "@(#) $RCSfile: sccp.c,v $ $Name:  $($Revision: 1.1.2.1 $) $Date: 2009-06-21 11:20:51 $"

static char const ident[] = "$RCSfile: sccp.c,v $ $Name:  $($Revision: 1.1.2.1 $) $Date: 2009-06-21 11:20:51 $";

/*
 *  This is an SCCP (Signalling Connection Control Part) multiplexing driver which can have MTP
 *  (Message Transfer Part) streams I_LINK'ed or I_PLINK'ed underneath it to form a complete
 *  Signalling Connection Control Part protocol layer for SS7.
 */

#define _SVR4_SOURCE	1
#define _MPS_SOURCE	1
#define _SUN_SOURCE	1

#include <sys/os7/compat.h>
#include <linux/socket.h>

#include <ss7/lmi.h>
#include <ss7/lmi_ioctl.h>
#include <ss7/mtpi.h>
#include <ss7/mtpi_ioctl.h>

#include <sys/npi.h>
#include <sys/npi_ss7.h>
#include <sys/npi_mtp.h>
#include <sys/npi_sccp.h>
#include <ss7/sccpi.h>
#include <ss7/sccpi_ioctl.h>
#include <sys/tihdr.h>
// #include <sys/tpi.h>
// #include <sys/tpi_ss7.h>
// #include <sys/tpi_mtp.h>
// #include <sys/tpi_sccp.h>
#include <sys/xti.h>
#include <sys/xti_mtp.h>
#include <sys/xti_sccp.h>

#define SCCP_DESCRIP	"SS7 SIGNALLING CONNECTION CONTROL PART (SCCP) STREAMS MULTIPLEXING DRIVER."
#define SCCP_REVISION	"LfS $RCSfile: sccp.c,v $ $Name:  $($Revision: 1.1.2.1 $) $Date: 2009-06-21 11:20:51 $"
#define SCCP_COPYRIGHT	"Copyright (c) 2008-2009  Monavacon Limited.  All Rights Reserved."
#define SCCP_DEVICE	"Part of the OpenSS7 Stack for Linux Fast-STREAMS."
#define SCCP_CONTACT	"Brian Bidulock <bidulock@openss7.org>"
#define SCCP_LICENSE	"GPL"
#define SCCP_BANNER	SCCP_DESCRIP	"\n" \
			SCCP_REVISION	"\n" \
			SCCP_COPYRIGHT	"\n" \
			SCCP_DEVICE	"\n" \
			SCCP_CONTACT

#ifdef LINUX
MODULE_AUTHOR(SCCP_CONTACT);
MODULE_DESCRIPTION(SCCP_DESCRIP);
MODULE_SUPPORTED_DEVICE(SCCP_DEVICE);
#ifdef MODULE_LICENSE
MODULE_LICENSE(SCCP_LICENSE);
#endif				/* MODULE_LICENSE */
#if defined MODULE_ALIAS
MODULE_ALIAS("streams-sccp");
#endif
#ifdef MODULE_VERSION
MODULE_VERSION(__stringify(PACKAGE_RPMEPOCH) ":" PACKAGE_VERSION "." PACKAGE_RELEASE
	       PACKAGE_PATCHLEVEL "-" PACKAGE_RPMRELEASE PACKAGE_RPMEXTRA2);
#endif
#endif				/* LINUX */

#define SCCP_DRV_ID	CONFIG_STREAMS_SCCP_MODID
#define SCCP_DRV_NAME	CONFIG_STREAMS_SCCP_NAME
#define SCCP_CMAJORS	CONFIG_STREAMS_SCCP_NMAJORS
#define SCCP_CMAJOR_0	CONFIG_STREAMS_SCCP_MAJOR
#define SCCP_UNITS	CONFIG_STREAMS_SCCP_NMINORS

#define SCCP_CMINOR_SCCPI   0
#define SCCP_CMINOR_TPI	    1
#define SCCP_CMINOR_NPI	    2
#define SCCP_CMINOR_GTT	    3
#define SCCP_CMINOR_MGMT    4
#define SCCP_CMINOR_FREE    5

#define SCCP_STYLE_SCCPI    SCCP_CMINOR_SCCPI
#define SCCP_STYLE_TPI	    SCCP_CMINOR_TPI
#define SCCP_STYLE_NPI	    SCCP_CMINOR_NPI
#define SCCP_STYLE_GTT	    SCCP_CMINOR_GTT
#define SCCP_STYLE_MGMT	    SCCP_CMINOR_MGMT

#define DRV_ID		SCCP_DRV_ID
#define DRV_NAME	SCCP_DRV_NAME
#define CMAJORS		SCCP_CMAJORS
#define CMAJOR_0	SCCP_CMAJOR_0
#define UNITS		SCCP_UNITS
#ifdef MODULE
#define DRV_BANNER	SCCP_BANNER
#else				/* MODULE */
#define DRV_BANNER	SCCP_SPLASH
#endif				/* MODULE */

#define STRLOGERR	0	/* log error information */
#define STRLOGNO	0	/* log notice information */
#define STRLOGST	1	/* log state transitions */
#define STRLOGTO	2	/* log timeouts */
#define STRLOGRX	3	/* log primitives received */
#define STRLOGTX	4	/* log primitives issued */
#define STRLOGTE	5	/* log timer events */
#define STRLOGIO	6	/* log additional data */
#define STRLOGDA	7	/* log data */

/*
 *  =========================================================================
 *
 *  Private Structures
 *
 *  =========================================================================
 */

#ifndef PAD4
#define PAD4(_a) (((_a)+sizeof(uint32_t)-1) & ~(sizeof(uint32_t)-1))
#endif

typedef struct sccp_options {
	t_uscalar_t pcl;
	t_uscalar_t ret;
	t_uscalar_t imp;
	t_uscalar_t seq;
	t_uscalar_t pri;
	t_uscalar_t sls;
	t_uscalar_t mp;
	t_uscalar_t debug;
} sccp_options_t;

struct sccp_options opt_defaults;

/*
 *  Signalling Connection Control Part
 *  -----------------------------------
 */
typedef struct sc {
	STR_DECLARATION (struct sc);	/* stream declaration */
	SLIST_LINKAGE (ss, sc, ss);	/* bound/listen subsystem linkage */
	SLIST_LINKAGE (sp, sc, sp);	/* connected local signalling point */
	SLIST_LINKAGE (sr, sc, sr);	/* connected remote signalling point */
	np_ulong token;			/* token for this stream */
	np_ulong conind;		/* max connection inds */
	np_ulong outcnt;		/* out connection inds */
	np_ulong pcl;			/* protocol class */
	np_ulong ret;			/* return on error */
	np_ulong slr;			/* src local reference */
	np_ulong dlr;			/* dst local reference */
	struct sccp_addr src;		/* bound src address */
	uint8_t saddr[SCCP_MAX_ADDR_LENGTH];
	struct sccp_addr dst;		/* bound dst address */
	uint8_t daddr[SCCP_MAX_ADDR_LENGTH];
	uint8_t pro;			/* MTP SI for SCCP (3) */
	N_qos_sel_conn_sccp_t cqos;	/* connection QOS parameters */
	N_qos_sel_info_sccp_t iqos;	/* information QOS parameters */
	N_qos_range_info_sccp_t iqor;	/* information QOS range */
	N_info_ack_t info;		/* information */
	np_ulong sls;			/* SLS value (seq. control) */
	np_ulong mp;			/* MP value (message prior.) */
	np_ulong imp;			/* SCCP importance */
	np_ulong credit;		/* window */
	np_ulong p_cred;		/* peer window */
	np_ulong ps;			/* send sequence number */
	np_ulong pr;			/* recv sequence number */
	np_ulong datinds;		/* unacked data inds */
	np_ulong mtu;			/* maximum transfer unit */
	bufq_t conq;			/* connection indication queue */
	bufq_t sndq;			/* send buffer */
	bufq_t urgq;			/* expedited data send buffer */
	bufq_t ackq;			/* recv awaiting ack buffer */
	bufq_t rcvq;			/* recv buffer */
	struct sccp_options options;	/* settable options */
	struct lmi_option proto;	/* sc protocol variant and options */
	struct sccp_notify_sc notify;	/* sc notifications */
	struct sccp_timers_sc timers;	/* sc protocol timers */
	struct sccp_opt_conf_sc config;	/* sc configuration */
	struct sccp_stats_sc statsp;	/* sc statistics periods */
	struct sccp_stats_sc stamp;	/* sc statistics timestamps */
	struct sccp_stats_sc stats;	/* sc statistics */
} sc_t;

#define SCCP_PRIV(__q) ((struct sc *)(__q)->q_ptr)

static struct sc *sccp_alloc_priv(queue_t *, struct sc **, dev_t *, cred_t *, minor_t);
static struct sc *sccp_get(struct sc *);
static struct sc *sccp_lookup(uint);
static uint sccp_get_id(uint);
static void sccp_free_priv(struct sc *);
static void sccp_put(struct sc *);

#define SCCPF_DEFAULT_RC_SEL	0x00000001
#define SCCPF_LOC_ED		0x00000002
#define SCCPF_REM_ED		0x00000004
#define SCCPF_REC_CONF_OPT	0x00000008

#define SCCPF_EX_DATA_OPT	0x00000010
#define SCCPF_RETURN_ERROR	0x00000020
#define SCCPF_ACK_OUT		0x00000040

#define SCCPF_PROHIBITED	0x00000100
#define SCCPF_CONGESTED		0x00000200
#define SCCPF_SCCP_UPU		0x00000400
#define SCCPF_SUBSYSTEM_TEST	0x00000800

#define SCCPF_UNEQUIPPED	0x00001000
#define SCCPF_INACCESSIBLE	0x00002000
#define SCCPF_STRICT_ROUTING	0x00004000
#define SCCPF_STRICT_CONFIG	0x00008000

#define SCCPF_HOPC_VIOLATION	0x00010000
#define SCCPF_XLAT_FAILURE	0x00020000
#define SCCPF_ASSOC_REQUIRED	0x00040000
#define SCCPF_CHANGE_REQUIRED	0x00080000

/*
 *  NA - Network Appearance
 *  -----------------------------------
 *  The NA structure defines a network appearance.  A network appearance is a combination of
 *  protocol variant, protocol options and point code format.  The network appearance identifies a
 *  point code numbering space and defines the service indicators within that numbering space.
 *  Signalling points belong to a single network appearance.
 */
typedef struct na {
	HEAD_DECLARATION (struct na);	/* head declaration */
	SLIST_HEAD (sp, sp);		/* signalling points in this na */
	struct lmi_option proto;	/* protocol variant and options */
	N_info_ack_t prot;		/* protocol profiles for SCCP for this na */
	struct sccp_timers_na timers;	/* network appearance protocol timers */
	struct sccp_opt_conf_na config;	/* network appearance configuration */
	struct sccp_stats_na statsp;	/* network appearance statistics periods */
	struct sccp_stats_na stats;	/* network appearance statistics */
	struct sccp_notify_na notify;	/* network appearance notifications */
} na_t;

static struct na *sccp_alloc_na(uint, struct lmi_option *);
static void sccp_free_na(struct na *);
static struct na *na_lookup(uint);
static uint na_get_id(uint);
static struct na *na_get(struct na *);
static void na_put(struct na *);

/*
 *  These structures define a single concerned signalling point.  These structures are listed
 *  against an equipped subsystem.  They can be added and removed with IOCTLs on either the
 *  management stream or any stream bound to the associated subsystem.
 *
 *  When MTP-PAUSE, RESUME or STATUS primitives are received by SCCP with respect to a remote
 *  singalling point, we scan the list of equipped subsystems and concerned signalling points
 *  associated with the MTP-provider on which these primitves were recieved.  If there is an
 *  equipped and bound subsystem with the local signalling point as a concerned signalling point, we
 *  deliver the N-PCSTATE primitive to any protocol class 0 or 1 SCCP-Users as an
 *  N_UDERROR_IND/T_UDERROR_IND.  Otherwise, we do not inform the SCCP-User using N-PCSTATE for
 *  remote point codes which are not concerned, nor for protocol classes 2 and 3.  (For protocol
 *  classes 2 and 3 we simply generate an error when an attempt is made to connect or send to the
 *  remote point code.) FIXME: we should send N_DISCON_IND/T_DISCON_IND with appropriate reason.
 *
 *  If we receive a subsystem allowed, prohibited or subsystem congestion indication for a concerned
 *  susbsystem, if there is an equipped subsystem that has the affected susbsystem as a concerned
 *  subsystem, we deliver the N-STATE primitive to any protocol class 0 or 1 SCCP-Users as a
 *  N_UDERROR_IND/T_UDERROR_IND.  Otherwise, we do not inform the SCCP-User using N-STATE for remote
 *  subsystems which are not concenred, nor for protocol classes 2 and 3.  (For protocol classes 2
 *  and 3 we simply generate an error when an attempt is made to connect to the remote subsystem.)
 *  For protocol variants (e.g, SS7_PVAR_JTTC) that do not support SCCP management, remote concerned
 *  subsystems are always considered available.
 *
 *  If an SCCP-User attempts to send to or connect to a non-concerned remote signalling point or
 *  subsystem, we add the signalling point or subsystem to the lists automagically.
 */

/*
 *  Connection Point
 *  -----------------------------------
 *  A connection point represents the association of two connection sections.
 */
typedef struct cp {
	HEAD_DECLARATION (struct cp);	/* head declaration */
	SLIST_LINKAGE (sp, cp, sp);	/* local signalling point linkage */
	np_ulong pcl;			/* protocol class */
	struct {
		struct sr *sr;		/* remote signalling point */
		np_ulong slr;		/* source local reference */
		np_ulong dlr;		/* destination local reference */
		np_ulong sls;		/* SLS value (seq. control) */
		np_ulong mp;		/* MP value (message priority) */
		np_ulong imp;		/* SCCP importance */
		np_ulong credit;	/* credit */
		np_ulong p_cred;	/* peer window */
		np_ulong ps;		/* send sequence number */
		np_ulong pr;		/* recv sequence number */
		np_ulong datinds;	/* unacked data inds */
		np_ulong mtu;		/* maximum transfer unit */
		bufq_t sndq;		/* send buffer */
		bufq_t urgq;		/* expedited data send buffer */
		bufq_t ackq;		/* recv awaiting ack buffer */
		bufq_t rcvq;		/* recv buffer */
	} csec[2];			/* two connection sections */
	struct sccp_notify_cp notify;	/* cp notifications */
	struct sccp_timers_cp timers;	/* cp timers */
	struct sccp_opt_conf_cp config;	/* cp configuration */
	struct sccp_stats_cp statsp;	/* cp statistics periods */
	struct sccp_stats_cp stamp;	/* cp statistics timestamps */
	struct sccp_stats_cp stats;	/* cp statistics */
} cp_t;

#if 0
static struct cp *sccp_alloc_cp(uint, struct sp *, uint, uint);
static struct cp *cp_get(struct cp *);
#endif
static struct cp *cp_lookup(uint);

#if 0
static struct cp *sccp_lookup_cp(struct sp *, uint, const int);
static uint cp_get_id(uint);
#endif
static void sccp_free_cp(struct cp *);
static void cp_put(struct cp *);

/*
 *  Subsystem (local)
 *  -----------------------------------
 */
typedef struct ss {
	HEAD_DECLARATION (struct ss);	/* head declaration */
	SLIST_LINKAGE (sp, ss, sp);	/* concerned signalling point */
	SLIST_HEAD (sc, cl);		/* bound CL SCCP users */
	SLIST_HEAD (sc, co);		/* bound CO SCCP users */
	SLIST_HEAD (sc, cr);		/* listening SCCP users */
	SLIST_HEAD (rs, rs);		/* affected subsystem list */
	np_ulong ssn;			/* subsystem number */
	np_ulong smi;			/* subsystem multiplicity indicator */
	np_ulong level;			/* subsystem congestion level */
	np_ulong count;			/* count of congestion messages */
	struct lmi_option proto;	/* ss protocol variant and options */
	struct sccp_notify_ss notify;	/* ss notifications */
	struct sccp_timers_ss timers;	/* ss timers */
	struct sccp_opt_conf_ss config;	/* ss configuration */
	struct sccp_stats_ss statsp;	/* ss statistics periods */
	struct sccp_stats_ss stamp;	/* ss statistics timestamps */
	struct sccp_stats_ss stats;	/* ss statistics */
} ss_t;

#define SS_STATUS_AVAILABLE	0	/* subsystem is available */
#define SS_STATUS_UNAVAILABLE	1	/* subsystem is unavailable */
#define SS_STATUS_CONGESTED	2	/* subsystem is congested */
#define SS_STATUS_BLOCKED	3	/* subsystem is management blocked */
#define SS_STATUS_UNEQUIPPED	4	/* subsystem is unequipped */

static struct ss *sccp_alloc_ss(uint, struct sp *, uint);
static struct ss *ss_get(struct ss *);
static struct ss *ss_lookup(uint);
static struct ss *sccp_lookup_ss(struct sp *, uint, const int);
static uint ss_get_id(uint);
static void sccp_free_ss(struct ss *);
static void ss_put(struct ss *);

/*
 *  Subsystem (remote)
 *  -----------------------------------
 */
typedef struct rs {
	HEAD_DECLARATION (struct rs);	/* head declaration */
	SLIST_LINKAGE (sr, rs, sr);	/* affected signalling point */
	SLIST_LINKAGE (ss, rs, ss);	/* concerned subsystem linkage */
	np_ulong ssn;			/* subsystem number */
	np_ulong smi;			/* subsystem multiplicity indicator */
	np_ulong level;			/* subsystem congestion level */
	np_ulong count;			/* count of congestion messages */
	struct lmi_option proto;	/* rs protocol variant and options */
	struct sccp_notify_rs notify;	/* rs notifications */
	struct sccp_timers_rs timers;	/* rs timers */
	struct sccp_opt_conf_rs config;	/* rs configuration */
	struct sccp_stats_rs statsp;	/* rs statistics periods */
	struct sccp_stats_rs stamp;	/* rs statistics timestamps */
	struct sccp_stats_rs stats;	/* rs statistics */
} rs_t;

#define RS_STATUS_AVAILABLE	0	/* subsystem is available */
#define RS_STATUS_UNAVAILABLE	1	/* subsystem is unavailable */
#define RS_STATUS_CONGESTED	2	/* subsystem is congested */
#define RS_STATUS_BLOCKED	3	/* subsystem is management blocked */
#define RS_STATUS_UNEQUIPPED	4	/* subsystem is unequipped */

static struct rs *sccp_alloc_rs(uint, struct sr *, uint);
static struct rs *rs_get(struct rs *);
static struct rs *rs_lookup(uint);
static struct rs *sccp_lookup_rs(struct sr *, uint ssn, const int);
static uint rs_get_id(uint);
static void sccp_free_rs(struct rs *);
static void rs_put(struct rs *);

/*
 *  Signalling Relation
 *  -----------------------------------
 */
typedef struct sr {
	HEAD_DECLARATION (struct sr);
	SLIST_LINKAGE (sp, sr, sp);	/* list linkage */
	SLIST_HEAD (rs, rs);		/* affected subsystems */
	SLIST_HEAD (sc, sc);		/* SCCP users connected to this remote signalling point */
	struct mtp_addr add;		/* SS7 address associated with remote signalling point */
	struct mt *mt;			/* MTP if linked at SR */
	np_ulong level;			/* affected signalling point level */
	np_ulong count;			/* count for congestion dropping */
	struct lmi_option proto;	/* sr protocol variant and options */
	struct sccp_notify_sr notify;	/* sr notifications */
	struct sccp_timers_sr timers;	/* sr timers */
	struct sccp_opt_conf_sr config;	/* sr configuration */
	struct sccp_stats_sr statsp;	/* sr statistics periods */
	struct sccp_stats_sr stamp;	/* sr statistics timestamps */
	struct sccp_stats_sr stats;	/* sr statistics */
} sr_t;

#define SR_STATUS_AVAILABLE	0	/* sr is available */
#define SR_STATUS_UNAVAILABLE	1	/* sr is unavailable */
#define SR_STATUS_CONGESTED	2	/* sr is congested */
#define SR_STATUS_BLOCKED	3	/* sr is management blocked */

static struct sr *sccp_alloc_sr(uint, struct sp *, uint);
static struct sr *sr_get(struct sr *);
static struct sr *sr_lookup(uint);
static struct sr *sccp_lookup_sr(struct sp *, uint, const int);
static uint sr_get_id(uint);
static void sccp_free_sr(struct sr *);
static void sr_put(struct sr *);

/*
 *  Signalling Point
 *  -----------------------------------
 */
#define SCCP_CL_HASH_SIZE   32	/* connection-less bind hash slots */
#define SCCP_CR_HASH_SIZE   16	/* connection-oriented listening hash slots */
#define SCCP_CO_HASH_SIZE   64	/* connection-oriented established hash slots */
#define SCCP_CP_HASH_SIZE   64	/* connection-oriented coupling slots */
typedef struct sp {
	HEAD_DECLARATION (struct sp);	/* head declaration */
	struct {
		spinlock_t lock;
		uint users;
	} sq;
	SLIST_HEAD (sr, sr);		/* signalling relation list */
	SLIST_HEAD (ss, ss);		/* concerned subsystem list */
	SLIST_HEAD (sc, sc);		/* bound, connected or listening users */
	SLIST_HEAD (sc, gt);		/* global title streams */
	SLIST_HEAD (cp, cp);		/* list of couplings */
	struct mtp_addr add;		/* MTP address associated with local signalling point */
	struct mt *mt;			/* MTP if linked at SP */
	SLIST_LINKAGE (na, sp, na);	/* network appearance linkage */
	np_ulong sccp_next_cl_sls;	/* rotate SLS values */
	np_ulong sccp_next_co_sls;	/* rotate SLS values */
	np_ulong sccp_next_slr;		/* rotate SLR values */
	np_ulong level;			/* congestion level */
	np_ulong count;			/* count for congestion dropping */
	struct sc *sccp_cl_hash[SCCP_CL_HASH_SIZE];	/* hash of connectionless binds */
	struct sc *sccp_cr_hash[SCCP_CR_HASH_SIZE];	/* hash of connection request listeners */
	struct sc *sccp_co_hash[SCCP_CO_HASH_SIZE];	/* hash of connection-oriented connections */
	struct cp *sccp_cp_hash[SCCP_CP_HASH_SIZE];	/* hash of connection-oriented couplings */
	struct lmi_option proto;	/* sp protocol variant and options */
	struct sccp_notify_sp notify;	/* sp nitifications */
	struct sccp_timers_sp timers;	/* sp protocol timers */
	struct sccp_opt_conf_sp config;	/* sp configuration */
	struct sccp_stats_sp statsp;	/* sp statistics periods */
	struct sccp_stats_sp stamp;	/* sp statistics timestamps */
	struct sccp_stats_sp stats;	/* sp statistics */
} sp_t;

static struct sp *sccp_alloc_sp(uint, struct na *, mtp_addr_t *);
static struct sp *sp_get(struct sp *);
static struct sp *sp_lookup(uint);
static struct sp *sccp_lookup_sp(uint ni, uint pc, const int);
static uint sp_get_id(uint);
static void sccp_free_sp(struct sp *);
static void sp_put(struct sp *);

/*
 *  Message Transfer Part
 *  -----------------------------------
 */
typedef struct mt {
	STR_DECLARATION (struct mt);	/* stream declaration */
	struct sp *sp;			/* signalling point */
	struct sr *sr;			/* signalling relation */
	// SLIST_LINKAGE (sp, mt, sp); /* signalling point list linkage */
	// SLIST_LINKAGE (sr, mt, sr); /* signalling relation list linkage */
	np_ulong psdu;			/* pSDU size */
	np_ulong pidu;			/* pIDU size */
	np_ulong podu;			/* pODU size */
	struct mtp_addr loc;		/* local signalling point */
	struct mtp_addr rem;		/* remote signalling point */
	struct lmi_option proto;	/* mt protocol variant and options */
	struct sccp_notify_mt notify;	/* notifications */
	struct sccp_timers_mt timers;	/* mt protocol timers */
	struct sccp_opt_conf_mt config;	/* mt configuration */
	struct sccp_stats_mt statsp;	/* mt statistics periods */
	struct sccp_stats_mt stamp;	/* mt statistics timestamps */
	struct sccp_stats_mt stats;	/* mpt statistics */
} mt_t;

#define MTP_PRIV(__q) ((struct mt *)(__q)->q_ptr)

static struct mt *sccp_alloc_link(queue_t *, struct mt **, uint, cred_t *);
static struct mt *mtp_get(struct mt *);
static struct mt *mtp_lookup(uint);
static uint mtp_get_id(uint);
static void sccp_free_link(struct mt *);
static void mtp_put(struct mt *);

/*
 *  Default structure
 *  -----------------------------------
 */
typedef struct df {
	spinlock_t lock;
	SLIST_HEAD (sc, sc);		/* master list of sc */
	SLIST_HEAD (na, na);		/* master list of network appearances */
	SLIST_HEAD (cp, cp);		/* master list of couplings */
	SLIST_HEAD (ss, ss);		/* master list of local subsystems */
	SLIST_HEAD (rs, rs);		/* master list of remote subsystems */
	SLIST_HEAD (sr, sr);		/* master list of remote signalling points */
	SLIST_HEAD (sp, sp);		/* master list of local signalling points */
	SLIST_HEAD (mt, mt);		/* master list of message transfer parts */
	struct lmi_option proto;	/* default protocol options */
	struct sccp_notify_df notify;	/* default notifications */
	struct sccp_opt_conf_df config;	/* default configuration */
	struct sccp_stats_df stats;	/* default statistics */
	struct sccp_stats_df stamp;	/* default statistics timestamps */
	struct sccp_stats_df statsp;	/* default statistics periods */
} df_t;
static struct df master;

/*
 *  =========================================================================
 *
 *  OPTION Handling
 *
 *  =========================================================================
 */
typedef struct sccp_opts {
	np_ulong flags;			/* success flags */
	t_uscalar_t *pcl;
	t_uscalar_t *ret;
	t_uscalar_t *imp;
	t_uscalar_t *seq;
	t_uscalar_t *pri;
	t_uscalar_t *sls;
	t_uscalar_t *mp;
	t_uscalar_t *debug;
} sccp_opts_t;

#define TF_OPT_PCL	(1<<0)	/* protocol class */
#define TF_OPT_RET	(1<<1)	/* return on error option */
#define TF_OPT_IMP	(1<<2)	/* importance */
#define TF_OPT_SEQ	(1<<3)	/* sequence control */
#define TF_OPT_PRI	(1<<4)	/* priority */
#define TF_OPT_SLS	(1<<5)	/* signalling link selection */
#define TF_OPT_MP	(1<<6)	/* message priority */
#define TF_OPT_DBG	(1<<7)	/* debug */

#ifndef _T_ALIGN_SIZE
#define _T_ALIGN_SIZE sizeof(t_uscalar_t)
#endif
#ifndef _T_ALIGN_SIZEOF
#define _T_ALIGN_SIZEOF(s) \
	((sizeof((s)) + _T_ALIGN_SIZE - 1) & ~(_T_ALIGN_SIZE - 1))
#endif

static size_t
sccp_opts_size(struct sc *sc, struct sccp_opts *ops)
{
	size_t len = 0;

	if (ops) {
		const size_t hlen = sizeof(struct t_opthdr);	/* 32 bytes */

		if (ops->pcl)
			len += hlen + _T_ALIGN_SIZEOF(*(ops->pcl));
		if (ops->ret)
			len += hlen + _T_ALIGN_SIZEOF(*(ops->ret));
		if (ops->imp)
			len += hlen + _T_ALIGN_SIZEOF(*(ops->imp));
		if (ops->seq)
			len += hlen + _T_ALIGN_SIZEOF(*(ops->seq));
		if (ops->pri)
			len += hlen + _T_ALIGN_SIZEOF(*(ops->pri));
		if (ops->sls)
			len += hlen + _T_ALIGN_SIZEOF(*(ops->sls));
		if (ops->mp)
			len += hlen + _T_ALIGN_SIZEOF(*(ops->mp));
		if (ops->debug)
			len += hlen + _T_ALIGN_SIZEOF(*(ops->debug));
	}
	return (len);
}
static void
sccp_build_opts(struct sc *sc, struct sccp_opts *ops, uchar *p)
{
	if (ops) {
		struct t_opthdr *oh;
		const size_t hlen = sizeof(struct t_opthdr);

		if (ops->pcl) {
			oh = (typeof(oh)) p++;
			oh->len = hlen + sizeof(*(ops->pcl));
			oh->level = T_SS7_SCCP;
			oh->name = T_SCCP_PCLASS;
			oh->status = (ops->flags & TF_OPT_PCL) ? T_SUCCESS : T_FAILURE;
			*((typeof(ops->pcl)) p) = *(ops->pcl);
			p += _T_ALIGN_SIZEOF(*ops->pcl);
		}
		if (ops->ret) {
			oh = (typeof(oh)) p++;
			oh->len = hlen + sizeof(*(ops->ret));
			oh->level = T_SS7_SCCP;
			oh->name = T_SCCP_RET_ERROR;
			oh->status = (ops->flags & TF_OPT_RET) ? T_SUCCESS : T_FAILURE;
			*((typeof(ops->ret)) p) = *(ops->ret);
			p += _T_ALIGN_SIZEOF(*ops->ret);
		}
		if (ops->imp) {
			oh = (typeof(oh)) p++;
			oh->len = hlen + sizeof(*(ops->imp));
			oh->level = T_SS7_SCCP;
			oh->name = T_SCCP_IMPORTANCE;
			oh->status = (ops->flags & TF_OPT_IMP) ? T_SUCCESS : T_FAILURE;
			*((typeof(ops->imp)) p) = *(ops->imp);
			p += _T_ALIGN_SIZEOF(*ops->imp);
		}
		if (ops->seq) {
			oh = (typeof(oh)) p++;
			oh->len = hlen + sizeof(*(ops->seq));
			oh->level = T_SS7_SCCP;
			oh->name = T_SCCP_SEQ_CTRL;
			oh->status = (ops->flags & TF_OPT_SEQ) ? T_SUCCESS : T_FAILURE;
			*((typeof(ops->seq)) p) = *(ops->seq);
			p += _T_ALIGN_SIZEOF(*ops->seq);
		}
		if (ops->pri) {
			oh = (typeof(oh)) p++;
			oh->len = hlen + sizeof(*(ops->pri));
			oh->level = T_SS7_SCCP;
			oh->name = T_SCCP_PRIORITY;
			oh->status = (ops->flags & TF_OPT_PRI) ? T_SUCCESS : T_FAILURE;
			*((typeof(ops->pri)) p) = *(ops->pri);
			p += _T_ALIGN_SIZEOF(*ops->pri);
		}
		if (ops->sls) {
			oh = (typeof(oh)) p++;
			oh->len = hlen + sizeof(*(ops->sls));
			oh->level = T_SS7_MTP;
			oh->name = T_MTP_SLS;
			oh->status = (ops->flags & TF_OPT_SLS) ? T_SUCCESS : T_FAILURE;
			*((typeof(ops->sls)) p) = *(ops->sls);
			p += _T_ALIGN_SIZEOF(*ops->sls);
		}
		if (ops->mp) {
			oh = (typeof(oh)) p++;
			oh->len = hlen + sizeof(*(ops->mp));
			oh->level = T_SS7_MTP;
			oh->name = T_MTP_MP;
			oh->status = (ops->flags & TF_OPT_MP) ? T_SUCCESS : T_FAILURE;
			*((typeof(ops->mp)) p) = *(ops->mp);
			p += _T_ALIGN_SIZEOF(*ops->mp);
		}
		if (ops->debug) {
			oh = (typeof(oh)) p++;
			oh->len = hlen + sizeof(*(ops->debug));
			oh->level = T_SS7_MTP;
			oh->name = T_MTP_DEBUG;
			oh->status = (ops->flags & TF_OPT_DBG) ? T_SUCCESS : T_FAILURE;
			*((typeof(ops->debug)) p) = *(ops->debug);
			p += _T_ALIGN_SIZEOF(*ops->debug);
		}
	}
}
static int
sccp_parse_opts(struct sc *sc, struct sccp_opts *ops, uchar *op, size_t len)
{
	struct t_opthdr *oh;

	for (oh = _T_OPT_FIRSTHDR_OFS(op, len, 0); oh; oh = _T_OPT_NEXTHDR_OFS(op, len, oh, 0)) {
		switch (oh->level) {
		case T_SS7_SCCP:
			switch (oh->name) {
			case T_SCCP_PCLASS:
				ops->pcl = (void *) _T_OPT_DATA_OFS(oh, 0);
				ops->flags |= TF_OPT_PCL;
				continue;
			case T_SCCP_RET_ERROR:
				ops->ret = (void *) _T_OPT_DATA_OFS(oh, 0);
				ops->flags |= TF_OPT_RET;
				continue;
			case T_SCCP_IMPORTANCE:
				ops->imp = (void *) _T_OPT_DATA_OFS(oh, 0);
				ops->flags |= TF_OPT_IMP;
				continue;
			case T_SCCP_SEQ_CTRL:
				ops->seq = (void *) _T_OPT_DATA_OFS(oh, 0);
				ops->flags |= TF_OPT_SEQ;
				continue;
			case T_SCCP_PRIORITY:
				ops->pri = (void *) _T_OPT_DATA_OFS(oh, 0);
				ops->flags |= TF_OPT_PRI;
				continue;
			}
			break;
		case T_SS7_MTP:
			switch (oh->name) {
			case T_MTP_SLS:
				ops->sls = (void *) _T_OPT_DATA_OFS(oh, 0);
				ops->flags |= TF_OPT_SLS;
				continue;
			case T_MTP_MP:
				ops->mp = (void *) _T_OPT_DATA_OFS(oh, 0);
				ops->flags |= TF_OPT_MP;
				continue;
			case T_MTP_DEBUG:
				ops->debug = (void *) _T_OPT_DATA_OFS(oh, 0);
				ops->flags |= TF_OPT_DBG;
				continue;
			}
			break;
		}
	}
	if (oh)
		return (TBADOPT);
	return (0);
}

#if 0
static int
sccp_parse_qos(struct sc *sc, struct sccp_opts *ops, uchar *op, size_t len)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}
#endif

/*
 *  =========================================================================
 *
 *  OPTIONS handling
 *
 *  =========================================================================
 */
static int
sccp_opt_check(struct sc *sc, struct sccp_opts *ops)
{
	if (ops->flags) {
		ops->flags = 0;
		if (ops->pcl)
			ops->flags |= TF_OPT_PCL;
		if (ops->ret)
			ops->flags |= TF_OPT_RET;
		if (ops->imp)
			ops->flags |= TF_OPT_IMP;
		if (ops->seq)
			ops->flags |= TF_OPT_SEQ;
		if (ops->pri)
			ops->flags |= TF_OPT_PRI;
		if (ops->sls)
			ops->flags |= TF_OPT_SLS;
		if (ops->mp)
			ops->flags |= TF_OPT_MP;
		if (ops->debug)
			ops->flags |= TF_OPT_DBG;
	}
	return (0);
}
static int
sccp_opt_default(struct sc *sc, struct sccp_opts *ops)
{
	if (ops) {
		int flags = ops->flags;

		ops->flags = 0;
		if (!flags || ops->pcl) {
			ops->pcl = &opt_defaults.pcl;
			ops->flags |= TF_OPT_PCL;
		}
		if (!flags || ops->ret) {
			ops->ret = &opt_defaults.ret;
			ops->flags |= TF_OPT_RET;
		}
		if (!flags || ops->imp) {
			ops->imp = &opt_defaults.imp;
			ops->flags |= TF_OPT_IMP;
		}
		if (!flags || ops->seq) {
			ops->seq = &opt_defaults.seq;
			ops->flags |= TF_OPT_SEQ;
		}
		if (!flags || ops->pri) {
			ops->pri = &opt_defaults.pri;
			ops->flags |= TF_OPT_PRI;
		}
		if (!flags || ops->sls) {
			ops->sls = &opt_defaults.sls;
			ops->flags |= TF_OPT_SLS;
		}
		if (!flags || ops->mp) {
			ops->mp = &opt_defaults.mp;
			ops->flags |= TF_OPT_MP;
		}
		if (!flags || ops->debug) {
			ops->debug = &opt_defaults.debug;
			ops->flags |= TF_OPT_DBG;
		}
		return (0);
	}
	swerr();
	return (-EFAULT);
}
static int
sccp_opt_current(struct sc *sc, struct sccp_opts *ops)
{
	int flags = ops->flags;

	ops->flags = 0;
	if (!flags || ops->pcl) {
		ops->pcl = &sc->options.pcl;
		ops->flags |= TF_OPT_PCL;
	}
	if (!flags || ops->ret) {
		ops->ret = &sc->options.ret;
		ops->flags |= TF_OPT_RET;
	}
	if (!flags || ops->imp) {
		ops->imp = &sc->options.imp;
		ops->flags |= TF_OPT_IMP;
	}
	if (!flags || ops->seq) {
		ops->seq = &sc->options.seq;
		ops->flags |= TF_OPT_SEQ;
	}
	if (!flags || ops->pri) {
		ops->pri = &sc->options.pri;
		ops->flags |= TF_OPT_PRI;
	}
	if (!flags || ops->sls) {
		ops->sls = &sc->options.sls;
		ops->flags |= TF_OPT_SLS;
	}
	if (!flags || ops->mp) {
		ops->mp = &sc->options.mp;
		ops->flags |= TF_OPT_MP;
	}
	if (!flags || ops->debug) {
		ops->debug = &sc->options.debug;
		ops->flags |= TF_OPT_DBG;
	}
	return (0);
}
static int
sccp_opt_negotiate(struct sc *sc, struct sccp_opts *ops)
{
	if (ops->flags) {
		ops->flags = 0;
		if (ops->pcl) {
			sc->options.pcl = *ops->pcl;
			ops->pcl = &sc->options.pcl;
			ops->flags |= TF_OPT_PCL;
		}
		if (ops->ret) {
			sc->options.ret = *ops->ret;
			ops->ret = &sc->options.ret;
			ops->flags |= TF_OPT_RET;
		}
		if (ops->imp) {
			sc->options.imp = *ops->imp;
			ops->imp = &sc->options.imp;
			ops->flags |= TF_OPT_IMP;
		}
		if (ops->seq) {
			sc->options.seq = *ops->seq;
			ops->seq = &sc->options.seq;
			ops->flags |= TF_OPT_SEQ;
		}
		if (ops->pri) {
			sc->options.pri = *ops->pri;
			ops->pri = &sc->options.pri;
			ops->flags |= TF_OPT_PRI;
		}
		if (ops->sls) {
			sc->options.sls = *ops->sls;
			ops->sls = &sc->options.sls;
			ops->flags |= TF_OPT_SLS;
		}
		if (ops->mp) {
			sc->options.mp = *ops->mp;
			ops->mp = &sc->options.mp;
			ops->flags |= TF_OPT_MP;
		}
		if (ops->debug) {
			sc->options.debug = *ops->debug;
			ops->debug = &sc->options.debug;
			ops->flags |= TF_OPT_DBG;
		}
	}
	return (0);
}

/*
 *  =========================================================================
 *
 *  STATE Changes
 *
 *  =========================================================================
 */
#define SS_UNBND	0	/* NS user not bound to network address */
#define SS_WACK_BREQ	1	/* Awaiting acknowledgement of N_BIND_REQ */
#define SS_WACK_UREQ	2	/* Pending acknowledgement for N_UNBIND_REQ */
#define SS_IDLE		3	/* Idle, no connection */
#define SS_WACK_OPTREQ	4	/* Pending acknowledgement of N_OPTMGMT_REQ */
#define SS_WACK_RRES	5	/* Pending acknowledgement of N_RESET_RES */
#define SS_WCON_CREQ	6	/* Pending confirmation of N_CONN_REQ */
#define SS_WRES_CIND	7	/* Pending response of N_CONN_REQ */
#define SS_WACK_CRES	8	/* Pending acknowledgement of N_CONN_RES */
#define SS_DATA_XFER	9	/* Connection-mode data transfer */
#define SS_WCON_RREQ	10	/* Pending confirmation of N_RESET_REQ */
#define SS_WRES_RIND	11	/* Pending response of N_RESET_IND */
#define SS_WACK_DREQ6	12	/* Waiting ack of N_DISCON_REQ */
#define SS_WACK_DREQ7	13	/* Waiting ack of N_DISCON_REQ */
#define SS_WACK_DREQ9	14	/* Waiting ack of N_DISCON_REQ */
#define SS_WACK_DREQ10	15	/* Waiting ack of N_DISCON_REQ */
#define SS_WACK_DREQ11	16	/* Waiting ack of N_DISCON_REQ */
#define SS_NOSTATES	18	/* No states */
#define SS_WRES_DIND	19	/* for now */
#define SS_WCON_DREQ	20	/* for now */
#define SS_BOTH_RESET	21	/* for now */
#define SS_WCON_CREQ2	22	/* for now */

#define SSN_ALLOWED	0
#define SSN_CONGESTED	1
#define SSN_PROHIBITED	2

#define SSF_UNBND	(1<<SS_UNBND		)
#define SSF_WACK_BREQ	(1<<SS_WACK_BREQ	)
#define SSF_WACK_UREQ	(1<<SS_WACK_UREQ	)
#define SSF_IDLE	(1<<SS_IDLE		)
#define SSF_WACK_OPTREQ	(1<<SS_WACK_OPTREQ	)
#define SSF_WACK_RRES	(1<<SS_WACK_RRES	)
#define SSF_WCON_CREQ	(1<<SS_WCON_CREQ	)
#define SSF_WRES_CIND	(1<<SS_WRES_CIND	)
#define SSF_WACK_CRES	(1<<SS_WACK_CRES	)
#define SSF_DATA_XFER	(1<<SS_DATA_XFER	)
#define SSF_WCON_RREQ	(1<<SS_WCON_RREQ	)
#define SSF_WRES_RIND	(1<<SS_WRES_RIND	)
#define SSF_WACK_DREQ6	(1<<SS_WACK_DREQ6	)
#define SSF_WACK_DREQ7	(1<<SS_WACK_DREQ7	)
#define SSF_WACK_DREQ9	(1<<SS_WACK_DREQ9	)
#define SSF_WACK_DREQ10	(1<<SS_WACK_DREQ10	)
#define SSF_WACK_DREQ11	(1<<SS_WACK_DREQ11	)
#define SSF_NOSTATES	(1<<SS_NOSTATES	)
#define SSF_WRES_DIND	(1<<SS_WRES_DIND	)
#define SSF_WCON_DREQ	(1<<SS_WCON_DREQ	)
#define SSF_BOTH_RESET	(1<<SS_BOTH_RESET	)
#define SSF_WCON_CREQ2	(1<<SS_WCON_CREQ2	)

static const char *
mtp_state(uint state)
{
	switch (state) {
	case MTPS_UNBND:
		return ("MTPS_UNBND");
	case MTPS_WACK_BREQ:
		return ("MTPS_WACK_BREQ");
	case MTPS_IDLE:
		return ("MTPS_IDLE");
	case MTPS_WACK_CREQ:
		return ("MTPS_WACK_CREQ");
	case MTPS_WCON_CREQ:
		return ("MTPS_WCON_CREQ");
	case MTPS_CONNECTED:
		return ("MTPS_CONNECTED");
	case MTPS_WACK_UREQ:
		return ("MTPS_WACK_UREQ");
	case MTPS_WACK_DREQ6:
		return ("MTPS_WACK_DREQ6");
	case MTPS_WACK_DREQ9:
		return ("MTPS_WACK_DREQ9");
	case MTPS_WACK_OPTREQ:
		return ("MTPS_WACK_OPTREQ");
	case MTPS_WREQ_ORDREL:
		return ("MTPS_WREQ_ORDREL");
	case MTPS_WIND_ORDREL:
		return ("MTPS_WIND_ORDREL");
	case MTPS_WRES_CIND:
		return ("MTPS_WRES_CIND");
	case MTPS_UNUSABLE:
		return ("MTPS_UNUSABLE");
	default:
		return ("(unknown)");
	}
}

static const char *
sccp_n_state(uint state)
{
	switch (state) {
	case NS_UNBND:
		return ("NS_UNBND");
	case NS_WACK_BREQ:
		return ("NS_WACK_BREQ");
	case NS_WACK_UREQ:
		return ("NS_WACK_UREQ");
	case NS_IDLE:
		return ("NS_IDLE");
	case NS_WACK_OPTREQ:
		return ("NS_WACK_OPTREQ");
	case NS_WACK_RRES:
		return ("NS_WACK_RRES");
	case NS_WCON_CREQ:
		return ("NS_WCON_CREQ");
	case NS_WRES_CIND:
		return ("NS_WRES_CIND");
	case NS_WACK_CRES:
		return ("NS_WACK_CRES");
	case NS_DATA_XFER:
		return ("NS_DATA_XFER");
	case NS_WCON_RREQ:
		return ("NS_WCON_RREQ");
	case NS_WRES_RIND:
		return ("NS_WRES_RIND");
	case NS_WACK_DREQ6:
		return ("NS_WACK_DREQ6");
	case NS_WACK_DREQ7:
		return ("NS_WACK_DREQ7");
	case NS_WACK_DREQ9:
		return ("NS_WACK_DREQ9");
	case NS_WACK_DREQ10:
		return ("NS_WACK_DREQ10");
	case NS_WACK_DREQ11:
		return ("NS_WACK_DREQ11");
	default:
		return ("(unknown)");
	}
}
static const char *
sccp_t_state(uint state)
{
	switch (state) {
	case TS_UNBND:
		return ("TS_UNBND");
	case TS_WACK_BREQ:
		return ("TS_WACK_BREQ");
	case TS_WACK_UREQ:
		return ("TS_WACK_UREQ");
	case TS_IDLE:
		return ("TS_IDLE");
	case TS_WACK_OPTREQ:
		return ("TS_WACK_OPTREQ");
	case TS_WACK_CREQ:
		return ("TS_WACK_CREQ");
	case TS_WCON_CREQ:
		return ("TS_WCON_CREQ");
	case TS_WRES_CIND:
		return ("TS_WRES_CIND");
	case TS_WACK_CRES:
		return ("TS_WACK_CRES");
	case TS_DATA_XFER:
		return ("TS_DATA_XFER");
	case TS_WIND_ORDREL:
		return ("TS_WIND_ORDREL");
	case TS_WREQ_ORDREL:
		return ("TS_WREQ_ORDREL");
	case TS_WACK_DREQ6:
		return ("TS_WACK_DREQ6");
	case TS_WACK_DREQ7:
		return ("TS_WACK_DREQ7");
	case TS_WACK_DREQ9:
		return ("TS_WACK_DREQ9");
	case TS_WACK_DREQ10:
		return ("TS_WACK_DREQ10");
	case TS_WACK_DREQ11:
		return ("TS_WACK_DREQ11");
	default:
		return ("(unknown)");
	}
}
static const char *
sccp_state(uint state)
{
	switch (state) {
	case SS_UNBND:
		return ("SS_UNBND");
	case SS_WACK_BREQ:
		return ("SS_WACK_BREQ");
	case SS_WACK_UREQ:
		return ("SS_WACK_UREQ");
	case SS_IDLE:
		return ("SS_IDLE");
	case SS_WACK_OPTREQ:
		return ("SS_WACK_OPTREQ");
	case SS_WACK_RRES:
		return ("SS_WACK_RRES");
	case SS_WCON_CREQ:
		return ("SS_WCON_CREQ");
	case SS_WRES_CIND:
		return ("SS_WRES_CIND");
	case SS_WACK_CRES:
		return ("SS_WACK_CRES");
	case SS_DATA_XFER:
		return ("SS_DATA_XFER");
	case SS_WCON_RREQ:
		return ("SS_WCON_RREQ");
	case SS_WRES_RIND:
		return ("SS_WRES_RIND");
	case SS_WACK_DREQ6:
		return ("SS_WACK_DREQ6");
	case SS_WACK_DREQ7:
		return ("SS_WACK_DREQ7");
	case SS_WACK_DREQ9:
		return ("SS_WACK_DREQ9");
	case SS_WACK_DREQ10:
		return ("SS_WACK_DREQ10");
	case SS_WACK_DREQ11:
		return ("SS_WACK_DREQ11");
	case SS_NOSTATES:
		return ("SS_NOSTATES");
	case SS_WRES_DIND:
		return ("SS_WRES_DIND");
	case SS_WCON_DREQ:
		return ("SS_WCON_DREQ");
	case SS_BOTH_RESET:
		return ("SS_BOTH_RESET");
	case SS_WCON_CREQ2:
		return ("SS_WCON_CREQ2");
	default:
		return ("(unknown)");
	}
}

static void
sccp_set_state(struct sc *sc, uint newstate)
{
	uint oldstate = sc->state;

	if (newstate != oldstate) {
		mi_strlog(sc->oq, STRLOGST, SL_TRACE, "%s <- %s", sccp_state(newstate),
			  sccp_state(oldstate));
		sc->state = newstate;
	}
}
static uint
sccp_get_state(struct sc *sc)
{
	return sc->state;
}
static void
sccp_set_n_state(struct sc *sc, uint newstate)
{
	uint oldstate = sc->i_state;

	if (newstate != oldstate) {
		mi_strlog(sc->oq, STRLOGST, SL_TRACE, "%s <- %s", sccp_n_state(newstate),
			  sccp_n_state(oldstate));
		sc->i_state = newstate;
	}
}
static uint
sccp_get_n_state(struct sc *sc)
{
	return sc->i_state;
}
static inline uint
sccp_chk_n_state(struct sc *sc, uint mask)
{
	return ((1 << sccp_get_n_state(sc)) & mask);
}
static void
sccp_set_t_state(struct sc *sc, uint newstate)
{
	uint oldstate = sc->i_state;

	if (newstate != oldstate) {
		mi_strlog(sc->oq, STRLOGST, SL_TRACE, "%s <- %s", sccp_t_state(newstate),
			  sccp_t_state(oldstate));
		sc->i_state = newstate;
	}
}
static uint
sccp_get_t_state(struct sc *sc)
{
	return sc->i_state;
}
static inline uint
sccp_chk_t_state(struct sc *sc, uint mask)
{
	return ((1 << sccp_get_t_state(sc)) & mask);
}

static uint
mtp_get_state(struct mt *mt)
{
	return mt->state;
}
static void
mtp_set_state(struct mt *mt, uint newstate)
{
	uint oldstate = mt->state;

	if (newstate != oldstate) {
		mi_strlog(mt->iq, STRLOGST, SL_TRACE, "%s <- %s", mtp_state(newstate),
			  mtp_state(oldstate));
		mt->state = newstate;
	}
}

/*
 *  TLI interface state flags
 */
#define TSF_UNBND	( 1 << TS_UNBND		)
#define TSF_WACK_BREQ	( 1 << TS_WACK_BREQ	)
#define TSF_WACK_UREQ	( 1 << TS_WACK_UREQ	)
#define TSF_IDLE	( 1 << TS_IDLE		)
#define TSF_WACK_OPTREQ	( 1 << TS_WACK_OPTREQ	)
#define TSF_WACK_CREQ	( 1 << TS_WACK_CREQ	)
#define TSF_WCON_CREQ	( 1 << TS_WCON_CREQ	)
#define TSF_WRES_CIND	( 1 << TS_WRES_CIND	)
#define TSF_WACK_CRES	( 1 << TS_WACK_CRES	)
#define TSF_DATA_XFER	( 1 << TS_DATA_XFER	)
#define TSF_WIND_ORDREL	( 1 << TS_WIND_ORDREL	)
#define TSF_WREQ_ORDREL	( 1 << TS_WREQ_ORDREL	)
#define TSF_WACK_DREQ6	( 1 << TS_WACK_DREQ6	)
#define TSF_WACK_DREQ7	( 1 << TS_WACK_DREQ7	)
#define TSF_WACK_DREQ9	( 1 << TS_WACK_DREQ9	)
#define TSF_WACK_DREQ10	( 1 << TS_WACK_DREQ10	)
#define TSF_WACK_DREQ11	( 1 << TS_WACK_DREQ11	)
#define TSF_NOSTATES	( 1 << TS_NOSTATES	)

#define TSF_WACK_DREQ	(TSF_WACK_DREQ6 \
			|TSF_WACK_DREQ7 \
			|TSF_WACK_DREQ9 \
			|TSF_WACK_DREQ10 \
			|TSF_WACK_DREQ11)

/*
 *  SCCP Message Structures
 *  -------------------------------------------------------------------------
 */
#define SCCP_MT_NONE	0x00	/* (reserved) */
#define SCCP_MT_CR	0x01	/* Connection Request */
#define SCCP_MT_CC	0x02	/* Connection Confirm */
#define SCCP_MT_CREF	0x03	/* Connection Refused */
#define SCCP_MT_RLSD	0x04	/* Released */
#define SCCP_MT_RLC	0x05	/* Release Complete */
#define SCCP_MT_DT1	0x06	/* Data Form 1 */
#define SCCP_MT_DT2	0x07	/* Data Form 2 */
#define SCCP_MT_AK	0x08	/* Data Acknowledgement */
#define SCCP_MT_UDT	0x09	/* Unitdata */
#define SCCP_MT_UDTS	0x0a	/* Unitdata Service */
#define SCCP_MT_ED	0x0b	/* Expedited Data */
#define SCCP_MT_EA	0x0c	/* Expedited Data Acknowledgement */
#define SCCP_MT_RSR	0x0d	/* Reset Request */
#define SCCP_MT_RSC	0x0e	/* Reset Confirm */
#define SCCP_MT_ERR	0x0f	/* Protocol Data Unit Error */
#define SCCP_MT_IT	0x10	/* Inactivity Test */
#define SCCP_MT_XUDT	0x11	/* Extended Unitdata */
#define SCCP_MT_XUDTS	0x12	/* Extended Unitdata Service */
#define SCCP_MT_LUDT	0x13	/* Long Unitdata */
#define SCCP_MT_LUDTS	0x14	/* Long Unitdata Service */
#define SCCP_NOMESSAGES	0x15	/* number of message types */

#define SCMG_MT_SSA	0x01	/* Subsystem allowed */
#define SCMG_MT_SSP	0x02	/* Subsystem prohibited */
#define SCMG_MT_SST	0x03	/* Subsystem status test */
#define SCMG_MT_SOR	0x04	/* Subsystem out of service request */
#define SCMG_MT_SOG	0x05	/* Subsystem out of service grant */
#define SCMG_MT_SSC	0x06	/* Subsystem congestion */

#define SCMG_MT_SBR	0xfd	/* Subsystem backup routing */
#define SCMG_MT_SNR	0xfe	/* Subsystem normal routing */
#define SCMG_MT_SRT	0xff	/* Subsystem routing status test */

#define SCCP_MP_CR	0x0	/* 0 or 1 */
#define SCCP_MP_CC	0x1
#define SCCP_MP_CREF	0x1
#define SCCP_MP_RLSD	0x2
#define SCCP_MP_RLC	0x2
#define SCCP_MP_DT1	0x0	/* 0 or 1 */
#define SCCP_MP_DT2	0x0	/* 0 or 1 */
#define SCCP_MP_AK	0x0	/* 0 or 1 */
#define SCCP_MP_UDT	0x0	/* 0, 1 or 2 */
#define SCCP_MP_UDTS	0x0	/* 0, 1 or 2 */
#define SCCP_MP_ED	0x1
#define SCCP_MP_EA	0x1
#define SCCP_MP_RSR	0x1
#define SCCP_MP_RSC	0x1
#define SCCP_MP_ERR	0x1
#define SCCP_MP_IT	0x1
#define SCCP_MP_XUDT	0x0	/* 0, 1 or 2 */
#define SCCP_MP_XUDTS	0x0	/* 0, 1 or 2 */
#define SCCP_MP_LUDT	0x0	/* 0, 1 or 2 */
#define SCCP_MP_LUDTS	0x0	/* 0, 1 or 2 */

/*
 *  Importance values from Table 2/Q.714-1996
 */
/*
   default importance values 
 */
#define SCCP_DI_CR	0x2	/* inc at relay */
#define SCCP_DI_CC	0x3	/* inc at relay */
#define SCCP_DI_CREF	0x2	/* inc at relay */
#define SCCP_DI_DT1	0x4	/* def at relay */
#define SCCP_DI_DT2	0x4	/* def at relay */
#define SCCP_DI_AK	0x6	/* def at relay */
#define SCCP_DI_IT	0x6	/* def at relay */
#define SCCP_DI_ED	0x7	/* def at relay */
#define SCCP_DI_EA	0x7	/* def at relay */
#define SCCP_DI_RSR	0x6	/* def at relay */
#define SCCP_DI_RSC	0x6	/* def at relay */
#define SCCP_DI_ERR	0x7	/* def at relay */
#define SCCP_DI_RLC	0x4	/* def at relay */
#define SCCP_DI_RLSD	0x6	/* inc at relay */
#define SCCP_DI_UDT	0x4	/* def at relay */
#define SCCP_DI_UDTS	0x3	/* def at relay */
#define SCCP_DI_XUDT	0x4	/* inc at relay */
#define SCCP_DI_XUDTS	0x3	/* inc at relay */
#define SCCP_DI_LUDT	0x4	/* inc at relay */
#define SCCP_DI_LUDTS	0x3	/* inc at relay */

/*
   maximum importance values 
 */
#define SCCP_MI_CR	0x4
#define SCCP_MI_CC	0x4
#define SCCP_MI_CREF	0x4
#define SCCP_MI_DT1	0x6
#define SCCP_MI_DT2	0x6
#define SCCP_MI_AK	SCCP_DI_AK
#define SCCP_MI_IT	SCCP_DI_IT
#define SCCP_MI_ED	SCCP_DI_ED
#define SCCP_MI_EA	SCCP_DI_EA
#define SCCP_MI_RSR	SCCP_DI_RSR
#define SCCP_MI_RSC	SCCP_DI_RSC
#define SCCP_MI_ERR	SCCP_DI_ERR
#define SCCP_MI_RLC	SCCP_DI_RLC
#define SCCP_MI_RLSD	0x6
#define SCCP_MI_UDT	0x6
#define SCCP_MI_UDTS	0x0	/* same as returned UDT */
#define SCCP_MI_XUDT	0x6
#define SCCP_MI_XUDTS	0x0	/* same as returned XUDT */
#define SCCP_MI_LUDT	0x6
#define SCCP_MI_LUDTS	0x0	/* same as returned LUDT */

#define SCMG_MP_SSA	0x3
#define SCMG_MP_SSP	0x3
#define SCMG_MP_SST	0x2
#define SCMG_MP_SOR	0x1
#define SCMG_MP_SOG	0x1
#define SCMG_MP_SSC	0x0	/* not defined for ANSI */

#define SCMG_MP_SBR	0x0
#define SCMG_MP_SNR	0x0
#define SCMG_MP_SRT	0x0

/*
 *  Notes on message priority:
 *
 *  (1) The UDTS, XUDTS and LUDTS messages assume the message priority of the
 *      message which they contain.
 *
 *  (2) The UDT, XUDT and LUDT messages can assume the message priority of 0
 *      or 1: the value 2 is reserved for use by OMAP.
 *
 *  (3) The priority of DT1, DT2 and AK should match that of the connection
 *      request.
 */

#define SCCP_MTF_NONE	(1<<SCCP_MT_NONE	)
#define SCCP_MTF_CR	(1<<SCCP_MT_CR		)
#define SCCP_MTF_CC	(1<<SCCP_MT_CC		)
#define SCCP_MTF_CREF	(1<<SCCP_MT_CREF	)
#define SCCP_MTF_RLSD	(1<<SCCP_MT_RLSD	)
#define SCCP_MTF_RLC	(1<<SCCP_MT_RLC		)
#define SCCP_MTF_DT1	(1<<SCCP_MT_DT1		)
#define SCCP_MTF_DT2	(1<<SCCP_MT_DT2		)
#define SCCP_MTF_AK	(1<<SCCP_MT_AK		)
#define SCCP_MTF_UDT	(1<<SCCP_MT_UDT		)
#define SCCP_MTF_UDTS	(1<<SCCP_MT_UDTS	)
#define SCCP_MTF_ED	(1<<SCCP_MT_ED		)
#define SCCP_MTF_EA	(1<<SCCP_MT_EA		)
#define SCCP_MTF_RSR	(1<<SCCP_MT_RSR		)
#define SCCP_MTF_RSC	(1<<SCCP_MT_RSC		)
#define SCCP_MTF_ERR	(1<<SCCP_MT_ERR		)
#define SCCP_MTF_IT	(1<<SCCP_MT_IT		)
#define SCCP_MTF_XUDT	(1<<SCCP_MT_XUDT	)
#define SCCP_MTF_XUDTS	(1<<SCCP_MT_XUDTS	)
#define SCCP_MTF_LUDT	(1<<SCCP_MT_LUDT	)
#define SCCP_MTF_LUDTS	(1<<SCCP_MT_LUDTS	)

/*
 *  Connection-Oriented messages
 */
#define SCCP_MTM_CO	(	SCCP_MTF_CR	\
			|	SCCP_MTF_CC	\
			|	SCCP_MTF_CREF	\
			|	SCCP_MTF_RLSD	\
			|	SCCP_MTF_DT1	\
			|	SCCP_MTF_DT2	\
			|	SCCP_MTF_AK	\
			|	SCCP_MTF_ED	\
			|	SCCP_MTF_EA	\
			|	SCCP_MTF_RSR	\
			|	SCCP_MTF_RSC	\
			|	SCCP_MTF_ERR	\
			|	SCCP_MTF_IT	\
			)
/*
 *  Conection-Less Messages
 */
#define SCCP_MTM_CL	(	SCCP_MTF_UDT	\
			|	SCCP_MTF_UDTS	\
			|	SCCP_MTF_XUDT	\
			|	SCCP_MTF_XUDTS	\
			|	SCCP_MTF_LUDT	\
			|	SCCP_MTF_LUDTS	\
			)
/*
 *  Messages that might require GTT
 */
#define SCCP_MTM_GT	( SCCP_MTM_CL | SCCP_MTF_CR )
/*
 *  Protocol Class 0 messages
 */
#define SCCP_MTM_PCLS0	(	SCCP_MTM_CL	)
/*
 *  Protocol Class 1 messages
 */
#define SCCP_MTM_PCLS1	(	SCCP_MTM_CL	)
/*
 *  Protocol Class 2 messages
 */
#define SCCP_MTM_PCLS2	(	SCCP_MTF_CR	\
			|	SCCP_MTF_CC	\
			|	SCCP_MTF_CREF	\
			|	SCCP_MTF_RLSD	\
			|	SCCP_MTF_DT1	\
			|	SCCP_MTF_ERR	\
			|	SCCP_MTF_IT	\
			)
/*
 *  Protocol Class 3 messages
 */
#define SCCP_MTM_PCLS3	(	SCCP_MTF_CR	\
			|	SCCP_MTF_CC	\
			|	SCCP_MTF_CREF	\
			|	SCCP_MTF_RLSD	\
			|	SCCP_MTF_DT2	\
			|	SCCP_MTF_AK	\
			|	SCCP_MTF_ED	\
			|	SCCP_MTF_EA	\
			|	SCCP_MTF_RSR	\
			|	SCCP_MTF_RSC	\
			|	SCCP_MTF_ERR	\
			|	SCCP_MTF_IT	\
			)

#define SCCP_PT_EOP	 0	/* End of Optional Parameters */
#define SCCP_PT_DLR	 1	/* Destination Local Reference */
#define SCCP_PT_SLR	 2	/* Source Local reference */
#define SCCP_PT_CDPA	 3	/* Called Party Address */
#define SCCP_PT_CGPA	 4	/* Calling Party Address */
#define SCCP_PT_PCLS	 5	/* Protocol Class */
#define SCCP_PT_SEG	 6	/* Segmenting/Reassembly */
#define SCCP_PT_RSN	 7	/* Receive Sequence Number */
#define SCCP_PT_SEQ	 8	/* Sequencing/Segmenting */
#define SCCP_PT_CRED	 9	/* Credit */
#define SCCP_PT_RELC	10	/* Release Cause */
#define SCCP_PT_RETC	11	/* Return Cause */
#define SCCP_PT_RESC	12	/* Reset Cause */
#define SCCP_PT_ERRC	13	/* Error Cause */
#define SCCP_PT_REFC	14	/* Refusal Cause */
#define SCCP_PT_DATA	15	/* Data */
#define SCCP_PT_SGMT	16	/* Segmentation */
#define SCCP_PT_HOPC	17	/* Hop Counter */
#define SCCP_PT_IMP	18	/* Importance */
#define SCCP_PT_LDATA	19	/* Long Data */
#define SCCP_PT_ISNI	-6	/* Intermediate Sig. Ntwk. Id */
#define SCCP_PT_INS	-7	/* Intermediate Network Selection */
#define SCCP_PT_MTI	-8	/* Message Type Interworking */

#define SCCP_PTF_EOP	(1<<SCCP_PT_EOP  )
#define SCCP_PTF_DLR	(1<<SCCP_PT_DLR  )
#define SCCP_PTF_SLR	(1<<SCCP_PT_SLR  )
#define SCCP_PTF_CDPA	(1<<SCCP_PT_CDPA )
#define SCCP_PTF_CGPA	(1<<SCCP_PT_CGPA )
#define SCCP_PTF_PCLS	(1<<SCCP_PT_PCLS )
#define SCCP_PTF_SEG	(1<<SCCP_PT_SEG  )
#define SCCP_PTF_RSN	(1<<SCCP_PT_RSN  )
#define SCCP_PTF_SEQ	(1<<SCCP_PT_SEQ  )
#define SCCP_PTF_CRED	(1<<SCCP_PT_CRED )
#define SCCP_PTF_RELC	(1<<SCCP_PT_RELC )
#define SCCP_PTF_RETC	(1<<SCCP_PT_RETC )
#define SCCP_PTF_RESC	(1<<SCCP_PT_RESC )
#define SCCP_PTF_ERRC	(1<<SCCP_PT_ERRC )
#define SCCP_PTF_REFC	(1<<SCCP_PT_REFC )
#define SCCP_PTF_DATA	(1<<SCCP_PT_DATA )
#define SCCP_PTF_SGMT	(1<<SCCP_PT_SGMT )
#define SCCP_PTF_HOPC	(1<<SCCP_PT_HOPC )
#define SCCP_PTF_IMP	(1<<SCCP_PT_IMP  )
#define SCCP_PTF_LDATA	(1<<SCCP_PT_LDATA)
#define SCCP_PTF_MTI	(0x80000000>>-SCCP_PT_MTI )
#define SCCP_PTF_INS	(0x80000000>>-SCCP_PT_INS )
#define SCCP_PTF_ISNI	(0x80000000>>-SCCP_PT_ISNI)

#define SCCP_MTI_OMT_UNQUAL	0x00
#define SCCP_MTI_OMT_UDT_S	0x01
#define SCCP_MTI_OMT_XUDT_S	0x02
#define SCCP_MTI_OMT_LUDT_S	0x03
#define SCCP_MTI_DROP		0x40

union nids {
	struct {
		uint8_t network;
		uint8_t cluster;
	} nid;
	uint16_t nsi __attribute__ ((packed));
};

typedef struct sccp_var {
	np_ulong len;
	uchar *ptr;
	// np_ulong val;
	mblk_t *buf;
} sccp_var_t;

typedef struct sccp_isni {
	np_ulong mi;			/* mark for identification indicator */
	np_ulong iri;			/* ISNI routing indicator */
	np_ulong ti;			/* type indicator */
	np_ulong count;			/* counter */
	np_ulong ns;			/* network specific (type 2 only) */
	size_t nids;			/* number of network ids */
	union nids u[7];		/* up to 7 network ids */
} sccp_isni_t;

typedef struct sccp_ins {
	np_ulong itype;			/* information type indicator */
	np_ulong rtype;			/* type of routing */
	np_ulong count;			/* conter */
	size_t nids;			/* number of network ids */
	union nids u[2];		/* up to 2 network ids */
} sccp_ins_t;

typedef struct sccp_sgmt {
	np_ulong first;			/* first bit */
	np_ulong pcl;			/* stored protocol class */
	np_ulong rem;			/* remaining segments */
	np_ulong slr;			/* source local reference */
} sccp_sgmt_t;

typedef struct sccp_msg {
	np_ulong flags;			/* translation result flags */
	mblk_t *bp;			/* identd (also unique GTT xaction id */
	queue_t *eq;			/* queue to write errors to */
	queue_t *xq;			/* queue to write results to */
	struct sc *sc;			/* SCCP-User to which this message belongs */
	struct mt *mt;			/* MTP-User to which this message belongs */
	np_ulong timestamp;		/* jiffie clock timestamp */
	np_ulong pvar;			/* protocol variant */
	struct {
		np_ulong opc;		/* origination point code */
		np_ulong dpc;		/* destination point code */
		np_ulong sls;		/* signalling link selection */
		np_ulong mp;		/* message priority */
	} rl;				/* routing label */
	np_ulong type;			/* message type */
	uint32_t parms;			/* parameter mask */
	np_ulong pcl;
	np_ulong ret;
	np_ulong slr;
	np_ulong dlr;
	np_ulong seg;
	np_ulong rsn;
	np_ulong ps;
	np_ulong more;
	np_ulong pr;
	np_ulong mti;
	sccp_ins_t ins;
	sccp_isni_t isni;
	struct sccp_addr cdpa;
	struct sccp_addr cgpa;
	np_ulong cred;
	np_ulong cause;
	struct sccp_sgmt sgmt;
	np_ulong hopc;
	np_ulong imp;
	struct sccp_var data;
	np_ulong fi;
	np_ulong assn;
	np_ulong apc;
	np_ulong smi;
	np_ulong cong;
} sccp_msg_t;

/*
 *  =========================================================================
 *
 *  LOCKING
 *
 *  =========================================================================
 */
static rwlock_t sccp_mux_lock = RW_LOCK_UNLOCKED;

static bool
sc_trylock(struct sc *sc, queue_t *q)
{
	return (mi_trylock(q) != NULL);
}
static void
sc_unlock(struct sc *sc)
{
	mi_release((caddr_t) sc);
}
static bool
sp_trylock(struct sp *sp, queue_t *q)
{
	unsigned long flags;
	bool result;

	spin_lock_irqsave(&sp->sq.lock, flags);
	if (sp->sq.users == 0) {
		sp->sq.users = 1;
		result = true;
	} else {
		result = false;
	}
	spin_unlock_irqrestore(&sp->sq.lock, flags);
	return (result);
}
static void
sp_unlock(struct sp *sp)
{
	unsigned long flags;

	spin_lock_irqsave(&sp->sq.lock, flags);
	sp->sq.users = 0;
	spin_unlock_irqrestore(&sp->sq.lock, flags);
}
static bool
mt_trylock(struct mt *mt, queue_t *q)
{
	return (mi_trylock(q) != NULL);
}
static void
mt_unlock(struct mt *mt)
{
	mi_release((caddr_t) mt);
}
static struct sc *
sc_acquire(queue_t *q)
{
	struct sp *sp;
	struct sc *sc;

	read_lock(&sccp_mux_lock);
	if (!(sc = SCCP_PRIV(q)) || !sc_trylock(sc, q)) {
		sc = NULL;
	} else if ((sp = sc->sp.sp) && !sp_trylock(sp, q)) {
		sc_unlock(sc);
		sc = NULL;
	}
	read_unlock(&sccp_mux_lock);
	return (sc);
}
static void
sc_release(struct sc *sc)
{
	struct sp *sp;

	if ((sp = sc->sp.sp))
		sp_unlock(sp);
	sc_unlock(sc);
}

/**
 * mt_acquire: - try to acquire a pointer to the MT structure from a MT queue procedure
 * @q: the upper queue
 *
 * Check the MT queue private pointer for NULL under mux read lock.  This is because unlinking lower
 * Streams can cause their private structures to be deallocated before their put and service
 * procedures are replaced.  Then walk the structures to the SP.
 *
 * Lower multiplex private structure can disappear btewen I_UNLINK/I_PUNLINK M_IOCTL and setq(9)
 * back  to the Stream head.
 */
static struct mt *
mt_acquire(queue_t *q)
{
	struct mt *mt;
	struct sp *sp;

	read_lock(&sccp_mux_lock);
	if (!(mt = MTP_PRIV(q)) || !mt_trylock(mt, q)) {
		mt = NULL;
	} else if ((sp = mt->sp) && !sp_trylock(sp, q)) {
		mt_unlock(mt);
		mt = NULL;
	}
	read_unlock(&sccp_mux_lock);
	return (mt);
}
static void
mt_release(struct mt *mt)
{
	struct sp *sp;

	if ((sp = mt->sp))
		sp_unlock(sp);
	mt_unlock(mt);
}

/*
 *  =========================================================================
 *
 *  PRIMITIVES
 *
 *  =========================================================================
 */
/*
 *  -------------------------------------------------------------------------
 *
 *  Primitives sent upstream
 *
 *  -------------------------------------------------------------------------
 */

/*
 *  M_FLUSH
 *  -----------------------------------
 */
static inline int
m_flush(queue_t *q, queue_t *pq, int band, int flags, int what)
{
	mblk_t *mp;

	if (likely((mp = mi_allocb(q, 2, BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_FLUSH;
		*mp->b_wptr++ = flags | (band ? FLUSHBAND : 0);
		*mp->b_wptr++ = band;
		mi_strlog(pq, STRLOGRX, SL_TRACE, "<- M_FLUSH");
		putq(pq, mp);
		return (0);
	}
	return (-ENOBUFS);
}

/*
 *  M_ERROR
 *  -----------------------------------
 */
static inline int
m_error(struct sc *sc, queue_t *q, mblk_t *bp, int error)
{
	mblk_t *mp;
	int hangup = 0;

	if (error < 0)
		error = -error;
	switch (error) {
	case EBUSY:
	case ENOBUFS:
	case ENOMEM:
	case EAGAIN:
		return (-error);
	case EPIPE:
	case ENETDOWN:
	case EHOSTUNREACH:
		hangup = 1;
	}
	if (likely((mp = mi_allocb(q, 2, BPRI_MED)) != NULL)) {
		if (hangup) {
			DB_TYPE(mp) = M_HANGUP;
			sccp_set_state(sc, NS_NOSTATES);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- M_HANGUP");
			ss7_oput(sc->oq, mp);
			return (-error);
		} else {
			DB_TYPE(mp) = M_ERROR;
			*mp->b_wptr++ = error < 0 ? -error : error;
			*mp->b_rptr++ = error < 0 ? -error : error;
			sccp_set_state(sc, NS_NOSTATES);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- M_ERROR");
			ss7_oput(sc->oq, mp);
			return (0);
		}
	}
	return (-ENOBUFS);
}

/*
 *  NPI Primitives
 *  -------------------------------------------------------------------------
 */

/**
 * n_conn_ind: - issue N_CONN_IND primitive
 * @sc: SCCP user private structure
 * @q: active queue
 * @msg: message to free upon success
 * @cp: connection pointer
 * @dp: user data
 *
 * FIXME: need to handle user data properly.
 */
static int
n_conn_ind(struct sc *sc, queue_t *q, mblk_t *msg, mblk_t *cp, mblk_t *dp)
{
	if (likely(canputnext(sc->oq))) {
		N_conn_ind_t *p;
		mblk_t *mp;

		size_t dst_len = sizeof(sc->dst) + sc->dst.alen;
		size_t src_len = sizeof(sc->src) + sc->src.alen;
		size_t qos_len = sizeof(sc->cqos);
		size_t msg_len = sizeof(*p) + PAD4(dst_len) + PAD4(src_len) + qos_len;

		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_CONN_IND;
			p->DEST_length = dst_len;
			p->DEST_offset = sizeof(*p);
			p->SRC_length = src_len;
			p->SRC_offset = p->DEST_offset + PAD4(p->DEST_length);
			p->SEQ_number = (np_ulong) (ulong) cp;
			p->CONN_flags = REC_CONF_OPT | EX_DATA_OPT;
			p->QOS_length = qos_len;
			p->QOS_offset = p->SRC_offset + PAD4(p->SRC_length);
			mp->b_wptr += sizeof(*p);
			fixme(("Need to take this from the cp not sc structure\n"));
			bcopy(&sc->dst, mp->b_wptr, dst_len);
			mp->b_wptr += PAD4(dst_len);
			fixme(("Need to take this from the cp not sc structure\n"));
			bcopy(&sc->src, mp->b_wptr, src_len);
			mp->b_wptr += PAD4(src_len);
			bcopy(&sc->cqos, mp->b_wptr, qos_len);
			mp->b_wptr += qos_len;
			mp->b_cont = dp;
			if (cp->b_cont == dp)
				cp->b_cont = NULL;
			if (msg && msg->b_cont == cp)
				msg->b_cont = NULL;
			freemsg(msg);
			bufq_queue(&sc->conq, cp);
			sccp_set_n_state(sc, NS_WRES_CIND);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_CONN_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/**
 * n_conn_con: - issue N_CONN_CON primitive
 * @sc: SCCP private structure
 * @q: active queue
 * @msg: message to free upon success
 * @flags: connection flags
 * @res: responding address
 * @dp: user data
 */
static int
n_conn_con(struct sc *sc, queue_t *q, mblk_t *msg, np_ulong flags, struct sccp_addr *res,
	   mblk_t *dp)
{
	if (likely(bcanputnext(sc->oq, 1))) {
		N_conn_con_t *p;
		mblk_t *mp;

		size_t res_len = res ? sizeof(*res) + res->alen : 0;
		size_t qos_len = sizeof(sc->cqos);
		size_t msg_len = sizeof(*p) + PAD4(res_len) + qos_len;

		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			mp->b_band = 1;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_CONN_CON;
			p->RES_length = res_len;
			p->RES_offset = sizeof(*p);
			p->CONN_flags = flags;
			p->QOS_length = qos_len;
			p->QOS_offset = p->RES_offset + PAD4(p->RES_length);
			mp->b_wptr += sizeof(*p);
			bcopy(res, mp->b_wptr, res_len);
			mp->b_wptr += PAD4(res_len);
			bcopy(&sc->cqos, mp->b_wptr, qos_len);
			mp->b_wptr += qos_len;
			mp->b_cont = dp;
			if (msg && msg->b_cont == dp)
				msg->b_cont = NULL;
			freemsg(msg);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_CONN_CON");
			sccp_set_n_state(sc, NS_DATA_XFER);
			putnext(sc->oq, mp);
			return (QR_DONE);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/**
 * n_discon_ind: - issue N_DISCON_IND primitive
 * @sc: SCCP private structure
 * @q: active queue
 * @msg: message to free upon success
 * @orig: origin of disconnect
 * @reason: reason for disconnect
 * @res: responding address
 * @cp: connection indication pointer
 * @dp: user data
 */
static int
n_discon_ind(struct sc *sc, queue_t *q, mblk_t *msg, np_ulong orig, np_long reason,
	     struct sccp_addr *res, mblk_t *cp, mblk_t *dp)
{
	if (likely(canputnext(sc->oq))) {
		N_discon_ind_t *p;
		mblk_t *mp;

		size_t res_len = res ? sizeof(*res) + res->alen : 0;
		size_t msg_len = sizeof(*p) + PAD4(res_len);

		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_DISCON_IND;
			p->DISCON_orig = orig;
			p->DISCON_reason = reason;
			p->RES_length = res_len;
			p->RES_offset = sizeof(*p);
			p->SEQ_number = (np_ulong) (ulong) cp;
			mp->b_wptr += sizeof(*p);
			bcopy(res, mp->b_wptr, res_len);
			mp->b_wptr += PAD4(res_len);
			mp->b_cont = dp;
			if (msg && msg->b_cont == dp)
				msg->b_cont = NULL;
			freemsg(msg);
			if (cp) {
				bufq_unlink(&sc->conq, cp);
				freemsg(cp);
			}
			sccp_set_n_state(sc, bufq_length(&sc->conq) ? NS_WRES_CIND : NS_IDLE);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_DISCON_IND");
			putnext(sc->oq, mp);
			return (QR_DONE);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/**
 * n_data_ind: - issue a N_DATA_IND primitive
 * @sc: SCCP private structure
 * @q: active queue
 * @msg: message to free upon success
 * @more: more data in NSDU
 * @qos: QOS parameters for data
 * @dp: user data
 */
static int
n_data_ind(struct sc *sc, queue_t *q, mblk_t *msg, np_ulong more, N_qos_sel_data_sccp_t * qos,
	   mblk_t *dp)
{
	if (likely(canputnext(sc->oq))) {
		N_data_ind_t *p;
		mblk_t *mp;

		size_t qos_len = qos ? sizeof(*qos) : 0;
		size_t msg_len = sizeof(*p) + qos_len;

		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_DATA_IND;
			p->DATA_xfer_flags = (more ? N_MORE_DATA_FLAG : 0);
			mp->b_wptr += sizeof(*p);
			bcopy(qos, mp->b_wptr, qos_len);
			mp->b_wptr += qos_len;
			mp->b_cont = dp;
			if (msg && msg->b_cont == dp)
				msg->b_cont = NULL;
			freemsg(msg);
			mi_strlog(q, STRLOGDA, SL_TRACE, "<- N_DATA_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/**
 * n_exdata_ind: - issue a N_EXDATA_IND primitive
 * @sc: SCCP private structure
 * @q: active queue
 * @msg: message to free upon success
 * @more: more data in ENSDU
 * @qos: QOS parameters for data
 * @dp: user data
 */
static int
n_exdata_ind(struct sc *sc, queue_t *q, mblk_t *msg, np_ulong more, N_qos_sel_data_sccp_t * qos,
	     mblk_t *dp)
{
	if (likely(bcanputnext(sc->oq, 1))) {
		N_exdata_ind_t *p;
		mblk_t *mp;

		size_t qos_len = qos ? sizeof(*qos) : 0;
		size_t msg_len = sizeof(*p) + qos_len;

		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			mp->b_band = 1;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_EXDATA_IND;
			mp->b_wptr += sizeof(*p);
			bcopy(qos, mp->b_wptr, qos_len);
			mp->b_wptr += qos_len;
			mp->b_cont = dp;
			if (msg && msg->b_cont == dp)
				msg->b_cont = NULL;
			freemsg(msg);
			mi_strlog(q, STRLOGDA, SL_TRACE, "<- N_EXDATA_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  N_INFO_ACK
 *  -----------------------------------
 */
static int
n_info_ack(struct sc *sc, queue_t *q, mblk_t *msg)
{
	N_info_ack_t *p;
	mblk_t *mp;

	size_t add_len = sizeof(sc->src) + sc->src.alen;
	size_t qos_len = sizeof(sc->iqos);
	size_t qor_len = sizeof(sc->iqor);
	size_t pro_len = sizeof(sc->pro);
	size_t msg_len = sizeof(*p) + PAD4(add_len) + qos_len + qor_len + PAD4(pro_len);

	if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_PCPROTO;
		p = (typeof(p)) mp->b_wptr;
		*p = sc->info;
		p->PRIM_type = N_INFO_ACK;
		p->ADDR_length = add_len;
		p->ADDR_offset = sizeof(*p);
		p->QOS_length = qos_len;
		p->QOS_offset = p->ADDR_offset + PAD4(p->ADDR_length);
		p->QOS_range_length = qor_len;
		p->QOS_range_offset = p->QOS_offset + p->QOS_length;
		p->CURRENT_state = sccp_get_n_state(sc);
		p->PROTOID_length = pro_len;
		p->PROTOID_offset = sizeof(*p) + add_len + qos_len + qor_len;
		mp->b_wptr += sizeof(*p);
		bcopy(&sc->src, mp->b_wptr, add_len);
		mp->b_wptr += PAD4(add_len);
		bcopy(&sc->iqos, mp->b_wptr, qos_len);
		mp->b_wptr += qos_len;
		bcopy(&sc->iqor, mp->b_wptr, qor_len);
		mp->b_wptr += qor_len;
		bcopy(&sc->pro, mp->b_wptr, pro_len);
		mp->b_wptr += PAD4(pro_len);
		freemsg(msg);
		mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_INFO_ACK");
		putnext(sc->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}

/*
 *  N_BIND_ACK
 *  -----------------------------------
 */
static int
n_bind_ack(struct sc *sc, queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	N_bind_ack_t *p;
	struct sccp_addr *add, *ss = &sc->src;
	uint32_t *pro;
	size_t add_len = ss ? sizeof(*ss) + ss->alen : 0;
	size_t pro_len = sizeof(*pro);
	size_t msg_len = sizeof(*p) + add_len + pro_len;

	if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_PCPROTO;
		p = (typeof(p)) mp->b_wptr;
		p->PRIM_type = N_BIND_ACK;
		p->ADDR_length = add_len;
		p->ADDR_offset = add_len ? sizeof(*p) : 0;
		p->CONIND_number = sc->conind;
		p->TOKEN_value = (np_ulong) (ulong) sc->oq;
		p->PROTOID_length = pro_len;
		p->PROTOID_offset = pro_len ? sizeof(*p) + add_len : 0;
		mp->b_wptr += sizeof(*p);
		if (ss) {
			add = (typeof(add)) mp->b_wptr;
			bcopy(ss, add, sizeof(*ss) + ss->alen);
			mp->b_wptr += add_len;
		}
		pro = (typeof(pro)) mp->b_wptr;
		mp->b_wptr += sizeof(*pro);
		*pro = 3;	/* SI value for SCCP */
		sccp_set_n_state(sc, NS_IDLE);
		freemsg(msg);
		mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_BIND_ACK");
		putnext(sc->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}

/*
 *  N_ERROR_ACK
 *  -----------------------------------
 */
static int
n_error_ack(struct sc *sc, queue_t *q, const np_long prim, int err)
{
	mblk_t *mp;
	N_error_ack_t *p;

	switch (err) {
	case -EBUSY:
	case -EAGAIN:
	case -ENOMEM:
	case -ENOBUFS:
		return (err);
	case 0:
		return (err);
	}
	if (likely((mp = mi_allocb(q, sizeof(*p), BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_PCPROTO;
		p = (typeof(p)) mp->b_wptr;
		p->PRIM_type = N_ERROR_ACK;
		p->ERROR_prim = prim;
		p->NPI_error = err < 0 ? NSYSERR : err;
		p->UNIX_error = err < 0 ? -err : 0;
		mp->b_wptr += sizeof(*p);
		switch (sccp_get_n_state(sc)) {
		case NS_WACK_OPTREQ:
		case NS_WACK_UREQ:
		case NS_WCON_CREQ:
			sccp_set_n_state(sc, NS_IDLE);
			break;
		case NS_WCON_RREQ:
			sccp_set_n_state(sc, NS_DATA_XFER);
			break;
		case NS_WACK_BREQ:
			sccp_set_n_state(sc, NS_UNBND);
			break;
		case NS_WACK_CRES:
			sccp_set_n_state(sc, NS_WRES_CIND);
			break;
		case NS_WACK_DREQ6:
			sccp_set_n_state(sc, NS_WCON_CREQ);
			break;
		case NS_WACK_DREQ7:
			sccp_set_n_state(sc, NS_WRES_CIND);
			break;
		case NS_WACK_DREQ9:
			sccp_set_n_state(sc, NS_DATA_XFER);
			break;
		case NS_WACK_DREQ10:
			sccp_set_n_state(sc, NS_WCON_RREQ);
			break;
		case NS_WACK_DREQ11:
			sccp_set_n_state(sc, NS_WRES_RIND);
			break;
		default:
			/* Note: if we are not in a WACK state we simply do not change state.  This 
			   occurs normally when we send NOUTSTATE or NNOTSUPPORT or are responding
			   to an N_OPTMGMT_REQ in other than the NS_IDLE state. */
			break;
		}
		mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_ERROR_ACK");
		putnext(sc->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}

/*
 *  N_OK_ACK
 *  -----------------------------------
 */
static int
n_ok_ack(struct sc *sc, queue_t *q, mblk_t *msg, np_long prim, np_ulong seq, np_ulong tok)
{
	mblk_t *mp;
	N_ok_ack_t *p;

	if (likely((mp = mi_allocb(q, sizeof(*p), BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_PCPROTO;
		p = (typeof(p)) mp->b_wptr;
		mp->b_wptr += sizeof(*p);
		p->PRIM_type = N_OK_ACK;
		p->CORRECT_prim = prim;
		switch (sccp_get_n_state(sc)) {
		case NS_WACK_OPTREQ:
			sccp_set_n_state(sc, NS_IDLE);
			break;
		case NS_WACK_RRES:
			sccp_set_n_state(sc, NS_DATA_XFER);
			break;
		case NS_WACK_UREQ:
			sccp_set_n_state(sc, NS_UNBND);
			break;
		case NS_WACK_CRES:
		{
			queue_t *aq = (queue_t *) (ulong) tok;	/* FIXME: look up instead */
			struct sc *ap = SCCP_PRIV(aq);

			if (ap) {
				ap->i_state = NS_DATA_XFER;
#if 0
				sccp_cleanup_read(sc->oq);	/* deliver to user what is possible 
								 */
				sccp_transmit_wakeup(sc->oq);	/* reply to peer what is necessary */
#endif
			}
			if (seq) {
				bufq_unlink(&sc->conq, (mblk_t *) (ulong) seq);	/* FIXME: look up
										   instead */
				freemsg((mblk_t *) (ulong) seq);	/* FIXME: look up instead */
			}
			if (aq != sc->oq) {
				if (bufq_length(&sc->conq))
					sccp_set_n_state(sc, NS_WRES_CIND);
				else
					sccp_set_n_state(sc, NS_IDLE);
			}
			break;
		}
		case NS_WACK_DREQ7:
			if (seq)
				bufq_unlink(&sc->conq, (mblk_t *) (ulong) seq);	/* FIXME: look up
										   instead */
		case NS_WACK_DREQ6:
		case NS_WACK_DREQ9:
		case NS_WACK_DREQ10:
		case NS_WACK_DREQ11:
			if (bufq_length(&sc->conq))
				sccp_set_n_state(sc, NS_WRES_CIND);
			else
				sccp_set_n_state(sc, NS_IDLE);
			break;
		default:
			/* Note: if we are not in a WACK state we simply do not change state.  This 
			   occurs normally when we are responding to a N_OPTMGMT_REQ in other than
			   the NS_IDLE state. */
			break;
		}
		freemsg(msg);
		mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_OK_ACK");
		putnext(sc->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}

/*
 *  N_UNITDATA_IND
 *  -----------------------------------
 */
static int
n_unitdata_ind(struct sc *sc, queue_t *q, mblk_t *msg, struct sccp_addr *src, struct sccp_addr *dst,
	       mblk_t *dp)
{
	mblk_t *mp;
	N_unitdata_ind_t *p;
	size_t src_len = src ? sizeof(*src) + src->alen : 0;
	size_t dst_len = dst ? sizeof(*dst) + dst->alen : 0;
	size_t msg_len = sizeof(*p) + PAD4(src_len) + PAD4(dst_len);

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_UNITDATA_IND;
			p->SRC_length = src_len;
			p->SRC_offset = sizeof(*p);
			p->DEST_length = dst_len;
			p->DEST_offset = sizeof(*p) + PAD4(src_len);
			p->ERROR_type = 0;
			mp->b_wptr += sizeof(*p);
			bcopy(src, mp->b_wptr, src_len);
			mp->b_wptr += PAD4(src_len);
			bcopy(dst, mp->b_wptr, dst_len);
			mp->b_wptr += PAD4(dst_len);
			mp->b_cont = dp;
			if (msg && msg->b_cont == dp)
				msg->b_cont = NULL;
			freemsg(msg);
			mi_strlog(q, STRLOGDA, SL_TRACE, "<- N_UNITDATA_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  N_UDERROR_IND
 *  -----------------------------------
 *
 *  NOTE:- We might want to do a special N-NOTICE primitive here because the N_UDERROR_IND primitive does not have
 *  that much information about the returned message.  Or, if we attach the M_DATA block, the use might be able to
 *  gleen more information from the message itself.  Another approach is to stuff some extra information at the end
 *  of the primitive.  Here we attache the returned message in the M_DATA portion.
 */
static int
n_uderror_ind(struct sc *sc, queue_t *q, mblk_t *msg, struct sccp_addr *dst, mblk_t *dp,
	      np_ulong etype)
{
	mblk_t *mp;
	N_uderror_ind_t *p;
	size_t dst_len = dst ? sizeof(*dst) + dst->alen : 0;
	size_t msg_len = sizeof(*p) + PAD4(dst_len);

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_UNITDATA_IND;
			p->DEST_length = dst_len;
			p->DEST_offset = sizeof(*p);
			p->RESERVED_field = 0;
			p->ERROR_type = etype;
			mp->b_wptr += sizeof(*p);
			bcopy(dst, mp->b_wptr, dst_len);
			mp->b_wptr += PAD4(dst_len);
			mp->b_cont = dp;
			if (msg && msg->b_cont == dp)
				msg->b_cont = NULL;
			freemsg(msg);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_UDERROR_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  N_DATACK_IND
 *  -----------------------------------
 */
static int
n_datack_ind(struct sc *sc, queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	N_datack_ind_t *p;
	N_qos_sel_data_sccp_t *qos;
	size_t qos_len = sizeof(*qos);
	size_t msg_len = sizeof(*p) + qos_len;

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_DATACK_IND;
			mp->b_wptr += sizeof(*p);
			qos = (typeof(qos)) mp->b_wptr;
			mp->b_wptr += sizeof(*qos);
			qos->n_qos_type = N_QOS_SEL_DATA_SCCP;
			qos->protocol_class = sc->pcl;	/* FIXME */
			qos->option_flags = sc->flags;	/* FIXME */
			qos->importance = sc->imp;	/* FIXME */
			qos->sequence_selection = sc->sls;	/* FIXME */
			qos->message_priority = sc->mp;	/* FIXME */
			freemsg(msg);
			mi_strlog(q, STRLOGDA, SL_TRACE, "<- N_DATACK_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  N_RESET_IND
 *  -----------------------------------
 */
static int
n_reset_ind(struct sc *sc, queue_t *q, mblk_t *msg, np_ulong orig, np_ulong reason, mblk_t *cp)
{
	mblk_t *mp;
	N_reset_ind_t *p;

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, sizeof(*p), BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_RESET_IND;
			p->RESET_orig = orig;
			p->RESET_reason = reason;
			mp->b_wptr += sizeof(*p);
			bufq_queue(&sc->conq, cp);
			sccp_set_n_state(sc, NS_WRES_RIND);
			freemsg(msg);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_RESET_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  N_RESET_CON
 *  -----------------------------------
 */
static int
n_reset_con(struct sc *sc, queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	N_reset_con_t *p;

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, sizeof(*p), BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_RESET_CON;
			mp->b_wptr += sizeof(*p);
			sccp_set_n_state(sc, NS_DATA_XFER);
			freemsg(msg);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_RESET_CON");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

#ifdef NEVER
/*
 *  N_RECOVER_IND       29 - NC Recovery indication
 *  ---------------------------------------------------------
 */
static int
n_recover_ind(struct sc *sc, queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	N_recover_ind_t *p;

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, sizeof(*p), BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_RECOVER_IND;
			mp->b_wptr += sizeof(*p);
			sccp_set_n_state(sc, NS_DATA_XFER);
			freemsg(msg);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_RECOVER_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  N_RETRIEVE_IND      32 - NC Retrieval indication
 *  ---------------------------------------------------------
 */
static int
n_retrieve_ind(struct sc *sc, queue_t *q, mblk_t *msg, mblk_t *dp)
{
	mblk_t *mp;
	N_retrieve_ind_t *p;

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, sizeof(*p), BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_RETREIVE_IND;
			mp->b_wptr += sizeof(*p);
			mp->b_cont = dp;
			sccp_set_n_state(sc, NS_IDLE);
			freemsg(msg);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_RETRIEVE_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  N_RETRIEVE_CON      33 - NC Retrieval complete confirmation
 *  -----------------------------------------------------------
 */
static int
n_retrieve_con(struct sc *sc, queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	N_retrieve_con_t *p;

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, sizeof(*p), BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_RETRIEVE_CON;
			mp->b_wptr += sizeof(*p);
			sccp_set_n_state(sc, NS_IDLE);
			freemsg(msg);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_RETRIEVE_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}
#endif

#if 0
/*
 *  N_ORDREL_IND
 *  -----------------------------------------------------------------
 *  There is really no orderly release for NPI, so we just echo an orderly
 *  releae request back.
 */
static int sccp_ordrel_req(struct sc *sc, queue_t *q, mblk_t *msg);
static int
n_ordrel_ind(struct sc *sc, queue_t *q, mblk_t *msg)
{
	return sccp_ordrel_req(sc, q);
}
#endif

/*
 *  N_NOTICE_IND
 *  -----------------------------------------------------------------
 */
static int
n_notice_ind(struct sc *sc, queue_t *q, mblk_t *msg, np_ulong cause, struct sccp_addr *dst,
	     struct sccp_addr *src, np_ulong pri, np_ulong seq, np_ulong pcl, np_ulong imp,
	     mblk_t *dp)
{
	mblk_t *mp;
	N_notice_ind_t *p;
	N_qos_sel_data_sccp_t *qos;
	size_t dst_len = dst ? sizeof(*dst) + dst->alen : 0;
	size_t src_len = src ? sizeof(*src) + src->alen : 0;
	size_t qos_len = sizeof(*qos);
	size_t msg_len = sizeof(*p) + PAD4(dst_len) + PAD4(src_len) + qos_len;

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_NOTICE_IND;
			p->DEST_length = dst_len;
			p->DEST_offset = dst_len ? sizeof(*p) : 0;
			p->SRC_length = src_len;
			p->SRC_offset = src_len ? sizeof(*p) + PAD4(dst_len) : 0;
			p->QOS_length = qos_len;
			p->QOS_offset = sizeof(*p) + PAD4(dst_len) + PAD4(src_len);
			p->RETURN_cause = cause;
			mp->b_wptr += sizeof(*p);
			bcopy(dst, mp->b_wptr, dst_len);
			mp->b_wptr += PAD4(dst_len);
			bcopy(src, mp->b_wptr, src_len);
			mp->b_wptr += PAD4(src_len);
			qos = (typeof(qos)) mp->b_wptr;
			mp->b_wptr += sizeof(*qos);
			qos->n_qos_type = N_QOS_SEL_DATA_SCCP;
			qos->protocol_class = pcl;
			qos->option_flags = 1;
			qos->importance = imp;
			qos->sequence_selection = seq;
			qos->message_priority = pri;
			freemsg(msg);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_NOTICE_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  N_INFORM_IND
 *  -----------------------------------------------------------------
 */
static inline int
n_inform_ind(struct sc *sc, queue_t *q, mblk_t *msg, N_qos_sel_infr_sccp_t * qos, np_ulong reason)
{
	mblk_t *mp;
	N_inform_ind_t *p;
	size_t qos_len = qos ? sizeof(*qos) : 0;
	size_t msg_len = sizeof(*p) + qos_len;

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_INFORM_IND;
			p->QOS_length = qos_len;
			p->QOS_offset = qos_len ? sizeof(*p) : 0;
			p->REASON = reason;
			mp->b_wptr += sizeof(*p);
			bcopy(qos, mp->b_wptr, qos_len);
			mp->b_wptr += qos_len;
			freemsg(msg);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_INFORM_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  N_COORD_IND
 *  -----------------------------------------------------------------
 */
static inline int
n_coord_ind(struct sc *sc, queue_t *q, mblk_t *msg, struct sccp_addr *add)
{
	mblk_t *mp;
	N_coord_ind_t *p;
	size_t add_len = add ? sizeof(*add) + add->alen : 0;
	size_t msg_len = sizeof(*p) + add_len;

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_COORD_IND;
			p->ADDR_length = add_len;
			p->ADDR_offset = add_len ? sizeof(*p) : 0;
			mp->b_wptr += sizeof(*p);
			bcopy(add, mp->b_wptr, add_len);
			mp->b_wptr += add_len;
			freemsg(msg);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_INFORM_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  N_COORD_CON
 *  -----------------------------------------------------------------
 */
static inline int
n_coord_con(struct sc *sc, queue_t *q, mblk_t *msg, struct sccp_addr *add, np_ulong smi)
{
	mblk_t *mp;
	N_coord_con_t *p;
	size_t add_len = add ? sizeof(*add) + add->alen : 0;
	size_t msg_len = sizeof(*p) + add_len;

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_COORD_CON;
			p->ADDR_length = add_len;
			p->ADDR_offset = sizeof(*p);
			p->SMI = smi;
			mp->b_wptr += sizeof(*p);
			bcopy(add, mp->b_wptr, add_len);
			mp->b_wptr += add_len;
			freemsg(msg);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_COORD_CON");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  N_STATE_IND
 *  -----------------------------------------------------------------
 */
static int
n_state_ind(struct sc *sc, queue_t *q, mblk_t *msg, struct sccp_addr *add, np_ulong status,
	    np_ulong smi)
{
	mblk_t *mp;
	N_state_ind_t *p;
	size_t add_len = add ? sizeof(*add) + add->alen : 0;
	size_t msg_len = sizeof(*p) + add_len;

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_STATE_IND;
			p->ADDR_length = add_len;
			p->ADDR_offset = sizeof(*p);
			p->STATUS = status;
			p->SMI = smi;
			mp->b_wptr += sizeof(*p);
			bcopy(add, mp->b_wptr, add_len);
			mp->b_wptr += add_len;
			freemsg(msg);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_STATE_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  N_PCSTATE_IND
 *  -----------------------------------------------------------------
 *  Indicate the state of a remote SCCP User or a remote Signalling Point (MTP).
 */
static int
n_pcstate_ind(struct sc *sc, queue_t *q, mblk_t *msg, struct mtp_addr *add, np_ulong status)
{
	mblk_t *mp;
	N_pcstate_ind_t *p;
	size_t add_len = add ? sizeof(*add) : 0;
	size_t msg_len = sizeof(*p) + add_len;

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_PCSTATE_IND;
			p->ADDR_length = add_len;
			p->ADDR_offset = add_len ? sizeof(*p) : 0;
			p->STATUS = status;
			mp->b_wptr += sizeof(*p);
			bcopy(add, mp->b_wptr, add_len);
			mp->b_wptr += add_len;
			freemsg(msg);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_PCSTATE_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  N_TRAFFIC_IND
 *  -----------------------------------------------------------------
 *  ANSI T1.112.1-2000 2.3.2.2.2 Affected User.  The parameter "affected user" identifies an affected user that is
 *  failed, withdrawn (has requested withdrawal), congested or allowed.  The affected user parameter contains the
 *  same type of information as the called address and calling address.  In the case of N-TRAFFIC primitive (see
 *  2.3.2.3.3), the parameter identifies the user from which a preferred subsystem is receiving backup traffic.
 */
static inline int
n_traffic_ind(struct sc *sc, queue_t *q, mblk_t *msg, struct sccp_addr *add, np_ulong tmix)
{
	mblk_t *mp;
	N_traffic_ind_t *p;
	size_t add_len = add ? sizeof(*add) : 0;
	size_t msg_len = sizeof(*p) + add_len;

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = N_TRAFFIC_IND;
			p->ADDR_length = add_len;
			p->ADDR_offset = add_len ? sizeof(*p) : 0;
			p->TRAFFIC_mix = tmix;
			mp->b_wptr += sizeof(*p);
			bcopy(add, mp->b_wptr, add_len);
			mp->b_wptr += add_len;
			freemsg(msg);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- N_TRAFFIC_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  TPI Primitives
 *  -------------------------------------------------------------------------
 */
/*
 *  T_CONN_IND          11 - connection indication
 *  -----------------------------------------------------------------
 *  We get the information from the CR message in the connection indication.  We queue the CR message (complete with
 *  decode parameter block) itself as the connection indication.  The sequence number is really just a pointer to
 *  the first mblk_t in the received CR message.
 */
static int
t_conn_ind(struct sc *sc, queue_t *q, mblk_t *msg, mblk_t *cp, mblk_t *dp)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) cp->b_rptr;
	struct T_conn_ind *p;
	struct t_opthdr *oh;
	struct sccp_addr *src = &m->cgpa;
	size_t src_len = sizeof(*src) + src->alen;
	size_t opt_len = sizeof(*oh) + sizeof(t_scalar_t);
	size_t msg_len = sizeof(*p) + PAD4(src_len) + opt_len;

	if (likely(bufq_length(&sc->conq) < sc->conind)) {
		if (likely(canputnext(sc->oq))) {
			if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
				DB_TYPE(mp) = M_PROTO;
				p = (typeof(p)) mp->b_wptr;
				p->PRIM_type = T_CONN_IND;
				p->SRC_length = src_len;
				p->SRC_offset = sizeof(*p);
				p->OPT_length = opt_len;
				p->OPT_offset = sizeof(*p) + PAD4(src_len);
				p->SEQ_number = (np_ulong) (ulong) cp;
				mp->b_wptr += sizeof(*p);
				bcopy(src, mp->b_wptr, src_len);
				mp->b_wptr += PAD4(src_len);
				oh = (typeof(oh)) mp->b_wptr;
				mp->b_wptr += sizeof(*oh);
				oh->len = opt_len;
				oh->level = T_SS7_SCCP;
				oh->name = T_SCCP_PCLASS;
				oh->status = T_SUCCESS;
				*(t_uscalar_t *) mp->b_wptr = m->pcl;
				mp->b_wptr += sizeof(t_uscalar_t);
				mp->b_cont = dp;
				if (msg && msg->b_cont == dp)
					msg->b_cont = NULL;
				freemsg(msg);
				bufq_queue(&sc->conq, cp);
				sccp_set_t_state(sc, TS_WRES_CIND);
				mi_strlog(q, STRLOGRX, SL_TRACE, "<- T_CONN_IND");
				putnext(sc->oq, mp);
				return (0);
			}
			return (-ENOBUFS);
		}
		return (-EBUSY);
	}
	noenable(q);
	return (-ERESTART);
}

/**
 * t_conn_con: - issue a T_CONN_CON primitive
 * @sc: SCCP private structure
 * @q: active queue
 * @msg: message to free upon success
 * @flags: connection flags
 * @res: responding address
 * @dp: user data
 *
 * The only options with end-to-end significance that are negotiated is the protocol class.
 */
static int
t_conn_con(struct sc *sc, queue_t *q, mblk_t *msg, np_ulong flags, struct sccp_addr *res,
	   mblk_t *dp)
{
	struct T_conn_con *p;
	struct t_opthdr *oh;
	mblk_t *mp;

	size_t res_len = res ? sizeof(*res) + res->alen : 0;
	size_t opt_len = sizeof(*oh) + sizeof(t_scalar_t);
	size_t msg_len = sizeof(*p) + PAD4(res_len) + opt_len;

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			mp->b_band = 1;	/* expedite */
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = T_CONN_CON;
			p->RES_length = res_len;
			p->RES_offset = sizeof(*p);
			p->OPT_length = opt_len;
			p->OPT_offset = p->RES_offset + PAD4(p->RES_length);
			mp->b_wptr += sizeof(*p);
			bcopy(res, mp->b_wptr, res_len);
			mp->b_wptr += PAD4(res_len);
			oh = (typeof(oh)) mp->b_wptr;
			mp->b_wptr += sizeof(*oh);
			oh->len = opt_len;
			oh->level = T_SS7_SCCP;
			oh->name = T_SCCP_PCLASS;
			oh->status = T_SUCCESS;
			*(t_uscalar_t *) mp->b_wptr = sc->cqos.protocol_class;
			mp->b_wptr += sizeof(t_uscalar_t);
			sccp_set_t_state(sc, TS_DATA_XFER);
			mp->b_cont = dp;
			if (msg && msg->b_cont == dp)
				msg->b_cont = NULL;
			freemsg(msg);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- T_CONN_CON");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  T_DISCON_IND        13 - disconnect indication
 *  -----------------------------------------------------------------
 *  We use the address of the mblk_t that contains the CR message as a SEQ_number for connection indications that
 *  are rejected with a disconnect indication as well.  We can use this to directly address the mblk in the
 *  connection indication bufq.
 */
static int
t_discon_ind(struct sc *sc, queue_t *q, mblk_t *msg, np_long reason, mblk_t *seq, mblk_t *dp)
{
	mblk_t *mp;
	struct T_discon_ind *p;
	size_t msg_len = sizeof(*p);

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = T_DISCON_IND;
			p->DISCON_reason = reason;
			p->SEQ_number = (np_ulong) (ulong) seq;
			mp->b_wptr += sizeof(*p);
			if (seq) {
				bufq_unlink(&sc->conq, seq);
				freemsg(seq);
			}
			if (!bufq_length(&sc->conq))
				sccp_set_t_state(sc, TS_IDLE);
			else
				sccp_set_t_state(sc, TS_WRES_CIND);
			mp->b_cont = dp;
			if (msg && msg->b_cont == dp)
				msg->b_cont = NULL;
			freemsg(msg);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- T_DISCON_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/**
 * t_data_ind: - issued a T_DATA_IND primitive
 * @sc: SCCP private structure
 * @q: active queue
 * @msg: message to free upon success
 * @more: more data in ETSDU
 * @dp: user data
 */
static int
t_data_ind(struct sc *sc, queue_t *q, mblk_t *msg, np_ulong more, mblk_t *dp)
{
	if (likely(canputnext(sc->oq))) {
		struct T_data_ind *p;
		mblk_t *mp;

		size_t msg_len = sizeof(*p);

		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = T_DATA_IND;
			p->MORE_flag = more;
			mp->b_wptr += sizeof(*p);
			mp->b_cont = dp;
			if (msg && msg->b_cont == dp)
				msg->b_cont = NULL;
			freemsg(msg);
			mi_strlog(q, STRLOGDA, SL_TRACE, "<- T_DATA_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/**
 * t_exdata_ind: - issued a T_EXDATA_IND primitive
 * @sc: SCCP private structure
 * @q: active queue
 * @msg: message to free upon success
 * @more: more data in ETSDU
 * @dp: user data
 */
static int
t_exdata_ind(struct sc *sc, queue_t *q, mblk_t *msg, np_ulong more, mblk_t *dp)
{

	if (likely(canputnext(sc->oq))) {
		struct T_exdata_ind *p;
		mblk_t *mp;

		size_t msg_len = sizeof(*p);

		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			mp->b_band = 1;	/* expedite */
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = T_EXDATA_IND;
			p->MORE_flag = more;
			mp->b_wptr += sizeof(*p);
			mp->b_cont = dp;
			if (msg && msg->b_cont == dp)
				msg->b_cont = NULL;
			freemsg(msg);
			mi_strlog(q, STRLOGDA, SL_TRACE, "<- T_EXDATA_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/**
 * t_optdata_ind: - issue a T_OPTDATA_IND primitive
 * @sc: SCCP private structure
 * @q: active queue
 * @msg: message to free upon success
 * @exp: data is expedited
 * @more: more data in ETSDU
 * @qos: QOS parameters for data
 * @dp: user data
 */
static int
t_optdata_ind(struct sc *sc, queue_t *q, mblk_t *msg, t_uscalar_t exp, t_uscalar_t more,
	      N_qos_sel_data_sccp_t * qos, mblk_t *dp)
{
	uchar b_band = exp ? 1 : 0;

	if (likely(bcanputnext(sc->oq, b_band))) {
		struct T_optdata_ind *p;
		struct t_opthdr *oh;
		mblk_t *mp;

		size_t qos_len = qos ? (5 * (sizeof(*oh) + sizeof(t_uscalar_t))) : 0;
		size_t msg_len = sizeof(*p) + qos_len;

		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			mp->b_band = b_band;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = T_OPTDATA_IND;
			p->DATA_flag = (exp ? T_ODF_EX : 0) | (more ? T_ODF_MORE : 0);
			p->OPT_length = qos_len;
			p->OPT_offset = sizeof(*p);
			mp->b_wptr += sizeof(*p);

			oh = (typeof(oh)) mp->b_wptr;
			oh->len = sizeof(*oh) + sizeof(t_uscalar_t);
			oh->level = T_SS7_SCCP;
			oh->name = T_SCCP_SEQ_CTRL;
			oh->status = T_SUCCESS;
			mp->b_wptr += sizeof(*oh);
			*(t_uscalar_t *) mp->b_wptr = qos->sequence_selection;
			mp->b_wptr += sizeof(t_uscalar_t);

			oh = (typeof(oh)) mp->b_wptr;
			oh->len = sizeof(*oh) + sizeof(t_uscalar_t);
			oh->level = T_SS7_SCCP;
			oh->name = T_SCCP_PRIORITY;
			oh->status = T_SUCCESS;
			mp->b_wptr += sizeof(*oh);
			*(t_uscalar_t *) mp->b_wptr = qos->message_priority;
			mp->b_wptr += sizeof(t_uscalar_t);

			oh = (typeof(oh)) mp->b_wptr;
			oh->len = sizeof(*oh) + sizeof(t_uscalar_t);
			oh->level = T_SS7_SCCP;
			oh->name = T_SCCP_PCLASS;
			oh->status = T_SUCCESS;
			mp->b_wptr += sizeof(*oh);
			*(t_uscalar_t *) mp->b_wptr = qos->protocol_class;
			mp->b_wptr += sizeof(t_uscalar_t);

			oh = (typeof(oh)) mp->b_wptr;
			oh->len = sizeof(*oh) + sizeof(t_uscalar_t);
			oh->level = T_SS7_SCCP;
			oh->name = T_SCCP_IMPORTANCE;
			oh->status = T_SUCCESS;
			mp->b_wptr += sizeof(*oh);
			*(t_uscalar_t *) mp->b_wptr = qos->importance;
			mp->b_wptr += sizeof(t_uscalar_t);

			oh = (typeof(oh)) mp->b_wptr;
			oh->len = sizeof(*oh) + sizeof(t_uscalar_t);
			oh->level = T_SS7_SCCP;
			oh->name = T_SCCP_RET_ERROR;
			oh->status = T_SUCCESS;
			mp->b_wptr += sizeof(*oh);
			*(t_uscalar_t *) mp->b_wptr = qos->option_flags;
			mp->b_wptr += sizeof(t_uscalar_t);
			mp->b_cont = dp;
			if (msg && msg->b_cont == dp)
				msg->b_cont = NULL;
			freemsg(msg);
			mi_strlog(q, STRLOGDA, SL_TRACE, "<- T_OPTDATA_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  T_INFO_ACK          16 - information acknowledgement
 *  -----------------------------------------------------------------
 */
static int
t_info_ack(struct sc *sc, queue_t *q, mblk_t *msg)
{
	struct T_info_ack *p;
	mblk_t *mp;

	size_t msg_len = sizeof(*p);

	if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
		t_uscalar_t serv = sc->pcl < 2 ? T_CLTS : T_COTS_ORD;

		DB_TYPE(mp) = M_PCPROTO;
		p = (typeof(p)) mp->b_wptr;
		fixme(("Still some things to double-check here\n"));
		p->PRIM_type = T_INFO_ACK;
		p->TSDU_size = sc->info.NSDU_size;	/* no limit on TSDU size */
		p->ETSDU_size = sc->info.ENSDU_size;	/* no limit on ETSDU size */
		p->CDATA_size = sc->info.CDATA_size;	/* no limit on CDATA size */
		p->DDATA_size = sc->info.DDATA_size;	/* no limit on DDATA size */
		p->ADDR_size = sc->info.ADDR_size;	/* no limit on ADDR size */
		p->OPT_size = T_INFINITE;	/* no limit on OPTIONS size */
		p->TIDU_size = sc->info.NIDU_size;	/* no limit on TIDU size */
		p->SERV_type = serv;	/* COTS or CLTS */
		p->CURRENT_state = sccp_get_t_state(sc);
		p->PROVIDER_flag = XPG4_1 & ~T_SNDZERO;
		mp->b_wptr += sizeof(*p);
		freemsg(msg);
		mi_strlog(q, STRLOGRX, SL_TRACE, "<- T_INFO_ACK");
		putnext(sc->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}

/*
 *  T_BIND_ACK          17 - bind acknowledgement
 *  -----------------------------------------------------------------
 */
static int
t_bind_ack(struct sc *sc, queue_t *q, mblk_t *msg, struct sccp_addr *add)
{
	mblk_t *mp;
	struct T_bind_ack *p;
	size_t add_len = add ? sizeof(*add) : 0;
	size_t msg_len = sizeof(*p) + add_len;

	if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_PCPROTO;
		p = (typeof(p)) mp->b_wptr;
		p->PRIM_type = T_BIND_ACK;
		p->ADDR_length = add_len;
		p->ADDR_offset = sizeof(*p);
		p->CONIND_number = sc->conind;
		mp->b_wptr += sizeof(*p);
		if (add_len) {
			bcopy(add, mp->b_wptr, add_len);
			mp->b_wptr += add_len;
		}
		sccp_set_t_state(sc, TS_IDLE);
		freemsg(msg);
		mi_strlog(q, STRLOGRX, SL_TRACE, "<- T_BIND_ACK");
		putnext(sc->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}

/*
 *  T_ERROR_ACK         18 - error acknowledgement
 *  -----------------------------------------------------------------
 */
static int
t_error_ack(struct sc *sc, queue_t *q, mblk_t *msg, const np_ulong prim, np_long error)
{
	int err = error;
	mblk_t *mp;
	struct T_error_ack *p;
	size_t msg_len = sizeof(*p);

	switch (error) {
	case -EBUSY:
	case -EAGAIN:
	case -ENOMEM:
	case -ENOBUFS:
		seldom();
		return (err);
	case 0:
		never();
		return (err);
	}
	if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_PCPROTO;
		p = (typeof(p)) mp->b_wptr;
		p->PRIM_type = T_ERROR_ACK;
		p->ERROR_prim = prim;
		p->TLI_error = error < 0 ? TSYSERR : error;
		p->UNIX_error = error < 0 ? -error : 0;
		mp->b_wptr += sizeof(*p);
		switch (sccp_get_t_state(sc)) {
#ifdef TS_WACK_OPTREQ
		case TS_WACK_OPTREQ:
#endif
		case TS_WACK_UREQ:
		case TS_WACK_CREQ:
			sccp_set_t_state(sc, TS_IDLE);
			break;
		case TS_WACK_BREQ:
			sccp_set_t_state(sc, TS_UNBND);
			break;
		case TS_WACK_CRES:
			sccp_set_t_state(sc, TS_WRES_CIND);
			break;
		case TS_WACK_DREQ6:
			sccp_set_t_state(sc, TS_WCON_CREQ);
			break;
		case TS_WACK_DREQ7:
			sccp_set_t_state(sc, TS_WRES_CIND);
			break;
		case TS_WACK_DREQ9:
			sccp_set_t_state(sc, TS_DATA_XFER);
			break;
		case TS_WACK_DREQ10:
			sccp_set_t_state(sc, TS_WIND_ORDREL);
			break;
		case TS_WACK_DREQ11:
			sccp_set_t_state(sc, TS_WREQ_ORDREL);
			break;
			/* Note: if we are not in a WACK state we simply do not change state.  This 
			   occurs normally when we send TOUTSTATE or TNOTSUPPORT or are responding
			   to a T_OPTMGMT_REQ in other then TS_IDLE state. */
		}
		freemsg(msg);
		mi_strlog(q, STRLOGRX, SL_TRACE, "<- T_ERROR_ACK");
		putnext(sc->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}

/*
 *  T_OK_ACK            19 - success acknowledgement
 *  -----------------------------------------------------------------
 */
static int
t_ok_ack(struct sc *sc, queue_t *q, mblk_t *msg, np_ulong prim, np_ulong seq, np_ulong tok)
{
	mblk_t *mp;
	struct T_ok_ack *p;
	size_t msg_len = sizeof(*p);

	if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_PCPROTO;
		p = (typeof(p)) mp->b_wptr;
		p->PRIM_type = T_OK_ACK;
		p->CORRECT_prim = prim;
		mp->b_wptr += sizeof(*p);
		switch (sccp_get_t_state(sc)) {
		case TS_WACK_CREQ:
			sccp_set_t_state(sc, TS_WCON_CREQ);
			break;
		case TS_WACK_UREQ:
			sccp_set_t_state(sc, TS_UNBND);
			break;
		case TS_WACK_CRES:
		{
			queue_t *aq = (queue_t *) (ulong) tok;	/* FIXME: look up instead */
			struct sc *ap = SCCP_PRIV(aq);

			if (ap) {
				ap->i_state = TS_DATA_XFER;
				// sccp_cleanup_read(q);
				// sccp_transmit_wakeup(q);
			}
			if (seq) {
				bufq_unlink(&sc->conq, (mblk_t *) (ulong) seq);	/* FIXME: look up
										   instead */
				freemsg((mblk_t *) (ulong) seq);	/* FIXME: look up instead */
			}
			if (aq != sc->oq) {
				if (bufq_length(&sc->conq))
					sccp_set_t_state(sc, TS_WRES_CIND);
				else
					sccp_set_t_state(sc, TS_IDLE);
			}
			break;
		}
		case TS_WACK_DREQ7:
			if (seq)
				bufq_unlink(&sc->conq, (mblk_t *) (ulong) seq);	/* FIXME: look up
										   instead */
		case TS_WACK_DREQ6:
		case TS_WACK_DREQ9:
		case TS_WACK_DREQ10:
		case TS_WACK_DREQ11:
			if (bufq_length(&sc->conq))
				sccp_set_t_state(sc, TS_WRES_CIND);
			else
				sccp_set_t_state(sc, TS_IDLE);
			break;
			/* Note: if we are not in a WACK state we simply do not change state.  This 
			   occurs normally when we are responding to a T_OPTMGMT_REQ in other than
			   the TS_IDLE state. */
		}
		freemsg(msg);
		mi_strlog(q, STRLOGRX, SL_TRACE, "<- T_OK_ACK");
		putnext(sc->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}

/*
 *  T_OPTMGMT_ACK       22 - options management acknowledgement
 *  -----------------------------------------------------------------
 */
static int
t_optmgmt_ack(struct sc *sc, queue_t *q, mblk_t *msg, np_ulong flags, struct sccp_opts *ops)
{
	mblk_t *mp;
	struct T_optmgmt_ack *p;
	size_t opt_len = sccp_opts_size(sc, ops);
	size_t msg_len = sizeof(*p) + opt_len;

	if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_PCPROTO;
		p = (typeof(p)) mp->b_wptr;
		p->PRIM_type = T_OPTMGMT_ACK;
		p->OPT_length = opt_len;
		p->OPT_offset = opt_len ? sizeof(*p) : 0;
		p->MGMT_flags = flags;
		mp->b_wptr += sizeof(*p);
		sccp_build_opts(sc, ops, mp->b_wptr);
		mp->b_wptr += opt_len;
#ifdef TS_WACK_OPTREQ
		if (sccp_get_t_state(sc) == TS_WACK_OPTREQ)
			sccp_set_t_state(sc, TS_IDLE);
#endif
		freemsg(msg);
		mi_strlog(q, STRLOGRX, SL_TRACE, "<- T_OPTMGMT_ACK");
		putnext(sc->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}

#if 0
/*
 *  T_ORDREL_IND        23 - orderly release indication
 *  -----------------------------------------------------------------
 */
static int
t_ordrel_ind(struct sc *sc, queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct T_ordrel_ind *p;
	size_t msg_len = sizeof(*p);

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			mp->b_wptr += sizeof(*p);
			p->PRIM_type = T_ORDREL_IND;
			switch (sccp_get_t_state(sc)) {
			case TS_DATA_XFER:
				sccp_set_t_state(sc, TS_WREQ_ORDREL);
				break;
			case TS_WIND_ORDREL:
				sccp_set_t_state(sc, TS_IDLE);
				break;
			}
			freemsg(msg);
			mi_strlog(q, STRLOGRX, SL_TRACE, "<- T_ORDREL_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}
#endif

/*
 *  T_ADDR_ACK          27 - address acknowledgement
 *  -----------------------------------------------------------------
 */
static int
t_addr_ack(struct sc *sc, queue_t *q, mblk_t *msg, struct sccp_addr *loc, struct sccp_addr *rem)
{
	mblk_t *mp;
	struct T_addr_ack *p;
	size_t loc_len = loc ? sizeof(*loc) : 0;
	size_t rem_len = rem ? sizeof(*rem) : 0;
	size_t msg_len = sizeof(*p) + loc_len + rem_len;

	if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_PCPROTO;
		p = (struct T_addr_ack *) mp->b_wptr;
		p->PRIM_type = T_ADDR_ACK;
		p->LOCADDR_length = loc_len;
		p->LOCADDR_offset = sizeof(*p);
		p->REMADDR_length = rem_len;
		p->REMADDR_offset = sizeof(*p) + loc_len;
		mp->b_wptr += sizeof(*p);
		bcopy(loc, mp->b_wptr, loc_len);
		mp->b_wptr += loc_len;
		bcopy(rem, mp->b_wptr, rem_len);
		mp->b_wptr += rem_len;
		freemsg(msg);
		putnext(sc->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}

#if 0
/*
 *  T_CAPABILITY_ACK    ?? - protocol capability ack
 *  -----------------------------------------------------------------
 */
static int
t_capability_ack(struct sc *sc, queue_t *q, mblk_t *msg, np_ulong caps)
{
	mblk_t *mp;
	struct T_capability_ack *p;
	size_t msg_len = sizeof(*p);
	np_ulong caps = (acceptor ? TC1_ACCEPTOR_ID : 0) | (info ? TC1_INFO : 0);

	if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_PCPROTO;
		p = (struct T_capability_ack *) mp->b_wptr;
		mp->b_wptr += sizeof(*p);
		p->PRIM_type = T_CAPABILITY_ACK;
		p->CAP_bits1 = caps;
		p->ACCEPTOR_id = (caps & TC1_ACCEPTOR_ID) ? (np_ulong) sc->oq : 0;
		if (caps & TC1_INFO) {
			p->INFO_ack.PRIM_type = T_INFO_ACK;
			p->INFO_ack.TSDU_size = sc->tsdu;
			p->INFO_ack.ETSDU_size = sc->etsdu;
			p->INFO_ack.CDATA_size = sc->cdata;
			p->INFO_ack.DDATA_size = sc->ddata;
			p->INFO_ack.ADDR_size = sc->addlen;
			p->INFO_ack.OPT_size = sc->optlen;
			p->INFO_ack.TIDU_size = sc->tidu;
			p->INFO_ack.SERV_type = sc->stype;
			p->INFO_ack.CURRENT_state = sccp_get_t_state(sc);
			p->INFO_ack.PROVIDER_flag = sc->ptype;
		} else
			bzero(&p->INFO_ack, sizeof(p->INFO_ack));
		freemsg(msg);
		mi_strlog(q, STRLOGRX, SL_TRACE, "<- T_CAPABILITY_ACK");
		putnext(sc->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}
#endif

/*
 *  T_UNITDATA_IND      20 - Unitdata indication
 *  -----------------------------------------------------------------
 *  Note: If we cannot deliver the destination address in the option header or somewhere, it will not be possible to
 *  bind to multiple alias addresses, but will only permit us to bind to a single alias address.  This might or
 *  might not be a problem to the user.
 */
static int
t_unitdata_ind(struct sc *sc, queue_t *q, mblk_t *msg, struct sccp_addr *src, np_ulong *seq,
	       np_ulong *prior, np_ulong *pcl, np_ulong *imp, np_ulong *rerr, mblk_t *dp)
{
	mblk_t *mp;
	struct t_opthdr *oh;
	const size_t olen = sizeof(*oh) + sizeof(t_scalar_t);
	struct T_unitdata_ind *p;
	size_t src_len = src ? sizeof(*src) : 0;
	size_t opt_len =
	    (seq ? olen : 0) + (prior ? olen : 0) + (pcl ? olen : 0) + (imp ? olen : 0) +
	    (rerr ? olen : 0);
	size_t msg_len = sizeof(*p) + src_len + opt_len;

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = T_UNITDATA_IND;
			p->SRC_length = src_len;
			p->SRC_offset = sizeof(*p);
			p->OPT_length = opt_len;
			p->OPT_offset = sizeof(*p) + src_len;
			mp->b_wptr += sizeof(*p);
			bcopy(src, mp->b_wptr, src_len);
			mp->b_wptr += src_len;
			if (opt_len) {
				if (seq) {
					oh = (typeof(oh)) mp->b_wptr;
					mp->b_wptr += sizeof(*oh);
					oh->len = olen;
					oh->level = T_SS7_SCCP;
					oh->name = T_SCCP_SEQ_CTRL;
					oh->status = T_SUCCESS;
					*(t_uscalar_t *) mp->b_wptr = *seq;
					mp->b_wptr += sizeof(t_uscalar_t);
				}
				if (prior) {
					oh = (typeof(oh)) mp->b_wptr;
					mp->b_wptr += sizeof(*oh);
					oh->len = olen;
					oh->level = T_SS7_SCCP;
					oh->name = T_SCCP_PRIORITY;
					oh->status = T_SUCCESS;
					*(t_uscalar_t *) mp->b_wptr = *prior;
					mp->b_wptr += sizeof(t_uscalar_t);
				}
				if (pcl) {
					oh = (typeof(oh)) mp->b_wptr;
					mp->b_wptr += sizeof(*oh);
					oh->len = olen;
					oh->level = T_SS7_SCCP;
					oh->name = T_SCCP_PCLASS;
					oh->status = T_SUCCESS;
					*(t_uscalar_t *) mp->b_wptr = *pcl;
					mp->b_wptr += sizeof(t_uscalar_t);
				}
				if (imp) {
					oh = (typeof(oh)) mp->b_wptr;
					mp->b_wptr += sizeof(*oh);
					oh->len = olen;
					oh->level = T_SS7_SCCP;
					oh->name = T_SCCP_IMPORTANCE;
					oh->status = T_SUCCESS;
					*(t_uscalar_t *) mp->b_wptr = *imp;
					mp->b_wptr += sizeof(t_uscalar_t);
				}
				if (rerr) {
					oh = (typeof(oh)) mp->b_wptr;
					mp->b_wptr += sizeof(*oh);
					oh->len = olen;
					oh->level = T_SS7_SCCP;
					oh->name = T_SCCP_RET_ERROR;
					oh->status = T_SUCCESS;
					*(t_uscalar_t *) mp->b_wptr = *rerr;
					mp->b_wptr += sizeof(t_uscalar_t);
				}
			}
			mp->b_cont = dp;
			if (msg && msg->b_cont == dp)
				msg->b_cont = NULL;
			freemsg(msg);
			mi_strlog(q, STRLOGDA, SL_TRACE, "<- T_UNITDATA_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  T_UDERROR_IND       21 - Unitdata error indication
 *  -----------------------------------------------------------------
 *  This primitive indicates to the transport user that a datagram with the specified destination address and
 *  options produced an error.
 */
static int
t_uderror_ind(struct sc *sc, queue_t *q, mblk_t *msg, np_ulong etype, struct sccp_addr *dst,
	      np_ulong *seq, np_ulong *pri, np_ulong *pcl, np_ulong *imp, np_ulong *ret, mblk_t *dp)
{
	mblk_t *mp;
	struct t_opthdr *oh;
	const size_t olen = sizeof(*oh) + sizeof(t_scalar_t);
	struct T_uderror_ind *p;
	size_t dst_len = dst ? sizeof(*dst) : 0;
	size_t opt_len =
	    (seq ? olen : 0) + (pri ? olen : 0) + (pcl ? olen : 0) + (imp ? olen : 0) +
	    (ret ? olen : 0);
	size_t msg_len = sizeof(*p) + dst_len;

	if (likely(canputnext(sc->oq))) {
		if (likely((mp = mi_allocb(q, msg_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			mp->b_band = 2;	/* XXX move ahead of data indications */
			p = (typeof(p)) mp->b_wptr;
			p->PRIM_type = T_UDERROR_IND;
			p->DEST_length = dst_len;
			p->DEST_offset = sizeof(*p);
			p->OPT_length = opt_len;
			p->OPT_offset = sizeof(*p) + dst_len;
			p->ERROR_type = etype;
			mp->b_wptr += sizeof(*p);
			bcopy(dst, mp->b_wptr, dst_len);
			mp->b_wptr += dst_len;
			if (opt_len) {
				if (seq) {
					oh = (typeof(oh)) mp->b_wptr;
					mp->b_wptr += sizeof(*oh);
					oh->len = olen;
					oh->level = T_SS7_SCCP;
					oh->name = T_SCCP_SEQ_CTRL;
					oh->status = T_SUCCESS;
					*(t_uscalar_t *) mp->b_wptr = *seq;
					mp->b_wptr += sizeof(t_uscalar_t);
				}
				if (pri) {
					oh = (typeof(oh)) mp->b_wptr;
					mp->b_wptr += sizeof(*oh);
					oh->len = olen;
					oh->level = T_SS7_SCCP;
					oh->name = T_SCCP_PRIORITY;
					oh->status = T_SUCCESS;
					*(t_uscalar_t *) mp->b_wptr = *pri;
					mp->b_wptr += sizeof(t_uscalar_t);
				}
				if (pcl) {
					oh = (typeof(oh)) mp->b_wptr;
					mp->b_wptr += sizeof(*oh);
					oh->len = olen;
					oh->level = T_SS7_SCCP;
					oh->name = T_SCCP_PCLASS;
					oh->status = T_SUCCESS;
					*(t_uscalar_t *) mp->b_wptr = *pcl;
					mp->b_wptr += sizeof(t_uscalar_t);
				}
				if (imp) {
					oh = (typeof(oh)) mp->b_wptr;
					mp->b_wptr += sizeof(*oh);
					oh->len = olen;
					oh->level = T_SS7_SCCP;
					oh->name = T_SCCP_IMPORTANCE;
					oh->status = T_SUCCESS;
					*(t_uscalar_t *) mp->b_wptr = *imp;
					mp->b_wptr += sizeof(t_uscalar_t);
				}
				if (ret) {
					oh = (typeof(oh)) mp->b_wptr;
					mp->b_wptr += sizeof(*oh);
					oh->len = olen;
					oh->level = T_SS7_SCCP;
					oh->name = T_SCCP_RET_ERROR;
					oh->status = T_SUCCESS;
					*(t_uscalar_t *) mp->b_wptr = *ret;
					mp->b_wptr += sizeof(t_uscalar_t);
				}
			}
			mp->b_cont = dp;
			if (msg && msg->b_cont == dp)
				msg->b_cont = NULL;
			freemsg(msg);
			mi_strlog(q, STRLOGDA, SL_TRACE, "<- T_UDERROR_IND");
			putnext(sc->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  -------------------------------------------------------------------------
 *
 *  Primitives sent downstream
 *
 *  -------------------------------------------------------------------------
 */
#ifdef NEVER
/*
 *  MTP_BIND_REQ
 *  -----------------------------------
 */
static int
mtp_bind_req(struct mt *mt, queue_t *q, mblk_t *msg, np_ulong flags, struct mtp_addr *add)
{
	mblk_t *mp;
	struct MTP_bind_req *p;
	size_t add_len = add ? sizeof(*add) : 0;

	if (likely((mp = mi_allocb(q, sizeof(*p) + add_len, BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_PCPROTO;
		p = (typeof(p)) mp->b_wptr;
		p->mtp_primitive = MTP_BIND_REQ;
		p->mtp_addr_length = add_len;
		p->mtp_addr_offset = sizeof(*p);
		p->mtp_bind_flags = flags;
		mp->b_wptr += sizeof(*p);
		bcopy(add, mp->b_wptr, add_len);
		mp->b_wptr += add_len;
		freemsg(msg);
		mi_strlog(q, STRLOGTX, SL_TRACE, "MTP_BIND_REQ ->");
		putnext(mt->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}
#endif

#ifdef NEVER
/*
 *  MTP_UNBIND_REQ
 *  -----------------------------------
 */
static int
mtp_unbind_req(struct mt *mt, queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct MTP_unbind_req *p;

	if (likely((mp = mi_allocb(q, sizeof(*p), BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_PCPROTO;
		p = (typeof(p)) mp->b_wptr;
		p->mtp_primitive = MTP_UNBIND_REQ;
		mp->b_wptr += sizeof(*p);
		freemsg(msg);
		mi_strlog(q, STRLOGTX, SL_TRACE, "MTP_UNBIND_REQ ->");
		puntext(mt->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}
#endif

#ifdef NEVER
/*
 *  MTP_CONN_REQ
 *  -----------------------------------
 */
static int
mtp_conn_req(struct mt *mt, queue_t *q, mblk_t *msg, np_ulong flags, struct mtp_addr *add)
{
	mblk_t *mp;
	struct MTP_conn_req *p;
	size_t add_len = add ? sizeof(*add) : 0;

	if (likely((mp = mi_allocb(q, sizeof(*p), BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_PCPROTO;
		p = (typeof(p)) mp->b_wptr;
		p->mtp_primitive = MTP_CONN_REQ;
		p->mtp_addr_length = add_len;
		p->mtp_addr_offset = sizeof(*p);
		p->mtp_conn_flags = flags;
		mp->b_wptr += sizeof(*p);
		bcopy(add, mp->b_wptr, add_len);
		mp->b_wptr += add_len;
		freemsg(msg);
		mi_strlog(q, STRLOGTX, SL_TRACE, "MTP_CONN_REQ ->");
		putnext(mt->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}
#endif

#ifdef NEVER
/*
 *  MTP_DISCON_REQ
 *  -----------------------------------
 */
static int
mtp_discon_req(struct mt *mt, queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct MTP_discon_req *p;

	if (likely(canputnext(mt->oq))) {
		if (likely((mp = mi_allocb(q, sizeof(*p), BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->mtp_primitive = MTP_DISCON_REQ;
			mp->b_wptr += sizeof(*p);
			freemsg(msg);
			mi_strlog(q, STRLOGTX, SL_TRACE, "MTP_DISCON_REQ ->");
			putnext(mt->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}
#endif

#ifdef NEVER
/*
 *  MTP_ADDR_REQ
 *  -----------------------------------
 */
static int
mtp_addr_req(struct mt *mt, queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct MTP_addr_req *p;

	if (likely((mp = mi_allocb(q, sizeof(*p), BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_PCPROTO;
		p = (typeof(p)) mp->b_wptr;
		p->mtp_primitive = MTP_ADDR_REQ;
		mp->b_wptr += sizeof(*p);
		freemsg(msg);
		mi_strlog(q, STRLOGTX, SL_TRACE, "MTP_ADDR_REQ ->");
		putnext(mt->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}
#endif

#ifdef NEVER
/*
 *  MTP_INFO_REQ
 *  -----------------------------------
 */
static int
mtp_info_req(struct mt *mt, queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct MTP_info_req *p;

	if (likely((mp = mi_allocb(q, sizeof(*p), BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_PCPROTO;
		p = (typeof(p)) mp->b_wptr;
		p->mtp_primitive = MTP_INFO_REQ;
		mp->b_wptr += sizeof(*p);
		freemsg(msg);
		mi_strlog(q, STRLOGTX, SL_TRACE, "MTP_INFO_REQ ->");
		putnext(mt->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}
#endif

#ifdef NEVER
/*
 *  MTP_OPTMGMT_REQ
 *  -----------------------------------
 */
static int
mtp_optmgmt_req(struct mt *mt, queue_t *q, mblk_t *msg, np_ulong flags, uchar *opt_ptr,
		size_t opt_len)
{
	mblk_t *mp;
	struct MTP_optmgmt_req *p;

	if (likely((mp = mi_allocb(q, sizeof(*p) + opt_len, BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_PCPROTO;
		p = (typeof(p)) mp->b_wptr;
		p->mtp_primitive = MTP_OPTMGMT_REQ;
		p->mtp_opt_length = opt_len;
		p->mtp_opt_offset = sizeof(*p);
		p->mtp_mgmt_flags = flags;
		mp->b_wptr += sizeof(*p);
		bcopy(opt_ptr, mp->b_wptr, opt_len);
		mp->b_wptr += opt_len;
		freemsg(msg);
		mi_strlog(q, STRLOGTX, SL_TRACE, "MTP_OPTMGMT_REQ ->");
		putnext(mt->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}
#endif

/*
 *  MTP_TRANSFER_REQ
 *  -----------------------------------
 */
static inline int
mtp_transfer_req(struct mt *mt, queue_t *q, mblk_t *msg, struct mtp_addr *add, np_ulong prior,
		 np_ulong sls, mblk_t *dp)
{
	mblk_t *mp;
	struct MTP_transfer_req *p;
	size_t add_len = add ? sizeof(*add) : 0;

	if (likely(canputnext(mt->oq))) {
		if (likely((mp = mi_allocb(q, sizeof(*p) + add_len, BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_PROTO;
			p = (typeof(p)) mp->b_wptr;
			p->mtp_primitive = MTP_TRANSFER_REQ;
			p->mtp_dest_length = add_len;
			p->mtp_dest_offset = sizeof(*p);
			p->mtp_mp = prior;
			p->mtp_sls = sls;
			mp->b_wptr += sizeof(*p);
			bcopy(add, mp->b_wptr, add_len);
			mp->b_wptr += add_len;
			mp->b_cont = dp;
			freemsg(msg);
			mi_strlog(q, STRLOGTX, SL_TRACE, "MTP_TRANSFER_REQ ->");
			putnext(mt->oq, mp);
			return (0);
		}
		return (-ENOBUFS);
	}
	return (-EBUSY);
}

/*
 *  -------------------------------------------------------------------------
 *
 *  Encode/Decode message functions
 *
 *  -------------------------------------------------------------------------
 */

/*
 *  Parameter sizing, packing and unpacking functions:
 *  -------------------------------------------------------------------------
 */
static inline int
size_one(void)
{
	return (1);
}
static inline int
pack_one(np_ulong val, register uchar **p)
{
	**p = val;
	*p += 1;
	return (1);
}

/*
 *  SCCP_PT_EOP   0x00 - End of Optional Parameters
 *  -------------------------------------------------------------------------
 */
static inline int
size_eop(void)
{
	return (0);
}
static inline int
pack_eop(struct sccp_msg *m, register uchar **p)
{
	(void) p;
	return (0);
}
static inline int
unpack_eop(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (p + size_eop() <= e) {
		int len = size_eop();

		m->parms |= SCCP_PTF_EOP;
		return (len);
	}
	return (-EMSGSIZE);
}

/*
 *  SCCP_PT_DLR   0x01 - Destination Local Reference
 *  -------------------------------------------------------------------------
 */
static inline int
size_lrn(void)
{
	return (3);
}
static inline int
pack_lrn(register uchar **p, np_ulong lrn)
{
	*p[0] = (lrn >> 16) & 0xff;
	*p[1] = (lrn >> 8) & 0xff;
	*p[2] = (lrn >> 0) & 0xff;
	*p += 3;
	return (3);
}
static inline int
size_dlr(void)
{
	return size_lrn();
}
static inline int
pack_dlr(struct sccp_msg *m, register uchar **p)
{
	return pack_lrn(p, m->dlr);
}
static inline int
unpack_dlr(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & SCCP_PTF_DLR)) {
		int len = size_dlr();

		if (p + len <= e) {
			m->dlr = 0;
			m->dlr |= ((np_ulong) (p[0]) << 0);
			m->dlr |= ((np_ulong) (p[1]) << 8);
			m->dlr |= ((np_ulong) (p[2]) << 16);
			m->parms |= SCCP_PTF_DLR;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_SLR   0x02 - Source Local reference
 *  -------------------------------------------------------------------------
 */
static inline int
size_slr(void)
{
	return size_lrn();
}
static inline int
pack_slr(struct sccp_msg *m, register uchar **p)
{
	return pack_lrn(p, m->slr);
}
static inline int
unpack_slr(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & SCCP_PTF_SLR)) {
		int len = size_slr();

		if (p + len <= e) {
			m->slr = 0;
			m->slr |= (((np_ulong) p[0]) << 0);
			m->slr |= (((np_ulong) p[1]) << 8);
			m->slr |= (((np_ulong) p[2]) << 16);
			m->parms |= SCCP_PTF_SLR;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_CDPA  0x03 - Called Party Address
 *  -------------------------------------------------------------------------
 */
static int
size_cpa(np_ulong pvar, struct sccp_addr *cpa)
{
	size_t len = 0;

	len += 1;
	switch (pvar & SS7_PVAR_MASK) {
	case SS7_PVAR_JTTC:
		fixme(("Not sure is this is correct for JTTC\n"));
		goto skip_ni;
	case SS7_PVAR_ANSI:
		if (cpa->ni) {
		      skip_ni:
			if (cpa->pc != -1)
				len += 3;
			// if ( cpa->ssn ) /* always code SSN */
			len += 1;
			if (cpa->gtt) {
				switch (cpa->gtt) {
				case 1:
					len += 2;
					break;
				case 2:
					len += 1;
					break;
				}
				len += cpa->alen;
			}
			break;
		}
	default:
		if (cpa->pc != -1)
			len += 2;
		// if ( cpa->ssn ) /* always code SSN */
		len += 1;
		if (cpa->gtt) {
			switch (cpa->gtt) {
			case 1:
				len += 1;
				break;
			case 2:
				len += 1;
				break;
			case 3:
				len += 2;
				break;
			case 4:
				len += 3;
				break;
			}
			len += cpa->alen;
		}
		break;
	}
	return (len);
}
static int
pack_cpa(np_ulong pvar, register uchar **p, struct sccp_addr *a)
{
	np_ulong len = 0;
	np_ulong pci = (a->pc == -1) ? 0 : 1;
	np_ulong ssni = (a->ssn == 0) ? 1 : 1;	/* always include ssn */

	**p = 0;
	**p |= (pci & 0x01) << 0;
	**p |= (ssni & 0x01) << 1;
	**p |= (a->gtt & 0x0f) << 2;
	**p |= (a->ri & 0x01) << 6;
	**p |= (a->ni & 0x01) << 7;
	*p += 1;
	len++;
	switch (pvar & SS7_PVAR_MASK) {
	case SS7_PVAR_JTTC:
		fixme(("Not sure is this is correct for JTTC\n"));
		goto skip_ni;
	case SS7_PVAR_ANSI:
		if (a->ni) {
		      skip_ni:
			if (pci) {
				*p[0] = (a->pc >> 0) & 0xff;
				*p[1] = (a->pc >> 8) & 0xff;
				*p[2] = (a->pc >> 16) & 0xff;
				*p += 3;
				len += 3;
			}
			if (ssni) {
				*p[0] = a->ssn;
				*p += 1;
				len++;
			}
			if (a->gtt) {
				switch (a->gtt) {
				case 1:	/* 0001 */
					*p[0] = a->tt;
					*p[1] = 0;
					*p[1] |= (a->es & 0xf) << 0;
					*p[1] |= (a->nplan & 0xf) << 4;
					*p += 2;
					len += 2;
					break;
				case 2:	/* 0010 */
					*p[0] = a->tt;
					*p += 1;
					len++;
					break;
				}
				bcopy(a->addr, *p, a->alen);
				*p += a->alen;
				len += a->alen;
			}
		}
	default:
		if (pci) {
			*p[0] = (a->pc >> 0) & 0xff;
			*p[1] = (a->pc >> 8) & 0x3f;
			*p += 2;
			len += 2;
		}
		if (ssni) {
			*p[0] = a->ssn;
			*p += 1;
			len++;
		}
		if (a->gtt) {
			switch (a->gtt) {
			case 1:	/* 0001 */
				*p[0] = a->nai;
				*p += 1;
				len++;
				break;
			case 2:	/* 0010 */
				*p[0] = a->tt;
				*p += 1;
				len++;
				break;
			case 3:	/* 0011 */
				*p[0] = a->tt;
				*p[1] = 0;
				*p[1] |= (a->es & 0xf) << 0;
				*p[1] |= (a->nplan & 0xf) << 4;
				*p += 2;
				len += 2;
				break;
			case 4:	/* 0100 */
				*p[0] = a->tt;
				*p[1] = 0;
				*p[1] |= (a->es & 0xf) << 0;
				*p[1] |= (a->nplan & 0xf) << 4;
				*p[2] = a->nai;
				*p += 3;
				len += 3;
				break;
			}
			bcopy(a->addr, *p, a->alen);
			*p += a->alen;
			len += a->alen;
		}
	}
	return (len);
}
static int
unpack_cpa(np_ulong pvar, register uchar *p, uchar *e, struct sccp_addr *a)
{
	np_ulong oe = 0, pci, ssni, len = 0;

	if (p + 1 > e)
		return (-EMSGSIZE);
	pci = (p[0] >> 0) & 0x01;
	ssni = (p[0] >> 1) & 0x01;
	a->gtt = (p[0] >> 2) & 0x0f;
	a->ri = (p[0] >> 6) & 0x01;
	a->ni = (p[0] >> 7) & 0x01;
	p++;
	len++;
	switch (pvar & SS7_PVAR_MASK) {
	case SS7_PVAR_JTTC:
		fixme(("Not sure is this is correct for JTTC\n"));
		goto skip_ni;
	case SS7_PVAR_ANSI:
		if (a->ni) {
		      skip_ni:
			if (pci) {
				if (p + 3 > e)
					return (-EMSGSIZE);
				a->pc = 0;
				a->pc |= (p[0] << 0) & 0xff;
				a->pc |= (p[1] << 8) & 0xff;
				a->pc |= (p[2] << 16) & 0xff;
				p += 3;
				len += 3;
			} else
				a->pc = -1;
			if (ssni) {
				if (p + 1 > e)
					return (-EMSGSIZE);
				a->ssn = p[0];
				p++;
				len++;
			} else
				a->ssn = 0;
			if (a->gtt) {
				switch (a->gtt) {
				case 1:	/* 0001 */
					if (p + 2 > e)
						return (-EMSGSIZE);
					a->tt = p[0];
					a->es = (p[1] >> 0) & 0x0f;
					a->nplan = (p[1] >> 4) & 0x0f;
					p += 2;
					len += 2;
					break;
				case 2:	/* 0010 */
					if (p + 1 > e)
						return (-EMSGSIZE);
					a->tt = p[0];
					p++;
					len++;
					break;
				default:
					return (-EPROTO);
				}
				/* followed by address bytes */
				if ((a->alen = e - p) > SCCP_MAX_ADDR_LENGTH)
					return (-EMSGSIZE);
				bcopy(p, a->addr, a->alen);
				p += a->alen;
				len += a->alen;
			}
			break;
		}
	default:
		if (pci) {
			if (p + 2 > e)
				return (-EMSGSIZE);
			a->pc = 0;
			a->pc |= (p[0] << 0) & 0xff;
			a->pc |= (p[1] << 8) & 0x3f;
			p += 2;
			len += 2;
		} else
			a->pc = -1;
		if (ssni) {
			if (p + 1 > e)
				return (-EMSGSIZE);
			a->ssn = p[0];
			p++;
			len++;
		} else
			a->ssn = 0;
		if (a->gtt) {
			switch (a->gtt) {
			case 1:	/* 0001 */
				if (p + 1 > e)
					return (-EMSGSIZE);
				oe = (p[0] >> 7) & 0x01;
				a->nai = p[0] & 0x7f;
				p++;
				len++;
				break;
			case 2:	/* 0010 */
				if (p + 1 > e)
					return (-EMSGSIZE);
				a->tt = p[0];
				p++;
				len++;
				break;
			case 3:	/* 0011 */
				if (p + 2 > e)
					return (-EMSGSIZE);
				a->tt = p[0];
				a->es = (p[1] >> 0) & 0x0f;
				a->nplan = (p[1] >> 4) & 0x0f;
				p += 2;
				len += 2;
				break;
			case 4:	/* 0100 */
				if (p + 3 > e)
					return (-EMSGSIZE);
				a->tt = p[0];
				a->es = (p[1] >> 0) & 0x0f;
				a->nplan = (p[1] >> 4) & 0x0f;
				a->nai = p[2] & 0x7f;
				p += 3;
				len += 3;
				break;
			default:
				return (-EPROTO);
			}
			/* followed by address bytes */
			if ((a->alen = e - p) > SCCP_MAX_ADDR_LENGTH)
				return (-EMSGSIZE);
			bcopy(p, a->addr, a->alen);
			p += a->alen;
			len += a->alen;
		}
		break;
	}
	return (len);
}
static inline int
size_cdpa(struct sccp_msg *m)
{
	return size_cpa(m->pvar, &m->cdpa);
}
static inline int
pack_cdpa(struct sccp_msg *m, register uchar **p)
{
	return pack_cpa(m->pvar, p, &m->cdpa);
}
static inline int
unpack_cdpa(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & SCCP_PTF_CDPA)) {
		int rtn;

		if ((rtn = unpack_cpa(m->pvar, p, e, &m->cdpa)) < 0)
			return (rtn);
		m->parms |= SCCP_PTF_CDPA;
		return (rtn);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_CGPA  0x04 - Calling Party Address
 *  -------------------------------------------------------------------------
 */
static inline int
size_cgpa(struct sccp_msg *m)
{
	return size_cpa(m->pvar, &m->cgpa);
}
static inline int
pack_cgpa(struct sccp_msg *m, register uchar **p)
{
	return pack_cpa(m->pvar, p, &m->cgpa);
}
static inline int
unpack_cgpa(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & SCCP_PTF_CGPA)) {
		int rtn;

		if ((rtn = unpack_cpa(m->pvar, p, e, &m->cgpa)) < 0)
			return (rtn);
		m->parms |= SCCP_PTF_CGPA;
		return (rtn);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_PCLS  0x05 - Protocol Class
 *  -------------------------------------------------------------------------
 */
static inline int
size_pcls(void)
{
	return (1);
}
static inline int
pack_pcls(struct sccp_msg *m, register uchar **p)
{
	return pack_one(m->pcl | ((m->pcl < 2) ? m->ret : 0), p);
}
static inline int
unpack_pcls(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & SCCP_PTF_PCLS)) {
		int len = size_pcls();

		if (p + len <= e) {
			m->pcl = p[0] & 0x0f;
			m->ret = p[0] & 0x80;
			m->parms |= SCCP_PTF_PCLS;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_SEG   0x06 - Segmenting/Reassembly
 *  -------------------------------------------------------------------------
 */
static inline int
size_seg(void)
{
	return (1);
}
static inline int
pack_seg(struct sccp_msg *m, register uchar **p)
{
	return pack_one(m->more ? 1 : 0, p);
}
static inline int
unpack_seg(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & SCCP_PTF_SEG)) {
		int len = size_seg();

		if (p + len >= e) {
			m->seg = p[0] & 0x1;
			m->parms |= SCCP_PTF_SEG;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_RSN   0x07 - Receive Sequence Number
 *  -------------------------------------------------------------------------
 */
static inline int
size_rsn(void)
{
	return (1);
}
static inline int
pack_rsn(struct sccp_msg *m, register uchar **p)
{
	return pack_one((m->rsn << 1) & 0xfe, p);
}
static inline int
unpack_rsn(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & SCCP_PTF_RSN)) {
		int len = size_rsn();

		if (p + len <= e) {
			m->pr = (p[0] & 0xfe) >> 1;
			m->parms |= SCCP_PTF_RSN;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_SEQ   0x08 - Sequencing/Segmenting
 *  -------------------------------------------------------------------------
 */
static inline int
size_seq(void)
{
	return (2);
}
static inline int
pack_seq(struct sccp_msg *m, register uchar **p)
{
	*p[0] = ((m->ps << 1) & 0xfe);
	*p[1] = ((m->pr << 1) & 0xfe) | (m->more ? 1 : 0);
	*p += 2;
	return (2);
}
static inline int
unpack_seq(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & SCCP_PTF_SEQ)) {
		int len = size_seq();

		if (p + len <= e) {
			m->ps = (p[0] & 0xfe) >> 1;
			m->more = p[1] & 0x1;
			m->pr = (p[1] & 0xfe) >> 1;
			m->parms |= SCCP_PTF_SEQ;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_CRED  0x09 - Credit
 *  -------------------------------------------------------------------------
 */
static inline int
size_cred(void)
{
	return (1);
}
static inline int
pack_cred(struct sccp_msg *m, register uchar **p)
{
	return pack_one(m->cred, p);
}
static inline int
unpack_cred(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & SCCP_PTF_CRED)) {
		int len = size_cred();

		if (p + len <= e) {
			m->cred = p[0];
			m->parms |= SCCP_PTF_CRED;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_RELC  0x0a - Release Cause
 *  -------------------------------------------------------------------------
 */
static inline int
size_relc(void)
{
	return size_one();
}
static inline int
pack_relc(struct sccp_msg *m, register uchar **p)
{
	return pack_one(m->cause, p);
}
static inline int
unpack_relc(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & (SCCP_PTF_RELC | SCCP_PTF_RETC | SCCP_PTF_RESC |
			  SCCP_PTF_ERRC | SCCP_PTF_REFC))) {
		int len = size_relc();

		if (p + len <= e) {
			m->cause = p[0];
			m->parms |= SCCP_PTF_RELC;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_RETC  0x0b - Return Cause
 *  -------------------------------------------------------------------------
 */
static inline int
size_retc(void)
{
	return size_one();
}
static inline int
pack_retc(struct sccp_msg *m, register uchar **p)
{
	return pack_one(m->cause, p);
}
static inline int
unpack_retc(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!
	    (m->
	     parms & (SCCP_PTF_RELC | SCCP_PTF_RETC | SCCP_PTF_RESC | SCCP_PTF_ERRC |
		      SCCP_PTF_REFC))) {
		int len = size_retc();

		if (p + len <= e) {
			m->cause = p[0];
			m->parms |= SCCP_PTF_RETC;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_RESC  0x0c - Reset Cause
 *  -------------------------------------------------------------------------
 */
static inline int
size_resc(void)
{
	return size_one();
}
static inline int
pack_resc(struct sccp_msg *m, register uchar **p)
{
	return pack_one(m->cause, p);
}
static inline int
unpack_resc(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & (SCCP_PTF_RELC | SCCP_PTF_RETC | SCCP_PTF_RESC |
			  SCCP_PTF_ERRC | SCCP_PTF_REFC))) {
		int len = size_resc();

		if (p + len <= e) {
			m->cause = p[0];
			m->parms |= SCCP_PTF_RESC;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_ERRC  0x0d - Error Cause
 *  -------------------------------------------------------------------------
 */
static inline int
size_errc(void)
{
	return size_one();
}
static inline int
pack_errc(struct sccp_msg *m, register uchar **p)
{
	return pack_one(m->cause, p);
}
static inline int
unpack_errc(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & (SCCP_PTF_RELC | SCCP_PTF_RETC | SCCP_PTF_RESC |
			  SCCP_PTF_ERRC | SCCP_PTF_REFC))) {
		int len = size_errc();

		if (p + len >= e) {
			m->cause = p[0];
			m->parms |= SCCP_PTF_ERRC;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_REFC  0x0e - Refusal Cause
 *  -------------------------------------------------------------------------
 */
static inline int
size_refc(void)
{
	return size_one();
}
static inline int
pack_refc(struct sccp_msg *m, register uchar **p)
{
	return pack_one(m->cause, p);
}
static inline int
unpack_refc(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & (SCCP_PTF_RELC | SCCP_PTF_RETC | SCCP_PTF_RESC |
			  SCCP_PTF_ERRC | SCCP_PTF_REFC))) {
		int len = size_refc();

		if (p + len <= e) {
			m->cause = p[0];
			m->parms |= SCCP_PTF_REFC;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_DATA  0x0f - Data
 *  -------------------------------------------------------------------------
 */
static inline int
size_data(struct sccp_msg *m)
{
	return (m->data.len);
}
static inline void
link_data(mblk_t *mp, mblk_t *data)
{
	/* IMPLEMENTATION NOTE:- Because sizeable mandatory variable part data exists, MVP DATA and 
	   MVP LDATA are linked at the end of the SCCP message rather than copying it into the SCCP 
	   M_DATA message block. */
	linkb(mp, data);
}
static inline int
pack_data(struct sccp_msg *m, register uchar **p)
{
	/* IMPLEMENTATION NOTE:- For optional data (in connection and disconnect) the farthest
	   towards the end of the message that DATA can be placed is just before the EOP byte.
	   Because optional data is not normally large, we byte-copy the data. */
	int len = 0;
	mblk_t *dp = m->bp->b_cont;

	while ((dp = dp->b_cont)) {
		size_t slen = (dp->b_wptr > dp->b_rptr) ? dp->b_wptr - dp->b_rptr : 0;

		bcopy(dp->b_rptr, *p, slen);
		*p += slen;
		len += slen;
	}
	return (len);
}
static inline int
unpack_data(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & (SCCP_PTF_DATA | SCCP_PTF_LDATA))) {
		int len = e - p;

		if (len >= 3) {
			m->data.ptr = p;
			m->data.len = len;
			m->parms |= SCCP_PTF_DATA;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_SGMT  0x10 - Segmentation
 *  -------------------------------------------------------------------------
 */
static inline int
size_sgmt(void)
{
	return (4);
}
static inline int
pack_sgmt(struct sccp_msg *m, register uchar **p)
{
	*p[0] = (m->sgmt.first << 7) | ((m->sgmt.pcl & 0x1) << 6) | (m->sgmt.rem & 0xf);
	*p[1] = (m->sgmt.slr >> 16) & 0xff;
	*p[2] = (m->sgmt.slr >> 8) & 0xff;
	*p[3] = (m->sgmt.slr >> 0) & 0xff;
	*p += 4;
	return (4);
}
static inline int
unpack_sgmt(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & SCCP_PTF_SGMT)) {
		int len = size_sgmt();

		if (p + len <= e) {
			m->sgmt.first = (p[0] & 0x80) >> 7;
			m->sgmt.pcl = (p[0] & 0x40) >> 6;
			m->sgmt.rem = (p[0] & 0x0f) >> 0;
			m->sgmt.slr = 0;
			m->sgmt.slr |= p[1] << 0;
			m->sgmt.slr |= p[2] << 8;
			m->sgmt.slr |= p[3] << 16;
			m->parms |= SCCP_PTF_SGMT;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_HOPC  0x11 - Hop Counter
 *  -------------------------------------------------------------------------
 */
static inline int
size_hopc(void)
{
	return size_one();
}
static inline int
pack_hopc(struct sccp_msg *m, register uchar **p)
{
	return pack_one(m->hopc, p);
}
static inline int
unpack_hopc(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & SCCP_PTF_HOPC)) {
		int len = size_hopc();

		if (p + len <= e) {
			m->hopc = p[0];
			m->parms |= SCCP_PTF_HOPC;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_IMP   0x12 - Importance
 *  -------------------------------------------------------------------------
 */
static inline int
size_imp(void)
{
	return size_one();
}
static inline int
pack_imp(struct sccp_msg *m, register uchar **p)
{
	return pack_one((m->imp & 0x7), p);
}
static inline int
unpack_imp(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & SCCP_PTF_IMP)) {
		int len = size_imp();

		if (p + len <= e) {
			m->imp = (p[0] & 0x7);
			m->parms |= SCCP_PTF_IMP;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_LDATA 0x13 - Long Data
 *  -------------------------------------------------------------------------
 */
static inline int
size_ldata(struct sccp_msg *m)
{
	return size_data(m);
}
static inline void
link_ldata(mblk_t *mp, mblk_t *data)
{
	return link_data(mp, data);
}
static inline int
pack_ldata(struct sccp_msg *m, register uchar **p)
{
	return pack_data(m, p);
}
static inline int
unpack_ldata(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & (SCCP_PTF_DATA | SCCP_PTF_LDATA))) {
		int len = e - p;

		if (len >= 3) {
			m->data.ptr = p;
			m->data.len = len;
			m->parms |= SCCP_PTF_LDATA;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_ISNI  0xfa - Intermediate Signalling Network Identification
 *  -------------------------------------------------------------------------
 */
static int
size_isni(struct sccp_msg *m)
{
	size_t num, len = 0;

	len++;
	if (m->isni.ti)
		len++;
	if ((num = m->isni.nids) > 7)
		num = 7;
	len += num << 1;
	return (len);
}
static int
pack_isni(struct sccp_msg *m, register uchar **p)
{
	int i, len = 0;

	*p[0] = 0;
	*p[0] |= (m->isni.mi & 0x01) << 0;
	*p[0] |= (m->isni.iri & 0x03) << 1;
	*p[0] |= (m->isni.ti & 0x01) << 4;
	*p[0] |= (m->isni.count & 0x07) << 5;
	*p += 1;
	len++;
	if (m->isni.ti & 0x1) {
		*p[0] = m->isni.ns & 0x3;
		*p += 1;
		len++;
	}
	for (i = 0; i < 7 && i < m->isni.nids; i++) {
		*p[0] = m->isni.u[i].nid.network;
		*p[1] = m->isni.u[i].nid.cluster;
		*p += 2;
		len += 2;
	}
	return (len);
}
static int
unpack_isni(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & SCCP_PTF_ISNI)) {
		np_ulong i, len = 0;

		if (p + 2 > e)
			return (-EMSGSIZE);
		if (p + 1 > e)
			return (-EMSGSIZE);
		else
			len++;
		m->isni.mi = (p[0] >> 0) & 0x01;
		m->isni.iri = (p[0] >> 1) & 0x03;
		m->isni.ti = (p[0] >> 4) & 0x01;
		m->isni.count = (p[0] >> 5) & 0x07;
		p++;
		if (m->isni.ti) {
			if (p + 1 > e)
				return (-EMSGSIZE);
			else
				len++;
			m->isni.ns = p[0] & 0x3;
			p++;
		}
		if (p + 2 > e)
			return (-EMSGSIZE);
		m->isni.nids = (e - p) >> 1;
		if (m->isni.nids > 7)
			m->isni.nids = 7;
		for (i = 0; i < m->isni.nids; i++) {
			m->isni.u[i].nid.network = p[0];
			m->isni.u[i].nid.cluster = p[1];
			p += 2;
			len += 2;
		}
		m->parms |= SCCP_PTF_ISNI;
		return (len);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_INS   0xf9 - Intermediate Network Selection
 *  -------------------------------------------------------------------------
 */
static int
size_ins(struct sccp_msg *m)
{
	size_t num, len = 0;

	len++;
	if ((num = m->ins.nids) > 2)
		num = 2;
	len += num << 1;
	return (len);
}
static int
pack_ins(struct sccp_msg *m, register uchar **p)
{
	np_ulong i, len = 0;

	*p[0] = 0;
	*p[0] |= (m->ins.itype & 0x03) << 0;
	*p[0] |= (m->ins.rtype & 0x03) << 2;
	*p[0] |= (m->ins.count & 0x03) << 6;
	*p += 1;
	len++;
	for (i = 0; i < 2 && i < m->ins.nids; i++) {
		*p[0] = m->ins.u[i].nid.network;
		*p[1] = m->ins.u[i].nid.cluster;
		*p += 2;
		len += 2;
	}
	return (len);
}
static int
unpack_ins(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & SCCP_PTF_INS)) {
		np_ulong i, len = 0;

		if (p + 2 > e)
			return (-EMSGSIZE);
		if (p + 1 > e)
			return (-EMSGSIZE);
		else
			len++;
		m->ins.itype = (p[0] >> 0) & 0x03;
		m->ins.rtype = (p[0] >> 2) & 0x03;
		m->ins.count = (p[0] >> 6) & 0x03;
		p++;
		if (p + 1 > e)
			return (-EMSGSIZE);
		else
			len++;
		m->ins.nids = (e - p) >> 1;
		if (m->ins.nids > 2)
			m->ins.nids = 2;
		for (i = 0; i < m->ins.nids; i++) {
			m->ins.u[i].nid.network = p[0];
			m->ins.u[i].nid.cluster = p[1];
			p += 2;
			len += 2;
		}
		m->parms |= SCCP_PTF_INS;
		return (len);
	}
	return (-EPROTO);
}

/*
 *  SCCP_PT_MTI   0xf8 - Message Type Interworking
 *  -------------------------------------------------------------------------
 */
static inline int
size_mti(void)
{
	return (1);
}
static inline int
pack_mti(struct sccp_msg *m, register uchar **p)
{
	return pack_one(m->mti, p);
}
static inline int
unpack_mti(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (!(m->parms & SCCP_PTF_MTI)) {
		int len = size_mti();

		if (p + len <= e) {
			m->mti = p[0];
			m->parms |= SCCP_PTF_MTI;
			return (len);
		}
		return (-EMSGSIZE);
	}
	return (-EPROTO);
}

/*
 *  SCMG FI
 *  -------------------------------------------------------------------------
 */
static inline int
size_scmgfi(void)
{
	return size_one();
}
static inline int
pack_scmgfi(struct sccp_msg *m, register uchar **p)
{
	return pack_one(m->fi, p);
}
static inline int
unpack_scmgfi(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (p + 1 <= e) {
		m->fi = p[0];
		return (1);
	}
	return (-EMSGSIZE);
}

/*
 *  SCMG ASSN
 *  -------------------------------------------------------------------------
 */
static inline int
size_assn(void)
{
	return size_one();
}
static inline int
pack_assn(struct sccp_msg *m, register uchar **p)
{
	return pack_one(m->assn, p);
}
static inline int
unpack_assn(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (p + 1 <= e) {
		m->assn = p[0];
		return (1);
	}
	return (-EMSGSIZE);
}

/*
 *  SCMG APC
 *  -------------------------------------------------------------------------
 */
static inline int
size_apc(void)
{
	return (3);
}
static inline int
pack_apc(struct sccp_msg *m, register uchar **p)
{
	*p[0] = (m->apc >> 0);
	*p[1] = (m->apc >> 8);
	*p[2] = (m->apc >> 16);
	*p += 3;
	return (3);
}
static inline int
unpack_apc(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (p + 1 <= e) {
		m->apc = 0;
		m->apc |= (((np_ulong) p[0]) << 0);
		m->apc |= (((np_ulong) p[1]) << 8);
		m->apc |= (((np_ulong) p[2]) << 16);
		p += 3;
		return (3);
	}
	return (-EMSGSIZE);
}

/*
 *  SCMG SMI
 *  -------------------------------------------------------------------------
 */
static inline int
size_smi(void)
{
	return size_one();
}
static inline int
pack_smi(struct sccp_msg *m, register uchar **p)
{
	return pack_one(m->smi, p);
}
static inline int
unpack_smi(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (p + 1 <= e) {
		m->smi = p[0];
		return (1);
	}
	return (-EMSGSIZE);
}

/*
 *  SCMG CONG
 *  -------------------------------------------------------------------------
 */
static inline int
size_cong(void)
{
	return size_one();
}
static inline int
pack_cong(struct sccp_msg *m, register uchar **p)
{
	return pack_one(m->cong, p);
}
static inline int
unpack_cong(register uchar *p, uchar *e, struct sccp_msg *m)
{
	if (p + 1 <= e) {
		m->cong = p[0];
		return (1);
	}
	return (-EMSGSIZE);
}

/*
 *  Optional Parameters
 *  -------------------------------------------------------------------------
 */
static inline int
size_opt(struct sccp_msg *m, size_t len)
{
	return (len);
}
static inline int
pack_opt(register uchar **p, uchar *ptr, size_t len)
{
	bcopy(ptr, *p, len);
	*p += len;
	return (len);
}
static inline int
unpack_opt(register uchar *p, uchar *e, struct sccp_var *v, size_t len)
{
	if (p + len - 1 < e) {
		v->len = len - 1;
		v->ptr = p;
		return (len);
	}
	trace();
	return (-EMSGSIZE);
}

#if 0
static inline int
sccp_check_opt(register uchar *p, size_t len)
{
	uchar ptype = 0;
	uchar *pp = p, *ep, *e = p + len;

	if (!len)
		return (0);
	for (pp = p, ep = pp + pp[1] + 2; pp < e && (ptype = *pp); pp = ep, ep = pp + pp[1] + 2) ;
	return ((((pp != (e - 1) || ptype)) && pp != e) ? -EINVAL : 0);
}
#endif
static int
sccp_dec_opt(uchar *p, uchar *e, struct sccp_msg *m)
{
	int err = 0;
	int8_t ptype;
	register uchar *pp = p, *ep = e;

	if (e - p < 3) {
		trace();
		return (-EMSGSIZE);
	}
	for (pp = p, ep = pp + pp[1] + 2; pp < e && (ptype = *pp); pp = ep, ep = pp + pp[1] + 2) {
		switch (ptype) {
		case SCCP_PT_MTI:
			err = unpack_mti(pp + 2, ep, m);
			break;
		case SCCP_PT_INS:
			err = unpack_ins(pp + 2, ep, m);
			break;
		case SCCP_PT_ISNI:
			err = unpack_isni(pp + 2, ep, m);
			break;
		case SCCP_PT_EOP:
			err = unpack_eop(pp + 2, ep, m);
			break;
		case SCCP_PT_DLR:
			err = unpack_dlr(pp + 2, ep, m);
			break;
		case SCCP_PT_SLR:
			err = unpack_slr(pp + 2, ep, m);
			break;
		case SCCP_PT_CDPA:
			err = unpack_cdpa(pp + 2, ep, m);
			break;
		case SCCP_PT_CGPA:
			err = unpack_cgpa(pp + 2, ep, m);
			break;
		case SCCP_PT_PCLS:
			err = unpack_pcls(pp + 2, ep, m);
			break;
		case SCCP_PT_SEG:
			err = unpack_seg(pp + 2, ep, m);
			break;
		case SCCP_PT_RSN:
			err = unpack_rsn(pp + 2, ep, m);
			break;
		case SCCP_PT_SEQ:
			err = unpack_seq(pp + 2, ep, m);
			break;
		case SCCP_PT_CRED:
			err = unpack_cred(pp + 2, ep, m);
			break;
		case SCCP_PT_RELC:
			err = unpack_relc(pp + 2, ep, m);
			break;
		case SCCP_PT_RETC:
			err = unpack_retc(pp + 2, ep, m);
			break;
		case SCCP_PT_RESC:
			err = unpack_resc(pp + 2, ep, m);
			break;
		case SCCP_PT_ERRC:
			err = unpack_errc(pp + 2, ep, m);
			break;
		case SCCP_PT_REFC:
			err = unpack_refc(pp + 2, ep, m);
			break;
		case SCCP_PT_DATA:
			err = unpack_data(pp + 2, ep, m);
			break;
		case SCCP_PT_SGMT:
			err = unpack_sgmt(pp + 2, ep, m);
			break;
		case SCCP_PT_HOPC:
			err = unpack_hopc(pp + 2, ep, m);
			break;
		case SCCP_PT_IMP:
			err = unpack_imp(pp + 2, ep, m);
			break;
#if 0
		case SCCP_PT_LDATA:	/* ldata cannot be optional */
			err = unpack_ldata(pp + 2, ep, m);
			break;
#endif
		default:
			err = 0;	/* ignore unregocnized optional parameters */
		}
		if (err < 0)
			return (err);
	}
	return (0);
}

/*
 *  -------------------------------------------------------------------------
 *
 *  Message Packing Functions
 *
 *  -------------------------------------------------------------------------
 *  These functions are used to take an unpacked message from the queue, pack
 *  it into a message for transmission and then place it back on the queue.
 */

/*
 *  SCCP_MT_CR    0x01 - Connection Request
 *  -------------------------------------------------------------------------
 *  ITUT: F(SLR, PCLS), V(CDPA), O(CRED, CGPA, DATA, HOPC, IMP, EOP)
 *  ASNI: F(SLR, PCLS), V(CDPA), O(CRED, CGPA, DATA, HOPC, EOP)
 */
static mblk_t *
sccp_pack_cr(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_slr() +		/* SLR */
	    size_pcls() +		/* PCLS */
	    2 + size_cdpa(m) +		/* CDPA */
	    1 +				/* oparms */
	    ((m->pcl == 3 && (m->parms & SCCP_PTF_CRED)) ? 2 + size_cred() : 0) +	/* CRED */
	    ((m->parms & SCCP_PTF_CGPA) ? 2 + size_cgpa(m) : 0) +	/* CGPA */
	    ((m->parms & SCCP_PTF_DATA) ? 2 + size_data(m) : 0) +	/* DATA */
	    ((m->parms & SCCP_PTF_HOPC) ? 2 + size_hopc() : 0) +	/* HOPC */
	    ((m->parms & SCCP_PTF_IMP) ? 2 + size_imp() : 0) +	/* IMP */
	    1;

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		register uchar *p;

		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_CR;
		pack_slr(m, &mp->b_wptr);	/* SLR F */
		pack_pcls(m, &mp->b_wptr);	/* PCLS F */
		p = mp->b_wptr;
		mp->b_wptr += 2;
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = size_cdpa(m);	/* CDPA V */
		pack_cdpa(m, &mp->b_wptr);
		*p = mp->b_wptr - p;
		p++;
		if (m->pcl == 3 && (m->parms & SCCP_PTF_CRED)) {
			*(mp->b_wptr)++ = SCCP_PT_CRED;	/* CRED O */
			*(mp->b_wptr)++ = size_cred();
			pack_cred(m, &mp->b_wptr);
		}
		if ((m->parms & SCCP_PTF_CGPA)) {
			*(mp->b_wptr)++ = SCCP_PT_CGPA;	/* CGPA O */
			*(mp->b_wptr)++ = size_cgpa(m);
			pack_cgpa(m, &mp->b_wptr);
		}
		if ((m->parms & SCCP_PTF_DATA)) {
			*(mp->b_wptr)++ = SCCP_PT_DATA;	/* DATA O */
			*(mp->b_wptr)++ = size_data(m);
			pack_data(m, &mp->b_wptr);
		}
		if ((m->parms & SCCP_PTF_HOPC)) {
			*(mp->b_wptr)++ = SCCP_PT_HOPC;	/* HOPC O */
			*(mp->b_wptr)++ = size_hopc();
			pack_hopc(m, &mp->b_wptr);
		}
		if ((m->parms & SCCP_PTF_IMP)) {
			*(mp->b_wptr)++ = SCCP_PT_IMP;	/* IMP O */
			*(mp->b_wptr)++ = size_imp();
			pack_imp(m, &mp->b_wptr);
		}
		*(mp->b_wptr)++ = SCCP_PT_EOP;	/* EOP O */
	}
	return (mp);
}

/*
 *  SCCP_MT_CC    0x02 - Connection Confirm
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR, PCLS), O(CRED, CDPA, DATA, IMP, EOP)
 *  ANSI: F(DLR, SLR, PCLS), O(CRED, CDPA, DATA, EOP)
 */
static mblk_t *
sccp_pack_cc(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_dlr() +		/* DLR */
	    size_slr() +		/* SLR */
	    size_pcls() +		/* PCLS */
	    1 +				/* oparms */
	    ((m->pcl == 3 && (m->parms & SCCP_PTF_CRED)) ? 2 + size_cred() : 0) +	/* PCLS */
	    ((m->parms & SCCP_PTF_CDPA) ? 2 + size_cdpa(m) : 0) +	/* CDPA */
	    ((m->parms & SCCP_PTF_DATA) ? 2 + size_data(m) : 0) +	/* DATA */
	    ((m->parms & SCCP_PTF_IMP) ? 2 + size_imp() : 0) +	/* IMP */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		register uchar *p;

		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_CC;
		pack_dlr(m, &mp->b_wptr);	/* DLR F */
		pack_slr(m, &mp->b_wptr);	/* SLR F */
		pack_pcls(m, &mp->b_wptr);	/* PCLS F */
		p = mp->b_wptr;
		mp->b_wptr += 1;
		*p = mp->b_wptr - p;
		p++;
		if (m->pcl == 3 && (m->parms & SCCP_PTF_CRED)) {
			*(mp->b_wptr)++ = SCCP_PT_CRED;	/* CRED O */
			*(mp->b_wptr)++ = size_cred();
			pack_cred(m, &mp->b_wptr);
		}
		if ((m->parms & SCCP_PTF_CDPA)) {
			*(mp->b_wptr)++ = SCCP_PT_CDPA;	/* CDPA O */
			*(mp->b_wptr)++ = size_cdpa(m);
			pack_cdpa(m, &mp->b_wptr);
		}
		if ((m->parms & SCCP_PTF_DATA)) {
			*(mp->b_wptr)++ = SCCP_PT_DATA;	/* DATA O */
			*(mp->b_wptr)++ = size_data(m);
			pack_data(m, &mp->b_wptr);
		}
		if ((m->parms & SCCP_PTF_IMP)) {
			*(mp->b_wptr)++ = SCCP_PT_IMP;	/* IMP O */
			*(mp->b_wptr)++ = size_imp();
			pack_imp(m, &mp->b_wptr);
		}
		*(mp->b_wptr)++ = SCCP_PT_EOP;	/* EOP O */
		m->rl.mp = SCCP_MP_CC;
	}
	return (mp);
}

/*
 *  SCCP_MT_CREF  0x03 - Connection Refused
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, REFC), O(CDPA, DATA, IMP, EOP)
 *  ANSI: F(DLR, REFC), O(CDPA, DATA, EOP)
 */
static mblk_t *
sccp_pack_cref(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_dlr() +		/* DLR */
	    size_refc() +		/* REFC */
	    1 +				/* oparms */
	    ((m->parms & SCCP_PTF_CDPA) ? 2 + size_cdpa(m) : 0) +	/* CDPA */
	    ((m->parms & SCCP_PTF_DATA) ? 2 + size_data(m) : 0) +	/* DATA */
	    ((m->parms & SCCP_PTF_IMP) ? 2 + size_imp() : 0) +	/* IMP */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		register uchar *p;

		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_CREF;
		pack_dlr(m, &mp->b_wptr);	/* DLR F */
		pack_refc(m, &mp->b_wptr);	/* REFC F */
		p = mp->b_wptr;
		mp->b_wptr += 1;
		*p = mp->b_wptr - p;
		p++;
		if ((m->parms & SCCP_PTF_CDPA)) {
			*(mp->b_wptr)++ = SCCP_PT_CDPA;	/* CDPA O */
			*(mp->b_wptr)++ = size_cdpa(m);
			pack_cdpa(m, &mp->b_wptr);
		}
		if ((m->parms & SCCP_PTF_DATA)) {
			*(mp->b_wptr)++ = SCCP_PT_DATA;	/* DATA O */
			*(mp->b_wptr)++ = size_data(m);
			pack_data(m, &mp->b_wptr);
		}
		if ((m->parms & SCCP_PTF_IMP)) {
			*(mp->b_wptr)++ = SCCP_PT_IMP;	/* IMP O */
			*(mp->b_wptr)++ = size_imp();
			pack_imp(m, &mp->b_wptr);
		}
		*(mp->b_wptr)++ = SCCP_PT_EOP;	/* EOP O */
		m->rl.mp = SCCP_MP_CREF;
	}
	return (mp);
}

/*
 *  SCCP_MT_RLSD  0x04 - Released
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR, RELC), O(DATA, IMP, EOP)
 *  ANSI: F(DLR, SLR, RELC), O(DATA, EOP)
 */
static mblk_t *
sccp_pack_rlsd(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_dlr() +		/* DLR */
	    size_slr() +		/* SLR */
	    size_relc() +		/* RELC */
	    1 +				/* oparms */
	    ((m->parms & SCCP_PTF_DATA) ? 2 + size_data(m) : 0) +	/* DATA */
	    ((m->parms & SCCP_PTF_IMP) ? 2 + size_imp() : 0) +	/* IMP */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		register uchar *p;

		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_RLSD;
		pack_dlr(m, &mp->b_wptr);	/* DLR F */
		pack_slr(m, &mp->b_wptr);	/* SLR F */
		pack_relc(m, &mp->b_wptr);	/* RELC F */
		p = mp->b_wptr;
		mp->b_wptr += 1;
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = SCCP_PT_DATA;	/* DATA O */
		*(mp->b_wptr)++ = size_data(m);
		pack_data(m, &mp->b_wptr);
		*(mp->b_wptr)++ = SCCP_PT_IMP;	/* IMP O */
		*(mp->b_wptr)++ = size_imp();
		pack_imp(m, &mp->b_wptr);
		*(mp->b_wptr)++ = SCCP_PT_EOP;	/* EOP O */
		m->rl.mp = SCCP_MP_RLSD;
	}
	return (mp);
}

/*
 *  SCCP_MT_RLC   0x05 - Release Complete
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR)
 *  ANSI: F(DLR, SLR)
 */
static mblk_t *
sccp_pack_rlc(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_dlr() +		/* DLR */
	    size_slr();			/* SLR */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_RLC;
		pack_dlr(m, &mp->b_wptr);	/* DLR F */
		pack_slr(m, &mp->b_wptr);	/* SLR F */
		m->rl.mp = SCCP_MP_RLC;
	}
	return (mp);
}

/*
 *  SCCP_MT_DT1   0x06 - Data Form 1
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SEG), V(DATA)
 *  ANSI: F(DLR, SEG), V(DATA)
 */
static mblk_t *
sccp_pack_dt1(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_dlr() +		/* DLR */
	    size_seg() +		/* SEG */
	    2 + size_data(m) +		/* DATA */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		register uchar *p;

		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_DT1;
		pack_dlr(m, &mp->b_wptr);	/* DLR F */
		pack_seg(m, &mp->b_wptr);	/* SEG F */
		p = mp->b_wptr;
		mp->b_wptr += 1;
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = size_data(m);	/* DATA V */
		link_data(mp, msg->b_cont);
	}
	return (mp);
}

/*
 *  SCCP_MT_DT2   0x07 - Data Form 2
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SEQ), V(DATA)
 *  ANSI: F(DLR, SEQ), V(DATA)
 */
static mblk_t *
sccp_pack_dt2(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_dlr() +		/* DLR */
	    size_seq() +		/* SEQ */
	    2 + size_data(m) +		/* DATA */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		register uchar *p;

		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_DT2;
		pack_dlr(m, &mp->b_wptr);	/* DLR F */
		pack_seq(m, &mp->b_wptr);	/* SEQ F */
		p = mp->b_wptr;
		mp->b_wptr += 1;
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = size_data(m);	/* DATA V */
		link_data(mp, msg->b_cont);
	}
	return (mp);
}

/*
 *  SCCP_MT_AK    0x08 - Data Acknowledgement
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, RSN, CRED)
 *  ANSI: F(DLR, RSN, CRED)
 */
static mblk_t *
sccp_pack_ak(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_dlr() +		/* DLR */
	    size_rsn() +		/* RSN */
	    size_cred() +		/* CRED */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_AK;
		pack_dlr(m, &mp->b_wptr);	/* DLR F */
		pack_rsn(m, &mp->b_wptr);	/* RSN F */
		pack_cred(m, &mp->b_wptr);	/* CRED F */
	}
	return (mp);
}

/*
 *  SCCP_MT_UDT   0x09 - Unitdata
 *  -------------------------------------------------------------------------
 *  ITUT: F(PCLS), V(CDPA, CGPA, DATA[2-254])
 *  ANSI: F(PCLS), V(CDPA, CGPA, DATA[2-252])
 */
static mblk_t *
sccp_pack_udt(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_pcls() +		/* PCLS */
	    2 + size_cdpa(m) +		/* CDPA */
	    2 + size_cgpa(m) +		/* CGPA */
	    2 + size_data(m) +		/* DATA */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		register uchar *p;

		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_UDT;
		pack_pcls(m, &mp->b_wptr);	/* PCLS F */
		p = mp->b_wptr;
		mp->b_wptr += 3;
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = size_cdpa(m);	/* CDPA V */
		pack_cdpa(m, &mp->b_wptr);
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = size_cgpa(m);	/* CGPA V */
		pack_cgpa(m, &mp->b_wptr);
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = size_data(m);	/* DATA V */
		link_data(mp, msg->b_cont);
	}
	return (mp);
}

/*
 *  SCCP_MT_UDTS  0x0a - Unitdata Service
 *  -------------------------------------------------------------------------
 *  ITUT: F(RETC), V(CDPA, CGPA, DATA[2-253])
 *  ANSI: F(RETC), V(CDPA, CGPA, DATA[2-251])
 */
static mblk_t *
sccp_pack_udts(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_retc() +		/* RETC */
	    2 + size_cdpa(m) +		/* CDPA */
	    2 + size_cgpa(m) +		/* CGPA */
	    2 + size_data(m) +		/* DATA */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		register uchar *p;

		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_UDTS;
		pack_retc(m, &mp->b_wptr);	/* RETC F */
		p = mp->b_wptr;
		mp->b_wptr += 3;
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = size_cdpa(m);	/* CDPA V */
		pack_cdpa(m, &mp->b_wptr);
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = size_cgpa(m);	/* CGPA V */
		pack_cgpa(m, &mp->b_wptr);
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = size_data(m);	/* DATA V */
		link_data(mp, msg->b_cont);
	}
	return (mp);
}

/*
 *  SCCP_MT_ED    0x0b - Expedited Data
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR), V(DATA)
 *  ANSI: F(DLR), V(DATA)
 */
static mblk_t *
sccp_pack_ed(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_dlr() +		/* DLR */
	    2 + size_data(m) +		/* DATA */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		register uchar *p;

		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_ED;
		pack_dlr(m, &mp->b_wptr);	/* DLR F */
		p = mp->b_wptr;
		mp->b_wptr += 1;
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = size_data(m);	/* DATA V */
		link_data(mp, msg->b_cont);
		m->rl.mp = SCCP_MP_ED;
	}
	return (mp);
}

/*
 *  SCCP_MT_EA    0x0c - Expedited Data Acknowledgement
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR)
 *  ANSI: F(DLR)
 */
static mblk_t *
sccp_pack_ea(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_dlr() +		/* DLR */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_EA;
		pack_dlr(m, &mp->b_wptr);	/* DLR F */
		m->rl.mp = SCCP_MP_EA;
	}
	return (mp);
}

/*
 *  SCCP_MT_RSR   0x0d - Reset Request
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR, RESC)
 *  ANSI: F(DLR, SLR, RESC)
 */
static mblk_t *
sccp_pack_rsr(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_dlr() +		/* DLR */
	    size_slr() +		/* SLR */
	    size_resc() +		/* RESC */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_RSR;
		pack_dlr(m, &mp->b_wptr);	/* DLR F */
		pack_slr(m, &mp->b_wptr);	/* SLR F */
		pack_resc(m, &mp->b_wptr);	/* RESC F */
		m->rl.mp = SCCP_MP_RSR;
	}
	return (mp);
}

/*
 *  SCCP_MT_RSC   0x0e - Reset Confirm
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR)
 *  ANSI: F(DLR, SLR)
 */
static mblk_t *
sccp_pack_rsc(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_dlr() +		/* DLR */
	    size_slr() +		/* SLR */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_RSC;
		pack_dlr(m, &mp->b_wptr);	/* DLR F */
		pack_slr(m, &mp->b_wptr);	/* SLR F */
		m->rl.mp = SCCP_MP_RSC;
	}
	return (mp);
}

/*
 *  SCCP_MT_ERR   0x0f - Protocol Data Unit Error
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, ERRC)
 *  ANSI: F(DLR, ERRC)
 */
static mblk_t *
sccp_pack_err(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_dlr() +		/* DLR */
	    size_errc() +		/* ERRC */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_ERR;
		pack_dlr(m, &mp->b_wptr);	/* DLR F */
		pack_errc(m, &mp->b_wptr);	/* ERRC F */
		m->rl.mp = SCCP_MP_ERR;
	}
	return (mp);
}

/*
 *  SCCP_MT_IT    0x10 - Inactivity Test
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR, PCLS, SEQ, CRED)
 *  ANSI: F(DLR, SLR, PCLS, SEQ, CRED)
 */
static mblk_t *
sccp_pack_it(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_dlr() +		/* DLR */
	    size_slr() +		/* SLR */
	    size_pcls() +		/* PCLS */
	    size_seq() +		/* SEQ */
	    size_cred() +		/* CRED */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_IT;
		pack_dlr(m, &mp->b_wptr);	/* DLR F */
		pack_slr(m, &mp->b_wptr);	/* SLR F */
		pack_pcls(m, &mp->b_wptr);	/* PCLS F */
		pack_seq(m, &mp->b_wptr);	/* SEQ F */
		pack_cred(m, &mp->b_wptr);	/* CRED F */
		m->rl.mp = SCCP_MP_IT;
	}
	return (mp);
}

/*
 *  SCCP_MT_XUDT  0x11 - Extended Unitdata
 *  -------------------------------------------------------------------------
 *  ITUT: F(PCLS, HOPC), V(CDPA, CGPA, DATA[2-253]), O(SGMT, IMP, EOP)
 *  ANSI: F(PCLS, HOPC), V(CDPA, CGPA, DATA[2-251]), O(SGMT, ISNI, INS, MTI, EOP)
 */
static mblk_t *
sccp_pack_xudt(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_pcls() +		/* PCLS */
	    size_hopc() +		/* HOPC */
	    2 + size_cdpa(m) +		/* CDPA */
	    2 + size_cgpa(m) +		/* CGPA */
	    2 + size_data(m) +		/* DATA */
	    1 +				/* optr */
	    ((m->parms & SCCP_PTF_SGMT) ? 2 + size_sgmt() : 0) +	/* SGMT */
	    ((m->parms & SCCP_PTF_IMP) ? 2 + size_imp() : 0) +	/* IMP */
	    ((m->parms & SCCP_PTF_ISNI) ? 2 + size_isni(m) : 0) +	/* ISNI */
	    ((m->parms & SCCP_PTF_INS) ? 2 + size_ins(m) : 0) +	/* INS */
	    ((m->parms & SCCP_PTF_MTI) ? 2 + size_mti() : 0) +	/* MTI */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		register uchar *p, *pd;

		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_XUDT;
		pack_pcls(m, &mp->b_wptr);	/* PCLS F */
		pack_hopc(m, &mp->b_wptr);	/* HOPC F */
		p = mp->b_wptr;
		mp->b_wptr += 4;
		*p = mp->b_wptr - p;	/* CDPA V */
		p++;
		*(mp->b_wptr)++ = size_cdpa(m);
		pack_cdpa(m, &mp->b_wptr);
		*p = mp->b_wptr - p;	/* CGPA V */
		p++;
		*(mp->b_wptr)++ = size_cgpa(m);
		pack_cgpa(m, &mp->b_wptr);
		pd = p++;	/* DATA V */
		*p = mp->b_wptr - p;	/* Opt Params */
		p++;
		if ((m->parms & SCCP_PTF_SGMT)) {
			*(mp->b_wptr)++ = SCCP_PT_SGMT;	/* SGMT O */
			*(mp->b_wptr)++ = size_sgmt();
			pack_sgmt(m, &mp->b_wptr);
		}
		if ((m->parms & SCCP_PTF_IMP)) {
			*(mp->b_wptr)++ = SCCP_PT_IMP;	/* IMP O */
			*(mp->b_wptr)++ = size_imp();
			pack_imp(m, &mp->b_wptr);
		}
		if ((m->parms & SCCP_PTF_ISNI)) {
			*(mp->b_wptr)++ = SCCP_PT_ISNI;	/* ISNI O */
			*(mp->b_wptr)++ = size_isni(m);
			pack_isni(m, &mp->b_wptr);
		}
		if ((m->parms & SCCP_PTF_INS)) {
			*(mp->b_wptr)++ = SCCP_PT_INS;	/* INS O */
			*(mp->b_wptr)++ = size_ins(m);
			pack_ins(m, &mp->b_wptr);
		}
		if ((m->parms & SCCP_PTF_MTI)) {
			*(mp->b_wptr)++ = SCCP_PT_MTI;	/* MTI O */
			*(mp->b_wptr)++ = size_mti();
			pack_mti(m, &mp->b_wptr);
		}
		*(mp->b_wptr)++ = SCCP_PT_EOP;	/* EOP O */
		*pd = mp->b_wptr - pd;	/* DATA V */
		*(mp->b_wptr)++ = size_data(m);
		link_data(mp, msg->b_cont);
	}
	return (mp);
}

/*
 *  SCCP_MT_XUDTS 0x12 - Extended Unitdata Service
 *  -------------------------------------------------------------------------
 *  ITUT: F(RETC, HOPC), V(CDPA, CGPA, DATA[2-253]), O(SGMT, IMP, EOP)
 *  ANSI: F(RETC, HOPC), V(CDPA, CGPA, DATA[2-251]), O(SGMT, INS, MTI, ISNI, EOP)
 */
static mblk_t *
sccp_pack_xudts(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_retc() +		/* RETC */
	    size_hopc() +		/* HOPC */
	    2 + size_cdpa(m) +		/* CDPA */
	    2 + size_cgpa(m) +		/* CGPA */
	    2 + size_data(m) +		/* DATA */
	    1 +				/* oparms */
	    ((m->parms & SCCP_PTF_SGMT) ? 2 + size_sgmt() : 0) +	/* SGMT */
	    ((m->parms & SCCP_PTF_IMP) ? 2 + size_imp() : 0) +	/* IMP */
	    ((m->parms & SCCP_PTF_INS) ? 2 + size_ins(m) : 0) +	/* INS */
	    ((m->parms & SCCP_PTF_MTI) ? 2 + size_mti() : 0) +	/* MTI */
	    ((m->parms & SCCP_PTF_ISNI) ? 2 + size_isni(m) : 0) +	/* ISNI */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		register uchar *p, *pd;

		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_XUDTS;
		pack_retc(m, &mp->b_wptr);	/* RETC F */
		pack_hopc(m, &mp->b_wptr);	/* HOPC F */
		p = mp->b_wptr;
		mp->b_wptr += 4;
		*p = mp->b_wptr - p;	/* CDPA V */
		p++;
		*(mp->b_wptr)++ = size_cdpa(m);
		pack_cdpa(m, &mp->b_wptr);
		*p = mp->b_wptr - p;	/* CGPA V */
		p++;
		*(mp->b_wptr)++ = size_cgpa(m);
		pack_cgpa(m, &mp->b_wptr);
		pd = p++;	/* DATA V */
		*p = mp->b_wptr - p;	/* Opt Parms */
		p++;
		if (m->parms & SCCP_PTF_SGMT) {
			*(mp->b_wptr)++ = SCCP_PT_SGMT;	/* SGMT O */
			*(mp->b_wptr)++ = size_sgmt();
			pack_sgmt(m, &mp->b_wptr);
		}
		if (m->parms & SCCP_PTF_IMP) {
			*(mp->b_wptr)++ = SCCP_PT_IMP;	/* IMP O */
			*(mp->b_wptr)++ = size_imp();
			pack_imp(m, &mp->b_wptr);
		}
		if (m->parms & SCCP_PTF_INS) {
			*(mp->b_wptr)++ = SCCP_PT_INS;	/* INS O */
			*(mp->b_wptr)++ = size_ins(m);
			pack_ins(m, &mp->b_wptr);
		}
		if (m->parms & SCCP_PTF_MTI) {
			*(mp->b_wptr)++ = SCCP_PT_MTI;	/* MTI O */
			*(mp->b_wptr)++ = size_mti();
			pack_mti(m, &mp->b_wptr);
		}
		if (m->parms & SCCP_PTF_ISNI) {
			*(mp->b_wptr)++ = SCCP_PT_ISNI;	/* ISNI O */
			*(mp->b_wptr)++ = size_isni(m);
			pack_isni(m, &mp->b_wptr);
		}
		*(mp->b_wptr)++ = SCCP_PT_EOP;	/* EOP O */
		*pd = mp->b_wptr - pd;	/* DATA V */
		*(mp->b_wptr)++ = size_data(m);
		link_data(mp, msg->b_cont);
	}
	return (mp);
}

/*
 *  SCCP_MT_LUDT  0x13 - Long Unitdata
 *  -------------------------------------------------------------------------
 *  ITUT: F(PCLS, HOPC), V(CDPA, CGPA, LDATA[3-3906]), O(SGMT, IMP, EOP)
 *  ANSI: F(PCLS, HOPC), V(CDPA, CGPA, LDATA[3-3904]), O(SGMT, ISNI, INS, EOP)
 */
static mblk_t *
sccp_pack_ludt(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_pcls() +		/* PCLS */
	    size_hopc() +		/* HOPC */
	    2 + size_cdpa(m) +		/* CDPA */
	    2 + size_cgpa(m) +		/* CGPA */
	    2 + size_data(m) +		/* DATA */
	    1 +				/* oparms */
	    ((m->parms & SCCP_PTF_SGMT) ? 2 + size_sgmt() : 0) +	/* SGMT */
	    ((m->parms & SCCP_PTF_IMP) ? 2 + size_imp() : 0) +	/* IMP */
	    ((m->parms & SCCP_PTF_ISNI) ? 2 + size_isni(m) : 0) +	/* ISNI */
	    ((m->parms & SCCP_PTF_INS) ? 2 + size_ins(m) : 0) +	/* INS */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		register uchar *p, *pd;

		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_LUDT;
		pack_pcls(m, &mp->b_wptr);	/* PCLS F */
		pack_hopc(m, &mp->b_wptr);	/* HOPC F */
		p = mp->b_wptr;
		mp->b_wptr += 4;
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = size_cdpa(m);	/* CDPA V */
		pack_cdpa(m, &mp->b_wptr);
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = size_cgpa(m);	/* CGPA V */
		pack_cgpa(m, &mp->b_wptr);
		pd = p++;	/* LDATA V */
		*p = mp->b_wptr - p;	/* Opt Parms */
		p++;
		if (m->parms & SCCP_PTF_SGMT) {
			*(mp->b_wptr)++ = SCCP_PT_SGMT;	/* SGMT O */
			*(mp->b_wptr)++ = size_sgmt();
			pack_sgmt(m, &mp->b_wptr);
		}
		if (m->parms & SCCP_PTF_IMP) {
			*(mp->b_wptr)++ = SCCP_PT_IMP;	/* IMP O */
			*(mp->b_wptr)++ = size_imp();
			pack_imp(m, &mp->b_wptr);
		}
		if (m->parms & SCCP_PTF_ISNI) {
			*(mp->b_wptr)++ = SCCP_PT_ISNI;	/* ISNI O */
			*(mp->b_wptr)++ = size_isni(m);
			pack_isni(m, &mp->b_wptr);
		}
		if (m->parms & SCCP_PTF_INS) {
			*(mp->b_wptr)++ = SCCP_PT_INS;	/* INS O */
			*(mp->b_wptr)++ = size_ins(m);
			pack_ins(m, &mp->b_wptr);
		}
		*(mp->b_wptr)++ = SCCP_PT_EOP;	/* EOP O */
		*pd = mp->b_wptr - pd;	/* LDATA V */
		*(mp->b_wptr)++ = size_ldata(m);
		link_data(mp, msg->b_cont);
	}
	return (mp);
}

/*
 *  SCCP_MT_LUDTS 0x14 - Long Unitdata Service
 *  -------------------------------------------------------------------------
 *  ITUT: F(RETC, HOPC), V(CDPA, CGPA, LDATA[3-3906]), O(SGMT, IMP, EOP)
 *  ANSI: F(RETC, HOPC), V(CDPA, CGPA, LDATA[3-3904]), O(SGMT, ISNI, INS, EOP)
 */
static mblk_t *
sccp_pack_ludts(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t mlen = 1 +		/* MT */
	    size_retc() +		/* RETC */
	    size_hopc() +		/* HOPC */
	    2 + size_cdpa(m) +		/* CDPA */
	    2 + size_cgpa(m) +		/* CGPA */
	    2 + size_data(m) +		/* DATA */
	    1 +				/* oparms */
	    ((m->parms & SCCP_PTF_SGMT) ? 2 + size_sgmt() : 0) +	/* SGMT */
	    ((m->parms & SCCP_PTF_IMP) ? 2 + size_imp() : 0) +	/* IMP */
	    ((m->parms & SCCP_PTF_ISNI) ? 2 + size_isni(m) : 0) +	/* ISNI */
	    ((m->parms & SCCP_PTF_INS) ? 2 + size_ins(m) : 0) +	/* INS */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		register uchar *p, *pd;

		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_LUDTS;
		pack_retc(m, &mp->b_wptr);	/* RETC F */
		pack_hopc(m, &mp->b_wptr);	/* HOPC F */
		p = mp->b_wptr;
		mp->b_wptr += 4;
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = size_cdpa(m);	/* CDPA V */
		pack_cdpa(m, &mp->b_wptr);
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = size_cgpa(m);	/* CGPA V */
		pack_cgpa(m, &mp->b_wptr);
		*p = mp->b_wptr - p;
		p++;
		pd = p++;	/* LDATA V */
		*p = mp->b_wptr - p;	/* Opt Parms */
		p++;
		if (m->parms & SCCP_PTF_SGMT) {
			*(mp->b_wptr)++ = SCCP_PT_SGMT;	/* SGMT O */
			*(mp->b_wptr)++ = size_sgmt();
			pack_sgmt(m, &mp->b_wptr);
		}
		if (m->parms & SCCP_PTF_IMP) {
			*(mp->b_wptr)++ = SCCP_PT_IMP;	/* IMP O */
			*(mp->b_wptr)++ = size_imp();
			pack_imp(m, &mp->b_wptr);
		}
		if (m->parms & SCCP_PTF_ISNI) {
			*(mp->b_wptr)++ = SCCP_PT_ISNI;	/* ISNI O */
			*(mp->b_wptr)++ = size_isni(m);
			pack_isni(m, &mp->b_wptr);
		}
		if (m->parms & SCCP_PTF_INS) {
			*(mp->b_wptr)++ = SCCP_PT_INS;	/* INS O */
			*(mp->b_wptr)++ = size_ins(m);
			pack_ins(m, &mp->b_wptr);
		}
		*(mp->b_wptr)++ = SCCP_PT_EOP;	/* EOP O */
		/* always link data at end of message */
		*pd = mp->b_wptr - pd;	/* LDATA V */
		*(mp->b_wptr)++ = size_ldata(m);
		link_data(mp, msg->b_cont);
	}
	return (mp);
}

static mblk_t *
sccp_pack_scmg(queue_t *q, mblk_t *msg)
{
	mblk_t *mp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	size_t dlen =
	    size_scmgfi() + size_assn() + size_apc() + size_smi() +
	    ((m->fi == SCMG_MT_SSC) ? size_cong() : 0);
	size_t mlen = 1 +		/* MT */
	    size_pcls() +		/* PCLS */
	    2 + size_cdpa(m) +		/* CDPA */
	    2 + size_cgpa(m) +		/* CGPA */
	    2 + dlen +			/* DATA */
	    1;				/* EOP */

	if ((mp = mi_allocb(q, mlen, BPRI_MED))) {
		register uchar *p;

		DB_TYPE(mp) = M_DATA;
		*(mp->b_wptr)++ = SCCP_MT_UDT;
		pack_pcls(m, &mp->b_wptr);	/* PCLS F */
		p = mp->b_wptr;
		mp->b_wptr += 3;
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = size_cdpa(m);	/* CDPA V */
		pack_cdpa(m, &mp->b_wptr);
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = size_cgpa(m);	/* CGPA V */
		pack_cgpa(m, &mp->b_wptr);
		*p = mp->b_wptr - p;
		p++;
		*(mp->b_wptr)++ = size_data(m);	/* DATA V */
		pack_scmgfi(m, &mp->b_wptr);
		pack_assn(m, &mp->b_wptr);
		pack_apc(m, &mp->b_wptr);
		pack_smi(m, &mp->b_wptr);
		if (m->fi == SCMG_MT_SSC) {
			pack_cong(m, &mp->b_wptr);
		}
	}
	return (mp);
}

static int
sccp_send_msg(struct mt *mt, queue_t *q, mblk_t *bp, mblk_t *msg)
{
	mblk_t *mp, *dp;
	struct sccp_msg *m = (typeof(m)) msg->b_rptr;
	struct mtp_addr *a;
	struct MTP_transfer_req *p;

	if (likely((mp = mi_allocb(q, sizeof(*p) + sizeof(*a), BPRI_MED)) != NULL)) {
		DB_TYPE(mp) = M_PROTO;
		p = (typeof(p)) mp->b_wptr;
		mp->b_wptr += sizeof(*p);
		p->mtp_primitive = MTP_TRANSFER_REQ;
		p->mtp_dest_length = sizeof(*a);
		p->mtp_dest_offset = sizeof(*p);
		p->mtp_mp = m->rl.mp;
		p->mtp_sls = m->rl.sls;
		a = (typeof(a)) mp->b_wptr;
		mp->b_wptr += sizeof(*a);
		*a = mt->loc;
		a->pc = m->rl.dpc;
		switch (m->type) {
		case SCCP_MT_CR:	/* Connection Request */
			dp = sccp_pack_cr(q, msg);
			break;
		case SCCP_MT_CC:	/* Connection Confirm */
			dp = sccp_pack_cc(q, msg);
			break;
		case SCCP_MT_CREF:	/* Connection Refused */
			dp = sccp_pack_cref(q, msg);
			break;
		case SCCP_MT_RLSD:	/* Released */
			dp = sccp_pack_rlsd(q, msg);
			break;
		case SCCP_MT_RLC:	/* Release Complete */
			dp = sccp_pack_rlc(q, msg);
			break;
		case SCCP_MT_DT1:	/* Data Form 1 */
			dp = sccp_pack_dt1(q, msg);
			break;
		case SCCP_MT_DT2:	/* Data Form 2 */
			dp = sccp_pack_dt2(q, msg);
			break;
		case SCCP_MT_AK:	/* Data Acknowledgement */
			dp = sccp_pack_ak(q, msg);
			break;
		case SCCP_MT_UDT:	/* Unitdata */
			switch (m->fi) {
			case 0:
				dp = sccp_pack_udt(q, msg);
				break;
			case SCMG_MT_SSA:	/* Subsystem allowed */
			case SCMG_MT_SSP:	/* Subsystem prohibited */
			case SCMG_MT_SST:	/* Subsystem status test */
			case SCMG_MT_SOR:	/* Subsystem out of service request */
			case SCMG_MT_SOG:	/* Subsystem out of service grant */
			case SCMG_MT_SSC:	/* Subsystem congestion */
			case SCMG_MT_SBR:	/* Subsystem backup routing */
			case SCMG_MT_SNR:	/* Subsystem normal routing */
			case SCMG_MT_SRT:	/* Subsystem routing status test */
				dp = sccp_pack_scmg(q, msg);
				break;
			default:
				swerr();
				goto error_free;
			}
			break;
		case SCCP_MT_UDTS:	/* Unitdata Service */
			dp = sccp_pack_udts(q, msg);
			break;
		case SCCP_MT_ED:	/* Expedited Data */
			dp = sccp_pack_ed(q, msg);
			break;
		case SCCP_MT_EA:	/* Expedited Data Acknowledgement */
			dp = sccp_pack_ea(q, msg);
			break;
		case SCCP_MT_RSR:	/* Reset Request */
			dp = sccp_pack_rsr(q, msg);
			break;
		case SCCP_MT_RSC:	/* Reset Confirm */
			dp = sccp_pack_rsc(q, msg);
			break;
		case SCCP_MT_ERR:	/* Protocol Data Unit Error */
			dp = sccp_pack_err(q, msg);
			break;
		case SCCP_MT_IT:	/* Inactivity Test */
			dp = sccp_pack_it(q, msg);
			break;
		case SCCP_MT_XUDT:	/* Extended Unitdata */
			dp = sccp_pack_xudt(q, msg);
			break;
		case SCCP_MT_XUDTS:	/* Extended Unitdata Service */
			dp = sccp_pack_xudts(q, msg);
			break;
		case SCCP_MT_LUDT:	/* Long Unitdata */
			dp = sccp_pack_ludt(q, msg);
			break;
		case SCCP_MT_LUDTS:	/* Long Unitdata Service */
			dp = sccp_pack_ludts(q, msg);
			break;
		default:
			swerr();
			goto error_free;
		}
		if (dp) {
			mp->b_cont = dp;
			if (likely(canputnext(mt->oq))) {
				printd(("%s: %p: MTP_TRANSFER_REQ ->\n", DRV_NAME, mt));
				ss7_oput(mt->oq, mp);
				return (QR_DONE);
			}
			freemsg(mp);
			return (-EBUSY);
		}
		freeb(mp);
		return (-ENOBUFS);
	}
	return (-ENOBUFS);
      error_free:
	freeb(mp);
	if (bp)
		freeb(bp);
	freemsg(msg);
	return (0);
}

/*
 *  Encode, decode, send message functions
 *  -------------------------------------------------------------------------
 */
#if 0
static int
sccp_send(queue_t *q, struct sr *sr, np_ulong pri, np_ulong sls, mblk_t *dp)
{
	int err;
	struct mt *mt;

	if (!sr || (!(mt = sr->mt) && !(mt = sr->sp.sp->mt)))
		goto efault;
	if ((err = mtp_transfer_req(mt, q, &sr->add, pri, sls, dp)) < 0)
		goto error;
	return (QR_DONE);
      efault:
	err = -EFAULT;
	swerr();
	goto error;
      error:
	freemsg(dp);
	return (err);
}
#endif

static struct sccp_msg *
sccp_enc_msg(struct sp *sp, queue_t *q, np_ulong dpc, np_ulong pri, np_ulong sls)
{
	mblk_t *mp;
	struct sccp_msg *m = NULL;

	if ((mp = mi_allocb(q, sizeof(*m), BPRI_MED))) {
		DB_TYPE(mp) = M_RSE;
		mp->b_band = 3;
		m = (typeof(m)) mp->b_wptr;
		mp->b_wptr += sizeof(*m);
		bzero(m, sizeof(*m));
		m->bp = mp;
		m->eq = q;
		m->xq = q;
		m->sc = NULL;
		m->mt = NULL;
		m->timestamp = jiffies;
		m->rl.opc = sp->add.pc;
		m->rl.dpc = dpc;
		m->rl.sls = sls;
		m->rl.mp = pri;
	}
	return (m);
}

static int sccp_orte_msg(struct sp *, queue_t *, mblk_t *, mblk_t *);

/*
 *  SCCP_MT_CR    0x01 - Connection Request
 *  -------------------------------------------------------------------------
 *  ITUT: F(SLR, PCLS), V(CDPA), O(CRED, CGPA, DATA, HOPC, IMP, EOP)
 *  ASNI: F(SLR, PCLS), V(CDPA), O(CRED, CGPA, DATA, HOPC, EOP)
 */
static int
sccp_send_cr(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong pri, np_ulong sls,
	     np_ulong slr, np_ulong pcl, struct sccp_addr *cdpa, np_ulong *cred,
	     struct sccp_addr *cgpa, mblk_t *data, np_ulong *hopc, np_ulong *imp)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, pri, sls))) {
		m->type = SCCP_MT_CR;
		m->slr = slr;	/* SLR F */
		m->pcl = pcl;	/* PCL F */
		bcopy(cdpa, &m->cdpa, sizeof(*cdpa) + cdpa->alen);	/* CDPA V */
		if (cred) {	/* CRED O */
			m->cred = *cred;
			m->parms |= SCCP_PTF_CRED;
		}
		if (cgpa) {	/* CGPA O */
			bcopy(cgpa, &m->cgpa, sizeof(*cgpa) + cgpa->alen);
			m->parms |= SCCP_PTF_CGPA;
		}
		if (data) {	/* DATA O */
			m->data.buf = data;
			m->data.ptr = data->b_rptr;
			m->data.len = msgdsize(data);
			m->parms |= SCCP_PTF_DATA;
		}
		if (hopc) {	/* HOPC O */
			m->hopc = *hopc;
			m->parms |= SCCP_PTF_HOPC;
		}
		if (imp && !(sp->proto.popt & SS7_POPT_MPLEV)) {	/* IMP O */
			m->imp = *imp;
			if (m->imp > SCCP_MI_CR)
				m->imp = SCCP_MI_CR;
			m->parms |= SCCP_PTF_IMP;
		} else
			m->imp = SCCP_DI_CR;
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_CC    0x02 - Connection Confirm
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR, PCLS), O(CRED, CDPA, DATA, IMP, EOP)
 *  ANSI: F(DLR, SLR, PCLS), O(CRED, CDPA, DATA, EOP)
 */
static int
sccp_send_cc(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong sls, np_ulong dlr,
	     np_ulong slr, np_ulong pcl, np_ulong *cred, struct sccp_addr *cdpa, mblk_t *data,
	     np_ulong *imp)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, SCCP_MP_CC, sls))) {
		m->type = SCCP_MT_CC;
		m->dlr = dlr;	/* DLR F */
		m->slr = slr;	/* SLR F */
		m->pcl = pcl;	/* PCLS F */
		if (pcl == 3 && cred) {	/* CRED O */
			m->cred = *cred;
			m->parms |= SCCP_PTF_CRED;
		}
		if (cdpa) {	/* CDPA O */
			bcopy(cdpa, &m->cdpa, sizeof(*cdpa) + cdpa->alen);
			m->parms |= SCCP_PTF_CDPA;
		}
		if (data) {	/* DATA O */
			m->data.buf = data;
			m->data.ptr = data->b_rptr;
			m->data.len = msgdsize(data);
			m->parms |= SCCP_PTF_DATA;
		}
		if (imp && !(sp->proto.popt & SS7_POPT_MPLEV)) {	/* IMP O */
			m->imp = *imp;
			if (m->imp > SCCP_MI_CC)
				m->imp = SCCP_MI_CC;
			m->parms |= SCCP_PTF_IMP;
		} else
			m->imp = SCCP_DI_CC;
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_CREF  0x03 - Connection Refused
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, REFC), O(CDPA, DATA, IMP, EOP)
 *  ANSI: F(DLR, REFC), O(CDPA, DATA, EOP)
 */
static int
sccp_send_cref(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong sls, np_ulong dlr,
	       np_ulong refc, struct sccp_addr *cdpa, mblk_t *data, np_ulong *imp)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, SCCP_MP_CREF, sls))) {
		m->type = SCCP_MT_CREF;
		m->dlr = dlr;	/* DLR F */
		m->cause = refc;	/* REFC F */
		if (cdpa) {	/* CDPA O */
			bcopy(cdpa, &m->cdpa, sizeof(*cdpa) + cdpa->alen);
			m->parms |= SCCP_PTF_CDPA;
		}
		if (data) {	/* DATA O */
			m->data.buf = data;
			m->data.ptr = data->b_rptr;
			m->data.len = msgdsize(data);
			m->parms |= SCCP_PTF_DATA;
		}
		if (imp && !(sp->proto.popt & SS7_POPT_MPLEV)) {	/* IMP O */
			m->imp = *imp;
			if (m->imp > SCCP_MI_CREF)
				m->imp = SCCP_MI_CREF;
			m->parms |= SCCP_PTF_IMP;
		} else
			m->imp = SCCP_DI_CREF;
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_RLSD  0x04 - Released
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR, RELC), O(DATA, IMP, EOP)
 *  ANSI: F(DLR, SLR, RELC), O(DATA, EOP)
 */
static int
sccp_send_rlsd(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong sls, np_ulong dlr,
	       np_ulong slr, np_ulong relc, mblk_t *data, np_ulong *imp)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, SCCP_MP_RLSD, sls))) {
		m->type = SCCP_MT_RLSD;
		m->dlr = dlr;	/* DLR F */
		m->slr = slr;	/* SLR F */
		m->cause = relc;	/* RELC F */
		if (data) {	/* DATA O */
			m->data.buf = data;
			m->data.ptr = data->b_rptr;
			m->data.len = msgdsize(data);
			m->parms |= SCCP_PTF_DATA;
		}
		if (imp && !(sp->proto.popt & SS7_POPT_MPLEV)) {	/* IMP O */
			m->imp = *imp;
			if (m->imp > SCCP_MI_RLSD)
				m->imp = SCCP_MI_RLSD;
			m->parms |= SCCP_PTF_IMP;
		} else
			m->imp = SCCP_DI_RLSD;
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_RLC   0x05 - Release Complete
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR)
 *  ANSI: F(DLR, SLR)
 */
static int
sccp_send_rlc(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong sls, np_ulong dlr,
	      np_ulong slr)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, SCCP_MP_RLC, sls))) {
		m->type = SCCP_MT_RLC;
		m->dlr = dlr;	/* DLR F */
		m->slr = slr;	/* SLR F */
		m->imp = SCCP_DI_RLC;
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_DT1   0x06 - Data Form 1
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SEG), V(DATA)
 *  ANSI: F(DLR, SEG), V(DATA)
 */
static inline int
sccp_send_dt1(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong pri, np_ulong sls,
	      np_ulong dlr, np_ulong more, mblk_t *data)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, pri, sls))) {
		m->type = SCCP_MT_DT1;
		m->dlr = dlr;	/* DLR F */
		m->more = more;	/* SEG F */
		m->data.buf = data;	/* DATA V */
		m->data.ptr = data->b_rptr;
		m->data.len = msgdsize(data);
		m->imp = SCCP_DI_DT1;
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_DT2   0x07 - Data Form 2
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SEQ), V(DATA)
 *  ANSI: F(DLR, SEQ), V(DATA)
 */
static inline int
sccp_send_dt2(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong pri, np_ulong sls,
	      np_ulong dlr, np_ulong ps, np_ulong pr, np_ulong more, mblk_t *data)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, pri, sls))) {
		m->type = SCCP_MT_DT2;
		m->dlr = dlr;	/* DLR F */
		m->more = more;	/* SEG F */
		m->data.buf = data;	/* DATA V */
		m->data.ptr = data->b_rptr;
		m->data.len = msgdsize(data);
		m->imp = SCCP_DI_DT2;
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_AK    0x08 - Data Acknowledgement
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, RSN, CRED)
 *  ANSI: F(DLR, RSN, CRED)
 */
static int
sccp_send_ak(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong pri, np_ulong sls,
	     np_ulong dlr, np_ulong rsn, np_ulong cred)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, pri, sls))) {
		m->type = SCCP_MT_AK;
		m->dlr = dlr;	/* DLR F */
		m->rsn = rsn;	/* RSN F */
		m->cred = cred;	/* CRED F */
		m->imp = SCCP_DI_AK;
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_UDT   0x09 - Unitdata
 *  -------------------------------------------------------------------------
 *  ITUT: F(PCLS), V(CDPA, CGPA, DATA[2-254])
 *  ANSI: F(PCLS), V(CDPA, CGPA, DATA[2-252])
 */
static inline int
sccp_send_udt(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong pri, np_ulong sls,
	      np_ulong pcl, np_ulong ret, struct sccp_addr *cdpa, struct sccp_addr *cgpa,
	      mblk_t *data)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, pri, sls))) {
		m->type = SCCP_MT_UDT;
		m->pcl = pcl;	/* PCLS F */
		m->ret = ret;
		bcopy(cdpa, &m->cdpa, sizeof(*cdpa) + cdpa->alen);	/* CDPA V */
		bcopy(cgpa, &m->cgpa, sizeof(*cgpa) + cgpa->alen);	/* CGPA V */
		m->data.buf = data;	/* DATA V */
		m->data.ptr = data->b_rptr;
		m->data.len = msgdsize(data);
		m->imp = SCCP_DI_UDT;
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_UDTS  0x0a - Unitdata Service
 *  -------------------------------------------------------------------------
 *  ITUT: F(RETC), V(CDPA, CGPA, DATA[2-253])
 *  ANSI: F(RETC), V(CDPA, CGPA, DATA[2-251])
 */
static int
sccp_send_udts(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong pri, np_ulong sls,
	       np_ulong retc, struct sccp_addr *cdpa, struct sccp_addr *cgpa, mblk_t *data)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, pri, sls))) {
		m->type = SCCP_MT_UDTS;
		m->cause = retc;	/* RETC F */
		bcopy(cdpa, &m->cdpa, sizeof(*cdpa) + cdpa->alen);	/* CDPA V */
		bcopy(cgpa, &m->cgpa, sizeof(*cgpa) + cgpa->alen);	/* CGPA V */
		m->data.buf = data;	/* DATA V */
		m->data.ptr = data->b_rptr;
		m->data.len = msgdsize(data);
		m->imp = SCCP_DI_UDTS;
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_ED    0x0b - Expedited Data
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR), V(DATA)
 *  ANSI: F(DLR), V(DATA)
 */
static int
sccp_send_ed(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong sls, np_ulong dlr,
	     mblk_t *data)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, SCCP_MP_ED, sls))) {
		m->type = SCCP_MT_ED;
		m->dlr = dlr;	/* DLR F */
		m->data.buf = data;	/* DATA V */
		m->data.ptr = data->b_rptr;
		m->data.len = msgdsize(data);
		m->imp = SCCP_DI_ED;
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_EA    0x0c - Expedited Data Acknowledgement
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR)
 *  ANSI: F(DLR)
 */
static int
sccp_send_ea(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong sls, np_ulong dlr)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, SCCP_MP_EA, sls))) {
		m->type = SCCP_MT_ED;
		m->dlr = dlr;	/* DLR F */
		m->imp = SCCP_DI_EA;
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_RSR   0x0d - Reset Request
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR, RESC)
 *  ANSI: F(DLR, SLR, RESC)
 */
static inline int
sccp_send_rsr(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong sls, np_ulong dlr,
	      np_ulong slr, np_ulong resc)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, SCCP_MP_RSR, sls))) {
		m->type = SCCP_MT_RSR;
		m->dlr = dlr;	/* DLR F */
		m->slr = slr;	/* SLR F */
		m->cause = resc;	/* RESC F */
		m->imp = SCCP_DI_RSR;
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_RSC   0x0e - Reset Confirm
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR)
 *  ANSI: F(DLR, SLR)
 */
static int
sccp_send_rsc(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong sls, np_ulong dlr,
	      np_ulong slr)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, SCCP_MP_RSC, sls))) {
		m->type = SCCP_MT_RSC;
		m->dlr = dlr;	/* DLR F */
		m->slr = slr;	/* SLR F */
		m->imp = SCCP_DI_RSC;
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_ERR   0x0f - Protocol Data Unit Error
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, ERRC)
 *  ANSI: F(DLR, ERRC)
 */
static int
sccp_send_err(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong sls, np_ulong dlr,
	      np_ulong errc)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, SCCP_MP_ERR, sls))) {
		m->type = SCCP_MT_ERR;
		m->dlr = dlr;	/* DLR F */
		m->cause = errc;	/* ERRC F */
		m->imp = SCCP_DI_ERR;
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_IT    0x10 - Inactivity Test
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR, PCLS, SEQ, CRED)
 *  ANSI: F(DLR, SLR, PCLS, SEQ, CRED)
 */
static int
sccp_send_it(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong sls, np_ulong dlr,
	     np_ulong slr, np_ulong pcl, np_ulong ps, np_ulong pr, np_ulong more, np_ulong cred)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, SCCP_MP_IT, sls))) {
		m->type = SCCP_MT_IT;
		m->dlr = dlr;	/* DLR F */
		m->slr = slr;	/* SLR F */
		m->pcl = pcl;	/* PCLS F */
		m->ps = ps;	/* SEQ F */
		m->pr = pr;
		m->more = more;
		m->cred = cred;	/* CRED F */
		m->imp = SCCP_DI_IT;
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_XUDT  0x11 - Extended Unitdata
 *  -------------------------------------------------------------------------
 *  ITUT: F(PCLS, HOPC), V(CDPA, CGPA, DATA[2-253]), O(SGMT, IMP, EOP)
 *  ANSI: F(PCLS, HOPC), V(CDPA, CGPA, DATA[2-251]), O(SGMT, ISNI, INS, MTI, EOP)
 */
static inline int
sccp_send_xudt(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong pri, np_ulong sls,
	       np_ulong pcl, np_ulong ret, np_ulong hopc, struct sccp_addr *cdpa,
	       struct sccp_addr *cgpa, mblk_t *data, sccp_sgmt_t * sgmt, np_ulong *imp,
	       sccp_isni_t * isni, sccp_ins_t * ins, np_ulong *mti)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, pri, sls))) {
		m->type = SCCP_MT_XUDT;
		m->pcl = pcl;	/* PCLS F */
		m->ret = ret;
		m->hopc = hopc;	/* HOPC F */
		bcopy(cdpa, &m->cdpa, sizeof(*cdpa) + cdpa->alen);	/* CDPA V */
		bcopy(cgpa, &m->cgpa, sizeof(*cgpa) + cgpa->alen);	/* CGPA V */
		m->data.buf = data;	/* DATA V */
		m->data.ptr = data->b_rptr;
		m->data.len = msgdsize(data);
		if (sgmt) {	/* SGMT O */
			m->sgmt = *sgmt;
			m->parms |= SCCP_PTF_SGMT;
		}
		if (imp && !(sp->proto.popt & SS7_POPT_MPLEV)) {	/* IMP O */
			m->imp = *imp;
			if (m->imp > SCCP_MI_XUDT)
				m->imp = SCCP_MI_XUDT;
			m->parms |= SCCP_PTF_IMP;
		} else
			m->imp = SCCP_DI_XUDT;
		if ((m->pvar & SS7_PVAR_MASK) == SS7_PVAR_ANSI) {
			if (isni) {	/* ISNI O */
				m->isni = *isni;
				m->parms |= SCCP_PTF_ISNI;
			}
			if (ins) {	/* INS O */
				m->ins = *ins;
				m->parms |= SCCP_PTF_INS;
			}
			if (mti) {	/* MTI O */
				m->mti = *mti;
				m->parms |= SCCP_PTF_MTI;
			}
		}
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_XUDTS 0x12 - Extended Unitdata Service
 *  -------------------------------------------------------------------------
 *  ITUT: F(RETC, HOPC), V(CDPA, CGPA, DATA[2-253]), O(SGMT, IMP, EOP)
 *  ANSI: F(RETC, HOPC), V(CDPA, CGPA, DATA[2-251]), O(SGMT, INS, MTI, ISNI, EOP)
 */
static int
sccp_send_xudts(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong pri, np_ulong sls,
		np_ulong retc, np_ulong hopc, struct sccp_addr *cdpa, struct sccp_addr *cgpa,
		mblk_t *data, sccp_sgmt_t * sgmt, np_ulong *imp, sccp_ins_t * ins, np_ulong *mti,
		sccp_isni_t * isni)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, pri, sls))) {
		m->type = SCCP_MT_XUDTS;
		m->cause = retc;	/* RETC F */
		bcopy(cdpa, &m->cdpa, sizeof(*cdpa) + cdpa->alen);	/* CDPA V */
		bcopy(cgpa, &m->cgpa, sizeof(*cgpa) + cgpa->alen);	/* CGPA V */
		m->data.buf = data;	/* DATA V */
		m->data.ptr = data->b_rptr;
		m->data.len = msgdsize(data);
		if (sgmt) {	/* SGMT O */
			m->sgmt = *sgmt;
			m->parms |= SCCP_PTF_SGMT;
		}
		if (imp && !(sp->proto.popt & SS7_POPT_MPLEV)) {	/* IMP O */
			m->imp = *imp;
			if (m->imp > SCCP_MI_XUDTS)
				m->imp = SCCP_MI_XUDTS;
			m->parms |= SCCP_PTF_IMP;
		} else
			m->imp = SCCP_DI_XUDTS;
		if ((m->pvar & SS7_PVAR_MASK) == SS7_PVAR_ANSI) {
			if (isni) {	/* ISNI O */
				m->isni = *isni;
				m->parms |= SCCP_PTF_ISNI;
			}
			if (ins) {	/* INS O */
				m->ins = *ins;
				m->parms |= SCCP_PTF_INS;
			}
			if (mti) {	/* MTI O */
				m->mti = *mti;
				m->parms |= SCCP_PTF_MTI;
			}
		}
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_LUDT  0x13 - Long Unitdata
 *  -------------------------------------------------------------------------
 *  ITUT: F(PCLS, HOPC), V(CDPA, CGPA, LDATA[3-3906]), O(SGMT, IMP, EOP)
 *  ANSI: F(PCLS, HOPC), V(CDPA, CGPA, LDATA[3-3904]), O(SGMT, ISNI, INS, EOP)
 */
static inline int
sccp_send_ludt(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong pri, np_ulong sls,
	       np_ulong pcl, np_ulong ret, np_ulong hopc, struct sccp_addr *cdpa,
	       struct sccp_addr *cgpa, mblk_t *data, sccp_sgmt_t * sgmt, np_ulong *imp,
	       sccp_isni_t * isni, sccp_ins_t * ins)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, pri, sls))) {
		m->type = SCCP_MT_LUDT;
		m->pcl = pcl;	/* PCLS F */
		m->ret = ret;
		m->hopc = hopc;	/* HOPC F */
		bcopy(cdpa, &m->cdpa, sizeof(*cdpa) + cdpa->alen);	/* CDPA V */
		bcopy(cgpa, &m->cgpa, sizeof(*cgpa) + cgpa->alen);	/* CGPA V */
		m->data.buf = data;	/* DATA V */
		m->data.ptr = data->b_rptr;
		m->data.len = msgdsize(data);
		if (sgmt) {	/* SGMT O */
			m->sgmt = *sgmt;
			m->parms |= SCCP_PTF_SGMT;
		}
		if (imp && !(sp->proto.popt & SS7_POPT_MPLEV)) {	/* IMP O */
			m->imp = *imp;
			if (m->imp > SCCP_MI_LUDT)
				m->imp = SCCP_MI_LUDT;
			m->parms |= SCCP_PTF_IMP;
		} else
			m->imp = SCCP_DI_LUDT;
		if ((m->pvar & SS7_PVAR_MASK) == SS7_PVAR_ANSI) {
			if (isni) {	/* ISNI O */
				m->isni = *isni;
				m->parms |= SCCP_PTF_ISNI;
			}
			if (ins) {	/* INS O */
				m->ins = *ins;
				m->parms |= SCCP_PTF_INS;
			}
		}
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCCP_MT_LUDTS 0x14 - Long Unitdata Service
 *  -------------------------------------------------------------------------
 *  ITUT: F(RETC, HOPC), V(CDPA, CGPA, LDATA[3-3906]), O(SGMT, IMP, EOP)
 *  ANSI: F(RETC, HOPC), V(CDPA, CGPA, LDATA[3-3904]), O(SGMT, ISNI, INS, EOP)
 */
static int
sccp_send_ludts(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong pri, np_ulong sls,
		np_ulong retc, np_ulong hopc, struct sccp_addr *cdpa, struct sccp_addr *cgpa,
		mblk_t *data, sccp_sgmt_t * sgmt, np_ulong *imp, sccp_isni_t * isni,
		sccp_ins_t * ins)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, pri, sls))) {
		m->type = SCCP_MT_LUDTS;
		m->cause = retc;	/* RETC F */
		bcopy(cdpa, &m->cdpa, sizeof(*cdpa) + cdpa->alen);	/* CDPA V */
		bcopy(cgpa, &m->cgpa, sizeof(*cgpa) + cgpa->alen);	/* CGPA V */
		m->data.buf = data;	/* DATA V */
		m->data.ptr = data->b_rptr;
		m->data.len = msgdsize(data);
		if (sgmt) {	/* SGMT O */
			m->sgmt = *sgmt;
			m->parms |= SCCP_PTF_SGMT;
		}
		if (imp && !(sp->proto.popt & SS7_POPT_MPLEV)) {	/* IMP O */
			m->imp = *imp;
			if (m->imp > SCCP_MI_LUDTS)
				m->imp = SCCP_MI_LUDTS;
			m->parms |= SCCP_PTF_IMP;
		} else
			m->imp = SCCP_DI_LUDTS;
		if ((m->pvar & SS7_PVAR_MASK) == SS7_PVAR_ANSI) {
			if (isni) {	/* ISNI O */
				m->isni = *isni;
				m->parms |= SCCP_PTF_ISNI;
			}
			if (ins) {	/* INS O */
				m->ins = *ins;
				m->parms |= SCCP_PTF_INS;
			}
		}
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCMG Message
 *  -------------------------------------------------------------------------
 */
static int
sccp_send_scmg(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong pri, np_ulong sls,
	       struct sccp_addr *cdpa, struct sccp_addr *cgpa, np_ulong scmgfi, np_ulong assn,
	       np_ulong apc, np_ulong smi, np_ulong cong)
{
	struct sccp_msg *m;

	if ((m = sccp_enc_msg(sp, q, dpc, pri, sls))) {
		m->type = SCCP_MT_UDT;
		m->pcl = 0;	/* PCLS F */
		m->ret = 0;
		bcopy(cdpa, &m->cdpa, sizeof(*cdpa) + cdpa->alen);	/* CDPA V */
		bcopy(cgpa, &m->cgpa, sizeof(*cgpa) + cgpa->alen);	/* CGPA V */
		m->fi = scmgfi;
		m->assn = assn;
		m->apc = apc;
		m->smi = smi;
		m->cong = cong;
		return sccp_orte_msg(sp, q, bp, m->bp);
	}
	return (-ENOBUFS);
}

/*
 *  SCMG_MT_SSA     0x01 - Subsystem allowed
 *  -------------------------------------------------------------------------
 */
static int
sccp_send_ssa(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, struct sccp_addr *cdpa,
	      struct sccp_addr *cgpa, np_ulong assn, np_ulong apc, np_ulong smi)
{
	return sccp_send_scmg(sp, q, bp, dpc, SCMG_MP_SSA, sp->sccp_next_cl_sls++, cdpa, cgpa,
			      SCMG_MT_SSA, assn, apc, smi, 0);
}
static inline int
sccp_bcast_ssa(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, np_ulong assn, np_ulong apc,
	       np_ulong smi)
{
	int err;
	struct sr *sr;
	struct sccp_addr cgpa;

	bzero(&cgpa, sizeof(cgpa));
	cgpa.ni = sp->add.ni;
	cgpa.ri = SCCP_RI_DPC_SSN;
	cgpa.pc = sp->add.pc;
	cgpa.ssn = 1;
	for (sr = sp->sr.list; sr; sr = sr->sp.next) {
		struct sccp_addr cdpa;

		bzero(&cdpa, sizeof(cdpa));
		cdpa.ni = sr->add.ni;
		cdpa.ri = SCCP_RI_DPC_SSN;
		cdpa.pc = sr->add.pc;
		cdpa.ssn = 1;
		if ((err = sccp_send_ssa(sp, q, NULL, dpc, &cdpa, &cgpa, assn, apc, smi)) < 0)
			goto error;
		if (bp)
			freeb(bp);
	}
	return (QR_DONE);
      error:
	return (err);
}

/*
 *  SCMG_MT_SSP     0x02 - Subsystem prohibited
 *  -------------------------------------------------------------------------
 */
static int
sccp_send_ssp(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, struct sccp_addr *cdpa,
	      struct sccp_addr *cgpa, np_ulong assn, np_ulong apc, np_ulong smi)
{
	return sccp_send_scmg(sp, q, bp, dpc, SCMG_MP_SSP, sp->sccp_next_cl_sls++, cdpa, cgpa,
			      SCMG_MT_SSP, assn, apc, smi, 0);
}

/*
 *  SCMG_MT_SST     0x03 - Subsystem status test
 *  -------------------------------------------------------------------------
 */
static inline int
sccp_send_sst(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, struct sccp_addr *cdpa,
	      struct sccp_addr *cgpa, np_ulong assn, np_ulong apc, np_ulong smi)
{
	return sccp_send_scmg(sp, q, bp, dpc, SCMG_MP_SST, sp->sccp_next_cl_sls++, cdpa, cgpa,
			      SCMG_MT_SST, assn, apc, smi, 0);
}

/*
 *  SCMG_MT_SOR     0x04 - Subsystem out of service request
 *  -------------------------------------------------------------------------
 */
static inline int
sccp_send_sor(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, struct sccp_addr *cdpa,
	      struct sccp_addr *cgpa, np_ulong assn, np_ulong apc, np_ulong smi)
{
	return sccp_send_scmg(sp, q, bp, dpc, SCMG_MP_SOR, sp->sccp_next_cl_sls++, cdpa, cgpa,
			      SCMG_MT_SOR, assn, apc, smi, 0);
}

/*
 *  SCMG_MT_SOG     0x05 - Subsystem out of service grant
 *  -------------------------------------------------------------------------
 */
static inline int
sccp_send_sog(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, struct sccp_addr *cdpa,
	      struct sccp_addr *cgpa, np_ulong assn, np_ulong apc, np_ulong smi)
{
	return sccp_send_scmg(sp, q, bp, dpc, SCMG_MP_SOG, sp->sccp_next_cl_sls++, cdpa, cgpa,
			      SCMG_MT_SOG, assn, apc, smi, 0);
}

/*
 *  SCMG_MT_SSC     0x06 - Subsystem congestion
 *  -------------------------------------------------------------------------
 */
static int
sccp_send_ssc(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, struct sccp_addr *cdpa,
	      struct sccp_addr *cgpa, np_ulong assn, np_ulong apc, np_ulong smi, np_ulong cong)
{
	return sccp_send_scmg(sp, q, bp, dpc, SCMG_MP_SSC, sp->sccp_next_cl_sls++, cdpa, cgpa,
			      SCMG_MT_SSC, assn, apc, smi, cong);
}

/*
 *  SCMG_MT_SBR     0xfd - Subsystem backup routing
 *  -------------------------------------------------------------------------
 */
static inline int
sccp_send_sbr(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, struct sccp_addr *cdpa,
	      struct sccp_addr *cgpa, np_ulong assn, np_ulong apc, np_ulong smi)
{
	return sccp_send_scmg(sp, q, bp, dpc, SCMG_MP_SBR, sp->sccp_next_cl_sls++, cdpa, cgpa,
			      SCMG_MT_SBR, assn, apc, smi, 0);
}

/*
 *  SCMG_MT_SNR     0xfe - Subsystem normal routing
 *  -------------------------------------------------------------------------
 */
static inline int
sccp_send_snr(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, struct sccp_addr *cdpa,
	      struct sccp_addr *cgpa, np_ulong assn, np_ulong apc, np_ulong smi)
{
	return sccp_send_scmg(sp, q, bp, dpc, SCMG_MP_SNR, sp->sccp_next_cl_sls++, cdpa, cgpa,
			      SCMG_MT_SNR, assn, apc, smi, 0);
}

/*
 *  SCMG_MT_SRT     0xff - Subsystem routing status test
 *  -------------------------------------------------------------------------
 */
static inline int
sccp_send_srt(struct sp *sp, queue_t *q, mblk_t *bp, np_ulong dpc, struct sccp_addr *cdpa,
	      struct sccp_addr *cgpa, np_ulong assn, np_ulong apc, np_ulong smi)
{
	return sccp_send_scmg(sp, q, bp, dpc, SCMG_MP_SRT, sp->sccp_next_cl_sls++, cdpa, cgpa,
			      SCMG_MT_SRT, assn, apc, smi, 0);
}

/*
 *  -------------------------------------------------------------------------
 *
 *  Configuration Defaults
 *
 *  -------------------------------------------------------------------------
 */

static sccp_opt_conf_na_t itut_na_config_default = {
};
static sccp_opt_conf_na_t etsi_na_config_default = {
};
static sccp_opt_conf_na_t ansi_na_config_default = {
};
static sccp_opt_conf_na_t jttc_na_config_default = {
};

#if 0
static sccp_opt_conf_sp_t itut_sp_config_default = {
};
static sccp_opt_conf_sp_t etsi_sp_config_default = {
};
static sccp_opt_conf_sp_t ansi_sp_config_default = {
};
static sccp_opt_conf_sp_t jttc_sp_config_default = {
};

static sccp_opt_conf_sr_t itut_sr_config_default = {
};
static sccp_opt_conf_sr_t etsi_sr_config_default = {
};
static sccp_opt_conf_sr_t ansi_sr_config_default = {
};
static sccp_opt_conf_sr_t jttc_sr_config_default = {
};
#endif

/*
 *  -------------------------------------------------------------------------
 *
 *  Timers
 *
 *  -------------------------------------------------------------------------
 */

enum { tall, tcon, tias, tiar, trel, trel2, tint, tguard, tres, trea, tack, tgtt,
	tattack, tdecay, tstatinfo, tsst, tisst, twsog, trsst
};

struct sc_timer {
	uint timer;
	union {
		struct sc *sc;
		struct ss *ss;
		struct rs *rs;
		struct sp *sp;
		struct sr *sr;
	};
};

/*
 *  SCCP timers
 *  -------------------------------------------------------------------------
 */
static void
sccp_timer_stop(struct sc *sc, const uint t)
{
	int single = 1;

	switch (t) {
	case tall:
		single = 0;
		/* fall through */
	case tcon:
		mi_timer_stop(sc->timers.tcon);
		if (single)
			break;
		/* fall through */
	case tias:
		mi_timer_stop(sc->timers.tias);
		if (single)
			break;
		/* fall through */
	case tiar:
		mi_timer_stop(sc->timers.tiar);
		if (single)
			break;
		/* fall through */
	case trel:
		mi_timer_stop(sc->timers.trel);
		if (single)
			break;
		/* fall through */
	case trel2:
		mi_timer_stop(sc->timers.trel2);
		if (single)
			break;
		/* fall through */
	case tint:
		mi_timer_stop(sc->timers.tint);
		if (single)
			break;
		/* fall through */
	case tguard:
		mi_timer_stop(sc->timers.tguard);
		if (single)
			break;
		/* fall through */
	case tres:
		mi_timer_stop(sc->timers.tres);
		if (single)
			break;
		/* fall through */
	case trea:
		mi_timer_stop(sc->timers.trea);
		if (single)
			break;
		/* fall through */
	case tack:
		mi_timer_stop(sc->timers.tack);
		if (single)
			break;
		/* fall through */
		break;
	default:
		swerr();
		break;
	}
}
static void
sccp_timer_start(struct sc *sc, const uint t)
{
	switch (t) {
	case tcon:
		mi_timer_MAC(sc->timers.tcon, sc->config.tcon);
		break;
	case tias:
		mi_timer_MAC(sc->timers.tias, sc->config.tias);
		break;
	case tiar:
		mi_timer_MAC(sc->timers.tiar, sc->config.tiar);
		break;
	case trel:
		mi_timer_MAC(sc->timers.trel, sc->config.trel);
		break;
	case trel2:
		mi_timer_MAC(sc->timers.trel2, sc->config.trel2);
		break;
	case tint:
		mi_timer_MAC(sc->timers.tint, sc->config.tint);
		break;
	case tguard:
		mi_timer_MAC(sc->timers.tguard, sc->config.tguard);
		break;
	case tres:
		mi_timer_MAC(sc->timers.tres, sc->config.tres);
		break;
	case trea:
		mi_timer_MAC(sc->timers.trea, sc->config.trea);
		break;
	case tack:
		mi_timer_MAC(sc->timers.tack, sc->config.tack);
		break;
	default:
		swerr();
		break;
	}
}

/*
 *  SS timers
 *  -------------------------------------------------------------------------
 */
static void
ss_timer_stop(struct ss *ss, const uint t)
{
	int single = 1;

	switch (t) {
	case tall:
		single = 0;
		/* fall through */
	case tisst:
		mi_timer_stop(ss->timers.tisst);
		if (single)
			break;
		/* fall through */
	case twsog:
		mi_timer_stop(ss->timers.twsog);
		if (single)
			break;
		/* fall through */
		break;
	default:
		swerr();
		break;
	}
}
static inline void
ss_timer_start(struct ss *ss, const uint t)
{
	switch (t) {
	case tisst:
		mi_timer_MAC(ss->timers.tisst, ss->config.tisst);
		break;
	case twsog:
		mi_timer_MAC(ss->timers.twsog, ss->config.twsog);
		break;
	default:
		swerr();
		break;
	}
}

/*
 *  RS timers
 *  -------------------------------------------------------------------------
 */
static void
rs_timer_stop(struct rs *rs, const uint t)
{
	int single = 1;

	switch (t) {
	case tall:
		single = 0;
		/* fall through */
	case trsst:
		mi_timer_stop(rs->timers.tsst);
		if (single)
			break;
		/* fall through */
		break;
	default:
		swerr();
		break;
	}
}
static inline void
rs_timer_start(struct rs *rs, const uint t)
{
	switch (t) {
	case trsst:
		mi_timer_MAC(rs->timers.tsst, rs->config.tsst);
		break;
	default:
		swerr();
		break;
	}
}

/*
 *  SP timers
 *  -------------------------------------------------------------------------
 */
static void
sp_timer_stop(struct sp *sp, const uint t)
{
	int single = 1;

	switch (t) {
	case tall:
		single = 0;
		/* fall through */
	case tgtt:
		mi_timer_stop(sp->timers.tgtt);
		if (single)
			break;
		/* fall through */
		break;
	default:
		swerr();
		break;
	}
}
static inline void
sp_timer_start(struct sp *sp, const uint t)
{
	switch (t) {
	case tgtt:
		mi_timer_MAC(sp->timers.tgtt, sp->config.tgtt);
		break;
	default:
		swerr();
		break;
	}
}

/*
 *  SR timers
 *  -------------------------------------------------------------------------
 */
static void
sr_timer_stop(struct sr *sr, const uint t)
{
	int single = 1;

	switch (t) {
	case tall:
		single = 0;
		/* fall through */
	case tattack:
		mi_timer_stop(sr->timers.tattack);
		if (single)
			break;
		/* fall through */
	case tdecay:
		mi_timer_stop(sr->timers.tdecay);
		if (single)
			break;
		/* fall through */
	case tstatinfo:
		mi_timer_stop(sr->timers.tstatinfo);
		if (single)
			break;
		/* fall through */
	case tsst:
		mi_timer_stop(sr->timers.tsst);
		if (single)
			break;
		/* fall through */
		break;
	default:
		swerr();
		break;
	}
}
static void
sr_timer_start(struct sr *sr, const uint t)
{
	switch (t) {
	case tattack:
		mi_timer_MAC(sr->timers.tattack, sr->config.tattack);
		break;
	case tdecay:
		mi_timer_MAC(sr->timers.tdecay, sr->config.tdecay);
		break;
	case tstatinfo:
		mi_timer_MAC(sr->timers.tstatinfo, sr->config.tstatinfo);
		break;
	case tsst:
		mi_timer_MAC(sr->timers.tsst, sr->config.tsst);
		break;
	default:
		swerr();
		break;
	}
}

/*
 *  -------------------------------------------------------------------------
 *
 *  SCCP State Machines
 *
 *  -------------------------------------------------------------------------
 */

/*
 *  -------------------------------------------------------------------------
 *
 *  SCCP Interface Functions
 *
 *  -------------------------------------------------------------------------
 *  These interface functions adapt between user interface events requested by the state machine and
 *  the particular style of interface that is supported by the particular SCCP user stream.  Not all
 *  events are supported by all interface styles.  The most complete interface style is the SCCPI
 *  which is the NPI Release 2 interface with the extended N-primitives described in Q.711 and
 *  T1.112.1.
 */

static int
sccp_conn_ind(struct sc *sc, mblk_t *msg, queue_t *q, mblk_t *cp)
{
	switch (sc->i_style) {
	case SCCP_STYLE_SCCPI:
	case SCCP_STYLE_NPI:
		return n_conn_ind(sc, q, msg, cp, NULL);
	case SCCP_STYLE_TPI:
		return t_conn_ind(sc, q, msg, cp, NULL);
	case SCCP_STYLE_GTT:
	case SCCP_STYLE_MGMT:
	default:
		mi_strlog(q, 0, SL_ERROR, "attempt to deliver to GTT or MGMT stream");
		freemsg(msg);
		freemsg(cp);
		return (0);
	}
}

static int
sccp_conn_con(struct sc *sc, queue_t *q, mblk_t *msg, np_ulong flags, struct sccp_addr *res,
	      mblk_t *dp)
{
	switch (sc->i_style) {
	case SCCP_STYLE_SCCPI:
	case SCCP_STYLE_NPI:
		return n_conn_con(sc, q, msg, flags, res, dp);
	case SCCP_STYLE_TPI:
		return t_conn_con(sc, q, msg, flags, res, dp);
	case SCCP_STYLE_GTT:
	case SCCP_STYLE_MGMT:
	default:
		mi_strlog(q, 0, SL_ERROR | SL_TRACE, "%s: attempt to deliver to GTT or MGMT stream",
			  __FUNCTION__);
		freemsg(dp);
		if (msg && msg->b_cont == dp)
			msg->b_cont = NULL;
		freemsg(msg);
		return (0);
	}
}

static int
sccp_data_ind(struct sc *sc, queue_t *q, mblk_t *msg, np_ulong exp, np_ulong more,
	      N_qos_sel_data_sccp_t * qos, mblk_t *dp)
{
	switch (sc->i_style) {
	case SCCP_STYLE_SCCPI:
	case SCCP_STYLE_NPI:
		if (exp)
			return n_exdata_ind(sc, q, msg, more, qos, dp);
		else
			return n_data_ind(sc, q, msg, more, qos, dp);
	case SCCP_STYLE_TPI:
		if (qos == NULL) {
			if (exp)
				return t_exdata_ind(sc, q, msg, more, dp);
			else
				return t_data_ind(sc, q, msg, more, dp);
		} else {
			return t_optdata_ind(sc, q, msg, exp, more, qos, dp);
		}
	case SCCP_STYLE_GTT:
	case SCCP_STYLE_MGMT:
	default:
		mi_strlog(q, 0, SL_ERROR, "attempt to deliver to GTT or MGMT stream");
		freemsg(dp);
		if (msg && msg->b_cont == dp)
			msg->b_cont = NULL;
		freemsg(msg);
		return (0);
	}
}

static int
sccp_datack_ind(struct sc *sc, queue_t *q)
{
	switch (sc->i_style) {
	case SCCP_STYLE_SCCPI:
	case SCCP_STYLE_NPI:
		return n_datack_ind(sc, q, NULL);
	case SCCP_STYLE_TPI:
		/* could possibly use t_opdata_ind here */
		return (-EOPNOTSUPP);
	case SCCP_STYLE_GTT:
	case SCCP_STYLE_MGMT:
	default:
		mi_strlog(q, 0, SL_ERROR, "attempt to deliver to GTT or MGMT stream");
		return (0);
	}
}

static int
sccp_reset_ind(struct sc *sc, queue_t *q, np_ulong orig, np_ulong reason, mblk_t *cp)
{
	switch (sc->i_style) {
	case SCCP_STYLE_SCCPI:
	case SCCP_STYLE_NPI:
		return n_reset_ind(sc, q, NULL, orig, reason, cp);
	case SCCP_STYLE_TPI:
		/* could possibly use t_opdata_ind here */
		return (-EOPNOTSUPP);
	case SCCP_STYLE_GTT:
	case SCCP_STYLE_MGMT:
	default:
		mi_strlog(q, 0, SL_ERROR, "attempt to deliver to GTT or MGMT stream");
		freemsg(cp);
		return (0);
	}
}

static int
sccp_reset_con(struct sc *sc, queue_t *q)
{
	switch (sc->i_style) {
	case SCCP_STYLE_SCCPI:
	case SCCP_STYLE_NPI:
		/* could possibly use t_opdata_ind here */
		return n_reset_con(sc, q, NULL);
	case SCCP_STYLE_TPI:
		return (-EOPNOTSUPP);
	case SCCP_STYLE_GTT:
	case SCCP_STYLE_MGMT:
	default:
		mi_strlog(q, 0, SL_ERROR, "attempt to deliver to GTT or MGMT stream");
		return (0);
	}
}

static int
sccp_discon_ind(struct sc *sc, queue_t *q, np_ulong orig, np_long reason, struct sccp_addr *res,
		mblk_t *cp, mblk_t *dp)
{
	switch (sc->i_style) {
	case SCCP_STYLE_SCCPI:
	case SCCP_STYLE_NPI:
		return n_discon_ind(sc, q, NULL, orig, reason, res, cp, dp);
	case SCCP_STYLE_TPI:
		return t_discon_ind(sc, q, NULL, reason, cp, dp);
	case SCCP_STYLE_GTT:
	case SCCP_STYLE_MGMT:
	default:
		mi_strlog(q, 0, SL_ERROR, "attempt to deliver to GTT or MGMT stream");
		freemsg(cp);
		freemsg(dp);
		return (0);
	}
}

#if 0
static int
sccp_ordrel_ind(struct sc *sc, queue_t *q, mblk_t *msg)
{
	switch (sc->i_style) {
	case SCCP_STYLE_SCCPI:
	case SCCP_STYLE_NPI:
		return n_ordrel_ind(sc, q, msg);
	case SCCP_STYLE_TPI:
		return t_ordrel_ind(sc, q, msg);
	case SCCP_STYLE_GTT:
	case SCCP_STYLE_MGMT:
	default:
		mi_strlog(q, 0, SL_ERROR, "attempt to deliver to GTT or MGMT stream");
		freemsg(msg);
		freemsg(dp);
		return (0);
	}
}
#endif

static int
sccp_unitdata_ind(struct sc *sc, queue_t *q, struct sccp_addr *dst, struct sccp_addr *src,
		  np_ulong pcl, np_ulong opt, np_ulong imp, np_ulong seq, np_ulong pri, mblk_t *dp)
{
	switch (sc->i_style) {
	case SCCP_STYLE_SCCPI:
	case SCCP_STYLE_NPI:
		return n_unitdata_ind(sc, q, NULL, src, dst, dp);
	case SCCP_STYLE_TPI:
		return t_unitdata_ind(sc, q, NULL, src, &seq, &pri, &pcl, &imp, &opt, dp);
	case SCCP_STYLE_GTT:
	case SCCP_STYLE_MGMT:
	default:
		mi_strlog(q, 0, SL_ERROR, "attempt to deliver to GTT or MGMT stream");
		freemsg(dp);
		return (0);
	}
}

static int
sccp_notice_ind(struct sc *sc, queue_t *q, np_ulong cause, struct sccp_addr *dst,
		struct sccp_addr *src, np_ulong pcl, np_ulong ret, np_ulong imp, np_ulong seq,
		np_ulong pri, mblk_t *dp)
{
	switch (sc->i_style) {
	case SCCP_STYLE_SCCPI:
		return n_notice_ind(sc, q, NULL, cause, dst, src, pri, seq, pcl, imp, dp);
	case SCCP_STYLE_NPI:
		return n_uderror_ind(sc, q, NULL, dst, dp, cause);
	case SCCP_STYLE_TPI:
		return t_uderror_ind(sc, q, NULL, cause, dst, &seq, &pri, &pcl, &imp, &ret, dp);
	case SCCP_STYLE_GTT:
	case SCCP_STYLE_MGMT:
	default:
		mi_strlog(q, 0, SL_ERROR, "attempt to deliver to GTT or MGMT stream");
		freemsg(dp);
		return (0);
	}
}

#if 0
static int
sccp_inform_ind(struct sc *sc, queue_t *q, mblk_t *msg, N_qos_sel_infr_sccp_t * qos,
		np_ulong reason)
{
	switch (sc->i_style) {
	case SCCP_STYLE_SCCPI:
		return n_inform_ind(sc, q, qos, reason);
	case SCCP_STYLE_NPI:
		/* XXX: could possibly use n_reset_ind */
	case SCCP_STYLE_TPI:
		/* XXX: could possible use t_optdata_ind with no data */
	case SCCP_STYLE_GTT:
	case SCCP_STYLE_MGMT:
	default:
		mi_strlog(q, 0, SL_ERROR, "attempt to deliver to GTT or MGMT stream");
		freemsg(msg);
		freemsg(dp);
		return (0);
	}
}

static int
sccp_coord_ind(struct sc *sc, queue_t *q, mblk_t *msg, struct sccp_addr *add)
{
	switch (sc->i_style) {
	case SCCP_STYLE_SCCPI:
	case SCCP_STYLE_MGMT:
	case SCCP_STYLE_GTT:
		return n_coord_ind(sc, q, msg, add);
	case SCCP_STYLE_NPI:
	case SCCP_STYLE_TPI:
		swerr();
		return (-EOPNOTSUPP);
	default:
		mi_strlog(q, 0, SL_ERROR, "attempt to deliver to GTT or MGMT stream");
		freemsg(msg);
		freemsg(dp);
		return (0);
	}
}

static int
sccp_coord_con(struct sc *sc, queue_t *q, mblk_t *msg, struct sccp_addr *add, np_ulong smi)
{
	switch (sc->i_style) {
	case SCCP_STYLE_SCCPI:
	case SCCP_STYLE_GTT:
	case SCCP_STYLE_MGMT:
		return n_coord_con(sc, q, msg, add, smi);
	case SCCP_STYLE_NPI:
	case SCCP_STYLE_TPI:
		swerr();
		return (-EOPNOTSUPP);
	default:
		mi_strlog(q, 0, SL_ERROR, "attempt to deliver to GTT or MGMT stream");
		freemsg(msg);
		freemsg(dp);
		return (0);
	}
}
#endif

static int
sccp_state_ind(struct sc *sc, queue_t *q, struct sccp_addr *add, np_ulong status, np_ulong smi)
{
	switch (sc->i_style) {
	case SCCP_STYLE_SCCPI:
	case SCCP_STYLE_GTT:
	case SCCP_STYLE_MGMT:
		return n_state_ind(sc, q, NULL, add, status, smi);
	case SCCP_STYLE_NPI:
		/* could possibly use n_uderror_ind or n_reset_ind */
	case SCCP_STYLE_TPI:
		/* could possibly use t_uderror_ind or t_optdata_ind */
		swerr();
		return (-EOPNOTSUPP);
	default:
		mi_strlog(q, 0, SL_ERROR, "attempt to deliver to GTT or MGMT stream");
		return (0);
	}
}

/*
   local broadcast of N-STATE indication 
 */
static int
sccp_state_lbr(queue_t *q, struct sccp_addr *add, np_ulong status, np_ulong smi)
{
	int err;
	struct na *na;
	struct sp *sp;
	struct ss *ss;
	struct sc *sc;

	if (!(na = na_lookup(add->ni)))
		goto done;
	for (sp = na->sp.list; sp; sp = sp->na.next) {
		for (sc = sp->gt.list; sc; sc = sc->sp.next) {
			if ((err = sccp_state_ind(sc, q, add, status, smi)) < 0)
				goto error;
		}
		for (ss = sp->ss.list; ss; ss = ss->sp.next) {
			for (sc = ss->cl.list; sc; sc = sc->ss.next) {
				if ((err = sccp_state_ind(sc, q, add, status, smi)) < 0)
					goto error;
			}
			for (sc = ss->cr.list; sc; sc = sc->ss.next) {
				if ((err = sccp_state_ind(sc, q, add, status, smi)) < 0)
					goto error;
			}
			for (sc = ss->co.list; sc; sc = sc->ss.next) {
				if ((err = sccp_state_ind(sc, q, add, status, smi)) < 0)
					goto error;
			}
		}
	}
      done:
	return (QR_DONE);
      error:
	return (err);
}

static int
sccp_pcstate_ind(struct sc *sc, queue_t *q, struct mtp_addr *add, np_ulong status)
{
	switch (sc->i_style) {
	case SCCP_STYLE_SCCPI:
	case SCCP_STYLE_GTT:
	case SCCP_STYLE_MGMT:
		return n_pcstate_ind(sc, q, NULL, add, status);
	case SCCP_STYLE_NPI:
		/* could possibly use n_uderror_ind or n_reset_ind */
	case SCCP_STYLE_TPI:
		/* could possibly use t_uderror_ind or t_optdata_ind */
		swerr();
		return (-EOPNOTSUPP);
	default:
		swerr();
		return (-EFAULT);
	}
}

/*
   local broadcast of N-PCSTATE indication 
 */
static int
sccp_pcstate_lbr(queue_t *q, struct mtp_addr *add, np_ulong status)
{
	int err;
	struct na *na;
	struct sp *sp;
	struct ss *ss;
	struct sc *sc;

	if (!(na = na_lookup(add->ni)))
		goto done;
	for (sp = na->sp.list; sp; sp = sp->na.next) {
		for (sc = sp->gt.list; sc; sc = sc->sp.next) {
			if ((err = sccp_pcstate_ind(sc, q, add, status)) < 0)
				goto error;
		}
		for (ss = sp->ss.list; ss; ss = ss->sp.next) {
			for (sc = ss->cl.list; sc; sc = sc->ss.next) {
				if ((err = sccp_pcstate_ind(sc, q, add, status)) < 0)
					goto error;
			}
			for (sc = ss->cr.list; sc; sc = sc->ss.next) {
				if ((err = sccp_pcstate_ind(sc, q, add, status)) < 0)
					goto error;
			}
			for (sc = ss->co.list; sc; sc = sc->ss.next) {
				if ((err = sccp_pcstate_ind(sc, q, add, status)) < 0)
					goto error;
			}
		}
	}
      done:
	return (QR_DONE);
      error:
	return (err);
}

#if 0
static int
sccp_traffic_ind(struct sc *sc, queue_t *q, struct sccp_addr *add, np_ulong tmix)
{
	switch (sc->i_style) {
	case SCCP_STYLE_SCCPI:
	case SCCP_STYLE_GTT:
	case SCCP_STYLE_MGMT:
		return n_traffic_ind(sc, q, add, tmix);
	case SCCP_STYLE_NPI:
		/* could possibly use n_uderror_ind or n_reset_ind */
	case SCCP_STYLE_TPI:
		/* could possibly use t_uderror_ind or t_optdata_ind */
		swerr();
		return (-EOPNOTSUPP);
	default:
		swerr();
		return (-EFAULT);
	}
}
#endif

static void
sccp_release(struct sc *sc)
{
	psw_t flags;

	sccp_timer_stop(sc, tcon);
	sccp_timer_stop(sc, tias);
	sccp_timer_stop(sc, tiar);
	sccp_timer_stop(sc, trel);
	sccp_timer_stop(sc, tint);
	sccp_timer_stop(sc, trel2);
	spin_lock_irqsave(&master.lock, flags);
	{
		if (sc->sp.sp) {
			fixme(("Freeze local reference\n"));
			// sc->slr = 0; /* Oooops need this for released message */ 
			if ((*(sc->sp.prev) = sc->sp.next))
				sc->sp.next->sp.prev = sc->sp.prev;
			sc->sp.next = NULL;
			sc->sp.prev = &sc->sp.next;
			sp_put(xchg(&sc->sp.sp, NULL));
			sccp_put(sc);
		}
	}
	spin_unlock_irqrestore(&master.lock, flags);
	return;
}

static int
sccp_release_error(struct sc *sc, queue_t *q)
{
	int err;

	/* Figure C.2/Q.714 Sheet 2 of 7: Figure 2A/T1.112.4-2000 Sheet 2 of 4: stop connection
	   timer, release resources and local reference, freeze local reference, send N_DISCON_IND
	   to user, and move to the idle state. */
	sccp_release(sc);
	if ((err =
	     sccp_discon_ind(sc, q, N_PROVIDER, SCCP_RELC_REMOTE_PROCEDURE_ERROR, NULL, NULL,
			     NULL)))
		return (err);
	sccp_set_state(sc, SS_IDLE);
	return (QR_DONE);
}

/*
 *  -------------------------------------------------------------------------
 *
 *  SCCP Receive Message Functions
 *
 *  -------------------------------------------------------------------------
 *  These messages are received at the upper queue to which they belong: they have already been routed to the
 *  appropriate upper queue by the lower queue discrimination functions and have been decoded and checked for the
 *  existence of mandatory parameters.
 */
/*
 *  SCCP_MT_CR    0x01 - Connection Request
 *  -------------------------------------------------------------------------
 *  ITUT: F(SLR, PCLS), V(CDPA), O(CRED, CGPA, DATA, HOPC, IMP, EOP)
 *  ASNI: F(SLR, PCLS), V(CDPA), O(CRED, CGPA, DATA, HOPC, EOP)
 */
static int
sccp_recv_cr(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* CRED is only used with protocol class 3, DATA is optional, HOPC is only used at a relay
	   node, CGPA is optional, the OPC of the message is used if the CGPA is absent, IMP is
	   used for congestion control */
	int err;

	switch (sccp_get_state(sc)) {
	case SS_IDLE:
	case SS_WRES_CIND:
		/* Notes: we defer all of this stuff until we get an N_CONN_RES from the listening
		   stream.  Then we associate all of this stuff with the connection section.  This
		   might not be completely correct.  SCCP only sends one CR and then assigns a new
		   reference to the next connection.  We could actually use the local reference as
		   the SEQUENCE_number in the N_CONN_IND. */
		/* Figure 2B/T1.112.4-2000 1of2: if resources are not available, send a CREF;
		   otherwise, assign a local reference and SLS to incoming connection section and
		   determine protocol class and credit, associate originating node of CR with the
		   incoming connection section, send an N_CONN_IND to the user and move to the
		   connection pending state. */
		/* Figure C.3/Q.714 Sheet 1 of 6: if resources are not available, send a CREF;
		   otherwise, associate the remote reference to the connection, determine protocol
		   class and credit, send user an N_CONN_IND, move to the connection pending state. 
		 */
		/* FIXME: should send pcl and src address in options */
		if ((err = sccp_conn_ind(sc, bp, q, msg)))
			return (err);
		sccp_set_state(sc, SS_WRES_CIND);
		return (0);
	case SS_WCON_CREQ2:
		/* Note that ITUT96 does not use SLR in the CR to send a RLSD message as does ANSI. 
		 */
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ITUT:
		case SS7_PVAR_ETSI:
			/* Figure C.2/Q.714 Sheet 3 of 7: stop connection timer, release resources
			   and local reference, freeze local reference, move to the idle state. */
			return sccp_release_error(sc, q);
		default:
		case SS7_PVAR_JTTC:
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 3 of 4: same action as for CC in this
			   state: stop connection timer, associate destination reference to
			   connection section, release resources, stop inactivity timers, send
			   RLSD, start released (ANSI: and interval timers), move to the disconnect 
			   pending state. */
			sccp_release(sc);
			sc->dlr = m->slr;
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					    sc->slr, SCCP_RELC_END_USER_ORIGINATED, NULL, NULL)))
				return (err);
			sccp_timer_start(sc, trel);
			sccp_timer_start(sc, tint);	/* ANSI */
			sccp_set_state(sc, SS_WCON_DREQ);
			return (0);
		}
	default:
		swerr();
		return (-EFAULT);
	}
}

/*
 *  SCCP_MT_CC    0x02 - Connection Confirm
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR, PCLS), O(CRED, CDPA, DATA, IMP, EOP)
 *  ANSI: F(DLR, SLR, PCLS), O(CRED, CDPA, DATA, EOP)
 */
static int
sccp_recv_cc(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* CRED is only used with protocol class 3, DATA is optional, CDPA is optional, the OPC of
	   the message is used in subseqent messages if CDPA is absent, IMP is used for congestion
	   control.  DLR is used to look up the stream in the NS_WCON_REQ state.  SLR is saved.
	   PCLS is passed to user.  This should result in an N_CONN_CON primitive to the SCCP-User. 
	 */
	int err;

	// mblk_t *mp;
	struct sccp_addr *res = m->parms & SCCP_PTF_CDPA ? &m->cdpa : NULL;
	np_ulong flags = m->pcl == 3 ? REC_CONF_OPT | EX_DATA_OPT : 0;

	switch (sccp_get_state(sc)) {
	case SS_WCON_CREQ:
		/* Figure C.2/Q.714 Sheet 2 of 7: Figure 2A/T1.112.4-2000 Sheet 2 of 4: stop
		   connection timer, start inactivity timers, assign protocol class and credit,
		   associate remote reference to connection section, (ANSI: associate originating
		   node of CC with connection section), send N_CONN_CON to user, and enter the data 
		   transfer state. */
		sccp_timer_stop(sc, tcon);
		sc->cqos.protocol_class = m->pcl;
		if ((err = sccp_conn_con(sc, q, bp, flags, res, msg->b_cont->b_cont)))
			return (err);
		sccp_timer_start(sc, tias);
		sccp_timer_start(sc, tiar);
		if ((sc->pcl = m->pcl) == 3)
			sc->p_cred = m->cred;
		sc->dlr = m->slr;
		sc->sp.sp->add.pc = m->rl.opc;	/* ANSI */
		sccp_set_state(sc, SS_DATA_XFER);
		return (QR_DONE);
	case SS_WCON_CREQ2:
		/* Figure C.2/Q.714 Sheet 3 of 7: Figure 2A/T1.112.4-2000 Sheet 3 of 4: Stop
		   connection timer, associate destination reference to connection section, release 
		   resources, stop inactivity timers, send RLSD, start released (ANSI: and interval 
		   timers), move to the disconnect pending state. */
		sccp_release(sc);
		sc->dlr = m->slr;
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					    sc->slr, SCCP_RELC_END_USER_ORIGINATED, NULL, NULL)))
				return (err);
			break;
		default:
		case SS7_PVAR_ITUT:
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					    sc->slr, SCCP_RELC_END_USER_ORIGINATED, NULL,
					    &sc->imp)))
				return (err);
			break;
		}
		sccp_timer_start(sc, trel);
		sccp_timer_start(sc, tint);	/* ANSI */
		sccp_set_state(sc, SS_WCON_DREQ);
		return (QR_DONE);
	case SS_WRES_RIND:
		/* Figure C.6/Q.714 Sheet 4 of 4: discard received message and generate internal
		   reset request, move to the active state. */
		fixme(("Internal reset request\n"));
		sccp_set_state(sc, SS_DATA_XFER);
		return (QR_DONE);
	case SS_WRES_DIND:
		goto error1;
	      error1:
		/* send RLSD */
		sccp_set_state(sc, SS_WCON_DREQ);
		return (QR_DONE);
	case SS_IDLE:
		/* Figure 2A/T1.112.4-2000 1of4: Figure 2B/T1.112.4-2000 1of2: send ERR. */
		if ((err =
		     sccp_send_err(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr,
				   SCCP_ERRC_UNQUALIFIED)))
			return (err);
		swerr();	/* Note: this shouldn't happen */
		return (QR_DONE);
	case SS_DATA_XFER:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000: Figure 2B/T1.112.4-2000 2of2: send an
			   N_DISCON_IND to the user, release resources for the connection, stop
			   inactivity timers, send RLSD, start released and interval timers, move
			   to the disconnect pending state. */
			sccp_release(sc);
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					    sc->slr, SCCP_RELC_REMOTE_PROCEDURE_ERROR, NULL, NULL)))
				return (err);
			{
				int cause = 0;

				fixme(("Determine cuase\n"));
				if ((err =
				     sccp_discon_ind(sc, q, N_PROVIDER, cause, NULL, NULL, NULL)))
					return (err);
			}
			sccp_timer_start(sc, trel);
			sccp_timer_start(sc, tint);
			sccp_set_state(sc, SS_WCON_DREQ);
			return (QR_DONE);
		default:
			/* Figure C.3/Q.714 Sheet 3 of 6: discard */
			return (QR_DONE);
		}
	case SS_BOTH_RESET:	/* Figure C.6/Q.714 Sheet 3 of 4 */
	case SS_WCON_RREQ:	/* Figure C.6/Q.714 Sheet 2 of 4 */
	case SS_WCON_DREQ:	/* Figure C.2/Q.714 Sheet 6 of 7 */
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
}

/*
 *  SCCP_MT_CREF  0x03 - Connection Refused
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, REFC), O(CDPA, DATA, IMP, EOP)
 *  ANSI: F(DLR, REFC), O(CDPA, DATA, EOP)
 */
static int
sccp_recv_cref(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* CDPA is completely optional and only identifies the ultimate refuser, the OPC of the
	   message is used if the CDPA is absent, IMP is used for congestion control.  DLR is used
	   to look up the stream in the NS_WCON_CREQ state.  This should result in a N_DISCON_IND
	   primitive to the SCCP-User. */
	int err;
	np_ulong cause = 0x5000 | m->cause;
	struct sccp_addr *res = m->parms & SCCP_PTF_CDPA ? &m->cdpa : NULL;

	switch (sccp_get_state(sc)) {
	case SS_WCON_CREQ:
		/* Figure C.2/Q.714 Sheet 2 of 7: Figure 2A/T1.112.4-2000 Sheet 2 of 4: stop the
		   connection timer, release resources and local reference, freeze local reference, 
		   and send N_DISCON_IND to the user.  Move to the idle state. */
		sccp_release(sc);
		if ((err = sccp_discon_ind(sc, q, N_USER, cause, res, NULL, msg->b_cont)))
			return (err);
		sccp_set_state(sc, SS_IDLE);
		return (QR_DONE);
	case SS_WCON_CREQ2:
		/* Figure C.2/Q.714 Sheet 3 of 7: Figure 2A/T1.112.4-2000 Sheet 3 of 4: Stop
		   connection timer, release resources and references, move to the idle state. */
		sccp_release(sc);
		sccp_set_state(sc, SS_IDLE);
		return (QR_DONE);
	case SS_WRES_RIND:
		/* Figure C.6/Q.714 Sheet 4 of 4: discard received message and generate internal
		   reset request, move to the active state. */
		fixme(("Internal reset request\n"));
		sccp_set_state(sc, SS_DATA_XFER);
		return (QR_DONE);
	case SS_WRES_DIND:
		goto error1;
	      error1:
		/* send RLSD */
		sccp_set_state(sc, SS_WCON_DREQ);
		return (QR_DONE);
	case SS_DATA_XFER:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000: Figure 2B/T1.112.4-2000: send an N_DISCON_IND
			   to the user, release resources for the connection, stop inactivity
			   timers, send RLSD, start released and interval timers, move to the
			   disconnect pending state. */
			sccp_release(sc);
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					    sc->slr, SCCP_RELC_END_USER_ORIGINATED, NULL, NULL)))
				return (err);
			{
				int cause = 0;

				fixme(("Determine cuase\n"));
				if ((err =
				     sccp_discon_ind(sc, q, N_PROVIDER, cause, NULL, NULL, NULL)))
					return (err);
			}
			sccp_timer_start(sc, trel);
			sccp_timer_start(sc, tint);
			sccp_set_state(sc, SS_WCON_DREQ);
			return (QR_DONE);
		default:
			/* Figure C.3/Q.714 Sheet 3 of 6: discard */
			return (QR_DONE);
		}
	case SS_BOTH_RESET:	/* Figure C.6/Q.714 Sheet 3 of 4 */
	case SS_WCON_RREQ:	/* Figure C.6/Q.714 Sheet 2 of 4 */
	case SS_WCON_DREQ:	/* Figure C.2/Q.714 Sheet 6 of 7 */
	case SS_IDLE:		/* Figure 2A/T1.112.4-2000 1of4: Figure 2B 1of2 */
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
}

/*
 *  SCCP_MT_RLSD  0x04 - Released
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR, RELC), O(DATA, IMP, EOP)
 *  ANSI: F(DLR, SLR, RELC), O(DATA, EOP)
 */
static int
sccp_recv_rlsd(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* DATA is optional, IMP is used for congestion control.  DLR is used to look up the stream 
	   in the NS_DATA_XFER state.  This should result in a N_DISCON_IND primitive to the
	   SCCP-User and a SCCP_MT_RLC message issued back to the peer. */
	int err;
	mblk_t *cp = NULL;
	struct sccp_addr *res = m->parms & SCCP_PTF_CDPA ? &m->cdpa : NULL;

	fixme(("Have to look up any connection indications and set cp\n"));
	switch (sccp_get_state(sc)) {
	case SS_IDLE:
		/* Figure 2A/T1.112.4-2000 1of4: Figure 2B/T1.112.4-2000 1of2: return a release
		   complete. */
		if ((err =
		     sccp_send_rlc(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr, sc->slr)))
			return (err);
		/* Note: this case cannot happen.  We handle this case in the bottom end routing,
		   so this should not happen. */
		swerr();
		return (QR_DONE);
	case SS_WRES_CIND:
		/* Note: the ANSI procedures permit the side initiating the connection to release
		   the connection before receiving the Connection Confirm, which is weird as it
		   waits for the connection confirm before releasing under user control. */
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2B/T1.112.4-2000 1of2: send N_DISCON_IND to user, release
			   resources and reference for the connection section, stop inactivity
			   timers, send RLC, move to the idle state. */
			fixme(("Look up cc cp in connection list\n"));
			sccp_release(sc);
			if ((err =
			     sccp_send_rlc(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					   sc->slr)))
				return (err);
			if ((err =
			     sccp_discon_ind(sc, q, N_USER, 0x4000 | m->cause, res, cp,
					     msg->b_cont)))
				return (err);
			sccp_set_state(sc, SS_IDLE);
			return (QR_DONE);
		default:
		case SS7_PVAR_ITUT:
			/* Figure C.2/Q.714 2of6: discard received message. */
			return (QR_DONE);
		}
	case SS_DATA_XFER:
		/* Figure C.2/Q.714 Sheet 5 of 7: Figure C.3/Q.714 Sheet 4 of 6: Figure
		   2A/T1.112.4-2000 Sheet 4 of 4: Figure 2B/T1.112.4-2000 2of2: send N_DISCON_IND
		   to user, release resources and local reference, freeze local reference, stop
		   inactivity timers, send RLC, and move to the IDLE state. */
		sccp_release(sc);
		if ((err =
		     sccp_send_rlc(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr, sc->slr)))
			return (err);
		if ((err = sccp_discon_ind(sc, q, N_USER, 0x1000 | m->cause, res, cp, msg->b_cont)))
			return (err);
		sccp_set_state(sc, SS_IDLE);
		return (QR_DONE);
	case SS_WCON_DREQ:
		/* Figure C.2/Q.714 Sheet 6 of 7: release resources and local reference, freeze
		   local reference, stop release and interval timers, move to the idle state.
		   Figure 2A/T1.112.4-2000: Figure 2B/T1.112.4-2000: Release references for the
		   connection section, stop release and interval timers, move to the idle state. */
		sccp_release(sc);
		sccp_set_state(sc, SS_IDLE);
		return (QR_DONE);
	case SS_WRES_RIND:
		/* Figure C.2/Q.714 Sheet 4 of 4: FIXME this is confusing. */
		fixme(("Figure this out\n"));
		return (-EFAULT);
	case SS_WCON_CREQ:
		/* Figure C.2/Q.714 Sheet 2 of 7: Figure 2A/T1.112.4-2000 Sheet 2 of 4: reply with
		   RLC, stop the connection timer, release resources and local reference, freeze
		   local reference, and send N_DISCON_IND to the user. */
		sccp_release(sc);
		if ((err =
		     sccp_send_rlc(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr, sc->slr)))
			return (err);
		if ((err = sccp_discon_ind(sc, q, N_USER, 0x1000 | m->cause, res, cp, msg->b_cont)))
			return (err);
		sccp_set_state(sc, SS_IDLE);
		return (QR_DONE);
	case SS_WCON_CREQ2:
		/* Figure C.2/Q.714 Sheet 3 of 7: In this case we have already received an
		   N_DISCON_REQ from the user.  Send an RLC, stop the connection timer, release
		   resources and local reference, freeze local reference, and move to the idle
		   state. */
		sccp_release(sc);
		if ((err =
		     sccp_send_rlc(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr, sc->slr)))
			return (err);
		sccp_set_state(sc, SS_IDLE);
		return (QR_DONE);
	case SS_BOTH_RESET:
		/* Figure C.6/Q.714 Sheet 3 of 4: same as below. */
	case SS_WCON_RREQ:
		/* Figure 2E/T1.112.4-2000 2of2: stop reset timer and go to Figure 2A/T1.112.4-2000 
		   4of4: send N_DISCON_IND to user, release resources and reference for the
		   connection section, stop inactivity timers, send RLC and move to the idle state. 
		   Same as below for ITU. */
		/* Figure C.6/Q.714 Sheet 2 of 4: Figure C.2/Q.714 Sheet 5 of 7: Send an
		   N_DISCON_IND to the user, release resource and local reference, freeze local
		   reference, stop inactivity timers, send a RLC, and move to the Idle state. */
		sccp_release(sc);
		if ((err =
		     sccp_send_rlc(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr, sc->slr)))
			return (err);
		if ((err = sccp_discon_ind(sc, q, N_USER, 0x1000 | m->cause, res, cp, msg->b_cont)))
			return (err);
		sccp_set_state(sc, SS_IDLE);
		return (QR_DONE);
	case SS_WRES_DIND:
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
}

/*
 *  SCCP_MT_RLC   0x05 - Release Complete
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR)
 *  ANSI: F(DLR, SLR)
 */
static int
sccp_recv_rlc(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* No optional parameters.  Optional parameters will be ignored.  DLR is used to look up
	   the stream in the NS_WACK_DREQ or NS_IDLE state.  This should result in a N_OK_ACK
	   primitive to the SCCP-User if not already sent. */
	int err;

	// struct sccp_addr *res = m->parms & SCCP_PTF_CDPA ? &m->cdpa : NULL;
	switch (sccp_get_state(sc)) {
	case SS_WCON_DREQ:
		/* Figure C.2/Q.714 Sheet 6 of 7: release resources and local reference for
		   connection section, freeze local reference, stop release and interval timers,
		   move to idle state. Figure 2A/T1.112.4-2000: Figure 2B/T1.112.4-2000 2of2:
		   Release references for the connection section, stop release and interval timers, 
		   move to the idle state. */
		sccp_release(sc);
		sccp_set_state(sc, SS_IDLE);
		return (QR_DONE);
	case SS_WRES_RIND:
		/* Figure C.6/Q.714 Sheet 4 of 4: discard received message and generate internal
		   reset request, move to the active state. */
		fixme(("Internal reset request\n"));
		sccp_set_state(sc, SS_DATA_XFER);
		return (QR_DONE);
	case SS_WRES_DIND:
		goto error1;
	      error1:
		/* send RLSD */
		sccp_set_state(sc, SS_WCON_DREQ);
		return (QR_DONE);
	case SS_WCON_CREQ:
		return sccp_release_error(sc, q);
	case SS_WCON_CREQ2:
		/* Note that ITUT96 does not use the SLR in the RLC to send a RLSD message as does
		   ANSI. */
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ITUT:
		case SS7_PVAR_ETSI:
			/* Figure C.2/Q.714 Sheet 3 of 7: stop connection timer, release resources
			   and local reference, freeze local reference, move to the idle state. */
			return sccp_release_error(sc, q);
		default:
		case SS7_PVAR_JTTC:
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 3 of 4: same action as for CC in this
			   state: stop connection timer, associate destination reference to
			   connection section, release resources, stop inactivity timers, send
			   RLSD, start released (ANSI: and interval timers), move to the disconnect 
			   pending state. */
			sccp_release(sc);
			sc->dlr = m->slr;
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					    sc->slr, SCCP_RELC_END_USER_ORIGINATED, NULL, NULL)))
				return (err);
			sccp_timer_start(sc, trel);
			sccp_timer_start(sc, tint);	/* ANSI */
			sccp_set_state(sc, SS_WCON_DREQ);
			return (QR_DONE);
		}
	case SS_DATA_XFER:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000: Figure 2B/T1.112.4-2000 2of2: send an
			   N_DISCON_IND to the user, release resources for the connection, stop
			   inactivity timers, send RLSD, start released and interval timers, move
			   to the disconnect pending state. */
			sccp_release(sc);
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					    sc->slr, SCCP_RELC_REMOTE_PROCEDURE_ERROR, NULL, NULL)))
				return (err);
			{
				int cause = 0;

				fixme(("Determine cuase\n"));
				if ((err =
				     sccp_discon_ind(sc, q, N_PROVIDER, cause, NULL, NULL, NULL)))
					return (err);
			}
			sccp_timer_start(sc, trel);
			sccp_timer_start(sc, tint);
			sccp_set_state(sc, SS_WCON_DREQ);
			return (QR_DONE);
		default:
			/* Figure C.3/Q.714 Sheet 3 of 6: discard */
			return (QR_DONE);
		}
	case SS_BOTH_RESET:	/* Figure C.6/Q.714 Sheet 3 of 4 */
	case SS_WCON_RREQ:	/* Figure C.6/Q.714 Sheet 2 of 4 */
	case SS_IDLE:		/* Figure 2A/T1.112.4-2000 1of4: Figure 2B 1of2 */
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
}

/*
 *  SCCP_MT_DT1   0x06 - Data Form 1
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SEG), V(DATA)
 *  ANSI: F(DLR, SEG), V(DATA)
 */
static int
sccp_recv_dt1(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* No optional parameters.  Optional parameters will be ignored. This message is only valid 
	   for protocol class 2.  DLR is used to look up the stream in the NS_DATA_XFER state.  SEG 
	   is passed as the more bit to the user.  This should result in an N_DATA_IND primitive to 
	   the SCCP-User. */
	int err;

	fixme(("Handle sequence numbers and credit\n"));
	switch (sccp_get_state(sc)) {
	case SS_DATA_XFER:
		/* Figure 2C/T1.112.4-2000 1of2: reset receive inactivity timer.  For protocol
		   class 3 if P(R) is in the rage from the last P(R) received up to including the
		   send sequence number of the next message to be transmitted, if P(R) is lower
		   than the window edge, update the lower window edge and perform the transmission
		   wakeup function.  Otherwise, if P(S) is the next in sequence, mark ack
		   outstanding and deliver to the user.  If P(S) is not next in sequence do an
		   internal reset. For protocol class 2 deliver data to the user. */
		/* Figure C.4/Q.714 Sheet 2 of 4: When we receive a DT1 we restart the receive
		   inactivity timer. If the M-bit is set, we can either reassemble before
		   delivering to the user, or we can deliver immediately using the more flag in the 
		   N_DATA_IND.  In either case, once we have a deliverable message we generate an
		   N_DATA_IND to the user. */
		sccp_timer_start(sc, tiar);
		if ((err = sccp_data_ind(sc, q, NULL, 0, m->seg, NULL, msg->b_cont)))
			return (err);
		return (QR_DONE);
	case SS_WRES_RIND:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2E/T1.112.4-2000 2of2: queue received message. */
			bufq_queue(&sc->rcvq, msg);
			return (QR_DONE);
		default:
		case SS7_PVAR_ITUT:
			/* Figure C.6/Q.714 Sheet 4 of 4: discard received message and generate
			   internal reset request, move to the active state. */
			fixme(("Internal reset request\n"));
			sccp_set_state(sc, SS_DATA_XFER);
			return (QR_DONE);
		}
	case SS_WRES_DIND:
		goto error1;
	      error1:
		/* send RLSD */
		sccp_set_state(sc, SS_WCON_DREQ);
		return (QR_DONE);
	case SS_WCON_CREQ:
		return sccp_release_error(sc, q);
	case SS_WCON_CREQ2:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ITUT:
		case SS7_PVAR_ETSI:
			return sccp_release_error(sc, q);
		default:
		case SS7_PVAR_JTTC:
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 3 of 4: discard, continue to wait for
			   connection confirm. */
			return (QR_DONE);
		}
	case SS_BOTH_RESET:	/* Figure C.6/Q.714 Sheet 3 of 4 */
		/* Figure 2E/T1.112.4-2000 Sheet 1 of 2 */
	case SS_WCON_RREQ:	/* Figure C.6/Q.714 Sheet 2 of 4 */
		/* Figure 2E/T1.112.4-2000 2of2 */
	case SS_WCON_DREQ:	/* Figure C.2/Q.714 Sheet 6 of 7 */
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
}

/*
 *  SCCP_MT_DT2   0x07 - Data Form 2
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SEQ), V(DATA)
 *  ANSI: F(DLR, SEQ), V(DATA)
 */
static int
sccp_recv_dt2(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* No optional parameters.  Optional parameters will be ignored. This message is only valid 
	   for protocol class 3.  DLR is used to look up the stream in the NS_DATA_XFER state.  SEQ 
	   is used for sequencing and the more bit is passed to the user.  This should result in a
	   N_DATA_IND primitive to the SCCP-User. */
	int err;

	fixme(("Handle sequence numbers and credit\n"));
	switch (sccp_get_state(sc)) {
	case SS_DATA_XFER:
		/* Figure 2C/T1.112.4-2000 1of2: reset receive inactivity timer.  For protocol
		   class 3 if P(R) is in the rage from the last P(R) received up to including the
		   send sequence number of the next message to be transmitted, if P(R) is lower
		   than the window edge, update the lower window edge and perform the transmission
		   wakeup function.  Otherwise, if P(S) is the next in sequence, mark ack
		   outstanding and deliver to the user.  If P(S) is not next in sequence do an
		   internal reset. For protocol class 2 deliver data to the user. */
		/* Figure C.4/Q.714 Sheet 2 of 4: If the received P(S) is not the next in sequence, 
		   or P(R) is outside the range from the last received P(R) to the last P(S) sent
		   plus 1, the an internal reset (see figure) is peformed.  Otherwise, perform a
		   Transmit Wakeup: ------------------------------------- If the received P(R) is
		   at the lower edge of the sending window, the sending window is set to the value
		   of P(R).  While we have a message in the transmit queue, (we are flow controlled
		   on the sending side) and the P(S) assigned to the message in queue is inside the
		   send window, we set P(R) to the last received P(S) plus 1 and send another of the
		   queued data messages and restart the send inactivity timer.  In this way we clear
		   any backlog of messages once we receive acknowledgement that opens the window.
		   Otherwise, if the received P(S) is equal to the upper edge of the window, or if we 
		   need to send a data ack (acknowledged service) then we construct a data ack.
		   message with credit = W and send it, restarting the send inactivity timer.  After
		   this operation, we deliver the data to the user in an N_DATA_IND.  IMPLEMENTATION
		   NOTE:- We allow the user to explicitly acknowledge receipt of data messages in
		   protocol class 3.  To do this, we queue the message to an ack queue rather than
		   performing the P(S) P(R) processing on it yet.  We postpone this processing until
		   a data ack request is received from the user. */
		if ((err = sccp_data_ind(sc, q, NULL, 0, m->seg, NULL, msg->b_cont)))
			return (err);
		return (QR_DONE);
	case SS_WRES_RIND:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2E/T1.112.4-2000 2of2: queue received message. */
			bufq_queue(&sc->rcvq, msg);
			return (QR_DONE);
		default:
		case SS7_PVAR_ITUT:
			/* Figure C.6/Q.714 Sheet 4 of 4: discard received message and generate
			   internal reset request, move to the active state. */
			fixme(("Internal reset request\n"));
			sccp_set_state(sc, SS_DATA_XFER);
			return (QR_DONE);
		}
	case SS_WRES_DIND:
		goto error1;
	      error1:
		/* send RLSD */
		sccp_set_state(sc, SS_WCON_DREQ);
		return (QR_DONE);
	case SS_WCON_CREQ:
		return sccp_release_error(sc, q);
	case SS_WCON_CREQ2:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ITUT:
		case SS7_PVAR_ETSI:
			return sccp_release_error(sc, q);
		default:
		case SS7_PVAR_JTTC:
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 3 of 4: discard, continue to wait for
			   connection confirm. */
			return (QR_DONE);
		}
	case SS_BOTH_RESET:	/* Figure C.6/Q.714 Sheet 3 of 4 */
		/* Figure 2E/T1.112.4-2000 Sheet 1 of 2 */
	case SS_WCON_RREQ:	/* Figure C.6/Q.714 Sheet 2 of 4 */
		/* Figure 2E/T1.112.4-2000 2of2 */
	case SS_WCON_DREQ:	/* Figure C.2/Q.714 Sheet 6 of 7 */
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
}

/*
 *  SCCP_MT_AK    0x08 - Data Acknowledgement
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, RSN, CRED)
 *  ANSI: F(DLR, RSN, CRED)
 */
static int
sccp_recv_ak(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* No optional parameters.  Optional parameters will be ignored. This message is only valid 
	   for protocol class 3.  DLR is used to look up the stream in the NS_DATA_XFER state.  RSN 
	   is used to acknowledge a specific data segment and CRED is used to update the flow
	   control window.  If RC_OPT_SEL has been set for the stream, this should result in a
	   N_DATACK_IND to the SCCP-User. */
	int err;

	fixme(("Handle sequence numbers\n"));
	switch (sccp_get_state(sc)) {
	case SS_DATA_XFER:
		/* Figure 2C/T1.112.4-2000 1of2: reset receive inactivity timer, update sequence
		   number for next message to be sent and update credit.  While P(R)>lower window
		   edge and message in transmit queue: update window edge, construct message, send
		   data, mark no ack outstanding, reset send inactivity timer.  If P(S) is next in
		   sequence mark ack oustanding and deliver data to user, otherwise do an internal
		   reset. */
		/* Figure C.4/Q.714 Sheet 3 of 4: Restart the receive inactivity timer.  If the
		   received P(R) is not in the correct range, we perform an internal reset.
		   Otherwise, we set the sending cred to the value received in the data ack.
		   Perform a Transmit Wakeup: -------------------------- If the received P(R) is
		   not at the lower edge of the window, we set the lower edge of the sending window 
		   to the valur of the received P(R).  And, if there is a message in the transmit
		   queue and the P(S) of the message is inside the sending window, we set the P(R)
		   of the message to the last received P(S)+1 and send the message, restarting the
		   receive inactivity timer. IMPLEMENTATION NOTES:- Q.714 does not specify an
		   N-DATACK Indication; however, the ISO NSD as provided by NPI does support this
		   (TPI doesn't directly).  Therefore, we deliver a data acknowledgement indication
		   to the user if the interface supports this. */
		sccp_timer_start(sc, tiar);
		if ((err = sccp_datack_ind(sc, q)))
			return (err);
		return (QR_DONE);
	case SS_WRES_RIND:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2E/T1.112.4-2000 2of2: queue received message. */
			bufq_queue(&sc->rcvq, msg);
			return (QR_DONE);
		default:
		case SS7_PVAR_ITUT:
			/* Figure C.6/Q.714 Sheet 4 of 4: discard received message and generate
			   internal reset request, move to the active state. */
			fixme(("Internal reset request\n"));
			sccp_set_state(sc, SS_DATA_XFER);
			return (QR_DONE);
		}
	case SS_WRES_DIND:
		goto error1;
	      error1:
		/* send RLSD */
		sccp_set_state(sc, SS_WCON_DREQ);
		return (QR_DONE);
	case SS_WCON_CREQ:
		return sccp_release_error(sc, q);
	case SS_WCON_CREQ2:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ITUT:
		case SS7_PVAR_ETSI:
			return sccp_release_error(sc, q);
		default:
		case SS7_PVAR_JTTC:
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 3 of 4: discard, continue to wait for
			   connection confirm. */
			return (QR_DONE);
		}
	case SS_BOTH_RESET:	/* Figure C.6/Q.714 Sheet 3 of 4 */
		/* Figure 2E/T1.112.4-2000 Sheet 1 of 2 */
	case SS_WCON_RREQ:	/* Figure C.6/Q.714 Sheet 2 of 4 */
		/* Figure 2E/T1.112.4-2000 2of2 */
	case SS_WCON_DREQ:	/* Figure C.2/Q.714 Sheet 6 of 7 */
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
}

/*
 *  SCCP_MT_ED    0x0b - Expedited Data
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR), V(DATA)
 *  ANSI: F(DLR), V(DATA)
 */
static int
sccp_recv_ed(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* No optional parameters.  Optional parameters will be ignored. This message is only valid 
	   in protocol class 3.  DLR is used to look up the stream in the NS_DATA_XFER state.  This 
	   should result in an N_EXDATA_IND to the SCCP-User. */
	int err;

	fixme(("Handle sequence numbers\n"));
	switch (sccp_get_state(sc)) {
	case SS_DATA_XFER:
		/* Figure 2D/T1.112.4-2000: this is shown a little differently (no internal reset
		   procedure), but we wait for ack from user if necessary.  Figure C.5/Q.714 Sheet
		   1 of 2: Restart the receive inactivity timer.  Deliever an N_DATA_IND for
		   expedited data to the user.  Wait for an acknowledgement from the user. */
		if (sc->flags & SCCPF_REM_ED) {
			/* Figure C.5/Q.714 Sheet 1 of 2: Perform an internal reset and move to the 
			   active state. */
			fixme(("Perform internal reset\n"));
			sc->flags &= ~SCCPF_REM_ED;
			return (-EPROTO);
		}
		if ((err = sccp_data_ind(sc, q, NULL, 1, 0, NULL, msg->b_cont)))
			return (err);
		sc->flags |= SCCPF_REM_ED;
		return (QR_DONE);
	case SS_WRES_RIND:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2E/T1.112.4-2000 2of2: queue received message. */
			bufq_queue(&sc->rcvq, msg);
			return (QR_DONE);
		default:
		case SS7_PVAR_ITUT:
			/* Figure C.6/Q.714 Sheet 4 of 4: discard received message and generate
			   internal reset request, move to the active state. */
			fixme(("Internal reset request\n"));
			sccp_set_state(sc, SS_DATA_XFER);
			return (QR_DONE);
		}
	case SS_WRES_DIND:
		goto error1;
	      error1:
		/* send RLSD */
		sccp_set_state(sc, SS_WCON_DREQ);
		return (QR_DONE);
	case SS_WCON_CREQ:
		return sccp_release_error(sc, q);
	case SS_WCON_CREQ2:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ITUT:
		case SS7_PVAR_ETSI:
			return sccp_release_error(sc, q);
		default:
		case SS7_PVAR_JTTC:
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 3 of 4: discard, continue to wait for
			   connection confirm. */
			return (QR_DONE);
		}
	case SS_BOTH_RESET:	/* Figure C.6/Q.714 Sheet 3 of 4 */
		/* Figure 2E/T1.112.4-2000 Sheet 1 of 2 */
	case SS_WCON_RREQ:	/* Figure C.6/Q.714 Sheet 2 of 4 */
		/* Figure 2E/T1.112.4-2000 2of2 */
	case SS_WCON_DREQ:	/* Figure C.2/Q.714 Sheet 6 of 7 */
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
}

/*
 *  SCCP_MT_EA    0x0c - Expedited Data Acknowledgement
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR)
 *  ANSI: F(DLR)
 */
static int
sccp_recv_ea(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* No optional parameters.  Optional parameters will be ignored. This message is only valid 
	   in protocol class 3.  DLR is used to look up the stream in the NS_DATA_XFER state.  No
	   indication is given to the user.  It will be used internally in protocol class 3 to
	   unlock the normal data flow. */
	int err;

	fixme(("Handle sequence numbers\n"));
	switch (sccp_get_state(sc)) {
	case SS_DATA_XFER:
		/* Figure 2D/T1.112.4-2000: same.  Figure C.5/Q.714 Sheet 1 of 2: Restart the
		   receive inactivity timer.  If there is expedited data in queue, send another
		   expedited data now. IMPLEMENTATION NOTE:- We deliver expedited data
		   acknolwedgements to the User as well. */
		if (!(sc->flags & SCCPF_LOC_ED))
			return (QR_DONE);
		if ((err = sccp_datack_ind(sc, q)))
			return (err);
		sc->flags &= ~SCCPF_LOC_ED;
		return (QR_DONE);
	case SS_WRES_RIND:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2E/T1.112.4-2000 2of2: queue received message. */
			bufq_queue(&sc->rcvq, msg);
			return (QR_DONE);
		default:
		case SS7_PVAR_ITUT:
			/* Figure C.6/Q.714 Sheet 4 of 4: discard received message and generate
			   internal reset request, move to the active state. */
			fixme(("Internal reset request\n"));
			sccp_set_state(sc, SS_DATA_XFER);
			return (QR_DONE);
		}
	case SS_WRES_DIND:
		goto error1;
	      error1:
		/* send RLSD */
		sccp_set_state(sc, SS_WCON_DREQ);
		return (QR_DONE);
	case SS_WCON_CREQ:
		return sccp_release_error(sc, q);
	case SS_WCON_CREQ2:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ITUT:
		case SS7_PVAR_ETSI:
			return sccp_release_error(sc, q);
		default:
		case SS7_PVAR_JTTC:
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 3 of 4: discard, continue to wait for
			   connection confirm. */
			return (QR_DONE);
		}
	case SS_BOTH_RESET:	/* Figure C.6/Q.714 Sheet 3 of 4 */
		/* Figure 2E/T1.112.4-2000 Sheet 1 of 2 */
	case SS_WCON_RREQ:	/* Figure C.6/Q.714 Sheet 2 of 4 */
		/* Figure 2E/T1.112.4-2000 2of2 */
	case SS_WCON_DREQ:	/* Figure C.2/Q.714 Sheet 6 of 7 */
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
}

/*
 *  SCCP_MT_RSR   0x0d - Reset Request
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR, RESC)
 *  ANSI: F(DLR, SLR, RESC)
 */
static int
sccp_recv_rsr(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* No optional parameters.  Optional parameters will be ignored. This message is only valid 
	   in protocol class 3.  DLR is used to look up the stream in the NS_DATA_XFER state.  This 
	   normally results in a N_RESET_IND primtiive to the SCCP-User.  The stream is reset. */
	int err;

	switch (sccp_get_state(sc)) {
	case SS_DATA_XFER:
		/* ANSI is peculiar in that it does not wait for a reset response from the user
		   before sending an RSC. */
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2E/T1.112.4-2000: send an RSC, send an N_RESET_IND to the user,
			   reset variables and discard all queued and unacknowledged messages, move 
			   to the reset incoming state. */
			if ((err =
			     sccp_send_rsc(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					   sc->slr)))
				return (err);
			fixme(("Reset variables and discard queued unacked messages\n"));
			if ((err = sccp_reset_ind(sc, q, N_USER, 0x3000 | m->cause, msg->b_cont)))
				return (err);
			return (QR_DONE);
			sccp_set_state(sc, SS_WRES_RIND);
		default:
		case SS7_PVAR_ITUT:
			/* Figure C.6/Q.714 Sheet 1 of 4: Restart the receive inactivity timer.
			   Deliver an N_RESET_IND to the user, reset variables and discard all
			   queued and unacked messages. Mover the reset incoming state. */
			sccp_timer_start(sc, tiar);
			fixme(("Reset variables and discard queued unacked messages\n"));
			if ((err = sccp_reset_ind(sc, q, N_USER, 0x3000 | m->cause, msg->b_cont)))
				return (err);
			sccp_set_state(sc, SS_WRES_RIND);
			return (QR_DONE);
		}
	case SS_BOTH_RESET:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2E/T1.112.4-2000: stop reset timer and move to the reset incoming 
			   state. */
			sccp_timer_stop(sc, tres);
			sccp_set_state(sc, SS_WRES_RIND);
			return (QR_DONE);
		default:
		case SS7_PVAR_ITUT:
			/* Figure C.6/Q.714 Sheet 3 of 4: send a N_RESET_CON to the user, stop the
			   reset timer, restart the receive inactivity timer, move to the reset
			   incoming state. */
			sccp_timer_stop(sc, tres);
			sccp_timer_start(sc, tiar);
			if ((err = sccp_reset_con(sc, q)))
				return (err);
			sccp_set_state(sc, SS_WRES_RIND);
			return (QR_DONE);
		}
	case SS_WCON_RREQ:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2E/T1.112.4-2000: stop reset timer, transfer all queued
			   information, move to the active state. */
			sccp_timer_stop(sc, tres);
			fixme(("Transfer all queued information\n"));
			sccp_set_state(sc, SS_DATA_XFER);
			return (QR_DONE);
		default:
		case SS7_PVAR_ITUT:
			/* Figure C.6/Q.714 Sheet 2 of 4: send N_RESET_CON, stop reset timer,
			   restart receive inactivity timer, move to the active state. */
			sccp_timer_stop(sc, tres);
			sccp_timer_start(sc, tiar);
			if ((err = sccp_reset_con(sc, q)))
				return (err);
			sccp_set_state(sc, SS_DATA_XFER);
			return (QR_DONE);
		}
	case SS_WRES_DIND:
		goto error1;
	      error1:
		/* send RLSD */
		sccp_set_state(sc, SS_WCON_DREQ);
		return (QR_DONE);
	case SS_WCON_CREQ:
		return sccp_release_error(sc, q);
	case SS_WCON_CREQ2:
		/* Note that ITUT96 does not use the SLR in the RLC to send a RLSD message as does
		   ANSI. */
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ITUT:
		case SS7_PVAR_ETSI:
			/* Figure C.2/Q.714 Sheet 3 of 7: stop connection timer, release resources
			   and local reference, freeze local reference, move to the idle state. */
			return sccp_release_error(sc, q);
		default:
		case SS7_PVAR_JTTC:
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 3 of 4: same action as for CC in this
			   state: stop connection timer, associate destination reference to
			   connection section, release resources, stop inactivity timers, send
			   RLSD, start released (ANSI: and interval timers), move to the disconnect 
			   pending state. */
			sccp_release(sc);
			sc->dlr = m->slr;
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					    sc->slr, SCCP_RELC_END_USER_ORIGINATED, NULL, NULL)))
				return (err);
			sccp_timer_start(sc, trel);
			sccp_timer_start(sc, tint);	/* ANSI */
			sccp_set_state(sc, SS_WCON_DREQ);
			return (QR_DONE);
		}
	case SS_WRES_RIND:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2E/T1.112.4-2000 2of2: send RSC, discard all queue and
			   unacknolwedged messages, */
			if ((err =
			     sccp_send_rsc(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					   sc->slr)))
				return (err);
			fixme(("Discard all queued and unacked messages\n"));
			return (QR_DONE);
		default:
		case SS7_PVAR_ITUT:
			/* Figure C.6/Q.714 Sheet 4 of 4: discard received message and generate
			   internal reset request, move to the active state. */
			fixme(("Internal reset request\n"));
			sccp_set_state(sc, SS_DATA_XFER);
			return (QR_DONE);
		}
	case SS_WCON_DREQ:	/* Figure C.2/Q.714 Sheet 6 of 7 */
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
}

/*
 *  SCCP_MT_RSC   0x0e - Reset Confirm
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR)
 *  ANSI: F(DLR, SLR)
 */
static int
sccp_recv_rsc(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* No optional parameters.  Optional parameters will be ignored. This message is only valid 
	   in protocol class 3.  DLR is used to look up the stream in the NS_DATA_XFER state.  This 
	   normally results in a N_RESET_CON primitive to the SCCP-User.  The stream is confirmed
	   reset. */
	int err;

	switch (sccp_get_state(sc)) {
	case SS_BOTH_RESET:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2E/T1.112.4-2000: stop reset timer and move to the reset incoming 
			   state. */
			sccp_timer_stop(sc, tres);
			sccp_set_state(sc, SS_WRES_RIND);
			return (QR_DONE);
		default:
		case SS7_PVAR_ITUT:
			/* Figure C.6/Q.714 Sheet 3 of 4: Send a N_RESET_CON to the user, stop the
			   reset timer, restart the inactivity timer and move to the reset incoming 
			   state. */
			sccp_timer_stop(sc, tres);
			sccp_timer_start(sc, tiar);
			if ((err = sccp_reset_con(sc, q)))
				return (err);
			sccp_set_state(sc, SS_WRES_CIND);
			return (QR_DONE);
		}
	case SS_WCON_RREQ:
		/* Figure C.6/Q.714 Sheet 2 of 4: Send an N_RESET_CON to the user, stop the reset
		   timer, restart the receive inactivity timer, resume data transfer, move to the
		   active state. */
		sccp_timer_stop(sc, tres);
		sccp_timer_start(sc, tiar);
		if ((err = sccp_reset_con(sc, q)))
			return (err);
		sccp_set_state(sc, SS_DATA_XFER);
		return (QR_DONE);
	case SS_DATA_XFER:
		/* Figure C.6/Q.714 Sheet 1 of 4: Discard received message. */
		return (QR_DONE);
	case SS_WRES_RIND:
		/* Figure C.6/Q.714 Sheet 4 of 4: discard received message and generate internal
		   reset request, move to the active state. */
		fixme(("Internal reset request\n"));
		sccp_set_state(sc, SS_DATA_XFER);
		return (QR_DONE);
	case SS_WRES_DIND:
		goto error1;
	      error1:
		/* send RLSD */
		sccp_set_state(sc, SS_WCON_DREQ);
		return (QR_DONE);
	case SS_WCON_CREQ:
		return sccp_release_error(sc, q);
	case SS_WCON_CREQ2:
		/* Note that ITUT96 does not use the SLR in the RLC to send a RLSD message as does
		   ANSI. */
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ITUT:
		case SS7_PVAR_ETSI:
			/* Figure C.2/Q.714 Sheet 3 of 7: stop connection timer, release resources
			   and local reference, freeze local reference, move to the idle state. */
			return sccp_release_error(sc, q);
		default:
		case SS7_PVAR_JTTC:
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 3 of 4: same action as for CC in this
			   state: stop connection timer, associate destination reference to
			   connection section, release resources, stop inactivity timers, send
			   RLSD, start released (ANSI: and interval timers), move to the disconnect 
			   pending state. */
			sccp_release(sc);
			sc->dlr = m->slr;
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					    sc->slr, SCCP_RELC_END_USER_ORIGINATED, NULL, NULL)))
				return (err);
			sccp_timer_start(sc, trel);
			sccp_timer_start(sc, tint);	/* ANSI */
			sccp_set_state(sc, SS_WCON_DREQ);
			return (QR_DONE);
		}
	case SS_WCON_DREQ:	/* Figure C.2/Q.714 Sheet 6 of 7 */
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
}

/*
 *  SCCP_MT_ERR   0x0f - Protocol Data Unit Error
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, ERRC)
 *  ANSI: F(DLR, ERRC)
 */
static int
sccp_recv_err(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* No optional parameters.  Optional parameters will be ignored. This message is only valid 
	   in protocol class 3.  DLR is used to look up the stream.  This does not result in a
	   primitive to the SCCP-User directly, any one of a number of primitives may be sent
	   depending on the disposition of the error. Management is informed of the error
	   condition. */
	int err;

	switch (sccp_get_state(sc)) {
		/* Figure C.2/Q.714 Sheet 5 of 7: Figure C.3/Q.714 Sheet 4 of 6: if the error is
		   not service class mismatch: release resourcesa and local reference, freeze local 
		   reference, send N_DISCON_IND to user, stop inactivity timers, move to the idle
		   state. */
	case SS_DATA_XFER:
		/* Notes: this is somewhat different between variants here. ANSI always sends RLSD
		   and goes through the released procedure.  ITUT only sends RLSD in the case of
		   service class mismatch, otherwise it simply aborts. */
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000: Figure 2B/T1.112.4-2000 2of2: send an
			   N_DISCON_IND to the user, release resources for the connection, stop
			   inactivity timers, send RLSD, start released and interval timers, move
			   to the disconnect pending state. */
			sccp_release(sc);
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					    sc->slr, SCCP_RELC_REMOTE_PROCEDURE_ERROR, NULL, NULL)))
				return (err);
			{
				int cause = 0;

				fixme(("Determine cuase\n"));
				if ((err =
				     sccp_discon_ind(sc, q, N_PROVIDER, cause, NULL, NULL, NULL)))
					return (err);
			}
			sccp_timer_start(sc, trel);
			sccp_timer_start(sc, tint);
			sccp_set_state(sc, SS_WCON_DREQ);
			return (QR_DONE);
		default:
		case SS7_PVAR_ITUT:
			if (m->cause == (SCCP_ERRC_SERVICE_CLASS_MISMATCH & 0xff)) {
				/* Figure C.2/Q.714 Sheet 4 of 7: send N_DISCON_IND to user, stop
				   inactivity timers, send RLSD, start released timer, move to
				   disconnect pending state. */
				sccp_timer_stop(sc, tias);
				sccp_timer_stop(sc, tiar);
				if ((err =
				     sccp_send_rlsd(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls,
						    sc->dlr, sc->slr,
						    SCCP_RELC_REMOTE_PROCEDURE_ERROR, NULL,
						    &sc->imp)))
					return (err);
				if ((err =
				     sccp_discon_ind(sc, q, N_PROVIDER, m->cause | 0x4000, NULL,
						     NULL, NULL)))
					return (err);
				sccp_timer_start(sc, trel);
				sccp_set_state(sc, SS_WCON_DREQ);
				return (QR_DONE);
			} else {
				/* Figure C.2/Q.714 Sheet 5 of 7: release resources and local
				   reference, deliver N_DISCON_IND to user, stop inactivity timers, 
				   move to idle state. */
				sccp_release(sc);
				if ((err =
				     sccp_discon_ind(sc, q, N_PROVIDER, m->cause | 0x4000, NULL,
						     NULL, NULL)))
					return (err);
				sccp_set_state(sc, SS_IDLE);
				return (QR_DONE);
			}
		}
	case SS_WRES_RIND:
		/* Figure C.6/Q.714 Sheet 4 of 4: Figure C.3/Q.714 Sheet 4 of 6: release resources
		   and local reference, freeze local reference, send N_DISCON_IND to the user, stop 
		   inactivity timers, move to the idle state. */
		return sccp_release_error(sc, q);
	case SS_WCON_CREQ:
		return sccp_release_error(sc, q);
	case SS_WCON_CREQ2:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ITUT:
		case SS7_PVAR_ETSI:
			return sccp_release_error(sc, q);
		default:
		case SS7_PVAR_JTTC:
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 3 of 4: discard, continue to wait for
			   connection confirm. */
			return (QR_DONE);
		}
	case SS_WCON_DREQ:
		/* Figure C.2/Q.714 Sheet 6 of 7: release resources and local reference, freeze
		   local reference, stop release and interval timers, move to the idle state.
		   Figure 2A/T1.112.4-2000: Figure 2B/T1.112.4-2000: Release references for the
		   connection section, stop release and interval timers, move to the idle state. */
		sccp_release(sc);
		sccp_set_state(sc, SS_IDLE);
		return (QR_DONE);
	case SS_WRES_DIND:
		sccp_release(sc);
		sccp_set_state(sc, SS_IDLE);
		return (QR_DONE);
	case SS_BOTH_RESET:
		/* Figure C.6/Q.714 Sheet 3 of 4: same as below. */
	case SS_WCON_RREQ:
		/* Figure C.6/Q.714 Sheet 2 of 4: Figure C.2/Q.714 Sheet 5 of 7: Release resources
		   and local reference, freeze local reference, send an N_DISCON_IND to the user,
		   stop inactivity timers and move to the idle state. */
		return sccp_release_error(sc, q);
		return (QR_DONE);
	default:
		swerr();
		return (-EFAULT);
	}
}

/*
 *  SCCP_MT_IT    0x10 - Inactivity Test
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR, PCLS, SEQ, CRED)
 *  ANSI: F(DLR, SLR, PCLS, SEQ, CRED)
 */
static int
sccp_recv_it(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* No optional parameters.  Optional parameters will be ignored.  DLR is used to look up
	   the stream. This does not result in a primitive to the SCCP-User.  This is a heartbeat
	   between SCCP peers.  It is responded to by the state machine. */
	int err;

	switch (sccp_get_state(sc)) {
	case SS_DATA_XFER:
		/* Figure 2D/T1.112.4-2000: just reset the receive inactivity timer.  No audit or
		   internal reset specified for ANSI. Figure C.4/Q.714 Sheet 4 of 4: If we are
		   protocol class 3 we audit the received P(S), P(R), M and W.  If these are ok, or 
		   we are protocol class 2, we reset the receive inactivity timer.  If these are
		   not ok, we perform an internal reset. */
		fixme(("Write this function\n"));
		return (-EFAULT);
	case SS_BOTH_RESET:	/* Figure C.6/Q.714 Sheet 3 of 4 */
		return (QR_DONE);
	case SS_WCON_CREQ2:
		/* Note that ITUT96 does not use the SLR in the RLC to send a RLSD message as does
		   ANSI. */
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ITUT:
		case SS7_PVAR_ETSI:
			/* Figure C.2/Q.714 Sheet 3 of 7: stop connection timer, release resources
			   and local reference, freeze local reference, move to the idle state. */
			return sccp_release_error(sc, q);
		default:
		case SS7_PVAR_JTTC:
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 3 of 4: same action as for CC in this
			   state: stop connection timer, associate destination reference to
			   connection section, release resources, stop inactivity timers, send
			   RLSD, start released (ANSI: and interval timers), move to the disconnect 
			   pending state. */
			sccp_release(sc);
			sc->dlr = m->slr;
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, bp, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					    sc->slr, SCCP_RELC_REMOTE_PROCEDURE_ERROR, NULL, NULL)))
				return (err);
			sccp_timer_start(sc, trel);
			sccp_timer_start(sc, tint);	/* ANSI */
			sccp_set_state(sc, SS_WCON_DREQ);
			return (QR_DONE);
		}
	default:
		swerr();
		return (-EFAULT);
	}
}

/*
 *  SCCP_MT_UDT   0x09 - Unitdata
 *  -------------------------------------------------------------------------
 *  ITUT: F(PCLS), V(CDPA, CGPA, DATA[2-254])
 *  ANSI: F(PCLS), V(CDPA, CGPA, DATA[2-252])
 */
static int
sccp_recv_udt(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* No optional parameters.  Optional parameters will be ignored. This message is only valid 
	   in protocol classes 0 and 1.  CDPA and the DPC in the message is used to look up a
	   connection-less stream in the NS_IDLE state.  This should result in a N_UNITDATA_IND to
	   the SCCP-User. */
	int err;

	fixme(("Perform segmentation\n"));
	switch (sccp_get_state(sc)) {
	case SS_IDLE:
		if ((err =
		     sccp_unitdata_ind(sc, q, &m->cgpa, &m->cdpa, m->pcl, m->ret, m->imp, m->rl.sls,
				       m->rl.mp, msg->b_cont)))
			return (err);
		return (QR_DONE);
	case SS_WCON_CREQ:
		return sccp_release_error(sc, q);
	case SS_WCON_CREQ2:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ITUT:
		case SS7_PVAR_ETSI:
			return sccp_release_error(sc, q);
		default:
		case SS7_PVAR_JTTC:
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 3 of 4: discard, continue to wait for
			   connection confirm. */
			return (QR_DONE);
		}
	case SS_WRES_RIND:
		/* Figure C.6/Q.714 Sheet 4 of 4: discard received message and generate internal
		   reset request, move to the active state. */
		fixme(("Internal reset request\n"));
		sccp_set_state(sc, SS_DATA_XFER);
		return (QR_DONE);
	case SS_DATA_XFER:
	case SS_BOTH_RESET:	/* Figure C.6/Q.714 Sheet 3 of 4 */
	case SS_WCON_RREQ:	/* Figure C.6/Q.714 Sheet 2 of 4 */
	case SS_WRES_DIND:
	case SS_WCON_DREQ:	/* Figure C.2/Q.714 Sheet 6 of 7 */
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
}

/*
 *  SCCP_MT_UDTS  0x0a - Unitdata Service
 *  -------------------------------------------------------------------------
 *  ITUT: F(RETC), V(CDPA, CGPA, DATA[2-253])
 *  ANSI: F(RETC), V(CDPA, CGPA, DATA[2-251])
 */
static int
sccp_recv_udts(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* No optional parameters.  Optional parameters will be ignored. This message is only valid 
	   in protocol classes 0 and 1.  CGPA and DPC in the message are used to look up a
	   connection-less stream in the NS_IDLE state.  This should result in a N_UDERROR_IND to
	   the SCCP-User. */
	int err;

	switch (sccp_get_state(sc)) {
	case SS_IDLE:
		if ((err =
		     sccp_notice_ind(sc, q, 0x2000 | m->cause, &m->cgpa, &m->cdpa, m->pcl, m->ret,
				     m->imp, m->rl.sls, m->rl.mp, msg->b_cont)))
			return (err);
		return (QR_DONE);
	case SS_WRES_RIND:
		/* Figure C.6/Q.714 Sheet 4 of 4: discard received message and generate internal
		   reset request, move to the active state. */
		fixme(("Internal reset request\n"));
		sccp_set_state(sc, SS_DATA_XFER);
		return (QR_DONE);
	case SS_WCON_CREQ:
		return sccp_release_error(sc, q);
	case SS_WCON_CREQ2:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ITUT:
		case SS7_PVAR_ETSI:
			return sccp_release_error(sc, q);
		default:
		case SS7_PVAR_JTTC:
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 3 of 4: discard, continue to wait for
			   connection confirm. */
			return (QR_DONE);
		}
	case SS_DATA_XFER:
	case SS_BOTH_RESET:	/* Figure C.6/Q.714 Sheet 3 of 4 */
	case SS_WCON_RREQ:	/* Figure C.6/Q.714 Sheet 2 of 4 */
	case SS_WRES_DIND:
	case SS_WCON_DREQ:	/* Figure C.2/Q.714 Sheet 6 of 7 */
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
}

/*
 *  SCCP_MT_XUDT  0x11 - Extended Unitdata
 *  -------------------------------------------------------------------------
 *  ITUT: F(PCLS, HOPC), V(CDPA, CGPA, DATA[2-253]), O(SGMT, IMP, EOP)
 *  ANSI: F(PCLS, HOPC), V(CDPA, CGPA, DATA[2-251]), O(SGMT, ISNI, INS, MTI, EOP)
 */
static int
sccp_recv_xudt(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* PCLS is passed to the user.  HOPC is only used by a relay node. CDPA and CGPA and DATA
	   are passed to the user.  SGMT is only used when segmentation of the XUDT message has
	   occured.  Segmented XUDT messages will be reassembled before passing to the user.  ISNI, 
	   INS and MTI are for special network interworking and are only used at relay points.  IMP 
	   is used for congestion control.  This message is only valid in protocol classes 0 and 1. 
	   If SGMT is present, the message must be protocol class 1 (with the original protocol
	   class from the SGMT parameter passed to the user).  CDPA was already used to look up
	   this CLNS stream in the NS_IDLE state.  After reassembly addresses and data are sent to
	   the user. This normally results in a N_UNITDATA_IND primitive to the SCCP-User. */
	int err;

	fixme(("Perform segmentation\n"));
	switch (sccp_get_state(sc)) {
	case SS_IDLE:
		if ((err =
		     sccp_unitdata_ind(sc, q, &m->cgpa, &m->cdpa, m->pcl, m->ret, m->imp, m->rl.sls,
				       m->rl.mp, msg->b_cont)))
			return (err);
		return (QR_DONE);
	case SS_WCON_CREQ:
		return sccp_release_error(sc, q);
	case SS_WCON_CREQ2:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ITUT:
		case SS7_PVAR_ETSI:
			return sccp_release_error(sc, q);
		default:
		case SS7_PVAR_JTTC:
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 3 of 4: discard, continue to wait for
			   connection confirm. */
			return (QR_DONE);
		}
	case SS_WRES_RIND:
		/* Figure C.6/Q.714 Sheet 4 of 4: discard received message and generate internal
		   reset request, move to the active state. */
		fixme(("Internal reset request\n"));
		sccp_set_state(sc, SS_DATA_XFER);
		return (QR_DONE);
	case SS_DATA_XFER:
	case SS_BOTH_RESET:	/* Figure C.6/Q.714 Sheet 3 of 4 */
	case SS_WCON_RREQ:	/* Figure C.6/Q.714 Sheet 2 of 4 */
	case SS_WRES_DIND:
	case SS_WCON_DREQ:	/* Figure C.2/Q.714 Sheet 6 of 7 */
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
}

/*
 *  SCCP_MT_XUDTS 0x12 - Extended Unitdata Service
 *  -------------------------------------------------------------------------
 *  ITUT: F(RETC, HOPC), V(CDPA, CGPA, DATA[2-253]), O(SGMT, IMP, EOP)
 *  ANSI: F(RETC, HOPC), V(CDPA, CGPA, DATA[2-251]), O(SGMT, INS, MTI, ISNI, EOP)
 */
static int
sccp_recv_xudts(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* Same as XUDT but this is passed as a N_UDERROR_IND. */
	int err;

	switch (sccp_get_state(sc)) {
	case SS_IDLE:
		if ((err =
		     sccp_notice_ind(sc, q, 0x2000 | m->cause, &m->cgpa, &m->cdpa, m->pcl, m->ret,
				     m->imp, m->rl.sls, m->rl.mp, msg->b_cont)))
			return (err);
		return (QR_DONE);
	case SS_WCON_CREQ:
		return sccp_release_error(sc, q);
	case SS_WCON_CREQ2:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ITUT:
		case SS7_PVAR_ETSI:
			return sccp_release_error(sc, q);
		default:
		case SS7_PVAR_JTTC:
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 3 of 4: discard, continue to wait for
			   connection confirm. */
			return (QR_DONE);
		}
	case SS_WRES_RIND:
		/* Figure C.6/Q.714 Sheet 4 of 4: discard received message and generate internal
		   reset request, move to the active state. */
		fixme(("Internal reset request\n"));
		sccp_set_state(sc, SS_DATA_XFER);
		return (QR_DONE);
	case SS_DATA_XFER:
	case SS_BOTH_RESET:	/* Figure C.6/Q.714 Sheet 3 of 4 */
	case SS_WCON_RREQ:	/* Figure C.6/Q.714 Sheet 2 of 4 */
	case SS_WRES_DIND:
	case SS_WCON_DREQ:	/* Figure C.2/Q.714 Sheet 6 of 7 */
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
}

/*
 *  SCCP_MT_LUDT  0x13 - Long Unitdata
 *  -------------------------------------------------------------------------
 *  ITUT: F(PCLS, HOPC), V(CDPA, CGPA, LDATA[3-3906]), O(SGMT, IMP, EOP)
 *  ANSI: F(PCLS, HOPC), V(CDPA, CGPA, LDATA[3-3904]), O(SGMT, ISNI, INS, EOP)
 */
static int
sccp_recv_ludt(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* Same as XUDT but longer data and no MTI. */
	int err;

	fixme(("Perform segmentation\n"));
	switch (sccp_get_state(sc)) {
	case SS_IDLE:
		if ((err =
		     sccp_unitdata_ind(sc, q, &m->cgpa, &m->cdpa, m->pcl, m->ret, m->imp, m->rl.sls,
				       m->rl.mp, msg->b_cont)))
			return (err);
		return (QR_DONE);
	case SS_WCON_CREQ:
		swerr();
		return sccp_release_error(sc, q);
	case SS_WCON_CREQ2:
		swerr();
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ITUT:
		case SS7_PVAR_ETSI:
			return sccp_release_error(sc, q);
		default:
		case SS7_PVAR_JTTC:
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 3 of 4: discard, continue to wait for
			   connection confirm. */
			return (QR_DONE);
		}
	case SS_WRES_RIND:
		swerr();
		/* Figure C.6/Q.714 Sheet 4 of 4: discard received message and generate internal
		   reset request, move to the active state. */
		fixme(("Internal reset request\n"));
		sccp_set_state(sc, SS_DATA_XFER);
		return (QR_DONE);
	case SS_DATA_XFER:
	case SS_BOTH_RESET:	/* Figure C.6/Q.714 Sheet 3 of 4 */
	case SS_WCON_RREQ:	/* Figure C.6/Q.714 Sheet 2 of 4 */
	case SS_WRES_DIND:
	case SS_WCON_DREQ:	/* Figure C.2/Q.714 Sheet 6 of 7 */
		swerr();
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
}

/*
 *  SCCP_MT_LUDTS 0x14 - Long Unitdata Service
 *  -------------------------------------------------------------------------
 *  ITUT: F(RETC, HOPC), V(CDPA, CGPA, LDATA[3-3906]), O(SGMT, IMP, EOP)
 *  ANSI: F(RETC, HOPC), V(CDPA, CGPA, LDATA[3-3904]), O(SGMT, ISNI, INS, EOP)
 */
static int
sccp_recv_ludts(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *msg, struct sccp_msg *m)
{
	/* Same as XUDTS but longer data and no MTI. */
	int err;

	switch (sccp_get_state(sc)) {
	case SS_IDLE:
		if ((err =
		     sccp_notice_ind(sc, q, 0x2000 | m->cause, &m->cgpa, &m->cdpa, m->pcl, m->ret,
				     m->imp, m->rl.sls, m->rl.mp, msg->b_cont)))
			return (err);
		return (QR_DONE);
	case SS_WCON_CREQ:
		return sccp_release_error(sc, q);
	case SS_WCON_CREQ2:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ITUT:
		case SS7_PVAR_ETSI:
			return sccp_release_error(sc, q);
		default:
		case SS7_PVAR_JTTC:
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 3 of 4: discard, continue to wait for
			   connection confirm. */
			return (QR_DONE);
		}
	case SS_WRES_RIND:
		/* Figure C.6/Q.714 Sheet 4 of 4: discard received message and generate internal
		   reset request, move to the active state. */
		fixme(("Internal reset request\n"));
		sccp_set_state(sc, SS_DATA_XFER);
		return (QR_DONE);
	case SS_DATA_XFER:
	case SS_BOTH_RESET:	/* Figure C.6/Q.714 Sheet 3 of 4 */
	case SS_WCON_RREQ:	/* Figure C.6/Q.714 Sheet 2 of 4 */
	case SS_WRES_DIND:
	case SS_WCON_DREQ:	/* Figure C.2/Q.714 Sheet 6 of 7 */
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
}

/*
 *  SCMG_MT_SSA
 *  -------------------------------------------------------------------------
 */
static int
scmg_recv_ssa(struct sp *sp, queue_t *q, mblk_t *bp, struct sr *sr, struct rs *rs, mblk_t *mp,
	      struct sccp_msg *m)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}

/*
 *  SCMG_MT_SSP
 *  -------------------------------------------------------------------------
 */
static int
scmg_recv_ssp(struct sp *sp, queue_t *q, mblk_t *bp, struct sr *sr, struct rs *rs, mblk_t *mp,
	      struct sccp_msg *m)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}

/*
 *  SCMG_MT_SST
 *  -------------------------------------------------------------------------
 */
static int
scmg_recv_sst(struct sp *sp, queue_t *q, mblk_t *bp, struct sr *sr, struct rs *rs, mblk_t *mp,
	      struct sccp_msg *m)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}

/*
 *  SCMG_MT_SOR
 *  -------------------------------------------------------------------------
 */
static int
scmg_recv_sor(struct sp *sp, queue_t *q, mblk_t *bp, struct sr *sr, struct rs *rs, mblk_t *mp,
	      struct sccp_msg *m)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}

/*
 *  SCMG_MT_SOG
 *  -------------------------------------------------------------------------
 */
static int
scmg_recv_sog(struct sp *sp, queue_t *q, mblk_t *bp, struct sr *sr, struct rs *rs, mblk_t *mp,
	      struct sccp_msg *m)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}

/*
 *  SCMG_MT_SSC
 *  -------------------------------------------------------------------------
 */
static int
scmg_recv_ssc(struct sp *sp, queue_t *q, mblk_t *bp, struct sr *sr, struct rs *rs, mblk_t *mp,
	      struct sccp_msg *m)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}

/*
 *  SCMG_MT_SBR
 *  -------------------------------------------------------------------------
 */
static int
scmg_recv_sbr(struct sp *sp, queue_t *q, mblk_t *bp, struct sr *sr, struct rs *rs, mblk_t *mp,
	      struct sccp_msg *m)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}

/*
 *  SCMG_MT_SNR
 *  -------------------------------------------------------------------------
 */
static int
scmg_recv_snr(struct sp *sp, queue_t *q, mblk_t *bp, struct sr *sr, struct rs *rs, mblk_t *mp,
	      struct sccp_msg *m)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}

/*
 *  SCMG_MT_SRT
 *  -------------------------------------------------------------------------
 */
static int
scmg_recv_srt(struct sp *sp, queue_t *q, mblk_t *bp, struct sr *sr, struct rs *rs, mblk_t *mp,
	      struct sccp_msg *m)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}

/*
 *  -------------------------------------------------------------------------
 *
 *  SCCP Decode Message Functions
 *
 *  -------------------------------------------------------------------------
 */
/*
 *  SCCP_MT_CR    0x01 - Connection Request
 *  -------------------------------------------------------------------------
 *  ITUT: F(SLR, PCLS), V(CDPA), O(CRED, CGPA, DATA, HOPC, IMP, EOP)
 *  ASNI: F(SLR, PCLS), V(CDPA), O(CRED, CGPA, DATA, HOPC, EOP)
 */
static int
sccp_dec_cr(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;
	uchar *pp, *ep;

	m->type = SCCP_MT_CR;
	m->parms = 0;
	if ((rtn = unpack_slr(p, e, m)) < 0)
		return (rtn);	/* SLR F */
	p += rtn;
	if ((rtn = unpack_pcls(p, e, m)) < 0)
		return (rtn);	/* PCLS F */
	p += rtn;
	if (m->pcl != 2 && m->pcl != 3)
		return (-EPROTO);
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_cdpa(p, e, m)) < 0)
		return (rtn);	/* CDPA V */
	if (*p)
		sccp_dec_opt(p + *p, e, m);
	if (!(m->parms & SCCP_PTF_IMP))
		m->imp = SCCP_DI_CR;
	return (0);
}

/*
 *  SCCP_MT_CC    0x02 - Connection Confirm
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR, PCLS), O(CRED, CDPA, DATA, IMP, EOP)
 *  ANSI: F(DLR, SLR, PCLS), O(CRED, CDPA, DATA, EOP)
 */
static int
sccp_dec_cc(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;

	m->type = SCCP_MT_CC;
	m->parms = 0;
	if ((rtn = unpack_dlr(p, e, m)) < 0)
		return (rtn);	/* DLR F */
	p += rtn;
	if ((rtn = unpack_slr(p, e, m)) < 0)
		return (rtn);	/* SLR F */
	p += rtn;
	if ((rtn = unpack_pcls(p, e, m)) < 0)
		return (rtn);	/* PCLS F */
	p += rtn;
	if (m->pcl != 2 && m->pcl != 3)
		return (-EPROTO);
	if (*p)
		sccp_dec_opt(p + *p, e, m);
	if (!(m->parms & SCCP_PTF_IMP))
		m->imp = SCCP_DI_CC;
	return (0);
}

/*
 *  SCCP_MT_CREF  0x03 - Connection Refused
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, REFC), O(CDPA, DATA, IMP, EOP)
 *  ANSI: F(DLR, REFC), O(CDPA, DATA, EOP)
 */
static int
sccp_dec_cref(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;

	m->type = SCCP_MT_CREF;
	m->parms = 0;
	if ((rtn = unpack_dlr(p, e, m)) < 0)
		return (rtn);	/* DLR F */
	p += rtn;
	if ((rtn = unpack_refc(p, e, m)) < 0)
		return (rtn);	/* REFC F */
	p += rtn;
	if (*p)
		sccp_dec_opt(p + *p, e, m);
	if (!(m->parms & SCCP_PTF_IMP))
		m->imp = SCCP_DI_CREF;
	return (0);
}

/*
 *  SCCP_MT_RLSD  0x04 - Released
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR, RELC), O(DATA, IMP, EOP)
 *  ANSI: F(DLR, SLR, RELC), O(DATA, EOP)
 */
static int
sccp_dec_rlsd(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;

	m->type = SCCP_MT_RLSD;
	m->parms = 0;
	if ((rtn = unpack_dlr(p, e, m)) < 0)
		return (rtn);	/* DLR F */
	p += rtn;
	if ((rtn = unpack_slr(p, e, m)) < 0)
		return (rtn);	/* SLR F */
	p += rtn;
	if ((rtn = unpack_relc(p, e, m)) < 0)
		return (rtn);	/* RELC F */
	p += rtn;
	if (*p)
		sccp_dec_opt(p + *p, e, m);
	if (!(m->parms & SCCP_PTF_IMP))
		m->imp = SCCP_DI_RLSD;
	return (0);
}

/*
 *  SCCP_MT_RLC   0x05 - Release Complete
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR)
 *  ANSI: F(DLR, SLR)
 */
static int
sccp_dec_rlc(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;

	m->type = SCCP_MT_RLC;
	m->parms = 0;
	if ((rtn = unpack_dlr(p, e, m)) < 0)
		return (rtn);	/* DLR F */
	p += rtn;
	if ((rtn = unpack_slr(p, e, m)) < 0)
		return (rtn);	/* SLR F */
	p += rtn;
	m->imp = SCCP_DI_RLC;
	return (0);
}

/*
 *  SCCP_MT_DT1   0x06 - Data Form 1
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SEG), V(DATA)
 *  ANSI: F(DLR, SEG), V(DATA)
 */
static int
sccp_dec_dt1(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;
	uchar *pp, *ep;

	m->type = SCCP_MT_DT1;
	m->parms = 0;
	if ((rtn = unpack_dlr(p, e, m)) < 0)
		return (rtn);	/* DLR F */
	p += rtn;
	if ((rtn = unpack_seg(p, e, m)) < 0)
		return (rtn);	/* SEG F */
	p += rtn;
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_data(pp, ep, m)) < 0)
		return (rtn);	/* DATA V */
	m->imp = SCCP_DI_DT1;
	return (0);
}

/*
 *  SCCP_MT_DT2   0x07 - Data Form 2
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SEQ), V(DATA)
 *  ANSI: F(DLR, SEQ), V(DATA)
 */
static int
sccp_dec_dt2(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;
	uchar *pp, *ep;

	m->type = SCCP_MT_DT2;
	m->parms = 0;
	if ((rtn = unpack_dlr(p, e, m)) < 0)
		return (rtn);	/* DLR F */
	p += rtn;
	if ((rtn = unpack_seq(p, e, m)) < 0)
		return (rtn);	/* SEQ F */
	p += rtn;
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_data(pp, ep, m)) < 0)
		return (rtn);	/* DATA V */
	m->imp = SCCP_DI_DT2;
	return (0);
}

/*
 *  SCCP_MT_AK    0x08 - Data Acknowledgement
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, RSN, CRED)
 *  ANSI: F(DLR, RSN, CRED)
 */
static int
sccp_dec_ak(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;

	m->type = SCCP_MT_AK;
	m->parms = 0;
	if ((rtn = unpack_dlr(p, e, m)) < 0)
		return (rtn);	/* DLR F */
	p += rtn;
	if ((rtn = unpack_rsn(p, e, m)) < 0)
		return (rtn);	/* RSN F */
	p += rtn;
	if ((rtn = unpack_cred(p, e, m)) < 0)
		return (rtn);	/* CRED F */
	p += rtn;
	m->imp = SCCP_DI_AK;
	return (0);
}

/*
 *  SCCP_MT_ED    0x0b - Expedited Data
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR), V(DATA)
 *  ANSI: F(DLR), V(DATA)
 */
static int
sccp_dec_ed(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;
	uchar *pp, *ep;

	m->type = SCCP_MT_ED;
	m->parms = 0;
	if ((rtn = unpack_dlr(p, e, m)) < 0)
		return (rtn);	/* DLR F */
	p += rtn;
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_data(pp, ep, m)) < 0)
		return (rtn);	/* DATA V */
	m->imp = SCCP_DI_ED;
	return (0);
}

/*
 *  SCCP_MT_EA    0x0c - Expedited Data Acknowledgement
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR)
 *  ANSI: F(DLR)
 */
static int
sccp_dec_ea(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;

	m->type = SCCP_MT_EA;
	m->parms = 0;
	if ((rtn = unpack_dlr(p, e, m)) < 0)
		return (rtn);	/* DLR F */
	p += rtn;
	m->imp = SCCP_DI_EA;
	return (0);
}

/*
 *  SCCP_MT_RSR   0x0d - Reset Request
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR, RESC)
 *  ANSI: F(DLR, SLR, RESC)
 */
static int
sccp_dec_rsr(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;

	m->type = SCCP_MT_RSR;
	m->parms = 0;
	if ((rtn = unpack_dlr(p, e, m)) < 0)
		return (rtn);	/* DLR F */
	p += rtn;
	if ((rtn = unpack_slr(p, e, m)) < 0)
		return (rtn);	/* SLR F */
	p += rtn;
	if ((rtn = unpack_resc(p, e, m)) < 0)
		return (rtn);	/* RESC F */
	p += rtn;
	m->imp = SCCP_DI_RSR;
	return (0);
}

/*
 *  SCCP_MT_RSC   0x0e - Reset Confirm
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR)
 *  ANSI: F(DLR, SLR)
 */
static int
sccp_dec_rsc(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;

	m->type = SCCP_MT_RSC;
	m->parms = 0;
	if ((rtn = unpack_dlr(p, e, m)) < 0)
		return (rtn);	/* DLR F */
	p += rtn;
	if ((rtn = unpack_slr(p, e, m)) < 0)
		return (rtn);	/* SLR F */
	p += rtn;
	m->imp = SCCP_DI_RSC;
	return (0);
}

/*
 *  SCCP_MT_ERR   0x0f - Protocol Data Unit Error
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, ERRC)
 *  ANSI: F(DLR, ERRC)
 */
static int
sccp_dec_err(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;

	m->type = SCCP_MT_ERR;
	m->parms = 0;
	if ((rtn = unpack_dlr(p, e, m)) < 0)
		return (rtn);	/* DLR F */
	p += rtn;
	if ((rtn = unpack_errc(p, e, m)) < 0)
		return (rtn);	/* ERRC F */
	p += rtn;
	m->imp = SCCP_DI_ERR;
	return (0);
}

/*
 *  SCCP_MT_IT    0x10 - Inactivity Test
 *  -------------------------------------------------------------------------
 *  ITUT: F(DLR, SLR, PCLS, SEQ, CRED)
 *  ANSI: F(DLR, SLR, PCLS, SEQ, CRED)
 */
static int
sccp_dec_it(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;

	m->type = SCCP_MT_IT;
	m->parms = 0;
	if ((rtn = unpack_dlr(p, e, m)) < 0)
		return (rtn);	/* DLR F */
	p += rtn;
	if ((rtn = unpack_slr(p, e, m)) < 0)
		return (rtn);	/* SLR F */
	p += rtn;
	if ((rtn = unpack_pcls(p, e, m)) < 0)
		return (rtn);	/* PCLS F */
	p += rtn;
	if (m->pcl != 2 && m->pcl != 3)
		return (-EPROTO);
	if ((rtn = unpack_seq(p, e, m)) < 0)
		return (rtn);	/* SEQ F */
	p += rtn;
	if ((rtn = unpack_cred(p, e, m)) < 0)
		return (rtn);	/* CRED F */
	p += rtn;
	m->imp = SCCP_DI_IT;
	return (0);
}

/*
 *  SCCP_MT_UDT   0x09 - Unitdata
 *  -------------------------------------------------------------------------
 *  ITUT: F(PCLS), V(CDPA, CGPA, DATA[2-254])
 *  ANSI: F(PCLS), V(CDPA, CGPA, DATA[2-252])
 */
static int
sccp_dec_udt(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;
	uchar *pp, *ep;

	m->type = SCCP_MT_UDT;
	m->parms = 0;
	if ((rtn = unpack_pcls(p, e, m)) < 0)
		return (rtn);	/* PCLS F */
	p += rtn;
	if (m->pcl != 0 && m->pcl != 1)
		return (-EPROTO);
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_cdpa(p, e, m)) < 0)
		return (rtn);	/* CDPA V */
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_cgpa(p, e, m)) < 0)
		return (rtn);	/* CGPA V */
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_data(pp, ep, m)) < 0)
		return (rtn);	/* DATA V */
	m->imp = SCCP_DI_UDT;
	return (0);
}

/*
 *  SCCP_MT_UDTS  0x0a - Unitdata Service
 *  -------------------------------------------------------------------------
 *  ITUT: F(RETC), V(CDPA, CGPA, DATA[2-253])
 *  ANSI: F(RETC), V(CDPA, CGPA, DATA[2-251])
 */
static int
sccp_dec_udts(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;
	uchar *pp, *ep;

	m->type = SCCP_MT_UDTS;
	m->parms = 0;
	if ((rtn = unpack_retc(p, e, m)) < 0)
		return (rtn);	/* RETC F */
	p += rtn;
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_cdpa(p, e, m)) < 0)
		return (rtn);	/* CDPA V */
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_cgpa(p, e, m)) < 0)
		return (rtn);	/* CGPA V */
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_data(pp, ep, m)) < 0)
		return (rtn);	/* DATA V */
	m->imp = SCCP_DI_UDTS;
	return (0);
}

/*
 *  SCCP_MT_XUDT  0x11 - Extended Unitdata
 *  -------------------------------------------------------------------------
 *  ITUT: F(PCLS, HOPC), V(CDPA, CGPA, DATA[2-253]), O(SGMT, IMP, EOP)
 *  ANSI: F(PCLS, HOPC), V(CDPA, CGPA, DATA[2-251]), O(SGMT, ISNI, INS, MTI, EOP)
 */
static int
sccp_dec_xudt(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;
	uchar *pp, *ep;

	m->type = SCCP_MT_XUDT;
	m->parms = 0;
	if ((rtn = unpack_pcls(p, e, m)) < 0)
		return (rtn);	/* PCLS F */
	p += rtn;
	if (m->pcl != 0 && m->pcl != 1)
		return (-EPROTO);
	if ((rtn = unpack_hopc(p, e, m)) < 0)
		return (rtn);	/* HOPC F */
	p += rtn;
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_cdpa(p, e, m)) < 0)
		return (rtn);	/* CDPA V */
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_cgpa(p, e, m)) < 0)
		return (rtn);	/* CGPA V */
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_data(pp, ep, m)) < 0)
		return (rtn);	/* DATA V */
	if (*p)
		sccp_dec_opt(p + *p, e, m);
	if (!(m->parms & SCCP_PTF_IMP))
		m->imp = SCCP_DI_XUDT;
	return (0);
}

/*
 *  SCCP_MT_XUDTS 0x12 - Extended Unitdata Service
 *  -------------------------------------------------------------------------
 *  ITUT: F(RETC, HOPC), V(CDPA, CGPA, DATA[2-253]), O(SGMT, IMP, EOP)
 *  ANSI: F(RETC, HOPC), V(CDPA, CGPA, DATA[2-251]), O(SGMT, INS, MTI, ISNI, EOP)
 */
static int
sccp_dec_xudts(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;
	uchar *pp, *ep;

	m->type = SCCP_MT_XUDTS;
	m->parms = 0;
	if ((rtn = unpack_retc(p, e, m)) < 0)
		return (rtn);	/* RETC F */
	p += rtn;
	if ((rtn = unpack_hopc(p, e, m)) < 0)
		return (rtn);	/* HOPC F */
	p += rtn;
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_cdpa(p, e, m)) < 0)
		return (rtn);	/* CDPA V */
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_cgpa(p, e, m)) < 0)
		return (rtn);	/* CGPA V */
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_data(pp, ep, m)) < 0)
		return (rtn);	/* DATA V */
	if (*p)
		sccp_dec_opt(p + *p, e, m);
	if (!(m->parms & SCCP_PTF_IMP))
		m->imp = SCCP_DI_XUDTS;
	return (0);
}

/*
 *  SCCP_MT_LUDT  0x13 - Long Unitdata
 *  -------------------------------------------------------------------------
 *  ITUT: F(PCLS, HOPC), V(CDPA, CGPA, LDATA[3-3906]), O(SGMT, IMP, EOP)
 *  ANSI: F(PCLS, HOPC), V(CDPA, CGPA, LDATA[3-3904]), O(SGMT, ISNI, INS, EOP)
 */
static int
sccp_dec_ludt(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;
	uchar *pp, *ep;

	m->type = SCCP_MT_LUDT;
	m->parms = 0;
	if ((rtn = unpack_pcls(p, e, m)) < 0)
		return (rtn);	/* PCLS F */
	p += rtn;
	if (m->pcl != 0 && m->pcl != 1)
		return (-EPROTO);
	if ((rtn = unpack_hopc(p, e, m)) < 0)
		return (rtn);	/* HOPC F */
	p += rtn;
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_cdpa(p, e, m)) < 0)
		return (rtn);	/* CDPA V */
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_cgpa(p, e, m)) < 0)
		return (rtn);	/* CGPA V */
	if ((pp = p + *p++) + 1 > e)
		return (-EMSGSIZE);
	if ((ep = pp + ((*pp++) | ((*p++) << 8))) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_ldata(pp, ep, m)) < 0)
		return (rtn);	/* LDATA V */
	if (*p)
		sccp_dec_opt(p + *p, e, m);
	if (!(m->parms & SCCP_PTF_IMP))
		m->imp = SCCP_DI_LUDT;
	return (0);
}

/*
 *  SCCP_MT_LUDTS 0x14 - Long Unitdata Service
 *  -------------------------------------------------------------------------
 *  ITUT: F(RETC, HOPC), V(CDPA, CGPA, LDATA[3-3906]), O(SGMT, IMP, EOP)
 *  ANSI: F(RETC, HOPC), V(CDPA, CGPA, LDATA[3-3904]), O(SGMT, ISNI, INS, EOP)
 */
static int
sccp_dec_ludts(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;
	uchar *pp, *ep;

	m->type = SCCP_MT_LUDTS;
	m->parms = 0;
	if ((rtn = unpack_retc(p, e, m)) < 0)
		return (rtn);	/* RETC F */
	p += rtn;
	if ((rtn = unpack_hopc(p, e, m)) < 0)
		return (rtn);	/* HOPC F */
	p += rtn;
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_cdpa(p, e, m)) < 0)
		return (rtn);	/* CDPA V */
	if ((pp = p + *p++) > e)
		return (-EMSGSIZE);
	if ((ep = pp + *pp++) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_cgpa(p, e, m)) < 0)
		return (rtn);	/* CGPA V */
	if ((pp = p + *p++) + 1 > e)
		return (-EMSGSIZE);
	if ((ep = pp + ((*pp++) | ((*p++) << 8))) > e)
		return (-EMSGSIZE);
	if ((rtn = unpack_ldata(pp, ep, m)) < 0)
		return (rtn);	/* LDATA V */
	if (*p)
		sccp_dec_opt(p + *p, e, m);
	if (!(m->parms & SCCP_PTF_IMP))
		m->imp = SCCP_DI_LUDTS;
	return (0);
}

/*
 *  SCMG Message
 *  -------------------------------------------------------------------------
 */
static int
sccp_dec_scmg(uchar *p, uchar *e, struct sccp_msg *m)
{
	int rtn;

	if ((rtn = unpack_scmgfi(p, e, m)) < 0)
		return (rtn);	/* SCMG FI */
	p += rtn;
	if ((rtn = unpack_assn(p, e, m)) < 0)
		return (rtn);	/* ASSN */
	p += rtn;
	if ((rtn = unpack_apc(p, e, m)) < 0)
		return (rtn);	/* APC */
	p += rtn;
	if ((rtn = unpack_smi(p, e, m)) < 0)
		return (rtn);	/* SMI */
	p += rtn;
	if (p != e)
		return (-EMSGSIZE);
	return (0);
}

/*
 *  -------------------------------------------------------------------------
 *
 *  SCCP Routing Functions
 *
 *  -------------------------------------------------------------------------
 */
static int
sccp_recv_msg(struct sc *sc, queue_t *q, mblk_t *bp, mblk_t *mp)
{
	struct sccp_msg *m = (typeof(m)) mp->b_rptr;

	switch (m->type) {
	case SCCP_MT_CR:
		return sccp_recv_cr(sc, q, bp, mp, m);
	case SCCP_MT_CC:
		return sccp_recv_cc(sc, q, bp, mp, m);
	case SCCP_MT_CREF:
		return sccp_recv_cref(sc, q, bp, mp, m);
	case SCCP_MT_RLSD:
		return sccp_recv_rlsd(sc, q, bp, mp, m);
	case SCCP_MT_RLC:
		return sccp_recv_rlc(sc, q, bp, mp, m);
	case SCCP_MT_DT1:
		return sccp_recv_dt1(sc, q, bp, mp, m);
	case SCCP_MT_DT2:
		return sccp_recv_dt2(sc, q, bp, mp, m);
	case SCCP_MT_AK:
		return sccp_recv_ak(sc, q, bp, mp, m);
	case SCCP_MT_UDT:
		return sccp_recv_udt(sc, q, bp, mp, m);
	case SCCP_MT_UDTS:
		return sccp_recv_udts(sc, q, bp, mp, m);
	case SCCP_MT_ED:
		return sccp_recv_ed(sc, q, bp, mp, m);
	case SCCP_MT_EA:
		return sccp_recv_ea(sc, q, bp, mp, m);
	case SCCP_MT_RSR:
		return sccp_recv_rsr(sc, q, bp, mp, m);
	case SCCP_MT_RSC:
		return sccp_recv_rsc(sc, q, bp, mp, m);
	case SCCP_MT_ERR:
		return sccp_recv_err(sc, q, bp, mp, m);
	case SCCP_MT_IT:
		return sccp_recv_it(sc, q, bp, mp, m);
	case SCCP_MT_XUDT:
		return sccp_recv_xudt(sc, q, bp, mp, m);
	case SCCP_MT_XUDTS:
		return sccp_recv_xudts(sc, q, bp, mp, m);
	case SCCP_MT_LUDT:
		return sccp_recv_ludt(sc, q, bp, mp, m);
	case SCCP_MT_LUDTS:
		return sccp_recv_ludts(sc, q, bp, mp, m);
	}
	/* any message with an unknown message type is discarded */
	mi_strlog(q, 0, SL_ERROR, "message with unknown message type");
	if (bp)
		freeb(bp);
	freemsg(mp);
	return (0);
}

static int
sccp_recv_scmg(struct sp *sp, queue_t *q, mblk_t *bp, mblk_t *mp, struct sccp_msg *m)
{
	int err;
	struct sr *sr = NULL;
	struct rs *rs = NULL;
	uchar *p, *e;

	if (!(m->parms & (SCCP_PTF_DATA | SCCP_PTF_LDATA)))
		goto eproto;
	p = m->data.ptr;
	e = p + m->data.len;
	if ((err = sccp_dec_scmg(p, e, m)) < 0)
		goto error;
	if (m->apc) {
		if (!(sr = sccp_lookup_sr(sp, m->apc, 1)))
			goto discard;
		if (m->assn)
			if (!(rs = sccp_lookup_rs(sr, m->assn, 1)))
				goto discard;
	}
	switch (m->fi) {
	case SCMG_MT_SSA:
		return scmg_recv_ssa(sp, q, bp, sr, rs, mp, m);
	case SCMG_MT_SSP:
		return scmg_recv_ssp(sp, q, bp, sr, rs, mp, m);
	case SCMG_MT_SST:
		return scmg_recv_sst(sp, q, bp, sr, rs, mp, m);
	case SCMG_MT_SOR:
		return scmg_recv_sor(sp, q, bp, sr, rs, mp, m);
	case SCMG_MT_SOG:
		return scmg_recv_sog(sp, q, bp, sr, rs, mp, m);
	case SCMG_MT_SSC:
		return scmg_recv_ssc(sp, q, bp, sr, rs, mp, m);
	case SCMG_MT_SBR:
		return scmg_recv_sbr(sp, q, bp, sr, rs, mp, m);
	case SCMG_MT_SNR:
		return scmg_recv_snr(sp, q, bp, sr, rs, mp, m);
	case SCMG_MT_SRT:
		return scmg_recv_srt(sp, q, bp, sr, rs, mp, m);
	}
	/* any message with an unknown message type is discarded */
	err = -ENOPROTOOPT;
	goto error;
      eproto:
	err = -EPROTO;
	rare();
	goto error;
      discard:
	err = QR_DONE;
	rare();
	goto error;
      error:
	if (bp)
		freeb(bp);
	freemsg(mp);
	return (0);
}

/*
 *  SCCP GTT ROUTING
 *  -------------------------------------------------------------------------
 */
#if 0
static int
sccp_grte_cr(queue_t *q, struct sp *src, mblk_t *mp, struct sccp_msg *m)
{
	int err, refc;
	struct sp *dst;
	np_ulong smi, cong;

	/* translate and handle hop counter */
	if (m->flags & SCCPF_HOPC_VIOLATION)
		goto sccp_hopc_violation;
	if (m->flags & SCCPF_XLAT_FAILURE)
		goto no_address_type_translation;
	if ((dst = sccp_lookup_sp(src->add.ni, m->cdpa.pc, 0))) {
		struct ss *ss;
		struct sc *sc;

		switch (m->cdpa.ri) {
		case SCCP_RI_DPC_SSN:
			if (dst->flags & SCCPF_PROHIBITED)
				goto destination_inaccessible;
			if (dst->flags & (SCCPF_UNEQUIPPED | SCCPF_INACCESSIBLE))
				goto sccp_failure;
			if (dst->flags & SCCPF_CONGESTED) {
				if (src->proto.popt & SS7_POPT_MPLEV) {
					if (m->rl.mp < dst->level)
						goto access_congestion;
				} else if (m->imp <= dst->level)
					if (m->imp < dst->level
					    || (dst->count++ & 0x3) < dst->level)
						goto access_congestion;
			}
			smi = 0;
			cong = 0;
			if (!(ss = sccp_lookup_ss(dst, m->cdpa.ssn, 0)))
				goto unequipped_user;
			smi = ss->smi;
			cong = ss->level;
			if ((ss->flags & SCCPF_PROHIBITED) || !(sc = ss->cr.list))
				goto subsystem_failure;
			if ((ss->flags & SCCPF_CONGESTED) && (ss->count++ & 0x7))
				goto subsystem_congestion;
			if (!canput(sc->oq))
				goto subsystem_congestion;
			ss7_oput(sc->oq, mp);
			return (QR_DONE);
		case SCCP_RI_GT:
			if (src == dst)
				/* inform maintenance of GTT loop */
				goto unqualified;
			if (!(sc = dst->gt.list))
				goto access_failure;
			if (!canput(sc->oq))
				goto access_congestion;
			ss7_oput(sc->oq, mp);
			return (QR_DONE);
		}
	} else {
		struct sr *sr;
		struct rs *rs;
		struct mt *mt;

		if (!(sr = sccp_lookup_sr(src, m->cdpa.pc, 1)))
			goto destination_address_unknown;
		if (sr->flags & SCCPF_PROHIBITED)
			goto destination_inaccessible;
		if (sr->flags & (SCCPF_UNEQUIPPED | SCCPF_INACCESSIBLE))
			goto sccp_failure;
		if (sr->flags & SCCPF_CONGESTED) {
			if (src->proto.popt & SS7_POPT_MPLEV) {
				if (m->rl.mp < sr->level)
					goto access_congestion;
			} else if (m->imp <= sr->level)
				if (m->imp < sr->level || (sr->count++ & 0x3) < sr->level)
					goto access_congestion;
		}
		if (m->cdpa.ri == SCCP_RI_DPC_SSN) {
			smi = 0;
			cong = 0;
			if (!(rs = sccp_lookup_rs(sr, m->cdpa.ssn, 1)))
				goto unequipped_user;
			smi = rs->smi;
			cong = rs->level;
			if ((rs->flags & SCCPF_PROHIBITED))
				goto subsystem_failure;
			if ((rs->flags & SCCPF_CONGESTED) && (rs->count++ & 0x7))
				goto subsystem_congestion;
		}
		if (!(mt = sr->mt))
			goto unqualified;
		if (canput(mt->oq))
			goto unqualified;
		if (m->flags & SCCPF_ASSOC_REQUIRED) {
			/* create coupling */
			/* process cr on coupling */
			fixme(("Write this function\n"));
			return (-EFAULT);
		} else {
			ss7_oput(mt->oq, mp);
			return (QR_ABSORBED);
		}
	}
      sccp_failure:
	refc = SCCP_REFC_SCCP_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "refusal: SCCP failure");
	goto do_refc;
      sccp_hopc_violation:
	refc = SCCP_REFC_SCCP_HOP_COUNTER_VIOLATION;
	mi_strlog(q, 0, SL_TRACE, "refusal: hop counter violation");
	goto do_refc;
      no_address_type_translation:
	refc = SCCP_REFC_NO_ADDRESS_TYPE_TRANSLATION;
	mi_strlog(q, 0, SL_TRACE, "refusal: no address type translation");
	goto do_refc;
      unqualified:
	/* inform maintenance of GTT loop */
	refc = SCCP_REFC_UNQUALIFIED;
	mi_strlog(q, 0, SL_TRACE, "refusal: GT translation loop");
	goto do_refc;
      access_failure:
	refc = SCCP_REFC_ACCESS_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "refusal: access failure");
	goto do_refc;
      access_congestion:
	refc = SCCP_REFC_ACCESS_CONGESTION;
	mi_strlog(q, 0, SL_TRACE, "refusal: access congestion");
	goto do_refc;
      unequipped_user:
	refc = SCCP_REFC_UNEQUIPPED_USER;
	mi_strlog(q, 0, SL_TRACE, "refusal: subsystem unequipped");
	goto do_ssp;
      subsystem_failure:
	refc = SCCP_REFC_SUBSYSTEM_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "refusal: subsystem failure");
	goto do_ssp;
      subsystem_congestion:
	refc = SCCP_REFC_SUBSYSTEM_CONGESTION;
	mi_strlog(q, 0, SL_TRACE, "refusal: flow controlled");
	if (!(src->proto.popt & SS7_POPT_MPLEV))
		goto do_ssc;
	goto do_refc;
      destination_address_unknown:
	refc = SCCP_REFC_DESTINATION_ADDRESS_UNKNOWN;
	mi_strlog(q, 0, SL_TRACE, "refusal: destination unknown");
	goto do_refc;
      destination_inaccessible:
	refc = SCCP_REFC_DESTINATION_INACCESSIBLE;
	mi_strlog(q, 0, SL_TRACE, "refusal: destination inaccessible");
	goto do_refc;
      do_ssc:
	if ((err =
	     sccp_send_ssc(src, q, bp, m->rl.opc, &m->cdpa,
			   (m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL, m->cdpa.ssn, m->cdpa.pc,
			   smi, cong)) < 0)
		return (err);
	goto do_refc;
      do_ssp:
	if ((err =
	     sccp_send_ssp(q, src, bp, m->rl.opc, &m->cdpa,
			   (m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL, m->cdpa.ssn, m->cdpa.pc,
			   smi)) < 0)
		return (err);
	goto do_refc;
      do_refc:
	/* invoke CREF procedures with REFC */
	if ((err =
	     sccp_send_cref(q, src, bp, m->rl.opc, m->rl.sls, m->slr, refc,
			    (m->parms & SCCP_PTF_CGPA ? &m->cgpa : NULL), mp->b_cont,
			    (m->parms & SCCP_PTF_IMP ? &m->imp : NULL))) < 0)
		return (err);
	return (QR_DONE);
}
#endif

#if 0
static int
sccp_grte_cl(queue_t *q, struct sp *src, mblk_t *mp, struct sccp_msg *m)
{
	int err, retc;
	struct sp *dst;
	np_ulong smi, cong;

	/* translate and handle hop counter */
	if (m->flags & SCCPF_HOPC_VIOLATION)
		goto sccp_hopc_violation;
	if (m->flags & SCCPF_XLAT_FAILURE)
		goto no_address_type_translation;
	if ((dst = sccp_lookup_sp(src->add.ni, m->cdpa.pc, 0))) {
		struct ss *ss;
		struct sc *sc;

		switch (m->cdpa.ri) {
		case SCCP_RI_DPC_SSN:
			if (dst->flags & SCCPF_PROHIBITED)
				goto mtp_failure;
			if (dst->flags & (SCCPF_UNEQUIPPED | SCCPF_INACCESSIBLE))
				goto sccp_failure;
			if (dst->flags & SCCPF_CONGESTED) {
				if (src->proto.popt & SS7_POPT_MPLEV) {
					if (m->rl.mp < dst->level)
						goto network_congestion;
				} else if (m->imp <= dst->level)
					if (m->imp < dst->level
					    || (dst->count++ & 0x3) < dst->level)
						goto network_congestion;
			}
			smi = 0;
			cong = 0;
			if (!(ss = sccp_lookup_ss(dst, m->cdpa.ssn, 0)))
				goto unequipped_user;
			smi = ss->smi;
			cong = ss->level;
			if ((ss->flags & SCCPF_PROHIBITED) || !(sc = ss->cl.list))
				goto subsystem_failure;
			if (!canput(sc->oq))
				goto subsystem_congestion;
			ss7_oput(sc->oq, mp);
			return (QR_DONE);
		case SCCP_RI_GT:
			if (src == dst)
				/* inform maintenance of GTT loop */
				goto unqualified;
			if (!(sc = dst->gt.list))
				goto no_address_translation;
			if (!canput(sc->oq))
				goto unqualified;
			ss7_oput(sc->oq, mp);
			return (QR_DONE);
		}
	} else {
		struct sr *sr;
		struct rs *rs;
		struct mt *mt;

		if (!(sr = sccp_lookup_sr(src, m->cdpa.pc, 1)))
			goto mtp_failure;
		if (sr->flags & SCCPF_PROHIBITED)
			goto mtp_failure;
		if (sr->flags & (SCCPF_UNEQUIPPED | SCCPF_INACCESSIBLE))
			goto sccp_failure;
		if (sr->flags & SCCPF_CONGESTED) {
			if (src->proto.popt & SS7_POPT_MPLEV) {
				if (m->rl.mp < sr->level)
					goto network_congestion;
			} else if (m->imp <= sr->level)
				if (m->imp < sr->level || (sr->count++ & 0x3) < sr->level)
					goto network_congestion;
		}
		smi = 0;
		cong = 0;
		if (m->cdpa.ri == SCCP_RI_DPC_SSN) {
			if (!(rs = sccp_lookup_rs(sr, m->cdpa.ssn, 1)))
				goto unequipped_user;
			smi = rs->smi;
			cong = rs->level;
			if (rs->flags & SCCPF_PROHIBITED)
				goto subsystem_failure;
			if (rs->flags & SCCPF_CONGESTED && (rs->count++ & 0x7))
				goto subsystem_congestion;
		}
		if (!(mt = sr->mt))
			goto local_processing_error;
		if (!canput(mt->oq))
			goto network_congestion;
		if (m->flags & SCCPF_CHANGE_REQUIRED) {
			/* change message */
			goto message_change_failure;
			goto segmentation_not_supported;
			goto segmentation_failure;
		}
		ss7_oput(mt->oq, mp);
		return (QR_DONE);
	}
      sccp_hopc_violation:
	retc = SCCP_RETC_SCCP_HOP_COUNTER_VIOLATION;
	mi_strlog(q, 0, SL_TRACE, "return: SCCP hop counter violation");
	goto do_retc;
      no_address_type_translation:
	retc = SCCP_RETC_NO_ADDRESS_TYPE_TRANSLATION;
	mi_strlog(q, 0, SL_TRACE, "return: no address type translation");
	goto do_retc;
      unqualified:
	retc = SCCP_RETC_UNQUALIFIED;
	mi_strlog(q, 0, SL_TRACE, "return: unqualified");
	goto do_retc;
      no_address_translation:
	retc = SCCP_RETC_NO_ADDRESS_TRANSLATION;
	mi_strlog(q, 0, SL_TRACE, "return: no address translation");
	goto do_retc;
      unequipped_user:
	retc = SCCP_RETC_UNEQUIPPED_USER;
	mi_strlog(q, 0, SL_TRACE, "return: unequipped user");
	goto do_ssp;
      subsystem_congestion:
	retc = SCCP_RETC_SUBSYSTEM_CONGESTION;
	mi_strlog(q, 0, SL_TRACE, "return: subsystem congestion");
	if (!(src->proto.popt & SS7_POPT_MPLEV))
		goto do_ssc;
	goto do_retc;
      mtp_failure:
	retc = SCCP_RETC_MTP_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "return: MTP failure");
	goto do_retc;
      sccp_failure:
	retc = SCCP_RETC_SCCP_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "return: SCCP failure");
	goto do_retc;
      subsystem_failure:
	retc = SCCP_RETC_SUBSYSTEM_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "return: subsystem failure");
	goto do_ssp;
      local_processing_error:
	retc = SCCP_RETC_LOCAL_PROCESSING_ERROR;
	mi_strlog(q, 0, SL_TRACE, "return: local processing error");
	goto do_retc;
      network_congestion:
	retc = SCCP_RETC_NETWORK_CONGESTION;
	mi_strlog(q, 0, SL_TRACE, "return: network congestion");
	goto do_retc;
      message_change_failure:
	retc = SCCP_RETC_MESSAGE_CHANGE_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "return: message change failure");
	goto do_retc;
      segmentation_not_supported:
	retc = SCCP_RETC_SEGMENTATION_NOT_SUPPORTED;
	mi_strlog(q, 0, SL_TRACE, "return: segmentation not supporte");
	goto do_retc;
      segmentation_failure:
	retc = SCCP_RETC_SEGMENTATION_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "return: segmentation failure");
	goto do_retc;
#if 0
      no_resassembly_at_destination:
	retc = SCCP_RETC_NO_REASSEMBLY_AT_DESTINATION;
	mi_strlog(q, 0, SL_TRACE, "return: no reassembly at destination");
	goto do_retc;
#endif
      do_ssc:
	if ((err =
	     sccp_send_ssc(src, q, bp, m->rl.opc, &m->cdpa,
			   (m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL, m->cdpa.ssn, m->cdpa.pc,
			   smi, cong)) < 0)
		return (err);
	goto do_retc;
      do_ssp:
	if ((err =
	     sccp_send_ssp(q, src, bp, m->rl.opc, &m->cdpa,
			   (m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL, m->cdpa.ssn, m->cdpa.pc,
			   smi)) < 0)
		return (err);
	goto do_retc;
      do_retc:
	if ((1 << m->type) & (SCCP_MTF_UDTS | SCCP_MTF_XUDTS | SCCP_MTF_LUDTS)) {
		mi_strlog(q, 0, SL_TRACE, "discard: service message");
		return (QR_DONE);
	}
	if (!m->ret) {
		mi_strlog(q, 0, SL_TRACE, "discard: no return on error");
		return (QR_DONE);
	}
	switch (m->type) {
	case SCCP_MT_UDT:
		if ((err =
		     sccp_send_udts(src, q, bp, m->rl.opc, m->rl.mp, m->rl.sls, retc,
				    ((m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL),
				    ((m->parms & SCCP_PTF_CDPA) ? &m->cdpa : NULL),
				    mp->b_cont)) < 0)
			return (err);
		return (QR_ABSORBED);
	case SCCP_MT_XUDT:
		if ((err =
		     sccp_send_xudts(src, q, bp, m->rl.opc, m->rl.mp, m->rl.sls, retc, m->hopc,
				     ((m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL),
				     ((m->parms & SCCP_PTF_CDPA) ? &m->cdpa : NULL), mp->b_cont,
				     ((m->parms & SCCP_PTF_SGMT) ? &m->sgmt : NULL),
				     ((m->parms & SCCP_PTF_IMP) ? &m->imp : NULL),
				     ((m->parms & SCCP_PTF_INS) ? &m->ins : NULL),
				     ((m->parms & SCCP_PTF_MTI) ? &m->mti : NULL),
				     ((m->parms & SCCP_PTF_ISNI) ? &m->isni : NULL))) < 0)
			return (err);
		return (QR_ABSORBED);
	case SCCP_MT_LUDT:
		if ((err =
		     sccp_send_ludts(src, q, bp, m->rl.opc, m->rl.mp, m->rl.sls, retc, m->hopc,
				     ((m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL),
				     ((m->parms & SCCP_PTF_CDPA) ? &m->cdpa : NULL), mp->b_cont,
				     ((m->parms & SCCP_PTF_SGMT) ? &m->sgmt : NULL),
				     ((m->parms & SCCP_PTF_IMP) ? &m->imp : NULL),
				     ((m->parms & SCCP_PTF_ISNI) ? &m->isni : NULL),
				     ((m->parms & SCCP_PTF_INS) ? &m->ins : NULL))) < 0)
			return (err);
		return (QR_ABSORBED);
	}
	pswerr(("%s: SWERR: RoE procedure no UDTS\n", DRV_NAME));
	return (-EFAULT);
}
#endif

/*
 *  SCCP GTT ROUTING
 *  -------------------------------------------------------------------------
 *  Process the result of a global title translation.
 */
#if 0
static int
sccp_grte_msg(queue_t *q, mblk_t *mp)
{
	struct sc *sc = SCCP_PRIV(q);
	struct sccp_msg *m = (typeof(m)) mp->b_rptr;
	struct sp *sp = sc->sp.sp;

	if (sp) {
		if ((1 << m->type) & (SCCP_MTM_CL))
			return sccp_grte_cl(sp, q, mp, m);
		if ((1 << m->type) & (SCCP_MTF_CR))
			return sccp_grte_cr(sp, q, mp, m);
		pswerr(("%s: ERROR: routing failure\n", DRV_NAME));
		return (-EPROTO);
	}
	swerr();
	return (QR_DISABLE);	/* FIXME: should be hangup */
}
#endif

/*
 *  SCCP O/G ROUTING
 *  -------------------------------------------------------------------------
 */
static int
sccp_orte_cr(struct sp *src, queue_t *q, mblk_t *bp, mblk_t *mp, struct sccp_msg *m)
{
	int err, refc;
	np_ulong dpc, smi = 0, cong = 0;

	if ((dpc = m->cdpa.pc) != -1 || (dpc = m->rl.dpc) != -1) {
		struct sp *dst;

		if ((dst = sccp_lookup_sp(src->add.ni, dpc, 0))) {
			struct ss *ss;
			struct sc *sc;

			switch (m->cdpa.ri) {
			case SCCP_RI_DPC_SSN:
				if (dst->flags & SCCPF_PROHIBITED)
					goto destination_inaccessible;
				if (dst->flags & (SCCPF_UNEQUIPPED | SCCPF_INACCESSIBLE))
					goto sccp_failure;
				if (dst->flags & SCCPF_CONGESTED) {
					if (src->proto.popt & SS7_POPT_MPLEV) {
						if (m->rl.mp < dst->level)
							goto access_congestion;
					} else if (m->imp <= dst->level)
						if (m->imp < dst->level
						    || (dst->count++ & 0x3) < dst->level)
							goto access_congestion;
				}
				if (!(ss = sccp_lookup_ss(dst, m->cdpa.ssn, 0)))
					goto unequipped_user;
				smi = ss->smi;
				cong = ss->level;
				if ((ss->flags & SCCPF_PROHIBITED) || !(sc = ss->cr.list))
					goto subsystem_failure;
				if (!canput(sc->oq))
					goto subsystem_congestion;
				if (bp)
					freeb(bp);
				ss7_oput(sc->oq, mp);
				return (QR_DONE);
			case SCCP_RI_GT:
				goto do_gtt;
			}
		} else {
			struct sr *sr;
			struct mt *mt;

			if (!(sr = sccp_lookup_sr(src, dpc, 1)))
				goto destination_address_unknown;
			if (sr->flags & SCCPF_PROHIBITED)
				goto destination_inaccessible;
			if (sr->flags & (SCCPF_UNEQUIPPED | SCCPF_INACCESSIBLE))
				goto sccp_failure;
			if (sr->flags & SCCPF_CONGESTED) {
				if (src->proto.popt & SS7_POPT_MPLEV) {
					if (m->rl.mp < sr->level)
						goto access_congestion;
				} else if (m->imp <= sr->level)
					if (m->imp < sr->level || (sr->count++ & 0x3) < sr->level)
						goto access_congestion;
			}
			if (m->cdpa.ri == SCCP_RI_DPC_SSN) {
				struct rs *rs;

				if (!(rs = sccp_lookup_rs(sr, m->cdpa.ssn, 1)))
					goto unequipped_user;
				smi = rs->smi;
				cong = rs->level;
				if (rs->flags & SCCPF_PROHIBITED)
					goto subsystem_failure;
			}
			if (!(mt = sr->mt))
				goto local_processing_error;
			if (!canput(mt->oq))
				goto access_congestion;
			if (bp)
				freeb(bp);
			ss7_oput(mt->oq, mp);
			return (QR_DONE);
		}
	} else {
		struct sc *sc;

		if (m->cdpa.ri == SCCP_RI_DPC_SSN)
			goto access_failure;
	      do_gtt:
		if (!(sc = src->gt.list))
			goto access_failure;
		if (!canput(sc->oq))
			goto access_congestion;
		if (bp)
			freeb(bp);
		ss7_oput(sc->oq, mp);
		return (QR_DONE);
	}
      unequipped_user:
	refc = SCCP_REFC_UNEQUIPPED_USER;
	mi_strlog(q, 0, SL_TRACE, "refusal: unequipped user");
	goto do_ssp;
      subsystem_failure:
	refc = SCCP_REFC_SUBSYSTEM_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "refusal: subsystem failure");
	goto do_ssp;
      subsystem_congestion:
	refc = SCCP_REFC_SUBSYSTEM_CONGESTION;
	mi_strlog(q, 0, SL_TRACE, "refusal: subsystem congestion");
	if (!(src->proto.popt & SS7_POPT_MPLEV))
		goto do_ssc;
	goto do_refc;
      destination_address_unknown:
	refc = SCCP_REFC_DESTINATION_ADDRESS_UNKNOWN;
	mi_strlog(q, 0, SL_TRACE, "refusal: destination address unknown");
	goto do_refc;
      destination_inaccessible:
	refc = SCCP_REFC_DESTINATION_INACCESSIBLE;
	mi_strlog(q, 0, SL_TRACE, "refusal: destination inaccessible");
	goto do_refc;
      sccp_failure:
	refc = SCCP_REFC_SCCP_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "refusal: SCCP failure");
	goto do_refc;
      local_processing_error:
	refc = SCCP_REFC_UNQUALIFIED;
	mi_strlog(q, 0, SL_TRACE, "refusal: unqualified");
	goto do_refc;
      access_failure:
	refc = SCCP_REFC_ACCESS_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "refusal: access failure");
	goto do_refc;
      access_congestion:
	refc = SCCP_REFC_ACCESS_CONGESTION;
	mi_strlog(q, 0, SL_TRACE, "refusal: access congestion");
	goto do_refc;
      do_ssc:
	if ((err =
	     sccp_send_ssc(src, q, bp, m->rl.opc, &m->cdpa,
			   (m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL, m->cdpa.ssn, dpc, smi,
			   cong)) < 0)
		return (err);
	goto do_refc;
      do_ssp:
	if ((err =
	     sccp_send_ssp(src, q, bp, m->rl.opc, &m->cdpa,
			   (m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL, m->cdpa.ssn, dpc,
			   smi)) < 0)
		return (err);
	goto do_refc;
      do_refc:
	/* invoke CREF procedures with REFC */
	if ((err =
	     sccp_send_cref(src, q, bp, m->rl.opc, m->rl.sls, m->slr, refc,
			    (m->parms & SCCP_PTF_CGPA ? &m->cgpa : NULL), mp->b_cont,
			    (m->parms & SCCP_PTF_IMP ? &m->imp : NULL))) < 0)
		return (err);
	return (QR_DONE);
}

static int
sccp_orte_co(struct sp *src, queue_t *q, mblk_t *bp, mblk_t *mp, struct sccp_msg *m)
{
	int err, relc = 0;
	struct sr *sr;
	struct mt *mt;

	if (!(sr = sccp_lookup_sr(src, src->add.pc, 1)))
		goto mtp_failure;
	if (sr->flags & SCCPF_PROHIBITED)
		goto mtp_failure;
	if (sr->flags & (SCCPF_UNEQUIPPED | SCCPF_INACCESSIBLE))
		goto sccp_failure;
	if (sr->flags & SCCPF_CONGESTED) {
		if (sr->proto.popt & SS7_POPT_MPLEV) {
			if (m->rl.mp < sr->level)
				goto network_congestion;
		} else if (m->imp <= sr->level)
			if (m->imp < sr->level || (sr->count++ & 0x3) < sr->level)
				goto network_congestion;
	}
	if (!(mt = sr->mt))
		goto mtp_failure;
	if (!canput(mt->oq))
		goto network_congestion;
	if (bp)
		freeb(bp);
	ss7_oput(mt->oq, mp);
	return (QR_DONE);
#if 0
      access_failure:
	relc = SCCP_RELC_ACCESS_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "release: access failure");
	goto do_relc;
      access_congestion:
	relc = SCCP_RELC_ACCESS_CONGESTION;
	mi_strlog(q, 0, SL_TRACE, "release: access congestion");
	goto do_relc;
      subsystem_failure:
	relc = SCCP_RELC_SUBSYSTEM_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "release: subsystem failure");
	goto do_relc;
      subsystem_congestion:
	relc = SCCP_RELC_SUBSYSTEM_CONGESTION;
	mi_strlog(q, 0, SL_TRACE, "release: subsystem congestion");
	goto do_relc;
#endif
      mtp_failure:
	relc = SCCP_RELC_MTP_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "release: MTP failure");
	goto do_relc;
      network_congestion:
	relc = SCCP_RELC_NETWORK_CONGESTION;
	mi_strlog(q, 0, SL_TRACE, "release: network congestion");
	goto do_relc;
#if 0
      unqualified:
	relc = SCCP_RELC_UNQUALIFIED;
	mi_strlog(q, 0, SL_TRACE, "release: unqualified");
	goto do_relc;
#endif
      sccp_failure:
	relc = SCCP_RELC_SCCP_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "release: SCCP failure");
	goto do_relc;
      do_relc:
	if ((err =
	     sccp_send_rlsd(src, q, bp, m->rl.opc, m->rl.sls, m->slr, m->dlr, relc,
			    mp->b_cont, (m->parms & SCCP_PTF_IMP) ? &m->imp : NULL)) < 0)
		return (err);
	return (QR_DONE);
}

static int
sccp_orte_cl(struct sp *src, queue_t *q, mblk_t *bp, mblk_t *mp, struct sccp_msg *m)
{
	int err, retc;
	np_ulong dpc, smi = 0, cong = 0;

	if ((dpc = m->cdpa.pc) != -1 || (dpc = m->rl.dpc) != -1) {
		struct sp *dst;

		if ((dst = sccp_lookup_sp(src->add.ni, dpc, 0))) {
			struct ss *ss;
			struct sc *sc;

			switch (m->cdpa.ri) {
			case SCCP_RI_DPC_SSN:
				if (dst->flags & SCCPF_PROHIBITED)
					goto mtp_failure;
				if (dst->flags & (SCCPF_UNEQUIPPED | SCCPF_INACCESSIBLE))
					goto sccp_failure;
				if (dst->flags & SCCPF_CONGESTED) {
					if (src->proto.popt & SS7_POPT_MPLEV) {
						if (m->rl.mp < dst->level)
							goto network_congestion;
					} else if (m->imp <= dst->level)
						if (m->imp < dst->level
						    || (dst->count++ & 0x3) < dst->level)
							goto network_congestion;
				}
				if (m->cdpa.ssn == 1)
					goto do_scmg;
				if (!(ss = sccp_lookup_ss(dst, m->cdpa.ssn, 0)))
					goto unequipped_user;
				smi = ss->smi;
				cong = ss->level;
				if ((ss->flags & SCCPF_PROHIBITED) || !(sc = ss->cl.list))
					goto subsystem_failure;
				if (!canput(sc->oq))
					goto subsystem_congestion;
				if (bp)
					freeb(bp);
				ss7_oput(sc->oq, mp);
				return (QR_DONE);
			case SCCP_RI_GT:
				goto do_gtt;
			}
		} else {
			struct sr *sr;
			struct rs *rs;
			struct mt *mt;

			if (!(sr = sccp_lookup_sr(src, dpc, 1)))
				goto mtp_failure;
			if (sr->flags & SCCPF_PROHIBITED)
				goto mtp_failure;
			if (sr->flags & (SCCPF_UNEQUIPPED | SCCPF_INACCESSIBLE))
				goto sccp_failure;
			if (sr->flags & SCCPF_CONGESTED) {
				if (src->proto.popt & SS7_POPT_MPLEV) {
					if (m->rl.mp < sr->level)
						goto network_congestion;
				} else if (m->imp <= sr->level)
					if (m->imp < sr->level || (sr->count++ & 0x3) < sr->level)
						goto network_congestion;
			}
			if (m->cdpa.ri == SCCP_RI_DPC_SSN) {
				if (!(rs = sccp_lookup_rs(sr, m->cdpa.ssn, 1)))
					goto unequipped_user;	/* XXX */
				smi = rs->smi;
				cong = rs->level;
				if (rs->flags & SCCPF_PROHIBITED)
					goto subsystem_failure;
			}
			if (!(mt = sr->mt))
				goto mtp_failure;
			if (!canput(mt->oq))
				goto network_congestion;
			if (m->flags & SCCPF_CHANGE_REQUIRED) {
				/* change message */
			}
			if (bp)
				freeb(bp);
			ss7_oput(mt->oq, mp);
			return (QR_DONE);
		}
	} else {
		struct sc *sc;

		if (m->cdpa.ri == SCCP_RI_DPC_SSN)
			goto mtp_failure;
	      do_gtt:
		if (!(sc = src->gt.list))
			goto no_address_translation;
		if (!canput(sc->oq))
			goto unqualified;
		if (bp)
			freeb(bp);
		ss7_oput(sc->oq, mp);
		return (QR_DONE);
	}
      do_scmg:
	return sccp_recv_scmg(src, q, bp, mp, m);
      unequipped_user:
	retc = SCCP_RETC_UNEQUIPPED_USER;
	printd(("%s: ERROR: unequipped user\n", DRV_NAME));
	goto do_ssp;
      subsystem_failure:
	retc = SCCP_RETC_SUBSYSTEM_FAILURE;
	printd(("%s: ERROR: subsystem failure\n", DRV_NAME));
	goto do_ssp;
      subsystem_congestion:
	retc = SCCP_RETC_SUBSYSTEM_CONGESTION;
	printd(("%s: ERROR: subsystem congestion\n", DRV_NAME));
	if (!(src->proto.popt & SS7_POPT_MPLEV))
		goto do_ssc;
	goto do_retc;
      mtp_failure:
	retc = SCCP_RETC_MTP_FAILURE;
	printd(("%s: ERROR: MTP failure\n", DRV_NAME));
	goto do_retc;
      sccp_failure:
	retc = SCCP_RETC_SCCP_FAILURE;
	printd(("%s: ERROR: SCCP failure\n", DRV_NAME));
	goto do_retc;
      network_congestion:
	retc = SCCP_RETC_NETWORK_CONGESTION;
	printd(("%s: ERROR: network congestion\n", DRV_NAME));
	goto do_retc;
      no_address_translation:
	retc = SCCP_RETC_NO_ADDRESS_TRANSLATION;
	printd(("%s: ERROR: no address translation\n", DRV_NAME));
	goto do_retc;
      unqualified:
	retc = SCCP_RETC_UNQUALIFIED;
	printd(("%s: ERROR: unqualified\n", DRV_NAME));
	goto do_retc;
      do_ssc:
	if ((err =
	     sccp_send_ssc(src, q, bp, m->rl.opc, &m->cdpa,
			   (m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL, m->cdpa.ssn, dpc, smi,
			   cong)) < 0)
		return (err);
	goto do_retc;
      do_ssp:
	if ((err =
	     sccp_send_ssp(src, q, bp, m->rl.opc, &m->cdpa,
			   (m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL, m->cdpa.ssn, dpc,
			   smi)) < 0)
		return (err);
	goto do_retc;
      do_retc:
	if ((1 << m->type) & (SCCP_MTF_UDTS | SCCP_MTF_XUDTS | SCCP_MTF_LUDTS)) {
		mi_strlog(q, 0, SL_TRACE, "discard: service message");
		return (QR_DONE);
	}
	if (!m->ret) {
		mi_strlog(q, 0, SL_TRACE, "discard: no return on error");
		return (QR_DONE);
	}
	switch (m->type) {
	case SCCP_MT_UDT:
		if ((err =
		     sccp_send_udts(src, q, bp, m->rl.opc, m->rl.mp, m->rl.sls, retc,
				    ((m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL),
				    ((m->parms & SCCP_PTF_CDPA) ? &m->cdpa : NULL),
				    mp->b_cont)) < 0)
			return (err);
		return (QR_ABSORBED);
	case SCCP_MT_XUDT:
		if ((err =
		     sccp_send_xudts(src, q, bp, m->rl.opc, m->rl.mp, m->rl.sls, retc, m->hopc,
				     ((m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL),
				     ((m->parms & SCCP_PTF_CDPA) ? &m->cdpa : NULL), mp->b_cont,
				     ((m->parms & SCCP_PTF_SGMT) ? &m->sgmt : NULL),
				     ((m->parms & SCCP_PTF_IMP) ? &m->imp : NULL),
				     ((m->parms & SCCP_PTF_INS) ? &m->ins : NULL),
				     ((m->parms & SCCP_PTF_MTI) ? &m->mti : NULL),
				     ((m->parms & SCCP_PTF_ISNI) ? &m->isni : NULL))) < 0)
			return (err);
		return (QR_ABSORBED);
	case SCCP_MT_LUDT:
		if ((err =
		     sccp_send_ludts(src, q, bp, m->rl.opc, m->rl.mp, m->rl.sls, retc, m->hopc,
				     ((m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL),
				     ((m->parms & SCCP_PTF_CDPA) ? &m->cdpa : NULL), mp->b_cont,
				     ((m->parms & SCCP_PTF_SGMT) ? &m->sgmt : NULL),
				     ((m->parms & SCCP_PTF_IMP) ? &m->imp : NULL),
				     ((m->parms & SCCP_PTF_ISNI) ? &m->isni : NULL),
				     ((m->parms & SCCP_PTF_INS) ? &m->ins : NULL))) < 0)
			return (err);
		return (QR_ABSORBED);
	}
	pswerr(("%s: SWERR: RoE procedure no UDTS\n", DRV_NAME));
	return (-EFAULT);
}

/*
 *  SCCP O/G ROUTING
 *  -------------------------------------------------------------------------
 *  This follows the routing rules for ANSI/ITU SCCP.  We process CL first for speed and then CO and lastly CR.
 *  This is in order of probability.
 */
static int
sccp_orte_msg(struct sp *sp, queue_t *q, mblk_t *bp, mblk_t *mp)
{
	struct sccp_msg *m = (typeof(m)) mp->b_rptr;

	if (sp) {
		if ((1 << m->type) & (SCCP_MTM_CL))
			return sccp_orte_cl(sp, q, bp, mp, m);
		if ((1 << m->type) & (SCCP_MTF_CR))
			return sccp_orte_cr(sp, q, bp, mp, m);
		if ((1 << m->type) & (SCCP_MTM_CO))
			return sccp_orte_co(sp, q, bp, mp, m);
		mi_strlog(q, 0, SL_ERROR, "routing failure");
		return (-EPROTO);
	}
	swerr();
	return (QR_DISABLE);	/* FIXME: should be hangup */
}

/*
 *  SCCP ROUTING Connection Request (CR) Messages only
 *  -------------------------------------------------------------------------
 */
static struct sc **
sccp_cr_lookup(struct sp *sp, np_ulong ssn)
{
	swerr();
	return (NULL);
}
static int
sccp_irte_cr(struct sp *sp, queue_t *q, mblk_t *bp, mblk_t *mp, struct sccp_msg *m)
{
	int err, refc;
	struct ss *ss;
	struct sc *sc;
	np_ulong smi, cong;

	switch (m->cdpa.ri) {
	case SCCP_RI_DPC_SSN:
		smi = 0;
		cong = 0;
		if (!(ss = sccp_lookup_ss(sp, m->cdpa.ssn, 0)))
			goto unequipped_user;
		smi = ss->smi;
		cong = ss->level;
		if (!(ss->flags & SCCPF_PROHIBITED))
			goto subsystem_failure;
		if (!(sc = ss->cr.list))
			goto subsystem_failure;
		if (!canput(sc->oq))
			goto subsystem_congestion;
		ss7_oput(sc->oq, mp);
		return (QR_DONE);
	case SCCP_RI_GT:
		if (!(sc = sp->gt.list))
			goto access_failure;
		if (!canput(sc->oq))
			goto access_congestion;
		ss7_oput(sc->oq, mp);
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
      unequipped_user:
	refc = SCCP_REFC_UNEQUIPPED_USER;
	mi_strlog(q, 0, SL_TRACE, "refusal: subsystem unqeuipped");
	goto do_ssp;
      subsystem_failure:
	refc = SCCP_REFC_SUBSYSTEM_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "refusal: subsystem failure");
	goto do_ssp;
      subsystem_congestion:
	refc = SCCP_REFC_SUBSYSTEM_CONGESTION;
	mi_strlog(q, 0, SL_TRACE, "refusal: flow controlled");
	if (!(sp->proto.popt & SS7_POPT_MPLEV))
		goto do_ssc;
	goto do_refc;
      access_failure:
	refc = SCCP_REFC_ACCESS_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "refusal: GT translation failure");
	goto do_refc;
      access_congestion:
	refc = SCCP_REFC_ACCESS_CONGESTION;
	mi_strlog(q, 0, SL_TRACE, "refusal: GT translation congested");
	goto do_refc;
      do_ssc:
	if ((err =
	     sccp_send_ssc(sp, q, bp, m->rl.opc, &m->cdpa,
			   (m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL, m->cdpa.ssn, m->cdpa.pc,
			   smi, cong)) < 0)
		return (err);
	goto do_refc;
      do_ssp:
	if ((err =
	     sccp_send_ssp(sp, q, bp, m->rl.opc, &m->cdpa,
			   (m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL, m->cdpa.ssn, m->cdpa.pc,
			   smi)) < 0)
		return (err);
	goto do_refc;
      do_refc:
	/* invoke CREF procedures with REFC */
	if ((err =
	     sccp_send_cref(sp, q, bp, m->rl.opc, m->rl.sls, m->slr, refc,
			    (m->parms & SCCP_PTF_CGPA ? &m->cgpa : NULL), mp->b_cont,
			    (m->parms & SCCP_PTF_IMP ? &m->imp : NULL))) < 0)
		return (err);
	return (QR_DONE);
}

/*
 *  SCCP ROUTING Connection-Less Messages only
 *  -------------------------------------------------------------------------
 */
static struct sc **
sccp_cl_lookup(struct sp *sp, np_ulong ssn)
{
	swerr();
	return (NULL);
}
static int
sccp_irte_cl(struct sp *sp, queue_t *q, mblk_t *bp, mblk_t *mp, struct sccp_msg *m)
{
	int err, retc;
	struct ss *ss;
	struct sc *sc;
	np_ulong smi, cong;

	switch (m->cdpa.ri) {
	case SCCP_RI_DPC_SSN:
		if (sp->flags & SCCPF_CONGESTED) {
			if (sp->proto.popt & SS7_POPT_MPLEV) {
				if (m->rl.mp < sp->level)
					goto network_congestion;
			} else if (m->imp <= sp->level)
				if (m->imp < sp->level || (sp->count++ & 0x3) < sp->level)
					goto network_congestion;
		}
		if (m->cdpa.ssn == 1)
			return sccp_recv_scmg(sp, q, bp, mp, m);
		smi = 0;
		cong = 0;
		if (!(ss = sccp_lookup_ss(sp, m->cdpa.ssn, 0)))
			goto unequipped_user;
		smi = ss->smi;
		cong = ss->level;
		if ((ss->flags & SCCPF_PROHIBITED) || !(sc = ss->cl.list))
			goto subsystem_failure;
		if (!canputnext(sc->oq))
			goto subsystem_congestion;
		if (bp)
			freeb(bp);
		ss7_oput(sc->oq, mp);
		return (QR_DONE);
	case SCCP_RI_GT:
		if (!(sc = sp->gt.list))
			goto no_address_translation;
		if (!canputnext(sc->oq))
			goto subsystem_congestion2;
		if (bp)
			freeb(bp);
		ss7_oput(sc->oq, mp);
		return (QR_DONE);
	}
	swerr();
	return (-EFAULT);
      unequipped_user:
	retc = SCCP_RETC_UNEQUIPPED_USER;
	mi_strlog(q, 0, SL_TRACE, "message returned: subsystem unequipped");
	goto do_ssp;
      subsystem_congestion:
	retc = SCCP_RETC_SUBSYSTEM_CONGESTION;
	mi_strlog(q, 0, SL_TRACE, "message returned: subsystem congestion");
	if (!(sp->proto.popt & SS7_POPT_MPLEV))
		goto do_ssc;
	goto do_retc;
      subsystem_failure:
	retc = SCCP_RETC_SUBSYSTEM_FAILURE;
	mi_strlog(q, 0, SL_TRACE, "message returned: subsystem failure");
	goto do_ssp;
      network_congestion:
	retc = SCCP_RETC_NETWORK_CONGESTION;
	mi_strlog(q, 0, SL_TRACE, "message returned: network congestion");
	goto do_retc;
      subsystem_congestion2:
	retc = SCCP_RETC_SUBSYSTEM_CONGESTION;
	mi_strlog(q, 0, SL_TRACE, "message returned: GT Translation congested");
	goto do_retc;
      no_address_translation:
	retc = SCCP_RETC_NO_ADDRESS_TRANSLATION;
	mi_strlog(q, 0, SL_TRACE, "message returned: GT Translation unavailable");
	goto do_retc;
      do_ssc:
	if ((err =
	     sccp_send_ssc(sp, q, bp, m->rl.opc, &m->cdpa,
			   (m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL, m->cdpa.ssn, m->cdpa.pc,
			   smi, cong)) < 0)
		return (err);
	goto do_retc;
      do_ssp:
	if ((err =
	     sccp_send_ssp(sp, q, bp, m->rl.opc, &m->cdpa,
			   (m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL, m->cdpa.ssn, m->cdpa.pc,
			   smi)) < 0)
		return (err);
	goto do_retc;
      do_retc:
	if ((1 << m->type) & (SCCP_MTF_UDTS | SCCP_MTF_XUDTS | SCCP_MTF_LUDTS)) {
		mi_strlog(q, 0, SL_TRACE, "message discarded: service message");
		return (QR_DONE);
	}
	if (!m->ret) {
		mi_strlog(q, 0, SL_TRACE, "message discarded: no return on error");
		return (QR_DONE);
	}
	switch (m->type) {
	case SCCP_MT_UDT:
		if ((err =
		     sccp_send_udts(sp, q, bp, m->rl.opc, m->rl.mp, m->rl.sls, retc,
				    ((m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL),
				    ((m->parms & SCCP_PTF_CDPA) ? &m->cdpa : NULL),
				    mp->b_cont)) < 0)
			return (err);
		return (QR_ABSORBED);
	case SCCP_MT_XUDT:
		if ((err =
		     sccp_send_xudts(sp, q, bp, m->rl.opc, m->rl.mp, m->rl.sls, retc, m->hopc,
				     ((m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL),
				     ((m->parms & SCCP_PTF_CDPA) ? &m->cdpa : NULL), mp->b_cont,
				     ((m->parms & SCCP_PTF_SGMT) ? &m->sgmt : NULL),
				     ((m->parms & SCCP_PTF_IMP) ? &m->imp : NULL),
				     ((m->parms & SCCP_PTF_INS) ? &m->ins : NULL),
				     ((m->parms & SCCP_PTF_MTI) ? &m->mti : NULL),
				     ((m->parms & SCCP_PTF_ISNI) ? &m->isni : NULL))) < 0)
			return (err);
		return (QR_ABSORBED);
	case SCCP_MT_LUDT:
		if ((err =
		     sccp_send_ludts(sp, q, bp, m->rl.opc, m->rl.mp, m->rl.sls, retc, m->hopc,
				     ((m->parms & SCCP_PTF_CGPA) ? &m->cgpa : NULL),
				     ((m->parms & SCCP_PTF_CDPA) ? &m->cdpa : NULL), mp->b_cont,
				     ((m->parms & SCCP_PTF_SGMT) ? &m->sgmt : NULL),
				     ((m->parms & SCCP_PTF_IMP) ? &m->imp : NULL),
				     ((m->parms & SCCP_PTF_ISNI) ? &m->isni : NULL),
				     ((m->parms & SCCP_PTF_INS) ? &m->ins : NULL))) < 0)
			return (err);
		return (QR_ABSORBED);
	}
	pswerr(("%s: SWERR: RoE procedure no UDTS\n", DRV_NAME));
	return (-EFAULT);
}

/*
 *  SCCP ROUTING Connection Oriented Messages (other than CR) only
 *  -------------------------------------------------------------------------
 */
static struct sc **
sccp_co_lookup(struct sp *sp, np_ulong slr)
{
	swerr();
	return (NULL);
}
static int
sccp_irte_co(struct sp *sp, queue_t *q, mblk_t *bp, mblk_t *mp, struct sccp_msg *m)
{
	/* pass message to SCCP user */
	return (QR_DONE);
}

#if 0
static int
sccp_irte_co(struct sp *sp, queue_t *q, mblk_t *mp, struct sccp_msg *m)
{
	int err;
	np_ulong relc;
	struct sr *sr;
	struct sc *sc, **scp;

	/* these are always route on DLR */
	if (!(scp = sccp_co_lookup(sp, m->dlr)) || !(sc = *scp))
		goto unassigned_dlr;
	/* we have a connected segment */
	if (((sc->pcl != 2) || (1 << m->type) & ~(SCCP_MTM_PCLS2))
	    && ((sc->pcl != 3) || (1 << m->type) & ~(SCCP_MTM_PCLS3)))
		goto unqualified_discard;
	if ((m->parms & SCCP_PTF_SLR) && m->slr != sc->slr)
		goto slr_mismatch;
	if (m->rl.opc != sc->opc.pc)
		goto pc_mismatch;
	/* Figure C.4/Q.714 Sheet 4 of 4: Construct an AK message with credit = 0 and send it,
	   restarting the send inactivity timer.  We need to mark that the connection is congested
	   as well so that when congestion ceases (i.e, when the service procedure runs and we are
	   ready to backenable, we will construct an AK message with credit = W and sent it,
	   restarting the inactivity timer). */
	/* Figure 2C/T1.112.4-2000 2of2: same. */
	if (!canput(sc->oq))
		goto ebusy;
	/* pass on for user */
	ss7_oput(sc->oq, mp);
	return (QR_ABSORBED);
      unassigned_dlr:
	/* Figure C.2/Q.714 Sheet 1 of 7 */
	/* Table B-1/T1.112.4-2000 case (a) */
	if ((1 << m->type) & (SCCP_MTF_CC | SCCP_MTF_RSR | SCCP_MTF_RSC))
		goto unassigned_dlrn;
	if ((1 << m->type) & ~(SCCP_MTF_RLSD))
		goto unqualified_discard2;
	goto unqualified_rlc;
      pc_mismatch:
	/* Table B-1/T1.112.4-2000 case (b) */
	if ((1 << m->type) & (SCCP_MTF_RSR | SCCP_MTF_RSC | SCCP_MTF_RLSD))
		goto point_code_mismatch_err;
	goto point_code_mismatch_discard;
      slr_mismatch:
	/* Figure C.2/Q.714 Sheet 1 of 7 */
	/* Table B-1/T1.112.4-2000 case (c) */
	if ((1 << m->type) & (SCCP_MTF_RSR | SCCP_MTF_RSC | SCCP_MTF_RLSD))
		goto inconsistent_slrn_err;
	if ((1 << m->type) & (SCCP_MTF_IT))
		goto inconsistent_connection_data;
	goto inconsistent_slrn_discard;
      point_code_mismatch_err:
	relc = SCCP_ERRC_POINT_CODE_MISMATCH;
	mi_strlog(q, 0, SL_TRACE, "connection released: OPC mismatch (send ERR)");
	/* send ERR */
	goto do_err;
      point_code_mismatch_discard:
	relc = SCCP_ERRC_POINT_CODE_MISMATCH;
	mi_strlog(q, 0, SL_TRACE, "connection released: OPC mismatch (discard)");
	/* discard */
	goto discard;
      inconsistent_slrn_err:
	relc = SCCP_ERRC_LRN_MISMATCH_INCONSISTENT_SOURCE_LRN;
	mi_strlog(q, 0, SL_TRACE, "connection released: SLR mismatch (send ERR)");
	goto do_err;
      inconsistent_slrn_discard:
	relc = SCCP_ERRC_LRN_MISMATCH_INCONSISTENT_SOURCE_LRN;
	mi_strlog(q, 0, SL_TRACE, "connection released: SLR mismatch (discard)");
	/* discard */
	goto discard;
      inconsistent_connection_data:
	relc = SCCP_RELC_INCONSISTENT_CONNECTION_DATA;
	mi_strlog(q, 0, SL_TRACE, "connection released: SLR mismatch (send RLSD)");
	/* Send RLSD(2) */
	goto do_rlsd;
      unassigned_dlrn:
	relc = SCCP_ERRC_LRN_MISMATCH_UNASSIGNED_DEST_LRN;
	mi_strlog(q, 0, SL_TRACE, "connection released: DLR mismatch (send ERR)");
	goto do_err;
      unqualified_rlc:
	relc = SCCP_RELC_UNQUALIFIED;
	mi_strlog(q, 0, SL_TRACE, "connection released: DLR mismatch (send RLC)");
	/* Send RLC */
	goto do_rlc;
      unqualified_discard:
	relc = SCCP_ERRC_UNQUALIFIED;
	mi_strlog(q, 0, SL_TRACE, "connection released: inconsistent CO message type (discard)");
	/* discard */
	goto discard;
      unqualified_discard2:
	relc = SCCP_RELC_UNQUALIFIED;
	mi_strlog(q, 0, SL_TRACE, "connection released: DLR unassigned (discard)");
	goto discard;
      ebusy:
	relc = -ENOSR;
	mi_strlog(q, 0, SL_TRACE, "connection released: flow controlled (dropping)");
	goto discard;
      discard:
	return (QR_DONE);
      do_err:
	if ((err = sccp_send_err(sc->sp.sp, q, bp, sc->sp.sp->add.pc, m->rl.sls, m->slr, relc)))
		goto error;
	goto discard;
      do_rlsd:
	if ((err =
	     sccp_send_rlsd(sc->sp.sp, q, bp, sc->sp.sp->add.pc, m->rl.sls, m->slr, m->dlr, relc,
			    NULL, (m->parms & SCCP_PTF_IMP) ? &m->imp : NULL)))
		goto error;
	goto discard;
      do_rlc:
	if ((err = sccp_send_rlc(sc->sp.sp, q, bp, sc->sp.sp->add.pc, m->rl.sls, m->slr, m->dlr)))
		goto error;
	goto discard;
      enomem:
	err = -ENOMEM;
	rare();
	goto error;
      error:
	return (err);
}
#endif

/*
 *  SCCP I/C ROUTING
 *  -------------------------------------------------------------------------
 *  This follows the routing rules for ANSI/ITU SCCP.  We process CL first for speed and then CO and lastly CR.
 *  This is in order of probability.
 */
static int
sccp_irte_msg(struct mt *mt, queue_t *q, mblk_t *bp, mblk_t *mp)
{
	struct sccp_msg *m = (typeof(m)) mp->b_rptr;
	struct sp *sp = mt->sp ? mt->sp : (mt->sr ? mt->sr->sp.sp : NULL);

	if (sp) {
		if ((1 << m->type) & (SCCP_MTM_CL))
			return sccp_irte_cl(sp, q, bp, mp, m);
		if ((1 << m->type) & (SCCP_MTF_CR))
			return sccp_irte_cr(sp, q, bp, mp, m);
		if ((1 << m->type) & (SCCP_MTM_CO))
			return sccp_irte_co(sp, q, bp, mp, m);
		mi_strlog(q, 0, SL_TRACE, "incoming message routing failure");
		if (bp)
			freeb(bp);
		return (0);
	}
	swerr();
	noenable(q);		/* disable queue temporarily */
	return (-EAGAIN);
}

/*
 *  -------------------------------------------------------------------------
 *
 *  SCCP Interface Functions
 *
 *  -------------------------------------------------------------------------
 */

/*
 *  SCCP BIND REQ
 *  -------------------------------------------------------------------------
 *  When we bind, we bind to a local Network Id, Point Code and Subsystem Number.  To accomplish this, we associate
 *  a small hash table on SSN with each of the lower MTP streams.  When we bind we look for the lower MTP stream
 *  which has the appropriate Network Id, Point Code (and Service Indicator(=3)).  If there is no upper stream bound
 *  for the subsystem number (or default listener) identified, the stream is bound and the position in the hash is
 *  filled and the upper stream linked into the lower stream's list.
 */
static int
sccp_bind_req(struct sc *sc, queue_t *q, struct sccp_addr *src, np_ulong cons)
{
	int err;
	struct sp *sp;
	struct ss *ss;
	struct sc **scp;

	/* look for bound MTP provider */
	if (!(sp = sccp_lookup_sp(src->ni, src->pc, 1)))
		goto eaddrnotavail;
	if (!(ss = sccp_lookup_ss(sp, src->ssn, 1)))
		goto eaddrnotavail;
	switch (sc->pcl) {
	case 0:		/* protocol class 0 */
	case 1:		/* protocol class 1 */
		/* look for bind slot on provider */
		if (!(scp = sccp_cl_lookup(sp, src->ssn)) || (*scp))
			goto eaddrinuse;
		/* we have a CL slot, use it */
		if ((sc->sp.next = *scp))
			sc->sp.next->sp.prev = &sc->sp.next;
		sc->sp.prev = scp;
		*scp = sccp_get(sc);
		sc->sp.sp = sp_get(sp);
		/* bind to subsystem */
		if ((sc->ss.next = ss->cl.list))
			sc->ss.next->ss.prev = &sc->ss.next;
		sc->ss.prev = &ss->cl.list;
		ss->cl.list = sccp_get(sc);
		sc->ss.ss = ss_get(ss);
		break;
	case 2:		/* protocol class 2 */
	case 3:		/* protocol class 3 */
		if (cons) {	/* listening */
			if (!(scp = sccp_cr_lookup(sp, src->ssn)) || (*scp))
				goto eaddrinuse;
			/* we have a CR slot, use it */
			if ((sc->sp.next = *scp))
				sc->sp.next->sp.prev = &sc->sp.next;
			sc->sp.prev = scp;
			*scp = sccp_get(sc);
			sc->sp.sp = sp_get(sp);
			/* bind to subsystem */
			if ((sc->ss.next = ss->cr.list))
				sc->ss.next->ss.prev = &sc->ss.next;
			sc->ss.prev = &ss->cr.list;
			ss->cr.list = sccp_get(sc);
			sc->ss.ss = ss_get(ss);
		} else {	/* not listening */
			/* We don't bind non-listening streams because until they are assigned an
			   SLR, they are not in the hashes.  SLRs are only assigned when we enter
			   the SS_WCON_CREQ state from SS_IDLE or the SS_DATA_XFER state from
			   SS_WRES_CIND. */
			/* bind to subsystem */
			if ((sc->ss.next = ss->co.list))
				sc->ss.next->ss.prev = &sc->ss.next;
			sc->ss.prev = &ss->co.list;
			ss->co.list = sccp_get(sc);
			sc->ss.ss = ss_get(ss);
		}
		break;
	}
	return (0);
      eaddrinuse:
	err = -EADDRINUSE;
	mi_strlog(q, 0, SL_TRACE, "ERROR: address already bound");
	goto error;
      eaddrnotavail:
	err = -EADDRNOTAVAIL;
	mi_strlog(q, 0, SL_TRACE, "ERROR: no such NI/PC pair available");
	goto error;
      error:
	return (err);
}

/*
 *  SCCP UNBIND REQ
 *  -------------------------------------------------------------------------
 *  Here we just take the SCCP stream out of the bind, listen or established hashes for the MTP providers to which
 *  it was bound.
 */
static int
sccp_unbind_req(struct sc *sc, queue_t *q)
{
	/* remove from bind/listen hash */
	if (sc->ss.ss) {
		if ((*sc->ss.prev = sc->ss.next))
			sc->ss.next->ss.prev = &sc->ss.next;
		sc->ss.next = NULL;
		sc->ss.prev = &sc->ss.next;
		ss_put(xchg(&sc->ss.ss, NULL));
		sccp_put(sc);
	}
	/* remove from established hash */
	if (sc->sp.sp) {
		if ((*sc->sp.prev = sc->sp.next))
			sc->sp.next->sp.prev = &sc->sp.next;
		sc->sp.next = NULL;
		sc->sp.prev = &sc->sp.next;
		sp_put(xchg(&sc->sp.sp, NULL));
		sccp_put(sc);
	}
	return (0);
}

/*
 *  SCCP CONN REQ
 *  -------------------------------------------------------------------------
 *  Launching a connection request is like a subsequent bind to a DLRN.  We select a new LRN for the connection and
 *  place ourselves in the LRN hashes for the MTP provider and send a Connection Request.
 */
#define SCCP_SLR_MAX_TRIES	10
static int
sccp_conn_req(struct sc *sc, queue_t *q, struct sccp_addr *dst, np_ulong *sls, np_ulong *pri,
	      np_ulong *pcl, np_ulong *imp, mblk_t *dp)
{
	int err = 0;
	int tries;
	unsigned short slr = 0;		/* just to shut up compiler */
	struct sc **scp = NULL;		/* just to shut up compiler */
	struct sp *sp;

	if (sccp_get_state(sc) != SS_IDLE)
		goto efault;
	if (!(sp = sc->ss.ss->sp.sp))
		goto efault;
	if (sc->sp.sp)
		goto eisconn;
	/* Figure C.2/Q.714: Figure 2A/T1.112.4-2000: If not sufficient resources, return an
	   N_DISCON_IND indication or simply return an N_ERROR_ACK. */
	if (!sc->slr) {
		struct sr *sr;

		/* normal N_CONN_REQ */
		if (!(sr = sccp_lookup_sr(sp, sc->sp.sp->add.pc, 1)))
			goto enomem;
		/* Figure C.2/Q.714: Figure 2A/T1.112.4-2000 1of4: Assign a local reference and an
		   SLS to the connection section. Determine proposed protocol class and credit. */
		for (tries = SCCP_SLR_MAX_TRIES; tries; tries--) {
			/* IMPLEMENTAION NOTE: This is where we would do labelling and offset for
			   SUA.  There would need to be a fixed SLR portion mask and value.  We
			   would then only select SLRs which are within the variable portion.  For
			   now, this uses the entire SLR numbering space. */
			slr = (sp->sccp_next_slr++) & 0xffff;
			if ((scp = sccp_co_lookup(sp, slr)) && !(*scp))
				break;
		}
		if (*scp)
			goto eagain2;
		/* we have CO slot, use it */
		if ((sc->sp.next = *scp))
			sc->sp.next->sp.prev = &sc->sp.next;
		sc->sp.prev = scp;
		*scp = sccp_get(sc);
		sc->sp.sp = sp_get(sp);
		/* connect to remote signalling point */
		if ((sc->sr.next = sr->sc.list))
			sc->sr.next->sr.prev = &sc->sr.next;
		sc->sr.prev = &sr->sc.list;
		sr->sc.list = sccp_get(sc);
		sc->sr.sr = sr_get(sr);
		fixme(("Figure out HOPC\n"));
		fixme(("Set selected sls\n"));
		if (sls) {
			sc->sls = *sls;
		} else {
			sls = &sc->sls;
		}
		if (pri) {
			sc->mp = *pri;
			if (sc->mp > 1) {
				mi_strlog(q, 0, SL_TRACE,
					  "ERROR: user selected message priority > 1");
				sc->mp = 1;
			}
		}
		if (pcl) {
			sc->pcl = *pcl;
			if (sc->pcl < 2) {
				mi_strlog(q, 0, SL_TRACE,
					  "ERROR: user selected protocol class < 2");
				sc->pcl = 2;
			}
			if (sc->pcl > 3) {
				mi_strlog(q, 0, SL_TRACE,
					  "ERROR: user selected protocol class > 3");
				sc->pcl = 3;
			}
		}
		if (imp) {
			sc->imp = *imp;
		}
		fixme(("Set selected slr\n"));
		if ((err =
		     sccp_send_cr(sp, q, NULL, sp->add.pc, sc->mp, sc->sls, sc->slr, sc->pcl,
				  &sc->dst, &sc->credit, &sc->src, dp, NULL, imp)))
			return (err);
	} else {
		/* Request Type 1 or Request Type 2 */
		/* Figure 2A/T1.112.4-2000 1of4: Request Type 1 has SLR, DLR, DPC, PCLASS and
		   credit already assigned (for permanent or semi-permanent connection). */
		/* Figure 2A/T1.112.4-2000 1of2: Request Type 2 has SLR, DLR, DPC, PCLASS and
		   credit already assigned (for permanent or semi-permanent connection). */
		slr = sc->slr;
		scp = sccp_co_lookup(sp, slr);
		if (scp && (*scp))
			goto eisconn;
	}
	sccp_timer_start(sc, tcon);
	sccp_set_state(sc, SS_WCON_CREQ);
	/* we have a blind slot, use it */
	sc->slr = slr;
	if ((sc->sp.next = *scp))
		sc->sp.next->sp.prev = &sc->sp.next;
	sc->sp.prev = scp;
	*scp = sccp_get(sc);
	sc->sp.sp = sp_get(sp);
	return (0);
      enomem:
	err = -ENOMEM;
	mi_strlog(q, 0, SL_TRACE, "ERROR: insufficient memory");
	goto error;
      eagain2:
	err = -EAGAIN;
	mi_strlog(q, 0, SL_TRACE, "ERROR: could not assign SLR");
	goto error;
      eisconn:
	err = -EISCONN;
	mi_strlog(q, 0, SL_TRACE, "ERROR: user already connected");
	goto error;
      efault:
	err = -EFAULT;
	mi_strlog(q, 0, SL_TRACE, "ERROR: connection request in wrong state");
	goto error;
      error:
	return (err);
}

/*
 *  SCCP CONN RES
 *  -------------------------------------------------------------------------
 */
static int
sccp_conn_res(struct sc *sc, queue_t *q, mblk_t *cp, struct sc *ap, mblk_t *dp)
{
	int err;
	int tries;
	unsigned short slr;
	struct sc **scp = NULL;		/* just to shut up compiler */
	struct sccp_msg *m = (struct sccp_msg *) cp->b_rptr;
	struct sp *sp;
	struct sr *sr;

	if ((1 << sccp_get_state(sc)) & ~(SSF_IDLE | SSF_WRES_CIND))
		goto efault;
	if (!(sp = sc->ss.ss->sp.sp))
		goto efault;
	if (ap->sp.sp)
		goto eisconn;
	/* Figure 2B/T1.112.4-2000 1of2: update protocol class and credit, assign local
	   referenceand SLS for incoming connection section, send CC, start activity timers and
	   move tot he active state.

	   Figure C.3/Q.714 Sheet 2 of 6: assign local reference, sls, protocol class and credit
	   for the incoming connection section, send a CC, start inactivity timers, move to the
	   active state. */
	fixme(("We should really get the user's protocol class and\n"
	       "credit preferences from the N_CONN_RES options\n"));
	for (tries = 0; tries < SCCP_SLR_MAX_TRIES; tries++) {
		slr = (sp->sccp_next_slr++) & 0xffff;
		if ((scp = sccp_co_lookup(sp, slr)) && !(*scp))
			break;
	}
	if (*scp)
		goto eagain2;
	if (ap->sp.sp)
		goto eisconn2;
	if (!(sr = sccp_lookup_sr(sp, sc->sp.sp->add.pc, 1)))
		goto enomem;
	/* we have CO slot, use it */
	if ((ap->sp.next = *scp))
		ap->sp.next->sp.prev = &ap->sp.next;
	ap->sp.prev = scp;
	*scp = sccp_get(ap);
	ap->sp.sp = sp_get(sp);
	/* connect to remote signalling point */
	if ((ap->sr.next = sr->sc.list))
		ap->sr.next->sr.prev = &ap->sr.next;
	ap->sr.prev = &sr->sc.list;
	sr->sc.list = sccp_get(ap);
	ap->sr.sr = sr_get(sr);
	if ((ap->pcl = m->pcl) == 3)
		ap->p_cred = m->cred;
	ap->dlr = m->slr;
	fixme(("copy more stuff and rebind ap if necessary"));
	switch (ap->proto.pvar & SS7_PVAR_MASK) {
	case SS7_PVAR_ANSI:
		if ((err =
		     sccp_send_cc(ap->sp.sp, q, NULL, ap->sp.sp->add.pc, ap->sls, ap->dlr, ap->slr,
				  ap->pcl, &ap->credit, &ap->dst, dp, NULL)))
			return (err);
		break;
	default:
		if ((err =
		     sccp_send_cc(ap->sp.sp, q, NULL, ap->sp.sp->add.pc, ap->sls, ap->dlr, ap->slr,
				  ap->pcl, &ap->credit, &ap->dst, dp, &ap->imp)))
			return (err);
		break;
	}
	fixme(("Assign subsystem\n"));
	sccp_timer_start(ap, tias);
	sccp_timer_start(ap, tiar);
	ap->state = SS_DATA_XFER;
	return (QR_DONE);
      enomem:
	err = -ENOMEM;
	mi_strlog(q, 0, SL_TRACE, "CONN_RES: insufficient memory");
	goto error;
      eisconn2:
	err = -EISCONN;
	mi_strlog(q, 0, SL_TRACE, "CONN_RES: user already connected");
	goto error;
      eagain2:
	mi_strlog(q, 0, SL_TRACE, "CONN_RES: slr assignment timed out");
	return (-EAGAIN);
	goto error;
      eisconn:
	err = -EISCONN;
	mi_strlog(q, 0, SL_TRACE, "CONN_RES: accepting stream already connected");
	goto error;
      efault:
	err = -EFAULT;
	mi_strlog(q, 0, SL_TRACE, "CONN_RES: connection response in wrong state");
	goto error;
      error:
	return (err);
}

static int
sccp_data_req(struct sc *sc, queue_t *q, mblk_t *bp, np_ulong exp, np_ulong more, np_ulong rctp,
	      mblk_t *dp)
{
	mblk_t *mp;
	struct sccp_msg *m;

	switch (sccp_get_state(sc)) {
	case SS_DATA_XFER:
		if (likely((mp = mi_allocb(q, sizeof(*m), BPRI_MED)) != NULL)) {
			DB_TYPE(mp) = M_DATA;
			mp->b_band = exp ? 1 : 0;
			m = (typeof(m)) mp->b_wptr;
			mp->b_wptr += sizeof(*m);
			bzero(m, sizeof(*m));
			m->type = (sc->pcl == 3) ? SCCP_MT_DT1 : SCCP_MT_DT2;
			fixme(("Fill out message structure\n"));
			m->rl.opc = sc->sp.sp->add.pc;
			m->rl.sls = sc->sls;
			m->rl.mp = sc->mp;
			m->parms = SCCP_PTF_DLR | SCCP_PTF_DATA;
			m->dlr = sc->dlr;
			m->data.ptr = dp->b_rptr;
			m->data.len = dp->b_wptr - dp->b_rptr;
			fixme(("Fill out message structure\n"));
			mp->b_cont = dp;
			/* Figure 2C/T1.112.4-2002 1of2: segment message, reset receive inactivity
			   timer, For class 3 peform sequence number function: if P(S) outside
			   window, queue segment, otherwise construct message and send it out.  For 
			   class 2 construct message and send it out.  Do this for each segment.
			   Figure C.4/Q.714 Sheet 1 of 4: We need to segment the NSDU if required
			   and assign a value to bit M and place the data in the send buffer
			   already segmented. For protocol class 3, for each segment in the M-bit
			   sequence, we need to assign the next value of value of P(S) to the data. 
			   If there are already messages in the send buffer, or P(S) is outside the 
			   sending window, we need to place the data at the end of the transmit
			   queue.  Otherwise we can send the message immediately. For protocol
			   class 2 and when we send a protocol class 3 message immediately, we send 
			   a DT1(class 2) or DT2(class 3) and restart the send inactivity timer.
			   This operation is performed for each message in the M-bit sequence. */
			if (bp)
				freeb(bp);
			bufq_queue(&sc->sndq, mp);
			return (0);
		}
		return (-ENOBUFS);
	case SS_BOTH_RESET:
		/* Figure 2E/T1.112.4-2000: dicard */
		/* Figure C.6/Q.714 Shee 2 of 4: discard */
		break;
	case SS_WCON_RREQ:
		/* Figure 2E/T1.112.4-2002 2of2: queue received information. */
		/* Figure C.6/Q.714 Sheet 2 of 4: Received information (whatever that means: see
		   figure). */
		mi_strlog(q, 0, SL_ERROR, "write this procedure");
		break;
	case SS_WRES_RIND:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2E/T1.112.4-2000 2of2: discard received information. */
			break;
		default:
			mi_strlog(q, 0, SL_ERROR, "invalid protocol variant");
			break;
		}
		break;
	default:
		mi_strlog(q, 0, SL_ERROR, "data received in invalid state");
		break;
	}
	if (bp)
		freeb(bp);
	freemsg(dp);
	return (0);
}

static int
sccp_exdata_req(struct sc *sc, queue_t *q, np_ulong more, np_ulong rctp, mblk_t *dp)
{
	int err;

	switch (sccp_get_state(sc)) {
	case SS_DATA_XFER:
		/* Figure 2D/T1.112.4-2000: same. */
		/* Figure C.5/Q.714 Sheet 1 of 2: If there is an unacknowledged ED, queue the
		   message, otherwise send the ED and restart send inactivity timer. */
		if (sc->flags & SCCPF_LOC_ED) {
			bufq_queue(&sc->urgq, dp);
			return (QR_TRIMMED);
		}
		if ((err =
		     sccp_send_ed(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, sc->sls, sc->dlr, dp)))
			return (err);
		sccp_timer_start(sc, tias);
		return (QR_TRIMMED);
	case SS_BOTH_RESET:
		/* Figure 2E/T1.112.4-2000: dicard */
		/* Figure C.6/Q.714 Shee 2 of 4: discard */
		return (QR_DONE);
	case SS_WCON_RREQ:
		/* Figure 2E/T1.112.4-2002 2of2: queue received information. */
		/* Figure C.6/Q.714 Sheet 2 of 4: Received information (whatever that means: see
		   figure). */
		fixme(("Write this procedure\n"));
		return (-EFAULT);
	case SS_WRES_RIND:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2E/T1.112.4-2000 2of2: discard received information. */
			return (QR_DONE);
		default:
			err = -EFAULT;
			swerr();
			return (err);
		}
	default:
		err = -EFAULT;
		swerr();
		return (err);
	}
}

static int
sccp_datack_req(struct sc *sc, queue_t *q)
{
	int err;

	switch (sccp_get_state(sc)) {
	case SS_DATA_XFER:
		if (sc->flags & SCCPF_REM_ED) {
			/* we are acking expedited data */
			/* Figure 2D/T1.112.4-2000: same. */
			/* Figure C.5/Q.714 Sheet 1 of 2: Send and EA and restart the send
			   inactivity timer. */
			if ((err =
			     sccp_send_ea(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, sc->sls, sc->dlr)))
				return (err);
			sccp_timer_start(sc, tias);
			sc->flags &= ~SCCPF_REM_ED;
			return (QR_DONE);
		} else {
			/* we are acking normal data */
			if (bufq_size(&sc->ackq)) {	/* we must have data to ack */
				/* Perform the processing deferred from when we received the data
				   message and queued it on the ack queue

				   Figure C.4/Q.714 Sheet 2 of 4: If the received P(S) is not the
				   next in sequence, or P(R) is outside the range from the last
				   received P(R) to the last P(S) sent plus 1, the an internal
				   reset (see figure) is peformed.

				   Otherwise, perform a Transmit Wakeup: If the received P(R) is at 
				   the lower edge of the sending window, the sending window is set
				   to the value of P(R). While we have a message in the transmit
				   queue, (we are flow controlled on the sending side) and the P(S) 
				   assigned to the message in queue is inside the send window, we
				   set P(R) to the last received P(S) plus 1 and send another of
				   the queued data messages and restart the send inactivity timer.
				   In this way we clear any backlog of messages once we receive
				   acknowledgement that opens the window.

				   Otherwise, if the received P(S) is equal to the upper edge of
				   the window, or if we need to send a data ack (acknowledged
				   service) then we construct a data ack. message with credit = W
				   and send it, restarting the send inactivity timer.

				   After this operation, we deliver the data to the user in an
				   N_DATA_IND.

				   IMPLEMENTATION NOTE:- We allow the user to explicitly
				   acknowledge receipt of data messages in protocol class 3.  To do 
				   this, we queue the message to an ack queue rather than
				   performing the P(S) P(R) processing on it yet. We postpone this
				   processing until a data ack request is received from the user. */
				fixme(("Write this procedure\n"));
				return (-EFAULT);
			}
		}
	default:
		err = -EFAULT;
		swerr();
		return (err);
	}
}

static int
sccp_reset_req(struct sc *sc, queue_t *q)
{
	switch (sccp_get_state(sc)) {
	case SS_DATA_XFER:
		/* Note: ANSI returns an N_RESET_CON immediately here.  That is strange. */
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2E/T1.112.4-2000 1of2: return N_RESET_CON, send RSR, start reset
			   timer, reset variables and discard all queued messages and
			   unacknowledged messages, move to the reset outgoing state. */
			fixme(("SWERR: write this function\n"));
			sccp_set_state(sc, SS_WCON_RREQ);
			return (-EFAULT);
		default:
		case SS7_PVAR_ITUT:
			/* Figure C.6/Q.714 Sheet 1 of 4: Send a reset request message to the perr, 
			   start the reset timer, restart the send inactivity timer, reset
			   variables and discard all queued and unacknowledged messages. Move to
			   the reset ongoing state. */
			fixme(("SWERR: write this function\n"));
			sccp_set_state(sc, SS_WCON_RREQ);
			return (-EFAULT);
		}
	case SS_BOTH_RESET:
		/* Figure 2E/T1.112.4-2000: same but no "received information" function. */
		/* Figure C.6/Q.714 Sheet 3 of 4: Received information (whatever that means), and
		   move to the reset outgoing state. */
		fixme(("Do received information function\n"));
		/* FIXME: We don't want to lose track of the fact that the reset was a both way
		   reset and not to send an N_RESET_CON to the user when the reset completes (for
		   ITU).  This state transition alone doesn't cut it. */
		sccp_set_state(sc, SS_WCON_RREQ);
		return (QR_DONE);
	case SS_WRES_RIND:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2E/T1.112.4-2000 2of2: transfer all queued information, move to
			   the active state. */
			fixme(("Transfer all queued information\n"));
			sccp_set_state(sc, SS_DATA_XFER);
			return (QR_DONE);
		default:
		case SS7_PVAR_ITUT:
			/* Figure C.6/Q.714 Sheet 4 of 4: received information (whatever that
			   means). */
			fixme(("Received information\n"));
			return (QR_DONE);
		}
	case SS_WCON_RREQ:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2E/T1.112.4-2002 2of2: send N_RESET_CON to the user, discard all
			   queue and unacknowledged messages. */
			fixme(("Write this function\n"));
			return (-EFAULT);
		default:
			swerr();
			return (-EFAULT);
		}
	default:
		swerr();
		return (-EFAULT);
	}
}

static int
sccp_reset_res(struct sc *sc, queue_t *q)
{
	switch (sccp_get_state(sc)) {
	case SS_BOTH_RESET:
		/* Figure 2E/T1.112.4-2000: same but no "received information" function. */
		/* Figure C.6/Q.714 Sheet 3 of 4: Received information (whatever that means), and
		   move to the reset outgoing state. */
		fixme(("Do received information function\n"));
		/* FIXME: We don't want to lose track of the fact that the reset was a both way
		   reset and not to send an N_RESET_CON to the user when the reset completes (for
		   ITU).  This state transition alone doesn't cut it. */
		sccp_set_state(sc, SS_WCON_RREQ);
		return (QR_DONE);
	case SS_WRES_RIND:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2E/T1.112.4-2000 2of2: transfer all queued information, move to
			   the active state. */
			fixme(("Transfer all queued information\n"));
			sccp_set_state(sc, SS_DATA_XFER);
			return (QR_DONE);
		default:
		case SS7_PVAR_ITUT:
			/* Figure C.6/Q.714 Sheet 4 of 4: received information (whatever that
			   means). */
			fixme(("Received information\n"));
			return (QR_DONE);
		}
	default:
		swerr();
		return (-EFAULT);
	}
}

static int
sccp_discon_req(struct sc *sc, queue_t *q, mblk_t *cp, mblk_t *dp)
{
	int err;

	if ((1 << sc->
	     state) & ~(SSF_IDLE | SSF_DATA_XFER | SSF_WRES_RIND | SSF_WCON_RREQ | SSF_WCON_CREQ |
			SSF_WRES_CIND))
		goto efault;
	switch (sccp_get_state(sc)) {
	case SS_IDLE:
	case SS_WRES_CIND:
	{
		/* Figure 2B/T1.112.4-2000 1of2: release allocated resources, send CREF, move to
		   the idle state. */
		/* Figure C.3/Q.714 Sheet 2 of 6: release resources, send CREF, move to the idle
		   state. */
		struct sccp_msg *m = (struct sccp_msg *) cp->b_rptr;

		fixme(("Select sr, but don't set sc->sr.sr\n"));
		sccp_send_cref(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, m->rl.sls, m->slr,
			       SCCP_REFC_END_USER_ORIGINATED, &sc->dst, dp,
			       (m->parms & SCCP_PTF_IMP ? &m->imp : NULL));
		sccp_set_state(sc, SS_IDLE);
		return (QR_DONE);
	}
	case SS_WCON_CREQ:
		/* Figure C.2/Q.714 Sheet 2 of 7: Figure 2A/T1.112.4-2000 Sheet 2 of 4: We need to
		   remember the fact that we got a disconnect request while an outgoing connection
		   was pending and wait for a connection confirmation anyway. When we receive the
		   connection confirmation we will disconnect. */
		sccp_set_state(sc, SS_WCON_CREQ2);
		return (QR_DONE);
	case SS_DATA_XFER:
	{
		/* Figure C.2/Q.714 Sheet 4 of 7: Figure C.3/Q.714 Sheet 3 of 6: Stop inactivity
		   timers, send RLSD, start release timer, move to disconnect pending state. */
		/* Figure 2A/T1.112.4-2000 Sheet 4 of 4: release resources for the connection
		   section, stop inactivity timers, send RLSD, start released and interval timers. */
		/* Notes: Although ANSI says to release resources for the connection section, it
		   does not talk about releasing the local reference (which is needed to receive
		   the RLC). */
		sccp_timer_stop(sc, tias);
		sccp_timer_stop(sc, tiar);
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					    sc->slr, SCCP_RELC_END_USER_ORIGINATED, dp, NULL)))
				return (err);
			sccp_timer_start(sc, trel);
			sccp_timer_start(sc, tint);
			break;
		default:
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					    sc->slr, SCCP_RELC_END_USER_ORIGINATED, dp, &sc->imp)))
				return (err);
			sccp_timer_start(sc, trel);
			break;
		}
		sccp_set_state(sc, SS_WCON_DREQ);
		return (QR_DONE);
	}
	}
      efault:
	fixme(("SWERR: write this function\n"));
	return (-EFAULT);
}

static int
sccp_ordrel_req(struct sc *sc, queue_t *q)
{
	fixme(("SWERR: write this function\n"));
	return (-EFAULT);
}

static int
sccp_unitdata_req(struct sc *sc, queue_t *q, struct sccp_addr *dst, np_ulong pcl, np_ulong opt,
		  np_ulong imp, np_ulong seq, np_ulong pri, mblk_t *dp)
{
	/* Figure 3/T1.112.4-2000 1of2: assign an sls based on the sequence parameter in the
	   N-UNITDATA Request, construct an [X/L]UDT message, deciding which message to use based
	   on local information, decide whether segmentation is necessary and if necessary segment
	   the message, send the resulting messages to routing control for routing */
	/* IMPLEMENTATION NOTE:- Actually we don't care at this point whether the message needs to
	   be segmented or not: we will decide that after routing and perform segmentation during
	   routing if necessary.  Also, we build just a UDT here.  If global title is performed and 
	   a hop counter is required, the message will be converted into an XUDT.  If segmentation
	   is required the message will be converted into an XUDT/LUDT when the need for
	   segementation is determined after routing. */
	fixme(("SWERR: write this function\n"));
	return (-EFAULT);
}

static int
sccp_inform_req(struct sc *sc, queue_t *q, N_qos_sel_infr_sccp_t * qos, np_ulong reason)
{
	fixme(("SWERR: write this function\n"));
	return (-EFAULT);
}

static int
sccp_coord_req(struct sc *sc, queue_t *q, struct sccp_addr *add)
{
	fixme(("SWERR: write this function\n"));
	return (-EFAULT);
}

static int
sccp_coord_res(struct sc *sc, queue_t *q, struct sccp_addr *add)
{
	fixme(("SWERR: write this function\n"));
	return (-EFAULT);
}

#define SSNF_IGNORE_SST_IN_PROGRESS	0x0001
#define SSNF_WAIT_FOR_GRANT		0x0002
static int
sccp_state_req(struct sc *sc, queue_t *q, struct sccp_addr *add, np_ulong status)
{
	int err;
	struct na *na;
	struct sp *sp;
	struct ss *ss;
	np_ulong smi = 0;		/* FIXME */

	if (!add->ni || !(na = na_lookup(add->ni)))
		goto enoaddr;
	if (!add->pc || !(sp = sccp_lookup_sp(add->ni, add->pc, 1)))
		goto enoaddr;
	if (!add->ssn || !(ss = sccp_lookup_ss(sp, add->ssn, 1)))
		goto enoaddr;
	switch (status) {
	case N_SCCP_STATUS_USER_IN_SERVICE:
		if (ss->state != SSN_ALLOWED) {
			struct sr *sr;

			/* broadcast SSA */
			for (sr = sp->sr.list; sr; sr = sr->sp.next) {
				/* send SSA */
				struct sccp_addr cdpa, cgpa;

				cdpa.ni = add->ni;
				cdpa.ri = SCCP_RI_DPC_SSN;
				cdpa.pc = sr->add.pc;
				cdpa.ssn = 1;
				cdpa.gtt = 0;
				cdpa.alen = 0;
				cgpa.ni = sp->add.ni;
				cgpa.ri = SCCP_RI_DPC_SSN;
				cgpa.pc = sp->add.pc;
				cgpa.ssn = 1;
				cgpa.gtt = 0;
				cgpa.alen = 0;
				if ((err =
				     sccp_send_ssa(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, &cdpa,
						   &cgpa, add->ssn, sp->add.pc, smi)) < 0)
					goto error;
			}
			/* local broadcast UIS (including GTT function) */
			if ((err = sccp_state_lbr(q, add, status, smi)) < 0)
				goto error;
			ss->state = SSN_ALLOWED;
		}
		break;
	case N_SCCP_STATUS_USER_OUT_OF_SERVICE:
		/* See ANSI Figure 8/T1.112.4-2000 sheet 1of1 */
		if (ss->state != SSN_PROHIBITED) {
			struct sr *sr;

			if (ss->flags & SSNF_IGNORE_SST_IN_PROGRESS) {
				ss_timer_stop(ss, tisst);
				ss->flags &= ~SSNF_IGNORE_SST_IN_PROGRESS;
			} else if (ss->flags & SSNF_WAIT_FOR_GRANT) {
				ss_timer_stop(ss, twsog);
				ss->flags &= ~SSNF_WAIT_FOR_GRANT;
			}
			/* broadcast SSP */
			for (sr = sp->sr.list; sr; sr = sr->sp.next) {
				/* send SSP */
				struct sccp_addr cdpa, cgpa;

				cdpa.ni = add->ni;
				cdpa.ri = SCCP_RI_DPC_SSN;
				cdpa.pc = sr->add.pc;
				cdpa.ssn = 1;
				cdpa.gtt = 0;
				cdpa.alen = 0;
				cgpa.ni = sp->add.ni;
				cgpa.ri = SCCP_RI_DPC_SSN;
				cgpa.pc = sp->add.pc;
				cgpa.ssn = 1;
				cgpa.gtt = 0;
				cgpa.alen = 0;
				if ((err =
				     sccp_send_ssp(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, &cdpa,
						   &cgpa, add->ssn, sp->add.pc, smi)) < 0)
					goto error;
			}
			/* local broadcast UOS (including GTT function) */
			if ((err = sccp_state_lbr(q, add, status, smi)) < 0)
				goto error;
			ss->state = SSN_PROHIBITED;
		}
		break;
	default:
		swerr();
		goto efault;
	}
	return (QR_DONE);
      enoaddr:
	err = -EADDRNOTAVAIL;
	rare();
	goto error;
      efault:
	err = -EFAULT;
	swerr();
	goto error;
      error:
	return (err);
}

/*
 *  =========================================================================
 *
 *  EVENTS
 *
 *  =========================================================================
 */

/*
 *  -------------------------------------------------------------------------
 *
 *  Primitives from below
 *
 *  -------------------------------------------------------------------------
 */

#define SCCPF_LOC_S_BLOCKED	0x01
/*
 *  M_ERROR, M_HANGUP
 *  -----------------------------------
 */
#if 0
static int
mtp_hangup(struct mt *mt)
{
	if (mt->sr) {
		struct sr *sr = mt->sr;

		if (!(sr->flags & SCCPF_LOC_S_BLOCKED)) {
			struct ss *ss;

			for (ss = sr->sp.sp->ss.list; ss; ss = ss->sp.next) {
				ss->flags |= SCCPF_LOC_S_BLOCKED;
			}
			sr->flags |= SCCPF_LOC_S_BLOCKED;
		}
	} else if (mt->sp) {
		struct sr *sr;
		struct sp *sp = mt->sp;

		for (sr = sp->sr.list; sr; sr = sr->sp.next) {
			if (!(sr->flags & SCCPF_LOC_S_BLOCKED)) {
				struct ss *ss;

				for (ss = sp->ss.list; ss; ss = ss->sp.next) {
					ss->flags |= SCCPF_LOC_S_BLOCKED;
				}
				sr->flags |= SCCPF_LOC_S_BLOCKED;
			}
		}
	} else {
		/* mt lower stream is not attached to a signalling point */
		rare();
	}
	fixme(("Notify management of failed lower stream\n"));
	return (QR_DISABLE);
}
#endif

/*
 *  M_DATA
 *  -----------------------------------
 */
static int
mtp_read(struct mt *mt, queue_t *q, mblk_t *bp, mblk_t *dp, struct sr *sr, np_ulong pri,
	 np_ulong sls)
{
	int err;
	mblk_t *mp = NULL;
	struct sccp_msg *m;
	uchar *p, *e;

	if (!dp)
		goto efault;
	if (dp->b_cont && !ss7_pullupmsg(q, dp, -1))
		goto enobufs;
	p = dp->b_rptr;
	e = dp->b_wptr;
	if (p + 1 > e)
		goto emsgsize;
	if (unlikely((mp = mi_allocb(q, sizeof(*m), BPRI_MED)) == NULL))
		return (-ENOBUFS);
	DB_TYPE(mp) = M_RSE;
	m = (typeof(m)) mp->b_wptr;
	mp->b_wptr += sizeof(*m);
	bzero(m, sizeof(*m));
	mp->b_cont = dp;
	m->bp = mp;
	m->eq = NULL;
	m->xq = q;
	m->sc = NULL;
	m->mt = NULL;
	m->timestamp = jiffies;
	m->pvar = mt->sp->proto.pvar;
	m->rl.opc = sr->add.pc;
	m->rl.dpc = mt->sp->add.pc;
	m->rl.mp = pri;
	m->rl.sls = sls;
	m->type = *p++;
	if (p >= e)
		goto emsgsize;
	switch (m->type) {
	case SCCP_MT_CR:
		err = sccp_dec_cr(p, e, m);
		break;
	case SCCP_MT_CC:
		err = sccp_dec_cc(p, e, m);
		break;
	case SCCP_MT_CREF:
		err = sccp_dec_cref(p, e, m);
		break;
	case SCCP_MT_RLSD:
		err = sccp_dec_rlsd(p, e, m);
		break;
	case SCCP_MT_RLC:
		err = sccp_dec_rlc(p, e, m);
		break;
	case SCCP_MT_DT1:
		err = sccp_dec_dt1(p, e, m);
		break;
	case SCCP_MT_DT2:
		err = sccp_dec_dt2(p, e, m);
		break;
	case SCCP_MT_AK:
		err = sccp_dec_ak(p, e, m);
		break;
	case SCCP_MT_UDT:
		err = sccp_dec_udt(p, e, m);
		break;
	case SCCP_MT_UDTS:
		err = sccp_dec_udts(p, e, m);
		break;
	case SCCP_MT_ED:
		err = sccp_dec_ed(p, e, m);
		break;
	case SCCP_MT_EA:
		err = sccp_dec_ea(p, e, m);
		break;
	case SCCP_MT_RSR:
		err = sccp_dec_rsr(p, e, m);
		break;
	case SCCP_MT_RSC:
		err = sccp_dec_rsc(p, e, m);
		break;
	case SCCP_MT_ERR:
		err = sccp_dec_err(p, e, m);
		break;
	case SCCP_MT_IT:
		err = sccp_dec_it(p, e, m);
		break;
	case SCCP_MT_XUDT:
		err = sccp_dec_xudt(p, e, m);
		break;
	case SCCP_MT_XUDTS:
		err = sccp_dec_xudts(p, e, m);
		break;
	case SCCP_MT_LUDT:
		err = sccp_dec_ludt(p, e, m);
		break;
	case SCCP_MT_LUDTS:
		err = sccp_dec_ludts(p, e, m);
		break;
	default:
		/* any message with an unknown message type is discarded */
		err = -ENOPROTOOPT;
		break;
	}
	/* handle return value */
	if (err < 0 || (err = sccp_irte_msg(mt, q, bp, mp)) < 0) {
		switch (err) {
		case -EOPNOTSUPP:	/* unexpected message */
			mi_strlog(q, 0, SL_ERROR, "unexpected message");
			break;
		case -EPROTONOSUPPORT:	/* parameter type error */
			mi_strlog(q, 0, SL_ERROR, "parameter type error");
			break;
		case -ENOPROTOOPT:	/* message type error */
			mi_strlog(q, 0, SL_ERROR, "message type error");
			/* ANSI T1.112.4-2000 Table B-1/T1.112.4: any message with an unknown
			   message type is discarded */
			break;
		}
		goto error;
	}
	return (err);
      emsgsize:
	err = -EMSGSIZE;
	mi_strlog(q, 0, SL_TRACE, "message size deconding error");
	goto error;
      enobufs:
	err = -ENOBUFS;
	swerr();
	goto error;
      efault:
	err = -EFAULT;
	swerr();
	goto error;
      error:
	if (mp)
		freeb(mp);
	if (bp)
		freeb(bp);
	freemsg(dp);
	return (err);
}

/*
 *  MTP_OK_ACK
 *  -----------------------------------
 *  We shouldn't get these.  All requests which require an MTP_OK_ACK must be performed before the stream is linked.
 */
static int
mtp_ok_ack(struct mt *mt, queue_t *q, mblk_t *mp)
{
	struct MTP_ok_ack *p = (typeof(p)) mp->b_rptr;

	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto emsgsize;
	goto efault;
      efault:
	swerr();
	return -EFAULT;
      emsgsize:
	swerr();
	return -EMSGSIZE;
}

/*
 *  MTP_ERROR_ACK
 *  -----------------------------------
 *  We shouldn't get these; however, we attempt a MTP_ADDR_REQ when we link to get information about the MTP we have
 *  linked.  This might be an error response to our request.
 */
static int
mtp_error_ack(struct mt *mt, queue_t *q, mblk_t *mp)
{
	struct MTP_error_ack *p = (typeof(p)) mp->b_rptr;

	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto emsgsize;
	goto efault;
      efault:
	swerr();
	return -EFAULT;
      emsgsize:
	swerr();
	return -EMSGSIZE;
}

/*
 *  MTP_BIND_ACK
 *  -----------------------------------
 */
static int
mtp_bind_ack(struct mt *mt, queue_t *q, mblk_t *mp)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}

/*
 *  MTP_ADDR_ACK
 *  -----------------------------------
 *  This is a response to our request for the addresses bound and connected to the MTP stream after linking.  What
 *  we want to do is find the associated Signalling Point structure for the local signalling point or Signalling
 *  Relation structure for the local and remote signalling point.  MTP streams can be bound in two fashions: with
 *  local signalling point code and wildcarded remote signalling point code, or with both local and remote
 *  signalling points specified.
 */
static int
mtp_addr_ack(struct mt *mt, queue_t *q, mblk_t *mp)
{
	struct sp *sp;
	struct sr *sr = NULL;
	struct MTP_addr_ack *p = (typeof(p)) mp->b_rptr;
	uchar *loc_ptr, *rem_ptr;
	size_t loc_len, rem_len;
	struct mtp_addr *loc, *rem;

	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto emsgsize;
	loc_ptr = mp->b_rptr + p->mtp_loc_offset;
	if ((loc_len = p->mtp_loc_length) < sizeof(*loc))
		goto emsgsize;
	if (mp->b_wptr < mp->b_rptr + loc_len)
		goto emsgsize;
	rem_ptr = mp->b_rptr + p->mtp_rem_offset;
	if ((rem_len = p->mtp_loc_length) < sizeof(*rem))
		goto emsgsize;
	if (mp->b_wptr < mp->b_rptr + rem_len)
		goto emsgsize;
	if (loc_len && loc_len < sizeof(*loc))
		goto efault;
	if (rem_len && rem_len < sizeof(*rem))
		goto efault;
	loc = (typeof(loc)) loc_ptr;
	rem = (typeof(rem)) rem_ptr;
	if (!loc_len)
		goto efault;
	for (sp = master.sp.list;
	     sp && (sp->add.ni != loc->ni || sp->add.pc != loc->pc || sp->add.si != loc->si);
	     sp = sp->next) ;
	if (!sp)
		goto notfound;
	if (!rem_len)
		goto noremote;
	for (sr = master.sr.list;
	     sr && (sr->add.ni != rem->ni || sr->add.pc != rem->pc || sr->add.si != rem->si);
	     sr = sr->next) ;
	if (!sr)
		goto notfound;
	unless(sr->mt || sp->mt, goto ebusy);
	sr->mt = mtp_get(mt);
	unless(mt->sr, goto efault);	/* FIXME */
	mt->sr = sr_get(sr);	/* FIXME */
	fixme(("Set proper state for SR and SP\n"));
	return (QR_DONE);
      noremote:
	unless(sp->mt, goto ebusy);
	sp->mt = mtp_get(mt);
	unless(mt->sp, goto efault);	/* FIXME */
	mt->sp = sp_get(sp);	/* FIXME */
	fixme(("Set proper state for SR and SP\n"));
	return (QR_DONE);
      notfound:
	noenable(mt->iq);
	noenable(mt->oq);
	swerr();
	mi_strlog(q, 0, SL_ERROR, "MTP_ADDR_ACK: MTP provider stream already linked");
	fixme(("Need to inform layer management to unlink stream\n"));
	return -EINVAL;
#if defined(_SAFE)||defined(_DEBUG)
      ebusy:
	noenable(mt->iq);
	noenable(mt->oq);
	swerr();
	mi_strlog(q, 0, SL_ERROR, "MTP_ADDR_ACK: MTP provider stream already linked");
	fixme(("Need to inform layer management to unlink stream\n"));
	return -EBUSY;
#endif
      efault:
	noenable(mt->iq);
	noenable(mt->oq);
	swerr();
	mi_strlog(q, 0, SL_ERROR, "MTP_ADDR_ACK: MTP provider stream already linked");
	fixme(("Need to inform layer management to unlink stream\n"));
	return -EFAULT;
      emsgsize:
	noenable(mt->iq);
	noenable(mt->oq);
	swerr();
	mi_strlog(q, 0, SL_ERROR, "MTP_ADDR_ACK: bad message size");
	fixme(("Need to inform layer management to unlink stream\n"));
	return -EMSGSIZE;
}

/*
 *  MTP_INFO_ACK
 *  -----------------------------------
 *  We can use this instead of MTP_ADDR_ACK which has less information in it.
 */
static int
mtp_info_ack(struct mt *mt, queue_t *q, mblk_t *mp)
{
	struct sp *sp;
	struct sr *sr = NULL;
	struct MTP_info_ack *p = (typeof(p)) mp->b_rptr;
	uchar *add_ptr, *loc_ptr = NULL, *rem_ptr = NULL;
	size_t add_len, loc_len = 0, rem_len = 0;
	struct mtp_addr *loc, *rem;

	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto emsgsize;
	add_ptr = mp->b_rptr + p->mtp_addr_offset;
	add_len = p->mtp_addr_length;
	if (mp->b_wptr < add_ptr + add_len)
		goto emsgsize;
	if (add_len) {
		if (add_len >= sizeof(*loc)) {
			loc_ptr = add_ptr;
			loc_len = sizeof(*loc);
			add_ptr += sizeof(*loc);
			add_len -= sizeof(*loc);
		} else if (add_len)
			goto efault;
	}
	if (add_len) {
		if (add_len >= sizeof(*rem)) {
			rem_ptr = add_ptr;
			rem_len = sizeof(*rem);
			add_ptr += sizeof(*rem);
			add_len -= sizeof(*rem);
		} else if (add_len)
			goto efault;
	}
	loc = (typeof(loc)) loc_ptr;
	rem = (typeof(rem)) rem_ptr;
	if (!loc)
		goto efault;
	for (sp = master.sp.list;
	     sp && (sp->add.ni != loc->ni || sp->add.pc != loc->pc || sp->add.si != loc->si);
	     sp = sp->next) ;
	if (!sp)
		goto notfound;
	if (!rem)
		goto noremote;
	for (sr = master.sr.list;
	     sr && (sr->add.ni != rem->ni || sr->add.pc != rem->pc || sr->add.si != rem->si);
	     sr = sr->next) ;
	if (!sr)
		goto notfound;
	unless(sr->mt || sp->mt, goto ebusy);
	unless(mt->sr, goto efault);	/* FIXME */
	sr->mt = mtp_get(mt);
	mt->sr = sr_get(sr);	/* FIXME */
	fixme(("Set proper state for SR and SP\n"));
	return (QR_DONE);
      noremote:
	unless(sp->mt, goto ebusy);
	unless(mt->sp, goto efault);	/* FIXME */
	sp->mt = mtp_get(mt);
	mt->sp = sp_get(sp);	/* FIXME */
	fixme(("Set proper state for SR and SP\n"));
	return (QR_DONE);
      notfound:
	noenable(mt->iq);
	noenable(mt->oq);
	swerr();
	mi_strlog(q, 0, SL_ERROR, "MTP_INFO_ACK: MTP provider stream already linked");
	fixme(("Need to inform layer management to unlink stream\n"));
	return -EINVAL;
#if defined(_SAFE)||defined(_DEBUG)
      ebusy:
	noenable(mt->iq);
	noenable(mt->oq);
	swerr();
	mi_strlog(q, 0, SL_ERROR, "MTP_INFO_ACK: MTP provider stream already linked");
	fixme(("Need to inform layer management to unlink stream\n"));
	return -EBUSY;
#endif
      efault:
	noenable(mt->iq);
	noenable(mt->oq);
	swerr();
	mi_strlog(q, 0, SL_ERROR, "MTP_INFO_ACK: MTP provider stream already linked");
	fixme(("Need to inform layer management to unlink stream\n"));
	return -EFAULT;
      emsgsize:
	noenable(mt->iq);
	noenable(mt->oq);
	swerr();
	mi_strlog(q, 0, SL_ERROR, "MTP_INFO_ACK: MTP provider stream already linked");
	fixme(("Need to inform layer management to unlink stream\n"));
	return -EMSGSIZE;
}

/*
 *  MTP_OPTMGMT_ACK
 *  -----------------------------------
 *  We shouldn't get these.  MTP streams must have options set before they are linked under the SCCP multiplexing
 *  driver.
 */
static int
mtp_optmgmt_ack(struct mt *mt, queue_t *q, mblk_t *mp)
{
	struct MTP_optmgmt_ack *p = (typeof(p)) mp->b_rptr;

	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto emsgsize;
	goto efault;
      efault:
	swerr();
	return (-EFAULT);
      emsgsize:
	swerr();
	return (-EMSGSIZE);
}

/*
 *  MTP_TRANSFER_IND
 *  -----------------------------------
 *  This is data.  It should normally come to use, however, as M_DATA blocks rather than MTP_TRANSFER_IND when SCCP
 *  is bound connection oriented (local and remote point codes fixed).  This is normal for GSM-A.
 */
static int
mtp_transfer_ind(struct mt *mt, queue_t *q, mblk_t *mp)
{
	int err;
	struct sr *sr;
	struct mtp_addr add;
	struct MTP_transfer_ind *p = (typeof(p)) mp->b_rptr;

	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto emsgsize;
	if (mp->b_wptr < mp->b_rptr + p->mtp_srce_offset + p->mtp_srce_length)
		goto emsgsize;
	if (p->mtp_srce_length < sizeof(add))
		goto badaddr;
	if (!mp->b_cont)
		goto baddata;
	bcopy(mp->b_rptr + p->mtp_srce_offset, &add, sizeof(add));
	if (!mt->sp)
		sr = mt->sr;
	else
		for (sr = mt->sp->sr.list; sr && sr->id != add.pc; sr = sr->sp.next) ;
	if (!sr)
		goto eproto;
	return mtp_read(mt, q, mp, mp->b_cont, sr, p->mtp_mp, p->mtp_sls);
      baddata:
	err = -EFAULT;
	pswerr(("%s: %p: SWERR: no data\n", DRV_NAME, mt));
	goto error;
      badaddr:
	err = -EFAULT;
	pswerr(("%s: %p: SWERR: bad address\n", DRV_NAME, mt));
	goto error;
      emsgsize:
	err = -EFAULT;
	pswerr(("%s: %p: SWERR: bad message size\n", DRV_NAME, mt));
	goto error;
      eproto:
	err = -EPROTO;
	fixme(("Handle message from SP not known to SCCP\n"));
	goto error;
      error:
	return (err);
}

/*
 *  MTP_PAUSE_IND
 *  -----------------------------------
 *  ANSI T1.112.4 5.2.2 Signalling Point Prohibited.  When SCCP management receives an MTP-PAUSE indication relating
 *  to a destination that becomes inaccessible, SCCP management: (1) markes as "prohibited" the status of that
 *  signalling point; (2) intitiates a local broadcast (5.3.6) of "signalling point inaccessible" information for
 *  that signalling point; (3) [optionally] discontinues any SCCP status test for that signalling point; (4)
 *  [optionally] marks as "prohibited" the status of that signalling point's SCCP; (5) [optionally] initiates a
 *  local broadcast (5.3.6) of "SCCP inaccessible" information for that signalling point's SCCP; (6) if the
 *  indicated signalling point is a preferred node, marks the translation as "translate to next preferred node" if
 *  that signalling point has a backup; or, if the indicated signalling point is a loadshare node, marks the
 *  translation as "translate to the remaining available loadshare nodes."  Otherwise, marks the affected node
 *  inaccessible.  (7) marks as "prohibited" the status of each subsystem at that signalling point; (8) discontinues
 *  any subsystem status tests (Section 5.3.4) it may be conducting to any subsystems at that signalling point; (9)
 *  if the signalling point is a preferred node, marks the translation as "translate to next preferred subsystem"
 *  for each subsystem at that signalling point for which a backup subsystem exists; or, if the indicated signalling
 *  point is a loadshare node, marks the translation as 'translate to the remaining available loadshare subsystems'
 *  for each subsystem at that signalling point for which a loadshare subsystem exists.  Otherwise, marks the
 *  affected subsystems as inaccessible.  (10) initiates a local broadcast (Section 5.3.6) of "User-out-of-service"
 *  information for each subsystem at that signalling point; (11) if the MTP-PAUSE pertains to the preferred node
 *  and traffic will be diverted to the next preferred node, sends a Subsystem-Backup-Routing message regarding each
 *  replicated subsystem at that signalling point to SCCP management at the location of the corresponding next
 *  preferred subsystem.  (This action is taken only if the node receiving the MTP-PAUSE is a "translator node" that
 *  is adjacent to the node at which the next preferred subsystem is located.  For example, a signalling transfer
 *  point which learns that an adjacent databse has failed sends a Subsystem-Backup-Routing message to SCCP
 *  management for each next preferred subsystem at the next preferred node.  If the singalling transfer point is
 *  not adjacent to the allowed next preferred subsystem, it does not send Subsystem-Backup-Routing messages); (12)
 *  makrs all local equipped duplicated subsystems back routed from the failed signalling point, if the failed
 *  signalling point is an adjacent translator node; (13) initiates the traffic-mix information procedure
 *  (5.4.2.1.1) to the local allowed users if the failed signalling point is an adjacent translator node; (14) stops
 *  all subsystem routing status tests for the failed signalling point (5.4.4).
 *
 *  ANSI Figure 5/T1.112.4-2000.
 */
static int
mtp_pause_ind(struct mt *mt, queue_t *q, mblk_t *mp)
{
	int err;
	struct sr *sr;
	struct sp *sp;
	struct ss *ss;
	struct sc *sc;
	struct mtp_addr *a;
	struct MTP_pause_ind *p = (typeof(p)) mp->b_rptr;

	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto emsgsize;
	if (mp->b_wptr < mp->b_rptr + p->mtp_addr_offset + p->mtp_addr_length)
		goto emsgsize;
	a = (typeof(a)) (mp->b_rptr + p->mtp_addr_offset);
	if (!(sp = mt->sp))
		sr = mt->sr;
	else
		for (sr = sp->sr.list; sr && sr->add.pc != a->pc; sr = sr->sp.next) ;
	if (sr) {
		/* ANSI Figure 5/T1.112.4-2000 1of3: mark sp prohibited; local broadcast of "SP
		   inactive", mark sc prohibited, local broadcast of SCCP Inactive, update PC
		   translation; for each subsystem stop SST; mark subsystem prohibited; update
		   subsystem translation information; local broadcast of "UOS" */
		if (!sp)
			sp = sr->sp.sp;
		if (!(sr->flags & SCCPF_PROHIBITED)) {
			np_ulong smi = 0;	/* FIXME */

			/* local broadcast SP inactive (N-PCSTATE) */
			if ((err =
			     sccp_pcstate_lbr(q, a, N_SCCP_STATUS_SIGNALLING_POINT_INACCESSIBLE)))
				return (err);
			if (!(sr->flags & SCCPF_PROHIBITED)) {
				struct rs *rs;
				struct sccp_addr add;

				add.ni = a->ni;
				add.ri = SCCP_RI_DPC_SSN;
				add.pc = a->pc;
				add.gtt = 0;
				add.alen = 0;
				/* local broadcast of SCCP Inactive */
				for (sc = sp->gt.list; sc; sc = sc->sp.next) {
				}
				for (ss = sp->ss.list; ss; ss = ss->sp.next) {
					for (sc = ss->cl.list; sc; sc = sc->ss.next) {
					}
					for (sc = ss->cr.list; sc; sc = sc->ss.next) {
					}
					for (sc = ss->co.list; sc; sc = sc->ss.next) {
					}
				}
				for (rs = sr->rs.list; rs; rs = rs->sr.next) {
					add.ssn = rs->ssn;
					if (!(rs->flags & SCCPF_PROHIBITED)) {
						/* local broadcast of "UOS" N-STATE */
						if ((err =
						     sccp_state_lbr(q, &add,
								    N_SCCP_STATUS_USER_OUT_OF_SERVICE,
								    smi)))
							return (err);
						rs->flags |= SCCPF_PROHIBITED;
					} else if (rs->flags & SCCPF_SUBSYSTEM_TEST) {
						rs_timer_stop(rs, trsst);
						rs->flags &= ~SCCPF_SUBSYSTEM_TEST;
					}
				}
				sr->flags |= SCCPF_PROHIBITED;
			} else if (sr->flags & SCCPF_SUBSYSTEM_TEST) {
				sr_timer_stop(sr, tsst);
				sr->flags &= ~SCCPF_SUBSYSTEM_TEST;
			}
			sr->flags |= SCCPF_PROHIBITED;
		}
		if (!(sr->flags & SCCPF_LOC_S_BLOCKED)) {
			struct ss *ss;

			for (ss = sr->sp.sp->ss.list; ss; ss = ss->sp.next) {
				fixme(("Need to send N-PCSTATUS to ss\n"));
				ss->flags |= SCCPF_LOC_S_BLOCKED;
			}
			sr->flags |= SCCPF_LOC_S_BLOCKED;
		}
	}
	return (QR_DONE);
      emsgsize:
	swerr();
	return (-EMSGSIZE);
}

/*
 *  MTP_RESUME_IND
 *  -----------------------------------
 *  ANSI T1.112.4 5.2.3 Signalling Point Allowed.  When SCCP management receives an MTP-RESUME indicaiton relating
 *  to a destination that becomes accessible, SCCP management: (1) marks as "allowed" the status of that signalling
 *  point; (2) resets the congesiton level of the signalling point; (3) initiates a local broadcast (5.3.6) of
 *  "signalling point accessible" information for that signalling point; (4) [optionally] discontines any SCCP
 *  status test for that signalling point; (5) [optionally] marks as "allowed" the status of the signalling point's
 *  SCCP; (6) [optionally] initiates a local broadcast (5.3.6) of "SCCP accessible" information for the signalling
 *  point's SCCP; (7) marks the translation as "translate to previously inaccessible higher priority
 *  node/subsystem," if the signalling point is a replicated higher priority node, or, marks the translation as
 *  'translate to remaining available loadshare nodes', if the signalling point is a loadshare node. (8) performs
 *  either action (a) or (b) for each subsystem; (a) except for the selected replicated subsystem (8b); i. marks as
 *  "allowed" the status of each remote subsystem; ii. initiates a local broadcast (5.3.6) of "User-in-service"
 *  information for each subssytem at the signalling point; iii. marks the translation as "translate to previously
 *  inaccessible higher priority subsystem" for each replicated subsystem at the signalling point if the signalling
 *  point contains replicated higher prioirty subsystems, or, marks the translation as 'translate to all available
 *  loadshare subsystems' for each loadshare subsystem at that signalling point. or (b) As a network provider
 *  option, for selected replicated subsystems (operating in either dominant or the loadshare mode), the subsystem
 *  status test procedure is intiated. (9) inities, if the recovered signalling point is an adjacnet translator
 *  node; (a) the subsystem routing status test procedure (5.4.4) for all local equipped duplicated subsystems; (b)
 *  the traffic-mix information procedure (5.4.2.1.1) to the local allowed subsystems.
 */
static int
mtp_resume_ind(struct mt *mt, queue_t *q, mblk_t *mp)
{
	struct sr *sr;
	struct mtp_addr *a;
	struct MTP_resume_ind *p = (typeof(p)) mp->b_rptr;

	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto emsgsize;
	if (mp->b_wptr < mp->b_rptr + p->mtp_addr_offset + p->mtp_addr_length)
		goto emsgsize;
	a = (typeof(a)) (mp->b_rptr + p->mtp_addr_offset);
	if (!mt->sp)
		sr = mt->sr;
	else
		for (sr = mt->sp->sr.list; sr && sr->add.pc != a->pc; sr = sr->sp.next) ;
	if (sr) {
		if (sr->flags & SCCPF_LOC_S_BLOCKED) {
			struct ss *ss;

			for (ss = sr->sp.sp->ss.list; ss; ss = ss->sp.next) {
				fixme(("Need to send N-PCSTATUS to ss\n"));
				ss->flags |= SCCPF_LOC_S_BLOCKED;
			}
			sr->flags &= ~SCCPF_LOC_S_BLOCKED;
		}
	}
	return (QR_DONE);
      emsgsize:
	swerr();
	return (-EMSGSIZE);
}

/*
 *  MTP_STATUS_IND
 *  -----------------------------------
 *  ANSI T1.111.4/2000 5.2.4 Signalling Point Congestion.  When SCCP management receives an MTP-STATUS indicaiton
 *  relating to singalling network congestion to a signalling point, SCCP mangement: (1) updates that signalling
 *  point status to reflect the congesiton; (2) intiates a local broadcast (5.3.6) of "signalling point congested"
 *  information for that signalling point.
 *
 *  The congestion status of a signalling point may be used during translation to determine whether to route a
 *  received Class 0 message to the preferred node/subsystem or to the next preferred node/subsystem if the option
 *  is selected to alternate route traffic because of signalling point congestion for the dominant mode (defined ona
 *  per subsystem number basis for replicated subsystems operating in the dominant mode).  If the loadshare mode is
 *  in use, the congestion status of a signalling point may also be used during translation to determine if any of
 *  the loadshare node/subsystem should NOT receive a Class 0 message due to congesiton.
 *
 *  ANSI T1.112.4/2000 5.5.2 SCCP Prohibited Control.  When SCCP management receives an MTP-STATUS primitive
 *  indicating that the status of a remote SCCP is "inaccessible", "unequipped" or "unknown", SCCP management: (1)
 *  if the statis indicated for the remote SCCP is "inaccessible" or "unknown", initiates the SCCP Status Test
 *  (Section 5.5.4); (2) performs action (4) through (14) described in section 5.2.2, Signalling Point Prohibited
 *  Control.
 *
 *  5.5.3 SCCP Allowed Control.  When, for a remote SCCP marked "prohibited." (1) a Subsystem-Allowed message with
 *  SSN=1 is received; or; (2) Timer T(stat_info) expires but an MTP-STATUS prmitive indicating remote SCCP
 *  unavailability has not been received since the timer was started or restarted, SCCP management: (1) performs
 *  actions (4) through (9) described in Section 5.2.3 terminates [sic], if Subsystem-Allowed message received; (2)
 *  performs actions (5) through (9) described in Section 5.2.3, if timer T(stat_info) expires.
 *
 *  5.5.4 SCCP Status Test.  An SCCP status test is intiated when MTP-STATUS indication for an inacessible or
 *  unknown SCCP is received (Section 5.5.2).
 *
 *  An SCCP status test associated with an inaccessible or unknown SCCP is commenced by sending a
 *  Subsystem-Status-Test message (with the subsystem number set to one to indicate an SCCP status test) to the
 *  remote SCCP and starting a timer T(stat_info).  The timer is reset if another SST initiation request is received
 *  for the inacessible or unknown SCCP.  This cycle continues until the test is treminated by another SCCP
 *  management function at that node.  Termination of the test causes the timer to be cancelled.
 *
 *  The expiration of the timer T(stat_info) for an inaccessible or unknown SCCP is handled by the SCCP Allowed
 *  Control procedure (Section 5.5.3).
 *
 *  When SCCP management receives a Subsystem-Status-Test message (SSN=1), Subsystem-Alloed (SSN=1) message is sent
 *  to SCCP management at the node conduction the test.
 *
 *  Q.714/1996 5.2.4 Signalling point congested.   When SCCP management receives an MTP-STATUS indication primitive
 *  relating to signalling network congestion to a signalling point, SCCP management: 1) Determines the severity of
 *  the congestion in the remote signalling point and updates that signalling point status to reflect the congesiton
 *  as follows: MTP provides a single level of congesiton indication (international method).  The severity is
 *  reflected by a local internal status variable referred to as "restriction level" RL(M).  Each of the N+1
 *  restriction levels except the highest level is further divided into M restriction sublevels, RSL(M) where: N =
 *  8, M = 4.  The method to compute these levels uses an attack timer Ta and a decay timer Td.  a) When timer Ta is
 *  not running then: Timer Ta is started and Td is (re)started.  If RL(M) is equal to N, then no further action is
 *  taken.  RSL(M) is incremented.  If RLS(M) reaches M, then RSL(M) is set to zero and RL(M) is incremented.  b)
 *  When timer Ta is running, the MTP-STATUS indication primitive is ignored.  2) Initiates the procedure of 5.2.8.
 *
 *  When congestion abates, the traffic is gradually resumed.  SCCP management: 1) Decreases the restriction level
 *  (RL(M)) in a time-controlled manner as follows: When timer Td expires, then RSL(M) is decremented and: a) if
 *  RSL(M) reachs -1 and RL(M) is not zero, then RSL(M) is reset to M-1 and RL(M) is decreased by one; b) if either
 *  RSL(M) or RL(M) is not zero, then timer Td is restarted again.  2) Initiates the procedure or 5.2.8.
 *
 *  When an indication of the end of MTP-RESTART is received, the associated RL(M) and RSL(M) are set to zero.
 *
 *  The values of M, N, Ta and Td parameters are administratable and provisional.
 *
 *  Q.714/1996 5.2.8  Inter- and Intra- SCMG congesiton reports procedure.  This SCMG procedure uses the values of
 *  the following internal status variables: 1) RL(M) restriction level due to receipt of MTP-STATUS indication of
 *  congestion for each affected SP (5.2.4).  2) RSL(M) restriction sublevel per RL(M) due to receipt of the
 *  MTP-STATUS indication of congestion for each affected SP (5.2.4).  3) CL(S) SCCP congestion level due to receipt
 *  of the congestion level parameter of SSC message for each affected SP and SSN=1 (5.2.7).
 *
 *  The above values are used as inputs to compute the values of the following variables:  a) RL, SCRC traffic
 *  restriction level for each affected SP.  b) RSL, restriction sublevel per RL for each affected SP.  c) RIL,
 *  restricted importance level parameter reported to SCCP users for each affected SP.
 *
 *  If there is any change in RL or RSL, SCRC is informed of the new values of RL and RSL.
 *
 *  If there is any change in restricted importance level, the local broadcast procedure (5.3.6.6) is initiated to
 *  report the new value of restricted importance level.
 *
 *  NOTE:  The computation is left for further study.
 */
static int
mtp_status_ind(struct mt *mt, queue_t *q, mblk_t *mp)
{
	struct sr *sr;
	struct mtp_addr *a;
	struct MTP_status_ind *p = (typeof(p)) mp->b_rptr;

	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto emsgsize;
	if (mp->b_wptr < mp->b_rptr + p->mtp_addr_offset + p->mtp_addr_length)
		goto emsgsize;
	a = (typeof(a)) (mp->b_rptr + p->mtp_addr_offset);
	if (!mt->sp)
		sr = mt->sr;
	else
		for (sr = mt->sp->sr.list; sr && sr->add.pc != a->pc; sr = sr->sp.next) ;
	if (!sr)
		goto ignore;	/* not for us */
	switch (p->mtp_type) {
	case MTP_STATUS_TYPE_CONG:
		if (sr->proto.popt & SS7_POPT_MPLEV) {
			int level;

			/* multiple congestion levels ala ANSI */
			switch (p->mtp_status) {
			case MTP_STATUS_CONGESTION_LEVEL0:
				level = 0;
				break;
			case MTP_STATUS_CONGESTION_LEVEL1:
				level = 1;
				break;
			case MTP_STATUS_CONGESTION_LEVEL2:
				level = 2;
				break;
			case MTP_STATUS_CONGESTION_LEVEL3:
				level = 3;
				break;
			default:
				goto efault;
			}
			if (sr->level < level) {
				if (!sr->level) {
					/* congestion begins */
#if 0
					if ((err =
					     sccp_sr_maint_ind(q, sc->sr.sr,
							       SCCP_MAINT_USER_PART_CONGESTED)))
						return (err);
#endif
				}
				sr->level = level;
				sr_timer_start(sr, tdecay);
			}
		} else {
			/* single congestion level ala ITU */
			if (!sr->timers.tattack) {
				if (!sr->level) {
					/* congestion begins */
#if 0
					if ((err =
					     sccp_sr_maint_ind(q, sc->sr.sr,
							       SCCP_MAINT_USER_PART_CONGESTED)))
						return (err);
#endif
				}
				sr->level++;
				sr_timer_start(sr, tattack);
				sr_timer_start(sr, tdecay);
			}
		}
		return (QR_DONE);
	case MTP_STATUS_TYPE_UPU:
		switch (p->mtp_status) {
		case MTP_STATUS_UPU_UNEQUIPPED:
			/* inform management */
#if 0
			if ((err =
			     sccp_sr_maint_ind(q, sc->sr.sr, SCCP_MAINT_USER_PART_UNEQUIPPED)))
				return (err);
#endif
			return (QR_DONE);
		case MTP_STATUS_UPU_UNKNOWN:
		case MTP_STATUS_UPU_INACCESSIBLE:
#if 0
			if ((err =
			     sccp_sr_maint_ind(q, sc->sr.sr, SCCP_MAINT_USER_PART_UNAVAILABLE)))
				return (err);
#endif
#define SCCPF_UPT_PENDING 2	/* FIXME */
#define SS7_POPT_UPT (1<<5)	/* FIXME */
			/* start UPU procedures */
			if (!(sr->flags & SCCPF_UPT_PENDING)) {
				if ((sr->proto.popt & SS7_POPT_UPT) && sr->sp.sp
				    && sr->sp.sp->ss.list) {
					fixme(("Send SST message\n"));
					sr_timer_start(sr, tstatinfo);
				} else {
					/* send some other message */
				}
				sr->flags |= SCCPF_UPT_PENDING;
			}
			return (QR_DONE);
		default:
			goto efault;
		}
	default:
		goto efault;
	}
	goto efault;
      ignore:
	return (QR_DONE);
      efault:
	return (-EFAULT);
      emsgsize:
	return (-EMSGSIZE);
}

/*
 *  MTP_RESTART_BEGINS_IND
 *  -----------------------------------
 *  Q.714/1996 5.2.6 Local MTP network unavailability.  Any action taken is implementation dependent.
 */
static int
mtp_restart_begins_ind(struct mt *mt, queue_t *q, mblk_t *mp)
{
	struct sr *sr;
	struct MTP_restart_begins_ind *p = (typeof(p)) mp->b_rptr;

	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto emsgsize;
	if (!mt->sp && (!mt->sr || !mt->sr->sp.sp))
		goto efault;	/* shouldn't happen */
	for (sr = mt->sp->sr.list; sr; sr = sr->sp.next) {
		if (!(sr->flags & SCCPF_LOC_S_BLOCKED)) {
			struct ss *ss;

			for (ss = sr->sp.sp->ss.list; ss; ss = ss->sp.next) {
				fixme(("Send N-STATUS to users\n"));
				ss->flags |= SCCPF_LOC_S_BLOCKED;
			}
			sr->flags |= SCCPF_LOC_S_BLOCKED;
		}
	}
	return (QR_DONE);
      emsgsize:
	swerr();
	return (-EMSGSIZE);
      efault:
	swerr();
	return (-EFAULT);
}

/*
 *  MTP_RESTART_COMPLETE_IND
 *  -----------------------------------
 *  ANSI T1.112.4/2000 5.2.5 SCCP Reaction to Local MTP Restart.  When SCCP management receives an indication of the
 *  end of MTP Restart, SCCP management: (1) resets the conestion level of the signalling points concerned by the
 *  restarintg MTP; (2) instructs the translation function to update the translation tables, taking into account the
 *  accessibility informaiton given by the MTP indicating the end of MTP Restart; (3) marks as "allowed" the status
 *  of the subsystems for each accessible point; (4) intiiates a local broadcast (5.3.6) of "signalling point
 *  accessible."
 *
 *  ANSI T1.112.4/2000 5.6 SCCP Restart.  On a signalling point restart, an indication is given to the SCCP by the
 *  MTP about the signallin points which are accessible after the restart actions.  For each accessible, concerned
 *  signalling point, all subsystems are marked allowed.  If SCCP Flow Control is used with the siganlling point,
 *  the status of its SCCP is also marked "allowed."  The response method is used to determine the status of
 *  subsystems and the SCCP in those signalling points.
 *
 *  At the restarted signalling point, the status of its own susbsystems are not broadcast to concerned signalling
 *  points.  In this case, the response method is used to inform other nodes attempting to access prohibited
 *  subsystems at the restarted signalling point.
 *
 *  The actions taken in cause of local MTP restart are described in section 5.2.5.
 *
 *  Q.714/1996 5.2.5 Local MTP network availability.  When SCCP management receives an indication reporting the end
 *  of MTP Restart, then it: 1) resets the congestion level of the associated signalling points; 2) instructs the
 *  translation function to update the translation tables, taking into account the accessibility given by the MTP
 *  indicating the end of MTP Restart; 3) marks as allowed the status of the SCCP and all subsystems for each
 *  accessible point; 4) initiates a local broadcast (5.3.6.4) of "signalling point accessible" information for the
 *  signalling point becoming accessible; 5) initiates a local broadcast of "remote SCCP accessible" for the remote
 *  SCCPs becoming accessible, and 6) initiates a local broadcast of "User-in-service" information for a subsystem
 *  associated with the end of the MTP-RESTART.
 */
static int
mtp_restart_complete_ind(struct mt *mt, queue_t *q, mblk_t *mp)
{
	struct sr *sr;
	struct MTP_restart_complete_ind *p = (typeof(p)) mp->b_rptr;

	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto emsgsize;
	if (!mt->sp && (!mt->sr || !mt->sr->sp.sp))
		goto efault;	/* shouldn't happen */
	for (sr = mt->sp->sr.list; sr; sr = sr->sp.next) {
		if (sr->flags & SCCPF_LOC_S_BLOCKED) {
			struct ss *ss;

			for (ss = sr->sp.sp->ss.list; ss; ss = ss->sp.next) {
				fixme(("Send N-STATUS to users\n"));
				ss->flags &= ~SCCPF_LOC_S_BLOCKED;
			}
			sr->flags &= ~SCCPF_LOC_S_BLOCKED;
		}
	}
	return (QR_DONE);
      emsgsize:
	swerr();
	return (-EMSGSIZE);
      efault:
	swerr();
	return (-EFAULT);
}

/*
 *  UNRECOGNIZED PRIMITIVE
 *  -----------------------------------
 */
static int
mtp_unrecognized_prim(struct mt *mt, queue_t *q, mblk_t *mp)
{
	swerr();
	fixme(("Notify management of unrecognized primitive\n"));
	return (-EFAULT);
}

/*
 *  -------------------------------------------------------------------------
 *
 *  Primitives from above
 *
 *  -------------------------------------------------------------------------
 */

/*
 *  NPI Primitives
 *  -------------------------------------------------------------------------
 */
/*
 *  N_CONN_REQ:
 *  -----------------------------------
 */
static int
n_conn_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	N_conn_req_t *p = ((typeof(p)) mp->b_rptr);
	N_qos_sel_conn_sccp_t *qos = (typeof(qos)) (mp->b_rptr + p->QOS_offset);
	struct sccp_addr *a;
	np_ulong *sls = NULL;
	np_ulong *pri = NULL;
	np_ulong *pcl = NULL;
	np_ulong *imp = NULL;

	if (sccp_get_n_state(sc) != NS_IDLE)
		goto noutstate;
	sccp_set_n_state(sc, NS_WCON_CREQ);
	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto einval;
	if (p->QOS_length) {
		if (mp->b_wptr < mp->b_rptr + p->QOS_offset + p->QOS_length)
			goto nbadopt3;
		if (qos->n_qos_type != N_QOS_SEL_CONN_SCCP)
			goto nbadqostype;
		if (qos->protocol_class > 3)
			goto nbadopt2;
		if (qos->protocol_class != 2 && qos->protocol_class != 3)
			goto nnotsupport;
		if (p->QOS_length != sizeof(*qos))
			goto nbadopt;
	}
	a = (struct sccp_addr *) (mp->b_rptr + p->DEST_offset);
	if (!p->DEST_length)
		goto nnoaddr;
	if ((mp->b_wptr < mp->b_rptr + p->DEST_offset + p->DEST_length)
	    || (p->DEST_length < sizeof(struct sccp_addr) + a->alen) || !a->ssn)
		goto nbadaddr;
	if (sc->cred.cr_uid != 0 && !a->ssn)
		goto naccess;
	fixme(("select a source local reference number\n"));
	if (p->QOS_length) {
		sls = &qos->sequence_selection;
		pri = &qos->message_priority;
		pcl = &qos->protocol_class;
		imp = &qos->importance;
	}
	sc->flags &= ~(SCCPF_REC_CONF_OPT | SCCPF_EX_DATA_OPT);
	if (p->CONN_flags & REC_CONF_OPT)
		sc->flags |= SCCPF_REC_CONF_OPT;
	if (p->CONN_flags & EX_DATA_OPT)
		sc->flags |= SCCPF_EX_DATA_OPT;
	if ((err = sccp_conn_req(sc, q, a, sls, pri, pcl, imp, mp->b_cont)))
		goto error;
	return (QR_TRIMMED);
      naccess:
	err = NACCESS;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_REQ: no permission for requested address");
	goto error;
      nbadaddr:
	err = NBADADDR;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_REQ: address is unusable");
	goto error;
      nnoaddr:
	err = NNOADDR;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_REQ: no destination address");
	goto error;
      nbadopt:
	err = NBADOPT;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_REQ: options are unusable");
	goto error;
      nnotsupport:
	err = NNOTSUPPORT;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_REQ: protocol class not supported");
	goto error;
      nbadopt2:
	err = NBADOPT;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_REQ: invalid protocol class");
	goto error;
      nbadqostype:
	err = NBADQOSTYPE;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_REQ: QOS structure type not supported");
	goto error;
      nbadopt3:
	err = NBADOPT;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_REQ: options are unusable");
	goto error;
      einval:
	err = -EINVAL;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_REQ: invalid message format");
	goto error;
      noutstate:
	err = NOUTSTATE;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_REQ: would palce interface out of state");
	goto error;
      error:
	return n_error_ack(sc, q, N_CONN_REQ, err);
}

/*
 *  N_CONN_RES:
 *  -----------------------------------
 *  IMPLEMENTATION NOTE:- Sequence numbers are actually the address of the mblk which contains the CR message and
 *  contains the contents of this message as a connection indication.  To find if a particular sequence number is
 *  valid, we walk the connection indication queue looking for an mblk with the same address as the sequence number.
 *  Sequence numbers are only valid on the stream for which the connection indication is queued. 
 */
static mblk_t *
n_seq_check(struct sc *sc, np_ulong seq)
{
	mblk_t *mp;
	psw_t flags;

	flags = bufq_lock(&sc->conq);
	for (mp = bufq_head(&sc->conq); mp && (np_ulong) (ulong) mp != seq; mp = mp->b_next) ;
	bufq_unlock(&sc->conq, flags);
	usual(mp);
	return (mp);
}

/*
 *  IMPLEMENTATION NOTE:- Tokens are actually the address of the read queue.  To find if a particular token is
 *  valid, we walk the open list looking for an open device with a read queue with the same address as the token.
 *  Tokens are only valid for SCCP streams of the same provider type.  (That is, a TPI connection indication cannot
 *  be accepted on an NPI stream. 
 */
static struct sc *
n_tok_check(struct sc *sc, np_ulong tok)
{
	struct sc *ap;

	for (ap = master.sc.list; ap && (np_ulong) (ulong) ap->oq != tok; ap = ap->next) ;
	usual(ap);
	return ((struct sc *) ap);
}
static int
n_conn_res(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	mblk_t *cp;
	struct sc *ap;
	N_conn_res_t *p = (typeof(p)) mp->b_rptr;
	N_qos_sel_conn_sccp_t *qos = (typeof(qos)) (mp->b_rptr + p->QOS_offset);

	if (sccp_get_n_state(sc) != NS_WRES_CIND)
		goto noutstate;
	sccp_set_n_state(sc, NS_WACK_CRES);
	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto einval;
	if (p->QOS_length) {
		if (mp->b_wptr < mp->b_rptr + p->QOS_offset + p->QOS_length)
			goto nbadopt2;
		if (qos->n_qos_type != N_QOS_SEL_CONN_SCCP)
			goto nbadqostype;
		if (p->QOS_length != sizeof(*qos))
			goto nbadopt;
	}
	if (!(cp = n_seq_check(sc, p->SEQ_number)))
		goto nbadseq;
	if (!(ap = n_tok_check(sc, p->TOKEN_value))
	    || (ap != sc && ((1 << ap->i_state) & ~(NSF_UNBND | NSF_IDLE))))
		goto nbadtoken2;
	if (ap->i_state == NS_IDLE && ap->conind)
		goto nbadtoken;
	/* protect at least r00t streams from users */
	if (sc->cred.cr_uid != 0 && ap->cred.cr_uid == 0)
		goto naccess;
	{
		np_ulong ap_oldstate = ap->i_state;
		np_ulong ap_oldflags = ap->flags;

		ap->i_state = NS_DATA_XFER;
		ap->flags &= ~(SCCPF_REC_CONF_OPT | SCCPF_EX_DATA_OPT);
		if (p->CONN_flags & REC_CONF_OPT)
			ap->flags |= SCCPF_REC_CONF_OPT;
		if (p->CONN_flags & EX_DATA_OPT)
			ap->flags |= SCCPF_EX_DATA_OPT;
		if ((err = sccp_conn_res(sc, q, cp, ap, mp->b_cont)))
			goto call_error;
		if ((err = n_ok_ack(sc, q, NULL, N_CONN_RES, p->SEQ_number, p->TOKEN_value)))
			goto call_error;
		return (QR_TRIMMED);
	      call_error:
		ap->i_state = ap_oldstate;
		ap->flags = ap_oldflags;
		goto error;
	}
      naccess:
	err = NACCESS;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_RES: no access to accepting queue");
	goto error;
      nbadtoken:
	err = NBADTOKEN;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_RES: accepting queue is listening");
	goto error;
      nbadtoken2:
	err = NBADTOKEN;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_RES: accepting queue id is invalid");
	goto error;
      nbadseq:
	err = NBADSEQ;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_RES: connection ind reference is invalid");
	goto error;
      nbadopt:
	err = NBADOPT;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_RES: quality of service options are bad");
	goto error;
      nbadqostype:
	err = NBADQOSTYPE;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_RES: quality of service options are bad");
	goto error;
      nbadopt2:
	err = NBADOPT;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_RES: quality of service options are bad");
	goto error;
      einval:
	err = -EINVAL;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_RES: invalid primitive format");
	goto error;
      noutstate:
	err = NOUTSTATE;
	mi_strlog(q, 0, SL_TRACE, "N_CONN_RES: would place interface out of state");
	goto error;
      error:
	return n_error_ack(sc, q, N_CONN_RES, err);
}

/*
 *  N_DISCON_REQ:
 *  -----------------------------------
 */
static int
n_discon_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	mblk_t *cp = NULL;
	N_discon_req_t *p = (typeof(p)) mp->b_rptr;

	if ((1 << sccp_get_n_state(sc)) &
	    ~(NSF_WCON_CREQ | NSF_WRES_CIND | NSF_DATA_XFER | NSF_WCON_RREQ | NSF_WRES_RIND))
		goto noutstate;
	switch (sccp_get_n_state(sc)) {
	case NS_WCON_CREQ:
		sccp_set_n_state(sc, NS_WACK_DREQ6);
		break;
	case NS_WRES_CIND:
		sccp_set_n_state(sc, NS_WACK_DREQ7);
		break;
	case NS_DATA_XFER:
		sccp_set_n_state(sc, NS_WACK_DREQ9);
		break;
	case NS_WCON_RREQ:
		sccp_set_n_state(sc, NS_WACK_DREQ10);
		break;
	case NS_WRES_RIND:
		sccp_set_n_state(sc, NS_WACK_DREQ11);
		break;
	}
	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto einval;
	if (p->RES_length)
		goto nbadaddr;
	if (sccp_get_n_state(sc) == NS_WACK_DREQ7 && !(cp = n_seq_check(sc, p->SEQ_number)))
		goto nbadseq;
	/* XXX: What to do with the disconnect reason? Nothing? */
	if ((err = sccp_discon_req(sc, q, cp, mp->b_cont)))
		goto error;
	if ((err = n_ok_ack(sc, q, NULL, p->PRIM_type, p->SEQ_number, 0)))
		goto error;
	return (QR_TRIMMED);
      nbadseq:
	err = NBADSEQ;
	mi_strlog(q, 0, SL_TRACE, "N_DISCON_REQ: connection ind reference is invalid");
	goto error;
      nbadaddr:
	err = NBADADDR;
	mi_strlog(q, 0, SL_TRACE, "N_DISCON_REQ: responding address is invalid");
	goto error;
      einval:
	err = -EINVAL;
	mi_strlog(q, 0, SL_TRACE, "N_DISCON_REQ: invalid primitive format");
	goto error;
      noutstate:
	err = NOUTSTATE;
	mi_strlog(q, 0, SL_TRACE, "N_DISCON_REQ: would place interface out of state");
	goto error;
      error:
	return n_error_ack(sc, q, N_DISCON_REQ, err);
}

/*
 *  N_DATA_REQ:
 *  -----------------------------------
 */
static int
n_error_reply(struct sc *sc, queue_t *q, mblk_t *msg, int err)
{
	mblk_t *mp;

	switch (err) {
	case -EBUSY:
	case -EAGAIN:
	case -ENOMEM:
	case -ENOBUFS:
	case -EDEADLK:
		return (err);
	}
	if ((mp = mi_allocb(q, 2, BPRI_HI))) {
		DB_TYPE(mp) = M_ERROR;
		*mp->b_wptr++ = err < 0 ? -err : err;
		*mp->b_wptr++ = err < 0 ? -err : err;
		freemsg(msg);
		mi_strlog(q, STRLOGRX, SL_TRACE, "<- M_ERROR");
		putnext(sc->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}
static int
n_write(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;

	if (sccp_get_n_state(sc) == NS_IDLE)
		goto discard;
	if ((1 << sccp_get_n_state(sc)) & ~(NSF_DATA_XFER | NSF_WRES_RIND))
		goto eproto;
	if ((err = sccp_data_req(sc, q, NULL, 0, 0, 0, mp)))
		goto error;
	return (0);
      eproto:
	err = -EPROTO;
	mi_strlog(q, 0, SL_ERROR, "would place interface out of state");
	goto error;
      discard:
	err = 0;
	mi_strlog(q, 0, SL_ERROR, "ignoring in idle state");
	goto error;
      error:
	return n_error_reply(sc, q, mp, err);
}
static int
n_data_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	N_qos_sel_data_sccp_t *qos = NULL;
	N_data_req_t *p = (typeof(p)) mp->b_rptr;

	if (sccp_get_n_state(sc) == NS_IDLE)
		goto discard;
	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto einval;
	if (mp->b_wptr >= mp->b_rptr + sizeof(*p) + sizeof(*qos)) {
		qos = (typeof(qos)) (p + 1);
		if (qos->n_qos_type != N_QOS_SEL_DATA_SCCP)
			qos = NULL;
	}
	if ((1 << sccp_get_n_state(sc)) & ~(NSF_DATA_XFER | NSF_WRES_RIND))
		goto eproto;
	{
		np_ulong exp = 0;	/* FIXME */
		np_ulong more = 0;	/* FIXME */
		np_ulong rcpt = 0;	/* FIXME */

		return sccp_data_req(sc, q, mp, exp, more, rcpt, mp->b_cont);
	}
      eproto:
	err = -EPROTO;
	mi_strlog(q, STRLOGDA, SL_TRACE, "N_DATA_REQ: would place interface out of state");
	goto error;
      einval:
	err = -EINVAL;
	mi_strlog(q, STRLOGDA, SL_TRACE, "N_DATA_REQ: invalid primitive format");
	goto error;
      discard:
	err = QR_DONE;
	mi_strlog(q, STRLOGDA, SL_TRACE, "N_DATA_REQ: ignore in idle state");
	goto error;
      error:
	return n_error_reply(sc, q, mp, err);
}

/*
 *  N_EXDATA_REQ:
 *  -----------------------------------
 */
static int
n_exdata_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	N_qos_sel_data_sccp_t *qos = NULL;
	N_exdata_req_t *p = (typeof(p)) mp->b_rptr;

	if (sccp_get_n_state(sc) == NS_IDLE)
		goto discard;
	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto einval;
	if (mp->b_wptr >= mp->b_rptr + sizeof(*p) + sizeof(*qos)) {
		qos = (typeof(qos)) (p + 1);
		if (qos->n_qos_type != N_QOS_SEL_DATA_SCCP)
			qos = NULL;
	}
	if ((1 << sccp_get_n_state(sc)) & ~(NSF_DATA_XFER | NSF_WRES_RIND))
		goto eproto;
	{
		np_ulong exp = 0;	/* FIXME */
		np_ulong more = 0;	/* FIXME */
		np_ulong rcpt = 0;	/* FIXME */

		(void) sccp_exdata_req;
		return sccp_data_req(sc, q, mp, exp, more, rcpt, mp->b_cont);
	}
      eproto:
	err = -EPROTO;
	mi_strlog(q, STRLOGDA, SL_TRACE, "N_EXDATA_REQ: would place interface out of state");
	goto error;
      einval:
	err = -EINVAL;
	mi_strlog(q, STRLOGDA, SL_TRACE, "N_EXDATA_REQ: invalid primitive format");
	goto error;
      discard:
	err = QR_DONE;
	mi_strlog(q, STRLOGDA, SL_TRACE, "N_EXDATA_REQ: ignore in idle state");
	goto error;
      error:
	return n_error_reply(sc, q, mp, err);
}

/*
 *  N_INFO_REQ:
 *  -----------------------------------
 */
static int
n_info_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	(void) mp;
	return n_info_ack(sc, q, NULL);
}

/*
 *  N_BIND_REQ:
 *  -----------------------------------
 */
static int
n_bind_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	N_bind_req_t *p = (typeof(p)) mp->b_rptr;
	struct sccp_addr *a;

	if (sccp_get_n_state(sc) != NS_UNBND)
		goto noutstate;
	sccp_set_n_state(sc, NS_WACK_BREQ);
	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto einval;
	a = (struct sccp_addr *) (mp->b_rptr + p->ADDR_offset);
	if ((mp->b_wptr < mp->b_rptr + p->ADDR_offset + p->ADDR_length)
	    || (p->ADDR_length < sizeof(struct sccp_addr) + a->alen) || !a->ssn)
		goto nbadaddr;
	fixme(("Deal with wildcards\n"));
	fixme(("Deal with default binds\n"));
	fixme(("Deal with slr assignment\n"));
	if (0)
		goto nnoaddr;
	if (sc->cred.cr_uid != 0 && !a->ssn)
		goto naccess;
	if ((err = sccp_bind_req(sc, q, a, p->CONIND_number)))
		goto error;
	return n_bind_ack(sc, q, NULL);
      naccess:
	err = NACCESS;
	mi_strlog(q, 0, SL_TRACE, "N_BIND_REQ: no permission for requested address");
	goto error;
      nnoaddr:
	err = NNOADDR;
	mi_strlog(q, 0, SL_TRACE, "N_BIND_REQ: could not allocate address");
	goto error;
      nbadaddr:
	err = NBADADDR;
	mi_strlog(q, 0, SL_TRACE, "N_BIND_REQ: address is invalid");
	goto error;
      einval:
	err = -EINVAL;
	mi_strlog(q, 0, SL_TRACE, "N_BIND_REQ: invalid primitive format");
	goto error;
      noutstate:
	err = NOUTSTATE;
	mi_strlog(q, 0, SL_TRACE, "N_BIND_REQ: would place interface out of state");
	goto error;
      error:
	return n_error_ack(sc, q, N_BIND_REQ, err);
}

/*
 *  N_UNBIND_REQ:
 *  -----------------------------------
 */
static int
n_unbind_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	N_unbind_req_t *p = (typeof(p)) mp->b_rptr;

	(void) p;
	if (sccp_get_n_state(sc) != NS_IDLE)
		goto noutstate;
	sccp_set_n_state(sc, NS_WACK_UREQ);
	if ((err = sccp_unbind_req(sc, q)))
		goto error;
	return n_ok_ack(sc, q, NULL, p->PRIM_type, 0, 0);
      noutstate:
	err = NOUTSTATE;
	mi_strlog(q, 0, SL_TRACE, "N_UNBIND_REQ: would place interface out of state");
	goto error;
      error:
	return n_error_ack(sc, q, N_UNBIND_REQ, err);
}

/*
 *  N_UNITDATA_REQ:
 *  -----------------------------------
 */
static int
n_uderror_reply(struct sc *sc, queue_t *q, mblk_t *mp, int etype)
{
	N_uderror_ind_t *p = (typeof(p)) mp->b_rptr;

	switch (etype) {
	case -EBUSY:
	case -EAGAIN:
	case -ENOMEM:
	case -ENOBUFS:
	case QR_DONE:
	case QR_ABSORBED:
	case QR_TRIMMED:
		mi_strlog(q, 0, SL_ERROR, "N_UDERROR_REPLY: no error condition");
		return (etype);
	}
	if (etype > 0) {
		mp->b_wptr = mp->b_rptr + sizeof(*p);
		p->PRIM_type = N_UDERROR_IND;
		p->ERROR_type = etype;
	} else {
		DB_TYPE(mp) = M_ERROR;
		mp->b_wptr = mp->b_rptr;
		*mp->b_wptr++ = -etype;
		*mp->b_wptr++ = -etype;
	}
	putnext(sc->oq, mp);
	return (QR_ABSORBED);
}
static int
n_unitdata_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	size_t dlen = mp->b_cont ? msgdsize(mp->b_cont) : 0;
	const N_unitdata_req_t *p = (typeof(p)) mp->b_rptr;
	struct sccp_addr *dst;
	N_qos_sel_data_sccp_t *qos;

	if (dlen == 0 || dlen > sc->mtu)
		goto eproto5;
	if (sccp_get_n_state(sc) != NS_IDLE)
		goto eproto4;
	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto eproto3;
	dst = (typeof(dst)) (mp->b_rptr + p->DEST_offset);
	qos = NULL;
	if ((mp->b_wptr < mp->b_rptr + p->DEST_offset + p->DEST_length)
	    || (!p->DEST_offset) || (p->DEST_offset < sizeof(*p))
	    || (p->DEST_length < sizeof(*dst)) || (p->DEST_length < sizeof(*dst) + dst->alen))
		goto eproto2;
	if (sc->cred.cr_uid != 0 && !dst->ssn)
		goto eproto;
	if (mp->b_wptr >=
	    mp->b_rptr + p->DEST_offset + p->DEST_length + PAD4(dst->alen) + sizeof(*qos)) {
		qos = (typeof(qos)) (mp->b_rptr + p->DEST_offset + p->DEST_length);
		if (qos->n_qos_type != N_QOS_SEL_DATA_SCCP)
			qos = NULL;
	}
	{
		np_ulong pcl = qos ? qos->protocol_class : sc->pcl;
		np_ulong ret = qos ? qos->option_flags : sc->flags;
		np_ulong imp = qos ? qos->importance : sc->imp;
		np_ulong seq = qos ? qos->sequence_selection : sc->sls;
		np_ulong pri = qos ? qos->message_priority : sc->mp;

		if ((err = sccp_unitdata_req(sc, q, dst, pcl, ret, imp, seq, pri, mp->b_cont)))
			goto error;
		return (QR_TRIMMED);
	}
      eproto:
	err = -EPROTO;
	mi_strlog(q, STRLOGDA, SL_TRACE, "N_UNITDATA_REQ: no permission to send to address");
	goto error;
      eproto2:
	err = -EPROTO;
	mi_strlog(q, STRLOGDA, SL_TRACE, "N_UNITDATA_REQ: band destination address");
	goto error;
      eproto3:
	err = -EPROTO;
	mi_strlog(q, STRLOGDA, SL_TRACE, "N_UNITDATA_REQ: invalid primitive");
	goto error;
      eproto4:
	err = -EPROTO;
	mi_strlog(q, STRLOGDA, SL_TRACE, "N_UNITDATA_REQ: would place interface out of state");
	goto error;
      eproto5:
	err = -EPROTO;
	mi_strlog(q, STRLOGDA, SL_TRACE, "N_UNITDATA_REQ: bad data size");
	goto error;
      error:
	return n_uderror_reply(sc, q, mp, err);
}

/*
 *  N_OPTMGMT_REQ:
 *  -----------------------------------
 */
static int
n_optmgmt_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	N_optmgmt_req_t *p = (typeof(p)) mp->b_rptr;
	N_qos_sel_info_sccp_t *qos = (typeof(qos)) (mp->b_rptr + p->QOS_offset);

	if (sccp_get_n_state(sc) == NS_IDLE)
		sccp_set_n_state(sc, NS_WACK_OPTREQ);
	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto einval;
	if (p->QOS_length) {
		if (mp->b_wptr < mp->b_rptr + p->QOS_offset + p->QOS_length)
			goto nbadopt3;
		if (qos->n_qos_type != N_QOS_SEL_INFO_SCCP)
			goto nbadqostype;
		if (p->QOS_length != sizeof(*qos))
			goto nbadopt2;
		if (qos->protocol_class > 3)
			goto nbadopt;
		if (qos->protocol_class != -1)
			sc->pcl = qos->protocol_class;
		if (qos->option_flags != -1)
			sc->flags = qos->option_flags;
	}
	if (p->OPTMGMT_flags & DEFAULT_RC_SEL)
		sc->flags |= SCCPF_DEFAULT_RC_SEL;
	else
		sc->flags &= ~SCCPF_DEFAULT_RC_SEL;
	return n_ok_ack(sc, q, NULL, p->PRIM_type, 0, 0);
      nbadopt:
	err = NBADOPT;
	mi_strlog(q, 0, SL_TRACE, "N_OPTMGMT_REQ: bad protocol class");
	goto error;
      nbadopt2:
	err = NBADOPT;
	mi_strlog(q, 0, SL_TRACE, "N_OPTMGMT_REQ: invalid qos options");
	goto error;
      nbadqostype:
	err = NBADQOSTYPE;
	mi_strlog(q, 0, SL_TRACE, "N_OPTMGMT_REQ: bad qos structure type");
	goto error;
      nbadopt3:
	err = NBADOPT;
	mi_strlog(q, 0, SL_TRACE, "N_OPTMGMT_REQ: bad qos structure format");
	goto error;
      einval:
	err = -EINVAL;
	mi_strlog(q, 0, SL_TRACE, "N_OPTMGMT_REQ: bad primitive format");
	goto error;
      error:
	return n_error_ack(sc, q, N_OPTMGMT_REQ, err);
}

/*
 *  N_DATACK_REQ:
 *  -----------------------------------
 */
static int
n_datack_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	N_datack_req_t *p = (typeof(p)) mp->b_rptr;

	(void) sccp_datack_req;
	fixme(("SWERR: Write this function\n"));
	return n_error_ack(sc, q, N_DATACK_REQ, NNOTSUPPORT);
}

/*
 *  N_RESET_REQ:
 *  -----------------------------------
 */
static int
n_reset_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	N_reset_req_t *p = (typeof(p)) mp->b_rptr;

	if (sccp_get_n_state(sc) == NS_IDLE)
		goto discard;
	if (sccp_get_n_state(sc) != NS_DATA_XFER)
		goto noutstate;
	sccp_set_n_state(sc, NS_WCON_RREQ);
	if ((err = sccp_reset_req(sc, q)))
		goto error;
	return (0);
      noutstate:
	err = NOUTSTATE;
	mi_strlog(q, 0, SL_TRACE, "N_RESET_REQ: would place interface out of state");
	goto error;
      discard:
	err = 0;
	mi_strlog(q, 0, SL_TRACE, "N_RESET_REQ: ignored in idle state");
	goto error;
      error:
	return n_error_ack(sc, q, N_RESET_REQ, err);
}

/*
 *  N_RESET_RES:
 *  -----------------------------------
 */
static int
n_reset_res(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	N_reset_res_t *p = (typeof(p)) mp->b_rptr;

	if (sccp_get_n_state(sc) == NS_IDLE)
		goto discard;
	if (sccp_get_n_state(sc) != NS_WRES_RIND)
		goto noutstate;
	sccp_set_n_state(sc, NS_WACK_RRES);
	if ((err = sccp_reset_res(sc, q)))
		goto error;
	return n_ok_ack(sc, q, NULL, p->PRIM_type, 0, 0);
      noutstate:
	err = NOUTSTATE;
	mi_strlog(q, 0, SL_TRACE, "N_RESET_RES: would place interface out of state");
	goto error;
      discard:
	err = 0;
	mi_strlog(q, 0, SL_TRACE, "N_RESET_RES: ignored in idle state");
	goto error;
      error:
	return n_error_ack(sc, q, N_RESET_RES, err);
}

/*
 *  N_INFORM_REQ
 *  -----------------------------------
 */
static int
n_inform_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	N_inform_req_t *p = (typeof(p)) mp->b_rptr;
	N_qos_sel_infr_sccp_t *qos = NULL;

	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto emsgsize;
	if (p->QOS_length) {
		if (mp->b_wptr < mp->b_rptr + p->QOS_offset + p->QOS_length)
			goto emsgsize;
		if (p->QOS_length != sizeof(*qos))
			goto badqostype;
		qos = (typeof(qos)) (mp->b_rptr + p->QOS_offset);
		if (qos->n_qos_type != N_QOS_SEL_INFR_SCCP)
			goto badqostype;
	}
	if ((err = sccp_inform_req(sc, q, qos, p->REASON)))
		goto error;
	return n_ok_ack(sc, q, NULL, N_INFORM_REQ, 0, 0);
      badqostype:
	err = NBADQOSTYPE;
	mi_strlog(q, 0, SL_TRACE, "N_INFORM_REQ: QOS structure type not supported");
	goto error;
      emsgsize:
	err = -EINVAL;
	mi_strlog(q, 0, SL_TRACE, "N_INFORM_REQ: bad primitive format");
	goto error;
      error:
	return n_error_ack(sc, q, N_INFORM_REQ, err);
}

/*
 *  N_COORD_REQ
 *  -----------------------------------
 */
static int
n_coord_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	N_coord_req_t *p = (typeof(p)) mp->b_rptr;
	struct sccp_addr *add;

	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto emsgsize;
	if (!p->ADDR_length)
		goto noaddr;
	if (mp->b_wptr < mp->b_rptr + p->ADDR_offset + p->ADDR_length)
		goto emsgsize;
	if (p->ADDR_length < sizeof(*add))
		goto badaddr;
	add = (typeof(add)) (mp->b_rptr + p->ADDR_offset);
	if (p->ADDR_length != sizeof(*add) + add->alen)
		goto badaddr;
	if ((err = sccp_coord_req(sc, q, add)))
		goto error;
	return n_ok_ack(sc, q, NULL, N_COORD_REQ, 0, 0);
      badaddr:
	err = NBADADDR;
	mi_strlog(q, 0, SL_TRACE, "N_COORD_REQ: bad address format");
	goto error;
      noaddr:
	err = NNOADDR;
	mi_strlog(q, 0, SL_TRACE, "N_COORD_REQ: could not assign address");
	goto error;
      emsgsize:
	err = -EINVAL;
	mi_strlog(q, 0, SL_TRACE, "N_COORD_REQ: bad primitive format");
	goto error;
      error:
	return n_error_ack(sc, q, N_COORD_REQ, err);
}

/*
 *  N_COORD_RES
 *  -----------------------------------
 */
static int
n_coord_res(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	N_coord_res_t *p = (typeof(p)) mp->b_rptr;
	struct sccp_addr *add;

	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto emsgsize;
	if (!p->ADDR_length)
		goto noaddr;
	if (mp->b_wptr < mp->b_rptr + p->ADDR_offset + p->ADDR_length)
		goto emsgsize;
	if (p->ADDR_length < sizeof(*add))
		goto badaddr;
	add = (typeof(add)) (mp->b_rptr + p->ADDR_offset);
	if (p->ADDR_length != sizeof(*add) + add->alen)
		goto badaddr;
	if ((err = sccp_coord_res(sc, q, add)))
		goto error;
	return n_ok_ack(sc, q, NULL, N_COORD_RES, 0, 0);
      badaddr:
	err = NBADADDR;
	mi_strlog(q, 0, SL_TRACE, "N_COORD_RES: bad address format");
	goto error;
      noaddr:
	err = NNOADDR;
	mi_strlog(q, 0, SL_TRACE, "N_COORD_RES: could not assign address");
	goto error;
      emsgsize:
	err = -EINVAL;
	mi_strlog(q, 0, SL_TRACE, "N_COORD_RES: bad primitive format");
	goto error;
      error:
	return n_error_ack(sc, q, N_COORD_RES, err);
}

/*
 *  N_STATE_REQ
 *  -----------------------------------
 */
static int
n_state_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	N_state_req_t *p = (typeof(p)) mp->b_rptr;
	struct sccp_addr *add;

	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto emsgsize;
	if (!p->ADDR_length)
		goto noaddr;
	if (mp->b_wptr < mp->b_rptr + p->ADDR_offset + p->ADDR_length)
		goto emsgsize;
	if (p->ADDR_length < sizeof(*add))
		goto badaddr;
	add = (typeof(add)) (mp->b_rptr + p->ADDR_offset);
	if (p->ADDR_length != sizeof(*add) + add->alen)
		goto badaddr;
	if ((err = sccp_state_req(sc, q, add, p->STATUS)))
		goto error;
	return n_ok_ack(sc, q, NULL, N_STATE_REQ, 0, 0);
      badaddr:
	err = NBADADDR;
	mi_strlog(q, 0, SL_TRACE, "N_STATE_REQ: bad address format");
	goto error;
      noaddr:
	err = NNOADDR;
	mi_strlog(q, 0, SL_TRACE, "N_STATE_REQ: could not assign address");
	goto error;
      emsgsize:
	err = -EINVAL;
	mi_strlog(q, 0, SL_TRACE, "N_STATE_REQ: bad primitive format");
	goto error;
      error:
	return n_error_ack(sc, q, N_STATE_REQ, err);
}

/*
 *  TPI Primitives
 *  -------------------------------------------------------------------------
 */
/*
 *  T_CONN_REQ
 *  -----------------------------------
 */
static int
t_conn_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	const struct T_conn_req *p = (typeof(p)) mp->b_rptr;
	struct sccp_addr *a;
	uchar *op, *oe;
	struct t_opthdr *oh;
	np_ulong *sls = NULL;
	np_ulong *pri = NULL;
	np_ulong *pcl = NULL;
	np_ulong *imp = NULL;

	if (sccp_get_t_state(sc) != TS_IDLE)
		goto outstate;
	sccp_set_t_state(sc, TS_WACK_CREQ);
	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto einval;
	if (p->OPT_length) {
		if (mp->b_wptr < mp->b_rptr + p->OPT_offset + p->OPT_length)
			goto einval;
		/* check options ranges and format */
		for (op = mp->b_rptr + p->OPT_offset, oe = op + p->OPT_length, oh = (typeof(oh)) op;
		     op + sizeof(*oh) <= oe && oh->len >= sizeof(*oh)
		     && op + oh->len <= oe; op += oh->len, oh = (typeof(oh)) op) {
			t_scalar_t val = *((t_scalar_t *) (oh + 1));

			switch (oh->level) {
			case T_SS7_SCCP:
				switch (oh->name) {
				case T_SCCP_PCLASS:
					if (val < 0 || val > 3)
						goto badopt3;
					if (val != 2 && val != 3)
						goto notsupport;
					continue;
				}
			}
		}
		if (op != oe)
			goto badopt;
	}
	a = (struct sccp_addr *) (mp->b_rptr + p->DEST_offset);
	if (!p->DEST_length)
		goto noaddr;
	if ((mp->b_wptr < mp->b_rptr + p->DEST_offset + p->DEST_length)
	    || (p->DEST_length < sizeof(struct sccp_addr) + a->alen) || !a->ssn)
		goto badaddr;
	if (sc->cred.cr_uid != 0 && !a->ssn)
		goto access;
	fixme(("select a source local reference number\n"));
	if (p->OPT_length) {
		for (op = mp->b_rptr + p->OPT_offset, oe = op + p->OPT_length, oh = (typeof(oh)) op;
		     op + sizeof(*oh) <= oe && oh->len >= sizeof(*oh)
		     && op + oh->len <= oe; op += oh->len, oh = (typeof(oh)) op) {
			switch (oh->level) {
			case T_SS7_SCCP:
				switch (oh->name) {
				case T_SCCP_SEQ_CTRL:
					sls = (np_ulong *) (oh + 1);
					continue;
				case T_SCCP_PRIORITY:
					pri = (np_ulong *) (oh + 1);
					continue;
				case T_SCCP_PCLASS:
					pcl = (np_ulong *) (oh + 1);
					continue;
				case T_SCCP_IMPORTANCE:
					imp = (np_ulong *) (oh + 1);
					continue;
				default:
				case T_SCCP_PVAR:
				case T_SCCP_MPLEV:
				case T_SCCP_DEBUG:
				case T_SCCP_CLUSTER:
				case T_SCCP_RET_ERROR:
					/* ignore */
					continue;
				}
			}
		}
	}
	if ((err = sccp_conn_req(sc, q, a, sls, pri, pcl, imp, mp->b_cont)) < 0)
		goto error;
	if ((err = t_ok_ack(sc, q, NULL, p->PRIM_type, 0, 0)) < 0)
		goto error;
	return (QR_TRIMMED);
      access:
	err = TACCES;
	mi_strlog(q, 0, SL_TRACE, "T_CONN_REQ: no permission for requested address");
	goto error;
      badaddr:
	err = TBADADDR;
	mi_strlog(q, 0, SL_TRACE, "T_CONN_REQ: address is unusable");
	goto error;
      noaddr:
	err = TNOADDR;
	mi_strlog(q, 0, SL_TRACE, "T_CONN_REQ: no destination address");
	goto error;
      badopt:
	err = TBADOPT;
	mi_strlog(q, 0, SL_TRACE, "T_CONN_REQ: options are unusable");
	goto error;
      notsupport:
	err = TNOTSUPPORT;
	mi_strlog(q, 0, SL_TRACE, "T_CONN_REQ: primitive not supported for protocol class");
	goto error;
      badopt3:
	err = TBADOPT;
	mi_strlog(q, 0, SL_TRACE, "T_CONN_REQ: invalid protocol class");
	goto error;
      einval:
	err = -EINVAL;
	mi_strlog(q, 0, SL_TRACE, "T_CONN_REQ: invalid message format");
	goto error;
      outstate:
	err = TOUTSTATE;
	mi_strlog(q, 0, SL_TRACE, "T_CONN_REQ: would place interface out of state");
	goto error;
      error:
	return t_error_ack(sc, q, NULL, T_CONN_REQ, err);
}

/*
 *  T_CONN_RES
 *  -----------------------------------
 */
static mblk_t *
t_seq_check(struct sc *sc, t_uscalar_t seq)
{
	/* IMPLEMENTATION NOTE:- Sequence numbers are actually the address of the mblk which
	   contains the CR message and contains the contents of this message as a connection
	   indication.  To find if a particular sequence number is valid, we walk the connection
	   indication queue looking for an mblk with the same address as the sequence number.
	   Sequence numbers are only valid on the stream for which the connection indication is
	   queued. */
	mblk_t *mp;
	psw_t flags;

	flags = bufq_lock(&sc->conq);
	for (mp = bufq_head(&sc->conq); mp && (t_uscalar_t) (ulong) mp != seq; mp = mp->b_next) ;
	bufq_unlock(&sc->conq, flags);
	usual(mp);
	return (mp);
}
static struct sc *
t_tok_check(struct sc *sc, t_uscalar_t tok)
{
	/* IMPLEMENTATION NOTE:- Tokens are actually the address of the read queue.  To find if a
	   particular token is valid, we walk the open list looking for an open device with a read
	   queue with the same address as the token.  Tokens are only valid for SCCP streams of the 
	   same provider type.  (That is, a TPI connection indication cannot be accepted on an NPI
	   stream. */
	struct sc *ap;

	for (ap = master.sc.list; ap && (t_uscalar_t) (ulong) ap->oq != tok; ap = ap->next) ;
	usual(ap);
	return ((struct sc *) ap);
}
static int
t_conn_res(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	mblk_t *cp;
	struct sc *ap;
	const struct T_conn_res *p = (typeof(p)) mp->b_rptr;
	uchar *op, *oe;
	struct t_opthdr *oh;
	t_uscalar_t ap_oldstate;

	if (sccp_get_t_state(sc) != TS_WRES_CIND)
		goto outstate;
	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto einval;
	if (p->OPT_length) {
		if (mp->b_wptr < mp->b_rptr + p->OPT_offset + p->OPT_length)
			goto einval;
		/* check options ranges and format */
		for (op = mp->b_rptr + p->OPT_offset, oe = op + p->OPT_length, oh = (typeof(oh)) op;
		     op + sizeof(*oh) <= oe && oh->len >= sizeof(*oh)
		     && op + oh->len <= oe; op += oh->len, oh = (typeof(oh)) op) {
			t_scalar_t val = *((t_scalar_t *) (oh + 1));

			switch (oh->level) {
			case T_SS7_SCCP:
				switch (oh->name) {
				case T_SCCP_PCLASS:
					if (val < 0 || val > 3)
						goto badopt3;
					if (val != 2 && val != 3)
						goto notsupport;
					continue;
				}
			}
		}
		if (op != oe)
			goto badopt;
	}
	if (!(cp = t_seq_check(sc, p->SEQ_number)))
		goto badseq;
	if (!(ap = t_tok_check(sc, p->ACCEPTOR_id))
	    || (ap != sc && !((1 << sccp_get_t_state(sc)) & (TSF_UNBND | TSF_IDLE))))
		goto badf;
	if ((ap_oldstate = sccp_get_t_state(ap)) == TS_IDLE && ap->conind)
		goto resqlen;
	/* protect at least r00t streams from users */
	if (sc->cred.cr_uid != 0 && ap->cred.cr_uid == 0)
		goto access;
	if (p->OPT_length) {
		for (op = mp->b_rptr + p->OPT_offset, oe = op + p->OPT_length, oh = (typeof(oh)) op;
		     op + sizeof(*oh) <= oe && oh->len >= sizeof(*oh)
		     && op + oh->len <= oe; op += oh->len, oh = (typeof(oh)) op) {
			t_scalar_t val = *((t_scalar_t *) (oh + 1));

			switch (oh->level) {
			case T_SS7_SCCP:
				switch (oh->name) {
				case T_SCCP_SEQ_CTRL:
					ap->sls = val;
					continue;
				case T_SCCP_PRIORITY:
					ap->mp = val;
					continue;
				case T_SCCP_PCLASS:
					ap->pcl = val;
					continue;
				case T_SCCP_IMPORTANCE:
					ap->imp = val;
					continue;
				default:
				case T_SCCP_PVAR:
				case T_SCCP_MPLEV:
				case T_SCCP_DEBUG:
				case T_SCCP_CLUSTER:
				case T_SCCP_RET_ERROR:
					/* ignore */
					continue;
				}
			}
		}
	}
	sccp_set_t_state(ap, TS_DATA_XFER);
	if ((err = sccp_conn_res(sc, q, cp, ap, mp->b_cont)) < 0)
		goto call_error;
	if ((err =
	     t_ok_ack(sc, q, NULL, p->PRIM_type, (t_uscalar_t) (ulong) cp,
		      (t_uscalar_t) (ulong) ap)) < 0)
		goto call_error;
	return (QR_TRIMMED);
      call_error:
	sccp_set_t_state(ap, ap_oldstate);
	goto error;
      access:
      resqlen:
	err = TRESQLEN;
	mi_strlog(q, 0, SL_TRACE, "T_CONN_RES: accepting queue is listening");
	goto error;
      badf:
	err = TBADF;
	mi_strlog(q, 0, SL_TRACE, "T_CONN_RES: accepting queue id is invalid");
	goto error;
      badseq:
	err = TBADSEQ;
	mi_strlog(q, 0, SL_TRACE, "T_CONN_RES: connection ind reference is invalid");
	goto error;
      badopt:
	err = TBADOPT;
	mi_strlog(q, 0, SL_TRACE, "T_CONN_RES: options are bad");
	goto error;
      notsupport:
	err = TNOTSUPPORT;
	mi_strlog(q, 0, SL_TRACE, "T_CONN_RES: primitive not supported for protocol class");
	goto error;
      badopt3:
	err = TBADOPT;
	mi_strlog(q, 0, SL_TRACE, "T_CONN_RES: invalid protocol class");
	goto error;
      einval:
	err = -EINVAL;
	mi_strlog(q, 0, SL_TRACE, "T_CONN_RES: invalid primitive format");
	goto error;
      outstate:
	err = TOUTSTATE;
	mi_strlog(q, 0, SL_TRACE, "T_CONN_RES: would place interface out of state");
	goto error;
      error:
	return t_error_ack(sc, q, NULL, T_CONN_RES, err);
}

/*
 *  T_DISCON_REQ
 *  -----------------------------------
 */
static int
t_discon_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	mblk_t *cp = NULL;
	struct T_discon_req *p = (typeof(p)) mp->b_rptr;

	if (((1 << sccp_get_t_state(sc)) &
	     ~(TSF_WCON_CREQ | TSF_WRES_CIND | TSF_DATA_XFER | TSF_WIND_ORDREL | TSF_WREQ_ORDREL)))
		goto outstate;
	switch (sccp_get_t_state(sc)) {
	case TS_WCON_CREQ:
		sccp_set_t_state(sc, TS_WACK_DREQ6);
		break;
	case TS_WRES_CIND:
		sccp_set_t_state(sc, TS_WACK_DREQ7);
		break;
	case TS_DATA_XFER:
		sccp_set_t_state(sc, TS_WACK_DREQ9);
		break;
	case TS_WIND_ORDREL:
		sccp_set_t_state(sc, TS_WACK_DREQ10);
		break;
	case TS_WREQ_ORDREL:
		sccp_set_t_state(sc, TS_WACK_DREQ11);
		break;
	default:
		goto outstate;
	}
	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto einval;
	if (sccp_get_t_state(sc) == TS_WACK_DREQ7 && !(cp = t_seq_check(sc, p->SEQ_number)))
		goto badseq;
	if ((err = sccp_discon_req(sc, q, cp, mp->b_cont)))
		goto error;
	if ((err = t_ok_ack(sc, q, NULL, p->PRIM_type, p->SEQ_number, 0)))
		goto error;
	return (QR_TRIMMED);
      badseq:
	err = TBADSEQ;
	mi_strlog(q, 0, SL_TRACE, "T_DISCON_REQ: connection ind reference is invalid");
	goto error;
      einval:
	err = -EINVAL;
	mi_strlog(q, 0, SL_TRACE, "T_DISCON_REQ: invalid primitive format");
	goto error;
      outstate:
	err = TOUTSTATE;
	mi_strlog(q, 0, SL_TRACE, "T_DISCON_REQ: would place interface out of state");
	goto error;
      error:
	return t_error_ack(sc, q, NULL, T_DISCON_REQ, err);
}

/*
 *  T_DATA_REQ
 *  -----------------------------------
 */
static int
t_error_reply(struct sc *sc, queue_t *q, mblk_t *msg, int err)
{
	mblk_t *mp;

	switch (err) {
	case -EBUSY:
	case -EAGAIN:
	case -ENOMEM:
	case -ENOBUFS:
	case -EDEADLK:
		return (err);
	}
	if ((mp = mi_allocb(q, 2, BPRI_HI))) {
		DB_TYPE(mp) = M_ERROR;
		*mp->b_wptr++ = err < 0 ? -err : err;
		*mp->b_wptr++ = err < 0 ? -err : err;
		freemsg(msg);
		mi_strlog(q, STRLOGTX, SL_TRACE, "<- M_ERROR");
		putnext(sc->oq, mp);
		return (0);
	}
	return (-ENOBUFS);
}
static int
t_write(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;

	if (sccp_get_t_state(sc) == TS_IDLE)
		goto discard;
	if ((1 << sccp_get_t_state(sc)) & ~(TSF_DATA_XFER | TSF_WREQ_ORDREL))
		goto eproto;
	return sccp_data_req(sc, q, NULL, 0, 0, 0, mp);
      eproto:
	err = -EPROTO;
	mi_strlog(q, 0, SL_TRACE, "M_DATA: would place interface out of state");
	goto error;
      discard:
	err = 0;
	mi_strlog(q, 0, SL_TRACE, "M_DATA: ignoring in idle state");
	goto error;
      error:
	return t_error_reply(sc, q, NULL, err);
}
static int
t_data_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	struct T_data_req *p = (typeof(p)) mp->b_rptr;

	if (sccp_get_t_state(sc) == TS_IDLE)
		goto discard;
	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto einval;
	if ((1 << sccp_get_t_state(sc)) & ~(TSF_DATA_XFER | TSF_WREQ_ORDREL))
		goto eproto;
	{
		t_uscalar_t exp = 0;	/* FIXME */
		t_uscalar_t more = 0;	/* FIXME */
		t_uscalar_t rcpt = 0;	/* FIXME */

		return sccp_data_req(sc, q, mp, exp, more, rcpt, mp->b_cont);
	}
      eproto:
	err = -EPROTO;
	mi_strlog(q, 0, SL_TRACE, "T_DATA_REQ: would place interface out of state");
	goto error;
      einval:
	err = -EINVAL;
	mi_strlog(q, 0, SL_TRACE, "T_DATA_REQ: invalid primitive format");
	goto error;
      discard:
	err = QR_DONE;
	mi_strlog(q, 0, SL_TRACE, "T_DATA_REQ: ignore in idle state");
	goto error;
      error:
	return t_error_reply(sc, q, NULL, err);
}

/*
 *  T_EXDATA_REQ
 *  -----------------------------------
 */
static int
t_exdata_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	const struct T_exdata_req *p = (typeof(p)) mp->b_rptr;

	if (sccp_get_t_state(sc) == TS_IDLE)
		goto discard;
	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto einval;
	if ((1 << sccp_get_t_state(sc)) & ~(TSF_DATA_XFER | TSF_WREQ_ORDREL))
		goto eproto;
	{
		t_uscalar_t exp = 0;	/* FIXME */
		t_uscalar_t more = 0;	/* FIXME */
		t_uscalar_t rcpt = 0;	/* FIXME */

		return sccp_data_req(sc, q, mp, exp, more, rcpt, mp->b_cont);
	}
      eproto:
	err = -EPROTO;
	mi_strlog(q, 0, SL_TRACE, "T_EXDATA_REQ: would place interface out of state");
	goto error;
      einval:
	err = -EINVAL;
	mi_strlog(q, 0, SL_TRACE, "T_EXDATA_REQ: invalid primitive format");
	goto error;
      discard:
	err = QR_DONE;
	mi_strlog(q, 0, SL_TRACE, "T_EXDATA_REQ: ignore in idle state");
	goto error;
      error:
	return t_error_reply(sc, q, NULL, err);
}

/*
 *  T_INFO_REQ           5 - Information Request
 *  -----------------------------------------------------------------
 */
static int
t_info_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	(void) mp;
	return t_info_ack(sc, q, NULL);
}

/*
 *  T_BIND_REQ           6 - Bind a TS user to a transport address
 *  -----------------------------------------------------------------
 */
static int
t_bind_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	const struct T_bind_req *p = (typeof(p)) mp->b_rptr;
	struct sccp_addr *add;
	const size_t add_len = sizeof(*add);

	if (sccp_get_t_state(sc) != TS_UNBND)
		goto toutstate;
	sccp_set_t_state(sc, TS_WACK_BREQ);
	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto einval;
	add = (typeof(add)) (mp->b_rptr + p->ADDR_offset);
	if ((mp->b_wptr < mp->b_rptr + p->ADDR_offset + p->ADDR_length)
	    || (p->ADDR_length != add_len))
		goto tbadaddr;
	/* we don't allow wildcards just yet */
	if (!add->ssn)
		goto tnoaddr;
	if (sc->cred.cr_uid != 0 && !add->ssn)
		goto tacces;
	if ((err = sccp_bind_req(sc, q, add, p->CONIND_number)))
		goto error;
	return t_bind_ack(sc, q, NULL, &sc->src);
      tacces:
	err = TACCES;
	mi_strlog(q, 0, SL_TRACE, "T_BIND_REQ: no permission for address");
	goto error;
      tnoaddr:
	err = TNOADDR;
	mi_strlog(q, 0, SL_TRACE, "T_BIND_REQ: couldn't allocate address");
	goto error;
      tbadaddr:
	err = TBADADDR;
	mi_strlog(q, 0, SL_TRACE, "T_BIND_REQ: address is invalid");
	goto error;
      einval:
	err = -EINVAL;
	mi_strlog(q, 0, SL_TRACE, "T_BIND_REQ: invalid primitive format");
	goto error;
      toutstate:
	err = TOUTSTATE;
	mi_strlog(q, 0, SL_TRACE, "T_BIND_REQ: would place i/f out of state");
	goto error;
      error:
	return t_error_ack(sc, q, NULL, T_BIND_REQ, err);
}

/*
 *  T_UNBIND_REQ         7 - Unbind TS user from transport address
 *  -----------------------------------------------------------------
 */
static int
t_unbind_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	const struct T_unbind_req *p = (typeof(p)) mp->b_rptr;

	(void) p;
	if (sccp_get_t_state(sc) != TS_IDLE)
		goto toutstate;
	sccp_set_t_state(sc, TS_WACK_UREQ);
	if ((err = sccp_unbind_req(sc, q)))
		goto error;
	return t_ok_ack(sc, q, NULL, T_UNBIND_REQ, 0, 0);
      toutstate:
	err = TOUTSTATE;
	mi_strlog(q, 0, SL_TRACE, "T_UNBIND_REQ: would place i/f out of state");
	goto error;
      error:
	return t_error_ack(sc, q, NULL, T_UNBIND_REQ, err);
}

/*
 *  T_UNITDATA_REQ      ?? - Unitdata request
 *  -----------------------------------------------------------------
 */
static int
t_uderror_reply(struct sc *sc, queue_t *q, mblk_t *mp, int etype)
{
	struct T_uderror_ind *p = (typeof(p)) mp->b_rptr;

	switch (etype) {
	case -EBUSY:
	case -EAGAIN:
	case -ENOMEM:
	case -ENOBUFS:
	case QR_DONE:
	case QR_ABSORBED:
	case QR_TRIMMED:
		mi_strlog(q, 0, SL_ERROR, "T_UDERROR_REPLY: no error condition");
		return (etype);
	}
	if (etype > 0) {
		static const int offset =
		    sizeof(struct T_uderror_ind) - sizeof(struct T_unitdata_req);
		if (offset > 0) {
			/* shift addresses, if necessary */
			bcopy(mp->b_rptr + sizeof(*p), mp->b_rptr + sizeof(*p) + offset,
			      mp->b_wptr - mp->b_rptr - sizeof(*p));
			if (p->DEST_offset)
				p->DEST_offset += offset;
			if (p->OPT_offset)
				p->OPT_offset += offset;
			mp->b_wptr += offset;
		}
		p->PRIM_type = T_UDERROR_IND;
		p->ERROR_type = etype;
	} else {
		DB_TYPE(mp) = M_ERROR;
		mp->b_wptr = mp->b_rptr;
		*mp->b_wptr++ = -etype;
		*mp->b_wptr++ = -etype;
	}
	putnext(sc->oq, mp);
	return (QR_ABSORBED);
}
static int
t_unitdata_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	size_t dlen = mp->b_cont ? msgdsize(mp->b_cont) : 0;
	const struct T_unitdata_req *p = (typeof(p)) mp->b_rptr;
	struct sccp_addr *dst;
	const size_t dst_len = sizeof(*dst);
	t_uscalar_t seq, pri, pcl, imp, ret;

	if (dlen == 0 || dlen > sc->mtu)
		goto tbaddata;
	if (sccp_get_t_state(sc) != TS_IDLE)
		goto toutstate;
	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto einval;
	dst = (typeof(dst)) (mp->b_rptr + p->DEST_offset);
	if ((mp->b_wptr < mp->b_rptr + p->DEST_offset + p->DEST_length)
	    || (p->DEST_length != dst_len))
		goto tbadaddr;
	if (sc->cred.cr_uid != 0 && dst->ssn != sc->src.ssn)
		goto eperm;
	seq = sc->sls;
	pri = sc->mp;
	pcl = sc->pcl;
	imp = sc->imp;
	ret = (sc->flags & SCCPF_RETURN_ERROR) ? 1 : 0;
	if (p->OPT_length) {
		uchar *op = mp->b_rptr + p->OPT_offset;
		uchar *oe = op + p->OPT_length;
		struct t_opthdr *oh = (typeof(oh)) op;

		for (; op + sizeof(*oh) <= oe && oh->len >= sizeof(*oh)
		     && op + oh->len <= oe; op += oh->len, oh = (typeof(oh)) op) {
			if (oh->level == T_SS7_SCCP)
				switch (oh->name) {
				case T_SCCP_SEQ_CTRL:
					seq = (*((t_uscalar_t *) (oh + 1)));
					continue;
				case T_SCCP_PRIORITY:
					pri = (*((t_uscalar_t *) (oh + 1)));
					continue;
				case T_SCCP_PCLASS:
					pcl = (*((t_uscalar_t *) (oh + 1)));
					continue;
				case T_SCCP_IMPORTANCE:
					imp = (*((t_uscalar_t *) (oh + 1)));
					continue;
				case T_SCCP_RET_ERROR:
					ret = (*((t_uscalar_t *) (oh + 1)));
					continue;
				}
		}
	}
	if ((err = sccp_unitdata_req(sc, q, dst, pcl, ret, imp, seq, pri, mp->b_cont)))
		goto error;
	return (QR_TRIMMED);
      eperm:
	err = -EPROTO;
	mi_strlog(q, 0, SL_TRACE, "T_UNITDATA_REQ: no permission to send to address");
	goto error;
      tbadaddr:
	err = -EPROTO;
	mi_strlog(q, 0, SL_TRACE, "T_UNITDATA_REQ: bad destination address");
	goto error;
      einval:
	err = -EPROTO;
	mi_strlog(q, 0, SL_TRACE, "T_UNITDATA_REQ: invalid primitive");
	goto error;
      toutstate:
	err = -EPROTO;
	mi_strlog(q, 0, SL_TRACE, "T_UNITDATA_REQ: would place i/f out of state");
	goto error;
      tbaddata:
	err = -EPROTO;
	mi_strlog(q, 0, SL_TRACE, "T_UNITDATA_REQ: bad data size");
	goto error;
      error:
	return t_uderror_reply(sc, q, mp, err);
}

/*
 *  T_OPTMGMT_REQ        9 - Options management request
 *  -----------------------------------------------------------------
 *  The T_OPTMGMT_REQ is responsible for establishing options while the stream is in the T_IDLE state.  When the
 *  stream is bound to a local address using the T_BIND_REQ, the settings of options with end-to-end significance
 *  will have an affect on how the driver response to an INIT with INIT-ACK for SCCP.  For example, the bound list
 *  of addresses is the list of addresses that will be sent in the INIT-ACK.  The number of inbound streams and
 *  outbound streams are the numbers that will be used in the INIT-ACK.
 */
/*
 *  Errors:
 *
 *  TACCES:     the user did not have proper permissions for the user of the requested options.
 *
 *  TBADFLAG:   the flags as specified were incorrect or invalid.
 *
 *  TBADOPT:    the options as specified were in an incorrect format, or they contained invalid information.
 *
 *  TOUTSTATE:  the primitive would place the transport interface out of state.
 *
 *  TNOTSUPPORT: this prmitive is not supported.
 *
 *  TSYSERR:    a system error has occured and the UNIX system error is indicated in the primitive.
 */
static int
t_optmgmt_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	const struct T_optmgmt_req *p = (typeof(p)) mp->b_rptr;

#ifdef TS_WACK_OPTREQ
	if (sccp_get_t_state(sc) == TS_IDLE)
		sccp_set_t_state(sc, TS_WACK_OPTREQ);
#endif
	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto einval;
	if (mp->b_wptr < mp->b_rptr + p->OPT_offset + p->OPT_length)
		goto tbadopt1;
	{
		t_uscalar_t flags = p->MGMT_flags;
		size_t opt_len = p->OPT_length;
		uchar *opt_ptr = mp->b_rptr + p->OPT_offset;
		struct sccp_opts ops = { 0L, NULL, };
		struct sccp_opts *opsp = NULL;

		if (!opt_len)
			goto tbadopt2;
		if ((err = sccp_parse_opts(sc, &ops, opt_ptr, opt_len)))
			goto tbadopt3;
		opsp = &ops;
		switch (flags) {
		case T_CHECK:
			sccp_opt_check(sc, opsp);
			return t_optmgmt_ack(sc, q, NULL, flags, opsp);
		case T_NEGOTIATE:
			sccp_opt_negotiate(sc, opsp);
			return t_optmgmt_ack(sc, q, NULL, flags, opsp);
		case T_DEFAULT:
			sccp_opt_default(sc, opsp);
			return t_optmgmt_ack(sc, q, NULL, flags, opsp);
		case T_CURRENT:
			sccp_opt_current(sc, opsp);
			return t_optmgmt_ack(sc, q, NULL, flags, opsp);
		default:
			goto tbadflag;
		}
	}
      tbadflag:
	err = TBADFLAG;
	mi_strlog(q, 0, SL_TRACE, "T_OPTMGMT_REQ: bad options flags");
	goto error;
      tbadopt3:
	err = err;
	mi_strlog(q, 0, SL_TRACE, "T_OPTMGMT_REQ: option parsing error");
	goto error;
      tbadopt2:
	err = TBADOPT;
	mi_strlog(q, 0, SL_TRACE, "T_OPTMGMT_REQ: invalid options");
	goto error;
      tbadopt1:
	err = TBADOPT;
	mi_strlog(q, 0, SL_TRACE, "T_OPTMGMT_REQ: invalid options");
	goto error;
      einval:
	err = -EINVAL;
	mi_strlog(q, 0, SL_TRACE, "T_OPTMGMT_REQ: invalid primitive format");
	goto error;
      error:
	return t_error_ack(sc, q, NULL, T_OPTMGMT_REQ, err);
}

/*
 *  T_ORDREL_REQ
 *  -----------------------------------
 */
static int
t_ordrel_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;

	if ((1 << sccp_get_t_state(sc)) & ~(TSF_DATA_XFER | TSF_WREQ_ORDREL))
		goto outstate;
	switch (sccp_get_t_state(sc)) {
	case TS_DATA_XFER:
		sccp_set_t_state(sc, TS_WIND_ORDREL);
		break;
	case TS_WREQ_ORDREL:
		sccp_set_t_state(sc, TS_IDLE);
		break;
	}
	if ((err = sccp_ordrel_req(sc, q)))
		goto error;
	return (QR_DONE);
      outstate:
	err = TOUTSTATE;
	mi_strlog(q, 0, SL_TRACE, "T_ORDREL_REQ: would place interface out of state");
	goto error;
      error:
	return t_error_ack(sc, q, NULL, T_ORDREL_REQ, err);
}

/*
 *  T_OPTDATA_REQ
 *  -----------------------------------
 */
static int
t_optdata_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	int err;
	const struct T_optdata_req *p = (typeof(p)) mp->b_rptr;
	uchar *op, *oe;
	struct t_opthdr *oh;
	t_uscalar_t exp, more, rcpt;

	if (sccp_get_t_state(sc) == TS_IDLE)
		goto discard;
	if (mp->b_wptr < mp->b_rptr + sizeof(*p))
		goto einval;
	if (p->OPT_length) {
		if (mp->b_wptr < mp->b_rptr + p->OPT_offset + p->OPT_length)
			goto einval;
		/* check options ranges and format */
		for (op = mp->b_rptr + p->OPT_offset, oe = op + p->OPT_length, oh = (typeof(oh)) op;
		     op + sizeof(*oh) <= oe && oh->len >= sizeof(*oh)
		     && op + oh->len <= oe; op += oh->len, oh = (typeof(oh)) op) ;
		if (op != oe)
			goto badopt;
	}
	if ((1 << sccp_get_t_state(sc)) & ~(TSF_DATA_XFER | TSF_WREQ_ORDREL))
		goto outstate;
	if (p->OPT_length) {
		for (op = mp->b_rptr + p->OPT_offset, oe = op + p->OPT_length, oh = (typeof(oh)) op;
		     op + sizeof(*oh) <= oe && oh->len >= sizeof(*oh)
		     && op + oh->len <= oe; op += oh->len, oh = (typeof(oh)) op) {
			t_scalar_t val = *((t_scalar_t *) (oh + 1));

			switch (oh->level) {
			case T_SS7_SCCP:
				switch (oh->name) {
				case T_SCCP_PRIORITY:
					sc->mp = val;
					continue;
				case T_SCCP_IMPORTANCE:
					sc->imp = val;
					continue;
				default:
				case T_SCCP_PVAR:
				case T_SCCP_MPLEV:
				case T_SCCP_DEBUG:
				case T_SCCP_PCLASS:
				case T_SCCP_CLUSTER:
				case T_SCCP_SEQ_CTRL:
				case T_SCCP_RET_ERROR:
					/* ignore */
					continue;
				}
			}
		}
	}
	fixme(("Set these options\n"));
	exp = 0;
	more = 0;
	rcpt = 0;
	return sccp_data_req(sc, q, mp, exp, more, rcpt, mp->b_cont);
      outstate:
	err = -EPROTO;
	mi_strlog(q, 0, SL_TRACE, "T_OPTDATA_REQ: would place interface out of state");
	goto error;
      badopt:
	err = -EPROTO;
	mi_strlog(q, 0, SL_TRACE, "T_OPTDATA_REQ: invalid options format");
	goto error;
      einval:
	err = -EINVAL;
	mi_strlog(q, 0, SL_TRACE, "T_OPTDATA_REQ: invalid primitive format");
	goto error;
      discard:
	err = QR_DONE;
	mi_strlog(q, 0, SL_TRACE, "T_OPTDATA_REQ: ignore in idle state");
	goto error;
      error:
	return t_error_reply(sc, q, NULL, err);
}

#ifdef T_ADDR_REQ
/*
 *  T_ADDR_REQ          25 - address request
 *  -----------------------------------------------------------------
 */
static int
t_addr_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	(void) mp;
	switch (sccp_get_t_state(sc)) {
	case TS_UNBND:
	case TS_WACK_BREQ:
		return t_addr_ack(sc, q, NULL, NULL, NULL);
	case TS_WACK_UREQ:
	case TS_IDLE:
		return t_addr_ack(sc, q, NULL, &sc->src, NULL);
	}
	return t_error_ack(sc, q, NULL, T_ADDR_REQ, TOUTSTATE);
}
#endif

#ifdef T_CAPABILITY_REQ
/*
 *  T_CAPABILITY_REQ
 *  -----------------------------------
 */
static int
t_capability_req(struct sc *sc, queue_t *q, mblk_t *mp)
{
	fixme(("Implement this function\n"));
	return t_error_ack(sc, q, NULL, T_CAPABILITY_REQ, TNOTSUPPORT);
}
#endif

/*
 *  -------------------------------------------------------------------------
 *
 *  Timer Events
 *
 *  -------------------------------------------------------------------------
 */

/*
 *  TIMEOUT Connection Establishment
 *  -----------------------------------
 *  Delay to send an IT message on a connection section when there are no messages to send. (5 to 10 minutes)
 */
static int
sc_tcon_timeout(struct sc *sc, queue_t *q)
{
	int err;

	switch (sccp_get_state(sc)) {
	case SS_WCON_CREQ:
		/* Figure C.2/Q.714 Sheet 2 of 7: Figure 2A/T1.112.4-2000 Sheet 2 of 4: release
		   resources and local reference. Freeze local reference.  Send an N_DISCON_IND to
		   the user. Move to the idle state. */
		sccp_release(sc);
		if ((err =
		     sccp_discon_ind(sc, q, N_PROVIDER,
				     SCCP_REFC_EXPIRATION_OF_NC_ESTAB_TIMER, NULL, NULL, NULL)))
			return (err);
		sccp_set_state(sc, SS_IDLE);
		return (0);
	case SS_WCON_CREQ2:
		/* Figure C.2/Q.714 Sheet 3 of 7: Figure 2A/T1.112.4-2000 Sheet 3 of 4: In this
		   case, we have received a disconnect from the user before receiving the CC from
		   the peer.  We have not received a CC from the peer within the timeout so we will 
		   now disconnect.  Release resource and local reference.  Freeze the local
		   reference.  Move to the idle state. */
		sccp_release(sc);
		sccp_set_state(sc, SS_IDLE);
		return (0);
	default:
		swerr();
		return (0);
	}
}

/*
 *  TIMEOUT Inactivity Send
 *  -----------------------------------
 *  Waiting to receive a message on a connection section. (11 to 21 minutes)
 */
static int
sc_tias_timeout(struct sc *sc, queue_t *q)
{
	np_ulong more = 0;

	switch (sccp_get_state(sc)) {
	case SS_DATA_XFER:
		/* Figure C.4/Q.714 Sheet 4 of 4: Send an inactivity test and restart the send
		   inactivity timer. Figure 2C/T1.112.4-2002 2of2: same. */
		sccp_timer_start(sc, tias);
		return sccp_send_it(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, sc->sls, sc->dlr,
				    sc->slr, sc->pcl, sc->ps, sc->pr, more, sc->credit);
	default:
		swerr();
		return (0);
	}
}

/*
 *  TIMEOUT Inactivity Receive
 *  -----------------------------------
 *  Waiting for release complete message. (10 to 20 seconds)
 */
static int
sc_tiar_timeout(struct sc *sc, queue_t *q)
{
	int err;

	switch (sccp_get_state(sc)) {
	case SS_DATA_XFER:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
		{
			int cause = 0;

			/* Figure 2A/T1.112.4-2000 4of4: Figure 2B/T1.112.4-2000 2of2: send
			   N_DISCON_IND to user, release resource for connection section, stop
			   inactivity timers, send RLSD, start released and interval timers. */
			sccp_timer_stop(sc, tiar);
			sccp_timer_stop(sc, tias);
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, sc->sls,
					    sc->dlr, sc->slr,
					    SCCP_RELC_EXPIRATION_OF_RECEIVE_INACTIVITY_TIMER, NULL,
					    NULL)))
				return (err);
			fixme(("Figure out cause\n"));
			if ((err = sccp_discon_ind(sc, q, N_PROVIDER, cause, NULL, NULL, NULL)))
				return (err);
			sccp_timer_start(sc, trel);
			sccp_timer_start(sc, tint);
			sccp_set_state(sc, SS_WCON_DREQ);
			return (0);
		}
		default:
		case SS7_PVAR_ITUT:
		{
			/* Figure C.2/Q.714 Sheet 5 of 7: Figure C.3/Q.714 Sheet 4 of 6: send
			   N_DISCON_IND to user, stop inactivity timers, start release timer, move
			   to disconnect pending state. */
			sccp_release(sc);
			if ((err =
			     sccp_discon_ind(sc, q, N_PROVIDER,
					     SCCP_RELC_EXPIRATION_OF_RECEIVE_INACTIVITY_TIMER, NULL,
					     NULL, NULL)))
				return (err);
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, sc->sls,
					    sc->dlr, sc->slr,
					    SCCP_RELC_EXPIRATION_OF_RECEIVE_INACTIVITY_TIMER, NULL,
					    &sc->imp)))
				return (err);
			sccp_timer_start(sc, trel);
			sccp_set_state(sc, SS_WCON_DREQ);
			return (0);
		}
		}
	default:
		swerr();
		return (0);
	}
}

/*
 *  TIMEOUT Released
 *  -----------------------------------
 *  Waiting for release complete message; or to repeat sending released message after the initial T(rel) expiry. (10
 *  to 20 seconds)
 */
static int
sc_trel_timeout(struct sc *sc, queue_t *q)
{
	int err;

	switch (sccp_get_state(sc)) {
	case SS_WCON_DREQ:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure 2A/T1.112.4-2000 Sheet 4 of 4: Figure 2B/T1.112.4-2000 Sheet 2 of 
			   2: send RLSD, restart released timer. */
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, sc->sls,
					    sc->dlr, sc->slr, SCCP_RELC_UNQUALIFIED, NULL, NULL)))
				return (err);
			sccp_timer_start(sc, trel);
			return (0);
		default:
		case SS7_PVAR_ITUT:
			/* Figure C.2/Q.714 Sheet 6 of 7: Send RLSD again, start the interval
			   timer, start the repeat release timer. */
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, sc->sls,
					    sc->dlr, sc->slr, SCCP_RELC_UNQUALIFIED, NULL,
					    &sc->imp)))
				return (err);
			sccp_timer_start(sc, tint);
			sccp_timer_start(sc, trel2);
			return (0);
		}
	default:
		swerr();
		return (0);
	}
}

/*
 *  TIMEOUT Repeat Released
 *  -----------------------------------
 *  Waiting for release complete message; or to repeat sending released message after the initial T(rel) expiry. (10
 *  to 20 seconds)
 */
static int
sc_trel2_timeout(struct sc *sc, queue_t *q)
{
	int err;

	switch (sccp_get_state(sc)) {
	case SS_WCON_DREQ:
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			/* Figure C.2/Q.714 Sheet 7 of 7: send another RLSD, restart the repeat
			   release timer. */
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, sc->sls,
					    sc->dlr, sc->slr, SCCP_RELC_UNQUALIFIED, NULL, NULL)))
				return (err);
			sccp_timer_start(sc, trel2);
			return (0);
		default:
		case SS7_PVAR_ITUT:
			/* Figure C.2/Q.714 Sheet 7 of 7: send another RLSD, restart the repeat
			   release timer. */
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, sc->sls,
					    sc->dlr, sc->slr, SCCP_RELC_UNQUALIFIED, NULL,
					    &sc->imp)))
				return (err);
			sccp_timer_start(sc, trel2);
			return (0);
		}
	default:
		swerr();
		return (0);
	}
}

/*
 *  TIMEOUT Interrupt Released
 *  -----------------------------------
 *  Waiting for release complete message; or to release connection resources, fress the LRN and alert a maintenance
 *  function after the initial T(rel) expiry. (extending to 1 minute)
 */
static int
sc_tint_timeout(struct sc *sc, queue_t *q)
{
	switch (sccp_get_state(sc)) {
	case SS_WCON_DREQ:
		/* Figure C.2/Q.714 Sheet 6 of 7: inform maintenance, release resources and local
		   reference number, freeze local reference, stop release and interval timers, move 
		   to the idle state. */
		fixme(("Inform maintenance\n"));
		/* Figure 2A/T1.112.4-2000 Sheet 4 of 4: Figure 2B/T1.112.4-2000 Sheet 2 of 2: ASNI 
		   says to inform maintenance and then place the connection section into the
		   Maintenance Blocking state. */
		fixme(("Handle ANSI\n"));
		sccp_release(sc);
		sccp_set_state(sc, SS_IDLE);
		return (0);
	default:
		swerr();
		return (0);
	}
}

/*
 *  TIMEOUT Guard
 *  -----------------------------------
 *  Waiting to resume normal procedure for temporary connection sections during the restart procedure (see
 *  Q.714/3.8). (23 to 25 minutes)
 */
static int
sc_tguard_timeout(struct sc *sc, queue_t *q)
{
	switch (sccp_get_state(sc)) {
	default:
		swerr();
		return (0);
	}
}

/*
 *  TIMEOUT Reset
 *  -----------------------------------
 *  Waiting to release temporary connection section or alert maintenance function after reset request message is
 *  sent.  (10 to 20 seconds).
 */
static int
sc_tres_timeout(struct sc *sc, queue_t *q)
{
	int err;

	switch (sccp_get_state(sc)) {
	case SS_BOTH_RESET:
		/* Figure C.6/Q.174 Sheet 3 of 4: same as Figure C.6/Q.714 Sheet 2 of 4: below. */
	case SS_WCON_RREQ:
		if (1) {	/* temporary connection */
			switch (sc->proto.pvar & SS7_PVAR_MASK) {
			case SS7_PVAR_ANSI:
				/* Figure 2E/T1.112.4-2000: if this is a temporary connection,
				   release resources for the connection section, stop inactivity
				   timers, send a RLSD, start release and interval timers, move to
				   the disconnect pending state. */
				sccp_timer_stop(sc, tiar);
				sccp_timer_stop(sc, tias);
				if ((err =
				     sccp_send_rlsd(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, sc->sls,
						    sc->dlr, sc->slr,
						    SCCP_RELC_EXPIRATION_OF_RESET_TIMER, NULL,
						    NULL)))
					return (err);
				sccp_timer_start(sc, trel);
				sccp_timer_start(sc, tint);
				sccp_set_state(sc, SS_WCON_DREQ);
				return (0);
			default:
			case SS7_PVAR_ITUT:
				/* Figure C.6/Q.714 Sheet 2 of 4: If this is a temporary connection 
				   section then (Figure C.2/Q.714 Sheet 5 of 7) release resources
				   an local reference, freeze local reference, send an N_DISCON_IND 
				   to the user, stop inactivity timers and move the idle state. */
				sccp_release(sc);
				if ((err =
				     sccp_discon_ind(sc, q, N_PROVIDER,
						     SCCP_RELC_EXPIRATION_OF_RESET_TIMER, NULL,
						     NULL, NULL)))
					return (err);
				sccp_set_state(sc, SS_IDLE);
				return (0);
			}
		} else {
			/* Figure 2E/T1.112.4-2000: same.  Figure C.6/Q.714 Sheet 2 of 4: If this
			   is a permanent connection, inform maintenance and place the connection
			   section in management blocking state. */
			fixme(("Handle permanent connection\n"));
			sccp_set_state(sc, SS_IDLE);
			return (0);
		}
	default:
		swerr();
		return (0);
	}
}

/*
 *  TIMEOUT Reassembly
 *  -----------------------------------
 *  Waiting to receive all the segments of the remaining segments single segmented message after receiving the first
 *  segment.  (10 to 20 seconds).
 */
static int
sc_trea_timeout(struct sc *sc, queue_t *q)
{
	switch (sccp_get_state(sc)) {
	default:
		swerr();
		return (0);
	}
}

/*
 *  TIMEOUT Ack
 *  -----------------------------------
 *  Waiting to send a data acknowledgement. (ANSI only.)
 */
static int
sc_tack_timeout(struct sc *sc, queue_t *q)
{
	int err;

	switch (sccp_get_state(sc)) {
	case SS_DATA_XFER:
		/* Figure 2C/T1.112.4-2000 2of2: if an ack is outstanding, send an AK, clear ack
		   outstanding, reset the send inactivity timer and reset the ack timer. */
		if (sc->flags & SCCPF_ACK_OUT) {
			if ((err =
			     sccp_send_ak(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, sc->mp, sc->sls,
					  sc->dlr, sc->pr, sc->credit)))
				return (err);
			sc->flags &= ~SCCPF_ACK_OUT;
			sccp_timer_start(sc, tias);
			sccp_timer_start(sc, tack);
		}
		return (0);
	default:
		swerr();
		return (0);
	}
}

/*
 *  TIMEOUT Gtt
 *  -----------------------------------
 *  Waiting to receive a GTT result.
 */
static int
sp_tgtt_timeout(struct sp *sp, queue_t *q)
{
	fixme(("Write this function\n"));
	swerr();
	return (0);
}

/*
 *  TIMEOUT Attack
 *  -----------------------------------
 */
static int
sr_tattack_timeout(struct sr *sr, queue_t *q)
{
	fixme(("Write this function\n"));
	swerr();
	return (0);
}

/*
 *  TIMEOUT Decay
 *  -----------------------------------
 */
static int
sr_tdecay_timeout(struct sr *sr, queue_t *q)
{
	fixme(("Write this function\n"));
	swerr();
	return (0);
}

/*
 *  TIMEOUT Status Info
 *  -----------------------------------
 */
static int
sr_tstatinfo_timeout(struct sr *sr, queue_t *q)
{
	fixme(("Write this function\n"));
	swerr();
	return (0);
}

/*
 *  TIMEOUT Subssytem Test
 *  -----------------------------------
 */
static int
sr_tsst_timeout(struct sr *sr, queue_t *q)
{
	fixme(("Write this function\n"));
	swerr();
	return (0);
}

/*
 *  TIMEOUT Inhibit Subsystem Test
 *  -----------------------------------
 */
static int
ss_tisst_timeout(struct ss *ss, queue_t *q)
{
	fixme(("Write this function\n"));
	swerr();
	return (0);
}

/*
 *  TIMEOUT Waiting for SOG
 *  -----------------------------------
 */
static int
ss_twsog_timeout(struct ss *ss, queue_t *q)
{
	fixme(("Write this function\n"));
	swerr();
	return (0);
}

/*
 *  TIMEOUT Inhibit Subsystem Test
 *  -----------------------------------
 */
static int
rs_trsst_timeout(struct rs *rs, queue_t *q)
{
	fixme(("Write this function\n"));
	swerr();
	return (0);
}

/*
 *  -------------------------------------------------------------------------
 *
 *  IO Controls
 *
 *  -------------------------------------------------------------------------
 */

/*
 *  GET - Get Object Configuration
 *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
static int
sccp_get_sc(struct sccp_config *arg, struct sc *sc, int size)
{
	struct sccp_conf_sc *cnf = (typeof(cnf)) (arg + 1);

	fixme(("Write this function\n"));
	return (-EFAULT);
}
static int
sccp_get_na(struct sccp_config *arg, struct na *na, int size)
{
	struct sp *sp;
	struct sccp_conf_sp *chd;
	struct sccp_conf_na *cnf = (typeof(cnf)) (arg + 1);

	if (!na || (size -= sizeof(*cnf)) < sizeof(*arg))
		return (-EINVAL);
	cnf->proto = na->proto;
	/* write out list of signalling point configurations following na configuration */
	arg = (typeof(arg)) (cnf + 1);
	chd = (typeof(chd)) (arg + 1);
	for (sp = na->sp.list; sp && size >= sizeof(*arg) + sizeof(*chd) + sizeof(*arg);
	     sp = sp->na.next, size -= sizeof(*arg) + sizeof(*chd), arg =
	     (typeof(arg)) (chd + 1), chd = (typeof(chd)) (arg + 1)) {
		arg->type = SCCP_OBJ_TYPE_SP;
		arg->id = sp->id;
		chd->naid = sp->na.na ? sp->na.na->id : 0;
		chd->proto = sp->proto;
		chd->add = sp->add;
	}
	/* terminate list with zero object type */
	arg->type = 0;
	arg->id = 0;
	return (QR_DONE);
}
static int
sccp_get_cp(struct sccp_config *arg, struct cp *cp, int size)
{
	struct sccp_conf_cp *cnf = (typeof(cnf)) (arg + 1);

	fixme(("Write this function\n"));
	return (-EFAULT);
}
static int
sccp_get_ss(struct sccp_config *arg, struct ss *ss, int size)
{
	struct sc *sc;
	struct sccp_conf_sc *chd;
	struct sccp_conf_ss *cnf = (typeof(cnf)) (arg + 1);

	if (!ss || (size -= sizeof(*cnf)) < sizeof(*arg))
		return (-EINVAL);
	cnf->spid = ss->sp.sp ? ss->sp.sp->id : 0;
	cnf->ssn = ss->ssn;
	/* write out list of SCCP users following subsystem configuration */
	arg = (typeof(arg)) (cnf + 1);
	chd = (typeof(chd)) (arg + 1);
	/* bound CL users */
	for (sc = ss->cl.list; sc && size >= sizeof(*arg) + sizeof(*chd) + sizeof(*arg);
	     sc = sc->ss.next, size -= sizeof(*arg) + sizeof(*chd), arg =
	     (typeof(arg)) (chd + 1), chd = (typeof(chd)) (arg + 1)) {
		arg->type = SCCP_OBJ_TYPE_SC;
		arg->id = ss->id;
		todo(("Write out child info\n"));
	}
	/* bound CO users */
	for (sc = ss->co.list; sc && size >= sizeof(*arg) + sizeof(*chd) + sizeof(*arg);
	     sc = sc->ss.next, size -= sizeof(*arg) + sizeof(*chd), arg =
	     (typeof(arg)) (chd + 1), chd = (typeof(chd)) (arg + 1)) {
		arg->type = SCCP_OBJ_TYPE_SC;
		arg->id = ss->id;
		todo(("Write out child info\n"));
	}
	/* listening users */
	for (sc = ss->cr.list; sc && size >= sizeof(*arg) + sizeof(*chd) + sizeof(*arg);
	     sc = sc->ss.next, size -= sizeof(*arg) + sizeof(*chd), arg =
	     (typeof(arg)) (chd + 1), chd = (typeof(chd)) (arg + 1)) {
		arg->type = SCCP_OBJ_TYPE_SC;
		arg->id = ss->id;
		todo(("Write out child info\n"));
	}
	/* terminate list with zero object type */
	arg->type = 0;
	arg->id = 0;
	return (QR_DONE);
}
static int
sccp_get_rs(struct sccp_config *arg, struct rs *rs, int size)
{
	struct sccp_conf_rs *cnf = (typeof(cnf)) (arg + 1);

	if (!rs || (size -= sizeof(*cnf)) < sizeof(*arg))
		return (-EINVAL);
	cnf->srid = rs->sr.sr ? rs->sr.sr->id : 0;
	cnf->ssn = rs->ssn;
	arg = (typeof(arg)) (cnf + 1);
	/* terminate list with zero object type */
	arg->type = 0;
	arg->id = 0;
	return (QR_DONE);
}
static int
sccp_get_sr(struct sccp_config *arg, struct sr *sr, int size)
{
	struct rs *rs;
	struct sccp_conf_rs *chd;
	struct sccp_conf_sr *cnf = (typeof(cnf)) (arg + 1);

	if (!sr || (size -= sizeof(*cnf)) < sizeof(*arg))
		return (-EINVAL);
	cnf->spid = sr->sp.sp ? sr->sp.sp->id : 0;
	cnf->proto = sr->proto;
	cnf->add = sr->add;
	/* write out list of remote subsystems */
	arg = (typeof(arg)) (cnf + 1);
	chd = (typeof(chd)) (arg + 1);
	for (rs = sr->rs.list; rs && size >= sizeof(*arg) + sizeof(*chd) + sizeof(*arg);
	     rs = rs->sr.next, size -= sizeof(*arg) + sizeof(*chd), arg =
	     (typeof(arg)) (chd + 1), chd = (typeof(chd)) (arg + 1)) {
		arg->type = SCCP_OBJ_TYPE_RS;
		arg->id = rs->id;
		chd->srid = rs->sr.sr ? rs->sr.sr->id : 0;
		chd->ssn = rs->ssn;
	}
	/* terminate list with zero object type */
	arg->type = 0;
	arg->id = 0;
	return (QR_DONE);
}
static int
sccp_get_sp(struct sccp_config *arg, struct sp *sp, int size)
{
	struct sr *sr;
	struct ss *ss;
	struct sccp_conf_sr *chr;
	struct sccp_conf_ss *chs;
	struct sccp_conf_sp *cnf = (typeof(cnf)) (arg + 1);

	if (!sp || (size -= sizeof(*cnf)) < sizeof(*arg))
		return (-EINVAL);
	cnf->naid = sp->na.na ? sp->na.na->id : 0;
	cnf->proto = sp->proto;
	cnf->add = sp->add;
	/* write out list of signalling relations */
	arg = (typeof(arg)) (cnf + 1);
	chr = (typeof(chr)) (arg + 1);
	for (sr = sp->sr.list; sr && size >= sizeof(*arg) + sizeof(*chr) + sizeof(*arg);
	     sr = sr->sp.next, size -= sizeof(*arg) + sizeof(*chr), arg =
	     (typeof(arg)) (chr + 1), chr = (typeof(chr)) (arg + 1)) {
		arg->type = SCCP_OBJ_TYPE_SR;
		arg->id = sr->id;
		chr->spid = sr->sp.sp ? sr->sp.sp->id : 0;
		chr->proto = sr->proto;
		chr->add = sr->add;
	}
	/* write out list of local subsystems */
	chs = (typeof(chs)) chr;
	for (ss = sp->ss.list; ss && size >= sizeof(*arg) + sizeof(*chs) + sizeof(*arg);
	     ss = ss->sp.next, size -= sizeof(*arg) + sizeof(*chs), arg =
	     (typeof(arg)) (chs + 1), chs = (typeof(chs)) (arg + 1)) {
		arg->type = SCCP_OBJ_TYPE_SS;
		arg->id = ss->id;
		chs->spid = ss->sp.sp ? ss->sp.sp->id : 0;
		chs->ssn = ss->ssn;
	}
	/* terminate list with zero object type */
	arg->type = 0;
	arg->id = 0;
	return (QR_DONE);
}
static int
sccp_get_mt(struct sccp_config *arg, struct mt *mt, int size)
{
	struct sr *sr;
	struct sp *sp;
	struct sccp_conf_sr *chr;
	struct sccp_conf_sp *chp;
	struct sccp_conf_mt *cnf = (typeof(cnf)) (arg + 1);

	if (!mt || (size -= sizeof(*cnf)) < sizeof(*arg))
		return (-EINVAL);
	cnf->proto = mt->proto;
	/* write out remote signalling point or local signalling point configuration following
	   message transfer part configuration */
	arg = (typeof(arg)) (cnf + 1);
	chr = (typeof(chr)) (arg + 1);
	/* remote signalling point */
	for (sr = mt->sr; sr && size >= sizeof(*arg) + sizeof(*chr) + sizeof(*arg);
	     sr = NULL, size -= sizeof(*arg) + sizeof(*chr), arg = (typeof(arg)) (chr + 1), chr =
	     (typeof(chr)) (arg + 1)) {
		arg->type = SCCP_OBJ_TYPE_SR;
		arg->id = sr->id;
		chr->spid = sr->sp.sp ? sr->sp.sp->id : 0;
		chr->proto = sr->proto;
		chr->add = sr->add;
	}
	chp = (typeof(chp)) chr;
	/* local signalling point */
	for (sp = mt->sp; sp && size >= sizeof(*arg) + sizeof(*chp) + sizeof(*arg);
	     sp = NULL, size -= sizeof(*arg) + sizeof(*chp), arg = (typeof(arg)) (chp + 1), chp =
	     (typeof(chp)) (arg + 1)) {
		arg->type = SCCP_OBJ_TYPE_SP;
		arg->id = sp->id;
		chp->naid = sp->na.na ? sp->na.na->id : 0;
		chp->proto = sp->proto;
		chp->add = sp->add;
	}
	/* terminate list with zero object type */
	arg->type = 0;
	arg->id = 0;
	return (QR_DONE);
}
static int
sccp_get_df(struct sccp_config *arg, struct df *df, int size)
{
	struct na *na;
	struct sccp_conf_na *cha;
	struct sccp_conf_df *cnf = (typeof(cnf)) (arg + 1);

	if (!df || (size -= sizeof(*cnf)) < sizeof(*arg))
		return (-EINVAL);
	cnf->proto = df->proto;
	/* write out list of network appearances following default configuration */
	arg = (typeof(arg)) (cnf + 1);
	cha = (typeof(cha)) (arg + 1);
	for (na = df->na.list; na && size >= sizeof(*arg) + sizeof(*cha) + sizeof(*arg);
	     na = na->next, size -= sizeof(*arg) + sizeof(*cha), arg =
	     (typeof(arg)) (cha + 1), cha = (typeof(cha)) (arg + 1)) {
		arg->type = SCCP_OBJ_TYPE_NA;
		arg->id = na->id;
		cha->proto = na->proto;
	}
	/* terminate list with zero object type */
	arg->type = 0;
	arg->id = 0;
	return (QR_DONE);
}
static int
sccp_get_conf(struct sccp_config *arg, int size)
{
	switch (arg->type) {
	case SCCP_OBJ_TYPE_SC:
		return sccp_get_sc(arg, sccp_lookup(arg->id), size);
	case SCCP_OBJ_TYPE_NA:
		return sccp_get_na(arg, na_lookup(arg->id), size);
	case SCCP_OBJ_TYPE_CP:
		return sccp_get_cp(arg, cp_lookup(arg->id), size);
	case SCCP_OBJ_TYPE_SS:
		return sccp_get_ss(arg, ss_lookup(arg->id), size);
	case SCCP_OBJ_TYPE_RS:
		return sccp_get_rs(arg, rs_lookup(arg->id), size);
	case SCCP_OBJ_TYPE_SR:
		return sccp_get_sr(arg, sr_lookup(arg->id), size);
	case SCCP_OBJ_TYPE_SP:
		return sccp_get_sp(arg, sp_lookup(arg->id), size);
	case SCCP_OBJ_TYPE_MT:
		return sccp_get_mt(arg, mtp_lookup(arg->id), size);
	case SCCP_OBJ_TYPE_DF:
		return sccp_get_df(arg, &master, size);
	default:
		rare();
		return -EINVAL;
	}
}

/*
 *  ADD - Add Object Configuration
 *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
static int
sccp_add_sc(struct sccp_config *arg, struct sc *sc, int size, int force, int test)
{
	struct sccp_conf_sc *cnf = (typeof(cnf)) (arg + 1);

	if (sc || (size -= sizeof(*cnf)) < 0)
		return (-EINVAL);
	return (-EINVAL);
}
static int
sccp_add_na(struct sccp_config *arg, struct na *na, int size, int force, int test)
{
	struct sccp_conf_na *cnf = (typeof(cnf)) (arg + 1);

	if (na || (size -= sizeof(*cnf)) < 0)
		return (-EINVAL);
	if (!force) {
	}
	if (!test) {
		if (!(na = sccp_alloc_na(na_get_id(arg->id), &cnf->proto)))
			return (-ENOMEM);
		arg->id = na->id;
	}
	return (QR_DONE);
}
static int
sccp_add_cp(struct sccp_config *arg, struct cp *cp, int size, int force, int test)
{
	struct sccp_conf_cp *cnf = (typeof(cnf)) (arg + 1);

	if (cp || (size -= sizeof(*cnf)) < 0)
		return (-EINVAL);
	return (-EINVAL);
}
static int
sccp_add_ss(struct sccp_config *arg, struct ss *ss, int size, int force, int test)
{
	struct sp *sp = NULL;
	struct sccp_conf_ss *cnf = (typeof(cnf)) (arg + 1);

	if (ss || (size -= sizeof(*cnf)) < 0)
		return (-EINVAL);
	if (cnf->spid)
		sp = sp_lookup(cnf->spid);
	if (!sp)
		return (-EINVAL);
	/* local subsystem number must be unique for local signalling point */
	for (ss = sp->ss.list; ss; ss = ss->sp.next)
		if (ss->ssn == cnf->ssn)
			return (-EBUSY);
	if (!force) {
	}
	if (!test) {
		if (!(ss = sccp_alloc_ss(ss_get_id(arg->id), sp, cnf->ssn)))
			return (-ENOMEM);
		arg->id = ss->id;
	}
	return (QR_DONE);
}
static int
sccp_add_rs(struct sccp_config *arg, struct rs *rs, int size, int force, int test)
{
	struct sr *sr = NULL;
	struct sccp_conf_rs *cnf = (typeof(cnf)) (arg + 1);

	if (sr || (size -= sizeof(*cnf)) < 0)
		return (-EINVAL);
	if (cnf->srid)
		sr = sr_lookup(cnf->srid);
	if (!sr)
		return (-EINVAL);
	/* remote subsystem number must be unique for remote signalling point */
	for (rs = sr->rs.list; rs; rs = rs->sr.next)
		if (rs->ssn == cnf->ssn)
			return (-EBUSY);
	if (!force) {
	}
	if (!test) {
		if (!(rs = sccp_alloc_rs(rs_get_id(arg->id), sr, cnf->ssn)))
			return (-ENOMEM);
		arg->id = rs->id;
	}
	return (QR_DONE);
}
static int
sccp_add_sr(struct sccp_config *arg, struct sr *sr, int size, int force, int test)
{
	struct sp *sp = NULL;
	struct sccp_conf_sr *cnf = (typeof(cnf)) (arg + 1);

	if (sr || (size -= sizeof(*cnf)) < 0)
		return (-EINVAL);
	if (cnf->spid)
		sp = sp_lookup(cnf->spid);
	if (!sp)
		return (-EINVAL);
	/* network appearance of remote must match network appearance of local */
	if (cnf->add.ni)
		if (sp->add.ni != cnf->add.ni)
			return (-EINVAL);
	/* service indicator of remote must match service indicator of local */
	if (cnf->add.si)
		if (sp->add.si != cnf->add.si)
			return (-EINVAL);
	/* remote address must not be local address */
	if (sp->add.pc == cnf->add.pc)
		return (-EINVAL);
	/* remote address must be unique for local signalling point */
	for (sr = sp->sr.list; sr; sr = sr->sp.next)
		if (sr->add.pc == cnf->add.pc)
			return (-EBUSY);
	if (!force) {
	}
	if (!test) {
		if (!(sr = sccp_alloc_sr(sr_get_id(arg->id), sp, cnf->add.pc)))
			return (-ENOMEM);
		sr->proto = cnf->proto;
		arg->id = sr->id;
	}
	return (QR_DONE);
}
static int
sccp_add_sp(struct sccp_config *arg, struct sp *sp, int size, int force, int test)
{
	struct na *na = NULL;
	struct sccp_conf_sp *cnf = (typeof(cnf)) (arg + 1);

	if (sp || (size -= sizeof(*cnf)) < 0)
		return (-EINVAL);
	if (cnf->naid)
		na = na_lookup(cnf->naid);
	if (!na)
		return (-EINVAL);
	if (cnf->add.ni) {
		if (na->id != cnf->add.ni)
			return (-EINVAL);
	} else
		cnf->add.ni = na->id;
	/* local address must be unique for network appearance */
	for (sp = na->sp.list; sp; sp = sp->na.next)
		if (sp->add.ni == cnf->add.ni && sp->add.pc == cnf->add.pc
		    && sp->add.si == cnf->add.si)
			return (-EBUSY);
	if (!force) {
	}
	if (!test) {
		if (!(sp = sccp_alloc_sp(sp_get_id(arg->id), na, &cnf->add)))
			return (-ENOMEM);
		sp->proto = cnf->proto;
		arg->id = sp->id;
	}
	return (QR_DONE);
}
static int
sccp_add_mt(struct sccp_config *arg, struct mt *mt, int size, int force, int test)
{
	struct sp *sp = NULL;
	struct sr *sr = NULL;
	struct sccp_conf_mt *cnf = (typeof(cnf)) (arg + 1);

	if (mt || (size -= sizeof(*cnf)) < 0)
		return (-EINVAL);
	if (cnf->spid)
		sp = sp_lookup(cnf->spid);
	if (cnf->srid)
		sr = sr_lookup(cnf->srid);
	if (!sp || (!sr && cnf->srid))
		return (-EINVAL);
	if (sp->mt || (sr && sr->mt))
		return (-EINVAL);
	for (mt = master.mt.list; mt; mt = mt->next)
		if (mt->u.mux.index == cnf->muxid) {
			if (mt->id)
				return (-EINVAL);
			break;
		}
	if (!mt)
		return (-EINVAL);
	if (!force) {
	}
	if (!test) {
		mt->id = mtp_get_id(arg->id);
		if (sr) {
			mt->sr = sr_get(sr);
			sr->mt = mtp_get(mt);
		} else {
			mt->sp = sp_get(sp);
			sp->mt = mtp_get(mt);
		}
		mt->proto = cnf->proto;
		arg->id = mt->id;
	}
	return (QR_DONE);
}
static int
sccp_add_df(struct sccp_config *arg, struct df *df, int size, int force, int test)
{
	struct sccp_conf_df *cnf = (typeof(cnf)) (arg + 1);

	if (df || (size -= sizeof(*cnf)) < 0)
		return (-EINVAL);
	return (-EINVAL);
}
static int
sccp_add_conf(struct sccp_config *arg, int size, const int force, const int test)
{
	switch (arg->type) {
	case SCCP_OBJ_TYPE_SC:
		return sccp_add_sc(arg, sccp_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_NA:
		return sccp_add_na(arg, na_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_CP:
		return sccp_add_cp(arg, cp_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_SS:
		return sccp_add_ss(arg, ss_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_RS:
		return sccp_add_rs(arg, rs_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_SR:
		return sccp_add_sr(arg, sr_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_SP:
		return sccp_add_sp(arg, sp_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_MT:
		return sccp_add_mt(arg, mtp_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_DF:
		return sccp_add_df(arg, &master, size, force, test);
	default:
		rare();
		return -EINVAL;
	}
}

/*
 *  CHA - Change Object Configuration
 *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
static int
sccp_cha_sc(struct sccp_config *arg, struct sc *sc, int size, int force, int test)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}
static int
sccp_cha_na(struct sccp_config *arg, struct na *na, int size, int force, int test)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}
static int
sccp_cha_cp(struct sccp_config *arg, struct cp *cp, int size, int force, int test)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}
static int
sccp_cha_ss(struct sccp_config *arg, struct ss *ss, int size, int force, int test)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}
static int
sccp_cha_rs(struct sccp_config *arg, struct rs *rs, int size, int force, int test)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}
static int
sccp_cha_sr(struct sccp_config *arg, struct sr *sr, int size, int force, int test)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}
static int
sccp_cha_sp(struct sccp_config *arg, struct sp *sp, int size, int force, int test)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}
static int
sccp_cha_mt(struct sccp_config *arg, struct mt *mt, int size, int force, int test)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}
static int
sccp_cha_df(struct sccp_config *arg, struct df *df, int size, int force, int test)
{
	fixme(("Write this function\n"));
	return (-EFAULT);
}
static int
sccp_cha_conf(struct sccp_config *arg, int size, const int force, const int test)
{
	switch (arg->type) {
	case SCCP_OBJ_TYPE_SC:
		return sccp_cha_sc(arg, sccp_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_NA:
		return sccp_cha_na(arg, na_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_CP:
		return sccp_cha_cp(arg, cp_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_SS:
		return sccp_cha_ss(arg, ss_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_RS:
		return sccp_cha_rs(arg, rs_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_SR:
		return sccp_cha_sr(arg, sr_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_SP:
		return sccp_cha_sp(arg, sp_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_MT:
		return sccp_cha_mt(arg, mtp_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_DF:
		return sccp_cha_df(arg, &master, size, force, test);
	default:
		rare();
		return -EINVAL;
	}
}

/*
 *  DEL - Delete Object Configuration
 *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
static int
sccp_del_sc(struct sccp_config *arg, struct sc *sc, int size, int force, int test)
{
	if (!sc)
		return (-EINVAL);
	if (!force) {
		/* bound to subsystem */
		if (sc->ss.ss)
			return (-EBUSY);
	}
	if (!test) {
		// sccp_free_priv(sc);
	}
	return (QR_DONE);
}
static int
sccp_del_na(struct sccp_config *arg, struct na *na, int size, int force, int test)
{
	if (!na)
		return (-EINVAL);
	if (!force) {
		/* attached signalling points */
		if (na->sp.list)
			return (-EBUSY);
	}
	if (!test) {
		sccp_free_na(na);
	}
	return (QR_DONE);
}
static int
sccp_del_cp(struct sccp_config *arg, struct cp *cp, int size, int force, int test)
{
	if (!cp)
		return (-EINVAL);
	if (!force) {
	}
	if (!test) {
		sccp_free_cp(cp);
	}
	return (QR_DONE);
}
static int
sccp_del_ss(struct sccp_config *arg, struct ss *ss, int size, int force, int test)
{
	if (!ss)
		return (-EINVAL);
	if (!force) {
		/* bound CL SCCP users */
		if (ss->cl.list)
			return (-EBUSY);
		/* bound CO SCCP users */
		if (ss->co.list)
			return (-EBUSY);
		/* listening SCCP users */
		if (ss->cr.list)
			return (-EBUSY);
	}
	if (!test) {
		sccp_free_ss(ss);
	}
	return (QR_DONE);
}
static int
sccp_del_rs(struct sccp_config *arg, struct rs *rs, int size, int force, int test)
{
	if (!rs)
		return (-EINVAL);
	if (!force) {
	}
	if (!test) {
		sccp_free_rs(rs);
	}
	return (QR_DONE);
}
static int
sccp_del_sr(struct sccp_config *arg, struct sr *sr, int size, int force, int test)
{
	if (!sr)
		return (-EINVAL);
	if (!force) {
		/* attached to a live MTP interface */
		if (sr->mt)
			return (-EBUSY);
		/* we have remote subsystems */
		if (sr->rs.list)
			return (-EBUSY);
		/* we have connected SCCP users */
		if (sr->sc.list)
			return (-EBUSY);
	}
	if (!test) {
		sccp_free_sr(sr);
	}
	return (QR_DONE);
}
static int
sccp_del_sp(struct sccp_config *arg, struct sp *sp, int size, int force, int test)
{
	if (!sp)
		return (-EINVAL);
	if (!force) {
		/* bound to SCCP user */
		if (sp->sc.list)
			return (-EBUSY);
		/* attached to a live MTP interface */
		if (sp->mt)
			return (-EBUSY);
		/* we have remote signalling points associated */
		if (sp->sr.list)
			return (-EBUSY);
		/* we have local subsystems */
		if (sp->ss.list)
			return (-EBUSY);
		/* we have couplings */
		if (sp->cp.list)
			return (-EBUSY);
	}
	if (!test) {
		sccp_free_sp(sp);
	}
	return (QR_DONE);
}
static int
sccp_del_mt(struct sccp_config *arg, struct mt *mt, int size, int force, int test)
{
	if (!mt)
		return (-EINVAL);
	if (!force) {
		/* bound to internal datastructures */
		if (mt->sp || mt->sr)
			return (-EBUSY);
	}
	if (!test) {
		// sccp_free_mtp(mt);
	}
	return (QR_DONE);
}
static int
sccp_del_df(struct sccp_config *arg, struct df *df, int size, int force, int test)
{
	struct sccp_conf_df *cnf = (typeof(cnf)) (arg + 1);

	if (!df || (size -= sizeof(*cnf)) < 0)
		return (-EINVAL);
	if (!force) {
	}
	if (!test) {
		// sccp_free_df(df);
	}
	return (QR_DONE);
}
static int
sccp_del_conf(struct sccp_config *arg, int size, const int force, const int test)
{
	switch (arg->type) {
	case SCCP_OBJ_TYPE_SC:
		return sccp_del_sc(arg, sccp_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_NA:
		return sccp_del_na(arg, na_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_CP:
		return sccp_del_cp(arg, cp_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_SS:
		return sccp_del_ss(arg, ss_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_RS:
		return sccp_del_rs(arg, rs_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_SR:
		return sccp_del_sr(arg, sr_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_SP:
		return sccp_del_sp(arg, sp_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_MT:
		return sccp_del_mt(arg, mtp_lookup(arg->id), size, force, test);
	case SCCP_OBJ_TYPE_DF:
		return sccp_del_df(arg, &master, size, force, test);
	default:
		rare();
		return -EINVAL;
	}
}

/*
 *  ALL Management
 *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
static int
sccp_mgmt_all_sc(queue_t *q, struct sc *sc)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_all_na(queue_t *q, struct na *na)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_all_cp(queue_t *q, struct cp *cp)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_all_ss(queue_t *q, struct ss *ss)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_all_rs(queue_t *q, struct rs *rs)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_all_sr(queue_t *q, struct sr *sr)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_all_sp(queue_t *q, struct sp *sp)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_all_mt(queue_t *q, struct mt *mt)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_all_df(queue_t *q, struct df *df)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}

/*
 *  PRO Management
 *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
static int
sccp_mgmt_pro_sc(queue_t *q, struct sc *sc)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_pro_na(queue_t *q, struct na *na)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_pro_cp(queue_t *q, struct cp *cp)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_pro_ss(queue_t *q, struct ss *ss)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_pro_rs(queue_t *q, struct rs *rs)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_pro_sr(queue_t *q, struct sr *sr)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_pro_sp(queue_t *q, struct sp *sp)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_pro_mt(queue_t *q, struct mt *mt)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_pro_df(queue_t *q, struct df *df)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}

/*
 *  BLO Management
 *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
static int
sccp_mgmt_blo_sc(queue_t *q, struct sc *sc)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_blo_na(queue_t *q, struct na *na)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_blo_cp(queue_t *q, struct cp *cp)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_blo_ss(queue_t *q, struct ss *ss)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_blo_rs(queue_t *q, struct rs *rs)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_blo_sr(queue_t *q, struct sr *sr)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_blo_sp(queue_t *q, struct sp *sp)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_blo_mt(queue_t *q, struct mt *mt)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_blo_df(queue_t *q, struct df *df)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}

/*
 *  UBL Management
 *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
static int
sccp_mgmt_ubl_sc(queue_t *q, struct sc *sc)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_ubl_na(queue_t *q, struct na *na)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_ubl_cp(queue_t *q, struct cp *cp)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_ubl_ss(queue_t *q, struct ss *ss)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_ubl_rs(queue_t *q, struct rs *rs)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_ubl_sr(queue_t *q, struct sr *sr)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_ubl_sp(queue_t *q, struct sp *sp)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_ubl_mt(queue_t *q, struct mt *mt)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_ubl_df(queue_t *q, struct df *df)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}

/*
 *  RES Management
 *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
static int
sccp_mgmt_res_sc(queue_t *q, struct sc *sc)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_res_na(queue_t *q, struct na *na)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_res_cp(queue_t *q, struct cp *cp)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_res_ss(queue_t *q, struct ss *ss)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_res_rs(queue_t *q, struct rs *rs)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_res_sr(queue_t *q, struct sr *sr)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_res_sp(queue_t *q, struct sp *sp)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_res_mt(queue_t *q, struct mt *mt)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}
static int
sccp_mgmt_res_df(queue_t *q, struct df *df)
{
	todo(("Write this function\n"));
	return (-EOPNOTSUPP);
}

/*
 *  SCCP_IOCGOPTION
 *  -----------------------------------
 */
static int
sccp_iocgoptions(queue_t *q, mblk_t *mp)
{
	if (mp->b_cont) {
		int size = msgdsize(mp);
		sccp_option_t *arg = (typeof(arg)) mp->b_cont->b_rptr;

		if ((size -= sizeof(*arg)) >= 0) {
			switch (arg->type) {
			case SCCP_OBJ_TYPE_SC:
			{
				struct sc *sc = sccp_lookup(arg->id);
				sccp_opt_conf_sc_t *opt = (typeof(opt)) (arg + 1);

				if (!sc || (size -= sizeof(*opt)) < 0)
					goto einval;
				*opt = sc->config;
				break;
			}
			case SCCP_OBJ_TYPE_NA:
			{
				struct na *na = na_lookup(arg->id);
				sccp_opt_conf_na_t *opt = (typeof(opt)) (arg + 1);

				if (!na || (size -= sizeof(*opt)) < 0)
					goto einval;
				*opt = na->config;
				break;
			}
			case SCCP_OBJ_TYPE_CP:
			{
				struct cp *cp = cp_lookup(arg->id);
				sccp_opt_conf_cp_t *opt = (typeof(opt)) (arg + 1);

				if (!cp || (size -= sizeof(*opt)) < 0)
					goto einval;
				*opt = cp->config;
				break;
			}
			case SCCP_OBJ_TYPE_SS:
			{
				struct ss *ss = ss_lookup(arg->id);
				sccp_opt_conf_ss_t *opt = (typeof(opt)) (arg + 1);

				if (!ss || (size -= sizeof(*opt)) < 0)
					goto einval;
				*opt = ss->config;
				break;
			}
			case SCCP_OBJ_TYPE_RS:
			{
				struct rs *rs = rs_lookup(arg->id);
				sccp_opt_conf_rs_t *opt = (typeof(opt)) (arg + 1);

				if (!rs || (size -= sizeof(*opt)) < 0)
					goto einval;
				*opt = rs->config;
				break;
			}
			case SCCP_OBJ_TYPE_SR:
			{
				struct sr *sr = sr_lookup(arg->id);
				sccp_opt_conf_sr_t *opt = (typeof(opt)) (arg + 1);

				if (!sr || (size -= sizeof(*opt)) < 0)
					goto einval;
				*opt = sr->config;
				break;
			}
			case SCCP_OBJ_TYPE_SP:
			{
				struct sp *sp = sp_lookup(arg->id);
				sccp_opt_conf_sp_t *opt = (typeof(opt)) (arg + 1);

				if (!sp || (size -= sizeof(*opt)) < 0)
					goto einval;
				*opt = sp->config;
				break;
			}
			case SCCP_OBJ_TYPE_MT:
			{
				struct mt *mt = mtp_lookup(arg->id);
				sccp_opt_conf_mt_t *opt = (typeof(opt)) (arg + 1);

				if (!mt || (size -= sizeof(*opt)) < 0)
					goto einval;
				*opt = mt->config;
				break;
			}
			case SCCP_OBJ_TYPE_DF:
			{
				struct df *df = &master;
				sccp_opt_conf_df_t *opt = (typeof(opt)) (arg + 1);

				if (!df || (size -= sizeof(*opt)) < 0)
					goto einval;
				*opt = df->config;
				break;
			}
			default:
				rare();
			      einval:
				return (-EINVAL);
			}
			return (QR_DONE);
		}
	}
	rare();
	return (-EINVAL);
}

/*
 *  SCCP_IOCSOPTION
 *  -----------------------------------
 */
static int
sccp_iocsoption(queue_t *q, mblk_t *mp)
{
	if (mp->b_cont) {
		int size = msgdsize(mp);
		sccp_option_t *arg = (typeof(arg)) mp->b_cont->b_rptr;

		if ((size -= sizeof(*arg)) >= 0) {
			switch (arg->type) {
			case SCCP_OBJ_TYPE_SC:
			{
				struct sc *sc = sccp_lookup(arg->id);
				sccp_opt_conf_sc_t *opt = (typeof(opt)) (arg + 1);

				if (!sc || (size -= sizeof(*opt)) < 0)
					goto einval;
				sc->config = *opt;
				break;
			}
			case SCCP_OBJ_TYPE_NA:
			{
				struct na *na = na_lookup(arg->id);
				sccp_opt_conf_na_t *opt = (typeof(opt)) (arg + 1);

				if (!na || (size -= sizeof(*opt)) < 0)
					goto einval;
				na->config = *opt;
				break;
			}
			case SCCP_OBJ_TYPE_CP:
			{
				struct cp *cp = cp_lookup(arg->id);
				sccp_opt_conf_cp_t *opt = (typeof(opt)) (arg + 1);

				if (!cp || (size -= sizeof(*opt)) < 0)
					goto einval;
				cp->config = *opt;
				break;
			}
			case SCCP_OBJ_TYPE_SS:
			{
				struct ss *ss = ss_lookup(arg->id);
				sccp_opt_conf_ss_t *opt = (typeof(opt)) (arg + 1);

				if (!ss || (size -= sizeof(*opt)) < 0)
					goto einval;
				ss->config = *opt;
				break;
			}
			case SCCP_OBJ_TYPE_RS:
			{
				struct rs *rs = rs_lookup(arg->id);
				sccp_opt_conf_rs_t *opt = (typeof(opt)) (arg + 1);

				if (!rs || (size -= sizeof(*opt)) < 0)
					goto einval;
				rs->config = *opt;
				break;
			}
			case SCCP_OBJ_TYPE_SR:
			{
				struct sr *sr = sr_lookup(arg->id);
				sccp_opt_conf_sr_t *opt = (typeof(opt)) (arg + 1);

				if (!sr || (size -= sizeof(*opt)) < 0)
					goto einval;
				sr->config = *opt;
				break;
			}
			case SCCP_OBJ_TYPE_SP:
			{
				struct sp *sp = sp_lookup(arg->id);
				sccp_opt_conf_sp_t *opt = (typeof(opt)) (arg + 1);

				if (!sp || (size -= sizeof(*opt)) < 0)
					goto einval;
				sp->config = *opt;
				break;
			}
			case SCCP_OBJ_TYPE_MT:
			{
				struct mt *mt = mtp_lookup(arg->id);
				sccp_opt_conf_mt_t *opt = (typeof(opt)) (arg + 1);

				if (!mt || (size -= sizeof(*opt)) < 0)
					goto einval;
				mt->config = *opt;
				break;
			}
			case SCCP_OBJ_TYPE_DF:
			{
				struct df *df = &master;
				sccp_opt_conf_df_t *opt = (typeof(opt)) (arg + 1);

				if (!df || (size -= sizeof(*opt)) < 0)
					goto einval;
				df->config = *opt;
				break;
			}
			default:
				rare();
			      einval:
				return (-EINVAL);
			}
			return (QR_DONE);
		}
	}
	rare();
	return (-EINVAL);
}

/*
 *  SCCP_IOCGCONFIG
 *  -----------------------------------
 */
static int
sccp_iocgconfig(queue_t *q, mblk_t *mp)
{
	if (mp->b_cont) {
		int size = msgdsize(mp);
		struct sccp_config *arg = (typeof(arg)) mp->b_cont->b_rptr;

		if ((size -= sizeof(*arg)) >= 0)
			return sccp_get_conf(arg, size);
	}
	rare();
	return (-EINVAL);
}

/*
 *  SCCP_IOCSCONFIG
 *  -----------------------------------
 */
static int
sccp_iocsconfig(queue_t *q, mblk_t *mp)
{
	if (mp->b_cont) {
		int size = msgdsize(mp);
		struct sccp_config *arg = (typeof(arg)) mp->b_cont->b_rptr;

		if ((size -= sizeof(*arg)) >= 0)
			switch (arg->cmd) {
			case SCCP_ADD:
				return sccp_add_conf(arg, size, 0, 0);
			case SCCP_CHA:
				return sccp_cha_conf(arg, size, 0, 0);
			case SCCP_DEL:
				return sccp_del_conf(arg, size, 0, 0);
			default:
				rare();
				return (-EINVAL);
			}
	}
	rare();
	return (-EINVAL);
}

/*
 *  SCCP_IOCTCONFIG
 *  -----------------------------------
 */
static int
sccp_ioctconfig(queue_t *q, mblk_t *mp)
{
	if (mp->b_cont) {
		int size = msgdsize(mp);
		struct sccp_config *arg = (typeof(arg)) mp->b_cont->b_rptr;

		if ((size -= sizeof(*arg)) >= 0)
			switch (arg->cmd) {
			case SCCP_ADD:
				return sccp_add_conf(arg, size, 0, 1);
			case SCCP_CHA:
				return sccp_cha_conf(arg, size, 0, 1);
			case SCCP_DEL:
				return sccp_del_conf(arg, size, 0, 1);
			default:
				rare();
				return (-EINVAL);
			}
	}
	rare();
	return (-EINVAL);
}

/*
 *  SCCP_IOCCCONFIG
 *  -----------------------------------
 */
static int
sccp_ioccconfig(queue_t *q, mblk_t *mp)
{
	if (mp->b_cont) {
		int size = msgdsize(mp);
		struct sccp_config *arg = (typeof(arg)) mp->b_cont->b_rptr;

		if ((size -= sizeof(*arg)) >= 0)
			switch (arg->cmd) {
			case SCCP_ADD:
				return sccp_add_conf(arg, size, 1, 0);
			case SCCP_CHA:
				return sccp_cha_conf(arg, size, 1, 0);
			case SCCP_DEL:
				return sccp_del_conf(arg, size, 1, 0);
			default:
				rare();
				return (-EINVAL);
			}
	}
	rare();
	return (-EINVAL);
}

/*
 *  SCCP_IOCGSTATEM
 *  -----------------------------------
 */
static int
sccp_iocgstatem(queue_t *q, mblk_t *mp)
{
	if (mp->b_cont) {
		psw_t flags;
		int ret = QR_DONE;
		int size = msgdsize(mp);
		struct sccp_statem *arg = (typeof(arg)) mp->b_cont->b_rptr;

		if ((size -= sizeof(*arg)) < 0)
			return -EMSGSIZE;
		spin_lock_irqsave(&master.lock, flags);
		switch (arg->type) {
		case SCCP_OBJ_TYPE_SC:
		{
			struct sc *sc = sccp_lookup(arg->id);
			sccp_statem_sc_t *sta = (typeof(sta)) (arg + 1);

			if (!sc || (size -= sizeof(*sta)) < 0)
				goto einval;
			arg->flags = sc->flags;
			arg->state = sc->state;
			sta->timers = sc->timers;
			break;
		}
		case SCCP_OBJ_TYPE_NA:
		{
			struct na *na = na_lookup(arg->id);
			sccp_statem_na_t *sta = (typeof(sta)) (arg + 1);

			if (!na || (size -= sizeof(*sta)) < 0)
				goto einval;
			arg->flags = na->flags;
			arg->state = na->state;
			sta->timers = na->timers;
			break;
		}
		case SCCP_OBJ_TYPE_CP:
		{
			struct cp *cp = cp_lookup(arg->id);
			sccp_statem_cp_t *sta = (typeof(sta)) (arg + 1);

			if (!cp || (size -= sizeof(*sta)) < 0)
				goto einval;
			arg->flags = cp->flags;
			arg->state = cp->state;
			sta->timers = cp->timers;
			break;
		}
		case SCCP_OBJ_TYPE_SS:
		{
			struct ss *ss = ss_lookup(arg->id);
			sccp_statem_ss_t *sta = (typeof(sta)) (arg + 1);

			if (!ss || (size -= sizeof(*sta)) < 0)
				goto einval;
			arg->flags = ss->flags;
			arg->state = ss->state;
			sta->timers = ss->timers;
			break;
		}
		case SCCP_OBJ_TYPE_RS:
		{
			struct rs *rs = rs_lookup(arg->id);
			sccp_statem_rs_t *sta = (typeof(sta)) (arg + 1);

			if (!rs || (size -= sizeof(*sta)) < 0)
				goto einval;
			arg->flags = rs->flags;
			arg->state = rs->state;
			sta->timers = rs->timers;
			break;
		}
		case SCCP_OBJ_TYPE_SR:
		{
			struct sr *sr = sr_lookup(arg->id);
			sccp_statem_sr_t *sta = (typeof(sta)) (arg + 1);

			if (!sr || (size -= sizeof(*sta)) < 0)
				goto einval;
			arg->flags = sr->flags;
			arg->state = sr->state;
			sta->timers = sr->timers;
			break;
		}
		case SCCP_OBJ_TYPE_SP:
		{
			struct sp *sp = sp_lookup(arg->id);
			sccp_statem_sp_t *sta = (typeof(sta)) (arg + 1);

			if (!sp || (size -= sizeof(*sta)) < 0)
				goto einval;
			arg->flags = sp->flags;
			arg->state = sp->state;
			sta->timers = sp->timers;
			break;
		}
		case SCCP_OBJ_TYPE_MT:
		{
			struct mt *mt = mtp_lookup(arg->id);
			sccp_statem_mt_t *sta = (typeof(sta)) (arg + 1);

			if (!mt || (size -= sizeof(*sta)) < 0)
				goto einval;
			arg->flags = mt->flags;
			arg->state = mt->state;
			sta->timers = mt->timers;
			break;
		}
		case SCCP_OBJ_TYPE_DF:
		{
			struct df *df = &master;
			sccp_statem_df_t *sta = (typeof(sta)) (arg + 1);

			if (!df || (size -= sizeof(*sta)) < 0)
				goto einval;
			// arg->flags = df->flags;
			// arg->state = df->state;
			// sta->timers = df->timers;
			break;
		}
		default:
			rare();
		      einval:
			ret = -EINVAL;
			break;
		}
		spin_unlock_irqrestore(&master.lock, flags);
		return (ret);
	}
	rare();
	return (-EINVAL);
}

/*
 *  SCCP_IOCCMRESET
 *  -----------------------------------
 */
static int
sccp_ioccmreset(queue_t *q, mblk_t *mp)
{
	if (mp->b_cont) {
		int size = msgdsize(mp);
		struct sccp_statem *arg = (typeof(arg)) mp->b_cont->b_rptr;

		if ((size -= sizeof(*arg)) < 0)
			return -EMSGSIZE;
		(void) arg;
		return -EOPNOTSUPP;
	}
	rare();
	return -EINVAL;
}

/*
 *  SCCP_IOCGSTATSP
 *  -----------------------------------
 */
static int
sccp_iocgstatsp(queue_t *q, mblk_t *mp)
{
	if (mp->b_cont) {
		psw_t flags;
		int ret = QR_DONE;
		int size = msgdsize(mp);
		struct sccp_stats *arg = (typeof(arg)) mp->b_cont->b_rptr;

		if ((size -= sizeof(*arg)) < 0)
			return -EMSGSIZE;
		spin_lock_irqsave(&master.lock, flags);
		switch (arg->type) {
		case SCCP_OBJ_TYPE_SC:
		{
			struct sc *sc = sccp_lookup(arg->id);
			sccp_stats_sc_t *sta = (typeof(sta)) (arg + 1);

			if (!sc || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = sc->statsp;
			break;
		}
		case SCCP_OBJ_TYPE_NA:
		{
			struct na *na = na_lookup(arg->id);
			sccp_stats_na_t *sta = (typeof(sta)) (arg + 1);

			if (!na || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = na->statsp;
			break;
		}
		case SCCP_OBJ_TYPE_CP:
		{
			struct cp *cp = cp_lookup(arg->id);
			sccp_stats_cp_t *sta = (typeof(sta)) (arg + 1);

			if (!cp || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = cp->statsp;
			break;
		}
		case SCCP_OBJ_TYPE_SS:
		{
			struct ss *ss = ss_lookup(arg->id);
			sccp_stats_ss_t *sta = (typeof(sta)) (arg + 1);

			if (!ss || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = ss->statsp;
			break;
		}
		case SCCP_OBJ_TYPE_RS:
		{
			struct rs *rs = rs_lookup(arg->id);
			sccp_stats_rs_t *sta = (typeof(sta)) (arg + 1);

			if (!rs || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = rs->statsp;
			break;
		}
		case SCCP_OBJ_TYPE_SR:
		{
			struct sr *sr = sr_lookup(arg->id);
			sccp_stats_sr_t *sta = (typeof(sta)) (arg + 1);

			if (!sr || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = sr->statsp;
			break;
		}
		case SCCP_OBJ_TYPE_SP:
		{
			struct sp *sp = sp_lookup(arg->id);
			sccp_stats_sp_t *sta = (typeof(sta)) (arg + 1);

			if (!sp || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = sp->statsp;
			break;
		}
		case SCCP_OBJ_TYPE_MT:
		{
			struct mt *mt = mtp_lookup(arg->id);
			sccp_stats_mt_t *sta = (typeof(sta)) (arg + 1);

			if (!mt || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = mt->statsp;
			break;
		}
		case SCCP_OBJ_TYPE_DF:
		{
			struct df *df = &master;
			sccp_stats_df_t *sta = (typeof(sta)) (arg + 1);

			if (!df || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = df->statsp;
			break;
		}
		default:
			rare();
		      einval:
			ret = -EINVAL;
			break;
		}
		spin_unlock_irqrestore(&master.lock, flags);
		return (ret);
	}
	rare();
	return -EINVAL;
}

/*
 *  SCCP_IOCSSTATSP
 *  -----------------------------------
 */
static int
sccp_iocsstatsp(queue_t *q, mblk_t *mp)
{
	if (mp->b_cont) {
		psw_t flags;
		int ret = QR_DONE;
		int size = msgdsize(mp);
		struct sccp_stats *arg = (typeof(arg)) mp->b_cont->b_rptr;

		if ((size -= sizeof(*arg)) < 0)
			return -EMSGSIZE;
		spin_lock_irqsave(&master.lock, flags);
		switch (arg->type) {
		case SCCP_OBJ_TYPE_SC:
		{
			struct sc *sc = sccp_lookup(arg->id);
			sccp_stats_sc_t *sta = (typeof(sta)) (arg + 1);

			if (!sc || (size -= sizeof(*sta)) < 0)
				goto einval;
			sc->statsp = *sta;
			break;
		}
		case SCCP_OBJ_TYPE_NA:
		{
			struct na *na = na_lookup(arg->id);
			sccp_stats_na_t *sta = (typeof(sta)) (arg + 1);

			if (!na || (size -= sizeof(*sta)) < 0)
				goto einval;
			na->statsp = *sta;
			break;
		}
		case SCCP_OBJ_TYPE_CP:
		{
			struct cp *cp = cp_lookup(arg->id);
			sccp_stats_cp_t *sta = (typeof(sta)) (arg + 1);

			if (!cp || (size -= sizeof(*sta)) < 0)
				goto einval;
			cp->statsp = *sta;
			break;
		}
		case SCCP_OBJ_TYPE_SS:
		{
			struct ss *ss = ss_lookup(arg->id);
			sccp_stats_ss_t *sta = (typeof(sta)) (arg + 1);

			if (!ss || (size -= sizeof(*sta)) < 0)
				goto einval;
			ss->statsp = *sta;
			break;
		}
		case SCCP_OBJ_TYPE_RS:
		{
			struct rs *rs = rs_lookup(arg->id);
			sccp_stats_rs_t *sta = (typeof(sta)) (arg + 1);

			if (!rs || (size -= sizeof(*sta)) < 0)
				goto einval;
			rs->statsp = *sta;
			break;
		}
		case SCCP_OBJ_TYPE_SR:
		{
			struct sr *sr = sr_lookup(arg->id);
			sccp_stats_sr_t *sta = (typeof(sta)) (arg + 1);

			if (!sr || (size -= sizeof(*sta)) < 0)
				goto einval;
			sr->statsp = *sta;
			break;
		}
		case SCCP_OBJ_TYPE_SP:
		{
			struct sp *sp = sp_lookup(arg->id);
			sccp_stats_sp_t *sta = (typeof(sta)) (arg + 1);

			if (!sp || (size -= sizeof(*sta)) < 0)
				goto einval;
			sp->statsp = *sta;
			break;
		}
		case SCCP_OBJ_TYPE_MT:
		{
			struct mt *mt = mtp_lookup(arg->id);
			sccp_stats_mt_t *sta = (typeof(sta)) (arg + 1);

			if (!mt || (size -= sizeof(*sta)) < 0)
				goto einval;
			mt->statsp = *sta;
			break;
		}
		case SCCP_OBJ_TYPE_DF:
		{
			struct df *df = &master;
			sccp_stats_df_t *sta = (typeof(sta)) (arg + 1);

			if (!df || (size -= sizeof(*sta)) < 0)
				goto einval;
			df->statsp = *sta;
			break;
		}
		default:
			rare();
		      einval:
			ret = -EINVAL;
			break;
		}
		spin_unlock_irqrestore(&master.lock, flags);
		return (ret);
	}
	rare();
	return -EINVAL;
}

/*
 *  SCCP_IOCGSTATS
 *  -----------------------------------
 */
static int
sccp_iocgstats(queue_t *q, mblk_t *mp)
{
	if (mp->b_cont) {
		psw_t flags;
		int ret = QR_DONE;
		int size = msgdsize(mp);
		struct sccp_stats *arg = (typeof(arg)) mp->b_cont->b_rptr;

		if ((size -= sizeof(*arg)) < 0)
			return -EMSGSIZE;
		spin_lock_irqsave(&master.lock, flags);
		switch (arg->type) {
		case SCCP_OBJ_TYPE_SC:
		{
			struct sc *sc = sccp_lookup(arg->id);
			sccp_stats_sc_t *sta = (typeof(sta)) (arg + 1);

			if (!sc || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = sc->stats;
			arg->header = jiffies;
			break;
		}
		case SCCP_OBJ_TYPE_NA:
		{
			struct na *na = na_lookup(arg->id);
			sccp_stats_na_t *sta = (typeof(sta)) (arg + 1);

			if (!na || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = na->stats;
			arg->header = jiffies;
			break;
		}
		case SCCP_OBJ_TYPE_CP:
		{
			struct cp *cp = cp_lookup(arg->id);
			sccp_stats_cp_t *sta = (typeof(sta)) (arg + 1);

			if (!cp || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = cp->stats;
			arg->header = jiffies;
			break;
		}
		case SCCP_OBJ_TYPE_SS:
		{
			struct ss *ss = ss_lookup(arg->id);
			sccp_stats_ss_t *sta = (typeof(sta)) (arg + 1);

			if (!ss || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = ss->stats;
			arg->header = jiffies;
			break;
		}
		case SCCP_OBJ_TYPE_RS:
		{
			struct rs *rs = rs_lookup(arg->id);
			sccp_stats_rs_t *sta = (typeof(sta)) (arg + 1);

			if (!rs || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = rs->stats;
			arg->header = jiffies;
			break;
		}
		case SCCP_OBJ_TYPE_SR:
		{
			struct sr *sr = sr_lookup(arg->id);
			sccp_stats_sr_t *sta = (typeof(sta)) (arg + 1);

			if (!sr || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = sr->stats;
			arg->header = jiffies;
			break;
		}
		case SCCP_OBJ_TYPE_SP:
		{
			struct sp *sp = sp_lookup(arg->id);
			sccp_stats_sp_t *sta = (typeof(sta)) (arg + 1);

			if (!sp || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = sp->stats;
			arg->header = jiffies;
			break;
		}
		case SCCP_OBJ_TYPE_MT:
		{
			struct mt *mt = mtp_lookup(arg->id);
			sccp_stats_mt_t *sta = (typeof(sta)) (arg + 1);

			if (!mt || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = mt->stats;
			arg->header = jiffies;
			break;
		}
		case SCCP_OBJ_TYPE_DF:
		{
			struct df *df = &master;
			sccp_stats_df_t *sta = (typeof(sta)) (arg + 1);

			if (!df || (size -= sizeof(*sta)) < 0)
				goto einval;
			*sta = df->stats;
			arg->header = jiffies;
			break;
		}
		default:
			rare();
		      einval:
			ret = -EINVAL;
			break;
		}
		spin_unlock_irqrestore(&master.lock, flags);
		return (ret);
	}
	rare();
	return -EINVAL;
}

/*
 *  SCCP_IOCCSTATS
 *  -----------------------------------
 */
static int
sccp_iocsstats(queue_t *q, mblk_t *mp)
{
	if (mp->b_cont) {
		psw_t flags;
		int ret = QR_DONE;
		int size = msgdsize(mp);
		struct sccp_stats *arg = (typeof(arg)) mp->b_cont->b_rptr;
		uchar *d, *s = (typeof(s)) (arg + 1);

		if ((size -= sizeof(*arg)) < 0)
			return -EMSGSIZE;
		spin_lock_irqsave(&master.lock, flags);
		arg->header = jiffies;
		switch (arg->type) {
		case SCCP_OBJ_TYPE_SC:
		{
			struct sc *sc = sccp_lookup(arg->id);
			sccp_stats_sc_t *sta = (typeof(sta)) (arg + 1);

			if (!sc || (size -= sizeof(*sta)) < 0)
				goto einval;
			d = (typeof(d)) & sc->stats;
			for (size = sizeof(*sta); size--; *d &= *s) ;
			break;
		}
		case SCCP_OBJ_TYPE_NA:
		{
			struct na *na = na_lookup(arg->id);
			sccp_stats_na_t *sta = (typeof(sta)) (arg + 1);

			if (!na || (size -= sizeof(*sta)) < 0)
				goto einval;
			d = (typeof(d)) & na->stats;
			for (size = sizeof(*sta); size--; *d &= *s) ;
			break;
		}
		case SCCP_OBJ_TYPE_CP:
		{
			struct cp *cp = cp_lookup(arg->id);
			sccp_stats_cp_t *sta = (typeof(sta)) (arg + 1);

			if (!cp || (size -= sizeof(*sta)) < 0)
				goto einval;
			d = (typeof(d)) & cp->stats;
			for (size = sizeof(*sta); size--; *d &= *s) ;
			break;
		}
		case SCCP_OBJ_TYPE_SS:
		{
			struct ss *ss = ss_lookup(arg->id);
			sccp_stats_ss_t *sta = (typeof(sta)) (arg + 1);

			if (!ss || (size -= sizeof(*sta)) < 0)
				goto einval;
			d = (typeof(d)) & ss->stats;
			for (size = sizeof(*sta); size--; *d &= *s) ;
			break;
		}
		case SCCP_OBJ_TYPE_RS:
		{
			struct rs *rs = rs_lookup(arg->id);
			sccp_stats_rs_t *sta = (typeof(sta)) (arg + 1);

			if (!rs || (size -= sizeof(*sta)) < 0)
				goto einval;
			d = (typeof(d)) & rs->stats;
			for (size = sizeof(*sta); size--; *d &= *s) ;
			break;
		}
		case SCCP_OBJ_TYPE_SR:
		{
			struct sr *sr = sr_lookup(arg->id);
			sccp_stats_sr_t *sta = (typeof(sta)) (arg + 1);

			if (!sr || (size -= sizeof(*sta)) < 0)
				goto einval;
			d = (typeof(d)) & sr->stats;
			for (size = sizeof(*sta); size--; *d &= *s) ;
			break;
		}
		case SCCP_OBJ_TYPE_SP:
		{
			struct sp *sp = sp_lookup(arg->id);
			sccp_stats_sp_t *sta = (typeof(sta)) (arg + 1);

			if (!sp || (size -= sizeof(*sta)) < 0)
				goto einval;
			d = (typeof(d)) & sp->stats;
			for (size = sizeof(*sta); size--; *d &= *s) ;
			break;
		}
		case SCCP_OBJ_TYPE_MT:
		{
			struct mt *mt = mtp_lookup(arg->id);
			sccp_stats_mt_t *sta = (typeof(sta)) (arg + 1);

			if (!mt || (size -= sizeof(*sta)) < 0)
				goto einval;
			d = (typeof(d)) & mt->stats;
			for (size = sizeof(*sta); size--; *d &= *s) ;
			break;
		}
		case SCCP_OBJ_TYPE_DF:
		{
			struct df *df = &master;
			sccp_stats_df_t *sta = (typeof(sta)) (arg + 1);

			if (!df || (size -= sizeof(*sta)) < 0)
				goto einval;
			d = (typeof(d)) & df->stats;
			for (size = sizeof(*sta); size--; *d &= *s) ;
			break;
		}
		default:
			rare();
		      einval:
			ret = -EINVAL;
			break;
		}
		spin_unlock_irqrestore(&master.lock, flags);
		return (ret);
	}
	rare();
	return -EINVAL;
}

/*
 *  SCCP_IOCGNOTIFY
 *  -----------------------------------
 */
static int
sccp_iocgnotify(queue_t *q, mblk_t *mp)
{
	if (mp->b_cont) {
		int size = msgdsize(mp);
		struct sccp_notify *arg = (typeof(arg)) mp->b_cont->b_rptr;

		if ((size -= sizeof(*arg)) >= 0) {
			switch (arg->type) {
			case SCCP_OBJ_TYPE_SC:
			{
				struct sc *sc = sccp_lookup(arg->id);
				sccp_notify_sc_t *not = (typeof(not)) (arg + 1);

				if (!sc || (size -= sizeof(*not)) < 0)
					goto einval;
				not->events = sc->notify.events;
				break;
			}
			case SCCP_OBJ_TYPE_NA:
			{
				struct na *na = na_lookup(arg->id);
				sccp_notify_na_t *not = (typeof(not)) (arg + 1);

				if (!na || (size -= sizeof(*not)) < 0)
					goto einval;
				not->events = na->notify.events;
				break;
			}
			case SCCP_OBJ_TYPE_CP:
			{
				struct cp *cp = cp_lookup(arg->id);
				sccp_notify_cp_t *not = (typeof(not)) (arg + 1);

				if (!cp || (size -= sizeof(*not)) < 0)
					goto einval;
				not->events = cp->notify.events;
				break;
			}
			case SCCP_OBJ_TYPE_SS:
			{
				struct ss *ss = ss_lookup(arg->id);
				sccp_notify_ss_t *not = (typeof(not)) (arg + 1);

				if (!ss || (size -= sizeof(*not)) < 0)
					goto einval;
				not->events = ss->notify.events;
				break;
			}
			case SCCP_OBJ_TYPE_RS:
			{
				struct rs *rs = rs_lookup(arg->id);
				sccp_notify_rs_t *not = (typeof(not)) (arg + 1);

				if (!rs || (size -= sizeof(*not)) < 0)
					goto einval;
				not->events = rs->notify.events;
				break;
			}
			case SCCP_OBJ_TYPE_SR:
			{
				struct sr *sr = sr_lookup(arg->id);
				sccp_notify_sr_t *not = (typeof(not)) (arg + 1);

				if (!sr || (size -= sizeof(*not)) < 0)
					goto einval;
				not->events = sr->notify.events;
				break;
			}
			case SCCP_OBJ_TYPE_SP:
			{
				struct sp *sp = sp_lookup(arg->id);
				sccp_notify_sp_t *not = (typeof(not)) (arg + 1);

				if (!sp || (size -= sizeof(*not)) < 0)
					goto einval;
				not->events = sp->notify.events;
				break;
			}
			case SCCP_OBJ_TYPE_MT:
			{
				struct mt *mt = mtp_lookup(arg->id);
				sccp_notify_mt_t *not = (typeof(not)) (arg + 1);

				if (!mt || (size -= sizeof(*not)) < 0)
					goto einval;
				not->events = mt->notify.events;
				break;
			}
			case SCCP_OBJ_TYPE_DF:
			{
				struct df *df = &master;
				sccp_notify_df_t *not = (typeof(not)) (arg + 1);

				if (!df || (size -= sizeof(*not)) < 0)
					goto einval;
				not->events = df->notify.events;
				break;
			}
			default:
				rare();
				goto einval;
			}
			return (QR_DONE);
		}
	}
	rare();
      einval:
	return -EINVAL;
}

/*
 *  SCCP_IOCSNOTIFY
 *  -----------------------------------
 */
static int
sccp_iocsnotify(queue_t *q, mblk_t *mp)
{
	if (mp->b_cont) {
		int size = msgdsize(mp);
		struct sccp_notify *arg = (typeof(arg)) mp->b_cont->b_rptr;

		if ((size -= sizeof(*arg)) >= 0) {
			switch (arg->type) {
			case SCCP_OBJ_TYPE_SC:
			{
				struct sc *sc = sccp_lookup(arg->id);
				sccp_notify_sc_t *not = (typeof(not)) (arg + 1);

				if (!sc || (size -= sizeof(*not)) < 0)
					goto einval;
				sc->notify.events |= not->events;
				break;
			}
			case SCCP_OBJ_TYPE_NA:
			{
				struct na *na = na_lookup(arg->id);
				sccp_notify_na_t *not = (typeof(not)) (arg + 1);

				if (!na || (size -= sizeof(*not)) < 0)
					goto einval;
				na->notify.events |= not->events;
				break;
			}
			case SCCP_OBJ_TYPE_CP:
			{
				struct cp *cp = cp_lookup(arg->id);
				sccp_notify_cp_t *not = (typeof(not)) (arg + 1);

				if (!cp || (size -= sizeof(*not)) < 0)
					goto einval;
				cp->notify.events |= not->events;
				break;
			}
			case SCCP_OBJ_TYPE_SS:
			{
				struct ss *ss = ss_lookup(arg->id);
				sccp_notify_ss_t *not = (typeof(not)) (arg + 1);

				if (!ss || (size -= sizeof(*not)) < 0)
					goto einval;
				ss->notify.events |= not->events;
				break;
			}
			case SCCP_OBJ_TYPE_RS:
			{
				struct rs *rs = rs_lookup(arg->id);
				sccp_notify_rs_t *not = (typeof(not)) (arg + 1);

				if (!rs || (size -= sizeof(*not)) < 0)
					goto einval;
				rs->notify.events |= not->events;
				break;
			}
			case SCCP_OBJ_TYPE_SR:
			{
				struct sr *sr = sr_lookup(arg->id);
				sccp_notify_sr_t *not = (typeof(not)) (arg + 1);

				if (!sr || (size -= sizeof(*not)) < 0)
					goto einval;
				sr->notify.events |= not->events;
				break;
			}
			case SCCP_OBJ_TYPE_SP:
			{
				struct sp *sp = sp_lookup(arg->id);
				sccp_notify_sp_t *not = (typeof(not)) (arg + 1);

				if (!sp || (size -= sizeof(*not)) < 0)
					goto einval;
				sp->notify.events |= not->events;
				break;
			}
			case SCCP_OBJ_TYPE_MT:
			{
				struct mt *mt = mtp_lookup(arg->id);
				sccp_notify_mt_t *not = (typeof(not)) (arg + 1);

				if (!mt || (size -= sizeof(*not)) < 0)
					goto einval;
				mt->notify.events |= not->events;
				break;
			}
			case SCCP_OBJ_TYPE_DF:
			{
				struct df *df = &master;
				sccp_notify_df_t *not = (typeof(not)) (arg + 1);

				if (!df || (size -= sizeof(*not)) < 0)
					goto einval;
				df->notify.events |= not->events;
				break;
			}
			default:
				rare();
				goto einval;
			}
		}
	}
	rare();
      einval:
	return -EINVAL;
}

/*
 *  SCCP_IOCCNOTIFY
 *  -----------------------------------
 */
static int
sccp_ioccnotify(queue_t *q, mblk_t *mp)
{
	if (mp->b_cont) {
		int size = msgdsize(mp);
		struct sccp_notify *arg = (typeof(arg)) mp->b_cont->b_rptr;

		if ((size -= sizeof(*arg)) >= 0) {
			switch (arg->type) {
			case SCCP_OBJ_TYPE_SC:
			{
				struct sc *sc = sccp_lookup(arg->id);
				sccp_notify_sc_t *not = (typeof(not)) (arg + 1);

				if (!sc || (size -= sizeof(*not)) < 0)
					goto einval;
				sc->notify.events &= ~not->events;
				break;
			}
			case SCCP_OBJ_TYPE_NA:
			{
				struct na *na = na_lookup(arg->id);
				sccp_notify_na_t *not = (typeof(not)) (arg + 1);

				if (!na || (size -= sizeof(*not)) < 0)
					goto einval;
				na->notify.events &= ~not->events;
				break;
			}
			case SCCP_OBJ_TYPE_CP:
			{
				struct cp *cp = cp_lookup(arg->id);
				sccp_notify_cp_t *not = (typeof(not)) (arg + 1);

				if (!cp || (size -= sizeof(*not)) < 0)
					goto einval;
				cp->notify.events &= ~not->events;
				break;
			}
			case SCCP_OBJ_TYPE_SS:
			{
				struct ss *ss = ss_lookup(arg->id);
				sccp_notify_ss_t *not = (typeof(not)) (arg + 1);

				if (!ss || (size -= sizeof(*not)) < 0)
					goto einval;
				ss->notify.events &= ~not->events;
				break;
			}
			case SCCP_OBJ_TYPE_RS:
			{
				struct rs *rs = rs_lookup(arg->id);
				sccp_notify_rs_t *not = (typeof(not)) (arg + 1);

				if (!rs || (size -= sizeof(*not)) < 0)
					goto einval;
				rs->notify.events &= ~not->events;
				break;
			}
			case SCCP_OBJ_TYPE_SR:
			{
				struct sr *sr = sr_lookup(arg->id);
				sccp_notify_sr_t *not = (typeof(not)) (arg + 1);

				if (!sr || (size -= sizeof(*not)) < 0)
					goto einval;
				sr->notify.events &= ~not->events;
				break;
			}
			case SCCP_OBJ_TYPE_SP:
			{
				struct sp *sp = sp_lookup(arg->id);
				sccp_notify_sp_t *not = (typeof(not)) (arg + 1);

				if (!sp || (size -= sizeof(*not)) < 0)
					goto einval;
				sp->notify.events &= ~not->events;
				break;
			}
			case SCCP_OBJ_TYPE_MT:
			{
				struct mt *mt = mtp_lookup(arg->id);
				sccp_notify_mt_t *not = (typeof(not)) (arg + 1);

				if (!mt || (size -= sizeof(*not)) < 0)
					goto einval;
				mt->notify.events &= ~not->events;
				break;
			}
			case SCCP_OBJ_TYPE_DF:
			{
				struct df *df = &master;
				sccp_notify_df_t *not = (typeof(not)) (arg + 1);

				if (!df || (size -= sizeof(*not)) < 0)
					goto einval;
				df->notify.events &= ~not->events;
				break;
			}
			default:
				rare();
				goto einval;
			}
			return (QR_DONE);
		}
	}
	rare();
      einval:
	return -EINVAL;
}

/*
 *  SCCP_IOCCMGMT
 *  -----------------------------------
 */
static int
sccp_ioccmgmt(queue_t *q, mblk_t *mp)
{
	if (mp->b_cont) {
		sccp_mgmt_t *arg = (typeof(arg)) mp->b_cont->b_rptr;

		switch (arg->cmd) {
		case SCCP_MGMT_ALLOW:
			switch (arg->type) {
			case SCCP_OBJ_TYPE_SC:
				return sccp_mgmt_all_sc(q, sccp_lookup(arg->id));
			case SCCP_OBJ_TYPE_NA:
				return sccp_mgmt_all_na(q, na_lookup(arg->id));
			case SCCP_OBJ_TYPE_CP:
				return sccp_mgmt_all_cp(q, cp_lookup(arg->id));
			case SCCP_OBJ_TYPE_SS:
				return sccp_mgmt_all_ss(q, ss_lookup(arg->id));
			case SCCP_OBJ_TYPE_RS:
				return sccp_mgmt_all_rs(q, rs_lookup(arg->id));
			case SCCP_OBJ_TYPE_SR:
				return sccp_mgmt_all_sr(q, sr_lookup(arg->id));
			case SCCP_OBJ_TYPE_SP:
				return sccp_mgmt_all_sp(q, sp_lookup(arg->id));
			case SCCP_OBJ_TYPE_MT:
				return sccp_mgmt_all_mt(q, mtp_lookup(arg->id));
			case SCCP_OBJ_TYPE_DF:
				return sccp_mgmt_all_df(q, &master);
			}
			break;
		case SCCP_MGMT_PROHIBIT:
			switch (arg->type) {
			case SCCP_OBJ_TYPE_SC:
				return sccp_mgmt_pro_sc(q, sccp_lookup(arg->id));
			case SCCP_OBJ_TYPE_NA:
				return sccp_mgmt_pro_na(q, na_lookup(arg->id));
			case SCCP_OBJ_TYPE_CP:
				return sccp_mgmt_pro_cp(q, cp_lookup(arg->id));
			case SCCP_OBJ_TYPE_SS:
				return sccp_mgmt_pro_ss(q, ss_lookup(arg->id));
			case SCCP_OBJ_TYPE_RS:
				return sccp_mgmt_pro_rs(q, rs_lookup(arg->id));
			case SCCP_OBJ_TYPE_SR:
				return sccp_mgmt_pro_sr(q, sr_lookup(arg->id));
			case SCCP_OBJ_TYPE_SP:
				return sccp_mgmt_pro_sp(q, sp_lookup(arg->id));
			case SCCP_OBJ_TYPE_MT:
				return sccp_mgmt_pro_mt(q, mtp_lookup(arg->id));
			case SCCP_OBJ_TYPE_DF:
				return sccp_mgmt_pro_df(q, &master);
			}
			break;
		case SCCP_MGMT_BLOCK:
			switch (arg->type) {
			case SCCP_OBJ_TYPE_SC:
				return sccp_mgmt_blo_sc(q, sccp_lookup(arg->id));
			case SCCP_OBJ_TYPE_NA:
				return sccp_mgmt_blo_na(q, na_lookup(arg->id));
			case SCCP_OBJ_TYPE_CP:
				return sccp_mgmt_blo_cp(q, cp_lookup(arg->id));
			case SCCP_OBJ_TYPE_SS:
				return sccp_mgmt_blo_ss(q, ss_lookup(arg->id));
			case SCCP_OBJ_TYPE_RS:
				return sccp_mgmt_blo_rs(q, rs_lookup(arg->id));
			case SCCP_OBJ_TYPE_SR:
				return sccp_mgmt_blo_sr(q, sr_lookup(arg->id));
			case SCCP_OBJ_TYPE_SP:
				return sccp_mgmt_blo_sp(q, sp_lookup(arg->id));
			case SCCP_OBJ_TYPE_MT:
				return sccp_mgmt_blo_mt(q, mtp_lookup(arg->id));
			case SCCP_OBJ_TYPE_DF:
				return sccp_mgmt_blo_df(q, &master);
			}
			break;
		case SCCP_MGMT_UNBLOCK:
			switch (arg->type) {
			case SCCP_OBJ_TYPE_SC:
				return sccp_mgmt_ubl_sc(q, sccp_lookup(arg->id));
			case SCCP_OBJ_TYPE_NA:
				return sccp_mgmt_ubl_na(q, na_lookup(arg->id));
			case SCCP_OBJ_TYPE_CP:
				return sccp_mgmt_ubl_cp(q, cp_lookup(arg->id));
			case SCCP_OBJ_TYPE_SS:
				return sccp_mgmt_ubl_ss(q, ss_lookup(arg->id));
			case SCCP_OBJ_TYPE_RS:
				return sccp_mgmt_ubl_rs(q, rs_lookup(arg->id));
			case SCCP_OBJ_TYPE_SR:
				return sccp_mgmt_ubl_sr(q, sr_lookup(arg->id));
			case SCCP_OBJ_TYPE_SP:
				return sccp_mgmt_ubl_sp(q, sp_lookup(arg->id));
			case SCCP_OBJ_TYPE_MT:
				return sccp_mgmt_ubl_mt(q, mtp_lookup(arg->id));
			case SCCP_OBJ_TYPE_DF:
				return sccp_mgmt_ubl_df(q, &master);
			}
			break;
		case SCCP_MGMT_RESTART:
			switch (arg->type) {
			case SCCP_OBJ_TYPE_SC:
				return sccp_mgmt_res_sc(q, sccp_lookup(arg->id));
			case SCCP_OBJ_TYPE_NA:
				return sccp_mgmt_res_na(q, na_lookup(arg->id));
			case SCCP_OBJ_TYPE_CP:
				return sccp_mgmt_res_cp(q, cp_lookup(arg->id));
			case SCCP_OBJ_TYPE_SS:
				return sccp_mgmt_res_ss(q, ss_lookup(arg->id));
			case SCCP_OBJ_TYPE_RS:
				return sccp_mgmt_res_rs(q, rs_lookup(arg->id));
			case SCCP_OBJ_TYPE_SR:
				return sccp_mgmt_res_sr(q, sr_lookup(arg->id));
			case SCCP_OBJ_TYPE_SP:
				return sccp_mgmt_res_sp(q, sp_lookup(arg->id));
			case SCCP_OBJ_TYPE_MT:
				return sccp_mgmt_res_mt(q, mtp_lookup(arg->id));
			case SCCP_OBJ_TYPE_DF:
				return sccp_mgmt_res_df(q, &master);
			}
			break;
		}
	}
	rare();
	return (-EINVAL);
}

/*
 *  SCCP_IOCCPASS
 *  -----------------------------------
 */
static int
sccp_ioccpass(queue_t *q, mblk_t *mp)
{
	if (mp->b_cont) {
		sccp_pass_t *arg = (typeof(arg)) mp->b_cont->b_rptr;
		mblk_t *bp, *dp;
		struct mt *mt;

		for (mt = master.mt.list; mt && mt->u.mux.index != arg->muxid; mt = mt->next) ;
		if (!mt || !mt->oq)
			return -EINVAL;
		if (arg->type < QPCTL && !canput(mt->oq))
			return -EBUSY;
		if (!(bp = ss7_dupb(q, mp)))
			return -ENOBUFS;
		if (!(dp = ss7_dupb(q, mp))) {
			freeb(bp);
			return -ENOBUFS;
		}
		bp->b_datap->db_type = arg->type;
		bp->b_band = arg->band;
		bp->b_cont = dp;
		bp->b_rptr += sizeof(*arg);
		bp->b_wptr = bp->b_rptr + arg->ctl_length;
		dp->b_datap->db_type = M_DATA;
		dp->b_rptr += sizeof(*arg) + arg->ctl_length;
		dp->b_wptr = dp->b_rptr + arg->dat_length;
		ss7_oput(mt->oq, bp);
		return (QR_DONE);
	}
	rare();
	return -EINVAL;
}

/*
 *  =========================================================================
 *
 *  STREAMS Message Handling
 *
 *  =========================================================================
 */
/*
 *  -------------------------------------------------------------------------
 *
 *  M_IOCTL Handling
 *
 *  -------------------------------------------------------------------------
 */
static int
sccp_w_ioctl(queue_t *q, mblk_t *mp)
{
	struct sc *sc = SCCP_PRIV(q);
	struct iocblk *iocp = (struct iocblk *) mp->b_rptr;
	void *arg = mp->b_cont ? mp->b_cont->b_rptr : NULL;
	int cmd = iocp->ioc_cmd, count = iocp->ioc_count;
	int type = _IOC_TYPE(cmd), nr = _IOC_NR(cmd), size = _IOC_SIZE(cmd);
	int ret = 0;

	(void) sc;
	switch (type) {
	case __SID:
	{
		struct mt *mt;
		struct linkblk *lb;

		if (!(lb = arg)) {
			swerr();
			ret = (-EINVAL);
			break;
		}
		switch (nr) {
		case _IOC_NR(I_PLINK):
			if (iocp->ioc_cr->cr_uid != 0) {
				ptrace(("%s: %p: ERROR: Non-root attempt to I_PLINK\n", DRV_NAME,
					sc));
				ret = -EPERM;
				break;
			}
		case _IOC_NR(I_LINK):
			if ((mt =
			     sccp_alloc_link(lb->l_qbot, &master.mt.list, lb->l_index,
					     iocp->ioc_cr)))
				break;
			ret = -ENOMEM;
			break;
		case _IOC_NR(I_PUNLINK):
			if (iocp->ioc_cr->cr_uid != 0) {
				ptrace(("%s: %p: ERROR: Non-root attempt to I_PUNLINK\n", DRV_NAME,
					sc));
				ret = -EPERM;
				break;
			}
		case _IOC_NR(I_UNLINK):
			for (mt = master.mt.list; mt; mt = mt->next)
				if (mt->u.mux.index == lb->l_index)
					break;
			if (!mt) {
				ret = -EINVAL;
				ptrace(("%s: %p: ERROR: could not find I_UNLINK muxid\n", DRV_NAME,
					sc));
				break;
			}
			sccp_free_link(mt);
			break;
		default:
		case _IOC_NR(I_STR):
			ptrace(("%s: %p: ERROR: Unsupported STREAMS ioctl %d\n", DRV_NAME, sc, nr));
			ret = (-EOPNOTSUPP);
			break;
		}
		break;
	}
	case SCCP_IOC_MAGIC:
	{
		if (count < size) {
			ret = -EINVAL;
			break;
		}
		switch (nr) {
		case _IOC_NR(SCCP_IOCGOPTION):
			printd(("%s: %p: -> SCCP_IOCGOPTION\n", DRV_NAME, sc));
			ret = sccp_iocgoptions(q, mp);
			break;
		case _IOC_NR(SCCP_IOCSOPTION):
			printd(("%s: %p: -> SCCP_IOCSOPTION\n", DRV_NAME, sc));
			ret = sccp_iocsoption(q, mp);
			break;
		case _IOC_NR(SCCP_IOCGCONFIG):
			printd(("%s: %p: -> SCCP_IOCGCONFIG\n", DRV_NAME, sc));
			ret = sccp_iocgconfig(q, mp);
			break;
		case _IOC_NR(SCCP_IOCSCONFIG):
			printd(("%s: %p: -> SCCP_IOCSCONFIG\n", DRV_NAME, sc));
			ret = sccp_iocsconfig(q, mp);
			break;
		case _IOC_NR(SCCP_IOCTCONFIG):
			printd(("%s: %p: -> SCCP_IOCTCONFIG\n", DRV_NAME, sc));
			ret = sccp_ioctconfig(q, mp);
			break;
		case _IOC_NR(SCCP_IOCCCONFIG):
			printd(("%s: %p: -> SCCP_IOCCCONFIG\n", DRV_NAME, sc));
			ret = sccp_ioccconfig(q, mp);
			break;
		case _IOC_NR(SCCP_IOCGSTATEM):
			printd(("%s: %p: -> SCCP_IOCGSTATEM\n", DRV_NAME, sc));
			ret = sccp_iocgstatem(q, mp);
			break;
		case _IOC_NR(SCCP_IOCCMRESET):
			printd(("%s: %p: -> SCCP_IOCCMRESET\n", DRV_NAME, sc));
			ret = sccp_ioccmreset(q, mp);
			break;
		case _IOC_NR(SCCP_IOCGSTATSP):
			printd(("%s: %p: -> SCCP_IOCGSTATSP\n", DRV_NAME, sc));
			ret = sccp_iocgstatsp(q, mp);
			break;
		case _IOC_NR(SCCP_IOCSSTATSP):
			printd(("%s: %p: -> SCCP_IOCSSTATSP\n", DRV_NAME, sc));
			ret = sccp_iocsstatsp(q, mp);
			break;
		case _IOC_NR(SCCP_IOCGSTATS):
			printd(("%s: %p: -> SCCP_IOCGSTATS\n", DRV_NAME, sc));
			ret = sccp_iocgstats(q, mp);
			break;
		case _IOC_NR(SCCP_IOCCSTATS):
			printd(("%s: %p: -> SCCP_IOCCSTATS\n", DRV_NAME, sc));
			ret = sccp_iocsstats(q, mp);
			break;
		case _IOC_NR(SCCP_IOCGNOTIFY):
			printd(("%s: %p: -> SCCP_IOCGNOTIFY\n", DRV_NAME, sc));
			ret = sccp_iocgnotify(q, mp);
			break;
		case _IOC_NR(SCCP_IOCSNOTIFY):
			printd(("%s: %p: -> SCCP_IOCSNOTIFY\n", DRV_NAME, sc));
			ret = sccp_iocsnotify(q, mp);
			break;
		case _IOC_NR(SCCP_IOCCNOTIFY):
			printd(("%s: %p: -> SCCP_IOCCNOTIFY\n", DRV_NAME, sc));
			ret = sccp_ioccnotify(q, mp);
			break;
		case _IOC_NR(SCCP_IOCCMGMT):
			printd(("%s: %p: -> SCCP_IOCCMGMT\n", DRV_NAME, sc));
			ret = sccp_ioccmgmt(q, mp);
			break;
		case _IOC_NR(SCCP_IOCCPASS):
			printd(("%s: %p: -> SCCP_IOCCPASS\n", DRV_NAME, sc));
			ret = sccp_ioccpass(q, mp);
			break;
		default:
			mi_strlog(q, 0, SL_TRACE, "unsupported SCCP ioctl %x", nr);
			ret = -EOPNOTSUPP;
			break;
		}
		break;
	}
		/* TODO: Need to add standard TPI/NPI ioctls */
	default:
		ret = (-EOPNOTSUPP);
		break;
	}
	if (ret > 0) {
		return (ret);
	} else if (ret == 0) {
		DB_TYPE(mp) = M_IOCACK;
		iocp->ioc_error = 0;
		iocp->ioc_rval = 0;
	} else {
		DB_TYPE(mp) = M_IOCNAK;
		iocp->ioc_error = -ret;
		iocp->ioc_rval = -1;
	}
	qreply(q, mp);
	return (QR_ABSORBED);
}

/*
 *  -------------------------------------------------------------------------
 *
 *  M_RSE, M_PCRSE Handling
 *
 *  -------------------------------------------------------------------------
 */
static inline int
sccp_w_rse(queue_t *q, mblk_t *mp)
{
	struct sc *sc;
	int err = -EAGAIN;

	if ((sc = sc_acquire(q))) {
		err = sccp_orte_msg(sc->sp.sp, q, NULL, mp);
		sc_release(sc);
	}
	return (err);
}
static inline int
sccp_r_rse(queue_t *q, mblk_t *mp)
{
	struct sc *sc;
	int err = -EAGAIN;

	if ((sc = sc_acquire(q))) {
		err = sccp_recv_msg(sc, q, NULL, mp);
		sc_release(sc);
	}
	return (err);
}
static inline int
mtp_w_rse(queue_t *q, mblk_t *mp)
{
	struct mt *mt;
	int err = -EAGAIN;

	if ((mt = mt_acquire(q))) {
		err = sccp_send_msg(mt, q, NULL, mp);
		mt_release(mt);
	}
	return (err);
}
static inline int
mtp_r_rse(queue_t *q, mblk_t *mp)
{
	struct mt *mt;
	int err = -EAGAIN;

	if ((mt = mt_acquire(q))) {
		err = sccp_irte_msg(mt, q, NULL, mp);
		mt_release(mt);
	}
	return (err);
}

/*
 *  -------------------------------------------------------------------------
 *
 *  M_PROTO, M_PCPROTO Handling
 *
 *  -------------------------------------------------------------------------
 */
static int
mtpi_r_proto(struct mt *mt, queue_t *q, mblk_t *mp)
{
	switch ((*(mtp_ulong *) mp->b_rptr)) {
	case MTP_OK_ACK:
		mi_strlog(q, STRLOGRX, SL_TRACE, "MTP_OK_ACK <-");
		return mtp_ok_ack(mt, q, mp);
	case MTP_ERROR_ACK:
		mi_strlog(q, STRLOGRX, SL_TRACE, "MTP_ERROR_ACK <-");
		return mtp_error_ack(mt, q, mp);
	case MTP_BIND_ACK:
		mi_strlog(q, STRLOGRX, SL_TRACE, "MTP_BIND_ACK <-");
		return mtp_bind_ack(mt, q, mp);
	case MTP_ADDR_ACK:
		mi_strlog(q, STRLOGRX, SL_TRACE, "MTP_ADDR_ACK <-");
		return mtp_addr_ack(mt, q, mp);
	case MTP_INFO_ACK:
		mi_strlog(q, STRLOGRX, SL_TRACE, "MTP_INFO_ACK <-");
		return mtp_info_ack(mt, q, mp);
	case MTP_OPTMGMT_ACK:
		mi_strlog(q, STRLOGRX, SL_TRACE, "MTP_OPTMGMT_ACK <-");
		return mtp_optmgmt_ack(mt, q, mp);
	case MTP_TRANSFER_IND:
		mi_strlog(q, STRLOGDA, SL_TRACE, "MTP_TRANSFER_IND <-");
		return mtp_transfer_ind(mt, q, mp);
	case MTP_PAUSE_IND:
		mi_strlog(q, STRLOGRX, SL_TRACE, "MTP_PAUSE_IND <-");
		return mtp_pause_ind(mt, q, mp);
	case MTP_RESUME_IND:
		mi_strlog(q, STRLOGRX, SL_TRACE, "MTP_RESUME_IND <-");
		return mtp_resume_ind(mt, q, mp);
	case MTP_STATUS_IND:
		mi_strlog(q, STRLOGRX, SL_TRACE, "MTP_STATUS_IND <-");
		return mtp_status_ind(mt, q, mp);
	case MTP_RESTART_BEGINS_IND:
		mi_strlog(q, STRLOGRX, SL_TRACE, "MTP_RESTART_BEGINS_IND <-");
		return mtp_restart_begins_ind(mt, q, mp);
	case MTP_RESTART_COMPLETE_IND:
		mi_strlog(q, STRLOGRX, SL_TRACE, "MTP_RESTART_COMPLETE_IND <-");
		return mtp_restart_complete_ind(mt, q, mp);
	default:
		mi_strlog(q, STRLOGRX, SL_TRACE, "MTP_????_IND <-");
		return mtp_unrecognized_prim(mt, q, mp);
	}
}
static int
mtp_r_proto(queue_t *q, mblk_t *mp)
{
	struct mt *mt;
	uint oldstate;
	int err;

	if (unlikely(mp->b_wptr < mp->b_rptr + sizeof(mtp_ulong))) {
		mi_strlog(q, 0, SL_ERROR, "primitive too short");
		freemsg(mp);
		return (0);
	}

	if (unlikely((mt = mt_acquire(q)) == NULL))
		return (-EAGAIN);

	oldstate = mtp_get_state(mt);

	if ((err = mtpi_r_proto(mt, q, mp)))
		mtp_set_state(mt, oldstate);

	mt_release(mt);
	return (err);
}

static int
sccpi_w_proto(struct sc *sc, queue_t *q, mblk_t *mp)
{
	np_ulong prim;

	switch ((prim = *(np_ulong *) mp->b_rptr)) {
	case N_CONN_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_CONN_REQ");
		return n_conn_req(sc, q, mp);
	case N_CONN_RES:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_CONN_RES");
		return n_conn_res(sc, q, mp);
	case N_DISCON_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_DISCON_REQ");
		return n_discon_req(sc, q, mp);
	case N_DATA_REQ:
		mi_strlog(q, STRLOGDA, SL_TRACE, "-> N_DATA_REQ");
		return n_data_req(sc, q, mp);
	case N_EXDATA_REQ:
		mi_strlog(q, STRLOGDA, SL_TRACE, "-> N_EXDATA_REQ");
		return n_exdata_req(sc, q, mp);
	case N_INFO_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_INFO_REQ");
		return n_info_req(sc, q, mp);
	case N_BIND_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_BIND_REQ");
		return n_bind_req(sc, q, mp);
	case N_UNBIND_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_UNBIND_REQ");
		return n_unbind_req(sc, q, mp);
	case N_UNITDATA_REQ:
		mi_strlog(q, STRLOGDA, SL_TRACE, "-> N_UNITDATA_REQ");
		return n_unitdata_req(sc, q, mp);
	case N_OPTMGMT_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_OPTMGMT_REQ");
		return n_optmgmt_req(sc, q, mp);
	case N_DATACK_REQ:
		mi_strlog(q, STRLOGDA, SL_TRACE, "-> N_DATACK_REQ");
		return n_datack_req(sc, q, mp);
	case N_RESET_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_RESET_REQ");
		return n_reset_req(sc, q, mp);
	case N_RESET_RES:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_RESET_RES");
		return n_reset_res(sc, q, mp);
/* Following are specific to SCCPI */
	case N_INFORM_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_INFORM_REQ");
		return n_inform_req(sc, q, mp);
	case N_COORD_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_COORD_REQ");
		return n_coord_req(sc, q, mp);
	case N_COORD_RES:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_COORD_RES");
		return n_coord_res(sc, q, mp);
	case N_STATE_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_STATE_REQ");
		return n_state_req(sc, q, mp);
	default:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_????_???");
		return n_error_ack(sc, q, prim, NNOTSUPPORT);
	}
}

static int
tpi_w_proto(struct sc *sc, queue_t *q, mblk_t *mp)
{
	t_scalar_t prim;

	switch ((prim = *(t_scalar_t *) mp->b_rptr)) {
	case T_CONN_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> T_CONN_REQ");
		return t_conn_req(sc, q, mp);
	case T_CONN_RES:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> T_CONN_RES");
		return t_conn_res(sc, q, mp);
	case T_DISCON_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> T_DISCON_REQ");
		return t_discon_req(sc, q, mp);
	case T_DATA_REQ:
		mi_strlog(q, STRLOGDA, SL_TRACE, "-> T_DATA_REQ");
		return t_data_req(sc, q, mp);
	case T_EXDATA_REQ:
		mi_strlog(q, STRLOGDA, SL_TRACE, "-> T_EXDATA_REQ");
		return t_exdata_req(sc, q, mp);
	case T_INFO_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> T_INFO_REQ");
		return t_info_req(sc, q, mp);
	case T_BIND_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> T_BIND_REQ");
		return t_bind_req(sc, q, mp);
	case T_UNBIND_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> T_UNBIND_REQ");
		return t_unbind_req(sc, q, mp);
	case T_UNITDATA_REQ:
		mi_strlog(q, STRLOGDA, SL_TRACE, "-> T_UNITDATA_REQ");
		return t_unitdata_req(sc, q, mp);
	case T_OPTMGMT_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> T_OPTMGMT_REQ");
		return t_optmgmt_req(sc, q, mp);
	case T_ORDREL_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> T_ORDREL_REQ");
		return t_ordrel_req(sc, q, mp);
	case T_OPTDATA_REQ:
		mi_strlog(q, STRLOGDA, SL_TRACE, "-> T_OPTDATA_REQ");
		return t_optdata_req(sc, q, mp);
#ifdef T_ADDR_REQ
	case T_ADDR_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> T_ADDR_REQ");
		return t_addr_req(sc, q, mp);
#endif
#ifdef T_CAPABILITY_REQ
	case T_CAPABILITY_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> T_CAPABILITY_REQ");
		return t_capability_req(sc, q, mp);
#endif
	default:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_????_???");
		return t_error_ack(sc, q, NULL, prim, TNOTSUPPORT);
	}
}

static int
npi_w_proto(struct sc *sc, queue_t *q, mblk_t *mp)
{
	np_ulong prim;

	switch ((prim = *(np_ulong *) mp->b_rptr)) {
	case N_CONN_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_CONN_REQ");
		return n_conn_req(sc, q, mp);
	case N_CONN_RES:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_CONN_RES");
		return n_conn_res(sc, q, mp);
	case N_DISCON_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_DISCON_REQ");
		return n_discon_req(sc, q, mp);
	case N_DATA_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_DATA_REQ");
		return n_data_req(sc, q, mp);
	case N_EXDATA_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_EXDATA_REQ");
		return n_exdata_req(sc, q, mp);
	case N_INFO_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_INFO_REQ");
		return n_info_req(sc, q, mp);
	case N_BIND_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_BIND_REQ");
		return n_bind_req(sc, q, mp);
	case N_UNBIND_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_UNBIND_REQ");
		return n_unbind_req(sc, q, mp);
	case N_UNITDATA_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_UNITDATA_REQ");
		return n_unitdata_req(sc, q, mp);
	case N_OPTMGMT_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_OPTMGMT_REQ");
		return n_optmgmt_req(sc, q, mp);
	case N_DATACK_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_DATACK_REQ");
		return n_datack_req(sc, q, mp);
	case N_RESET_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_RESET_REQ");
		return n_reset_req(sc, q, mp);
	case N_RESET_RES:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_RESET_RES");
		return n_reset_res(sc, q, mp);
	default:
		return n_error_ack(sc, q, prim, NNOTSUPPORT);
	}
}

static int
gtt_w_proto(struct sc *sc, queue_t *q, mblk_t *mp)
{
	np_ulong prim;

	switch ((prim = *(np_ulong *) mp->b_rptr)) {
	case N_BIND_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_BIND_REQ");
		return n_bind_req(sc, q, mp);
	case N_UNBIND_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_UNBIND_REQ");
		return n_unbind_req(sc, q, mp);
	case N_INFORM_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_INFORM_REQ");
		return n_inform_req(sc, q, mp);
	case N_STATE_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_STATE_REQ");
		return n_state_req(sc, q, mp);
#ifdef N_TRANSLATE_RES
	case N_TRANSLATE_RES:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_TRANSLATE_REQ");
		return n_translate_res(sc, q, mp);
#endif
	default:
		return -EOPNOTSUPP;
	}
}

static int
mgm_w_proto(struct sc *sc, queue_t *q, mblk_t *mp)
{
	np_ulong prim;

	switch ((prim = *(np_ulong *) mp->b_rptr)) {
	case N_INFORM_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_INFORM_REQ");
		return n_inform_req(sc, q, mp);
	case N_COORD_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_COORD_REQ");
		return n_coord_req(sc, q, mp);
	case N_COORD_RES:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_COORD_RES");
		return n_coord_res(sc, q, mp);
	case N_STATE_REQ:
		mi_strlog(q, STRLOGTX, SL_TRACE, "-> N_STATE_REQ");
		return n_state_req(sc, q, mp);
	default:
		return -EOPNOTSUPP;
	}
}
static inline fastcall int
sccp_w_proto(queue_t *q, mblk_t *mp)
{
	struct sc *sc;
	uint oldstate;
	int err;

	if (unlikely(mp->b_wptr < mp->b_rptr + sizeof(uint32_t))) {
		mi_strlog(q, 0, SL_ERROR, "primitive too short");
		freemsg(mp);
		return (0);
	}

	if (unlikely((sc = sc_acquire(q)) == NULL))
		return (-EDEADLK);

	oldstate = sccp_get_state(sc);

	switch (sc->i_style) {
	case SCCP_STYLE_SCCPI:
		err = sccpi_w_proto(sc, q, mp);
		break;
	case SCCP_STYLE_NPI:
		err = npi_w_proto(sc, q, mp);
		break;
	case SCCP_STYLE_TPI:
		err = tpi_w_proto(sc, q, mp);
		break;
	case SCCP_STYLE_GTT:
		err = gtt_w_proto(sc, q, mp);
		break;
	case SCCP_STYLE_MGMT:
		err = mgm_w_proto(sc, q, mp);
		break;
	default:
		mi_strlog(q, 0, SL_ERROR, "invalid stream type");
		freemsg(mp);
		err = 0;
		break;
	}
	if (err)
		sccp_set_state(sc, oldstate);
	sc_release(sc);
	return (err);
}

/*
 *  -------------------------------------------------------------------------
 *
 *  M_DATA Handling
 *
 *  -------------------------------------------------------------------------
 */
static int
sccpi_w_data(queue_t *q, mblk_t *mp)
{
	struct sc *sc = SCCP_PRIV(q);

	return n_write(sc, q, mp);
}
static int
tpi_w_data(queue_t *q, mblk_t *mp)
{
	struct sc *sc = SCCP_PRIV(q);

	return t_write(sc, q, mp);
}
static int
npi_w_data(queue_t *q, mblk_t *mp)
{
	struct sc *sc = SCCP_PRIV(q);

	return n_write(sc, q, mp);
}
static int
gtt_w_data(queue_t *q, mblk_t *mp)
{
	swerr();
	return (-EFAULT);
}
static int
mgm_w_data(queue_t *q, mblk_t *mp)
{
	swerr();
	return (-EFAULT);
}

#ifndef SCCP_TYPE_SCCPI
#define SCCP_TYPE_SCCPI	1
#define SCCP_TYPE_NPI	2
#define SCCP_TYPE_TPI	3
#define SCCP_TYPE_GTT	4
#define SCCP_TYPE_MGM	5
#endif
static inline fastcall int
sccp_w_data(queue_t *q, mblk_t *mp)
{
	struct sc *sc = SCCP_PRIV(q);

	switch (sc->type) {
	case SCCP_TYPE_SCCPI:
		return sccpi_w_data(q, mp);
	case SCCP_TYPE_NPI:
		return npi_w_data(q, mp);
	case SCCP_TYPE_TPI:
		return tpi_w_data(q, mp);
	case SCCP_TYPE_GTT:
		return gtt_w_data(q, mp);
	case SCCP_TYPE_MGM:
		return mgm_w_data(q, mp);
	}
	mi_strlog(q, 0, SL_ERROR, "invalid stream type");
	freemsg(mp);
	return (0);
}
static int
mtp_r_data(queue_t *q, mblk_t *mp)
{
	struct mt *mt = MTP_PRIV(q);
	struct sr *sr;

	if ((sr = mt->sr))
		return mtp_read(mt, q, NULL, mp, sr, 0, 0);
	mi_strlog(q, 0, SL_ERROR, "data for detached stream");
	return (-EAGAIN);
}

/*
 *  -------------------------------------------------------------------------
 *
 *  M_SIG, M_PCSIG Handling
 *
 *  -------------------------------------------------------------------------
 */
static int
do_timeout(queue_t *q, mblk_t *mp)
{
	struct sc_timer *t = (typeof(t)) mp->b_rptr;

	switch (t->timer) {
	case tcon:
		mi_strlog(q, STRLOGTO, SL_TRACE, "tcon expiry at %lu", jiffies);
		return sc_tcon_timeout(t->sc, q);
	case tias:
		mi_strlog(q, STRLOGTO, SL_TRACE, "tias expiry at %lu", jiffies);
		return sc_tias_timeout(t->sc, q);
	case tiar:
		mi_strlog(q, STRLOGTO, SL_TRACE, "tiar expiry at %lu", jiffies);
		return sc_tiar_timeout(t->sc, q);
	case trel:
		mi_strlog(q, STRLOGTO, SL_TRACE, "trel expiry at %lu", jiffies);
		return sc_trel_timeout(t->sc, q);
	case trel2:
		mi_strlog(q, STRLOGTO, SL_TRACE, "trel2 expiry at %lu", jiffies);
		return sc_trel2_timeout(t->sc, q);
	case tint:
		mi_strlog(q, STRLOGTO, SL_TRACE, "tint expiry at %lu", jiffies);
		return sc_tint_timeout(t->sc, q);
	case tguard:
		mi_strlog(q, STRLOGTO, SL_TRACE, "tguard expiry at %lu", jiffies);
		return sc_tguard_timeout(t->sc, q);
	case tres:
		mi_strlog(q, STRLOGTO, SL_TRACE, "tres expiry at %lu", jiffies);
		return sc_tres_timeout(t->sc, q);
	case trea:
		mi_strlog(q, STRLOGTO, SL_TRACE, "trea expiry at %lu", jiffies);
		return sc_trea_timeout(t->sc, q);
	case tack:
		mi_strlog(q, STRLOGTO, SL_TRACE, "tack expiry at %lu", jiffies);
		return sc_tack_timeout(t->sc, q);
	case tgtt:
		mi_strlog(q, STRLOGTO, SL_TRACE, "tgtt expiry at %lu", jiffies);
		return sp_tgtt_timeout(t->sp, q);
	case tattack:
		mi_strlog(q, STRLOGTO, SL_TRACE, "tattack expiry at %lu", jiffies);
		return sr_tattack_timeout(t->sr, q);
	case tdecay:
		mi_strlog(q, STRLOGTO, SL_TRACE, "tdecay expiry at %lu", jiffies);
		return sr_tdecay_timeout(t->sr, q);
	case tstatinfo:
		mi_strlog(q, STRLOGTO, SL_TRACE, "tstatinfo expiry at %lu", jiffies);
		return sr_tstatinfo_timeout(t->sr, q);
	case tsst:
		mi_strlog(q, STRLOGTO, SL_TRACE, "tsst expiry at %lu", jiffies);
		return sr_tsst_timeout(t->sr, q);
	case tisst:
		mi_strlog(q, STRLOGTO, SL_TRACE, "tisst expiry at %lu", jiffies);
		return ss_tisst_timeout(t->ss, q);
	case twsog:
		mi_strlog(q, STRLOGTO, SL_TRACE, "twsog expiry at %lu", jiffies);
		return ss_twsog_timeout(t->ss, q);
	case trsst:
		mi_strlog(q, STRLOGTO, SL_TRACE, "trsst expiry at %lu", jiffies);
		return rs_trsst_timeout(t->rs, q);
	default:
		mi_strlog(q, 0, SL_ERROR, "unknown timer %u", t->timer);
		return 0;
	}
}
static int
sccp_w_sig(queue_t *q, mblk_t *mp)
{
	struct sc *sc;
	uint oldstate;
	int err = 0;

	if (!(sc = sc_acquire(q)))
		return (mi_timer_requeue(mp) ? -EAGAIN : 0);

	oldstate = sccp_get_state(sc);

	if (likely(mi_timer_valid(mp))) {
		err = do_timeout(q, mp);

		if (err) {
			sccp_set_state(sc, oldstate);
			err = mi_timer_requeue(mp) ? err : 0;
		}
	}

	sc_release(sc);
	return (err);
}
static int
mtp_r_sig(queue_t *q, mblk_t *mp)
{
	struct mt *mt;
	uint oldstate;
	int err = 0;

	if (!(mt = mt_acquire(q)))
		return (mi_timer_requeue(mp) ? -EAGAIN : 0);

	oldstate = mtp_get_state(mt);

	if (likely(mi_timer_valid(mp))) {
		err = do_timeout(q, mp);

		if (err) {
			mtp_set_state(mt, oldstate);
			err = mi_timer_requeue(mp) ? err : 0;
		}
	}

	mt_release(mt);
	return (err);
}

/*
 *  -------------------------------------------------------------------------
 *
 *  M_ERROR Handling
 *
 *  -------------------------------------------------------------------------
 *  A hangup from below indicates that a message transfer part has failed badly.  Move the MTP to the out-of-service
 *  state, notify management, and perform normal MTP failure actions.
 */
static int
mtp_r_error(queue_t *q, mblk_t *mp)
{
	struct mt *mt = MTP_PRIV(q);

	(void) mt;
	// mtp_set_i_state(mt, LMI_UNUSABLE);
	fixme(("Complete this function\n"));
	return (-EFAULT);
}
static inline int
sccp_r_error(queue_t *q, mblk_t *mp)
{
	int err;
	struct sc *sc = SCCP_PRIV(q);

	switch (sccp_get_state(sc)) {
	case SS_DATA_XFER:
	{
		/* Figure C.2/Q.714 Sheet 4 of 7: Figure 2A/T1.112.4-2000 Sheet 4 of 4: Figure
		   2B/T1.112.4-2000 2of2: This is an internal disconnect.  Provide an N_DISCON_IND
		   to the user, release resources for the connection section, stop inactivity
		   timers, send a RLSD (if possible), start the release and interval timers, move
		   to the disconnect pending state. */
		sccp_release(sc);
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					    sc->slr, SCCP_RELC_MTP_FAILURE, NULL, NULL)))
				return (err);
			break;
		default:
		case SS7_PVAR_ITUT:
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					    sc->slr, SCCP_RELC_MTP_FAILURE, NULL, &sc->imp)))
				return (err);
			break;
		}
		{
			int cause = 0;

			fixme(("Determine appropriate error\n"));
			if ((err = sccp_discon_ind(sc, q, N_PROVIDER, cause, NULL, NULL, NULL)))
				return (err);
		}
		sccp_timer_start(sc, trel);
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			sccp_timer_start(sc, tint);
		}
		sccp_set_state(sc, SS_WCON_DREQ);
		return (QR_DONE);
	}
	case SS_WCON_DREQ:
		/* Figure 2B/T1.112.4-2000 2of2: release reference for the connection section, stop 
		   release and interval timer, move to the idle state. */
		sccp_release(sc);
		sccp_set_state(sc, SS_IDLE);
		return (QR_DONE);
	default:
		return (QR_PASSALONG);
	}
}

/*
 *  -------------------------------------------------------------------------
 *
 *  M_HANGUP Handling
 *
 *  -------------------------------------------------------------------------
 *  A hangup from below indicates that a message transfer part has failed badly.  Move the MTP to the out-of-service
 *  state, notify management, and perform normal MTP failure actions.
 */
static int
mtp_r_hangup(queue_t *q, mblk_t *mp)
{
	struct mt *mt = MTP_PRIV(q);

	(void) mt;
	// mtp_set_i_state(mt, LMI_UNUSABLE);
	fixme(("Complete this function\n"));
	return (-EFAULT);
}
static inline int
sccp_r_hangup(queue_t *q, mblk_t *mp)
{
	int err;
	struct sc *sc = SCCP_PRIV(q);

	switch (sccp_get_state(sc)) {
	case SS_DATA_XFER:
	{
		/* Figure C.2/Q.714 Sheet 4 of 7: Figure 2A/T1.112.4-2000 Sheet 4 of 4: Figure
		   2B/T1.112.4-2000 2of2: This is an internal disconnect.  Provide an N_DISCON_IND
		   to the user, release resources for the connection section, stop inactivity
		   timers, send a RLSD (if possible), start the release and interval timers, move
		   to the disconnect pending state. */
		sccp_release(sc);
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					    sc->slr, SCCP_RELC_MTP_FAILURE, NULL, NULL)))
				return (err);
			break;
		default:
		case SS7_PVAR_ITUT:
			if ((err =
			     sccp_send_rlsd(sc->sp.sp, q, NULL, sc->sp.sp->add.pc, sc->sls, sc->dlr,
					    sc->slr, SCCP_RELC_MTP_FAILURE, NULL, &sc->imp)))
				return (err);
			break;
		}
		{
			int cause = 0;

			fixme(("Determine appropriate error\n"));
			if ((err = sccp_discon_ind(sc, q, N_PROVIDER, cause, NULL, NULL, NULL)))
				return (err);
		}
		sccp_timer_start(sc, trel);
		switch (sc->proto.pvar & SS7_PVAR_MASK) {
		case SS7_PVAR_ANSI:
			sccp_timer_start(sc, tint);
		}
		sccp_set_state(sc, SS_WCON_DREQ);
		return (QR_DONE);
	}
	case SS_WCON_DREQ:
		/* Figure 2B/T1.112.4-2000 2of2: release reference for the connection section, stop 
		   release and interval timer, move to the idle state. */
		sccp_release(sc);
		sccp_set_state(sc, SS_IDLE);
		return (QR_DONE);
	default:
		return (QR_PASSALONG);
	}
}

/*
 *  =========================================================================
 *
 *  PUT and SRV
 *
 *  =========================================================================
 */
static noinline fastcall int
sccp_w_msg_slow(queue_t *q, mblk_t *mp)
{
	switch (DB_TYPE(mp)) {
	case M_DATA:
	case M_HPDATA:
		return sccp_w_data(q, mp);
	case M_PROTO:
	case M_PCPROTO:
		return sccp_w_proto(q, mp);
	case M_SIG:
	case M_PCSIG:
		return sccp_w_sig(q, mp);
	case M_IOCTL:
		return sccp_w_ioctl(q, mp);
#if 0
	case M_IOCDATA:
		return sccp_w_iocdata(q, mp);
#endif
	case M_FLUSH:
		return ss7_w_flush(q, mp);
	}
#if 0
	return sccp_w_other(q, mp);
#else
	freemsg(mp);
	return (0);
#endif
}
static inline fastcall int
sccp_w_msg(queue_t *q, mblk_t *mp)
{
	/* FIXME: write a fast path */
	return sccp_w_msg_slow(q, mp);
}

static noinline fastcall int
mtp_r_msg_slow(queue_t *q, mblk_t *mp)
{
	switch (DB_TYPE(mp)) {
	case M_DATA:
	case M_HPDATA:
		return mtp_r_data(q, mp);
	case M_PROTO:
	case M_PCPROTO:
		return mtp_r_proto(q, mp);
	case M_SIG:
	case M_PCSIG:
		return mtp_r_sig(q, mp);
#if 0
	case M_IOCACK:
	case M_IOCNAK:
	case M_COPYIN:
	case M_COPYOUT:
		return mtp_r_copy(q, mp);
#endif
	case M_FLUSH:
		return ss7_r_flush(q, mp);
	case M_ERROR:
		return mtp_r_error(q, mp);
	case M_HANGUP:
	case M_UNHANGUP:
		return mtp_r_hangup(q, mp);
	}
#if 0
	return mtp_r_other(q, mp);
#else
	freemsg(mp);
	return (0);
#endif
}
static inline fastcall int
mtp_r_msg(queue_t *q, mblk_t *mp)
{
	/* FIXME: write a fast path */
	return mtp_r_msg_slow(q, mp);
}

/*
 *  QUEUE PUT and SERVICE procedures
 *  =========================================================================
 */
static streamscall __hot_put int
sccp_wput(queue_t *q, mblk_t *mp)
{
	if ((pcmsg(DB_TYPE(mp)) && (q->q_first || (q->q_flag & QSVCBUSY))) || sccp_w_msg(q, mp))
		putq(q, mp);
	return (0);
}
static streamscall __hot_out int
sccp_wsrv(queue_t *q)
{
	mblk_t *mp;

	while ((mp = getq(q))) {
		if (sccp_w_msg(q, mp)) {
			putbq(q, mp);
			break;
		}
	}
	return (0);
}
static streamscall __hot_read int
sccp_rsrv(queue_t *q)
{
	/* FIXME: need to backenable across the mux */
	return (0);
}
static streamscall __unlikely int
sccp_rput(queue_t *q, mblk_t *mp)
{
	mi_strlog(q, 0, SL_ERROR, "invalid call to sccp_rput()");
	putnext(q, mp);
	return (0);
}
static streamscall __unlikely int
mtp_wput(queue_t *q, mblk_t *mp)
{
	mi_strlog(q, 0, SL_ERROR, "invalid call to mtp_wput()");
	putnext(q, mp);
	return (0);
}
static streamscall __hot_out int
mtp_wsrv(queue_t *q)
{
	/* FIXME: need to backenable across the mux */
	return (0);
}
static streamscall __hot_get int
mtp_rsrv(queue_t *q)
{
	mblk_t *mp;

	while ((mp = getq(q))) {
		if (mtp_r_msg(q, mp)) {
			putbq(q, mp);
			break;
		}
	}
	return (0);
}
static streamscall __hot_in int
mtp_rput(queue_t *q, mblk_t *mp)
{
	if ((pcmsg(DB_TYPE(mp)) && (q->q_first || (q->q_flag & QSVCBUSY))) || mtp_r_msg(q, mp))
		putq(q, mp);
	return (0);
}

/*
 *  =========================================================================
 *
 *  OPEN and CLOSE
 *
 *  =========================================================================
 *
 *  OPEN
 *  -------------------------------------------------------------------------
 */
static int sccp_majors[SCCP_CMAJORS] = {
	SCCP_CMAJOR_0,
};
static streamscall int
sccp_qopen(queue_t *q, dev_t *devp, int flag, int sflag, cred_t *crp)
{
	psw_t flags;
	int mindex = 0;
	major_t cmajor = getmajor(*devp);
	minor_t cminor = getminor(*devp);
	minor_t bminor = cminor;
	struct sc *sc, **scp = &master.sc.list;

	MOD_INC_USE_COUNT;	/* keep module from unloading */
	if (q->q_ptr != NULL) {
		MOD_DEC_USE_COUNT;
		return (0);	/* already open */
	}
	if (sflag == MODOPEN || WR(q)->q_next) {
		ptrace(("%s: ERROR: cannot push as module\n", DRV_NAME));
		MOD_DEC_USE_COUNT;
		return (EIO);
	}
	if (cmajor != SCCP_CMAJOR_0 || cminor >= SCCP_CMINOR_FREE) {
		MOD_DEC_USE_COUNT;
		return (ENXIO);
	}
	/* allocate a new device */
	cminor = SCCP_CMINOR_FREE;
	spin_lock_irqsave(&master.lock, flags);
	for (; *scp; scp = &(*scp)->next) {
		major_t dmajor = (*scp)->u.dev.cmajor;

		if (cmajor != dmajor)
			break;
		if (cmajor == dmajor) {
			minor_t dminor = (*scp)->u.dev.cminor;

			if (cminor < dminor)
				break;
			if (cminor > dminor)
				continue;
			if (cminor == dminor) {
				if (++cminor >= NMINORS) {
					if (++mindex >= SCCP_CMAJORS
					    || !(cmajor = sccp_majors[mindex]))
						break;
					cminor = 0;
				}
				continue;
			}
		}
	}
	if (mindex >= SCCP_CMAJORS || !cmajor) {
		ptrace(("%s: ERROR: no device numbers available\n", DRV_NAME));
		spin_unlock_irqrestore(&master.lock, flags);
		MOD_DEC_USE_COUNT;
		return (ENXIO);
	}
	printd(("%s: opened character device %d:%d\n", DRV_NAME, cmajor, cminor));
	*devp = makedevice(cmajor, cminor);
	if (!(sc = sccp_alloc_priv(q, scp, devp, crp, bminor))) {
		ptrace(("%s: ERROR: no memory\n", DRV_NAME));
		spin_unlock_irqrestore(&master.lock, flags);
		MOD_DEC_USE_COUNT;
		return (ENOMEM);
	}
	spin_unlock_irqrestore(&master.lock, flags);
	return (0);
}

/*
 *  CLOSE
 *  -------------------------------------------------------------------------
 */
static streamscall int
sccp_qclose(queue_t *q, int flag, cred_t *crp)
{
	struct sc *sc = SCCP_PRIV(q);
	psw_t flags;

	(void) flag;
	(void) crp;
	(void) sc;
	printd(("%s: closing character device %d:%d\n", DRV_NAME, sc->u.dev.cmajor,
		sc->u.dev.cminor));
	spin_lock_irqsave(&master.lock, flags);
	sccp_free_priv(sc);
	spin_unlock_irqrestore(&master.lock, flags);
	MOD_DEC_USE_COUNT;
	return (0);
}

/*
 *  =========================================================================
 *
 *  Private Structure allocation, deallocation and cache
 *
 *  =========================================================================
 */
static kmem_cachep_t sccp_sc_cachep = NULL;	/* SCCP User cache */
static kmem_cachep_t sccp_na_cachep = NULL;	/* Network Appearance cache */
static kmem_cachep_t sccp_cp_cachep = NULL;	/* Coupling */
static kmem_cachep_t sccp_sp_cachep = NULL;	/* Signalling Point cache */
static kmem_cachep_t sccp_sr_cachep = NULL;	/* Route Set cache */
static kmem_cachep_t sccp_ss_cachep = NULL;	/* Controlled Rerouting Buffer cache */
static kmem_cachep_t sccp_rs_cachep = NULL;	/* Route List cache */
static kmem_cachep_t sccp_mt_cachep = NULL;	/* Route cache */
static int
sccp_init_caches(void)
{
	if (!sccp_sc_cachep
	    && !(sccp_sc_cachep =
		 kmem_create_cache("sccp_sc_cachep", sizeof(struct sc), 0, SLAB_HWCACHE_ALIGN, NULL,
				   NULL))) {
		cmn_err(CE_PANIC, "%s: did not allocate sccp_sc_cachep", DRV_NAME);
		goto failed_mt;
	} else
		printd(("%s: initialized sc structure cache\n", DRV_NAME));
	if (!sccp_na_cachep
	    && !(sccp_na_cachep =
		 kmem_create_cache("sccp_na_cachep", sizeof(struct na), 0, SLAB_HWCACHE_ALIGN, NULL,
				   NULL))) {
		cmn_err(CE_PANIC, "%s: did not allocate sccp_na_cachep", DRV_NAME);
		goto failed_na;
	} else
		printd(("%s: initialized na structure cache\n", DRV_NAME));
	if (!sccp_cp_cachep
	    && !(sccp_cp_cachep =
		 kmem_create_cache("sccp_cp_cachep", sizeof(struct cp), 0, SLAB_HWCACHE_ALIGN, NULL,
				   NULL))) {
		cmn_err(CE_PANIC, "%s: did not allocate sccp_cp_cachep", DRV_NAME);
		goto failed_cp;
	} else
		printd(("%s: initialized cp structure cache\n", DRV_NAME));
	if (!sccp_sp_cachep
	    && !(sccp_sp_cachep =
		 kmem_create_cache("sccp_sp_cachep", sizeof(struct sp), 0, SLAB_HWCACHE_ALIGN, NULL,
				   NULL))) {
		cmn_err(CE_PANIC, "%s: did not allocate sccp_sp_cachep", DRV_NAME);
		goto failed_sp;
	} else
		printd(("%s: initialized sp structure cache\n", DRV_NAME));
	if (!sccp_sr_cachep
	    && !(sccp_sr_cachep =
		 kmem_create_cache("sccp_sr_cachep", sizeof(struct sr), 0, SLAB_HWCACHE_ALIGN, NULL,
				   NULL))) {
		cmn_err(CE_PANIC, "%s: did not allocate sccp_sr_cachep", DRV_NAME);
		goto failed_rs;
	} else
		printd(("%s: initialized rs structure cache\n", DRV_NAME));
	if (!sccp_ss_cachep
	    && !(sccp_ss_cachep =
		 kmem_create_cache("sccp_ss_cachep", sizeof(struct ss), 0, SLAB_HWCACHE_ALIGN, NULL,
				   NULL))) {
		cmn_err(CE_PANIC, "%s: did not allocate sccp_ss_cachep", DRV_NAME);
		goto failed_cr;
	} else
		printd(("%s: initialized cr structure cache\n", DRV_NAME));
	if (!sccp_rs_cachep
	    && !(sccp_rs_cachep =
		 kmem_create_cache("sccp_rs_cachep", sizeof(struct rs), 0, SLAB_HWCACHE_ALIGN, NULL,
				   NULL))) {
		cmn_err(CE_PANIC, "%s: did not allocate sccp_rs_cachep", DRV_NAME);
		goto failed_rl;
	} else
		printd(("%s: initialized rl structure cache\n", DRV_NAME));
	if (!sccp_mt_cachep
	    && !(sccp_mt_cachep =
		 kmem_create_cache("sccp_mt_cachep", sizeof(struct mt), 0, SLAB_HWCACHE_ALIGN, NULL,
				   NULL))) {
		cmn_err(CE_PANIC, "%s: did not allocate sccp_mt_cachep", DRV_NAME);
		goto failed_rt;
	} else
		printd(("%s: initialized rt structure cache\n", DRV_NAME));
	return (0);
	// failed_sl:
	// kmem_cache_destroy(sccp_mt_cachep);
      failed_rt:
	kmem_cache_destroy(sccp_rs_cachep);
      failed_rl:
	kmem_cache_destroy(sccp_ss_cachep);
      failed_cr:
	kmem_cache_destroy(sccp_sr_cachep);
      failed_rs:
	kmem_cache_destroy(sccp_sp_cachep);
      failed_sp:
	kmem_cache_destroy(sccp_cp_cachep);
      failed_cp:
	kmem_cache_destroy(sccp_na_cachep);
      failed_na:
	kmem_cache_destroy(sccp_sc_cachep);
      failed_mt:
	return (-ENOMEM);
}
static int
sccp_term_caches(void)
{
	int err = 0;

	if (sccp_sc_cachep) {
#ifdef HAVE_KTYPE_KMEM_CACHE_T_P
		if (kmem_cache_destroy(sccp_sc_cachep)) {
			cmn_err(CE_WARN, "%s: did not destroy sccp_sc_cachep", __FUNCTION__);
			err = -EBUSY;
		} else
			printd(("%s: destroyed sccp_sc_cachep\n", DRV_NAME));
#else
		kmem_cache_destroy(sccp_sc_cachep);
#endif
	}
	if (sccp_na_cachep) {
#ifdef HAVE_KTYPE_KMEM_CACHE_T_P
		if (kmem_cache_destroy(sccp_na_cachep)) {
			cmn_err(CE_WARN, "%s: did not destroy sccp_na_cachep", __FUNCTION__);
			err = -EBUSY;
		} else
			printd(("%s: destroyed sccp_na_cachep\n", DRV_NAME));
#else
		kmem_cache_destroy(sccp_na_cachep);
#endif
	}
	if (sccp_sp_cachep) {
#ifdef HAVE_KTYPE_KMEM_CACHE_T_P
		if (kmem_cache_destroy(sccp_sp_cachep)) {
			cmn_err(CE_WARN, "%s: did not destroy sccp_sp_cachep", __FUNCTION__);
			err = -EBUSY;
		} else
			printd(("%s: destroyed sccp_sp_cachep\n", DRV_NAME));
#else
		kmem_cache_destroy(sccp_sp_cachep);
#endif
	}
	if (sccp_cp_cachep) {
#ifdef HAVE_KTYPE_KMEM_CACHE_T_P
		if (kmem_cache_destroy(sccp_cp_cachep)) {
			cmn_err(CE_WARN, "%s: did not destroy sccp_cp_cachep", __FUNCTION__);
			err = -EBUSY;
		} else
			printd(("%s: destroyed sccp_cp_cachep\n", DRV_NAME));
#else
		kmem_cache_destroy(sccp_cp_cachep);
#endif
	}
	if (sccp_sr_cachep) {
#ifdef HAVE_KTYPE_KMEM_CACHE_T_P
		if (kmem_cache_destroy(sccp_sr_cachep)) {
			cmn_err(CE_WARN, "%s: did not destroy sccp_sr_cachep", __FUNCTION__);
			err = -EBUSY;
		} else
			printd(("%s: destroyed sccp_sr_cachep\n", DRV_NAME));
#else
		kmem_cache_destroy(sccp_sr_cachep);
#endif
	}
	if (sccp_ss_cachep) {
#ifdef HAVE_KTYPE_KMEM_CACHE_T_P
		if (kmem_cache_destroy(sccp_ss_cachep)) {
			cmn_err(CE_WARN, "%s: did not destroy sccp_ss_cachep", __FUNCTION__);
			err = -EBUSY;
		} else
			printd(("%s: destroyed sccp_ss_cachep\n", DRV_NAME));
#else
		kmem_cache_destroy(sccp_ss_cachep);
#endif
	}
	if (sccp_rs_cachep) {
#ifdef HAVE_KTYPE_KMEM_CACHE_T_P
		if (kmem_cache_destroy(sccp_rs_cachep)) {
			cmn_err(CE_WARN, "%s: did not destroy sccp_rs_cachep", __FUNCTION__);
			err = -EBUSY;
		} else
			printd(("%s: destroyed sccp_rs_cachep\n", DRV_NAME));
#else
		kmem_cache_destroy(sccp_rs_cachep);
#endif
	}
	if (sccp_mt_cachep) {
#ifdef HAVE_KTYPE_KMEM_CACHE_T_P
		if (kmem_cache_destroy(sccp_mt_cachep)) {
			cmn_err(CE_WARN, "%s: did not destroy sccp_mt_cachep", __FUNCTION__);
			err = -EBUSY;
		} else
			printd(("%s: destroyed sccp_mt_cachep\n", DRV_NAME));
#else
		kmem_cache_destroy(sccp_mt_cachep);
#endif
	}
	return (err);
}

/*
 *  SCCP allocation and deallocation
 *  -------------------------------------------------------------------------
 */
static struct sc *
sccp_alloc_priv(queue_t *q, struct sc **scp, dev_t *devp, cred_t *crp, minor_t bminor)
{
	struct sc *sc;

	if ((sc = kmem_cache_alloc(sccp_sc_cachep, GFP_ATOMIC))) {
		printd(("%s: %p: allocated sc private structure\n", DRV_NAME, sc));
		bzero(sc, sizeof(*sc));
		sc->priv_put = &sccp_put;
		sc->u.dev.cmajor = getmajor(*devp);
		sc->u.dev.cminor = getminor(*devp);
		sc->cred = *crp;
		(sc->oq = RD(q))->q_ptr = sccp_get(sc);
		(sc->iq = WR(q))->q_ptr = sccp_get(sc);
		spin_lock_init(&sc->qlock);	/* "sc-queue-lock" */
		// sc->o_prim = &sccp_r_prim;
		/* style of interface depends on bminor */
		switch (bminor) {
		case SCCP_STYLE_SCCPI:
			// sc->i_prim = &sccp_w_prim;
			sc->i_state = NS_UNBND;
			sc->i_style = bminor;
			sc->i_version = N_CURRENT_VERSION;
			break;
		case SCCP_STYLE_TPI:
			// sc->i_prim = &tpi_w_prim;
			sc->i_state = TS_UNBND;
			sc->i_style = bminor;
			sc->i_version = T_CURRENT_VERSION;
			break;
		case SCCP_STYLE_NPI:
			// sc->i_prim = &npi_w_prim;
			sc->i_state = NS_UNBND;
			sc->i_style = bminor;
			sc->i_version = N_CURRENT_VERSION;
			break;
		case SCCP_STYLE_GTT:
			// sc->i_prim = &gtt_w_prim;
			sc->i_state = NS_UNBND;
			sc->i_style = bminor;
			sc->i_version = N_CURRENT_VERSION;
			break;
		case SCCP_STYLE_MGMT:
			// sc->i_prim = &mgm_w_prim;
			sc->i_state = NS_UNBND;
			sc->i_style = bminor;
			sc->i_version = N_CURRENT_VERSION;
			break;
		}
		spin_lock_init(&sc->lock);	/* "sc-queue-lock" */
		if ((sc->next = *scp))
			sc->next->prev = &sc->next;
		sc->prev = scp;
		*scp = sccp_get(sc);
		/* set defaults */
		sc->info.PRIM_type = N_INFO_ACK;
		sc->info.NSDU_size = T_INFINITE;	/* unlimited */
		sc->info.ENSDU_size = T_INFINITE;	/* unlimited */
		sc->info.CDATA_size = T_INFINITE;	/* unlimited XXX */
		sc->info.DDATA_size = T_INFINITE;	/* unlimited XXX */
		sc->info.ADDR_size = sizeof(sc->src) + sizeof(sc->saddr);
		sc->info.ADDR_length = sizeof(sc->src) + sizeof(sc->saddr);
		sc->info.QOS_length = sizeof(sc->iqos);
		sc->info.QOS_offset = sc->info.ADDR_offset + sc->info.ADDR_length;
		sc->info.QOS_range_length = sizeof(sc->iqor);
		sc->info.QOS_range_offset = sc->info.QOS_offset + sc->info.QOS_length;
		sc->info.OPTIONS_flags =
		    (REC_CONF_OPT | EX_DATA_OPT | DEFAULT_RC_SEL) & ~DEFAULT_RC_SEL;
		sc->info.NIDU_size = T_INFINITE;	/* unlimited */
		sc->info.SERV_type = N_CONS | N_CLNS;
		sc->info.PROVIDER_type = N_SUBNET;
		sc->info.NODU_size = 254;
		sc->info.CURRENT_state = NS_NOSTATES;
		sc->info.PROTOID_length = sizeof(sc->pro);
		sc->info.PROTOID_offset = sc->info.QOS_range_offset + sc->info.QOS_range_length;
		sc->info.NPI_version = N_CURRENT_VERSION;

		sc->iqos.n_qos_type = N_QOS_SEL_INFO_SCCP;
		sc->iqos.protocol_class = 0;
		sc->iqos.option_flags = 0;
		sc->iqos.sequence_selection = 0;
		sc->iqos.message_priority = 0;
		sc->iqos.importance = 0;

		sc->iqor.n_qos_type = N_QOS_RANGE_INFO_SCCP;
		sc->iqor.protocol_classes = 0x0f;
		sc->iqor.sequence_selection = 0x1f;

		printd(("%s: %p: linked sc private structure\n", DRV_NAME, sc));
	} else
		ptrace(("%s: ERROR: Could not allocate sc private structure\n", DRV_NAME));
	return (sc);
}
static void
sccp_free_priv(struct sc *sc)
{
	psw_t flags;

	ensure(sc, return);
	spin_lock_irqsave(&sc->lock, flags);
	{
		ss7_unbufcall((str_t *) sc);
		if (sc->ss.next || sc->ss.prev != &sc->ss.next) {
			/* disconnect from local subsystem */
			if ((*sc->ss.prev = sc->ss.next))
				sc->ss.next->ss.prev = sc->ss.prev;
			sc->ss.next = NULL;
			sc->ss.prev = &sc->ss.next;
			ss_put(xchg(&sc->ss.ss, NULL));
			sccp_put(sc);
		}
		if (sc->sr.next || sc->sr.prev != &sc->sr.next) {
			/* disconnect from remote signalling point */
			if ((*sc->sr.prev = sc->sr.next))
				sc->sr.next->sr.prev = sc->sr.prev;
			sc->sr.next = NULL;
			sc->sr.prev = &sc->sr.next;
			sr_put(xchg(&sc->sr.sr, NULL));
			sccp_put(sc);
		}
		if (sc->next || sc->prev != &sc->next) {
			/* remove from master list */
			if ((*sc->prev = sc->next))
				sc->next->prev = sc->prev;
			sc->next = NULL;
			sc->prev = &sc->next;
			sccp_put(sc);
		}
		sc->oq->q_ptr = NULL;
		flushq(sc->oq, FLUSHDATA);
		sc->oq = NULL;
		sccp_put(sc);
		sc->iq->q_ptr = NULL;
		flushq(sc->iq, FLUSHDATA);
		sc->iq = NULL;
	}
	spin_unlock_irqrestore(&sc->lock, flags);
	sccp_put(sc);		/* final put */
}
static struct sc *
sccp_lookup(uint id)
{
	struct sc *sc = NULL;

	if (id)
		for (sc = master.sc.list; sc && sc->id != id; sc = sc->next) ;
	return (sc);
}
static inline uint
sccp_get_id(uint id)
{
	static uint sequence = 0;

	if (!id)
		for (id = ++sequence; sccp_lookup(id); id = ++sequence) ;
	return (id);
}
static struct sc *
sccp_get(struct sc *sc)
{
	atomic_inc(&sc->refcnt);
	return (sc);
}
static void
sccp_put(struct sc *sc)
{
	if (atomic_dec_and_test(&sc->refcnt)) {
		kmem_cache_free(sccp_sc_cachep, sc);
		printd(("%s: %s: %p: deallocated sc private structure", DRV_NAME, __FUNCTION__,
			sc));
	}
}

/*
 *  NA allocation and deallocation
 *  -------------------------------------------------------------------------
 */
static struct na *
sccp_alloc_na(uint id, struct lmi_option *proto)
{
	struct na *na;

	if ((na = kmem_cache_alloc(sccp_na_cachep, GFP_ATOMIC))) {
		bzero(na, sizeof(*na));
		na->priv_put = &na_put;
		spin_lock_init(&na->lock);	/* "na-lock" */
		na_get(na);	/* first get */
		/* add to master list */
		if ((na->next = master.na.list))
			na->next->prev = &na->next;
		na->prev = &master.na.list;
		master.na.list = na_get(na);
		master.na.numb++;
		/* fill out structure */
		na->id = na_get_id(id);
		na->type = MTP_OBJ_TYPE_NA;
		na->proto = *proto;
		/* populate defaults based on protoocl variant */
		switch ((na->proto.pvar & SS7_PVAR_MASK)) {
		default:
		case SS7_PVAR_ITUT:
			na->config = itut_na_config_default;
			break;
		case SS7_PVAR_ETSI:
			na->config = etsi_na_config_default;
			break;
		case SS7_PVAR_ANSI:
			na->config = ansi_na_config_default;
			break;
		case SS7_PVAR_JTTC:
			na->config = jttc_na_config_default;
			break;
		}
	}
	return (na);
}
static void
sccp_free_na(struct na *na)
{
	psw_t flags;

	ensure(na, return);
	spin_lock_irqsave(&na->lock, flags);
	{
		struct sp *sp;

		/* remove all attached signalling points */
		while ((sp = na->sp.list))
			sccp_free_sp(sp);
		if (na->next || na->prev != &na->next) {
			/* remove from master list */
			if ((*na->prev = na->next))
				na->next->prev = na->prev;
			na->next = NULL;
			na->prev = &na->next;
			master.na.numb--;
			na_put(na);
		}
	}
	spin_unlock_irqrestore(&na->lock, flags);
	na_put(na);		/* final put */
	return;
}
static struct na *
na_lookup(uint id)
{
	struct na *na = NULL;

	if (id)
		for (na = master.na.list; na && na->id != id; na = na->next) ;
	return (na);
}

static uint
na_get_id(uint id)
{
	static uint sequence = 0;

	if (!id)
		id = ++sequence;
	return (id);
}
static struct na *
na_get(struct na *na)
{
	atomic_inc(&na->refcnt);
	return (na);
}
static void
na_put(struct na *na)
{
	if (atomic_dec_and_test(&na->refcnt)) {
		kmem_cache_free(sccp_na_cachep, na);
		printd(("%s: %p: freed na structure\n", DRV_NAME, na));
	}
}

/*
 *  CP allocation and deallocation
 *  -------------------------------------------------------------------------
 */
#if 0
static struct cp *
sccp_alloc_cp(uint id, struct sp *sp, np_ulong slr0, np_ulong slr1)
{
	struct cp *cp;

	if ((cp = kmem_cache_alloc(sccp_cp_cachep, GFP_ATOMIC))) {
		bzero(cp, sizeof(*cp));
		cp->priv_put = &cp_put;
		spin_lock_init(&cp->lock);	/* "cp-lock" */
		cp_get(cp);	/* first get */
		/* add to master list */
		if ((cp->next = master.cp.list))
			cp->next->prev = &cp->next;
		cp->prev = &master.cp.list;
		master.cp.list = cp_get(cp);
		master.cp.numb++;
		/* link to signalling point */
		if ((cp->sp.next = sp->cp.list))
			cp->sp.next->sp.prev = &cp->sp.next;
		cp->sp.prev = &sp->cp.list;
		cp->sp.sp = sp_get(sp);
		sp->cp.list = cp_get(cp);
		sp->cp.numb++;
		/* fill out structure */
		cp->csec[0].slr = slr0;
		cp->csec[1].slr = slr1;
		fixme(("fill out structure\n"));
		/* populate defaults */
		fixme(("populate defaults\n"));
	}
	return (cp);
}
#endif
static void
sccp_free_cp(struct cp *cp)
{
	psw_t flags;

	ensure(cp, return);
	spin_lock_irqsave(&cp->lock, flags);
	{
		if (cp->sp.next || cp->sp.prev != &cp->sp.next) {
			/* remove from sp list */
			if ((*cp->sp.prev = cp->sp.next))
				cp->sp.next->sp.prev = cp->sp.prev;
			cp->sp.next = NULL;
			cp->sp.prev = &cp->sp.next;
			cp->sp.sp->cp.numb--;
			sp_put(xchg(&cp->sp.sp, NULL));
			cp_put(cp);
		}
		if (cp->next || cp->prev != &cp->next) {
			/* remove from master list */
			if ((*cp->prev = cp->next))
				cp->next->prev = cp->prev;
			cp->next = NULL;
			cp->prev = &cp->next;
			master.cp.numb--;
			cp_put(cp);
		}
	}
	spin_unlock_irqrestore(&cp->lock, flags);
	cp_put(cp);		/* final put */
	return;
}

#if 0
static struct cp *
cp_get(struct cp *cp)
{
	atomic_inc(&cp->refcnt);
	return (cp);
}
#endif
static void
cp_put(struct cp *cp)
{
	if (atomic_dec_and_test(&cp->refcnt)) {
		kmem_cache_free(sccp_cp_cachep, cp);
		printd(("%s: %p: freed cp structure\n", DRV_NAME, cp));
	}
}
static struct cp *
cp_lookup(uint id)
{
	struct cp *cp = NULL;

	if (id)
		for (cp = master.cp.list; cp && cp->id != id; cp = cp->next) ;
	return (cp);
}

#if 0
static uint
cp_get_id(uint id)
{
	static uint sequence = 0;

	if (!id)
		id = ++sequence;
	return (id);
}
static struct cp *
sccp_lookup_cp(struct sp *sp, uint slr, const int create)
{
	struct cp *cp;

	(void) create;
	for (cp = sp->cp.list; cp && cp->csec[0].slr != slr && cp->csec[1].slr != slr;
	     cp = cp->sp.next) ;
	return (cp);
}
#endif

/*
 *  SS allocation and deallocation
 *  -------------------------------------------------------------------------
 */
static struct ss *
sccp_alloc_ss(uint id, struct sp *sp, uint ssn)
{
	struct ss *ss;

	if ((ss = kmem_cache_alloc(sccp_ss_cachep, GFP_ATOMIC))) {
		bzero(ss, sizeof(*ss));
		ss->priv_put = &ss_put;
		spin_lock_init(&ss->lock);	/* "ss-lock" */
		ss_get(ss);	/* first get */
		/* add to master list */
		if ((ss->next = master.ss.list))
			ss->next->prev = &ss->next;
		ss->prev = &master.ss.list;
		master.ss.list = ss_get(ss);
		master.ss.numb++;
		/* link to signalling point */
		if ((ss->sp.next = sp->ss.list))
			ss->sp.next->sp.prev = &ss->sp.next;
		ss->sp.prev = &sp->ss.list;
		ss->sp.sp = sp_get(sp);
		sp->ss.list = ss_get(ss);
		sp->ss.numb++;
		/* fill out structure */
		ss->ssn = ssn;
		fixme(("fill out structure\n"));
		/* populate defaults */
		fixme(("populate defaults\n"));
	}
	return (ss);
}
static void
sccp_free_ss(struct ss *ss)
{
	psw_t flags;

	ensure(ss, return);
	spin_lock_irqsave(&ss->lock, flags);
	{
		struct rs *rs;

		while ((rs = ss->rs.list))
			sccp_free_rs(rs);
		if (ss->sp.next || ss->sp.prev != &ss->sp.next) {
			/* remove from sp list */
			if ((*ss->sp.prev = ss->sp.next))
				ss->sp.next->sp.prev = ss->sp.prev;
			ss->sp.next = NULL;
			ss->sp.prev = &ss->sp.next;
			ss->sp.sp->ss.numb--;
			sp_put(xchg(&ss->sp.sp, NULL));
			ss_put(ss);
		}
		if (ss->next || ss->prev != &ss->next) {
			/* remove from master list */
			if ((*ss->prev = ss->next))
				ss->next->prev = ss->prev;
			ss->next = NULL;
			ss->prev = &ss->next;
			master.ss.numb--;
			ss_put(ss);
		}
	}
	spin_unlock_irqrestore(&ss->lock, flags);
	ss_put(ss);		/* final put */
	return;
}
static struct ss *
ss_get(struct ss *ss)
{
	atomic_inc(&ss->refcnt);
	return (ss);
}
static void
ss_put(struct ss *ss)
{
	if (atomic_dec_and_test(&ss->refcnt)) {
		kmem_cache_free(sccp_ss_cachep, ss);
		printd(("%s: %p: freed ss structure\n", DRV_NAME, ss));
	}
}
static struct ss *
ss_lookup(uint id)
{
	struct ss *ss = NULL;

	if (id)
		for (ss = master.ss.list; ss && ss->id != id; ss = ss->next) ;
	return (ss);
}

static uint
ss_get_id(uint id)
{
	static uint sequence = 0;

	if (!id)
		id = ++sequence;
	return (id);
}
static struct ss *
sccp_lookup_ss(struct sp *sp, uint ssn, const int create)
{
	struct ss *ss;

	for (ss = sp->ss.list; ss && ss->ssn != ssn; ss = ss->sp.next) ;
	if (!ss && create && !(sp->flags & SCCPF_STRICT_CONFIG))
		ss = sccp_alloc_ss(ss_get_id(0), sp, ssn);
	return (ss);
}

/*
 *  RS allocation and deallocation
 *  -------------------------------------------------------------------------
 */
static struct rs *
sccp_alloc_rs(uint id, struct sr *sr, uint ssn)
{
	struct rs *rs;

	if ((rs = kmem_cache_alloc(sccp_rs_cachep, GFP_ATOMIC))) {
		bzero(rs, sizeof(*rs));
		rs->priv_put = &rs_put;
		spin_lock_init(&rs->lock);	/* "rs-lock" */
		rs_get(rs);	/* first get */
		/* add to master list */
		if ((rs->next = master.rs.list))
			rs->next->prev = &rs->next;
		rs->prev = &master.rs.list;
		master.rs.list = rs_get(rs);
		master.rs.numb++;
		/* link to sr */
		if ((rs->sr.next = sr->rs.list))
			rs->sr.next->sr.prev = &rs->sr.next;
		rs->sr.prev = &sr->rs.list;
		rs->sr.sr = sr_get(sr);
		sr->rs.list = rs_get(rs);
		sr->rs.numb++;
#if 0
		/* link to ss */
		if ((rs->ss.next = ss->rs.list))
			rs->ss.next->ss.prev = &rs->ss.next;
		rs->ss.prev = &ss->rs.list;
		rs->ss.ss = ss_get(ss);
		ss->rs.list = rs_get(rs);
		ss->rs.numb++;
#endif
		/* fill out structure */
		fixme(("fill out structure\n"));
		rs->ssn = ssn;
		/* populate defaults */
		fixme(("populate defaults\n"));
	}
	return (rs);
}
static void
sccp_free_rs(struct rs *rs)
{
	psw_t flags;

	ensure(rs, return);
	spin_lock_irqsave(&rs->lock, flags);
	{
		if (rs->sr.next || rs->sr.prev != &rs->sr.next) {
			/* remove from sr list */
			if ((*rs->sr.prev = rs->sr.next))
				rs->sr.next->sr.prev = rs->sr.prev;
			rs->sr.next = NULL;
			rs->sr.prev = &rs->sr.next;
			rs->sr.sr->rs.numb--;
			sr_put(xchg(&rs->sr.sr, NULL));
			rs_put(rs);
		}
		if (rs->ss.next || rs->ss.prev != &rs->ss.next) {
			/* remove from ss list */
			if ((*rs->ss.prev = rs->ss.next))
				rs->ss.next->ss.prev = rs->ss.prev;
			rs->ss.next = NULL;
			rs->ss.prev = &rs->ss.next;
			rs->ss.ss->rs.numb--;
			ss_put(xchg(&rs->ss.ss, NULL));
			rs_put(rs);
		}
		if (rs->next || rs->prev != &rs->next) {
			/* remove from master list */
			if ((*rs->prev = rs->next))
				rs->next->prev = rs->prev;
			rs->next = NULL;
			rs->prev = &rs->next;
			master.rs.numb--;
			rs_put(rs);
		}
	}
	spin_unlock_irqrestore(&rs->lock, flags);
	rs_put(rs);		/* final put */
	return;
}
static struct rs *
rs_get(struct rs *rs)
{
	atomic_inc(&rs->refcnt);
	return (rs);
}
static void
rs_put(struct rs *rs)
{
	if (atomic_dec_and_test(&rs->refcnt)) {
		kmem_cache_free(sccp_rs_cachep, rs);
		printd(("%s: %p: freed rs structure\n", DRV_NAME, rs));
	}
}
static struct rs *
rs_lookup(uint id)
{
	struct rs *rs = NULL;

	if (id)
		for (rs = master.rs.list; rs && rs->id != id; rs = rs->next) ;
	return (rs);
}

static uint
rs_get_id(uint id)
{
	static uint sequence = 0;

	if (!id)
		id = ++sequence;
	return (id);
}
static struct rs *
sccp_lookup_rs(struct sr *sr, uint ssn, const int create)
{
	struct rs *rs;

	for (rs = sr->rs.list; rs && rs->ssn != ssn; rs = rs->sr.next) ;
	if (!rs && create && !(sr->flags & SCCPF_STRICT_ROUTING))
		rs = sccp_alloc_rs(rs_get_id(0), sr, ssn);
	return (rs);
}

/*
 *  SR allocation and deallocation
 *  -------------------------------------------------------------------------
 */
static struct sr *
sccp_alloc_sr(uint id, struct sp *sp, uint pc)
{
	struct sr *sr, **srp;

	printd(("%s: %s: create sr->id = %ld\n", DRV_NAME, __FUNCTION__, id));
	if ((sr = kmem_cache_alloc(sccp_sr_cachep, GFP_ATOMIC))) {
		bzero(sr, sizeof(*sr));
		sr_get(sr);	/* first get */
		spin_lock_init(&sr->lock);	/* "sr-lock" */
		sr->id = id;
		/* add to master list */
		for (srp = &master.sr.list; *srp && (*srp)->id < id; srp = &(*srp)->next) ;
		if ((sr->next = *srp))
			sr->next->prev = &sr->next;
		sr->prev = srp;
		*srp = sr_get(sr);
		master.sr.numb++;
		/* add to signalling point list of signalling relations */
		sr->sp.sp = sp_get(sp);
		if ((sr->sp.next = sp->sr.list))
			sr->sp.next->sp.prev = &sr->sp.next;
		sr->sp.prev = &sp->sr.list;
		sp->sr.list = sr_get(sr);
		sp->sr.numb++;
		sr->add = sp->add;
		sr->add.pc = pc;
	} else
		printd(("%s: %s: ERROR: failed to allocate sr structure %lu\n", DRV_NAME,
			__FUNCTION__, id));
	return (sr);
}
static void
sccp_free_sr(struct sr *sr)
{
	psw_t flags;

	ensure(sr, return);
	printd(("%s: %s: %p free sr->id = %ld\n", DRV_NAME, __FUNCTION__, sr, sr->id));
	spin_lock_irqsave(&sr->lock, flags);
	{
		struct rs *rs;
		struct sc *sc;

		/* stop all timers */
		sr_timer_stop(sr, tall);
		/* remove all remote subsystems */
		while ((rs = sr->rs.list))
			sccp_free_rs(rs);
		/* release all connected SCCP users */
		while ((sc = sr->sc.list))
			sccp_release(sc);
		/* unlink from message transfer part */
		if (sr->mt) {
			sr_put(xchg(&sr->mt->sr, NULL));
			mtp_put(xchg(&sr->mt, NULL));
		}
		/* remove from signalling point list */
		if (sr->sp.sp) {
			if ((*sr->sp.prev = sr->sp.next))
				sr->sp.next->sp.prev = sr->sp.prev;
			sr->sp.next = NULL;
			sr->sp.prev = &sr->sp.next;
			sp_put(xchg(&sr->sp.sp, NULL));
			ensure(atomic_read(&sr->refcnt) > 1, sr_get(sr));
			sr_put(sr);
		}
		/* remove from master list */
		if ((*sr->prev = sr->next))
			sr->next->prev = sr->prev;
		sr->next = NULL;
		sr->prev = &sr->next;
		ensure(atomic_read(&sr->refcnt) > 1, sr_get(sr));
		sr_put(sr);
		assure(master.sr.numb > 0);
		master.sr.numb--;
		/* done, check final count */
		if (atomic_read(&sr->refcnt) != 1) {
			__printd(("%s: %s: %p: ERROR: sr lingering reference count = %d\n",
				  DRV_NAME, __FUNCTION__, sr, atomic_read(&sr->refcnt)));
			atomic_set(&sr->refcnt, 1);
		}
	}
	spin_unlock_irqrestore(&sr->lock, flags);
	sr_put(sr);		/* final put */
}
static struct sr *
sr_get(struct sr *sr)
{
	assure(sr);
	if (sr)
		atomic_inc(&sr->refcnt);
	return (sr);
}
static void
sr_put(struct sr *sr)
{
	assure(sr);
	if (sr) {
		assure(atomic_read(&sr->refcnt) > 1);
		if (atomic_dec_and_test(&sr->refcnt)) {
			kmem_cache_free(sccp_sr_cachep, sr);
			printd(("%s: %s: %p: deallocated sr structure\n", DRV_NAME, __FUNCTION__,
				sr));
		}
	}
}
static struct sr *
sr_lookup(uint id)
{
	struct sr *sr;

	for (sr = master.sr.list; sr && sr->id != id; sr = sr->next) ;
	return (sr);
}

static uint
sr_get_id(uint id)
{
	struct sr *sr;

	if (!id) {
		id = 1;
		for (sr = master.sr.list; sr; sr = sr->next)
			if (sr->id == id)
				id++;
			else if (sr->id > id)
				break;
	}
	return (id);
}
static struct sr *
sccp_lookup_sr(struct sp *sp, uint pc, const int create)
{
	struct sr *sr;

	for (sr = sp->sr.list; sr && sr->add.pc != pc; sr = sr->sp.next) ;
	if (!sr && create && !(sp->flags & SCCPF_STRICT_ROUTING))
		sr = sccp_alloc_sr(sr_get_id(0), sp, pc);
	return (sr);
}

/*
 *  SP allocation and deallocation
 *  -------------------------------------------------------------------------
 */
static struct sp *
sccp_alloc_sp(uint id, struct na *na, mtp_addr_t * add)
{
	struct sp *sp, **spp;

	printd(("%s: %s: create sp->id = %ld\n", DRV_NAME, __FUNCTION__, id));
	if ((sp = kmem_cache_alloc(sccp_sp_cachep, GFP_ATOMIC))) {
		bzero(sp, sizeof(*sp));
		sp_get(sp);	/* first get */
		spin_lock_init(&sp->lock);	/* "sp-lock" */
		sp->id = id;
		for (spp = &master.sp.list; *spp && (*spp)->id < id; spp = &(*spp)->next) ;
		if ((sp->next = *spp))
			sp->next->prev = &sp->next;
		sp->prev = spp;
		*spp = sp_get(sp);
		master.sp.numb++;
		/* add to network appearance list of signalling points */
		sp->na.na = na_get(na);
		if ((sp->na.next = na->sp.list))
			sp->na.next->na.prev = &sp->na.next;
		sp->na.prev = &na->sp.list;
		na->sp.list = sp_get(sp);
		na->sp.numb++;
		sp->add = *add;
	} else
		printd(("%s: %s: ERROR: failed to allocate sp structure %lu\n", DRV_NAME,
			__FUNCTION__, id));
	return (sp);
}
static void
sccp_free_sp(struct sp *sp)
{
	psw_t flags;

	ensure(sp, return);
	printd(("%s: %s: %p free sp->id = %ld\n", DRV_NAME, __FUNCTION__, sp, sp->id));
	spin_lock_irqsave(&sp->lock, flags);
	{
		sp_timer_stop(sp, tall);
		/* freeing all signalling relations */
		while (sp->sr.list)
			sccp_free_sr(sp->sr.list);
		/* unlink from message transfer part */
		if (sp->mt) {
			sp_put(xchg(&sp->mt->sp, NULL));
			mtp_put(xchg(&sp->mt, NULL));
		}
		/* remote from network appearance list */
		if (sp->na.na) {
			if ((*sp->na.prev = sp->na.next))
				sp->na.next->na.prev = sp->na.prev;
			sp->na.next = NULL;
			sp->na.prev = &sp->na.next;
			na_put(xchg(&sp->na.na, NULL));
			ensure(atomic_read(&sp->refcnt) < 1, sp_get(sp));
			sp_put(sp);
		}
		/* remove from master list */
		if ((*sp->prev = sp->next))
			sp->next->prev = sp->prev;
		sp->next = NULL;
		sp->prev = &sp->next;
		ensure(atomic_read(&sp->refcnt) > 1, sp_get(sp));
		sp_put(sp);
		assure(master.sp.numb > 0);
		master.sp.numb--;
		/* done, check final count */
		if (atomic_read(&sp->refcnt) != 1) {
			__printd(("%s: %s: %p: ERROR: sp lingering reference count = %d\n",
				  DRV_NAME, __FUNCTION__, sp, atomic_read(&sp->refcnt)));
			atomic_set(&sp->refcnt, 1);
		}
	}
	spin_unlock_irqrestore(&sp->lock, flags);
	sp_put(sp);		/* final put */
}
static struct sp *
sp_get(struct sp *sp)
{
	assure(sp);
	if (sp)
		atomic_inc(&sp->refcnt);
	return (sp);
}
static void
sp_put(struct sp *sp)
{
	assure(sp);
	if (sp) {
		assure(atomic_read(&sp->refcnt) > 1);
		if (atomic_dec_and_test(&sp->refcnt)) {
			kmem_cache_free(sccp_sp_cachep, sp);
			printd(("%s: %s: %p: deallocated sp structure\n", DRV_NAME, __FUNCTION__,
				sp));
		}
	}
}
static struct sp *
sp_lookup(uint id)
{
	struct sp *sp;

	for (sp = master.sp.list; sp && sp->id != id; sp = sp->next) ;
	return (sp);
}

static uint
sp_get_id(uint id)
{
	struct sp *sp;

	if (!id) {
		id = 1;
		for (sp = master.sp.list; sp; sp = sp->next)
			if (sp->id == id)
				id++;
			else if (sp->id > id)
				break;
	}
	return (id);
}
static struct sp *
sccp_lookup_sp(uint ni, uint pc, const int create)
{
	struct na *na;
	struct sp *sp = NULL;

	for (na = master.na.list; na && na->id != ni; na = na->next) ;
	if (na) {
		for (sp = na->sp.list; sp && sp->add.pc != pc; sp = sp->next) ;
		if (!sp && create && !(na->flags & SCCPF_STRICT_CONFIG)) {
			struct mtp_addr add;

			add.family = AF_MTP;
			add.ni = ni;
			add.si = 3;
			add.pc = pc;
			sp = sccp_alloc_sp(sp_get_id(0), na, &add);
		}
	}
	return (sp);
}

/*
 *  MT allocation and deallocation
 *  -------------------------------------------------------------------------
 */
static struct mt *
sccp_alloc_link(queue_t *q, struct mt **mpp, uint index, cred_t *crp)
{
	struct mt *mt;

	printd(("%s: %s: create mt index = %lu\n", DRV_NAME, __FUNCTION__, index));
	if ((mt = kmem_cache_alloc(sccp_mt_cachep, GFP_ATOMIC))) {
		bzero(mt, sizeof(*mt));
		mtp_get(mt);	/* first get */
		mt->u.mux.index = index;
		mt->cred = *crp;
		spin_lock_init(&mt->qlock);	/* "mt-queue-lock" */
		(mt->iq = RD(q))->q_ptr = mtp_get(mt);
		(mt->oq = WR(q))->q_ptr = mtp_get(mt);
		// mt->o_prim = mtp_w_prim;
		// mt->i_prim = mtp_r_prim;
		mt->o_wakeup = NULL;
		mt->i_wakeup = NULL;
		mt->i_state = LMI_UNUSABLE;
		mt->i_style = LMI_STYLE1;
		mt->i_version = 1;
		spin_lock_init(&mt->lock);	/* "mt-lock" */
		/* place in master list */
		if ((mt->next = *mpp))
			mt->next->prev = &mt->next;
		mt->prev = mpp;
		*mpp = mtp_get(mt);
		master.mt.numb++;
	} else
		printd(("%s: %s: ERROR: failed to allocate mt structure %lu\n", DRV_NAME,
			__FUNCTION__, index));
	return (mt);
}
static void
sccp_free_link(struct mt *mt)
{
	psw_t flags;

	ensure(mt, return);
	printd(("%s: %s: %p free mt index = %lu\n", DRV_NAME, __FUNCTION__, mt, mt->u.mux.index));
	spin_lock_irqsave(&mt->lock, flags);
	{
		/* flushing buffers */
		ss7_unbufcall((str_t *) mt);
		flushq(mt->iq, FLUSHDATA);
		flushq(mt->oq, FLUSHDATA);
		/* unlink from signalling relation */
		if (mt->sr) {
			struct sr *sr = mt->sr;

			(void) sr;
#if 0
			if (!(sr->flags & CCTF_LOC_S_BLOCKED)) {
				/* software block all circuits */
				struct ct *ct;

				for (ct = sr->ct.list; ct; ct = ct->sr.next) {
					ct_set(ct, CCTF_LOC_S_BLOCKED);
				}
				sr->flags |= CCTF_LOC_S_BLOCKED;
			}
#endif
			mtp_put(xchg(&mt->sr->mt, NULL));
			sr_put(xchg(&mt->sr, NULL));
		}
		/* unlink from signalling point */
		if (mt->sp) {
			struct sr *sr;

			for (sr = mt->sp->sr.list; sr; sr = sr->sp.next) {
#if 0
				if (!(sr->flags & CCTF_LOC_S_BLOCKED)) {
					/* software block all circuits */
					struct ct *ct;

					for (ct = sr->ct.list; ct; ct = ct->sr.next) {
						ct_set(ct, CCTF_LOC_S_BLOCKED);
					}
					sr->flags |= CCTF_LOC_S_BLOCKED;
				}
#endif
			}
			mtp_put(xchg(&mt->sp->mt, NULL));
			sp_put(xchg(&mt->sp, NULL));
		}
		/* remote from master list */
		if ((*mt->prev = mt->next))
			mt->next->prev = mt->prev;
		mt->next = NULL;
		mt->prev = &mt->next;
		ensure(atomic_read(&mt->refcnt) > 1, mtp_get(mt));
		mtp_put(mt);
		assure(master.mt.numb > 0);
		master.mt.numb--;
		/* remove from queues */
		ensure(atomic_read(&mt->refcnt) > 1, mtp_get(mt));
		mtp_put(xchg(&mt->iq->q_ptr, NULL));
		ensure(atomic_read(&mt->refcnt) > 1, mtp_get(mt));
		mtp_put(xchg(&mt->oq->q_ptr, NULL));
		/* done, check final count */
		if (atomic_read(&mt->refcnt) != 1) {
			__printd(("%s: %s: %p: ERROR: mt lingering reference count = %d\n",
				  DRV_NAME, __FUNCTION__, mt, atomic_read(&mt->refcnt)));
			atomic_set(&mt->refcnt, 1);
		}
	}
	spin_unlock_irqrestore(&mt->lock, flags);
	mtp_put(mt);		/* final put */
}
static struct mt *
mtp_get(struct mt *mt)
{
	assure(mt);
	if (mt)
		atomic_inc(&mt->refcnt);
	return (mt);
}
static void
mtp_put(struct mt *mt)
{
	assure(mt);
	if (mt) {
		assure(atomic_read(&mt->refcnt) > 1);
		if (atomic_dec_and_test(&mt->refcnt)) {
			kmem_cache_free(sccp_mt_cachep, mt);
			printd(("%s: %s: %p: deallocated mt structure\n", DRV_NAME, __FUNCTION__,
				mt));
		}
	}
}
static struct mt *
mtp_lookup(uint id)
{
	struct mt *mt;

	for (mt = master.mt.list; mt && mt->id != id; mt = mt->next) ;
	return (mt);
}

static uint
mtp_get_id(uint id)
{
	static uint identifier = 0;

	if (!id) {
		id = ++identifier;
	}
	return (id);
}

/*
 *  =========================================================================
 *
 *  STREAMS Definitions
 *
 *  =========================================================================
 */

static struct module_stat sccp_wstat __attribute__ ((__aligned__(SMP_CACHE_BYTES)));
static struct module_stat sccp_rstat __attribute__ ((__aligned__(SMP_CACHE_BYTES)));
static struct module_stat mtp_wstat __attribute__ ((__aligned__(SMP_CACHE_BYTES)));
static struct module_stat mtp_rstat __attribute__ ((__aligned__(SMP_CACHE_BYTES)));

static struct module_info sccp_winfo = {
	.mi_idnum = DRV_ID,		/* Module ID number */
	.mi_idname = DRV_NAME,		/* Module ID name */
	.mi_minpsz = 1,			/* Min packet size accepted */
	.mi_maxpsz = 272 + 1,		/* Max packet size accepted */
	.mi_hiwat = STRHIGH,		/* Hi water mark */
	.mi_lowat = STRLOW,		/* Lo water mark */
};
static struct module_info sccp_rinfo = {
	.mi_idnum = DRV_ID,		/* Module ID number */
	.mi_idname = DRV_NAME,		/* Module ID name */
	.mi_minpsz = 1,			/* Min packet size accepted */
	.mi_maxpsz = 272 + 1,		/* Max packet size accepted */
	.mi_hiwat = STRHIGH,		/* Hi water mark */
	.mi_lowat = STRLOW,		/* Lo water mark */
};
static struct module_info mtp_winfo = {
	.mi_idnum = DRV_ID,		/* Module ID number */
	.mi_idname = DRV_NAME,		/* Module ID name */
	.mi_minpsz = 1,			/* Min packet size accepted */
	.mi_maxpsz = 272 + 1,		/* Max packet size accepted */
	.mi_hiwat = STRHIGH,		/* Hi water mark */
	.mi_lowat = STRLOW,		/* Lo water mark */
};
static struct module_info mtp_rinfo = {
	.mi_idnum = DRV_ID,		/* Module ID number */
	.mi_idname = DRV_NAME,		/* Module ID name */
	.mi_minpsz = 1,			/* Min packet size accepted */
	.mi_maxpsz = 272 + 1,		/* Max packet size accepted */
	.mi_hiwat = STRHIGH,		/* Hi water mark */
	.mi_lowat = STRLOW,		/* Lo water mark */
};

static struct qinit sccp_rinit = {
	.qi_putp = sccp_rput,		/* Read put (message from below) */
	.qi_srvp = sccp_rsrv,		/* Read queue service */
	.qi_qopen = sccp_qopen,		/* Each open */
	.qi_qclose = sccp_qclose,	/* Last close */
	.qi_minfo = &sccp_rinfo,	/* Information */
	.qi_mstat = &sccp_rstat,	/* Statistics */
};
static struct qinit sccp_winit = {
	.qi_putp = sccp_wput,		/* Write put (message from above) */
	.qi_srvp = sccp_wsrv,		/* Write queue service */
	.qi_minfo = &sccp_winfo,	/* Information */
	.qi_mstat = &sccp_wstat,	/* Statistics */
};
static struct qinit mtp_rinit = {
	.qi_putp = mtp_rput,		/* Read put (message from below) */
	.qi_srvp = mtp_rsrv,		/* Read queue service */
	.qi_minfo = &mtp_rinfo,		/* Information */
	.qi_mstat = &mtp_rstat,		/* Statistics */
};
static struct qinit mtp_winit = {
	.qi_putp = mtp_wput,		/* Write put (message from above) */
	.qi_srvp = mtp_wsrv,		/* Write queue service */
	.qi_minfo = &mtp_winfo,		/* Information */
	.qi_mstat = &mtp_wstat,		/* Statistics */
};

static struct streamtab sccpinfo = {
	.st_rdinit = &sccp_rinit,	/* Upper read queue */
	.st_wrinit = &sccp_winit,	/* Upper write queue */
	.st_muxrinit = &mtp_rinit,	/* Lower read queue */
	.st_muxwinit = &mtp_winit,	/* Lower write queue */
};

/*
 *  =========================================================================
 *
 *  Registration and initialization
 *
 *  =========================================================================
 */
#ifdef LINUX
/*
 *  Linux Registration
 *  -------------------------------------------------------------------------
 */

unsigned short modid = DRV_ID;

#ifndef module_param
MODULE_PARM(modid, "h");
#else
module_param(modid, ushort, 0444);
#endif
MODULE_PARM_DESC(modid, "Module ID for the SCCP driver. (0 for allocation.)");

major_t major = CMAJOR_0;

#ifndef module_param
MODULE_PARM(major, "h");
#else
module_param(major, uint, 0444);
#endif
MODULE_PARM_DESC(major, "Device number for the SCCP driver. (0 for allocation.)");

/*
 *  Linux Fast-STREAMS Registration
 *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

static struct cdevsw sccp_cdev = {
	.d_name = DRV_NAME,
	.d_str = &sccpinfo,
	.d_flag = D_MP,
	.d_fop = NULL,
	.d_mode = S_IFCHR,
	.d_kmod = THIS_MODULE,
};

MODULE_STATIC void __exit
sccpterminate(void)
{
	int err;

	if ((err = unregister_strdev(&sccp_cdev, major)))
		cmn_err(CE_PANIC, "%s: cannot unregister major %d", DRV_NAME, major);
	if ((err = sccp_term_caches()))
		cmn_err(CE_WARN, "%s: could not terminate caches", DRV_NAME);
	if (major)
		major = 0;
	return;
}

MODULE_STATIC int __init
sccpinit(void)
{
	int err;

	cmn_err(CE_NOTE, DRV_BANNER);	/* console splash */
	if ((err = sccp_init_caches())) {
		cmn_err(CE_WARN, "%s: could not init caches, err = %d", DRV_NAME, err);
		sccpterminate();
		return (err);
	}
	if ((err = register_strdev(&sccp_cdev, major)) < 0) {
		cmn_err(CE_WARN, "%s: could not register driver, err = %d", DRV_NAME, err);
		sccpterminate();
		return (err);
	}
	if (major == 0)
		major = err;
	return (0);
}

/*
 *  Linux Kernel Module Initialization
 *  -------------------------------------------------------------------------
 */
module_init(sccpinit);
module_exit(sccpterminate);

#endif				/* LINUX */
