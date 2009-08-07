/***************************************************************************

  CTabStrip.cpp

  The TabStrip class

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

#define __CTABSTRIP_CPP

#include <qapplication.h>
#include <qtabbar.h>
#include <QShowEvent>
#include <QWheelEvent>
#include <QEvent>
#include <QTabWidget>

#include "gambas.h"

#include "CPicture.h"
#include "CConst.h"
#include "CTabStrip.h"


DECLARE_METHOD(CCONTAINER_x);
DECLARE_METHOD(CCONTAINER_y);

DECLARE_EVENT(EVENT_Click);

/** CTab *****************************************************************/

class CTab
{
public:
	QWidget *widget;
	QString text;
	CPICTURE *icon;
	int id;
	bool visible;
	bool enabled;
	
	CTab(CTABSTRIP *parent, QWidget *page);
	~CTab();
	
	int index() const { return WIDGET->indexOf(widget); }
	void ensureVisible() const { WIDGET->setCurrentIndex(index()); }
	void setEnabled(bool e) { enabled = e; WIDGET->setTabEnabled(index(), e && WIDGET->isEnabled()); }
	bool isEnabled() const { return enabled; }
	bool isVisible() const { return visible; }
	void setVisible(bool v);
	void updateIcon();
	int count() const;
	bool isEmpty() const { return count() == 0; }

private:
	CTABSTRIP *_object;
};

CTab::CTab(CTABSTRIP *parent, QWidget *page)
{ 
	_object = parent;
	widget = page; 
	icon = 0; 
	id = WIDGET->stack.count();
	visible = true; 
  setEnabled(true);
	
	page->setAutoFillBackground(true);
	page->hide();
}	

CTab::~CTab()
{
	GB.Unref(POINTER(&icon));
}

int CTab::count() const
{
	QObjectList list = widget->children();
	QObject *ob;
	int n = 0;
	int i;
	
	for(i = 0; i < list.count(); i++)
	{
		ob = list.at(i);
		if (ob->isWidgetType() && CWidget::getRealExisting(ob))
			n++;
	}
	
	return n;
}

void CTab::setVisible(bool v)
{
	int i, ind;
	CTab *page;
	
	if (v == visible)
		return;
		
	visible = v;
	
	if (!visible)
	{
		text = WIDGET->tabText(index());
		WIDGET->removeTab(index());
		//widget->hide();
	}
	else
	{
		ind = 0;
		for (i = 0; i < (int)WIDGET->stack.count(); i++)
		{
			page = WIDGET->stack.at(i);
			if (!page->isVisible())
				continue;
			if (id == WIDGET->stack.at(i)->id)
				break;
			ind++;
		}
		WIDGET->insertTab(ind, widget, text);
	  setEnabled(enabled);
		updateIcon();
		if (WIDGET->count() == 1)
			ensureVisible();
	}
}


void CTab::updateIcon()
{
	QIcon iconset;
	
	if (icon)
		CWIDGET_iconset(iconset, *(icon->pixmap));
	
	WIDGET->setTabIcon(index(), iconset);
}

/** MyTabWidget **********************************************************/

MyTabWidget::MyTabWidget(QWidget *parent) : QTabWidget(parent)
{
	tabBar()->installEventFilter(this);
}

MyTabWidget::~MyTabWidget()
{
	int i;
	
	for (i = 0; i < stack.count(); i++)
		delete stack.at(i);
}

void MyTabWidget::setEnabled(bool e)
{
	CWIDGET *_object = CWidget::get(this);
	int i;
	
	QTabWidget::setEnabled(e);
	
	for (i = 0; i < (int)WIDGET->stack.count(); i++)
		WIDGET->stack.at(i)->widget->setEnabled(e);
}

void MyTabWidget::forceLayout()
{
	bool b = isVisible();
	QShowEvent e;
		
	setAttribute(Qt::WA_WState_Visible, true);
	qApp->sendEvent(this, &e);
	if (!b)
		setAttribute(Qt::WA_WState_Visible, false);
		
	//qDebug("Y = %d", tabBar()->height());
}

bool MyTabWidget::eventFilter(QObject *o, QEvent *e)
{
	if (e->type() == QEvent::Wheel && qobject_cast<QTabBar *>(o))
	{
		QWheelEvent *event = (QWheelEvent *)e;
		QTabBar *tabBar = (QTabBar *)o;
		int id = tabBar->currentIndex();
		
		if (id >= 0)
		{
			for(;;)
			{
				if (event->delta() < 0)
					id++;
				else
					id--;
					
				if (id < 0 || id >= tabBar->count())
					break;
				
				if (tabBar->isTabEnabled(id))
				{
					tabBar->setCurrentIndex(id);
					break;
				}
			}
		}
			
		return true;
	}

  return QObject::eventFilter(o, e);
}


