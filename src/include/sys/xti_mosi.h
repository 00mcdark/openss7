/*****************************************************************************

 @(#) $Id: xti_mosi.h,v 1.1.2.1 2009-06-21 11:25:39 brian Exp $

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

 Last Modified $Date: 2009-06-21 11:25:39 $ by $Author: brian $

 -----------------------------------------------------------------------------

 $Log: xti_mosi.h,v $
 Revision 1.1.2.1  2009-06-21 11:25:39  brian
 - added files to new distro

 *****************************************************************************/

#ifndef _SYS_XTI_MOSI_H
#define _SYS_XTI_MOSI_H

/* This file can be processed with doxygen(1). */

/** @addtogroup xnet
  * @{ */

/** @file
  * XTI Minimal OSI header file.  */

/*
 * Minimal OSI Header File.
 */

#define T_AP_MAX_ADDR	128

/**
  * MOSI address structure.
  */
struct t_mosiaddr {
	t_uscalar_t flags;
	t_scalar_t osi_ap_inv_id;
	t_scalar_t osi_ae_inv_id;
	unsigned int osi_apt_len;
	unsigned int osi_aeq_len;
	unsigned int osi_paddr_len;
	unsigned char osi_addr[T_AP_MAX_ADDR];
};

#define T_OSI_AP_IID_BIT    (1<<0)  /**< The osi_ap_inv_id field is valid. */
#define T_OSI_AE_IID_BIT    (1<<1)  /**< The osi_ae_inv_id field is valid. */

#define T_ISO_APCO	0x0200
#define T_ISO_APCL	0x0300
#define T_AP_CNTX_NAME	0x1
#define T_AP_PCL	0x2

/** { iso(1) standard(0) curl(11188) mosi(3) default-application-context(3) } */
#define T_AP_CNTX_NAME_DEFAULT	"\x28\xB4\x57\x3\x3"
/** { iso(1) standard(0) curl(11188) mosi(3) default-abstract-syntax(1) version(1) } */
#define T_AP_ABST_SNTX_DEFAULT	"\x28\xB4\x57\x3\x1\x1"
/** { iso(1) standard(0) curl(11188) mosi(3) default-transfer-syntax(2) version(1) } */
#define T_AP_TRAN_SNTX_DEFAULT	"\x28\xB4\x57\x3\x2\x1"

#define T_OPT_VALEN(opt) (opt->len - sizeof(struct t_opthdr)).

/**
  * Presentation Context structure.
  * Presentation context definition and result list option.
  */
struct t_ap_pco_el {
	t_scalar_t count;
	t_scalar_t offset;
};

/**
  * Presentation Context item header.
  */
struct t_ap_pc_item {
	t_scalar_t pci;			/**< Unique odd integer. */
	t_scalar_t res;			/**< Result of negotiation. */
};

/**
  * Presentation Context item element.
  */
struct t_ap_syn_off {
	t_scalar_t size;		/**< Length of syntax object identifier contents. */
	t_scalar_t offset;		/**< Offset of object identifier for the syntax. */
};

/**
  * @name Presentation Context Results
  * Values for res of a presentation context item.
  *
  * @{ */
#define T_PCL_ACCEPT		    0x0000	/**< Pres context accepted. */
#define T_PCL_USER_REJ		    0x0100	/**< Pres context rejected by peer application. */
#define T_PCL_PREJ_RSN_NSPEC	    0x0200	/**< Prov reject: no reason specified. */
#define T_PCL_PREJ_A_SYTX_NSUP	    0x0201	/**< Prov reject: abstract syntax not supported. */
#define T_PCL_PREJ_T_SYTX_NSUP	    0x0202	/**< Prov reject: transfer syntax not supported. */
#define T_PCL_PREJ_LMT_DCS_EXCEED   0x0203	/**< Prov reject: local limit on DCS exceeded. */
/** @} */

/**
  * @name Disconnect Reasons
  * Reason codes for disconnection.
  *
  * @{ */
#define T_AC_U_AARE_NONE	    0x0001	/**< Con rej by peer user: no reason given. */
#define T_AC_C_U_AARE_ACN	    0x0002	/**< Con rej: application context name not supported. */
#define T_AC_U_AARE_APT		    0x0003	/**< Con rej: AP title not recognized. */
#define T_AC_U_AARE_AEQ		    0x0005	/**< Con rej: AE qualifier not recognized. */
#define T_AC_U_AARE_PEER_AUTH	    0x000e	/**< Con rej: authentication required. */
#define T_AC_P_ABRT_NSPEC	    0x0011	/**< Aborted by peer: not reason given. */
#define T_AC_P_AARE_VERSION	    0x0012	/**< Con rej: no common version. */
/** @} */

#endif				/* _SYS_XTI_MOSI_H */

/** @} */

// vim: com=srO\:/**,mb\:*,ex\:*/,srO\:/*,mb\:*,ex\:*/,b\:TRANS
