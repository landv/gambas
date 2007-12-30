/***************************************************************************

  CTabStrip.cpp

  The TabStrip class

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

#define __CTABSTRIP_CPP



#include <qapplication.h>
#include <qframe.h>
#include <qtabbar.h>
#include <qtabwidget.h>

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
	
	CTab(CTABSTRIP *parent, QWidget *page);
	~CTab();
	
	bool isEmpty() { return widget->children() == NULL; }
	void ensureVisible() { WIDGET->showPage(widget); }
	void setEnabled(bool e) { WIDGET->setTabEnabled(widget, e); }
	bool isEnabled() { return WIDGET->isTabEnabled(widget); }
	bool isVisible() { return visible; }
	void setVisible(bool v);
	void updateIcon();

private:
	CTABSTRIP *_object;
};

CTab::CTab(CTABSTRIP *parent, QWidget *page)
{ 
	_object = parent;
	widget = page; 
	icon = 0; 
	THIS->id++;
	id = THIS->id;
	visible = true; 
  setEnabled(WIDGET->isEnabled());
}	

CTab::~CTab()
{
	GB.Unref((void **)&icon);
}

void CTab::setVisible(bool v)
{
	int i;
	
	if (v == visible)
		return;
		
	visible = v;
	
	if (!visible)
	{
		text = WIDGET->tabLabel(widget);
		WIDGET->removePage(widget);
	}
	else
	{
		for (i = 0; i < (int)THIS->stack->count(); i++)
		{
			if (id == THIS->stack->at(i)->id)
				break;
		}
		WIDGET->insertTab(widget, text, i);
	  setEnabled(WIDGET->isEnabled());
		updateIcon();
	}
}


void CTab::updateIcon()
{
	QIconSet iconset;
	
	if (icon)
		CWIDGET_iconset(iconset, *(icon->pixmap));
	
	WIDGET->setTabIconSet(widget, iconset);
}

/** MyTabWidget **********************************************************/

void MyTabWidget::setEnabled(bool e)
{
	CWIDGET *_object = CWidget::get(this);
	int i;
	
	QTabWidget::setEnabled(e);
	
	for (i = 0; i < THIS->stack->count(); i++)
		THIS->stack->at(i)->widget->setEnabled(e);
}

void MyTabWidget::forceLayout()
{
	bool b = isVisible();
		
	setWState(WState_Visible);
	qApp->sendEvent(this, new QShowEvent());
	if (!b)
		clearWState(WState_Visible);
	//qDebug("Y = %d", tabBar()->height());
}


/***************************************************************************

  TabStrip

***************************************************************************/

static bool remove_page(void *_object, int i)
{
	CTab *tab = THIS->stack->at(i);

	if (!tab->isEmpty())
	{
		GB.Error("Tab is not empty");
		return true;
	}

	THIS->lock = true;
	WIDGET->removePage(tab->widget);
	delete tab->widget;
	THIS->stack->remove(i);
	THIS->lock = false;

	return false;
}

static bool set_tab_count(void *_object, long new_count)
{
  long count = THIS->stack->count();
  long i;
  long index;
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

      label.sprintf("Tab %ld", i);
      WIDGET->insertTab(tab->widget, label);
      
      THIS->stack->append(tab);
    }

    index = new_count - 1;

		THIS->stack->at(index)->ensureVisible();
		THIS->container = THIS->stack->at(index)->widget;    
  }
  else
  {
    index = WIDGET->currentPageIndex();
    //same = (id == PARAM(index));

    for (i = new_count; i < count; i++)
    {
      if (!THIS->stack->at(i)->isEmpty())
      {
        GB.Error("Tab is not empty");
        return true;
      }
    }

		if (index >= new_count)
			index = new_count - 1;
	
		THIS->stack->at(index)->ensureVisible();
		THIS->container = THIS->stack->at(index)->widget;
    
    for (i = count - 1; i >= new_count; i--)
    {
			remove_page(THIS, i);
    }

    //THIS->stack->resize(new_count);
    //THIS->icon->resize(new_count);
  }

  return false;
}


BEGIN_METHOD(CTABSTRIP_new, GB_OBJECT parent)

  MyTabWidget *wid = new MyTabWidget(QCONTAINER(VARG(parent))); //, 0, Qt::WNoMousePropagation);

  CWIDGET_new(wid, (void *)_object, "TabStrip");

  THIS->container = NULL;
  THIS->index = -1;
  THIS->stack = new QPtrList<CTab>;
  THIS->stack->setAutoDelete(true);

  QObject::connect(wid, SIGNAL(currentChanged(QWidget *)), &CTabStrip::manager, SLOT(currentChanged(QWidget *)));

  set_tab_count(THIS, 1);

  wid->show();
  //wid->updateLayout();

END_METHOD


BEGIN_METHOD_VOID(CTABSTRIP_free)

  delete OBJECT(CTABSTRIP)->stack;

END_METHOD


BEGIN_PROPERTY(CTABSTRIP_count)

  if (READ_PROPERTY)
    GB.ReturnInteger(THIS->stack->count());
  else
    set_tab_count(THIS, VPROP(GB_INTEGER));

END_PROPERTY


static bool check_index(CTABSTRIP *_object, long index)
{
  if (index < 0 || index >= (long)THIS->stack->count())
  {
    GB.Error("Bad index");
    return true;
  }
  else
    return false;
}

