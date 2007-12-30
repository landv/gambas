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

guint32 drag_time=0;

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

static gboolean sg_motion(GtkWidget *widget,GdkEventMotion *event,gControl *data)
{
	long bt=0;
	bool cont=false;
	
	if (!gApplication::userEvents()) return false;
	
	if ((data->getClass() & 0x1000) ) { cont=true; }
	else if ( (event->state & 0x1F00) ) { cont=true; } 
	else if ( data->getClass()==Type_gDrawingArea )
	{
		if ( ((gDrawingArea*)data)->track ) cont=true;	
	}

	if (cont && data->onMouseEvent)
	{
		gMouse::setValid(1,(long)event->x,(long)event->y,bt,event->state,data->screenX(),data->screenY());
		data->onMouseEvent(data,gEvent_MouseMove);
		gMouse::setValid(0,0,0,0,0,0,0);
	}
	return FALSE;
}

static gboolean sg_button_Press (GtkWidget *widget,GdkEventButton *event,gControl *data)
{
	if (!gApplication::userEvents()) return false;

	if (data->onMouseEvent)
	{
		gMouse::setValid(1,(long)event->x,(long)event->y,event->button,event->state,data->screenX(),data->screenY());
		data->onMouseEvent(data,gEvent_MousePress);
		gMouse::setValid(0,0,0,0,0,0,0);
		
		if (event->button == 3 && event->type == GDK_BUTTON_PRESS)
		{
			return data->onMouseEvent(data, gEvent_MouseMenu);
		}
	}
	return false;
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
		gMouse::setValid(2,(long)event->x,(long)event->y,event->button,event->state,data->screenX(),data->screenY());
		data->onMouseEvent(data,gEvent_MouseRelease);
		gMouse::setValid(0,0,0,0,0,0,0);
	}
	
	return false;
}

