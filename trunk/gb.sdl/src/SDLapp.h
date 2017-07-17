/***************************************************************************

  SDLapp.h

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#ifndef __SDLAPP_H
#define __SDLAPP_H

#include "SDL_h.h"

class SDLapplication
{
public:
	SDLapplication(int &argc, char **argv);
	virtual ~SDLapplication();

	void ManageEvents(bool no_input = false);
	bool HaveWindows(void );

	int DesktopWidth(void );
	int DesktopHeight(void );
	Window X11appRootWin(void );
	Window CurrentWin(void );
	Display* X11appDisplay(void );

	virtual void ManageError(const char* ) = 0;
	// needed if calling X11 funcs !
	void LockX11(void );
	void UnlockX11(void );

private:
	// datas
	static int AppCount;
	static int LockX11Count;
	SDL_SysWMinfo info;
	Display *display;
	Window window;
};

extern SDLapplication *SDLapp;

#endif /* __SDLAPP_H */

