/***************************************************************************

	gbx.c

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
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
	MA 02110-1301, USA.

***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#define USE_PROFILE 1

#include "gb_common.h"
#include "gb_alloc.h"
#include "gb_error.h"

#include <dlfcn.h>
#include <stdarg.h>

#include "gbx_class.h"
#include "gbx_exec.h"
#include "gbx_stack.h"
#include "gbx_debug.h"
#include "gb_file.h"
#include "gbx_component.h"
#include "gbx_project.h"
#include "gbx_local.h"
#include "gbx_watch.h"
#include "gbx_event.h"
#include "gbx_extern.h"
#include "gbx_eval.h"
#include "gbx_subr.h"
#include "gbx_math.h"
#include "gb_common_buffer.h"
#include "gbx_api.h"

#if USE_PROFILE
#include "gbx_profile.h"
#endif

#include "gbx_c_file.h"
#include "gbx_c_application.h"

extern void _exit(int) NORETURN;
FILE *log_file;

static bool _welcome = FALSE;
static bool _quit_after_main = FALSE;

static void NORETURN my_exit(int ret)
{
	LOCAL_exit();
	COMPONENT_exit();
	EXTERN_exit();
	//fclose(log_file);
	exit(ret);
}

static void NORETURN fatal(const char *msg, ...)
{
	va_list args;

	va_start(args, msg);
	fputs(EXEC_arch ? "gbr" GAMBAS_VERSION_STRING : "gbx" GAMBAS_VERSION_STRING, stderr);
	fputs(": ", stderr);
	vfprintf(stderr, msg, args);
	va_end(args);
	putc('\n', stderr);
	my_exit(1);
} 

static void init(const char *file)
{
	#if USE_PROFILE
	if (EXEC_profile)
		PROFILE_init();
	#endif
	
	COMPONENT_init();
	FILE_init();
	EXEC_init();
	CLASS_init();
	CFILE_init();
	WATCH_init();
	MATH_init();
	PROJECT_init(file);
	DEBUG_init();

	if (file)
	{
		if (PROJECT_load()) // Call STACK_init()
		{
			if (!strcmp(file, "."))
				fatal("no project file in current directory.");
			else
				fatal("no project file in '%s'.", file);
		}
	}
	else
		STACK_init();
		
	LOCAL_init();

	if (EXEC_debug)
	{
		DEBUG.Welcome();
		DEBUG.Main(FALSE);
	}
	_welcome = TRUE;
}


static void main_exit(bool silent)
{
	// If the stack has not been initialized because the project could not be started, do it know
	if (!SP)
		STACK_init();
	
	TRY
	{
		EXTERN_release();
		STREAM_exit();
		OBJECT_exit();
		CLASS_clean_up(silent);
		SUBR_exit();
		DEBUG_exit();
		CFILE_exit();
		WATCH_exit();
		#if USE_PROFILE
		PROFILE_exit();
		#endif
		CLASS_exit();
		COMPONENT_exit();
		EXTERN_exit();
		PROJECT_exit();
		LOCAL_exit();
		EVENT_exit();
		FILE_exit();
		STACK_exit();
		ERROR_exit();
		STRING_exit();
	}
	END_TRY
}

static bool is_option(const char *arg, char option)
{
	return (arg[0] == '-' && arg[1] == option && arg[2] == 0);
}

static bool is_long_option(const char *arg, char option, char *long_option)
{
	if (is_option(arg, option))
		return TRUE;
	else
		return (arg[0] == '-' && arg[1] == '-' && !strcmp(&arg[2], long_option));
}

int main(int argc, char **argv)
{
	//CLASS *class = NULL;
	CLASS_DESC_METHOD *startup = NULL;
	int i, n;
	char *file = NULL;
	int ret = 0;

	//char log_path[256];
	//sprintf(log_path, "/tmp/gambas-%d.log", getuid());
	//log_file = freopen(log_path, "w+", stderr);
	//fprintf(stderr, "Fichier log Gambas\n");

	MEMORY_init();
	COMMON_init();
	//STRING_init();

	EXEC_arch = (strcmp(FILE_get_name(argv[0]), "gbr" GAMBAS_VERSION_STRING) == 0);

	if (argc == 2)
	{
		if (is_long_option(argv[1], 'h', "help"))
		{
			if (EXEC_arch)
			{
				printf(
					"Usage: gbr" GAMBAS_VERSION_STRING " [options] <executable file> [<arguments>]\n\n"
					);
			}
			else
			{
				printf(
					"Usage: gbx" GAMBAS_VERSION_STRING " [options] [<project file>] [-- <arguments>]\n"
					"       gbx" GAMBAS_VERSION_STRING " -e <expression>\n\n"
					);
			}
			printf(
				"Options:\n"
				"  -V --version   display version\n"
				"  -h --help      display this help\n"
				"  -L --license   display license\n"
				"  -g             enter debugging mode\n"
				"  -k             do not unload shared libraries\n"
				);
			if (!EXEC_arch)
			{
				printf("  -e             evaluate an expression\n");
			}

			my_exit(0);
		}
		else if (is_long_option(argv[1], 'V', "version"))
		{
			printf(VERSION "\n");
			my_exit(0);
		}
		else if (is_long_option(argv[1], 'L', "license"))
		{
			printf(
				"Gambas interpreter version " VERSION " " __DATE__ " " __TIME__ "\n"
				COPYRIGHT
				);
			my_exit(0);
		}
	}
	
	if (!EXEC_arch && argc >= 2 && is_option(argv[1], 'e'))
	{
		if (argc < 3)
			fatal("-e option needs an expression.");
		
		TRY
		{
			init(NULL);
			EVAL_string(argv[2]);
		}
		CATCH
		{
			if (ERROR_current->info.code && ERROR_current->info.code != E_ABORT)
				ERROR_print_at(stderr, TRUE, TRUE);
			main_exit(TRUE);
			_exit(1);
		}
		END_TRY
		
		main_exit(FALSE);
		_exit(0);	
	}
	
	for (i = 1; i < argc; i++)
	{
		if (is_option(argv[i], 'g'))
		{
			EXEC_debug = TRUE;
		}
		#if USE_PROFILE
		else if (is_option(argv[i], 'p'))
		{
			EXEC_profile = TRUE;
		}
		#endif
		else if (is_option(argv[i], 'f'))
		{
			EXEC_fifo = TRUE;
			if (i < (argc - 1) && *argv[i + 1] && *argv[i + 1] != '-')
			{
				EXEC_fifo_name = argv[i + 1];
				i++;
			}
		}
		else if (is_option(argv[i], 'k'))
		{
			EXEC_keep_library = TRUE;
		}
		else if (is_option(argv[i], 'q'))
		{
			_quit_after_main = TRUE;
		}
		else if (is_option(argv[i], '-'))
		{
			i++;
			break;
		}
		else
		{
			if (file)
			{
				fatal("too many %s.", EXEC_arch ? "executable files" : "project directories");
				my_exit(1);
			}
			file = argv[i];
			
			if (EXEC_arch)
			{
				i++;
				break;
			}
		}
	}

	n = i;
	if (!file)
		file = ".";

	if (EXEC_arch)
		argv[0] = file;
	
	for (i = 1; i <= (argc - n); i++)
		argv[i] = argv[i + n - 1];

	argc -= n - 1;
	
	TRY
	{
		init(file);
		
		if (!EXEC_arch)
			argv[0] = PROJECT_name;

		HOOK(main)(&argc, argv);
		EXEC_main_hook_done = TRUE;

		/* Startup class */
		CLASS_load(PROJECT_class);
		startup = (CLASS_DESC_METHOD *)CLASS_get_symbol_desc_kind(PROJECT_class, "main", CD_STATIC_METHOD, 0);
		if (startup == NULL)
			THROW(E_MAIN);

		//CAPP_init(); /* needs startup class */
		CFILE_init_watch();

		PROJECT_argc = argc;
		PROJECT_argv = argv;
	}
	CATCH
	{
		if (EXEC_debug)
		{
			if (!_welcome)
				DEBUG.Main(TRUE);
			DEBUG.Main(TRUE);
			main_exit(FALSE);
			_exit(0);
		}
		else
		{
			if (ERROR->info.code && ERROR->info.code != E_ABORT)
				ERROR_print();
			main_exit(TRUE);
			_exit(1);
		}
	}
	END_TRY

	TRY
	{
		EXEC_public_desc(PROJECT_class, NULL, startup, 0);
		
		if (TYPE_is_boolean(startup->type))
			ret = RP->_boolean.value ? 1 : 0;
		else if (TYPE_is_integer(startup->type))
			ret = RP->_integer.value & 0xFF;

		EXEC_release_return_value();
		
		if (_quit_after_main)
		{
			main_exit(TRUE);
			_exit(0);
		}
		
		if (!ret)
		{
			HOOK_DEFAULT(loop, WATCH_loop)();
			EVENT_check_post();
		}
	}
	CATCH
	{
		if (ERROR->info.code && ERROR->info.code != E_ABORT)
		{
			if (EXEC_debug)
			{
				DEBUG.Main(TRUE);
				main_exit(TRUE);
				_exit(0);
			}
			else
			{
				ERROR_print();
				main_exit(TRUE);
				_exit(1);
			}
		}
	}
	END_TRY

	main_exit(FALSE);

	MEMORY_exit();

	fflush(NULL);

	exit(ret);
}

