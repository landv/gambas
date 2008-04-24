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
#include <q3frame.h>
#include <qcolor.h>
#include <qpalette.h>

#include "gambas.h"

#include "CWidget.h"
#include "CColor.h"

static int _color;

static int _h = 0;
static int _s = 0;
static int _v = 0;

static void get_hsv(int col)
{
  static int last = 0;

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

BEGIN_METHOD(CCOLOR_hsv, GB_INTEGER h; GB_INTEGER s; GB_INTEGER v; GB_INTEGER a)

  QColor col(VARG(h), VARG(s), VARG(v), QColor::Hsv);

  GB.ReturnInteger((uint)col.rgb() & 0xFFFFFF | ((VARGOPT(a, 0) & 0xFF) << 24));

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

	QColor col;
	int h, s, v;

	get_hsv(QApplication::palette().active().color(QColorGroup::Highlight).rgb() & 0xFFFFFF);
	h = _h; s = _s; v = _v;
	
	get_hsv(QApplication::palette().active().color(QColorGroup::Base).rgb() & 0xFFFFFF);
	
	col = QColor(h, (_s * 3 + s) / 4, (_v * 3 + v) / 4, QColor::Hsv);
	
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

BEGIN_METHOD(CCOLOR_mix, GB_INTEGER color1; GB_INTEGER color2; GB_FLOAT weight)

	int col1, col2;
	int r, g, b, a;
	double weight = VARGOPT(weight, 0.5);
	
	col1 = VARG(color1);
	col2 = VARG(color2);
	
	if (weight == 0.0)
		GB.ReturnInteger(col1);
	else if (weight == 1.0)
		GB.ReturnInteger(col2);
	else
	{
		#define MIX_COLOR(_shift) (int)((((col2 >> _shift) & 0xFF) * weight + ((col1 >> _shift) & 0xFF) * (1 - weight)) + 0.5)
		
		a = MIX_COLOR(24);
		r = MIX_COLOR(16);
		g = MIX_COLOR(8);
		b = MIX_COLOR(0);
		
		GB.ReturnInteger(qRgb(r, g, b) & 0x00FFFFFF | ((a & 0xFF) << 24));
	}

END_METHOD

BEGIN_METHOD(CCOLOR_blend, GB_INTEGER src; GB_INTEGER dst)

	uint src = VARG(src);
	uint dst = VARG(dst);
	uchar rs, gs, bs;
	uchar rd, gd, bd;
	uchar as = src >> 24;
	uchar ad = dst >> 24;

	if (as == 0xFF)
	{
		GB.ReturnInteger(dst);
		return;
	}
	else if (as == 0)
	{
		GB.ReturnInteger(src);
		return;
	}
	
	ad ^= 0xFF;
	as ^= 0xFF;
	
	bs = src & 0xFF;
	gs = (src >> 8) & 0xFF;
	rs = (src >> 16) & 0xFF;
	
	bd = dst & 0xFF;
	gd = (dst >> 8) & 0xFF;
	rd = (dst >> 16) & 0xFF;
	
	// D = S * alpha(S) + D * (1 - alpha(S))
	
	bd = (((bs - bd) * as) >> 8) + bd;
	rd = (((rs - rd) * as) >> 8) + rd;
	gd = (((gs - gd) * as) >> 8) + gd;
	if (ad < as) ad = as;
	ad ^= 0xFF;
	
	GB.ReturnInteger(bd | (gd << 8) | (rd << 16) | (ad << 24));

END_METHOD


GB_DESC CColorInfoDesc[] =
{
  GB_DECLARE(".ColorInfo", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY("Alpha", "i", CCOLOR_info_alpha),
  GB_STATIC_PROPERTY("Red", "i", CCOLOR_info_red),
  GB_STATIC_PROPERTY("Green", "i", CCOLOR_info_green),
  GB_STATIC_PROPERTY("Blue", "i", CCOLOR_info_blue),
  GB_STATIC_PROPERTY("Hue", "i", CCOLOR_info_hue),
  GB_STATIC_PROPERTY("Saturation", "i", CCOLOR_info_saturation),
  GB_STATIC_PROPERTY("Value", "i", CCOLOR_info_value),

  GB_END_DECLARE
};


GB_DESC CColorDesc[] =
{
  GB_DECLARE("Color", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("Default", "i", COLOR_DEFAULT),

  GB_CONSTANT("Black", "i", 0),

  GB_CONSTANT("White", "i", 0xFFFFFF),
  GB_CONSTANT("LightGray", "i", 0xC0C0C0),
  GB_CONSTANT("Gray", "i", 0x808080),
  GB_CONSTANT("DarkGray", "i", 0x404040),

  GB_CONSTANT("Blue", "i", 0xFF),
  GB_CONSTANT("DarkBlue", "i", 0x80),

  GB_CONSTANT("Green", "i", 0xFF00L),
  GB_CONSTANT("DarkGreen", "i", 0x8000),

  GB_CONSTANT("Red", "i", 0xFF0000),
  GB_CONSTANT("DarkRed", "i", 0x800000),

  GB_CONSTANT("Cyan", "i", 0xFFFFL),
  GB_CONSTANT("DarkCyan", "i", 0x8080L),

  GB_CONSTANT("Magenta", "i", 0xFF00FF),
  GB_CONSTANT("DarkMagenta", "i", 0x800080),

  GB_CONSTANT("Yellow", "i", 0xFFFF00),
  GB_CONSTANT("DarkYellow", "i", 0x808000),

  GB_CONSTANT("Orange", "i", 0xFF8000),
  GB_CONSTANT("Violet", "i", 0x8000FF),
  GB_CONSTANT("Pink", "i", 0xFF80FF),

  GB_CONSTANT("Transparent", "i", -1),

  GB_STATIC_PROPERTY("Background", "i", CCOLOR_background),
  GB_STATIC_PROPERTY("SelectedBackground", "i", CCOLOR_selected_background),
  GB_STATIC_PROPERTY("LightBackground", "i", CCOLOR_light_background),
  GB_STATIC_PROPERTY("TextBackground", "i", CCOLOR_text_background),
  GB_STATIC_PROPERTY("ButtonBackground", "i", CCOLOR_button_background),

  GB_STATIC_PROPERTY("Foreground", "i", CCOLOR_foreground),
  GB_STATIC_PROPERTY("SelectedForeground", "i", CCOLOR_selected_foreground),
  GB_STATIC_PROPERTY("TextForeground", "i", CCOLOR_text_foreground),
  GB_STATIC_PROPERTY("ButtonForeground", "i", CCOLOR_button_foreground),

  GB_STATIC_METHOD("RGB", "i", CCOLOR_rgb, "(Red)i(Green)i(Blue)i[(Alpha)i]"),
  GB_STATIC_METHOD("HSV", "i", CCOLOR_hsv, "(Hue)i(Saturation)i(Value)i[(Alpha)i]"),
  
  GB_STATIC_METHOD("Lighter", "i", CCOLOR_lighter, "(Color)i"),
  GB_STATIC_METHOD("Darker", "i", CCOLOR_darker, "(Color)i"),
  GB_STATIC_METHOD("Mix", "i", CCOLOR_mix, "(Color1)i(Color2)i[(Weight)f]"),
  GB_STATIC_METHOD("Blend", "i", CCOLOR_blend, "(Source)i(Destination)i"),

  GB_STATIC_METHOD("_get", ".ColorInfo", CCOLOR_get, "(Color)i"),

  GB_END_DECLARE
};


