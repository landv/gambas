/***************************************************************************

  c_color.c

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

#define __C_COLOR_C

#include <math.h>
#include "c_color.h"

void gt_color_to_rgba(uint color, int *r, int *g, int *b, int *a)
{
	*b = color & 0xFF;
	*g = (color >> 8) & 0xFF;
	*r = (color >> 16) & 0xFF;
	*a = (color >> 24) & 0xFF;
}

static uint gt_rgba_to_color(int r, int g, int b, int a)
{
	return (uint)((uchar)b | ((uchar)g << 8) | ((uchar)r << 16) | ((uchar)a << 24));
}

void COLOR_rgb_to_hsv(int r, int g, int b, int *H, int *S, int *V)
{
	int v, x, f;
	int i;

	x = r;
	if (g < x) x = g;
	if (b < x) x = b;

	v = r;
	if (g > v) v = g;
	if (b > v) v = b;

	if (v == x) 
	{
		*H = -1;
		*S = 0;
		*V = v;
	}
	else
	{
		f = (r == x) ? g - b : ((g == x) ? b - r : r - g);
		i = (r == x) ? 3 : ((g == x) ? 5 : 1);
		*H = (int)((i - (double)f / (v - x)) * 60);
		*S = ((v - x) * 255) / v;
		*V = v;
		if (*H == 360) *H = 0;
	}	
}

static void gt_rgb_to_hsv_cached(int r, int g, int b, int *h, int *s, int *v)
{
	static int old_r = 0, old_g = 0, old_b = 0, old_h = -1, old_s = 0, old_v = 0;

	if (r == old_r && g == old_g && b == old_b)
	{
		*h = old_h;
		*s = old_s;
		*v = old_v;
		return;
	}

	COLOR_rgb_to_hsv(r, g, b, h, s, v);

	old_r = r;
	old_g = g;
	old_b = b;
	old_h = *h;
	old_s = *s;
	old_v = *v;
}

void COLOR_hsv_to_rgb(int h, int s, int v, int *R, int *G, int *B)
{
	double var_h;
	int var_i;
	int var_1, var_2, var_3;
	int tmp_r, tmp_g, tmp_b;

	if (h < 0)
		h = 360 - ((-h) % 360);
	else
		h = h % 360;

	 /*H = ((double)h) / 360;
	 S = ((double)s) / 255;
	 V = ((double)v) / 255;*/

	if (s == 0)
	{
		*R = v;
		*G = v;
		*B = v;
	}
	else
	{
		var_i = h / 60;
		var_h = h % 60; //((double)h / 60) - var_i;

		//var_1 = V * ( 1 - S );
		var_1 = v * (255 - s) / 255;

		//var_2 = V * ( 1 - S * ( var_h - var_i ) );
		var_2 = v * (255 - s * var_h / 60) / 255;

		//var_3 = V * ( 1 - S * ( 1 - ( var_h - var_i ) ) );
		var_3 = v * (255 - s * (60 - var_h) / 60) / 255;

		switch (var_i)
		{
			case 0:
				tmp_r = v;
				tmp_g = var_3;
				tmp_b = var_1;
				break;

			case 1:
				tmp_r = var_2;
				tmp_g = v;
				tmp_b = var_1;
				break;

			case 2:
				tmp_r = var_1;
				tmp_g = v;
				tmp_b = var_3;
				break;

			case 3:
				tmp_r = var_1;
				tmp_g = var_2;
				tmp_b = v;
				break;

			case 4:
				tmp_r = var_3;
				tmp_g = var_1;
				tmp_b = v;
				break;

			default:
				tmp_r = v;
				tmp_g = var_1;
				tmp_b = var_2;
				break;
		}

		*R = tmp_r;
		*G = tmp_g;
		*B = tmp_b;

	}
}

GB_COLOR COLOR_merge(GB_COLOR col1, GB_COLOR col2, double weight)
{
	int r, g, b;
	int h1, s1, v1, a1;
	int h2, s2, v2, a2;
	
	if (weight == 0.0)
		return col1;
	else if (weight == 1.0)
		return col2;
	else
	{
		gt_color_to_rgba(col1, &r, &g, &b, &a1);
		COLOR_rgb_to_hsv(r, g, b, &h1, &s1, &v1);
		gt_color_to_rgba(col2, &r, &g, &b, &a2);
		COLOR_rgb_to_hsv(r, g, b, &h2, &s2, &v2);
		
		#define MIX(_val1, _val2) ((int)((_val1) * (1 - weight) + (_val2) * weight + 0.5))
		
		if (h1 < 0)
			h1 = h2;
		else if (h2 < 0)
			h2 = h1;
		else
			h1 = MIX(h1, h2);
		
		COLOR_hsv_to_rgb(h1, MIX(s1, s2), MIX(v1, v2), &r, &g, &b);
		
		return gt_rgba_to_color(r, g, b, MIX(a1, a2));
	}
}

