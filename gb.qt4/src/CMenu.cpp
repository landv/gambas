/***************************************************************************

  CMenu.cpp

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CMENU_CPP

#undef QT3_SUPPORT

#include <QMenuBar>
#include <QMenu>
#include <QKeyEvent>

#include "gambas.h"
#include "gb_common.h"

#include "CWidget.h"
#include "CWindow.h"
#include "CMenu.h"

DECLARE_EVENT(EVENT_Click);
DECLARE_EVENT(EVENT_Show);
DECLARE_EVENT(EVENT_Hide);

DECLARE_METHOD(Control_Window);
DECLARE_METHOD(Control_Name);
DECLARE_METHOD(CMENU_hide);

static bool _popup_immediate = false;
static CMENU *_popup_menu_clicked = NULL;

static void clear_menu(CMENU *_object);

static int check_menu(void *_object)
{
  return THIS->deleted || ACTION == 0;
}

static void refresh_menubar(CMENU *menu)
{
	int i;
	QList<QAction *> list;
	QAction *action;
	MyMainWindow *toplevel;
	CWINDOW *window;
	QMenuBar *menuBar;
	
	if (!CMENU_is_toplevel(menu))
		return;
	
	toplevel = (MyMainWindow *)(menu->toplevel);
	window = ((CWINDOW *)(menu->parent));
	menuBar = window->menuBar;
	if (!menuBar)
		return;
	
	list = menuBar->actions();
	 
	for (i = 0; i < list.count(); i++)
	{
		action = list.at(i);
		menu = CMenu::dict[action];
		if (!menu || menu->deleted)
			continue;
		if (action->isVisible() && !action->isSeparator())
			break;
	}
	
	window->hideMenuBar = i == list.count();
	toplevel->configure();
}

static void unregister_menu(CMENU *_object)
{
	CACTION_register((CWIDGET *)THIS, NULL);
}

static void set_menu_visible(void *_object, bool v)
{
	THIS->visible = v;
	ACTION->setVisible(v);
	refresh_menubar(THIS);
	//update_accel_recursive(THIS);
}

static void delete_menu(CMENU *_object)
{
	if (THIS->deleted)
		return;
	
	//qDebug("delete_menu: %s %p", THIS->widget.name, THIS);
	
	THIS->deleted = true;
	
	clear_menu(THIS);
	
	if (THIS->menu)
	{
		delete THIS->menu;
		THIS->menu = 0;
	}
	
	if (THIS->accel)
		delete THIS->accel;

	if (ACTION)
	{
		refresh_menubar(THIS);
		delete ACTION;
	}
	
// 	if (ACTION)
// 	{
// 		QAction *action = ACTION;
// 		THIS->widget.widget = 0;
// 		delete action;
// 	}
}

static void clear_menu(CMENU *_object)
{
  int i;
	CMENU *menu;

  if (THIS->menu)
  {
    QList<QAction *> list = THIS->menu->actions();
    
    for (i = 0; i < list.count(); i++)
		{
			menu = CMenu::dict[list.at(i)];
			//GB.Ref(menu);
			//delete ((QAction *)(menu->widget.widget));
			if (menu)
				delete_menu(menu);
			//GB.Unref(POINTER(&menu));
		}
  }
}

static bool is_fully_enabled(CMENU *_object)
{
	for(;;)
	{
		if (THIS->exec)
			return true;
		
		if (THIS->disabled)
			return false;
		
		if (CMENU_is_toplevel(THIS))
			return true;
		
		_object = (CMENU *)THIS->parent;
	}
}

static void update_accel(CMENU *_object)
{
	if (CMENU_is_toplevel(THIS))
		return;
	
	if (THIS->accel && !THIS->accel->isEmpty() && is_fully_enabled(THIS))
	{
		//if (!qstrcmp(THIS->widget.name, "mnuCopy"))
		//	qDebug("update_accel: %s: %s", THIS->widget.name, (const char *)THIS->accel->toString().toUtf8());
		ACTION->setShortcut(*(THIS->accel));
	}
	else
	{
		//if (!qstrcmp(THIS->widget.name, "mnuCopy"))
		//	qDebug("update_accel: %s: NULL", THIS->widget.name);
		ACTION->setShortcut(QKeySequence());
	}
}

static void update_accel_recursive(CMENU *_object)
{
	if (THIS->exec)
		return;
	
	update_accel(THIS);
	
	if (THIS->menu)
	{
		int i;
		
		for (i = 0; i < THIS->menu->actions().count(); i++)
			update_accel_recursive(CMenu::dict[THIS->menu->actions().at(i)]);
	}
}

static void update_check(CMENU *_object)
{
	if (THIS->checked || THIS->toggle)
	{
		ACTION->setCheckable(true);
		ACTION->setChecked(THIS->checked);
	}
	else
	{
		ACTION->setCheckable(false);
		ACTION->setChecked(false);
	}
}

#if 0
static void toggle_menu(CMENU *_object)
{
	if (CMENU_is_toplevel(THIS))
		return;

	qDebug("toggle_menu: %s %d", TO_UTF8(ACTION->text()), ACTION->isChecked());

	//ACTION->setCheckable(true);
	ACTION->setChecked(!ACTION->isChecked());
	//ACTION->setCheckable(false);
	
	qDebug("--> %d", ACTION->isChecked());
}
#endif

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
	set_menu_visible(THIS, !VARGOPT(hidden, FALSE));

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

	//qDebug("CMENU_new: (%s %p)", THIS->widget.name, THIS);
	
END_METHOD


BEGIN_METHOD_VOID(CMENU_free)

#ifdef DEBUG_MENU
  qDebug("CMENU_free: item = %p '%s' (%d) parent = %p (%d)", item, item->text, item->id, item->parent, item->parent ? item->parent->id : 0);
#endif

	//qDebug("CMENU_free: (%s %p)", THIS->widget.name, THIS);
	
	delete_menu(THIS);

  GB.StoreObject(NULL, POINTER(&(THIS->picture)));

#ifdef DEBUG_MENU
  qDebug("*** CMENU_free: free tag");
#endif

  GB.StoreVariant(NULL, &THIS->widget.tag);

	//qDebug("free_name: %s %p (CMENU_free)", THIS->widget.name, THIS->widget.name);
	//if (!strcmp(THIS->widget.name, "mnuCut"))
	//	BREAKPOINT();
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
  	ACTION->setSeparator(text.isNull());
  	ACTION->setText(text);
		refresh_menubar(THIS);
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
		GB.ReturnBoolean(!THIS->disabled);
	else
	{
		bool b = VPROP(GB_BOOLEAN);
		THIS->disabled = !b;
		ACTION->setEnabled(b);
  	//CMenu::enableAccel(THIS, b && !THIS->noshortcut);
		update_accel_recursive(THIS);
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
			GB.ReturnBoolean(THIS->checked);
		else
		{
			THIS->checked = VPROP(GB_BOOLEAN);
			update_check(THIS);
		}
	}
  	
END_PROPERTY


BEGIN_PROPERTY(CMENU_toggle)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->toggle);
	else
	{
		THIS->toggle = VPROP(GB_BOOLEAN);
		update_check(THIS);
	}

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
		qDebug("CMENU_value: %s", THIS->widget.name);
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
  	GB.ReturnNewZeroString(THIS->accel ? (const char *)THIS->accel->toString().toUtf8() : NULL);
  else
	{
		if (THIS->accel)
			delete THIS->accel;
		THIS->accel = new QKeySequence;
		*(THIS->accel) = QKeySequence::fromString(QSTRING_PROP());
		
		update_accel(THIS);
	}

END_PROPERTY


BEGIN_PROPERTY(CMENU_visible)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->visible);
	else
		set_menu_visible(THIS, VPROP(GB_BOOLEAN));
	
END_PROPERTY


BEGIN_METHOD_VOID(CMENU_show)

	set_menu_visible(THIS, true);

END_METHOD


BEGIN_METHOD_VOID(CMENU_hide)

	set_menu_visible(THIS, false);

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

	clear_menu(THIS);

END_METHOD

void CMENU_popup(CMENU *_object, const QPoint &pos)
{
	bool disabled;

	if (THIS->menu && !THIS->exec)
	{
		disabled = THIS->disabled;
		if (disabled)
		{
			THIS->disabled = false;
			update_accel_recursive(THIS);
			THIS->disabled = true;
		}
		
		// The Click event is posted, it does not occur immediately.
		THIS->exec = true;
		_popup_immediate = true;
		THIS->menu->exec(pos);
		_popup_immediate = false;
		THIS->exec = false;
		
		//qDebug("_popup_menu_clicked = %p", _popup_menu_clicked);
		update_accel_recursive(THIS);
		
		if (_popup_menu_clicked)
		{
			send_click_event(_popup_menu_clicked);
			_popup_menu_clicked = NULL;
		}
		
		//MyMainWindow *toplevel = (MyMainWindow *)(THIS->toplevel);
		//CWINDOW_fix_menubar((CWINDOW *)CWidget::get(toplevel));
	}
}

BEGIN_METHOD(Menu_Popup, GB_INTEGER x; GB_INTEGER y)

	QPoint pos;
	
	if (MISSING(x) || MISSING(y))
		pos = QCursor::pos();
	else
		pos = QPoint(VARG(x), VARG(y));
	
	CMENU_popup(THIS, pos);
	
END_METHOD


BEGIN_PROPERTY(CMENU_window)

  GB.ReturnObject(CWidget::get(THIS->toplevel));

END_PROPERTY

/*BEGIN_PROPERTY(CMENU_tear_off)

	if (!THIS->menu)
		return;

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->menu->isTearOffEnabled());
	else
		THIS->menu->setTearOffEnabled(VPROP(GB_BOOLEAN));

END_PROPERTY*/


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

  GB_PROPERTY("Name", "s", Control_Name),
  GB_PROPERTY("Caption", "s", CMENU_text),
  GB_PROPERTY("Text", "s", CMENU_text),
  GB_PROPERTY("Enabled", "b", CMENU_enabled),
  GB_PROPERTY("Checked", "b", CMENU_checked),
  GB_PROPERTY("Tag", "v", Control_Tag),
  GB_PROPERTY("Picture", "Picture", CMENU_picture),
  //GB_PROPERTY("Stretch", "b", CMENU_stretch),
  GB_PROPERTY("Shortcut", "s", CMENU_shortcut),
  GB_PROPERTY("Visible", "b", CMENU_visible),
  GB_PROPERTY("Toggle", "b", CMENU_toggle),
  GB_PROPERTY("Value", "b", CMENU_value),
  //GB_PROPERTY("TearOff", "b", CMENU_tear_off),
  GB_PROPERTY("Action", "s", Control_Action),
  GB_PROPERTY_READ("Window", "Window", CMENU_window),

  GB_PROPERTY_SELF("Children", ".MenuChildren"),
  //GB_PROPERTY_READ("Index", "i", CMENU_item_index),

	MENU_DESCRIPTION,

  GB_METHOD("Popup", NULL, Menu_Popup, "[(X)i(Y)i]"),
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
	{
		THIS->checked = !THIS->checked;
		update_check(THIS);
	}
	
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
  GET_MENU_SENDER(parent);
  CMENU *menu = CMenu::dict[action];

	if (menu->parent != parent)
		return;

	//qDebug("slotTriggered: %s %s", menu->widget.name, (const char *)action->text().toUtf8());
	GB.Ref(menu);
	if (_popup_immediate)
		_popup_menu_clicked = menu;
	else
		GB.Post((GB_POST_FUNC)send_click_event, (intptr_t)menu);
}

