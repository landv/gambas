/***************************************************************************

  gapplication.cpp

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#include <ctype.h>
#include <time.h>
#include <unistd.h>

#include "widgets.h"
#include "widgets_private.h"
#include "gapplication.h"
#include "gtrayicon.h"
#include "gdesktop.h"
#include "gkey.h"
#include "gmenu.h"
#include "gmessage.h"
#include "gdialog.h"
#include "gclipboard.h"
#include "gmouse.h"
#include "gmainwindow.h"

//#define DEBUG_IM 1

/*************************************************************************

gKey

**************************************************************************/

bool gKey::_valid = false;
bool gKey::_no_input_method = false;
GdkEventKey gKey::_event;
GtkIMContext *gKey::_im_context = NULL;
gControl *gKey::_im_control = NULL;
char *_im_text = NULL;

const char *gKey::text()
{
	if (!_valid) 
		return 0;
	else
		return _event.string;
}

int gKey::code()
{
	if (!_valid)
		return 0;
	
	int code = _event.keyval;
	
	if (code >= GDK_a && code <= GDK_z)
		code += GDK_A - GDK_a;
	
	return code;
}

int gKey::state()
{
	if (!_valid)
		return 0;
	else
		return _event.state;
}

bool gKey::alt()
{
	return state() & GDK_MOD1_MASK || code() == GDK_Alt_L || code() == GDK_Alt_R;
}

bool gKey::control()
{
	return state() & GDK_CONTROL_MASK || code() == GDK_Control_L || code() == GDK_Control_R;
}

bool gKey::meta()
{
	return state() & GDK_MOD2_MASK || code() == GDK_Meta_L || code() == GDK_Meta_R;
}

bool gKey::normal()
{
	return (state() & 0xFF) != 0;
}

bool gKey::shift()
{
	return state() & GDK_SHIFT_MASK || code() == GDK_Shift_L || code() == GDK_Shift_R;
}

int gKey::fromString(char *str)
{
	char *lstr;
	int key;
	
	lstr = g_ascii_strup(str, -1);
	key = gdk_keyval_from_name(lstr);
	g_free(lstr);
	if (key) return key;
	
	lstr = g_ascii_strdown(str, -1);
	key = gdk_keyval_from_name(lstr);
	g_free(lstr);
	if (key) return key;

	key = gdk_keyval_from_name(str);
	return key;
}

void gKey::disable()
{
	if (!_valid)
		return;
		
	_valid = false;
	_event.keyval = 0;
	_event.state = 0;
	g_free(_event.string);
}

bool gKey::enable(gControl *control, GdkEventKey *event)
{
	bool filter;
	
	//if (widget != _im_widget)
	//	return true;
	
	if (_valid)
		disable();
		
	_valid = true;
	_event = *event;
	
	if (_event.type == GDK_KEY_PRESS && !_no_input_method && control == _im_control)
	{
		#if DEBUG_IM
		fprintf(stderr, "gKey::enable: event->string = '%s'\n", event->string);
		#endif
		filter = gtk_im_context_filter_keypress(_im_context, &_event);
		#if DEBUG_IM
		fprintf(stderr, "gKey::enable: filter -> %d event->string = '%s'\n", filter, event->string);
		#endif
	}
	else
		filter = false;
  
  if (filter && _im_text)
  {
		_event.string = g_strdup(_im_text);
		//_event.keyval = 0;
		filter = false;
  }
  else
  	_event.string = g_strdup(_event.string);
  
  if (!filter)
  {
		//#if DEBUG_IM
  	//fprintf(stderr, "gKey::enable: gtk_im_context_reset\n");
		//#endif
  	//gtk_im_context_reset(_im_context);
  	if (_im_text)
  	{
  		g_free(_im_text);
  		_im_text = NULL;
  	}
  }
  
  //fprintf(stderr, "gKey::enable: --> %d\n", filter);
  return filter;
}

static void cb_im_commit(GtkIMContext *context, const gchar *str, gpointer pointer)
{
	#if DEBUG_IM
	fprintf(stderr, "cb_im_commit: %s\n", str);
	#endif
	
	if (_im_text)
		g_free(_im_text);
		
	_im_text = g_strdup(str);
}

