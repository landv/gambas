/***************************************************************************

  SDLcore.cpp

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

#include "SDLcore.h"

const SDL_VideoInfo *SDLcore::hVideoInfo = NULL;
SDLwindow *SDLcore::hWindow = NULL;
SDLapplication *SDLcore::hApplication = NULL; 

void SDLcore::RegisterWindow(SDLwindow *window)
{
	if (window)
	{
		if (hWindow)
			hWindow->Close();
	}

	hWindow = window;
}

void SDLcore::Init(void)
{
	hVideoInfo = SDL_GetVideoInfo();
}

void SDLcore::RaiseError(std::string error)
{
	if (hApplication)
		hApplication->ManageError(error.c_str());
}
