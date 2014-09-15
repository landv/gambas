/***************************************************************************

  gbx_c_system.c

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

#define __GBX_C_SYSTEM_C

#include "gambas.h"
#include "gbx_info.h"

#ifndef GBX_INFO

#include "config.h"

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>

#include "gb_common.h"
#include "gb_common_case.h"
#include "gb_error.h"
#include "gbx_api.h"
#include "gbx_class.h"
#include "gbx_date.h"
#include "gbx_project.h"
#include "gbx_local.h"
#include "gbx_event.h"
#include "gbx_string.h"
#include "gbx_exec.h"
#include "gbx_extern.h"
#include "gbx_object.h"
#include "gbx_c_process.h"
#include "gbx_c_system.h"

typedef
	struct {
		const char *prefix;
		int prio;
	}
	SYSLOG_PREFIX;

static const SYSLOG_PREFIX _syslog_prefix[] =
{
	{ "[error]", LOG_ERR },
	{ "[warning]", LOG_WARNING },
	{ "[notice]", LOG_NOTICE },
	{ "[info]", LOG_INFO },
	{ "[debug]", LOG_DEBUG },
	{ NULL, 0 }
};


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

	if (LOCAL_is_UTF8)
		GB_ReturnConstZeroString("UTF-8");
	else
		GB_ReturnString(LOCAL_encoding);

END_PROPERTY


BEGIN_PROPERTY(System_RightToLeft)

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

	STACK_BACKTRACE *bt = STACK_get_backtrace();

	GB_ReturnObject(DEBUG_get_string_array_from_backtrace(bt));
	STACK_free_backtrace(&bt);
	
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


BEGIN_PROPERTY(System_Shell)

	if (READ_PROPERTY)
	{
		if (!CPROCESS_shell)
			GB_ReturnConstZeroString("/bin/sh");
		else
			GB_ReturnString(CPROCESS_shell);
	}
	else
		GB_StoreString(PROP(GB_STRING), &CPROCESS_shell);

END_PROPERTY

BEGIN_PROPERTY(System_Profile)

	if (READ_PROPERTY)
		GB_ReturnBoolean(EXEC_profile_instr);
	else
		EXEC_profile_instr = EXEC_profile && VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_METHOD_VOID(System_Breakpoint)

	BREAKPOINT();

END_METHOD

BEGIN_PROPERTY(System_TimeZone)

	GB_ReturnInteger(DATE_get_timezone());

END_PROPERTY

BEGIN_PROPERTY(System_BreakOnError)

	if (READ_PROPERTY)
		GB_ReturnBoolean(EXEC_break_on_error);
	else if (EXEC_debug)
		EXEC_break_on_error = VPROP(GB_BOOLEAN);

END_METHOD

BEGIN_METHOD(System_Log, GB_STRING message)

	static bool _opened = FALSE;

	const SYSLOG_PREFIX *p;
	char *msg;
	int len;
	int lenp;
	int prio;

	msg = STRING(message);
	len = LENGTH(message);
	prio = LOG_INFO;

	for (p = _syslog_prefix; p->prefix; p++)
	{
		lenp = strlen(p->prefix);
		if (len >= (lenp + 2) && strncasecmp(msg, p->prefix, lenp) == 0)
		{
			msg += lenp + 2;
			len -= lenp + 2;
			prio = p->prio;
			break;
		}
	}

	while (len > 0 && *msg == ' ')
		msg++, len--;

	if (len <= 0)
		return;

	if (!_opened)
	{
		_opened = TRUE;
		openlog(PROJECT_name, LOG_PID, LOG_INFO);
	}

	syslog(prio, "%.*s", len, msg);

END_METHOD

BEGIN_METHOD(System_Find, GB_STRING program)

	const char *path;

	path = CPROCESS_search_program_in_path(GB_ToZeroString(ARG(program)));
	if (!path)
		GB_ReturnNull();
	else
		GB_ReturnNewZeroString(path);

END_METHOD

BEGIN_METHOD(System_Exist, GB_STRING program)

	GB_ReturnBoolean(CPROCESS_search_program_in_path(GB_ToZeroString(ARG(program))) != NULL);

END_METHOD

#endif

//-------------------------------------------------------------------------

GB_DESC NATIVE_User[] =
{
	GB_DECLARE_STATIC("User"),

	GB_STATIC_PROPERTY_READ("Name", "s", User_Name),
	GB_STATIC_PROPERTY_READ("Id", "i", User_Id),
	GB_STATIC_PROPERTY_READ("Group", "i", User_Group),
	GB_STATIC_PROPERTY_READ("Home", "s", User_Home),

	GB_END_DECLARE
};

GB_DESC NATIVE_System[] =
{
	GB_DECLARE_STATIC("System"),

	GB_STATIC_PROPERTY_READ("Path", "s", System_Path),
	GB_CONSTANT("Version", "s", GAMBAS_VERSION_STRING),
	GB_CONSTANT("FullVersion", "s", VERSION),
	GB_STATIC_PROPERTY_READ("Backtrace", "String[]", System_Backtrace),
	GB_STATIC_PROPERTY("BreakOnError", "b", System_BreakOnError),

	GB_STATIC_PROPERTY("Language", "s", System_Language),
	GB_STATIC_PROPERTY("FirstDayOfWeek", "i", System_FirstDayOfWeek),
	GB_STATIC_PROPERTY("Shell", "s", System_Shell),
	GB_STATIC_PROPERTY("Profile", "b", System_Profile),

	GB_STATIC_PROPERTY_READ("RightToLeft", "b", System_RightToLeft),
	GB_STATIC_PROPERTY_READ("Charset", "s", System_Charset),
	GB_STATIC_PROPERTY_READ("Host", "s", System_Host),
	GB_STATIC_PROPERTY_READ("Domain", "s", System_Domain),
	GB_STATIC_PROPERTY_READ("ByteOrder", "i", System_ByteOrder),
	GB_STATIC_PROPERTY_READ("Error", "i", System_Error),
	
	GB_STATIC_PROPERTY_READ("TimeZone", "i", System_TimeZone),

	GB_CONSTANT("Family", "s", SYSTEM),
	GB_CONSTANT("Architecture", "s", ARCHITECTURE),
	
	GB_STATIC_METHOD("GetExternSymbol", "p", System_GetExternSymbol, "(Library)s(Symbol)s"),
	GB_STATIC_METHOD("_Breakpoint", NULL, System_Breakpoint, NULL),

	GB_STATIC_PROPERTY_SELF("User", "User"),

	GB_STATIC_METHOD("Log", NULL, System_Log, "(Message)s"),

	GB_STATIC_METHOD("Exist", "b", System_Exist, "(Program)s"),
	GB_STATIC_METHOD("Find", "s", System_Find, "(Program)s"),

	GB_END_DECLARE
};
