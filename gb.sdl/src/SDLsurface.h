/***************************************************************************

  SDLsurface.h

  Gambas extension using SDL

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>
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

#ifndef __SDLSURFACE_H
#define __SDLSURFACE_H

#include "SDL_h.h"

class SDLsurface
{
	friend class SDLgfx;
public:
	SDLsurface();
	SDLsurface(const SDLsurface& surf);
	SDLsurface(char *data, int width, int height);
	~SDLsurface();

	void Create(int Width, int Height, int Depth = 0);
	void LoadFromMem(char* addr, long len);

	int GetWidth(void );
	int width(void) { return GetWidth(); }
	int GetHeight(void );
	int height(void) { return GetHeight(); }
	int GetDepth(void );
	void* GetData(void );
	unsigned char *data(void) { return (unsigned char *)GetData(); }

	void SetAlphaBuffer(bool );
	void ConvertDepth(int );
	void Fill(Uint32 color = 0);
	void Resize(int width, int height);

	bool IsNull(void ) { return (hSurfaceInfo->Surface != NULL); };
private:
	SDL_INFO *hSurfaceInfo;
};

#endif /* __SDLSURFACE_H */

