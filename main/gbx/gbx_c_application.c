/***************************************************************************

  gbx_c_application.c

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __GBX_C_APPLICATION_C

#include "gambas.h"
#include "gbx_info.h"

#ifndef GBX_INFO

#include "config.h"

#include <signal.h>
#include <sys/wait.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gbx_api.h"
#include "gbx_class.h"
#include "gbx_project.h"
#include "gbx_local.h"
#include "gbx_event.h"
#include "gbx_string.h"
#include "gbx_exec.h"

#include "gbx_object.h"

#include "gbx_c_application.h"

static GB_FUNCTION signal_func;
static bool has_signal_func;
static bool _daemon = FALSE;

extern char **environ;

static void got_signal_after(intptr_t param)
{
  GB_Call(&signal_func, 0, FALSE);
}


void CAPP_got_signal(void)
{
  if (!has_signal_func)
    return;

  EVENT_post(got_signal_after, 0);
}


void CAPP_init()
{
  CLASS *class = PROJECT_class; //CLASS_find(PROJECT_startup);

  has_signal_func = GB_GetFunction(&signal_func, class, "Application_Signal", "", "") == 0;

  /*printf("has_signal_func = %d\n", has_signal_func);*/
}


BEGIN_PROPERTY(CAPPLICATION_path)

  GB_ReturnString(PROJECT_path);

END_PROPERTY


BEGIN_PROPERTY(CUSER_home)

	GB_ReturnString(PROJECT_get_home());

END_PROPERTY


BEGIN_PROPERTY(CUSER_name)

  struct passwd *info = getpwuid(getuid());

  if (info)
    GB_ReturnNewString(info->pw_name, 0);
  else
    GB_Error((char *)E_MEMORY);

END_PROPERTY


BEGIN_PROPERTY(CUSER_id)

  GB_ReturnInteger(getuid());

END_PROPERTY


BEGIN_PROPERTY(CUSER_group)

  GB_ReturnInteger(getgid());

END_PROPERTY


BEGIN_PROPERTY(CAPPLICATION_name)

  GB_ReturnConstZeroString(PROJECT_name);

END_PROPERTY


BEGIN_PROPERTY(CAPPLICATION_title)

  GB_ReturnConstZeroString(LOCAL_gettext(PROJECT_title));

END_PROPERTY


BEGIN_PROPERTY(CAPPLICATION_id)

  GB_ReturnInt(getpid());

END_PROPERTY


BEGIN_PROPERTY(CAPPLICATION_dir)

  GB_ReturnString(PROJECT_oldcwd);

END_PROPERTY


BEGIN_PROPERTY(CAPPLICATION_version)

  GB_ReturnConstZeroString(PROJECT_version);

END_PROPERTY


BEGIN_PROPERTY(CAPPLICATION_args)

  static OBJECT args;

  args.class = CLASS_AppArgs;
  GB_ReturnObject(&args);

END_PROPERTY


BEGIN_PROPERTY(CAPPLICATION_args_count)

  GB_ReturnInt(PROJECT_argc);

END_PROPERTY


BEGIN_METHOD(CAPPLICATION_args_get, GB_INTEGER index)

  int index = VARG(index);

  if (index < 0)
  {
    GB_Error((char *)E_ARG);
    return;
  }

  if (index >= PROJECT_argc)
    GB_ReturnNull();
  else
    GB_ReturnConstZeroString(PROJECT_argv[index]);

END_METHOD


BEGIN_METHOD_VOID(CAPPLICATION_args_next)

  int *index = (int*)GB_GetEnum();

  if (*index >= PROJECT_argc)
    GB_StopEnum();
  else
  {
    GB_ReturnConstZeroString(PROJECT_argv[*index]);
    (*index)++;
  }

END_METHOD


BEGIN_PROPERTY(CAPPLICATION_env_count)

	int n = 0;
	
	while(environ[n])
		n++;

  GB_ReturnInt(n);

END_PROPERTY


BEGIN_METHOD(CAPPLICATION_env_get, GB_STRING key)

  GB_ReturnConstZeroString(getenv(GB_ToZeroString(ARG(key))));

END_METHOD


BEGIN_METHOD(CAPPLICATION_env_put, GB_STRING value; GB_STRING key)

  setenv(GB_ToZeroString(ARG(key)), GB_ToZeroString(ARG(value)), 1);

END_METHOD


BEGIN_METHOD_VOID(CAPPLICATION_env_next)

  int *index = (int*)GB_GetEnum();
  char *pos;
  char *key;

  if (environ[*index] == NULL)
    GB_StopEnum();
  else
  {
    key = environ[*index];
    pos = strchr(key, '=');
    if (!pos)
      GB_ReturnNull();
    else
      GB_ReturnConstString(key, pos - key);
    (*index)++;
  }

END_METHOD

BEGIN_PROPERTY(CAPPLICATION_return)

  if (READ_PROPERTY)
  	GB_ReturnInteger(EXEC_return_value);
	else
		EXEC_return_value = VPROP(GB_INTEGER) & 0xFF;

END_PROPERTY

static void init_again(int old_pid)
{
	char old[PATH_MAX];
	
	FILE_remove_temp_file();
	snprintf(old, sizeof(old),FILE_TEMP_DIR, getuid(), old_pid);
	rename(old, FILE_make_temp(NULL, NULL));
}

BEGIN_PROPERTY(CAPPLICATION_daemon)

	int old_pid;

	if (READ_PROPERTY)
		GB_ReturnBoolean(_daemon);
	else
	{
		if (!_daemon && VPROP(GB_BOOLEAN))
		{
			old_pid = getpid();
			if (daemon(FALSE, FALSE))
				THROW_SYSTEM(errno, NULL);
			// Argh! daemon() changes the current process id...
			_daemon = TRUE;
			init_again(old_pid);
		}
	}

