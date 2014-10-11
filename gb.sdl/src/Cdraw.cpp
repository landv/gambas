/***************************************************************************

  Cdraw.cpp

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#define __CDRAW_CPP

#include "main.h"
#include "Cdraw.h"
#include "Cwindow.h"
#include "Cimage.h"

#define DRAW_STACK_MAX 8

static CDRAW draw_stack[DRAW_STACK_MAX];
static CDRAW *draw_current = 0;
static CFONT *_default_font = NULL;

#define THIS (draw_current)
#define GFX (THIS->graphic)
#define FONT (THIS->font)
#define WINDOWID(object) ((CWINDOW *)object)->id
#define IMAGEID(object) (CIMAGE_get((CIMAGE *)object))

/**************************************************************************/

static CFONT *get_default_font()
{
	if (!_default_font)
	{
		_default_font = (CFONT *)GB.New(GB.FindClass("Font"), NULL, NULL);
		GB.Ref(_default_font);
	}

	return _default_font;
}

static bool check_graphic(void)
{
	if (UNLIKELY(THIS == NULL))
	{
		GB.Error("No device");
		return true;
	}
	else
		return false;
}

#define CHECK_GRAPHIC() if (check_graphic()) return

void DRAW_begin(void *device)
{
	if (THIS >= &draw_stack[DRAW_STACK_MAX - 1]) {
		GB.Error("Too many nested drawings");
		return;
	}

	if (GB.CheckObject(device))
		return;

	if (THIS == 0)
		THIS = draw_stack;
	else
		THIS++;

	THIS->backcolor = 0x00000000;
	THIS->forecolor = 0x00FFFFFF;

	if (FONT)
		GB.Unref(POINTER(&FONT));

	FONT = get_default_font();
	GB.Ref(FONT);
	
	if (GB.Is(device, CLASS_Window)) {
		THIS->device = device;
		THIS->graphic = new SDLgfx(WINDOWID(device));
		GB.Ref(THIS->device);
		return;
	}
#if 0
	if (GB.Is(device, CLASS_Image))
	{
		THIS->device = device;
		THIS->graphic = new SDLgfx(IMAGEID(device));
		GB.Ref(THIS->device);
		return;
	}
#endif
	GB.Error("Device not supported !");
}

 void DRAW_end()
{
	if (!THIS)
		return;

	delete THIS->graphic;
	GB.Unref(POINTER(&THIS->device));
	THIS->device = 0;
	
	if (FONT)
		GB.Unref(POINTER(&FONT));
	
	FONT = 0;

	if (THIS == draw_stack)
		THIS = 0;
	else
		THIS--;
}

BEGIN_METHOD_VOID(Draw_exit)

	if (_default_font)
		GB.Unref(POINTER(&_default_font));

END_METHOD

BEGIN_METHOD(CDRAW_begin, GB_OBJECT device)

	void *device = VARG(device);
	DRAW_begin(device);

END_METHOD

BEGIN_METHOD_VOID(CDRAW_end)

	DRAW_end();

END_METHOD

BEGIN_METHOD(CDRAW_rotate, GB_FLOAT angle)

	CHECK_GRAPHIC();

	GFX->Rotate(VARG(angle));
	
END_METHOD

BEGIN_METHOD(CDRAW_scale, GB_FLOAT x; GB_FLOAT y)

	CHECK_GRAPHIC();

	GFX->Scale(VARG(x), VARG(y));
	
END_METHOD

BEGIN_METHOD(CDRAW_point, GB_INTEGER x; GB_INTEGER y)

	CHECK_GRAPHIC();

	GFX->SetColor(THIS->forecolor);
	GFX->DrawPixel(VARG(x), VARG(y));

END_METHOD

BEGIN_METHOD(CDRAW_line, GB_INTEGER x1; GB_INTEGER y1; GB_INTEGER x2; GB_INTEGER y2)

	CHECK_GRAPHIC();

	GFX->SetColor(THIS->forecolor);
	GFX->DrawLine(VARG(x1), VARG(y1), VARG(x2), VARG(y2));

END_METHOD

