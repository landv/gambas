/***************************************************************************

  error.h

  Error management

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

#ifndef __GB_ERROR_H
#define __GB_ERROR_H

#include <errno.h>
#include <setjmp.h>

#include "gb_limit.h"

#define E_MEMORY    "Out of memory"
#define E_SYNTAX    "Syntax error"

typedef
  struct {
    long code;
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


#define ERROR_LEAVE_DONE ((ERROR_CONTEXT *)-1)

#define TRY                                 \
  {                                         \
     ERROR_CONTEXT *_err = alloca(sizeof(ERROR_CONTEXT));    \
     ERROR_enter(_err);                      \
     _err->code = setjmp(_err->env);          \
     if (_err->code == 0)

#define FINALLY

#define CATCH \
     if (_err->code != 0 && !(_err->code = 0))

#define CATCH_GET(get_it) \
     if (_err->code != 0 && (get_it = _err->code) && !(_err->code = 0))

#define END_TRY          \
    if (_err->code == 0)  \
      ERROR_leave(_err);  \
    else                 \
      PROPAGATE();  \
  }

#define MAX_MUST_FREE 8


PUBLIC void ERROR_clear(void);
PUBLIC char *ERROR_get(void);

PUBLIC void ERROR_enter(ERROR_CONTEXT *err);
PUBLIC void ERROR_leave(ERROR_CONTEXT *err);

PUBLIC void PROPAGATE() NORETURN;
/*PUBLIC void ERROR(long code, ...);*/
PUBLIC void THROW(const char *msg) NORETURN;
PUBLIC void THROW2(const char *pattern, const char *msg) NORETURN;

PUBLIC void ERROR_panic(char *error, ...) NORETURN;

PUBLIC void ERROR_print(void);
PUBLIC void ERROR_print_at(FILE *where);

/*PUBLIC void ERROR_must_free(void *object);*/

#endif
