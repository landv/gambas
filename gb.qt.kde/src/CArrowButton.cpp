/***************************************************************************

  CArrowButton.cpp

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

#define __CARROWBUTTON_CPP


#include <karrowbutton.h>

#include "gambas.h"
#include "main.h"
#include "CArrowButton.h"

DECLARE_EVENT(EVENT_Click);

/***************************************************************************

  class CArrowButton

***************************************************************************/

CArrowButton CArrowButton::manager;

void CArrowButton::clicked()
{
  void *object = QT.GetObject((QWidget *)sender());
  GB.Raise(object, EVENT_Click, 0);
}


/***************************************************************************

  ArrowButton

***************************************************************************/

BEGIN_METHOD(CARROWBUTTON_new, GB_OBJECT parent)

  KArrowButton *wid = new KArrowButton(QT.GetContainer(VARG(parent)));

  QT.InitWidget(wid, _object);

  QObject::connect(wid, SIGNAL(clicked()), &CArrowButton::manager, SLOT(clicked()));

  wid->setArrowType(Qt::UpArrow);
  THIS->orient = Qt::UpArrow;
  wid->show();

END_METHOD


BEGIN_PROPERTY(CARROWBUTTON_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->text()));
  else
    WIDGET->setText(QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(CARROWBUTTON_orientation)

  if (READ_PROPERTY)
    GB.ReturnInteger(THIS->orient);
  else
  {
    THIS->orient = VPROP(GB_INTEGER);
    WIDGET->setArrowType((Qt::ArrowType)THIS->orient);
  }

END_PROPERTY


BEGIN_PROPERTY(CARROWBUTTON_border)

  if (READ_PROPERTY)
    GB.ReturnBoolean(!WIDGET->isFlat());
  else
    WIDGET->setFlat(!VPROP(GB_BOOLEAN));

END_PROPERTY



GB_DESC CArrowButtonDesc[] =
{
  GB_DECLARE("ArrowButton", sizeof(CArrowButton)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CARROWBUTTON_new, "(Parent)Container;"),

  GB_CONSTANT("Up", "i", Qt::UpArrow),
  GB_CONSTANT("Down", "i", Qt::DownArrow),
  GB_CONSTANT("Left", "i", Qt::LeftArrow),
  GB_CONSTANT("Right", "i", Qt::RightArrow),

  GB_PROPERTY("Text", "s", CARROWBUTTON_text),
  GB_PROPERTY("Caption", "s", CARROWBUTTON_text),
  //GB_PROPERTY("Picture", "Picture", CBUTTON_picture),
  //GB_PROPERTY("Flat", "b", CARROWBUTTON_flat),
  GB_PROPERTY("Border", "b", CARROWBUTTON_border),
  GB_PROPERTY("Orientation", "i<ArrowButton>", CARROWBUTTON_orientation),
  //GB_PROPERTY("Value", "b", CBUTTON_value),

  GB_CONSTANT("_Properties", "s", CARROWBUTTON_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),

  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  GB_END_DECLARE
};

