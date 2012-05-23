/***************************************************************************

  jit_api.c

  gb.jit component

  (c) 2012 Emil Lenngren <emil.lenngren [at] gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __JIT_API_C

#include "main.h"

extern "C" {

GB_JIT_INTERFACE JIF;

STACK_CONTEXT *_EXEC_current;
VALUE **_SP;
VALUE *_TEMP;
VALUE *_RET;

char *_GAMBAS_StopEvent;
char **_EXEC_enum;
EXEC_GLOBAL *_EXEC;
const char **_EXEC_unknown_name;

void **_EVENT_Last;

ERROR_CONTEXT **_ERROR_current;
ERROR_HANDLER **_ERROR_handler;

const char *_STRING_char_string;

void JIT_init(GB_JIT_INTERFACE *jif, STACK_CONTEXT *__EXEC_current, VALUE **__SP, VALUE *__TEMP,
	VALUE *__RET, char *__GAMBAS_StopEvent, char **__EXEC_enum, EXEC_GLOBAL *__EXEC,
	const char **__EXEC_unknown_name, void **__EVENT_Last, ERROR_CONTEXT **__ERROR_current,
	ERROR_HANDLER **__ERROR_handler, const char *__STRING_char_string)
{
	JIF = *jif;
	_EXEC_current = __EXEC_current;
	_SP = __SP;
	_TEMP = __TEMP;
	_RET = __RET;
	_GAMBAS_StopEvent = __GAMBAS_StopEvent;
	_EXEC_enum = __EXEC_enum;
	_EXEC = __EXEC;
	_EXEC_unknown_name = __EXEC_unknown_name;
	_EVENT_Last = __EVENT_Last;
	_ERROR_current = __ERROR_current;
	_ERROR_handler = __ERROR_handler;
	_STRING_char_string = __STRING_char_string;
}

}
