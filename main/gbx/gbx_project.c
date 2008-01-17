/***************************************************************************

  gbx_project.c

  Project loader

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

#define __GBX_PROJECT_C

#include "config.h"

#include "gb_common.h"
#include "gb_common_case.h"
#include "gb_alloc.h"
#include "gb_error.h"

#include <unistd.h>
#include <pwd.h>

#include "gb_limit.h"
#include "gb_buffer.h"
#include "gb_file.h"
#include "gbx_stream.h"
#include "gbx_archive.h"
#include "gbx_exec.h"
#include "gbx_stack.h"
#include "gb_component.h"
#include "gbx_component.h"

#include "gbx_project.h"

char *PROJECT_path = NULL;
char *PROJECT_exec_path = NULL;
char *PROJECT_user_home = NULL;

char *PROJECT_name = NULL;
char *PROJECT_title = NULL;
char *PROJECT_startup = NULL;
char *PROJECT_version = NULL;
CLASS *PROJECT_class = NULL;

int PROJECT_argc = 0;
char **PROJECT_argv = NULL;

char *PROJECT_oldcwd = NULL;

static char *project_buffer;

static char *project_ptr;
static int project_line;


static void raise_error(const char *msg)
{
  char line[16];

  snprintf(line, sizeof(line), "%d", project_line);
  THROW(E_PROJECT, line, msg);
}


static void project_title(char *name, int len)
{
  name[len] = 0;
  PROJECT_title = name;
}


static void project_version(char *name, int len)
{
  name[len] = 0;
  PROJECT_version = name;
}


static void project_library(char *name, int len)
{
  const char *delim = ",";
  char *comp;

  name[len] = 0;

  comp = strtok(name, delim);
  while (comp != NULL)
  {
  	//if (strcmp(comp, PROJECT_name))
		COMPONENT_create(comp);
    comp = strtok(NULL, delim);
  }
}


static void project_exec(char *name, int len)
{
  if (len == 0)
    raise_error("Bad project startup class");

  name[len] = 0;
  PROJECT_startup = name;
}


static void project_stack(char *name, int len)
{
  int size;

  name[len] = 0;
  size = atoi(name);

  if (size >= 1 && size <= 64)
    STACK_size = size * 1024L * sizeof(VALUE);
}


static void project_command(char *line, int len)
{
  static PROJECT_COMMAND command[] = {
    { "PROJECT", NULL },
    { "TITLE", project_title },
    { "LIBRARY", project_library },
    { "COMPONENT", project_library },
    { "STARTUP", project_exec },
    { "STACK", project_stack },
    { "VERSION", project_version },
    { "ARGUMENTS", NULL },
    { NULL }
    };

  PROJECT_COMMAND *pc;
  char cmd[32];
  int len_cmd;

  for (len_cmd = 0; len_cmd < len; len_cmd++)
  {
    if (line[len_cmd] == '=')
      break;
  }

  if (len_cmd >= len || len_cmd >= sizeof(cmd) || len_cmd == 0)
    raise_error("Syntax error");

  strncpy(cmd, line, len_cmd);

  for (pc = command; ; pc++)
  {
    if (pc->command == NULL)
      break;
      /*raise_error("Unknown command");*/

    if (strncasecmp(pc->command, cmd, len_cmd) == 0)
    {
      if (pc->func)
        (*pc->func)(&line[len_cmd + 1], len - len_cmd - 1);
      break;
    }
  }
}


