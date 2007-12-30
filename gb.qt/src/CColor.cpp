/***************************************************************************

  CColor.cpp

  The Color constants and functions

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __CCOLOR_CPP

#include <qapplication.h>
#include <qframe.h>
#include <qcolor.h>
#include <qpalette.h>

#include "gambas.h"

#include "CWidget.h"
#include "CColor.h"

static long _color;

static int _h = 0;
static int _s = 0;
static int _v = 0;

static void get_hsv(long col)
{
  static long last = 0;

  if (last == col)
    return;

  QColor c(col);
  c.getHsv(&_h, &_s, &_v);
  if (_h < 0)
    _h = 0;
  last = col;
}



BEGIN_METHOD(CCOLOR_rgb, GB_INTEGER r; GB_INTEGER g; GB_INTEGER b; GB_INTEGER a)

  GB.ReturnInteger(qRgb(VARG(r), VARG(g), VARG(b)) & 0x00FFFFFF | ((VARGOPT(a, 0) & 0xFF) << 24));

END_METHOD

BEGIN_METHOD(CCOLOR_hsv, GB_INTEGER h; GB_INTEGER s; GB_INTEGER v)

  QColor col(VARG(h), VARG(s), VARG(v), QColor::Hsv);

  GB.ReturnInteger((uint)col.rgb() & 0xFFFFFF);

END_METHOD

static void return_color(QColorGroup::ColorRole role)
{
  GB.ReturnInteger(QApplication::palette().active().color(role).rgb() & 0xFFFFFF);
}

BEGIN_PROPERTY(CCOLOR_background)

  return_color(QColorGroup::Background);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_foreground)

  return_color(QColorGroup::Foreground);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_text_background)

  return_color(QColorGroup::Base);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_text_foreground)

  return_color(QColorGroup::Text);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_selected_background)

  return_color(QColorGroup::Highlight);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_light_background)

	get_hsv(QApplication::palette().active().color(QColorGroup::Highlight).rgb() & 0xFFFFFF);

  QColor col(_h, _s / 8, 255 - (255 - _v) / 8, QColor::Hsv);

  GB.ReturnInteger((uint)col.rgb() & 0xFFFFFF);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_selected_foreground)

  return_color(QColorGroup::HighlightedText);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_button_background)

  return_color(QColorGroup::Button);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_button_foreground)

  return_color(QColorGroup::ButtonText);

END_PROPERTY

BEGIN_METHOD(CCOLOR_get, GB_INTEGER color)

  _color = VARG(color);
  RETURN_SELF();

END_METHOD

BEGIN_PROPERTY(CCOLOR_info_alpha)

  GB.ReturnInteger((_color >> 24) & 0xFF);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_red)

  GB.ReturnInteger((_color >> 16) & 0xFF);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_green)

  GB.ReturnInteger((_color >> 8) & 0xFF);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_blue)

  GB.ReturnInteger(_color & 0xFF);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_hue)

  get_hsv(_color);
  GB.ReturnInteger(_h);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_saturation)

  get_hsv(_color);
  GB.ReturnInteger(_s);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_value)

  get_hsv(_color);
  GB.ReturnInteger(_v);

END_PROPERTY

BEGIN_METHOD(CCOLOR_lighter, GB_INTEGER color)

	int a = VARG(color) & 0xFF000000;
	get_hsv(VARG(color) & 0xFFFFFF);

  QColor col(_h, _s / 2, 255 - (255 - _v) / 2, QColor::Hsv);

  GB.ReturnInteger((uint)col.rgb() & 0xFFFFFF | a);

END_METHOD

BEGIN_METHOD(CCOLOR_darker, GB_INTEGER color)

	int a = VARG(color) & 0xFF000000;
	get_hsv(VARG(color) & 0xFFFFFF);

  QColor col(_h, 255 - (255 - _s) / 2, _v / 2, QColor::Hsv);

  GB.ReturnInteger((uint)col.rgb() & 0xFFFFFF | a);

END_METHOD

BEGIN_METHOD(CCOLOR_medium, GB_INTEGER color1; GB_INTEGER color2)

	int col1, col2;
	int r, g, b, a;
	
	col1 = VARG(color1);
	col2 = VARG(color2);
	
	a = (((col1 >> 24) & 0xFF) + ((col2 >> 24) & 0xFF)) >> 1;
	r = (((col1 >> 16) & 0xFF) + ((col2 >> 16) & 0xFF)) >> 1;
	g = (((col1 >> 8) & 0xFF) + ((col2 >> 8) & 0xFF)) >> 1;
	b = ((col1 & 0xFF) + (col2 & 0xFF)) >> 1;
	
  GB.ReturnInteger(qRgb(r, g, b) & 0x00FFFFFF | ((a & 0xFF) << 24));

END_METHOD


#include "CColor_desc.h"
