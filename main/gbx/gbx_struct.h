/***************************************************************************

  gbx_struct.h

  (c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __GBX_STRUCT_H
#define __GBX_STRUCT_H

#include "gbx_object.h"
#include "gbx_value.h"

#if 0
#ifndef __GBX_STRUCT_C
extern GB_DESC CSTRUCT_desc[];
#else
#define THIS ((CSTRUCT *)_object)
#endif

#define CSTRUCT_NDESC 3
#endif

typedef
	struct {
		OBJECT ob;
		void *ref;
	}
	CSTRUCT;
	
typedef
	struct {
		OBJECT ob;
		void *ref;
		char *addr;
	}
	CSTATICSTRUCT;
	
void *CSTRUCT_create_static(void *ref, CLASS *class, char *addr);
int CSTRUCT_get_size(CLASS *class);
void CSTRUCT_release(CSTRUCT *ob);

#endif
