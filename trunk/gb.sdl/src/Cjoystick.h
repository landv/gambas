/***************************************************************************

  Cjoystick.h

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

#ifndef __CJOYSTICK_H
#define __CJOYSTICK_H

#include "main.h"
#include "SDL.h"

typedef
  struct {
    bool valid;
    Uint8 device;   /* Joystick that raised the event */
    int id;         /* Can be axis, ball, hat, button id */
    Sint16 value1;  /* Can be value for axis, hat; xrel for ball */
    Sint16 value2;  /* yrel value for ball */
    }
  CJOY_INFO;

#ifndef __CJOYSTICK_CPP
extern GB_DESC CJoyInfos[];
extern GB_DESC CQueryJoys[];
extern GB_DESC CJoystick[];
extern CJOY_INFO CJOY_info;
#else

#define JOYSTICK ((CJOYSTICK *)_object)->joy
#define THIS     ((CJOYSTICK *)_object)

#endif /* __CJOYSTICK_CPP */
#endif /* __CJOYSTCIK_H */

