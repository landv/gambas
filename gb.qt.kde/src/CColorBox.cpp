/***************************************************************************

  CColorBox.cpp

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

#define __CCOLORBOX_CPP


#include <kcolorcombo.h>

#include "gambas.h"
#include "main.h"
#include "CColorBox.h"

DECLARE_EVENT(EVENT_Click);

/***************************************************************************

  class CColorBox

***************************************************************************/

CColorBox CColorBox::manager;

void CColorBox::clicked()
{
  void *object = QT.GetObject((QWidget *)sender());
  GB.Raise(object, EVENT_Click, 0);
}


/***************************************************************************

  ColorBox

***************************************************************************/

BEGIN_METHOD(CCOLORBOX_new, GB_OBJECT parent)

  KColorCombo *wid = new KColorCombo(QT.GetContainer(VARG(parent)));

  QT.InitWidget(wid, _object);

  QObject::connect(wid, SIGNAL(activated(const QColor &)), &CColorBox::manager, SLOT(clicked()));

  wid->show();

END_METHOD


BEGIN_PROPERTY(CCOLORBOX_color)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->color().rgb() & 0xFFFFFF);
  else
    WIDGET->setColor(QColor((QRgb)VPROP(GB_INTEGER)));

END_PROPERTY


GB_DESC CColorBoxDesc[] =
{
  GB_DECLARE("ColorBox", sizeof(CCOLORBOX)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CCOLORBOX_new, "(Parent)Container;"),

  GB_PROPERTY("Color", "i", CCOLORBOX_color),
  GB_PROPERTY("Value", "i", CCOLORBOX_color),
  //GB_PROPERTY("Value", "b", CBUTTON_value),

  GB_CONSTANT("_Properties", "s", CCOLORBOX_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),

  GB_EVENT("Click", NULL, NULL, &EVENT_Click),

  GB_END_DECLARE
};

