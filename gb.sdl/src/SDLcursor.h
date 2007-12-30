/***************************************************************************

  SDLcursor.h

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

#ifndef __SDLCURSOR_H
#define __SDLCURSOR_H

#include "SDL_h.h"
#include "SDLsurface.h"

#include <X11/Xlib.h> // needed with xorg 7.0, already included with xorg 7.1 in Xcursor.h
#include <X11/Xcursor/Xcursor.h>

class SDLcursor
{
public:
	SDLcursor();
	~SDLcursor() {};

	void Show(void );
	void SetShape(int );
	void SetShape(SDLsurface* image, int xhot = -1, int yhot = -1);

private:
	Cursor hCursor;
	int hShape;
	XcursorImage *hImgCursor;
};

#endif /* SDLCURSOR_H */