GB_COLOR COLOR_gradient(GB_COLOR col1, GB_COLOR col2, double weight)
{
	int r1, g1, b1, a1;
	int r2, g2, b2, a2;
	
	if (weight == 0.0)
		return col1;
	else if (weight == 1.0)
		return col2;
	else
	{
		gt_color_to_rgba(col1, &r1, &g1, &b1, &a1);
		gt_color_to_rgba(col2, &r2, &g2, &b2, &a2);
		
		#define MIX(_val1, _val2) ((int)((_val1) * (1 - weight) + (_val2) * weight + 0.5))
		
		return gt_rgba_to_color(MIX(r1, r2), MIX(g1, g2), MIX(b1, b2), MIX(a1, a2));
	}
}

GB_COLOR COLOR_lighter(GB_COLOR color)
{
  int h, s, v;
  int r, g, b, a;
  
	gt_color_to_rgba(color, &r, &g, &b, &a);
  COLOR_rgb_to_hsv(r, g, b, &h, &s, &v);
  COLOR_hsv_to_rgb(h, s / 2, 255 - (255 - v) / 2, &r, &g, &b);

  return gt_rgba_to_color(r, g, b, a);  
}

GB_COLOR COLOR_darker(GB_COLOR color)
{
  int h, s, v;
  int r, g, b, a;
  
	gt_color_to_rgba(color, &r, &g, &b, &a);
  COLOR_rgb_to_hsv(r, g, b, &h, &s, &v);
  COLOR_hsv_to_rgb(h, s ? 255 - (255 - s) / 2 : 0, v / 2, &r, &g, &b);

  v = gt_rgba_to_color(r, g, b, a);
	
	return v;
}

BEGIN_METHOD(Color_RGB, GB_INTEGER r; GB_INTEGER g; GB_INTEGER b; GB_INTEGER a)

  GB.ReturnInteger(gt_rgba_to_color(VARG(r), VARG(g), VARG(b), VARGOPT(a, 0)));

END_METHOD

BEGIN_METHOD(Color_SetRGB, GB_INTEGER color; GB_INTEGER red; GB_INTEGER green; GB_INTEGER blue; GB_INTEGER alpha)

	int r, g, b, a;
	gt_color_to_rgba(VARG(color), &r, &g, &b, &a);
  GB.ReturnInteger(gt_rgba_to_color(VARGOPT(red, r), VARGOPT(green, g), VARGOPT(blue, b), VARGOPT(alpha, a)));

END_METHOD

BEGIN_METHOD(Color_HSV, GB_INTEGER h; GB_INTEGER s; GB_INTEGER v; GB_INTEGER a)

	int r, g, b;
	COLOR_hsv_to_rgb(VARG(h), VARG(s), VARG(v), &r, &g, &b);
  GB.ReturnInteger(gt_rgba_to_color(r, g, b, VARGOPT(a, 0)));

END_METHOD

BEGIN_METHOD(Color_SetHSV, GB_INTEGER color; GB_INTEGER hue; GB_INTEGER saturation; GB_INTEGER value; GB_INTEGER alpha)

	int r, g, b, a, h, s, v;
	gt_color_to_rgba(VARG(color), &r, &g, &b, &a);
	gt_rgb_to_hsv_cached(r, g, b, &h, &s, &v);
	COLOR_hsv_to_rgb(VARGOPT(hue, h), VARGOPT(saturation, s), VARGOPT(value, v), &r, &g, &b);
  GB.ReturnInteger(gt_rgba_to_color(r, g, b, VARGOPT(alpha, a)));

END_METHOD

BEGIN_METHOD(Color_get, GB_INTEGER color)

	CCOLOR *info;

	info = GB.New(GB.FindClass("ColorInfo"), NULL, NULL);
	gt_color_to_rgba(VARG(color), &info->r, &info->g, &info->b, &info->a);
  GB.ReturnObject(info);

END_METHOD

static void handle_rgba_property(CCOLOR *_object, void *_param, int prop)
{
	if (READ_PROPERTY)
	{
		switch(prop)
		{
			case CC_R: GB.ReturnInteger(THIS->r); break;
			case CC_G: GB.ReturnInteger(THIS->g); break;
			case CC_B: GB.ReturnInteger(THIS->b); break;
			case CC_A: GB.ReturnInteger(THIS->a); break;
		}
	}
	else
	{
		int v = VPROP(GB_INTEGER);
		if (v < 0) v = 0; else if (v > 255) v = 255;
		switch(prop)
		{
			case CC_R: THIS->r = v; break;
			case CC_G: THIS->g = v; break;
			case CC_B: THIS->b = v; break;
			case CC_A: THIS->a = v; break;
		}
	}
}