END_PROPERTY

BEGIN_PROPERTY(Application_Startup)

	GB_ReturnNewZeroString(PROJECT_startup);

END_PROPERTY


BEGIN_PROPERTY(System_Language)

  if (READ_PROPERTY)
    GB_ReturnNewZeroString(LOCAL_get_lang());
  else
    LOCAL_set_lang(GB_ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(System_Charset)

  GB_ReturnString(LOCAL_encoding);

END_PROPERTY


BEGIN_PROPERTY(System_Rtl)

  GB_ReturnBoolean(LOCAL_local.rtl);

END_PROPERTY


BEGIN_PROPERTY(System_Path)

  GB_ReturnString(PROJECT_exec_path);

END_PROPERTY


BEGIN_PROPERTY(System_Host)

  char buffer[256];

  gethostname(buffer, 255);
  GB_ReturnNewZeroString(buffer);

END_PROPERTY


BEGIN_PROPERTY(System_Domain)

  char buffer[256];

  getdomainname(buffer, 255);
  GB_ReturnNewZeroString(buffer);

END_PROPERTY


BEGIN_PROPERTY(System_ByteOrder)

  GB_ReturnInteger(EXEC_big_endian);

END_PROPERTY


BEGIN_PROPERTY(System_Backtrace)

	DEBUG_BACKTRACE *bt = DEBUG_backtrace();
	
	GB_ReturnObject(DEBUG_get_string_array_from_backtrace(bt));
	DEBUG_free_backtrace(&bt);
	
END_PROPERTY


BEGIN_PROPERTY(System_Error)

  GB_ReturnInteger(errno);

END_PROPERTY

#endif

GB_DESC NATIVE_AppArgs[] =
{
  GB_DECLARE(".ApplicationArgs", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("Count", "i", CAPPLICATION_args_count),
  GB_STATIC_METHOD("_get", "s", CAPPLICATION_args_get, "(Index)i"),
  GB_STATIC_METHOD("_next", "s", CAPPLICATION_args_next, NULL),

  GB_END_DECLARE
};


GB_DESC NATIVE_AppEnv[] =
{
  GB_DECLARE(".ApplicationEnv", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("Count", "i", CAPPLICATION_env_count),
  GB_STATIC_METHOD("_get", "s", CAPPLICATION_env_get, "(Key)s"),
  GB_STATIC_METHOD("_put", NULL, CAPPLICATION_env_put, "(Value)s(Key)s"),
  GB_STATIC_METHOD("_next", "s", CAPPLICATION_env_next, NULL),

  GB_END_DECLARE
};


GB_DESC NATIVE_App[] =
{
  GB_DECLARE("Application", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_SELF("Args", ".ApplicationArgs"),
  GB_STATIC_PROPERTY_SELF("Env", ".ApplicationEnv"),
  GB_STATIC_PROPERTY_READ("Path", "s", CAPPLICATION_path),
  GB_STATIC_PROPERTY_READ("Name", "s", CAPPLICATION_name),
  GB_STATIC_PROPERTY_READ("Title", "s", CAPPLICATION_title),
  GB_STATIC_PROPERTY_READ("Id", "i", CAPPLICATION_id),
  GB_STATIC_PROPERTY_READ("Handle", "i", CAPPLICATION_id),
  GB_STATIC_PROPERTY_READ("Version", "s", CAPPLICATION_version),
  GB_STATIC_PROPERTY_READ("Dir", "i", CAPPLICATION_dir),
  GB_STATIC_PROPERTY("Return", "i", CAPPLICATION_return),
  GB_STATIC_PROPERTY("Daemon", "b", CAPPLICATION_daemon),
  GB_STATIC_PROPERTY_READ("Startup", "s", Application_Startup),

  GB_END_DECLARE
};

GB_DESC NATIVE_System[] =
{
  GB_DECLARE("System", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("Path", "s", System_Path),
  GB_CONSTANT("Version", "s", GAMBAS_VERSION_STRING),
  GB_CONSTANT("FullVersion", "s", GAMBAS_FULL_VERSION_STRING),
  GB_STATIC_PROPERTY_READ("Backtrace", "String[]", System_Backtrace),

	GB_STATIC_PROPERTY("Language", "s", System_Language),
  GB_STATIC_PROPERTY_READ("RightToLeft", "b", System_Rtl),
  GB_STATIC_PROPERTY_READ("Charset", "s", System_Charset),
  GB_STATIC_PROPERTY_READ("Host", "s", System_Host),
  GB_STATIC_PROPERTY_READ("Domain", "s", System_Domain),
  GB_STATIC_PROPERTY_READ("ByteOrder", "i", System_ByteOrder),
  GB_STATIC_PROPERTY_READ("Error", "i", System_Error),
  
  GB_CONSTANT("Family", "s", SYSTEM),
  GB_CONSTANT("Architecture", "s", ARCHITECTURE),
  
  GB_STATIC_PROPERTY_SELF("User", "User"),

  GB_END_DECLARE
};

GB_DESC NATIVE_User[] =
{
  GB_DECLARE("User", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("Name", "s", CUSER_name),
  GB_STATIC_PROPERTY_READ("Id", "i", CUSER_id),
  GB_STATIC_PROPERTY_READ("Group", "i", CUSER_group),
  GB_STATIC_PROPERTY_READ("Home", "s", CUSER_home),

  GB_END_DECLARE
};

