/***************************************************************************

  CWatcher.cpp

  (c) 2004-2005 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  GTK+ component
  
  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx
  
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

#include <gtk/gtk.h>
#include "main.h"
#include "CWatcher.h"

DECLARE_EVENT(EVENT_Move);
DECLARE_EVENT(EVENT_Resize);
DECLARE_EVENT(EVENT_Show);
DECLARE_EVENT(EVENT_Hide);
DECLARE_EVENT(EVENT_Title);
DECLARE_EVENT(EVENT_Icon);
DECLARE_EVENT(EVENT_Close);

void watcher_show (GtkWidget *widget,CWATCHER *wid)
{
	GB.Raise((void*)wid,EVENT_Show,0);
}

void watcher_hide (GtkWidget *widget,CWATCHER *wid)
{
	GB.Raise((void*)wid,EVENT_Hide,0);
}

/** Watcher class *********************************************************/

BEGIN_METHOD(CWATCHER_new, GB_OBJECT control)

	GtkWidget *wid;

	THIS->wid=(CWIDGET*)VARG(control);
	if (!THIS->wid) return;
	if (!THIS->wid->widget->border) { THIS->wid=NULL; return; }

	GB.Ref((void*)THIS->wid);

	wid=THIS->wid->widget->border;
	g_signal_connect(G_OBJECT(wid),"show",G_CALLBACK(watcher_show),_object);
	g_signal_connect(G_OBJECT(wid),"hide",G_CALLBACK(watcher_hide),_object);

END_METHOD

BEGIN_METHOD_VOID(CWATCHER_free)

	if (THIS->wid) GB.Unref((void**)&THIS->wid);

END_METHOD

BEGIN_PROPERTY(CWATCHER_control)

	GB.ReturnObject((void*)THIS->wid);

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
  GB_EVENT("Title", NULL, NULL, &EVENT_Title),
  GB_EVENT("Icon", NULL, NULL, &EVENT_Icon),
  GB_EVENT("Close", NULL, NULL, &EVENT_Close),

  GB_CONSTANT("_DefaultEvent", "s", "Resize"),

  GB_END_DECLARE
};
