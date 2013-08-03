/***************************************************************************

  CTabStrip.cpp

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

#define __CTABSTRIP_CPP

#include <qapplication.h>
#include <qtabbar.h>
#include <QShowEvent>
#include <QWheelEvent>
#include <QEvent>
#include <QStackedWidget>
#include <QTabWidget>

#include "gb_common.h"
#include "gambas.h"

#include "CPicture.h"
#include "CConst.h"
#include "CTabStrip.h"


DECLARE_METHOD(Container_X);
DECLARE_METHOD(Container_Y);

DECLARE_EVENT(EVENT_Click);
DECLARE_EVENT(EVENT_Close);

void CTABSTRIP_arrange(void *_object)
{
	WIDGET->layoutContainer();
}

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
	void ensureVisible();
	void setEnabled(bool e);
	bool isEnabled() const { return enabled; }
	bool isVisible() const { return visible; }
	void setVisible(bool v);
	void updateIcon();
	void updateText();
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
	
	//page->setAutoFillBackground(true);
	page->hide();
}	

CTab::~CTab()
{
	GB.Unref(POINTER(&icon));
}

void CTab::ensureVisible()
{
	setVisible(true);
	int i = index();
	if (i >= 0)
	{
		WIDGET->setCurrentIndex(i);
		if (!WIDGET->isVisible())
			WIDGET->layoutContainer();
	}
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
		i = index();
		if (i >= 0)
		{
			text = WIDGET->tabText(i);
			WIDGET->removeTab(i);
		}
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

void CTab::setEnabled(bool e)
{ 
	int i = index();
	enabled = e;
	if (i >= 0)
		WIDGET->setTabEnabled(i, e && WIDGET->isEnabled()); 
}

void CTab::updateIcon()
{
	int i = index();
	QIcon iconset;
	
	if (icon)
		CWIDGET_iconset(iconset, *(icon->pixmap));

	if (i >= 0)
		WIDGET->setTabIcon(i, iconset);
}

void CTab::updateText()
{
	int i = index();
	if (i >= 0)
		WIDGET->setTabText(i, text);
}

/** MyTabWidget **********************************************************/

MyTabWidget::MyTabWidget(QWidget *parent) : QTabWidget(parent)
{
	_oldw = _oldh = 0;
	//setMovable(true);
	//tabBar()->installEventFilter(this);
}

MyTabWidget::~MyTabWidget()
{
	int i;
	CWIDGET *_object = CWidget::getReal(this);
	
	for (i = 0; i < stack.count(); i++)
		delete stack.at(i);

	CWIDGET_set_flag(THIS, WF_DELETED);
}

void MyTabWidget::setEnabled(bool e)
{
	CWIDGET *_object = CWidget::get(this);
	int i;
	
	QTabWidget::setEnabled(e);
	
	for (i = 0; i < (int)WIDGET->stack.count(); i++)
		WIDGET->stack.at(i)->widget->setEnabled(e);
}

void MyTabWidget::layoutContainer()
{
	CWIDGET *_object = CWidget::get(this);
	#if QT_VERSION >= 0x040600
	QStyleOptionTabWidgetFrameV2 option;
	#else
	QStyleOptionTabWidgetFrame option;
	#endif
	QWidget *w = findChild<QStackedWidget *>();
	QRect contentsRect;

	if (_oldw != width() || _oldh != height())
	{
		initStyleOption(&option);
		contentsRect = style()->subElementRect(QStyle::SE_TabWidgetTabContents, &option, this);
		_oldw = width();
		_oldh = height();
		w->setGeometry(contentsRect);
	}
	else
		contentsRect = w->geometry();

	if (THIS->container)
		THIS->container->setGeometry(0, 0, contentsRect.width(), contentsRect.height());
}

void MyTabWidget::updateTextFont()
{
	CWIDGET *_object = CWidget::get(this);
	if (THIS->textFont)
		tabBar()->setFont(*THIS->textFont->font);
	else
		tabBar()->setFont(QFont());
}

#if 0
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
#endif

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
	WIDGET->stack.removeAt(i);
	
	i = tab->index();
	if (i >= 0)
		WIDGET->removeTab(i);
	
	delete tab->widget;
	delete tab;
	THIS->lock = false;

	return false;
}

static void set_current_index(void *_object, int index)
{
	if (index < 0)
		return;
	
	if (index >= (int)WIDGET->stack.count())
		index = WIDGET->stack.count() - 1;

	while (index > 0 && !WIDGET->stack.at(index)->isVisible())
		index--;

	WIDGET->stack.at(index)->ensureVisible();
	THIS->container = WIDGET->stack.at(index)->widget;
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

		set_current_index(THIS, index);
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
	
		set_current_index(THIS, index);
		
		for (i = count - 1; i >= new_count; i--)
		{
			remove_page(THIS, i);
		}

		//WIDGET->stack.resize(new_count);
		//THIS->icon->resize(new_count);
	}

	return false;
}

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