void gKey::init()
{
	_im_context = gtk_im_multicontext_new();
  g_signal_connect (_im_context, "commit", G_CALLBACK(cb_im_commit), NULL);
}

void gKey::exit()
{
	disable();
	if (_im_text)
		g_free(_im_text);
	g_object_unref(_im_context);
}

void gKey::setActiveControl(gControl *control)
{
	if (_im_control)
	{
		if (!_no_input_method)
		{
			#if DEBUG_IM
			fprintf(stderr, "gtm_im_context_focus_out\n");
			#endif
	  	gtk_im_context_set_client_window (_im_context, 0);
			gtk_im_context_focus_out(_im_context);
		}
		_im_control = NULL;
		_no_input_method = false;
	}
	
	if (control)
	{
		_im_control = control;
		_no_input_method = control->noInputMethod();
		
		if (!_no_input_method)
		{
	  	gtk_im_context_set_client_window (_im_context, _im_control->widget->window);
			gtk_im_context_focus_in(_im_context);
			gtk_im_context_reset(_im_context);
			#if DEBUG_IM
			fprintf(stderr, "gtm_im_context_focus_in\n");
			#endif
		}		
	}
}

/**************************************************************************
	
	Global event handler
	
**************************************************************************/

static bool _focus_change = false;

static bool check_button(gControl *w)
{
	return w && w->isVisible() && w->enabled();
}

#if 0
static GtkWindowGroup *get_window_group(GtkWidget *widget)
{
  GtkWidget *toplevel = NULL;

  if (widget)
    toplevel = gtk_widget_get_toplevel(widget);

  if (GTK_IS_WINDOW (toplevel))
    return gtk_window_get_group(GTK_WINDOW (toplevel));
  else
    return gtk_window_get_group(NULL);
}
#endif

/*static gboolean close_dialog(GtkButton *button)
{
	gtk_button_clicked(button);
	return FALSE;
}*/

static bool raise_key_event_to_parent_window(gControl *control, int type)
{
	gMainWindow *win;
	
	while (control->parent())
	{
		win = control->parent()->window();
		if (win->onKeyEvent && win->canRaise(win, type))
		{
			if (win->onKeyEvent(win, type))
				return true;
		}
		
		control = win;
	}
	
	return false;
}

