/***************************************************************************

  c_color.c

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __C_COLOR_C

#include <math.h>
#include "c_color.h"

/*static void gt_color_to_rgb(uint color, int *r, int *g, int *b)
{
	*b = color & 0xFF;
	*g = (color >> 8) & 0xFF;
	*r = (color >> 16) & 0xFF;
}

static uint gt_rgb_to_color(int r, int g, int b)
{
	return (uint)(b | (g << 8) | (r << 16));
}*/

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
	float R, G, B;
	float v, x, f;
	int i;

	R = (float)r / 255;
	G = (float)g / 255;
	B = (float)b / 255;

	x = R;
	if (G < x) x = G;
	if (B < x) x = B;

	v = R;
	if (G > v) v = G;
	if (B > v) v = B;

	if (v == x) 
	{
		*H = -1;
		*S = 0;
		*V = (int)(v * 255);
	}
	else
	{
		f = (R == x) ? G - B : ((G == x) ? B - R : R - G);
		i = (R == x) ? 3 : ((G == x) ? 5 : 1);
		*H = (int)((i - f / (v - x)) * 60);
		*S = (int)(((v - x) / v) * 255);
		*V = (int)(v * 255);
	}	
}

static void gt_rgb_to_hsv_cached(int r, int g, int b, int *H, int *S, int *V)
{
	static int old_r = 0, old_g = 0, old_b = 0, old_h = -1, old_s = 0, old_v = 0;

	if (r == old_r && g == old_g && b == old_b)
	{
		*H = old_h;
		*S = old_s;
		*V = old_v;
		return;
	}

	COLOR_rgb_to_hsv(r, g, b, H, S, V);

	old_r = r;
	old_g = g;
	old_b = b;
	old_h = *H;
	old_s = *S;
	old_v = *V;
}

void COLOR_hsv_to_rgb(int h, int s, int v, int *R, int *G, int *B)
{
	 double H,S,V;
	 double var_h, var_i, var_1, var_2, var_3, tmp_r, tmp_g, tmp_b;

	if (h < 0)
		h = 360 - ((-h) % 360);
	else
		h = h % 360;

	 H = ((double)h) / 360;
	 S = ((double)s) / 255;
	 V = ((double)v) / 255;

	if (s == 0)
	{
		*R = (int)(V * 255);
		*G = (int)(V * 255);
		*B = (int)(V * 255);
	}
	else
	{
		var_h = H * 6;
		var_i = (int)var_h;
		var_1 = V * ( 1 - S );
		var_2 = V * ( 1 - S * ( var_h - var_i ) );
		var_3 = V * ( 1 - S * ( 1 - ( var_h - var_i ) ) );

		switch ((int)var_i)
		{
			case 0:
				tmp_r = V;
				tmp_g = var_3;
				tmp_b = var_1;
				break;

			case 1:
				tmp_r = var_2;
				tmp_g = V;
				tmp_b = var_1;
				break;

			case 2:
				tmp_r = var_1;
				tmp_g = V;
				tmp_b = var_3;
				break;

			case 3:
				tmp_r = var_1;
				tmp_g = var_2;
				tmp_b = V;
				break;

			case 4:
				tmp_r = var_3;
				tmp_g = var_1;
				tmp_b = V;
				break;

			default:
				tmp_r = V;
				tmp_g = var_1;
				tmp_b = var_2;
				break;
		}

		*R = (int)(tmp_r * 255);
		*G = (int)(tmp_g * 255);
		*B = (int)(tmp_b * 255);

	}
}

GB_COLOR COLOR_merge(GB_COLOR col1, GB_COLOR col2, double weight)
{
	int a, r, g, b;
	
	if (weight == 0.0)
		return col1;
	else if (weight == 1.0)
		return col2;
	else
	{
		#define MIX_COLOR(_shift) (int)((((col2 >> _shift) & 0xFF) * weight + ((col1 >> _shift) & 0xFF) * (1 - weight)) + 0.5)
		
		a = MIX_COLOR(24);
		r = MIX_COLOR(16);
		g = MIX_COLOR(8);
		b = MIX_COLOR(0);
		
		return gt_rgba_to_color(r, g, b, a);
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

BEGIN_METHOD(CCOLOR_rgb, GB_INTEGER r; GB_INTEGER g; GB_INTEGER b; GB_INTEGER a)

  GB.ReturnInteger(gt_rgba_to_color(VARG(r), VARG(g), VARG(b), VARGOPT(a, 0)));

END_METHOD

BEGIN_METHOD(CCOLOR_hsv, GB_INTEGER h; GB_INTEGER s; GB_INTEGER v; GB_INTEGER a)

	int r, g, b;
	COLOR_hsv_to_rgb(VARG(h), VARG(s), VARG(v), &r, &g, &b);
  GB.ReturnInteger(gt_rgba_to_color(r, g, b, VARGOPT(a, 0)));

END_METHOD

BEGIN_METHOD(CCOLOR_get, GB_INTEGER color)

	CCOLOR *info;

	GB.New(POINTER(&info), GB.FindClass("ColorInfo"), NULL, NULL);
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

BEGIN_METHOD(CCOLOR_lighter, GB_INTEGER color)

  GB.ReturnInteger(COLOR_lighter(VARG(color)));
  
END_METHOD

BEGIN_METHOD(CCOLOR_darker, GB_INTEGER color)

  GB.ReturnInteger(COLOR_darker(VARG(color)));

END_METHOD

BEGIN_METHOD(CCOLOR_merge, GB_INTEGER color1; GB_INTEGER color2; GB_FLOAT weight)

	GB.ReturnInteger(COLOR_merge(VARG(color1), VARG(color2), VARGOPT(weight, 0.5)));

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
  GB_DECLARE("Color", 0), GB_NOT_CREATABLE(),

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

  GB_CONSTANT("Transparent", "i", 0xFFFFFFFF),

  GB_STATIC_METHOD("RGB", "i", CCOLOR_rgb, "(Red)i(Green)i(Blue)i[(Alpha)i]"),
  GB_STATIC_METHOD("HSV", "i", CCOLOR_hsv, "(Hue)i(Saturation)i(Value)i[(Alpha)i]"),
  
  GB_STATIC_METHOD("Lighter", "i", CCOLOR_lighter, "(Color)i"),
  GB_STATIC_METHOD("Darker", "i", CCOLOR_darker, "(Color)i"),
  GB_STATIC_METHOD("Merge", "i", CCOLOR_merge, "(Color1)i(Color2)i[(Weight)f]"),
  GB_STATIC_METHOD("Blend", "i", CCOLOR_blend, "(Source)i(Destination)i"),

  GB_STATIC_METHOD("_get", "ColorInfo", CCOLOR_get, "(Color)i"),
  //GB_STATIC_METHOD("_call", "ColorInfo", CCOLOR_get, "(Color)i"),
	
  GB_END_DECLARE
};


