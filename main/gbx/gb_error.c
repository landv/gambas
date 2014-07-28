/***************************************************************************

	gb_error.c

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

#define __GB_ERROR_C

#include "gb_common.h"
#include <stdarg.h>
#include <ctype.h>

#include "gb_buffer.h"
#include "gbx_debug.h"
#include "gbx_exec.h"
#include "gbx_api.h"
#include "gbx_stack.h"
#include "gbx_project.h"
#include "gb_error.h"

//#define DEBUG_ERROR 1

ERROR_CONTEXT *ERROR_current = NULL;
ERROR_INFO ERROR_last = { 0 };
void *ERROR_backtrace = NULL;
ERROR_HANDLER *ERROR_handler = NULL;
#if DEBUG_ERROR
int ERROR_depth = 0;
#endif

static int _lock = 0;

static const char *const _message[72] =
{
	/*  0 E_UNKNOWN */ "Unknown error",
	/*  1 E_MEMORY */ "Out of memory",
	/*  2 E_CLASS */ ".3Cannot load class '&1': &2&3",
	/*  3 E_STACK */ "Stack overflow",
	/*  4 E_NEPARAM */ "Not enough arguments",
	/*  5 E_TMPARAM */ "Too many arguments",
	/*  6 E_TYPE */ ".2Type mismatch: wanted &1, got &2 instead",
	/*  7 E_OVERFLOW */ "Overflow",
	/*  8 E_ILLEGAL */ "Illegal instruction",
	/*  9 E_NFUNC */ "Not a function",
	/* 10 E_CSTATIC */ ".1Class '&1' is not creatable",
	/* 11 E_NSYMBOL */ ".2Unknown symbol '&2' in class '&1'",
	/* 12 E_NOBJECT */ "Not an object",
	/* 13 E_NULL */ "Null object",
	/* 14 E_STATIC */ ".2'&1.&2' is static",
	/* 15 E_NREAD */ ".2'&1.&2' is write only",
	/* 16 E_NWRITE */ ".2'&1.&2' is read only",
	/* 17 E_NPROPERTY */ ".2'&1.&2' is not a property",
	/* 18 E_NRETURN */ "No return value",
	/* 19 E_MATH*/ "Mathematic error",
	/* 20 E_ARG */ "Bad argument",
	/* 21 E_BOUND */ "Out of bounds",
	/* 22 E_NDIM */ "Bad number of dimensions",
	/* 23 E_NARRAY */ "Not an array",
	/* 24 E_MAIN */ "No startup method",
	/* 25 E_NNEW */ "No instantiation method",
	/* 26 E_ZERO */ "Division by zero",
	/* 27 E_LIBRARY */ ".2Cannot load component '&1': &2",
	/* 28 E_EVENT */ ".3Bad event handler in &1.&2(): &3",
	/* 29 E_IOBJECT */ "Invalid object",
	/* 30 E_ENUM */ "Not an enumeration",
	/* 31 E_UCONV */ "Unsupported string conversion",
	/* 32 E_CONV */ "Bad string conversion",
	/* 33 E_DATE */ "Invalid date",
	/* 34 E_BADPATH */ "Invalid path",
	/* 35 E_OPEN */ ".2Cannot open file '&1': &2",
	/* 36 E_PROJECT */ ".2Bad project file: line &1: &2",
	/* 37 E_FULL */ "Device is full",
	/* 38 E_EXIST */ "File already exists",  /* &1 */
	/* 39 E_EOF */ "End of file",
	/* 40 E_FORMAT */ "Bad format string",
	/* 41 E_DYNAMIC */ ".2'&1.&2' is not static",
	/* 42 E_SYSTEM */ ".2System error #&1: &2",
	/* 43 E_ACCESS */ "Access forbidden",
	/* 44 E_TOOLONG */ "File name is too long",
	/* 45 E_NEXIST */ "File or directory does not exist", /* &1 */
	/* 46 E_DIR */ "File is a directory", /* &1 */
	/* 47 E_READ */ "Read error",
	/* 48 E_WRITE */ "Write error",
	/* 49 E_NDIR */ ".1Not a directory: &1",
	/* 50 E_REGEXP */ ".1Bad regular expression: &1",
	/* 51 E_ARCH */ ".2Bad archive: &1: &2",
	/* 52 E_REGISTER */ ".1Cannot register class '&1'",
	/* 53 E_CLOSED */ "Stream is closed",
	/* 54 E_VIRTUAL */ "Bad use of virtual class",
	/* 55 E_STOP */ "STOP instruction encountered",
	/* 56 E_STRING */ "Too many simultaneous new strings",
	/* 57 E_EVAL */ ".1Bad expression: &1",
	/* 58 E_LOCK */ "File is locked",
	/* 59 E_PARENT */ "No parent class",
	/* 60 E_EXTLIB */ ".2Cannot find dynamic library '&1': &2",
	/* 61 E_EXTSYM */ ".2Cannot find symbol '&2' in dynamic library '&1'",
	/* 62 E_BYREF */ "Argument cannot be passed by reference",
	/* 63 E_OVERRIDE */ ".3'&1.&2' is incorrectly overridden in class '&3'",
	/* 64 E_NKEY */ "Void key",
	/* 65 E_SARRAY */ "Embedded array",
	/* 66 E_EXTCB */ ".1Cannot create callback: &1",
	/* 67 E_SERIAL */ "Serialization error",
	/* 68 E_CHILD */ ".2Cannot run child process: &1&2",
	/* 69 E_USER */ "Unknown user or group",
	/* 70 E_NEMPTY */ "Directory is not empty",
	/* 71 E_UTYPE */ "Unsupported datatype"
};

