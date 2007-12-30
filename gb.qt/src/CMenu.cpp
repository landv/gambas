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

#include <qapplication.h>
#include <qeventloop.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qaccel.h>
#include <qcursor.h>
#include <qiconset.h>
#include <qpixmap.h>

#include "gambas.h"

#include "CWidget.h"
#include "CWindow.h"
#include "CMenu.h"

//#define DEBUG_MENU

//static void *CLASS_Menu;
//static void *CLASS_Window;

DECLARE_EVENT(EVENT_Click);
DECLARE_EVENT(EVENT_Show);
DECLARE_EVENT(EVENT_Hide);

DECLARE_METHOD(CCONTROL_window);
DECLARE_METHOD(CCONTROL_name);

static long menu_id = 0;


static int check_menu(void *object)
{
  return ((CMENU *)object)->deleted;
}


static void set_text(CMENU *item, char *text)
{
#ifdef DEBUG_MENU
  qDebug("set_text: item->text = '%s' text = '%s'", item->text, text);
#endif

  if (item->text)
  {
    GB.FreeString(&item->text);
    item->text = NULL;
  }

  if (text)
    GB.NewString(&item->text, text, 0);
}


static void hide_menu(CMENU *item)
{
  if (!CWIDGET_test_flag(item, WF_VISIBLE))
    return;

#ifdef DEBUG_MENU
  qDebug("hide_menu: item = %p (%d)", item, item->id);
#endif

  //index = item->container->indexOf(item->id);
  //if (index >= 0)
  item->container->removeItem(item->id);

  if (CMENU_is_top(item))
  {
    if (item->container->count() == 0)
    {
      ((QMenuBar *)item->container)->hide();
      ((MyMainWindow *)item->toplevel)->configure();
		}
  }

  CWIDGET_clear_flag(item, WF_VISIBLE);
}


static void show_menu(CMENU *item)
{
  CMENU *parent;
  int pos;
  //bool change;
  QIconSet icon;
  QString text;


  if (CWIDGET_test_flag(item, WF_VISIBLE))
    return;

#ifdef DEBUG_MENU
  qDebug("show_menu: item = %p (%d) parent = %p (%d)", item, item->id, item->parent, item->parent ? item->parent->id : 0);
#endif

  parent = item->parent;

  if (parent)
  {
    /*
    if (QPOPUPMENU(parent) == 0)
    {
#ifdef DEBUG_MENU
      qDebug("show_menu: add_popup");
#endif

      QPopupMenu *popup = new QPopupMenu(item->toplevel);

      change = CMENU_is_visible(parent);

      if (change)
        hide_menu(parent);

      QPOPUPMENU(parent) = popup;

      //parent->dict = new QIntDict<CMENU>;

      QObject::connect(popup, SIGNAL(activated(int)), &CMenu::manager, SLOT(activated(int)));
      QObject::connect(popup, SIGNAL(aboutToShow()), &CMenu::manager, SLOT(shown()));
      QObject::connect(popup, SIGNAL(destroyed()), &CMenu::manager, SLOT(destroy()));

      // un popupmenu est r��enc�deux fois !
      // Et il ne faut pas effacer le tag !
      //qDebug("*** show_menu %p (Popup)", parent);
      CWIDGET_new(popup, (void *)parent, "Menu", true, true);

      if (change)
        show_menu(parent);
    }
    */

    item->container = QPOPUPMENU(parent);
  }


  for (pos = 0; pos < (int)item->container->count(); pos++)
  {
    if (item->container->idAt(pos) >= item->id)
      break;
  }

  if (item->picture && !item->checked)
  {
  	int size;
  	
  	if (item->stretch)
  		size = MAIN_scale * 2 + 2;
  	else
  		size = -1;
  		
  	CWIDGET_iconset(icon, *(item->picture->pixmap), size);
  }

  text = TO_QSTRING(item->text);

  if (CMENU_is_separator(item))
  {
#ifdef DEBUG_MENU
    qDebug("show_menu: insertSeparator(%d)", pos);
#endif
    //item->container->insertSeparator(pos);
    item->container->insertItem((QWidget *)0, item->id, pos);
  }
  else if (QPOPUPMENU(item) != 0)
  {
#ifdef DEBUG_MENU
    qDebug("show_menu: insertItem('%s', %p, %d, %d)", item->text, QPOPUPMENU(item), item->id, pos);
#endif
    if (icon.isNull())
      item->container->insertItem(text, QPOPUPMENU(item), item->id, pos);
    else
      item->container->insertItem(icon, text, QPOPUPMENU(item), item->id, pos);
  }
  else
  {
#ifdef DEBUG_MENU
    qDebug("show_menu: insertItem('%s', %d, %d)", item->text, item->id, pos);
#endif
    if (icon.isNull())
      item->container->insertItem(text, item->id, pos);
    else
      item->container->insertItem(icon, text, item->id, pos);
  }

  //if (pos < 0)
  //  item->pos = item->container->indexOf(item->id);

  item->pos = pos;
  if (!item->noshortcut)
  	item->container->setAccel(*(item->accel), item->id);
  item->container->setItemEnabled(item->id, item->enabled);
  item->container->setItemChecked(item->id, item->checked);

  if (CMENU_is_top(item))
  {
    if (item->container->count() == 1)
		{
			((QMenuBar *)item->container)->show();
			((MyMainWindow *)item->toplevel)->configure();
		}
	}

#ifdef DEBUG_MENU
  qDebug("show_menu: pos = %d", pos);
#endif

  CWIDGET_set_flag(item, WF_VISIBLE);
}


