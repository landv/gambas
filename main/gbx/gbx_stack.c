/***************************************************************************

  stack.c

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

#define __GBX_STACK_C

#include "gb_common.h"
#include "gb_error.h"
#include "gb_alloc.h"
#include "gbx_exec.h"
#include "gb_error.h"
#include "gbx_string.h"

#include "gbx_stack.h"

size_t STACK_size = 1024 * sizeof(VALUE);
char *STACK_base = NULL;
char *STACK_limit = NULL;
STACK_CONTEXT *STACK_frame;
int STACK_frame_count;

void STACK_init(void)
{
	//fprintf(stderr, "STACK_size = %ld\n", STACK_size);
  ALLOC_ZERO(&STACK_base, STACK_size, "STACK_init");

  STACK_limit = (STACK_base + STACK_size);
  STACK_frame = (STACK_CONTEXT *)STACK_limit;
  STACK_frame_count = 0;

  SP = (VALUE *)STACK_base;
}


void STACK_exit(void)
{
  if (STACK_base)
    FREE(&STACK_base, "STACK_exit");
}

void STACK_check(int need)
{
#if DEBUG_STACK
  static VALUE *old = NULL;

  if (SP > old)
  {
    printf("STACK = %d bytes\n", ((char *)SP - STACK_base));
    old = SP;
  }
#endif

  if ((char *)(SP + need + 8) >= STACK_limit)
    THROW(E_STACK);
}


void STACK_push_frame(STACK_CONTEXT *context)
{
  if (((char *)SP + sizeof(STACK_CONTEXT) * 2) >= (char *)STACK_frame)
    THROW(E_STACK);

  STACK_frame--;
  *STACK_frame = *context;
  STACK_frame_count++;
  STACK_limit = (char *)STACK_frame;

  //fprintf(stderr, "STACK_push_frame: [%d]  PC = %p  FP = %p (%s)\n", STACK_frame_count, context->pc, context->fp,
  //  context->fp ? (context->fp->debug ? context->fp->debug->name : 0) : 0);
}


void STACK_pop_frame(STACK_CONTEXT *context)
{
  if (STACK_frame_count <= 0)
    ERROR_panic("STACK_pop_frame: Stack frame is void");

  *context = *STACK_frame;
  STACK_frame++;
  STACK_frame_count--;
  STACK_limit = (char *)STACK_frame;

  //fprintf(stderr, "STACK_pop_frame: [%d] PC = %p  FP = %p (%s)\n", STACK_frame_count, context->pc, context->fp,
  //  context->fp ? (context->fp->debug ? context->fp->debug->name : 0) : 0);
}

boolean STACK_has_error_handler(void)
{
  int i;

  for (i = 0; i < STACK_frame_count; i++)
    if (STACK_frame[i].ec != NULL)
      return TRUE;

  return FALSE;
}

STACK_CONTEXT *STACK_get_frame(int frame)
{
	if (frame >= 0 && frame < STACK_frame_count)
		return &STACK_frame[frame];
	else
		return NULL;
}
