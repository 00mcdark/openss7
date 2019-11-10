/*****************************************************************************

 @(#) File: src/util/strace.c

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2019  Monavacon Limited <http://www.monavacon.com/>
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

static char const ident[] = "src/util/strace.c (" PACKAGE_ENVR ") " PACKAGE_DATE;

/*
 *  SVR 4.2 Utility: strace - Prints STREAMS trace messages.
 */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif

#include <stddef.h>
#include <stdint.h>
#include <stropts.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#ifdef _GNU_SOURCE
#include <getopt.h>
#endif
#include <time.h>
#include <signal.h>
#include <syslog.h>
#include <sys/utsname.h>
#include <sys/strlog.h>

const char *program = "strace";
const char *loggername = "trace";

static int nomead = 0;			/* default foreground mode */
static int debug = 0;			/* default no debug */
static int output = 1;			/* default normal output */

int strlog_fd = -1;

char outfile[127] = "";
char errfile[127] = "";
char outpath[256] = "";
char errpath[256] = "";
char basname[64] = "";
char cfgfile[127] = "";
char outpdir[127] = "/var/log/streams";
char devname[127] = "";
char mailuid[127] = "";
char pidfile[127] = "";

/* search path for log devices */
static const char *logdevices[] = {
	"/dev/streams/clone/log",
	"/dev/streams/log",
	"/dev/strlog",
	NULL
};

#define MY_FACILITY(__pri)	(LOG_DAEMON|(__pri))

#define FLAG_ZEROPAD	(1<<0)	/* pad with zero */
#define FLAG_SIGN	(1<<1)	/* unsigned/signed long */
#define FLAG_PLUS	(1<<2)	/* show plus */
#define FLAG_SPACE	(1<<3)	/* space if plus */
#define FLAG_LEFT	(1<<4)	/* left justified */
#define FLAG_SPECIAL	(1<<5)	/* 0x */
#define FLAG_LARGE	(1<<6)	/* use 'ABCDEF' instead of 'abcdef' */

#define PROMOTE_TYPE		int
#define PROMOTE_SIZE		(sizeof(PROMOTE_TYPE))
#define PROMOTE_ALIGN(__x)	(((__x) + PROMOTE_SIZE - 1) & ~(PROMOTE_SIZE - 1))
#define PROMOTE_SIZEOF(__x)	((sizeof(__x) < PROMOTE_SIZE) ? PROMOTE_SIZE : sizeof(__x))
#define PROMOTE_ARGVAL(__type,__ptr) \
	(long long)({ \
		long long val; \
		if (sizeof(__type) < PROMOTE_SIZEOF(__type)) { \
			val = (long long)*((PROMOTE_TYPE *)(__ptr)); \
			__ptr = (typeof(__ptr))((char *)__ptr + PROMOTE_SIZE); \
		} else { \
			val = (long long)*((__type *)(__ptr)); \
			__ptr = (typeof(__ptr))((char *)__ptr + sizeof(__type)); \
		} \
		val; \
	})

/*
 *  This function prints a formatted number to a buffer.  It is largely
 *  taken from /usr/src/linux/lib/vsprintf.c
 */
static int
number(char *sbuf, const char *end, long long num, int base, int width, int decimal, int flags)
{
	char sign;
	int i, iszero = (num == 0);
	char *str;
	char tmp[66];

	str = sbuf;
	if (flags & FLAG_LEFT)
		flags &= ~FLAG_ZEROPAD;
	if (base < 2 || base > 16)
		goto done;
	sign = '\0';
	if (flags & FLAG_SIGN) {
		if (num < 0) {
			sign = '-';
			num = -num;
			width--;
		} else if (flags & FLAG_PLUS) {
			sign = '+';
			width--;
		} else if (flags & FLAG_SPACE) {
			sign = ' ';
			width--;
		}
	}
	if (flags & FLAG_SPECIAL) {
		if (!iszero) {
			switch (base) {
			case 16:
				width -= 2;	/* for 0x */
				break;
			case 8:
				width--;	/* for 0 */
				break;
			}
		}
	}
	i = 0;
	if (num == 0)
		tmp[i++] = '0';
	else {
		const char *hexdig = (flags & FLAG_LARGE) ? "0123456789ABCDEF" : "0123456789abcdef";

		while (num != 0) {
			lldiv_t result = lldiv(num, base);

			tmp[i++] = hexdig[result.rem];
			num = result.quot;
		}
	}
	if (i > decimal)
		decimal = i;
	width -= decimal;
	if (!(flags & (FLAG_ZEROPAD | FLAG_LEFT)))
		if (width > 0) {
			while (width-- > 0) {
				*str = ' ';
				if (++str >= end)
					goto done;
			}
		}
	if (sign != '\0') {
		*str = sign;
		if (++str >= end)
			goto done;
	}
	if (flags & FLAG_SPECIAL) {
		if (!iszero) {
			switch (base) {
			case 8:
				*str = '0';
				if (++str >= end)
					goto done;
				break;
			case 16:
				*str = '0';
				if (++str >= end)
					goto done;
				if (flags & FLAG_LARGE)
					*str = 'X';
				else
					*str = 'x';
				if (++str >= end)
					goto done;
				break;
			}
		}
	}
	if (!(flags & FLAG_LEFT)) {
		char pad = (flags & FLAG_ZEROPAD) ? '0' : ' ';

		if (width > 0) {
			while (width-- > 0) {
				*str = pad;
				if (++str >= end)
					goto done;
			}
		}
	}
	if (i < decimal) {
		while (i < decimal--) {
			*str = '0';
			if (++str >= end)
				goto done;
		}
	}
	if (i > 0) {
		while (i-- > 0) {
			*str = tmp[i];
			if (++str >= end)
				goto done;
		}
	}
	if (width > 0) {
		while (width-- > 0) {
			*str = ' ';
			if (++str >= end)
				goto done;
		}
	}
      done:
	return (str - sbuf);
}

