/*****************************************************************************

 @(#) $RCSfile: scls.c,v $ $Name:  $($Revision: 1.1.2.1 $) $Date: 2009-06-21 11:47:54 $

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

 Last Modified $Date: 2009-06-21 11:47:54 $ by $Author: brian $

 -----------------------------------------------------------------------------

 $Log: scls.c,v $
 Revision 1.1.2.1  2009-06-21 11:47:54  brian
 - added files to new distro

 *****************************************************************************/

#ident "@(#) $RCSfile: scls.c,v $ $Name:  $($Revision: 1.1.2.1 $) $Date: 2009-06-21 11:47:54 $"

static char const ident[] = "$RCSfile: scls.c,v $ $Name:  $($Revision: 1.1.2.1 $) $Date: 2009-06-21 11:47:54 $";

/* 
 *  AIX Utility: scls - Produces a list of module and driver names.
 */

#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#ifdef _GNU_SOURCE
#include <getopt.h>
#endif

#include <stropts.h>
#include <sys/sc.h>

static int debug = 0;			/* default no debug */
static int output = 1;			/* default normal output */

static void
copying(int argc, char *argv[])
{
	if (!output && !debug)
		return;
	(void) fprintf(stdout, "\
--------------------------------------------------------------------------------\n\
%1$s\n\
--------------------------------------------------------------------------------\n\
Copyright (c) 2008-2009  Monavacon Limited <http://www.monavacon.com/>\n\
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
version(int argc, char *argv[])
{
	if (!output && !debug)
		return;
	(void) fprintf(stdout, "\
%1$s (OpenSS7 %2$s) %3$s (%4$s)\n\
Written by Brian Bidulock.\n\
\n\
Copyright (c) 2008, 2009  Monavacon Limited.\n\
Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008  OpenSS7 Corporation.\n\
Copyright (c) 1997, 1998, 1999, 2000, 2001  Brian F. G. Bidulock.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
\n\
Distributed by OpenSS7 under GNU Affero General Public License Version 3,\n\
with conditions, incorporated herein by reference.\n\
\n\
See `%1$s --copying' for copying permissions.\n\
", NAME, PACKAGE, VERSION, "$Revision: 1.1.2.1 $ $Date: 2009-06-21 11:47:54 $");
}

static void
usage(int argc, char *argv[])
{
	if (!output && !debug)
		return;
	(void) fprintf(stderr, "\
Usage:\n\
    %1$s [options] [{-c|--count}|{-l|--long}] [MODULE ...]\n\
    %1$s {-h|--help}\n\
    %1$s {-V|--version}\n\
    %1$s {-C|--copying}\n\
", argv[0]);
}

static void
help(int argc, char *argv[])
{
	if (!output && !debug)
		return;
	(void) fprintf(stdout, "\
Usage:\n\
    %1$s [options] [{-l|--long}] [{-c|--count}] [{-b|--both}] [MODULE ...]\n\
    %1$s {-h|--help}\n\
    %1$s {-V|--version}\n\
    %1$s {-C|--copying}\n\
Arguments:\n\
    MODULE ...\n\
	specific drivers and modules to list instead of all drivers and\n\
	modules\n\
Options:\n\
    -l, --long\n\
        print module_info only\n\
    -c, --count\n\
        print module_stat only\n\
    -b, --both\n\
        print module_info and module_stat only\n\
    -a, --all\n\
        print info or stats for all queues\n\
    -r, --reset\n\
        reset all statistics upon collection\n\
    -q, --quiet\n\
        suppress normal output (equivalent to --verbose=0)\n\
    -d, --debug [LEVEL]\n\
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
", argv[0]);
}

enum { CMN_NONE, CMN_NAMES, CMN_LONG, CMN_COUNT, CMN_BOTH, } command = CMN_NONE;
int option_all = 0;
int option_reset = 0;

void
printit(struct sc_mlist *l, int cmd, int all)
{
	int i;

	if (output <= 0 || l->major == -1)
		return;
	for (i = 0; i < 4; i++) {
		if (i > 0) {
			if (all == 0)
				continue;
			if (l->mi[i].index == 0 && l->ms[i].index == 0)
				continue;
			if (l->mi[i].index == 0 && cmd == CMN_LONG)
				continue;
			if (l->ms[i].index == 0 && cmd == CMN_COUNT)
				continue;
			if (cmd != CMN_LONG && cmd != CMN_BOTH && cmd != CMN_COUNT)
				continue;
		}
		if (i == 0)
			fprintf(stdout, "%-9s", l->name);
		else
			fprintf(stdout, "         ");
		switch (cmd) {
		case CMN_LONG:
			if (i == 0) {
				if (l->major != 0) {
					fprintf(stdout, " driver");
					fprintf(stdout, " %4ld", (long) l->major);
				} else {
					fprintf(stdout, " module");
					fprintf(stdout, "     ");
				}
			} else {
				fprintf(stdout, "            ");
			}
			if (l->mi[i].index != 0) {
				fprintf(stdout, " %5u", l->mi[i].mi_idnum);
				fprintf(stdout, " %8ld", (long) l->mi[i].mi_minpsz);
				fprintf(stdout, " %8ld", (long) l->mi[i].mi_maxpsz);
				fprintf(stdout, " %8ld", (long) l->mi[i].mi_hiwat);
				fprintf(stdout, " %8ld", (long) l->mi[i].mi_lowat);
				if (all) {
					fprintf(stdout, " :");
					if (l->mi[i].index & 0x8)
						fprintf(stdout, " wr");
					if (l->mi[i].index & 0x4)
						fprintf(stdout, " rd");
					if (l->mi[i].index & 0x2)
						fprintf(stdout, " muxw");
					if (l->mi[i].index & 0x1)
						fprintf(stdout, " muxr");
				}
			}
			break;
		case CMN_COUNT:
			if (i == 0 || l->ms[i].index != 0) {
				if (l->ms[i].index != 0) {
					fprintf(stdout, " %8ld", (long) l->ms[i].ms_pcnt);
					fprintf(stdout, " %8ld", (long) l->ms[i].ms_scnt);
					fprintf(stdout, " %8ld", (long) l->ms[i].ms_ocnt);
					fprintf(stdout, " %8ld", (long) l->ms[i].ms_ccnt);
					fprintf(stdout, " %8ld", (long) l->ms[i].ms_acnt);
					fprintf(stdout, " %8x", l->ms[i].ms_flags);
				} else {
					fprintf(stdout, "         ");
					fprintf(stdout, "         ");
					fprintf(stdout, "         ");
					fprintf(stdout, "         ");
					fprintf(stdout, "         ");
					fprintf(stdout, "         ");
					l->ms[i].index = 0xf;
				}
				if (all) {
					fprintf(stdout, " :");
					if (l->ms[i].index & 0x8)
						fprintf(stdout, " wr");
					if (l->ms[i].index & 0x4)
						fprintf(stdout, " rd");
					if (l->ms[i].index & 0x2)
						fprintf(stdout, " muxw");
					if (l->ms[i].index & 0x1)
						fprintf(stdout, " muxr");
				}
			}
			break;
		case CMN_BOTH:
			if (i == 0) {
				if (l->major != 0) {
					fprintf(stdout, " driver");
					fprintf(stdout, " %4ld", (long) l->major);
				} else {
					fprintf(stdout, " module");
					fprintf(stdout, "     ");
				}
			} else {
				fprintf(stdout, "            ");
			}
			if (l->mi[i].index != 0) {
				fprintf(stdout, " %5u", l->mi[i].mi_idnum);
				fprintf(stdout, " %8ld", (long) l->mi[i].mi_minpsz);
				fprintf(stdout, " %8ld", (long) l->mi[i].mi_maxpsz);
				fprintf(stdout, " %8ld", (long) l->mi[i].mi_hiwat);
				fprintf(stdout, " %8ld", (long) l->mi[i].mi_lowat);
			} else {
				fprintf(stdout, "      ");
				fprintf(stdout, "         ");
				fprintf(stdout, "         ");
				fprintf(stdout, "         ");
				fprintf(stdout, "         ");
				l->mi[i].index = 0xf;
			}
			if (l->ms[i].index != 0) {
				fprintf(stdout, " %8ld", (long) l->ms[i].ms_pcnt);
				fprintf(stdout, " %8ld", (long) l->ms[i].ms_scnt);
				fprintf(stdout, " %8ld", (long) l->ms[i].ms_ocnt);
				fprintf(stdout, " %8ld", (long) l->ms[i].ms_ccnt);
				fprintf(stdout, " %8ld", (long) l->ms[i].ms_acnt);
				fprintf(stdout, " %8x", l->ms[i].ms_flags);
			} else {
				fprintf(stdout, "         ");
				fprintf(stdout, "         ");
				fprintf(stdout, "         ");
				fprintf(stdout, "         ");
				fprintf(stdout, "         ");
				fprintf(stdout, "         ");
				l->ms[i].index = 0xf;
			}
			if (all) {
				fprintf(stdout, " :");
				if (l->mi[i].index & l->ms[i].index & 0x8)
					fprintf(stdout, " wr");
				if (l->mi[i].index & l->ms[i].index & 0x4)
					fprintf(stdout, " rd");
				if (l->mi[i].index & l->ms[i].index & 0x2)
					fprintf(stdout, " muxw");
				if (l->mi[i].index & l->ms[i].index & 0x1)
					fprintf(stdout, " muxr");
			}
			break;
		}
		fprintf(stdout, "\n");
	}
//      if (cmd == CMN_LONG || cmd == CMN_BOTH || cmd == CMN_COUNT)
//              fprintf(stdout, "\n");
};

int
main(int argc, char *argv[])
{
	int i, fd, count;
	struct sc_list *list;

	for (;;) {
		int c, val;

#if defined _GNU_SOURCE
		int option_index = 0;
		/* *INDENT-OFF* */
		static struct option long_options[] = {
			{"long",	no_argument,		NULL, 'l'},
			{"count",	no_argument,		NULL, 'c'},
			{"both",	no_argument,		NULL, 'b'},
			{"all",		no_argument,		NULL, 'a'},
			{"reset",	no_argument,		NULL, 'r'},
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

		c = getopt_long_only(argc, argv, "lcbarqD::v::hVC?W:", long_options, &option_index);
#else				/* defined _GNU_SOURCE */
		c = getopt(argc, argv, "lcbarqDvhVC?");
#endif				/* defined _GNU_SOURCE */
		if (c == -1) {
			if (debug)
				fprintf(stderr, "%s: done options processing\n", argv[0]);
			break;
		}
		switch (c) {
		case 0:
			goto bad_usage;
		case 'a':
			if (debug)
				fprintf(stderr, "%s: setting option all\n", argv[0]);
			option_all = 1;
			break;
		case 'r':
			if (debug)
				fprintf(stderr, "%s: setting option reset\n", argv[0]);
			option_reset = 1;
			break;
		case 'b':
			if (command != CMN_NONE && command != CMN_LONG && command != CMN_COUNT)
				goto bad_option;
			if (debug)
				fprintf(stderr, "%s: setting both command\n", argv[0]);
			command = CMN_BOTH;
			break;
		case 'l':
			if (command != CMN_NONE)
				goto bad_option;
			if (debug)
				fprintf(stderr, "%s: setting long command\n", argv[0]);
			command = CMN_LONG;
			break;
		case 'c':
			if (command != CMN_NONE && command != CMN_LONG)
				goto bad_option;
			if (debug)
				fprintf(stderr, "%s: setting count command\n", argv[0]);
			if (command == CMN_LONG)
				command = CMN_BOTH;
			else
				command = CMN_COUNT;
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
			break;
		case 'q':	/* -q, --quiet */
			if (debug)
				fprintf(stderr, "%s: suppressing normal output\n", argv[0]);
			debug = 0;
			output = 0;
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
			help(argc, argv);
			exit(0);
		case 'V':	/* -V, --version */
			if (debug)
				fprintf(stderr, "%s: printing version message\n", argv[0]);
			version(argc, argv);
			exit(0);
		case 'C':	/* -C, --copying */
			if (debug)
				fprintf(stderr, "%s: printing copying message\n", argv[0]);
			copying(argc, argv);
			exit(0);
		case '?':
		default:
		      bad_option:
			optind--;
			goto bad_nonopt;
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
				usage(argc, argv);
			}
			exit(2);
		}
	}
	if (debug)
		fprintf(stderr, "%s: opening /dev/nuls\n", argv[0]);
	if ((fd = open("/dev/nuls", O_RDWR)) < 0) {
		if (debug)
			fprintf(stderr, "%s: could not open /dev/nuls\n", argv[0]);
		perror(argv[0]);
		if (debug)
			fprintf(stderr, "%s: opening /dev/streams/clone/nuls\n", argv[0]);
		if ((fd = open("/dev/streams/clone/nuls", O_RDWR)) < 0) {
			if (debug)
				fprintf(stderr, "%s: could not open /dev/streams/clone/nuls\n",
					argv[0]);
			perror(argv[0]);
			exit(1);
		}
	}
	if (debug)
		fprintf(stderr, "%s: pushing sc module\n", argv[0]);
	if (ioctl(fd, I_PUSH, "sc") < 0) {
		if (debug)
			fprintf(stderr, "%s: could not push sc module\n", argv[0]);
		perror(argv[0]);
		exit(1);
	}
	if (debug)
		fprintf(stderr, "%s: getting size of list\n", argv[0]);
	/* go out and get all the names anyway */
	if ((count = ioctl(fd, SC_IOC_LIST, NULL)) < 0) {
		if (debug)
			fprintf(stderr, "%s: could not perform SC_IOC_LIST command\n", argv[0]);
		perror(argv[0]);
		exit(1);
	}
	if (debug)
		fprintf(stderr, "%s: size of list = %d\n", argv[0], count);
	/* get entire list */
	if ((list = malloc(sizeof(struct sc_list) + count * sizeof(struct sc_mlist))) == NULL) {
		if (debug)
			fprintf(stderr, "%s: could not allocate memory\n", argv[0]);
		fprintf(stderr, "%s: %s\n", argv[0], strerror(ENOMEM));
		fflush(stderr);
		exit(1);
	}
	list->sc_nmods = count;
	list->sc_mlist = (struct sc_mlist *) (list + 1);
	if (option_reset) {
		if (ioctl(fd, SC_IOC_RESET, list) < 0) {
			if (debug)
				fprintf(stderr,
					"%s: could not perform second SC_IOC_RESET command\n",
					argv[0]);
			perror(argv[0]);
			exit(1);
		}
	} else {
		if (ioctl(fd, SC_IOC_LIST, list) < 0) {
			if (debug)
				fprintf(stderr,
					"%s: could not perform second SC_IOC_LIST command\n",
					argv[0]);
			perror(argv[0]);
			exit(1);
		}
	}
	if (optind < argc) {
		int ret = 0;

		/* have module name arguments - iterate through them */
		for (; optind < argc; optind++) {
			for (i = 0; i < count; i++)
				if (strncmp(argv[optind], list->sc_mlist[i].name, FMNAMESZ)
				    == 0)
					break;
			if (i < count)
				printit(&list->sc_mlist[i], command, option_all);
			else {
				fprintf(stderr, "%s: %s: %s\n", argv[0], argv[optind],
					strerror(ENXIO));
				fflush(stderr);
				ret = 1;
			}
		}
		exit(ret);
	} else {
		/* have no module name arguments - operate on all */
		for (i = 0; i < count; i++)
			printit(&list->sc_mlist[i], command, option_all);
	}
	if (ioctl(fd, I_POP, NULL) < 0) {
		perror(argv[0]);
	}
	if (close(fd) < 0) {
		perror(argv[0]);
	}
	exit(0);
}
