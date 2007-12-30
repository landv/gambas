/***************************************************************************

  Number.h

  Numbers management routines

  Datatype management routines. Conversions between each Gambas datatype,
  and conversions between Gambas datatypes and native datatypes.

  (c) 2000-2005 Benoît Minisini <gambas@users.sourceforge.net>

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


PUBLIC bool NUMBER_from_string(int option, const char *str, long len, VALUE *value);
PUBLIC void NUMBER_int_to_string(unsigned long long nbr, int prec, int base, VALUE *value);

#endif /* */
