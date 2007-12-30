/***************************************************************************

  CWindow.cpp

  The Window and Form classes

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

#define __CWINDOW_CPP

#include <qnamespace.h>
#include <qapplication.h>
#include <qmenubar.h>
#include <qframe.h>
#include <qabstractlayout.h>
#include <qsizepolicy.h>
#include <qtoolbar.h>
#include <qkeycode.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qobjectlist.h>

#if QT_VERSION >= 0x030100
  #include <qeventloop.h>
#endif

#include "main.h"

#ifndef NO_X_WINDOW
#include "qtxembed.h"
#endif

#include "gambas.h"

#include "CWidget.h"
#include "CMenu.h"
#include "CKey.h"
#include "CDraw.h"
#include "CWindow.h"

#ifndef NO_X_WINDOW
#include "x11.h"
#endif


//#define DEBUG_STATE

DECLARE_EVENT(EVENT_Open);
DECLARE_EVENT(EVENT_Close);
DECLARE_EVENT(EVENT_Activate);
DECLARE_EVENT(EVENT_Deactivate);
DECLARE_EVENT(EVENT_Move);
DECLARE_EVENT(EVENT_Resize);
DECLARE_EVENT(EVENT_Show);
DECLARE_EVENT(EVENT_Hide);
DECLARE_EVENT(EVENT_Title);
DECLARE_EVENT(EVENT_Icon);

CWINDOW *CWINDOW_Main = 0;
CWINDOW *CWINDOW_Current = 0;
CWINDOW *CWINDOW_LastActive = 0;
CWINDOW *CWINDOW_Active = 0;

long CWINDOW_Embedder = 0;
bool CWINDOW_Embedded = false;
static int CWINDOW_EmbedState = 0;

#ifndef NO_X_WINDOW
void CWINDOW_change_property(QWidget *w, Atom property, bool set)
{
  if (!w->isTopLevel())
    return;

  X11_window_change_property(w->winId(), w->isVisible(), property, set);
}

bool CWINDOW_has_property(QWidget *w, Atom property)
{
  if (!w->isTopLevel())
    return false;

  return X11_window_has_property(w->winId(), property);
}
#endif

/*---- Utility routines --------------------------------------------------------------*/

static void clear_mask(CWINDOW *_object)
{
	//qDebug("clear_mask: %p", _object);

  if (!THIS->toplevel)
  {
    WIDGET->clearMask();
    return;
  }

  if (THIS->masked && THIS->picture)
  {
    WINDOW->clearMask();

    bool v = !WINDOW->isHidden() && WINDOW->isVisible();
    WINDOW->setBorder(WINDOW->getBorder(), true);
    if (v)
      WINDOW->show();
  }
}

static void define_mask(CWINDOW *_object, CPICTURE *new_pict, bool new_mask)
{
  QPixmap p;
  QColor c;
  QWidget *root = THIS->container;
  //QBitmap b;

  //qDebug("define_mask: visible: %d", WIDGET->isVisible());

	//qDebug("define_mask: %p  new_pict = %p  new_mask = %d", _object, new_pict, new_mask);

	//if (THIS->embedded)
	//	return;

  if (!new_pict)
  {
    root->clearMask();
  	root->setErasePixmap(0);
    root->setBackgroundOrigin(QWidget::WidgetOrigin);
    root->setBackgroundMode(Qt::PaletteBackground);

    clear_mask(THIS);
  }
  else
  {
    p = *new_pict->pixmap;

    if (new_mask)
    {
      if (p.hasAlpha())
        WIDGET->setMask(*(p.mask()));
      else
        clear_mask(THIS);

      root->setErasePixmap(p);

      if (THIS->toplevel)
      {
        root->setBackgroundOrigin(QWidget::WindowOrigin);
        root->move(0, 0);
      }
    }
    else
    {
      clear_mask(THIS);
      //root->clearMask();

      //if (THIS->masked != new_mask)

      //if (THIS->toplevel)
      //	WINDOW->setCentralWidget(root);

      root->setBackgroundOrigin(QWidget::WidgetOrigin);
      root->setErasePixmap(p);
    }
  }

  THIS->masked = new_mask;

  if (new_pict != THIS->picture)
  {
    GB.Ref(new_pict);
    GB.Unref(POINTER(&THIS->picture));
    THIS->picture = new_pict;
  }
}


/***************************************************************************

  Window

***************************************************************************/

#if 1
static void show_later(CWINDOW *_object)
{
  /* If the user has explicitely hidden the window since the posting of this routines
     then do nothing
  */
  //qDebug("show_later %s %p: hidden = %d", GB.GetClassName(THIS), THIS, THIS->hidden);
  if (!THIS->hidden && WIDGET)
  {
  	//qDebug("show");
    WIDGET->show();
	}
  GB.Unref(POINTER(&_object));
}
#endif

BEGIN_METHOD(CWINDOW_new, GB_OBJECT parent)

  MyMainWindow *win = 0;
  QWidget *container;
  #ifndef NO_X_WINDOW
  QtXEmbedClient *client = 0;
  #endif
  const char *name = GB.GetClassName(THIS);

  if (MISSING(parent) || !VARG(parent))
  {
	  #ifndef NO_X_WINDOW
    if (CWINDOW_Embedder && !CWINDOW_Embedded)
    {
      client = new QtXEmbedClient;
      //container = new MyContainer(client);
		  //container->raise();
      container = client;
      THIS->embedded = true;
      THIS->toplevel = false;
      THIS->xembed = true;

      // To be called first!
      QObject::connect(client, SIGNAL(destroyed()), &CWindow::manager, SLOT(destroy()));
      client->installEventFilter(&CWindow::manager);
      CWIDGET_new(client, (void *)_object, NULL);
    }
    else
    #endif
    {
      win = new MyMainWindow(CWINDOW_Main ? (MyMainWindow *)QWIDGET(CWINDOW_Main) : 0, name);
      container = new MyContainer(win);
		  container->raise();
      THIS->embedded = false;
      THIS->toplevel = true;
      THIS->xembed = false;

      CWIDGET_new(win, (void *)_object, NULL);
    }
  }
  else
  {
  	if (GB.Conv((GB_VALUE *)(void *)ARG(parent), (GB_TYPE)CLASS_Container))
  		return;

    //frame = new MyEmbeddedWindow(QCONTAINER(VARG(parent)));
    //frame->setName(name);
    //container = frame;
    //THIS->embedded = true;
    //THIS->toplevel = false;
    //container->installEventFilter(&CWindow::manager);
    //CWIDGET_new(frame, (void *)_object, NULL);

    win = new MyMainWindow(QCONTAINER(VARG(parent)), name, true);
    container = new MyContainer(win);
    container->raise();
    THIS->embedded = true;
    THIS->toplevel = false;
		THIS->xembed = false;

    CWIDGET_new(win, (void *)_object, NULL);
  }
  /*else
  {
    GB.Error("The parent of a Window must be a Container or a Workspace");
    return;
  }*/

  THIS->container = container;
  //container->setPaletteBackgroundColor(Qt::yellow);
  //container->setBackgroundOrigin(QWidget::WindowOrigin);

  if (win)
  {
		win->_object = THIS;
    //win->setCentralWidget(container);
    //win->setOpaqueMoving(true);
    win->installEventFilter(&CWindow::manager);
  }

  if (THIS->toplevel || THIS->xembed)
  {
    /*if (CWindow::count >= 64)
    {
      GB.Error("Too many windows opened");
      return;
    }*/

    CWindow::dict.insert(_object, OBJECT(const CWINDOW));
    CWindow::count = CWindow::dict.count();

    //qDebug("CWindow::count = %d (%p %s)", CWindow::count, _object, THIS->embedded ? "E" : "W");

    if (CWINDOW_Main == 0)
    {
      //qDebug("CWINDOW_Main -> %p", THIS);
      CWINDOW_Main = THIS;
    }
  }

	#ifndef NO_X_WINDOW
  if (THIS->xembed)
  {
    CWINDOW_Embedded = true;

    QObject::connect(XEMBED, SIGNAL(embedded()), &CWindow::manager, SLOT(embedded()));
    QObject::connect(XEMBED, SIGNAL(containerClosed()), &CWindow::manager, SLOT(closed()));
    QObject::connect(XEMBED, SIGNAL(error(int)), &CWindow::manager, SLOT(error()));

    //qDebug("XEMBED: EmbedInto %ld", CWINDOW_Embedder);
    XEMBED->embedInto(CWINDOW_Embedder);
    //qDebug("XEMBED: show");
    //XEMBED->show();
    //define_mask(THIS);

    for(;;)
    {
      qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
      if (CWINDOW_EmbedState)
        break;
      usleep(10000);
    }

    //qDebug("XEMBED: EmbedState: %d", CWINDOW_EmbedState);

    if (CWINDOW_EmbedState == EMBED_ERROR)
    {
      CWINDOW_Embedded = false;
      CWINDOW_Embedder  = 0;
      GB.Error("Embedding has failed");
    }
  }
  #endif

  if (THIS->embedded && !THIS->xembed)
  {
    /* ### This can call post_show_event() directly, whereas the function is not terminated */
    //frame->show();
    GB.Ref(THIS);
    GB.Post((void (*)())show_later, (long)THIS);
    //WIDGET->show();
  }