BEGIN_PROPERTY(CCOLOR_info_alpha)

	handle_rgba_property(_object, _param, CC_A);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_red)

	handle_rgba_property(_object, _param, CC_R);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_green)

	handle_rgba_property(_object, _param, CC_G);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_blue)

	handle_rgba_property(_object, _param, CC_B);

END_PROPERTY

static void handle_hsv_property(CCOLOR *_object, void *_param, int prop)
{
  int h, s, v;
  gt_rgb_to_hsv_cached(THIS->r, THIS->g, THIS->b, &h, &s, &v);
	
	if (READ_PROPERTY)
	{
		switch(prop)
		{
			case CC_H: GB.ReturnInteger(h); break;
			case CC_V: GB.ReturnInteger(v); break;
			case CC_S: GB.ReturnInteger(s); break;
		}
	}
	else
	{
		switch(prop)
		{
			case CC_H: h = VPROP(GB_INTEGER) % 360; break;
			case CC_V: v = VPROP(GB_INTEGER); if (v < 0) v = 0; else if (v > 255) v = 255; break;
			case CC_S: s = VPROP(GB_INTEGER); if (s < 0) s = 0; else if (s > 255) s = 255; break;
		}
		COLOR_hsv_to_rgb(h, s, v, &THIS->r, &THIS->g, &THIS->b);
	}
}

BEGIN_PROPERTY(CCOLOR_info_hue)

	handle_hsv_property(THIS, _param, CC_H);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_saturation)

	handle_hsv_property(THIS, _param, CC_S);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_value)

	handle_hsv_property(THIS, _param, CC_V);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_color)

	CCOLOR *info = THIS;

	if (READ_PROPERTY)
		GB.ReturnInteger(gt_rgba_to_color(info->r, info->g, info->b, info->a));
	else
		gt_color_to_rgba(VPROP(GB_INTEGER), &info->r, &info->g, &info->b, &info->a);

END_PROPERTY

BEGIN_METHOD(Color_Lighter, GB_INTEGER color)

  GB.ReturnInteger(COLOR_lighter(VARG(color)));
  
END_METHOD

BEGIN_METHOD(Color_Darker, GB_INTEGER color)

  GB.ReturnInteger(COLOR_darker(VARG(color)));

END_METHOD

BEGIN_METHOD(Color_Merge, GB_INTEGER color1; GB_INTEGER color2; GB_FLOAT weight)

	GB.ReturnInteger(COLOR_merge(VARG(color1), VARG(color2), VARGOPT(weight, 0.5)));

END_METHOD

BEGIN_METHOD(Color_Gradient, GB_INTEGER color1; GB_INTEGER color2; GB_FLOAT weight)

	GB.ReturnInteger(COLOR_gradient(VARG(color1), VARG(color2), VARGOPT(weight, 0.5)));

END_METHOD

BEGIN_METHOD(Color_Desaturate, GB_INTEGER color)

	int r, g, b, a, gray;
	
	gt_color_to_rgba(VARG(color), &r, &g, &b, &a);
	gray = (r * 11 + g * 16 + b * 5) / 32;
	GB.ReturnInteger(gt_rgba_to_color(gray, gray, gray, a));

END_METHOD

BEGIN_METHOD(Color_Blend, GB_INTEGER src; GB_INTEGER dst)

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

BEGIN_METHOD(Color_GetAlpha, GB_INTEGER color)

	int r, g, b, a;
	
	gt_color_to_rgba(VARG(color), &r, &g, &b, &a);
	GB.ReturnInteger(a);

END_METHOD

BEGIN_METHOD(Color_SetAlpha, GB_INTEGER color; GB_INTEGER alpha)

	int r, g, b, a;
	
	gt_color_to_rgba(VARG(color), &r, &g, &b, &a);
	a = VARG(alpha);
	GB.ReturnInteger(gt_rgba_to_color(r, g, b, a));

END_METHOD

BEGIN_METHOD(Color_Distance, GB_INTEGER col1; GB_INTEGER col2)

	int r1, g1, b1, a1;
	int r2, g2, b2, a2;

	gt_color_to_rgba(VARG(col1), &r1, &g1, &b1, &a1);
	gt_color_to_rgba(VARG(col2), &r2, &g2, &b2, &a2);

	r1 -= r2;
	g1 -= g2;
	b1 -= b2;
	a1 -= a2;
	r1 *= r1;
	g1 *= g1;
	b1 *= b1;
	a1 *= a1;

	GB.ReturnFloat(sqrt(r1 + b1 + g1 + a1) / 510.0);

