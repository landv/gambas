/***************************************************************************

  gbx_number.h

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

#ifndef __GBX_NUMBER_H
#define __GBX_NUMBER_H

#include "gbx_type.h"
#include "gbx_value.h"

enum {
  NB_READ_NOTHING = 0,
  NB_READ_INTEGER = 1,
  NB_READ_LONG = 2,
  NB_READ_INT_LONG = 3,
  NB_READ_FLOAT = 4,
  NB_READ_ALL = 7,
  NB_READ_HEX_BIN = 8,
  NB_LOCAL = 16
  };


bool NUMBER_from_string(int option, const char *str, int len, VALUE *value);
void NUMBER_int_to_string(uint64_t nbr, int prec, int base, VALUE *value);

#endif /* */
