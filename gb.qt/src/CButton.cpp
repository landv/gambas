/***************************************************************************

  CButton.cpp

  The Button and ToggleButton  class

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


#define __CBUTTON_CPP

#include <qapplication.h>
#include <qevent.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qsizepolicy.h>
#include <qiconset.h>
#include <qlayout.h>
#include <qobjectlist.h>

#include "gambas.h"

#include "CWidget.h"
#include "CPicture.h"
#include "CWindow.h"
#include "CButton.h"

/*#define DEBUG_CBUTTON*/

DECLARE_EVENT(EVENT_Click);
DECLARE_EVENT(EVENT_ClickToggle);
DECLARE_EVENT(EVENT_ClickTool);


BEGIN_METHOD(CBUTTON_new, GB_OBJECT parent)

  MyPushButton *wid = new MyPushButton(QCONTAINER(VARG(parent)));

  QObject::connect(wid, SIGNAL(clicked()), &CButton::manager, SLOT(clicked()));

  CWIDGET_new(wid, (void *)_object, "Button");
  OBJECT(CBUTTON)->picture = NULL;

  // We assume that the button widget destructor will always be called before
  // the its gambas window is released.
  WIDGET->top = CWidget::getWindow((CWIDGET *)THIS);
  //qDebug("top = %p %s", WIDGET->top, GB.GetClassName(CWidget::get(WIDGET->top)));

  //wid->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  wid->setAutoDefault(false);
  wid->show();

END_METHOD


BEGIN_METHOD(CTOGGLEBUTTON_new, GB_OBJECT parent)

  QPushButton *wid = new MyPushButton(QCONTAINER(VARG(parent)));

  QObject::connect(wid, SIGNAL(toggled(bool)), &CButton::manager, SLOT(clickedToggle()));

  CWIDGET_new(wid, (void *)_object, "ToggleButton");
  OBJECT(CBUTTON)->picture = NULL;

  //wid->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  wid->setAutoDefault(false);
  wid->setToggleButton(TRUE);
  wid->show();

END_METHOD


BEGIN_METHOD(CTOOLBUTTON_new, GB_OBJECT parent)

  MyToolButton *wid = new MyToolButton(QCONTAINER(VARG(parent)));

  QObject::connect(wid, SIGNAL(clicked()), &CButton::manager, SLOT(clickedTool()));

  CWIDGET_new(wid, (void *)_object);

  //wid->setToggleButton(TRUE);
  wid->setTextPosition(QToolButton::Right);
  wid->setAutoRaise(true);
  wid->show();

END_METHOD

/*
BEGIN_METHOD(CTOOLBUTTON_new, void *parent)

  QToolButton *wid = new QToolButton(CONTAINER(PARAM(parent)));

  QObject::connect(wid, SIGNAL(clicked()), &CButton::manager, SLOT(clickedTool()));

  CWIDGET_new(wid, (void *)_object, "ToolButton");
  OBJECT(CBUTTON)->picture = NULL;

  wid->show();

END_METHOD
*/

BEGIN_METHOD_VOID(CBUTTON_free)

  CLEAR_PICTURE(&(OBJECT(CBUTTON)->picture));

END_METHOD


static void set_button(CBUTTON *_object, const char *text)
{
  QPixmap p;
  QString qtext;
  QIconSet icon;

  if (text == NULL)
    qtext = WIDGET->text();
  else
    qtext = TO_QSTRING(text);

  if (THIS->picture)
  {
    p = *(THIS->picture->pixmap);

    WIDGET->setText(qtext);

    if (qtext.length())
    {
    	CWIDGET_iconset(icon, p);
      WIDGET->setIconSet(icon);
    }
    else
    {
      WIDGET->setPixmap(p);
      if (WIDGET->iconSet())
        WIDGET->setIconSet(icon);
    }
  }
  else
  {
    if (WIDGET->pixmap())
      WIDGET->setPixmap(QPixmap());
    if (WIDGET->iconSet())
      WIDGET->setIconSet(icon);

    WIDGET->setText(qtext);
  }

  WIDGET->calcMinimumHeight();
}


