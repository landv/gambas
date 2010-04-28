/***************************************************************************

  CWindow.cpp

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

#define __CWINDOW_CPP

#include <qnamespace.h>
#include <qapplication.h>
#include <qmenubar.h>
#include <qsizepolicy.h>
#include <qnamespace.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qobject.h>

//Added by qt3to4:
#include <QMoveEvent>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QShowEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QLayout>
#include <QEventLoop>
#include <QDesktopWidget>
#include <QAction>
#include <QX11Info>

#include "main.h"

#ifndef NO_X_WINDOW
#include <QX11EmbedWidget>
#include <QX11EmbedContainer>
#endif

#include "gambas.h"

#include "CWidget.h"
#include "CMenu.h"
#include "CKey.h"
#include "CDraw.h"
#include "CWindow.h"

#ifndef NO_X_WINDOW
#include "x11.h"
#undef FontChange
#else
enum
{
	_NET_WM_WINDOW_TYPE_NORMAL,
	_NET_WM_WINDOW_TYPE_DESKTOP,
	_NET_WM_WINDOW_TYPE_DOCK,
	_NET_WM_WINDOW_TYPE_TOOLBAR,
	_NET_WM_WINDOW_TYPE_MENU,
	_NET_WM_WINDOW_TYPE_UTILITY,
	_NET_WM_WINDOW_TYPE_SPLASH,
	_NET_WM_WINDOW_TYPE_DIALOG,
	_NET_WM_WINDOW_TYPE_DROPDOWN_MENU,
	_NET_WM_WINDOW_TYPE_POPUP_MENU,
	_NET_WM_WINDOW_TYPE_TOOLTIP,
	_NET_WM_WINDOW_TYPE_NOTIFICATION,
	_NET_WM_WINDOW_TYPE_COMBO,
	_NET_WM_WINDOW_TYPE_DND
};
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

DECLARE_METHOD(CWINDOW_show);

CWINDOW *CWINDOW_Main = 0;
CWINDOW *CWINDOW_Current = 0;
CWINDOW *CWINDOW_LastActive = 0;
CWINDOW *CWINDOW_Active = 0;

int CWINDOW_Embedder = 0;
bool CWINDOW_Embedded = false;
static int CWINDOW_EmbedState = 0;

#ifndef NO_X_WINDOW
void CWINDOW_change_property(QWidget *w, Atom property, bool set)
{
	if (!w->isWindow())
		return;

	X11_window_change_property(w->winId(), w->isVisible(), property, set);
}

bool CWINDOW_has_property(QWidget *w, Atom property)
{
	if (!w->isWindow())
		return false;

	return X11_window_has_property(w->winId(), property);
}
#endif

// Fix a QT little boring visual bug on menubars
#if 0
void CWINDOW_fix_menubar(CWINDOW *window)
{
	if (window && window->menuBar)
	{
		QWidget *save = qApp->focusWidget();
		window->menuBar->setFocus();
		QKeyEvent e(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
		qApp->sendEvent(window->menuBar, &e);
		if (save) 
			save->setFocus();
	}
}
#endif

/*---- Utility routines --------------------------------------------------------------*/

static void clear_mask(CWINDOW *_object)
{
	WINDOW->clearMask();

	if (THIS->toplevel)
	{
		#ifndef NO_X_WINDOW
		bool v = !WINDOW->isHidden() && WINDOW->isVisible();
		//WINDOW->setBorder(WINDOW->hasBorder(), true);
		//WINDOW->setResizable(WINDOW->isResizable(), true);
		if (v && THIS->reallyMasked)
		{
			X11_window_remap(WINDOW->winId());
			WINDOW->initProperties();
		}
		#endif
	}
}

void CWINDOW_define_mask(CWINDOW *_object)
{
	QPixmap background;
	QColor c;
	QPalette palette;

	//qDebug("define_mask: (%s %p)  picture = %p  masked = %d", GB.GetClassName(THIS), THIS, THIS->picture, THIS->masked);

	//if (THIS->embedded)
	//	return;

	if (THIS->picture)
		background = *(THIS->picture->pixmap);
	
	if (background.isNull())
	{
		clear_mask(THIS);
		THIS->reallyMasked = false;
		THIS->container->setPixmap(0);
		//THIS->container->setPalette(WINDOW->palette());
	}
	else
	{
		if (THIS->masked && background.hasAlpha())
		{
			THIS->reallyMasked = true;
			WINDOW->setMask(background.mask());
		}
		else
		{
			clear_mask(THIS);
			THIS->reallyMasked = false;
		}

		THIS->container->setPixmap(THIS->picture->pixmap);
	}
	
	THIS->container->update();
}

static bool emit_open_event(void *_object)
{
	if (THIS->opening)
		return true;
	
	CWIDGET_clear_flag(THIS, WF_CLOSED);
	
	if (!THIS->opened)
	{
		if (!THIS->minw && !THIS->minh)
		{
			THIS->minw = THIS->w;
			THIS->minh = THIS->h;
		}
		#if DEBUG_WINDOW
		qDebug("emit_open_event: %s %p", GB.GetClassName(THIS), THIS);
		#endif
		THIS->opening = true;
		GB.Raise(THIS, EVENT_Open, 0);
		THIS->opening = false;
		if (CWIDGET_test_flag(THIS, WF_CLOSED))
		{
			#if DEBUG_WINDOW
			qDebug("emit_open_event: %s %p [CANCELED]", GB.GetClassName(THIS), THIS);
			#endif
			return true;
		}
		THIS->opened = true;
	}
	
	THIS->hidden = false;
	return false;
}

static void post_show_event(void *_object)
{
	GB.Raise(THIS, EVENT_Move, 0);
	GB.Raise(THIS, EVENT_Resize, 0);
	
	if (THIS->focus)
	{
		THIS->focus->widget->setFocus();
		GB.Unref(POINTER(&THIS->focus));
		THIS->focus = NULL;
	}	
}

