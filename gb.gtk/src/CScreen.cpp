/***************************************************************************

  CScreen.cpp

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

#define __CSCREEN_CPP

#include "CWindow.h"
#include "CPicture.h"
#include "CFont.h"
#include "CScreen.h"

#include "gapplication.h"
#include "gmainwindow.h"

extern long CWINDOW_Embedder;
extern bool CWINDOW_Embedded;

extern int MAIN_scale;

char *CAPPLICATION_Theme = 0;

static int _busy = 0;

BEGIN_PROPERTY(CSCREEN_width)

	GB.ReturnInteger(gDesktop::width());

END_PROPERTY


BEGIN_PROPERTY(CSCREEN_height)

	GB.ReturnInteger(gDesktop::height());

END_PROPERTY


BEGIN_PROPERTY(CSCREEN_resolution)

	GB.ReturnInteger(gDesktop::resolution());

END_PROPERTY

BEGIN_PROPERTY(CSCREEN_charset)

  GB.ReturnConstZeroString("UTF-8");

END_PROPERTY


BEGIN_METHOD_VOID(CSCREEN_grab)

	CPICTURE *pic=NULL;
	gPicture *buf=gDesktop::grab();
  
	GB.New(POINTER(&pic), GB.FindClass("Picture"), 0, 0);
	if (pic->picture) pic->picture->unref();
	pic->picture=buf;
	GB.ReturnObject(pic);

END_METHOD


static void set_font(gFont *font, void *object = 0)
{
  gDesktop::setFont(font);
	MAIN_scale = gDesktop::scale();
}

BEGIN_PROPERTY(CAPP_font)

  if (READ_PROPERTY)
    GB.ReturnObject(CFONT_create(gDesktop::font(), set_font));
  else if (VPROP(GB_OBJECT))
    set_font(((CFONT*)VPROP(GB_OBJECT))->font);

END_PROPERTY

#if 0
BEGIN_PROPERTY(CSCREEN_font)

  	CFONT *Font;
	
	
	if (READ_PROPERTY)
	{
		GB.New((void **)&Font, GB.FindClass("Font"), 0, 0);
		Font->font->unref();
		Font->font=gDesktop::font();
		Font->font->ref();
		GB.ReturnObject(Font);
		return;
	}
	
	Font=(CFONT*)VPROP(GB_OBJECT);
	if (!Font) return;
	if (!Font->font) return;
	gDesktop::setFont(Font->font);

END_PROPERTY
#endif

BEGIN_PROPERTY(CSCREEN_active_window)

	GB.ReturnObject(CWINDOW_Active);

END_PROPERTY

BEGIN_PROPERTY(CSCREEN_active_control)

  gControl *win=gDesktop::activeControl();
  
  if (!win) { GB.ReturnNull(); return; }
  GB.ReturnObject(GetObject(win));

END_PROPERTY

BEGIN_PROPERTY(CSCREEN_busy)

  int busy;

  if (READ_PROPERTY)
    GB.ReturnInteger(_busy);
  else
  {
    busy = VPROP(GB_INTEGER);

    if (_busy == 0 && busy != 0)
      gApplication::setBusy(true);
    else if (_busy > 0 && busy == 0)
      gApplication::setBusy(false);

    _busy = busy;
  }

END_PROPERTY

BEGIN_PROPERTY(CSCREEN_scale)

	GB.ReturnInteger(MAIN_scale);

END_PROPERTY



BEGIN_PROPERTY(CAPP_tooltip_enabled)

  if (READ_PROPERTY) { GB.ReturnBoolean(gApplication::toolTips()); return; }
  gApplication::enableTooltips(VPROP(GB_BOOLEAN));


END_PROPERTY


BEGIN_PROPERTY(CAPP_tooltip_font)

  	CFONT *Font;
	
	
	if (READ_PROPERTY)
	{
		GB.ReturnObject(CFONT_create(gApplication::toolTipsFont()));
		return;
	}
	
	Font=(CFONT*)VPROP(GB_OBJECT);
	if (!Font) return;
	if (!Font->font) return;
	gApplication::setToolTipsFont(Font->font);
  
END_PROPERTY


BEGIN_PROPERTY(CAPP_main_window)

  GB.ReturnObject(WINDOW_get_main());

END_PROPERTY

BEGIN_METHOD_VOID(CAPP_exit)

	GB.FreeString(&CAPPLICATION_Theme);

END_METHOD

BEGIN_PROPERTY(CAPP_theme)

	if (READ_PROPERTY) { GB.ReturnString(CAPPLICATION_Theme); return; }
	GB.StoreString(PROP(GB_STRING), &CAPPLICATION_Theme);

END_PROPERTY




GB_DESC CDesktopDesc[] =
{
  GB_DECLARE("Desktop", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("W", "i", CSCREEN_width),
  GB_STATIC_PROPERTY_READ("H", "i", CSCREEN_height),
  GB_STATIC_PROPERTY_READ("Width", "i", CSCREEN_width),
  GB_STATIC_PROPERTY_READ("Height", "i", CSCREEN_height),
  GB_STATIC_PROPERTY_READ("Charset", "s", CSCREEN_charset),
  GB_STATIC_PROPERTY_READ("Resolution", "i", CSCREEN_resolution),
  GB_STATIC_PROPERTY_READ("Scale","i",CSCREEN_scale),

  GB_STATIC_METHOD("Grab", "Picture", CSCREEN_grab, 0),

  GB_END_DECLARE
};

GB_DESC CApplicationTooltipDesc[] =
{
  GB_DECLARE(".ApplicationTooltip", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY("Enabled", "b", CAPP_tooltip_enabled),
  GB_STATIC_PROPERTY("Font", "Font", CAPP_tooltip_font),

  GB_END_DECLARE
};

BEGIN_PROPERTY(CAPP_embedder)

  if (READ_PROPERTY)
    GB.ReturnInteger(CWINDOW_Embedder);
  else
  {
    if (CWINDOW_Embedded)
    {
      GB.Error("Application is already embedded");
      return;
    }
  
    CWINDOW_Embedder = VPROP(GB_INTEGER);
  }

END_PROPERTY

GB_DESC CApplicationDesc[] =
{
  GB_DECLARE("Application", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_exit", NULL, CAPP_exit, 0),

  GB_STATIC_PROPERTY("Font", "Font", CAPP_font),
  GB_STATIC_PROPERTY_READ("ActiveControl","Control",CSCREEN_active_control),
  GB_STATIC_PROPERTY_READ("ActiveWindow", "Window", CSCREEN_active_window),
  GB_STATIC_PROPERTY_READ("MainWindow", "Window", CAPP_main_window),
  GB_STATIC_PROPERTY("Busy", "i", CSCREEN_busy),
  GB_STATIC_PROPERTY_SELF("ToolTip", ".ApplicationTooltip"),
  
  GB_STATIC_PROPERTY("Embedder", "i", CAPP_embedder),
  GB_STATIC_PROPERTY("Theme", "s", CAPP_theme),

  GB_END_DECLARE
};