BEGIN_METHOD(TabStrip_new, GB_OBJECT parent)

	MyTabWidget *wid = new MyTabWidget(QCONTAINER(VARG(parent))); //, 0, Qt::WNoMousePropagation);

	QObject::connect(wid, SIGNAL(currentChanged(int)), &CTabStrip::manager, SLOT(currentChanged(int)));
	QObject::connect(wid, SIGNAL(tabCloseRequested(int)), &CTabStrip::manager, SLOT(tabCloseRequested(int)));

	//THIS->widget.flag.fillBackground = TRUE;
	THIS->container = NULL;
	THIS->index = -1;

	CWIDGET_new(wid, (void *)_object);
	set_tab_count(THIS, 1);

	//wid->updateLayout();

END_METHOD


BEGIN_METHOD_VOID(TabStrip_free)

	GB.Unref(POINTER(&THIS->textFont));

END_METHOD


BEGIN_PROPERTY(TabStrip_Count)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET->stack.count());
	else
		set_tab_count(THIS, VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(TabStrip_Index)

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


BEGIN_PROPERTY(TabStrip_Current)

	THIS->index = get_real_index(THIS);
	if (THIS->index < 0)
		GB.ReturnNull();
	else
		RETURN_SELF();

END_PROPERTY


BEGIN_METHOD(TabStrip_get, GB_INTEGER index)

	int index = VARG(index);

	if (check_index(THIS, index))
		return;

	THIS->index = index;
	RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(TabStrip_Orientation)

	if (READ_PROPERTY) 
	{
		switch(WIDGET->tabPosition())
		{
			case QTabWidget::North: GB.ReturnInteger(ALIGN_TOP); break;
			case QTabWidget::South: GB.ReturnInteger(ALIGN_BOTTOM); break;
			case QTabWidget::West: GB.ReturnInteger(ALIGN_LEFT); break;
			case QTabWidget::East: GB.ReturnInteger(ALIGN_RIGHT); break;
			default: GB.ReturnInteger(ALIGN_NORMAL); break;
		}
	}
	else
	{
		switch(VPROP(GB_INTEGER))
		{
			case ALIGN_TOP: WIDGET->setTabPosition(QTabWidget::North); break;
			case ALIGN_BOTTOM: WIDGET->setTabPosition(QTabWidget::South); break;
			case ALIGN_LEFT: WIDGET->setTabPosition(QTabWidget::West); break;
			case ALIGN_RIGHT: WIDGET->setTabPosition(QTabWidget::East); break;
		}
	}
	
END_PROPERTY



/***************************************************************************

	.Tab

***************************************************************************/

BEGIN_PROPERTY(CTAB_text)

	int index = THIS->index;
	if (index < 0)
		index = get_real_index(THIS);

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(WIDGET->stack.at(index)->text));
	else
	{
		WIDGET->stack.at(index)->text = QSTRING_PROP();
		WIDGET->stack.at(index)->updateText();
	}

END_PROPERTY


