/***************************************************************************

  gbx_c_subcollection.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GBX_C_SUBCOLLECTION_H
#define __GBX_C_SUBCOLLECTION_H

#include "gambas.h"

#include "gb_hash.h"
#include "gbx_object.h"

typedef
  struct {
    OBJECT object;
    HASH_TABLE *hash_table;
    int mode;
    void *container;
    //GB_SUBCOLLECTION *store;
    GB_SUBCOLLECTION_DESC *desc;
    char **list;
    }
  CSUBCOLLECTION;

#ifndef __GBX_C_SUBCOLLECTION_C

extern GB_DESC NATIVE_SubCollection[];

#else

#define THIS ((CSUBCOLLECTION *)_object)

#endif

#endif