static void project_analyze(char *addr, int len)
{
  char *end = &addr[len];
  char c;
  char *start;


  project_ptr = addr;
  project_line = 1;
  start = project_ptr;

  for(;;)
  {
    if (project_ptr >= end)
      break;

    c = *project_ptr++;

    if (c == '\n')
    {
      project_line++;
      start = project_ptr;
      continue;
    }

    if (c == '#')
    {
      while ((project_ptr < end) && c != '\n')
        c = *project_ptr++;
      project_ptr--;
      continue;
    }

    if (c <= ' ')
      continue;

    project_ptr--;
    start = project_ptr;

    while ((project_ptr < end) && (c != '\n'))
      c = *project_ptr++;

    project_command(start, project_ptr - start - 1);
    project_ptr--;
  }

  if (!PROJECT_name || PROJECT_name[0] == 0)
    raise_error("No project name");

  if (!PROJECT_startup || PROJECT_startup[0] == 0)
    raise_error("No startup class");

  if (!PROJECT_title || PROJECT_title[0] == 0)
    PROJECT_title = PROJECT_name;
}


void PROJECT_init(const char *file)
{
  int len;
  const char *path;
  struct passwd *info;

  /* Save the working directory */

  STRING_new(&PROJECT_oldcwd, FILE_getcwd(NULL), 0);

  /* Gambas installation path */

  path = FILE_find_gambas();

  STRING_new(&PROJECT_exec_path, FILE_get_dir(FILE_get_dir(path)), 0);

	/* Home directory */

  info = getpwuid(getuid());
  if (info)
	  STRING_new(&PROJECT_user_home, info->pw_dir, 0);
	else
		PROJECT_user_home = NULL;

	/* Component paths */

  STRING_new(&COMPONENT_path, FILE_cat(PROJECT_exec_path, "lib/gambas" GAMBAS_VERSION_STRING, NULL), 0);
  STRING_new(&COMPONENT_user_path, FILE_cat(PROJECT_user_home, ".local/lib/gambas" GAMBAS_VERSION_STRING, NULL), 0);

  /* Project path & name*/

	if (!file)
	{
		// "gbx2 -e" case
		STRING_new(&PROJECT_path, NULL, 0);
		STRING_new(&PROJECT_name, NULL, 0);
		return;
	}

  if (EXEC_arch)
  {
    path = FILE_get_dir(file);

    chdir(path);

    path = FILE_getcwd(NULL);
    if (path == NULL)
      goto _PANIC;
  }
  else
  {
    if (*file == '/')
    {
      path = file;
    }
    else
    {
      if (*file == '.' && file[1] == '/')
        file += 2;

      path = FILE_getcwd(file);
      if (path == NULL)
        goto _PANIC;

      chdir(path);

      path = FILE_getcwd(NULL);
      if (path == NULL)
        goto _PANIC;
    }
  }

  len = strlen(path);

  while (len > 1)
  {
    if (path[len - 1] != '/')
      break;

    len--;
    /*path[len] = 0;*/
  }

  STRING_new(&PROJECT_path, path, len);

  chdir(PROJECT_path);

  /* Project name */

  if (EXEC_arch)
  	STRING_new(&PROJECT_name, FILE_get_basename(file), 0);
	else
  	STRING_new(&PROJECT_name, FILE_get_name(PROJECT_path), 0);

	/* Main archive creation */

	ARCHIVE_create_main(EXEC_arch ? FILE_get_name(file) : NULL);

	return;

_PANIC:
  ERROR_panic("Cannot initialize project: %s", strerror(errno));
}


void PROJECT_load()
{
	const char *file;
	int len;

	/* Project file analyze */

  if (EXEC_arch)
    file = ".project";
  else
    file = FILE_cat(PROJECT_path, ".project", NULL);

  STREAM_load(file, &project_buffer, &len);

  project_analyze(project_buffer, len);

  STACK_init();

	/* Loads all component */
	COMPONENT_load_all();

	/* Loads main archive */
	ARCHIVE_load_main();

  /* Startup class */
  // we make the class global, because some components may look for event handler in it!
  PROJECT_class = CLASS_find_global(PROJECT_startup);
}


void PROJECT_exit(void)
{
  if (project_buffer)
    FREE(&project_buffer, "PROJECT_exit");

  STRING_free(&PROJECT_name);
  STRING_free(&PROJECT_oldcwd);
  STRING_free(&PROJECT_path);
  STRING_free(&PROJECT_exec_path);
  STRING_free(&PROJECT_user_home);
}
