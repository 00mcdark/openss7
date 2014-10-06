/*****************************************************************************

 @(#) $Id: m2ua_as.h,v 1.1.2.1 2009-06-21 11:25:36 brian Exp $

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

 Last Modified $Date: 2009-06-21 11:25:36 $ by $Author: brian $

 -----------------------------------------------------------------------------

 $Log: m2ua_as.h,v $
 Revision 1.1.2.1  2009-06-21 11:25:36  brian
 - added files to new distro

 *****************************************************************************/

#ifndef __SYS_M2UA_AS_H__
#define __SYS_M2UA_AS_H__

/* This file can be processed by doxygen(1). */

typedef lmi_long m2_long;
typedef lmi_ulong m2_ulong;
typedef lmi_ushort m2_ushort;
typedef lmi_uchar m2_uchar;

#define M2UA_AS_IOC_MAGIC			('2' + 30)

#define M2UA_AS_OBJ_TYPE_DF	0	/* Defaults */
#define M2UA_AS_OBJ_TYPE_LM	1	/* Layer Management */
#define M2UA_AS_OBJ_TYPE_SL	2	/* Signalling Link (User) */
#define M2UA_AS_OBJ_TYPE_AS	3	/* Application Server */
#define M2UA_AS_OBJ_TYPE_SP	4	/* Application Server Process */
#define M2UA_AS_OBJ_TYPE_SG	5	/* Signalling Gateway */
#define M2UA_AS_OBJ_TYPE_XP	6	/* SCTP Transport Provider */

/* Note that the SL and XP objects are more for obtaining information about streams opened on the
 * upper multiplex and linked on the lower multiplex. */

/*
 * Rather than get too detailed with the object model, we define a more complete address for M2UA.
 */

#define M2UA_AS_CLEI_MAX	32

struct m2ua_ppa {
	m2_long sgid;
	m2_ushort sdti;
	m2_ushort sdli;
	m2_ulong iid;
	char iid_text[M2UA_AS_CLEI_MAX];
	m2_ulong tmode;
};

#define M2UA_AS_TMODE_NONE		0
#define M2UA_AS_TMODE_OVERRIDE		1
#define M2UA_AS_TMODE_LOADSHARE		2
#define M2UA_AS_TMODE_BROADCAST		3

/* Default options */
typedef struct m2ua_opt_conf_df {
	struct lmi_option sl_proto;
	struct lmi_option sp_proto;
	struct lmi_option sg_proto;
	lmi_ulong tack;
	lmi_ulong tbeat;
	lmi_ulong tidle;
	lmi_ulong testab;
	lmi_ulong ppi;
	lmi_ushort istreams;
	lmi_ushort ostreams;
} m2ua_opt_conf_df_t;

/* Layer Management options */
typedef struct m2ua_opt_conf_lm {
} m2ua_opt_conf_lm_t;

/* Signalling Link options */
/* Signalling Link options are specified using SLI IOCTLs, however, the values of timers
   T1, T2, T3, and T4 are echoed here for use by M2UA. */
typedef struct m2ua_opt_conf_sl {
	lmi_ulong t1;
	lmi_ulong t2;
	lmi_ulong t2h;
	lmi_ulong t2l;
	lmi_ulong t3;
	lmi_ulong t4e;
	lmi_ulong t4n;
} m2ua_opt_conf_sl_t;

/* Application Server options */
typedef struct m2ua_opt_conf_as {
	lmi_ulong tack;			/* duration of ack timer */
	lmi_ulong tbeat;		/* duration of heartbeat time */
	lmi_ulong testab;		/* duration of establish timer (TS 102 141) */
} m2ua_opt_conf_as_t;

/* Application Server Process options */
typedef struct m2ua_opt_conf_sp {
	struct lmi_option options;
	lmi_ulong aspid;		/* External M2UA ASP Identifier */
	lmi_ulong tack;			/* duration of ack timer */
	lmi_ulong tbeat;		/* duration of heartbeat timer */
	lmi_ulong tidle;		/* duration of idle timer */
} m2ua_opt_conf_sp_t;

