/***************************************************************************

  stack.h

  Stack management

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

#ifndef __GBX_STACK_H
#define __GBX_STACK_H

#include "gbx_value.h"
#include "gb_pcode.h"


typedef
  struct _stack_context {
  	struct _stack_context *next;
    VALUE *bp;        /* local variables */
    VALUE *pp;        /* local parameters */
    CLASS *cp;        /* current class */
    char *op;         /* current object */
    VALUE *ep;        /* error pointer */
    FUNCTION *fp;     /* current function */
    PCODE *pc;        /* instruction */
    PCODE *ec;        /* instruction if error */
    PCODE *et;        /* TRY save */
    PCODE *tc;        /* Last break in the function */
    VALUE *tp;        /* Stack at the last break in the function */
    }
  PACKED STACK_CONTEXT;

#ifndef __GBX_STACK_C

EXTERN char *STACK_base;
EXTERN long STACK_size;
EXTERN char *STACK_limit;

EXTERN int STACK_frame_count;
EXTERN STACK_CONTEXT *STACK_frame;

#endif

PUBLIC void STACK_init(void);
PUBLIC void STACK_exit(void);
PUBLIC void STACK_check(int need);

PUBLIC void STACK_push_frame(STACK_CONTEXT *context);
PUBLIC void STACK_pop_frame(STACK_CONTEXT *context);
PUBLIC bool STACK_has_error_handler(void);
PUBLIC STACK_CONTEXT *STACK_get_frame(int frame);

#define STACK_get_current() ((STACK_frame_count > 0) ? STACK_frame : NULL)


#endif /* */
