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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __CJOYSTICK_CPP

#include "Cjoystick.h"

#include <map>
#include <iostream>

// store joysticks infos
typedef
  struct {
    Uint8 Axes;
    Uint8 Balls;
    Uint8 Buttons;
    Uint8 Hats;
    std::string Name;
  }
  JOY_info;

static std::map <int, JOY_info> joyinfos;
static std::map <int, SDL_Joystick*> joyobjects;
static int joyindex = 0;

CJOY_INFO CJOY_info = { 0 };

#define CHECK_VALID() \
  if (UNLIKELY(CJOY_info.valid == NULL)) \
  { \
    GB.Error("No joystick event data"); \
    return; \
  }

static void filljoyinfos()
{
	
	int numjoy = SDL_NumJoysticks();
	JOY_info myinfo;

	if (!numjoy)
		return;
	
	for (int i=0; i<numjoy; i++)
	{
		SDL_Joystick* joy = SDL_JoystickOpen(i);
		
		if (!joy)
		{
			std::cerr << "Failed to open joystick " << i << ", skipping!" << std::endl;
			continue;
		}
		
		myinfo.Axes = SDL_JoystickNumAxes(joy);
		myinfo.Balls = SDL_JoystickNumBalls(joy);
		myinfo.Buttons = SDL_JoystickNumButtons(joy);
		myinfo.Hats = SDL_JoystickNumHats(joy);
		myinfo.Name = SDL_JoystickName(i);
		joyinfos[i] = myinfo;
		SDL_JoystickClose(joy);
	}
}

/***************************************************************************/

BEGIN_PROPERTY(JOYINFOS_enable)

	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(joyobjects.count(joyindex));
		return;
	}
	
	if (bool(VPROP(GB_BOOLEAN)) == bool(joyobjects.count(joyindex)))
		return;
	
	if (!VPROP(GB_BOOLEAN))
	{
		SDL_JoystickClose(joyobjects[joyindex]);
		joyobjects.erase(joyindex);
	}
	else
	{
		SDL_Joystick *joy = SDL_JoystickOpen(joyindex);
		
		if (!joy)
			GB.Error(SDL_GetError());
		else
			joyobjects[joyindex] = joy;
	}

END_PROPERTY

BEGIN_PROPERTY(JOYINFOS_numofaxes)

	int num = 0;
	
	if (joyinfos.count(joyindex))
		num = joyinfos[joyindex].Axes;

	GB.ReturnInteger(num);

END_PROPERTY

BEGIN_PROPERTY(JOYINFOS_numofballs)

	int num = 0;
	
	if (joyinfos.count(joyindex))
		num = joyinfos[joyindex].Balls;

	GB.ReturnInteger(num);

END_PROPERTY

BEGIN_PROPERTY(JOYINFOS_numofbuts)

	int num = 0;
	
	if (joyinfos.count(joyindex))
		num = joyinfos[joyindex].Buttons;

	GB.ReturnInteger(num);

END_PROPERTY

BEGIN_PROPERTY(JOYINFOS_numofhats)

	int num = 0;
	
	if (joyinfos.count(joyindex))
		num = joyinfos[joyindex].Hats;

	GB.ReturnInteger(num);

END_PROPERTY

BEGIN_PROPERTY(JOYINFOS_name)

	std::string joyname = "Unknown";
	
	if (joyinfos.count(joyindex))
		joyname = joyinfos[joyindex].Name;

	GB.ReturnNewZeroString(joyname.c_str());

END_PROPERTY

BEGIN_METHOD(JOYSTICKS_get, GB_INTEGER index)

	int numjoy = SDL_NumJoysticks();
	int index = VARGOPT(index, 0);

	if (!numjoy)
	{
		GB.Error("no Joystick found !");
		return;
	}

	if (index>=numjoy || index<0)
	{
		GB.Error("Joystick &1 not available !", VARG(index));
		return;
	}

        joyindex = index;
	
	if (!joyinfos.size())
		filljoyinfos();
	
	RETURN_SELF();

END_METHOD

BEGIN_PROPERTY(JOYSTICKS_count)

	GB.ReturnInteger(SDL_NumJoysticks());

END_METHOD

BEGIN_PROPERTY(JOYSTICK_device)

	CHECK_VALID();
	GB.ReturnInteger(CJOY_info.device);

END_METHOD

BEGIN_PROPERTY(JOYSTICK_id)

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

GB_DESC CJoyInfos[] =
{
  GB_DECLARE(".Joystick", 0), GB_VIRTUAL_CLASS(),
  
  GB_STATIC_PROPERTY("Enable", "b", JOYINFOS_enable),

  GB_STATIC_PROPERTY_READ("Axes", "i", JOYINFOS_numofaxes),
  GB_STATIC_PROPERTY_READ("Balls", "i", JOYINFOS_numofballs),
  GB_STATIC_PROPERTY_READ("Buttons", "i", JOYINFOS_numofbuts),
  GB_STATIC_PROPERTY_READ("Hats", "i", JOYINFOS_numofhats),
  GB_STATIC_PROPERTY_READ("Name", "s", JOYINFOS_name),
 
  GB_END_DECLARE
};

GB_DESC CQueryJoys[] =
{
  GB_DECLARE("Joysticks", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_get", ".Joystick", JOYSTICKS_get, "(Index)i"),
  GB_STATIC_PROPERTY_READ("Count", "i",  JOYSTICKS_count),

  GB_END_DECLARE
};

GB_DESC CJoystick[] =
{
  GB_DECLARE("Joystick", 0), GB_VIRTUAL_CLASS(),

//  TODO close all opened joysticks on exit ?
//  GB_STATIC_METHOD("_exit", NULL, JOYSTICK_exit, NULL),

  GB_STATIC_PROPERTY_READ("Device", "i", JOYSTICK_device),
  GB_STATIC_PROPERTY_READ("Id", "i", JOYSTICK_id),
  GB_STATIC_PROPERTY_READ("Axis", "i", JOYSTICK_axisvalue),
  GB_STATIC_PROPERTY_READ("Hat", "i", JOYSTICK_hatvalue),
  GB_STATIC_PROPERTY_READ("BallX", "i", JOYSTICK_ballx),
  GB_STATIC_PROPERTY_READ("BallY", "i", JOYSTICK_bally),
  
  GB_CONSTANT("LeftUp", "i", SDL_HAT_LEFTUP),
  GB_CONSTANT("Left", "i", SDL_HAT_LEFT),
  GB_CONSTANT("LeftDown", "i", SDL_HAT_LEFTDOWN),
  GB_CONSTANT("Up", "i", SDL_HAT_UP),
  GB_CONSTANT("Centered", "i", SDL_HAT_CENTERED),
  GB_CONSTANT("Down", "i", SDL_HAT_DOWN),
  GB_CONSTANT("RightUp", "i", SDL_HAT_RIGHTUP),
  GB_CONSTANT("Right", "i", SDL_HAT_RIGHT),
  GB_CONSTANT("RightDown", "i", SDL_HAT_RIGHTDOWN),

  GB_END_DECLARE
};