END_METHOD


BEGIN_METHOD_VOID(CFORM_new)

  GB.Attach(_object, _object, "Form");

END_METHOD


BEGIN_METHOD_VOID(CFORM_main)

  CWINDOW *form;

  form = (CWINDOW *)GB.AutoCreate(GB.GetClass(NULL), 0);

  //GB.New(POINTER(&form), GB.GetClass(NULL), NULL, NULL);

  ((MyMainWindow *)form->widget.widget)->showActivate();

END_METHOD


BEGIN_METHOD(CFORM_load, GB_OBJECT parent)

  if (!MISSING(parent))
  {
    GB.Push(1, GB_T_OBJECT, VARG(parent));
    //qDebug("CFORM_load + parent");
  }

  GB.AutoCreate(GB.GetClass(NULL), MISSING(parent) ? 0 : 1);

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_free)

  //qDebug("CWINDOW_free");

  if (THIS->menu)
    delete THIS->menu;

  //PICTURE_set(&(window->icon), 0);
  GB.StoreObject(NULL, POINTER(&(THIS->icon)));
  GB.StoreObject(NULL, POINTER(&(THIS->picture)));
  GB.Unref(POINTER(&THIS->focus));

	if (THIS->controls)
		delete THIS->controls;

  /*CALL_METHOD_VOID(CWIDGET_delete);*/

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_next)

  CWINDOW *next;
  QPtrDictIterator<CWINDOW> *iter = ENUM(QPtrDictIterator<CWINDOW> *);

  if (iter == NULL)
  {
    iter = new QPtrDictIterator<CWINDOW>(CWindow::dict);
    ENUM(QPtrDictIterator<CWINDOW> *) = iter;
  }

  next = iter->current();

  // ### Memory leak if you abort the enumeration!

  if (next == NULL)
  {
    delete iter;
    //ENUM(QPtrDictIterator<CWINDOW>) = NULL;
    GB.StopEnum();
    return;
  }

  ++(*iter);
  GB.ReturnObject(next);

END_METHOD


BEGIN_PROPERTY(CWINDOW_count)

  GB.ReturnInteger(CWindow::dict.count());

END_PROPERTY


BEGIN_METHOD(CWINDOW_get_from_id, GB_INTEGER id)

  QWidget *wid = QWidget::find(VARG(id));

  //qDebug("id = %d wid = %p", PARAM(id), wid);

  if (wid != 0 && wid->isTopLevel())
  {
    //qDebug("-> %p", CWidget::getReal(wid));
    GB.ReturnObject(CWidget::getReal(wid));
  }
  else
  {
    //qDebug("-> %p", 0);
    GB.ReturnNull();
  }

END_METHOD


static bool do_close(CWINDOW *_object, long ret, bool destroyed = false)
{
  bool closed;

	//qDebug("do_close: (%s %p) %d %d", GB.GetClassName(THIS), THIS, CWIDGET_test_flag(THIS, WF_IN_CLOSE), CWIDGET_test_flag(THIS, WF_CLOSED));

  if (CWIDGET_test_flag(THIS, WF_IN_CLOSE) || CWIDGET_test_flag(THIS, WF_CLOSED)) // || WIDGET->isHidden())
    return false;

  if (!THIS->toplevel)
  {
  	if (THIS->shown)
  	{
			//qDebug("THIS->shown = %d: %p: %s", THIS->shown, THIS, GB.GetClassName(THIS));
    	CWIDGET_set_flag(THIS, WF_IN_CLOSE);
    	closed = !GB.Raise(THIS, EVENT_Close, 0);
    	CWIDGET_clear_flag(THIS, WF_IN_CLOSE);
		}
		else
			closed = true;

    if (destroyed || closed)
    {
      CWIDGET_set_flag(THIS, WF_CLOSED);
    	/*CWIDGET_set_flag(THIS, WF_IN_CLOSE);
			qApp->sendEvent(WIDGET, new QEvent(EVENT_CLOSE));
    	CWIDGET_clear_flag(THIS, WF_IN_CLOSE);*/
    }

    if (closed)
    {
      WIDGET->hide();
      if (!CWIDGET_test_flag(_object, WF_PERSISTENT))
        CWIDGET_destroy((CWIDGET *)THIS);
    }
  }
  else
  {
    if (!THIS->shown)
    {
    	//qDebug("send close event");
      QCloseEvent e;
      QApplication::sendEvent(WINDOW, &e);
      closed = e.isAccepted();
    }
    else
    {
    	//qDebug("call WINDOW->close()");
      closed = WINDOW->close();
		}
  }

  #if 0
	if (closed || destroyed) 
	{
		if (CWINDOW_Active == THIS)
			CWINDOW_activate(CWidget::get(WIDGET->parentWidget()));
		if (CWINDOW_LastActive == THIS)
		{
			//GB.Unref(POINTER(&CWINDOW_LastActive));
			CWINDOW_LastActive = 0;
			//qDebug("CWINDOW_LastActive = 0");
		}
		THIS->shown = FALSE;
	}
	#endif

  if (closed)
    THIS->ret = ret;

  //qDebug("CWINDOW_close: ret = %d", THIS->ret);

  return (!closed);
}

BEGIN_METHOD(CWINDOW_close, GB_INTEGER ret)

  long ret = VARGOPT(ret, 0);

  GB.ReturnBoolean(do_close(THIS, ret));

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_raise)

  if (!THIS->toplevel)
  {
    if (!WIDGET->isVisible())
      WIDGET->show();
    WIDGET->raise();
  }
  else
  {
    if (!WINDOW->isVisible())
      WINDOW->showActivate();
    else
      WINDOW->raise();
  }

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_show)

  if (!THIS->toplevel)
  {
	  CWIDGET_clear_flag(THIS, WF_CLOSED);
    WIDGET->show();
  }
  else
  {
    //if (CWINDOW_Current)
    //  WINDOW->showModal(); //GB.Error("A modal window is already displayed");
    //else
    WINDOW->showActivate();
  }

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_hide)

  if (THIS->toplevel && WINDOW->testWFlags(Qt::WShowModal))
  {
    do_close(THIS, 0);
  }
  else
  	WINDOW->hide();

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_show_modal)

  THIS->ret = 0;

  if (THIS->toplevel)
    WINDOW->showModal();
  //qDebug("CWINDOW_show_modal: ret = %d", THIS->ret);
  GB.ReturnInteger(THIS->ret);

END_METHOD


BEGIN_PROPERTY(CWINDOW_modal)

  if (THIS->toplevel)
    GB.ReturnBoolean(WINDOW->isModal());
  else
    GB.ReturnBoolean(false);

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_top_level)

  GB.ReturnBoolean(THIS->toplevel);

END_PROPERTY

/*BEGIN_METHOD_VOID(CWINDOW_dialog)

  CWINDOW *win;

  GB.New(POINTER(&win), GB.GetClass(NULL), NULL, NULL);

  win->ret = 0;
  ((MyMainWindow *)win->widget.widget)->showModal();
  GB.ReturnInteger(win->ret);

END_METHOD*/


BEGIN_PROPERTY(CWINDOW_persistent)

  /*
  if (READ_PROPERTY)
    GB.ReturnBoolean(WIDGET->isPersistent());
  else
    WIDGET->setPersistent(PROPERTY(char) != 0);
  */

  if (!THIS->toplevel)
  {
    if (READ_PROPERTY)
      GB.ReturnBoolean(true);
  }
  else
  {
    if (READ_PROPERTY)
      GB.ReturnBoolean(CWIDGET_test_flag(THIS, WF_PERSISTENT));
    else
    {
      if (VPROP(GB_BOOLEAN))
        CWIDGET_set_flag(THIS, WF_PERSISTENT);
      else
        CWIDGET_clear_flag(THIS, WF_PERSISTENT);
    }
  }

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(WIDGET->caption()));
	else
	{
		QString s = QSTRING_PROP();
		THIS->title = s.length() > 0;
		WIDGET->setCaption(s);
		GB.Raise(THIS, EVENT_Title, 0);
    qApp->sendEvent(WIDGET, new QEvent(EVENT_TITLE));
	}


