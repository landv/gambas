/***************************************************************************

  SDLcursor.cpp

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

#include "SDLcursor.h"
#include "SDLcore.h"

#include <iostream>

SDLcursor::SDLcursor()
{
	Display *myDisplay = SDLapp->X11appDisplay();
	hCursor = XcursorLibraryLoadCursor(myDisplay, XcursorGetTheme(myDisplay));
	hShape = SDL::DefaultCursor;
	hImgCursor = NULL;
}

SDLcursor::SDLcursor(const SDLcursor& curs)
{
	hCursor = curs.hCursor;
	hShape = curs.hShape;
	hImgCursor = NULL;

	if (curs.hImgCursor)
	{
		std::cout << curs.hImgCursor->width << " " << curs.hImgCursor->height << std::endl;
		hImgCursor = XcursorImageCreate(curs.hImgCursor->width, curs.hImgCursor->height);
		memcpy (hImgCursor->pixels, curs.hImgCursor->pixels, hImgCursor->width * hImgCursor->height * 4);
	}

}

SDLcursor::~SDLcursor()
{
	if (hImgCursor)
		XcursorImageDestroy(hImgCursor);
}

void SDLcursor::Show(Window w)
{
	Cursor cursor = hCursor;
	int shape = hShape;
	Display *myDisplay = SDLapp->X11appDisplay();

	if (hShape == SDL::BlankCursor)
	{
		SDL_ShowCursor(SDL_DISABLE);
		return;
	}

	if (SDL_ShowCursor(SDL_QUERY) == SDL_DISABLE)
		SDL_ShowCursor(SDL_ENABLE);


	if (shape == SDL::DefaultCursor)
		shape = SDL::ArrowCursor;

	SDLapp->LockX11();
	if (shape != SDL::CustomCursor)
		cursor = XcursorShapeLoadCursor(myDisplay, shape);
	else
		cursor = XcursorImageLoadCursor(myDisplay, hImgCursor);

	XDefineCursor(myDisplay, w, cursor);
	SDLapp->UnlockX11();

}

void SDLcursor::SetShape(int shape)
{
	if (hShape == shape)
		return;

	if (hShape == SDL::CustomCursor && !hImgCursor)
		return;

	hShape = shape;

}

void SDLcursor::SetCursor(SDLsurface *image, int xhot, int yhot)
{
	if (image->IsNull()) {
		hShape = SDL::BlankCursor;
		return;
	}
	
	if (hImgCursor)
		XcursorImageDestroy(hImgCursor);

	hImgCursor = XcursorImageCreate(image->GetWidth(), image->GetHeight());

	if ((xhot < 0))
		xhot = 0;
	if (((unsigned int)xhot > hImgCursor->width))
		xhot = hImgCursor->width;
	if ((yhot < 0))
		yhot = 0;
	if (((unsigned int)yhot > hImgCursor->height))
		yhot = hImgCursor->height;

	memcpy (hImgCursor->pixels, image->GetData(), image->GetWidth() * image->GetHeight() * 4);
	hImgCursor->xhot = xhot;
	hImgCursor->yhot = yhot;
	hShape = SDL::CustomCursor;
}

