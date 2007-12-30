/***************************************************************************

  CWatcher.cpp

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

#define __CWATCHER_CPP

#include <qevent.h>
#include <qscrollview.h>

#include "main.h"
#include "CWatcher.h"

DECLARE_EVENT(EVENT_Move);
DECLARE_EVENT(EVENT_Resize);
DECLARE_EVENT(EVENT_Show);
DECLARE_EVENT(EVENT_Hide);
//DECLARE_EVENT(EVENT_Remove);

/** ChildEvent class ********************************************************/

struct ChildEvent
{
	int event;
	void *watcher;
	void *child;

	ChildEvent(int e, void *w, void *c);
	~ChildEvent();
};

ChildEvent::ChildEvent(int e, void *w, void *c)
{
	event = e;
	watcher = w;
	child = c;

	GB.Ref(watcher);
	if (child)
		GB.Ref(child);
}

ChildEvent::~ChildEvent()
{
	GB.Unref(&watcher);
	if (child)
		GB.Unref(&child);
}

/** CWatcher class ********************************************************/

CWatcher::CWatcher(CWATCHER *w, CWIDGET *o)
{
	watcher = w;
	control = o;

	GB.Ref(control);

	widget = QWIDGET(control);
	cont = 0;

	if (GB.Is(control, CLASS_Container))
		cont = QCONTAINER(control);
	if (cont == widget)
		cont = 0;

	widget->installEventFilter(this);
	if (cont)
		cont->installEventFilter(this);
}

CWatcher::~CWatcher()
{
	if (control)
	{
		if (control->widget)
		{
			if (cont)
				cont->removeEventFilter(this);
			widget->removeEventFilter(this);
		}

		GB.Unref(POINTER(&control));
	}
}

/*void post_child_event(ChildEvent *e)
{
	GB.Raise(e->watcher, e->event, 1, GB_T_OBJECT, e->child);
	delete e;
}*/

void send_event(void *watcher, long event)
{
	GB.Raise(watcher, event, 0);
	GB.Unref(&watcher);
}

bool CWatcher::eventFilter(QObject* o, QEvent *e)
{
	if (o == widget)
	{
		if (e->type() == QEvent::Move)
			GB.Raise(watcher, EVENT_Move, 0);
		else if (e->type() == QEvent::Resize)
		{
			//GB.Ref(watcher);
			//GB.Post2((void (*)())send_event, (long)watcher, (long)EVENT_Resize);
			GB.Raise(watcher, EVENT_Resize, 0);
		}
		else if (e->type() == QEvent::Show)
			GB.Raise(watcher, EVENT_Show, 0);
		else if (e->type() == QEvent::Hide)
			GB.Raise(watcher, EVENT_Hide, 0);
	}

	/*if (e->type() == EVENT_TITLE)
		GB.Raise(watcher, EVENT_Title, 0);
	else if (e->type() == EVENT_ICON)
		GB.Raise(watcher, EVENT_Icon, 0);
	else if (e->type() == EVENT_CLOSE)
		GB.Raise(watcher, EVENT_Close, 0);
	else*/ if (e->type() == EVENT_DESTROY)
	{
	  //qDebug("Watcher: EVENT_DESTROY");
		GB.Unref(POINTER(&control));
		control = 0;
	}
	/*else if (e->type() == QEvent::ChildInserted)
	{
		void *child = CWidget::get((QWidget *)(((QChildEvent *)e)->child()));
		if (child && child != control && GB.CanRaise(watcher, EVENT_Insert))
			GB.Post((GB_POST_FUNC)post_child_event, (long)new ChildEvent(EVENT_Insert, watcher, child));
		//GB.Raise(watcher, EVENT_Insert, 1, GB_T_OBJECT, );
	}*/
	
		//GB.Post((void (*)())post_child_event, (long)new ChildEvent(EVENT_Insert, watcher, ((QChildEvent *)e)->child()));
	/*else if (e->type() == QEvent::ChildRemoved)
		//GB.Post((void (*)())post_child_event, (long)new ChildEvent(EVENT_Remove, watcher, ((QChildEvent *)e)->child()));
		post_child_event(new ChildEvent(EVENT_Remove, watcher, ((QChildEvent *)e)->child()));*/

	return false; //return QObject::eventFilter(o, e);
}

/** Watcher class *********************************************************/

BEGIN_METHOD(CWATCHER_new, GB_OBJECT control)

	CWIDGET *control = (CWIDGET *)VARG(control);

	if (GB.CheckObject(control))
		return;

	THIS->watcher = new CWatcher(THIS, control);

	// No need to reference control, as it is already referenced as event observer!

END_METHOD

BEGIN_METHOD_VOID(CWATCHER_free)

	delete THIS->watcher;
	THIS->watcher = 0;

END_METHOD

BEGIN_PROPERTY(CWATCHER_control)

	GB.ReturnObject(THIS->watcher->getControl());

END_PROPERTY

GB_DESC CWatcherDesc[] =
{
  GB_DECLARE("Watcher", sizeof(CWATCHER)),

  GB_METHOD("_new", NULL, CWATCHER_new, "(Control)Control;"),
  GB_METHOD("_free", NULL, CWATCHER_free, NULL),

	GB_PROPERTY("Control", "Control", CWATCHER_control),

  GB_EVENT("Move", NULL, NULL, &EVENT_Move),
  GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),
  GB_EVENT("Show", NULL, NULL, &EVENT_Show),
  GB_EVENT("Hide", NULL, NULL, &EVENT_Hide),

  GB_CONSTANT("_DefaultEvent", "s", "Resize"),

  GB_END_DECLARE
};
