/***************************************************************************

  gbx.c

  Interpreter startup

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gb_common.h"
#include "gb_alloc.h"
#include "gb_error.h"

#include <dlfcn.h>

#ifdef __GNU_LIBRARY__
//#define _GNU_SOURCE
#include <getopt.h>
#endif

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

#include "gbx_c_file.h"
#include "gbx_c_application.h"

extern void _exit(int) NORETURN;
FILE *log_file;

static bool _welcome = FALSE;

static void init(const char *file)
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

	if (EXEC_debug)
	{
		DEBUG.Welcome();
		DEBUG.Main(FALSE);
	}
	_welcome = TRUE;

	if (file)
		PROJECT_load(); // Call STACK_init()
	else
		STACK_init();
		
 	LOCAL_init();
}


static void my_exit(int ret)
{
  LOCAL_exit();
  COMPONENT_exit();
  EXTERN_exit();
	//fclose(log_file);
  exit(ret);
}


static void main_exit(bool silent)
{
  STREAM_exit();
  OBJECT_exit();
	CLASS_clean_up(silent);
  SUBR_exit();
  DEBUG_exit();
  CFILE_exit();
  WATCH_exit();
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
	//fclose(log_file);
}


int main(int argc, char **argv)
{
  //CLASS *class = NULL;
  CLASS_DESC_METHOD *startup = NULL;

  int i, n;
  char *file = ".";
  bool nopreload = FALSE;

 	//char log_path[256];
 	//sprintf(log_path, "/tmp/gambas-%d.log", getuid());
	//log_file = freopen(log_path, "w+", stderr);
 	//fprintf(stderr, "Fichier log Gambas\n");

  MEMORY_init();
  COMMON_init();
  //STRING_init();

	if (strcmp(argv[0], "gbr" GAMBAS_VERSION_STRING) == 0)
	{
    if (argc == 1)
    {
      fprintf(stderr, "gbr" GAMBAS_VERSION_STRING ": no archive file.\n");
      my_exit(1);
    }

    EXEC_arch = TRUE;

    if (strcmp(argv[1], "-p") == 0)
      n = 2;
    else
      n = 1;

    file = argv[n];
    if (n == 1)
      LIBRARY_preload(file, argv);

    for (i = 0; i < (argc - n); i++)
      argv[i] = argv[i + n];

    argc -= n;
	}
  /*else if (argc >= 2 && strcmp(argv[1], "-x") == 0)
  {
    if (argc == 2)
    {
      fprintf(stderr, "gbx: no archive file.\n");
      my_exit(1);
    }

    EXEC_arch = TRUE;

    if (strcmp(argv[2], "-p") == 0)
      n = 3;
    else
      n = 2;

    file = argv[n];
    if (n == 2)
      LIBRARY_preload(file, argv);

    for (i = 1; i < (argc - n); i++)
      argv[i] = argv[i + n];

    argc -= n;
  }*/
  else
  {
  	if (argc == 2)
  	{
  		if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
  		{
        printf(
          "\n"
          "GAMBAS Interpreter version " VERSION " " __DATE__ " " __TIME__ "\n"
          COPYRIGHT
          "Usage: gbx" GAMBAS_VERSION_STRING " [options] [<project file>] -- ...\n"
          "       gbx" GAMBAS_VERSION_STRING " -e <expression>\n\n"
          "Options:\n"
          "  -V --version   display version\n"
          "  -h --help      display this help\n"
          "  -e             evaluate an expression\n"
          "  -g             enter debugging mode\n"
#if DO_PRELOADING          
          "  -p             disable preloading\n"
#endif
          "  -k             do not unload shared libraries\n"
          "  -x             execute an archive\n"
          "\n"
          );

        my_exit(0);
  		}
      else if (!strcmp(argv[1], "-V") || !strcmp(argv[1], "--version"))
      {
        printf(VERSION "\n");
        my_exit(0);
      }
  	}
  	else
  	
  	if (argc == 3 && !strcmp(argv[1], "-e"))
  	{
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
      if (strcmp(argv[i], "-g") == 0)
      {
        EXEC_debug = TRUE;
      }
      else if (strcmp(argv[i], "-f") == 0)
      {
        EXEC_fifo = TRUE;
      }
      else if (strcmp(argv[i], "-p") == 0)
      {
        nopreload = TRUE;
      }
      else if (strcmp(argv[i], "-k") == 0)
      {
        EXEC_keep_library = TRUE;
      }
      else
      {
        if (strcmp(argv[i], "--"))
        {
          file = argv[i];
          i++;
        }
        break;
      }
    }

    if (i < argc)
    {
      if (file && strcmp(argv[i], "--"))
      {
        fprintf(stderr, "gbx" GAMBAS_VERSION_STRING ": too many project files.\n");
        my_exit(1);
      }

      i++;
    }

    n = i;

    if (!nopreload)
      LIBRARY_preload(file, argv);

    for (i = 1; i <= (argc - n); i++)
      argv[i] = argv[i + n - 1];

    argc -= n - 1;

    //printf("argc = %d\n", argc);
    /*for (i = 0; i < argc; i++)
      fprintf(stderr, "argv[%d] = '%s'\n", i, argv[i]);
    fprintf(stderr, "\n");*/

  }


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

    CAPP_init(); /* needs startup class */
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
    EXEC.class = PROJECT_class;
    EXEC.object = NULL;
    EXEC.drop = TRUE;
    EXEC.nparam = 0;

    if (FUNCTION_is_native(startup))
    {
      EXEC.native = TRUE;
      EXEC.use_stack = FALSE;
      EXEC.desc = startup;

      EXEC_native();
    }
    else
    {
      EXEC.native = FALSE;
      EXEC.index = (int)(intptr_t)startup->exec;

      EXEC_function();
    }

    HOOK_DEFAULT(loop, WATCH_loop)();

    EVENT_check_post();
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

  exit(EXEC_return_value);
}

