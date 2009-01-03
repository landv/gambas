/***************************************************************************

  CMenu.cpp

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

#define __CMENU_CPP

#include <QMenuBar>
#include <QMenu>

#include "gambas.h"

#include "CWidget.h"
#include "CWindow.h"
#include "CMenu.h"

DECLARE_EVENT(EVENT_Click);
DECLARE_EVENT(EVENT_Show);
DECLARE_EVENT(EVENT_Hide);

DECLARE_METHOD(CCONTROL_window);
DECLARE_METHOD(CCONTROL_name);

static int check_menu(void *_object)
{
  return THIS->deleted;
}

static void refresh_menubar(CMENU *menu)
{
	int i;
	QList<QAction *> list;
	MyMainWindow *toplevel;
	QMenuBar *menuBar;
	
	if (!GB.Is(menu->parent, CLASS_Window))
		return;
	
	toplevel = (MyMainWindow *)(menu->toplevel);
	menuBar = ((CWINDOW *)(menu->parent))->menuBar;
	list = menuBar->actions();
	 
	for (i = 0; i < list.count(); i++)
	{
		if (list.at(i)->isVisible())
			break;
	}
	
	if (i < list.count())
		menuBar->show();
	else
		menuBar->hide();
	
	toplevel->configure();
}

static void unregister_menu(CMENU *_object)
{
	CACTION_register((CWIDGET *)THIS, NULL);
}

static void delete_menu(CMENU *_object)
{
	if (THIS->deleted)
		return;
		
	//qDebug("delete_menu: THIS = %p", _object);
		
	if (THIS->menu)
	{
		delete THIS->menu;
		THIS->menu = 0;
	}

  CMenu::dict.remove(ACTION);

	//delete ACTION;
	THIS->widget.widget = 0;

	THIS->deleted = true;
}

static void toggle_menu(CMENU *_object)
{
	if (CMENU_is_toplevel(THIS))
		return;

	//qDebug("toggle_menu: %s", THIS->text);

	ACTION->setChecked(!ACTION->isChecked());
}


BEGIN_METHOD(CMENU_new, GB_OBJECT parent; GB_BOOLEAN hidden)

	QAction *action;
  void *parent = VARG(parent);
  QWidget *topLevel = 0;
  QMenuBar *menuBar = 0;

  //printf("CMENU_new %p\n", _object);

  if (GB.CheckObject(parent))
    return;

	//qDebug("CMENU_new: (%s %p)", GB.GetClassName(THIS), THIS);
	
  if (GB.Is(parent, CLASS_Menu))
  {
  	CMENU *menu = (CMENU *)parent;
  	
    topLevel = menu->toplevel;
    
    if (!menu->menu)
    {
    	menu->menu = new QMenu();
    	menu->menu->setSeparatorsCollapsible(true);
    	((QAction *)(menu->widget.widget))->setMenu(menu->menu);
	
			QObject::connect(menu->menu, SIGNAL(triggered(QAction *)), &CMenu::manager, SLOT(slotTriggered(QAction *)));
			QObject::connect(menu->menu, SIGNAL(aboutToShow()), &CMenu::manager, SLOT(slotShown()));
			QObject::connect(menu->menu, SIGNAL(aboutToHide()), &CMenu::manager, SLOT(slotHidden()));
    }
    
		action = new QAction(menu->menu);
		action->setSeparator(true);
		action->setVisible(!VARGOPT(hidden, FALSE));
		QObject::connect(action, SIGNAL(destroyed()), &CMenu::manager, SLOT(slotDestroyed()));
    
    menu->menu->addAction(action);
    //qDebug("New action %p for Menu %p", action, THIS);
  }
  else if (GB.Is(parent, CLASS_Window))
  {
    CWINDOW *window = (CWINDOW *)parent;
  	
    topLevel = QWIDGET(CWidget::getWindow((CWIDGET *)parent));
    menuBar = window->menuBar;
    if (!menuBar)
    {
      menuBar = new QMenuBar(topLevel);
      window->menuBar = menuBar;
    }
    
		action = new QAction(menuBar);
		action->setSeparator(true);
		action->setVisible(!VARGOPT(hidden, FALSE));
		QObject::connect(action, SIGNAL(destroyed()), &CMenu::manager, SLOT(slotDestroyed()));
    
    menuBar->addAction(action);
    //qDebug("New action %p for top level Menu %p", action, THIS);
  }
  else
  {
    GB.Error("Type mismatch. The parent control of a Menu must be a Window or another Menu.");
    return;
  }
	
  THIS->widget.widget = (QWidget *)action;
  CMenu::dict.insert(action, THIS);

	THIS->parent = parent;
  THIS->widget.tag.type = GB_T_NULL;
  THIS->widget.name = NULL;
  THIS->picture = NULL;
  THIS->deleted = false;
	
	CWIDGET_init_name((CWIDGET *)THIS);
  
#ifdef DEBUG_MENU
  qDebug("CMENU_new: item = %p (%d) parent = %p (%d) toplevel = %p", item, item->id, item->parent, item->parent ? item->parent->id : 0, item->toplevel);
#endif

	THIS->toplevel = topLevel;
	refresh_menubar(THIS);
  //qDebug("*** CMENU_new %p", _object);
  GB.Ref(THIS);

END_METHOD


BEGIN_METHOD_VOID(CMENU_free)

#ifdef DEBUG_MENU
  qDebug("CMENU_free: item = %p '%s' (%d) parent = %p (%d)", item, item->text, item->id, item->parent, item->parent ? item->parent->id : 0);
#endif

	//qDebug("CMENU_free: (%s %p)", GB.GetClassName(THIS), THIS);
	
	delete_menu(THIS);

  GB.StoreObject(NULL, POINTER(&(THIS->picture)));

#ifdef DEBUG_MENU
  qDebug("*** CMENU_free: free tag");
#endif

  GB.StoreVariant(NULL, &THIS->widget.tag);

	GB.FreeString(&THIS->widget.name);

  #ifdef DEBUG_MENU
    qDebug("CMENU_free: item = %p '%s' (%d) is freed!", item, item->text, item->id);
  #endif

END_METHOD


BEGIN_PROPERTY(CMENU_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(ACTION->text()));
  else
  {
  	QString text = QSTRING_PROP();
  	ACTION->setText(text);
  	ACTION->setSeparator(text.isNull());
  }

END_PROPERTY


BEGIN_PROPERTY(CMENU_picture)

  if (READ_PROPERTY)
    GB.ReturnObject(THIS->picture);
  else
  {
  	QIcon icon;
  	
    GB.StoreObject(PROP(GB_OBJECT), POINTER(&(THIS->picture)));
    if (THIS->picture)
    	icon = QIcon(*THIS->picture->pixmap);
    ACTION->setIcon(icon);
  }

END_PROPERTY


BEGIN_PROPERTY(CMENU_enabled)

	if (READ_PROPERTY)
		GB.ReturnBoolean(ACTION->isEnabled());
	else
	{
		ACTION->setEnabled(VPROP(GB_BOOLEAN));
		//qDebug("CMENUITEM_enabled: %p %s '%s'", item, enabled ? "1" : "0", ((QString)(*item->accel)).latin1());
  	CMenu::enableAccel(THIS, ACTION->isEnabled());
	}

END_PROPERTY


BEGIN_PROPERTY(CMENU_checked)

	if (CMENU_is_toplevel(THIS))
	{
		if (READ_PROPERTY)
			GB.ReturnBoolean(0);
	}
	else
	{
		if (READ_PROPERTY)
			GB.ReturnBoolean(ACTION->isChecked());
		else
			ACTION->setChecked(VPROP(GB_BOOLEAN));
	}
  	
END_PROPERTY


BEGIN_PROPERTY(CMENU_toggle)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->toggle);
	else
		THIS->toggle = VPROP(GB_BOOLEAN);

END_PROPERTY


static void send_click_event(CMENU *_object);

BEGIN_PROPERTY(CMENU_value)

  if (THIS->toggle)
  {
    CMENU_checked(_object, _param);
    return;
  }

  if (READ_PROPERTY)
  {
    GB.ReturnBoolean(0);
  }
  else if (!CMENU_is_toplevel(THIS))
  {
    GB.Ref(THIS);
    send_click_event(THIS);
  }
  
END_PROPERTY


BEGIN_PROPERTY(CMENU_shortcut)

  if (CMENU_is_toplevel(THIS) || THIS->menu)
  {
    if (READ_PROPERTY)
      GB.ReturnNull();

    return;
  }

  if (READ_PROPERTY)
  	GB.ReturnNewZeroString(ACTION->shortcut().toString().toUtf8());
  else
  	ACTION->setShortcut(QKeySequence::fromString(QSTRING_PROP()));

END_PROPERTY


BEGIN_PROPERTY(CMENU_visible)

	if (READ_PROPERTY)
		GB.ReturnBoolean(ACTION->isVisible());
	else
	{
		ACTION->setVisible(VPROP(GB_BOOLEAN));
		refresh_menubar(THIS);
	}
	
END_PROPERTY


BEGIN_METHOD_VOID(CMENU_show)

	ACTION->setVisible(true);
	refresh_menubar(THIS);

END_METHOD


BEGIN_METHOD_VOID(CMENU_hide)

	ACTION->setVisible(false);
	refresh_menubar(THIS);

END_METHOD


BEGIN_METHOD_VOID(CMENU_delete)

	delete_menu(THIS);

END_METHOD


BEGIN_PROPERTY(CMENU_count)

  if (THIS->menu)
    GB.ReturnInteger(THIS->menu->actions().count());
  else
    GB.ReturnInteger(0);

END_PROPERTY


BEGIN_METHOD_VOID(CMENU_next)

  int index;

  if (!THIS->menu)
  {
    GB.StopEnum();
    return;
  }

  index = ENUM(int);

  if (index >= THIS->menu->actions().count())
  {
    GB.StopEnum();
    return;
  }

  GB.ReturnObject(CMenu::dict[THIS->menu->actions().at(index)]);

  ENUM(int) = index + 1;

END_METHOD


BEGIN_METHOD(CMENU_get, GB_INTEGER index)

  int index = VARG(index);

  if (!THIS->menu || index < 0 || index >= THIS->menu->actions().count())
  {
    GB.Error(GB_ERR_BOUND);
    return;
  }

  GB.ReturnObject(CMenu::dict[THIS->menu->actions().at(index)]);

END_METHOD


BEGIN_METHOD_VOID(CMENU_clear)

  int i;

  if (THIS->menu)
  {
    QList<QAction *> list = THIS->menu->actions();
    
    for (i = 0; i < list.count(); i++)
    	delete_menu(CMenu::dict[list.at(i)]);
  }

END_METHOD


BEGIN_METHOD(CMENU_popup, GB_INTEGER x; GB_INTEGER y)

	if (THIS->menu && !THIS->exec)
	{
		THIS->exec = TRUE;
		CMenu::enableAccel(THIS, true);
		
		if (MISSING(x) || MISSING(y))
			THIS->menu->exec(QCursor::pos());
		else
			THIS->menu->exec(QPoint(VARG(x), VARG(y)));
			
		THIS->exec = FALSE;
		MAIN_process_events();
  }

END_METHOD


BEGIN_PROPERTY(CMENU_window)

  GB.ReturnObject(CWidget::get(THIS->toplevel));

END_PROPERTY


GB_DESC CMenuChildrenDesc[] =
{
  GB_DECLARE(".MenuChildren", sizeof(CMENU)), GB_VIRTUAL_CLASS(),

  GB_METHOD("_next", "Menu", CMENU_next, NULL),
  GB_METHOD("_get", "Menu", CMENU_get, "(Index)i"),
  GB_METHOD("Clear", NULL, CMENU_clear, NULL),
  GB_PROPERTY_READ("Count", "i", CMENU_count),

  GB_END_DECLARE
};


GB_DESC CMenuDesc[] =
{
  GB_DECLARE("Menu", sizeof(CMENU)), //GB_HOOK_CHECK(CWIDGET_check),
  GB_HOOK_CHECK(check_menu),

  //GB_STATIC_METHOD("_init", NULL, CMENU_init, NULL),
  GB_METHOD("_new", NULL, CMENU_new, "(Parent)o[(Hidden)b]"),
  //GB_METHOD("_new", NULL, CMENU_new, "(Parent)o[(Visible)b]"),
  GB_METHOD("_free", NULL, CMENU_free, NULL),

  //GB_PROPERTY("Name", "s", CWIDGET_name),

  //GB_PROPERTY_READ("Count", "i", CMENU_count),

  //GB_PROPERTY_READ("Parent", "Control", CWIDGET_parent),

  GB_PROPERTY("Name", "s", CCONTROL_name),
  GB_PROPERTY("Caption", "s", CMENU_text),
  GB_PROPERTY("Text", "s", CMENU_text),
  GB_PROPERTY("Enabled", "b", CMENU_enabled),
  GB_PROPERTY("Checked", "b", CMENU_checked),
  GB_PROPERTY("Tag", "v", CCONTROL_tag),
  GB_PROPERTY("Picture", "Picture", CMENU_picture),
  //GB_PROPERTY("Stretch", "b", CMENU_stretch),
  GB_PROPERTY("Shortcut", "s", CMENU_shortcut),
  GB_PROPERTY("Visible", "b", CMENU_visible),
  GB_PROPERTY("Toggle", "b", CMENU_toggle),
  GB_PROPERTY("Value", "b", CMENU_value),
  GB_PROPERTY("Action", "s", CCONTROL_action),
  GB_PROPERTY_READ("Window", "Window", CMENU_window),

  GB_PROPERTY_SELF("Children", ".MenuChildren"),
  //GB_PROPERTY_READ("Index", "i", CMENU_item_index),

	MENU_DESCRIPTION,

  GB_METHOD("Popup", NULL, CMENU_popup, "[(X)i(Y)i]"),
  GB_METHOD("Delete", NULL, CMENU_delete, NULL),
  GB_METHOD("Show", NULL, CMENU_show, NULL),
  GB_METHOD("Hide", NULL, CMENU_hide, NULL),

  //GB_EVENT("Delete", NULL, NULL, &EVENT_Destroy), // Must be first
  GB_EVENT("Click", NULL, NULL, &EVENT_Click),
  GB_EVENT("Show", NULL, NULL, &EVENT_Show),
  GB_EVENT("Hide", NULL, NULL, &EVENT_Hide),

  GB_END_DECLARE
};



/* CMenu class */