static void gambas_handle_event(GdkEvent *event)
{
  GtkWidget *widget;
  GtkWidget *grab;
	gControl *control;
	int x, y, xc, yc;
	bool real;
	bool cancel;
	
	if (gApplication::_close_next_window)
	{
		widget = gtk_get_event_widget(event);
		if (widget)
		{
			//fprintf(stderr, "type: %s\n", G_OBJECT_TYPE_NAME(widget));
			if (!strcmp(G_OBJECT_TYPE_NAME(gtk_widget_get_toplevel(widget)), "GtkPrintUnixDialog"))
			{
				if (event->type == GDK_WINDOW_STATE)
				{
					//fprintf(stderr, "event: MAP!\n");
					widget = gtk_window_get_default_widget(GTK_WINDOW(gtk_widget_get_toplevel(widget)));
					if (widget && GTK_IS_BUTTON(widget))
					{
						gApplication::_close_next_window = false;
						gtk_button_clicked(GTK_BUTTON(widget));
						return;
						goto __HANDLE_EVENT;
						//g_timeout_add(0, (GSourceFunc)close_dialog, GTK_BUTTON(widget));
					}
					//fprintf(stderr, "event: MAP! <<< end\n");
				}
			}
		}
	}

	if (!((event->type >= GDK_MOTION_NOTIFY && event->type <= GDK_FOCUS_CHANGE) || event->type == GDK_SCROLL))
		goto __HANDLE_EVENT;
	
	widget = gtk_get_event_widget(event);
	if (!widget)
		goto __HANDLE_EVENT;
	
	if (event->type == GDK_FOCUS_CHANGE)
	{
		if (GTK_IS_WINDOW(widget))
		{
			control = (gControl *)g_object_get_data(G_OBJECT(widget), "gambas-control");
			if (control)
				gApplication::setActiveControl(control, event->focus_change.in);
			else if (event->focus_change.in);
				gMainWindow::setActiveWindow(NULL);
		}
		goto __HANDLE_EVENT;
	}
	
	grab = gtk_grab_get_current();
	if (grab)
	{
		if (widget != grab && !gtk_widget_is_ancestor(widget, grab))
			widget = grab;
	}
	
	real = true;
	while (widget)
	{
		control = (gControl *)g_object_get_data(G_OBJECT(widget), "gambas-control");
		if (control)
			break;
		widget = widget->parent;
		real = false;
	}

	/*if (event->type == GDK_KEY_PRESS)
		fprintf(stderr, "KEY_PRESS: %p %s\n", control, control ? control->name() : 0);
	else if (event->type == GDK_BUTTON_PRESS)
		fprintf(stderr, "BUTTON_PRESS: %p %s\n", control, control ? control->name() : 0);*/
	
	/*if (event->type == GDK_BUTTON_PRESS)
		fprintf(stderr, "GDK_BUTTON_PRESS: %p %s\n", widget, control ? control->name() : NULL);
	else if (event->type == GDK_KEY_PRESS)
		fprintf(stderr, "GDK_KEY_PRESS: %p %s\n", widget, control ? control->name() : NULL);*/
	
	if (!widget || !control)
		goto __HANDLE_EVENT;
	
	/*switch ((int)event->type)
	{
		case GDK_ENTER_NOTIFY:
			fprintf(stderr, "ENTER: %p %s\n", control, control ? control->name() : 0);
			break;
		
		case GDK_LEAVE_NOTIFY:
			fprintf(stderr, "LEAVE: %p %s\n", control, control ? control->name() : 0);
			break;
	}*/
		
	//group = get_window_group(widget);
	//if (group != gApplication::currentGroup())
	//	goto __HANDLE_EVENT;
	
	if (event->type != GDK_ENTER_NOTIFY)
	{
		if (gApplication::_leave)
		{
			if (event->type != GDK_LEAVE_NOTIFY || gApplication::_leave != control)
				gApplication::_leave->emit(SIGNAL(gApplication::_leave->onEnterLeave), gEvent_Leave);
			gApplication::_leave = NULL;
		}
	}
	
	cancel = false;
	
	gApplication::updateLastEventTime(event);
	
	switch ((int)event->type)
	{
		case GDK_ENTER_NOTIFY:
			
			//gApplication::dispatchEnterLeave(control);
			if (gApplication::_leave == control)
				gApplication::_leave = NULL;
			else if (gApplication::_enter != control)
			{
				gApplication::_enter = control;
				control->emit(SIGNAL(control->onEnterLeave), gEvent_Enter);
			}
			break;
		
		case GDK_LEAVE_NOTIFY:
			if (gdk_events_pending())
				gApplication::_leave = control;
			else
			{
				if (gApplication::_enter == control)
					gApplication::_enter = NULL;
				control->emit(SIGNAL(control->onEnterLeave), gEvent_Leave);
			}
			break;
			
		case GDK_BUTTON_PRESS:
		case GDK_2BUTTON_PRESS:
		case GDK_BUTTON_RELEASE:
			
			if (control->_proxy_for)
				control = control->_proxy_for;
		
		__BUTTON_TRY_PROXY:
		
			if (control->onMouseEvent)
			{
				control->getScreenPos(&xc, &yc);
				x = (int)event->button.x_root - xc;
				y = (int)event->button.y_root - yc;
				
				gMouse::validate();
				gMouse::setStart(x, y);
				gMouse::setMouse(x, y, event->button.button, event->button.state);
				//gMouse::setValid(1,(int)event->x,(int)event->y,event->button,event->state,data->screenX(),data->screenY());
				switch ((int)event->type)
				{
					case GDK_BUTTON_PRESS: 
						control->onMouseEvent(control, gEvent_MousePress);
						if (control->isTopLevel())
						{
							gMainWindow *win = ((gMainWindow *)control);
							if (win->isPopup() && (x < 0 || y < 0 || x >= win->width() || y >= win->height()))
								win->close();
						}
						
						break;
					
					case GDK_2BUTTON_PRESS: 
						control->onMouseEvent(control, gEvent_MouseDblClick); 
						break;
					
					case GDK_BUTTON_RELEASE: 
						control->onMouseEvent(control, gEvent_MouseRelease); 
						if (control->_grab)
							gApplication::exitLoop(control);
						break;
				}
				
				gMouse::invalidate();
				
				if (event->button.button == 3 && event->type == GDK_BUTTON_PRESS)
					control->onMouseEvent(control, gEvent_MouseMenu);
			}
			
			if (control->_proxy)
			{
				control = control->_proxy;
				goto __BUTTON_TRY_PROXY;
			}
			
			break;
			
		case GDK_MOTION_NOTIFY:

			if (control->_proxy_for)
				control = control->_proxy_for;
		
		__MOTION_TRY_PROXY:
		
			if (control->onMouseEvent && (control->isTracking() || (event->motion.state & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK))))
			{
				control->getScreenPos(&xc, &yc);
				x = (int)event->motion.x_root - xc;
				y = (int)event->motion.y_root - yc;
				
				gMouse::validate();
				gMouse::setMouse(x, y, 0, event->motion.state);
				control->onMouseEvent(control, gEvent_MouseMove);
				//if (data->acceptDrops() && gDrag::checkThreshold(data, gMouse::x(), gMouse::y(), gMouse::startX(), gMouse::startY()))
				if ((event->motion.state & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK)) 
						//&& (abs(gMouse::x() - gMouse::y()) + abs(gMouse::startX() - gMouse::startY())) > 8)
						&& gDrag::checkThreshold(control, gMouse::x(), gMouse::y(), gMouse::startX(), gMouse::startY()))
				{
					control->onMouseEvent(control, gEvent_MouseDrag);
				}
				gMouse::invalidate();
			}

			if (control->_proxy)
			{
				control = control->_proxy;
				goto __MOTION_TRY_PROXY;
			}
			
			break;
			
		case GDK_SCROLL:
			
			if (control->_proxy_for)
				control = control->_proxy_for;
		
		__SCROLL_TRY_PROXY:
		
			if (control->onMouseEvent)
			{
				int dt, ort;
				
				control->getScreenPos(&xc, &yc);
				x = (int)event->scroll.x_root - xc;
				y = (int)event->scroll.y_root - yc;

				switch (event->scroll.direction)
				{
					case GDK_SCROLL_DOWN: dt = -1; ort = 1; break;
					case GDK_SCROLL_LEFT: dt = -1; ort = 0; break;
					case GDK_SCROLL_RIGHT:  dt = 1; ort = 0; break;
					case GDK_SCROLL_UP: default: dt = 1; ort = 1; break;
				}
				
				gMouse::validate();
				gMouse::setMouse(x, y, 0, event->scroll.state);
				gMouse::setWheel(dt, ort);
				control->onMouseEvent(control, gEvent_MouseWheel);
				gMouse::invalidate();
			}

			if (control->_proxy)
			{
				control = control->_proxy;
				goto __SCROLL_TRY_PROXY;
			}
			
			break;

		case GDK_KEY_PRESS:
		{
			gMainWindow *win;
			
			if (gApplication::activeControl())
				control = gApplication::activeControl();
			
			if (control)
			{
				win = control->window();
				
				if (!gKey::enable(control, &event->key))
				{
					if (gApplication::onKeyEvent)
						cancel = gApplication::onKeyEvent(gEvent_KeyPress);
					if (!cancel && control->_proxy_for && control->_proxy_for->onKeyEvent)
					{
						cancel = control->_proxy_for->onKeyEvent(control->_proxy_for, gEvent_KeyPress);
					}
					if (!cancel && control->onKeyEvent) 
					{
						//fprintf(stderr, "gEvent_KeyPress on %p %s\n", control, control->name());
						cancel = control->onKeyEvent(control, gEvent_KeyPress);
					}
					if (!cancel)
						cancel = raise_key_event_to_parent_window(control, gEvent_KeyPress);
				}
				gKey::disable();
				
				if (cancel)
					return;
				
				if (event->key.keyval == GDK_Escape)
				{
					if (control->_grab)
					{
						gApplication::exitLoop(control);
						return;
					}
					
					if (check_button(win->_cancel))
					{
						win->_cancel->animateClick(false);
						return;
					}
				}
				else if (event->key.keyval == GDK_Return || event->key.keyval == GDK_KP_Enter)
				{
					if (check_button(win->_default))
					{
						win->_default->animateClick(false);
						return;
					}
				}
			}

			break;
		}
			
		case GDK_KEY_RELEASE:
		{
			bool cancel = false;
			gMainWindow *win;
			
			if (gApplication::activeControl())
				control = gApplication::activeControl();
			
			if (control)
			{
				win =  control->window();
				
				if (!gKey::enable(control, &event->key))
				{
					if (control->_proxy_for && control->_proxy_for->onKeyEvent)
					{
						cancel = control->_proxy_for->onKeyEvent(control->_proxy_for, gEvent_KeyRelease);
					}
					if (!cancel)
						control->emit(SIGNAL(control->onKeyEvent), gEvent_KeyRelease);
					if (!cancel)
						cancel = raise_key_event_to_parent_window(control, gEvent_KeyRelease);
				}
				gKey::disable();
				
				if (cancel)
					return;
				
				if (event->key.keyval == GDK_Escape)
				{
					gMainWindow *win = control->window();
					if (check_button(win->_cancel))
						win->_cancel->animateClick(true);
				}
				else if (event->key.keyval == GDK_Return || event->key.keyval == GDK_KP_Enter)
				{
					if (check_button(win->_default))
						win->_default->animateClick(true);
				}
			}
			
			break;
		}
	}
	
