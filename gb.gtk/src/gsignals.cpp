/***************************************************************************

  gsignals.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#include "widgets.h"
#include <gdk/gdkkeysyms.h>

#include "gapplication.h"
#include "gdrawingarea.h"
#include "gkey.h"
#include "gmouse.h"
#include "gmainwindow.h"
#include "gdrag.h"
#include "gdesktop.h"

//#define DEBUG_DND 1

static void  sg_destroy (GtkWidget *object,gControl *data)
{
	if (data->_no_delete)
		return;
		
	//if (!data->_destroyed)
	delete data;
}

static gboolean sg_menu(GtkWidget *widget, gControl *data)
{
	if (!gApplication::userEvents()) return false;
	if (data->onMouseEvent)
		return data->onMouseEvent(data, gEvent_MouseMenu);
	else
		return false;
}

gboolean gcb_focus_in(GtkWidget *widget,GdkEventFocus *event,gControl *data)
{
	if (!gApplication::allEvents()) return false;

	//fprintf(stderr, "gcb_focus_in: %s\n", data->name());
	
	gApplication::setActiveControl(data, true);
	
	return false;
}

gboolean gcb_focus_out(GtkWidget *widget,GdkEventFocus *event,gControl *data)
{	
	if (!gApplication::allEvents()) return false;
	
	//fprintf(stderr, "gcb_focus_out: %s\n", data->name());
	
	gApplication::setActiveControl(data, false);
	
	return false;
}


/****************************************************
 Drag 
*****************************************************/

static void sg_drag_data_get(GtkWidget *widget, GdkDragContext *context, GtkSelectionData *dt, guint i, guint time, gControl *data)
{
	char *text;
	int len;
	gPicture *pic;
	//g_debug("sg_drag_data_get\n");
	
	context = gDrag::enable(context, data, time);
	
	text = gDrag::getText(&len, NULL, true);
	if (text)
	{
		gtk_selection_data_set_text(dt, text, len);
		return;
	}
	
	pic = gDrag::getImage(true);
	if (pic)
	{
		gtk_selection_data_set_pixbuf(dt, pic->getPixbuf());
	}
	
	gDrag::disable(context);
}

static void sg_drag_end(GtkWidget *widget,GdkDragContext *ct,gControl *data)
{
	#if DEBUG_DND
	fprintf(stderr, "sg_drag_end: %s\n", data->name());
	#endif
	
	gDrag::end();
}


/****************************************************
 Drop 
*****************************************************/

// BM: What for?
//static guint32 _drag_time = 0;

static gboolean sg_drag_motion(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time, gControl *data)
{
	bool retval = true;
	int action;
	gControl *source;
	
	if (!gApplication::allEvents()) return true;
	
	#if DEBUG_DND
	fprintf(stderr, "sg_drag_motion: %s\n", data->name());
	#endif	
	/*if (_drag_time != context->start_time) 
	{ 
		g_debug("sg_drag_motion: cancel!\n");
		gDrag::cancel();
		data->_drop_0 = true; 
		data->_drop_1 = false; 
		data->_drop_2 = false; 
	}*/
	
#if GTK_CHECK_VERSION(2, 22, 0)
	switch (gdk_drag_context_get_suggested_action(context))
#else
	switch (context->suggested_action)
#endif
	{
  	case GDK_ACTION_MOVE: 
  		action = gDrag::Move; 
  		break;
  	case GDK_ACTION_LINK: 
  		action = gDrag::Link; 
  		break;
		case GDK_ACTION_COPY:
		default:
			action = gDrag::Copy;
	}
	
	source = gApplication::controlItem(gtk_drag_get_source_widget(context));
	gDrag::setDropData(action, x, y, source, NULL);
	
	context = gDrag::enable(context, data, time);
	
	if (!data->_drag_enter)
	{
		//fprintf(stderr, "sg_drag_motion: onDrag: %p\n", widget);
	
		x = 0; 
		y = 0; 
		
		if (data->onDrag) 
			retval = !data->onDrag(data);
		data->_drag_enter = true;
	}
	
	if (retval)
	{
		//fprintf(stderr, "sg_drag_motion: onDragMove: %p\n", widget);
		gControl *control = data;
		
		//while (control->_proxy)
		//	control = control->_proxy;
		
		while (control)
		{
			#if DEBUG_DND
			fprintf(stderr, "drag move %s\n", control->name());
			#endif
			if (control->canRaise(control, gEvent_DragMove))
			{
				if (control->onDragMove) 
				{
					retval = !control->onDragMove(control);
					if (!retval)
						break;
				}
			}
			control = control->_proxy;
		}
	}
	
	context = gDrag::disable(context);
	
	if (retval) 
	{
		//fprintf(stderr, "sg_drag_motion: accept\n");
#if GTK_CHECK_VERSION(2, 22, 0)
		gdk_drag_status(context, gdk_drag_context_get_suggested_action(context), time);
#else
		gdk_drag_status(context, context->suggested_action, time);
#endif
		return true;
	}
	
	//fprintf(stderr, "sg_drag_motion: cancel\n");
	gDrag::hide(data);
	return false;
}


