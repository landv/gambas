/***************************************************************************

  CFrame.cpp

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

#define __CFRAME_CPP

#include <QApplication>
#include <QEvent>

#include "gambas.h"

#include "CFrame.h"

//---------------------------------------------------------------------------

MyGroupBox::MyGroupBox(QWidget *parent) : QGroupBox(parent)
{
}

void MyGroupBox::changeEvent(QEvent *e)
{
  QGroupBox::changeEvent(e);
	if (e->type() == QEvent::FontChange || e->type() == QEvent::StyleChange)
		updateInside();
}

void MyGroupBox::updateInside()
{
	int f = qApp->style()->pixelMetric(QStyle::QStyle::PM_ComboBoxFrameWidth);
	setContentsMargins(f, fontMetrics().height() * 3 / 2, f, f);
}

// Warning! showEvent and hideEvent must be the same as in MyContainer class.

void MyGroupBox::showEvent(QShowEvent *e)
{
	void *_object = CWidget::get(this);
	QWidget::showEvent(e);
	THIS->widget.flag.shown = TRUE;
	CCONTAINER_arrange(THIS);
}

void MyGroupBox::hideEvent(QHideEvent *e)
{
	void *_object = CWidget::get(this);
	QWidget::hideEvent(e);
	THIS->widget.flag.shown = FALSE;
}

//---------------------------------------------------------------------------

BEGIN_METHOD(Frame_new, GB_OBJECT parent)

	MyGroupBox *wid = new MyGroupBox(QCONTAINER(VARG(parent)));

	THIS->container = wid;

	CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_PROPERTY(Frame_Text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(WIDGET->title()));
	else
	{
		WIDGET->setTitle(QSTRING_PROP());
		WIDGET->updateInside();
	}

END_PROPERTY


GB_DESC CFrameDesc[] =
{
	GB_DECLARE("Frame", sizeof(CFRAME)), GB_INHERITS("Container"),

	GB_METHOD("_new", NULL, Frame_new, "(Parent)Container;"),

	GB_PROPERTY("Caption", "s", Frame_Text),
	GB_PROPERTY("Text", "s", Frame_Text),
	GB_PROPERTY("Title", "s", Frame_Text),

	GB_PROPERTY("Arrangement", "i", Container_Arrangement),
	GB_PROPERTY("AutoResize", "b", Container_AutoResize),
	GB_PROPERTY("Spacing", "b", Container_Spacing),
	GB_PROPERTY("Margin", "b", Container_Margin),
	GB_PROPERTY("Padding", "i", Container_Padding),
	GB_PROPERTY("Indent", "b", Container_Indent),
	GB_PROPERTY("Invert", "b", Container_Invert),

	FRAME_DESCRIPTION,

	GB_END_DECLARE
};


