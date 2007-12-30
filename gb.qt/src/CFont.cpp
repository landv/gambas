/***************************************************************************

  CFont.cpp

  (c) 2000-2006 Benoît Minisini <gambas@users.sourceforge.net>

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


#include "gambas.h"
#include "main.h"

#include <math.h>
#include <qapplication.h>
#include <qstringlist.h>
#include <qfontmetrics.h>
#include <qfontdatabase.h>

#include "CWidget.h"
#include "CDraw.h"
#include "CFont.h"

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


CFONT *CFONT_create(const QFont &font, int type)
{
  CFONT *_object;

  GB.New((void **)&_object, CLASS_Font, NULL, NULL);
  *(THIS->font) = font;
  THIS->control = (CWIDGET *)type;

  return THIS;
}

CFONT *CFONT_create_control(CWIDGET *control)
{
  CFONT *_object;

  GB.New((void **)&_object, CLASS_Font, NULL, NULL);
  *(THIS->font) = ((CWIDGET *)control)->widget->font();
  THIS->control = (CWIDGET *)control;
  GB.Ref(control);

  return THIS;
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
  QFont f(qApp->font());

  // (!) Remove this test later, it is for backward compatibility

  if (str.length())
  {
    list = QStringList::split(",", str);

    f.setBold(false);
    f.setItalic(false);
    f.setUnderline(false);
    f.setStrikeOut(false);

    for (QStringList::Iterator it = list.begin(); it != list.end(); ++it )
    {
      elt = (*it);
      flag = elt.upper();
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
        f.setPointSizeFloat((int)(powf(qApp->font().pointSizeFloat(), 1.0 + ((int)size / 10.0)) + 0.5));
      }
      else if (number && size > 0.0)
        f.setPointSizeFloat(SIZE_VIRTUAL_TO_REAL(size));
      else
      {
        if (!name.isEmpty())
          name += ' ';
        name += elt;
      }
    }

    if (name.length())
      f.setFamily(name);
  }

  *(THIS->font) = f;
}


BEGIN_METHOD_VOID(CFONT_init)

  CLASS_Font = GB.FindClass("Font");

END_METHOD


BEGIN_METHOD_VOID(CFONT_exit)

  exit_font_database();

END_METHOD


BEGIN_METHOD(CFONT_new, GB_STRING font)

  QString s;

  THIS->font = new QFont;

  if (!MISSING(font))
    QString s = QSTRING_ARG(font);

  set_font_from_string(THIS, s);

END_METHOD


BEGIN_METHOD_VOID(CFONT_free)

  if ((ulong)THIS->control > 10)
    GB.Unref((void **)&THIS->control);
  delete THIS->font;

END_METHOD


static void CFONT_manage(int prop, CFONT *_object, void *_param)
{
  bool noResize = false;
  QFont *f = THIS->font;

  noResize = ((long)THIS->control == CFONT_DRAW && !DRAW_must_resize_font());

  if (READ_PROPERTY)
  {
    switch(prop)
    {
      case CFONT::Name: GB.ReturnNewZeroString(f->family()); break;
      case CFONT::Size:
        if (noResize)
          GB.ReturnFloat(f->pointSizeFloat());
        else
          GB.ReturnFloat(SIZE_REAL_TO_VIRTUAL(f->pointSizeFloat()));
        break;
      case CFONT::Grade:
        {
          float r = logf(f->pointSizeFloat()) / logf(qApp->font().pointSizeFloat());
          GB.ReturnInteger((int)(10 * r + 0.5) - 10);
        }
        break;
      case CFONT::Bold: GB.ReturnBoolean(f->bold()); break;
      case CFONT::Italic: GB.ReturnBoolean(f->italic()); break;
      case CFONT::Underline: GB.ReturnBoolean(f->underline()); break;
      case CFONT::StrikeOut: GB.ReturnBoolean(f->strikeOut()); break;
    }
  }
  else
  {
    switch (prop)
    {
      case CFONT::Name: f->setFamily(GB.ToZeroString(PROP(GB_STRING))); break;
      case CFONT::Size:
        if (noResize)
          f->setPointSizeFloat(VPROP(GB_FLOAT));
        else
          f->setPointSizeFloat(SIZE_VIRTUAL_TO_REAL(VPROP(GB_FLOAT)));
        break;
      case CFONT::Grade:
        {
          int g = VPROP(GB_INTEGER);
          if (g < -9)
            g = -9;
          else if (g > 9)
            g = 9;
          f->setPointSizeFloat((int)(powf(qApp->font().pointSizeFloat(), 1.0 + ((int)g / 10.0)) + 0.5));
        }
        break;
      case CFONT::Bold: f->setBold(VPROP(GB_BOOLEAN)); break;
      case CFONT::Italic: f->setItalic(VPROP(GB_BOOLEAN)); break;
      case CFONT::Underline: f->setUnderline(VPROP(GB_BOOLEAN)); break;
      case CFONT::StrikeOut: f->setStrikeOut(VPROP(GB_BOOLEAN)); break;
    }

    if (THIS->control)
    {
      if ((long)THIS->control == CFONT_APPLICATION)
        qApp->setFont(*f);
      else if ((long)THIS->control == CFONT_DRAW)
        DRAW_set_font(*f);
      else if (THIS->control->widget)
      {
        // THIS->control->widget->setFont(*f); - Not needed anymore
        // TODO Make a Gambas API to call SetProperty faster

      	GB_FUNCTION func;

        GB.GetFunction(&func, GB.FindClass("Object"), "SetProperty", NULL, NULL);
        GB.Push(3,
        	GB_T_OBJECT, THIS->control,
          GB_T_STRING, "Font", 4,
          GB_T_OBJECT, THIS
				);
				GB.Call(&func, 3, TRUE);
			}
    }
  }
}


BEGIN_PROPERTY(CFONT_name)

  CFONT_manage(CFONT::Name, OBJECT(CFONT), _param);

END_PROPERTY

BEGIN_PROPERTY(CFONT_size)

  CFONT_manage(CFONT::Size, OBJECT(CFONT), _param);

END_PROPERTY

BEGIN_PROPERTY(CFONT_grade)

  CFONT_manage(CFONT::Grade, OBJECT(CFONT), _param);

END_PROPERTY

BEGIN_PROPERTY(CFONT_bold)

  CFONT_manage(CFONT::Bold, OBJECT(CFONT), _param);

END_PROPERTY

BEGIN_PROPERTY(CFONT_italic)

  CFONT_manage(CFONT::Italic, OBJECT(CFONT), _param);

END_PROPERTY

BEGIN_PROPERTY(CFONT_underline)

  CFONT_manage(CFONT::Underline, OBJECT(CFONT), _param);

END_PROPERTY

BEGIN_PROPERTY(CFONT_strikeout)

  CFONT_manage(CFONT::StrikeOut, OBJECT(CFONT), _param);

END_PROPERTY


static void add(QString &str, const QString& data)
{
  if (str.length())
    str += ',';

  str += data;
}

BEGIN_METHOD_VOID(CFONT_to_string)

  QFont *f = THIS->font;
  QString str;
  double size;

  //str = qfont.family().left(1).upper() + qfont.family().mid(1).lower() + " " + QString::number(qfont.pointSize());
  add(str, f->family());
  size = SIZE_REAL_TO_VIRTUAL(f->pointSizeFloat());
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


BEGIN_METHOD(CFONT_get, GB_STRING str)

  CFONT *font;
  QString s = QSTRING_ARG(str);

  //qDebug(">> CFONT_get: %s", s.latin1());

  GB.New((void **)&font, CLASS_Font, NULL, NULL);
  set_font_from_string(font, s);

  GB.ReturnObject(font);

  //qDebug("<< CFONT_get");

END_METHOD


BEGIN_METHOD(CFONT_width, GB_STRING text)

  QFontMetrics fm(*(THIS->font));
  QStringList sl;
  int w, width = 0;
  int i;

  QString str;

  sl = QStringList::split('\n', QSTRING_ARG(text));

  for (i = 0; i < (int)sl.count(); i++)
  {
    w = fm.width(sl[i]);
    if (w > width) width = w;
  }

  GB.ReturnInteger(width);

END_METHOD


BEGIN_PROPERTY(CFONT_ascent)

  QFontMetrics fm(*(THIS->font));

  GB.ReturnInteger(fm.ascent());

END_PROPERTY


BEGIN_PROPERTY(CFONT_descent)

  QFontMetrics fm(*(THIS->font));

  GB.ReturnInteger(fm.descent());

END_PROPERTY


BEGIN_METHOD(CFONT_height, GB_STRING text)

  QFontMetrics fm(*(THIS->font));
  QString s;
  int nl;

  s = QSTRING_ARG(text);
  nl = s.contains('\n');

  GB.ReturnInteger(fm.height() * (1 + nl) + fm.leading() * nl);

END_METHOD


#ifdef USE_DPI
BEGIN_PROPERTY(CFONT_resolution)

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

BEGIN_METHOD_VOID(CFONTS_next)

  QString s;
  uint *index = (uint *)GB.GetEnum();

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


BEGIN_PROPERTY(CFONTS_count)

  init_font_database();
  GB.ReturnBoolean(_families.count());

END_PROPERTY


BEGIN_PROPERTY(CFONT_fixed)

  init_font_database();
  GB.ReturnBoolean(_info->isFixedPitch(THIS->font->family()));

END_PROPERTY


BEGIN_PROPERTY(CFONT_scalable)

  init_font_database();
  GB.ReturnBoolean(_info->isSmoothlyScalable(THIS->font->family()));

END_PROPERTY


BEGIN_PROPERTY(CFONT_styles)

  QStringList styles;
  GB_ARRAY array;
  uint i;

  init_font_database();
  styles = _info->styles(THIS->font->family());

  GB.Array.New(&array, GB_T_STRING, styles.count());
  for (i = 0; i < styles.count(); i++)
    GB.NewString((char **)GB.Array.Get(array, i), TO_UTF8(styles[i]), 0);

  GB.ReturnObject(array);

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
  //GB_NOT_CREATABLE(),

  GB_STATIC_METHOD("_init", NULL, CFONT_init, NULL),
  GB_STATIC_METHOD("_exit", NULL, CFONT_exit, NULL),
  GB_METHOD("_new", NULL, CFONT_new, "[(Font)s]"),
  GB_METHOD("_free", NULL, CFONT_free, NULL),

  GB_PROPERTY("Name", "s", CFONT_name),
  GB_PROPERTY("Size", "f", CFONT_size),
  GB_PROPERTY("Grade", "i", CFONT_grade),
  GB_PROPERTY("Bold", "b", CFONT_bold),
  GB_PROPERTY("Italic", "b", CFONT_italic),
  GB_PROPERTY("Underline", "b", CFONT_underline),
  GB_PROPERTY("StrikeOut", "b", CFONT_strikeout),

  GB_METHOD("ToString", "s", CFONT_to_string, NULL),

  GB_METHOD("Width", "i", CFONT_width, "(Text)s"),
  GB_METHOD("Height", "i", CFONT_height, "(Text)s"),

  GB_STATIC_METHOD("_get", "Font", CFONT_get, "(Font)s"),

  #ifdef USE_DPI
  GB_STATIC_PROPERTY("Resolution", "i", CFONT_resolution),
  #endif

  GB_PROPERTY_READ("Ascent", "i", CFONT_ascent),
  GB_PROPERTY_READ("Descent", "i", CFONT_descent),

  GB_PROPERTY_READ("Fixed", "b", CFONT_fixed),
  GB_PROPERTY_READ("Scalable", "b", CFONT_scalable),
  GB_PROPERTY_READ("Styles", "String[]", CFONT_styles),

  GB_END_DECLARE
};


