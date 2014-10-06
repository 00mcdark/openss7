/*****************************************************************************

 @(#) $RCSfile: x400config.h,v $ $Name:  $($Revision: 1.1.2.3 $) $Date: 2010-11-28 14:22:39 $

 -----------------------------------------------------------------------------

 Copyright (c) 2008-2010  Monavacon Limited <http://www.monavacon.com/>
 Copyright (c) 2001-2008  OpenSS7 Corporation <http://www.openss7.com/>
 Copyright (c) 1997-2000  Brian F. G. Bidulock <bidulock@openss7.org>

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

 Last Modified $Date: 2010-11-28 14:22:39 $ by $Author: brian $

 -----------------------------------------------------------------------------

 $Log: x400config.h,v $
 Revision 1.1.2.3  2010-11-28 14:22:39  brian
 - remove #ident, protect _XOPEN_SOURCE

 Revision 1.1.2.2  2009-06-21 12:41:53  brian
 - updated headers

 Revision 1.1.2.1  2009-06-21 11:47:56  brian
 - added files to new distro

 Revision 0.9.2.2  2008-04-29 07:11:33  brian
 - updating headers for release

 Revision 0.9.2.1  2007/08/18 03:53:15  brian
 - working up configuration files

 *****************************************************************************/

#ifndef __LOCAL_X400CONFIG_H__
#define __LOCAL_X400CONFIG_H__

#ifndef YYSTYPE
#define YYSTYPE int
#endif

extern YYSTYPE x400config_yylval;

#endif				/* __LOCAL_X400CONFIG_H__ */
