/***************************************************************************

  SDLcursor.h

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

#ifndef __SDLCURSOR_H
#define __SDLCURSOR_H

#include "SDL_h.h"
#include "SDLsurface.h"

#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>

class SDLcursor
{
public:
	SDLcursor();
	SDLcursor(const SDLcursor& cursor);
	~SDLcursor();

	void Show(Window );
	void SetShape(int );
	void SetCursor(SDLsurface* image, int xhot = -1, int yhot = -1);

	int GetShape(void ) { return hShape; }

private:
	Cursor hCursor;
	int hShape;
	XcursorImage *hImgCursor;
};

#endif /* SDLCURSOR_H */