#if DEBUG_ERROR
void ERROR_debug(const char *msg, ...)
{
	int i;
	va_list args;
	ERROR_CONTEXT *err;

	va_start(args, msg);

	for (i = 0; i < ERROR_depth; i++)
		fprintf(stderr, "- ");

	vfprintf(stderr, msg, args);
	
	fprintf(stderr, "\t\t\t\t\t\t\t\t\t");
	DEBUG_where();
	err = ERROR_current;
	while (err)
	{
		fprintf(stderr, "[%p] -> ", err);
		err = err->prev;
	}
	fprintf(stderr, "NULL\n");
	
	va_end(args);
}
#endif

void ERROR_lock()
{
	_lock++;
}


void ERROR_unlock()
{
	_lock--;
}

void ERROR_reset(ERROR_INFO *info)
{
	if (!info->code)
		return;
	
	info->code = 0;
	if (info->free)
	{
		STRING_unref(&info->msg);
		info->free = FALSE;
	}
	info->msg = NULL;
}

void ERROR_clear()
{
	if (_lock)
	{
		#if DEBUG_ERROR
		ERROR_debug("ERROR_clear: (%p) *LOCKED*\n", ERROR_current);
		#endif
		return;
	}
	
	#if DEBUG_ERROR
	fprintf(stderr, "ERROR_clear: (%p)\n", ERROR_current);
	#endif
	ERROR_reset(&ERROR_current->info);
}

#if 0
void ERROR_enter(ERROR_CONTEXT *err)
{
	err->prev = ERROR_current;
	err->info.code = 0;
	//err->info.free = FALSE;
	//err->info.msg = NULL;
	//err->info.backtrace = NULL;
	
	ERROR_current = err;

	#if DEBUG_ERROR
	fprintf(stderr, "ERROR_enter: (%p)\n", ERROR_current);
	fprintf(stderr, ">> ERROR_enter");
	{
		ERROR_CONTEXT *e = err;
		while (e)
		{
			fprintf(stderr, " -> %p", e);
			e = e->prev;
		}
		fprintf(stderr, "\n");
	}
	#endif
}
#endif

#if 0
void ERROR_leave(ERROR_CONTEXT *err)
{
	if (err->prev == ERROR_LEAVE_DONE)
		return;
		
	#if DEBUG_ERROR
	fprintf(stderr, "<< ERROR_leave");
	{
		ERROR_CONTEXT *e = err;
		while (e)
		{
			fprintf(stderr, " -> %p", e);
			e = e->prev;
		}
		fprintf(stderr, " : %d %s\n", err->info.code, err->info.msg);
	}
	#endif
	
	//if (!err->prev)
	//	BREAKPOINT();
	
	ERROR_current = err->prev;
	
	if (ERROR_current)
	{
		#if DEBUG_ERROR
		fprintf(stderr, "ERROR_leave: (%p)\n", ERROR_current);
		#endif
		if (err->info.code)
		{
			ERROR_reset(&ERROR_current->info);
			ERROR_current->info = err->info;
		}
	}
	else
		ERROR_reset(&err->info);

	err->prev = ERROR_LEAVE_DONE;
	//ERROR_reset(err);
}
#endif

