/***************************************************************************

  Cjoystick.cpp

  (c) 2011 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#define __CJOYSTICK_CPP

#include "Cjoystick.h"

#include <map>

// associate joy ids with gambas objects
std::map <int, void*> joyobjects;

CJOY_INFO CJOY_info = { 0 };

#define CHECK_VALID() \
  if (UNLIKELY(CJOY_info.valid == NULL)) \
  { \
    GB.Error("No joystick event data"); \
    return; \
  }

/***************************************************************************/

BEGIN_METHOD(JOYSTICKS_get, GB_INTEGER index)

	int numjoy = SDL_NumJoysticks();
	int index = VARGOPT(index, 0);
	const char* joyname;

	if (!numjoy)
	{
		GB.Error("no Joystick found !");
		return;
	}

	if (index>numjoy || index<0)
	{
		GB.Error("Joystick &1 not available !", VARG(index));
		return;
	}

	joyname = SDL_JoystickName(index);
	
	if (!joyname)
		GB.ReturnConstZeroString("Unknown");
	else
		GB.ReturnConstZeroString(joyname);

END_METHOD

BEGIN_PROPERTY(JOYSTICKS_count)

	GB.ReturnInteger(SDL_NumJoysticks());

END_METHOD

BEGIN_METHOD(JOYSTICK_new, GB_INTEGER index)

	int numjoy = SDL_NumJoysticks();
	int index = VARGOPT(index, 0);

	if (!numjoy)
	{
		GB.Error("no Joystick found !");
		return;
	}

	if (index>numjoy || index <0)
	{
		GB.Error("Invalid joystick index!");
		return;
	}

	if (joyobjects.count(index))
	{
		GB.Error("Joystick &1 already opened!", index);
		return;
	}
	
	JOYSTICK = SDL_JoystickOpen(index);
	THIS->id = index;

	if (!JOYSTICK)
		GB.Error(SDL_GetError());

	joyobjects[THIS->id] = THIS;

END_METHOD

BEGIN_METHOD_VOID(JOYSTICK_free)

	joyobjects.erase(THIS->id);
	SDL_JoystickClose(JOYSTICK);

END_METHOD

BEGIN_PROPERTY(JOYSTICK_numofaxes)

	GB.ReturnInteger(SDL_JoystickNumAxes(JOYSTICK));

END_PROPERTY

BEGIN_PROPERTY(JOYSTICK_numofballs)

	GB.ReturnInteger(SDL_JoystickNumBalls(JOYSTICK));

END_PROPERTY

BEGIN_PROPERTY(JOYSTICK_numofbuts)

	GB.ReturnInteger(SDL_JoystickNumButtons(JOYSTICK));

END_PROPERTY

BEGIN_PROPERTY(JOYSTICK_numofhats)

	GB.ReturnInteger(SDL_JoystickNumHats(JOYSTICK));

END_PROPERTY

BEGIN_PROPERTY(JOYSTICK_device)

	CHECK_VALID();
	GB.ReturnInteger(CJOY_info.id);

END_PROPERTY

BEGIN_PROPERTY(JOYSTICK_axisvalue)

	CHECK_VALID();
	GB.ReturnInteger(CJOY_info.value1);

END_PROPERTY

BEGIN_PROPERTY(JOYSTICK_hatvalue)

	CHECK_VALID();
	GB.ReturnInteger(CJOY_info.value1);

END_PROPERTY

BEGIN_PROPERTY(JOYSTICK_ballx)

	CHECK_VALID();
	GB.ReturnInteger(CJOY_info.value1);

END_PROPERTY

BEGIN_PROPERTY(JOYSTICK_bally)

	CHECK_VALID();
	GB.ReturnInteger(CJOY_info.value2);

END_PROPERTY

/***************************************************************************/

GB_DESC CQueryJoys[] =
{
  GB_DECLARE("Joysticks", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_get", "s", JOYSTICKS_get, "(Index)i"),
  GB_STATIC_PROPERTY_READ("Count", "i",  JOYSTICKS_count),

  GB_END_DECLARE
};

GB_DESC CJoystick[] =
{
  GB_DECLARE("Joystick", sizeof(CJOYSTICK)),

  GB_METHOD("_new",  NULL, JOYSTICK_new,  "[(Index)i]"),
  GB_METHOD("_free", NULL, JOYSTICK_free, NULL),

  GB_PROPERTY_READ("NumberOfAxes", "i", JOYSTICK_numofaxes),
  GB_PROPERTY_READ("NumberOfBalls", "i", JOYSTICK_numofballs),
  GB_PROPERTY_READ("NumberOfButtons", "i", JOYSTICK_numofbuts),
  GB_PROPERTY_READ("NumberOfHats", "i", JOYSTICK_numofhats),
  
  GB_PROPERTY_READ("Device", "i", JOYSTICK_device),
  GB_PROPERTY_READ("AxisValue", "i", JOYSTICK_axisvalue),
  GB_PROPERTY_READ("HatValue", "i", JOYSTICK_hatvalue),
  GB_PROPERTY_READ("BallXValue", "i", JOYSTICK_ballx),
  GB_PROPERTY_READ("BallYValue", "i", JOYSTICK_bally),
  
  GB_CONSTANT("HatLeftUp", "i", SDL_HAT_LEFTUP),
  GB_CONSTANT("HatLeft", "i", SDL_HAT_LEFT),
  GB_CONSTANT("HatLeftDown", "i", SDL_HAT_LEFTDOWN),
  GB_CONSTANT("HatUp", "i", SDL_HAT_UP),
  GB_CONSTANT("HatCentered", "i", SDL_HAT_CENTERED),
  GB_CONSTANT("HatDown", "i", SDL_HAT_DOWN),
  GB_CONSTANT("HatRightUp", "i", SDL_HAT_RIGHTUP),
  GB_CONSTANT("HatRight", "i", SDL_HAT_RIGHT),
  GB_CONSTANT("HatRightDown", "i", SDL_HAT_RIGHTDOWN),

  GB_EVENT("AxisMotion", NULL, NULL, &EVENT_AxisMotion),
  GB_EVENT("BallMotion", NULL, NULL, &EVENT_BallMotion),
  GB_EVENT("ButtonPressed", NULL, NULL, &EVENT_ButtonPressed),
  GB_EVENT("ButtonReleased", NULL, NULL, &EVENT_ButtonReleased),
  GB_EVENT("HatMotion", NULL, NULL, &EVENT_HatMotion),

  GB_END_DECLARE
};

