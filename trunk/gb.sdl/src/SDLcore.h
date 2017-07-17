/***************************************************************************

  SDLcore.h

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

#ifndef __SDLCORE_H
#define __SDLCORE_H

#include "SDL_h.h"

#include <string>

class SDLwindow;
class SDLapplication;

class SDLcore
{
public:
	static void Init(void);

	// this window will receive the events
	static void RegisterWindow(SDLwindow* );
	static SDLwindow* GetWindow(void ) { return (hWindow); };
	static void RaiseError(std::string );
private:
	static const SDL_VideoInfo* hVideoInfo;
	// shown window (for events)
	static SDLwindow* hWindow;
};

#endif /* __SDLCORE_H */

