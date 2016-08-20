/***************************************************************************

  CWatcher.cpp

  (c) 2004-2005 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#define __CWATCHER_CPP

#include <gtk/gtk.h>
#include "main.h"
#include "CWatcher.h"

DECLARE_EVENT(EVENT_Move);
DECLARE_EVENT(EVENT_Resize);
DECLARE_EVENT(EVENT_Show);
DECLARE_EVENT(EVENT_Hide);
//DECLARE_EVENT(EVENT_Remove);

static void raise_show(GtkWidget *widget, CWATCHER *_object)
{
	GB.Raise(THIS, EVENT_Show, 0);
}

static void raise_hide(GtkWidget *widget, CWATCHER *_object)
{
	GB.Raise(THIS, EVENT_Hide, 0);
}

static void raise_configure(GtkWidget *widget, GdkEventConfigure *e, CWATCHER *_object)
{
	GB.Ref(_object);
	
	if (THIS->x != e->x || THIS->y != e->y)
	{
		THIS->x = e->x;
		THIS->y = e->y;
		GB.Raise(THIS, EVENT_Move, 0);
	}
	
	if (THIS->w != e->width || THIS->h != e->height)
	{
		THIS->w = e->width;
		THIS->h = e->height;
		GB.Raise(THIS, EVENT_Resize, 0);
	}
	
	GB.Unref(POINTER(&_object));
}

static void cb_destroy(GtkWidget *widget, CWATCHER *_object)
{
	GB.Unref(POINTER(&THIS->wid));
	THIS->wid = 0;
}


/** Watcher class *********************************************************/

BEGIN_METHOD(CWATCHER_new, GB_OBJECT control)

	GtkWidget *wid;
	gControl *control;

	THIS->wid = (CWIDGET*)VARG(control);
	
	if (GB.CheckObject(THIS->wid))
		return;

	//fprintf(stderr, "Watcher %p: Ref %p (%s %p)\n", _object, THIS->wid, GB.GetClassName(THIS->wid), THIS->wid);
	GB.Ref((void*)THIS->wid);

	control = THIS->wid->widget;
	
	THIS->x = control->left() - 1;
	THIS->y = control->top() - 1;
	THIS->w = control->width() - 1;
	THIS->h = control->height() - 1;

	wid = THIS->wid->widget->border;
	g_signal_connect(G_OBJECT(wid), "map", G_CALLBACK(raise_show), _object);
	g_signal_connect(G_OBJECT(wid), "unmap", G_CALLBACK(raise_hide), _object);
	g_signal_connect(G_OBJECT(wid), "configure-event", G_CALLBACK(raise_configure), _object);
	g_signal_connect(G_OBJECT(wid), "destroy", G_CALLBACK(cb_destroy), _object);

END_METHOD

BEGIN_METHOD_VOID(CWATCHER_free)

	//fprintf(stderr, "Watcher %p: UnRef %p %p ?\n", THIS, THIS->wid, THIS->wid ? THIS->wid->widget : 0);
	
	if (THIS->wid)
	{
		if (THIS->wid->widget)
			g_signal_handlers_disconnect_matched(G_OBJECT(THIS->wid->widget->border), G_SIGNAL_MATCH_DATA, 0, (GQuark)0, NULL, NULL, _object);
		GB.Unref(POINTER(&THIS->wid));
	}

END_METHOD

BEGIN_PROPERTY(CWATCHER_control)

	GB.ReturnObject((void*)THIS->wid);

END_PROPERTY

GB_DESC CWatcherDesc[] =
{
  GB_DECLARE("Watcher", sizeof(CWATCHER)),

  GB_METHOD("_new", 0, CWATCHER_new, "(Control)Control;"),
  GB_METHOD("_free", 0, CWATCHER_free, 0),

  GB_PROPERTY("Control", "Control", CWATCHER_control),

  GB_EVENT("Move", 0, 0, &EVENT_Move),
  GB_EVENT("Resize", 0, 0, &EVENT_Resize),
  GB_EVENT("Show", 0, 0, &EVENT_Show),
  GB_EVENT("Hide", 0, 0, &EVENT_Hide),

  GB_CONSTANT("_DefaultEvent", "s", "Resize"),

  GB_END_DECLARE
};

