/***************************************************************************

  eval_read.c

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

#define __EVAL_READ_C

#include <ctype.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_table.h"

#include "eval.h"
#include "eval_read.h"


/*#define DEBUG*/

PUBLIC long READ_source_ptr;
#define source_ptr READ_source_ptr

static int last_pattern_type = -1;
static PATTERN last_pattern = 0;
static boolean begin_line = FALSE;

/* We declare the well known class to the compiler */

/*
static void READ_exit(void)
{
  static char *wkclass[] = { "gb", "Class", "File", "Error", "App", "Sys", NULL };
  long index;
  char **wkc;

  for (wkc = wkclass; *wkc; wkc++)
  {
    if (TABLE_find_symbol(JOB->class->table, *wkc, strlen(*wkc), NULL, &index))
      CLASS_add_class(JOB->class, index);
  }
}
*/
static bool _no_quote = FALSE;
#define BUF_MAX 255
static char _buffer[BUF_MAX + 1];

PUBLIC char *READ_get_pattern(PATTERN *pattern)
{
  int type = PATTERN_type(*pattern);
  long index = PATTERN_index(*pattern);
  const char *str;
  const char *before = _no_quote ? "" : "'";
  const char *after = _no_quote ? "" : "'";

  switch(type)
  {
    case RT_RESERVED:
      //snprintf(COMMON_buffer, COMMON_BUF_MAX, "%s%s%s", before, TABLE_get_symbol_name(COMP_res_table, index), after);
      str = TABLE_get_symbol_name(COMP_res_table, index);
      if (ispunct(*str))
        snprintf(_buffer, BUF_MAX, "%s%s%s", before, str, after);
      else
        strcpy(_buffer, str);
      break;

    case RT_NUMBER:
    case RT_IDENTIFIER:
    case RT_CLASS:
      snprintf(_buffer, BUF_MAX, "%s%s%s", before, TABLE_get_symbol_name(EVAL->table, index), after);
      break;

    case RT_STRING:
    case RT_TSTRING:
      snprintf(_buffer, BUF_MAX, "\"%s\"", TABLE_get_symbol_name(EVAL->string, index));
      break;

    case RT_NEWLINE:
      strcpy(_buffer, "end of line");
      break;

    case RT_END:
      strcpy(_buffer, "end of file");
      break;

    case RT_SUBR:
      //snprintf(COMMON_buffer, COMMON_BUF_MAX, "%s%s%s", bafore, COMP_subr_info[index].name, after);
      strcpy(_buffer, COMP_subr_info[index].name);
      break;

    default:
      sprintf(_buffer, "%s?%p?%s", before, (void *)*pattern, after);
  }

  return _buffer;
}


PUBLIC void READ_dump_pattern(PATTERN *pattern)
{
  int type = PATTERN_TYPE(*pattern);
  long index = PATTERN_INDEX(*pattern);
  long pos;

  pos = (long)(pattern - EVAL->pattern);
  if (pos >= 0 && pos < ARRAY_count(EVAL->pattern))
    printf("%ld ", pos);

  if (PATTERN_flag(*pattern) & RT_FIRST)
    printf("!");
  else
    printf(" ");

  if (PATTERN_flag(*pattern) & RT_POINT)
    printf(".");
  else
    printf(" ");

  if (PATTERN_flag(*pattern) & RT_CLASS)
    printf("C");
  else
    printf(" ");

  printf(" ");

  _no_quote = TRUE;

  if (type == RT_RESERVED)
    printf("RESERVED     %s\n", READ_get_pattern(pattern));
  else if (type == RT_NUMBER)
    printf("NUMBER       %s\n", READ_get_pattern(pattern));
  else if (type == RT_IDENTIFIER)
    printf("IDENTIFIER   %s\n", READ_get_pattern(pattern));
  else if (type == RT_CLASS)
    printf("CLASS        %s\n", READ_get_pattern(pattern));
  else if (type == RT_STRING)
    printf("STRING       %s\n", READ_get_pattern(pattern));
  else if (type == RT_TSTRING)
    printf("TSTRING      %s\n", READ_get_pattern(pattern));
  else if (type == RT_NEWLINE)
    printf("NEWLINE      (%ld)\n", index);
  else if (type == RT_END)
    printf("END\n");
  else if (type == RT_PARAM)
    printf("PARAM        %ld\n", index);
  else if (type == RT_SUBR)
    printf("SUBR         %s\n", READ_get_pattern(pattern));
  else
    printf("?            %ld\n", index);

  _no_quote = FALSE;
}

