/*****************************************************************************

 @(#) File: src/util/strtune.c

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2015  Monavacon Limited <http://www.monavacon.com/>
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

static char const ident[] = "src/util/strtune.c (" PACKAGE_ENVR ") " PACKAGE_DATE;

/* 
 *  AIX Utility: strtune - Produces a list of module and driver names.
 */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif

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

enum {
	STRTUNE_NONE = 0,
	STRTUNE_GET = 1,
	STRTUNE_SET = 2,
};

long hiwat_val = 0;			/* hiwat value */
uint hiwat_cmd = 0;			/* hiwat command */
long lowat_val = 0;			/* lowat value */
uint lowat_cmd = 0;			/* lowat command */
long minpsz_val = 0;			/* minpsz value */
uint minpsz_cmd = 0;			/* minpsz command */
long maxpsz_val = 0;			/* maxpsz value */
uint maxpsz_cmd = 0;			/* maxpsz command */
long trclevel_val = 0;			/* trclevel value */
uint trclevel_cmd = 0;			/* trclevel command */

enum {
	TUNABLE_HIWAT = 0,
	TUNABLE_LOWAT = 1,
	TUNABLE_MINPSZ = 2,
	TUNABLE_MAXPSZ = 3,
	TUNABLE_TRCLEVEL = 4,
	TUNABLE_MAX = 5,
};

struct tunable_struct {
	const char *name;
	long value;
	int command;
};

static struct tunable_struct tunables[] = {
	/* *INDENT-OFF* */
	[TUNABLE_HIWAT]    = {.name = "hiwat",    .value = 0, .command = STRTUNE_NONE, },
	[TUNABLE_LOWAT]    = {.name = "lowat",    .value = 0, .command = STRTUNE_NONE, },
	[TUNABLE_MINPSZ]   = {.name = "minpsz",   .value = 0, .command = STRTUNE_NONE, },
	[TUNABLE_MAXPSZ]   = {.name = "maxpsz",   .value = 0, .command = STRTUNE_NONE, },
	[TUNABLE_TRCLEVEL] = {.name = "trclevel", .value = 0, .command = STRTUNE_NONE, },
	/* *INDENT-ON* */
};

enum {
	COMMAND_NONE = 0,
	COMMAND_NAME = 1,
	COMMAND_ADDR = 2,
	COMMAND_MODS = 3,
	COMMAND_QUES = 4,
	COMMAND_FILE = 5,
};

static int command = COMMAND_NONE;

static int do_trcl = 0;
static int do_tune = 0;
static int do_allm = 0;

#define SIDE_RD = 0x01
#define SIDE_WR = 0x02

static int side = 0;

#define POS_BOTH_MUXS  0
#define POS_LOWER_MUX  1

static int position = POS_BOTH_MUXS;

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
Copyright (c) 2008-2015  Monavacon Limited <http://www.monavacon.com/>\n\
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
Copyright (c) 2008, 2009, 2010, 2015  Monavacon Limited.\n\
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
    %1$s [options] {-n |--name=}NAME | {-q |--queue=}ADDR
         {-o |--option=}TUNABLE[=VALUE] {-o |--option=}TUNABLE[=VALUE] ...
    %1$s [options] [{-n |--name=}NAME | {-q |--queue=}ADDR [{-a|--all}]]
         {-o |--option=}TRCLEVEL[=VALUE]
    %1$s [{-M|--modules}]
    %1$s [{-Q|--queues}]
    %1$s [{-f |--file}=FILE]
    %1$s {-h|--help}\n\
    %1$s {-V|--version}\n\
    %1$s {-C|--copying}\n\
Arguments:\n\
    None.\n\
Options:\n\
    -n, --name NAME\n\
        STREAMS module or driver name\n\
    -q, --queue ADDR\n\
        modify only active Stream queue or pair\n\
    -a, --all\n\
        species all queue pairs in Stream or module\n\
    -r, --rd, --read\n\
        specification applies to read queues only\n\
    -w, --wr, --write\n\
        specification applies to write queues only\n\
    -m, --mux\n\
        specification applies to lower multiplex only\n\
    -M, --queues\n\
        list tunable values for each module\n\
    -Q, --queues\n\
        list tunable values for each active queue\n\
    -f, --file FILENAME\n\
    -s, --silent, --quiet\n\
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

