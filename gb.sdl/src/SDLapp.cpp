/***************************************************************************

  SDLapp.cpp

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

#include "SDLapp.h"

#include "SDLcore.h"

#include "SDL.h"
#include "SDL_syswm.h"

#include <iostream>

int SDLapplication::AppCount = 0;
int SDLapplication::LockX11Count = 0;
SDLapplication *SDLapp;

SDLapplication::SDLapplication()
{
	// init is already done !
	if (SDLapplication::AppCount)
	{
		SDLapplication::AppCount++;
		return;
	}

	std::string sMsg = "Failed to init : ";
	Uint32 sysInit = SDL_WasInit(SDL_INIT_EVERYTHING);

	// if audio is defined, sdl was init by gb.sdl.sound component !
	if (sysInit & SDL_INIT_AUDIO)
	{
		if (SDL_InitSubSystem(SDL_INIT_VIDEO)<0)
		{
			sMsg =+ SDL_GetError();
			goto _error;
		}
	}
	else
	{
 		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTTHREAD | SDL_INIT_NOPARACHUTE)<0)
		{
			sMsg =+ SDL_GetError();
			goto _error;
		}
	}

	SDLcore::Init();
	SDLcore::RegisterApplication(this);
	SDLapp = this;

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

	Uint32 sysInit = SDL_WasInit(SDL_INIT_EVERYTHING);

	// if audio is defined, gb.sdl.audio component still not closed !
	if (sysInit & SDL_INIT_AUDIO)
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	else
		SDL_Quit();	
}

void SDLapplication::ManageEvents()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
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
				if (this->HaveWindows() && !SDLcore::GetWindow()->IsOpenGL())
					SDLcore::GetWindow()->Show();
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

long SDLapplication::GetExtents()
{
	long RetData = 0;
#if 0
	XEvent xevent;
	XEvent notifyXevent;
	Window windowId;
	Atom type_ret;
	int format_ret;
	unsigned long nitems_ret, unused;
	unsigned char *data_ret;

	LockX11();
	Atom AtomReqExtents = XInternAtom(SDLapp->X11appDisplay(), "_NET_REQUEST_FRAME_EXTENTS", false);
	Atom AtomFraExtents = XInternAtom(SDLapp->X11appDisplay(), "_NET_FRAME_EXTENTS", false);

	if ((AtomReqExtents == None) || (AtomFraExtents == None))
		std::cout << "Frame extents : something goes wrong" << std::endl;

	windowId = X11appRootWin();
	xevent.xclient.type = ClientMessage;
	xevent.xclient.message_type = AtomReqExtents;
	xevent.xclient.display = X11appDisplay();
	xevent.xclient.window = windowId;
	xevent.xclient.format = 32;
	xevent.xclient.data.l[0] = 0;
	xevent.xclient.data.l[1] = 0;
	xevent.xclient.data.l[2] = 0;
	xevent.xclient.data.l[3] = 0;
	xevent.xclient.data.l[4] = 0;

	XSendEvent (X11appDisplay(), X11appRootWin(), False,
		(SubstructureRedirectMask | SubstructureNotifyMask),
		&xevent);

	XIfEvent(X11appDisplay(), &notifyXevent,
		property_notify_predicate, (XPointer) &windowId);

	if (XGetWindowProperty(X11appDisplay(), CurrentWin(), AtomFraExtents, 0l,
		sizeof (unsigned long) * 4, False, XA_CARDINAL, &type_ret,
		&format_ret, &nitems_ret, &unused, &data_ret) == Success)
	{
		RetData = data_ret[1];
		XFree(data_ret);
	}
	else
		std::cout << "Unable to get FrameExtents !" << std::endl;	

	UnlockX11();

	std::cout << RetData << std::endl;
#endif
	return RetData;
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
