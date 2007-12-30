/***************************************************************************

  gbc_read_common.h

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

#ifndef __GBC_READ_COMMON_H
#define __GBC_READ_COMMON_H

typedef
  int PATTERN;

enum {
  RT_END = 0,
  RT_NEWLINE = 1,
  RT_RESERVED = 2,
  RT_IDENTIFIER = 3,
  RT_NUMBER = 4,
  RT_STRING = 5,
  RT_TSTRING = 6,
  RT_PARAM = 7,
  RT_SUBR = 8,
  RT_CLASS = 9,
  RT_COMMENT = 10, /* Used by Eval() */
  RT_OPERATOR = 11, /* Used by Eval() */
  RT_OUTPUT = 0x20,
  RT_POINT = 0x40,
  RT_FIRST = 0x80
  };
   
#define NULL_PATTERN ((PATTERN)0L)

#define PATTERN_make(type, index) ((PATTERN)((type) << 24) | (index))

#define PATTERN_flag(pattern)   (((pattern) >> 24) & ~0xF)
#define PATTERN_type(pattern)   (((pattern) >> 24) & 0xF)
#define PATTERN_index(pattern)  ((pattern) & 0x00FFFFFFL)

#define PATTERN_is(pattern, res) (pattern == PATTERN_make(RT_RESERVED, res))

#define PATTERN_is_end(pattern)         (PATTERN_type(pattern) == RT_END)
#define PATTERN_is_reserved(pattern)    (PATTERN_type(pattern) == RT_RESERVED)

#define PATTERN_is_identifier(pattern)  (PATTERN_type(pattern) == RT_IDENTIFIER)
#define PATTERN_is_class(pattern) 		  (PATTERN_type(pattern) == RT_CLASS)
#define PATTERN_is_newline(pattern)     (PATTERN_type(pattern) == RT_NEWLINE)
#define PATTERN_is_param(pattern)       (PATTERN_type(pattern) == RT_PARAM)
#define PATTERN_is_subr(pattern)        (PATTERN_type(pattern) == RT_SUBR)
#define PATTERN_is_number(pattern)      (PATTERN_type(pattern) == RT_NUMBER)
#define PATTERN_is_string(pattern)      (PATTERN_type(pattern) == RT_STRING)
#define PATTERN_is_tstring(pattern)     (PATTERN_type(pattern) == RT_TSTRING)

#define PATTERN_is_newline_end(pattern) (PATTERN_is_newline(pattern) || PATTERN_is_end(pattern))

#define PATTERN_is_first(pattern)       ((PATTERN_flag(pattern) & RT_FIRST) != 0)
#define PATTERN_is_point(pattern)       ((PATTERN_flag(pattern) & RT_POINT) != 0)
#define PATTERN_is_output(pattern)      ((PATTERN_flag(pattern) & RT_OUTPUT) != 0)

#define PATTERN_set_flag(pattern, flag)    ((pattern) | (flag << 24))
#define PATTERN_unset_flag(pattern, flag)    ((pattern) & ~(flag << 24))

#define PATTERN_is_operand(pattern)   (PATTERN_is_reserved(pattern) && RES_is_operand(PATTERN_index(pattern)))
#define PATTERN_is_type(pattern)      (PATTERN_is_reserved(pattern) && RES_is_type(PATTERN_index(pattern)))

#endif
