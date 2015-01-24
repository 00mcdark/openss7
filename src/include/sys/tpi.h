/*****************************************************************************

 @(#) src/include/sys/tpi.h

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2015  Monavacon Limited <http://www.monavacon.com/>
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

 *****************************************************************************/

#ifndef _SYS_TPI_H
#define _SYS_TPI_H

/* This file can be processed with doxygen(1). */

/** @addtogroup tpi
  * @{ */

/** @file
  * TPI Common Header File.
  *
  * This file contains definitions that are common to XTI, TLI and TPI.  It is
  * included by <xti.h> and <tiuser.h> as well as <tihdr.h>.  */

#if 0
#if !defined _XTI_H && !defined _TIUSER_H && !defined _TIHDR_H && !defined __KERNEL__
#error ****
#error **** Do not include system header files directly in user-space
#error **** programs.  Likely you should have included <timod.h>
#error **** instead of <sys/timod.h>.
#error ****
#endif
#endif

/*
 * TPI Common Header File.
 *
 * This file contains definitions that are common to XTI, TLI and TPI.  It is
 * included by <xti.h> and <tiuser.h> as well as <tihdr.h>.
 */

/**
  * @name XTI Errors
  * Transport service error numbers (error codes used by TLI transport
  * providers) The following are the error codes needed by both the kernel level
  * transport providers and the user level library.
  * @{
  */
#define TBADADDR	 1	/**< Bad address format. */
#define TBADOPT		 2	/**< Bad options format. */
#define TACCES		 3	/**< Bad permissions. */
#define TBADF		 4	/**< Bad file descriptor. */
#define TNOADDR		 5	/**< Unable to allocate an address. */
#define TOUTSTATE	 6	/**< Would place interface out of state. */
#define TBADSEQ		 7	/**< Bad call sequence number. */
#define TSYSERR		 8	/**< System error. */
#define TLOOK		 9	/**< Has to t_look() for event			(not TPI). */
#define TBADDATA	10	/**< Bad amount of data. */
#define TBUFOVFLW	11	/**< Buffer was too small			(not TPI). */
#define TFLOW		12	/**< Would block due to flow control		(not TPI). */
#define TNODATA		13	/**< No data indication				(not TPI). */
#define TNODIS		14	/**< No disconnect indication			(not TPI). */
#define TNOUDERR	15	/**< No unitdata error indication		(not TPI). */
#define TBADFLAG	16	/**< Bad flags. */
#define TNOREL		17	/**< No orderly release indication		(not TPI). */
#define TNOTSUPPORT	18	/**< Not supported. */
#define TSTATECHNG	19	/**< State is currently changing		(not TPI). */
/*
   The following are XPG3 and up.
 */
#if defined _XPG3 || defined _XOPEN_SOURCE || defined __KERNEL__
#define TNOSTRUCTYPE	20	/**< Structure type not supported		(not TPI). */
#define TBADNAME	21	/**< Bad transport provider name		(not TPI). */
#define TBADQLEN	22	/**< Listener queue length limit is zero	(not TPI). */
#define TADDRBUSY	23	/**< Address already in use			(not TPI). */
#endif
/*
   The following are _XOPEN_SOURCE (XPG4 and up).
 */
#if defined _XOPEN_SOURCE || defined __KERNEL__
#define TINDOUT		24	/**< Outstanding connect indications		(not TPI). */
#define TPROVMISMATCH	25	/**< Not same transport provider		(not TPI). */
#define TRESQLEN	26	/**< Connection acceptor has qlen > 0		(not TPI). */
#define TRESADDR	27	/**< Conn. acceptor bound to different address	(not TPI). */
#define TQFULL		28	/**< Connection indicator queue is full		(not TPI). */
#define TPROTO		29	/**< Protocol error				(not TPI). */
#endif
/** @} */

/**
  * @name Transport Service Types
  * Transport service types.
  * @{ */
#define T_COTS		1	/**< Connection oriented transport service. */
#define T_COTS_ORD	2	/**< COTS with orderly release. */
#define T_CLTS		3	/**< Connectionless transport service. */
/** @} */

/**
  * @name Transport Provider Flags
  * Transport provider flags.
  * @{ */
#define T_SNDZERO	0x001	/**< Must match <sys/strops.h>: SNDZERO. */
#define T_SENDZERO	0x001	/**< Supports 0-length TSDUs. */
#define T_ORDRELDATA	0x002	/**< Supports orderly release data. */
#define XPG4_1		0x100	/**< This indicates that the transport provider conforms to XTI in
				     XPG4 and supports the new primitives T_ADDR_REQ and T_ADDR_ACK. */
#define T_XPG4_1	0x100	/**< XPG4 and higher. */
/** @} */

/**
  * @name XTI Library Flags
  * Flags used from user level library routines.
  * @{ */
#define T_MORE		0x001	/**< More data. */
#define T_EXPEDITED	0x002	/**< Expedited data. */
#define T_PUSH		0x004	/**< Send data immediately. */
#define T_NEGOTIATE	0x0004	/**< Set options. */
#define T_CHECK		0x0008	/**< Check options. */
#define T_DEFAULT	0x0010	/**< Get default options. */
#define T_SUCCESS	0x0020	/**< Success. */
#define T_FAILURE	0x0040	/**< Failure. */
/*
   The following are _XOPEN_SOURCE (XPG4 and up).
 */
