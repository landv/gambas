/***************************************************************************

  main.c

  Welcome to the compiler !

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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

#define __MAIN_C

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>

#include <unistd.h>
#ifdef __GNU_LIBRARY__
#include <getopt.h>
#endif

#include "gb_common.h"
#include "gb_error.h"
#include "gb_str.h"
#include "gb_file.h"
#include "gb_common_buffer.h"

#include "gbc_compile.h"

#include "gb_reserved.h"
#include "gbc_read.h"
#include "gbc_form.h"
#include "gbc_trans.h"
#include "gbc_header.h"
#include "gbc_output.h"


#ifdef __GNU_LIBRARY__
static struct option Long_options[] =
{
  { "debug", 0, NULL, 'g' },
  { "version", 0, NULL, 'V' },
  { "help", 0, NULL, 'h' },
  { "verbose", 0, NULL, 'v' },
  { "trans", 0, NULL, 't' },
  { "public", 0, NULL, 'p' },
  { "swap", 0, NULL, 's' },
  { "class", 1, NULL, 'c' },
  /*{ "dump", 0, NULL, 'd' },*/
  { "all", 0, NULL, 'a' },
  { 0 }
};
#endif

static char **path_list;
static int path_current;

static bool main_debug = FALSE;
static bool main_verbose = FALSE;
static bool main_compile_all = FALSE;
static bool main_trans = FALSE;
static bool main_public = FALSE;
static bool main_swap = FALSE;
//static char *main_class_file = NULL;

static void get_arguments(int argc, char **argv)
{
  int opt;
  #ifdef __GNU_LIBRARY__
  int index = 0;
  #endif

  for(;;)
  {
    #ifdef __GNU_LIBRARY__
      opt = getopt_long(argc, argv, "gvaVhtpsc:", Long_options, &index);
    #else
      opt = getopt(argc, argv, "gvaVhtpsc:");
    #endif
    if (opt < 0) break;

    switch (opt)
    {
      case 'V':
        printf(VERSION "\n");
        exit(0);

      case 'g':
        main_debug = TRUE;
        break;

      case 'v':
        main_verbose = TRUE;
        break;

      case 'a':
        main_compile_all = TRUE;
        break;

      case 't':
        main_trans = TRUE;
        break;

      case 'p':
        main_public = TRUE;
        break;

      case 's':
        main_swap = TRUE;
        break;

      //case 'c':
      //  main_class_file = optarg;
      //:  break;

      case 'h': case '?':
        printf(
          "\n"
          "GAMBAS Compiler version " VERSION " " __DATE__ " " __TIME__ "\n"
          COPYRIGHT
          "Usage: gbc" GAMBAS_VERSION_STRING " [options] [<project directory>]\n\n"
          "Options:"
          #ifdef __GNU_LIBRARY__
          "\n"
          "  -g  --debug                add debugging information\n"
          "  -v  --verbose              verbose output\n"
          "  -a  --all                  compile all\n"
          "  -t  --trans                output translation files\n"
          "  -p  --public               form controls are public\n"
          "  -s  --swap                 swap endianness\n"
          "  -V  --version              display version\n"
          "  -h  --help                 display this help\n"
          #else
          " (no long options on this system)\n"
          "  -g                         add debugging information\n"
          "  -v                         verbose output\n"
          "  -a                         compile all\n"
          "  -t                         output translation files\n"
          "  -p                         form controls are public\n"
          "  -s                         swap endianness\n"
          "  -V                         display version\n"
          "  -h                         display this help\n"
          #endif
          "\n"
          );

        exit(0);

      default:
        exit(1);

    }
  }

  if (optind < (argc - 1))
  {
    fprintf(stderr, "gbc: too many arguments.\n");
    exit(1);
  }

  /*COMP_project = STR_copy(FILE_cat(argv[optind], "Gambas", NULL));*/
  if (optind < argc)
    chdir(argv[optind]);

  COMP_project = STR_copy(FILE_cat(FILE_get_current_dir(), ".project", NULL));

  if (!FILE_exist(COMP_project))
  {
    fprintf(stderr, "gbc: project file not found: %s\n", COMP_project);
    exit(1);
  }
}


