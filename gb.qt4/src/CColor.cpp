/***************************************************************************

  CColor.cpp

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

#define __CCOLOR_CPP

#include <qapplication.h>
#include <qcolor.h>
#include <qpalette.h>

#include "gambas.h"

#include "CWidget.h"
#include "CColor.h"

static int _h = 0;
static int _s = 0;
static int _v = 0;

QColor CCOLOR_merge(const QColor &colorA, const QColor &colorB, int factor)
{
	const int maxFactor = 100;
	QColor tmp = colorA;
	tmp.setRed((tmp.red() * factor) / maxFactor + (colorB.red() * (maxFactor - factor)) / maxFactor);
	tmp.setGreen((tmp.green() * factor) / maxFactor + (colorB.green() * (maxFactor - factor)) / maxFactor);
	tmp.setBlue((tmp.blue() * factor) / maxFactor + (colorB.blue() * (maxFactor - factor)) / maxFactor);
	return tmp;
}

static void get_hsv(int col)
{
  static int last = 0;

  if (last == col)
    return;

  QColor c(col);
  c.getHsv(&_h, &_s, &_v);
  if (_h < 0)
    _h = 0;
  last = col;
}


static void return_color(QPalette::ColorRole role)
{
  GB.ReturnInteger(QApplication::palette().color(role).rgb() & 0xFFFFFF);
}

BEGIN_PROPERTY(CCOLOR_background)

  return_color(QPalette::Window);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_foreground)

  return_color(QPalette::WindowText);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_text_background)

  return_color(QPalette::Base);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_text_foreground)

  return_color(QPalette::Text);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_selected_background)

  return_color(QPalette::Highlight);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_light_background)

	QColor col;
	int h, s, v;

	get_hsv(QApplication::palette().color(QPalette::Highlight).rgb() & 0xFFFFFF);
	h = _h; s = _s; v = _v;
	
	get_hsv(QApplication::palette().color(QPalette::Base).rgb() & 0xFFFFFF);
	
	col = QColor::fromHsv(h, (_s * 3 + s) / 4, (_v * 3 + v) / 4);
	
  GB.ReturnInteger((uint)col.rgb() & 0xFFFFFF);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_selected_foreground)

  return_color(QPalette::HighlightedText);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_button_background)

  return_color(QPalette::Button);

END_PROPERTY

BEGIN_PROPERTY(CCOLOR_button_foreground)

  return_color(QPalette::ButtonText);

END_PROPERTY

GB_DESC CColorDesc[] =
{
  GB_DECLARE("Color", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY("Background", "i", CCOLOR_background),
  GB_STATIC_PROPERTY("SelectedBackground", "i", CCOLOR_selected_background),
  GB_STATIC_PROPERTY("LightBackground", "i", CCOLOR_light_background),
  GB_STATIC_PROPERTY("TextBackground", "i", CCOLOR_text_background),
  GB_STATIC_PROPERTY("ButtonBackground", "i", CCOLOR_button_background),

  GB_STATIC_PROPERTY("Foreground", "i", CCOLOR_foreground),
  GB_STATIC_PROPERTY("SelectedForeground", "i", CCOLOR_selected_foreground),
  GB_STATIC_PROPERTY("TextForeground", "i", CCOLOR_text_foreground),
  GB_STATIC_PROPERTY("ButtonForeground", "i", CCOLOR_button_foreground),

  GB_END_DECLARE
};