static uchar get_char_offset(int offset)
{
  offset += source_ptr;

  if (offset >= EVAL->len || offset < 0)
    return 0;
  else
    return (uchar)(EVAL->source[offset]);
}


static uchar get_char(void)
{
  return get_char_offset(0);
}

static uchar next_char(void)
{
  source_ptr++;
  return get_char();
}


static void add_pattern(int type, long index)
{
  PATTERN *pattern;

  pattern = ARRAY_add(&EVAL->pattern);

  *pattern = PATTERN_make(type, index);
  last_pattern_type = type;
  last_pattern = *pattern;

  #ifdef DEBUG
  READ_dump_pattern(pattern);
  #endif
}


static void add_newline()
{
  add_pattern(RT_NEWLINE, 0);
  source_ptr++;
}


static void add_end()
{
  add_pattern(RT_END, 0);
  source_ptr++;
}


static bool is_number()
{
  int pos = 0;
  unsigned char car;
  unsigned char car2;

  car = get_char_offset(pos);

  if (car == '-' || car == '+')
  {
    /*car = get_char_offset(-1);
    if (car && !isspace(car))
      return FALSE;*/
    pos++;
    car = get_char_offset(pos);
  }

  if (isdigit(car))
    return TRUE;

  car2 = toupper(get_char_offset(pos + 1));

  if (car == '&')
  {
    if (car2 == 'H')
    {
      pos += 2;
      goto __HEX;
    }

    if (car2 == 'X')
    {
      pos += 2;
      goto __BIN;
    }

    pos++;
    goto __HEX;
  }
  else if (car == '%')
  {
    pos ++;
    goto __BIN;
  }
  else
    return FALSE;

__HEX:

  car = get_char_offset(pos);
  return (isdigit(car) || index("abcdefABCDEF", car) != NULL);

__BIN:

  car = get_char_offset(pos);
  return (car == '0' || car == '1');
}


static void add_number()
{
  unsigned char car;
  long start;
  long index;
  char sign;

  start = source_ptr;
  car = get_char();

  if (car == '-' || car == '+')
  {
    sign = car;
    car = next_char();
  }
  else
    sign = 0;

  if (car == '&')
  {
    car = toupper(next_char());

    if (car == 'H')
      goto READ_HEXA;
    else if (car == 'X')
      goto READ_BINARY;
    else
    {
      source_ptr--;
      goto READ_HEXA;
    }
  }
  else if (car == '%')
    goto READ_BINARY;
  else
    goto READ_NUMBER;

READ_BINARY:

  for (;;)
  {
    car = next_char();
    if (car != '0' && car != '1')
      break;
  }

  if (car == '&')
    car = next_char();

  goto END;

READ_HEXA:

  for (;;)
  {
    car = next_char();
    if (!isxdigit(car))
      break;
  }

  if (car == '&')
    car = next_char();

  goto END;

READ_NUMBER:

  while (isdigit(car))
    car = next_char();

  if (car == '.')
  {
    do
    {
      car = next_char();
    }
    while (isdigit(car));
  }

  if (toupper(car) == 'E')
  {
    car = next_char();
    if (car == '+' || car == '-')
      car = next_char();

    while (isdigit(car))
      car = next_char();
  }

  goto END;

END:

  if (sign && (last_pattern_type != RT_RESERVED || PATTERN_is(last_pattern, RS_RBRA) || PATTERN_is(last_pattern, RS_RSQR)))
  {
    TABLE_find_symbol(COMP_res_table, &sign, 1, NULL, &index);
    add_pattern(RT_RESERVED, index);
    TABLE_add_symbol(EVAL->table, &EVAL->source[start + 1], source_ptr - start - 1, NULL, &index);
    add_pattern(RT_NUMBER, index);
  }
  else
  {
    TABLE_add_symbol(EVAL->table, &EVAL->source[start], source_ptr - start, NULL, &index);
    add_pattern(RT_NUMBER, index);
  }
}


