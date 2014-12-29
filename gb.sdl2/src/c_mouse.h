/***************************************************************************

  c_mouse.h

  (c) 2014 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __C_MOUSE_H
#define __C_MOUSE_H

#include "main.h"

#ifndef __C_MOUSE_C
extern GB_DESC MouseDesc[];
#endif

typedef
	struct {
		int x;
		int y;
		int wheel_x;
		int wheel_y;
		int state;
		int button;
		int start_x;
		int start_y;
	}
	MOUSE_INFO;

SDL_Event *MOUSE_enter_event(SDL_Event *event);
void MOUSE_leave_event(SDL_Event *event);

#endif /* __C_MOUSE_H */

