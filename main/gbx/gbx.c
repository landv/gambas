/***************************************************************************

  gbx.c

  Interpreter startup

  (c) 2000-2006 Benoît Minisini <gambas@users.sourceforge.net>

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

	PROJECT_load(); // Call STACK_init()
 	LOCAL_init();
}


static void my_exit(int ret)
{
  LOCAL_exit();
  COMPONENT_exit();
  EXTERN_exit();
  exit(ret);
}


static void main_exit()
{
  SUBR_exit();
  OBJECT_exit();
  DEBUG_exit();
  WATCH_exit();
  CLASS_exit();
  COMPONENT_exit();
  EXTERN_exit();
  PROJECT_exit();
  LOCAL_exit();
  EVENT_exit();
  FILE_exit();
  STRING_exit();
  STACK_exit();
}


int main(int argc, char **argv)
{
  //CLASS *class = NULL;
  CLASS_DESC_METHOD *startup = NULL;

  int i, n;
  char *file = NULL;
  bool nopreload = FALSE;

  MEMORY_init();
  COMMON_init();

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

    for (i = 1; i < (argc - n); i++)
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
    for (i = 1; i < argc; i++)
    {
      if (strcmp(argv[i], "-h") == 0)
      {
        printf(
          "\n"
          "GAMBAS Interpreter version " VERSION " " __DATE__ " " __TIME__ "\n"
          COPYRIGHT
          "Usage: gbx" GAMBAS_VERSION_STRING " [options] [<project file>] -- ...\n"
          "       gbx" GAMBAS_VERSION_STRING " -x <archive file> ...\n\n"
          "Options:\n"
          "  -g   enter debugging mode\n"
          "  -V   display version\n"
          "  -h   display this help\n"
          "  -p   disable preloading\n"
          "  -x   execute an archive\n"
          "\n"
          );

        my_exit(0);
      }
      else if (strcmp(argv[i], "-V") == 0)
      {
        printf(VERSION "\n");
        my_exit(0);
      }
      else if (strcmp(argv[i], "-g") == 0)
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
    /*printf("argc = %d\n", argc);
    for (i = 0; i < argc; i++)
      printf("argv[%d] = '%s'\n", i, argv[i]);
    printf("\n");*/

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
      main_exit();
      _exit(0);
		}
		else
		{
	    if (ERROR_info.code && ERROR_info.code != E_ABORT)
    		ERROR_print();
    	main_exit();
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
      EXEC.index = (int)startup->exec;
      //EXEC.func = &class->load->func[(long)startup->exec]

      EXEC_function();
    }

    HOOK_DEFAULT(loop, WATCH_loop)();

    EVENT_check_post();
  }
  CATCH
  {
    if (ERROR_info.code && ERROR_info.code != E_ABORT)
    {
			if (EXEC_debug)
			{
				DEBUG.Main(TRUE);
				main_exit();
				_exit(0);
			}
			else
			{
				ERROR_print();
				main_exit();
				_exit(1);
			}
    }
  }
  END_TRY

  ERROR_info.code = 0;
  main_exit();

  if (MEMORY_count)
    fprintf(stderr, "WARNING: %ld allocation(s) non freed.\n", MEMORY_count);

  MEMORY_exit();

  fflush(NULL);

  exit(EXEC_return_value);
}

