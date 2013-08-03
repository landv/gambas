/***************************************************************************

  CButton.cpp

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

#define __CBUTTON_CPP

#include <qapplication.h>
#include <qevent.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qsizepolicy.h>
#include <qicon.h>
#include <qlayout.h>
#include <qobject.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QPixmap>
#include <QStylePainter>
#include <QStyleOptionToolButton>

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

  wid->setAutoDefault(false);
  
  CWIDGET_new(wid, (void *)_object);

  // We assume that the button widget destructor will always be called before
  // its gambas window is released.
  WIDGET->top = CWidget::getWindow((CWIDGET *)THIS);

END_METHOD


BEGIN_METHOD(CTOGGLEBUTTON_new, GB_OBJECT parent)

  QPushButton *wid = new MyPushButton(QCONTAINER(VARG(parent)));

  QObject::connect(wid, SIGNAL(toggled(bool)), &CButton::manager, SLOT(clickedToggle()));

  //wid->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
  wid->setAutoDefault(false);
  wid->setCheckable(TRUE);
  
  CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_METHOD(CTOOLBUTTON_new, GB_OBJECT parent)

  MyToolButton *wid = new MyToolButton(QCONTAINER(VARG(parent)));

  QObject::connect(wid, SIGNAL(clicked()), &CButton::manager, SLOT(clickedTool()));

	//wid->setToggleButton(TRUE);
  wid->setAutoRaise(true);
  
  CWIDGET_new(wid, (void *)_object);

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


static void set_button(CBUTTON *_object, const char *text, bool resize = false)
{
  QPixmap p;
  QString qtext;
  QIcon icon;
  int size;

	size = qMin(WIDGET->width(), WIDGET->height()) - 6;
	if (resize && size == THIS->last_size)
		return;

  if (text == NULL) // We are changing picture
  {
    qtext = WIDGET->text();
  
		if (THIS->picture)
		{
			p = *(THIS->picture->pixmap);
			/*if (THIS->stretch)
			{
				if (size > 0)
					CWIDGET_iconset(icon, p, size);
			}
			else*/
				CWIDGET_iconset(icon, p);
			WIDGET->setIcon(icon);
			WIDGET->setIconSize(p.size());
		}
		else
		{
			WIDGET->setIcon(icon);
		}
  }
  else // We are changing text
  {
    qtext = TO_QSTRING(text);
		WIDGET->setText(qtext);
  }

  WIDGET->calcMinimumSize();
}


static void set_tool_button(CBUTTON *_object, const char *text, bool resize = false)
{
  QPixmap p;
  QString qtext;
  QIcon icon;
  int size;

	size = qMin(WIDGET_TOOL->width(), WIDGET_TOOL->height()) - 6;
    
	if (resize && size == THIS->last_size)
		return;
	
  if (text == NULL)
    qtext = WIDGET_TOOL->text();
  else
    qtext = TO_QSTRING(text);

  if (THIS->picture)
  {
    p = *(THIS->picture->pixmap);

    WIDGET_TOOL->setText(qtext);
 		/*if (THIS->stretch)
 		{
    	if (size > 0)
	    	CWIDGET_iconset(icon, p, size);
	  }
	  else*/
	    CWIDGET_iconset(icon, p);
	    	
    WIDGET_TOOL->setIcon(icon);
		WIDGET->setIconSize(p.size());
    //WIDGET_TOOL->setUsesTextLabel(qtext.length() > 0);
		if (qtext.length())
			WIDGET_TOOL->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		else
			WIDGET_TOOL->setToolButtonStyle(Qt::ToolButtonIconOnly);
    
		THIS->last_size = size;
  }
  else
  {
    WIDGET_TOOL->setIcon(icon);
    WIDGET_TOOL->setText(qtext);
		WIDGET_TOOL->setToolButtonStyle(Qt::ToolButtonTextOnly);
    //WIDGET_TOOL->setUsesTextLabel(qtext.length() > 0);
  }

  //CWidget::resetTooltip((CWIDGET *)_object);
  WIDGET->calcMinimumSize();
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
    GB.StoreObject(PROP(GB_OBJECT), POINTER(&(THIS->picture)));
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
    GB.StoreObject(PROP(GB_OBJECT), POINTER(&(THIS->picture)));
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
    GB.ReturnBoolean(WIDGET->isChecked());
  else
    WIDGET->setChecked(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTOOLBUTTON_value)

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET_TOOL->isChecked());
  else
  {
    if (WIDGET->isCheckable())
      WIDGET_TOOL->setChecked(VPROP(GB_BOOLEAN));
    else
      WIDGET->animateClick();
    //qApp->postEvent(WIDGET_TOOL, new QEvent(QEvent::Leave));
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
    GB.ReturnBoolean(WIDGET_TOOL->isCheckable());
  else
  {
    WIDGET_TOOL->setCheckable(VPROP(GB_BOOLEAN));
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

BEGIN_PROPERTY(CBUTTON_autoresize)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->autoresize);
	else if (THIS->autoresize != VPROP(GB_BOOLEAN))
	{
		THIS->autoresize = VPROP(GB_BOOLEAN);
		WIDGET->calcMinimumSize();
	}

END_PROPERTY

BEGIN_PROPERTY(CTOOLBUTTON_autoresize)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->autoresize);
	else if (THIS->autoresize != VPROP(GB_BOOLEAN))
	{
		THIS->autoresize = VPROP(GB_BOOLEAN);
		WIDGET_TOOL->calcMinimumSize();
	}