static void add_identifier(bool no_res)
{
  unsigned char car;
  long start;
  int len;
  long index;
  int type;
  boolean not_first;
  boolean can_be_reserved;
  boolean can_be_subr;
  boolean is_type;
  boolean last_func, last_declare, last_type; //, last_event;

	type = RT_IDENTIFIER;

  start = source_ptr;
  len = 1;

  for(;;)
  {
    source_ptr++;
    car = get_char();
    if (car == 0 || !isascii(car) || ((!isalnum(car)) && (strchr("$_?@", (int)car) == NULL)))
      break;
    len++;
  }

  //last_event = PATTERN_is(last_pattern, RS_EVENT) || PATTERN_is(last_pattern, RS_RAISE);

  if (no_res)
  {
    if (!EVAL->analyze)
    {
      if (get_char() == '}')
        source_ptr++;
    }
    goto IDENTIFIER;
  }

  not_first = PATTERN_IS(last_pattern, RS_PT) || PATTERN_IS(last_pattern, RS_EXCL);
  last_func = PATTERN_IS(last_pattern, RS_PROCEDURE) || PATTERN_IS(last_pattern, RS_SUB) || PATTERN_IS(last_pattern, RS_FUNCTION);
  last_declare = PATTERN_is(last_pattern, RS_PUBLIC) || PATTERN_is(last_pattern, RS_PRIVATE)
                 || PATTERN_is(last_pattern, RS_DIM) || PATTERN_is(last_pattern, RS_PROPERTY)
                 || PATTERN_is(last_pattern, RS_READ) || PATTERN_is(last_pattern, RS_INHERITS);
  last_type = PATTERN_is(last_pattern, RS_AS) || PATTERN_is(last_pattern, RS_NEW) || PATTERN_is(last_pattern, RS_IS) || PATTERN_is(last_pattern, RS_INHERITS);
  car = get_char();

  if (car == '}')
    can_be_reserved = FALSE;
  else
    can_be_reserved = !not_first && TABLE_find_symbol(COMP_res_table, &EVAL->source[start], len, NULL, &index);

  if (can_be_reserved)
  {
    if (index == RS_ME || index == RS_NEW || index == RS_LAST || index == RS_SUPER)
    {
      can_be_reserved = !last_declare;
      /* Toujours r�erv� ne rien faire */
    }
    else if (index == RS_CLASS)
    {
      can_be_reserved = begin_line && isspace(car);
    }
    else
    {
      is_type = (PATTERN_is_type(PATTERN_make(RT_RESERVED, index)) || index == RS_NEW);

      if (last_type)
        can_be_reserved = is_type;
      else if (last_func)
        can_be_reserved = FALSE;
      else
        can_be_reserved = !is_type && (car != ':') && (car != '.') && (car != '!') && (car != '(');
    }
  }

  can_be_subr = !not_first && !last_func && !last_declare && !last_type && (car != '.' && car != '!' );

  if (can_be_reserved)
  {
    type = RT_RESERVED;
  }
  else if (can_be_subr && TABLE_find_symbol(COMP_subr_table, &EVAL->source[start], len, NULL, &index))
  {
    type = RT_SUBR;
    /*index = COMP_subr_info[index].opcode;*/
  }
  else
  {
  	if (last_type)
  		type = RT_CLASS;
    goto IDENTIFIER;
	}

  add_pattern(type, index);
  return;

IDENTIFIER:

  TABLE_add_symbol(EVAL->table, &EVAL->source[start], len, NULL, &index);
  add_pattern(type, index);
}


static void add_operator()
{
  unsigned char car;
  long start, end;
  int len;
  long op = NO_SYMBOL;
  long index;

  start = source_ptr;
  end = start;
  len = 1;

  for(;;)
  {
    source_ptr++;

    if (TABLE_find_symbol(COMP_res_table, &EVAL->source[start], len, NULL, &index))
    {
      op = index;
      end = source_ptr;
    }

    car = get_char();
    if (!isascii(car) || !ispunct(car))
      break;
    len++;
  }

  source_ptr = end;

	if (EVAL->analyze && op == RS_QUES)
		op = RS_PRINT;

  if (op < 0)
    THROW("Unknown operator");

  add_pattern(RT_RESERVED, op);
}


static int xdigit_val(unsigned char c)
{
  c = tolower(c);

  if (c >= '0' && c <= '9')
    return (c - '0');
  else if (c >= 'a' && c <= 'f')
    return (c - 'a' + 10);
  else
    return (-1);
}