CMenu CMenu::manager;
QHash<QAction *, CMENU *> CMenu::dict;

static void send_click_event(CMENU *_object)
{
	if (THIS->toggle)
		toggle_menu(THIS);
  GB.Raise(THIS, EVENT_Click, 0);
	CACTION_raise((CWIDGET *)THIS);
  GB.Unref(POINTER(&_object));
}

static void send_menu_event(CMENU *_object, intptr_t event)
{
  GB.Raise(THIS, event, 0);
  GB.Unref(POINTER(&_object));
}

void CMenu::slotTriggered(QAction *action)
{
  CMENU *menu = CMenu::dict[action];

	GB.Ref(menu);
	GB.Post((GB_POST_FUNC)send_click_event, (intptr_t)menu);
}



void CMenu::slotShown(void)
{
	GET_SENDER(menu);
	hideSeparators((CMENU *)menu);
  GB.Raise(menu, EVENT_Show, 0);
}


void CMenu::slotHidden(void)
{
	GET_SENDER(menu);

	if (GB.CanRaise(menu, EVENT_Hide))
	{
	  GB.Ref(menu);
  	GB.Post2((GB_POST_FUNC)send_menu_event, (intptr_t)menu, EVENT_Hide);
	}
}


void CMenu::enableAccel(CMENU *item, bool enable)
{
	//if (((QString)(*item->accel)).latin1())
	//	qDebug("CMenu::enableAccel: %p %s '%s'", item, enable ? "1" : "0", ((QString)(*item->accel)).latin1());

	#if 0
	if (enable)
  	CONTAINER_CALL(item, setAccel(*(item->accel), item->id))
	else
  	CONTAINER_CALL(item, setAccel(QKeySequence(), item->id))
  	
	item->noshortcut = !enable;

	if (item->children)
	{
		int i;
		/*QListIterator<CMENU> it(*item->children);

		while ((child = it.current()))
		{
			++it;
			CMenu::enableAccel(child, enable);
		}*/
		for (i = 0; i < item->children->count(); i++)
			CMenu::enableAccel(item->children->at(i), enable);
	}
	#endif
}


