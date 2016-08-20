/***************************************************************************

  effect.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __EFFECT_H
#define __EFFECT_H

#include "main.h"

class Effect
{
public:

	enum {
		Red = 1,
		Green = 2,
		Blue = 4,
		All = 7
	};

	static void balance(GB_IMAGE img, int channels, int brightness, int contrast, int gamma);
	static void invert(GB_IMAGE img, int channels);
};

#endif