void sg_drag_leave(GtkWidget *widget, GdkDragContext *context, guint time, gControl *data)
{
	#if DEBUG_DND
	fprintf(stderr, "sg_drag_leave: %s\n", data->name());
	#endif
	
	data->_drag_enter = false;
	gDrag::hide(data);

	gControl *control = data;
	
	//while (control->_proxy)
	//	control = control->_proxy;
	
	while (control)
	{
		control->emit(SIGNAL(control->onDragLeave));
		control = control->_proxy;
	}
}


gboolean sg_drag_drop(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time, gControl *data)
{
	gControl *source;

	#if DEBUG_DND
	fprintf(stderr, "sg_drag_drop: %s\n", data->name());
	#endif
	
	// sg_drag_leave() is automatically called when a drop occurs
	//sg_drag_leave(widget, context, time, data);
	
	if (!data->canRaise(data, gEvent_Drop))
	{
		gtk_drag_finish(context, false, false, time);
		return false;
	}

	source = gApplication::controlItem(gtk_drag_get_source_widget(context));

	gDrag::setDropData(gDrag::getAction(), x, y, source, data);
	
	context = gDrag::enable(context, data, time);
	data->_drag_get_data = true;
	
	if (data->onDrop)
		data->onDrop(data);
	
	context = gDrag::disable(context);

	//fprintf(stderr, "cancel = %d\n", cancel);
	
	gtk_drag_finish(context, true, false, time);
	
	//data->_drag_enter = false;
	data->_drag_get_data = false;

	return true;
}

// void sg_size(GtkWidget *widget,GtkRequisition *req, gContainer *data)
// {
// 	if (data->parent()) data->parent()->performArrange();
// 	if (data->isContainer()) data->performArrange();
// }

static void cb_show(GtkWidget *widget, gContainer *data)
{
	data->performArrange();
}

void gControl::borderSignals()
{	
	g_signal_connect(G_OBJECT(border),"destroy",G_CALLBACK(sg_destroy),(gpointer)this);
	//g_signal_connect(G_OBJECT(border),"drag-data-received",G_CALLBACK(sg_drag_data_received),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"drag-motion",G_CALLBACK(sg_drag_motion),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"drag-leave",G_CALLBACK(sg_drag_leave),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"drag-drop",G_CALLBACK(sg_drag_drop),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"drag-data-get",G_CALLBACK(sg_drag_data_get),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"drag-end",G_CALLBACK(sg_drag_end),(gpointer)this);
	//g_signal_connect(G_OBJECT(border),"enter-notify-event",G_CALLBACK(sg_enter),(gpointer)this);
	//g_signal_connect(G_OBJECT(border),"leave-notify-event",G_CALLBACK(sg_enter),(gpointer)this);
	
	//g_signal_connect_after(G_OBJECT(border),"size-allocate",G_CALLBACK(sg_size),(gpointer)this);
	
	if (isContainer())
		g_signal_connect(G_OBJECT(border), "show", G_CALLBACK(cb_show), (gpointer)this);

	if (border != widget && !_scroll)
	{
		/*if (!_no_default_mouse_event)
		{
			g_signal_connect(G_OBJECT(border),"button-release-event",G_CALLBACK(gcb_button_release),(gpointer)this);
			g_signal_connect(G_OBJECT(border),"button-press-event",G_CALLBACK(gcb_button_press),(gpointer)this);
		}*/
		g_signal_connect(G_OBJECT(border),"popup-menu",G_CALLBACK(sg_menu),(gpointer)this);	
		//g_signal_connect_after(G_OBJECT(border),"motion-notify-event",G_CALLBACK(sg_motion),(gpointer)this);
		//g_signal_connect(G_OBJECT(border),"scroll-event",G_CALLBACK(sg_scroll),(gpointer)this);
	}
}

void gControl::widgetSignals()
{
	if (!(border != widget && !_scroll))
	{
		//g_signal_connect(G_OBJECT(widget),"scroll-event",G_CALLBACK(sg_scroll),(gpointer)this);
		/*if (!_no_default_mouse_event)
		{
			g_signal_connect(G_OBJECT(widget),"button-release-event",G_CALLBACK(gcb_button_release),(gpointer)this);
			g_signal_connect(G_OBJECT(widget),"button-press-event",G_CALLBACK(gcb_button_press),(gpointer)this);
		}*/
		//g_signal_connect(G_OBJECT(widget),"motion-notify-event",G_CALLBACK(sg_motion),(gpointer)this);
		g_signal_connect(G_OBJECT(widget),"popup-menu",G_CALLBACK(sg_menu),(gpointer)this);
	}	
	
	//g_signal_connect(G_OBJECT(widget),"key-press-event",G_CALLBACK(gcb_keypress),(gpointer)this);
	//g_signal_connect(G_OBJECT(widget),"key-release-event",G_CALLBACK(gcb_keyrelease),(gpointer)this);
	g_signal_connect(G_OBJECT(widget),"focus-in-event",G_CALLBACK(gcb_focus_in),(gpointer)this);
	g_signal_connect(G_OBJECT(widget),"focus-out-event",G_CALLBACK(gcb_focus_out),(gpointer)this);
	//g_signal_connect(G_OBJECT(widget),"event",G_CALLBACK(sg_event),(gpointer)this);
	if (widget != border)
	{
		g_signal_connect(G_OBJECT(widget), "drag-end", G_CALLBACK(sg_drag_end), (gpointer)this);
	}
}

void gControl::initSignals()
{	
	borderSignals();
	widgetSignals();
}
