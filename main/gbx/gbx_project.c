/***************************************************************************

	gbx_project.c

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

char *PROJECT_name = NULL;
char *PROJECT_title = NULL;
const char *PROJECT_startup = NULL;
char *PROJECT_version = NULL;
CLASS *PROJECT_class = NULL;

int PROJECT_argc = 0;
char **PROJECT_argv = NULL;

char *PROJECT_oldcwd = NULL;

bool PROJECT_run_httpd = FALSE;

static char *project_buffer;

//static char *project_ptr;
static int project_line;

static char *_home = NULL;
static uid_t _uid = 0;

static const char *_last_component = NULL;

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


static void project_component(char *name, int len)
{
	_last_component = name;
	
	name[len] = 0;

	COMPONENT_create(name);

	// If 'gb.httpd' is set explicitely, then always run through it.

	if (strcmp(name, "gb.httpd") == 0)
		PROJECT_run_httpd = TRUE;

	_last_component = NULL;
}


static void project_startup(char *name, int len)
{
	if (PROJECT_startup)
		return;

	if (len == 0)
		raise_error("Project startup class name is void");

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

static void project_stacktrace(char *name, int len)
{
	//ERROR_backtrace = !(len == 1 && *name == '0');
	// Backtrace is always printed now.
}

#if 0
static void project_command(char *line, int len)
{
	static PROJECT_COMMAND command[] = {
		{ "PROJECT", NULL },
		{ "TITLE", project_title },
		{ "LIBRARY", project_component },
		{ "COMPONENT", project_component },
		{ "STARTUP", project_startup },
		{ "STACK", project_stack },
		{ "VERSION", project_version },
		{ "STACKTRACE", project_stacktrace },
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
#endif

static void check_after_analyze()
{
	if (!PROJECT_name || PROJECT_name[0] == 0)
		raise_error("No project name");

	if (!PROJECT_startup || PROJECT_startup[0] == 0)
		raise_error("No startup class");

	if (!PROJECT_title || PROJECT_title[0] == 0)
		PROJECT_title = PROJECT_name;
}

#if 0
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

	check_after_analyze();
}
#endif

static bool get_line(char **addr, const char *end, char **start, int *len)
{
	char *p = *addr;
	
	if (p >= end)
		return FALSE;
	
	while (p < end && *p && *p != '\n')
		p++;
	
	*start = *addr;
	*len = p - *start;
	*addr = p + 1;
	
	return (*len > 0);
}

void PROJECT_analyze_startup(char *addr, int len, PROJECT_COMPONENT_CALLBACK cb)
{
	char *end = &addr[len];
	char *p;
	int l, i;

	if (!cb)
	{
		if (get_line(&addr, end, &p, &l))
			project_startup(p, l);
		if (get_line(&addr, end, &p, &l))
			project_title(p, l);
		if (get_line(&addr, end, &p, &l))
			project_stack(p, l);
		if (get_line(&addr, end, &p, &l))
			project_stacktrace(p, l);
		if (get_line(&addr, end, &p, &l))
			project_version(p, l);
	}
	else
	{
		for (i = 1; i <= 5; i++)
			get_line(&addr, end, &p, &l);
	}

	while (get_line(&addr, end, &p, &l));

	if (!cb)
	{
		while (get_line(&addr, end, &p, &l))
			project_component(p, l);
		
		check_after_analyze();
	}
	else
	{
		while (get_line(&addr, end, &p, &l))
		{
			p[l] = 0;
			(*cb)(p);
		}
	}
}

char *PROJECT_get_home(void)
{
	struct passwd *info;
	uid_t uid = getuid();
	
	if (!_home || _uid != uid)
	{
		STRING_free(&_home);
		info = getpwuid(uid);
		if (info)
			_home = STRING_new_zero(info->pw_dir);
		_uid = uid;
	}

	return _home;
}

void PROJECT_init(const char *file)
{
	int len;
	const char *path;

	/* Save the working directory */

	PROJECT_oldcwd = STRING_new_zero(FILE_getcwd(NULL));

	/* Gambas installation path */

	path = FILE_find_gambas();

	PROJECT_exec_path = STRING_new_zero(FILE_get_dir(FILE_get_dir(path)));

	/* Component paths */

	#ifdef OS_64BITS
	COMPONENT_path = STRING_new_zero(FILE_cat(PROJECT_exec_path, GAMBAS_LIB64_PATH, NULL));
	if (access(COMPONENT_path, F_OK))
	{	
		STRING_free(&COMPONENT_path);
		COMPONENT_path = STRING_new_zero(FILE_cat(PROJECT_exec_path, GAMBAS_LIB_PATH, NULL));
	}
	#else
	COMPONENT_path = STRING_new_zero(FILE_cat(PROJECT_exec_path, GAMBAS_LIB_PATH, NULL));
	#endif
	
	//STRING_new(&COMPONENT_user_path, FILE_cat(PROJECT_get_home(), ".local", GAMBAS_LIB_PATH, NULL), 0);

	/* Project path & name*/

	if (!file)
	{
		// "gbx3 -e" case
		PROJECT_path = STRING_new("", 0);
		PROJECT_name = STRING_new("", 0);
		return;
	}

	if (EXEC_arch)
	{
		if (FILE_is_absolute(file))
		{
			path = FILE_get_dir(file);
			FILE_chdir(path);
		}

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

			if (!chdir(path))
			{
				path = FILE_getcwd(NULL);
				if (path == NULL)
					goto _PANIC;
			}
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

	PROJECT_path = STRING_new(path, len);

	FILE_chdir(PROJECT_path);

	/* Project name */

	if (EXEC_arch)
		PROJECT_name = STRING_new_zero(FILE_get_basename(file));
	else
		PROJECT_name = STRING_new_zero(FILE_get_name(PROJECT_path));

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

	STACK_init();
	
	if (EXEC_arch)
		file = ".startup";
	else
		file = FILE_cat(PROJECT_path, ".startup", NULL);

	TRY
	{
		STREAM_load(file, &project_buffer, &len);
	}
	CATCH
	{
		ERROR_fatal("unable to find startup file");
	}
	END_TRY
	
	TRY
	{
		PROJECT_analyze_startup(project_buffer, len, NULL);
	}
	CATCH
	{
		if (_last_component)
			ERROR_fatal("unable to load component: %s", _last_component);
		else
			ERROR_fatal("unable to analyze startup file");
	}
	END_TRY
	
	// Loads all component
	COMPONENT_load_all();
}

void PROJECT_load_finish(void)
{
	// Load exported class of components written in Gambas
	COMPONENT_load_all_finish();
	
	// Loads main archive
	ARCHIVE_load_main();

	// Startup class
	PROJECT_class = CLASS_find(PROJECT_startup);
}

void PROJECT_exit(void)
{
	if (project_buffer)
		FREE(&project_buffer);

	STRING_free(&PROJECT_name);
	STRING_free(&PROJECT_path);
	
	STRING_free(&PROJECT_oldcwd);
	STRING_free(&PROJECT_exec_path);
	STRING_free(&_home);
}
