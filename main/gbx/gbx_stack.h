/***************************************************************************

  gbx_stack.h

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GBX_STACK_H
#define __GBX_STACK_H

#include "gbx_value.h"
#include "gb_pcode.h"

typedef
	struct {
		void *cp;
		void *fp;
		void *pc;
	}
	PACKED
	STACK_BACKTRACE;

//#define MAX_GOSUB_LEVEL 119

typedef
	struct _STACK_GOSUB {
		ushort pc;
		VALUE *ctrl;
	}
	STACK_GOSUB;

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
    STACK_GOSUB *gp;  /* GOSUB pointer */
    }
  PACKED STACK_CONTEXT;

#ifndef __GBX_STACK_C

EXTERN char *STACK_base;
EXTERN size_t STACK_size;
EXTERN char *STACK_limit;
EXTERN size_t STACK_relocate;

EXTERN int STACK_frame_count;
EXTERN STACK_CONTEXT *STACK_frame;

EXTERN uintptr_t STACK_process_stack_limit;

#endif

void STACK_init(void);
void STACK_exit(void);

#if DEBUG_STACK
bool STACK_check(int need);
#else
#define STACK_check(_need) (((char *)(SP + (_need) + 8) >= STACK_limit) ? STACK_grow(), 1 : 0)

#endif

void STACK_push_frame(STACK_CONTEXT *context, int check);
void STACK_pop_frame(STACK_CONTEXT *context);

bool STACK_has_error_handler(void);

STACK_BACKTRACE *STACK_get_backtrace(void);
#define STACK_free_backtrace(_backtrace) FREE(_backtrace, "STRACK_free_backtrace")
#define STACK_backtrace_is_end(_bt) ((((intptr_t)((_bt)->cp)) & 1) != 0)
#define STACK_backtrace_set_end(_bt) ((_bt)->cp = (void *)(((intptr_t)((_bt)->cp)) | 1))
#define STACK_backtrace_clear_end(_bt) ((_bt)->cp = (void *)(((intptr_t)((_bt)->cp)) & ~1))


STACK_CONTEXT *STACK_get_frame(int frame);
void STACK_grow(void);

void STACK_free_gosub_stack(STACK_GOSUB *gosub);

#define STACK_get_previous_pc() ((STACK_frame_count <= 0) ? NULL : STACK_frame->pc)

#define STACK_get_current() ((STACK_frame_count > 0) ? STACK_frame : NULL)

/*#define STACK_copy(_dst, _src) \
  (_dst)->next = (_src)->next; \
	(_dst)->bp = (_src)->bp; \
	(_dst)->pp = (_src)->pp; \
	(_dst)->cp = (_src)->cp; \
	(_dst)->op = (_src)->op; \
	(_dst)->ep = (_src)->ep; \
	(_dst)->fp = (_src)->fp; \
	(_dst)->pc = (_src)->pc; \
	(_dst)->ec = (_src)->ec; \
	(_dst)->et = (_src)->et; \
	(_dst)->tc = (_src)->tc; \
	(_dst)->tp = (_src)->tp;*/

#define STACK_copy(_dst, _src) *(_dst) = *(_src)

#define STACK_RELOCATE(_ptr) if (_ptr) _ptr = (void *)((char *)_ptr + STACK_relocate)
	
#define STACK_push_frame(_context, _need) \
({ \
	int stack; \
	\
	if ((uintptr_t)&stack < STACK_process_stack_limit) \
		THROW(E_STACK); \
	\
	if ((char *)(SP + (_need) + 8 + sizeof(STACK_CONTEXT)) >= STACK_limit) \
		STACK_grow(); \
	\
  STACK_frame--; \
  \
  STACK_copy(STACK_frame, _context); \
  \
  STACK_frame_count++; \
  STACK_limit = (char *)STACK_frame; \
})

#define STACK_pop_frame(_context) \
({ \
  if ((_context)->gp) STACK_free_gosub_stack((_context)->gp); \
  STACK_copy(_context, STACK_frame); \
  STACK_frame++; \
  STACK_frame_count--; \
  STACK_limit = (char *)STACK_frame; \
})

#endif
