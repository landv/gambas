/***************************************************************************

  ldap.h

  Ldap routines

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#ifndef __CLDAP_H
#define __CLDAP_H

#include "../gambas.h"
#include <ldap.h>

#ifndef __CLDAP_C
extern GB_DESC CLDAPDesc[];
extern GB_DESC CLDAPModDesc[];
extern GB_DESC CLDAPMessageDesc[];
extern GB_DESC CLDAPSearchEntryDesc[];
#endif

#define LDAP_DEPRECATED 1
#define LOOPS 100

typedef
  struct {
	GB_BASE ob;
	char *dn;
	GB_COLLECTION *attributes;
  }
  CLDAPSEARCHENTRY;
  
typedef
  struct {
	GB_BASE ob;
	LDAPMod *mod;
  }
  CLDAPMOD;

typedef 
	struct {
		GB_BASE ob;
		LDAPMessage *msg;
	}
	CLDAPMESSAGE;
	
typedef
  struct {
    GB_BASE ob;
    LDAP *ld;
    }
  CLDAP;

void LDAP_init(void);
void LDAP_exit(void);

#endif /* __LDAP_H */
