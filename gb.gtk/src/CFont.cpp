/***************************************************************************

  CFont.cpp

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

#define __CFONT_CPP

#include <stdlib.h>
#include <math.h>
#include "main.h"
#include "gambas.h"
#include "widgets.h"
#include "CFont.h"


BEGIN_METHOD(CFONT_new, GB_STRING font)

	FONT=new gFont();

END_METHOD


BEGIN_METHOD_VOID(CFONT_free)

	FONT->unref();	

END_METHOD



BEGIN_PROPERTY(CFONT_name)

	const char *buf;
	
	if (READ_PROPERTY)
	{
		buf=FONT->name();
		GB.ReturnNewString(buf,0);
		return;
	}
	
	FONT->setName(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(CFONT_size)

	if (READ_PROPERTY) { GB.ReturnFloat(FONT->size()); return; }
	FONT->setSize(VPROP(GB_FLOAT));

END_PROPERTY

BEGIN_PROPERTY(CFONT_bold)

	if (READ_PROPERTY){ GB.ReturnBoolean(FONT->bold()); return; }
	FONT->setBold(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CFONT_italic)

	if (READ_PROPERTY){ GB.ReturnBoolean(FONT->italic()); return; }
	FONT->setItalic(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD_VOID(CFONT_to_string)

	const char *buf;

	buf=FONT->toString();
	GB.ReturnNewString(buf,0);
	free((void*)buf);

END_METHOD


BEGIN_METHOD(CFONT_get, GB_STRING str)

	CFONT *font;
	
	GB.New((void **)&font, GB.FindClass("Font"), 0, 0);
	delete font->font;
	font->font=new gFont(GB.ToZeroString(PROP(GB_STRING)));
	GB.ReturnObject((void*)font);
  
END_METHOD


BEGIN_METHOD(CFONT_width, GB_STRING text)

	GB.ReturnInteger( FONT->width( STRING(text) ) );

END_METHOD


BEGIN_PROPERTY(CFONT_ascent)

	GB.ReturnInteger(FONT->ascent());

END_PROPERTY


BEGIN_PROPERTY(CFONT_descent)

	GB.ReturnInteger(FONT->descent());

END_PROPERTY


BEGIN_METHOD(CFONT_height, GB_STRING text)

	GB.ReturnInteger( FONT->height( STRING(text) ) );

END_METHOD

BEGIN_PROPERTY(CFONT_fixed)

	GB.ReturnBoolean(FONT->fixed());

END_PROPERTY

#if 0
BEGIN_PROPERTY(CFONT_resolution)



END_PROPERTY
#endif

BEGIN_METHOD_VOID(CFONTS_next)

	long *pos;
	
	pos=(long*)GB.GetEnum();
	if ( pos[0]>=gFont::count() )
	{
		GB.StopEnum ();
		return;
	}
	
	GB.ReturnNewString ( gFont::familyItem(pos[0]++),0 );  
  
END_METHOD


BEGIN_PROPERTY(CFONTS_count)
	
	GB.ReturnInteger(gFont::count());

END_PROPERTY


BEGIN_PROPERTY(CFONT_styles)

	stub("CFONT_styles");

END_PROPERTY

BEGIN_PROPERTY(CFONT_grade)

	if (READ_PROPERTY)
	{
		float r=logf(FONT->size()) / logf(gDesktop::font()->size());
        GB.ReturnInteger((int)(10 * r + 0.5) - 10);
		return;
	}
	
	long g=VPROP(GB_INTEGER);
	if (g<-9) g=-9;
	if (g>9) g=9;
	FONT->setSize((long)(powf(gDesktop::font()->size(), 1.0 + ((int)g / 10.0)) + 0.5));
		
END_PROPERTY

BEGIN_PROPERTY(CFONT_scalable)

	GB.ReturnBoolean(FONT->scalable());

END_PROPERTY

BEGIN_PROPERTY(CFONT_underline)

	if (READ_PROPERTY) { GB.ReturnBoolean(FONT->underline()); return; }
	FONT->setUnderline(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CFONT_strikeout)

	if (READ_PROPERTY) { GB.ReturnBoolean(FONT->strikeOut()); return; }
	FONT->setStrikeOut(VPROP(GB_BOOLEAN));

END_PROPERTY


GB_DESC CFontsDesc[] =
{
  GB_DECLARE("Fonts", 0), 
  GB_NOT_CREATABLE(),

  GB_STATIC_METHOD("_next", "s", CFONTS_next, NULL),
  GB_STATIC_PROPERTY_READ("Count", "i", CFONTS_count),
  
  GB_END_DECLARE
};


GB_DESC CFontDesc[] =
{
  GB_DECLARE("Font", sizeof(CFONT)), 

  GB_METHOD("_new", NULL, CFONT_new, "[(Font)s]"),
  GB_METHOD("_free", NULL, CFONT_free, NULL),
  
  GB_PROPERTY("Name", "s", CFONT_name),
  GB_PROPERTY("Size", "f", CFONT_size),
  GB_PROPERTY("Bold", "b", CFONT_bold),
  GB_PROPERTY("Italic", "b", CFONT_italic),
  GB_PROPERTY("Underline", "b", CFONT_underline),
  GB_PROPERTY("StrikeOut", "b", CFONT_strikeout),
  GB_PROPERTY("Grade", "i", CFONT_grade),

  GB_METHOD("ToString", "s", CFONT_to_string, NULL),

  GB_METHOD("Width", "i", CFONT_width, "(Text)s"),
  GB_METHOD("Height", "i", CFONT_height, "(Text)s"),

  GB_STATIC_METHOD("_get", "Font", CFONT_get, "(Font)s"),
  #if 0
  GB_STATIC_PROPERTY("Resolution", "i", CFONT_resolution),
  #endif
  GB_PROPERTY_READ("Ascent", "i", CFONT_ascent),
  GB_PROPERTY_READ("Descent", "i", CFONT_descent),

  GB_PROPERTY_READ("Fixed", "b", CFONT_fixed),
  GB_PROPERTY_READ("Scalable", "b", CFONT_scalable),
  GB_PROPERTY_READ("Styles", "String[]", CFONT_styles),

  GB_END_DECLARE
};


