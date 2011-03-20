/***************************************************************************

  SDLtexture.h

  Gambas extension using SDL

  (c) 2008 Laurent Carlier <lordheavy@users.sourceforge.net>
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

/* manage transparently window/surfaces as texture */
 
#ifndef __SDLTEXTURE_H
#define __SDLTEXTURE_H

#include "SDL_h.h"
#include "SDLosrender.h"

// internal texture status values
#define TEXTURE_OK		(0)
#define TEXTURE_TO_UPLOAD	(1)
#define TEXTURE_TO_DOWNLOAD	(2)

class SDLwindow;
class SDLsurface;

typedef struct {
	GLuint Index;
	GLdouble Width, Height;
	GLint RealWidth, RealHeight;
	Uint8 State;
	}
	texinfo;

class SDLtexture
{
public:
	SDLtexture(SDLsurface* surface);
	~SDLtexture();

	static void init(void );
	void Select(void );
	static void Unselect(void );
	SDLsurface* GetSurface(void ) { return hSurface; };
	void Sync(void );

	void ToLoad(void ) { hTexinfo->State = TEXTURE_TO_UPLOAD; };

	Uint8 GetStatus(void ) { return hTexinfo->State; };
	void GetAsTexture(texinfo *);

	static int GetPowerOfTwo(int size);
private:
	SDLsurface *hSurface;
	texinfo *hTexinfo;
	FBOrender *hRenderBuffer;
};

#endif /* __SDLTEXTURE_H */