static void set_tool_button(CBUTTON *_object, const char *text)
{
  QPixmap p;
  QString qtext;
  QIconSet icon;

  if (text == NULL)
    qtext = WIDGET_TOOL->textLabel();
  else
    qtext = TO_QSTRING(text);

  if (THIS->picture)
  {
    p = *(THIS->picture->pixmap);

    WIDGET_TOOL->setTextLabel(qtext);
    CWIDGET_iconset(icon, p);
    WIDGET_TOOL->setIconSet(icon);
    WIDGET_TOOL->setUsesTextLabel(qtext.length() > 0);
  }
  else
  {
    WIDGET_TOOL->setIconSet(icon);
    WIDGET_TOOL->setTextLabel(qtext);
    WIDGET_TOOL->setUsesTextLabel(qtext.length() > 0);
  }

  CWidget::resetTooltip((CWIDGET *)_object);
  WIDGET->calcMinimumHeight();
}


BEGIN_PROPERTY(CBUTTON_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->text()));
  else
    set_button(THIS, GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(CBUTTON_picture)

  if (READ_PROPERTY)
    GB.ReturnObject(THIS->picture);
  else
  {
    GB.StoreObject(PROP(GB_OBJECT), (void **)&(THIS->picture));
    set_button(THIS, NULL);
  }

END_PROPERTY


BEGIN_PROPERTY(CTOOLBUTTON_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->text()));
  else
    set_tool_button(THIS, GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_PROPERTY(CTOOLBUTTON_picture)

  if (READ_PROPERTY)
    GB.ReturnObject(THIS->picture);
  else
  {
    GB.StoreObject(PROP(GB_OBJECT), (void **)&(THIS->picture));
    set_tool_button(THIS, NULL);
  }

END_PROPERTY

/*
BEGIN_PROPERTY(CTOOLBUTTON_picture)

  QToolButton *wid = QTOOLBUTTON(_object);
  char **pict = (char **)&(OBJECT(CBUTTON)->picture);

  if (READ_PROPERTY)
    GB.ReturnString(*pict, 0);
  else
    wid->setIconSet(QIconSet(*PIXMAP_set_widget(PROPERTY(GB_STRING), pict), QIconSet::Small));

END_PROPERTY
*/

BEGIN_PROPERTY(CBUTTON_value)

  if (READ_PROPERTY)
    GB.ReturnBoolean(0);
  else if (VPROP(GB_BOOLEAN))
    WIDGET->animateClick();

END_PROPERTY


BEGIN_PROPERTY(CTOGGLEBUTTON_value)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isOn());
  else
    WIDGET->setOn(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTOOLBUTTON_value)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET_TOOL->isOn());
  else
  {
    WIDGET_TOOL->setOn(VPROP(GB_BOOLEAN));
    qApp->postEvent(WIDGET_TOOL, new QEvent(QEvent::Leave));
  }

END_PROPERTY


BEGIN_PROPERTY(CBUTTON_flat)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isFlat());
  else
    WIDGET->setFlat(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CBUTTON_border)

  if (READ_PROPERTY)
    GB.ReturnBoolean(!WIDGET->isFlat());
  else
    WIDGET->setFlat(!VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTOOLBUTTON_border)

  if (READ_PROPERTY)
    GB.ReturnBoolean(!WIDGET_TOOL->autoRaise());
  else
    WIDGET_TOOL->setAutoRaise(!VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTOOLBUTTON_toggle)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET_TOOL->isToggleButton());
  else
  {
    WIDGET_TOOL->setToggleButton(VPROP(GB_BOOLEAN));
    QObject::disconnect(WIDGET_TOOL, 0, &CButton::manager, 0);
    if (VPROP(GB_BOOLEAN))
	  	QObject::connect(WIDGET_TOOL, SIGNAL(toggled(bool)), &CButton::manager, SLOT(clickedTool()));
		else
  		QObject::connect(WIDGET_TOOL, SIGNAL(clicked()), &CButton::manager, SLOT(clickedTool()));
	}

END_PROPERTY


BEGIN_PROPERTY(CBUTTON_default)

  CWINDOW *top = CWidget::getWindow((CWIDGET *)THIS);

  if (READ_PROPERTY)
    GB.ReturnBoolean(top->defaultButton == WIDGET);
  else
  	CWINDOW_set_default_button(top, WIDGET, VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CBUTTON_cancel)

  CWINDOW *top = CWidget::getWindow((CWIDGET *)THIS);

  if (READ_PROPERTY)
    GB.ReturnBoolean(top->cancelButton == WIDGET);
  else
  	CWINDOW_set_cancel_button(top, WIDGET, VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CBUTTON_radio)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->radio);
  else
  	THIS->radio = VPROP(GB_BOOLEAN);

END_PROPERTY

