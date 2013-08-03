/***************************************************************************

  gb_error.h

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

#ifndef __GB_ERROR_H
#define __GB_ERROR_H

#include <errno.h>
#include <setjmp.h>

#include "gb_common.h"
#include "gb_limit.h"

//#define DEBUG_ERROR 1

#include "gb_error_common.h"

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
	E_EXTCB,
	E_SERIAL,
	E_CHILD,
	E_USER,
	E_NEMPTY,
	E_UTYPE
	};

#ifndef __GB_ERROR_C

EXTERN ERROR_CONTEXT *ERROR_current;
EXTERN ERROR_INFO ERROR_last;
EXTERN ERROR_HANDLER *ERROR_handler;
EXTERN void *ERROR_backtrace;

#if DEBUG_ERROR
EXTERN int ERROR_depth;

void ERROR_debug(const char *msg, ...);
#endif

#endif

#define ON_ERROR(_handler) \
	{ \
		ERROR_HANDLER __handler; \
		__handler.handler = (_handler); \
		__handler.prev = ERROR_handler; \
		__handler.context = ERROR_current; \
		ERROR_handler = &__handler;
		//fprintf(stderr, "%s.%d: ERROR_handler -> %p @ %p\n", __FUNCTION__, __LINE__, ERROR_handler, ERROR_current);

#define ON_ERROR_1(_handler, _arg1) \
	ON_ERROR(_handler) \
	__handler.arg1 = (intptr_t)(_arg1);

#define ON_ERROR_2(_handler, _arg1, _arg2) \
	ON_ERROR_1(_handler, _arg1) \
	__handler.arg2 = (intptr_t)(_arg2);

#define END_ERROR \
		ERROR_handler = __handler.prev; \
	}
		//fprintf(stderr, "%s.%d: ERROR_handler <- %p @ %p (%p)\n", __FUNCTION__, __LINE__, ERROR_handler, ERROR_handler ? ERROR_handler->context : NULL, ERROR_current);

const char *ERROR_get(void);

void ERROR_define(const char *pattern, char *arg[]);

void ERROR_propagate(void) NORETURN;

void THROW(int code, ...) NORETURN;
void THROW_SYSTEM(int err, const char *path);
void THROW_ILLEGAL(void) NORETURN;
void THROW_STACK(void) NORETURN;
void THROW_CLASS(void *class, char *arg1, char *arg2) NORETURN;

void ERROR_fatal(const char *error, ...) NORETURN;
void ERROR_panic(const char *error, ...) NORETURN;
void ERROR_warning(const char *warning, ...);

void ERROR_print(void);
void ERROR_print_at(FILE *where, bool msgonly, bool newline);
void ERROR_hook(void);

void ERROR_save(ERROR_INFO *save, ERROR_INFO *last);
void ERROR_restore(ERROR_INFO *save, ERROR_INFO *last);

void ERROR_clear(void);
void ERROR_reset(ERROR_INFO *info);
void ERROR_lock(void);
void ERROR_unlock(void);
void ERROR_set_last(bool bt);
void ERROR_define_last(void);
//void ERROR_deprecated(const char *msg);

void ERROR_exit(void);

#endif
