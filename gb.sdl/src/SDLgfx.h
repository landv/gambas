/***************************************************************************

  SDLgfx.h

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

#ifndef __SDLGFX_H
#define __SDLGFX_H

#include "SDL_h.h"
#include "SDLwindow.h"
#include "SDLsurface.h"

class SDLgfx 
{
public:
	SDLgfx(SDLwindow *window);
	~SDLgfx();

	void resetGfx(void );
	Uint32 GetBackColor(void ) { return hBackColor; }
	Uint32 GetForeColor(void ) { return hForeColor; }
	void SetBackColor(Uint32 color) { hBackColor = color; }
	void SetForeColor(Uint32 color) { hForeColor = color; }

	int GetLineStyle(void ) { return hLine; }
	int GetLineWidth(void ) { return hLineWidth; }
	int GetFillStyle(void ) { return hFill; }
	void SetLineStyle(int style);
	void SetLineWidth(int width) { hLineWidth = width; }
	void SetFillStyle(int style);

	void Clear(void);
	void Blit(SDLsurface* s, int x, int y, int srcX = 0, int srcY = 0, int srcWidth = -1, int srcHeight = -1);
	void DrawPixel(int x, int y);
	void DrawLine(int x1, int y1, int x2, int y2);
	void DrawRect(int x, int y, int w, int h);
	void DrawEllipse(int x, int y, int w, int h);

private:
	SDL_INFO *hSurfaceInfo;
	// texture Management
	void ManageTexture(void);
	// colors (RRGGBBAA format)
	Uint32 hBackColor;
	Uint32 hForeColor;
	// lines and fills
	int hLine, hLineWidth;
	int hFill;
};

#endif /* __SDLGFX_H */

