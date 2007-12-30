/***************************************************************************

  CCorbaApplication.h

  CORBA DII routines

  (c) 2005 Carlo Sorda <gambas@users.sourceforge.net><csorda@libero.it>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __CCORBA_H
#define __CCORBA_H

#include "gambas.h"
#include <omniORB4/CORBA.h>

#ifndef __CCORBAAPPLICATION_CPP
	extern GB_DESC CCORBAApplicationDesc[];
	extern GB_DESC CCORBAObjectDesc[];
	extern GB_DESC CCORBACosNamingNamesDesc[];
	extern GB_DESC CCORBACosNamingNameDesc[];
#endif

typedef
  struct {
	GB_BASE ob;
	CosNaming::Name name;
	 char* id;
	char* kind;
	 int index;
	}
CCORBA_COSNAMING;

typedef
  struct {
	GB_BASE ob;
	GB_COLLECTION *cosname;
  }
  CCORBA_COSNAMINGS;

typedef
  struct {
	GB_BASE ob;
	char* URI;
	char* IOR;
	char* CorbaServerName;
	// Storage of the object objref.;	
	CORBA::Object_var obj_var_;
	}
CCORBA_OBJECT;

  
typedef
  struct {
    	GB_BASE ob;
		char* name;
    	char* object;
		char* URI;
		char* IOR;
		char* CorbaServerName;
		CORBA::ORB_var orb;
		CORBA::Object_var obj_var_;
    }
  CCORBA_APPLICATION;

void CORBA_init(void);
void CORBA_exit(void);

#endif /* __CCORBA_H */
