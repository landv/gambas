/***************************************************************************

  gbx_debug.h

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

#ifndef __GBX_DEBUG_H
#define __GBX_DEBUG_H

#include "gbx_class.h"
#include "gbx_stack.h"
#include "gb_pcode.h"
#include "../lib/debug/gb.debug.h"

/* Object referencement debugging */
#define DEBUG_REF      0
/* String referencement debugging */
#define DEBUG_STRING   0
/* Bytecode debugging */
#define DEBUG_PCODE    0
/* Class loading debugging */
#define DEBUG_LOAD     0
/* Event debugging */
#define DEBUG_EVENT    0
/* Component management debugging */
#define DEBUG_COMP     0
/* Stream opening debugging */
#define DEBUG_STREAM   0

#ifndef __GBX_DEBUG_C

EXTERN DEBUG_INTERFACE DEBUG;
EXTERN DEBUG_INFO *DEBUG_info;
EXTERN int DEBUG_inside_eval;

#endif

#define DEBUG_is_init() (DEBUG_info != NULL)

void DEBUG_init(void);
void DEBUG_exit(void);

void DEBUG_enter_event_loop(void);
void DEBUG_leave_event_loop(void);

const char *DEBUG_get_position(CLASS *cp, FUNCTION *fp, PCODE *pc);
const char *DEBUG_get_current_position(void);
void DEBUG_where(void);

bool DEBUG_get_value(const char *sym, int len, GB_VARIANT *ret);
int DEBUG_set_value(const char *sym, int len, VALUE *value);
int DEBUG_get_object_access_type(void *object, CLASS *class, int *count);
GB_CLASS DEBUG_find_class(const char *name);
void DEBUG_enum_keys(void *object, GB_DEBUG_ENUM_CB cb);

void DEBUG_print_backtrace(STACK_BACKTRACE *bt);
void DEBUG_print_current_backtrace(void);
GB_ARRAY DEBUG_get_string_array_from_backtrace(STACK_BACKTRACE *bt);

void DEBUG_enter_eval(void);
void DEBUG_leave_eval(void);

#define PROFILE_ENTER_FUNCTION() \
	if (EXEC_profile && CP && CP->component == COMPONENT_main) \
		DEBUG.Profile.Begin(CP, FP); \

#define PROFILE_LEAVE_FUNCTION() \
	if (EXEC_profile && CP && CP->component == COMPONENT_main) \
		DEBUG.Profile.End(CP, FP); \

#endif