/* Signalling Gateway options */
typedef struct m2ua_opt_conf_sg {
	struct lmi_option options;
} m2ua_opt_conf_sg_t;

/* protocol options */
#define M2UA_SG_ASPEXT		(1<<0)	/* SG supports draft-bidulock-sigtran-aspext */
#define M2UA_SG_SGINFO		(1<<1)	/* SG supports draft-bidulock-sigtran-sginfo */
#define M2UA_SG_LOADSEL		(1<<2)
#define M2UA_SG_LOADGRP		(1<<3)
#define M2UA_SG_CORID		(1<<4)
#define M2UA_SG_REGEXT		(1<<5)
#define M2UA_SG_SESSID		(1<<6)
#define M2UA_SG_DYNAMIC		(1<<7)	/* SG supports dynamic registration */
/* (1<<8) reserved */
#define M2UA_SG_ASPCONG		(1<<9)
#define M2UA_SG_TEXTIID		(1<<10)	/* SG supports text interface identifiers */

/* protocol variants */
#define M2UA_VERSION_NONE	0	/* interoperable */
#define M2UA_VERSION_RFC3331	1	/* per RFC 3331 */
#define M2UA_VERSION_TS102141	2	/* per ETSI TS 102 141 */
#define M2UA_VERSION_DEFAULT	M2UA_VERSION_RFC3331

/* SCTP Transport Provider options */
/* Other transport options are specified using the TPI interface. */
typedef struct m2ua_opt_conf_xp {
	lmi_ulong ppi;			/* Payload Protocol Identifier (2) */
	lmi_ushort istreams;		/* Maximum number of input streams (default 257). */
	lmi_ushort ostreams;		/* Requested number of output streams (default 257). */
	lmi_ulong loc_len;		/* loc_add significant length */
	lmi_ulong rem_len;		/* rem_add significant length */
	struct sockaddr loc_add;	/* local address */
	struct sockaddr rem_add;	/* remote address */
} m2ua_opt_conf_xp_t;

#define M2UA_AS_ISTREAMS_DEFAULT	257
#define M2UA_AS_OSTREAMS_DEFAULT	257

/*
 *  OPTIONS
 */

typedef struct m2ua_option {
	lmi_ulong type;
	lmi_ulong id;
	/* followed by object-specific option structure */
} m2ua_option_t;

#define M2UA_AS_IOCGOPTIONS	_IOR(	M2UA_AS_IOC_MAGIC,	 0,	m2ua_option_t		)
#define M2UA_AS_IOCSOPTIONS	_IOW(	M2UA_AS_IOC_MAGIC,	 1,	m2ua_option_t		)

/* Default configuration */
typedef struct m2ua_conf_df {
	/* No configuration required */
} m2ua_conf_df_t;

/* Layer Management configuration */
/* No configuration required. The object identifier is the dev_t device number of the
   opened layer management stream. */
typedef struct m2ua_conf_lm {
} m2ua_conf_lm_t;

/* Signalling Link configuration */
/* No configuration required. The object identifier is the dev_t device number of the
   opened signalling link stream. */
typedef struct m2ua_conf_sl {
} m2ua_conf_sl_t;

/* Application Server configuration */
typedef struct m2ua_conf_as {
	struct m2ua_ppa ppa;
} m2ua_conf_as_t;

/* Application Server Process configuration */
typedef struct m2ua_conf_sp {
} m2ua_conf_sp_t;

/* Signalling Gateway configuration */
typedef struct m2ua_conf_sg {
} m2ua_conf_sg_t;

/* SCTP Transport Provider configuration */
/* The linked lower stream must be associated with an sgid.  The object identifier is the
   muxid of the linked transport provider stream. */
typedef struct m2ua_conf_xp {
	lmi_ulong sgid;			/* SG identifier */
} m2ua_conf_xp_t;

/*
 *  CONFIGURATION
 */