static gboolean sg_scroll(GtkWidget *widget,GdkEventScroll *event,gControl *data)
{
	long bt=0;
	long dt=0;
	long ort=0;
	
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
		
		gMouse::setValid(1,(long)event->x,(long)event->y,bt,event->state,data->screenX(),data->screenY());
		gMouse::setWheel(dt,ort);
		data->onMouseEvent(data,gEvent_MouseWheel);
		gMouse::setValid(0,0,0,0,0,0,0);
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
void sg_drag_get(GtkWidget *widget,GdkDragContext *ct,GtkSelectionData *dt, \
                 guint i,guint t,gControl *data)
{

	if (gControl::dragTextBuffer())
	{
		gtk_selection_data_set_text(dt,gControl::dragTextBuffer(),-1);
		return;
	}
    
		
	if (gControl::dragPictureBuffer())
	{
		gtk_selection_data_set_pixbuf(dt,gControl::dragPictureBuffer());
		return;
	}
}

void sg_drag_end(GtkWidget *widget,GdkDragContext *ct,gControl *data)
{

	gControl::freeDragBuffer();
}
				 

/****************************************************
 Drop
 
*****************************************************/
void sg_drag_rec(GtkWidget *widget,GdkDragContext *context,gint x,gint y, \
                 GtkSelectionData *sel,guint info,guint time,gControl *data)
{
	long type;

	if (!gApplication::allEvents()) return;
	
	

	if (data->drops & 4)
	{
		if ( gtk_drag_get_source_widget(context) )
		{
			if (gControl::dragTextBuffer()) type=1;
			else type=2;
		}
		else
		{
			type=drag_Type();
		}
		
		if (type==1)
		{
			if (sel->length != -1)
				gDrag_setText((char*)sel->data);
			else
				gDrag_setText(NULL);
		}
		
		if (type==2)
		{
			if (sel->length != -1)
				gDrag_setImage((char*)sel->data,sel->length);
			else
				gDrag_setImage(NULL,0);
		}
		
		data->drops |= 8;
	}
}

gboolean sg_drag(GtkWidget *widget,GdkDragContext *context,gint x,gint y,guint time,gControl *data)
{

	GList *tg;
	gchar *buf;
	bool retval=true;
	

	if (!gApplication::allEvents()) return true;
	
	if (drag_time != context->start_time) { gDrag_Clear(); data->drops=1; }
	
	if (!(data->drops & 4))
	{
		data->drops |= 4;
		context->start_time=time;
		drag_time=time;
		gDrag_setTarget(0,NULL);
		tg=g_list_first(context->targets);
		while (tg)
		{
			buf=gdk_atom_name((GdkAtom)tg->data);
			if (strlen(buf)>=4)
				if ( (!strncmp(buf,"text",4)) || (!strncmp(buf,"TEXT",4)) ) 
				{ 
					gtk_drag_get_data (widget,context,(GdkAtom)tg->data,time);
					g_free(buf);
					tg=g_list_first(tg);
					buf=gdk_atom_name((GdkAtom)tg->data);
					gDrag_setTarget(1,buf); 
					g_free(buf); 
					break; 
				}
				
			if (strlen(buf)>=5)
				if (!strncmp(buf,"image",5)) 
				{ 
					gtk_drag_get_data (widget,context,(GdkAtom)tg->data,time);
					g_free(buf);
					tg=g_list_first(tg);
					buf=gdk_atom_name((GdkAtom)tg->data);
					gDrag_setTarget(2,buf); 
					g_free(buf); 
					break; 
				}
			
			g_free(buf);
			tg=g_list_next(tg);
		}
		return false;
	}
	

	if (!(data->drops & 2)) { x=0; y=0; }

	switch (context->suggested_action)
	{
		case GDK_ACTION_COPY: gDrag_Enable(x,y,0); break;
  		case GDK_ACTION_MOVE: gDrag_Enable(x,y,1); break;
  		case GDK_ACTION_LINK: gDrag_Enable(x,y,2); break;
		default: gDrag_Enable(x,y,0); break;
	}
	
	GtkWidget *src=gtk_drag_get_source_widget (context);
	gControl::setDragWidget(gApplication::controlItem(src));

	if (!(data->drops & 2))
	{
		if (data->onDrag) data->onDrag(data);
		data->drops |= 2;
	}
	else
		if (data->onDragMove) retval=!data->onDragMove(data);
	
	gDrag_Disable();
	if (retval) {
		gdk_drag_status(context,context->suggested_action,time);
		return true;
	}
	
	return false;
}


gboolean sg_drop(GtkWidget *widget,GdkDragContext *context,gint x,gint y,guint time,gControl *data)
{

	GtkWidget *src;

	if (!gApplication::allEvents()) return true;
	gDrag_Enable(x,y,drag_Action());
	src=gtk_drag_get_source_widget (context);
	gControl::setDragWidget(gApplication::controlItem(src));
	if (data->onDrop) data->onDrop(data);
	gDrag_Disable();
	gDrag_Clear();
	gtk_drag_finish (context,true,false,time);
	data->drops=1;
	drag_time=0;
	return true;
}

void sg_size(GtkWidget *widget,GtkRequisition *req, gContainer *data)
{
	if (data->parent()) data->parent()->performArrange();
	if (data->isContainer()) data->performArrange();
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
		g_signal_connect_after(G_OBJECT(widget),"motion-notify-event",G_CALLBACK(sg_motion),(gpointer)this);
		g_signal_connect(G_OBJECT(widget),"popup-menu",G_CALLBACK(sg_menu),(gpointer)this);
		g_signal_connect(G_OBJECT(widget),"key-press-event",G_CALLBACK(gcb_keypress),(gpointer)this);
		g_signal_connect(G_OBJECT(widget),"key-release-event",G_CALLBACK(gcb_keyrelease),(gpointer)this);
	}	
	
	g_signal_connect(G_OBJECT(widget),"focus-in-event",G_CALLBACK(sg_focus_In),(gpointer)this);
	g_signal_connect(G_OBJECT(widget),"focus-out-event",G_CALLBACK(sg_focus_Out),(gpointer)this);
	g_signal_connect(G_OBJECT(widget),"event",G_CALLBACK(sg_event),(gpointer)this);
}

void gControl::initSignals()
{	
	g_signal_connect(G_OBJECT(border),"destroy",G_CALLBACK(sg_destroy),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"drag-data-received",G_CALLBACK(sg_drag_rec),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"drag-motion",G_CALLBACK(sg_drag),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"drag-drop",G_CALLBACK(sg_drop),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"drag-data-get",G_CALLBACK(sg_drag_get),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"drag-end",G_CALLBACK(sg_drag_end),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"enter-notify-event",G_CALLBACK(sg_enter),(gpointer)this);
	g_signal_connect(G_OBJECT(border),"leave-notify-event",G_CALLBACK(sg_enter),(gpointer)this);
	
	g_signal_connect_after(G_OBJECT(border),"size-allocate",G_CALLBACK(sg_size),(gpointer)this);
	
	widgetSignals();
}
