/***************************************************************************

  CRadioButton.cpp

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

#define __CRADIOBUTTON_CPP

#include <QApplication>
#include <QRadioButton>
#include <QResizeEvent>

#include "gambas.h"

#include "CRadioButton.h"


/** MyRadioButton **********************************************************/

MyRadioButton::MyRadioButton(QWidget *parent) : QRadioButton(parent)
{
	_autoResize = false;
}


void MyRadioButton::changeEvent(QEvent *e)
{
  QRadioButton::changeEvent(e);
	if (e->type() == QEvent::FontChange || e->type() == QEvent::StyleChange)
		adjust();
}

void MyRadioButton::adjust(bool force)
{
	void *_object = CWidget::getReal(this);
	bool a;
	QSize hint;

	if (!THIS || (!_autoResize && !force) || CWIDGET_test_flag(THIS, WF_DESIGN) || text().length() <= 0)
		return;
	
	a = _autoResize;
	_autoResize = false;
	hint = sizeHint();
	CWIDGET_resize(THIS, hint.width(), QMAX(hint.height(), height()));
	_autoResize = a;
}

void MyRadioButton::resizeEvent(QResizeEvent *e)
{
	QRadioButton::resizeEvent(e);
  
  if (_autoResize && e->oldSize().width() != e->size().width())
  	adjust();
}

/** RadioButton ************************************************************/

DECLARE_EVENT(EVENT_Click);


BEGIN_METHOD(RadioButton_new, GB_OBJECT parent)

  MyRadioButton *wid = new MyRadioButton(QCONTAINER(VARG(parent)));

  QObject::connect(wid, SIGNAL(toggled(bool)), &CRadioButton::manager, SLOT(clicked(bool)));

  CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_PROPERTY(RadioButton_Text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->text()));
  else
	{
    WIDGET->setText(QSTRING_PROP());
		WIDGET->adjust();
	}

END_PROPERTY


BEGIN_PROPERTY(RadioButton_Value)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isChecked());
  else
    WIDGET->setChecked(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(RadioButton_AutoResize)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isAutoResize());
  else
    WIDGET->setAutoResize(VPROP(GB_BOOLEAN));

END_PROPERTY


// BEGIN_METHOD_VOID(RadioButton_Adjust)
// 
// 	WIDGET->adjust(true);
// 
// END_METHOD


GB_DESC CRadioButtonDesc[] =
{
  GB_DECLARE("RadioButton", sizeof(CRADIOBUTTON)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, RadioButton_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", RadioButton_Text),
  GB_PROPERTY("Caption", "s", RadioButton_Text),
  GB_PROPERTY("Value", "b", RadioButton_Value),
  GB_PROPERTY("AutoResize", "b", RadioButton_AutoResize),

  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  
  RADIOBUTTON_DESCRIPTION,

  GB_END_DECLARE
};


/** CCheckBox **************************************************************/

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