typedef struct m2ua_config {
	lmi_ulong type;			/* object type */
	lmi_ulong id;			/* object id */
	lmi_ulong cmd;			/* object command/count */
	/* followed by object-specific config structure */
} m2ua_config_t;

#define M2UA_GET	0
#define M2UA_ADD	1
#define M2UA_CHA	2
#define M2UA_MOD	3
#define M2UA_DEL	4

#define M2UA_AS_IOCGCONFIG	_IOWR(	M2UA_AS_IOC_MAGIC,	 2,	m2ua_config_t		)
#define M2UA_AS_IOCSCONFIG	_IOWR(	M2UA_AS_IOC_MAGIC,	 3,	m2ua_config_t		)
#define M2UA_AS_IOCTCONFIG	_IOWR(	M2UA_AS_IOC_MAGIC,	 4,	m2ua_config_t		)
#define M2UA_AS_IOCCCONFIG	_IOWR(	M2UA_AS_IOC_MAGIC,	 5,	m2ua_config_t		)
#define M2UA_AS_IOCLCONFIG	_IOWR(	M2UA_AS_IOC_MAGIC,	14,	m2ua_config_t		)

typedef struct m2ua_timers_df {
} m2ua_timers_df_t;
typedef struct m2ua_statem_df {
	struct m2ua_timers_df timers;
	lmi_ulong df_numb;		/* number of id/state pairs for DF */
	/* followed by id/state paris for DF */
	/* terminated by zero */
} m2ua_statem_df_t;

typedef struct m2ua_timers_lm {
} m2ua_timers_lm_t;
typedef struct m2ua_statem_lm {
	struct m2ua_timers_lm timers;
	lmi_ulong lm_numb;		/* number of id/state pairs for LM */
	/* followed by id/state paris for LM */
	/* terminated by zero */
} m2ua_statem_lm_t;

typedef struct m2ua_timers_sl {
} m2ua_timers_sl_t;
typedef struct m2ua_statem_sl {
	struct m2ua_timers_sl timers;
	lmi_ulong sl_numb;		/* number of id/state pairs for SL */
	/* followed by id/state paris for SL */
	/* terminated by zero */
} m2ua_statem_sl_t;

typedef struct m2ua_timers_as {
} m2ua_timers_as_t;
typedef struct m2ua_statem_as {
	struct m2ua_timers_as timers;
	lmi_ulong as_numb;		/* number of id/state pairs for AS */
	/* followed by id/state paris for AS */
	/* terminated by zero */
} m2ua_statem_as_t;

typedef struct m2ua_timers_sp {
} m2ua_timers_sp_t;
typedef struct m2ua_statem_sp {
	struct m2ua_timers_sp timers;
	lmi_ulong asp_numb;		/* number of id/state pairs for ASP */
	/* followed by id/state paris for SG */
	/* terminated by zero */
} m2ua_statem_sp_t;

typedef struct m2ua_timers_sg {
} m2ua_timers_sg_t;
typedef struct m2ua_statem_sg {
	struct m2ua_timers_sg timers;
	lmi_ulong sg_numb;		/* number of id/state pairs for SG */
	/* followed by id/state paris for SG */
	/* terminated by zero */
} m2ua_statem_sg_t;

typedef struct m2ua_timers_xp {
} m2ua_timers_xp_t;
typedef struct m2ua_statem_xp {
	struct m2ua_timers_xp timers;
	lmi_ulong xp_numb;		/* number of id/state pairs for XP */
	/* followed by id/state paris for XP */
	/* terminated by zero */
} m2ua_statem_xp_t;

/*
 *  STATE
 */

typedef struct m2ua_statem {
	lmi_ulong type;			/* object type */
	lmi_ulong id;			/* object id */
	lmi_ulong flags;		/* object flags */
	lmi_ulong state;		/* object state */
	/* followed by object-specific state structure */
} m2ua_statem_t;

