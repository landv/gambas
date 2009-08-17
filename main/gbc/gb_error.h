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

#define E_UNKNOWN     ((const char *)0)
#define E_MEMORY      ((const char *)1)
#define E_OPEN        ((const char *)2)
#define E_READ        ((const char *)3)
#define E_SYNTAX      ((const char *)4)
#define E_UNEXPECTED  ((const char *)5)

typedef
  struct {
    int code;
    char msg[MAX_ERROR_MSG + 1];
    }
  ERROR_INFO;

typedef
  struct _ERROR {
    struct _ERROR *prev;
    int code;
    jmp_buf env;
    }
  ERROR_CONTEXT;

#ifndef __GB_ERROR_C
EXTERN ERROR_INFO ERROR_info;
#endif


#define ERROR_LEAVE_DONE ((ERROR_CONTEXT *)-1)

#define TRY \
  { \
		ERROR_CONTEXT __err_context; \
    { \
     	ERROR_CONTEXT *__err = &__err_context; \
     	ERROR_enter(__err); \
     	__err->code = setjmp(__err->env); \
     	if (__err->code == 0)

#define FINALLY

#define CATCH \
			if (__err->code != 0 && !(__err->code = 0))

#define CATCH_GET(get_it) \
     	if (__err->code != 0 && (get_it = __err->code) && !(__err->code = 0))

#define END_TRY \
			if (__err->code == 0) \
				ERROR_leave(__err); \
    	else \
      	PROPAGATE(); \
  	} \
	}

#define ERROR_clear() (ERROR_info.code = 0)

char *ERROR_get(void);

void ERROR_enter(ERROR_CONTEXT *err);
void ERROR_leave(ERROR_CONTEXT *err);

void ERROR_define(const char *pattern, const char *arg[]);

void PROPAGATE() NORETURN;
void THROW(const char *code, ...) NORETURN;
void THROW_SYSTEM(int err, const char *path);

void ERROR_panic(const char *error, ...) NORETURN;

void ERROR_print(void);
void ERROR_print_at(FILE *where);

/*PUBLIC void ERROR_must_free(void *object);*/

#endif
