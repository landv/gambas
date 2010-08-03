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

#include "gb_common.h"
#include "gb_limit.h"

//#define DEBUG_ERROR 1

enum {
  E_ABORT = -2,
  E_CUSTOM = -1,
  E_UNKNOWN = 0,
  E_MEMORY,
  E_CLASS,
  E_STACK,
  E_NEPARAM,
  E_TMPARAM,
  E_TYPE,
  E_OVERFLOW,
  E_ILLEGAL,
  E_NFUNC,
  E_CSTATIC,
  E_NSYMBOL,
  E_NOBJECT,
  E_NULL,
  E_STATIC,
  E_NREAD,
  E_NWRITE,
  E_NPROPERTY,
  E_NRETURN,
  E_MATH,
  E_ARG,
  E_BOUND,
  E_NDIM,
  E_NARRAY,
  E_MAIN,
  E_NNEW,
  E_ZERO,
  E_LIBRARY,
  E_EVENT,
  E_IOBJECT,
  E_ENUM,
  E_UCONV,
  E_CONV,
  E_DATE,
  E_BADPATH,
  E_OPEN,
  E_PROJECT,
  E_FULL,
  E_EXIST,
  E_EOF,
  E_FORMAT,
  E_DYNAMIC,
  E_SYSTEM,
  E_ACCESS,
  E_TOOLONG,
  E_NEXIST,
  E_DIR,
  E_READ,
  E_WRITE,
  E_NDIR,
  E_REGEXP,
  E_ARCH,
  E_REGISTER,
  E_CLOSED,
  E_VIRTUAL,
  E_STOP,
  E_STRING,
  E_EVAL,
  E_LOCK,
  E_PARENT,
  E_EXTLIB,
  E_EXTSYM,
  E_BYREF,
  E_OVERRIDE,
	E_VKEY,
	E_SARRAY,
	E_EXTCB
  };

typedef
  struct {
  	short code;
    bool free;
    void *cp;
    void *fp;
    void *pc;
    char *msg;
		int bt_count;
    }
  ERROR_INFO;

typedef
	struct _ERROR_CONTEXT {
		struct _ERROR_CONTEXT *prev;
		jmp_buf env;
		int ret;
		struct _ERROR_HANDLER *handler;
		ERROR_INFO info;
		}
	ERROR_CONTEXT;

typedef
	struct _ERROR_HANDLER {
		struct _ERROR_HANDLER *prev;
		ERROR_CONTEXT *context;
		void (*handler)();
		}
	ERROR_HANDLER;
	
#ifndef __GB_ERROR_C
EXTERN ERROR_CONTEXT *ERROR_current;
EXTERN ERROR_INFO ERROR_last;
EXTERN ERROR_HANDLER *ERROR_handler;
#endif

#define ERROR_LEAVE_DONE ((ERROR_CONTEXT *)-1)

#if DEBUG_ERROR

#define TRY \
  { \
		ERROR_CONTEXT __err_context; \
    { \
     	ERROR_CONTEXT *__err = &__err_context; \
     	fprintf(stderr, "TRY %s\n", __FUNCTION__); \
     	ERROR_enter(__err); \
     	__err->ret = setjmp(__err->env); \
     	if (__err->ret == 0)

/*#define CATCH \
	  	fprintf(stderr, "%p == %p ? %d\n", ERROR_current, __err, __err->ret); \
			if (__err->ret != 0 && (__err->ret = 2))*/

#define CATCH \
			if (__err->ret) fprintf(stderr, "CATCH %p\n", __err); \
			if (__err->ret)

#define END_TRY \
     	fprintf(stderr, "END TRY %s\n", __FUNCTION__); \
			ERROR_leave(__err); \
  	} \
	}

#define PROPAGATE() fprintf(stderr, "PROPAGATE %s\n", __FUNCTION__), ERROR_propagate()

#else /* DEBUG_ERROR */

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

#define PROPAGATE() ERROR_propagate()

#endif

#define ON_ERROR(_handler) \
	{ \
		ERROR_HANDLER __handler; \
		__handler.handler = (_handler); \
		__handler.prev = ERROR_handler; \
		__handler.context = ERROR_current; \
		ERROR_handler = &__handler;
		//fprintf(stderr, "%s.%d: ERROR_handler -> %p @ %p\n", __FUNCTION__, __LINE__, ERROR_handler, ERROR_current);

#define END_ERROR \
		ERROR_handler = __handler.prev; \
	}
		//fprintf(stderr, "%s.%d: ERROR_handler <- %p @ %p (%p)\n", __FUNCTION__, __LINE__, ERROR_handler, ERROR_handler ? ERROR_handler->context : NULL, ERROR_current);

#define ERROR __err

const char *ERROR_get(void);

//void ERROR_enter(ERROR_CONTEXT *err);

#define ERROR_enter(_err) \
do { \
  _err->prev = ERROR_current; \
  _err->info.code = 0; \
  _err->handler = ERROR_handler; \
  ERROR_current = _err; \
} while(0)


//void ERROR_leave(ERROR_CONTEXT *err);

#define ERROR_leave(_err) \
do { \
	ERROR_CONTEXT *_prev = (_err); \
  if (_prev->prev != ERROR_LEAVE_DONE) \
	{ \
		ERROR_current = _prev->prev; \
		if (ERROR_current) \
		{ \
			if (_prev->info.code) \
			{ \
				ERROR_reset(&ERROR_current->info); \
				ERROR_current->info = _prev->info; \
			} \
		} \
		else \
			ERROR_reset(&_prev->info); \
		_prev->prev = ERROR_LEAVE_DONE; \
	} \
} while(0)

void ERROR_define(const char *pattern, char *arg[]);

void ERROR_propagate(void) NORETURN;
void THROW(int code, ...) NORETURN;
void THROW_SYSTEM(int err, const char *path);
void THROW_ILLEGAL(void) NORETURN;

void ERROR_panic(const char *error, ...) NORETURN;

void ERROR_print(void);
void ERROR_print_at(FILE *where, bool msgonly, bool newline);

void ERROR_save(ERROR_INFO *save);
void ERROR_restore(ERROR_INFO *save);

void ERROR_clear(void);
void ERROR_reset(ERROR_INFO *info);
void ERROR_lock(void);
void ERROR_unlock(void);
void ERROR_set_last(void);
void ERROR_deprecated(const char *msg);

#define ERROR_exit() ERROR_reset(&ERROR_last)

#endif