#if defined _XPG4_2 || defined _XOPEN_SOURCE || defined __KERNEL__
#define T_CURRENT	0x0080	/**< Get current options. */
#define T_PARTSUCCESS	0x0100	/**< Partial success. */
#define T_READONLY	0x0200	/**< Option is read only. */
#define T_NOTSUPPORT	0x0400	/**< Option is not supported. */
#endif

#define T_ALLOPT	0	/**< All options at a level. */
/** @} */

/**
  * @name XTI Constants
  * General purpose constants.
  * @{ */
#define T_UNSPEC	(~0-2)	/**< Unspecified value. */
#define T_INVALID	(-2)	/**< No sense. */
#define T_INFINITE	(-1)	/**< No limit. */
#define T_NO		0	/**< No/off/false. */
#define T_YES		1	/**< Yes/on/true. */
#define T_GARBAGE	2
#define T_UNUSED	(-1)
#define	T_NULL		0
#define T_ABSREQ	0x8000	/**< Can be used to determine whether an option
				     name is an absolute requirement. */
/** @} */

#ifndef t_uscalar_t
#define t_uscalar_t uint32_t
#endif

#if !(defined _XPG4_2 || defined _XOPEN_SOURCE) || defined __KERNEL__
/**
  * @name Old TLI Options
  * Older TLI options format and support macros.
  * @{
  */
/** Old TLI options header. */
struct opthdr {
	t_uscalar_t level;		/**< Option level. */
	t_uscalar_t name;		/**< Option name. */
	t_uscalar_t len;		/**< Length of option value. */
	char value[0];			/**< and onwards...  */
};

/** Obtain the length of the option value. */
#define OPTLEN(len)								\
	((((len) + sizeof(t_uscalar_t) - 1) sizeof(t_uscalar_t)) * sizeof(t_uscalar_t))

/** Obtain a pointer to the option value. */
#define OPTVAL(opt)								\
	((char *)((struct opthdr *)opt + 1))
#endif
/** @} */

/**
  * @name XTI Options
  * Newer XOPEN options format and support macros.
  * @{
  */
#if defined _XPG4_2 || defined _XOPEN_SOURCE || defined __KERNEL__

/** XTI option header. */
struct t_opthdr {
	t_uscalar_t len;		/**< Option length, incl. header. */
	t_uscalar_t level;		/**< Option level. */
	t_uscalar_t name;		/**< Option name. */
	t_uscalar_t status;		/**< Negotiation result. */
	char value[0];			/**< and onwards...  */
};

#define _T_ALIGN_SIZE sizeof(t_uscalar_t)

#define _T_ALIGN_OFS(p, o)							\
	((char *)(((unsigned long)(p) - (o) + _T_ALIGN_SIZE - 1)		\
		    & ~(_T_ALIGN_SIZE - 1)) + (o))

#define _T_ALIGN_OFFSET(p)							\
	((unsigned long)(p) & (_T_ALIGN_SIZE - 1))

#define _T_ALIGN_OFS_OFS(p, l, o) _T_ALIGN_OFS((char *)(p) + l, (o))

#define _T_OPT_FIRSTHDR_OFS(b, l, o)						\
	((struct t_opthdr *)(							\
	  (_T_ALIGN_OFS((b), (o)) + sizeof(struct t_opthdr)			\
	   <= (char *)(b) + (l)) ?						\
	   _T_ALIGN_OFS((b), (o)) : NULL))

#define _T_OPT_NEXTHDR_OFS(b, l, p, o)						\
	((struct t_opthdr *)(							\
	  (_T_ALIGN_OFS_OFS((p), (p)->len + sizeof(struct t_opthdr), (o))	\
	   <= ((char *)(b) + (l))) ?						\
	  _T_ALIGN_OFS_OFS((p), (p)->len, (o)) : NULL				\
	))

#define _T_OPT_DATA_OFS(p, o)							\
	((unsigned char *)(_T_ALIGN_OFS((struct t_opthdr *)(p)+1, (o))))

/**
  * Get aligned start of first option header.  This implementation assumes
  * option buffers are allocated by t_alloc() and hence aligned to start any
  * sized object (including option header) is guaranteed.
  */
#define T_OPT_FIRSTHDR(b)	_T_OPT_FIRSTHDR_OFS((b)->buf, (b)->len, 0)

/**
  * Get aligned start of data part after option header This implementation
  * assumes "sizeof (t_uscalar_t)" as the alignment size for its option data and
  * option header with no padding in "struct t_opthdr" definition.
  */
#define T_OPT_NEXTHDR(b, p)	_T_OPT_NEXTHDR_OFS((b)->buf, (b)->len, p, 0)

/**
  * Get aligned start of data part after option header This implementation
  * assumes "sizeof (t_uscalar_t)" as the alignment size for its option data and
  * option header with no padding in "struct t_opthdr" definition.
 */
#define T_OPT_DATA(p)		_T_OPT_DATA_OFS((p), 0)

#endif
/** @} */

#endif				/* _SYS_TPI_H */

/** @} */

// vim: com=srO\:/**,mb\:*,ex\:*/,srO\:/*,mb\:*,ex\:*/,b\:TRANS