static void compile_file(const char *file)
{
  time_t time_src, time_form, time_pot, time_output;

  COMPILE_begin(file, main_trans);

  if (!main_compile_all)
  {
    if (FILE_exist(JOB->output))
    {
      time_src = FILE_get_time(JOB->name);
      time_output = FILE_get_time(JOB->output);

      if (JOB->form)
        time_form = FILE_get_time(JOB->form);
      else
        time_form = time_src;

      if (main_trans)
        time_pot = FILE_get_time(JOB->tname);
      else
        time_pot = time_src;

      if (time_src <= time_output && time_src <= time_pot && time_form <= time_output)
        goto _FIN;
    }
  }

  JOB->all = main_compile_all;
  JOB->debug = main_debug;
  JOB->verbose = main_verbose;
  JOB->swap = main_swap;
  //JOB->class_file = main_class_file;

  if (JOB->verbose)
    puts(JOB->name);

  COMPILE_load();
  FORM_do(main_public);
  BUFFER_add(&JOB->source, "\n\n\n\n", 4);

  READ_do();

  #ifdef DEBUG
  TABLE_print(JOB->class->table, TRUE);
  #endif

  HEADER_do();
  TRANS_code();

  #ifdef DEBUG
  TABLE_print(JOB->class->string, FALSE);
  #endif

  OUTPUT_do(main_swap);

_FIN:
  COMPILE_end();
}


static void path_add(const char *path)
{
  *((char **)ARRAY_add(&path_list)) = STR_copy(path);
}


static void path_init(const char *first)
{
  ARRAY_create(&path_list);

  if (*first)
    chdir(first);

  path_add(FILE_get_current_dir());

  path_current = 0;
}


static void path_exit(void)
{
  ARRAY_delete(&path_list);
}


static long path_count(void)
{
  return ARRAY_count(path_list);
}


int main(int argc, char **argv)
{
  DIR *dir;
  char *path;
  struct dirent *dirent;
  char *file_name;
  const char *file;
  struct stat info;
  const char *ext;

  MEMORY_init();
  COMMON_init();

  get_arguments(argc, argv);

  TRY
  {
    COMPILE_init();

    path_init(FILE_get_dir(COMP_project));

    for(;;)
    {
      if (path_current >= path_count())
        break;

      path = path_list[path_current++];
      dir = opendir(path);

      if (dir == NULL)
      {
        fprintf(stderr, "gbc: Warning: Cannot open dir: %s\n", path);
        goto _NEXT_PATH;
      }

      if (chdir(path) != 0)
      {
        fprintf(stderr, "gbc: Warning: Cannot change dir: %s\n", path);
        goto _NEXT_PATH;
      }

      while ((dirent = readdir(dir)) != NULL)
      {
        file_name = dirent->d_name;
        if (*file_name == '.')
          continue;

        file = FILE_cat(path, file_name, NULL);

        if (stat(file_name, &info))
        {
          fprintf(stderr, "gbc: Warning: Cannot stat file: %s\n", file);
          continue;
        }

        // No recursion anymore!
        //if (S_ISDIR(info.st_mode))
        //  path_add(file);
        //else
        if (!S_ISDIR(info.st_mode))
        {
          ext = FILE_get_ext(file);

          if ((strcasecmp(FILE_get_ext(file), "module") == 0)
              || (strcasecmp(FILE_get_ext(file), "class") == 0))
            compile_file(file);
        }
      }

_NEXT_PATH:
      if (dir != NULL) closedir(dir);
      FREE(&path, "main");
    }

    path_exit();

    COMPILE_exit();
    printf("OK\n");
  }
  CATCH
  {
    fflush(NULL);
    if (JOB->line)
      fprintf(stderr, "%s:%ld: ", JOB->name, JOB->line); /*, (long)(JOB->current - JOB->pattern));*/
    else
      fprintf(stderr, "gbc: ERROR: ");
    ERROR_print();
    exit(1);
  }
  END_TRY

  return 0;
}