__HANDLE_EVENT:

	gtk_main_do_event(event);
}



/**************************************************************************

gApplication

**************************************************************************/

int appEvents;
GtkTooltips *app_tooltips_handle;
bool app_tooltips=true;
gFont *app_tooltips_font=NULL;

bool gApplication::_busy = false;
char *gApplication::_title = NULL;
int gApplication::_loopLevel = 0;
int gApplication::_tooltip_delay = 500; // GTK+ default
void *gApplication::_loop_owner = 0;
GtkWindowGroup *gApplication::_group = NULL;
gControl *gApplication::_enter = NULL;
gControl *gApplication::_leave = NULL;
gControl *gApplication::_active_control = NULL;
gControl *gApplication::_old_active_control = NULL;
bool (*gApplication::onKeyEvent)(int) = NULL;
guint32 gApplication::_event_time = 0;
bool gApplication::_close_next_window = false;

GtkTooltips* gApplication::tipHandle()
{
	return app_tooltips_handle;
}

bool gApplication::toolTips()
{
	return app_tooltips;
}

void gApplication::setToolTipsFont(gFont *ft)
{
	GList *chd;
	PangoFontDescription *desc;
	
	gFont::set(&app_tooltips_font, ft->copy());
	
	if (ft)
    desc = pango_context_get_font_description(ft->ct);
  else
    desc = NULL;
	
	gtk_widget_modify_font(app_tooltips_handle->tip_window,desc);
	
	chd=gtk_container_get_children(GTK_CONTAINER(app_tooltips_handle->tip_window));
	if (chd)
	{
		do { gtk_widget_modify_font(GTK_WIDGET(chd->data),desc);
		} while (chd->next);
		g_list_free(chd);
	}
}

