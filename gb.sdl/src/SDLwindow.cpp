/***************************************************************************

  SDLwindow.cpp

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#include "SDLcore.h"
#include "SDLwindow.h"

#include <iostream>

SDLwindow::SDLwindow(bool openGL)
{
	hSurfaceInfo = new SDL_INFO;
	hCursor = new SDLcursor();
	hSurface = 0;
	hTexture = 0;
	hTextureWidth = 0;
	hTextureHeight = 0;
	hContext = NULL;
	hOpenGL = openGL;
	hX = hY = 0;
	hWidth = 640;
	hHeight = 480;
	hFullScreen = false;
	hResizable = false;
	hTitle = (char *)"SDL application";
}

SDLwindow::~SDLwindow()
{
	this->Close();

	delete hCursor;
	// find the good way to manage windows surfaces !
	delete hSurfaceInfo;
}

void SDLwindow::Show()
{
	Uint32 myFlags = (SDL_ASYNCBLIT | SDL_DOUBLEBUF);

	if (hOpenGL)
	{
		myFlags = myFlags | SDL_OPENGL;
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	}

	if (hFullScreen)
		myFlags = myFlags | SDL_FULLSCREEN;

	if (hResizable)
		myFlags = myFlags | SDL_RESIZABLE;

	hSurface = SDL_SetVideoMode(hWidth, hHeight , 0, myFlags);

	if (!hSurface)
	{
		SDLcore::RaiseError(SDL_GetError());
		return;
	}

	/* Get this, needed for glXMakeCurrent() */
	hContext = glXGetCurrentContext();
	/* Set our custom cursor */
	hCursor->Show();

	SDL_WM_SetCaption(hTitle, hTitle);

	if (SDLcore::GetWindow() != this)
		SDLcore::RegisterWindow(this);

	Clear();
	Open();

}

void SDLwindow::Close()
{
	if (!hSurface)
		return;

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	SDLcore::RegisterWindow(NULL);
	hSurface = NULL;
}

void SDLwindow::Refresh()
{
	if (!hSurface)
		return;

	if (hOpenGL)
		SDL_GL_SwapBuffers();
	else
		SDL_Flip(hSurface);
}

void SDLwindow::Clear(Uint32 color)
{
	if (!hSurface)
		return;

	glClearColor((GLfloat((color >> 24) & 0xFF)/255), (GLfloat((color >> 16) & 0xFF)/255), (GLfloat((color >> 8) & 0xFF)/255), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

}

Uint32 SDLwindow::Id(void )
{
	if (!hSurface)
		return 0;

	return (SDLapp->CurrentWin());
}

int SDLwindow::GetWidth()
{
	if (hSurface)
		return (hSurface->w);
	else
		return (hWidth);
}

int SDLwindow::GetHeight()
{
	if (hSurface)
		return (hSurface->h);
	else
		return (hHeight);
}

int SDLwindow::GetDepth()
{
	if (hSurface)
		return (hSurface->format->BitsPerPixel);
	else
		return (0);
}

void SDLwindow::SetWidth(int width)
{
	hWidth = width;

	if (hSurface)
		this->Show();
}

void SDLwindow::SetHeight(int height)
{
	hHeight = height;

	if (hSurface)
		this->Show();
}

void SDLwindow::SetFullScreen(bool choice)
{
	if ((choice && !hFullScreen) || (!choice && hFullScreen))
	{
		if (hSurface)
		{
			if (!SDL_WM_ToggleFullScreen(hSurface))
				SDLcore::RaiseError(SDL_GetError());
		}
		hFullScreen = !hFullScreen;
	}
}

void SDLwindow::SetResizable(bool choice)
{
	if (!hSurface)
	{
		hResizable = !hResizable;
		return;
	}

	if (((hSurface->flags & SDL_RESIZABLE) && !choice) || (!(hSurface->flags & SDL_RESIZABLE) && choice))
	{
		hResizable = !hResizable;
		this->Show();
	}
}

void SDLwindow::SetTitle(char *title)
{
	if (!hSurface)
		hTitle = title;
	else
		SDL_WM_SetCaption(hTitle, hTitle);
}

void SDLwindow::SetCursor(SDLcursor *cursor)
{
	if (hCursor)
		delete hCursor;

	SDLcursor *curs=new SDLcursor(*cursor);
	hCursor = curs;

	if (this->IsShown())
		hCursor->Show();
}

bool SDLwindow::IsShown()
{
	if (!hSurface)
		return (false);

	if (SDLcore::GetWindow() == this)
		return (true);
	else
		return (false);
}

