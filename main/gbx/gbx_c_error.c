/***************************************************************************

  gbx_c_error.c

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

#define __GBX_C_ERROR_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include "gb_common.h"
#include "gbx_class.h"
#include "gbx_debug.h"

#include "gambas.h"
#include "gbx_api.h"

#include "gbx_object.h"
#include "gbx_string.h"
#include "gbx_split.h"
#include "gbx_stack.h"
#include "gbx_exec.h"
#include "gbx_c_array.h"

#include "gbx_c_error.h"


BEGIN_PROPERTY(Error_Code)

  GB_ReturnInt(ERROR_last.code);

END_PROPERTY

static char **_arg;

static void get_subst(int np, char **str, int *len)
{
	*str = _arg[np];
	*len = STRING_length(_arg[np]);
}


BEGIN_PROPERTY(Error_Text)

	if (ERROR_last.code && ERROR_last.msg)
	{
		if (EXEC_debug)
		{
			GB_ARRAY array;
			char *result;
			
			array = STRING_split(ERROR_last.msg, strlen(ERROR_last.msg), "|", 1, NULL, 0, FALSE, FALSE);
			_arg = (char **)GB_ArrayGet(array, 0);
			
			result = STRING_subst(_arg[0], STRING_length(_arg[0]), get_subst);
			
			OBJECT_UNREF(array);
			
			GB_ReturnNewZeroString(result);
		}
		else
		{
			if (ERROR_last.free)
				GB_ReturnString(ERROR_last.msg);
			else
				GB_ReturnConstZeroString(ERROR_last.msg);
		}
  }
	else
		GB_ReturnVoidString();

END_PROPERTY


BEGIN_PROPERTY(Error_Class)

	if (ERROR_last.code)
  	GB_ReturnObject(ERROR_last.cp);
	else
		GB_ReturnNull();

END_PROPERTY


BEGIN_PROPERTY(Error_Where)

	if (ERROR_last.code)
  	GB_ReturnNewZeroString(DEBUG_get_position(ERROR_last.cp, ERROR_last.fp, ERROR_last.pc));
	else
		GB_ReturnVoidString();

END_PROPERTY


BEGIN_METHOD_VOID(Error_Clear)

  ERROR_reset(&ERROR_last);

END_METHOD


BEGIN_METHOD(Error_Raise, GB_STRING msg)

  ERROR_define(GB_ToZeroString(ARG(msg)), NULL);
  EXEC_set_native_error(TRUE);

END_METHOD


BEGIN_METHOD_VOID(Error_Propagate)

	if (ERROR_last.code)
	{
		ERROR_define_last();
		EXEC_set_native_error(TRUE);
	}

END_METHOD


BEGIN_PROPERTY(Error_Backtrace)

	if (ERROR_backtrace)
		GB_ReturnObject(DEBUG_get_string_array_from_backtrace(ERROR_backtrace));
	else
		GB_ReturnNull();
	
END_PROPERTY

#endif

GB_DESC NATIVE_Error[] =
{
  GB_DECLARE_STATIC("Error"),

  GB_STATIC_PROPERTY_READ("Code", "i", Error_Code),
  GB_STATIC_PROPERTY_READ("Text", "s", Error_Text),
  GB_STATIC_PROPERTY_READ("Class", "Class", Error_Class),
  GB_STATIC_PROPERTY_READ("Where", "s", Error_Where),
  GB_STATIC_PROPERTY_READ("Backtrace", "String[]", Error_Backtrace),

  GB_STATIC_METHOD("Clear", NULL, Error_Clear, NULL),
  GB_STATIC_METHOD("Raise", NULL, Error_Raise, "(Message)s"),
  GB_STATIC_METHOD("Propagate", NULL, Error_Propagate, NULL),

  GB_END_DECLARE
};