void CMenu::hideSeparators(CMENU *item)
{
	if (!item->menu)
		return;
		
	#if 0
	CMENU *child;
	CMENU *last_child;
	//QListIterator<CMENU> it(*item->children);
	bool is_sep;
	bool last_sep;
	QList<CMENU *> children = *(item->children);
	int i;

	//qDebug("checking separators");

	last_sep = true;
	last_child = 0;
	
	for(i = 0; i < children.count(); i++)
	{	
		child = children.at(i);
		
		is_sep = CMENU_is_separator(child);

		//qDebug("separator = %d  visible = %d  (%p -> %p)", is_sep, CMENU_is_visible(child), child, it.current());

		if (is_sep)
		{
			if (last_sep)
			{
				hide_menu(child);
			}
			else
			{
				show_menu(child);
				last_sep = true;
				last_child = child;
			}
		}
		else
		{
			if (CMENU_is_visible(child))
				last_sep = false;
		}
	}
	
	if (last_sep && last_child)
		hide_menu(last_child);
	#endif
}


/*
void CMenu::unrefChildren(QWidget *w)
{
	int i;
	QList<QAction *> list = w->actions();
	CMENU *child;
	
	for (i = 0; i < list.count(); i++)
	{
		child = dict[list.at(i)];
		//qDebug("CMenu::unrefChildren: (%s %p)", GB.GetClassName(child), child);
		GB.Detach(child);
		unregister_menu(child);
		//qDebug("*** CMenu::destroy %p (child)", child);
		GB.Unref(POINTER(&child));
	}
}*/

void CMenu::slotDestroyed(void)
{
  CMENU *_object = dict[(QAction *)sender()];

  #ifdef DEBUG_MENU
  qDebug("*** { CMenu::destroy %p", menu);
  #endif

	//qDebug("CMenu::slotDestroyed: action = %p  THIS = %p", sender(), _object);
	
	if (!_object)
		return;

	//qDebug("CMenu::slotDestroyed: (%s %p)", GB.GetClassName(THIS), THIS);

	//if (THIS->menu)
  //	unrefChildren(THIS->menu);

  #ifdef DEBUG_MENU
  qDebug("***  CMenu::destroy %p (UNREF)", menu);
  #endif

	unregister_menu(THIS);
  GB.Unref(POINTER(&_object));

  //menu->dict = dict;

  #ifdef DEBUG_MENU
  qDebug("*** } CMenu::destroy: %p", menu);
  #endif
}