/*
 *  This function does an snprintf from the buffer using a slight variation of
 *  the snprintf that is identical to that used inside the kernel.  There are
 *  some constructs that are not supported.  Also, the kernel passes a format
 *  string followed by the arguments on the next word aligned boundary, and this
 *  is what is necessary to format them.
 *
 *  This function is largely adapted from /usr/src/linux/lib/vsprintf.c
 */
static int
snprintf_text(char *sbuf, size_t slen, const char *buf, int len)
{
	const char *fmt, *args, *aend, *end;
	char *str;
	char type;
	size_t flen, plen;
	int flags = 0, width = -1, decimal = -1, base = 10;

	if (slen == 0)
		return (0);
	if (output > 2)
		fprintf(stderr, "format string and arguments contain %d bytes\n", len);
	fmt = buf;
	if (output > 2)
		fprintf(stderr, "format string at %p\n", fmt);
	flen = strnlen(fmt, len);
	if (output > 2)
		fprintf(stderr, "format string of length %zu\n", flen);
	plen = PROMOTE_ALIGN(flen + 1);
	if (output > 2)
		fprintf(stderr, "format string of promoted length %zu\n", plen);
	args = &buf[plen];
	if (output > 2)
		fprintf(stderr, "arguments start at %p\n", args);
	aend = buf + len;
	if (output > 2) {
		fprintf(stderr, "arguments end at %p\n", aend);
		fprintf(stderr, "arguments contain %zu bytes\n", (size_t) (aend - args));
	}
	str = sbuf;
	end = str + slen - 1;	/* room for null */
	for (; *fmt; ++fmt) {
		const char *pos;
		unsigned long long num = 0;

		if (*fmt != '%') {
			*str = *fmt;
			if (++str >= end)
				goto done;
			continue;
		}
		flags = 0;
		width = -1;
		decimal = -1;
		base = 10;
		pos = fmt;	/* remember position of % */
		/* process flags */
		for (++fmt;; ++fmt) {
			switch (*fmt) {
			case '-':
				flags |= FLAG_LEFT;
				continue;
			case '+':
				flags |= FLAG_PLUS;
				continue;
			case ' ':
				flags |= FLAG_SPACE;
				continue;
			case '#':
				flags |= FLAG_SPECIAL;
				continue;
			case '0':
				flags |= FLAG_ZEROPAD;
				continue;
			default:
				break;
			}
			break;
		}
		/* get field width */
		if (isdigit(*fmt))
			for (width = 0; isdigit(*fmt); width *= 10, width += (*fmt++ - '0')) ;
		else if (*fmt == '*') {
			++fmt;
			if (args + PROMOTE_SIZEOF(int) <= aend) {
				width = PROMOTE_ARGVAL(int, args);

				if (width < 0) {
					width = -width;
					flags |= FLAG_LEFT;
				}
			} else
				args = aend;
		}
		/* get the decimal precision */
		if (*fmt == '.') {
			++fmt;
			if (isdigit(*fmt))
				for (decimal = 0; isdigit(*fmt); decimal *= 10, decimal += (*fmt - '0')) ;
			else if (*fmt == '*') {
				if (args + PROMOTE_SIZEOF(int) <= aend) {
					decimal = PROMOTE_ARGVAL(int, args);

					if (decimal < 0)
						decimal = 0;
				} else
					args = aend;
			}
		}
		/* get conversion type */
		type = 'i';
		switch (*fmt) {
		case 'h':
			type = *fmt;
			++fmt;
			if (*fmt == 'h') {
				type = 'c';
				++fmt;
			}
			break;
		case 'l':
			type = *fmt;
			++fmt;
			if (*fmt == 'l') {
				type = 'L';
				++fmt;
			}
			break;
		case 'q':
		case 'L':
		case 'Z':
		case 'z':
		case 't':
			type = *fmt;
			++fmt;
			break;
		}
		switch (*fmt) {
		case 'c':
		{
			char c = ' ';

			if (!(flags & FLAG_LEFT))
				if (width > 0) {
					while (--width > 0) {
						*str = ' ';
						if (++str >= end)
							goto done;
					}
				}
			if (args + PROMOTE_SIZEOF(char) <= aend)
				 c = PROMOTE_ARGVAL(char, args);

			else
				args = aend;
			*str = c;
			if (++str >= end)
				goto done;
			if (width > 0) {
				while (--width > 0) {
					*str = ' ';
					if (++str >= end)
						goto done;
				}
			}
			continue;
		}
		case 's':
		{
			const char *s;
			int i;
			int slen = 0, splen = 0;

			s = args;
			if (args < aend) {
				slen = strlen(s);
				splen = PROMOTE_ALIGN(slen + 1);
			} else
				args = aend;
			if (args + splen <= aend)
				args += splen;
			else
				args = aend;
			if (slen > decimal)
				slen = decimal;
			if (!(flags & FLAG_LEFT))
				if (slen < width) {
					while (slen < width--) {
						*str = ' ';
						if (++str >= end)
							goto done;
					}
				}
			for (i = 0; i < slen; ++i, ++s) {
				*str = *s;
				if (++str >= end)
					goto done;
			}
			if (slen < width) {
				while (slen < width--) {
					*str = ' ';
					if (++str >= end)
						goto done;
				}
			}
			continue;
		}
		case '%':
			*str = '%';
			if (++str >= end)
				goto done;
			continue;
		case 'p':
		case 'o':
		case 'X':
		case 'x':
		case 'd':
		case 'i':
		case 'u':
			switch (*fmt) {
			case 'p':
				type = 'p';
				if (width == -1) {
					width = 2 * sizeof(void *);
					flags |= FLAG_ZEROPAD;
				}
				flags |= FLAG_SPECIAL;
				base = 16;
				break;
			case 'o':
				base = 8;
				break;
			case 'X':
				flags |= FLAG_LARGE;
				__attribute__((fallthrough));
			case 'x':
				base = 16;
				break;
			case 'd':
			case 'i':
				flags |= FLAG_SIGN;
			case 'u':
				break;
			}
			switch (type) {
			case 'c':	/* really 'hh' type */
				if (args + PROMOTE_SIZEOF(unsigned char) <= aend)
					 num = PROMOTE_ARGVAL(unsigned char, args);

				else
					args = aend;
				if (flags & FLAG_SIGN)
					num = (signed char) num;
				break;
			case 'h':
				if (args + PROMOTE_SIZEOF(unsigned short) <= aend)
					 num = PROMOTE_ARGVAL(unsigned short, args);

				else
					args = aend;
				if (flags & FLAG_SIGN)
					num = (signed short) num;
				break;
			case 'p':	/* really 'p' conversion */
				if (args + PROMOTE_SIZEOF(uintptr_t) <= aend)
					num = PROMOTE_ARGVAL(uintptr_t, args);
				else
					args = aend;
				flags &= ~FLAG_SIGN;
				break;
			case 'l':
				if (args + PROMOTE_SIZEOF(unsigned long) <= aend)
					 num = PROMOTE_ARGVAL(unsigned long, args);

				else
					args = aend;
				if (flags & FLAG_SIGN)
					num = (signed long) num;
				break;
			case 'q':
			case 'L':	/* really 'll' type */
				if (args + PROMOTE_SIZEOF(unsigned long long) <= aend)
					 num = PROMOTE_ARGVAL(unsigned long long, args);

				else
					args = aend;
				if (flags & FLAG_SIGN)
					num = (signed long long) num;
				break;
			case 'Z':
			case 'z':
				if (args + PROMOTE_SIZEOF(size_t) <= aend)
					 num = PROMOTE_ARGVAL(size_t, args);

				else
					args = aend;
				if (flags & FLAG_SIGN)
					num = (ssize_t) num;
				break;
			case 't':
				if (args + PROMOTE_SIZEOF(ptrdiff_t) <= aend)
					num = PROMOTE_ARGVAL(ptrdiff_t, args);
				else
					args = aend;
				break;
			default:
				if (args + PROMOTE_SIZEOF(unsigned int) <= aend)
					 num = PROMOTE_ARGVAL(unsigned int, args);

				else
					args = aend;
				if (flags & FLAG_SIGN)
					num = (signed int) num;
				break;
			}
			str += number(str, end, num, base, width, decimal, flags);
			if (str >= end)
				goto done;
			continue;
		case '\0':
		default:
			*str = '%';	/* put the original % back */
			if (++str >= end)
				goto done;
			/* put the bad format characters */
			for (; fmt > pos; pos++) {
				*str = *pos;
				if (++str >= end)
					goto done;
			}
			if (*fmt == '\0')
				break;
			continue;
		}
		break;
	}
      done:
	*str = '\0';
	return (str - sbuf);
}

