/***************************************************************************

  eval_read.h

  Lexical parser

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

#ifndef __EVAL_READ_H
#define __EVAL_READ_H

typedef
  long PATTERN;

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
  RT_COMMENT = 10,
  RT_OPERATOR = 11,
  RT_OUTPUT = 0x20,  /* si un appel de fonction poss�e des param�res de sortie */
  RT_POINT = 0x40,   /* si le pattern est deuxi�e op�ande de l'op�ateur '.' */
  RT_FIRST = 0x80    /* si premier identificateur d'un op�ateur '.' */
  };

#ifndef __EVAL_READ_C
EXTERN long READ_source_ptr;
#endif

#define NULL_PATTERN ((PATTERN)0L)

#define PATTERN_make(type, index) ((PATTERN)((type) << 24) | (index))

#define PATTERN_flag(pattern)   (((pattern) >> 24) & ~0xF)
#define PATTERN_type(pattern)   (((pattern) >> 24) & 0xF)
#define PATTERN_index(pattern)  ((pattern) & 0x00FFFFFFL)

#define PATTERN_IS(pattern, res) (pattern == MAKE_PATTERN(RT_RESERVED, res))
#define PATTERN_is(pattern, res) (pattern == MAKE_PATTERN(RT_RESERVED, res))

#define PATTERN_is_end(pattern)         (PATTERN_TYPE(pattern) == RT_END)
#define PATTERN_is_reserved(pattern)    (PATTERN_TYPE(pattern) == RT_RESERVED)

#define PATTERN_is_identifier(pattern)  (PATTERN_TYPE(pattern) == RT_IDENTIFIER)
#define PATTERN_is_class(pattern)			  (PATTERN_TYPE(pattern) == RT_CLASS)
#define PATTERN_is_newline(pattern)     (PATTERN_TYPE(pattern) == RT_NEWLINE)
#define PATTERN_is_param(pattern)       (PATTERN_TYPE(pattern) == RT_PARAM)
#define PATTERN_is_subr(pattern)        (PATTERN_TYPE(pattern) == RT_SUBR)
#define PATTERN_is_number(pattern)      (PATTERN_TYPE(pattern) == RT_NUMBER)
#define PATTERN_is_string(pattern)      (PATTERN_TYPE(pattern) == RT_STRING)
#define PATTERN_is_tstring(pattern)     (PATTERN_type(pattern) == RT_TSTRING)

#define PATTERN_is_newline_end(pattern) (PATTERN_is_newline(pattern) || PATTERN_is_end(pattern))

#define PATTERN_is_first(pattern)       ((PATTERN_flag(pattern) & RT_FIRST) != 0)
#define PATTERN_is_point(pattern)       ((PATTERN_flag(pattern) & RT_POINT) != 0)
#define PATTERN_is_output(pattern)      ((PATTERN_flag(pattern) & RT_OUTPUT) != 0)

#define PATTERN_set_flag(pattern, flag)    ((pattern) | (flag << 24))
#define PATTERN_unset_flag(pattern, flag)    ((pattern) & ~(flag << 24))

#define PATTERN_IS_OPERAND(pattern)   (PATTERN_is_reserved(pattern) && RES_is_operand(PATTERN_index(pattern)))
#define PATTERN_is_type(pattern)      (PATTERN_is_reserved(pattern) && RES_is_type(PATTERN_index(pattern)))

/* r�ro-compatibilit�*/

#define PATTERN_IS_IDENTIFIER   PATTERN_is_identifier
#define PATTERN_IS_NEWLINE      PATTERN_is_newline
#define PATTERN_IS_STRING       PATTERN_is_string
#define PATTERN_IS_NUMBER       PATTERN_is_number
#define PATTERN_IS_END          PATTERN_is_end
#define PATTERN_IS_RESERVED     PATTERN_is_reserved
#define PATTERN_INDEX           PATTERN_index
#define PATTERN_TYPE            PATTERN_type
#define MAKE_PATTERN            PATTERN_make
#define PATTERN_IS_TYPE         PATTERN_is_type

/*PUBLIC void READ_dump_pattern(PATTERN *pattern);*/

PUBLIC void EVAL_read(void);
PUBLIC char *READ_get_pattern(PATTERN *pattern);

#endif
