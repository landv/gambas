/***************************************************************************

  gsignals.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  Gtkmae "GTK+ made easy" classes
  
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

#include "widgets.h"
#include "widgets_private.h"
#include <gdk/gdkkeysyms.h>

#include "gapplication.h"
#include "gdrawingarea.h"
#include "gkey.h"
#include "gmouse.h"
#include "gmainwindow.h"
#include "gdrag.h"

static void  sg_destroy (GtkWidget *object,gControl *data)
{
	//fprintf(stderr, "sg_destroy: %p\n", data);
	
	if (data->_no_delete)
		return;
		
	//if (!data->_destroyed)
	delete data;
}


static gboolean  sg_enter(GtkWidget *widget, GdkEventCrossing *e,gControl *data)
{
	if (!gApplication::userEvents()) return false;

	if (data->onEnterLeave)
	{
		if (e->type==GDK_ENTER_NOTIFY)
			data->onEnterLeave(data,gEvent_Enter);
		else
			data->onEnterLeave(data,gEvent_Leave);
	}
	return false;
}

/*
static gboolean sg_configure(GtkWidget *widget,GdkEventConfigure *event,gControl *data)
{
	GtkWidget *fr;
	GList *chd;

	if (!gApplication::allEvents()) return false;

	if ( (event->width!=data->bufW) || (event->height!=data->bufH)  )
	{
		data->bufW=event->width;
		data->bufH=event->height;
	}
	
	if ( data->getClass() != Type_gMainWindow )
	{
		data->bufX=event->x;
		data->bufY=event->y;
	}
	
	if ( data->getClass()==Type_gFrame )
	{
		chd=gtk_container_get_children(GTK_CONTAINER(data->widget));
		fr=(GtkWidget*)chd->data;
		g_list_free(chd);
		gtk_widget_set_size_request(fr,event->width,event->height);
	}
	
	
	return false;
}
*/

static bool check_button(gControl *w)
{
	return w && w->isVisible() && w->enabled();
}

gboolean gcb_keypress (GtkWidget *widget, GdkEventKey *event, gControl *data)
{
	bool vl = false;
	
	if (!gApplication::userEvents()) return false;
	
	gKey::enable(event);
	if (data->onKeyEvent) 
		vl = data->onKeyEvent(data,gEvent_KeyPress);
	gKey::disable();
	
	if (vl)
		return true;
	
	if (event->keyval == GDK_Escape)
	{
		gMainWindow *win = data->window();
		if (check_button(win->_cancel))
		{
			win->_cancel->animateClick(false);
			vl = true;
		}
	}
	else if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
	{
		gMainWindow *win = data->window();
		if (check_button(win->_default))
		{
			win->_default->animateClick(false);
			vl = true;
		}
	}
	
	return vl;
}

gboolean gcb_keyrelease (GtkWidget *widget, GdkEventKey *event, gControl *data)
{
	if (!gApplication::userEvents()) return false;
	
	gKey::enable(event);
	if (data->onKeyEvent) data->onKeyEvent(data,gEvent_KeyRelease);
	gKey::disable();
	
	if (event->keyval == GDK_Escape)
	{
		gMainWindow *win = data->window();
		if (check_button(win->_cancel))
			win->_cancel->animateClick(true);
	}
	else if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
	{
		gMainWindow *win = data->window();
		if (check_button(win->_default))
			win->_default->animateClick(true);
	}
	
	return false;
}

static gboolean sg_button_Press (GtkWidget *widget,GdkEventButton *event,gControl *data)
{
	if (!gApplication::userEvents()) return false;

	if (data->onMouseEvent)
	{
		gMouse::validate();
		gMouse::setStart((int)event->x, (int)event->y);
		gMouse::setMouse((int)event->x, (int)event->y, event->button, event->state);
		//gMouse::setValid(1,(int)event->x,(int)event->y,event->button,event->state,data->screenX(),data->screenY());
		data->onMouseEvent(data,gEvent_MousePress);
		gMouse::invalidate();
		
		if (event->button == 3 && event->type == GDK_BUTTON_PRESS)
		{
			return data->onMouseEvent(data, gEvent_MouseMenu);
		}
	}
	return false;
}

