/***************************************************************************

	CFont.cpp

	(c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#define __CFONT_CPP

#include <math.h>
#include "CFont.h"
#include "gdesktop.h"
#include "ggambastag.h"

#include "gb.form.font.h"

CFONT *CFONT_create(gFont *font, FONT_FUNC func, void *object)
{
	CFONT *fnt;
	
	if (font && font->getTag())
		return (CFONT *)font->getTagValue();
	
	fnt = (CFONT *)GB.New(GB.FindClass("Font"), 0, 0);
	
	if (font)
	{
		fnt->font->unref();
		fnt->font = font;
		//font->ref();
		font->setTag(new gGambasTag((void *)fnt));
	}
	
	fnt->func = func;
	fnt->object = object;
	if (object)
		GB.Ref(object);
	
	return fnt;
}

BEGIN_METHOD(Font_new, GB_STRING font)

	if (!MISSING(font))
		FONT = new gFont(GB.ToZeroString(ARG(font)));
	else
		FONT = gDesktop::font()->copy();

END_METHOD


BEGIN_METHOD_VOID(Font_free)

	GB.Unref(POINTER(&THIS->object));
	gFont::assign(&THIS->font);

END_METHOD

static void CFONT_manage(int prop, CFONT *_object, void *_param)
{
	gFont *f = FONT;
	double size;
	
	if (!f)
	{
		THIS->font = ((CWIDGET *)THIS->object)->widget->font()->copy();
		f = FONT;
	}
	
	if (READ_PROPERTY)
	{
		switch(prop)
		{
			case CFONT::Name: GB.ReturnNewZeroString(f->name()); break;
			case CFONT::Size: GB.ReturnFloat(f->size()); break;
			case CFONT::Grade: GB.ReturnInteger(f->grade()); break;
			case CFONT::Bold: GB.ReturnBoolean(f->bold()); break;
			case CFONT::Italic: GB.ReturnBoolean(f->italic()); break;
			case CFONT::Underline: GB.ReturnBoolean(f->underline()); break;
			case CFONT::Strikeout: GB.ReturnBoolean(f->strikeout()); break;
		}
	}
	else
	{
		switch (prop)
		{
			case CFONT::Name: f->setName(GB.ToZeroString(PROP(GB_STRING))); break;
			case CFONT::Size: 
				size = VPROP(GB_FLOAT);
				if (size <= 0)
				{
					GB.Error("Bad font size");
					return;
				}
				f->setSize(VPROP(GB_FLOAT)); 
				break;
			case CFONT::Grade: f->setGrade(VPROP(GB_INTEGER)); break;
			case CFONT::Bold: f->setBold(VPROP(GB_BOOLEAN)); break;
			case CFONT::Italic: f->setItalic(VPROP(GB_BOOLEAN)); break;
			case CFONT::Underline: f->setUnderline(VPROP(GB_BOOLEAN)); break;
			case CFONT::Strikeout: f->setStrikeout(VPROP(GB_BOOLEAN)); break;
		}

		if (THIS->func)
			(*(THIS->func))(f, THIS->object);
		else if (THIS->object)
		{
			// THIS->control->widget->setFont(*f); - Not needed anymore
			// TODO Make a Gambas API to call SetProperty faster
	
			//fprintf(stderr, "applying font to (%s %p)\n", GB.GetClassName(THIS->object), THIS->object);
	
			GB_FUNCTION func;
	
			GB.GetFunction(&func, (void *)GB.FindClass("Object"), "SetProperty", NULL, NULL);
			GB.Push(3,
				GB_T_OBJECT, THIS->object,
				GB_T_STRING, "Font", 4,
				GB_T_OBJECT, THIS
			);
			GB.Call(&func, 3, TRUE);
		}
	}
}


BEGIN_PROPERTY(Font_Name)

	CFONT_manage(CFONT::Name, OBJECT(CFONT), _param);

END_PROPERTY

BEGIN_PROPERTY(Font_Size)

	CFONT_manage(CFONT::Size, OBJECT(CFONT), _param);

END_PROPERTY

BEGIN_PROPERTY(Font_Grade)

	CFONT_manage(CFONT::Grade, OBJECT(CFONT), _param);

END_PROPERTY

BEGIN_PROPERTY(Font_Bold)

	CFONT_manage(CFONT::Bold, OBJECT(CFONT), _param);

END_PROPERTY

BEGIN_PROPERTY(Font_Italic)

	CFONT_manage(CFONT::Italic, OBJECT(CFONT), _param);

END_PROPERTY

BEGIN_PROPERTY(Font_Underline)

	CFONT_manage(CFONT::Underline, OBJECT(CFONT), _param);

END_PROPERTY

BEGIN_PROPERTY(Font_Strikeout)

	CFONT_manage(CFONT::Strikeout, OBJECT(CFONT), _param);

END_PROPERTY


BEGIN_METHOD_VOID(Font_ToString)

	GB.ReturnNewZeroString(FONT->toString());
	
END_METHOD


BEGIN_METHOD(Font_get, GB_STRING str)

	CFONT *font;
	gFont *fnt;
	
	fnt = new gFont(GB.ToZeroString(ARG(str)));
	font = CFONT_create(fnt);
	//gFont::assign(&fnt);
	
	GB.ReturnObject(font);
	
END_METHOD

BEGIN_METHOD_VOID(Font_Copy)

	GB.ReturnObject(CFONT_create(FONT->copy()));

END_METHOD


BEGIN_PROPERTY(Font_Ascent)

	GB.ReturnInteger(FONT->ascent());

END_PROPERTY


BEGIN_PROPERTY(Font_Descent)

	GB.ReturnInteger(FONT->descent());

END_PROPERTY


BEGIN_PROPERTY(Font_Height)

	GB.ReturnInteger(FONT->height());

END_PROPERTY


BEGIN_METHOD(Font_TextWidth, GB_STRING text)

	GB.ReturnInteger(FONT->width(STRING(text), LENGTH(text)));

END_METHOD


BEGIN_METHOD(Font_TextHeight, GB_STRING text)

	GB.ReturnInteger(FONT->height(STRING(text), LENGTH(text)));

END_METHOD


BEGIN_METHOD(Font_RichTextWidth, GB_STRING text)

	float w;
	
	FONT->richTextSize(STRING(text), LENGTH(text), -1, &w, NULL);
	GB.ReturnInteger(ceil(w));

END_METHOD


BEGIN_METHOD(Font_RichTextHeight, GB_STRING text; GB_INTEGER width)

	float h;
	
	FONT->richTextSize(STRING(text), LENGTH(text), VARGOPT(width, -1), NULL, &h);
	GB.ReturnInteger(ceil(h));

END_METHOD


BEGIN_PROPERTY(Font_Fixed)

	GB.ReturnBoolean(FONT->fixed());

END_PROPERTY


BEGIN_METHOD_VOID(Fonts_next)

	int *pos;
	
	pos = (int *)GB.GetEnum();
	
	if (pos[0] >= gFont::count())
	{
		GB.StopEnum ();
		return;
	}
	
	GB.ReturnNewZeroString(gFont::familyItem(pos[0]++));
	
END_METHOD


BEGIN_METHOD(Fonts_Exist, GB_STRING family)

	int i;
	char *family = GB.ToZeroString(ARG(family));
	
	for (i = 0; i < gFont::count(); i++)
	{
		if (strcmp(gFont::familyItem(i), family) == 0)
		{
			GB.ReturnBoolean(TRUE);
			return;
		}
	}
	
	GB.ReturnBoolean(FALSE);

END_METHOD


BEGIN_PROPERTY(Fonts_Count)
	
	GB.ReturnInteger(gFont::count());

END_PROPERTY


BEGIN_PROPERTY(Font_Styles)

	stub("Font_Styles");

END_PROPERTY


BEGIN_PROPERTY(Font_Scalable)

	GB.ReturnBoolean(FONT->scalable());

END_PROPERTY

#if 0
BEGIN_PROPERTY(Font_Underline)

	if (READ_PROPERTY) { GB.ReturnBoolean(FONT->underline()); return; }
	FONT->setUnderline(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(Font_Strikeout)

	if (READ_PROPERTY) { GB.ReturnBoolean(FONT->strikeOut()); return; }
	FONT->setStrikeOut(VPROP(GB_BOOLEAN));

END_PROPERTY
#endif

GB_DESC CFontsDesc[] =
{
	GB_DECLARE("Fonts", 0), 
	GB_NOT_CREATABLE(),

	GB_STATIC_METHOD("_next", "s", Fonts_next, NULL),
	GB_STATIC_METHOD("Exist", "b", Fonts_Exist, "(Family)s"),
	GB_STATIC_PROPERTY_READ("Count", "i", Fonts_Count),
	
	GB_END_DECLARE
};


GB_DESC CFontDesc[] =
{
	GB_DECLARE("Font", sizeof(CFONT)), 

	GB_METHOD("_new", 0, Font_new, "[(Font)s]"),
	GB_METHOD("_free", 0, Font_free, 0),
	GB_METHOD("Copy", "Font", Font_Copy, NULL),
	
	GB_PROPERTY("Name", "s", Font_Name),
	GB_PROPERTY("Size", "f", Font_Size),
	GB_PROPERTY("Bold", "b", Font_Bold),
	GB_PROPERTY("Italic", "b", Font_Italic),
	GB_PROPERTY("Underline", "b", Font_Underline),
	GB_PROPERTY("Strikeout", "b", Font_Strikeout),
	GB_PROPERTY("Grade", "i", Font_Grade),

	GB_METHOD("ToString", "s", Font_ToString, 0),

	GB_METHOD("TextWidth", "i", Font_TextWidth, "(Text)s"),
	GB_METHOD("TextHeight", "i", Font_TextHeight, "(Text)s"),

	GB_METHOD("RichTextWidth", "i", Font_RichTextWidth, "(Text)s"),
	GB_METHOD("RichTextHeight", "i", Font_RichTextHeight, "(Text)s[(Width)i]"),

	GB_STATIC_METHOD("_get", "Font", Font_get, "(Font)s"),
	#if 0
	GB_STATIC_PROPERTY("Resolution", "i", CFONT_resolution),
	#endif
	GB_PROPERTY_READ("Ascent", "i", Font_Ascent),
	GB_PROPERTY_READ("Descent", "i", Font_Descent),
	GB_PROPERTY_READ("Height", "i", Font_Height),

	GB_PROPERTY_READ("Fixed", "b", Font_Fixed),
	GB_PROPERTY_READ("Scalable", "b", Font_Scalable),
	GB_PROPERTY_READ("Styles", "String[]", Font_Styles),

	GB_END_DECLARE
};


