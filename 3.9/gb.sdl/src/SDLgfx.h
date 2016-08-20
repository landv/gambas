/***************************************************************************

  SDLgfx.h

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

#ifndef __SDLGFX_H
#define __SDLGFX_H

#include "SDL_h.h"

class SDLwindow;
class SDLsurface;
class SDLtexture;

class SDLgfx 
{
public:
	SDLgfx(SDLwindow* window);
	SDLgfx(SDLsurface* surface);
	~SDLgfx() {};

	void resetGfx(void );
	void SetColor(Uint32 color);
	void Scale(GLfloat x, GLfloat y) { scalex = x; scaley = y; }
	void Rotate(GLfloat z) { rotz = z; }

	int GetLineStyle(void ) { return hLine; }
	int GetLineWidth(void ) { return hLineWidth; }
	int GetFillStyle(void ) { return hFill; }
	void SetLineStyle(int style);
	void SetLineWidth(int width) { hLineWidth = width; }
	void SetFillStyle(int style);

	void Clear(void);
	void Blit(SDLsurface* s, int x, int y, int srcX = 0, int srcY = 0, int srcWidth = -1, int srcHeight = -1, int width = -1, int height = -1, bool no_filter = false);
	void DrawPixel(int x, int y);
	void DrawLine(int x1, int y1, int x2, int y2);
	void DrawRect(int x, int y, int w, int h);
	void DrawEllipse(int x, int y, int w, int h);

private:
	// hTex is null if we are drawing on a window !
	SDLtexture* hTex;

	void SetContext(void );
	SDL_Surface* GetDestSurface(void );

	// lines and fills
	int hLine, hLineWidth;
	int hFill;
	
	// rotations & scaling
	GLfloat rotx, roty, rotz;
	GLfloat scalex, scaley;
};

#endif /* __SDLGFX_H */