static void init_menu(CMENU *item)
{
  CMENU *parent;
  //int pos;
  bool change;
  QString text;

#ifdef DEBUG_MENU
  qDebug("init_menu: item = %p (%d) parent = %p (%d)", item, item->id, item->parent, item->parent ? item->parent->id : 0);
#endif

  parent = item->parent;

  if (parent)
  {
    if (QPOPUPMENU(parent) == 0)
    {
#ifdef DEBUG_MENU
      qDebug("init_menu: add_popup");
#endif

      QPopupMenu *popup = new QPopupMenu(item->toplevel);

      change = CMENU_is_visible(parent);

      if (change)
        hide_menu(parent);

      //QPOPUPMENU(parent) = popup;
      SET_WIDGET(parent, popup);

      //parent->dict = new QIntDict<CMENU>;

      QObject::connect(popup, SIGNAL(activated(int)), &CMenu::manager, SLOT(activated(int)));
      QObject::connect(popup, SIGNAL(aboutToShow()), &CMenu::manager, SLOT(shown()));
      QObject::connect(popup, SIGNAL(aboutToHide()), &CMenu::manager, SLOT(hidden()));
      QObject::connect(popup, SIGNAL(destroyed()), &CMenu::manager, SLOT(destroy()));

      // un popupmenu est r��enc�deux fois !
      // Et il ne faut pas effacer le tag !
      //qDebug("*** show_menu %p (Popup)", parent);
      CWIDGET_new(popup, (void *)parent, NULL, true, true);

      if (change)
        show_menu(parent);
    }

    item->container = QPOPUPMENU(parent);
  }


  if (CMENU_is_top(item))
  {
    if (item->container->count() == 0)
    {
      ((QMenuBar *)item->container)->hide();
      ((MyMainWindow *)item->toplevel)->configure();
		}
  }
}


static void delete_menu(CMENU *item)
{
#ifdef DEBUG_MENU
  qDebug("delete_menu: item = %p (%d)", item, item->id);
#endif

  if (item->deleted)
    return;

  hide_menu(item);

  /* Evite de se retrouver dans la liste des enfants
     Dans CMENU_free, le removeRef() n'aura aucun effet */
  if (item->parent)
    item->parent->children->removeRef(item);

  GB.Detach(item);

  if (QPOPUPMENU(item))
  {
    delete QPOPUPMENU(item);
    //QPOPUPMENU(item) = 0;
    CLEAR_WIDGET(item);
    item->deleted = true;
  }
  else
  {
    item->deleted = true;
    GB.Unref(POINTER(&item));
  }
}

