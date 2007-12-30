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

#include "widgets.h"
#include "main.h"
#include "gambas.h"
#include "CColor.h"
#include <stdio.h>
#include <math.h>

void to_hsv( int r,int g, int b,int *H, int *S,int *V )
{

	float R = (float)r, G = (float)g, B = (float)b;
	float v, x, f;
	int i;

	R/=255;
	G/=255;
	B/=255;

	x=R;
	if (G<x) x=G;
	if (B<x) x=B;

	v=R;
	if (G>v) v=G;
	if (B>v) v=B;


	if(v == x) {
		*H=-1;
		*S=0;
		*V=v*255;
		return;
	}

	f = (R == x) ? G - B : ((G == x) ? B - R : R - G);
	i = (R == x) ? 3 : ((G == x) ? 5 : 1);
	*H=(int)((i - f /(v - x))*60);
	*S=(int)(((v - x)/v)*255);
	*V=(int)(v*255);



}

void to_rgb(int h,int s,int v,int *R,int *G,int *B)
{
	 double H,S,V;
	 double var_h,var_i,var_1,var_2,var_3,tmp_r,tmp_g,tmp_b;

	 H=((double)h)/360;
	 S=((double)s)/255;
	 V=((double)v)/255;

	if ( S == 0 )
	{
		*R = V * 255;
		*G = V * 255;
		*B = V * 255;
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

		*R = tmp_r * 255;
		*G = tmp_g * 255;
		*B = tmp_b * 255;

	}
}


BEGIN_METHOD(CCOLOR_rgb, GB_INTEGER r; GB_INTEGER g; GB_INTEGER b; GB_INTEGER a)

  long hCol;

  hCol=VARG(b) & 0xFF;
  hCol|= ( (VARG(g) & 0xFF)<<8 );
  hCol|= ( (VARG(r) & 0xFF)<<16 );

  GB.ReturnInteger(hCol & 0x00FFFFFF | ((VARGOPT(a, 0) & 0xFF) << 24));

END_METHOD

BEGIN_METHOD(CCOLOR_hsv, GB_INTEGER h; GB_INTEGER s; GB_INTEGER v)

	int R,G,B;
	long hCol;

    to_rgb(VARG(h),VARG(s),VARG(v),&R,&G,&B);

	hCol=B & 0xFF;
  	hCol|= ( (G & 0xFF)<<8 );
  	hCol|= ( (R & 0xFF)<<16 );


	GB.ReturnInteger(hCol & 0x00FFFFFF);

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

BEGIN_PROPERTY(CCOLOR_selected_foreground)

	GB.ReturnInteger(gDesktop::selfgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_button_background)

	GB.ReturnInteger(gDesktop::buttonbgColor());

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_button_foreground)

	GB.ReturnInteger(gDesktop::buttonfgColor());

END_PROPERTY

static long _color;

BEGIN_METHOD(CCOLOR_get, GB_INTEGER color)

  _color = VARG(color);
  RETURN_SELF();

END_METHOD

BEGIN_PROPERTY(CCOLOR_info_alpha)

  GB.ReturnInteger(((_color >> 24) & 0xFF) ^ 0xFF);

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

  int H,S,V;

  to_hsv((_color >> 16) & 0xFF,(_color >> 8) & 0xFF, _color & 0xFF,&H,&S,&V);
  GB.ReturnInteger(H);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_saturation)

  int H,S,V;

  to_hsv((_color >> 16) & 0xFF,(_color >> 8) & 0xFF, _color & 0xFF,&H,&S,&V);
  GB.ReturnInteger(S);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_info_value)

  int H,S,V;

  to_hsv((_color >> 16) & 0xFF,(_color >> 8) & 0xFF, _color & 0xFF,&H,&S,&V);
  GB.ReturnInteger(V);

END_PROPERTY

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

  GB_CONSTANT("Default", "i", -1),

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
  GB_STATIC_PROPERTY("TextBackground", "i", CCOLOR_text_background),
  GB_STATIC_PROPERTY("ButtonBackground", "i", CCOLOR_button_background),

  GB_STATIC_PROPERTY("Foreground", "i", CCOLOR_foreground),
  GB_STATIC_PROPERTY("SelectedForeground", "i", CCOLOR_selected_foreground),
  GB_STATIC_PROPERTY("TextForeground", "i", CCOLOR_text_foreground),
  GB_STATIC_PROPERTY("ButtonForeground", "i", CCOLOR_button_foreground),

  GB_STATIC_METHOD("RGB", "i", CCOLOR_rgb, "(Red)i(Green)i(Blue)i[(Alpha)i]"),
  GB_STATIC_METHOD("HSV", "i", CCOLOR_hsv, "(Hue)i(Saturation)i(Value)i"),

  GB_STATIC_METHOD("_get", ".ColorInfo", CCOLOR_get, "(Color)i"),

  GB_END_DECLARE
};