int
main(int argc, char *argv[])
{
	int i, fd, count;
	struct sc_list *list;
	struct sc_tune tune;

	for (;;) {
		int c, val, i;
		char *ptr, *token;

#if defined _GNU_SOURCE
		int option_index = 0;
		/* *INDENT-OFF* */
		static struct option long_options[] = {
			{"name",	required_argument,	NULL, 'n'},
			{"queue",	required_argument,	NULL, 'q'},
			{"option",	required_argument,	NULL, 'o'},
			{"all",		no_argument,		NULL, 'a'},
			{"read",	no_argument,		NULL, 'r'},
			{"rd",		no_argument,		NULL, 'r'},
			{"write",	no_argument,		NULL, 'w'},
			{"wr",		no_argument,		NULL, 'w'},
			{"mux",		no_argmunet,		NULL, 'm'},
			{"modules",	no_argument,		NULL, 'M'},
			{"queues",	no_argument,		NULL, 'Q'},
			{"file",	required_argument,	NULL, 'f'},
			{"silent",	no_argument,		NULL, 's'},
			{"quiet",	no_argument,		NULL, 's'},
			{"debug",	optional_argument,	NULL, 'D'},
			{"verbose",	optional_argument,	NULL, 'v'},
			{"help",	no_argument,		NULL, 'h'},
			{"version",	no_argument,		NULL, 'V'},
			{"copying",	no_argument,		NULL, 'C'},
			{"?",		no_argument,		NULL, 'H'},
			{ 0, }
		};
		/* *INDENT-ON* */

		c = getopt_long_only(argc, argv, "n:q:o:aMQf:sD::v::hVC?W:", long_options,
				     &option_index);
#else				/* defined _GNU_SOURCE */
		c = getopt(argc, argv, "n:q:o:aMQf:sDvhVC?");
#endif				/* defined _GNU_SOURCE */
		if (c == -1) {
			if (debug)
				fprintf(stderr, "%s: done options processing\n", argv[0]);
			break;
		}
		switch (c) {
		case 0:
			goto bad_usage;
		case 'n':	/* -n, --name=NAME */
			if (command != COMMAND_NONE)
				goto bad_option;
			command = COMMAND_NAME;
			break;
		case 'q':	/* -q, --queue=ADDR */
			if (command != COMMAND_NONE)
				goto bad_option;
			command = COMMAND_ADDR;
			break;
		case 'o':	/* -o, --option=TUNABLE[=VALUE] */
			switch (command) {
			case COMMAND_NONE:
			case COMMAND_NAME:
			case COMMAND_ADDR:
				break;
			default:
				goto bad_opt;
			}
			if ((token = strtok_r(optarg, "=", &ptr)) == NULL)
				goto bad_option;
			for (i = 0; i < TUNABLE_MAX; i++) {
				if (strcmp(token, tunables[i].name) == 0) {
					if ((token = strtok_r(NULL, "", &ptr)) != NULL) {
						if ((tunables[i].value =
						     strtol(token, &ptr, 0)) == 0 && ptr == token)
							goto bad_option;
						tunables[i].command = STRTUNE_SET;
					} else
						tunables[i].command = STRTUNE_GET;
					break;
				}
			}
			if (i == TUNABLE_MAX)
				goto bad_option;
			if (i = TUNABLE_TRCLEVEL) {
				if (do_trcl != 0 || do_tune != 0)
					goto bad_option;
				do_trcl = 1;
			} else {
				if (dl_trcl != 0)
					goto bad_option;
				do_tune = 1;
			}
			break;
		case 'a':	/* -a, --all */
			switch (command) {
			case COMMAND_NONE:
			case COMMAND_NAME:
			case COMMAND_ADDR:
				break;
			default:
				goto bad_option;
			}
			if (do_tune != 0)
				goto bad_option;
			do_allm = 1;
			break;
		case 'r':	/* -r, --rd, --read */
			side |= SIDE_RD;
			break;
		case 'w':	/* -w, --wr, --write */
			side |= SIDE_WR;
			break;
		case 'm':	/* -m, --mux */
			position = POS_LOWER_MUX;
			break;
		case 'M':	/* -M, --modules */
			if (command != COMMAND_NONE || do_trcl || do_tune || do_allm)
				goto bad_option;
			command = COMMAND_MODS;
			break;
		case 'Q':	/* -Q, --queues */
			if (command != COMMAND_NONE || do_trcl || do_tune || do_allm)
				goto bad_option;
			command = COMMAND_QUES;
			break;
		case 'f':	/* -f, --file=FILE */
			if (command != COMMAND_NONE || do_trcl || do_tune || do_alm)
				goto bad_option;
			command = COMMAND_FILE;
			break;
		case 's':	/* -s, --silent or --quiet */
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
	if (optind < argc) {
		/* hey, why not just ignore non-option arguments... */
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

}