static void unregister_menu(CMENU *_object)
{
	CACTION_register(CONTROL, NULL);
}



/*BEGIN_METHOD(CMENU_new, GB_OBJECT parent; GB_BOOLEAN visible)*/
BEGIN_METHOD(CMENU_new, GB_OBJECT parent; GB_BOOLEAN hidden)

  CMENU *item = OBJECT(CMENU);
  void *parent = VARG(parent);
  QWidget *topLevel = 0;
  QMenuBar *menuBar = 0;
  QList<CMENU> **list;

  //printf("CMENU_new %p\n", _object);

  if (GB.Is(parent, CLASS_Menu))
    topLevel = ((CMENU *)parent)->toplevel;
  else if (GB.Is(parent, CLASS_Window))
  {
    CWINDOW *window = (CWINDOW *)parent;
    topLevel = QWIDGET(CWidget::getWindow((CWIDGET *)parent)); //->topLevelWidget();
    menuBar = window->menuBar;
    if (!menuBar)
    {
      if (topLevel->isA("QMainWindow"))
        menuBar = ((QMainWindow *)topLevel)->menuBar();
      else
        menuBar = new QMenuBar(topLevel);
      window->menuBar = menuBar;
    }
    // parent = CWidget::get(QWIDGET(parent)->topLevelWidget());
    //if (parent)
    //  qwindow = (QMainWindow *)QWIDGET(parent);
  }
  else
  {
    GB.Error("Type mismatch. The parent control of a Menu must be a Window or another Menu.");
    return;
  }

  if (GB.CheckObject(parent))
    return;

  CLEAR_WIDGET(item);
  item->widget.tag.type = GB_T_NULL;
  item->widget.name = NULL;
  item->children = NULL;
  item->text = NULL;
  item->picture = NULL;
  item->toplevel = topLevel;
  item->accel = new QKeySequence();
  item->enabled = true;
  item->deleted = false;
  item->stretch = false;
	
	CWIDGET_init_name((CWIDGET *)item);

  menu_id++;
  item->id = menu_id;

  CWIDGET_clear_flag(item, WF_VISIBLE);

  if (GB.Is(parent, CLASS_Menu))
  {
    CMENU *pmenu = (CMENU *)parent;

    item->container = NULL; /* plus tard */
    item->parent = pmenu;

    list = &(pmenu->children);

    GB.Ref(parent);
  }
  else
  {
    item->container = menuBar;
    menuBar->setSeparator(QMenuBar::Never);
    item->parent = NULL;

    list = &(((CWINDOW *)parent)->menu);
  }

  CMenu::dict.insert(item->id, item);

  if (*list == NULL)
    *list = new QList<CMENU>;

  (*list)->append(item);

#ifdef DEBUG_MENU
  qDebug("CMENU_new: item = %p (%d) parent = %p (%d) toplevel = %p", item, item->id, item->parent, item->parent ? item->parent->id : 0, item->toplevel);
#endif

  //qDebug("*** CMENU_new %p", _object);
  GB.Ref(_object);

  init_menu(item);

  if (VARGOPT(hidden, FALSE))
    hide_menu(item);
  else
    show_menu(item);

END_METHOD


BEGIN_METHOD_VOID(CMENU_free)

  CMENU *item = OBJECT(CMENU);

#ifdef DEBUG_MENU
  qDebug("CMENU_free: item = %p '%s' (%d) parent = %p (%d)", item, item->text, item->id, item->parent, item->parent ? item->parent->id : 0);
#endif

  set_text(item, NULL);
  GB.StoreObject(NULL, POINTER(&(THIS->picture)));

#ifdef DEBUG_MENU
  qDebug("*** CMENU_free: free tag");
