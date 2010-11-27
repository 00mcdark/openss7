/*****************************************************************************

 @(#) $Id: timod.h,v 1.1.2.1 2009-06-21 11:25:38 brian Exp $

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

 $Log: timod.h,v $
 Revision 1.1.2.1  2009-06-21 11:25:38  brian
 - added files to new distro

 *****************************************************************************/

#ifndef _SYS_TIMOD_H
#define _SYS_TIMOD_H

/* This file can be processed with doxygen(1). */

/** @addtogroup timod
  * @{ */

/** @file
  * Transport Interface Module (timod) header file.  */

/*
 * Transport Interface "timod" Header File.
 */

/** @name Old (TLI) transport interface input-output controls.
  * @{ */
#define TIMOD			('T'<<8)
#define O_TI_GETINFO		(TIMOD|100)	/**< Get info (1 under OSF). */
#define O_TI_OPTMGMT		(TIMOD|101)	/**< Manage options (2 under OSF). */
#define O_TI_BIND		(TIMOD|102)	/**< Bind TP (3 under OSF). */
#define O_TI_UNBIND		(TIMOD|103)	/**< Unbind TP (4 under OSF). */
/** @} */

/** @name Old ti_flags Flags.
  * @{ */
#define O_TI_USED		0x01	/**< Data structure in use. */
#define O_TI_FATAL		0x02	/**< Fatal M_ERROR_occured. */
#define O_TI_WAITIOCACK		0x04	/**< Waiting for info for ioctl act. */
#define O_TI_MORE		0x08	/**< More data. */
/** @} */

/** Old (TLI) transport interface user structure.
  */
struct _o_ti_user {
	ushort ti_flags;		/**< Flags. */
	int ti_rcvsize;			/**< Receive buffer size. */
	char *ti_rcvbuf;		/**< Receive buffer. */
	int ti_ctlsize;			/**< Control buffer size. */
	char *ti_ctlbuf;		/**< Control buffer. */
	char *ti_lookdbuf;		/**< Look data buffer. */
	char *ti_lookcbuf;		/**< Look ctrl buffer. */
	int ti_lookdsize;		/**< Look data buffer size. */
	int ti_lookcsize;		/**< Look ctrl buffer size. */
	int ti_maxpsz;			/**< TIDU size. */
#ifdef __LP64__
	u_int32_t ti_servtype;		/**< Service type. */
#else					/* __LP64__ */
	long ti_servtype;		/**< Service type. */
#endif					/* __LP64__ */
	int ti_lookflg;			/**< Buffered look flag. */
};

/**
  * @name Old timod Input/Output Controls
  * OSF specific compatibility constants.
  * These clash with termios ioctls if 'T' is used.
  * OSF uses 't' instead.
  * @{
  */
#define _O_TI_GETINFO		(TIMOD|1)	/**< (OSF) Get information from TP. */
#define _O_TI_OPTMGMT		(TIMOD|2)	/**< (OSF) Manage options for TP. */
#define _O_TI_BIND		(TIMOD|3)	/**< (OSF) Bind a Transport Provider. */
#define _O_TI_UNBIND		(TIMOD|4)	/**< (OSF) Unbind a Transport Provider. */
#define _O_TI_GETMYNAME		(TIMOD|5)	/**< (OSF) Get local addresses. */
#define _O_TI_GETPEERNAME	(TIMOD|6)	/**< (OSF) Get remote addresses. */
#define _O_TI_XTI_HELLO		(TIMOD|7)	/**< (OSF) Accept a Transport Connection. */
#define _O_TI_XTI_GET_STATE	(TIMOD|8)	/**< (OSF) Form a Transport Connection. */
#define _O_TI_XTI_CLEAR_EVENT	(TIMOD|9)	/**< (OSF) Synchronize user/kernel data. */
#define _O_TI_XTI_MODE		(TIMOD|10)	/**< (OSF) Get addresses from TP. */
#define _O_TI_TLI_MODE		(TIMOD|11)	/**< (OSF) Get TP capabilities. */
/** @} */

typedef struct xti_state {
	unsigned int xtis_qlen;		/* Saved qlen parameter from t_bind */
} XTIS, *XTISP;

/** 
  * @name timod Input/Output Controls
  * Some of these clash with BSD termios ioctls if 'T' is used.
  * Linux wrongly uses 'T' instead of 't' for BSD ioctls.
  * @{
  */
#define TI_GETINFO		(TIMOD|140)	/**< Get information from TP. */
#define TI_OPTMGMT		(TIMOD|141)	/**< Manage options for TP. */
#define TI_BIND			(TIMOD|142)	/**< Bind a Transport Provider. */
#define TI_UNBIND		(TIMOD|143)	/**< Unbind a Transport Provider. */
#define TI_GETMYNAME		(TIMOD|144)	/**< Get local addresses. */
#define TI_GETPEERNAME		(TIMOD|145)	/**< Get remote addresses. */
#define TI_SETMYNAME		(TIMOD|146)	/**< Accept a Transport Connection. */
#define TI_SETPEERNAME		(TIMOD|147)	/**< Form a Transport Connection. */
#define TI_SYNC			(TIMOD|148)	/**< Synchronize user/kernel data. */
#define TI_GETADDRS		(TIMOD|149)	/**< Get addresses from TP. */
#define TI_CAPABILITY		(TIMOD|150)	/**< Get TP capabilities. */
/** @} */

/** Structure for use with TI_SYNC request.
  */
struct ti_sync_req {
	u_int32_t tsr_flags;
};

/** @name tsr_flags Flags
  * Flags for use with #ti_sync_req#tsr_flags.
  * @{ */
#define TSRF_INFO_REQ		01		/**< Request information. */
#define TSRF_IS_EXP_IN_RCVBUF	02		/**< Is Exp data in Receive buffer? */
#define TSRF_QLEN_REQ		04		/**< Request queue length. */
/** @} */

/** Structure for use with TI_SYNC response.
  */
struct ti_sync_ack {
	t_scalar_t PRIM_type;
	t_scalar_t TSDU_size;
	t_scalar_t ETSDU_size;
	t_scalar_t CDATA_size;
	t_scalar_t DDATA_size;
	t_scalar_t ADDR_size;
	t_scalar_t OPT_size;
	t_scalar_t TIDU_size;
	t_scalar_t SERV_type;
	t_scalar_t CURRENT_state;
	t_scalar_t PROVIDER_flag;

	t_uscalar_t tsa_qlen;
	u_int32_t tsa_flags;
};

/** @name tsa_flags Flags
  * Flags for use with #ti_sync_ack#tsa_flags.
  * @{ */
#define TSA_EXP_QUEUED		01		/**< Expedited data is queued. */
/** @} */

#endif				/* _SYS_TIMOD_H */

/** @} */

// vim: com=srO\:/**,mb\:*,ex\:*/,srO\:/*,mb\:*,ex\:*/,b\:TRANS