BEGIN_METHOD(CDRAW_rect, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	CHECK_GRAPHIC();

	GFX->SetColor(THIS->forecolor);
	GFX->DrawRect(VARG(x), VARG(y), VARG(w), VARG(h));

END_METHOD

BEGIN_METHOD(CDRAW_ellipse, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	CHECK_GRAPHIC();

	GFX->SetColor(THIS->forecolor);
	GFX->DrawEllipse(VARG(x), VARG(y), VARG(w), VARG(h));

END_METHOD

BEGIN_METHOD(CDRAW_text, GB_STRING text; GB_INTEGER x; GB_INTEGER y)

	int scale;

	CHECK_GRAPHIC();

	if (GB.CheckObject(THIS->font))
		return;

	SDLsurface *txt = FONT->font->RenderText(STRING(text), LENGTH(text));
	if (!txt)
		return;
	
/*	if (THIS->backcolor != 0)
	{
		int fill = GFX->GetFillStyle();
		GFX->SetFillStyle(SDL::SolidFill);
		GFX->SetColor(THIS->backcolor);
		GFX->DrawRect(VARG(x), VARG(y), txt->GetWidth(), txt->GetHeight());
		GFX->SetFillStyle(fill);
	}*/
	
	GFX->SetColor(THIS->forecolor);

	scale = FONT->font->GetScale();

	if (scale == 1)
		GFX->Blit(txt, VARG(x), VARG(y));
	else
		GFX->Blit(txt, VARG(x), VARG(y), 0, 0, -1, -1, txt->GetWidth() * scale, txt->GetHeight() * scale, true);

	txt->Unref();
	
END_METHOD

BEGIN_METHOD(CDRAW_image, GB_OBJECT image; GB_INTEGER x; GB_INTEGER y; GB_INTEGER width; GB_INTEGER height;
		GB_INTEGER srcx; GB_INTEGER srcy; GB_INTEGER srcw; GB_INTEGER srch)

	CHECK_GRAPHIC();

	CIMAGE *image = (CIMAGE *) VARG(image);

	if (!image)
		return;

	GFX->SetColor(THIS->forecolor);
	GFX->Blit(IMAGEID(image), VARG(x), VARG(y), VARGOPT(srcx,0), VARGOPT(srcy,0),
		VARGOPT(srcw,-1), VARGOPT(srch,-1), VARGOPT(width,-1), VARGOPT(height,-1));

END_METHOD

BEGIN_PROPERTY(CDRAW_defaultfont)

	CHECK_GRAPHIC();

END_PROPERTY

BEGIN_PROPERTY(CDRAW_font)

	CHECK_GRAPHIC();

	if (READ_PROPERTY)
		GB.ReturnObject(FONT);
	else
	{
		if (FONT)
			GB.Unref(POINTER(&FONT));
		
		FONT = (CFONT *) VPROP(GB_OBJECT);
		GB.Ref(FONT);
	}
	
END_PROPERTY

BEGIN_PROPERTY(CDRAW_linestyle)

	CHECK_GRAPHIC();

	if (READ_PROPERTY)
		GB.ReturnInteger(GFX->GetLineStyle());
	else
		GFX->SetLineStyle(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CDRAW_linewidth)

	CHECK_GRAPHIC();

	if (READ_PROPERTY)
		GB.ReturnInteger(GFX->GetLineWidth());
	else
		GFX->SetLineWidth(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CDRAW_fillstyle)

	CHECK_GRAPHIC();

	if (READ_PROPERTY)
		GB.ReturnInteger(GFX->GetFillStyle());
	else
		GFX->SetFillStyle(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CDRAW_background)

	CHECK_GRAPHIC();

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->backcolor);
	else
		THIS->backcolor = VPROP(GB_INTEGER);

END_PROPERTY

BEGIN_PROPERTY(CDRAW_foreground)

	CHECK_GRAPHIC();

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->forecolor);
	else
		THIS->forecolor = VPROP(GB_INTEGER);

END_PROPERTY

/**************************************************************************/

GB_DESC CDraw[] =
{
  GB_DECLARE("Draw", 0), GB_NOT_CREATABLE(),

  GB_STATIC_METHOD("_exit", NULL, Draw_exit, NULL),

  GB_STATIC_METHOD("Begin", NULL, CDRAW_begin, "(Device)o"),
  GB_STATIC_METHOD("End", NULL, CDRAW_end, NULL),

  GB_STATIC_METHOD("Rotate", NULL, CDRAW_rotate, "(Angle)f"),
  GB_STATIC_METHOD("Scale", NULL, CDRAW_scale, "(X)f(Y)f"),

  GB_STATIC_METHOD("Point", NULL, CDRAW_point, "(X)i(Y)i"),
  GB_STATIC_METHOD("Line", NULL, CDRAW_line, "(X1)i(Y1)i(X2)i(Y2)i"),
  GB_STATIC_METHOD("Rect", NULL, CDRAW_rect, "(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("Ellipse", NULL, CDRAW_ellipse, "(X)i(Y)i(Width)i(Height)i"),

  GB_STATIC_METHOD("Text", NULL, CDRAW_text, "(text)s(X)i(Y)i"),
  GB_STATIC_METHOD("Image", NULL, CDRAW_image, "(Image)Image;(X)i(Y)i[(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),

//  GB_STATIC_PROPERTY("DefaultFont",  "Font", CDRAW_defaultfont),
  GB_STATIC_PROPERTY("Font", "Font", CDRAW_font),

  GB_STATIC_PROPERTY("LineStyle", "i", CDRAW_linestyle),
  GB_STATIC_PROPERTY("LineWidth", "i", CDRAW_linewidth),
  GB_STATIC_PROPERTY("FillStyle", "i", CDRAW_fillstyle),
  GB_STATIC_PROPERTY("Background", "i", CDRAW_background),
  GB_STATIC_PROPERTY("BackColor", "i", CDRAW_background),
  GB_STATIC_PROPERTY("Foreground", "i", CDRAW_foreground),
  GB_STATIC_PROPERTY("ForeColor", "i", CDRAW_foreground),

  GB_END_DECLARE
};