#endif

  GB.StoreVariant(NULL, &CONTROL->tag);

  CMenu::dict.remove(item->id);

  if (item->parent)
  {
    item->parent->children->removeRef(item);
    GB.Unref(POINTER(&item->parent));
  }
  else
  {
    // item->toplevel may have been destroyed, and another widget can have taken the same address!
    // But if we ensure that the returned control is a CWINDOW, and that CWINDOW has a menu list,
    // it is safe to call removeRef on it, as it cannot have the same item address in it!

    CWINDOW *parent = (CWINDOW *)CWidget::getReal(item->toplevel);

    #ifdef DEBUG_MENU
      qDebug("CMENU_free: item = %p '%s' (%d) parent = %p (%d) toplevel = %p => parent = %p", item, item->text, item->id, item->parent, item->parent ? item->parent->id : 0, item->toplevel, parent);
    #endif

    if (parent && GB.Is(parent, CLASS_Window) && parent->menu)
    {
      parent->menu->removeRef(item);
      //GB.Unref(POINTER(&parent));
    }
  }

  if (item->children)
  {
    delete item->children;
    item->children = NULL;
  }

  delete item->accel;

	GB.FreeString(&item->widget.name);

  #ifdef DEBUG_MENU
    qDebug("CMENU_free: item = %p '%s' (%d) is freed!", item, item->text, item->id);
  #endif


  //qDebug("< CMENU_free");

  //if (item->parent != NULL)
  //  if (item->parent->dict != NULL)
  //    item->parent->dict->remove(item->id);

  //if (item->children != NULL)
  //{
  //  delete item->dict;
  //  item->dict = NULL;
  //}

END_METHOD


BEGIN_PROPERTY(CMENU_text)

  if (READ_PROPERTY)
  {
    GB.ReturnString(THIS->text);
  }
  else
  {
    bool change = CMENU_is_visible(THIS);

    if (change)
      hide_menu(THIS);

    set_text(THIS, GB.ToZeroString(PROP(GB_STRING)));

    if (change)
      show_menu(THIS);

    //change_menu_item(item, GB.ToZeroString(PROPERTY(GB_STRING)));
  }

END_PROPERTY


BEGIN_PROPERTY(CMENU_picture)

  if (READ_PROPERTY)
    GB.ReturnObject(THIS->picture);
  else
  {
    bool change = CMENU_is_visible(THIS);

    if (change)
      hide_menu(THIS);

    GB.StoreObject(PROP(GB_OBJECT), POINTER(&(THIS->picture)));

    if (change)
      show_menu(THIS);
  }

END_PROPERTY


#if 0
BEGIN_PROPERTY(CMENU_stretch)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->stretch);
  else
  {
    bool change = CMENU_is_visible(THIS);

    if (change)
      hide_menu(THIS);

		THIS->stretch = VPROP(GB_BOOLEAN);

    if (change)
      show_menu(THIS);
  }

END_PROPERTY
#endif

BEGIN_PROPERTY(CMENUITEM_enabled)

  CMENU *item = OBJECT(CMENU);
  bool enabled;

  if (item->parent == 0)
  {
    if (READ_PROPERTY)
      GB.ReturnBoolean(((QMenuBar *)item->container)->isItemEnabled(item->id));
    else
    {
      enabled = VPROP(GB_BOOLEAN);
      ((QMenuBar *)item->container)->setItemEnabled(item->id, enabled);
      //qDebug("setItemEnabled");
		}
  }
  else
  {
    if (READ_PROPERTY)
      GB.ReturnBoolean(((QPopupMenu *)item->container)->isItemEnabled(item->id));
    else
    {
      enabled = VPROP(GB_BOOLEAN);
      ((QPopupMenu *)item->container)->setItemEnabled(item->id, enabled);
      item->enabled = enabled;
    }
  }

	if (!READ_PROPERTY)
	{
		//qDebug("CMENUITEM_enabled: %p %s '%s'", item, enabled ? "1" : "0", ((QString)(*item->accel)).latin1());
  	CMenu::enableAccel(item, enabled);
	}

END_PROPERTY


