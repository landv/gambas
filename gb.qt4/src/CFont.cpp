/***************************************************************************

	CFont.cpp

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

#define __CFONT_CPP

#include "gambas.h"
#include "main.h"

#include <math.h>

#include <QApplication>
#include <QStringList>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QTextDocument>

#include "CWidget.h"
#include "CDraw.h"
#include "CFont.h"

#include "gb.form.font.h"

#ifdef USE_DPI
int CFONT_dpi = 96;
#endif

static GB_CLASS CLASS_Font;

static QStringList _families;
static QFontDatabase *_info = 0;

static void init_font_database()
{
	if (_info)
		return;

	_info = new QFontDatabase();
	_families = _info->families();
}

static void exit_font_database()
{
	if (_info)
		delete _info;
}


CFONT *CFONT_create(const QFont &font, FONT_FUNC func, void *object)
{
	CFONT *_object = (CFONT *)GB.New(CLASS_Font, NULL, NULL);
	
	*(THIS->font) = font;
	THIS->func = func;
	THIS->object = object;
	if (object)
		GB.Ref(object);

	return THIS;
}

void CFONT_set(FONT_FUNC func, void *font, void *object)
{
	if (!font)
	{
		QFont f;
		(*func)(f, object);
	}
	else
		(*func)(*(((CFONT *)font)->font), object);
}


double CFONT_size_real_to_virtual(double size)
{
	#ifdef USE_DPI
	return size * (double)QPaintDevice::x11AppDpiY() / CFONT_dpi;
	#else
	return size;
	#endif
}

double CFONT_size_virtual_to_real(double size)
{
	#ifdef USE_DPI
	return size * CFONT_dpi / (double)QPaintDevice::x11AppDpiY();
	#else
	return size;
	#endif
}


static void set_font_from_string(CFONT *_object, QString &str)
{
	QStringList list;
	QString name, elt, flag;
	double size = 0;
	bool number;
	QFont f;

	// (!) Remove this test later, it is for backward compatibility

	if (str.length())
	{
		list = str.split(",");

		for (QStringList::Iterator it = list.begin(); it != list.end(); ++it )
		{
			elt = (*it);
			elt = elt.trimmed();
			flag = elt.toUpper();
			size = elt.toDouble(&number);

			if (flag == "BOLD")
				f.setBold(true);
			else if (flag == "ITALIC")
				f.setItalic(true);
			else if (flag == "UNDERLINE")
				f.setUnderline(true);
			else if (flag == "STRIKEOUT")
				f.setStrikeOut(true);
			else if (flag[0] == '+' || flag[0] == '-' || flag[0] == '0')
			{
				//f.setPointSizeFloat((int)(powf(qApp->font().pointSizeFloat(), 1.0 + ((int)size / 10.0)) + 0.5));
				f.setPointSizeF(GRADE_TO_SIZE(size, qApp->font().pointSizeF()));
			}
			else if (number && size > 0.0)
				f.setPointSizeF(SIZE_VIRTUAL_TO_REAL(size));
			else if (elt.length())
			{
				f.setBold(false);
				f.setItalic(false);
				f.setUnderline(false);
				f.setStrikeOut(false);
				f.setFamily(elt);
			}
		}
	}

	*(THIS->font) = f;
}


BEGIN_METHOD_VOID(Font_init)

	CLASS_Font = GB.FindClass("Font");

END_METHOD


BEGIN_METHOD_VOID(Font_exit)

	exit_font_database();

END_METHOD


BEGIN_METHOD(Font_new, GB_STRING font)

	QString s;

	THIS->font = new QFont;
	THIS->func = 0;
	THIS->object = 0;

	if (!MISSING(font))
		QString s = QSTRING_ARG(font);

	set_font_from_string(THIS, s);

END_METHOD


BEGIN_METHOD_VOID(Font_free)

	if (THIS->object)
		GB.Unref(POINTER(&THIS->object));
	delete THIS->font;

END_METHOD


static void CFONT_manage(int prop, CFONT *_object, void *_param)
{
	bool noResize = false;
	QFont *f = THIS->font;
	double size;

	noResize = true; //((long)THIS->control == CFONT_DRAW && !DRAW_must_resize_font());

	if (READ_PROPERTY)
	{
		switch(prop)
		{
			case CFONT::Name: GB.ReturnNewZeroString(f->family().toUtf8()); break;
			case CFONT::Size:
				if (noResize)
					GB.ReturnFloat(f->pointSizeF());
				else
					GB.ReturnFloat(SIZE_REAL_TO_VIRTUAL(f->pointSizeF()));
				break;
			case CFONT::Grade:
				/*{
					float r = logf(f->pointSizeFloat()) / logf(qApp->font().pointSizeFloat());
					GB.ReturnInteger((int)(10 * r + 0.5) - 10);
				}*/
				GB.ReturnInteger(SIZE_TO_GRADE(f->pointSizeF(), qApp->font().pointSizeF()));
				break;
			case CFONT::Bold: GB.ReturnBoolean(f->bold()); break;
			case CFONT::Italic: GB.ReturnBoolean(f->italic()); break;
			case CFONT::Underline: GB.ReturnBoolean(f->underline()); break;
			case CFONT::Strikeout: GB.ReturnBoolean(f->strikeOut()); break;
		}
	}
	else
	{
		switch (prop)
		{
			case CFONT::Name: f->setFamily(GB.ToZeroString(PROP(GB_STRING))); break;
			case CFONT::Size:
				if (noResize)
					size = VPROP(GB_FLOAT);
				else
					size = SIZE_VIRTUAL_TO_REAL(VPROP(GB_FLOAT));
				
				if (size <= 0)
				{
					GB.Error("Bad font size");
					return;
				}
				
				f->setPointSizeF(size);
				break;
			case CFONT::Grade:
				{
					int g = VPROP(GB_INTEGER);
					if (g < -4)
						g = -4;
					else if (g > 24)
						g = 24;
					//f->setPointSizeFloat((int)(powf(qApp->font().pointSizeFloat(), 1.0 + ((int)g / 16.0)) + 0.5));
					f->setPointSizeF(GRADE_TO_SIZE(g, qApp->font().pointSizeF()));
				}
				break;
			case CFONT::Bold: f->setBold(VPROP(GB_BOOLEAN)); break;
			case CFONT::Italic: f->setItalic(VPROP(GB_BOOLEAN)); break;
			case CFONT::Underline: f->setUnderline(VPROP(GB_BOOLEAN)); break;
			case CFONT::Strikeout: f->setStrikeOut(VPROP(GB_BOOLEAN)); break;
		}

		if (THIS->func)
			(*(THIS->func))(*f, THIS->object);
		else if (THIS->object)
		{
			GB_VALUE value;
			value.type = GB_T_OBJECT;
			value._object.value = THIS;
			GB.SetProperty(THIS->object, "Font", &value);
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


static void add(QString &str, const QString& data)
{
	if (str.length())
		str += ',';

	str += data;
}

BEGIN_METHOD_VOID(Font_ToString)

	QFont *f = THIS->font;
	QString str;
	double size;

	//str = qfont.family().left(1).upper() + qfont.family().mid(1).lower() + " " + QString::number(qfont.pointSize());
	add(str, f->family());
	size = SIZE_REAL_TO_VIRTUAL(f->pointSizeF());
	size = (double)((int)(size * 10 + 0.5)) / 10;
	add(str, QString::number(size));
	if (f->bold())
		add(str, "Bold");
	if (f->italic())
		add(str, "Italic");
	if (f->underline())
		add(str, "Underline");
	if (f->strikeOut())
		add(str, "StrikeOut");

	GB.ReturnNewZeroString(TO_UTF8(str));

END_METHOD


BEGIN_METHOD(Font_get, GB_STRING str)

	CFONT *font;
	QString s = QSTRING_ARG(str);

	//qDebug(">> Font_get: %s", s.latin1());

	font = (CFONT *)GB.New(CLASS_Font, NULL, NULL);
	set_font_from_string(font, s);

	GB.ReturnObject(font);

	//qDebug("<< Font_get");

END_METHOD

BEGIN_METHOD_VOID(Font_Copy)

	GB.ReturnObject(CFONT_create(*THIS->font));

END_METHOD


BEGIN_PROPERTY(Font_Ascent)

	QFontMetrics fm(*(THIS->font));

	GB.ReturnInteger(fm.ascent());

END_PROPERTY


BEGIN_PROPERTY(Font_Descent)

	QFontMetrics fm(*(THIS->font));

	GB.ReturnInteger(fm.descent());

END_PROPERTY


BEGIN_PROPERTY(Font_Height)

	QFontMetrics fm(*(THIS->font));

	GB.ReturnInteger(fm.height() + fm.leading());

END_PROPERTY


BEGIN_METHOD(Font_TextHeight, GB_STRING text)

	QFontMetrics fm(*(THIS->font));
	QString s;
	int nl;

	if (!MISSING(text))
		s = QSTRING_ARG(text);
	nl = s.count('\n');

	GB.ReturnInteger(fm.height() * (1 + nl) + fm.leading() * nl);

END_METHOD


BEGIN_METHOD(Font_TextWidth, GB_STRING text)

	QFontMetrics fm(*(THIS->font));
	QStringList sl;
	int w, width = 0;
	int i;

	QString str = QSTRING_ARG(text);

	sl = str.split('\n');

	for (i = 0; i < (int)sl.count(); i++)
	{
		w = fm.width(sl[i]);
		if (w > width) width = w;
	}

	GB.ReturnInteger(width);

END_METHOD


static void rich_text_size(CFONT *_object, char *text, int len, int sw, int *w, int *h)
{
	QTextDocument rt;
	
	rt.setDocumentMargin(0);
	rt.setHtml(QString::fromUtf8((const char *)text, len));
	rt.setDefaultFont(*(THIS->font));

	if (sw > 0)
		rt.setTextWidth(sw);
	
	if (w) *w = rt.idealWidth();
	if (h) *h = rt.size().height();
}


BEGIN_METHOD(Font_RichTextWidth, GB_STRING text)

	int w;
	
	rich_text_size(THIS, STRING(text), LENGTH(text), -1, &w, NULL);
	GB.ReturnInteger(w);

END_METHOD


BEGIN_METHOD(Font_RichTextHeight, GB_STRING text; GB_INTEGER width)

	int h;
	
	rich_text_size(THIS, STRING(text), LENGTH(text), VARGOPT(width, -1), NULL, &h);
	GB.ReturnInteger(h);

END_METHOD


#ifdef USE_DPI
BEGIN_PROPERTY(Font_Resolution)

	if (READ_PROPERTY)
		GB.ReturnInteger(CFONT_dpi);
	else
	{
		CFONT_dpi = VPROP(GB_INTEGER);
		if (CFONT_dpi < 1)
			CFONT_dpi = 96;
	}

END_PROPERTY
#endif

BEGIN_METHOD_VOID(Fonts_next)

	QString s;
	int *index = (int *)GB.GetEnum();

	if (*index == 0)
		init_font_database();

	if (*index >= _families.count())
		GB.StopEnum();
	else
	{
		s = _families[*index];
		GB.ReturnNewZeroString(TO_UTF8(s));
		(*index)++;
	}

END_METHOD


BEGIN_METHOD(Fonts_Exist, GB_STRING family)

	int i;
	char *family = GB.ToZeroString(ARG(family));
	
	init_font_database();
	
	for (i = 0; i < _families.count(); i++)
	{
		if (_families[i] == family)
		{
			GB.ReturnBoolean(true);
			return;
		}
	}

	GB.ReturnBoolean(false);

END_METHOD


BEGIN_PROPERTY(Fonts_Count)

	init_font_database();
	GB.ReturnInteger(_families.count());

END_PROPERTY


BEGIN_PROPERTY(Font_Fixed)

	init_font_database();
	GB.ReturnBoolean(_info->isFixedPitch(THIS->font->family()));

END_PROPERTY


BEGIN_PROPERTY(Font_Scalable)

	init_font_database();
	GB.ReturnBoolean(_info->isSmoothlyScalable(THIS->font->family()));

END_PROPERTY


BEGIN_PROPERTY(Font_Styles)

	QStringList styles;
	GB_ARRAY array;
	int i;

	init_font_database();
	styles = _info->styles(THIS->font->family());

	GB.Array.New(&array, GB_T_STRING, styles.count());
	for (i = 0; i < styles.count(); i++)
		*(char **)GB.Array.Get(array, i) = GB.NewZeroString(TO_UTF8(styles[i]));

	GB.ReturnObject(array);

END_PROPERTY


GB_DESC CFontsDesc[] =
{
	GB_DECLARE("Fonts", 0),
	GB_NOT_CREATABLE(),

	GB_STATIC_METHOD("Exist", "b", Fonts_Exist, "(Family)s"),
	GB_STATIC_METHOD("_next", "s", Fonts_next, NULL),
	GB_STATIC_PROPERTY_READ("Count", "i", Fonts_Count),

	GB_END_DECLARE
};


GB_DESC CFontDesc[] =
{
	GB_DECLARE("Font", sizeof(CFONT)),
	//GB_NOT_CREATABLE(),

	GB_STATIC_METHOD("_init", NULL, Font_init, NULL),
	GB_STATIC_METHOD("_exit", NULL, Font_exit, NULL),
	GB_METHOD("_new", NULL, Font_new, "[(Font)s]"),
	GB_METHOD("_free", NULL, Font_free, NULL),
	GB_METHOD("Copy", "Font", Font_Copy, NULL),

	GB_PROPERTY("Name", "s", Font_Name),
	GB_PROPERTY("Size", "f", Font_Size),
	GB_PROPERTY("Grade", "i", Font_Grade),
	GB_PROPERTY("Bold", "b", Font_Bold),
	GB_PROPERTY("Italic", "b", Font_Italic),
	GB_PROPERTY("Underline", "b", Font_Underline),
	GB_PROPERTY("Strikeout", "b", Font_Strikeout),

	GB_METHOD("ToString", "s", Font_ToString, NULL),

	GB_METHOD("TextWidth", "i", Font_TextWidth, "(Text)s"),
	GB_METHOD("TextHeight", "i", Font_TextHeight, "(Text)s"),

	GB_METHOD("RichTextWidth", "i", Font_RichTextWidth, "(Text)s"),
	GB_METHOD("RichTextHeight", "i", Font_RichTextHeight, "(Text)s[(Width)i]"),

	GB_STATIC_METHOD("_get", "Font", Font_get, "(Font)s"),

	#ifdef USE_DPI
	GB_STATIC_PROPERTY("Resolution", "i", Font_Resolution),
	#endif

	GB_PROPERTY_READ("Ascent", "i", Font_Ascent),
	GB_PROPERTY_READ("Descent", "i", Font_Descent),
	GB_PROPERTY_READ("Height", "i", Font_Height),

	GB_PROPERTY_READ("Fixed", "b", Font_Fixed),
	GB_PROPERTY_READ("Scalable", "b", Font_Scalable),
	GB_PROPERTY_READ("Styles", "String[]", Font_Styles),

	GB_END_DECLARE
};