END_PROPERTY

#if 0
BEGIN_PROPERTY(CBUTTON_stretch)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->stretch);
	else if (THIS->stretch != VPROP(GB_BOOLEAN))
	{
		THIS->stretch = VPROP(GB_BOOLEAN);
		set_button(THIS, NULL, true);
	}

END_PROPERTY

BEGIN_PROPERTY(CTOOLBUTTON_stretch)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->stretch);
	else if (THIS->stretch != VPROP(GB_BOOLEAN))
	{
		THIS->stretch = VPROP(GB_BOOLEAN);
		set_tool_button(THIS, NULL, true);
	}

END_PROPERTY
#endif

#define CTOGGLEBUTTON_free CBUTTON_free
#define CTOGGLEBUTTON_text CBUTTON_text
#define CTOGGLEBUTTON_picture CBUTTON_picture
#define CTOGGLEBUTTON_border CBUTTON_border
#define CTOGGLEBUTTON_radio CBUTTON_radio
#define CTOGGLEBUTTON_stretch CBUTTON_stretch

#define CTOOLBUTTON_free CBUTTON_free
#define CTOOLBUTTON_radio CBUTTON_radio

GB_DESC CButtonDesc[] =
{
  GB_DECLARE("Button", sizeof(CBUTTON)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CBUTTON_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CBUTTON_free, NULL),

  GB_PROPERTY("Text", "s", CBUTTON_text),
  GB_PROPERTY("Caption", "s", CBUTTON_text),
  GB_PROPERTY("Picture", "Picture", CBUTTON_picture),

  GB_PROPERTY("Border", "b", CBUTTON_border),
  GB_PROPERTY("Default", "b", CBUTTON_default),
  GB_PROPERTY("Cancel", "b", CBUTTON_cancel),
  GB_PROPERTY("Value", "b", CBUTTON_value),
  GB_PROPERTY("AutoResize", "b", CBUTTON_autoresize),

	BUTTON_DESCRIPTION,

  GB_EVENT("Click", NULL, NULL, &EVENT_Click),

  GB_END_DECLARE
};

GB_DESC CToggleButtonDesc[] =
{
  GB_DECLARE("ToggleButton", sizeof(CBUTTON)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CTOGGLEBUTTON_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CTOGGLEBUTTON_free, NULL),

  GB_PROPERTY("Text", "s", CTOGGLEBUTTON_text),
  GB_PROPERTY("Caption", "s", CTOGGLEBUTTON_text),
  GB_PROPERTY("Picture", "Picture", CTOGGLEBUTTON_picture),
  GB_PROPERTY("Value", "b", CTOGGLEBUTTON_value),
  //GB_PROPERTY("Flat", "b", CBUTTON_flat),
  GB_PROPERTY("Border", "b", CTOGGLEBUTTON_border),
  GB_PROPERTY("Radio", "b", CTOGGLEBUTTON_radio),
  GB_PROPERTY("AutoResize", "b", CBUTTON_autoresize),

	TOGGLEBUTTON_DESCRIPTION,

  GB_EVENT("Click", NULL, NULL, &EVENT_ClickToggle),

  GB_END_DECLARE
};


GB_DESC CToolButtonDesc[] =
{
  GB_DECLARE("ToolButton", sizeof(CBUTTON)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CTOOLBUTTON_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CTOOLBUTTON_free, NULL),

  GB_PROPERTY("Text", "s", CTOOLBUTTON_text),
  GB_PROPERTY("Caption", "s", CTOOLBUTTON_text),
  GB_PROPERTY("Picture", "Picture", CTOOLBUTTON_picture),
  GB_PROPERTY("Value", "b", CTOOLBUTTON_value),
  GB_PROPERTY("Toggle", "b", CTOOLBUTTON_toggle),
  GB_PROPERTY("Border", "b", CTOOLBUTTON_border),
  GB_PROPERTY("Radio", "b", CTOOLBUTTON_radio),
  GB_PROPERTY("AutoResize", "b", CTOOLBUTTON_autoresize),

	TOOLBUTTON_DESCRIPTION,

  GB_EVENT("Click", NULL, NULL, &EVENT_ClickTool),

  GB_END_DECLARE
};