/***************************************************************************

  TabStrip

***************************************************************************/

static bool remove_page(void *_object, int i)
{
	CTab *tab = WIDGET->stack.at(i);

	if (!tab->isEmpty())
	{
		GB.Error("Tab is not empty");
		return true;
	}

	THIS->lock = true;
	WIDGET->removeTab(tab->index());
	delete tab->widget;
	delete tab;
	WIDGET->stack.removeAt(i);
	THIS->lock = false;

	return false;
}

static bool set_tab_count(void *_object, int new_count)
{
  int count = WIDGET->stack.count();
  int i;
  int index;
  QString label;
  CTab *tab;

  if (new_count < 1 || new_count > 256)
  {
    GB.Error(GB_ERR_ARG);
    return true;
  }

  if (new_count == count)
    return false;

  if (new_count > count)
  {
    for (i = count; i < new_count; i++)
    {
    	tab = new CTab(THIS, new MyContainer(WIDGET));

      label.sprintf("Tab %d", i);
      WIDGET->addTab(tab->widget, label);
      
      WIDGET->stack.append(tab);
    }

    index = new_count - 1;

		WIDGET->stack.at(index)->ensureVisible();
		THIS->container = WIDGET->stack.at(index)->widget;    
  }
  else
  {
    index = WIDGET->currentIndex();
    //same = (id == PARAM(index));

    for (i = new_count; i < count; i++)
    {
      if (!WIDGET->stack.at(i)->isEmpty())
      {
        GB.Error("Tab is not empty");
        return true;
      }
    }

		if (index >= new_count)
			index = new_count - 1;
	
		WIDGET->stack.at(index)->ensureVisible();
		THIS->container = WIDGET->stack.at(index)->widget;
    
    for (i = count - 1; i >= new_count; i--)
    {
			remove_page(THIS, i);
    }

    //WIDGET->stack.resize(new_count);
    //THIS->icon->resize(new_count);
  }

  return false;
}


BEGIN_METHOD(CTABSTRIP_new, GB_OBJECT parent)

  MyTabWidget *wid = new MyTabWidget(QCONTAINER(VARG(parent))); //, 0, Qt::WNoMousePropagation);

  QObject::connect(wid, SIGNAL(currentChanged(int)), &CTabStrip::manager, SLOT(currentChanged(int)));

  THIS->container = NULL;
  THIS->index = -1;

  CWIDGET_new(wid, (void *)_object, true);
  set_tab_count(THIS, 1);
  wid->show();

  //wid->updateLayout();

END_METHOD



BEGIN_PROPERTY(CTABSTRIP_count)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->stack.count());
  else
    set_tab_count(THIS, VPROP(GB_INTEGER));

END_PROPERTY


static bool check_index(CTABSTRIP *_object, int index)
{
  if (index < 0 || index >= (int)WIDGET->stack.count())
  {
    GB.Error("Bad index");
    return true;
  }
  else
    return false;
}

static int get_real_index(CTABSTRIP *_object)
{
	int i;
	QWidget *current = WIDGET->currentWidget();

	for (i = 0; i < (int)WIDGET->stack.count(); i++)
	{
		if (WIDGET->stack.at(i)->widget == current)
			return i;
	}
	
	return -1;
}

BEGIN_PROPERTY(CTABSTRIP_index)

  if (READ_PROPERTY)
  {
    GB.ReturnInteger(get_real_index(THIS));
	}
  else
  {
    int index = VPROP(GB_INTEGER);

    if (check_index(THIS, index))
      return;

    if (index == get_real_index(THIS))
      return;

		if (!WIDGET->stack.at(index)->isVisible())
			return;
			
    WIDGET->stack.at(index)->ensureVisible();
  }

END_PROPERTY


BEGIN_PROPERTY(CTABSTRIP_current)

  THIS->index = get_real_index(THIS);
  RETURN_SELF();

END_PROPERTY


BEGIN_METHOD(CTABSTRIP_get, GB_INTEGER index)

  int index = VARG(index);

  if (check_index(THIS, index))
    return;

  THIS->index = index;
  RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CTABSTRIP_orientation)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->tabPosition() == QTabWidget::North ? ALIGN_TOP : ALIGN_BOTTOM);
  else
    WIDGET->setTabPosition(VPROP(GB_INTEGER) == ALIGN_BOTTOM ? QTabWidget::South : QTabWidget::North);

END_PROPERTY



/***************************************************************************

  .Tab

***************************************************************************/

