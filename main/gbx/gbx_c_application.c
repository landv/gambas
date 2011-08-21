/***************************************************************************

  gbx_c_application.c

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
#include "gbx_extern.h"
#include "gbx_object.h"

#include "gbx_c_application.h"

//static GB_FUNCTION signal_func;
//static bool has_signal_func;
static bool _daemon = FALSE;

extern char **environ;

#if 0
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
#endif

BEGIN_PROPERTY(Application_Path)

  GB_ReturnString(PROJECT_path);

END_PROPERTY


BEGIN_PROPERTY(User_Home)

	GB_ReturnString(PROJECT_get_home());

END_PROPERTY


BEGIN_PROPERTY(User_Name)

  struct passwd *info = getpwuid(getuid());

  if (info)
    GB_ReturnNewZeroString(info->pw_name);
  else
    GB_Error((char *)E_MEMORY);

END_PROPERTY


BEGIN_PROPERTY(User_Id)

  GB_ReturnInteger(getuid());

END_PROPERTY


BEGIN_PROPERTY(User_Group)

  GB_ReturnInteger(getgid());

END_PROPERTY


BEGIN_PROPERTY(Application_Name)

  GB_ReturnConstZeroString(PROJECT_name);

END_PROPERTY


BEGIN_PROPERTY(Application_Title)

  GB_ReturnConstZeroString(LOCAL_gettext(PROJECT_title));

END_PROPERTY


BEGIN_PROPERTY(Application_Id)

  GB_ReturnInt(getpid());

END_PROPERTY


BEGIN_PROPERTY(Application_Dir)

  GB_ReturnString(PROJECT_oldcwd);

END_PROPERTY


BEGIN_PROPERTY(Application_Version)

  GB_ReturnConstZeroString(PROJECT_version);

END_PROPERTY


BEGIN_PROPERTY(CAPPLICATION_args)

  static OBJECT args;

  args.class = CLASS_AppArgs;
  GB_ReturnObject(&args);

END_PROPERTY


BEGIN_PROPERTY(Application_Args_Count)

  GB_ReturnInt(PROJECT_argc);

END_PROPERTY


BEGIN_METHOD(Application_Args_get, GB_INTEGER index)

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


BEGIN_METHOD_VOID(Application_Args_next)

  int *index = (int*)GB_GetEnum();

  if (*index >= PROJECT_argc)
    GB_StopEnum();
  else
  {
    GB_ReturnConstZeroString(PROJECT_argv[*index]);
    (*index)++;
  }

END_METHOD


BEGIN_PROPERTY(Application_Env_Count)

	int n = 0;
	
	while(environ[n])
		n++;

  GB_ReturnInt(n);

END_PROPERTY


BEGIN_METHOD(Application_Env_get, GB_STRING key)

  GB_ReturnConstZeroString(getenv(GB_ToZeroString(ARG(key))));

END_METHOD


BEGIN_METHOD(Application_Env_put, GB_STRING value; GB_STRING key)

  setenv(GB_ToZeroString(ARG(key)), GB_ToZeroString(ARG(value)), 1);

END_METHOD


BEGIN_METHOD_VOID(Application_Env_next)

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

static void init_again(int old_pid)
{
	char old[PATH_MAX];
	
	FILE_remove_temp_file();
	snprintf(old, sizeof(old),FILE_TEMP_DIR, getuid(), old_pid);
	rename(old, FILE_make_temp(NULL, NULL));
}

BEGIN_PROPERTY(Application_Daemon)

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

	GB_ReturnObject(PROJECT_class);

END_PROPERTY


BEGIN_PROPERTY(System_Language)

  if (READ_PROPERTY)
    GB_ReturnNewZeroString(LOCAL_get_lang());
  else
    LOCAL_set_lang(GB_ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(System_FirstDayOfWeek)

  if (READ_PROPERTY)
    GB_ReturnInteger(LOCAL_get_first_day_of_week());
  else
    LOCAL_set_first_day_of_week(VPROP(GB_INTEGER));

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

  if (getdomainname(buffer, 255))
		GB_Error("Unable to retrieve domain name: &1", strerror(errno));
	else
		GB_ReturnNewZeroString(buffer);

END_PROPERTY


BEGIN_PROPERTY(System_ByteOrder)

  GB_ReturnInteger(EXEC_big_endian);

END_PROPERTY


BEGIN_PROPERTY(System_Backtrace)

	ERROR_INFO err;
	
  err.cp = CP;
  err.fp = FP;
  err.pc = PC;
	err.bt_count = STACK_frame_count;

	GB_ReturnObject(DEBUG_get_string_array_from_backtrace(&err));
	
END_PROPERTY


BEGIN_PROPERTY(System_Error)

  GB_ReturnInteger(errno);

END_PROPERTY


BEGIN_METHOD(System_GetExternSymbol, GB_STRING library; GB_STRING name)

	char *library = GB_ToZeroString(ARG(library));
	char *name = GB_ToZeroString(ARG(name));
	void *ptr = NULL;
	
	if (*library && *name)
		ptr = EXTERN_get_symbol(library, name);
	
	GB_ReturnPointer(ptr);

END_METHOD

#endif

GB_DESC NATIVE_AppArgs[] =
{
  GB_DECLARE(".Application.Args", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("Count", "i", Application_Args_Count),
  GB_STATIC_METHOD("_get", "s", Application_Args_get, "(Index)i"),
  GB_STATIC_METHOD("_next", "s", Application_Args_next, NULL),

  GB_END_DECLARE
};


GB_DESC NATIVE_AppEnv[] =
{
  GB_DECLARE(".Application.Env", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("Count", "i", Application_Env_Count),
  GB_STATIC_METHOD("_get", "s", Application_Env_get, "(Key)s"),
  GB_STATIC_METHOD("_put", NULL, Application_Env_put, "(Value)s(Key)s"),
  GB_STATIC_METHOD("_next", "s", Application_Env_next, NULL),

  GB_END_DECLARE
};


GB_DESC NATIVE_App[] =
{
  GB_DECLARE("Application", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_SELF("Args", ".Application.Args"),
  GB_STATIC_PROPERTY_SELF("Env", ".Application.Env"),
  GB_STATIC_PROPERTY_READ("Path", "s", Application_Path),
  GB_STATIC_PROPERTY_READ("Name", "s", Application_Name),
  GB_STATIC_PROPERTY_READ("Title", "s", Application_Title),
  GB_STATIC_PROPERTY_READ("Id", "i", Application_Id),
  GB_STATIC_PROPERTY_READ("Handle", "i", Application_Id),
  GB_STATIC_PROPERTY_READ("Version", "s", Application_Version),
  GB_STATIC_PROPERTY_READ("Dir", "i", Application_Dir),
  GB_STATIC_PROPERTY("Daemon", "b", Application_Daemon),
  GB_STATIC_PROPERTY_READ("Startup", "Class", Application_Startup),

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
	GB_STATIC_PROPERTY("FirstDayOfWeek", "i", System_FirstDayOfWeek),
  GB_STATIC_PROPERTY_READ("RightToLeft", "b", System_Rtl),
  GB_STATIC_PROPERTY_READ("Charset", "s", System_Charset),
  GB_STATIC_PROPERTY_READ("Host", "s", System_Host),
  GB_STATIC_PROPERTY_READ("Domain", "s", System_Domain),
  GB_STATIC_PROPERTY_READ("ByteOrder", "i", System_ByteOrder),
  GB_STATIC_PROPERTY_READ("Error", "i", System_Error),
  
  GB_CONSTANT("Family", "s", SYSTEM),
  GB_CONSTANT("Architecture", "s", ARCHITECTURE),
  
  GB_STATIC_METHOD("GetExternSymbol", "p", System_GetExternSymbol, "(Library)s(Symbol)s"),
  
  GB_STATIC_PROPERTY_SELF("User", "User"),

  GB_END_DECLARE
};

GB_DESC NATIVE_User[] =
{
  GB_DECLARE("User", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("Name", "s", User_Name),
  GB_STATIC_PROPERTY_READ("Id", "i", User_Id),
  GB_STATIC_PROPERTY_READ("Group", "i", User_Group),
  GB_STATIC_PROPERTY_READ("Home", "s", User_Home),

  GB_END_DECLARE
};