void ERROR_propagate()
{
	ERROR_HANDLER *ph, *prev;
	#if DEBUG_ERROR
	ERROR_debug("ERROR_propagate: %p %d %s (ret = %d)\n", ERROR_current, ERROR_current->info.code, ERROR_current->info.msg, ERROR_current->ret);
	#endif
	
	//fprintf(stderr, "ERROR_propagate: %p\n", ERROR_handler);

	if (ERROR_in_catch(ERROR_current))
		ERROR_leave(ERROR_current);
	
	while (ERROR_handler)
	{
		ph = ERROR_handler;
		if (ERROR_current && ERROR_current->handler == ph)
			break;
		
		//fprintf(stderr, "ERROR_propagate: %p @ %p (%p)\n", ERROR_handler, ERROR_handler->context, ERROR_current);
		prev = ph->prev;
		(*ph->handler)(ph->arg1, ph->arg2);
		ERROR_handler = prev;
	}
	
	longjmp(ERROR_current->env, 1);
}



const char *ERROR_get(void)
{
	/*
	if (code > 0 && code < 256)
		return strerror(code);
	else
		return ERROR_Message[code - 256];
	*/
	return strerror(errno);
}

static int get_message_length(const char *pattern, char *arg[], int narg)
{
	int len;
	int i;
	
	len = strlen(pattern) + narg;
	for (i = 0; i < narg; i++)
		len += strlen(arg[i]);
	
	if (!EXEC_debug)
		len -= narg * 3;

	return len;
}

void ERROR_define(const char *pattern, char *arg[])
{
	uchar c;
	char *msg = NULL;
	int len;
	int narg = 0;

	ERROR_clear();

	if ((intptr_t)pattern >= 0 && (intptr_t)pattern < 256)
	{
		ERROR_current->info.code = (int)(intptr_t)pattern;
		pattern = _message[(int)(intptr_t)pattern];
		if (*pattern == '.')
		{
			narg = pattern[1] - '0';
			pattern += 2;
		}
	}
	else if ((intptr_t)pattern == E_ABORT)
	{
		ERROR_current->info.code = E_ABORT;
		pattern = "";
	}
	else
	{
		ERROR_current->info.code = E_CUSTOM;
		
		if (arg)
		{
			msg = (char *)pattern;
			for (;;)
			{
				c = *msg++;
				if (c == 0)
					break;
					
				if (c == '&')
				{
					c = *msg++;
					if (c >= '1' && c <= '4')
					{
						c -= '0';
						if (c > narg)
							narg = c;
					}
				}
			}
		}
	}

	if (narg)
	{
		len = get_message_length(pattern, arg, narg);
		if (len)
		{
			msg = STRING_new(NULL, len);
			ERROR_current->info.msg = msg;
			ERROR_current->info.free = TRUE;
		
			if (EXEC_debug)
			{
				int i;
				strcpy(msg, pattern);
				msg += strlen(pattern);
				for (i = 0; i < narg; i++)
				{
					*msg++ = '|';
					if (arg[i])
					{
						strcpy(msg, arg[i]);
						msg += strlen(arg[i]);
					}
				}
			}
			else
			{
				for (;;)
				{
					c = *pattern++;
					if (c == 0)
						break;
						
					if (c == '&')
					{
						c = *pattern++;
						if (c >= '1' && c <= '4')
						{
							c -= '1';
							if (arg[c])
							{
								len = strlen(arg[c]);
								memcpy(msg, arg[c], len);
								msg += len;
							}
						}
					}
					else
						*msg++ = c;
				}
				
				*msg = 0;
			}

			/*fprintf(stderr, "msg: %s\n", ERROR_current->info.msg);
			if (strcmp(ERROR_current->info.msg, "Type mismatch: wanted WebView, got Function instead") == 0)
			{
				BREAKPOINT();
				STRING_watch = ERROR_current->info.msg;
			}*/
		}
	}
	else if (ERROR_current->info.code == E_CUSTOM)
	{
		if (pattern && *pattern)
		{
			ERROR_current->info.msg = STRING_new_zero(pattern);
			ERROR_current->info.free = TRUE;
		}
		else
		{
			ERROR_current->info.msg = (char *)_message[E_UNKNOWN];
			ERROR_current->info.free = FALSE;
		}
	}
	else
	{
		ERROR_current->info.msg = (char *)pattern;
		ERROR_current->info.free = FALSE;
	}

	//fprintf(stderr, "ERROR_define: %p %d '%s'\n", ERROR_current, ERROR_current->info.code, ERROR_current->info.msg);

	//STRING_add_char(&ERROR_current->info.msg, 0);

	ERROR_current->info.cp = CP;
	ERROR_current->info.fp = FP;
	ERROR_current->info.pc = PC;
	
	#if DEBUG_ERROR
	ERROR_debug("ERROR_define: %s\n", ERROR_current->info.msg);
	#endif
}