/** class MyPushButton *****************************************************/

MyPushButton::MyPushButton(QWidget *parent) :
  QPushButton(parent)
{
  calcMinimumSize();
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

void MyPushButton::changeEvent(QEvent *e)
{
  QPushButton::changeEvent(e);
	if (e->type() == QEvent::FontChange || e->type() == QEvent::StyleChange)
		calcMinimumSize();
}

void MyPushButton::calcMinimumSize()
{
	CBUTTON *_object = (CBUTTON *)CWidget::getReal(this);
	QSize size;
	
	if (!THIS || CWIDGET_test_flag(THIS, WF_DESIGN))
		return;

	if (text().length() > 0)
  {
    QFontMetrics fm = fontMetrics();
    setMinimumHeight(fm.lineSpacing() + 4);
  }
  else
    setMinimumHeight(0);

	setMinimumWidth(0);
	if (THIS->autoresize)
	{
		size = sizeHint();
		CWIDGET_resize(THIS, size.width(), height());
		setMinimumWidth(size.width());
	}

	//qDebug("%p: %s: %d", this, text().latin1(), minimumHeight());
}

/*void MyPushButton::resizeEvent(QResizeEvent *e)
{
	set_button((CBUTTON *)CWidget::get(this), NULL, true);
}*/

/*void MyPushButton::paintEvent(QPaintEvent *)
{
	CBUTTON *_object = (CBUTTON *)CWidget::get(this);
	QStylePainter p(this);
	QStyleOptionToolButton opt;
	initStyleOption(&opt);
	if (THIS->picture)
		opt.iconSize = THIS->picture->pixmap->size();
	p.drawComplexControl(QStyle::CC_PushButton, opt);
}*/


/** class MyToolButton *****************************************************/

MyToolButton::MyToolButton(QWidget *parent) :
  QToolButton(parent)
{
  calcMinimumSize();
}

MyToolButton::~MyToolButton()
{
}

void MyToolButton::changeEvent(QEvent *e)
{
  QToolButton::changeEvent(e);
	if (e->type() == QEvent::FontChange || e->type() == QEvent::StyleChange)
		calcMinimumSize();
}

void MyToolButton::calcMinimumSize()
{
	CBUTTON *_object = (CBUTTON *)CWidget::get(this);
	QSize size;

	if (!THIS || CWIDGET_test_flag(THIS, WF_DESIGN))
		return;
	
	if (text().length() > 0)
  {
    QFontMetrics fm = fontMetrics();
    setMinimumHeight(fm.lineSpacing() + 4);
  }
  else
    setMinimumHeight(0);

	setMinimumWidth(0);
	if (THIS->autoresize)
	{
		size = sizeHint();
		CWIDGET_resize(THIS, size.width(), height());
		setMinimumWidth(size.width());
	}
}

/*void MyToolButton::resizeEvent(QResizeEvent *e)
{
	//set_tool_button((CBUTTON *)CWidget::get(this), NULL, true);
}*/

/*void MyToolButton::paintEvent(QPaintEvent *)
{
	CBUTTON *_object = (CBUTTON *)CWidget::get(this);
	QStylePainter p(this);
	QStyleOptionToolButton opt;
	initStyleOption(&opt);
	if (THIS->picture)
		opt.iconSize = THIS->picture->pixmap->size();
	p.drawComplexControl(QStyle::CC_ToolButton, opt);
}*/


/* Class CButton */

CButton CButton::manager;

void CButton::onlyMe(CBUTTON *_object)
{
	QWidget *parent = WIDGET->parentWidget();
  QObjectList list = parent->children();
  int i;
  QObject *o;
  CBUTTON *other;
  
  for (i = 0; i < list.count(); i++)
  {
  	o = list.at(i);
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
		/*if (qobject_cast<MyPushButton *>(o))
			((MyPushButton *)o)->setChecked(false);
		else
			((MyToolButton *)o)->setChecked(false);*/
		(qobject_cast<QAbstractButton *>(o))->setChecked(false);
		o->blockSignals(false);
  }
}

void CButton::clicked(void)
{
  RAISE_EVENT_ACTION(EVENT_Click);
}

void CButton::clickedToggle(void)
{
	GET_SENDER();
	
	if (THIS->radio)
	{ 
		if (WIDGET->isChecked())
			onlyMe(THIS);
		else
		{
			WIDGET->setChecked(true);
			return;
		}
	}

  RAISE_EVENT_ACTION(EVENT_ClickToggle);
}

void CButton::clickedTool(void)
{
	GET_SENDER();
	
	if (THIS->radio)
	{ 
		if (WIDGET_TOOL->isChecked())
			onlyMe(THIS);
		else
		{
			WIDGET_TOOL->setChecked(true);
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


