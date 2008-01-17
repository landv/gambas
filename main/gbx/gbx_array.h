/***************************************************************************

  Array.h

  Array management routines

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

#ifndef __GBX_ARRAY_H
#define __GBX_ARRAY_H

#include "gbx_type.h"

typedef
  struct {
    CTYPE type;
    int dim[0];
    }
  ARRAY_DESC;

size_t ARRAY_get_size(ARRAY_DESC *desc);
void ARRAY_new(void **data, ARRAY_DESC *desc);
void ARRAY_free_data(void *data, ARRAY_DESC *desc);
void ARRAY_free(void **data, ARRAY_DESC *desc);
void *ARRAY_get_address(ARRAY_DESC *desc, void *addr, int nparam, int *param);

#endif
