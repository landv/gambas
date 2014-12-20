/***************************************************************************

  gbx.c

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

#define __GBX_C

#include "config.h"
#include "../trunk_version.h"

//#define USE_PROFILE 1

#include "gb_common.h"
#include "gb_alloc.h"
#include "gb_error.h"

#include <dlfcn.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/resource.h>

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
#include "gbx_signal.h"

#if USE_PROFILE
#include "gbx_profile.h"
#endif

#include "gbx_c_file.h"
#include "gbx_c_application.h"

extern void _exit(int) NORETURN;
FILE *log_file;

static bool _welcome = FALSE;
static bool _quit_after_main = FALSE;
static bool _eval = FALSE;

static void NORETURN my_exit(int ret)
{
	LOCAL_exit();
	COMPONENT_exit();
	EXTERN_exit();
	//fclose(log_file);
	exit(ret);
}

static void init(const char *file, int argc, char **argv)
{
	COMPONENT_init();
	FILE_init();
	
	EXEC_init();
	CLASS_init();
	CFILE_init();
	WATCH_init();
	MATH_init();
	PROJECT_init(file);
	DEBUG_init();

	LOCAL_init();

	if (file)
	{
		PROJECT_load();

		if (PROJECT_run_httpd)
			COMPONENT_exec("gb.httpd", argc, argv);
		
		PROJECT_load_finish();
	}
	else
		STACK_init();
		
	if (EXEC_debug)
	{
		DEBUG.Welcome();
		DEBUG.Main(FALSE);
	}
	_welcome = TRUE;
}


static void main_exit(bool silent)
{
	// If the stack has not been initialized because the project could not be started, do it now
	if (!SP)
		STACK_init();
	
	TRY
	{
		SIGNAL_exit();
		EXTERN_release();
		STREAM_exit();
		OBJECT_exit();
		CFILE_exit();

		CLASS_clean_up(silent);

		SUBR_exit();
		DEBUG_exit();
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
	}
	CATCH
	{
		if (!silent)
			ERROR_print_at(stderr, _eval, TRUE);
		_exit(1);
	}
	END_TRY

	STRING_exit();
}

static bool is_option(const char *arg, char option)
{
	return (arg[0] == '-' && arg[1] == option && arg[2] == 0);
}

static bool is_option_arg(char **argv, int argc, int *i, char option, const char **param)
{
	if (is_option(argv[*i], option))
	{
		if (*i < (argc - 1) && *argv[*i + 1] && *argv[*i + 1] != '-')
		{
			*param = argv[*i + 1];
			(*i)++;
		}
		else
			*param = NULL;
		
		return TRUE;
	}
	else
		return FALSE;
}


static bool is_long_option(const char *arg, char option, char *long_option)
{
	if (is_option(arg, option))
		return TRUE;
	else
		return (arg[0] == '-' && arg[1] == '-' && !strcmp(&arg[2], long_option));
}


int main(int argc, char *argv[])
{
	CLASS_DESC_METHOD *startup = NULL;
	int i, n;
	char *file = NULL;
	int ret = 0;
	const char *redirect_stderr = NULL;

	//char log_path[256];
	//sprintf(log_path, "/tmp/gambas-%d.log", getuid());
	//log_file = freopen(log_path, "w+", stderr);
	//fprintf(stderr, "Fichier log Gambas\n");
	
	/*struct rlimit rl = { 64000000, 64000000 };
	if (setrlimit(RLIMIT_CORE, &rl))
		perror(strerror(errno));*/

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
					"\nExecute a Gambas executable.\n"
					"\nUsage: gbr" GAMBAS_VERSION_STRING " [options] <executable file> [<arguments>]\n\n"
					);
			}
			else
			{
				printf(
					"\nExecute a Gambas project or evaluate a Gambas expression (-e option).\n"
					"\nUsage: gbx" GAMBAS_VERSION_STRING " [options] [<project file>] [-- <arguments>]\n"
					"       gbx" GAMBAS_VERSION_STRING " -e <expression>\n\n"
					);
			}
			printf(
				"Options:\n"
				"  -g               enter debugging mode\n"
				"  -s <class>       override startup class\n"
				"  -p <path>        activate profiling and debugging mode\n"
				"  -k               do not unload shared libraries\n"
				"  -H --httpd       run through an embedded http server\n"
				);

			if (!EXEC_arch)
				printf("  -e               evaluate an expression\n");

			printf(
				"  -V --version     display version\n"
				"  -L --license     display license\n"
				"  -h --help        display this help\n"
				"\n"
				);

			my_exit(0);
		}
		else if (is_long_option(argv[1], 'V', "version"))
		{
#ifdef TRUNK_VERSION
			printf(VERSION " r" TRUNK_VERSION "\n");
#else
			printf(VERSION "\n");
#endif
			my_exit(0);
		}
		else if (is_long_option(argv[1], 'L', "license"))
		{
			printf(
				"\nGambas interpreter version " VERSION " " __DATE__ " " __TIME__ "\n"
				COPYRIGHT
				);
			my_exit(0);
		}
	}
	
	if (!EXEC_arch && argc >= 2 && is_option(argv[1], 'e'))
	{
		if (argc < 3)
			ERROR_fatal("-e option needs an expression.");
		
		_eval = TRUE;

		TRY
		{
			init(NULL, argc, argv);
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
		else if (is_option_arg(argv, argc, &i, 'p', &EXEC_profile_path))
		{
			EXEC_debug = TRUE;
			EXEC_profile = TRUE;
		}
		else if (is_option_arg(argv, argc, &i, 'f', &EXEC_fifo_name))
		{
			EXEC_fifo = TRUE;
		}
		else if (is_option_arg(argv, argc, &i, 's', &PROJECT_startup))
		{
			continue;
		}
		else if (is_option_arg(argv, argc, &i, 't', &redirect_stderr))
		{
			int fd = open(redirect_stderr, O_WRONLY | O_CLOEXEC);
			if (fd < 0)
				ERROR_fatal("cannot redirect stderr.");
			dup2(fd, STDERR_FILENO);
		}
		else if (is_option(argv[i], 'k'))
		{
			EXEC_keep_library = TRUE;
		}
		else if (is_option(argv[i], 'q'))
		{
			_quit_after_main = TRUE;
		}
		else if (is_long_option(argv[i], 'H', "httpd"))
		{
			PROJECT_run_httpd = TRUE;
		}
		else if (is_option(argv[i], '-'))
		{
			i++;
			break;
		}
		else
		{
			if (file)
				ERROR_fatal("too many %s.", EXEC_arch ? "executable files" : "project directories");
			
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
		init(file, argc, argv);
		
		if (!EXEC_arch)
			argv[0] = PROJECT_name;

		HOOK(main)(&argc, &argv);
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
		ERROR_hook();
		
		if (EXEC_debug && DEBUG_is_init())
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
			DEBUG_enter_event_loop();
			HOOK_DEFAULT(loop, WATCH_loop)();
			DEBUG_leave_event_loop();
			EVENT_check_post();
		}
	}
	CATCH
	{
		if (ERROR->info.code && ERROR->info.code != E_ABORT)
		{
			ERROR_hook();
			
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
		
		ret = EXEC_quit_value;
	}
	END_TRY

	main_exit(FALSE);

	MEMORY_exit();

	fflush(NULL);

	return ret;
}

