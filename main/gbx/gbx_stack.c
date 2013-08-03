/***************************************************************************

  gbx_stack.c

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

#define __GBX_STACK_C

#include <sys/resource.h>
#include <sys/mman.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_alloc.h"
#include "gbx_exec.h"
#include "gb_error.h"
#include "gbx_string.h"
#include "gbx_stack.h"

char *STACK_base = NULL;
size_t STACK_size;
char *STACK_limit = NULL;
STACK_CONTEXT *STACK_frame;
int STACK_frame_count;

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
		max = 64 << 20; // 64 Mb if there is no limit.
	else
		max = (uintptr_t)limit.rlim_cur;
	
	STACK_size = max - sizeof(VALUE) * 256; // some security
	#if DEBUG_STACK
		fprintf(stderr, "STACK_size = %ld\n", STACK_size);
	#endif
	
	STACK_base = mmap(NULL, STACK_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);

	//fprintf(stderr, "Stack = %p %ld\n", STACK_base, STACK_size);

	STACK_process_stack_limit = (uintptr_t)&stack - max + 65536;
	
  STACK_limit = (STACK_base + STACK_size);
  STACK_frame = (STACK_CONTEXT *)STACK_limit;
  STACK_frame_count = 0;

  SP = (VALUE *)STACK_base;
}


void STACK_exit(void)
{
  if (STACK_base)
	{
		munmap(STACK_base, STACK_size);
    STACK_base = NULL;
	}
}

#if DEBUG_STACK
bool STACK_check(int need)
{
  static VALUE *old = NULL;

	fprintf(stderr, "STACK_check: SP = %d need = %d limit = %d\n", (int)(((char *)SP - STACK_base) / sizeof(VALUE)), need, (int)((STACK_limit - STACK_base) / sizeof(VALUE)));
	
  if (SP > old)
  {
    fprintf(stderr, "**** STACK_check: -> %ld bytes\n", ((char *)SP - STACK_base));
    old = SP;
  }
	
  if (((char *)(SP + need) + sizeof(STACK_CONTEXT)) >= STACK_limit)
	{
    THROW_STACK();
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

STACK_BACKTRACE *STACK_get_backtrace(void)
{
	STACK_BACKTRACE *bt, *pbt;
	int i;
	
	if (STACK_frame_count == 0)
		return NULL;
	
	ALLOC(&bt, sizeof(STACK_BACKTRACE) * (1 + STACK_frame_count));
	
	bt->cp = CP;
	bt->fp = FP;
	bt->pc = PC;
	
	for (i = 0, pbt = &bt[1]; i < STACK_frame_count; i++, pbt++)
	{
		pbt->cp = STACK_frame[i].cp;
		pbt->fp = STACK_frame[i].fp;
		pbt->pc = STACK_frame[i].pc;
	}
	
	// Mark the end of the backtrace
	pbt--;
	STACK_backtrace_set_end(pbt);
	 
	return bt;
}

/*void STACK_free_gosub_stack(STACK_GOSUB *gosub)
{
	int i, j;
	STACK_GOSUB *p;
	
	if (FP->n_ctrl)
	{
		for (i = 0, p = gosub; i < ARRAY_count(gosub); i++, p++)
		{
			for (j = 0; j < FP->n_ctrl; j++)
				RELEASE(&p->ctrl[j]);
			
			FREE(&p->ctrl, "STACK_free_gosub_stack");
		}
	}
	
	ARRAY_delete(&gosub);
}*/
