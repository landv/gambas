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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#include "SDLwindow.h"
#include "SDLcore.h"
#include "SDLtexture.h"

#include <iostream>

SDLwindow::SDLwindow(void )
{
	hSurface = 0;
	hCursor = new SDLcursor();
	hX = hY = 0;
	hWidth = 640;
	hHeight = 480;
	hFullScreen = false;
	hResizable = true;
	hTitle = "Gambas SDL application";
}

SDLwindow::~SDLwindow()
{
	this->Close();
	delete hCursor;
}

void SDLwindow::Show()
{
	Uint32 myFlags = (SDL_ASYNCBLIT | SDL_DOUBLEBUF | SDL_OPENGL);
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );

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

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		SDLcore::RaiseError((char *)glewGetErrorString(err));
		return;
	}

	hCtx = glXGetCurrentContext();
	hDrw = glXGetCurrentDrawable();
	hDpy = glXGetCurrentDisplay();

	/* Set our custom cursor */
	hCursor->Show(SDLapp->CurrentWin());

	SDL_WM_SetCaption(hTitle.c_str(), hTitle.c_str());

	if (SDLcore::GetWindow() != this)
		SDLcore::RegisterWindow(this);

//	GL::init();
	SDLtexture::init();

	Clear();
	Open();

}

void SDLwindow::Close()
{
	if (!hSurface)
		return;

	GrabInput(false);
	SDLcore::RegisterWindow(NULL);
	hSurface = NULL;
}

void SDLwindow::Refresh()
{
	if (!hSurface)
		return;

	SDL_GL_SwapBuffers();
}

void SDLwindow::Select(void )
{
	if (!hSurface)
		return;

	if ((glXGetCurrentContext()!=hCtx) && (glXGetCurrentDrawable()!=hDrw))
	{
		std::cout << "Set window current with glXMakeCurrent()" << std::endl;
		glXMakeCurrent(hDpy, hDrw, hCtx);
	}
	else
	{
		SDLtexture::Unselect();
/*		this->Open();*/
	}
}

void SDLwindow::Clear(Uint32 color)
{
	if (!hSurface)
		return;

	glClearColor((GLfloat((color >> 16) & 0xFF)/255), (GLfloat((color >> 8) & 0xFF)/255),
                   (GLfloat((color & 0xFF)/255)), (GLfloat(~(color >> 24) & 0xFF)/255));
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

void* SDLwindow::GetData()
{
	if (hSurface)
		return (hSurface->pixels);
	else
		return (0);
}

void SDLwindow::SetCursorShape(int shape)
{
      if (!hCursor)
            return;

      hCursor->SetShape(shape);
      
      if (this->IsShown())
            hCursor->Show(SDLapp->CurrentWin());
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
		hResizable = choice;
		return;
	}

	if (((hSurface->flags & SDL_RESIZABLE) && !choice) || (!(hSurface->flags & SDL_RESIZABLE) && choice))
	{
		hResizable = choice;
		this->Show();
	}
}

void SDLwindow::SetTitle(char *title)
{
      hTitle = title;
      
	if (hSurface)
		SDL_WM_SetCaption(title, title);
}
/*
void SDLwindow::SetCursor(SDLcursor *cursor)
{
	if (hCursor)
		delete hCursor;

	SDLcursor *curs=new SDLcursor(*cursor);
	hCursor = curs;

	if (this->IsShown())
		hCursor->Show();
}
*/
bool SDLwindow::IsShown()
{
	if (!hSurface)
		return (false);

	if (SDLcore::GetWindow() == this)
		return (true);
	else
		return (false);
}

void SDLwindow::GrabInput(bool grab)
{
	SDL_WM_GrabInput(grab ? SDL_GRAB_ON : SDL_GRAB_OFF);
}

bool SDLwindow::IsInputGrabbed(void) const
{
	return SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON;
}