END_PROPERTY


BEGIN_PROPERTY(CWINDOW_menu_count)

  if (THIS->menu)
    GB.ReturnInteger(THIS->menu->count());
  else
    GB.ReturnInteger(0);

END_PROPERTY


BEGIN_METHOD_VOID(CWINDOW_menu_next)

  CWINDOW *window = OBJECT(CWINDOW);
  unsigned int index;

  if (window->menu == NULL)
  {
    GB.StopEnum();
    return;
  }

  index = ENUM(int);

  if (index >= window->menu->count())
  {
    GB.StopEnum();
    return;
  }

  GB.ReturnObject(window->menu->at(index));

  ENUM(int) = index + 1;

END_PROPERTY


BEGIN_METHOD(CWINDOW_menu_get, GB_INTEGER index)

  CWINDOW *window = OBJECT(CWINDOW);
  int index = VARG(index);

  if (window->menu == NULL || index < 0 || index >= (int)window->menu->count())
  {
    GB.Error(GB_ERR_BOUND);
    return;
  }

  GB.ReturnObject(window->menu->at(index));

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_border)

  if (!THIS->toplevel)
  {
    if (READ_PROPERTY)
      GB.ReturnInteger(0);
  }
  else
  {
    if (READ_PROPERTY)
      GB.ReturnInteger(WINDOW->getBorder());
    else
      WINDOW->setBorder(VPROP(GB_INTEGER));
  }

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_icon)

	if (READ_PROPERTY)
		GB.ReturnObject(THIS->icon);
	else
	{
		//if (THIS->toplevel)
		//	SET_PIXMAP(WINDOW->setIcon, &(THIS->icon), PROP(GB_OBJECT));
		//else
			SET_PIXMAP(WIDGET->setIcon, &(THIS->icon), PROP(GB_OBJECT));
		//WIDGET->setIcon(PICTURE_set(&(THIS->icon), PROPERTY(CPICTURE *)));
    GB.Raise(THIS, EVENT_Icon, 0);
    qApp->sendEvent(WIDGET, new QEvent(EVENT_ICON));
	}

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_picture)

  if (READ_PROPERTY)
    GB.ReturnObject(THIS->picture);
  else
  {
    //GB.StoreObject(PROP(GB_OBJECT), POINTER(&THIS->picture));
    define_mask(THIS, (CPICTURE *)VPROP(GB_OBJECT), THIS->masked);;
  }

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_mask)

  /*if (THIS->embedded)
  {
    if (READ_PROPERTY)
      GB.ReturnBoolean(FALSE);
  }
  else*/
  {
    if (READ_PROPERTY)
      GB.ReturnBoolean(THIS->masked);
    else
    {
      //THIS->masked = VPROP(GB_BOOLEAN);
      define_mask(THIS, THIS->picture, VPROP(GB_BOOLEAN));
    }
  }

END_PROPERTY

#if 0
BEGIN_PROPERTY(CWINDOW_state)

  if (!THIS->toplevel)
  {
    if (READ_PROPERTY)
      GB.ReturnInteger(0);
  }
  else
  {
    if (READ_PROPERTY)
      GB.ReturnInteger(WINDOW->getState());
    else
      WINDOW->setState(VPROP(GB_INTEGER));
  }

END_PROPERTY
#endif

static void manage_window_state(void *_object, void *_param, Qt::WindowState state)
{
  if (!THIS->toplevel)
  {
    if (READ_PROPERTY)
      GB.ReturnBoolean(FALSE);
  }
  else
  {
    if (READ_PROPERTY)
      GB.ReturnBoolean(WINDOW->windowState() & state);
    else
    {
    	if (VPROP(GB_BOOLEAN))
	      WINDOW->setWindowState(WINDOW->windowState() | state);
    	else
	      WINDOW->setWindowState(WINDOW->windowState() & ~state);
		}
  }
}

BEGIN_PROPERTY(CWINDOW_minimized)

  manage_window_state(_object, _param, Qt::WindowMinimized);

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_maximized)

  manage_window_state(_object, _param, Qt::WindowMaximized);

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_full_screen)

  manage_window_state(_object, _param, Qt::WindowFullScreen);

END_PROPERTY


#ifdef NO_X_WINDOW //------------------------------------------------------------------------------

BEGIN_PROPERTY(CWINDOW_stacking)

	if (READ_PROPERTY)
		GB.ReturnInteger(0);

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_top_only)

  if (READ_PROPERTY)
    GB.ReturnBoolean(FALSE);
    
END_PROPERTY


BEGIN_PROPERTY(CWINDOW_skip_taskbar)

  if (READ_PROPERTY)
    GB.ReturnBoolean(FALSE);

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_sticky)

  if (READ_PROPERTY)
    GB.ReturnBoolean(FALSE);

END_PROPERTY

#else //-------------------------------------------------------------------------------------------

static void manage_window_property(void *_object, void *_param, Atom property)
{
  if (!THIS->toplevel)
  {
    if (READ_PROPERTY)
      GB.ReturnBoolean(FALSE);
  }
  else
  {
    if (READ_PROPERTY)
      GB.ReturnBoolean(CWINDOW_has_property(WINDOW, property));
    else
      CWINDOW_change_property(WINDOW, property, VPROP(GB_BOOLEAN));
  }
}

BEGIN_PROPERTY(CWINDOW_stacking)

  int p;
  
  if (!THIS->toplevel)
  {
    if (READ_PROPERTY)
      GB.ReturnInteger(0);
		return;
  }
	
	if (READ_PROPERTY)
	{
		if (CWINDOW_has_property(WINDOW, X11_atom_net_wm_state_above))
			p = 1;
		else if (CWINDOW_has_property(WINDOW, X11_atom_net_wm_state_below))
			p = 2;
		else
			p = 0;

		GB.ReturnInteger(p);
	}
	else
	{
		THIS->stacking = p = VPROP(GB_INTEGER);
		CWINDOW_change_property(WINDOW, X11_atom_net_wm_state_above, p == 1);
		CWINDOW_change_property(WINDOW, X11_atom_net_wm_state_stays_on_top, p == 1);
		CWINDOW_change_property(WINDOW, X11_atom_net_wm_state_below, p == 2);
	}

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_top_only)

  if (!THIS->toplevel)
  {
    if (READ_PROPERTY)
      GB.ReturnBoolean(FALSE);
		return;
  }
	
	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(CWINDOW_has_property(WINDOW, X11_atom_net_wm_state_above));
	}
	else
	{
		THIS->stacking = VPROP(GB_BOOLEAN) ? 1 : 0;
		CWINDOW_change_property(WINDOW, X11_atom_net_wm_state_above, THIS->stacking == 1);
		CWINDOW_change_property(WINDOW, X11_atom_net_wm_state_stays_on_top, THIS->stacking == 1);
	}
	
END_PROPERTY


BEGIN_PROPERTY(CWINDOW_skip_taskbar)

  manage_window_property(_object, _param, X11_atom_net_wm_state_skip_taskbar);

  if (!READ_PROPERTY)
    THIS->skipTaskbar = VPROP(GB_BOOLEAN);

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_sticky)

  if (!THIS->toplevel)
  {
    if (READ_PROPERTY)
      GB.ReturnBoolean(FALSE);
		return;
  }
	
	if (READ_PROPERTY)
		GB.ReturnBoolean(X11_window_get_desktop(WINDOW->winId()) < 0);
	else
		X11_window_set_desktop(WINDOW->winId(), WINDOW->isVisible(), VPROP(GB_BOOLEAN) ? 0xFFFFFFFF : X11_get_current_desktop());

END_PROPERTY

#endif //------------------------------------------------------------------------------------------


BEGIN_PROPERTY(CWINDOW_tool)

  if (!THIS->toplevel)
  {
    if (READ_PROPERTY)
      GB.ReturnBoolean(FALSE);
  }
  else
  {
  	/*
    if (READ_PROPERTY)
      GB.ReturnBoolean(X11_get_window_tool(WINDOW->winId()));
    else
    {
      THIS->toolbar = VPROP(GB_BOOLEAN);
      X11_set_window_tool(WINDOW->winId(), THIS->toolbar, CWINDOW_Main ? ((MyMainWindow *)QWIDGET(CWINDOW_Main))->winId() : 0);
		}*/
		if (READ_PROPERTY)
			GB.ReturnBoolean(WINDOW->getTool());
		else
			WINDOW->setTool(VPROP(GB_BOOLEAN));
  }

