/***************************************************************************

  gbx_jit.c

  (c) 2012 Emil Lenngren <emil.lenngren [at] gmail.com>

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

#define __GBX_JIT_C

#include "gbx_jit.h"
#include "gbx_api.h"
#include "gbx_event.h"

JIT_INTERFACE JIT;

bool JIT_load(){
	static bool loaded = FALSE;
	static bool available = TRUE;
	
	if (loaded)
		return TRUE;
	
	if (!available)
		return FALSE;
	
	TRY
	{
		COMPONENT_load(COMPONENT_create("gb.jit"));
		LIBRARY_get_interface_by_name("gb.jit", JIT_INTERFACE_VERSION, &JIT);
		
		JIT.Init((GB_JIT_INTERFACE *)(void *)GAMBAS_JitApi, &EXEC_current, &SP, &TEMP, &RET, &GAMBAS_StopEvent,
			(char **)&EXEC_enum, &EXEC, &EXEC_unknown_name, &EVENT_Last, &ERROR_current, &ERROR_handler, &STRING_char_string[0]);
		
		loaded = TRUE;
	}
	CATCH
	{
		available = FALSE;
	}
	END_TRY
	
	return loaded;
}

void JIT_default_jit_function()
{
	JIT.CompileAndExecute();
}
