/***************************************************************************

  SDLcore.h

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

#ifndef __SDLCORE_H
#define __SDLCORE_H

#include "SDL.h"
#include "SDL_syswm.h"

#include "SDLwindow.h"
#include "SDLapp.h"

#include <string>

class SDLcore
{
public:
	static void Init(void);

	// this window will receive the events
	static void RegisterWindow(SDLwindow* );
	static SDLwindow* GetWindow(void ) { return (hWindow); };
	// for error !
	static void RegisterApplication(SDLapplication* app) { hApplication = app; };
	static void RaiseError(std::string );
private:
	static const SDL_VideoInfo* hVideoInfo;
	// shown window (for events)
	static SDLwindow* hWindow;
	// error propagation
	static SDLapplication* hApplication;
};

#endif /* __SDLCORE_H */