BEGIN_PROPERTY(CTABSTRIP_index)

  if (READ_PROPERTY)
    GB.ReturnInteger(WIDGET->currentPageIndex());
  else
  {
    long index = VPROP(GB_INTEGER);

    if (check_index(THIS, index))
      return;

    if (index == WIDGET->currentPageIndex())
      return;

    THIS->stack->at(index)->ensureVisible();
  }

END_PROPERTY


BEGIN_PROPERTY(CTABSTRIP_current)

  THIS->index = WIDGET->currentPageIndex();
  RETURN_SELF();

END_PROPERTY


BEGIN_METHOD(CTABSTRIP_get, GB_INTEGER index)

  long index = VARG(index);

  if (check_index(THIS, index))
    return;

  THIS->index = index;
  RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CTABSTRIP_orientation)

  if (READ_PROPERTY)
    GB.ReturnInteger(CCONST_convert(WIDGET->tabPosition(), 2, QTabWidget::Top, 0, QTabWidget::Bottom, 1));
  else
    WIDGET->setTabPosition((QTabWidget::TabPosition)CCONST_convert(VPROP(GB_INTEGER), -2, QTabWidget::Top, 0, QTabWidget::Bottom, 1));

END_PROPERTY



/***************************************************************************

  .Tab

***************************************************************************/

static QWidget *get_page(CTABSTRIP *_object)
{
  int index = THIS->index;

  if (index < 0)
    return (QFrame *)WIDGET->currentPage();

  THIS->index = (-1);
  return THIS->stack->at(index)->widget;
}


BEGIN_PROPERTY(CTAB_text)

  QWidget *page = get_page(THIS);

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(WIDGET->tabLabel(page)));
  else
  {
    WIDGET->changeTab(page, QSTRING_PROP());
	}

END_PROPERTY


BEGIN_PROPERTY(CTAB_picture)

  int index;
  QWidget *page;

  index = THIS->index;
  if (index < 0)
    index = WIDGET->currentPageIndex();

  page = get_page(THIS);

  if (READ_PROPERTY)
    GB.ReturnObject(THIS->stack->at(index)->icon);
  else
  {
    CPICTURE *pict;

    GB.StoreObject(PROP(GB_OBJECT), (void **)&(THIS->stack->at(index)->icon));
    pict = (CPICTURE *)VPROP(GB_OBJECT);
		THIS->stack->at(index)->updateIcon();
  }

END_PROPERTY


BEGIN_PROPERTY(CTAB_enabled)

  QWidget *page = get_page(THIS);

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isTabEnabled(page));
  else
    WIDGET->setTabEnabled(page, VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTAB_visible)

  CTab *tab = THIS->stack->at(THIS->index);

  if (READ_PROPERTY)
    GB.ReturnBoolean(tab->isVisible());
  else
    tab->setVisible(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD_VOID(CTAB_next)

  CTABSTRIP_ENUM *iter;
  QWidget *page;
  QObjectList *list;
  unsigned int child;
  CWIDGET *widget;

  iter = (CTABSTRIP_ENUM *)GB.GetEnum();
  if (!iter->init)
  {
    iter->child = 0;
    iter->index = THIS->index;
    iter->init = true;
  }

  //qDebug("CTAB_next: iter = (%d, %d, %d)", iter->init, iter->index, iter->child);

  page = THIS->stack->at(iter->index)->widget;
  list = (QObjectList *)page->children();

  for(;;)
  {
    child = iter->child;

    if (list == NULL || child >= list->count())
    {
      GB.StopEnum();
      return;
    }

    iter->child = child + 1;

    widget = CWidget::getReal(list->at(child));
    if (widget)
    {
      GB.ReturnObject(widget);
      return;
    }
  }

END_METHOD


BEGIN_PROPERTY(CTAB_count)

  QWidget *page = THIS->stack->at(THIS->index)->widget;

  if (page->children() == NULL)
    GB.ReturnInteger(0);
  else
    GB.ReturnInteger(page->children()->count());

END_PROPERTY


BEGIN_METHOD_VOID(CTAB_delete)

	int index = WIDGET->currentPageIndex();

	if (THIS->stack->count() == 1)
	{
		GB.Error("TabStrip cannot be empty");
		return;
	}

	if (remove_page(THIS, THIS->index))
		return;

	if (index >= (int)THIS->stack->count())
		index = THIS->stack->count() - 1;

  THIS->stack->at(index)->ensureVisible();

  THIS->index = -1;

END_METHOD


BEGIN_PROPERTY(CTABSTRIP_enabled)

  uint i;

  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isEnabled());
  else
  {
    WIDGET->setEnabled(VPROP(GB_BOOLEAN));
    for (i = 0; i < THIS->stack->count(); i++)
      THIS->stack->at(i)->setEnabled(VPROP(GB_BOOLEAN));
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

void CTabStrip::currentChanged(QWidget *wid)
{
  GET_SENDER(_object);

  //qDebug("CTabStrip::currentChanged: %p -> %p", THIS->container, wid);

	if (wid != THIS->container)
	{
		THIS->container = wid;
		CCONTAINER_arrange(THIS);

		//if (wid->isVisible() && !THIS->lock)
		if (!THIS->lock)
			RAISE_EVENT(EVENT_Click);
	}
}


/** Descriptions *********************************************************/

#include "CTabStrip_desc.h"

