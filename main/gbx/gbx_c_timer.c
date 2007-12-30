/***************************************************************************

  gbx_c_timer.c

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

#define __GBX_C_TIMER_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include "gb_common.h"
#include "gbx_watch.h"
#include "gbx_api.h"
#include "gbx_exec.h"
#include "gbx_c_timer.h"

DECLARE_EVENT(EVENT_Timer);


PUBLIC void CTIMER_raise(void *_object)
{
	GB_Raise(THIS, EVENT_Timer, 0);
}

BEGIN_METHOD_VOID(CTIMER_new)

	THIS->id = 0;
	THIS->delay = 1000;

END_METHOD

BEGIN_METHOD_VOID(CTIMER_free)

	if (THIS->id)
		HOOK_DEFAULT(timer, WATCH_timer)((GB_TIMER *)THIS, FALSE);

END_METHOD

BEGIN_PROPERTY(CTIMER_enabled)

	if (READ_PROPERTY)
		GB_ReturnBoolean(THIS->id != 0);
	else
	{
		bool on = VPROP(GB_BOOLEAN);
		if (on != (THIS->id != 0))
			HOOK_DEFAULT(timer, WATCH_timer)((GB_TIMER *)THIS, on);
		if (on && (THIS->id == 0))
			GB_Error("Too many active timers");
	}

END_PROPERTY

BEGIN_PROPERTY(CTIMER_delay)

	if (READ_PROPERTY)
		GB_ReturnInteger(THIS->delay);
	else
	{
		int delay = VPROP(GB_INTEGER);
		bool enabled = THIS->id != 0;

		if (delay > 0)
		{
			if (enabled)
				HOOK_DEFAULT(timer, WATCH_timer)((GB_TIMER *)THIS, FALSE);

			THIS->delay = delay;

			if (enabled)
				HOOK_DEFAULT(timer, WATCH_timer)((GB_TIMER *)THIS, TRUE);
		}
	}

END_PROPERTY

#endif

PUBLIC GB_DESC NATIVE_Timer[] =
{
  GB_DECLARE("Timer", sizeof(CTIMER)),

  GB_METHOD("_new", NULL, CTIMER_new, NULL),
  GB_METHOD("_free", NULL, CTIMER_free, NULL),

  GB_PROPERTY("Enabled", "b", CTIMER_enabled),
  GB_PROPERTY("Delay", "i", CTIMER_delay),

  GB_CONSTANT("_Properties", "s", "Enabled,Delay{Range:0;86400000;10;ms}=1000"),
  GB_CONSTANT("_DefaultEvent", "s", "Timer"),

  GB_EVENT("Timer", NULL, NULL, &EVENT_Timer),

  GB_END_DECLARE
};


