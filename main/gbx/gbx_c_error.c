/***************************************************************************

  CError.c

  The native class Error

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
#include "gbx_stack.h"
#include "gbx_exec.h"

#include "gbx_c_error.h"


BEGIN_PROPERTY(CERROR_code)

  GB_ReturnInt(ERROR_current->info.code);

END_PROPERTY


BEGIN_PROPERTY(CERROR_text)

	if (ERROR_current->info.code)
  	GB_ReturnNewZeroString(ERROR_current->info.msg);
	else
		GB_ReturnNull();

END_PROPERTY


BEGIN_PROPERTY(CERROR_class)

	if (ERROR_current->info.code)
  	GB_ReturnObject(ERROR_current->info.cp);
	else
		GB_ReturnNull();

END_PROPERTY


BEGIN_PROPERTY(CERROR_where)

	if (ERROR_current->info.code)
  	GB_ReturnNewZeroString(DEBUG_get_position(ERROR_current->info.cp, ERROR_current->info.fp, ERROR_current->info.pc));
	else
		GB_ReturnNull();

END_PROPERTY


BEGIN_METHOD_VOID(CERROR_clear)

  ERROR_clear();

END_METHOD


BEGIN_METHOD(CERROR_raise, GB_STRING msg)

  ERROR_define(GB_ToZeroString(ARG(msg)), NULL);
  GAMBAS_Error = TRUE;

END_METHOD


BEGIN_METHOD_VOID(CERROR_propagate)

	if (ERROR_current->info.code)
		GAMBAS_Error = TRUE;

END_METHOD


BEGIN_PROPERTY(CERROR_backtrace)

	DEBUG_BACKTRACE *bt = (DEBUG_BACKTRACE *)ERROR_current->info.backtrace;
	
	if (!bt)
		GB_ReturnNull();
	else
		GB_ReturnObject(DEBUG_get_string_array_from_backtrace(bt));
	
END_PROPERTY

#endif

GB_DESC NATIVE_Error[] =
{
  GB_DECLARE("Error", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("Code", "i", CERROR_code),
  GB_STATIC_PROPERTY_READ("Text", "s", CERROR_text),
  GB_STATIC_PROPERTY_READ("Class", "Class", CERROR_class),
  GB_STATIC_PROPERTY_READ("Where", "s", CERROR_where),
  GB_STATIC_PROPERTY_READ("Backtrace", "String[]", CERROR_backtrace),

  GB_STATIC_METHOD("Clear", NULL, CERROR_clear, NULL),
  GB_STATIC_METHOD("Raise", NULL, CERROR_raise, "(Message)s"),
  GB_STATIC_METHOD("Propagate", NULL, CERROR_propagate, NULL),

  GB_END_DECLARE
};


