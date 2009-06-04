/* ***************************************************************************

 @(#) $RCSfile$ $Name$($Revision$) $Date$

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

 Last Modified $Date$ by $Author$

 -----------------------------------------------------------------------------

 $Log$
 *****************************************************************************/

package javax.jcat;

import javax.csapi.cc.jcc.*;

/**
  * This interface is an extension of the JccConnectionListener interface and reports
  * state changes both of the JccCall and its JcatConnections. <p>
  *
  * The methods provided on this interface are on account of the fact that the states
  * defined in the JcatConnection interface provide more detail to the states defined in
  * the JccConnection interface. As a result each state in the JccConnection interface
  * corresponds to a state defined in the JcatConnection interface. Conversely, each
  * JcatConnection state corresponds to exactly one JccConnection state. This arrangement
  * permits applications to view either the JccConnection state or the JcatConnection
  * state and still see a consistent view. <p>
  *
  * Additionally, note that the JccConnectionEvent.CONNECTION_CONNECTED state now also
  * allows a transition to JcatConnectionEvent.CONNECTION_SUSPENDED state and back.
  *
  * @since 1.0
  * @author Monavacon Limited
  * @version 1.2.2
  */
public interface JcatConnectionListener extends JccConnectionListener {
    /**
      * Indicates that the JccConnection has just been placed in the
      * JcatConnection.SUSPENDED state
      *
      * @param connectionevent
      * Event resulting from state change.
      *
      * @since 1.0
      */
    public void connectionSuspended(JcatConnectionEvent connectionevent);
}

// vim: sw=4 et tw=90 com=srO\:/**,mb\:*,ex\:*/,srO\:/*,mb\:*,ex\:*/,b\:TRANS,\://,b\:#,\:%,\:XCOMM,n\:>,fb\:-

