/***************************************************************************

  gb_error_common.h

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

#ifndef __GB_ERROR_COMMON_H
#define __GB_ERROR_COMMON_H

typedef
	struct {
		char code;          // Error code
		char native;        // A native method has raised an error
		char free;          // If 'msg' sould be freed
		char _reserved;
		void *cp;
		void *fp;
		void *pc;
		char *msg;
		}
	ERROR_INFO;

typedef
	struct _ERROR_CONTEXT {
		struct _ERROR_CONTEXT *prev;
		struct _ERROR_HANDLER *handler;
		ERROR_INFO info;
		jmp_buf env;
		char ret;
		}
	ERROR_CONTEXT;

typedef
	struct _ERROR_HANDLER {
		struct _ERROR_HANDLER *prev;
		ERROR_CONTEXT *context;
		void (*handler)();
		intptr_t arg1;
		intptr_t arg2;
		}
	ERROR_HANDLER;

#ifdef NO_ERROR_HANDLER
#define ERROR_handler NULL
#endif
	
#define ERROR_LEAVE_DONE ((ERROR_CONTEXT *)-1)

#if DEBUG_ERROR

#define TRY \
	{ \
		ERROR_CONTEXT __err_context; \
		{ \
			ERROR_debug("TRY %s %p\n", __FUNCTION__, &__err_context); \
			ERROR_depth++; \
			ERROR_enter(&__err_context); \
			__err_context.ret = setjmp(__err_context.env); \
			if (__err_context.ret == 0)

/*#define CATCH \
			fprintf(stderr, "%p == %p ? %d\n", ERROR_current, __err, __err->ret); \
			if (__err->ret != 0 && (__err->ret = 2))*/

#define CATCH \
			if (__err_context.ret) { ERROR_depth--; ERROR_debug("CATCH %s %p\n", __FUNCTION__, &__err_context); ERROR_depth++; } \
			if (__err_context.ret)

#define END_TRY \
			ERROR_depth--; \
			ERROR_debug("END TRY %s %p\n", __FUNCTION__, &__err_context); \
			ERROR_depth++; \
			ERROR_leave(&__err_context); \
		} \
	}

#define PROPAGATE() fprintf(stderr, "PROPAGATE %s\n", __FUNCTION__), ERROR_propagate()

#define ERROR_enter(_err) \
do { \
	(_err)->prev = ERROR_current; \
	(_err)->info.code = 0; \
	(_err)->info.native = 0; \
	(_err)->handler = ERROR_handler; \
	ERROR_current = (_err); \
} while(0)

#define ERROR_leave(_err) \
do { \
	ERROR_CONTEXT *_prev = (_err); \
	ERROR_depth--; \
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

#define ERROR_enter(_err) \
do { \
	_err->prev = ERROR_current; \
	_err->info.code = 0; \
	_err->info.native = 0; \
	_err->handler = ERROR_handler; \
	ERROR_current = _err; \
} while(0)

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
				ERROR_current->info.native = FALSE; \
			} \
		} \
		else \
			ERROR_reset(&_prev->info); \
		_prev->prev = ERROR_LEAVE_DONE; \
	} \
} while(0)

#endif

#define ERROR (&__err_context)

#define ERROR_in_catch(_err) ((_err)->ret)

#endif

