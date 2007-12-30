/***************************************************************************

  compare.h

  Comparison functions

  (c) 2000-2006 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GBX_COMPARE_H
#define __GBX_COMPARE_H

#ifndef GBX_INFO
#include "gbx_type.h"
#endif

#define GB_COMP_BINARY   0
#define GB_COMP_TEXT     1

#define GB_COMP_TYPE_MASK  7

#define GB_COMP_ASCENT   0
#define GB_COMP_DESCENT  16

#ifndef GBX_INFO
typedef
  int (*COMPARE_FUNC)();

PUBLIC COMPARE_FUNC COMPARE_get(TYPE type, int mode);
PUBLIC int COMPARE_object(void **a, void **b);
#endif

#endif
