/***************************************************************************

  CCheckBox.cpp

  The CheckBox class

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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

#define __CCHECKBOX_CPP

#include <qapplication.h>
#include <qcheckbox.h>

#include "gambas.h"

#include "CCheckBox.h"


DECLARE_EVENT(EVENT_Click);


BEGIN_METHOD(CCHECKBOX_new, GB_OBJECT parent)

  QCheckBox *wid = new QCheckBox(QCONTAINER(VARG(parent)));

  QObject::connect(wid, SIGNAL(stateChanged(int)), &CCheckBox::manager, SLOT(clicked()));

  CWIDGET_new(wid, (void *)_object, "CheckBox");

  wid->show();

END_METHOD


BEGIN_PROPERTY(CCHECKBOX_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->text()));
  else
    WIDGET->setText(QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(CCHECKBOX_value)

  /*if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isChecked());
  else
    WIDGET->setChecked(VPROP(GB_BOOLEAN));*/

  if (READ_PROPERTY)
  {
		switch(WIDGET->state())
		{
			case QButton::Off: GB.ReturnInteger(0); break;
			case QButton::On: GB.ReturnInteger(-1); break;
			case QButton::NoChange: GB.ReturnInteger(1); break;
		}
	}
  else
  {
  	if (WIDGET->isTristate() && VPROP(GB_INTEGER) == 1)
  		WIDGET->setNoChange();
		else
	    WIDGET->setChecked(VPROP(GB_INTEGER));
	}

END_PROPERTY


BEGIN_PROPERTY(CCHECKBOX_tristate)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isTristate());
  else
    WIDGET->setTristate(VPROP(GB_BOOLEAN));

END_PROPERTY


/* Class CCheckBox */

CCheckBox CCheckBox::manager;

void CCheckBox::clicked(void)
{
  RAISE_EVENT_ACTION(EVENT_Click);
}

#include "CCheckBox_desc.h"
