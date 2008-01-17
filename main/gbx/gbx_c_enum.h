/***************************************************************************

  gbx_c_enum.h

  (c) 2000-2005 Beno�t Minisini <gambas@users.sourceforge.net>

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

#ifndef __GBX_C_ENUM_H
#define __GBX_C_ENUM_H

#include "gambas.h"
#include "gb_list.h"
#include "gbx_variant.h"

typedef
  struct CENUM {
    GB_BASE object;
    LIST list;
    void *enum_object;
    void *data[4];
    unsigned stop : 1;
    unsigned variant : 1;
    }
  CENUM;

#ifndef __GBX_C_ENUM_C
extern GB_DESC NATIVE_Enum[];
#else

#define THIS ((CENUM *)_object)

#endif

PUBLIC CENUM *CENUM_create(void *object);
PUBLIC CENUM *CENUM_get_next(CENUM *);

#endif