static gboolean sg_motion(GtkWidget *widget,GdkEventMotion *event,gControl *data)
{
	bool cont = false;
	
	if (!gApplication::userEvents()) return false;
	
	// BM: What are these tests?
	if ((data->getClass() & 0x1000) ) { cont=true; }
	else if ( (event->state & 0x1F00) ) { cont=true; } 
	else if ( data->getClass()==Type_gDrawingArea )
	{
		if ( ((gDrawingArea*)data)->track ) cont=true;	
	}

	if (cont && data->onMouseEvent)
	{
		gMouse::validate();
		gMouse::setMouse((int)event->x, (int)event->y, 0, event->state);
		data->onMouseEvent(data, gEvent_MouseMove);
		//if (data->acceptDrops() && gDrag::checkThreshold(data, gMouse::x(), gMouse::y(), gMouse::startX(), gMouse::startY()))
		if ((event->state & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK)) 
		    //&& (abs(gMouse::x() - gMouse::y()) + abs(gMouse::startX() - gMouse::startY())) > 8)
		    && gDrag::checkThreshold(data, gMouse::x(), gMouse::y(), gMouse::startX(), gMouse::startY()))
			data->onMouseEvent(data, gEvent_MouseDrag);
		gMouse::invalidate();
	}
	
	return FALSE;
}

static gboolean sg_menu(GtkWidget *widget, gControl *data)
{
	if (!gApplication::userEvents()) return false;
	if (data->onMouseEvent)
		return data->onMouseEvent(data, gEvent_MouseMenu);
	else
		return false;
}

static gboolean sg_button_Release (GtkWidget *widget,GdkEventButton *event,gControl *data)
{	
	if (!gApplication::userEvents()) return false;

	if (data->onMouseEvent)
	{
		gMouse::validate();
		gMouse::setMouse((int)event->x, (int)event->y, 0, event->state);
		data->onMouseEvent(data,gEvent_MouseRelease);
		gMouse::invalidate();
	}
	
	return false;
}

static gboolean sg_scroll(GtkWidget *widget,GdkEventScroll *event,gControl *data)
{
	int dt = 0;
	int ort = 0;
	
	if (!gApplication::userEvents()) return false;

	if (data->onMouseEvent)
	{
		switch (event->direction)
		{
			case GDK_SCROLL_UP: dt=1; ort=1; break;
			case GDK_SCROLL_DOWN: dt=-1; ort=1; break;
			case GDK_SCROLL_LEFT: dt=-1; ort=0; break;
			case GDK_SCROLL_RIGHT:  dt=1; ort=0; break;
		}
		
		gMouse::validate();
		gMouse::setMouse((int)event->x, (int)event->y, 0, event->state);
		gMouse::setWheel(dt, ort);
		data->onMouseEvent(data,gEvent_MouseWheel);
		gMouse::invalidate();
	}
	
	return false;
}

static gboolean sg_focus_In(GtkWidget *widget,GdkEventFocus *event,gControl *data)
{	
	if (!gApplication::allEvents()) return false;

	gMainWindow::setActiveWindow(data);
	if (data->onFocusEvent) data->onFocusEvent(data,gEvent_FocusIn);
	return false;
}

static gboolean sg_focus_Out(GtkWidget *widget,GdkEventFocus *event,gControl *data)
{	
	if (!gApplication::allEvents()) return false;
	
	gMainWindow::setActiveWindow(NULL);
	if (data->onFocusEvent) data->onFocusEvent(data,gEvent_FocusOut);
	return false;
}

static gboolean sg_event(GtkWidget *widget, GdkEvent *event,gControl *data)
{	
	if (!gApplication::userEvents()) return false;
	
	if (event->type==GDK_2BUTTON_PRESS)
	{
		if (data->onMouseEvent) data->onMouseEvent(data,gEvent_MouseDblClick);
		return false;
	}
	
	return false;
}

/****************************************************
 Drag 
*****************************************************/

static void sg_drag_data_get(GtkWidget *widget,GdkDragContext *ct,GtkSelectionData *dt, guint i,guint t,gControl *data)
{
	//g_debug("sg_drag_data_get\n");
	
	if (gDrag::getDropText())
		gtk_selection_data_set_text(dt, gDrag::getDropText(), -1);
	else if (gDrag::getDropImage())
		gtk_selection_data_set_pixbuf(dt, gDrag::getDropImage()->getPixbuf());
}

static void sg_drag_end(GtkWidget *widget,GdkDragContext *ct,gControl *data)
{
	//g_debug("sg_drag_end\n");
	
	gDrag::cancel();
}