END_PROPERTY


BEGIN_METHOD_VOID(CWINDOW_center)

  if (!THIS->toplevel)
    return;

  WINDOW->center(true);

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_delete)

  //qDebug("CWINDOW_delete %p", THIS);

  do_close(THIS, 0);

  if (THIS->toplevel)
    CWIDGET_clear_flag(THIS, WF_PERSISTENT);

  CWIDGET_destroy((CWIDGET *)THIS);

END_METHOD


BEGIN_PROPERTY(CWINDOW_visible)

  if (READ_PROPERTY)
    GB.ReturnBoolean(!WINDOW->isHidden());
  else
  {
    if (VPROP(GB_BOOLEAN))
      CWINDOW_show(_object, _param);
    else
      CWINDOW_hide(_object, _param);
  }

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_control_count)

	QObjectList *list = WINDOW->queryList("QWidget");
  QObjectListIt it(*list);
  QObject *obj;
  int n = 0;

  while ((obj = it.current()) != 0)
  {
    ++it;
    if (CWidget::getReal((QWidget *)obj))
    	n++;
  }

  GB.ReturnInteger(n);

END_PROPERTY


BEGIN_METHOD_VOID(CWINDOW_control_next)

	CWIDGET *control;
	int index;

	index = ENUM(int);

	if (index == 0)
	{
		if (THIS->controls)
			delete THIS->controls;

		THIS->controls = WINDOW->queryList("QWidget");
	}

	do
	{
  	if (index >= (int)THIS->controls->count())
  	{
	    GB.StopEnum();
    	return;
  	}

  	control = CWidget::getReal((QWidget *)THIS->controls->at(index));
  	index++;
	}
	while (!control);

  ENUM(int) = index;
  GB.ReturnObject(control);


END_PROPERTY


BEGIN_METHOD(CWINDOW_reparent, GB_OBJECT container; GB_INTEGER x; GB_INTEGER y)

	QPoint p;
	void *parent = VARG(container);
	QWidget *newParentWidget;
	//bool showIt = !WIDGET->isHidden();

	if (!MISSING(x) && !MISSING(y))
	{
		p.setX(VARG(x));
		p.setY(VARG(y));
	}
	else if (THIS->toplevel)
  {
	   p.setX(THIS->x);
	   p.setY(THIS->y);
  }
  else
    p = WIDGET->pos();

	if (!parent)
		newParentWidget = 0;
	else
	{
		if (GB.CheckObject(parent))
			return;
		newParentWidget = QCONTAINER(parent);
	}

	WINDOW->doReparent(newParentWidget, p);

END_METHOD


BEGIN_METHOD(CWINDOW_get, GB_STRING name)

	GB.ReturnObject(WINDOW->names[GB.ToZeroString(ARG(name))]);

END_METHOD


BEGIN_PROPERTY(CWINDOW_closed)

	GB.ReturnBoolean(CWIDGET_test_flag(THIS, WF_CLOSED));

END_PROPERTY


/***************************************************************************/

GB_DESC CWindowMenusDesc[] =
{
  GB_DECLARE(".WindowMenus", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_next", "Menu", CWINDOW_menu_next, NULL),
  GB_METHOD("_get", "Menu", CWINDOW_menu_get, "(Index)i"),
  GB_PROPERTY_READ("Count", "i", CWINDOW_menu_count),

  GB_END_DECLARE
};

GB_DESC CWindowControlsDesc[] =
{
  GB_DECLARE(".WindowControls", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_next", "Control", CWINDOW_control_next, NULL),
  GB_METHOD("_get", "Control", CWINDOW_get, "(Name)s"),
  GB_PROPERTY_READ("Count", "i", CWINDOW_control_count),

  GB_END_DECLARE
};

/*
GB_DESC CWindowToolBarsDesc[] =
{
  GB_DECLARE(".Window.ToolBars", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("Add", NULL, CWINDOW_toolbar_add, "'Toolbar'Toolbar;"),
  GB_METHOD("Remove", NULL, CWINDOW_toolbar_remove, "'Toolbar'Toolbar;"),

  GB_END_DECLARE
};
*/


GB_DESC CWindowDesc[] =
{
  GB_DECLARE("Window", sizeof(CWINDOW)), GB_INHERITS("Container"),

  GB_CONSTANT("None", "i", 0),
  GB_CONSTANT("Fixed", "i", 1),
  GB_CONSTANT("Resizable", "i", 2),

  GB_CONSTANT("Normal", "i", 0),
  GB_CONSTANT("Above", "i", 1),
  GB_CONSTANT("Below", "i", 2),

  GB_METHOD("_new", NULL, CWINDOW_new, "[(Parent)Control;]"),
  GB_METHOD("_free", NULL, CWINDOW_free, NULL),
  GB_METHOD("_get", "Control", CWINDOW_get, "(Name)s"),

  GB_METHOD("Close", "b", CWINDOW_close, "[(Return)i]"),
  GB_METHOD("Raise", NULL, CWINDOW_raise, NULL),
  GB_METHOD("Show", NULL, CWINDOW_show, NULL),
  GB_METHOD("Hide", NULL, CWINDOW_hide, NULL),
  GB_METHOD("ShowModal", "i", CWINDOW_show_modal, NULL),
  GB_METHOD("ShowDialog", "i", CWINDOW_show_modal, NULL),
  GB_METHOD("Center", NULL, CWINDOW_center, NULL),
  GB_PROPERTY_READ("Modal", "b", CWINDOW_modal),
  GB_PROPERTY_READ("TopLevel", "b", CWINDOW_top_level),
  GB_PROPERTY_READ("Closed", "b", CWINDOW_closed),

  GB_METHOD("Delete", NULL, CWINDOW_delete, NULL),

  GB_METHOD("Reparent", NULL, CWINDOW_reparent, "(Container)Container;[(X)i(Y)i]"),

  GB_PROPERTY("Persistent", "b", CWINDOW_persistent),
  GB_PROPERTY("Text", "s", CWINDOW_text),
  GB_PROPERTY("Title", "s", CWINDOW_text),
  GB_PROPERTY("Caption", "s", CWINDOW_text),
  GB_PROPERTY("Icon", "Picture", CWINDOW_icon),
  GB_PROPERTY("Picture", "Picture", CWINDOW_picture),
  GB_PROPERTY("Mask", "b", CWINDOW_mask),
  GB_PROPERTY("Border", "i", CWINDOW_border),
  GB_PROPERTY("Minimized", "b", CWINDOW_minimized),
  GB_PROPERTY("Maximized", "b", CWINDOW_maximized),
  GB_PROPERTY("FullScreen", "b", CWINDOW_full_screen),
  GB_PROPERTY("TopOnly", "b", CWINDOW_top_only),
  GB_PROPERTY("Stacking", "i", CWINDOW_stacking),
  GB_PROPERTY("Sticky", "b", CWINDOW_sticky),
  GB_PROPERTY("SkipTaskbar", "b", CWINDOW_skip_taskbar),
  GB_PROPERTY("ToolBox", "b", CWINDOW_tool),
  GB_PROPERTY("Visible", "b", CWINDOW_visible),
  GB_PROPERTY("Arrangement", "i", CCONTAINER_arrangement),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),
  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),
  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),

  GB_PROPERTY_SELF("Menus", ".WindowMenus"),
  GB_PROPERTY_SELF("Controls", ".WindowControls"),

  GB_CONSTANT("_Properties", "s", CWINDOW_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Open"),
  GB_CONSTANT("_Arrangement", "i", ARRANGE_FILL),

  GB_EVENT("Close", "b", NULL, &EVENT_Close),
  GB_EVENT("Open", NULL, NULL, &EVENT_Open),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("Deactivate", NULL, NULL, &EVENT_Deactivate),
  GB_EVENT("Move", NULL, NULL, &EVENT_Move),
  GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),
  GB_EVENT("Show", NULL, NULL, &EVENT_Show),
  GB_EVENT("Hide", NULL, NULL, &EVENT_Hide),
  GB_EVENT("Title", NULL, NULL, &EVENT_Title),
  GB_EVENT("Icon", NULL, NULL, &EVENT_Icon),
  
  GB_INTERFACE("Draw", &DRAW_Interface),

  GB_END_DECLARE
};