#define CTOGGLEBUTTON_free CBUTTON_free
#define CTOGGLEBUTTON_text CBUTTON_text
#define CTOGGLEBUTTON_picture CBUTTON_picture
#define CTOGGLEBUTTON_border CBUTTON_border
#define CTOGGLEBUTTON_radio CBUTTON_radio

#define CTOOLBUTTON_free CBUTTON_free
#define CTOOLBUTTON_radio CBUTTON_radio

#include "CButton_desc.h"



/** class MyPushButton *****************************************************/

MyPushButton::MyPushButton(QWidget *parent) :
  QPushButton(parent)
{
  calcMinimumHeight();
  top = 0;
}

MyPushButton::~MyPushButton()
{
	if (top)
	{
  	CWINDOW_set_default_button(top, this, false);
  	CWINDOW_set_cancel_button(top, this, false);
	}
}

/*QSize MyPushButton::sizeHint(void) const
{
  return QSize(width(), height());
}*/

void MyPushButton::fontChange(const QFont &font)
{
  QWidget::fontChange(font);
  calcMinimumHeight();
}

void MyPushButton::calcMinimumHeight()
{
  if (text().length() > 0)
  {
    QFontMetrics fm = fontMetrics();
    setMinimumHeight(fm.lineSpacing() + 4);
  }
  else
    setMinimumHeight(0);

	//qDebug("%p: %s: %d", this, text().latin1(), minimumHeight());
}

/** class MyToolButton *****************************************************/

MyToolButton::MyToolButton(QWidget *parent) :
  QToolButton(parent)
{
  calcMinimumHeight();
}

MyToolButton::~MyToolButton()
{
}

void MyToolButton::fontChange(const QFont &font)
{
  QToolButton::fontChange(font);
  calcMinimumHeight();
}

void MyToolButton::calcMinimumHeight()
{
  if (text().length() > 0)
  {
    QFontMetrics fm = fontMetrics();
    setMinimumHeight(fm.lineSpacing() + 4);
  }
  else
    setMinimumHeight(0);
}


/* Class CButton */

CButton CButton::manager;

void CButton::onlyMe(CBUTTON *_object)
{
	QWidget *parent = WIDGET->parentWidget();
  QObjectList *list = parent->queryList(0, 0, false, false);
  QObject *o;
  CBUTTON *other;
  
  for (o = list->first(); o; o = list->next())
  {
  	if (!o->isWidgetType())
  		continue;
		other = (CBUTTON *)CWidget::get(o);
		if (other == THIS)
			continue;
		if (other->widget.ob.klass != THIS->widget.ob.klass)
			continue;
		if (!other->radio)
			continue;
		o->blockSignals(true);
		if (o->isA("MyPushButton"))
			((MyPushButton *)o)->setOn(false);
		else
			((MyToolButton *)o)->setOn(false);
		o->blockSignals(false);
  }
  
  delete list;
}

void CButton::clicked(void)
{
  RAISE_EVENT_ACTION(EVENT_Click);
}

void CButton::clickedToggle(void)
{
	GET_SENDER(_object);
	
	if (THIS->radio)
	{ 
		if (WIDGET->isOn())
			onlyMe(THIS);
		else
		{
			WIDGET->setOn(true);
			return;
		}
	}

  RAISE_EVENT_ACTION(EVENT_ClickToggle);
}

void CButton::clickedTool(void)
{
	GET_SENDER(_object);
	
	if (THIS->radio)
	{ 
		if (WIDGET_TOOL->isOn())
			onlyMe(THIS);
		else
		{
			WIDGET_TOOL->setOn(true);
			return;
		}
	}

  RAISE_EVENT_ACTION(EVENT_ClickTool);
}


/*
GB_DESC CToolButtonDesc[] =
{
  GB_DECLARE("ToolButton", sizeof(CBUTTON)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CTOOLBUTTON_new, "Container;"),
  GB_METHOD("_free", NULL, CBUTTON_delete, NULL),

  GB_PROPERTY("Text", "s", CBUTTON_text),
  GB_PROPERTY("Caption", "s", CBUTTON_text),
  GB_PROPERTY("Picture", "s", CTOOLBUTTON_picture),
  GB_PROPERTY("Value", "b", CBUTTON_value),
  GB_PROPERTY("Flat", "b", CBUTTON_flat),

  GB_EVENT("Click", NULL, NULL, &EVENT_ClickTool),

  GB_END_DECLARE
};
*/


