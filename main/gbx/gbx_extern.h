/***************************************************************************

  gbx_extern.h

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

#ifndef __GBX_EXTERN_H
#define __GBX_EXTERN_H

#include "config.h"

#include <dlfcn.h>

#ifndef DONT_USE_LTDL
  #include <ltdl.h>
#else
  #define lt_dlsym dlsym
  #define lt_dlclose dlclose
  #define lt_dlerror dlerror
  #define lt_dlhandle void *
#endif

#include "gbx_value.h"

typedef
	struct EXTERN_FUNC_INFO {
		void *call;
		char *alias;
	}
	EXTERN_FUNC_INFO;


void EXTERN_release(void);
void EXTERN_exit(void);
EXTERN_FUNC_INFO EXTERN_get_function_info(CLASS_EXTERN *ext);
void EXTERN_call(void);
void *EXTERN_make_callback(VALUE_FUNCTION *value);
void *EXTERN_get_symbol(const char *library, const char *symbol);

#endif