static void reparent_window(CWINDOW *_object, void *parent, bool move, int x = 0, int y = 0)
{
	QPoint p;
	QWidget *newParentWidget;
	
	if (move)
	{
		p.setX(x);
		p.setY(y);
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

	if (newParentWidget != WINDOW->parentWidget())
	{
		//qDebug("reparent_window: -> %s %p", parent ? ((CWIDGET *)parent)->name : "", parent);
		WINDOW->doReparent(newParentWidget, p);
		WINDOW->setResizable(WINDOW->isResizable(), true);
	}
	else
		CWIDGET_move(THIS, p.x(), p.y());
}




/***************************************************************************

	Window

***************************************************************************/

static void show_later(CWINDOW *_object)
{
	/* If the user has explicitely hidden the window since the posting of this routines
		then do nothing
	*/
	//qDebug("show_later %s %p: hidden = %d", GB.GetClassName(THIS), THIS, THIS->hidden);
	if (!THIS->hidden && WIDGET)
	{
		if (!emit_open_event(THIS))
			CWIDGET_set_visible((CWIDGET *)THIS, true);
	}
	GB.Unref(POINTER(&_object));
}

BEGIN_METHOD(CWINDOW_new, GB_OBJECT parent)

	MyMainWindow *win = 0;
	MyContainer *container;
	#ifndef NO_X_WINDOW
	QX11EmbedWidget *client = 0;
	#endif
	const char *name = GB.GetClassName(THIS);

	//THIS->widget.flag.fillBackground = true;

	if (MISSING(parent) || !VARG(parent))
	{
		#ifndef NO_X_WINDOW
		if (CWINDOW_Embedder && !CWINDOW_Embedded)
		{
			client = new QX11EmbedWidget;
			
			win = new MyMainWindow(client, name, true);
			container = new MyContainer(win);
			container->raise();
			THIS->embedded = true;
			THIS->toplevel = false;
			THIS->xembed = true;
		}
		else
		#endif
		{
			//win = new MyMainWindow(CWINDOW_Main ? (MyMainWindow *)QWIDGET(CWINDOW_Main) : 0, name);
			win = new MyMainWindow(0, name);
			container = new MyContainer(win);
			container->raise();
			THIS->embedded = false;
			THIS->toplevel = true;
			THIS->xembed = false;
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
	}
	/*else
	{
		GB.Error("The parent of a Window must be a Container or a Workspace");
		return;
	}*/

	THIS->container = container;
	CWIDGET_new(win, (void *)_object, true);
	//container->setPaletteBackgroundColor(Qt::yellow);
	//container->setBackgroundOrigin(QWidget::WindowOrigin);

	//qDebug("CWINDOW_new: %p: %s", THIS, GB.Debug.GetCurrentPosition());
	
	if (win)
	{
		win->_object = THIS;
		win->installEventFilter(&CWindow::manager);
	}

	if (THIS->toplevel || THIS->xembed)
	{
		CWindow::insertTopLevel(THIS);
		
		if (CWINDOW_Main == 0)
		{
			#if DEBUG_WINDOW
			qDebug("CWINDOW_Main -> %p", THIS);
			#endif
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
			MAIN_process_events();
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
		#if DEBUG_WINDOW
		qDebug("post show_later %s %p", GB.GetClassName(THIS), THIS);
		#endif
		GB.Ref(THIS);
		//show_later(THIS);
		GB.Post((void (*)())show_later, (intptr_t)THIS);
		//WIDGET->show();
	}
	else
		THIS->hidden = TRUE;
	
	THIS->showMenuBar = true;

END_METHOD


BEGIN_METHOD_VOID(CFORM_new)

	if (!GB.Parent(_object))
		GB.Attach(_object, _object, "Form");

END_METHOD


BEGIN_METHOD_VOID(CFORM_main)

	CWINDOW *form = (CWINDOW *)GB.AutoCreate(GB.GetClass(NULL), 0);
	//if (!((MyMainWindow *)form->widget.widget)->isHidden())
	//if (!form->hidden)
		CWINDOW_show(form, NULL);
		
END_METHOD


BEGIN_METHOD(CFORM_load, GB_OBJECT parent)

	//qDebug("CFORM_load");
	reparent_window((CWINDOW *)GB.AutoCreate(GB.GetClass(NULL), 0), VARGOPT(parent, 0), false);

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_free)

	//qDebug("CWINDOW_free");

	GB.StoreObject(NULL, POINTER(&(THIS->icon)));
	GB.StoreObject(NULL, POINTER(&(THIS->picture)));
	GB.Unref(POINTER(&THIS->focus));

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_next)

	int index = ENUM(int);

	if (index >= CWindow::list.count())
	{
		GB.StopEnum();
		return;
	}

	GB.ReturnObject(CWindow::list.at(index));
	ENUM(int) = index + 1;

END_METHOD


BEGIN_PROPERTY(CWINDOW_count)

	GB.ReturnInteger(CWindow::list.count());

END_PROPERTY


BEGIN_METHOD(CWINDOW_get_from_id, GB_INTEGER id)

	QWidget *wid = QWidget::find(VARG(id));

	//qDebug("id = %d wid = %p", PARAM(id), wid);

	if (wid != 0 && wid->isWindow())
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


static bool do_close(CWINDOW *_object, int ret, bool destroyed = false)
{
	bool closed;
	
	#if DEBUG_WINDOW
	qDebug("do_close: (%s %p) %d %d", GB.GetClassName(THIS), THIS, THIS->closing, CWIDGET_test_flag(THIS, WF_CLOSED));
	#endif

	if (THIS->closing || CWIDGET_test_flag(THIS, WF_CLOSED)) // || WIDGET->isHidden())
		return false;

	if (!THIS->toplevel)
	{
		//qDebug("Close event: %s %p opened = %d", GB.GetClassName(THIS), THIS, THIS->opened);
		if (THIS->opened)
		{
			THIS->closing = true;
			closed = !GB.Raise(THIS, EVENT_Close, 0);
			THIS->closing = false;
		}
		else
			closed = true;

		if (destroyed || closed)
		{
			CWIDGET_set_flag(THIS, WF_CLOSED);
			THIS->opened = false;
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
		if (!THIS->opened)
		{
			#if DEBUG_WINDOW
			qDebug("send close event");
			#endif
			QCloseEvent e;
			QApplication::sendEvent(WINDOW, &e);
			closed = e.isAccepted();
		}
		else
		{
			#if DEBUG_WINDOW
			qDebug("call WINDOW->close()");
			#endif
			closed = WINDOW->close();
		}
		#if DEBUG_WINDOW
		qDebug("--> closed = %d", closed);
		#endif
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
		THIS->opened = FALSE;
	}
	#endif

	if (closed)
		THIS->ret = ret;

	//qDebug("CWINDOW_close: ret = %d", THIS->ret);

	return (!closed);
}

BEGIN_METHOD(CWINDOW_close, GB_INTEGER ret)

	int ret = VARGOPT(ret, 0);

	GB.ReturnBoolean(do_close(THIS, ret));

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_raise)

	if (!THIS->toplevel)
	{
		if (!WIDGET->isVisible())
			CWIDGET_set_visible((CWIDGET *)THIS, true);
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

	if (emit_open_event(THIS))
		return;
	
	if (!THIS->toplevel)
	{
		CWIDGET_set_visible((CWIDGET *)THIS, true);
    #ifndef NO_X_WINDOW
    if (THIS->xembed)
    	XEMBED->show();
    #endif
		post_show_event(THIS);
	}
	else
	{
		WINDOW->showActivate(); //parent ? parent->widget.widget : 0);
		//THIS->widget.flag.visible = true;
	}

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_hide)

	THIS->hidden = true;
	
	if (THIS->toplevel && WINDOW->isModal())
	{
		do_close(THIS, 0);
		//THIS->widget.flag.visible = false;
	}
	else
		CWIDGET_set_visible((CWIDGET *)THIS, false);

END_METHOD


BEGIN_METHOD_VOID(CWINDOW_show_modal)

	THIS->ret = 0;

	if (!emit_open_event(THIS))
	{
		if (THIS->toplevel)
		{
			//THIS->widget.flag.visible = true;
			WINDOW->showModal();
			//THIS->widget.flag.visible = false;
		}
	}

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
		GB.ReturnNewZeroString(TO_UTF8(WIDGET->windowTitle()));
	else
	{
		QString s = QSTRING_PROP();
		THIS->title = s.length() > 0;
		WIDGET->setWindowTitle(s);
		GB.Raise(THIS, EVENT_Title, 0);
	}


END_PROPERTY


#if 0
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
#endif

BEGIN_PROPERTY(CWINDOW_border)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WINDOW->hasBorder());
	else
		WINDOW->setBorder(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_resizable)

	if (READ_PROPERTY)
		GB.ReturnBoolean(WINDOW->isResizable());
	else
		WINDOW->setResizable(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_icon)

	if (READ_PROPERTY)
		GB.ReturnObject(THIS->icon);
	else
	{
		SET_PIXMAP(WIDGET->setWindowIcon, &(THIS->icon), PROP(GB_OBJECT));
		GB.Raise(THIS, EVENT_Icon, 0);
	}

