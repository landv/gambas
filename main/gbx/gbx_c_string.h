/***************************************************************************

  gbx_c_string.h

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#ifndef __GBX_C_STRING_H
#define __GBX_C_STRING_H

#include "gambas.h"

#ifndef __GBX_C_STRING_C

EXTERN GB_DESC StringDesc[];
EXTERN GB_DESC BoxedStringDesc[];

EXTERN const char STRING_char_length[];

#endif

#define UNICODE_INVALID 0xFFFFFFFFU

bool STRING_convert_to_unicode(wchar_t **pwstr, int *pwlen, const char *str, int len);
void STRING_utf8_from_unicode(uint code, char *sstr);
uint STRING_utf8_to_unicode(const char *sstr, int len);

#define STRING_utf8_get_char_length(_c) ((int)STRING_char_length[(unsigned char)(_c)])

int COMMON_get_unicode_char();

void BoxedString_get(ushort code);

#endif
