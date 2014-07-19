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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef __SDLWINDOW_H
#define __SDLWINDOW_H

#include "SDL_h.h"
#include "SDLcursor.h"

#include <GL/glx.h>

class SDLwindow
{
public:
	SDLwindow(void );
	virtual ~SDLwindow();

	void Show(void );
	void Close(void );
	void Refresh();
	void Select(void );
	void Clear(Uint32 color = 0);
	Uint32 Id(void );

	int GetWidth(void );
	int GetHeight(void );
	int GetDepth(void );
	void* GetData(void );

	void Lock(void) { if (hSurface) SDL_LockSurface(hSurface); }
	void Unlock(void) { if (hSurface) SDL_UnlockSurface(hSurface); }

	const char* GetTitle(void ) { return (hTitle.c_str()); };
	SDL_Surface* GetSdlSurface(void ) { return (hSurface); };
	SDLcursor* GetCursor(void ) { return (hCursor); }
	int GetCursorShape(void ) { return (hCursor->GetShape()); }

	void SetX(int );
	void SetY(int );
	void SetWidth(int );
	void SetHeight(int );
	void SetFullScreen(bool );
	void SetResizable(bool );
	void SetTitle(char* );

	void SetCursor(SDLcursor* );
	void SetCursorShape(int );

	bool IsFullScreen(void ) {return (hFullScreen); };
	bool IsResizable(void ) { return (hResizable); };
	bool IsShown(void );

	void GrabInput(bool grab);
	bool IsInputGrabbed(void) const;

	// events :)
	virtual void Quit(void ) = 0;
	virtual void Resize(void ) = 0;
	virtual void GotFocus(void ) = 0;
	virtual void LostFocus(void ) = 0;
	virtual void MouseEnter(void ) = 0;
	virtual void MouseLeave(void ) = 0;
	virtual void Update(void ) = 0;
	virtual void JoyEvent(SDL_Event& ) = 0;
	virtual void KeyEvent(SDL_KeyboardEvent* , int ) = 0;
	virtual void MouseButtonEvent(SDL_MouseButtonEvent* ) = 0;
	virtual void MouseMotionEvent(SDL_MouseMotionEvent* ) = 0;
	virtual void Open(void ) =0;

private:
	SDL_Surface *hSurface;
	SDLcursor *hCursor;
	int hX, hY;
	int hWidth, hHeight;
	bool hFullScreen;
	bool hResizable;
	std::string hTitle;
	// context and drawable (for GlxMakeCurrent)
	GLXContext hCtx;
	GLXDrawable hDrw;
	Display *hDpy;
};

#endif /* __SDLWINDOW_H */