gFont *gApplication::toolTipsFont()
{
	return app_tooltips_font;
}

void gApplication::enableTooltips(bool vl)
{
	if (vl)
		gtk_tooltips_enable(app_tooltips_handle);
	else
		gtk_tooltips_disable(app_tooltips_handle);
}

void gApplication::setToolTipsDelay(int vl)
{
	if (vl < 50)
		vl = 50;
	else if (vl > 10000)
		vl = 10000;
	
	_tooltip_delay = vl;
	gtk_tooltips_set_delay(app_tooltips_handle, vl);
}

void gApplication::suspendEvents(bool vl)
{
	if (!vl) appEvents=3; //all
	else appEvents=1;     //user
}

void gApplication::enableEvents()
{
	appEvents=0;
}

bool gApplication::userEvents()
{
	if (appEvents) return false;
	return true;
}

bool gApplication::allEvents()
{
	if (appEvents & 2) return false;
	return true;
}

void gApplication::init(int *argc,char ***argv)
{
	appEvents=0;
	
	gtk_init(argc,argv);
 
	gdk_event_handler_set((GdkEventFunc)gambas_handle_event, NULL, NULL);
	
	app_tooltips_handle = gtk_tooltips_new();
	g_object_ref(G_OBJECT(app_tooltips_handle));
	gtk_tooltips_force_window (app_tooltips_handle);
	app_tooltips_font = new gFont(app_tooltips_handle->tip_window);
	
	gClipboard::init();
	gKey::init();
	_loop_owner = 0;
}