BEGIN_PROPERTY(CMENU_checked)

  CMENU *item = OBJECT(CMENU);

  if (item->parent == 0)
  {
    if (READ_PROPERTY)
      //GB.ReturnBoolean(((QMenuBar *)item->container)->isItemChecked(item->id));
      GB.ReturnBoolean(0);
    //else
      //((QMenuBar *)item->container)->setItemChecked(item->id, PROPERTY(char) != 0);
  }
  else
  {
    if (READ_PROPERTY)
      GB.ReturnBoolean(((QPopupMenu *)item->container)->isItemChecked(item->id));
    else
    {
	    bool change = CMENU_is_visible(THIS);

  	  if (change)
    	  hide_menu(THIS);

      item->checked = VPROP(GB_BOOLEAN);
    
    	if (change)
      	show_menu(THIS);
    }
  }

END_PROPERTY

static void toggle_menu(CMENU *_object)
{
	QPopupMenu *cont;

	if (!THIS->parent)
		return;

	//qDebug("toggle_menu: %s", THIS->text);

	cont = (QPopupMenu *)THIS->container;
	cont->setItemChecked(THIS->id, !cont->isItemChecked(THIS->id));
	THIS->checked = cont->isItemChecked(THIS->id);
}

BEGIN_PROPERTY(CMENU_toggle)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->toggle);
	else
		THIS->toggle = VPROP(GB_BOOLEAN);

END_PROPERTY


static void send_click_event(CMENU *_object);

BEGIN_PROPERTY(CMENU_value)

  CMENU *item = OBJECT(CMENU);

  if (THIS->toggle)
  {
    CMENU_checked(_object, _param);
    return;
  }

  if (READ_PROPERTY)
  {
    GB.ReturnBoolean(0);
  }
  else if (item->parent)
  {
    GB.Ref(THIS);
    send_click_event(THIS);
  }
  
END_PROPERTY

BEGIN_PROPERTY(CMENU_shortcut)

  CMENU *item = OBJECT(CMENU);
  QPopupMenu *parent;

  if (CMENU_is_popup(item))
  {
    if (READ_PROPERTY)
      GB.ReturnNull();

    return;
  }

  parent = (QPopupMenu *)item->container;

  if (READ_PROPERTY)
  {
    if (((int)(*(item->accel))) == 0)
      GB.ReturnNull();
    else
      GB.ReturnNewZeroString(((QString)*(item->accel)).latin1());
  }
  else
  {
    delete item->accel;
    item->accel = new QKeySequence(QSTRING_PROP());
    if (!item->noshortcut)
    	parent->setAccel(*(item->accel), item->id);
  }

END_PROPERTY


BEGIN_PROPERTY(CMENU_visible)

  CMENU *item = OBJECT(CMENU);

  if (READ_PROPERTY)
    GB.ReturnBoolean(CWIDGET_test_flag(item, WF_VISIBLE));
  else
  {
    if (VPROP(GB_BOOLEAN))
      show_menu(item);
    else
      hide_menu(item);
  }

END_PROPERTY


BEGIN_METHOD_VOID(CMENU_show)

	show_menu(THIS);

END_METHOD


BEGIN_METHOD_VOID(CMENU_hide)

	hide_menu(THIS);

END_METHOD


BEGIN_METHOD_VOID(CMENU_delete)

  CMENU *item = OBJECT(CMENU);

  delete_menu(item);

END_METHOD


BEGIN_PROPERTY(CMENU_count)

  CMENU *item = OBJECT(CMENU);

  if (CMENU_is_popup(item))
    GB.ReturnInteger(item->children->count());
  else
    GB.ReturnInteger(0);

END_PROPERTY


BEGIN_METHOD_VOID(CMENU_next)

  CMENU *item = OBJECT(CMENU);
  unsigned int index;

  if (item->children == NULL)
  {
    GB.StopEnum();
    return;
  }

  index = ENUM(int);

  if (index >= item->children->count())
  {
    GB.StopEnum();
    return;
  }

  GB.ReturnObject(item->children->at(index));

  ENUM(int) = index + 1;

END_METHOD


