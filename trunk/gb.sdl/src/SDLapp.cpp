/***************************************************************************

  SDLapp.cpp

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

#include "SDLapp.h"

#include "SDLcore.h"
#include "SDLwindow.h"
#include "SDL_ttf.h"

#include <iostream>
#include <string>

int SDLapplication::AppCount = 0;
int SDLapplication::LockX11Count = 0;

SDLapplication *SDLapp;

SDLapplication::SDLapplication(int &argc, char **argv)
{
	// init is already done !
	if (SDLapplication::AppCount)
	{
		SDLapplication::AppCount++;
		return;
	}

	std::string sMsg = "Failed to init: ";
	Uint32 sysInit = SDL_WasInit(SDL_INIT_EVERYTHING);

	// if audio is defined, sdl was init by gb.sdl.sound component !
	if (sysInit & SDL_INIT_AUDIO)
	{
		if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)<0)
		{
			sMsg =+ SDL_GetError();
			goto _error;
		}
	}
	else
	{
 		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE | SDL_INIT_JOYSTICK)<0)
		{
			sMsg =+ SDL_GetError();
			goto _error;
		}
	}

	if (TTF_Init()<0)
	{
		sMsg =+ TTF_GetError();
		goto _error;
	}

	SDLapp = this;
	
	SDL_EnableUNICODE(1);
	
	SDLcore::Init();
	SDLdebug::Init();

	return;

_error:
	std::cout << sMsg << std::endl;
	exit (-1);
}

SDLapplication::~SDLapplication()
{
	// stop SDL only if it's the last ~SDLapplication call
	if (SDLapplication::AppCount>1)
	{
		SDLapplication::AppCount--;
		return;
	}

	TTF_Quit();
	Uint32 sysInit = SDL_WasInit(SDL_INIT_EVERYTHING);

	// if audio is defined, gb.sdl.audio component still not closed !
	if (sysInit & SDL_INIT_AUDIO)
		SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
	else
		SDL_Quit();
}

static int poll_event(SDL_Event *event, bool no_input)
{
	uint mask;
	
	SDL_PumpEvents();

	mask = SDL_ALLEVENTS;
	if (no_input)
		mask ^= SDL_KEYEVENTMASK | SDL_MOUSEEVENTMASK | SDL_JOYEVENTMASK | SDL_QUITMASK;
	
	/* We can't return -1, just return 0 (no event) on error */
	if ( SDL_PeepEvents(event, 1, SDL_GETEVENT, mask) <= 0 )
		return 0;
	return 1;
}


void SDLapplication::ManageEvents(bool no_input)
{
	SDL_Event event;

	while (poll_event(&event, no_input))
	{
		if (!this->HaveWindows())
			break;

		switch(event.type)
			{
			case SDL_QUIT:
				SDLcore::GetWindow()->Quit();
				break;
			case SDL_ACTIVEEVENT:
				if (event.active.state==SDL_APPINPUTFOCUS)
				{
					if (event.active.gain)
						SDLcore::GetWindow()->GotFocus();
					else
						SDLcore::GetWindow()->LostFocus();
				}
				if (event.active.state==SDL_APPMOUSEFOCUS)
				{
					if (event.active.gain)
						SDLcore::GetWindow()->MouseEnter();
					else
						SDLcore::GetWindow()->MouseLeave();
				}
				break;
			case SDL_VIDEORESIZE:
				SDLcore::GetWindow()->SetWidth(event.resize.w);
				SDLcore::GetWindow()->SetHeight(event.resize.h);
				SDLcore::GetWindow()->Resize();

				 // take care if window is closed during resize
				if (this->HaveWindows())
					SDLcore::GetWindow()->Show();
				break;
			case SDL_JOYAXISMOTION:
			case SDL_JOYHATMOTION:
			case SDL_JOYBALLMOTION:
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				// Only raise joysticks events if window has input keyboard focus
				if (SDL_GetAppState() & SDL_APPINPUTFOCUS)
					SDLcore::GetWindow()->JoyEvent(event);
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				SDLcore::GetWindow()->KeyEvent(&event.key, event.type);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				SDLcore::GetWindow()->MouseButtonEvent(&event.button);
				break;					
			case SDL_MOUSEMOTION:
				SDLcore::GetWindow()->MouseMotionEvent(&event.motion);
				break;
			default:
				break;
			}
	}

	if (this->HaveWindows()) // take care if window wasn't close during events
		SDLcore::GetWindow()->Update();

}

bool SDLapplication::HaveWindows()
{
	return (SDLcore::GetWindow() ? true : false);
}

int SDLapplication::DesktopWidth()
{
	LockX11();
	int Width = XDisplayWidth(display, DefaultScreen(display));
	UnlockX11();

	return (Width);
}

int SDLapplication::DesktopHeight()
{
	LockX11();
	int Height = XDisplayHeight(display, DefaultScreen(display));
	UnlockX11();

	return (Height);
}

Window SDLapplication::X11appRootWin()
{
	LockX11();
	Window win = XDefaultRootWindow(display);
	UnlockX11();

	return (win);
}

Window SDLapplication::CurrentWin()
{
	LockX11();
	// refresh window variable ;)
	UnlockX11();

	return (window);
}

Display *SDLapplication::X11appDisplay()
{
	LockX11();
	// refresh display variable ;)
	UnlockX11();

	return (display);
}

void SDLapplication::LockX11()
{
	SDLapplication::LockX11Count++;
	SDL_VERSION(&info.version);
	SDL_GetWMInfo(&info);

	if (SDLapplication::LockX11Count==1)
		info.info.x11.lock_func();

	display = info.info.x11.display;
	window = info.info.x11.window;
}

void SDLapplication::UnlockX11()
{
	SDLapplication::LockX11Count--;

	if (SDLapplication::LockX11Count>1)
		return;

	SDLapplication::LockX11Count = 0;
	info.info.x11.unlock_func();
}

