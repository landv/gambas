/***************************************************************************

  gb_error.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GB_ERROR_H
#define __GB_ERROR_H

#include <errno.h>
#include <setjmp.h>

#include "gb_limit.h"

#define E_MEMORY    "Out of memory"
#define E_SYNTAX    "Syntax error"

typedef
  struct {
  	short code;
    bool free;
    void *cp;
    void *fp;
    void *pc;
    //char msg[MAX_ERROR_MSG + 1];
    char *msg;
		int bt_count;
    }
  ERROR_INFO;

typedef
  struct _ERROR {
    struct _ERROR *prev;
    jmp_buf env;
    int ret;
    ERROR_INFO info;
    }
  ERROR_CONTEXT;

#ifndef __GB_ERROR_C
EXTERN ERROR_CONTEXT *ERROR_current;
#endif

#define ERROR_LEAVE_DONE ((ERROR_CONTEXT *)-1)

#define TRY \
  { \
		ERROR_CONTEXT __err_context; \
    { \
     	ERROR_CONTEXT *__err = &__err_context; \
     	ERROR_enter(__err); \
     	__err->ret = setjmp(__err->env); \
     	if (__err->ret == 0)

#define CATCH \
			else

#define END_TRY \
			ERROR_leave(__err); \
  	} \
	}

#define ERROR_enter(_err) \
do { \
  _err->prev = ERROR_current; \
  _err->info.code = 0; \
  ERROR_current = _err; \
} while(0)


#define ERROR_leave(_err) \
do { \
  if (_err->prev != ERROR_LEAVE_DONE) \
	{ \
		ERROR_current = _err->prev; \
		if (ERROR_current) \
		{ \
			if (_err->info.code) \
			{ \
				ERROR_reset(&ERROR_current->info); \
				ERROR_current->info = _err->info; \
			} \
		} \
		else \
			ERROR_reset(&_err->info); \
    \
		_err->prev = ERROR_LEAVE_DONE; \
	} \
} while(0)

#define ERROR __err

#define PROPAGATE() ERROR_propagate()

void ERROR_clear(void);
char *ERROR_get(void);
void ERROR_reset(ERROR_INFO *info);

//void ERROR_enter(ERROR_CONTEXT *err);
//void ERROR_leave(ERROR_CONTEXT *err);

void PROPAGATE() NORETURN;
void THROW(const char *msg) NORETURN;
void THROW2(const char *pattern, const char *msg) NORETURN;

void ERROR_panic(const char *error, ...) NORETURN;

void ERROR_print(void);
void ERROR_print_at(FILE *where);

#endif