int
strace_pstrlog(FILE *file, struct strbuf *ctrl, struct strbuf *data)
{
	char sbuf[LOGMSGSZ << 2];
	struct log_ctl lc;
	int len;

	if (!ctrl || !data || !ctrl->buf || !data->buf || ctrl->len < (int) sizeof(lc)) {
		errno = -EINVAL;
		return (-1);
	}
	memcpy(&lc, ctrl->buf, sizeof(lc));

	snprintf_text(sbuf, sizeof(sbuf), (char *) data->buf, data->len);
	len = fprintf(file, "%d", lc.seq_no);
	if (len != -1) {
		char fchar[] = "          ";
		char *fstr = fchar, *tp;
		time_t ltime = lc.ltime;
		char timebuf[26];

		ctime_r(&ltime, timebuf);
		for (tp = timebuf;; tp++) {
			if (*tp == '\n') {
				*tp = '\0';
				break;
			}
			if (*tp == '\0')
				break;
			if (tp > timebuf + sizeof(timebuf) - 1) {
				*tp = '\0';
				break;
			}
		}
		len += fprintf(file, " %s", timebuf);
		len += fprintf(file, " %lu", (unsigned long) lc.ttime);
		len += fprintf(file, " %3d", lc.level);
		if (lc.flags & SL_ERROR)
			*fstr++ = 'E';
		if (lc.flags & SL_FATAL)
			*fstr++ = 'F';
		if (lc.flags & SL_NOTIFY)
			*fstr++ = 'N';
		*fstr++ = '\0';
		len += fprintf(file, " %s", fchar);
		len += fprintf(file, " %d", lc.mid);
		len += fprintf(file, " %d", lc.sid);
		len += fprintf(file, " %s", sbuf);
		len += fprintf(file, "\n");
	}
	return (len);
}

