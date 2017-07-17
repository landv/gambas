/***************************************************************************

  Cwindow.h

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

#ifndef __CWINDOW_H
#define __CWINDOW_H

#include "gambas.h"
#include "main.h"
#include "SDLwindow.h"
#include "Cfont.h"
#include "Cmouse.h"

class myWin : public SDLwindow
{
public:
	myWin(void *win):SDLwindow() { hWindow = win; };
	~myWin() {};

	void Quit(void );
	void Resize(void );
	void GotFocus(void );
	void LostFocus(void );
	void MouseEnter(void );
	void MouseLeave(void );
	void Update(void );
	void Open(void );
	void JoyEvent(SDL_Event& );
	void KeyEvent(SDL_KeyboardEvent* , int);
	void MouseButtonEvent(SDL_MouseButtonEvent* );
	void MouseMotionEvent(SDL_MouseMotionEvent* );
private:
	void *hWindow;
};

typedef
	struct {
		GB_BASE ob;
		CCURSOR *cursor;

		myWin *id;
		bool openGL;
		bool tracking;
		// framerate control
		double FPSLimit; // duration of a frame in milliseconds, if 0 -> no framerate limit
		double lastTime;
		// framerate count
		Uint32 startTime;
		Uint32 countFrames;
		double currentFPS;
	}
	CWINDOW;

#ifndef __CWINDOW_CPP
extern GB_DESC CWindow[];
#endif /* __CWINDOW_CPP */

#endif /* __CWINDOW_H */

