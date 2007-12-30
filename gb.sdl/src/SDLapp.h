/***************************************************************************

  SDLapp.h

  Gambas extension using SDL

  (c) 2006 Laurent Carlier <lordheavy@infonie.fr>
           Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __SDLAPP_H
#define __SDLAPP_H

#include "SDL_syswm.h"

class SDLapplication
{
public:
	SDLapplication();
	virtual ~SDLapplication();

	void ManageEvents(void );
	bool HaveWindows(void );

	int DesktopWidth(void );
	int DesktopHeight(void );
	Window X11appRootWin(void );
	Window CurrentWin(void );
	Display* X11appDisplay(void );
	long GetExtents(void );

	virtual void ManageError(const char* ) = 0;
	// needed is calling X11 funcs !
	void LockX11(void );
	void UnlockX11(void );
private:

	static int AppCount;
	static int LockX11Count;
	SDL_SysWMinfo info;
	Display *display;
	Window window;
};

extern SDLapplication *SDLapp;

#endif /* __SDLAPP_H */

