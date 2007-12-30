/***************************************************************************

  CSpinBox.cpp

  The SpinBox class

  Hacked together by Nigel Gerrard using code provided by
  
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

#define __CSPINBOX_CPP

#include "main.h"

#include <qapplication.h>
#include <qspinbox.h>
#include <qlineedit.h>

#include "CWidget.h"
#include "CSpinBox.h"

DECLARE_EVENT(EVENT_Change);


BEGIN_METHOD(CSPINBOX_new, GB_OBJECT parent)

  QSpinBox *wid = new QSpinBox(QCONTAINER(VARG(parent)));
  
  CWIDGET_new(wid, _object);
  //QT.SetBackgroundRole(_object, QColorGroup::Base);

  QObject::connect(wid, SIGNAL(valueChanged(int)), &CSpinBox::manager, SLOT(event_change()));

  //printf("Up Details %i %i %i %i\n", wid->upRect());
  // printf("Down Details %i %i %i %i\n", wid->downRect());
  //printf("Geometry Details %i %i %i %i\n", wid->geometry());

	wid->setMinValue(0);
	wid->setMaxValue(100);
	wid->setLineStep(1);

  wid->show();

END_METHOD


BEGIN_PROPERTY(CSPINBOX_prefix)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->prefix()));
  else
    WIDGET->setPrefix(QSTRING_PROP());

END_PROPERTY

BEGIN_PROPERTY(CSPINBOX_suffix)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->suffix()));
  else
    WIDGET->setSuffix(QSTRING_PROP());

END_PROPERTY

/*
BEGIN_PROPERTY(CSPINBOX_specialtext)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(WIDGET->specialValueText());
  else
    WIDGET->setSpecialValueText(GB.ToZeroString(PROPERTY(GB_STRING)));

END_PROPERTY
*/

BEGIN_PROPERTY(CSPINBOX_wrapping)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->wrapping());
  else
    WIDGET->setWrapping(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CSPINBOX_value)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->value());
  else
    WIDGET->setValue(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CSPINBOX_text)

  GB.ReturnNewZeroString(WIDGET->text());

END_PROPERTY

BEGIN_PROPERTY(CSPINBOX_cleantext)

  GB.ReturnNewZeroString(WIDGET->cleanText());

END_PROPERTY

BEGIN_PROPERTY(CSPINBOX_minval)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->minValue());
  else
    WIDGET->setMinValue(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CSPINBOX_maxval)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->maxValue());
  else
    WIDGET->setMaxValue(VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CSPINBOX_linestep)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->lineStep());
  else
    WIDGET->setLineStep(VPROP(GB_INTEGER));

END_PROPERTY

/*BEGIN_PROPERTY(CSPINBOX_border)

	QLineEdit *lw = (QLineEdit *)WIDGET->child("qt_spinbox_edit", "QLineEdit");
	
	if (!lw)
	{
		if (READ_PROPERTY)
			GB.ReturnBoolean(0);
		return;
	}
		
	if (READ_PROPERTY)
		GB.ReturnBoolean(lw->frame());
	else
		lw->setFrame(VPROP(GB_BOOLEAN));

END_PROPERTY*/

/*
BEGIN_PROPERTY(CSPINBOX_button)

 if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->buttonSymbols());
 else
 {
  if ( PROPERTY(GB_INTEGER)  == 1)
    WIDGET->setButtonSymbols(QSpinBox::PlusMinus);
  else
    WIDGET->setButtonSymbols(QSpinBox::UpDownArrows);

    // The hide() and show() are needed to force an immediate update which could not be produced through
    // repaint()
    WIDGET->hide();
    WIDGET->show();
 }

END_PROPERTY
*/

BEGIN_METHOD_VOID(CSPINBOX_select_all)

	WIDGET->selectAll();

END_METHOD

/***************************************************************************

  class CSpinBox

***************************************************************************/

CSpinBox CSpinBox::manager;

void CSpinBox::event_change(void)
{
  RAISE_EVENT(EVENT_Change);  
}

GB_DESC CSpinBoxDesc[] =
{
  GB_DECLARE("SpinBox", sizeof(CSPINBOX)), GB_INHERITS("Control"),

  //GB_CONSTANT("UpDownArrows", "i", QSpinBox::UpDownArrows),
  //GB_CONSTANT("PlusMinus","i", QSpinBox::PlusMinus),

  GB_METHOD("_new", NULL, CSPINBOX_new, "(Parent)Container;"),

  GB_PROPERTY("Prefix", "s", CSPINBOX_prefix),
  GB_PROPERTY("Suffix", "s", CSPINBOX_suffix),
  GB_PROPERTY("Value", "i", CSPINBOX_value),
  //GB_PROPERTY("SpecialValueText", "s", CSPINBOX_specialtext),
  GB_PROPERTY_READ("FullText", "s", CSPINBOX_text),
  GB_PROPERTY_READ("Text", "s", CSPINBOX_cleantext),
  GB_PROPERTY("MinValue", "i", CSPINBOX_minval),
  GB_PROPERTY("MaxValue", "i", CSPINBOX_maxval),
  GB_PROPERTY("Step", "i", CSPINBOX_linestep),
  //GB_PROPERTY("ButtonSymbols", "i<SpinBox,UpDownArrows,PlusMinus>", CSPINBOX_button),

  GB_PROPERTY("Wrap", "b", CSPINBOX_wrapping),
  //GB_PROPERTY("Border", "b", CSPINBOX_border),

  GB_METHOD("SelectAll", NULL, CSPINBOX_select_all, NULL),
  
  GB_CONSTANT("_Properties", "s", "*,MinValue=0,MaxValue=100,Step=1,Prefix,Suffix,Wrap,Value"),
  GB_CONSTANT("_DefaultEvent", "s", "Change"),
  GB_CONSTANT("_DefaultSize", "s", "9,3"),

  GB_EVENT("Change", NULL, NULL, &EVENT_Change),

  GB_END_DECLARE
};