void gApplication::exit()
{
	if (_title)
		g_free(_title);

	gKey::exit();
	gTrayIcon::exit();
  gDesktop::exit();
  gMessage::exit();
  gDialog::exit();
  gFont::assign(&app_tooltips_font);
  gFont::exit();
  gt_exit();
}

int gApplication::controlCount()
{
	GList *iter;
	int ct=1;
	
	if (!gControl::controlList()) return 0;
	
	iter=g_list_first(gControl::controlList());
	while (iter->next)
	{
		ct++;
		iter=iter->next;
	}
	
	return ct;
}

gControl* gApplication::controlItem(GtkWidget *wid)
{
	GList *iter;
	
	if (!wid) return NULL;

	if (gControl::controlList())
	{
		iter=g_list_first(gControl::controlList());
		
		while (iter)
		{
			if (((gControl*)iter->data)->border == wid )
				return (gControl*)iter->data;
				
			if (((gControl*)iter->data)->widget == wid )
				return (gControl*)iter->data;
		
			iter=iter->next;
		}
		
	}
	
	return NULL;
}

gControl* gApplication::controlItem(int index)
{
	GList *iter;
	
	if (!gControl::controlList()) return NULL;
	iter=g_list_nth(gControl::controlList(),index);
	if (!iter) return NULL;
	return (gControl*)iter->data;
}

void gApplication::setBusy(bool b)
{
	GList *iter;
	gControl *control;

  if (b == _busy)
    return;

  _busy = b;
  
  iter = g_list_first(gControl::controlList());
  
  while (iter)
  {
    control = (gControl *)iter->data;
    
    if (control->mustUpdateCursor())
    	control->setMouse(control->mouse());
    
    iter = g_list_next(iter);
  }
  
  do_iteration(true, true);
}

static bool _dirty = false;

static gboolean update_geometry(void *data)
{
	GList *iter;
	gControl *control;
	
	if (gContainer::_arrangement_level)
		return true;
	
	_dirty = false;
	//g_debug(">>>> update_geometry");
	iter = g_list_first(gControl::controlList());
	while (iter)
	{
		control = (gControl *)iter->data;
		control->updateGeometry();
		iter = iter->next;
	}
	//g_debug("<<<<");
	
	return false;
}

void gApplication::setDirty()
{
	if (_dirty)
		return;
		
	_dirty = true;
	g_timeout_add(0, (GSourceFunc)update_geometry, NULL);
}

void gApplication::setDefaultTitle(const char *title)
{
	if (_title)
		g_free(_title);
	_title = g_strdup(title);
}

GtkWindowGroup *gApplication::enterGroup()
{
	gControl *control = _enter;
	GtkWindowGroup *oldGroup = _group;
	_group = gtk_window_group_new();
	
	_enter = _leave = NULL;
		
	while (control)
	{
		control->emit(SIGNAL(control->onEnterLeave), gEvent_Leave);
		control = control->parent();
	}
	
	return oldGroup;
}

void gApplication::exitGroup(GtkWindowGroup *oldGroup)
{
	g_object_unref(_group);
	_group = oldGroup;	
}

void gApplication::enterLoop(void *owner, bool showIt)
{
	void *old_owner = _loop_owner;
	int l = _loopLevel;
	GtkWindowGroup *oldGroup;
	
	oldGroup = enterGroup();
	
	if (showIt) ((gControl *)owner)->show();

	_loopLevel++;
	_loop_owner = owner;
	
	do
	{
		do_iteration(false);
	}
	while (_loopLevel > l);
	
	_loop_owner = old_owner;

	exitGroup(oldGroup);
}

