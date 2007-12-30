/***************************************************************************

  Ccolor.cpp

  The SDL component

  (c) 2006 Laurent Carlier <lordheavy@infonie.fr>
           Benoit Minisini <gambas@users.sourceforge.net>

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

#include "gambas.h"
#include "main.h"

#include "Ccolor.h"

#include "SDL.h"

static long _color;
static int _h = 0;
static int _s = 0;
static int _v = 0;

static Uint32 HSVtoRGB(int h, int s, int v)
{
	if ( h < -1 || (uint)s > 255 || (uint)v > 255 )
		return 0;

	int r=v, g=v, b=v;

	if ( s == 0 || h == -1 )
	{}
	else
	{
		if ( (uint)h >= 360 )
			h %= 360;

		uint f = h%60;
		h /= 60;
		uint p = (uint)(2*v*(255-s)+255)/510;
		uint q, t;

		if ( h&1 )
		{
			q = (uint)(2*v*(15300-s*f)+15300)/30600;

			switch( h )
			{
				case 1:
				{
					r=(int)q;
					g=(int)v;
					b=(int)p;
					break;
				}
				case 3:
				{
					r=(int)p;
					g=(int)q;
					b=(int)v;
					break;
				}
				case 5:
				{
					r=(int)v;
					g=(int)p;
					b=(int)q;
					break;
				}
		    	}
		}
		else
		{
			t = (uint)(2*v*(15300-(s*(60-f)))+15300)/30600;

			switch( h )
			{
				case 0:
				{
					r=(int)v;
					g=(int)t;
					b=(int)p;
					break;
				}
				case 2:
				{
					r=(int)p;
					g=(int)v;
					b=(int)t;
					break;
				}
				case 4:
				{
					r=(int)t;
					g=(int)p;
					b=(int)v;
					break;
		    		}
			}
		}
	}

	return (((r & 0xFF) << 24) + ((g & 0xFF) << 16) + ((b & 0xFF) << 8) + 0xFF);
}

static void RGBtoHSV(int color)
{
	int r = ((color >> 24) & 0xFF);
	int g = ((color >> 16) & 0xFF);
	int b = ((color >> 8) & 0xFF);
	int mn = r, mx = r;
	int maxVal = 0; 

	if (g > mx)
	{
		mx = g;
		maxVal = 1;
	}

	if (b > mx)
	{
		mx = b;
		maxVal = 2;
	}

	if (g < mn)
		mn = g;

	if (b < mn)
		mn = b;

	int delta = mx - mn;
	_v = mx;

	_s = mx ? ((510*delta+mx)/(2*mx)) : 0;


	if (_s==0) 
	{
		_h = -1;
	}
	else
	{
		switch (maxVal)
		{
			case 0:
			{
				if ( g >= b )
					_h = (120*(g-b)+delta)/(2*delta);
				else
		    			_h = (120*(g-b+delta)+delta)/(2*delta) + 300;
				break;
			} 
			case 1:
			{
				if ( b > r )
					_h = 120 + (120*(b-r)+delta)/(2*delta);
				else
					_h = 60 + (120*(b-r+delta)+delta)/(2*delta);
				break;
			}
			case 2:
			{
				if ( r > g )
					_h = 240 + (120*(r-g)+delta)/(2*delta);
				else
					_h = 180 + (120*(r-g+delta)+delta)/(2*delta);
				break;
			}
		}
    	}

	if( _h < 0 )
		_h = 0;
}

static void get_hsv(long col)
{
	static long last = 0;

	if (last == col)
		return;

	RGBtoHSV(col);
	last = col;
}

BEGIN_METHOD(CCOLOR_rgb, GB_INTEGER r; GB_INTEGER g; GB_INTEGER b; GB_INTEGER a)

  Uint8 red, green, blue, alpha;

  red   = VARG(r);
  green = VARG(g);
  blue  = VARG(b);
  alpha = VARGOPT(a, 0xFF);

  GB.ReturnInteger((red << 24) | (green << 16) | (blue << 8) | alpha);

END_METHOD

BEGIN_METHOD(CCOLOR_hsv, GB_INTEGER h; GB_INTEGER s; GB_INTEGER v)

  Uint8 rh, gs, bv;

  rh   = VARG(h);
  gs = VARG(s);
  bv  = VARG(v);

  GB.ReturnInteger(HSVtoRGB(rh, gs, bv));

END_METHOD

BEGIN_METHOD(CCOLOR_get, GB_INTEGER color)

  _color = VARG(color);
  RETURN_SELF();

END_METHOD

BEGIN_PROPERTY(CCOLORINFO_alpha)

  GB.ReturnInteger(_color & 0xFF);

END_PROPERTY

BEGIN_PROPERTY(CCOLORINFO_red)

  GB.ReturnInteger((_color >> 24) & 0xFF);

END_PROPERTY

BEGIN_PROPERTY(CCOLORINFO_green)

  GB.ReturnInteger((_color >> 16) & 0xFF);

END_PROPERTY

BEGIN_PROPERTY(CCOLORINFO_blue)

  GB.ReturnInteger((_color >> 8) & 0xFF);

END_PROPERTY

BEGIN_PROPERTY(CCOLORINFO_hue)

  get_hsv(_color);
  GB.ReturnInteger(_h);

END_PROPERTY

BEGIN_PROPERTY(CCOLORINFO_saturation)

  get_hsv(_color);
  GB.ReturnInteger(_s);

END_PROPERTY

BEGIN_PROPERTY(CCOLORINFO_value)

  get_hsv(_color);
  GB.ReturnInteger(_v);

END_PROPERTY

GB_DESC CColorInfo[] =
{
  GB_DECLARE(".ColorInfo", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY("Alpha", "i", CCOLORINFO_alpha),
  GB_STATIC_PROPERTY("Red", "i", CCOLORINFO_red),
  GB_STATIC_PROPERTY("Green", "i", CCOLORINFO_green),
  GB_STATIC_PROPERTY("Blue", "i", CCOLORINFO_blue),
  GB_STATIC_PROPERTY("Hue", "i", CCOLORINFO_hue),
  GB_STATIC_PROPERTY("Saturation", "i", CCOLORINFO_saturation),
  GB_STATIC_PROPERTY("Value", "i", CCOLORINFO_value),

  GB_END_DECLARE
};

GB_DESC CColor[] =
{
  GB_DECLARE("Color", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("RGB", "i", CCOLOR_rgb, "(Red)i(Green)i(Blue)i[(Alpha)i]"),
  GB_STATIC_METHOD("HSV", "i", CCOLOR_hsv, "(Hue)i(Saturation)i(Value)i"),
  GB_STATIC_METHOD("_get", ".ColorInfo", CCOLOR_get, "(Color)i"),

  GB_CONSTANT("Black", "i", 0x000000FF),
  GB_CONSTANT("White", "i", 0xFFFFFFFF),
  GB_CONSTANT("LightGray", "i", 0xC0C0C0FF),
  GB_CONSTANT("Gray", "i", 0x808080FF),
  GB_CONSTANT("DarkGray", "i", 0x404040FF),
  GB_CONSTANT("Blue", "i", 0x0000FFFF),
  GB_CONSTANT("DarkBlue", "i", 0x000080FF),
  GB_CONSTANT("Green", "i", 0x00FF00FF),
  GB_CONSTANT("DarkGreen", "i", 0x008000FF),
  GB_CONSTANT("Red", "i", 0xFF0000FF),
  GB_CONSTANT("DarkRed", "i", 0x800000FF),
  GB_CONSTANT("Cyan", "i", 0x00FFFFFF),
  GB_CONSTANT("DarkCyan", "i", 0x008080FF),
  GB_CONSTANT("Magenta", "i", 0xFF00FFFF),
  GB_CONSTANT("DarkMagenta", "i", 0x800080FF),
  GB_CONSTANT("Yellow", "i", 0xFFFF00FF),
  GB_CONSTANT("DarkYellow", "i", 0x808000FF),
  GB_CONSTANT("Orange", "i", 0xFF8000FF),
  GB_CONSTANT("Violet", "i", 0x8000FFFF),
  GB_CONSTANT("Pink", "i", 0xFF80FFFF),

  GB_END_DECLARE
};