void CMenu::slotShown(void)
{
  GET_MENU_SENDER(menu);
	GB.Raise(menu, EVENT_Show, 0);
}

void CMenu::slotHidden(void)
{
  GET_MENU_SENDER(menu);

	if (GB.CanRaise(menu, EVENT_Hide))
	{
	  GB.Ref(menu);
  	GB.Post2((GB_POST_FUNC)send_menu_event, (intptr_t)menu, EVENT_Hide);
	}
}


#if 0
void CMenu::enableAccel(CMENU *item, bool enable, bool rec)
{
	// Do not disable shortcuts when a menu is executed
	if (item->exec && !enable)
		return;

	if (!rec)
		qDebug("CMenu::enableAccel: %s: %s", item->widget.name, enable ? "ON" : "OFF");
	
	item->noshortcut = !enable;
	update_accel(item);

	if (item->menu)
	{
		int i;
		
		for (i = 0; i < item->menu->actions().count(); i++)
			CMenu::enableAccel(CMenu::dict[item->menu->actions().at(i)], enable, true);
	}
}
#endif

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

  CMenu::dict.remove(ACTION);
	//qDebug("CMenu::slotDestroyed: (%s %p)", GB.GetClassName(THIS), THIS);

	//if (THIS->menu)
  //	unrefChildren(THIS->menu);

  #ifdef DEBUG_MENU
  qDebug("UNREF (%s %p)", THIS->widget.name, THIS);
  #endif
	
	THIS->widget.widget = 0;

	unregister_menu(THIS);
  GB.Unref(POINTER(&_object));

  //menu->dict = dict;

  #ifdef DEBUG_MENU
  qDebug("*** } CMenu::destroy: %p", menu);
  #endif
}

