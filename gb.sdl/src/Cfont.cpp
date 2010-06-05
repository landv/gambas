/***************************************************************************

  Cfont.cpp

  (c) 2007 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#define __CFONT_CPP

#include "Cfont.h"

static StringList FontList;

static void init_font_list(void )
{
	if (FontList.empty())
		FontList = SDLfont::GetFontList();
}

BEGIN_METHOD_VOID(CFONTS_next)

	std::string s;

	uint *index = (uint *)GB.GetEnum();

	if (*index == 0)
		init_font_list();

	if (*index >= FontList.size())
		GB.StopEnum();
	else
	{
		s = FontList[*index];
		GB.ReturnNewZeroString(s.c_str());
		(*index)++;
	}

END_METHOD


BEGIN_PROPERTY(CFONTS_count)

	init_font_list();
	GB.ReturnInteger(FontList.size());

END_PROPERTY

BEGIN_METHOD(CFONT_load, GB_STRING path)

	CFONT *font;
	GB.New(POINTER(&font), CLASS_Font, NULL, NULL);
	font->font = new SDLfont(GB.RealFileName(STRING(path), LENGTH(path)));
	GB.ReturnObject(font);

END_METHOD

BEGIN_METHOD(CFONT_width, GB_STRING text)

	int width, height;
	FONT->SizeText(STRING(text), &width, &height);
	GB.ReturnInteger(width);

END_METHOD

BEGIN_METHOD(CFONT_height, GB_STRING text)

	int width, height;
	FONT->SizeText(STRING(text), &width, &height);
	GB.ReturnInteger(height);

END_METHOD

BEGIN_METHOD(CFONT_new, GB_STRING font)

	THIS->font = new SDLfont();

END_METHOD

BEGIN_METHOD_VOID(CFONT_free)

	if (FONT)
		delete (FONT);

END_METHOD

BEGIN_PROPERTY(CFONT_name)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(FONT->GetFontName());
	else
		FONT->SetFontName(GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

BEGIN_PROPERTY(CFONT_size)

	if (READ_PROPERTY)
		GB.ReturnInteger(FONT->GetFontSize());
	else
		FONT->SetFontSize(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CFONT_ascent)

	GB.ReturnInteger(FONT->GetFontAscent());

END_PROPERTY

BEGIN_PROPERTY(CFONT_descent)

	GB.ReturnInteger(FONT->GetFontDescent());

END_PROPERTY

BEGIN_PROPERTY(CFONT_fixed)

	GB.ReturnBoolean(FONT->IsFontFixed());

END_PROPERTY

BEGIN_PROPERTY(CFONT_scalable)

	GB.ReturnBoolean(FONT->IsFontScalable());

END_PROPERTY

GB_DESC CFonts[] =
{
  GB_DECLARE("Fonts", 0),
  GB_NOT_CREATABLE(),

  GB_STATIC_METHOD("_next", "s", CFONTS_next, NULL),
  GB_STATIC_PROPERTY_READ("Count", "i", CFONTS_count),

  GB_END_DECLARE
};

GB_DESC CFont[] =
{
  GB_DECLARE("Font", sizeof(CFONT)),

  GB_STATIC_METHOD("Load", "Font", CFONT_load, "(Path)s"),

  GB_METHOD("_new", NULL, CFONT_new, "[(Font)s]"),
  GB_METHOD("_free", NULL, CFONT_free, NULL),

  GB_PROPERTY("Name", "s", CFONT_name),
  GB_PROPERTY("Size", "i", CFONT_size),

  GB_PROPERTY_READ("Ascent", "i", CFONT_ascent),
  GB_PROPERTY_READ("Descent", "i", CFONT_descent),
  GB_PROPERTY_READ("Fixed", "b", CFONT_fixed),
  GB_PROPERTY_READ("Scalable", "b", CFONT_scalable),

  GB_METHOD("Width", "i", CFONT_width, "(Text)s"),
  GB_METHOD("Height", "i", CFONT_height, "(Text)s"),

/*
  GB_STATIC_METHOD("_init", NULL, CFONT_init, NULL),
  GB_STATIC_METHOD("_exit", NULL, CFONT_exit, NULL),

  GB_PROPERTY("Grade", "i", CFONT_grade),
  GB_PROPERTY("Bold", "b", CFONT_bold),
  GB_PROPERTY("Italic", "b", CFONT_italic),
  GB_PROPERTY("Underline", "b", CFONT_underline),
  GB_PROPERTY("StrikeOut", "b", CFONT_strikeout),

  GB_METHOD("ToString", "s", CFONT_to_string, NULL),
  GB_STATIC_METHOD("_get", "Font", CFONT_get, "(Font)s"),

  GB_PROPERTY_READ("Styles", "String[]", CFONT_styles),
*/
  GB_END_DECLARE
};