static void
copying()
{
	if (!output && !debug)
		return;
	(void) fprintf(stdout, "\
--------------------------------------------------------------------------------\n\
%1$s\n\
--------------------------------------------------------------------------------\n\
Copyright (c) 2008-2019  Monavacon Limited <http://www.monavacon.com/>\n\
Copyright (c) 2001-2008  OpenSS7 Corporation <http://www.openss7.com/>\n\
Copyright (c) 1997-2001  Brian F. G. Bidulock <bidulock@openss7.org>\n\
\n\
All Rights Reserved.\n\
--------------------------------------------------------------------------------\n\
This program is free software: you can  redistribute it  and/or modify  it under\n\
the terms of the  GNU Affero  General  Public  License  as published by the Free\n\
Software Foundation, version 3 of the license.\n\
\n\
This program is distributed in the hope that it will  be useful, but WITHOUT ANY\n\
WARRANTY; without even  the implied warranty of MERCHANTABILITY or FITNESS FOR A\n\
PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.\n\
\n\
You should have received a copy of the  GNU Affero General Public License  along\n\
with this program.   If not, see <http://www.gnu.org/licenses/>, or write to the\n\
Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n\
--------------------------------------------------------------------------------\n\
U.S. GOVERNMENT RESTRICTED RIGHTS.  If you are licensing this Software on behalf\n\
of the U.S. Government (\"Government\"), the following provisions apply to you. If\n\
the Software is supplied by the Department of Defense (\"DoD\"), it is classified\n\
as \"Commercial  Computer  Software\"  under  paragraph  252.227-7014  of the  DoD\n\
Supplement  to the  Federal Acquisition Regulations  (\"DFARS\") (or any successor\n\
regulations) and the  Government  is acquiring  only the  license rights granted\n\
herein (the license rights customarily provided to non-Government users). If the\n\
Software is supplied to any unit or agency of the Government  other than DoD, it\n\
is  classified as  \"Restricted Computer Software\" and the Government's rights in\n\
the Software  are defined  in  paragraph 52.227-19  of the  Federal  Acquisition\n\
Regulations (\"FAR\")  (or any successor regulations) or, in the cases of NASA, in\n\
paragraph  18.52.227-86 of  the  NASA  Supplement  to the FAR (or any  successor\n\
regulations).\n\
--------------------------------------------------------------------------------\n\
Commercial  licensing  and  support of this  software is  available from OpenSS7\n\
Corporation at a fee.  See http://www.openss7.com/\n\
--------------------------------------------------------------------------------\n\
", ident);
}