static gboolean
popup_grab_on_window (GdkWindow *window,
		      guint32    activate_time,
		      gboolean   grab_keyboard)
{
  /*gdk_display_pointer_ungrab (gdk_drawable_get_display (window), gApplication::lastEventTime());
  gdk_display_keyboard_ungrab(gdk_drawable_get_display (window), gApplication::lastEventTime());
	gdk_display_flush(gdk_drawable_get_display (window));*/
	
  int ret = gdk_pointer_grab(window, TRUE,
			 (GdkEventMask)(GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
			 GDK_POINTER_MOTION_MASK),
			 NULL, NULL, activate_time);
	
	if (ret == 0)
	{
		if (grab_keyboard)
			ret = gdk_keyboard_grab(window, TRUE, activate_time);
		
		if (ret == 0)
			return TRUE;
		
		gdk_display_pointer_ungrab (gdk_drawable_get_display (window), activate_time);
	}

	fprintf(stderr, "grab failed: %d\n", ret);
	return FALSE;
}


void gApplication::enterPopup(gMainWindow *owner)
{
	void *old_owner = _loop_owner;
	int l = _loopLevel;
	GtkWindowGroup *oldGroup;
	GtkWindow *window = GTK_WINDOW(owner->border);
	
	oldGroup = enterGroup();
	
	gtk_window_set_modal(window, true);
	owner->show();
	
  if (popup_grab_on_window(owner->border->window, GDK_CURRENT_TIME, FALSE))
	{
		_loopLevel++;
		_loop_owner = owner;
		
		do
		{
			do_iteration(false);
		}
		while (_loopLevel > l);
		
		//active = gDesktop::activeControl();
		//if (active)
		//	gcb_focus_out(active->widget, NULL, active);
		
		_loop_owner = old_owner;
	}

	//gtk_window_set_type_hint(window, GDK_WINDOW_TYPE_HINT_NORMAL);
	gtk_window_set_modal(window, false);
	exitGroup(oldGroup);
}

void gApplication::exitLoop(void *owner)
{
	if (!hasLoop(owner))
		return;
	
	if (_loopLevel > 0)
		_loopLevel--;
}

GtkWindowGroup *gApplication::currentGroup()
{
	if (_group)
		return _group;
	else
		return gtk_window_get_group(NULL);
}

void gApplication::updateLastEventTime(GdkEvent *e)
{
	guint32 time;
	
	switch (e->type)
	{
		case GDK_KEY_PRESS: case GDK_KEY_RELEASE: 
			time = e->key.time; break;
		case GDK_BUTTON_PRESS: case GDK_2BUTTON_PRESS: case GDK_3BUTTON_PRESS: case GDK_BUTTON_RELEASE:
			time = e->button.time; break;
		case GDK_MOTION_NOTIFY:
			time = e->motion.time; break;
		case GDK_SCROLL:
			time = e->scroll.time; break;
		case GDK_ENTER_NOTIFY: case GDK_LEAVE_NOTIFY:
			time = e->crossing.time; break;
		default:
			time = GDK_CURRENT_TIME;
	}
	
	_event_time = time;
}

static void post_focus_change(void *)
{
	gControl *current;
	
	//fprintf(stderr, "post_focus_change\n");
	
	for(;;)
	{
		current = gApplication::activeControl();
		if (current == gApplication::_old_active_control)
			break;

		if (gApplication::_old_active_control && gApplication::_old_active_control->onFocusEvent)
			gApplication::_old_active_control->onFocusEvent(gApplication::_old_active_control, gEvent_FocusOut);
		
		gApplication::_old_active_control = current;
		gMainWindow::setActiveWindow(current);
		
		if (current && current->onFocusEvent)
			current->onFocusEvent(current, gEvent_FocusIn);
	}
	
	_focus_change = FALSE;
}

static void handle_focus_change()
{
	if (_focus_change)
		return;
	
	_focus_change = TRUE;
	GB.Post((void (*)())post_focus_change, NULL);
}

void gApplication::setActiveControl(gControl *control, bool on)
{
	if (on == (_active_control == control))
		return;
	
	//fprintf(stderr, "setActiveControl: %s %d\n", control->name(), on);

	_active_control = on ? control : NULL;
	gKey::setActiveControl(_active_control);
	handle_focus_change();
}