void THROW(int code, ...)
{
	va_list args;
	int i;
	char *arg[4];

	va_start(args, code);
	
	for (i = 0; i < 4; i++)
		arg[i] = va_arg(args, char *);

	ERROR_define((char *)(intptr_t)code, arg);

	va_end(args);
	
	PROPAGATE();
}

void THROW_CLASS(void *class, char *arg1, char *arg2)
{
	THROW(E_CLASS, CLASS_get_name((CLASS *)class), arg1, arg2);
}

void THROW_ILLEGAL()
{
	THROW(E_ILLEGAL);
}

void THROW_STACK()
{
	#if DEBUG_STACK
	fprintf(stderr, "THROW STACK!\n");
	#endif
	THROW(E_STACK);
}

void THROW_SYSTEM(int err, const char *path)
{
	char buf[6];
	
	switch(err)
	{
		case ENOENT:
			THROW(E_NEXIST, path);

		case EISDIR:
			THROW(E_DIR, path);

		case ENOTDIR:
			THROW(E_NDIR, path);

		case ENOMEM:
			THROW(E_MEMORY);

		case EACCES:
			THROW(E_ACCESS);

		case ENAMETOOLONG:
			THROW(E_TOOLONG);

		case ENOSPC:
			THROW(E_FULL);

		case EEXIST:
			THROW(E_EXIST, path);
			
		default:
			sprintf(buf, "%d", err);
			THROW(E_SYSTEM, buf, strerror(err));
	}
}

void ERROR_fatal(const char *error, ...)
{
	va_list args;

	va_start(args, error);
	fputs(EXEC_arch ? "gbr" GAMBAS_VERSION_STRING : "gbx" GAMBAS_VERSION_STRING, stderr);
	fputs(": ", stderr);
	vfprintf(stderr, error, args);
	va_end(args);
	putc('\n', stderr);
	_exit(1);
}

void ERROR_panic(const char *error, ...)
{
	va_list args;

	va_start(args, error);

	fflush(NULL);

	fprintf(stderr, "\n** Oops! Internal error! **\n** ");
	vfprintf(stderr, error, args);
	
	va_end(args);
	
	putc('\n', stderr);
	if (ERROR_current->info.code)
	{
		ERROR_print();
	}
	fprintf(stderr, "** Program aborting. Sorry! :-(\n** Please send a bug report at gambas@users.sourceforge.net\n");
	_exit(1);
}


