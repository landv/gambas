/***************************************************************************

  SDLcursor.cpp

  Gambas extension using SDL

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>
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

#include "SDLcursor.h"
#include "SDLcore.h"


SDLcursor::SDLcursor()
{
	Display *myDisplay = SDLapp->X11appDisplay();
	hCursor = XcursorLibraryLoadCursor(myDisplay, XcursorGetTheme(myDisplay));
	hShape = SDL::DefaultCursor;
	hImgCursor = NULL;
}

void SDLcursor::Show()
{
	if (hShape == SDL::BlankCursor)
	{
		SDL_ShowCursor(SDL_DISABLE);
		return;
	}

	if (SDL_ShowCursor(SDL_QUERY) == SDL_DISABLE)
		SDL_ShowCursor(SDL_ENABLE);

	Display *myDisplay = SDLapp->X11appDisplay();

	if (hShape == SDL::DefaultCursor)
		hCursor = XcursorLibraryLoadCursor(myDisplay, XcursorGetTheme(myDisplay));
	else
		hCursor = XcursorShapeLoadCursor(myDisplay, hShape);

	XDefineCursor(myDisplay, SDLapp->CurrentWin(), hCursor);
}

void SDLcursor::SetShape(int shape)
{
	if (hShape == shape)
		return;

	hShape = shape;

	if (!SDLcore::GetWindow())
		return;

	if ((SDLcore::GetWindow()->GetCursor() == this) && (SDLcore::GetWindow()->IsShown()))
		this->Show();
}

void SDLcursor::SetShape(SDLsurface *image, int xhot, int yhot)
{
	if (image->IsNull())
		return;
}