static void add_string()
{
  unsigned char car;
  long start;
  int len;
  long index;
  int newline;
  bool jump;
  char *p;
  int i;

  start = source_ptr;
  len = 0;
  newline = 0;
  jump = FALSE;
  p = &EVAL->source[source_ptr];

  for(;;)
  {
    source_ptr++;
    car = get_char();

    if (jump)
    {
      if (car == '\n')
        newline++;
      else if (car == '"')
        jump = FALSE;
      else if (!isspace(car))
        break;
    }
    else
    {
      p++;
      len++;

      if (car == '\n')
        THROW("Non terminated string");

      if (car == '\\')
      {
        source_ptr++;
        car = get_char();

        if (car == 'n')
          *p = '\n';
        else if (car == 't')
          *p = '\t';
        else if (car == 'r')
          *p = '\r';
        else if (car == '\"' || car == '\'' || car == '\\')
          *p = car;
        else
        {
          if (car == 'x')
          {
            i = xdigit_val(get_char_offset(1));
            if (i >= 0)
            {
              car = i;
              i = xdigit_val(get_char_offset(2));
              if (i >= 0)
              {
                car = (car << 4) | i;
                *p = car;
                source_ptr += 2;
                continue;
              }
            }
          }

          THROW("Bad character constant in string");
        }
      }
      else if (car == '"')
      {
        p--;
        len--;
        jump = TRUE;
      }
      else
        *p = car;
    }
  }

  p[1] = 0;

  TABLE_add_symbol(EVAL->string, &EVAL->source[start + 1], len, NULL, &index);

  add_pattern(RT_STRING, index);

  for (i = 0; i < newline; i++)
    add_newline();
}


static void add_comment()
{
  unsigned char car;
  long start;
  int len;
  long index;
  int type;

  start = source_ptr;
  len = 1;

  for(;;)
  {
    source_ptr++;
    car = get_char();
    if (car == 0 || car == '\n')
      break;
    len++;
  }

  TABLE_add_symbol(EVAL->string, &EVAL->source[start], len, NULL, &index);
  type = RT_COMMENT;

  add_pattern(type, index);
}


static void add_string_for_analyze()
{
  unsigned char car;
  long start;
  int len;
  long index;
  int type;

  start = source_ptr;
  len = 0;

  for(;;)
  {
    source_ptr++;
    car = get_char();
    if (car == '\\')
    {
      source_ptr++;
      car = get_char();
      len++;
      if (car == 0)
        break;
    }
    else if (car == 0 || car == '\n' || car == '"')
      break;
    len++;
  }

  if (car == '"')
    source_ptr++;

  TABLE_add_symbol(EVAL->string, &EVAL->source[start + 1], len, NULL, &index);
  type = RT_STRING;

  add_pattern(type, index);
  //fprintf(stderr, "add_string_for_analyze: %s\n", TABLE_get_symbol_name(EVAL->string, index));
}


PUBLIC void EVAL_read(void)
{
  uchar car;

  source_ptr = 0;
  begin_line = TRUE;

  while (source_ptr < EVAL->len)
  {
    car = get_char();

    if (car == '\n')
    {
      add_newline();
      begin_line = TRUE;
      continue;
    }

    if (car <= ' ')
    {
      source_ptr++;
      continue;
    }

    if (car == '\'')
    {
      if (EVAL->analyze)
      {
        add_comment();
      }
      else
      {
        do
        {
          source_ptr++;
          car = get_char();
        }
        while (car != '\n' && car != 0);
      }

      begin_line = FALSE;
      continue;
    }

    if (car == '"')
    {
      if (EVAL->analyze)
        add_string_for_analyze();
      else
        add_string();
      begin_line = FALSE;
      continue;
    }

    if (!isascii(car))
      THROW("Syntax error");

    if (isalpha(car) || car == '_' || car == (unsigned char)'$')
    {
      add_identifier(FALSE);
      begin_line = FALSE;
      continue;
    }

    if (is_number())
    {
      add_number();
      begin_line = FALSE;
      continue;
    }

    if (car == '{')
    {
      if (EVAL->analyze)
      {
        add_operator();
        add_identifier(FALSE);
        add_operator();
      }
      else
      {
        source_ptr++;
        add_identifier(TRUE);
      }
      begin_line = FALSE;
      continue;
    }

    add_operator();
    begin_line = FALSE;
  }

	// Add end delimiters to allow compiler to look patterns forward

  add_end();
  add_end();
  add_end();
  add_end();

  /*READ_exit();*/
}