/****************************************************
 Drop 
*****************************************************/

// BM: What for?
//static guint32 _drag_time = 0;

static void sg_drag_data_received(GtkWidget *widget, GdkDragContext *context,
	gint x, gint y, GtkSelectionData *sel, guint info, guint time, gControl *data)
{
	if (!gApplication::allEvents()) return;
	
	//g_debug("sg_drag_data_received: length = %d\n", sel->length);
	
	// if (gDrag::getType()) // local DnD
	//	return;
	
	if (data->_drag_get_data)
	{
		//g_debug("sg_drag_data_received: OK\n");
	
/*		if (gtk_drag_get_source_widget(context) )
		{
			if (gControl::dragTextBuffer()) type=1;
			else type=2;
		}
		else
		{
			type=drag_Type();
		}*/
		
		if (gDrag::getType() == gDrag::Text)
		{
			//fprintf(stderr, "Text: %s\n", sel->data);
			if (sel->length != -1)
				gDrag::setDropText((char*)sel->data, sel->length);
			else
				gDrag::setDropText(NULL);
		}
		
		if (gDrag::getType() == gDrag::Image)
		{
			//fprintf(stderr, "Image\n");
			if (sel->length != -1)
				gDrag::setDropImage((char*)sel->data, sel->length);
			else
				gDrag::setDropImage(NULL);
		}
		
		// BM: ??
		// data->drops |= 8;
	}
}

static void set_from_context(GtkWidget *widget, GdkDragContext *context, guint time)
{
	GList *tg;
	gchar *format = NULL;
	
	if (gDrag::getType()) // local DnD
		return;
	
	//g_debug("set_from_context: non local\n");
	
	tg = g_list_first(context->targets);
	
	while (tg)
	{
		g_free(format);
		format = gdk_atom_name((GdkAtom)tg->data);
		//fprintf(stderr, "format = '%s'\n", format);
		
		if (strlen(format) >= 5 && !strncasecmp(format, "text/", 5)) 
		{ 
			//fprintf(stderr, "text/*: gtk_drag_get_data: %p\n", widget);
			gDrag::setDropInfo(gDrag::Text, format); 
			gtk_drag_get_data (widget, context, (GdkAtom)tg->data, time);
			break; 
		}
			
		if (strlen(format) >= 6 &&! strncasecmp(format, "image/", 6))
		{ 
			//fprintf(stderr, "image/*: gtk_drag_get_data: %p\n", widget);
			gDrag::setDropInfo(gDrag::Image, format); 
			gtk_drag_get_data (widget, context, (GdkAtom)tg->data, time);
			break; 
		}
		
		tg = g_list_next(tg);
	}
	
	g_free(format);
}


static gboolean sg_drag_motion(GtkWidget *widget,GdkDragContext *context,gint x,gint y,guint time,gControl *data)
{
	bool retval = true;
	int action;
	gControl *source;
	
	if (!gApplication::allEvents()) return true;
	
	//g_debug("sg_drag_motion\n");
	
	/*if (_drag_time != context->start_time) 
	{ 
		g_debug("sg_drag_motion: cancel!\n");
		gDrag::cancel();
		data->_drop_0 = true; 
		data->_drop_1 = false; 
		data->_drop_2 = false; 
	}*/
	
	if (!data->_drag_get_data)
	{
		//g_debug("sg_drag_motion: set_from_context\n");
	
		data->_drag_get_data = true;
		context->start_time = time;
		//_drag_time = time;
		
		set_from_context(widget, context, time);		
		return false;
	}
	
	switch (context->suggested_action)
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
	gDrag::setDropData(action, x, y, source);
	
	gDrag::enable();
	
	if (!data->_drag_enter)
	{
		//g_debug("sg_drag_motion: onDrag\n");
	
		x = 0; 
		y = 0; 
		
		if (data->onDrag) 
			retval = !data->onDrag(data);
		data->_drag_enter = true;
	}
	else
	{
		//g_debug("sg_drag_motion: onDragMove\n");
		
		if (data->onDragMove) 
			retval = !data->onDragMove(data);
	}
	
	gDrag::disable();
	
	if (retval) 
	{
		gdk_drag_status(context, context->suggested_action, time);
		return true;
	}
	
	gDrag::hide(data);
	return false;
}

