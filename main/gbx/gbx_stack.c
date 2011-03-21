/***************************************************************************

  gbx_stack.c

  (c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __GBX_STACK_C

#include <sys/resource.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_alloc.h"
#include "gbx_exec.h"
#include "gb_error.h"
#include "gbx_string.h"

#include "gbx_stack.h"

// The stack grows by 4K slot (8K on 64 bits CPU)
#define STACK_INC 256 * sizeof(VALUE)

size_t STACK_size = STACK_INC;
char *STACK_base = NULL;
char *STACK_limit = NULL;
STACK_CONTEXT *STACK_frame;
int STACK_frame_count;
size_t STACK_relocate = 0;

uintptr_t STACK_process_stack_limit;

void STACK_init(void)
{
	int stack;
	struct rlimit limit;
	uintptr_t max;
	
	// Get the maximum stack size allowed
	if (getrlimit(RLIMIT_STACK, &limit))
		ERROR_panic("Cannot get stack size limit");
	
	if (limit.rlim_cur == RLIM_INFINITY)
		max = 128 << 20; // 128 Mb if there is no limit.
	else
		max = (uintptr_t)limit.rlim_cur;
	
	max -= STACK_INC * 8; // 32 Kb security (64 Kb on 64 bits OS)
	
	STACK_process_stack_limit = (uintptr_t)&stack - max;
	
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

#if DEBUG_STACK
bool STACK_check(int need)
{
  static VALUE *old = NULL;

  if (SP > old)
  {
    printf("STACK = %d bytes\n", ((char *)SP - STACK_base));
    old = SP;
  }
	
  if ((char *)(SP + need + 8) >= STACK_limit)
	{
    STACK_grow();
		return TRUE;
	}
	else
		return FALSE;
}
#endif

#if 0
void STACK_push_frame(STACK_CONTEXT *context, int need)
{
	int stack;
  //fprintf(stderr, "current_stack = %u -> %u / %u\n", current_stack, _process_stack_base - current_stack, _process_stack_max);
	
	if ((uintptr_t)&stack < _process_stack_limit)
		THROW(E_STACK);
	
	if ((char *)(SP + need + 8 + sizeof(STACK_CONTEXT)) >= STACK_limit) 
	{
		//fprintf(stderr, "**** STACK_GROW: STACK_push_frame\n");
		//THROW(E_STACK);
		STACK_grow();
	}
  
  //if (((char *)SP + sizeof(STACK_CONTEXT) * 2) >= (char *)STACK_frame)
  //  THROW(E_STACK);

  STACK_frame--;
  
  //*STACK_frame = *context;
  STACK_copy(STACK_frame, context);
  
  STACK_frame_count++;
  STACK_limit = (char *)STACK_frame;

  //fprintf(stderr, "STACK_push_frame: [%d]  PC = %p  FP = %p (%s)\n", STACK_frame_count, context->pc, context->fp,
  //  context->fp ? (context->fp->debug ? context->fp->debug->name : 0) : 0);
}

void STACK_pop_frame(STACK_CONTEXT *context)
{
  if (STACK_frame_count <= 0)
    ERROR_panic("STACK_pop_frame: Stack frame is void");

  //*context = *STACK_frame;
  STACK_copy(context, STACK_frame);
  
  STACK_frame++;
  STACK_frame_count--;
  STACK_limit = (char *)STACK_frame;

  //fprintf(stderr, "STACK_pop_frame: [%d] PC = %p  FP = %p (%s)\n", STACK_frame_count, context->pc, context->fp,
  //  context->fp ? (context->fp->debug ? context->fp->debug->name : 0) : 0);
}
#endif

bool STACK_has_error_handler(void)
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

static void relocate(STACK_CONTEXT *context)
{
	STACK_RELOCATE(context->next);
	STACK_RELOCATE(context->bp);
	STACK_RELOCATE(context->pp);
	STACK_RELOCATE(context->ep);
	STACK_RELOCATE(context->tp);
}  

void STACK_grow(void)
{
	size_t new_size = STACK_size + STACK_INC;
	char *new_base;
	size_t size;
	STACK_CONTEXT *context;
	#if DEBUG_STACK
	fprintf(stderr, "STACK_grow: before SP = %d\n", SP - (VALUE *)STACK_base);
	fprintf(stderr, "STACK_grow: before STACK_limit = %d\n", (VALUE *)STACK_limit - (VALUE *)STACK_base);
	fprintf(stderr, "STACK_grow: before STACK_frame = %d\n", (STACK_CONTEXT *)STACK_frame - (STACK_CONTEXT *)STACK_base);
	BREAKPOINT();
	#endif
	
  ALLOC_ZERO(&new_base, new_size, "STACK_grow");
	STACK_relocate = new_base - STACK_base;

	// copy the stack data
	size = (char *)SP - STACK_base;
	#if DEBUG_STACK
	fprintf(stderr, "stack size = %d ", size);
	#endif
	memcpy(new_base, STACK_base, size);

	// relocate the current context
	relocate(&EXEC_current);
	
	// relocate frames
	for (context = (STACK_CONTEXT *)STACK_limit; context < (STACK_CONTEXT *)(STACK_base + STACK_size); context++)
		relocate(context);
	
	// copy the frame data
	size = STACK_base + STACK_size - STACK_limit;
	#if DEBUG_STACK
	fprintf(stderr, "frame size = %d\n", size);
	#endif
	memcpy(&new_base[new_size - size], STACK_limit, size);

	// update limit
	//fprintf(stderr, "STACK_grow: limit = %d -> %d\n", STACK_limit - (char *)STACK_base, new_size - size);
	STACK_limit = &new_base[new_size - size];
	
	// free old stack
	FREE(&STACK_base, "STACK_grow");
	
	// update stack pointers
	STACK_frame = (STACK_CONTEXT *)STACK_limit;
	STACK_RELOCATE(SP);
	STACK_RELOCATE(EXEC_super);
	STACK_base = new_base;
	STACK_size = new_size;
	
	#if DEBUG_STACK
	fprintf(stderr, "STACK_grow: new size = %d\n", new_size / sizeof(VALUE));
	
	fprintf(stderr, "STACK_grow: after SP = %d\n", SP - (VALUE *)STACK_base);	
	fprintf(stderr, "STACK_grow: after STACK_limit = %d\n", (VALUE *)STACK_limit - (VALUE *)STACK_base);
	fprintf(stderr, "STACK_grow: after STACK_frame = %d\n", (STACK_CONTEXT *)STACK_frame - (STACK_CONTEXT *)STACK_base);
	#endif
}