static void
version()
{
	if (!output && !debug)
		return;
	(void) fprintf(stdout, "\
%1$s (OpenSS7 %2$s) %3$s (%4$s)\n\
Written by Brian Bidulock.\n\
\n\
Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2015, 2016, 2017, 2018, 2019  Monavacon Limited.\n\
Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008  OpenSS7 Corporation.\n\
Copyright (c) 1997, 1998, 1999, 2000, 2001  Brian F. G. Bidulock.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
\n\
Distributed by OpenSS7 under GNU Affero General Public License Version 3,\n\
with conditions, incorporated herein by reference.\n\
\n\
See `%1$s --copying' for copying permissions.\n\
", NAME, PACKAGE, VERSION, PACKAGE_ENVR " " PACKAGE_DATE);
}

static void
usage(char *argv[])
{
	if (!output && !debug)
		return;
	(void) fprintf(stderr, "\
Usage:\n\
    %1$s [options] [MODULE UNIT PRIORITY] ...\n\
    %1$s {-h|--help}\n\
    %1$s {-V|--version}\n\
    %1$s {-C|--copying}\n\
", argv[0]);
}

static void
help(char *argv[])
{
	if (!output && !debug)
		return;
	(void) fprintf(stdout, "\
Usage:\n\
    %1$s [options] [MODULE UNIT PRIORITY] ...\n\
    %1$s {-h|--help}\n\
    %1$s {-V|--version}\n\
    %1$s {-C|--copying}\n\
Arguments:\n\
    MODULE\n\
        the module ID of the module to trace, -1 for any module\n\
    UNIT\n\
	the unit ID (typically minor number) of the instance of the module\n\
	to trace, -1 for any unit\n\
    PRIORITY\n\
        the maximum priority of messages to display\n\
Options:\n\
    -a, --admin MAILID\n\
        specify a mail address for notifications, default: 'root'\n\
    -n, --noforeground\n\
        daemonize, do not run in the foreground, default: foreground\n\
    -d, --directory DIRECTORY\n\
        specify a directory for log files, default: '/var/log/streams'\n\
    -b, --basename BASENAME\n\
        file basename, default: '%3$s'\n\
    -o, --outfile OUTFILE\n\
        redirect output to OUTFILE, default: ${BASENAME}.mm-dd\n\
    -e, --errfile ERRFILE\n\
        redirect errors to ERRFILE, default: ${BASENAME}.errors\n\
    -p, --pidfile PIDFILE\n\
        when running as daemon, output pid to PIDFILE, default: /var/run/%2$s.pid\n\
    -l, --logdev LOGDEVICE\n\
        log device to open, default: '/dev/streams/clone/log'\n\
    -q, --quiet\n\
        suppress normal output (equivalent to --verbose=0)\n\
    -D, --debug [LEVEL]\n\
        increment or set debug LEVEL [default: 0]\n\
    -v, --verbose [LEVEL]\n\
        increment or set output verbosity LEVEL [default: 1]\n\
        this option may be repeated.\n\
    -h, --help, -?, --?\n\
        print this usage information and exit\n\
    -V, --version\n\
        print version and exit\n\
    -C, --copying\n\
        print copying permission and exit\n\
", argv[0], program, loggername);
}

/* events */
enum {
	STRLOG_NONE = 0,
	STRLOG_SUCCESS = 0,
	STRLOG_TIMEOUT,
	STRLOG_PCPROTO,
	STRLOG_PROTO,
	STRLOG_DATA,
	STRLOG_FAILURE = -1,
};

int
sig_register(int signum, RETSIGTYPE(*handler) (int))
{
	sigset_t mask;
	struct sigaction act;

	act.sa_handler = handler ? handler : SIG_DFL;
	act.sa_flags = handler ? SA_RESTART : 0;
	sigemptyset(&act.sa_mask);
	if (sigaction(signum, &act, NULL))
		return STRLOG_FAILURE;
	sigemptyset(&mask);
	sigaddset(&mask, signum);
	sigprocmask(handler ? SIG_UNBLOCK : SIG_BLOCK, &mask, NULL);
	return STRLOG_SUCCESS;
}

int alm_signal = 0;
int hup_signal = 0;
int trm_signal = 0;

RETSIGTYPE
alm_handler(int signum __attribute__((unused)))
{
	alm_signal = 1;
	return (RETSIGTYPE) (0);
}

int
alm_catch(void)
{
	return sig_register(SIGALRM, &alm_handler);
}

int
alm_block(void)
{
	return sig_register(SIGALRM, NULL);
}

int
alm_action(void)
{
	alm_signal = 0;
	return (0);
}

RETSIGTYPE
hup_handler(int signum __attribute__((unused)))
{
	hup_signal = 1;
	return (RETSIGTYPE) (0);
}

int
hup_catch(void)
{
	return sig_register(SIGHUP, &hup_handler);
}

int
hup_block(void)
{
	return sig_register(SIGHUP, NULL);
}

int
hup_action(void)
{
	hup_signal = 0;
	syslog(MY_FACILITY(LOG_WARNING), "Caught SIGHUP, reopening files.");
	if (output > 1)
		syslog(MY_FACILITY(LOG_NOTICE), "Reopening output file %s", outpath);
	if (outpath[0] != '\0') {
		fflush(stdout);
		fclose(stdout);
		if (freopen(outpath, "a", stdout) == NULL) {
			syslog(MY_FACILITY(LOG_ERR), "%m");
			syslog(MY_FACILITY(LOG_ERR), "Could not reopen stdout file %s", outpath);
		}
		// output_header(void);
	}
	if (output > 1)
		syslog(MY_FACILITY(LOG_NOTICE), "Reopening error file %s", errpath);
	if (errpath[0] != '\0') {
		fflush(stderr);
		fclose(stderr);
		if (freopen(errpath, "a", stderr) == NULL) {
			syslog(MY_FACILITY(LOG_ERR), "%m");
			syslog(MY_FACILITY(LOG_ERR), "Could not reopen stderr file %s", errpath);
		}
	}
	return (0);
}

RETSIGTYPE
trm_handler(int signum __attribute__((unused)))
{
	trm_signal = 1;
	return (RETSIGTYPE) (0);
}

int
trm_catch(void)
{
	return sig_register(SIGTERM, &trm_handler);
}

int
trm_block(void)
{
	return sig_register(SIGTERM, NULL);
}

void strlog_exit(int retval);

int
trm_action(void)
{
	trm_signal = 0;
	syslog(MY_FACILITY(LOG_WARNING), "%s: Caught SIGTERM, shutting down", program);
	strlog_exit(0);
	return (0);		/* should be no return */
}

void
sig_catch(void)
{
	alm_catch();
	hup_catch();
	trm_catch();
}

void
sig_block(void)
{
	alm_block();
	hup_block();
	trm_block();
}

int
start_timer(long duration)
{
	struct itimerval setting = {
		{0, 0},
		{duration / 1000, (duration % 1000) * 1000}
	};

	if (alm_catch())
		return STRLOG_FAILURE;
	if (setitimer(ITIMER_REAL, &setting, NULL))
		return STRLOG_FAILURE;
	alm_signal = 0;
	return STRLOG_SUCCESS;
}

void
strlog_exit(int retval)
{
	syslog(MY_FACILITY(LOG_NOTICE), "%s: Exiting %d", program, retval);
	fflush(stdout);
	fflush(stderr);
	sig_block();
	closelog();
	exit(retval);
}

void
strlog_enter(int argc __attribute__((unused)), char *argv[])
{
	if (nomead) {
		pid_t pid;

		if ((pid = fork()) < 0) {
			perror(argv[0]);
			exit(2);
		} else if (pid != 0) {
			/* parent exits */
			exit(0);
		}
		setsid();	/* become a session leader */
		/* fork once more for SVR4 */
		if ((pid = fork()) < 0) {
			perror(argv[0]);
			exit(2);
		} else if (pid != 0) {
			if (nomead || pidfile[0] != '\0') {
				FILE *pidf;

				/* initialize default filename */
				if (pidfile[0] == '\0')
					snprintf(pidfile, sizeof(pidfile), "/var/run/%s.pid", program);
				if (output > 1)
					syslog(MY_FACILITY(LOG_NOTICE), "%s: Writing daemon pid to file %s", program, pidfile);
				if ((pidf = fopen(pidfile, "w+"))) {
					fprintf(pidf, "%d", (int) pid);
					fflush(pidf);
					fclose(pidf);
				} else {
					syslog(MY_FACILITY(LOG_ERR), "%s: %m", program);
					syslog(MY_FACILITY(LOG_ERR), "%s: Could not write pid to file %s", program, pidfile);
					strlog_exit(2);
				}
			}
			/* parent exits */
			exit(0);
		}
		/* release current directory */
		if (chdir("/") < 0) {
			perror(argv[0]);
			exit(2);
		}
		umask(0);	/* clear file creation mask */
		/* rearrange file streams */
		fclose(stdin);
	}
	/* continue as foreground or background */
	openlog(NULL, LOG_CONS | LOG_NDELAY | (nomead ? 0 : LOG_PERROR), MY_FACILITY(0));
	if (basname[0] == '\0')
		snprintf(basname, sizeof(basname), "%s", loggername);
	if (nomead || outfile[0] != '\0') {
		struct tm tm;
		time_t curtime;

		time(&curtime);
		localtime_r(&curtime, &tm);
		/* initialize default filename */
		if (outfile[0] == '\0')
			snprintf(outpath, sizeof(outpath), "%s/%s.%02d-%02d", outpdir, basname, tm.tm_mon + 1, tm.tm_mday);
		else
			snprintf(outpath, sizeof(outpath), "%s/%s", outpdir, outfile);
		if (output > 1)
			syslog(MY_FACILITY(LOG_NOTICE), "%s: Redirecting stdout to file %s", program, outpath);
		fflush(stdout);
		if (freopen(outpath, "a", stdout) == NULL) {
			syslog(MY_FACILITY(LOG_ERR), "%s: %m", program);
			syslog(MY_FACILITY(LOG_ERR), "%s: Could not redirect stdout to %s", program, outpath);
			strlog_exit(2);
		}
	}
	if (nomead || errfile[0] != '\0') {
		/* initialize default filename */
		if (errfile[0] == '\0')
			snprintf(errpath, sizeof(errpath), "%s/%s.errors", outpdir, basname);
		else
			snprintf(errpath, sizeof(errpath), "%s/%s", outpdir, errfile);
		if (output > 1)
			syslog(MY_FACILITY(LOG_NOTICE), "%s: Redirecting stderr to file %s", program, errpath);
		fflush(stderr);
		if (freopen(errpath, "a", stderr) == NULL) {
			syslog(MY_FACILITY(LOG_ERR), "%s: %m", program);
			syslog(MY_FACILITY(LOG_ERR), "%s: Could not redirect stderr to %s", program, errpath);
			strlog_exit(2);
		}
	}
	sig_catch();
	// output_header();
	syslog(MY_FACILITY(LOG_NOTICE), "%s: Startup complete.", program);
}

void
strlog_open(int argc __attribute__((unused)), char *argv[], struct trace_ids *tids, size_t count)
{
	struct strioctl ioc;

	/* open log device */
	if (devname[0] == '\0') {
		const char **dev;

		/* search if not specified */
		if (debug)
			fprintf(stderr, "%s: searching for log device\n", argv[0]);
		for (dev = logdevices; (*dev); dev++) {
			if (debug)
				fprintf(stderr, "%s: trying device %s\n", argv[0], (*dev));
			if ((strlog_fd = open((*dev), O_RDWR)) != -1)
				break;
		}
		if ((*dev) == NULL) {
			perror(argv[0]);
			strlog_exit(1);
		}
	} else {
		if (debug)
			fprintf(stderr, "%s: opening specified device %s\n", argv[0], devname);
		if ((strlog_fd = open(devname, O_RDWR)) < 0) {
			perror(argv[0]);
			strlog_exit(1);
		}
	}
	if (debug)
		fprintf(stderr, "%s: success\n", argv[0]);
	/* setup log device for logging */
	ioc.ic_cmd = I_TRCLOG;
	ioc.ic_timout = 0;
	ioc.ic_len = count * sizeof(struct trace_ids);
	ioc.ic_dp = (char *) tids;
	if (debug)
		fprintf(stderr, "%s: performing ioctl on log device\n", argv[0]);
	if (ioctl(strlog_fd, I_STR, &ioc) < 0) {
		perror(argv[0]);
		strlog_exit(1);
	}
	if (debug)
		fprintf(stderr, "%s: success\n", argv[0]);
}

void
strlog_close(int argc __attribute__((unused)), char *argv[])
{
	if (close(strlog_fd) < 0)
		perror(argv[0]);
}

int
main(int argc, char *argv[])
{
	while (1) {
		int c, val;

#if defined _GNU_SOURCE
		int option_index = 0;
		/* *INDENT-OFF* */
		static struct option long_options[] = {
			{"admin",	required_argument,	NULL, 'a'},
			{"noforeground",no_argument,		NULL, 'n'},
			{"directory",	required_argument,	NULL, 'd'},
			{"basename",	required_argument,	NULL, 'b'},
			{"outfile",	required_argument,	NULL, 'o'},
			{"errfile",	required_argument,	NULL, 'e'},
			{"pidfile",	required_argument,	NULL, 'p'},
			{"logdev",	required_argument,	NULL, 'l'},
			{"quiet",	no_argument,		NULL, 'q'},
			{"debug",	optional_argument,	NULL, 'D'},
			{"verbose",	optional_argument,	NULL, 'v'},
			{"help",	no_argument,		NULL, 'h'},
			{"version",	no_argument,		NULL, 'V'},
			{"copying",	no_argument,		NULL, 'C'},
			{"?",		no_argument,		NULL, 'H'},
			{ 0, }
		};
		/* *INDENT-ON* */

		c = getopt_long_only(argc, argv, "a:d:nb:o:e:p:l:qD::v::hVC?W:", long_options, &option_index);
#else				/* defined _GNU_SOURCE */
		c = getopt(argc, argv, "a:d:nb:o:e:p:l:qDvhVC?");
#endif				/* defined _GNU_SOURCE */
		if (c == -1) {
			if (debug)
				fprintf(stderr, "%s: done options processing\n", argv[0]);
			break;
		}
		switch (c) {
		case 0:
			goto bad_usage;
		case 'n':	/* -n, --noforeground */
			if (debug)
				fprintf(stderr, "%s: suppressing deamon mode\n", argv[0]);
			nomead = 1;
			break;
		case 'a':	/* -a, --admin MAILID */
			if (debug)
				fprintf(stderr, "%s: setting mail id to %s\n", argv[0], optarg);
			strncpy(mailuid, optarg, sizeof(mailuid) - 1);
			break;
		case 'd':	/* -d, --directory DIRECTORY */
			strncpy(outpdir, optarg, sizeof(outpdir) - 1);
			break;
		case 'b':	/* -b, --basename BASNAME */
			strncpy(basname, optarg, sizeof(basname) - 1);
			break;
		case 'o':	/* -o, --outfile OUTFILE */
			strncpy(outfile, optarg, sizeof(outfile) - 1);
			break;
		case 'e':	/* -e, --errfile ERRFILE */
			strncpy(errfile, optarg, sizeof(errfile) - 1);
			break;
		case 'p':	/* -p, --pidfile PIDFILE */
			if (debug)
				fprintf(stderr, "%s: setting pid file to %s\n", argv[0], optarg);
			strncpy(pidfile, optarg, sizeof(pidfile) - 1);
			break;
		case 'l':	/* -l, --logdev DEVNAME */
			if (debug)
				fprintf(stderr, "%s: setting device name to %s\n", argv[0], optarg);
			strncpy(devname, optarg, sizeof(devname) - 1);
			break;
		case 'q':	/* -q, --quiet */
			if (debug)
				fprintf(stderr, "%s: suppressing normal output\n", argv[0]);
			debug = 0;
			output = 0;
			break;
		case 'D':	/* -D, --debug [level] */
			if (debug)
				fprintf(stderr, "%s: increasing debug verbosity\n", argv[0]);
			if (optarg == NULL) {
				debug++;
			} else {
				if ((val = strtol(optarg, NULL, 0)) < 0)
					goto bad_option;
				debug = val;
			}
			if (debug)
				nomead = 0;
			break;
		case 'v':	/* -v, --verbose [level] */
			if (debug)
				fprintf(stderr, "%s: increasing output verbosity\n", argv[0]);
			if (optarg == NULL) {
				output++;
				break;
			}
			if ((val = strtol(optarg, NULL, 0)) < 0)
				goto bad_option;
			output = val;
			break;
		case 'h':	/* -h, --help */
		case 'H':	/* -H, --? */
			if (debug)
				fprintf(stderr, "%s: printing help message\n", argv[0]);
			help(argv);
			exit(0);
		case 'V':	/* -V, --version */
			if (debug)
				fprintf(stderr, "%s: printing version message\n", argv[0]);
			version();
			exit(0);
		case 'C':	/* -C, --copying */
			if (debug)
				fprintf(stderr, "%s: printing copying message\n", argv[0]);
			copying();
			exit(0);
		case '?':
		default:
		      bad_option:
			optind--;
		      bad_nonopt:
			if (output || debug) {
				if (optind < argc) {
					fprintf(stderr, "%s: syntax error near '", argv[0]);
					while (optind < argc)
						fprintf(stderr, "%s ", argv[optind++]);
					fprintf(stderr, "'\n");
				} else {
					fprintf(stderr, "%s: missing option or argument", argv[0]);
					fprintf(stderr, "\n");
				}
				fflush(stderr);
			      bad_usage:
				usage(argv);
			}
			exit(2);
		}
	}
	if (debug) {
		fprintf(stderr, "%s: option index = %d\n", argv[0], optind);
		fprintf(stderr, "%s: option count = %d\n", argv[0], argc);
	}
	{
		int count = 0;
		struct trace_ids *tids = NULL;

		if (optind == argc) {
			count = 1;
			if ((tids = calloc(count, sizeof(struct trace_ids))) == NULL) {
				perror(argv[0]);
				exit(1);
			}
			tids->ti_mid = -1;
			tids->ti_sid = -1;
			tids->ti_level = -1;
			tids->ti_flags = -1;
		} else if (optind < argc && (argc - optind) % 3 == 0) {
			int i;

			count = (argc - optind) / 3;
			if (debug)
				fprintf(stderr, "%s: allocating %d trace id structures\n", argv[0],
					count);
			if ((tids = calloc(count, sizeof(struct trace_ids))) == NULL) {
				perror(argv[0]);
				exit(1);
			}
			for (i = 0; i < count; i++) {
				if (strncmp(argv[optind], "all", 4) == 0) {
					tids[i].ti_mid = -1;
					if (debug)
						fprintf(stderr, "%s: mid: all\n", argv[0]);
				} else {
					tids[i].ti_mid = strtol(argv[optind], NULL, 0);
					if (debug)
						fprintf(stderr, "%s: mid: %d\n", argv[0],
							(int) tids[i].ti_mid);
				}
				optind++;
				if (strncmp(argv[optind], "all", 4) == 0) {
					tids[i].ti_sid = -1;
					if (debug)
						fprintf(stderr, "%s: sid: all\n", argv[0]);
				} else {
					tids[i].ti_sid = strtol(argv[optind], NULL, 0);
					if (debug)
						fprintf(stderr, "%s: sid: %d\n", argv[0],
							(int) tids[i].ti_sid);
				}
				optind++;
				if (strncmp(argv[optind], "all", 4) == 0) {
					tids[i].ti_level = -1;
					if (debug)
						fprintf(stderr, "%s: lev: all\n", argv[0]);
				} else {
					tids[i].ti_level = strtol(argv[optind], NULL, 0);
					if (debug)
						fprintf(stderr, "%s: lev: %d\n", argv[0],
							(int) tids[i].ti_level);
				}
				optind++;
				tids[i].ti_flags = -1;
			}
		} else
			goto bad_nonopt;
		/* open log device */
		strlog_open(argc, argv, tids, count);
	}
	strlog_enter(argc, argv);
	for (;;) {
		struct pollfd pfd[] = {
			{strlog_fd, POLLIN | POLLPRI | POLLERR | POLLHUP, 0}
		};

		if (trm_signal)
			trm_action();
		if (hup_signal)
			hup_action();
		if (alm_signal)
			alm_action();
		if (output > 2)
			fprintf(stderr, "entering poll loop\n");
		switch (poll(pfd, 1, -1)) {
		case -1:
			if (output > 2)
				fprintf(stderr, "got -1 from poll\n");
			if (errno == EAGAIN || errno == EINTR || errno == ERESTART)
				continue;
			syslog(MY_FACILITY(LOG_ERR), "%s: %m", program);
			syslog(MY_FACILITY(LOG_ERR), "%s: poll error", program);
			strlog_exit(1);
			return STRLOG_FAILURE;
		case 0:
			if (output > 2)
				fprintf(stderr, "got 0 from poll\n");
			return STRLOG_NONE;
		case 1:
			if (output > 2)
				fprintf(stderr, "got 1 from poll\n");
			if (pfd[0].revents & (POLLIN | POLLPRI)) {
				int ret, flags = 0;
				char cbuf[1024];
				char dbuf[2048];
				struct strbuf ctl = { 1024, 1024, cbuf };
				struct strbuf dat = { 2048, 2048, dbuf };
				struct log_ctl *lc;

#if 0
				char sbuf[1024];
				char fchar[] = "          ";
				char *fstr = fchar;
#endif

				if (output > 2)
					fprintf(stderr, "got POLLIN|POLLPRI from poll\n");
				if ((ret = getmsg(strlog_fd, &ctl, &dat, &flags)) < 0) {
					perror(argv[0]);
					exit(1);
				}
				if (ret != 0) {
					errno = ERANGE;
					perror(argv[0]);
					exit(1);
				}
				lc = (struct log_ctl *) cbuf;
				if (ctl.len < (int) sizeof(*lc)) {
					if (output > 2)
						fprintf(stderr, "ctl.len = %d, skipping\n", ctl.len);
					continue;
				}
				if (dat.len <= 0) {
					if (output > 2)
						fprintf(stderr, "dat.len = %d, skipping\n", dat.len);
					continue;
				}
				if (!nomead || outfile[0] == '\0') {
#if 1
					strace_pstrlog(stdout, &ctl, &dat);
#else
					time_t ltime = lc->ltime;
					char tbuf[64];

					snprintf_text(sbuf, sizeof(sbuf), dbuf, dat.len);
					fprintf(stdout, "%d", lc->seq_no);
					snprintf(tbuf, sizeof(tbuf), ctime(&ltime));
					tbuf[strnlen(tbuf, sizeof(tbuf)) - 1] = '\0';
					fprintf(stdout, " %s", tbuf);
					fprintf(stdout, " %lu", (unsigned long) lc->ttime);
					fprintf(stdout, " %3d", lc->level);
					if (lc->flags & SL_ERROR)
						*fstr++ = 'E';
					if (lc->flags & SL_FATAL)
						*fstr++ = 'F';
					if (lc->flags & SL_NOTIFY)
						*fstr++ = 'N';
					*fstr++ = '\0';
					fprintf(stdout, " %3s", fchar);
					fprintf(stdout, " %d", lc->mid);
					fprintf(stdout, " %d", lc->sid);
					fprintf(stdout, " %s", sbuf);
					fprintf(stdout, "\n");
#endif
					fflush(stdout);
				}
			}
			if (pfd[0].revents & POLLNVAL) {
				if (output > 2)
					fprintf(stderr, "got POLLNVAL from poll\n");
				syslog(MY_FACILITY(LOG_ERR), "%s: device invalid", program);
				strlog_exit(1);
				return STRLOG_FAILURE;
			}
			if (pfd[0].revents & POLLHUP) {
				if (output > 2)
					fprintf(stderr, "got POLLHUP from poll\n");
				syslog(MY_FACILITY(LOG_ERR), "%s: device hangup", program);
				strlog_exit(1);
				return STRLOG_FAILURE;
			}
			if (pfd[0].revents & POLLERR) {
				if (output > 2)
					fprintf(stderr, "got POLLERR from poll\n");
				syslog(MY_FACILITY(LOG_ERR), "%s: device error", program);
				strlog_exit(1);
				return STRLOG_FAILURE;
			}
			break;
		default:
			if (output > 2)
				fprintf(stderr, "poll error\n");
			syslog(MY_FACILITY(LOG_ERR), "%s: poll error", program);
			strlog_exit(1);
			return STRLOG_FAILURE;
		}
	}
	strlog_close(argc, argv);
	exit(0);
}