void sg_drag_leave(GtkWidget *widget, GdkDragContext *context, guint time, gControl *data)
{
	data->_drag_enter = false;
	gDrag::hide(data);
}


gboolean sg_drag_drop(GtkWidget *widget,GdkDragContext *context,gint x,gint y,guint time,gControl *data)
{
	gControl *source;

	if (!gApplication::allEvents()) return true;
	
	//g_debug("sg_drag_drop\n");
	
	source = gApplication::controlItem(gtk_drag_get_source_widget(context));

	gDrag::setDropData(gDrag::getAction(), x, y, source);
	
	gDrag::enable();
	
	if (data->onDrop) 
	{
		//fprintf(stderr, "sg_drag_drop: getDropText: %s\n", gDrag::getDropText());
		data->onDrop(data);
	}
	
	gDrag::disable();
	gDrag::cancel();
	
	gtk_drag_finish (context, true, false, time);
	
	data->_drag_enter = false;
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

void gControl::widgetSignals()
{
	
	if (border != widget && !GTK_IS_SCROLLED_WINDOW(border))
	{
		g_signal_connect(G_OBJECT(border),"button-release-event",G_CALLBACK(sg_button_Release),(gpointer)this);
		g_signal_connect(G_OBJECT(border),"button-press-event",G_CALLBACK(sg_button_Press),(gpointer)this);
		g_signal_connect(G_OBJECT(border),"popup-menu",G_CALLBACK(sg_menu),(gpointer)this);	
		g_signal_connect_after(G_OBJECT(border),"motion-notify-event",G_CALLBACK(sg_motion),(gpointer)this);
		g_signal_connect(G_OBJECT(border),"scroll-event",G_CALLBACK(sg_scroll),(gpointer)this);
		g_signal_connect(G_OBJECT(border),"key-press-event",G_CALLBACK(gcb_keypress),(gpointer)this);
		g_signal_connect(G_OBJECT(border),"key-release-event",G_CALLBACK(gcb_keyrelease),(gpointer)this);
	}
	else
	{
		g_signal_connect(G_OBJECT(widget),"scroll-event",G_CALLBACK(sg_scroll),(gpointer)this);
		g_signal_connect(G_OBJECT(widget),"button-release-event",G_CALLBACK(sg_button_Release),(gpointer)this);
		g_signal_connect(G_OBJECT(widget),"button-press-event",G_CALLBACK(sg_button_Press),(gpointer)this);
		g_signal_connect(G_OBJECT(widget),"motion-notify-event",G_CALLBACK(sg_motion),(gpointer)this);
		g_signal_connect(G_OBJECT(widget),"popup-menu",G_CALLBACK(sg_menu),(gpointer)this);
		g_signal_connect(G_OBJECT(widget),"key-press-event",G_CALLBACK(gcb_keypress),(gpointer)this);
		g_signal_connect(G_OBJECT(widget),"key-release-event",G_CALLBACK(gcb_keyrelease),(gpointer)this);
	}	
	
	g_signal_connect(G_OBJECT(widget),"focus-in-event",G_CALLBACK(sg_focus_In),(gpointer)this);
	g_signal_connect(G_OBJECT(widget),"focus-out-event",G_CALLBACK(sg_focus_Out),(gpointer)this);
	g_signal_connect(G_OBJECT(widget),"event",G_CALLBACK(sg_event),(gpointer)this);
	
	if (isContainer())
		g_signal_connect(G_OBJECT(border), "show", G_CALLBACK(cb_show), (gpointer)this);
}

void gControl::initSignals()
{	
	g_signal_connect(G_OBJECT(border),"destroy",G_CALLBACK(sg_destroy),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"drag-data-received",G_CALLBACK(sg_drag_data_received),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"drag-motion",G_CALLBACK(sg_drag_motion),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"drag-leave",G_CALLBACK(sg_drag_leave),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"drag-drop",G_CALLBACK(sg_drag_drop),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"drag-data-get",G_CALLBACK(sg_drag_data_get),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"drag-end",G_CALLBACK(sg_drag_end),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"enter-notify-event",G_CALLBACK(sg_enter),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"leave-notify-event",G_CALLBACK(sg_enter),(gpointer)this);
	
	//g_signal_connect_after(G_OBJECT(border),"size-allocate",G_CALLBACK(sg_size),(gpointer)this);
	
	widgetSignals();
}
