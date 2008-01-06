/***************************************************************************

  output.h

  The object file creator

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

#ifndef __GBC_OUTPUT_H
#define __GBC_OUTPUT_H

#include "gb_table.h"
#include "gb_class_desc_common.h"

typedef
  struct {
    SYMBOL sym;
    int value;
    }
  OUTPUT_SYMBOL;

typedef
  struct {
    off_t pos;
    uint val;
  }
  OUTPUT_CHANGE;

#define OUTPUT_BUFFER_SIZE 16384

PUBLIC void OUTPUT_do(bool swap);
PUBLIC const char *OUTPUT_get_file(const char *file);
PUBLIC const char *OUTPUT_get_trans_file(const char *file);

#endif


