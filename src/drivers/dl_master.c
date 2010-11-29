/*****************************************************************************

 @(#) $RCSfile: dl_master.c,v $ $Name:  $($Revision: 1.1.2.2 $) $Date: 2010-11-28 14:21:31 $

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2010  Monavacon Limited <http://www.monavacon.com/>
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

 Last Modified $Date: 2010-11-28 14:21:31 $ by $Author: brian $

 -----------------------------------------------------------------------------

 $Log: dl_master.c,v $
 Revision 1.1.2.2  2010-11-28 14:21:31  brian
 - remove #ident, protect _XOPEN_SOURCE

 Revision 1.1.2.1  2009-06-21 11:20:45  brian
 - added files to new distro

 *****************************************************************************/

static char const ident[] = "$RCSfile: dl_master.c,v $ $Name:  $($Revision: 1.1.2.2 $) $Date: 2010-11-28 14:21:31 $";

#include <sys/os7/compat.h>
#include <linux/kmod.h>
/*
 *  It is not necessary to use this module for Linux Fast-STREAMS.  Linux
 *  Fast-STREAMS has the Named STREAMS Device which, among other mechanisms,
 *  obviates the need for this driver.
 */

#define DL_DESCRIP	"Data Link (DL) STREAMS MULTIPLEXING DRIVER ($Revision: 1.1.2.2 $)"
#define DL_REVISION	"OpenSS7 $RCSfile: dl_master.c,v $ $Name:  $($Revision: 1.1.2.2 $) $Date: 2010-11-28 14:21:31 $"
#define DL_COPYRIGHT	"Copyright (c) 2008-2010  Monavacon Limited.  All Rights Reserved."
#define DL_DEVICE	"OpenSS7 CDI Devices."
#define DL_CONTACT	"Brian Bidulock <bidulock@openss7.org>"
#define DL_LICENSE	"GPL"
#define DL_BANNER	DL_DESCRIP	"\n" \
			DL_REVISION	"\n" \
			DL_COPYRIGHT	"\n" \
			DL_DEVICE	"\n" \
			DL_CONTACT
#define DL_SPLASH	DL_DESCRIP	"\n" \
			DL_REVISION

#ifdef LINUX
MODULE_AUTHOR(DL_CONTACT);
MODULE_DESCRIPTION(DL_DESCRIP);
MODULE_SUPPORTED_DEVICE(DL_DEVICE);
#ifdef MODULE_LICENSE
MODULE_LICENSE(DL_LICENSE);
#endif				/* MODULE_LICENSE */
#if defined MODULE_ALIAS
MODULE_ALIAS("streams-dl");
#endif
#ifdef MODULE_VERSION
MODULE_VERSION(__stringify(PACKAGE_RPMEPOCH) ":" PACKAGE_VERSION "." PACKAGE_RELEASE
	       PACKAGE_PATCHLEVEL "-" PACKAGE_RPMRELEASE PACKAGE_RPMEXTRA2);
#endif
#endif				/* LINUX */

#define DL_DRV_ID	CONFIG_STREAMS_DL_MODID
#define DL_DRV_NAME	CONFIG_STREAMS_DL_NAME
#define DL_CMAJORS	CONFIG_STREAMS_DL_NMAJORS
#define DL_CMAJOR_0	CONFIG_STREAMS_DL_MAJOR
#define DL_UNITS	CONFIG_STREAMS_DL_NMINORS

/*
 *  =========================================================================
 *
 *  STREAMS Definitions
 *
 *  =========================================================================
 */

#define DRV_ID		DL_DRV_ID
#define DRV_NAME	DL_DRV_NAME
#define CMAJORS		DL_CMAJORS
#define CMAJOR_0	DL_CMAJOR_0
#define UNITS		DL_UNITS
#ifdef MODULE
#define DRV_BANNER	DL_BANNER
#else				/* MODULE */
#define DRV_BANNER	DL_SPLASH
#endif				/* MODULE */

STATIC struct module_info dl_xinfo = {
	mi_idnum:DRV_ID,		/* Module ID number */
	mi_idname:DRV_NAME,		/* Module ID name */
	mi_minpsz:(0),			/* Min packet size accepted */
	mi_maxpsz:INFPSZ,		/* Max packet size accepted */
	mi_hiwat:(1),			/* Hi water mark */
	mi_lowat:(0),			/* Lo water mark */
};

STATIC streamscall int dl_open(queue_t *, dev_t *, int, int, cred_t *);
STATIC streamscall int dl_close(queue_t *, int, cred_t *);

STATIC struct qinit dl_xinit = {
	qi_qopen:dl_open,		/* Each open */
	qi_qclose:dl_close,		/* Last close */
	qi_minfo:&dl_xinfo,		/* Information */
};

MODULE_STATIC struct streamtab dlinfo = {
	st_rdinit:&dl_xinit,		/* Upper read queue */
	st_wrinit:&dl_xinit,		/* Upper write queue */
};

/*
 *  =========================================================================
 *
 *  OPEN AND CLOSE
 *
 *  =========================================================================
 */
STATIC int dl_majors[DL_CMAJORS] = {
	DL_CMAJOR_0,
};
STATIC char drvname[FMNAMESZ + 9];
STATIC const char *dl_modules[256] = {
	"dl",
	"dl-dua",
	"dl-gr303",
	"dl-gr303ua",
	"dl-gsma",
	"dl-h225",
	"dl-iua",
	"dl-lapb",
	"dl-lapd",
	"dl-lapf",
	"dl-v52",
	"dl-v5ua",
};

