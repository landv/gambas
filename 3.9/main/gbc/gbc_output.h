/***************************************************************************

  gbc_output.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

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

void OUTPUT_do(bool swap);
char *OUTPUT_get_file(const char *file);
char *OUTPUT_get_trans_file(const char *file);

#endif