END_PROPERTY


BEGIN_PROPERTY(CWINDOW_picture)

	if (READ_PROPERTY)
		GB.ReturnObject(THIS->picture);
	else
	{
		CPICTURE *new_pict = (CPICTURE *)VPROP(GB_OBJECT);
		
		if (new_pict != THIS->picture)
		{
			CPICTURE *old = THIS->picture;
			GB.Ref(new_pict);
			THIS->picture = new_pict;
			//CWINDOW_define_mask(THIS);
			CWIDGET_reset_color((CWIDGET *)THIS);
			GB.Unref(POINTER(&old));
		}
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
			bool new_masked = VPROP(GB_BOOLEAN);
			
			if (new_masked != THIS->masked)
			{
				THIS->masked = new_masked;
				//CWINDOW_define_mask(THIS);
				CWIDGET_reset_color((CWIDGET *)THIS);
			}
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

static void show_window_state(void *_object)
{
	THIS->stateChange = false;
	
	if (WINDOW)
	{
		if (WINDOW->windowState() & Qt::WindowMinimized)
			WINDOW->showMinimized();
		else if (WINDOW->windowState() & Qt::WindowFullScreen)
			WINDOW->showFullScreen();
		else if (WINDOW->windowState() & Qt::WindowMaximized)
			WINDOW->showMaximized();
		else
			WINDOW->showNormal();
	}
	
	//qDebug("show_window_state %s %p", GB.GetClassName(THIS), THIS);
	GB.Unref(POINTER(&_object));
}

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
			
			if (WINDOW->isVisible())
			{
				if (WINDOW->windowState() & Qt::WindowMinimized)
					WINDOW->showMinimized();
				else if (WINDOW->windowState() & Qt::WindowFullScreen)
					WINDOW->showFullScreen();
				else if (WINDOW->windowState() & Qt::WindowMaximized)
					WINDOW->showMaximized();
				else
					WINDOW->showNormal();
			}
			/*if (WINDOW->isVisible() && !THIS->stateChange)
			{
				THIS->stateChange = true;
				//qDebug("post show_window_state %s %p", GB.GetClassName(THIS), THIS);
				GB.Ref(THIS);
				GB.Post((GB_POST_FUNC)show_window_state, (intptr_t)THIS);
			}*/
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

BEGIN_PROPERTY(CWINDOW_type)

	if (READ_PROPERTY)
		GB.ReturnInteger(0);

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
		GB.ReturnInteger(THIS->stacking);
	}
	else
	{
		p = VPROP(GB_INTEGER);
		if (p >= 0 && p <= 2)
		{
			THIS->stacking = p;
			WINDOW->initProperties();
		}
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
		GB.ReturnBoolean(THIS->stacking == 1);
	}
	else
	{
		THIS->stacking = VPROP(GB_BOOLEAN) ? 1 : 0;
		WINDOW->initProperties();
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
		GB.ReturnBoolean(THIS->sticky);
	else
	{
		THIS->sticky = VPROP(GB_BOOLEAN);
		X11_window_set_desktop(WINDOW->winId(), WINDOW->isVisible(), THIS->sticky ? 0xFFFFFFFF : X11_get_current_desktop());
	}

END_PROPERTY

BEGIN_PROPERTY(CWINDOW_type)

	if (READ_PROPERTY)
		GB.ReturnInteger(WINDOW->getType());
	else
		WINDOW->setType(VPROP(GB_INTEGER));

END_PROPERTY

#endif //------------------------------------------------------------------------------------------


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

	QList<QWidget *> list = WINDOW->findChildren<QWidget *>();
	int i;
	int n = 0;

	for (i = 0; i < list.count(); i++)
	{
		if (CWidget::getReal(list.at(i)))
			n++;
	}

	GB.ReturnInteger(n);

END_PROPERTY


BEGIN_METHOD_VOID(CWINDOW_control_next)

	QList<QWidget *> children = WINDOW->findChildren<QWidget *>();
	CWIDGET *control;
	int index;

	index = ENUM(int);

	control = NULL;

	do
	{
		if (index >= children.count())
		{
			GB.StopEnum();
			return;
		}

		control = CWidget::getReal(children.at(index));
		index++;
	}
	while (!control);

	ENUM(int) = index;
	GB.ReturnObject(control);

END_PROPERTY


BEGIN_METHOD(CWINDOW_reparent, GB_OBJECT container; GB_INTEGER x; GB_INTEGER y)

	//qDebug("CWINDOW_reparent");
	reparent_window(THIS, VARG(container), !MISSING(x) && !MISSING(y), VARG(x), VARG(y));

END_METHOD


BEGIN_METHOD(CWINDOW_get, GB_STRING name)

	GB.ReturnObject(WINDOW->names[GB.ToZeroString(ARG(name))]);

END_METHOD


BEGIN_PROPERTY(CWINDOW_closed)

	GB.ReturnBoolean(!THIS->opened);

END_PROPERTY

/***************************************************************************/

BEGIN_PROPERTY(CWINDOW_menu_count)

  if (THIS->menuBar)
    GB.ReturnInteger(THIS->menuBar->actions().count());
  else
    GB.ReturnInteger(0);

END_PROPERTY


BEGIN_METHOD_VOID(CWINDOW_menu_next)

  int index;

  if (!THIS->menuBar)
  {
    GB.StopEnum();
    return;
  }

  index = ENUM(int);

  if (index >= THIS->menuBar->actions().count())
  {
    GB.StopEnum();
    return;
  }

  GB.ReturnObject(CMenu::dict[THIS->menuBar->actions().at(index)]);

  ENUM(int) = index + 1;

END_PROPERTY


BEGIN_METHOD(CWINDOW_menu_get, GB_INTEGER index)

  int index = VARG(index);

  if (!THIS->menuBar || index < 0 || index >= THIS->menuBar->actions().count())
  {
    GB.Error(GB_ERR_BOUND);
    return;
  }

  GB.ReturnObject(CMenu::dict[THIS->menuBar->actions().at(index)]);

END_PROPERTY


static bool are_menus_visible(void *_object)
{
	return !THIS->hideMenuBar && THIS->showMenuBar;
}


static void set_menus_visible(void *_object, bool v)
{
	THIS->showMenuBar = v;
	WINDOW->configure();
}

BEGIN_PROPERTY(CWINDOW_menu_visible)

	if (READ_PROPERTY)
		GB.ReturnBoolean(are_menus_visible(THIS));
	else
		set_menus_visible(THIS, VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD_VOID(CWINDOW_menu_show)

	set_menus_visible(THIS, true);

END_METHOD

BEGIN_METHOD_VOID(CWINDOW_menu_hide)

	set_menus_visible(THIS, false);

END_METHOD

BEGIN_PROPERTY(Window_Opacity)

	if (READ_PROPERTY)
		GB.ReturnInteger(WIDGET->windowOpacity() * 100);
	else
	{
		double opacity = VPROP(GB_INTEGER) / 100.0;
		
		if (opacity < 0.0)
			opacity = 0.0;
		else if (opacity > 1.0)
			opacity = 1.0;
		
		WIDGET->setWindowOpacity(opacity);
	}

END_PROPERTY


/***************************************************************************/

GB_DESC CWindowMenusDesc[] =
{
	GB_DECLARE(".WindowMenus", 0), GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "Menu", CWINDOW_menu_next, NULL),
	GB_METHOD("_get", "Menu", CWINDOW_menu_get, "(Index)i"),
	GB_PROPERTY_READ("Count", "i", CWINDOW_menu_count),
  GB_METHOD("Show", NULL, CWINDOW_menu_show, NULL),
  GB_METHOD("Hide", NULL, CWINDOW_menu_hide, NULL),
  GB_PROPERTY("Visible", "b", CWINDOW_menu_visible),
  
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

GB_DESC CWindowTypeDesc[] =
{
  GB_DECLARE("WindowType", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("Normal", "i", _NET_WM_WINDOW_TYPE_NORMAL),
  GB_CONSTANT("Dock", "i", _NET_WM_WINDOW_TYPE_DOCK),
  GB_CONSTANT("Toolbar", "i", _NET_WM_WINDOW_TYPE_TOOLBAR),
  GB_CONSTANT("Menu", "i", _NET_WM_WINDOW_TYPE_MENU),
  GB_CONSTANT("Utility", "i", _NET_WM_WINDOW_TYPE_UTILITY),
  GB_CONSTANT("Splash", "i", _NET_WM_WINDOW_TYPE_SPLASH),
  GB_CONSTANT("Dialog", "i", _NET_WM_WINDOW_TYPE_DIALOG),
  GB_CONSTANT("DropDownMenu", "i", _NET_WM_WINDOW_TYPE_DROPDOWN_MENU),
  GB_CONSTANT("PopupMenu", "i", _NET_WM_WINDOW_TYPE_POPUP_MENU),
  GB_CONSTANT("Tooltip", "i", _NET_WM_WINDOW_TYPE_TOOLTIP),
  GB_CONSTANT("Notification", "i", _NET_WM_WINDOW_TYPE_NOTIFICATION),
  GB_CONSTANT("Combo", "i", _NET_WM_WINDOW_TYPE_COMBO),
  GB_CONSTANT("DragAndDrop", "i", _NET_WM_WINDOW_TYPE_DND),
  GB_CONSTANT("Desktop", "i", _NET_WM_WINDOW_TYPE_DESKTOP),

	GB_END_DECLARE
};

GB_DESC CWindowDesc[] =
{
	GB_DECLARE("Window", sizeof(CWINDOW)), GB_INHERITS("Container"),

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
	GB_PROPERTY("Minimized", "b", CWINDOW_minimized),
	GB_PROPERTY("Maximized", "b", CWINDOW_maximized),
	GB_PROPERTY("FullScreen", "b", CWINDOW_full_screen),
	GB_PROPERTY("TopOnly", "b", CWINDOW_top_only),
	GB_PROPERTY("Stacking", "i", CWINDOW_stacking),
	GB_PROPERTY("Sticky", "b", CWINDOW_sticky),
	GB_PROPERTY("SkipTaskbar", "b", CWINDOW_skip_taskbar),
	GB_PROPERTY("Visible", "b", CWINDOW_visible),
	GB_PROPERTY("Opacity", "i", Window_Opacity),
	
	GB_PROPERTY("Arrangement", "i", CCONTAINER_arrangement),
	GB_PROPERTY("Padding", "i", CCONTAINER_padding),
	GB_PROPERTY("Spacing", "b", CCONTAINER_spacing),
	GB_PROPERTY("Margin", "b", CCONTAINER_margin),
	GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),
	
	GB_PROPERTY("Type", "i", CWINDOW_type),
	GB_PROPERTY("Border", "b", CWINDOW_border),
	GB_PROPERTY("Resizable", "b", CWINDOW_resizable),

	GB_PROPERTY_SELF("Menus", ".WindowMenus"),
	GB_PROPERTY_SELF("Controls", ".WindowControls"),

	WINDOW_DESCRIPTION,

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
	
	FORM_DESCRIPTION,

	GB_END_DECLARE
};


/***************************************************************************

	MyMainWindow

***************************************************************************/

MyMainWindow::MyMainWindow(QWidget *parent, const char *name, bool embedded) :
	QWidget::QWidget(parent, embedded ? Qt::Widget : Qt::Window)
{
	sg = 0;
	//shown = false;
	//border = BorderResizable;
	_border = true;
	_resizable = true;
	//state = StateNormal;
	mustCenter = false;
	_deleted = false;
	_type = _NET_WM_WINDOW_TYPE_NORMAL;
	
	setAttribute(Qt::WA_KeyCompression, true);
	setAttribute(Qt::WA_InputMethodEnabled, true);
	setAttribute(Qt::WA_QuitOnClose, false);
	setAttribute(Qt::WA_StaticContents, true);
	setObjectName(name);
	//setFocusPolicy(ClickFocus);

	_activate = false;
}


MyMainWindow::~MyMainWindow()
{
	CWINDOW *_object = (CWINDOW *)CWidget::get(this);

	#if DEBUG_WINDOW
	qDebug("~MyMainWindow: %s %s %p", GB.GetClassName(THIS), THIS->widget.name, THIS);
	#endif
	
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
		
	GB.Detach(THIS);

	if (THIS->menuBar)
	{
		//CMenu::unrefChildren(THIS->menuBar);
		//qDebug("delete menuBar");
		QMenuBar *menuBar = THIS->menuBar;
		THIS->menuBar = 0;
		delete menuBar;
	}

	CWindow::removeTopLevel(THIS);
	
	_deleted = true;

	//qDebug("~MyMainWindow %p (end)", this);
}


void MyMainWindow::showEvent(QShowEvent *e)
{
	CWINDOW *_object = (CWINDOW *)CWidget::get(this);
	
	emit_open_event(THIS);
	
	//CWINDOW_fix_menubar(THIS);

	if (_activate)
	{
		raise();
		//setFocus();
		activateWindow();
		//X11_window_activate(winId());
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
	X11_set_window_type(winId(), _type);
	#endif
}

void MyMainWindow::afterShow()
{
	if (!CWIDGET_test_flag(THIS, WF_CLOSED))
	{
		//define_mask(THIS, THIS->picture, THIS->masked);	
		THIS->loopLevel = CWINDOW_Current ? CWINDOW_Current->loopLevel : 0;
	}
}

void MyMainWindow::showActivate(QWidget *transient)
{
	CWIDGET *_object = CWidget::get(this);
	QWidget *newParentWidget = 0;

	//qDebug("showActivate: %s %d", THIS->widget.name, isToolbar());

	// Reparent the window if, for example, there is an already modal window displayed
	
	if (CWINDOW_Current && THIS != CWINDOW_Current)
	{
		newParentWidget = CWINDOW_Current->widget.widget;
		
		if (!isVisible())
		{
			if (newParentWidget && parentWidget() != newParentWidget)
			{
				//qDebug("showActivate");
				doReparent(newParentWidget, windowFlags(), pos());
			}
		}
	}

	#ifndef NO_X_WINDOW
	if (isToolbar())
	{
		if (!newParentWidget && CWINDOW_Main && THIS != CWINDOW_Main)
			newParentWidget = CWINDOW_Main->widget.widget;
		
		if (newParentWidget)
			X11_set_transient_for(winId(), newParentWidget->winId());
	}
	#endif

	//qDebug("showActivate %p", _object);

	//CWIDGET_clear_flag(THIS, WF_CLOSED);

	if (!THIS->title && _border)
		setWindowTitle(GB.Application.Title());

	initProperties();

	if (!isVisible())
	{
		//GB.Raise(THIS, EVENT_Open, 0);

		//X11_window_startup(WINDOW->winId(), THIS->x, THIS->y, THIS->w, THIS->h);

		if (isToolbar() && _resizable)
		{
			setMinimumSize(THIS->minw, THIS->minh);
    	setSizeGrip(true);
		}
		else
			setSizeGrip(false);
		
		if (windowState() & Qt::WindowMinimized)
			showMinimized();
		else if (windowState() & Qt::WindowFullScreen)
			showFullScreen();
		else if (windowState() & Qt::WindowMaximized)
			showMaximized();
		else
			show();

		/*if (_type == _NET_WM_WINDOW_TYPE_NORMAL
			  || _type == _NET_WM_WINDOW_TYPE_DOCK
			  || _type == _NET_WM_WINDOW_TYPE_TOOLBAR
			  || _type == _NET_WM_WINDOW_TYPE_MENU
			  || _type == _NET_WM_WINDOW_TYPE_UTILITY
			  || _type == _NET_WM_WINDOW_TYPE_DIALOG
			  || _type == _NET_WM_WINDOW_TYPE_DROPDOWN_MENU
			  || _type == _NET_WM_WINDOW_TYPE_POPUP_MENU
			  || _type == _NET_WM_WINDOW_TYPE_COMBO)
		{*/
		if (hasBorder())
		{
			MAIN_process_events();
			usleep(50000);
			activateWindow();
		}
	}
	else
	{
		//_activate = true;
		
		if (windowState() & Qt::WindowMinimized)
		{
			setWindowState(windowState() & ~Qt::WindowMinimized);
			//qDebug("_activate set #2");
		}
		
		raise();
		if (hasBorder())
			activateWindow();
	}

	afterShow();
}

void MyMainWindow::showModal(void)
{
	Qt::WindowFlags flags = windowFlags();
	CWIDGET *_object = CWidget::get(this);
	bool persistent = CWIDGET_test_flag(THIS, WF_PERSISTENT);
	CWINDOW *save = CWINDOW_Current;
	QPoint p = pos();
	QEventLoop *old;
	QEventLoop eventLoop;

	if (isModal())
		return;

	old = MyApplication::eventLoop;
	MyApplication::eventLoop = &eventLoop;

	mustCenter = true;

	#ifndef NO_X_WINDOW
	if (CWINDOW_Active)
		X11_set_transient_for(winId(), CWINDOW_Active->widget.widget->winId());
	#endif

	setWindowModality(Qt::ApplicationModal);

	if (_resizable && _border)
	{
		setMinimumSize(THIS->minw, THIS->minh);
		setSizeGrip(true);
	}

	THIS->enterLoop = false; // Do not call exitLoop() if we do not entered the loop yet!
	
	show();
	afterShow();
	
	THIS->loopLevel++;
	CWINDOW_Current = THIS;
	
	THIS->enterLoop = true;
	
	eventLoop.exec();
	//eventLoop.processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::DeferredDeletion, 0);
	
	MyApplication::eventLoop = old;
	CWINDOW_Current = save;
		
	if (persistent)
	{
		setSizeGrip(false);
		setWindowModality(Qt::NonModal);
	}
}

bool MyMainWindow::isToolbar(void)
{
	#ifdef NO_X_WINDOW
	return false;
	#else
	return getType() == _NET_WM_WINDOW_TYPE_UTILITY;
	#endif
}

#if 0
void MyMainWindow::setTool(bool t)
{
	WFlags f = getWFlags();

	if (t)
		f |=  WStyle_Tool | WStyle_Customize;
	else
		f &= ~WStyle_Tool;

	doReparent(CWINDOW_Main ? (MyMainWindow *)QWIDGET(CWINDOW_Main) : 0, f, pos());
}
#endif

void MyMainWindow::moveSizeGrip()
{
	CWINDOW *window;
	QWidget *cont;

	if (sg == 0)
		return;

	window = (CWINDOW *)CWidget::get(this);
	cont = window->container;

	if (qApp->isRightToLeft())
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
		//if (paletteBackgroundPixmap())
		//  sg->setBackgroundOrigin(QWidget::AncestorOrigin);
		sg->show();
	}
}

void MyMainWindow::setBorder(bool b, bool force)
{
	Qt::WindowFlags flags;
	
	if (_border == b && !force)
		return;
		
	_border = b;
	flags = windowFlags();
	
	if (b)
		flags &= ~(Qt::FramelessWindowHint|Qt::ToolTip);
	else
		flags |= Qt::FramelessWindowHint|Qt::ToolTip;
	
	doReparent(parentWidget(), flags, pos());
}

void MyMainWindow::setResizable(bool b, bool force)
{
	if (_resizable == b && !force)
		return;
		
	_resizable = b;
	
	if (!b && isWindow())
	{
		setMinimumSize(width(), height());
		setMaximumSize(width(), height());
	}
	else
	{
		setMinimumSize(0, 0);
		setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
	}
}

#ifdef NO_X_WINDOW
#else
int MyMainWindow::getType()
{
	if (!isWindow())
		return 0;
	else
		return _type;
	//return X11_get_window_type(winId());
}

void MyMainWindow::setType(int type)
{
	if (!isWindow())
		return;
	X11_set_window_type(winId(), type);
	_type = type;
}
#endif

void MyMainWindow::paintUnclip(bool on)
{
	setAttribute(Qt::WA_PaintUnclipped, on);
}


void MyMainWindow::moveEvent(QMoveEvent *e)
{
	CWIDGET *_object = CWidget::getReal(this);

	//qDebug("Move: (%s %p) %d %d", GB.GetClassName(THIS), THIS, e->pos().x(), e->pos().y());

	QWidget::moveEvent(e);

	//qDebug("Move (pos %d %d) (oldPos %d %d)", e->pos().x(), e->pos().y(), e->oldPos().x(), e->oldPos().y());
	//qDebug("     (geom %d %d) (fgeom %d %d)", geometry().x(), geometry().y(), frameGeometry().x(), frameGeometry().y());
	//qDebug("     Visible = %s Hidden = %s", (isVisible() ? "Yes" : "No"), (isHidden() ? "Yes" : "No"));
	//qDebug("     Flags = 0x%s State = 0x%s", QString::number(getWFlags(), 16).latin1(), QString::number(getWState(), 16).latin1());

	//if (CWIDGET_test_flag(ob, WF_IGNORE_MOVE))

	//if (THIS->embedded)
	//	return;

	if (THIS->toplevel)
	{
		if (hasBorder() && !THIS->reallyMasked)
			if (geometry().x() == frameGeometry().x() && geometry().y() == frameGeometry().y())
				return;

		if (!isHidden())
		{
			THIS->x = x();
			THIS->y = y();
			//qDebug("moveEvent: x= %d y = %d", x(), y());
		}
	}

	//qDebug("moveEvent %ld %ld isHidden:%s shown:%s ", THIS->x, THIS->y, isHidden() ? "1" : "0", shown ? "1" : "0");

	if (THIS->opened)
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

	//qDebug("Resize %p: %d %d <- %d %d", _object, e->size().width(), e->size().height(), e->oldSize().width(), e->oldSize().height());

	//QMainWindow::resizeEvent(e);

	configure();

	if (sg)
		moveSizeGrip();

	if (!isHidden())
	{
		THIS->w = THIS->container->width();
		THIS->h = THIS->container->height();
		if (isTopLevel())
			CCONTAINER_arrange(THIS);
	}

  #ifndef NO_X_WINDOW
  if (THIS->xembed)
  	XEMBED->resize(THIS->w, THIS->h);
  #endif
  	
  	//qDebug("resizeEvent %ld %ld isHidden:%s shown:%s ", THIS->w, THIS->h, isHidden() ? "1" : "0", shown ? "1" : "0");
	//qDebug("THIS->h = %ld  THIS->container->height() = %ld  height() = %ld", THIS->h, THIS->container->height(), height());

	if (THIS->opened)
	{
		/*w = THIS->w;
		h = THIS->h;*/
		GB.Raise(THIS, EVENT_Resize, 0);
		/*if (w != THIS->w || h != THIS->h)
		{
			GB.Ref(THIS);
			GB.Post((void (*)())post_resize_event, (intptr_t)THIS);
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
		CKEY_info.state = e->modifiers();
		CKEY_info.code = e->key();

		cancel = GB.Raise(THIS, EVENT_KeyPress, 0);

		CKEY_clear(false);

		if (cancel)
			return;
	}
	
	if ((e->modifiers() == Qt::NoModifier || (e->modifiers() & Qt::KeypadModifier && e->key() == Qt::Key_Enter )))
	{
		switch (e->key())
		{
			case Qt::Key_Enter:
			case Qt::Key_Return:

				test = THIS->defaultButton;
				break;

			case Qt::Key_Escape:

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
	QList<CWINDOW *> list(CWindow::list);
	CWINDOW *win;
	int i;

	#if DEBUG_WINDOW
	qDebug("CLOSE ALL");
	#endif

	for (i = 0; i < list.count(); i++)
	{
		win = list.at(i);
		if (win != CWINDOW_Main && do_close(win, 0))
		{
			return true;
		}
	}

	return false;
}

static void deleteAll()
{
	QList<CWINDOW *> list(CWindow::list);
	CWINDOW *win;
	int i;

	#if DEBUG_WINDOW
	qDebug("<<< DELETE ALL");
	#endif

	for (i = 0; i < list.count(); i++)
	{
		win = list.at(i);

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

	#if DEBUG_WINDOW
	qDebug("DELETE ALL >>>");
	#endif

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

	/*if (MAIN_in_wait)
	{
		qDebug("Ignore close event: MAIN_in_wait");
		goto IGNORE;
	}*/

	//if (CWINDOW_Current && (THIS != CWINDOW_Current))
	if (CWINDOW_Current && (THIS->loopLevel != CWINDOW_Current->loopLevel))
	{
		//qDebug("ignore close event");
		goto IGNORE;
	}

	if (THIS->opened)
	{
		//qDebug("THIS->opened = %d: %p: %s", THIS->opened, THIS, GB.GetClassName(THIS));
		THIS->closing = true;
		//qDebug("Close event: %s %p", GB.GetClassName(THIS), THIS);
		cancel = GB.Raise(THIS, EVENT_Close, 0);
		THIS->closing = false;
	}

	if (!cancel && THIS == CWINDOW_Main)
	{
		if (closeAll())
			cancel = true;
	}

	if (cancel)
		goto IGNORE;

	modal = isModal(); //testWFlags(Qt::WShowModal); // && THIS->opened;

	CWIDGET_set_flag(THIS, WF_CLOSED);
	//qApp->sendEvent(WIDGET, new QEvent(EVENT_CLOSE));

	if (!CWIDGET_test_flag(_object, WF_PERSISTENT))
	{
		if (CWINDOW_Main == THIS)
		{
			deleteAll();
			#if DEBUG_WINDOW
			qDebug("CWINDOW_Main -> 0");
			#endif
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
	//qDebug("THIS->opened <- false: %p: %s", THIS, GB.GetClassName(THIS));
	THIS->opened = false;
	
	//qDebug("THIS->enterLoop = %d", THIS->enterLoop);
	if (modal && THIS->enterLoop)
	{
		//qDebug("exitLoop: %p", THIS);
		MyApplication::eventLoop->exit();
	}

	return;

IGNORE:

	CWIDGET_clear_flag(THIS, WF_CLOSED);
	e->ignore();
}



bool MyMainWindow::isPersistent(void)
{
	return !testAttribute(Qt::WA_DeleteOnClose); //testWFlags(WDestructiveClose);
}


void MyMainWindow::setPersistent(bool pers)
{
	setAttribute(Qt::WA_DeleteOnClose, !pers);
}

void MyMainWindow::doReparent(QWidget *parent, Qt::WindowFlags f, const QPoint &pos)
{
	CWINDOW *_object = (CWINDOW *)CWidget::get(this);
	QIcon icon;
	bool old_toplevel;
	bool hidden;
	#ifndef NO_X_WINDOW
	bool active = qApp->activeWindow() == this;
	#endif

	icon = windowIcon();

	old_toplevel = THIS->toplevel;
	THIS->toplevel = !parent || parent->isWindow();
	THIS->embedded = !THIS->toplevel;

	if (THIS->toplevel)
	{
		f |= Qt::Window;
		if (!old_toplevel)
			CWindow::insertTopLevel(THIS);
	}
	else	
	{
		f &= ~Qt::WindowType_Mask;
		if (old_toplevel)
		{
			THIS->toplevel = true;
			CWindow::removeTopLevel(THIS);
			THIS->toplevel = false;
		}
	}

	//qDebug("doReparent: %s %p: visible = %d opened = %d hidden = %d isVisible = %d isHidden = %d shown = %d", 
	//				THIS->widget.name, THIS, THIS->widget.flag.visible, THIS->opened, THIS->hidden, isVisible(), isHidden(), THIS->widget.flag.shown);
	//if (!THIS->hidden) showIt = true;
	//hide();
	hidden = THIS->hidden;
	//qDebug("doReparent: %s %p: hidden = %d", THIS->widget.name, THIS, hidden);
	setParent(parent, f);
	move(pos);
	//qDebug("doReparent: (%s %p) (%d %d) -> (%d %d)", GB.GetClassName(THIS), THIS, pos.x(), pos.y(), WIDGET->x(), WIDGET->y());
	
	if (!THIS->embedded)
	{
		#ifndef NO_X_WINDOW
			initProperties();
			if (active && hasBorder())
				activateWindow();
		#endif

		setWindowIcon(icon);
	}
	
	//qDebug("--> isVisible = %d isHidden = %d", isVisible(), isHidden());
	
	/*if (THIS->embedded && !THIS->hidden)
	{
		qDebug("doReparent: %s %p: show_later", THIS->widget.name, THIS);
		#if DEBUG_WINDOW
		qDebug("post show_later %s %p", GB.GetClassName(THIS), THIS);
		#endif
		GB.Ref(THIS);
		//GB.Post((void (*)())show_later, (intptr_t)THIS);
		show_later(THIS);
		//WIDGET->show();
	}*/
	
	/*if (parentWidget())
		qDebug("doReparent (%s %p): new parent = (%s %p)", THIS->widget.name, THIS, CWidget::get(parentWidget())->name, CWidget::get(parentWidget()));
	else
		qDebug("doReparent (%s %p): new parent = 0", THIS->widget.name, THIS);*/
	
	if (!hidden)
		CWINDOW_show(THIS, NULL);
}


void MyMainWindow::center(bool force = false)
{
	CWINDOW *_object = (CWINDOW *)CWidget::get(this);
	QPoint p;

	if (!force && !mustCenter)
		return;

	mustCenter = false;

	p.setX((qApp->desktop()->availableGeometry().width() - width()) / 2);
	p.setY((qApp->desktop()->availableGeometry().height() - height()) / 2);

	CWIDGET_move(THIS, p.x(), p.y());
}

void MyMainWindow::configure()
{
	CWINDOW *_object = (CWINDOW *)CWidget::get(this);
	QMenuBar *menuBar = THIS->menuBar;
	int h;
	
	//qDebug("THIS->menuBar = %p  menuBar() = %p", THIS->menuBar, menuBar());

	if (menuBar && THIS->showMenuBar && !THIS->hideMenuBar)
	{
		h = menuBar->sizeHint().height();
		menuBar->setGeometry(0, 0, this->width(), h);
		menuBar->show();
		THIS->container->setGeometry(0, h, this->width(), this->height() - h);
	}
	else
	{
		if (menuBar)
		{
			menuBar->move(0, -menuBar->height());
			//menuBar->lower();
		}
		//qDebug("configure: %s (%d %d)", GB.GetClassName(THIS), this->width(), this->height());
		THIS->container->setGeometry(0, 0, this->width(), this->height());
		//THIS->container->setGeometry(0, 0, THIS->w, THIS->h);
		THIS->container->raise();
	}
	
	CCONTAINER_arrange(THIS);

	//qDebug(">>> THIS->menuBar = %p  menuBar() = %p", THIS->menuBar, menuBar());

	//qDebug("configure: %p (%d %d %d %d)", THIS, ((QFrame *)(THIS->container))->contentsRect().x(), ((QFrame *)(THIS->container))->contentsRect().y(), ((QFrame *)(THIS->container))->contentsRect().width(), ((QFrame *)(THIS->container))->contentsRect().height());
}


void MyMainWindow::setName(const char *name, CWIDGET *control)
{
	if (_deleted)
		return;
		
	names.remove(name);
	if (control)
		names.insert(name, control);
}

void MyMainWindow::resize(int w, int h)
{
	bool save = _resizable;
	
	if (!_resizable)
		setResizable(true);
		
	QWidget::resize(w, h);
	
	if (_resizable != save)
		setResizable(save);
}

void MyMainWindow::setGeometry(int x, int y, int w, int h)
{
	bool save = _resizable;
	
	if (!_resizable)
		setResizable(true);
		
	QWidget::setGeometry(x, y, w, h);
	
	if (_resizable != save)
		setResizable(save);
}


void MyMainWindow::changeEvent(QEvent *e)
{
	QWidget::changeEvent(e);
	if (e->type() == QEvent::StyleChange || e->type() == QEvent::FontChange)
		configure();
}


/***************************************************************************

	CWindow

***************************************************************************/

CWindow CWindow::manager;
int CWindow::count = 0;
QList<CWINDOW *> CWindow::list;

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
			GB.Post((void (*)())post_deactivate_event, (intptr_t)CWINDOW_Active);
		}*/
		CWINDOW_Active = 0;
	}

	if (active)
	{
		GB.Raise(active, EVENT_Activate, 0);
		/*if (GB.CanRaise(active, EVENT_Activate))
		{
			GB.Ref(active);
			GB.Post((void (*)())post_activate_event, (intptr_t)active);
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


bool CWindow::eventFilter(QObject *o, QEvent *e)
{
	CWINDOW *_object = (CWINDOW *)CWidget::get(o);

	if (THIS != NULL)
	{
		if (e->type() == QEvent::WindowActivate && e->spontaneous())
		{
			if (THIS->toplevel)
			{
				//qDebug("Activate: ob = %p", THIS);
				
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
				//qDebug("Deactivate: ob = %p", THIS);
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
				
			post_show_event(THIS);
			//CWINDOW_define_mask(THIS);
			
			GB.Raise(THIS, EVENT_Show, 0);
			if (!e->spontaneous())
				CACTION_raise(THIS);			
		}
		else if (e->type() == QEvent::Hide) // && !e->spontaneous())
		{
			//qDebug("Hide: %s %d (%d)", GB.GetClassName(THIS), WINDOW->isHidden(), e->spontaneous());
			//if (WINDOW->isHidden())
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
		CWindow::removeTopLevel(THIS);
	}

	CWINDOW_EmbedState = EMBED_WAIT;
	CWINDOW_Embedded = false;
	CWINDOW_Embedder = 0;
}

void CWindow::insertTopLevel(CWINDOW *_object)
{
	if (!THIS->toplevel)
		return;
	
	list.append(THIS);
	count = list.count();

	#if DEBUG_WINDOW
	qDebug("insertTopLevel: count = %d (%p %s %s)", count, _object, THIS->widget.name, THIS->embedded ? "E" : "W");
	#endif
}

void CWindow::removeTopLevel(CWINDOW *_object)
{
	if (!THIS->toplevel)
		return;

	list.removeAll(THIS);
  count = list.count();
	
	#if DEBUG_WINDOW
  qDebug("removeTopLevel: count = %d (%p %s %s)", count, THIS, THIS->widget.name, THIS->embedded ? "E" : "W");
	#endif

	MAIN_check_quit();
}