#define M2UA_AS_IOCGSTATEM	_IOWR(	M2UA_AS_IOC_MAGIC,	6,	m2ua_statem_t		)
#define M2UA_AS_IOCCMRESET	_IOWR(	M2UA_AS_IOC_MAGIC,	7,	m2ua_statem_t		)

typedef struct m2ua_stats_df {
} m2ua_stats_df_t;

typedef struct m2ua_stats_lm {
} m2ua_stats_lm_t;

typedef struct m2ua_stats_sl {
} m2ua_stats_sl_t;

typedef struct m2ua_stats_as {
} m2ua_stats_as_t;

typedef struct m2ua_stats_sp {
} m2ua_stats_sp_t;

typedef struct m2ua_stats_sg {
} m2ua_stats_sg_t;

typedef struct m2ua_stats_xp {
} m2ua_stats_xp_t;

/*
 *  STATISTICS
 */
typedef struct m2ua_stats {
	lmi_ulong type;			/* object type */
	lmi_ulong id;			/* object id */
	lmi_ulong header;		/* object stats header */
	/* followed by object-specific stats structure */
} m2ua_stats_t;

#define M2UA_AS_IOCGSTATSP	_IOWR(	M2UA_AS_IOC_MAGIC,	 8,	m2ua_stats_t		)
#define M2UA_AS_IOCSSTATSP	_IOWR(	M2UA_AS_IOC_MAGIC,	 9,	m2ua_stats_t		)
#define M2UA_AS_IOCGSTATS	_IOWR(	M2UA_AS_IOC_MAGIC,	10,	m2ua_stats_t		)
#define M2UA_AS_IOCCSTATS	_IOWR(	M2UA_AS_IOC_MAGIC,	11,	m2ua_stats_t		)

/*
 *  EVENTS
 */
typedef struct m2ua_notify {
	lmi_ulong type;			/* object type */
	lmi_ulong id;			/* object id */
	lmi_notify_t notify;		/* notifications */
} m2ua_notify_t;

#define M2UA_AS_IOCGNOTIFY	_IOR(	M2UA_AS_IOC_MAGIC,	12,	m2ua_notify_t		)
#define M2UA_AS_IOCSNOTIFY	_IOR(	M2UA_AS_IOC_MAGIC,	13,	m2ua_notify_t		)
#define M2UA_AS_IOCCNOTIFY	_IOR(	M2UA_AS_IOC_MAGIC,	14,	m2ua_notify_t		)

/*
 *  MANAGEMENT
 */
typedef struct m2ua_mgmt {
	lmi_ulong type;			/* object type */
	lmi_ulong id;			/* object id */
	lmi_ulong cmd;			/* object command */
} m2ua_mgmt_t;

#define M2UA_AS_MGMT_UP			1
#define M2UA_AS_MGMT_DOWN		2
#define M2UA_AS_MGMT_ACTIVATE		3
#define M2UA_AS_MGMT_DEACTIVATE		4
#define M2UA_AS_MGMT_UP_BLOCK		5
#define M2UA_AS_MGMT_UP_UNBLOCK		6
#define M2UA_AS_MGMT_ACT_BLOCK		7
#define M2UA_AS_MGMT_ACT_UNBLOCK	8

#define M2UA_AS_IOCCMGMT	_IOWR(	M2UA_AS_IOC_MAGIC,	15,	m2ua_mgmt_t		)

/*
 *  CONTROL LOWER
 */
typedef struct m2ua_pass {
	lmi_ulong muxid;
	lmi_ulong type;
	lmi_ulong band;
	lmi_ulong ctl_length;
	lmi_ulong dat_length;
	/* followed by cntl and data part of message to pass to lower */
} m2ua_pass_t;

#define M2UA_AS_IOCCPASS	_IOWR(	M2UA_AS_IOC_MAGIC,	16,	m2ua_pass_t		)

#define M2UA_AS_IOC_FIRST	0
#define M2UA_AS_IOC_LAST	17
#define M2UA_AS_IOC_PRIVATE	32

#endif				/* __SYS_M2UA_AS_H__ */
