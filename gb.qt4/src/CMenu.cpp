/***************************************************************************

	CMenu.cpp

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

#define __CMENU_CPP

#undef QT3_SUPPORT

#include <QMenuBar>
#include <QMenu>
#include <QKeyEvent>
#include <QActionGroup>

#include "gambas.h"
#include "gb_common.h"

#include "CWidget.h"
#include "CWindow.h"
#include "CMenu.h"

//#define DEBUG_MENU 1

DECLARE_EVENT(EVENT_Click);
DECLARE_EVENT(EVENT_Show);
DECLARE_EVENT(EVENT_Hide);

DECLARE_METHOD(Control_Window);
DECLARE_METHOD(Control_Name);
DECLARE_METHOD(Menu_Hide);

static bool _popup_immediate = false;
static CMENU *_popup_menu_clicked = NULL;
int MENU_popup_count = 0;

static void clear_menu(CMENU *_object);

static GB_FUNCTION _init_shortcut_func;


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
	if (THIS->action)
	{
		CACTION_register((CWIDGET *)THIS, THIS->action, NULL);
		GB.FreeString(&THIS->action);
	}
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
		//GB.Post((GB_CALLBACK)delete_later, (intptr_t)THIS->menu);
		THIS->menu->deleteLater();
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

		THIS->init_shortcut = FALSE;
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
	if (THIS->checked || THIS->toggle || THIS->radio)
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

static void update_action_group(QMenu *parent)
{
	int i;
	QAction *action;
	CMENU *menu;
	QActionGroup *group = NULL;

	for (i = 0; i < parent->actions().count(); i++)
	{
		action = parent->actions().at(i);
		menu = CMenu::dict[action];
		if (!menu || menu->deleted)
			continue;

		if (menu->radio)
		{
			if (!group)
			{
				if (action->actionGroup())
					group = action->actionGroup();
				else
					group = new QActionGroup(parent);
			}
			action->setActionGroup(group);
		}
		else
		{
			group = NULL;
			action->setActionGroup(NULL);
		}

		//qDebug("action '%s' -> %p", TO_UTF8(action->text()), (void *)group);
	}
}

//---------------------------------------------------------------------------

MyAction::MyAction(QObject *parent): QAction(parent)
{
}

bool MyAction::event(QEvent *e)
{
	if (e->type() == QEvent::Shortcut)
	{
		activate(Trigger);
		return true;
	}

	return QAction::event(e);
}

//---------------------------------------------------------------------------

BEGIN_METHOD(Menu_new, GB_OBJECT parent; GB_BOOLEAN hidden)

	QAction *action;
	void *parent = VARG(parent);
	QWidget *topLevel = 0;
	QMenuBar *menuBar = 0;

	//printf("Menu_new %p\n", _object);

	if (GB.CheckObject(parent))
		return;

	//qDebug("Menu_new: (%s %p)", GB.GetClassName(THIS), THIS);

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

		action = new MyAction(menu->menu);
		action->setSeparator(true);
		QObject::connect(action, SIGNAL(toggled(bool)), &CMenu::manager, SLOT(slotToggled(bool)));
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
			//menuBar->setAutoFillBackground(true);
			window->menuBar = menuBar;
		}

		action = new MyAction(menuBar);
		menuBar->addAction(action);

		action->setSeparator(true);
		QObject::connect(action, SIGNAL(destroyed()), &CMenu::manager, SLOT(slotDestroyed()));

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
	THIS->widget.name = NULL;
	THIS->picture = NULL;
	THIS->deleted = false;

	CWIDGET_init_name((CWIDGET *)THIS);

#ifdef DEBUG_MENU
	//qDebug("Menu_new: item = %p (%d) parent = %p (%d) toplevel = %p", item, item->id, item->parent, item->parent ? item->parent->id : 0, item->toplevel);
#endif

	THIS->toplevel = topLevel;
	refresh_menubar(THIS);
	//qDebug("*** Menu_new %p", _object);
	GB.Ref(THIS);

	//qDebug("Menu_new: (%s %p)", THIS->widget.name, THIS);

END_METHOD


BEGIN_METHOD_VOID(Menu_free)

#ifdef DEBUG_MENU
	qDebug("Menu_free: item = %p", THIS);
#endif

	//qDebug("Menu_free: (%s %p)", THIS->widget.name, THIS);

	delete_menu(THIS);

	GB.StoreObject(NULL, POINTER(&(THIS->picture)));

#ifdef DEBUG_MENU
	qDebug("*** Menu_free: free tag");
#endif

	if (THIS->widget.ext)
	{
		GB.StoreVariant(NULL, &THIS->widget.ext->tag);
		GB.Free(POINTER(&THIS->widget.ext));
	}

	//qDebug("free_name: %s %p (Menu_free)", THIS->widget.name, THIS->widget.name);
	//if (!strcmp(THIS->widget.name, "mnuCut"))
	//	BREAKPOINT();
	GB.FreeString(&THIS->widget.name);
	GB.FreeString(&THIS->save_text);

	#ifdef DEBUG_MENU
		qDebug("Menu_free: item = %p is freed!", THIS);
	#endif

END_METHOD


BEGIN_PROPERTY(Menu_Text)

	if (READ_PROPERTY)
	{
		if (THIS->save_text)
			GB.ReturnString(THIS->save_text);
		else
			GB.ReturnNewZeroString(TO_UTF8(ACTION->text()));
	}
	else
	{
		QString text = QSTRING_PROP();
		ACTION->setText(text);
		ACTION->setSeparator(text.isEmpty());
		refresh_menubar(THIS);

		if (!CMENU_is_toplevel(THIS))
			((CMENU *)THIS->parent)->init_shortcut = FALSE;

		GB.FreeString(&THIS->save_text);
	}

END_PROPERTY


BEGIN_PROPERTY(Menu_Picture)

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


BEGIN_PROPERTY(Menu_Enabled)

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

BEGIN_PROPERTY(Menu_Checked)

	if (CMENU_is_toplevel(THIS))
	{
		if (READ_PROPERTY)
			GB.ReturnBoolean(0);
	}
	else
	{
		if (READ_PROPERTY)
		{
			GB.ReturnBoolean(THIS->checked);
		}
		else
		{
			THIS->checked = VPROP(GB_BOOLEAN);
			update_check(THIS);
		}
	}

END_PROPERTY


BEGIN_PROPERTY(Menu_Toggle)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->toggle);
	else
	{
		THIS->toggle = VPROP(GB_BOOLEAN);
		update_check(THIS);
	}

END_PROPERTY


BEGIN_PROPERTY(Menu_Radio)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->radio);
	else if (THIS->radio != VPROP(GB_BOOLEAN))
	{
		THIS->radio = VPROP(GB_BOOLEAN);
		if (!CMENU_is_toplevel(THIS))
			update_action_group(((CMENU *)THIS->parent)->menu);
		update_check(THIS);
	}

END_PROPERTY


static void send_click_event(CMENU *_object);

BEGIN_PROPERTY(Menu_Value)

	if (THIS->toggle || THIS->radio)
	{
		Menu_Checked(_object, _param);
		return;
	}

	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(0);
	}
	else if (!CMENU_is_toplevel(THIS))
	{
		//qDebug("Menu_Value: %s", THIS->widget.name);
		GB.Ref(THIS);
		send_click_event(THIS);
	}

END_PROPERTY


BEGIN_PROPERTY(Menu_Shortcut)

	if (CMENU_is_toplevel(THIS) || THIS->menu)
	{
		if (READ_PROPERTY)
			GB.ReturnVoidString();

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


BEGIN_PROPERTY(Menu_Visible)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->visible);
	else
		set_menu_visible(THIS, VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD_VOID(Menu_Show)

	set_menu_visible(THIS, true);

END_METHOD


BEGIN_METHOD_VOID(Menu_Hide)

	set_menu_visible(THIS, false);

END_METHOD


BEGIN_METHOD_VOID(Menu_Delete)

	delete_menu(THIS);

END_METHOD


BEGIN_PROPERTY(MenuChildren_Count)

	if (THIS->menu)
		GB.ReturnInteger(THIS->menu->actions().count());
	else
		GB.ReturnInteger(0);

END_PROPERTY


BEGIN_METHOD_VOID(MenuChildren_next)

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


BEGIN_METHOD(MenuChildren_get, GB_INTEGER index)

	int index = VARG(index);

	if (!THIS->menu || index < 0 || index >= THIS->menu->actions().count())
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}

	GB.ReturnObject(CMenu::dict[THIS->menu->actions().at(index)]);

END_METHOD


BEGIN_METHOD_VOID(MenuChildren_Clear)

	clear_menu(THIS);

END_METHOD

void CMENU_popup(CMENU *_object, const QPoint &pos)
{
	bool disabled;
	void *save;

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
		save = CWIDGET_enter_popup();
		THIS->exec = true;
		_popup_immediate = true;
		THIS->menu->exec(pos);
		_popup_immediate = false;
		THIS->exec = false;
		CWIDGET_leave_popup(save);

		//qDebug("_popup_menu_clicked = %p", _popup_menu_clicked);
		update_accel_recursive(THIS);

		//CWIDGET_check_hovered();

		if (_popup_menu_clicked)
		{
			CMENU *menu = _popup_menu_clicked;
			_popup_menu_clicked = NULL;
			send_click_event(menu);
		}

		MENU_popup_count++;

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


BEGIN_PROPERTY(Menu_Window)

	GB.ReturnObject(CWidget::get(THIS->toplevel));

END_PROPERTY

BEGIN_PROPERTY(Menu_Action)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->action);
	else
	{
		char *action = PLENGTH() ? GB.NewString(PSTRING(), PLENGTH()) : NULL;
		CACTION_register(THIS, THIS->action, action);
		GB.FreeString(&THIS->action);
		THIS->action = action;
	}

END_PROPERTY

BEGIN_PROPERTY(Menu_SaveText)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->save_text);
	else
		GB.StoreString(PROP(GB_STRING), &THIS->save_text);

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
	GB_DECLARE(".Menu.Children", sizeof(CMENU)), GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "Menu", MenuChildren_next, NULL),
	GB_METHOD("_get", "Menu", MenuChildren_get, "(Index)i"),
	GB_METHOD("Clear", NULL, MenuChildren_Clear, NULL),
	GB_PROPERTY_READ("Count", "i", MenuChildren_Count),

	GB_END_DECLARE
};


GB_DESC CMenuDesc[] =
{
	GB_DECLARE("Menu", sizeof(CMENU)), //GB_HOOK_CHECK(CWIDGET_check),
	GB_HOOK_CHECK(check_menu),

	GB_METHOD("_new", NULL, Menu_new, "(Parent)o[(Hidden)b]"),
	GB_METHOD("_free", NULL, Menu_free, NULL),

	GB_PROPERTY("Name", "s", Control_Name),
	GB_PROPERTY("Caption", "s", Menu_Text),
	GB_PROPERTY("Text", "s", Menu_Text),
	GB_PROPERTY("_Text", "s", Menu_SaveText),
	GB_PROPERTY("Enabled", "b", Menu_Enabled),
	GB_PROPERTY("Checked", "b", Menu_Checked),
	GB_PROPERTY("Tag", "v", Control_Tag),
	GB_PROPERTY("Picture", "Picture", Menu_Picture),
	GB_PROPERTY("Shortcut", "s", Menu_Shortcut),
	GB_PROPERTY("Visible", "b", Menu_Visible),
	GB_PROPERTY("Toggle", "b", Menu_Toggle),
	GB_PROPERTY("Radio", "b", Menu_Radio),
	GB_PROPERTY("Value", "b", Menu_Value),
	GB_PROPERTY("Action", "s", Menu_Action),
	GB_PROPERTY_READ("Window", "Window", Menu_Window),

	GB_PROPERTY_SELF("Children", ".Menu.Children"),

	MENU_DESCRIPTION,

	GB_METHOD("Popup", NULL, Menu_Popup, "[(X)i(Y)i]"),
	GB_METHOD("Delete", NULL, Menu_Delete, NULL),
	GB_METHOD("Show", NULL, Menu_Show, NULL),
	GB_METHOD("Hide", NULL, Menu_Hide, NULL),

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
	if (THIS->toggle && !THIS->radio)
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
		GB.Post((GB_CALLBACK)send_click_event, (intptr_t)menu);
}

void CMenu::slotShown(void)
{
	static bool init = FALSE;

	GET_MENU_SENDER(menu);

	GB.Ref(menu);

	GB.Raise(menu, EVENT_Show, 0);

	if (!init)
	{
		GB.GetFunction(&_init_shortcut_func, (void *)GB.FindClass("_Gui"), "_DefineShortcut", NULL, NULL);
		init = TRUE;
	}

	GB.Push(1, GB_T_OBJECT, menu);
	GB.Call(&_init_shortcut_func, 1, FALSE);

	GB.Unref(POINTER(&menu));
}

void CMenu::slotHidden(void)
{
	GET_MENU_SENDER(menu);

	if (GB.CanRaise(menu, EVENT_Hide))
	{
		GB.Ref(menu);
		GB.Post2((GB_CALLBACK)send_menu_event, (intptr_t)menu, EVENT_Hide);
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

void CMenu::slotToggled(bool checked)
{
	CMENU *_object = dict[(QAction *)sender()];

	if (!_object)
		return;

	if (!THIS->radio)
		return;

	THIS->checked = checked;
}

void CMenu::slotDestroyed(void)
{
	CMENU *_object = dict[(QAction *)sender()];

	#ifdef DEBUG_MENU
	qDebug("*** { CMenu::destroy %p", THIS);
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

	unregister_menu(THIS);
	THIS->widget.widget = NULL;
	GB.Unref(POINTER(&_object));

	//menu->dict = dict;

	#ifdef DEBUG_MENU
	qDebug("*** } CMenu::destroy: %p", THIS);
	#endif
}

