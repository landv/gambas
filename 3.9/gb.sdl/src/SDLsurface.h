/***************************************************************************

  SDLsurface.h

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

#ifndef __SDLSURFACE_H
#define __SDLSURFACE_H

#include "SDL_h.h"

class SDLtexture;

class SDLsurface
{
public:
	SDLsurface();
	SDLsurface(const SDLsurface& surf);
	SDLsurface(SDL_Surface* surf); /* SDLsurface will free surf automaticly */
	SDLsurface(int Width, int Height);
	~SDLsurface();

	void Ref() { ref++; }
	void Unref() { ref--; if (ref <= 0) delete this; }

	void Create(int Width, int Height, int Depth = 0);
//	void LoadFromMem(char* addr, long len);

	int GetWidth(void );
	int GetHeight(void );
	int GetDepth(void );
	void* GetData(void );

	SDL_Surface* GetSdlSurface(void ) { return hSurface; }
	SDLtexture* GetTexture(void ) { return hTexture; };

	void SetAlphaBuffer(bool );
	void ConvertDepth(int );
	void Fill(Uint32 color = 0);
	void Resize(int width, int height);

	bool IsNull(void ) { return (hSurface ? true: false); };

	// Compatibility //
	SDLsurface(char *data, int width, int height);
	int width(void) { return GetWidth(); }
	int height(void) { return GetHeight(); }
	unsigned char *data(void) { return (unsigned char *)GetData(); }

private:
	int ref;
	SDLtexture *hTexture;
	SDL_Surface *hSurface;
};

#endif /* __SDLSURFACE_H */