BEGIN_PROPERTY(TabStrip_TextFont)

	if (READ_PROPERTY)
		GB.ReturnObject(THIS->textFont);
	else
	{
		GB.StoreObject(PROP(GB_OBJECT), POINTER(&THIS->textFont));
		WIDGET->updateTextFont();
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
		GB.StoreObject(PROP(GB_OBJECT), POINTER(&(WIDGET->stack.at(index))->icon));
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

	set_current_index(THIS, index);
	
	THIS->index = -1;

END_METHOD


BEGIN_PROPERTY(TabStrip_Enabled)

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


BEGIN_PROPERTY(TabStrip_Text)

	THIS->index = -1;
	CTAB_text(_object, _param);

	/*if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(WIDGET->tabLabel(WIDGET->currentPage())));
	else
		WIDGET->changeTab(WIDGET->currentPage(), QSTRING_PROP());*/

END_PROPERTY


BEGIN_PROPERTY(TabStrip_Picture)

	THIS->index = -1;
	CTAB_picture(_object, _param);

END_PROPERTY

BEGIN_PROPERTY(TabStrip_Closable)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WIDGET->tabsClosable());
	else
		WIDGET->setTabsClosable(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(TabStrip_ClientX)

	Container_X(_object, _param);

END_PROPERTY


BEGIN_PROPERTY(TabStrip_ClientY)

	Container_Y(_object, _param);

END_PROPERTY


BEGIN_PROPERTY(TabStrip_ClientWidth)

	GB.ReturnInteger(THIS->container->width());

END_PROPERTY


BEGIN_PROPERTY(TabStrip_ClientHeight)

	GB.ReturnInteger(THIS->container->height());

END_PROPERTY


BEGIN_METHOD(TabStrip_FindIndex, GB_OBJECT child)

	void *child = VARG(child);
	int i;
	QWidget *parent;
	
	if (GB.CheckObject(child))
		return;
	
	parent = QWIDGET(child)->parentWidget();
	
	for (i = 0; i < WIDGET->stack.count(); i++)
	{
		if (parent == WIDGET->stack.at(i)->widget)
		{
			GB.ReturnInteger(i);
			return;
		}
	}
	
	GB.ReturnInteger(-1);

END_METHOD


/** Class CTabStrip ******************************************************/


CTabStrip CTabStrip::manager;

void CTabStrip::currentChanged(int index)
{
	QWidget *wid;
	GET_SENDER();

	wid = WIDGET->currentWidget(); // wid can be null!

	if (wid != THIS->container)
	{
		//qDebug("CTabStrip::currentChanged: %d %p -> %p", index, THIS->container, wid);
		if (THIS->container)
			THIS->container->hide();
		THIS->container = wid;
		
		if (wid)
			wid->show();
		
		CCONTAINER_arrange(THIS);

		//if (wid->isVisible() && !THIS->lock)
		if (!THIS->lock)
			RAISE_EVENT(EVENT_Click);
	}
}

void CTabStrip::tabCloseRequested(int index)
{
	GET_SENDER();

	GB.Raise(THIS, EVENT_Close, 1, GB_T_INTEGER, index);
}


/** Descriptions *********************************************************/

GB_DESC CTabStripContainerChildrenDesc[] =
{
	GB_DECLARE_VIRTUAL(".TabStripContainer.Children"),

	GB_METHOD("_next", "Control", CTAB_next, NULL),
	GB_PROPERTY_READ("Count", "i", CTAB_count),
	GB_METHOD("_get", "Control", CTAB_get, "(Index)i"),

	GB_END_DECLARE
};


GB_DESC CTabStripContainerDesc[] =
{
	GB_DECLARE_VIRTUAL(".TabStripContainer"),

	GB_PROPERTY("Text", "s", CTAB_text),
	GB_PROPERTY("Picture", "Picture", CTAB_picture),
	GB_PROPERTY("Caption", "s", CTAB_text),
	GB_PROPERTY("Enabled", "b", CTAB_enabled),
	GB_PROPERTY("Visible", "b", CTAB_visible),
	GB_PROPERTY_SELF("Children", ".TabStripContainer.Children"),
	GB_METHOD("Delete", NULL, CTAB_delete, NULL),

	GB_END_DECLARE
};


GB_DESC CTabStripDesc[] =
{
	GB_DECLARE("TabStrip", sizeof(CTABSTRIP)), GB_INHERITS("Container"),

	GB_METHOD("_new", NULL, TabStrip_new, "(Parent)Container;"),
	GB_METHOD("_free", NULL, TabStrip_free, NULL),

	GB_PROPERTY("Count", "i", TabStrip_Count),
	GB_PROPERTY("Text", "s", TabStrip_Text),
	GB_PROPERTY("TextFont", "Font", TabStrip_TextFont),
	GB_PROPERTY("Picture", "Picture", TabStrip_Picture),
  GB_PROPERTY("Closable", "b", TabStrip_Closable),
	GB_PROPERTY("Caption", "s", TabStrip_Text),
	GB_PROPERTY_READ("Current", ".TabStripContainer", TabStrip_Current),
	GB_PROPERTY("Index", "i", TabStrip_Index),
	GB_PROPERTY("Orientation", "i", TabStrip_Orientation),
	GB_PROPERTY("Enabled", "b", TabStrip_Enabled),

	GB_PROPERTY_READ("ClientX", "i", TabStrip_ClientX),
	GB_PROPERTY_READ("ClientY", "i", TabStrip_ClientY),
	GB_PROPERTY_READ("ClientW", "i", TabStrip_ClientWidth),
	GB_PROPERTY_READ("ClientWidth", "i", TabStrip_ClientWidth),
	GB_PROPERTY_READ("ClientH", "i", TabStrip_ClientHeight),
	GB_PROPERTY_READ("ClientHeight", "i", TabStrip_ClientHeight),

	GB_PROPERTY("Arrangement", "i", Container_Arrangement),
	GB_PROPERTY("AutoResize", "b", Container_AutoResize),
	GB_PROPERTY("Padding", "i", Container_Padding),
	GB_PROPERTY("Spacing", "b", Container_Spacing),
	GB_PROPERTY("Margin", "b", Container_Margin),
	GB_PROPERTY("Indent", "b", Container_Indent),
  GB_PROPERTY("Invert", "b", Container_Invert),

	GB_METHOD("_get", ".TabStripContainer", TabStrip_get, "(Index)i"),
	GB_METHOD("FindIndex", "i", TabStrip_FindIndex, "(Child)Control;"),

	TABSTRIP_DESCRIPTION,

	GB_EVENT("Click", NULL, NULL, &EVENT_Click),
	GB_EVENT("Close", NULL, "(Index)i", &EVENT_Close),

	GB_END_DECLARE
};

