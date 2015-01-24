/*****************************************************************************

 @(#) src/drivers/sctp_compat.h

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

#ifndef __LOCAL_SCTP_COMPAT_H__
#define __LOCAL_SCTP_COMPAT_H__

#include <sys/os7/compat.h>

#undef sctp_addr
#define sctp_addr n_sctp_addr
#undef sctp_addr_t
#define sctp_addr_t n_sctp_addr_t
#include <sys/npi.h>
#include <sys/npi_sctp.h>
#undef sctp_addr

#if defined HAVE_TIHDR_H
#   include <tihdr.h>
#else
#   include <sys/tihdr.h>
#endif

#include <sys/xti.h>
#include <sys/xti_xti.h>
#include <sys/xti_ip.h>
#include <sys/xti_inet.h>
#include <sys/xti_sctp.h>

#define T_ALLLEVELS -1

#ifdef LINUX
#include <linux/interrupt.h>	/* for local_irq functions */
#ifdef HAVE_KINC_ASM_SOFTIRQ_H
#include <asm/softirq.h>	/* for start_bh_atomic, end_bh_atomic */
#endif
#include <linux/random.h>	/* for secure_tcp_sequence_number */
#ifdef HAVE_KINC_LINUX_RCUPDATE_H
#include <linux/rcupdate.h>
#endif
#endif				/* LINUX */

#define sctp_addr	        sctp_addr__
#define sctp_daddr		sctp_daddr__
#define sctp_saddr		sctp_saddr__
#define sctp_strm		sctp_strm__
#define sctp_dup		sctp_dup__
#define sctp_bind_bucket	sctp_bind_bucket__
#if defined HAVE_OPENSS7_SCTP
#define sctp_mib		sctp_mib__
#endif
#define sctphdr			sctphdr__
#define sctp_cookie		sctp_cookie__
#define sctp_chunk		sctp_chunk__

#ifdef ASSERT
#undef ASSERT
#endif

#include <linux/bitops.h>

#define sctp_tst_bit(nr,addr)	test_bit(nr,addr)
#define sctp_set_bit(nr,addr)	__set_bit(nr,addr)
#define sctp_clr_bit(nr,addr)	__clear_bit(nr,addr)

#include <net/sock.h>
#include <net/ip.h>
#include <net/icmp.h>
#include <net/route.h>
#include <net/inet_ecn.h>
#include <net/snmp.h>
#include <net/protocol.h>

#undef sctp_addr
#undef sctp_daddr
#undef sctp_saddr
#undef sctp_strm
#undef sctp_dup
#undef sctp_bind_bucket
#if defined HAVE_OPENSS7_SCTP
#undef sctp_mib
#endif
#undef sctphdr
#undef sctp_cookie
#undef sctp_chunk

#ifdef ASSERT
#undef ASSERT
#endif

#if defined HAVE_OPENSS7_SCTP
#   undef sctp_bind_bucket
#   undef sctp_dup
#   undef sctp_strm
#   undef sctp_saddr
#   undef sctp_daddr
#   undef sctp_protolist
#endif

#if defined HAVE_LKSCTP_SCTP
#undef sctp_bind_bucket
#undef sctp_mib
#undef sctphdr
#undef sctp_cookie
#undef sctp_chunk
#endif

#ifdef SCTP_CONFIG_MODULE
#define __SCTP_STATIC STATIC
#define __SCTP_INIT
#else
#define __SCTP_STATIC
#define __SCTP_INIT __init
#endif

#ifdef LINUX
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/inetdevice.h>
#endif				/* LINUX */

#endif				/* __LOCAL_SCTP_COMPAT_H__ */
