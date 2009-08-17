/***************************************************************************

  SDLwindow.h

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

#ifndef __SDLWINDOW_H
#define __SDLWINDOW_H

#include "SDL_h.h"
#include "SDLapp.h"
#include "SDLcursor.h"

class SDLwindow
{
	friend class SDLgfx;
public:
	SDLwindow(bool openGL = true);
	virtual ~SDLwindow();

	void Show(void );
	void Close(void );
	void Refresh();
	void Clear(Uint32 color = 0);
	Uint32 Id(void );

	int GetWidth(void );
	int GetHeight(void );
	int GetDepth(void );
	char* GetTitle(void ) { return (hTitle); };
	SDLcursor* GetCursor(void ) { return (hCursor); }

	void SetX(int );
	void SetY(int );
	void SetWidth(int );
	void SetHeight(int );
	void SetFullScreen(bool );
	void SetResizable(bool );
	void SetTitle(char* );
	void SetCursor(SDLcursor *cursor);

	bool IsFullScreen(void ) {return (hFullScreen); };
	bool IsOpenGL(void ) { return (hOpenGL); };
	bool IsResizable(void ) { return (hResizable); };
	bool IsShown(void );

	// events :)
	virtual void Quit(void ) = 0;
	virtual void Resize(void ) = 0;
	virtual void GotFocus(void ) = 0;
	virtual void LostFocus(void ) = 0;
	virtual void MouseEnter(void ) = 0;
	virtual void MouseLeave(void ) = 0;
	virtual void Update(void ) = 0;
	virtual void KeyEvent(SDL_KeyboardEvent* , int ) = 0;
	virtual void MouseButtonEvent(SDL_MouseButtonEvent* ) = 0;
	virtual void MouseMotionEvent(SDL_MouseMotionEvent* ) = 0;
	virtual void Open(void ) =0;

private:
	SDL_INFO *hSurfaceInfo;
	SDLcursor *hCursor;
	int hX, hY;
	int hWidth, hHeight;
	bool hOpenGL;
	bool hFullScreen;
	bool hResizable;
	char* hTitle;
};

#endif /* __SDLWINDOW_H */