END_METHOD

GB_DESC CColorInfoDesc[] =
{
  GB_DECLARE("ColorInfo", sizeof(CCOLOR)), GB_NOT_CREATABLE(),

  GB_PROPERTY("Alpha", "i", CCOLOR_info_alpha),
  GB_PROPERTY("Red", "i", CCOLOR_info_red),
  GB_PROPERTY("Green", "i", CCOLOR_info_green),
  GB_PROPERTY("Blue", "i", CCOLOR_info_blue),
  GB_PROPERTY("Hue", "i", CCOLOR_info_hue),
  GB_PROPERTY("Saturation", "i", CCOLOR_info_saturation),
  GB_PROPERTY("Value", "i", CCOLOR_info_value),
  GB_PROPERTY("Color", "i", CCOLOR_info_color),
  
  GB_END_DECLARE
};

GB_DESC CColorDesc[] =
{
  GB_DECLARE_STATIC("Color"),

  GB_CONSTANT("Default", "i", COLOR_DEFAULT),

  GB_CONSTANT("Black", "i", 0x000000),
  GB_CONSTANT("White", "i", 0xFFFFFF),
  
  GB_CONSTANT("LightGray", "i", 0xC0C0C0),
  GB_CONSTANT("Gray", "i", 0x808080),
  GB_CONSTANT("DarkGray", "i", 0x404040),

  GB_CONSTANT("Blue", "i", 0x0000FF),
  GB_CONSTANT("DarkBlue", "i", 0x000080),

  GB_CONSTANT("Green", "i", 0x00FF00),
  GB_CONSTANT("DarkGreen", "i", 0x008000),

  GB_CONSTANT("Red", "i", 0xFF0000),
  GB_CONSTANT("DarkRed", "i", 0x800000),

  GB_CONSTANT("Cyan", "i", 0x00FFFF),
  GB_CONSTANT("DarkCyan", "i", 0x008080),

  GB_CONSTANT("Magenta", "i", 0x00FF00FF),
  GB_CONSTANT("DarkMagenta", "i", 0x00800080),

  GB_CONSTANT("Yellow", "i", 0xFFFF00),
  GB_CONSTANT("DarkYellow", "i", 0x808000),

  GB_CONSTANT("Orange", "i", 0xFF8000),
  GB_CONSTANT("Violet", "i", 0x8000FF),
  GB_CONSTANT("Pink", "i", 0xFF80FF),

  GB_CONSTANT("Transparent", "i", 0xFF000000),

  GB_STATIC_METHOD("RGB", "i", Color_RGB, "(Red)i(Green)i(Blue)i[(Alpha)i]"),
  GB_STATIC_METHOD("HSV", "i", Color_HSV, "(Hue)i(Saturation)i(Value)i[(Alpha)i]"),

  GB_STATIC_METHOD("Lighter", "i", Color_Lighter, "(Color)i"),
  GB_STATIC_METHOD("Darker", "i", Color_Darker, "(Color)i"),
  GB_STATIC_METHOD("Merge", "i", Color_Merge, "(Color1)i(Color2)i[(Weight)f]"),
  GB_STATIC_METHOD("Gradient", "i", Color_Gradient, "(Color1)i(Color2)i[(Weight)f]"),
  GB_STATIC_METHOD("Blend", "i", Color_Blend, "(Source)i(Destination)i"),
  GB_STATIC_METHOD("Desaturate", "i", Color_Desaturate, "(Color)i"),

	GB_STATIC_METHOD("SetAlpha", "i", Color_SetAlpha, "(Color)i(Alpha)i"),
	GB_STATIC_METHOD("SetRGB", "i", Color_SetRGB, "(Color)i[(Red)i(Green)i(Blue)i(Alpha)i]"),
	GB_STATIC_METHOD("SetHSV", "i", Color_SetHSV, "(Color)i[(Hue)i(Saturation)i(Value)i(Alpha)i]"),
	GB_STATIC_METHOD("GetAlpha", "i", Color_GetAlpha, "(Color)i"),

  GB_STATIC_METHOD("Distance", "f", Color_Distance, "(Color1)i(Color2)i"),

  GB_STATIC_METHOD("_get", "ColorInfo", Color_get, "(Color)i"),
  //GB_STATIC_METHOD("_call", "ColorInfo", Color_get, "(Color)i"),
	
  GB_END_DECLARE
};


