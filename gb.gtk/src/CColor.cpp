/***************************************************************************

  CColor.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

  GTK+ component

  Realizado para la Junta de Extremadura.
  Consejería de Educación Ciencia y Tecnología.
  Proyecto gnuLinEx

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

#include <math.h>

#include "CColor.h"
#include "gdesktop.h"
#include "gcolor.h"

BEGIN_METHOD(CCOLOR_rgb, GB_INTEGER r; GB_INTEGER g; GB_INTEGER b; GB_INTEGER a)

  GB.ReturnInteger(gt_rgba_to_color(VARG(r), VARG(g), VARG(b), VARGOPT(a, 0)));

END_METHOD

BEGIN_METHOD(CCOLOR_hsv, GB_INTEGER h; GB_INTEGER s; GB_INTEGER v; GB_INTEGER a)

	int r, g, b;
	gt_hsv_to_rgb(VARG(h), VARG(s), VARG(v), &r, &g, &b);
  GB.ReturnInteger(gt_rgba_to_color(r, g, b, VARGOPT(a, 0)));

END_METHOD

BEGIN_PROPERTY(CCOLOR_background)

	GB.ReturnInteger(gDesktop::bgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_foreground)

	GB.ReturnInteger(gDesktop::fgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_text_background)

	GB.ReturnInteger(gDesktop::textbgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_text_foreground)

	GB.ReturnInteger(gDesktop::textfgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_selected_background)

	GB.ReturnInteger(gDesktop::selbgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_light_background)

  int h, s, v;
  int h2, s2, v2;
  int r, g, b;
  
	gt_color_to_rgb(gDesktop::selbgColor(), &r, &g, &b);
  gt_rgb_to_hsv(r, g, b, &h, &s, &v);
  
	gt_color_to_rgb(gDesktop::textbgColor(), &r, &g, &b);
  gt_rgb_to_hsv(r, g, b, &h2, &s2, &v2);
  
  gt_hsv_to_rgb(h, (s2 * 3 + s) / 4, (v2 * 3 + v) / 4, &r, &g, &b);

  GB.ReturnInteger(gt_rgb_to_color(r, g, b));

END_PROPERTY


BEGIN_PROPERTY(CCOLOR_selected_foreground)

	GB.ReturnInteger(gDesktop::selfgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_button_background)

	GB.ReturnInteger(gDesktop::buttonbgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_button_foreground)

	GB.ReturnInteger(gDesktop::buttonfgColor());

END_PROPERTY

static int _r, _g, _b, _a;

BEGIN_METHOD(CCOLOR_get, GB_INTEGER color)

	gt_color_to_rgba(VARG(color), &_r, &_g, &_b, &_a);
  RETURN_SELF();

END_METHOD

BEGIN_PROPERTY(CCOLOR_info_alpha)

  GB.ReturnInteger(_a);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_red)

  GB.ReturnInteger(_r);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_green)

  GB.ReturnInteger(_g);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_blue)

  GB.ReturnInteger(_b);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_hue)

  int h, s, v;
  gt_rgb_to_hsv(_r, _g, _b, &h, &s, &v);
  GB.ReturnInteger(h);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_saturation)

  int h, s, v;
  gt_rgb_to_hsv(_r, _g, _b, &h, &s, &v);
  GB.ReturnInteger(s);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_value)

  int h, s, v;
  gt_rgb_to_hsv(_r, _g, _b, &h, &s, &v);
  GB.ReturnInteger(v);
END_PROPERTY

BEGIN_METHOD(CCOLOR_lighter, GB_INTEGER color)

  int h, s, v;
  int r, g, b, a;
  
	gt_color_to_rgba(VARG(color), &r, &g, &b, &a);
  gt_rgb_to_hsv(r, g, b, &h, &s, &v);
  gt_hsv_to_rgb(h, s / 2, 255 - (255 - v) / 2, &r, &g, &b);

  GB.ReturnInteger(gt_rgba_to_color(r, g, b, a));
  
END_METHOD

BEGIN_METHOD(CCOLOR_darker, GB_INTEGER color)

  int h, s, v;
  int r, g, b, a;
  
	gt_color_to_rgba(VARG(color), &r, &g, &b, &a);
  gt_rgb_to_hsv(r, g, b, &h, &s, &v);
  gt_hsv_to_rgb(h, 255 - (255 - s) / 2, v / 2, &r, &g, &b);

  GB.ReturnInteger(gt_rgba_to_color(r, g, b, a));

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
	
  GB.ReturnInteger(gt_rgba_to_color(r, g, b, a));

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
  GB_CONSTANT("DarkCyan", "i", 0x8080),

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
  GB_STATIC_METHOD("Medium", "i", CCOLOR_medium, "(Color1)i(Color2)i"),
  GB_STATIC_METHOD("Blend", "i", CCOLOR_blend, "(Source)i(Destination)i"),

  GB_STATIC_METHOD("_get", ".ColorInfo", CCOLOR_get, "(Color)i"),

  GB_END_DECLARE
};


