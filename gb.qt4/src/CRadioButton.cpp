/***************************************************************************

  CRadioButton.cpp

  The RadioButton class

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __CRADIOBUTTON_CPP

#include <qapplication.h>
#include <qradiobutton.h>
#include <qobject.h>

#include "gambas.h"

#include "CRadioButton.h"


DECLARE_EVENT(EVENT_Click);


BEGIN_METHOD(CRADIOBUTTON_new, GB_OBJECT parent)

  QRadioButton *wid = new QRadioButton(QCONTAINER(VARG(parent)));

  QObject::connect(wid, SIGNAL(toggled(bool)), &CRadioButton::manager, SLOT(clicked(bool)));

  CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_PROPERTY(CRADIOBUTTON_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->text()));
  else
    WIDGET->setText(QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(CRADIOBUTTON_value)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isChecked());
  else
    WIDGET->setChecked(VPROP(GB_BOOLEAN));

END_PROPERTY


/* Class CCheckBox */

CRadioButton CRadioButton::manager;

void CRadioButton::clicked(bool on)
{
  QRadioButton *wid = (QRadioButton *)sender();
  GET_SENDER();
  QObject *parent = wid->parent();

  QList<QRadioButton *> list = parent->findChildren<QRadioButton *>();
  int i;
  QRadioButton *obj = 0;

  if (on)
  {
  	for (i = 0; i < list.count(); i++)
  	{
  		obj = list.at(i);
      if (obj != wid && obj->isChecked())
      {
        obj->setChecked(false);
        on = false;
      }
    }

    //if (!on)
      GB.Raise(THIS, EVENT_Click, 0);
  }
  else
  {
  	for (i = 0; i < list.count(); i++)
  	{
  		obj = list.at(i);
      if (obj->isChecked())
        break;
    }

    if (!obj)
      wid->setChecked(true);
    //else
    //  RAISE_EVENT(EVENT_Click);
  }
}

GB_DESC CRadioButtonDesc[] =
{
  GB_DECLARE("RadioButton", sizeof(CRADIOBUTTON)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CRADIOBUTTON_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", CRADIOBUTTON_text),
  GB_PROPERTY("Caption", "s", CRADIOBUTTON_text),

  GB_PROPERTY("Value", "b", CRADIOBUTTON_value),

  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  
  RADIOBUTTON_DESCRIPTION,

  GB_END_DECLARE
};