BEGIN_METHOD(CMENU_get, GB_INTEGER index)

  CMENU *item = OBJECT(CMENU);
  int index = VARG(index);

  if (item->children == NULL || index < 0 || index >= (int)item->children->count())
  {
    GB.Error(GB_ERR_BOUND);
    return;
  }

  GB.ReturnObject(item->children->at(index));

END_METHOD


BEGIN_METHOD_VOID(CMENU_clear)

  CMENU *item = OBJECT(CMENU);
  CMENU *child;

  if (CMENU_is_popup(item))
  {
    QListIterator<CMENU> it(*(item->children));

    while ((child = it.current()))
    {
      ++it;
      delete_menu(child);
    }
  }

END_METHOD


BEGIN_METHOD(CMENU_popup, GB_INTEGER x; GB_INTEGER y)

  CMENU *item = OBJECT(CMENU);

  if (CMENU_is_popup(item))
  {
    QPopupMenu *popup = QPOPUPMENU(item);

    if (popup)
    {
    	if (MISSING(x) || MISSING(y))
      	popup->exec(QCursor::pos());
			else
      	popup->exec(QPoint(VARG(x), VARG(y)));
      
      qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput, 0);
		}
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
  GB_PROPERTY("Enabled", "b", CMENUITEM_enabled),
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
CMenuDict CMenu::dict;

static void send_click_event(CMENU *_object)
{
	if (THIS->toggle)
		toggle_menu(THIS);
  GB.Raise(THIS, EVENT_Click, 0);
	CACTION_raise(CONTROL);
  GB.Unref(POINTER(&_object));
}

static void send_menu_event(CMENU *_object, long event)
{
  GB.Raise(THIS, event, 0);
  GB.Unref(POINTER(&_object));
}

void CMenu::activated(int id)
{
  CMENU *menu = CMenu::dict[id];

	GB.Ref(menu);
	GB.Post((GB_POST_FUNC)send_click_event, (long)menu);
}



void CMenu::shown(void)
{
	GET_SENDER(menu);
	hideSeparators((CMENU *)menu);
  GB.Raise(menu, EVENT_Show, 0);
}


void CMenu::hidden(void)
{
	GET_SENDER(menu);

	if (GB.CanRaise(menu, EVENT_Hide))
	{
	  GB.Ref(menu);
  	GB.Post2((GB_POST_FUNC)send_menu_event, (long)menu, EVENT_Hide);
	}
}


void CMenu::enableAccel(CMENU *item, bool enable)
{
	//if (((QString)(*item->accel)).latin1())
	//	qDebug("CMenu::enableAccel: %p %s '%s'", item, enable ? "1" : "0", ((QString)(*item->accel)).latin1());

	if (enable)
  	item->container->setAccel(*(item->accel), item->id);
	else
  	item->container->setAccel(QKeySequence(), item->id);
  	
	item->noshortcut = !enable;

	if (item->children)
	{
		CMENU *child;
		QListIterator<CMENU> it(*item->children);

		while ((child = it.current()))
		{
			++it;
			CMenu::enableAccel(child, enable);
		}
	}
}


void CMenu::hideSeparators(CMENU *item)
{
	if (item->children)
	{
		CMENU *child;
		CMENU *last_child;
		QListIterator<CMENU> it(*item->children);
		bool is_sep;
		bool last_sep;

		//qDebug("checking separators");

		last_sep = true;
		last_child = 0;
		
		for(;;)
		{	
			child = it.current();
			if (!child)
				break;
			
			++it;
			
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
	}
}



void CMenu::unrefChildren(CMenuList *list)
{
  CMENU *child;

  QListIterator<CMENU> it(*list);

  while ((child = it.current()))
  {
    ++it;

    /* ne d���encer que les enfant simples. */
    if (child->children == 0)
    {
      GB.Detach(child);
			unregister_menu(child);
      //qDebug("*** CMenu::destroy %p (child)", child);
      GB.Unref(POINTER(&child));
    }
  }
}

void CMenu::destroy(void)
{
  GET_SENDER(_object);

  #ifdef DEBUG_MENU
  qDebug("*** { CMenu::destroy %p", menu);
  #endif

  unrefChildren(THIS->children);

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

