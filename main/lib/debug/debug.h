/***************************************************************************

  debug.h

  (c) 2000-2007 Benoit Minisini <gambas@freesurf.fr>

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

#ifndef __DEBUG_H
#define __DEBUG_H

#include "gambas.h"
#include "main.h"
#include "gb.debug.h"
#include "gb_pcode.h"
#include "gbx_value.h"

typedef
  struct {
    int id;
    FUNCTION *func;
    PCODE *addr;
    CLASS *class;
    ushort line;
    VALUE *bp;
    FUNCTION *fp;
    }
  DEBUG_BREAK;

typedef
  enum {
    TC_NONE     = 0,
    TC_STEP     = 1,
    TC_NEXT     = 2,
    TC_GO       = 3,
    TC_FROM     = 4
    }
  DEBUG_TYPE;

typedef
  struct {
    char *pattern;
    DEBUG_TYPE type;
    void (*func)(const char *);
    boolean loop;
    }
  DEBUG_COMMAND;


#ifndef __GBX_DEBUG_C
EXTERN DEBUG_INFO DEBUG_info;
EXTERN GB_DEBUG_INTERFACE *DEBUG_interface;
EXTERN char DEBUG_buffer[];
#endif

#define DEBUG_BUFFER_MAX 255

#define GB_DEBUG (*DEBUG_interface)

#define DEBUG_WELCOME   "Welcome to the Gambas debugger.\n"

PUBLIC void DEBUG_breakpoint(int id);
PUBLIC void DEBUG_main(boolean error);

PUBLIC const char *DEBUG_get_position(CLASS *cp, FUNCTION *fp, PCODE *pc);
PUBLIC const char *DEBUG_get_current_position(void);
PUBLIC void DEBUG_init_breakpoints(CLASS *class);

PUBLIC void DEBUG_break_on_next_line(void);

PUBLIC DEBUG_INFO *DEBUG_init(GB_DEBUG_INTERFACE *debug, bool fifo, const char*fifo_name);
PUBLIC void DEBUG_exit(void);
PUBLIC void DEBUG_welcome(void);
PUBLIC void DEBUG_where(void);
PUBLIC void DEBUG_backtrace(FILE *out);

#endif