static int get_page_index(CTABSTRIP *_object)
{
  int index = THIS->index;

  if (index < 0)
    return get_real_index(THIS);

  return WIDGET->stack.at(index)->index();
}

BEGIN_PROPERTY(CTAB_text)

  int index = get_page_index(THIS);

  if (READ_PROPERTY)
  {
  	if (index >= 0)
    	GB.ReturnNewZeroString(TO_UTF8(WIDGET->tabText(index)));
		else
			GB.ReturnNull();
	}
  else
  {
  	if (index >= 0)
    	WIDGET->setTabText(index, QSTRING_PROP());
	}

END_PROPERTY


BEGIN_PROPERTY(CTAB_picture)

  int index;

  index = THIS->index;
  if (index < 0)
    index = get_real_index(THIS);
    
  if (READ_PROPERTY)
  {
  	if (index < 0)
  		GB.ReturnNull();
		else
    	GB.ReturnObject(WIDGET->stack.at(index)->icon);
	}
  else if (index >= 0)
  {
    CPICTURE *pict;

    GB.StoreObject(PROP(GB_OBJECT), POINTER(&(WIDGET->stack.at(index))->icon));
    pict = (CPICTURE *)VPROP(GB_OBJECT);
		WIDGET->stack.at(index)->updateIcon();
  }

END_PROPERTY


BEGIN_PROPERTY(CTAB_enabled)

  CTab *tab = WIDGET->stack.at(THIS->index);

  if (READ_PROPERTY)
    GB.ReturnBoolean(tab->isEnabled());
  else
    tab->setEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTAB_visible)

  CTab *tab = WIDGET->stack.at(THIS->index);

  if (READ_PROPERTY)
    GB.ReturnBoolean(tab->isVisible());
  else
    tab->setVisible(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD_VOID(CTAB_next)

  CTABSTRIP_ENUM *iter;
  QWidget *page;
  QObjectList list;
  int child;
  CWIDGET *widget;

  iter = (CTABSTRIP_ENUM *)GB.GetEnum();
  if (!iter->init)
  {
    iter->child = 0;
    iter->index = THIS->index;
    iter->init = true;
  }

  //qDebug("CTAB_next: iter = (%d, %d, %d)", iter->init, iter->index, iter->child);

  page = WIDGET->stack.at(iter->index)->widget;
  list = page->children();

  for(;;)
  {
    child = iter->child;

    if (child >= list.count())
    {
      GB.StopEnum();
      return;
    }

    iter->child = child + 1;

    widget = CWidget::getRealExisting(list.at(child));
    if (widget)
    {
      GB.ReturnObject(widget);
      return;
    }
  }

END_METHOD

BEGIN_METHOD(CTAB_get, GB_INTEGER index)

	QObjectList list = WIDGET->stack.at(THIS->index)->widget->children();
	int index = VARG(index);
	int i;
	CWIDGET *widget;

	if (index >= 0)
	{
		i = 0;
		for(i = 0; i < list.count(); i++)
		{
			widget = CWidget::getRealExisting(list.at(i));
			if (!widget)
				continue;
			if (index == 0)
			{
				GB.ReturnObject(widget);
				return;
			}
			index--;
		}
	}

	GB.Error(GB_ERR_BOUND);

END_METHOD


BEGIN_PROPERTY(CTAB_count)

	GB.ReturnInteger(WIDGET->stack.at(THIS->index)->count());

END_PROPERTY


BEGIN_METHOD_VOID(CTAB_delete)

	int index = get_real_index(THIS);

	if (WIDGET->stack.count() == 1)
	{
		GB.Error("TabStrip cannot be empty");
		return;
	}

	if (remove_page(THIS, THIS->index))
		return;

	if (index >= (int)WIDGET->stack.count())
		index = WIDGET->stack.count() - 1;

  WIDGET->stack.at(index)->ensureVisible();

  THIS->index = -1;

END_METHOD


BEGIN_PROPERTY(CTABSTRIP_enabled)

  int i;

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isEnabled());
  else
  {
    WIDGET->setEnabled(VPROP(GB_BOOLEAN));
    for (i = 0; i < WIDGET->stack.count(); i++)
      WIDGET->stack.at(i)->setEnabled(VPROP(GB_BOOLEAN));
  }

END_PROPERTY


BEGIN_PROPERTY(CTABSTRIP_text)

  THIS->index = -1;
  CTAB_text(_object, _param);

  /*if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->tabLabel(WIDGET->currentPage())));
  else
    WIDGET->changeTab(WIDGET->currentPage(), QSTRING_PROP());*/

END_PROPERTY


