/***************************************************************************

  gxb_print.c

  Prints values and objects

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

#define __GBX_PRINT_C

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gb_common_case.h"

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include "gbx_exec.h"
#include "gbx_local.h"
#include "gbx_date.h"
#include "gbx_print.h"


static PRINT_FUNCTION _print;
static bool _trace = FALSE;

static void to_string(VALUE *value, char **addr, int *len)
{
  static void *jump[16] = {
    &&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, &&__DATE,
    &&__STRING, &&__STRING, &&__VARIANT, &&__ARRAY, &&__FUNCTION, &&__CLASS, &&__NULL
    };

  VALUE conv;

  //*more = FALSE;

__CONV:

  if (TYPE_is_object(value->type))
    goto __OBJECT;
  else
    goto *jump[value->type];

__NULL:

  *addr = "NULL";
  *len = 4;
  return;

__BOOLEAN:

  if (value->_boolean.value)
  {
    *addr = "TRUE";
    *len = 4;
  }
  else
  {
    *addr = "FALSE";
    *len = 5;
  }
  return;

__BYTE:
__SHORT:
__INTEGER:

  *len = sprintf(COMMON_buffer, "%d", value->_integer.value);
  *addr = COMMON_buffer;

  return;

__LONG:

  *len = sprintf(COMMON_buffer, "%lld", value->_long.value);
  *addr = COMMON_buffer;

  return;

__DATE:

  LOCAL_format_date(DATE_split(value), LF_STANDARD, NULL, 0, addr, len);
  return;

__SINGLE:
__FLOAT:

  LOCAL_format_number(value->_float.value, LF_STANDARD, NULL, 0, addr, len, TRUE);
  return;

__STRING:

  {
    int i;
    char *d;
    const char *s;
    uchar c;

    s = value->_string.addr + value->_string.start;
    d = COMMON_buffer;

    *d++ = '"';

    for (i = 0; i < value->_string.len; i++)
    {
      if (i > 128)
      {
        strcat(d, "...");
        *len += 3;
        break;
      }

      c = s[i];

      if (c < 32)
      {
        *d++ = '\\';

        if (c == 10)
          *d++ = 'n';
        else if (c == 13)
          *d++ = 'r';
        else if (c == 9)
          *d++ = 't';
        else
          d += sprintf(d, "x%02X", c);
      }
      else if (c == '\"')
      {
        *d++ = '\\';
        *d++ = c;
      }
      else
      {
        *d++ = c;
      }
    }

    *d++ = '"';

    *addr = COMMON_buffer;
    *len = d - COMMON_buffer;
  }

  return;

__OBJECT:

  if (VALUE_is_null(value))
    goto __NULL;

  //*more = !CLASS_is_native(OBJECT_class(value->_object.object));

  *len = sprintf(COMMON_buffer, "(%s %p)", OBJECT_class(value->_object.object)->name, value->_object.object);
  *addr = COMMON_buffer;
  return;

__VARIANT:

  conv = *value;
  value = &conv;
  VARIANT_undo(value);
  goto __CONV;

__VOID:

  *addr = "(void)";
  *len = 6;
  return;

__CLASS:

  {
    CLASS *class = value->_class.class;
    //*more = (!CLASS_is_native(class) && class->load->n_stat > 0);

    *len = sprintf(COMMON_buffer, "%s %p", class->name, class);
    *addr = COMMON_buffer;
    return;
  }

__ARRAY:

  *len = sprintf(COMMON_buffer, "(ARRAY %p)", value->_array.addr);
  *addr = COMMON_buffer;
  return;

__FUNCTION:

  THROW(E_TYPE, TYPE_get_name(T_STRING), TYPE_get_name(value->type));
}


PUBLIC void PRINT_init(PRINT_FUNCTION func, bool trace)
{
  _print = func;
  _trace = trace;
}


PUBLIC void PRINT_value(VALUE *value)
{
  char *addr;
  int len;

  /*if (TYPE_is_object(value->type))
  {
  	void *object = value->_object.object;
  	CLASS *class = OBJECT_class(object);

    if (!EXEC_spec(SPEC_PRINT, class, object, 0, TRUE))
      return;
  }*/

  if (_trace)
    to_string(value, &addr, &len);
  else
    VALUE_to_string(value, &addr, &len);

  (*_print)(addr, len);
}


PUBLIC void PRINT_string(char *addr, int len)
{
  (*_print)(addr, len);
}
