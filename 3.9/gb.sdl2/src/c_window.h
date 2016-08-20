/***************************************************************************

  c_window.h

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

#ifndef __C_WINDOW_H
#define __C_WINDOW_H

#include "main.h"
#include "c_mouse.h"
#include "c_key.h"

typedef
	struct CWINDOW {
		GB_BASE ob;
		LIST list;
		SDL_Window *window;
		SDL_Renderer *renderer;
		SDL_GLContext context;
		int id;
		int x;
		int y;
		int width;
		int height;
		int save_x, save_y, save_width, save_height;
		uint start_time;
		uint frame_count;
		uint total_frame_count;
		double last_time;
		double frame_time;
		double frame_rate;
		MOUSE_INFO mouse;
		unsigned opengl : 1;
		unsigned opened : 1;
		unsigned fullscreen : 1;
		unsigned clear : 1;
		unsigned resizable : 1;
	}
	CWINDOW;

#ifndef __C_WINDOW_C

extern GB_DESC WindowDesc[];

extern CWINDOW *WINDOW_list;

CWINDOW *WINDOW_get_from_event(SDL_Event *event);
void WINDOW_handle_event(SDL_Event *event);
void WINDOW_update();

#endif
	
#endif /* __C_WINDOW_H */

