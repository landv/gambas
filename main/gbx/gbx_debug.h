/***************************************************************************

  debug.h

  What are we going to debug ?

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

#ifndef __GBX_DEBUG_H
#define __GBX_DEBUG_H

#include "gbx_class.h"
#include "gb_pcode.h"
#include "../lib/debug/gb.debug.h"

/* Object referencement debugging */
#define DEBUG_REF      0
/* String referencement debugging */
#define DEBUG_STRING   0
/* Bytecode debugging */
#define DEBUG_PCODE    0
/* Stack debugging */
#define DEBUG_STACK    0
/* Class loading debugging */
#define DEBUG_LOAD     0
/* Error management debugging */
#define DEBUG_ERROR    0
/* Event debugging */
#define DEBUG_EVENT    0
/* Component management debugging */
#define DEBUG_COMP     0

typedef
	struct {
		void *pc;
		void *cp;
		void *fp;
	}
	DEBUG_BACKTRACE;

#ifndef __GBX_DEBUG_C

EXTERN DEBUG_INTERFACE DEBUG;
EXTERN DEBUG_INFO *DEBUG_info;

#endif

void DEBUG_init(void);
void DEBUG_exit(void);
const char *DEBUG_get_position(CLASS *cp, FUNCTION *fp, PCODE *pc);
const char *DEBUG_get_current_position(void);
void DEBUG_where(void);
bool DEBUG_get_value(const char *sym, int len, GB_VARIANT *ret);
int DEBUG_can_be_used_like_an_array(void *object, CLASS *class);
DEBUG_BACKTRACE *DEBUG_backtrace();
GB_ARRAY DEBUG_get_string_array_from_backtrace(DEBUG_BACKTRACE *bt);

#define DEBUG_free_backtrace(_bt) ARRAY_delete(_bt)

#endif
