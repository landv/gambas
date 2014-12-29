/***************************************************************************

  default_font.h

  (c) 2014 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __DEFAULT_FONT_H
#define __DEFAULT_FONT_H

#include "main.h"

#define DEFAULT_FONT_WIDTH 7
#define DEFAULT_FONT_HEIGHT 13
#define DEFAULT_FONT_ASCENT 10
#define DEFAULT_FONT_DESCENT 3

int UTF8_get_length(const char *sstr, int len);
void FONT_render_default(uint *dest, int size, const char *text, int len);

#endif /* __DEFAULT_FONT_H */

