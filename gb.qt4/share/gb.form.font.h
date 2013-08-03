/***************************************************************************

  gb.form.font.h

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

#ifndef __GB_FORM_FONT_H
#define __GB_FORM_FONT_H

#define FONT_STEP 20
#define GRADE_TO_SIZE(_grade, _desktop) ((int)(powf(_desktop, 1.0 + ((_grade) / (double)FONT_STEP)) + 0.5))
#define SIZE_TO_GRADE(_size, _desktop)  ((int)(FONT_STEP * (logf(_size) / logf(_desktop)) + 0.5) - FONT_STEP)
#define GET_DESKTOP_SCALE(_font_size, _dpi) (1 + ((_font_size) * (_dpi) * 2 / 3 / 96))

#endif


