/***************************************************************************

  gbx_compare.h

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

#ifndef __GBX_COMPARE_H
#define __GBX_COMPARE_H

#ifndef GBX_INFO
#include "gbx_type.h"
#include "gbx_variant.h"
#endif

#define GB_COMP_BINARY   0
#define GB_COMP_NOCASE   1
#define GB_COMP_LANG     2
#define GB_COMP_LIKE     4
#define GB_COMP_NATURAL  8

#define GB_COMP_TYPE_MASK  15

#define GB_COMP_ASCENT   0
#define GB_COMP_DESCENT  16

#ifndef GBX_INFO
typedef
  int (*COMPARE_FUNC)();

COMPARE_FUNC COMPARE_get(TYPE type, int mode);
int COMPARE_object(void **a, void **b);
int COMPARE_string_lang(const char *s1, int l1, const char *s2, int l2, bool nocase, bool throw);
int COMPARE_string_like(const char *s1, int l1, const char *s2, int l2);
int COMPARE_string_natural(const char *a, int la, const char *b, int lb, bool nocase);
int COMPARE_variant(VARIANT *a, VARIANT *b);
#endif

#endif
