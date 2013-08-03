/***************************************************************************

  CCheckBox.cpp

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

#define __CCHECKBOX_CPP

#include "gambas.h"

#include <QResizeEvent>

#include "CCheckBox.h"

/** MyCheckBox *************************************************************/

MyCheckBox::MyCheckBox(QWidget *parent) : QCheckBox(parent)
{
	_autoResize = false;
}


void MyCheckBox::changeEvent(QEvent *e)
{
  QCheckBox::changeEvent(e);
	if (e->type() == QEvent::FontChange || e->type() == QEvent::StyleChange)
		adjust();
}

void MyCheckBox::adjust(bool force)
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

void MyCheckBox::resizeEvent(QResizeEvent *e)
{
	QCheckBox::resizeEvent(e);
  
  if (_autoResize && e->oldSize().width() != e->size().width())
  	adjust();
}

/** CheckBox ***************************************************************/

DECLARE_EVENT(EVENT_Click);


BEGIN_METHOD(CCHECKBOX_new, GB_OBJECT parent)

  QCheckBox *wid = new MyCheckBox(QCONTAINER(VARG(parent)));

  QObject::connect(wid, SIGNAL(stateChanged(int)), &CCheckBox::manager, SLOT(clicked()));

	wid->setMinimumHeight(wid->sizeHint().height());

  CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_PROPERTY(CheckBox_Text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->text()));
  else
	{
    WIDGET->setText(QSTRING_PROP());
		WIDGET->adjust();
	}

END_PROPERTY


BEGIN_PROPERTY(CheckBox_Value)

  /*if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isChecked());
  else
    WIDGET->setChecked(VPROP(GB_BOOLEAN));*/

  if (READ_PROPERTY)
  {
		switch(WIDGET->checkState())
		{
			case Qt::Unchecked: GB.ReturnInteger(0); break;
			case Qt::Checked: GB.ReturnInteger(-1); break;
			case Qt::PartiallyChecked: GB.ReturnInteger(1); break;
		}
	}
  else
  {
  	if (WIDGET->isTristate() && VPROP(GB_INTEGER) == 1)
  		WIDGET->setCheckState(Qt::PartiallyChecked);
		else
	    WIDGET->setCheckState(VPROP(GB_INTEGER) == 0 ? Qt::Unchecked : Qt::Checked);
	}

END_PROPERTY


BEGIN_PROPERTY(CheckBox_TriState)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isTristate());
  else
    WIDGET->setTristate(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CheckBox_AutoResize)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isAutoResize());
  else
    WIDGET->setAutoResize(VPROP(GB_BOOLEAN));

END_PROPERTY


GB_DESC CCheckBoxDesc[] =
{
	GB_DECLARE("CheckBox", sizeof(CCHECKBOX)), GB_INHERITS("Control"),

	GB_METHOD("_new", NULL, CCHECKBOX_new, "(Parent)Container;"),

	GB_CONSTANT("False", "i", 0),
	GB_CONSTANT("True", "i", -1),
	GB_CONSTANT("None", "i", 1),

	GB_PROPERTY("Text", "s", CheckBox_Text),
	GB_PROPERTY("Caption", "s", CheckBox_Text),
	GB_PROPERTY("Value", "i", CheckBox_Value),
	GB_PROPERTY("Tristate", "b", CheckBox_TriState),
  GB_PROPERTY("AutoResize", "b", CheckBox_AutoResize),

	CHECKBOX_DESCRIPTION,

	GB_EVENT("Click", NULL, NULL, &EVENT_Click),

	GB_END_DECLARE
};


/** CCheckBox *************************************************************/

CCheckBox CCheckBox::manager;

void CCheckBox::clicked(void)
{
  RAISE_EVENT_ACTION(EVENT_Click);
}