GB_DESC CWindowsDesc[] =
{
  GB_DECLARE("Windows", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_next", "Window", CWINDOW_next, NULL),
  GB_STATIC_METHOD("_get", "Window", CWINDOW_get_from_id, "(Id)i"),
  GB_STATIC_PROPERTY_READ("Count", "i", CWINDOW_count),

  GB_END_DECLARE
};


GB_DESC CFormDesc[] =
{
  GB_DECLARE("Form", sizeof(CFORM)), GB_INHERITS("Window"),
  GB_AUTO_CREATABLE(),

  GB_STATIC_METHOD("Main", NULL, CFORM_main, NULL),
  GB_STATIC_METHOD("Load", NULL, CFORM_load, "[(Parent)Control;]"),
  GB_METHOD("_new", NULL, CFORM_new, NULL),

  GB_END_DECLARE
};


/***************************************************************************

  MyMainWindow

***************************************************************************/

#if QT_VERSION >= 0x030005
MyMainWindow::MyMainWindow(QWidget *parent, const char *name, bool embedded) :
  QMainWindow::QMainWindow(parent, name, embedded ? 0 : (Qt::WType_TopLevel | (parent ? 0 : Qt::WGroupLeader)))
#else
MyMainWindow::MyMainWindow(QWidget *parent, const char *name, bool embedded) :
  QMainWindow::QMainWindow(0, name) //, 0, Qt::WType_TopLevel) // | (copy ? copy->getWFlags() : 0)) // | Qt::WDestructiveClose)
#endif
{
  sg = 0;
  //shown = false;
  border = BorderResizable;
  //state = StateNormal;
  mustCenter = false;

	setKeyCompression(true);
	//setFocusPolicy(ClickFocus);
	setInputMethodEnabled(true);

	_activate = false;
}


static void remove_window_check_quit(CWINDOW *ob)
{
  CWindow::dict.remove(ob);

  //if (ob == window_main)
  //  window_main = NULL;

  CWindow::count = CWindow::dict.count();
  //qDebug("~MyMainWindow: CWindow::count = %d (%p %s)", CWindow::count, ob, ob->embedded ? "E" : "W");

  MAIN_check_quit();
}


MyMainWindow::~MyMainWindow()
{
  CWINDOW *_object = (CWINDOW *)CWidget::get(this);

  do_close(THIS, 0, true);

	if (CWINDOW_Active == THIS)
		CWINDOW_Active = 0;
	if (CWINDOW_LastActive == THIS)
	{
		CWINDOW_LastActive = 0;
		//qDebug("CWINDOW_LastActive = 0");
	}

  if (sg)
    delete sg;

  /*if (THIS == NULL)
  {
    qWarning("~MyMainWindow: ob == NULL");
    return;
  }*/

  //do_close(ob, 0, true);

  GB.Detach(THIS);

	if (THIS->menu)
		CMenu::unrefChildren(THIS->menu);

	remove_window_check_quit(THIS);

  //qDebug("~MyMainWindow %p (end)", this);
}


void MyMainWindow::showEvent(QShowEvent *e)
{
  if (_activate)
  {
	  //CWINDOW *ob = (CWINDOW *)CWidget::get(this);
  	//qDebug("_activate: %s %p", GB.GetClassName(ob), ob);
    setActiveWindow();
    raise();
    setFocus();
    _activate = false;
  }
}

void MyMainWindow::initProperties()
{
	#ifndef NO_X_WINDOW
		CWIDGET *_object = CWidget::get(this);
	
		if (!THIS->toplevel)
			return;
	
		CWINDOW_change_property(this, X11_atom_net_wm_state_above, THIS->stacking == 1);
		CWINDOW_change_property(this, X11_atom_net_wm_state_stays_on_top, THIS->stacking == 1);
		CWINDOW_change_property(this, X11_atom_net_wm_state_below, THIS->stacking == 2);
		CWINDOW_change_property(this, X11_atom_net_wm_state_skip_taskbar, THIS->skipTaskbar);
  #endif
}

void MyMainWindow::afterShow()
{
	if (!CWIDGET_test_flag(THIS, WF_CLOSED))
	{
		define_mask(THIS, THIS->picture, THIS->masked);	
		THIS->loopLevel = CWINDOW_Current ? CWINDOW_Current->loopLevel : 0;
	}
}

void MyMainWindow::showActivate()
{
  CWIDGET *_object = CWidget::get(this);
  CWINDOW *parent;
  QWidget *newParentWidget;

  //qDebug(">> Show %d %d %d %d", x(), y(), width(), height());

  if (CWIDGET_test_flag(THIS, WF_IN_CLOSE) || CWIDGET_test_flag(THIS, WF_IN_SHOW))
  {
    //qDebug("Showing form during a close event !");
    return;
  }

	// Reparent the window if, for example, there is an already modal window displayed
	
	parent = CWINDOW_Current;
	if (!parent && THIS != CWINDOW_Main)
		parent = CWINDOW_Main;
		
	if (parent)
		newParentWidget = parent->widget.widget;
	else
		newParentWidget = 0;
		
	if (parentWidget() != newParentWidget)
	{
		//qDebug("reparent (%s %p) to (%s %p) / %d", GB.GetClassName(THIS), THIS, parent ? GB.GetClassName(parent) : "", parent, THIS->toplevel);
		doReparent(newParentWidget, getWFlags(), pos());
	}

  //qDebug("showActivate %p", _object);

  CWIDGET_clear_flag(THIS, WF_CLOSED);

  CWIDGET_set_flag(THIS, WF_IN_SHOW);

	if (!THIS->title && border != BorderNone)
		setCaption(GB.Application.Title());

  initProperties();

  if (!THIS->shown)
  {
    //GB.Raise(THIS, EVENT_Open, 0);

    //X11_window_startup(WINDOW->winId(), THIS->x, THIS->y, THIS->w, THIS->h);

		if (windowState() & Qt::WindowMinimized)
			showMinimized();
		else if (windowState() & Qt::WindowFullScreen)
			showFullScreen();
		else if (windowState() & Qt::WindowMaximized)
			showMaximized();
		else
			show();

		if (getTool())
		{
			qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
			usleep(50000);
			setActiveWindow();
		}

    //THIS->shown = true;
    //qDebug("THIS->shown <- true: %p: %s", THIS, GB.GetClassName(THIS));

		//if (!(THIS->toplevel && THIS->embedded))
		//	GB.Raise(THIS, EVENT_Move, 0);
   	//GB.Raise(THIS, EVENT_Resize, 0);

  }
  else
  {
    _activate = true;
    
    if (windowState() & WindowMinimized)
    {
      setWindowState(windowState() & ~WindowMinimized);
			//qDebug("_activate set #2");
    }
    
    if (!isVisible())
    {
      show();
      if (getTool())
      {
	      qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
				usleep(50000);
      	setActiveWindow();
			}
    }
    else
    {
			raise();
			setActiveWindow();
    }

  }

	afterShow();

  CWIDGET_clear_flag(THIS, WF_IN_SHOW);
}

static void post_show_event(void *_object);

void MyMainWindow::showModal(void)
{
  WFlags flags = getWFlags();
  CWIDGET *_object = CWidget::get(this);
  bool persistent = CWIDGET_test_flag(THIS, WF_PERSISTENT);
  QWidget *parent = parentWidget();
  QWidget *reparent;
  CWINDOW *save = CWINDOW_Current;
  QPoint p = pos();

  if (CWIDGET_test_flag(THIS, WF_IN_CLOSE) || CWIDGET_test_flag(THIS, WF_IN_SHOW))
    return;
  
  if (isModal())
    return;

  mustCenter = true;

  reparent = qApp->activeWindow();
  if (!reparent && CWINDOW_Main)
  {
    reparent = CWINDOW_Main->widget.widget;
    if (reparent == this)
      reparent = 0;
  }

  #if QT_VERSION >= 030300
    doReparent(reparent, getWFlags() | WStyle_DialogBorder | WShowModal, p);
  #else
    doReparent(reparent, getWFlags() | WType_Dialog | WShowModal | WStyle_DialogBorder | WStyle_Dialog, p);
  #endif

  if (border == BorderResizable)
  {
  	if (!THIS->minsize)
  	{
			setMinimumSize(width(), height());
			THIS->minsize = true;
		}
    setSizeGrip(true);
	}

	THIS->enterLoop = false; // Do not call exitLoop() if we do not entered the loop yet!
	
  CWIDGET_clear_flag(THIS, WF_CLOSED); // Normaly done in showActivate()
  
  CWIDGET_set_flag(THIS, WF_IN_SHOW);
	post_show_event(THIS);
  CWIDGET_clear_flag(THIS, WF_IN_SHOW);

	if (!CWIDGET_test_flag(THIS, WF_CLOSED))
	{
		//qDebug("showModal: %p", THIS);
  	show();
		afterShow();
  	
	  THIS->loopLevel++;
		CWINDOW_Current = THIS;
		THIS->enterLoop = true;
		//qDebug("enterLoop: %p\n", THIS);
		qApp->eventLoop()->enterLoop();
		CWINDOW_Current = save;
	}
	
  if (persistent)
  {
    setSizeGrip(false);
    clearWState(WShowModal);
    setWFlags(flags);
    doReparent(parent, flags, p);
  }
}


void MyMainWindow::setTool(bool t)
{
  WFlags f = getWFlags();

	if (t)
		f |=  WStyle_Tool | WStyle_Customize;
	else
		f &= ~WStyle_Tool;

 	doReparent(CWINDOW_Main ? (MyMainWindow *)QWIDGET(CWINDOW_Main) : 0, f, pos());
}

void MyMainWindow::moveSizeGrip()
{
  CWINDOW *window;
  QWidget *cont;

  if (sg == 0)
    return;

  window = (CWINDOW *)CWidget::get(this);
  cont = window->container;

	if (qApp->reverseLayout())
  	sg->move(cont->rect().bottomLeft() - sg->rect().bottomLeft());
	else
  	sg->move(cont->rect().bottomRight() - sg->rect().bottomRight());
}

void MyMainWindow::setSizeGrip(bool on)
{
  if (on == (sg != 0))
    return;

  if (!on)
  {
    delete sg;
    sg = 0;
  }
  else //if (!parentWidget())
  {
    sg = new QSizeGrip(((CWINDOW *)CWidget::get(this))->container);
    sg->adjustSize();
    moveSizeGrip();
    sg->lower();
    if (paletteBackgroundPixmap())
      sg->setBackgroundOrigin(QWidget::AncestorOrigin);
    sg->show();
  }
}

void MyMainWindow::setBorder(int b, bool force)
{
  int f;

  if (force)
    border = BorderNone;
  else
  {
    if (b == border || b < 0 || b > 2)
      return;
  }

  if (b == BorderNone)
  {
    //clearWFlags(Qt::WStyle_NormalBorder);
    //clearWFlags(Qt::WStyle_DialogBorder);
    //setWFlags(Qt::WStyle_NoBorderEx);
    //reparent(parentWidget(), getWFlags(), pos());

    f = WStyle_Customize | WStyle_NoBorderEx | getWFlags(); // & 0xffff0000);

    /*if (f & WStyle_StaysOnTop)
      f |= WType_Popup;
    else
      f &= ~WType_Popup;*/

		f |= WType_TopLevel;

    doReparent(parentWidget(), f, pos());

    border = b;
    return;
  }


  if (border == BorderNone)
    doReparent(parentWidget(), WType_TopLevel | (getWFlags() /*& 0xffff0000*/), pos()); //QPoint(0,0) );

  if (b == BorderFixed)
  {
  	if (layout())
    	layout()->setResizeMode(QLayout::FreeResize);
    setMinimumSize(width(), height());
    setMaximumSize(width(), height());
  }
  else
  {
    setMinimumSize(0, 0);
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
  	if (layout())
	    layout()->setResizeMode(QLayout::Minimum);
  }

  border = b;
}


void MyMainWindow::paintUnclip(bool on)
{
  if (on)
    setWFlags(Qt::WPaintUnclipped);
  else
    clearWFlags(Qt::WPaintUnclipped);
}


void MyMainWindow::moveEvent(QMoveEvent *e)
{
  CWIDGET *_object = CWidget::getReal(this);

  //qDebug("Move: (%s %p) %d %d", GB.GetClassName(THIS), THIS, e->pos().x(), e->pos().y());

  QMainWindow::moveEvent(e);

  //qDebug("Move (pos %d %d) (oldPos %d %d)", e->pos().x(), e->pos().y(), e->oldPos().x(), e->oldPos().y());
  //qDebug("     (geom %d %d) (fgeom %d %d)", geometry().x(), geometry().y(), frameGeometry().x(), frameGeometry().y());
  //qDebug("     Visible = %s Hidden = %s", (isVisible() ? "Yes" : "No"), (isHidden() ? "Yes" : "No"));
  //qDebug("     Flags = 0x%s State = 0x%s", QString::number(getWFlags(), 16).latin1(), QString::number(getWState(), 16).latin1());

  //if (CWIDGET_test_flag(ob, WF_IGNORE_MOVE))

	//if (THIS->embedded)
	//	return;

	if (THIS->toplevel)
	{
		if (!testWFlags(Qt::WStyle_NoBorderEx))
		{
			if (geometry().x() == frameGeometry().x() && geometry().y() == frameGeometry().y())
			{
				//qDebug("...Ignore");
				return;
			}
		}

		if (!isHidden())
		{
			THIS->x = x();
			THIS->y = y();
			//qDebug("moveEvent: x= %d y = %d", x(), y());
		}
	}

  //qDebug("moveEvent %ld %ld isHidden:%s shown:%s ", THIS->x, THIS->y, isHidden() ? "1" : "0", shown ? "1" : "0");

  if (THIS->shown)
    GB.Raise(THIS, EVENT_Move, 0);
}



/*
static void post_resize_event(CWINDOW *_object)
{
  qDebug("post resize: %d %d", THIS->w, THIS->h);
  WINDOW->resize(THIS->w, THIS->h);
  GB.Unref(POINTER(&_object));
}
*/

void MyMainWindow::resizeEvent(QResizeEvent *e)
{
  CWINDOW *_object = (CWINDOW *)CWidget::getReal(this);
  //int w, h;

  //qDebug("Resize");

  //qDebug("Resize %p: %d %d <- %d %d", _object, e->size().width(), e->size().height(), e->oldSize().width(), e->oldSize().height());

  //QMainWindow::resizeEvent(e);

	configure();

  if (sg)
    moveSizeGrip();

  if (!isHidden())
  {
    THIS->w = THIS->container->width();
    THIS->h = THIS->container->height();
    //qDebug("THIS->w = %d  THIS->h = %d", THIS->w, THIS->h);
  }

  //qDebug("resizeEvent %ld %ld isHidden:%s shown:%s ", THIS->w, THIS->h, isHidden() ? "1" : "0", shown ? "1" : "0");
  //qDebug("THIS->h = %ld  THIS->container->height() = %ld  height() = %ld", THIS->h, THIS->container->height(), height());

  if (THIS->shown)
  {
    /*w = THIS->w;
    h = THIS->h;*/
    GB.Raise(THIS, EVENT_Resize, 0);
    /*if (w != THIS->w || h != THIS->h)
    {
      GB.Ref(THIS);
      GB.Post((void (*)())post_resize_event, (long)THIS);
    }*/
  }
}


void MyMainWindow::keyPressEvent(QKeyEvent *e)
{
  CWINDOW *_object = (CWINDOW *)CWidget::getReal(this);
  QPushButton *test = 0;
  CWIDGET *ob;
  bool cancel;
  
	e->ignore();

  //qDebug("MyMainWindow::keyPressEvent: (%p '%s' %s)", this, this ? this->caption().latin1() : 0, GB.GetClassName(CWidget::get(this)));
  
  if (GB.CanRaise(THIS, EVENT_KeyPress))
  {
  
    CKEY_clear(true);

    GB.FreeString(&CKEY_info.text);
    GB.NewString(&CKEY_info.text, TO_UTF8(e->text()), 0);
    CKEY_info.state = e->state();
    CKEY_info.code = e->key();

    cancel = GB.Raise(THIS, EVENT_KeyPress, 0);

    CKEY_clear(false);

    if (cancel)
      return;
  }
  
  if ((e->state() == 0 || (e->state() & Keypad && e->key() == Key_Enter )))
  {
    switch (e->key())
    {
      case Key_Enter:
      case Key_Return:

        test = THIS->defaultButton;
        break;

      case Key_Escape:

        test = THIS->cancelButton;
        break;
    }

    if (!test)
    	return;

		ob = CWidget::get(test);
		if (!ob)
			return;

		if (CWIDGET_test_flag(ob, WF_DESIGN))
			return;

    if (!test->isVisible() || !test->isEnabled())
    	return;

	  test->animateClick();
		e->accept();
	}
}


static bool closeAll()
{
  CWINDOW *win;
  QPtrDictIterator<CWINDOW> iter(CWindow::dict);

  //qDebug("CLOSE ALL");

  for(;;)
  {
    win = iter.current();
    if (!win)
      break;
    if (win != CWINDOW_Main && do_close(win, 0))
    {
      //qDebug("ABORTED %p", win);
      return true;
    }
    ++iter;
  }

  return false;
}

static void deleteAll()
{
  CWINDOW *win;
  QPtrDictIterator<CWINDOW> iter(CWindow::dict);

  //qDebug("DELETE ALL");

  for(;;)
  {
    win = iter.current();
    if (!win)
      break;

    ++iter;

    if (win != CWINDOW_Main)
    {
      //CWIDGET_set_flag(win, WF_CLOSED);
      //qDebug("post DELETE to %p", win);
      //qApp->postEvent(win->widget.widget, new QEvent(EVENT_CLOSE));
      CWIDGET_destroy((CWIDGET *)win);
      //GB.Post((void *)deleteAfter, win
      //delete win;
    }
  }

  //qApp->eventLoop()->processEvents(QEventLoop::AllEvents);
}

void MyMainWindow::closeEvent(QCloseEvent *e)
{
  CWINDOW *_object = (CWINDOW *)CWidget::get(this);
  bool cancel = false;
  bool modal;

  e->ignore();

  //if (qApp->loopLevel() != THIS->level)
  //  return;

  //if (_object == CWINDOW_Main && qApp->loopLevel() > 1)
  //  return;

  //qDebug("closeEvent");

  if (MAIN_in_wait)
    goto IGNORE;

  //if (CWINDOW_Current && (THIS != CWINDOW_Current))
  if (CWINDOW_Current && (THIS->loopLevel != CWINDOW_Current->loopLevel))
  {
  	//qDebug("ignore close event");
    goto IGNORE;
  }

	if (THIS->shown)
	{
		//qDebug("THIS->shown = %d: %p: %s", THIS->shown, THIS, GB.GetClassName(THIS));
		CWIDGET_set_flag(_object, WF_IN_CLOSE);
		cancel = GB.Raise(_object, EVENT_Close, 0);
		CWIDGET_clear_flag(_object, WF_IN_CLOSE);
	}

  if (!cancel && THIS == CWINDOW_Main)
  {
    if (closeAll())
      cancel = true;
  }

	if (cancel)
		goto IGNORE;

	modal = testWFlags(Qt::WShowModal); // && THIS->shown;

  CWIDGET_set_flag(THIS, WF_CLOSED);
 	//qApp->sendEvent(WIDGET, new QEvent(EVENT_CLOSE));

  if (!CWIDGET_test_flag(_object, WF_PERSISTENT))
  {
    if (CWINDOW_Main == THIS)
    {
      deleteAll();
      CWINDOW_Main = 0;
    }

    CWIDGET_destroy((CWIDGET *)THIS);
  }

  e->accept();

  if (CWINDOW_Active == THIS)
    CWINDOW_activate(CWidget::get(WIDGET->parentWidget()));
  if (CWINDOW_LastActive == THIS)
  {
    //GB.Unref(POINTER(&CWINDOW_LastActive));
    CWINDOW_LastActive = 0;
    //qDebug("CWINDOW_LastActive = 0");
  }
  THIS->shown = FALSE;
  
  //qDebug("THIS->enterLoop = %d", THIS->enterLoop);
	if (modal && THIS->enterLoop)
	{
		//qDebug("exitLoop: %p", THIS);
		qApp->eventLoop()->exitLoop();
	}

  return;

IGNORE:

  CWIDGET_clear_flag(THIS, WF_CLOSED);
  e->ignore();
}



bool MyMainWindow::isPersistent(void)
{
  return !testWFlags(WDestructiveClose);
}


void MyMainWindow::setPersistent(bool pers)
{
  if (!pers)
    setWFlags(WDestructiveClose);
  else
    clearWFlags(WDestructiveClose);
}

#if 0
static void set_origin_rec(QWidget *top, QWidget::BackgroundOrigin origin)
{
  QObjectList *l = top->queryList("QWidget");
  QObjectListIt it( *l );
  QObject *obj;

  while ( (obj = it.current()) != 0 )
  {
    ++it;
    ((QWidget*)obj)->setBackgroundOrigin(origin);
  }

  top->setBackgroundOrigin(origin);
  delete l;
}

void MyMainWindow::defineMask()
{
  CWINDOW *_object = (CWINDOW *)CWidget::get(this);
  QPixmap *p;
  QColor c;
  QWidget *root = WINDOW;
  //QBitmap b;

  if (!THIS->picture)
  {
    if (THIS->masked)
      clearMask();

    //root->setBackgroundOrigin(QWidget::WidgetOrigin);
    //root->setPaletteBackgroundColor(THIS->container->paletteBackgroundColor());
    root->setErasePixmap(0);

    //set_origin_rec(root, QWidget::WidgetOrigin);
  }
  else
  {
    //set_origin_rec(root, QWidget::AncestorOrigin);

    p = THIS->picture->pixmap;

    if (THIS->masked)
    {
      //root->setPaletteBackgroundPixmap(*p);
      THIS->container->setBackgroundOrigin(QWidget::WindowOrigin);
      THIS->container->setErasePixmap(*p);
      if (p->hasAlpha())
        setMask(*(p->mask()));
      else
        clearMask();
    }
    else
    {
      clearMask();
      THIS->container->setBackgroundOrigin(QWidget::WidgetOrigin);
      THIS->container->setErasePixmap(0);
      //root->setBackgroundOrigin(QWidget::WidgetOrigin);
      //root->setPaletteBackgroundPixmap(*p);
      //root->setErasePixmap(0);
    }
  }

  //doReparent(parentWidget(), getWFlags(), pos());
}
#endif

void MyMainWindow::doReparent(QWidget *parent, WFlags f, const QPoint &pos, bool showIt)
{
  CWINDOW *_object = (CWINDOW *)CWidget::get(this);
  bool hasIcon;
  QPixmap p;
 	#ifndef NO_X_WINDOW
  bool saveProp = false;
 	#endif

  hasIcon = icon() != 0;
  if (hasIcon)
    p = *icon();

 	#ifndef NO_X_WINDOW
  if (THIS->toplevel && THIS->shown)
  {
  	saveProp = true;
  	X11_window_save_properties(this->winId());
	}
  #endif

	THIS->toplevel = !parent || parent->isTopLevel();
	THIS->embedded = !THIS->toplevel;

	if (THIS->toplevel)
	{
		f |= Qt::WType_TopLevel;
		if (!parent)
			f |= Qt::WGroupLeader;
		else
			f &= ~Qt::WGroupLeader;
	}
	else	
		f &= ~Qt::WType_TopLevel;

  reparent(parent, f, pos, showIt);
  move(pos);
  //qDebug("doReparent: (%s %p) (%d %d) -> (%d %d)", GB.GetClassName(THIS), THIS, pos.x(), pos.y(), WIDGET->x(), WIDGET->y());
  
 	#ifndef NO_X_WINDOW
		if (THIS->toplevel)
		{
			if (saveProp)
				X11_window_restore_properties(this->winId());
			else
				initProperties();
		}
	#endif

  if (hasIcon)
    setIcon(p);
  //qDebug("new parent = %p", parentWidget());
}


void MyMainWindow::center(bool force = false)
{
  QPoint p;

  if (!force && !mustCenter)
    return;

  mustCenter = false;

  p.setX((qApp->desktop()->width() - width()) / 2);
  p.setY((qApp->desktop()->height() - height()) / 2);

  move(p);
}

void MyMainWindow::configure()
{
  CWINDOW *_object = (CWINDOW *)CWidget::get(this);

	//qDebug("THIS->menuBar = %p  menuBar() = %p", THIS->menuBar, menuBar());

	if (THIS->menuBar && !menuBar()->isHidden())
	{
		THIS->container->setGeometry(0, menuBar()->height(), this->width(), this->height() - menuBar()->height());
	}
	else
	{
    //qDebug("configure: %s (%d %d)", GB.GetClassName(THIS), this->width(), this->height());
		THIS->container->setGeometry(0, 0, this->width(), this->height());
		//THIS->container->setGeometry(0, 0, THIS->w, THIS->h);
		THIS->container->raise();
		CCONTAINER_arrange(THIS);
	}

	//qDebug(">>> THIS->menuBar = %p  menuBar() = %p", THIS->menuBar, menuBar());

	//qDebug("configure: %p (%d %d %d %d)", THIS, ((QFrame *)(THIS->container))->contentsRect().x(), ((QFrame *)(THIS->container))->contentsRect().y(), ((QFrame *)(THIS->container))->contentsRect().width(), ((QFrame *)(THIS->container))->contentsRect().height());
}


void MyMainWindow::hide(void)
{
  CWIDGET *_object = CWidget::get(this);
  THIS->hidden = TRUE;
  QMainWindow::hide();
}

void MyMainWindow::setName(const char *name, CWIDGET *control)
{
	names.remove(name);
	if (control)
		names.insert(name, control);
}


/***************************************************************************

  CWindow

***************************************************************************/

CWindow CWindow::manager;
int CWindow::count = 0;
QPtrDict<CWINDOW> CWindow::dict;

/*static void post_activate_event(void *ob)
{
  GB.Raise(ob, EVENT_Activate, 0);
  GB.Unref(&ob);
}

static void post_deactivate_event(void *ob)
{
  GB.Raise(ob, EVENT_Deactivate, 0);
  GB.Unref(&ob);
}*/

void CWINDOW_activate(CWIDGET *ob)
{
	CWINDOW *active;

	if (ob)
	{
		active = CWidget::getWindow(ob);
		for(;;)
		{
			if (active->toplevel)
				break;
			if (GB.CanRaise(active, EVENT_Activate))
				break;
			active = CWidget::getWindow(CWidget::get(QWIDGET(active)->parentWidget()));
		}
	}
	else
		active = 0;

	if (active == CWINDOW_Active)
		return;

	//qDebug("activate: (%s %p): (%s %p) -> (%s %p)", ob ? GB.GetClassName(ob) : "", ob, CWINDOW_Active ? GB.GetClassName(CWINDOW_Active) : "", CWINDOW_Active, active ? GB.GetClassName(active) : "", active);

	if (CWINDOW_Active)
	{
	  GB.Raise(CWINDOW_Active, EVENT_Deactivate, 0);
		/*if (GB.CanRaise(CWINDOW_Active, EVENT_Deactivate))
		{
			GB.Ref(CWINDOW_Active);
			GB.Post((void (*)())post_deactivate_event, (long)CWINDOW_Active);
		}*/
		CWINDOW_Active = 0;
	}

	if (active)
	{
	  GB.Raise(active, EVENT_Activate, 0);
		/*if (GB.CanRaise(active, EVENT_Activate))
		{
			GB.Ref(active);
			GB.Post((void (*)())post_activate_event, (long)active);
		}*/
	}

	//CWINDOW_LastActive = CWINDOW_Active;
  CWINDOW_Active = active;
  //qDebug("CWINDOW_Active = (%s %p) %p", active ? GB.GetClassName(active) : "", active, active ? QWIDGET(active) : 0);
}

void CWINDOW_set_default_button(CWINDOW *win, QPushButton *button, bool on)
{
  if (on)
  {
    if (win->defaultButton)
      win->defaultButton->setDefault(false);

    win->defaultButton = button;
		button->setDefault(true);
  }
  else
  {
    if (win->defaultButton == button)
    {
      button->setDefault(false);
      win->defaultButton = 0;
    }
  }
}

void CWINDOW_set_cancel_button(CWINDOW *win, QPushButton *button, bool on)
{
  //qDebug("CWINDOW_set_cancel_button: (%s %p) (%s %p) %d", GB.GetClassName(win), win, GB.GetClassName(CWidget::get(button)), CWidget::get(button), on); 
  if (on)
  {
    win->cancelButton = button;
  }
  else
  {
    if (button == win->cancelButton)
      win->cancelButton = 0;
  }
}


static void post_show_event(void *_object)
{
	//qDebug("post_show_event");
  //qDebug("> post_show_event %s %p", GB.GetClassName(THIS), THIS);
  if (THIS->shown)
    return;

	//qDebug("EVENT_Open: %s %p %d", GB.GetClassName(THIS), THIS);

  GB.Raise(THIS, EVENT_Open, 0);
  if (CWIDGET_test_flag(THIS, WF_CLOSED))
  	return;
  // a move event is generated just after for real windows
	//if (!THIS->toplevel)
    GB.Raise(THIS, EVENT_Move, 0);
  GB.Raise(THIS, EVENT_Resize, 0);
  THIS->shown = true;
  //qDebug("THIS->shown <- true: %p: %s", THIS, GB.GetClassName(THIS));
  //qDebug("< post_show_event %p", THIS);
  //GB.Unref(&_object);
}

/*static void post_hide_event(void *_object)
{
  qDebug("Hide: %s %d", GB.GetClassName(THIS), !WINDOW->isHidden());
	GB.Raise(THIS, EVENT_Hide, 0);
	CACTION_raise(THIS);
  GB.Unref(&_object);
}*/

bool CWindow::eventFilter(QObject *o, QEvent *e)
{
  CWINDOW *_object = (CWINDOW *)CWidget::get(o);

  if (THIS != NULL)
  {
    if (e->type() == QEvent::WindowActivate && e->spontaneous())
    {
      if (THIS->toplevel)
      {
        //qDebug("Activate: CWINDOW_Current = %p  ob = %p", CWINDOW_Current, THIS);
				
				CWINDOW_activate((CWIDGET *)(CWINDOW_LastActive ? CWINDOW_LastActive : THIS));
				//GB.Unref(POINTER(&CWINDOW_LastActive));
				CWINDOW_LastActive = 0;
				//qDebug("CWINDOW_LastActive = 0");
      }
    }
    else if (e->type() == QEvent::WindowDeactivate && e->spontaneous())
    {
      if (THIS->toplevel)
      {
        //qDebug("Deactivate: CWINDOW_Current = %p  ob = %p", CWINDOW_Current, THIS);
        /*#if QT_VERSION >= 0x030100
        if ((THIS == CWINDOW_Current) || (qApp->eventLoop()->loopLevel() <= 1))
        #else
        if ((THIS == CWINDOW_Current) || (qApp->loopLevel() <= 1))
        #endif*/
        if (!CWINDOW_LastActive)
        {
        	CWINDOW_LastActive = CWINDOW_Active;
					//qDebug("CWINDOW_LastActive = %p", CWINDOW_LastActive);
        	//GB.Ref(CWINDOW_LastActive);
				}
				CWINDOW_activate(0);
      }
    }
    else if (e->type() == QEvent::Show) // && !e->spontaneous())
    {
      MyMainWindow *w = (MyMainWindow *)o;

			//qDebug("Show: %s %d (%d)", GB.GetClassName(THIS), !WINDOW->isHidden(), e->spontaneous());
			
      if (THIS->toplevel)
        w->center();

      if (!THIS->shown)
        post_show_event(THIS);
      
      if (THIS->shown)
      {
				if (THIS->focus)
				{
					THIS->focus->widget->setFocus();
					GB.Unref(POINTER(&THIS->focus));
					THIS->focus = NULL;
				}
	
				GB.Raise(THIS, EVENT_Show, 0);
				if (!e->spontaneous())
					CACTION_raise(THIS);
			}
			else
				return true;
    }
    else if (e->type() == QEvent::Hide) // && !e->spontaneous())
    {
		  //qDebug("Hide: %s %d (%d)", GB.GetClassName(THIS), !WINDOW->isHidden(), e->spontaneous());
			if (WINDOW->isHidden())
			{
				GB.Raise(THIS, EVENT_Hide, 0);
	      if (!e->spontaneous())
					CACTION_raise(THIS);
			}
    }
  }

  return QObject::eventFilter(o, e);    // standard event processing
}


void CWindow::error(void)
{
  //CWINDOW *_object = (CWINDOW *)CWidget::getReal((QObject *)sender());
  //qDebug("XEMBED: CWindow::error %p -> %p", sender(), THIS);
  CWINDOW_EmbedState = EMBED_ERROR;
}

void CWindow::embedded(void)
{
  //CWINDOW *_object = (CWINDOW *)CWidget::getReal((QObject *)sender());
  //qDebug("XEMBED: CWindow::embedded %p -> %p", sender(), THIS);
  CWINDOW_EmbedState = EMBED_OK;
}

void CWindow::closed(void)
{
  //CWINDOW *_object = (CWINDOW *)CWidget::getReal((QObject *)sender());
  //qDebug("XEMBED: CWindow::closed %p -> %p", sender(), THIS);
  //CWIDGET_destroy(CWidget::getReal((QObject *)sender()));
  delete sender();
}

void CWindow::destroy(void)
{
  CWINDOW *_object = (CWINDOW *)CWidget::getReal((QObject *)sender());
  //qDebug("XEMBED: CWindow::destroy %p -> %p", sender(), THIS);

  if (THIS)
  {
    do_close(THIS, 0, true);
    remove_window_check_quit(THIS);
  }

  CWINDOW_EmbedState = EMBED_WAIT;
  CWINDOW_Embedded = false;
  CWINDOW_Embedder = 0;
}

