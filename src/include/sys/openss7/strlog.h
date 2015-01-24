/*****************************************************************************

 @(#) src/include/sys/openss7/strlog.h

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

#ifndef __SYS_OPENSS7_STRLOG_H__
#define __SYS_OPENSS7_STRLOG_H__

/* This file can be processed with doxygen(1). */

#ifndef __SYS_STRLOG_H__
#warning "Do not include sys/openss7/strlog.h directly, include sys/strlog.h instead."
#endif

#include <stdarg.h>

#define STRLOGERR	0	/* log error information */
#define STRLOGNO	0	/* log notice information */
#define STRLOGST	1	/* log state transitions */
#define STRLOGTO	2	/* log timeouts */
#define STRLOGRX	3	/* log primitives received */
#define STRLOGTX	4	/* log primitives issued */
#define STRLOGTE	5	/* log timer events */
#define STRLOGIO	6	/* log additional data */
#define STRLOGDA	7	/* log data */

#define SL_ERROR    0x0001
#define SL_TRACE    0x0002
#define SL_NOTIFY   0x0004
#define SL_CONSOLE  0x0008
#define SL_FATAL    0x0010
#define SL_WARN	    0x0020
#define SL_NOTE	    0x0040
#define SL_NOPUTBUF 0x0080	/* uw7 src compatibility (does nothing) */

#define LOGMSGSZ    1024	/* max format string length */
#define NLOGARGS    20		/* max number of arguments (really unlimited) */

#define LOGCTL		(('L')<<8)
#define I_ERRLOG	(LOGCTL | 1)	/* error logger */
#define I_TRCLOG	(LOGCTL | 2)	/* trace logger */
#define I_CONSLOG	(LOGCTL | 3)	/* console logger */

#ifdef __KERNEL__

#ifndef streams_fastcall
#if defined __i386__ || defined __x86_64__ || defined __k8__
#define streams_fastcall __attribute__((__regparm__(3)))
#else
#define streams_fastcall
#endif
#endif

#ifndef __EXTERN
#define __EXTERN extern
#endif

#ifndef __STREAMS_EXTERN
#define __STREAMS_EXTERN __EXTERN streams_fastcall
#endif

__STREAMS_EXTERN int strlog(short mid, short sid, char level, unsigned short flags, char *fmt, ...)
    __attribute__ ((__format__(__printf__, 5, 6)));
__STREAMS_EXTERN int vstrlog(short mid, short sid, char level, unsigned short flag, char *fmt,
			     va_list args);

typedef int (*vstrlog_t) (short, short, char, unsigned short, char *, va_list);
__STREAMS_EXTERN vstrlog_t register_strlog(vstrlog_t newlog);

#else				/* __KERNEL__ */

#include <stdio.h>

extern int strlog(short mid, short sid, char level, unsigned short flags, char *fmt, ...)
    __attribute__ ((__format__(__printf__, 5, 6)));
extern int vstrlog(short mid, short sid, char level, unsigned short flag, char *fmt, va_list args);
extern int pstrlog(FILE *file, struct strbuf *ctrl, struct strbuf *data);

#endif				/* __KERNEL__ */

#ifdef __LP64__

struct trace_ids {
	int16_t ti_mid;
	int16_t ti_sid;
	char ti_level;
	int16_t ti_flags;		/* not for Solaris */
};

struct log_ctl {
	int16_t mid;
	int16_t sid;
	char level;
	int16_t flags;
	int32_t ltime;			/* clock32_t or clock_t under Solaris */
	int32_t ttime;			/* time32_t or time_t under Solaris */
	int32_t seq_no;
	int32_t pri;			/* priority = (facility|level) except HPUX */
};

#else				/* __LP64__ */

struct trace_ids {
	short ti_mid;
	short ti_sid;
	char ti_level;
	short ti_flags;			/* not for Solaris */
};

struct log_ctl {
	short mid;
	short sid;
	char level;
	short flags;
	clock_t ltime;			/* clock32_t or clock_t under Solaris */
	clock_t ttime;			/* time32_t or time_t under Solaris */
	int seq_no;
	int pri;			/* priority = (facility|level) except HPUX */
};

#endif				/* __LP64__ */

#endif				/* __SYS_OPENSS7_STRLOG_H__ */