BEGIN_PROPERTY(CTABSTRIP_picture)

  THIS->index = -1;
  CTAB_picture(_object, _param);

  /*if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->tabLabel(WIDGET->currentPage())));
  else
    WIDGET->changeTab(WIDGET->currentPage(), QSTRING_PROP());*/

END_PROPERTY


BEGIN_PROPERTY(CTABSTRIP_client_x)

	WIDGET->forceLayout();
	CCONTAINER_x(_object, _param);

END_PROPERTY



BEGIN_PROPERTY(CTABSTRIP_client_y)

	WIDGET->forceLayout();
	CCONTAINER_y(_object, _param);

END_PROPERTY


BEGIN_PROPERTY(CTABSTRIP_client_width)

	WIDGET->forceLayout();
  GB.ReturnInteger(THIS->container->width());

END_PROPERTY


BEGIN_PROPERTY(CTABSTRIP_client_height)

	WIDGET->forceLayout();
  GB.ReturnInteger(THIS->container->height());

END_PROPERTY



/** Class CTabStrip ******************************************************/


CTabStrip CTabStrip::manager;

void CTabStrip::currentChanged(int index)
{
	QWidget *wid;
  GET_SENDER();

	wid = WIDGET->currentWidget();

  //qDebug("CTabStrip::currentChanged: %d %p -> %p", index, THIS->container, wid);

	if (wid != THIS->container)
	{
		if (THIS->container)
			THIS->container->hide();
		THIS->container = wid;
		wid->show();
		WIDGET->forceLayout();
		CCONTAINER_arrange(THIS);

		//if (wid->isVisible() && !THIS->lock)
		if (!THIS->lock)
			RAISE_EVENT(EVENT_Click);
	}
}


/** Descriptions *********************************************************/

GB_DESC CTabChildrenDesc[] =
{
  GB_DECLARE(".TabChildren", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_next", "Control", CTAB_next, NULL),
  GB_PROPERTY_READ("Count", "i", CTAB_count),
	GB_METHOD("_get", "Control", CTAB_get, "(Index)i"),

  GB_END_DECLARE
};


GB_DESC CTabDesc[] =
{
  GB_DECLARE(".Tab", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", CTAB_text),
  GB_PROPERTY("Picture", "Picture", CTAB_picture),
  GB_PROPERTY("Caption", "s", CTAB_text),
  GB_PROPERTY("Enabled", "b", CTAB_enabled),
  GB_PROPERTY("Visible", "b", CTAB_visible),
  GB_PROPERTY_SELF("Children", ".TabChildren"),
  GB_METHOD("Delete", NULL, CTAB_delete, NULL),

  GB_END_DECLARE
};


GB_DESC CTabStripDesc[] =
{
  GB_DECLARE("TabStrip", sizeof(CTABSTRIP)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CTABSTRIP_new, "(Parent)Container;"),

  GB_PROPERTY("Count", "i", CTABSTRIP_count),
  GB_PROPERTY("Text", "s", CTABSTRIP_text),
  GB_PROPERTY("Picture", "Picture", CTABSTRIP_picture),
  GB_PROPERTY("Caption", "s", CTABSTRIP_text),
  GB_PROPERTY_READ("Current", ".Tab", CTABSTRIP_current),
  GB_PROPERTY("Index", "i", CTABSTRIP_index),
  GB_PROPERTY("Orientation", "i", CTABSTRIP_orientation),
  GB_PROPERTY("Enabled", "b", CTABSTRIP_enabled),

	#ifdef GB_QT_COMPONENT
  GB_PROPERTY_READ("ClientX", "i", CTABSTRIP_client_x),
  GB_PROPERTY_READ("ClientY", "i", CTABSTRIP_client_y),
  GB_PROPERTY_READ("ClientW", "i", CTABSTRIP_client_width),
  GB_PROPERTY_READ("ClientWidth", "i", CTABSTRIP_client_width),
  GB_PROPERTY_READ("ClientH", "i", CTABSTRIP_client_height),
  GB_PROPERTY_READ("ClientHeight", "i", CTABSTRIP_client_height),
  #endif

  GB_PROPERTY("Arrangement", "i", CCONTAINER_arrangement),
  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),
  GB_PROPERTY("Spacing", "b", CCONTAINER_spacing),
  GB_PROPERTY("Margin", "b", CCONTAINER_margin),
  GB_PROPERTY("Indent", "b", CCONTAINER_indent),

  GB_METHOD("_get", ".Tab", CTABSTRIP_get, "(Index)i"),

	TABSTRIP_DESCRIPTION,

  GB_EVENT("Click", NULL, NULL, &EVENT_Click),

  GB_END_DECLARE
};