/*
 *  =========================================================================
 *
 *  OPEN AND CLOSE
 *
 *  =========================================================================
 */
STATIC int
dl_find_strdev(const char *devname)
{
	if (__cdev_search(devname) != NULL)
		return (1);
	return (-1);
}

/*
 *  Open
 *  -------------------------------------------------------------------------
 *  This open function automatically loads the appropriate kernel modules and
 *  calls the appropriate driver for the minor device number opened.  Note
 *  that we do an lis_setq and call the actual driver's open function, so we
 *  should never return here.
 */
STATIC streamscall int
dl_open(queue_t *q, dev_t *devp, int flag, int sflag, cred_t *crp)
{
	uchar cmajor;
	uchar cminor = getminor(*devp);
	const char *devname;
	struct streamtab *stab = NULL;
	int err;
	struct cdevsw *cdev;

	if (cminor == 0)
		return (ENOENT);	/* would loop */
	if ((cmajor = dl_majors[cminor])) {
		if ((cdev = __cdev_lookup(dl_majors[cminor])))
			stab = cdev->d_str;
		if (!stab)
			cmajor = dl_majors[cminor] = 0;
		else if (strncmp(dl_modules[cminor], cdev->d_name, FMNAMESZ) != 0) {
			/* 
			   name changed */
			stab = NULL;
			cmajor = dl_majors[cminor] = 0;
		}
	}
	if (!cmajor) {
		if (!(devname = dl_modules[cminor]))
			return (ENOENT);
		if ((err = dl_find_strdev(devname)) < 0) {
#ifdef CONFIG_KMOD
			sprintf(drvname, "streams-%s", devname);
			if ((err = request_module(drvname)))
				return (err < 0 ? -err : err);
			if ((err = dl_find_strdev(devname)) < 0)
				return (ENOENT);
#else
			return (ENOENT);
#endif
		}
		cmajor = err;
	}
	if (!(cdev = __cdev_lookup(cmajor)))
		return (ENOENT);
	setq(q, stab->st_rdinit, stab->st_wrinit);
	if (!q->q_qinfo->qi_qopen) {
		swerr();
		return (EIO);
	}
	return (*q->q_qinfo->qi_qopen) (q, devp, flag, CLONEOPEN, crp);
}

/*
 *  Close
 *  -------------------------------------------------------------------------
 */
STATIC streamscall int
dl_close(queue_t *q, int flag, cred_t *crp)
{
	(void) q;
	(void) flag;
	(void) crp;
	swerr();
	return (EIO);
}

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
MODULE_PARM_DESC(modid, "Module ID for the DL driver. (0 for allocation.)");

major_t major = CMAJOR_0;

#ifndef module_param
MODULE_PARM(major, "h");
#else
module_param(major, uint, 0444);
#endif
MODULE_PARM_DESC(major, "Device number for the DL driver. (0 for allocation.)");

/*
 *  Linux Fast-STREAMS Registration
 *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

STATIC struct cdevsw dl_cdev = {
	.d_name = DRV_NAME,
	.d_str = &dlinfo,
	.d_flag = D_MP,
	.d_mode = S_IFCHR,
	.d_fop = NULL,
	.d_kmod = THIS_MODULE,
};

STATIC int
dl_register_strdev(major_t major)
{
	int err;

	if ((err = register_strdev(&dl_cdev, major)) < 0)
		return (err);
	return (0);
}

STATIC int
dl_unregister_strdev(major_t major)
{
	int err;

	if ((err = unregister_strdev(&dl_cdev, major)) < 0)
		return (err);
	return (0);
}

MODULE_STATIC void __exit
dlterminate(void)
{
	int err, mindex;

	for (mindex = CMAJORS - 1; mindex >= 0; mindex--) {
		if (dl_majors[mindex]) {
			if ((err = dl_unregister_strdev(dl_majors[mindex])))
				cmn_err(CE_PANIC, "%s: cannot unregister major %d", DRV_NAME,
					dl_majors[mindex]);
			if (mindex)
				dl_majors[mindex] = 0;
		}
	}
	return;
}

MODULE_STATIC int __init
dlinit(void)
{
	int err, mindex = 0;

	cmn_err(CE_NOTE, DRV_BANNER);	/* console splash */
	for (mindex = 0; mindex < CMAJORS; mindex++) {
		if ((err = dl_register_strdev(dl_majors[mindex])) < 0) {
			if (mindex) {
				cmn_err(CE_WARN, "%s: could not register major %d", DRV_NAME,
					dl_majors[mindex]);
				continue;
			} else {
				cmn_err(CE_WARN, "%s: could not register driver, err = %d",
					DRV_NAME, err);
				dlterminate();
				return (err);
			}
		}
		if (dl_majors[mindex] == 0)
			dl_majors[mindex] = err;
#if 0
		LIS_DEVFLAGS(dl_majors[mindex]) |= LIS_MODFLG_CLONE;
#endif
		if (major == 0)
			major = dl_majors[0];
	}
	return (0);
}

/*
 *  Linux Kernel Module Initialization
 *  -------------------------------------------------------------------------
 */
module_init(dlinit);
module_exit(dlterminate);

#endif				/* LINUX */