void ERROR_print_at(FILE *where, bool msgonly, bool newline)
{
	if (!ERROR_current->info.code)
		return;

	if (!msgonly)
	{
		if (ERROR_current->info.cp && ERROR_current->info.fp && ERROR_current->info.pc)
			fprintf(where, "%s: ", DEBUG_get_position(ERROR_current->info.cp, ERROR_current->info.fp, ERROR_current->info.pc));
		else
			fprintf(where, "ERROR: ");
		/*if (ERROR_current->info.code > 0 && ERROR_current->info.code < 256)
			fprintf(where, "%ld:", ERROR_current->info.code);*/
		if (ERROR_current->info.code > 0)
			fprintf(where, "#%d: ", ERROR_current->info.code);
		if (ERROR_current->info.msg)
			fprintf(where, "%s", ERROR_current->info.msg);
	}
	else
	{
		char *p = ERROR_current->info.msg;
		unsigned char c;
		
		if (p)
		{
			while ((c = *p++))
			{
				if (c < ' ') c = ' ';
				fputc(c, where);
			}
		}
	}

	if (newline)
		fputc('\n', where);
}

void ERROR_print(void)
{
	static bool lock = FALSE;
	
	if (EXEC_main_hook_done && !EXEC_debug && EXEC_Hook.error && !lock)
	{
		lock = TRUE;
		GAMBAS_DoNotRaiseEvent = TRUE;
		HOOK(error)(ERROR_current->info.code, ERROR_current->info.msg, DEBUG_get_position(ERROR_current->info.cp, ERROR_current->info.fp, ERROR_current->info.pc));
		lock = FALSE;
	}

	ERROR_print_at(stderr, FALSE, TRUE);
	
	if (ERROR_backtrace)
		DEBUG_print_backtrace(ERROR_backtrace);
}

static void ERROR_copy(ERROR_INFO *save, ERROR_INFO *last)
{
	ERROR_reset(save);
	*save = ERROR_current->info;

	if (last)
	{
		ERROR_reset(last);
		*last = ERROR_last;
	}
}

void ERROR_save(ERROR_INFO *save, ERROR_INFO *last)
{
	ERROR_copy(save, last);

	CLEAR(&ERROR_current->info);
	if (last)
		CLEAR(&ERROR_last);
}

void ERROR_restore(ERROR_INFO *save, ERROR_INFO *last)
{
	ERROR_reset(&ERROR_current->info);
	ERROR_current->info = *save;
	CLEAR(save);

	if (last)
	{
		ERROR_reset(&ERROR_last);
		ERROR_last = *last;
		CLEAR(last);
	}
}

void ERROR_set_last(bool bt)
{
	ERROR_reset(&ERROR_last);
	ERROR_last = ERROR_current->info;
	if (ERROR_last.free)
		STRING_ref(ERROR_last.msg);
	STACK_free_backtrace(&ERROR_backtrace);
	if (bt)
		ERROR_backtrace = STACK_get_backtrace();
}

void ERROR_define_last(void)
{
	ERROR_reset(&ERROR_current->info);
	ERROR_current->info = ERROR_last;
	if (ERROR_last.free)
		STRING_ref(ERROR_last.msg);
}

void ERROR_warning(const char *warning, ...)
{
	va_list args;

	va_start(args, warning);

	fflush(NULL);

	fprintf(stderr, "gbx" GAMBAS_VERSION_STRING ": warning: ");
	vfprintf(stderr, warning, args);
	
	va_end(args);
	
	putc('\n', stderr);
}

/*void ERROR_deprecated(const char *msg)
{
	ERROR_warning("%s: %s is deprecated.", DEBUG_get_current_position(), msg);
}*/

void ERROR_exit(void)
{
	ERROR_reset(&ERROR_last);
	STACK_free_backtrace(&ERROR_backtrace);
}

void ERROR_hook(void)
{
	static bool no_rec = FALSE;
	
	ERROR_INFO save = { 0 };
	ERROR_INFO last = { 0 };
	CLASS_DESC_METHOD *handle_error;
	
	if (no_rec)
		return;
	
	if (PROJECT_class && CLASS_is_loaded(PROJECT_class))
	{
		handle_error = (CLASS_DESC_METHOD *)CLASS_get_symbol_desc_kind(PROJECT_class, "Application_Error", CD_STATIC_METHOD, 0);
		
		if (handle_error)
		{
			no_rec = TRUE;
			ERROR_save(&save, &last);
			
			TRY
			{
				EXEC_public_desc(PROJECT_class, NULL, handle_error, 0);
			}
			CATCH
			{
				ERROR_save(&save, &last);
			}
			END_TRY

			ERROR_restore(&save, &last);
			no_rec = FALSE;
		}
	}
}

