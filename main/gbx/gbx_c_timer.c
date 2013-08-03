/***************************************************************************

  gbx_c_timer.c

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

#define __GBX_C_TIMER_C

#include "gbx_info.h"

#ifndef GBX_INFO

#include "gb_common.h"
#include "gbx_watch.h"
#include "gbx_api.h"
#include "gbx_exec.h"
#include "gbx_event.h"
#include "gbx_c_timer.h"

DECLARE_EVENT(EVENT_Timer);


static void enable_timer(CTIMER *_object, bool on)
{
	if (on != (THIS->id != 0))
		HOOK_DEFAULT(timer, WATCH_timer)((GB_TIMER *)THIS, on);
	if (on && (THIS->id == 0))
		GB_Error("Too many active timers");
}

CTIMER *CTIMER_every(int delay, GB_TIMER_CALLBACK callback, intptr_t param)
{
	CTIMER *timer;
	
	timer = OBJECT_create_native(CLASS_Timer, NULL);
	OBJECT_REF(timer);
	timer->callback = callback;
	timer->delay = delay;
	timer->tag = param;
	
	enable_timer(timer, TRUE);
	
	return timer;
}

void CTIMER_raise(void *_object)
{
	if (THIS->callback)
	{
		if (!(*(THIS->callback))(THIS->tag))
			return;
	}
	else
	{
		if (!GB_Raise(THIS, EVENT_Timer, 0))
			return;
	}
	
	enable_timer(THIS, FALSE);
}


BEGIN_METHOD_VOID(Timer_new)

	THIS->id = 0;
	THIS->delay = 1000;

END_METHOD


BEGIN_METHOD_VOID(Timer_free)

	if (THIS->id)
		HOOK_DEFAULT(timer, WATCH_timer)((GB_TIMER *)THIS, FALSE);

END_METHOD


BEGIN_METHOD_VOID(Timer_Start)

	enable_timer(THIS, TRUE);

END_METHOD


BEGIN_METHOD_VOID(Timer_Stop)

	enable_timer(THIS, FALSE);

END_METHOD


BEGIN_PROPERTY(Timer_Enabled)

	if (READ_PROPERTY)
		GB_ReturnBoolean(THIS->id != 0);
	else
		enable_timer(THIS, VPROP(GB_BOOLEAN));

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

static void trigger_timer(void *_object)
{
	THIS->triggered = FALSE;
	GB_Raise(THIS, EVENT_Timer, 0);
	OBJECT_UNREF(_object);
}

BEGIN_METHOD_VOID(Timer_Trigger)

	if (THIS->triggered)
		return;
	
	THIS->triggered = TRUE;
	OBJECT_REF(THIS);
	EVENT_post(trigger_timer, (intptr_t)THIS);

END_METHOD

#endif

GB_DESC NATIVE_Timer[] =
{
  GB_DECLARE("Timer", sizeof(CTIMER)),

  GB_METHOD("_new", NULL, Timer_new, NULL),
  GB_METHOD("_free", NULL, Timer_free, NULL),

  GB_PROPERTY("Enabled", "b", Timer_Enabled),
  GB_PROPERTY("Delay", "i", CTIMER_delay),
  //GB_PROPERTY_READ("Timeout", "f", Timer_Timeout),

  GB_METHOD("Start", NULL, Timer_Start, NULL),
  GB_METHOD("Stop", NULL, Timer_Stop, NULL),
  GB_METHOD("Trigger", NULL, Timer_Trigger, NULL),
  
  GB_CONSTANT("_IsControl", "b", TRUE),
  GB_CONSTANT("_IsVirtual", "b", TRUE),
  GB_CONSTANT("_Group", "s", "Special"),
  GB_CONSTANT("_Properties", "s", "Enabled,Delay{Range:0;86400000;10;ms}=1000"),
  GB_CONSTANT("_DefaultEvent", "s", "Timer"),

  GB_EVENT("Timer", NULL, NULL, &EVENT_Timer),

  GB_END_DECLARE
};


