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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#include <ctype.h>
#include <time.h>
#include <unistd.h>

#include "widgets.h"
#include "x11.h"
#include "gapplication.h"
#include "gtrayicon.h"
#include "gdesktop.h"
#include "gkey.h"
#include "gmenu.h"
#include "gmessage.h"
#include "gdialog.h"
#include "gclipboard.h"
#include "gmouse.h"
#include "gprinter.h"
#include "sm/sm.h"
#include "gmainwindow.h"

//#define DEBUG_ENTER_LEAVE 1

static bool _debug_keypress = false;

/**************************************************************************
	
	Global event handler
	
**************************************************************************/

static bool _focus_change = false;

static bool check_button(gControl *w)
{
	return w && w->isVisible() && w->isEnabled();
}

static GtkWindowGroup *get_window_group(GtkWidget *widget)
{
  GtkWidget *toplevel = NULL;

  if (widget)
    toplevel = gtk_widget_get_toplevel(widget);

  if (GTK_IS_WINDOW(toplevel))
    return gtk_window_get_group(GTK_WINDOW(toplevel));
  else
    return gtk_window_get_group(NULL);
}

/*static gboolean close_dialog(GtkButton *button)
{
	gtk_button_clicked(button);
	return FALSE;
}*/

/*static bool raise_key_event_to_parent_window(gControl *control, int type)
{
	gMainWindow *win;
	
	while (control->parent())
	{
		win = control->parent()->window();
		if (win->onKeyEvent && win->canRaise(win, type))
		{
			//fprintf(stderr, "onKeyEvent: %d %p %s\n", type, win, win->name());
			if (win->onKeyEvent(win, type))
				return true;
		}
		
		control = win;
	}
	
	return false;
}*/

static bool check_crossing_event(GdkEvent *event)
{
	#if DEBUG_ENTER_LEAVE
	fprintf(stderr, "check_crossing_event: %d / %d\n", event->crossing.detail, event->crossing.mode);
	#endif
	
	if (event->crossing.detail != GDK_NOTIFY_INFERIOR 
	    && (event->crossing.mode == GDK_CROSSING_NORMAL || event->crossing.mode == GDK_CROSSING_STATE_CHANGED))
		// || event->crossing.mode == GDK_CROSSING_UNGRAB || event->crossing.mode == GDK_CROSSING_GTK_UNGRAB))
		return true;
	else
		return false;
}

static gControl *find_child(gControl *control, int rx, int ry, gControl *button_grab = NULL)
{
	gContainer *cont;
	gControl *child;
	int x, y;
	
	if (gApplication::_control_grab)
		return gApplication::_control_grab;

	/*grab = gtk_grab_get_current();
	if (grab)
	{
		child = gt_get_control(grab);
		if (child)
			return child;
	}*/

	if (button_grab)
		return button_grab;

	//fprintf(stderr, "find_child: %s\n", control->name());

	control = control->topLevel();
	
	while (control->isContainer())
	{
		control->getScreenPos(&x, &y);
		cont = (gContainer *)control;
		child = cont->find(rx - x, ry - y);
		if (!child)
			break;
		control = child;
	}

	//fprintf(stderr, "-> %s\n", control->name());
	
	return control;
}

void gApplication::checkHoveredControl(gControl *control)
{
	if (gApplication::_enter != control)
	{
		#if DEBUG_ENTER_LEAVE
		fprintf(stderr, "checkHoveredControl: %s\n", control->name());
		#endif
	
		gControl *leave = gApplication::_enter;
		
		while (leave && leave != control && !leave->isAncestorOf(control))
		{
			#if DEBUG_ENTER_LEAVE
			fprintf(stderr, "checkHoveredControl: leave: %s\n", leave->name());
			#endif
			leave->emitLeaveEvent();
			leave = leave->parent();
		}
		
		#if DEBUG_ENTER_LEAVE
		fprintf(stderr, "checkHoveredControl: _enter <- %s\n", control ? control->name() : "ø");
		#endif
		gApplication::_enter = control;
		
		if (control)
		{
			#if DEBUG_ENTER_LEAVE
			fprintf(stderr, "checkHoveredControl: enter: %s\n", control->name());
			#endif
			control->emitEnterEvent();
		}
	}
}

static void gambas_handle_event(GdkEvent *event)
{
  GtkWidget *widget;
	GtkWidget *current_grab;
  GtkWidget *grab;
	gControl *control, *save_control;
	gControl *button_grab;
	int x, y, xs, ys, xc, yc;
	bool cancel;
	int type;

	if (gApplication::_fix_printer_dialog)
	{
		widget = gtk_get_event_widget(event);
		if (widget)
		{
			//fprintf(stderr, "type: %s\n", G_OBJECT_TYPE_NAME(widget));
			if (!strcmp(G_OBJECT_TYPE_NAME(gtk_widget_get_toplevel(widget)), "GtkPrintUnixDialog"))
			{
				if (event->type == GDK_WINDOW_STATE)
				{
					//fprintf(stderr, "event: GDK_WINDOW_STATE!\n");
					widget = gtk_window_get_default_widget(GTK_WINDOW(gtk_widget_get_toplevel(widget)));
					if (widget && GTK_IS_BUTTON(widget))
					{
						GtkPrintUnixDialog *dialog = GTK_PRINT_UNIX_DIALOG(gtk_widget_get_toplevel(widget));
						gPrinter::fixPrintDialog(dialog);
						gApplication::_fix_printer_dialog = false;
						//fprintf(stderr, "gtk_button_clicked: %s\n", gtk_button_get_label(GTK_BUTTON(widget)));
						if (gApplication::_close_next_window)
							gtk_button_clicked(GTK_BUTTON(widget));
						gApplication::_close_next_window = false;
						//return;
						//g_timeout_add(0, (GSourceFunc)close_dialog, GTK_BUTTON(widget));
						goto __HANDLE_EVENT;
					}
					//fprintf(stderr, "event: MAP! <<< end\n");
				}
			}
		}
	}
	
	/*if (event->type == GDK_GRAB_BROKEN)
	{
		if (gApplication::_in_popup)
			fprintf(stderr, "**** GDK_GRAB_BROKEN inside popup: %s %swindow = %p grab_window = %p popup_window = %p\n", event->grab_broken.keyboard ? "keyboard" : "pointer",
							event->grab_broken.implicit ? "implicit " : "", event->grab_broken.window, event->grab_broken.grab_window, gApplication::_popup_grab_window);
	}*/
	
	if (!((event->type >= GDK_MOTION_NOTIFY && event->type <= GDK_FOCUS_CHANGE) || event->type == GDK_SCROLL))
		goto __HANDLE_EVENT;

	widget = gtk_get_event_widget(event);
	if (!widget)
		goto __HANDLE_EVENT;

	if (_debug_keypress && (event->type == GDK_KEY_PRESS || event->type == GDK_KEY_RELEASE))
	{
		fprintf(stderr, "%s: keyval = %d state = %08X (%08X) is_modifier = %d\n", event->type == GDK_KEY_PRESS ? "GDK_KEY_PRESS" : "GDK_KEY_RELEASE",
						event->key.keyval, event->key.state, event->key.state & ~GDK_MODIFIER_MASK, event->key.is_modifier);
	}

	/*if ((event->type == GDK_KEY_PRESS || event->type == GDK_KEY_RELEASE))
	{
		if (event->key.state & ~GDK_MODIFIER_MASK) // == 0)
		{
			if (_debug_keypress)
				fprintf(stderr, "ignore key event\n");
			goto __HANDLE_EVENT;
		}
	}*/

	current_grab = gtk_window_group_get_current_grab(get_window_group(widget)); //gtk_grab_get_current();

	button_grab = gApplication::_button_grab;
	if (event->type == GDK_BUTTON_RELEASE)
		gApplication::setButtonGrab(NULL);

	if (gApplication::_control_grab)
	{
		control = gApplication::_control_grab;
		widget = control->border;
		//fprintf(stderr, "[1] _control_grab: %s -> widget = %p\n", control->name(), widget);
		goto __FOUND_WIDGET;
	}

	if (gMenu::currentPopup())
	{
		grab = GTK_WIDGET(gMenu::currentPopup()->child);
		//fprintf(stderr, "[2] popup menu: grab = %p\n", grab);
		if (get_window_group(grab) != get_window_group(widget) && (event->type == GDK_ENTER_NOTIFY || event->type == GDK_LEAVE_NOTIFY))
			goto __RETURN;
	}
	else
	{
		grab = current_grab; //gtk_window_group_get_current_grab(get_window_group(widget));
		//fprintf(stderr, "[3] popup: grab = %p / %p / %p\n", gApplication::_popup_grab, grab, gtk_grab_get_current());
		if (!grab)
			grab = gApplication::_popup_grab;
		//fprintf(stderr, "[4] grab = %p\n", grab);
		//fprintf(stderr, "search grab for widget %p -> group = %p -> grab = %p WINDOW = %d\n", widget, get_window_group(widget), grab, GTK_IS_WINDOW(grab));
		//if (grab && grab != widget && !GTK_IS_WINDOW(grab))
		//	goto __HANDLE_EVENT;

		//if (!grab && gApplication::_popup_grab)
		//	grab = gApplication::_popup_grab;
	}
		//gdk_window_get_user_data(gApplication::_popup_grab_window, (gpointer *)&grab);
	
	//if (grab && !gApplication::_popup_grab && !gApplication::_button_grab)
	//	goto __HANDLE_EVENT;
		
	//if (event->type == GDK_BUTTON_RELEASE)
	//	fprintf(stderr, "GDK_BUTTON_RELEASE #2\n");
	
	if (event->type == GDK_FOCUS_CHANGE)
	{
		control = NULL;
		//if (GTK_IS_WINDOW(widget))
		//	control = gt_get_control(widget);
		//fprintf(stderr, "GDK_FOCUS_CHANGE: widget = %p %d : %s  grab = %p\n", widget, GTK_IS_WINDOW(widget), control ? control->name() : NULL, grab);
		
		//if (GTK_IS_WINDOW(widget))
		{
			control = gt_get_control(widget);
			if (control)
				gApplication::setActiveControl(control, event->focus_change.in);
			else if (event->focus_change.in);
			{
				//fprintf(stderr, "GDK_FOCUS_CHANGE: setActiveWindow(NULL)\n");
				gMainWindow::setActiveWindow(NULL);
			}
		}

		if (event->focus_change.in && grab && widget != grab && !gtk_widget_is_ancestor(widget, grab))
		{
			//fprintf(stderr, "Check popup grab\n");
			gApplication::grabPopup();
			// Must continue, otherwise things are broken by some styles
			//return;
		}
			
		goto __HANDLE_EVENT;
	}
	
	if (grab && widget != grab && !gtk_widget_is_ancestor(widget, grab))
	{
		//fprintf(stderr, "-> widget = grab\n");
		widget = grab;
	}

	//fprintf(stderr, "grab = %p widget = %p %d\n", grab, widget, grab && !gtk_widget_is_ancestor(widget, grab));
	
	while (widget)
	{
		control = gt_get_control(widget);
		if (control || grab)
			break;
		widget = gtk_widget_get_parent(widget);
	}
	
	/*if (event->type == GDK_BUTTON_PRESS || event->type == GDK_BUTTON_RELEASE || event->type == GDK_MOTION_NOTIFY)
	{
		fprintf(stderr, "[%s] widget = %p grab = %p _popup_grab = %p _button_grab = %p\n",
						event->type == GDK_BUTTON_PRESS ? "down" : event->type == GDK_BUTTON_RELEASE ? "up" : "move",
						widget, grab, gApplication::_popup_grab, gApplication::_button_grab);
		//fprintf(stderr, "widget = %p (%p) grab = %p (%p)\n", widget, widget ? g_object_get_data(G_OBJECT(widget), "gambas-control") : 0,
		//				grab, grab ? g_object_get_data(G_OBJECT(grab), "gambas-control") : 0);
	}*/

	/*if (event->type == GDK_KEY_PRESS)
	{
		fprintf(stderr, "[GDK_KEY_PRESS] widget = %p grab = %p _popup_grab = %p _button_grab = %p\n",
						widget, grab, gApplication::_popup_grab, gApplication::_button_grab);
	}*/

	if (!widget || !control)
		goto __HANDLE_EVENT;

__FOUND_WIDGET:
	
	//fprintf(stderr, "control = %p %s\n", control, control->name());
	
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
	
	if (!gdk_events_pending() && event->type != GDK_ENTER_NOTIFY && event->type != GDK_LEAVE_NOTIFY)
	{
		if (gApplication::_leave)
		{
			//if () // || (gApplication::_leave != control && check_crossing_event(event)))
			#if DEBUG_ENTER_LEAVE
			fprintf(stderr, "post leave: %s\n", gApplication::_leave->name());
			#endif
			
			if (gApplication::_enter == gApplication::_leave)
				gApplication::_enter = NULL;
			
			gApplication::_leave->emitLeaveEvent();
			
			gApplication::_leave = NULL;
		}
	}
	
	cancel = false;
	
	gApplication::updateLastEvent(event);
	
	switch ((int)event->type)
	{
		case GDK_ENTER_NOTIFY:
			
			control = find_child(control, (int)event->button.x_root, (int)event->button.y_root);
			
			//fprintf(stderr, "GDK_ENTER_NOTIFY: %s (%s)\n", control->name(), gApplication::_enter ? gApplication::_enter->name() : "ø");
			
			if (gApplication::_leave == control)
			{
				#if DEBUG_ENTER_LEAVE
				fprintf(stderr, "enter ignored: %s\n", control->name());
				#endif
				gApplication::_leave = NULL;
			}
			else if (gApplication::_enter != control)
			{
				if (check_crossing_event(event))
				{
					#if DEBUG_ENTER_LEAVE
					fprintf(stderr, "enter: %s\n", control->name());
					#endif
					gApplication::checkHoveredControl(control);
				}
			}

			break;
		
		case GDK_LEAVE_NOTIFY:
			
			//fprintf(stderr, "GDK_LEAVE_NOTIFY: %s\n", control->name());
			
			//control = find_child(control, (int)event->button.x_root, (int)event->button.y_root);
			
			if (gdk_events_pending() && gApplication::_leave == NULL)
			{
				if (check_crossing_event(event))
				{
					#if DEBUG_ENTER_LEAVE
					fprintf(stderr, "leave later: %s\n", control->name());
					#endif
					gApplication::_leave = control;
				}
			}
			else if (gApplication::_leave != control)
			{
				if (check_crossing_event(event))
				{
					if (gApplication::_enter == control)
						gApplication::_enter = NULL;
					
					if (gApplication::_leave == control)
						gApplication::_leave = NULL;
			
					#if DEBUG_ENTER_LEAVE
					fprintf(stderr, "leave: %s\n", control->name());
					#endif
					control->emitLeaveEvent();
				}
			}
			
			//if (widget != control->border && widget != control->widget)
			//	goto __RETURN;
			
			break;
			
		case GDK_BUTTON_PRESS:
		case GDK_2BUTTON_PRESS:
		case GDK_BUTTON_RELEASE:
		{
			save_control = control = find_child(control, (int)event->button.x_root, (int)event->button.y_root, button_grab);

			bool menu = false;
			
			switch ((int)event->type)
			{
				case GDK_BUTTON_PRESS: type = gEvent_MousePress; break;
				case GDK_2BUTTON_PRESS: type = gEvent_MouseDblClick; break;
				default: type = gEvent_MouseRelease; break;
			}
			
			if (event->type != GDK_BUTTON_RELEASE)
			{
				if (control->canFocus() && !control->hasFocus())
					control->setFocus();
				if (!control->_no_auto_grab)
					gApplication::setButtonGrab(control);
			}

			
		__BUTTON_TRY_PROXY:
		
			cancel = false;
		
			if (control->onMouseEvent)
			{
				if (control->canRaise(control, type))
				{
					control->getScreenPos(&xc, &yc);
					xs = (int)event->button.x_root;
					ys = (int)event->button.y_root;
					x = xs - xc;
					y = ys - yc;
					
					gMouse::validate();
					gMouse::setEvent(event);
					//gMouse::setValid(1,(int)event->x,(int)event->y,event->button,event->state,data->screenX(),data->screenY());
					gMouse::setMouse(x, y, xs, ys, event->button.button, event->button.state);
					switch ((int)event->type)
					{
						case GDK_BUTTON_PRESS: 
							gMouse::setStart(x, y);
							cancel = control->onMouseEvent(control, gEvent_MousePress);
							break;
						
						case GDK_2BUTTON_PRESS: 
							cancel = control->onMouseEvent(control, gEvent_MouseDblClick); 
							break;
						
						case GDK_BUTTON_RELEASE: 
							cancel = control->onMouseEvent(control, gEvent_MouseRelease); 
							break;
					}
					
					gMouse::invalidate();
				}
			}
				
			if (type == gEvent_MousePress && control->isTopLevel())
			{
				gMainWindow *win = ((gMainWindow *)control);
				if (win->isPopup())
				{
					control->getScreenPos(&xc, &yc);
					xs = (int)event->button.x_root;
					ys = (int)event->button.y_root;
					x = xs - xc;
					y = ys - yc;
				
					if (x < 0 || y < 0 || x >= win->width() || y >= win->height())
						win->close();
				}
			}
			else if (type == gEvent_MouseRelease && control->_grab)
			{
				gApplication::exitLoop(control);
			}
			
#if GTK_CHECK_VERSION(3, 4, 0)
			if (gdk_event_triggers_context_menu(event))
#else
			if (event->button.button == 3 && event->type == GDK_BUTTON_PRESS)
#endif
				menu = true;
			
			if (!cancel)
			{
				if (control->_proxy_for)
				{
					control = control->_proxy_for;
					//fprintf(stderr, "PRESS: try %s\n", control->name());
					goto __BUTTON_TRY_PROXY;
				}
			}
			
			if (menu)
			{
				control = save_control;
				while (control)
				{
					if (control->onMouseEvent(control, gEvent_MouseMenu))
					{
						cancel = true;
						break;
					}
					control = control->_proxy_for;
				}
			}
			
			if (cancel)
			{
				gMouse::resetTranslate();
				goto __RETURN;
			}

			if (widget != save_control->border && widget != save_control->widget)
			{
				//fprintf(stderr, "widget = %p, control = %p %p %s\n", widget, save_control->border, save_control->widget, save_control->name());
				gMouse::resetTranslate();
				goto __RETURN;
			}
			
			break;
		}
			
		case GDK_MOTION_NOTIFY:

			gdk_event_request_motions(&event->motion);

			save_control = control = find_child(control, (int)event->motion.x_root, (int)event->motion.y_root, button_grab);
			
			//fprintf(stderr, "GDK_MOTION_NOTIFY: (%p %s) grab = %p\n", control, control->name(), button_grab);
			
			gApplication::checkHoveredControl(control);
			
		__MOTION_TRY_PROXY:
		
			if (control->onMouseEvent && (control->canRaise(control, gEvent_MouseMove) || control->canRaise(control, gEvent_MouseDrag))
					&& (control->isTracking() || (event->motion.state & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK))))
			{
				control->getScreenPos(&xc, &yc);
				xs = (int)event->motion.x_root;
				ys = (int)event->motion.y_root;
				x = xs - xc;
				y = ys - yc;
				
				gMouse::validate();
				gMouse::setEvent(event);
				gMouse::setMouse(x, y, xs, ys, 0, event->motion.state);
				
				//fprintf(stderr, "pressure = %g\n", gMouse::getAxis(GDK_AXIS_PRESSURE));
				
				cancel = control->onMouseEvent(control, gEvent_MouseMove);
			
				//if (data->acceptDrops() && gDrag::checkThreshold(data, gMouse::x(), gMouse::y(), gMouse::startX(), gMouse::startY()))
				if (!cancel && (event->motion.state & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK))
						//&& (abs(gMouse::x() - gMouse::y()) + abs(gMouse::startX() - gMouse::startY())) > 8)
						&& gDrag::checkThreshold(control, gMouse::x(), gMouse::y(), gMouse::startX(), gMouse::startY()))
				{
					//fprintf(stderr, "gEvent_MouseDrag: event = %p\n", gApplication::lastEvent());
					cancel = control->onMouseEvent(control, gEvent_MouseDrag);
				}
				gMouse::invalidate();
				
				if (cancel)
					goto __RETURN;
			}

			if (control->_proxy_for)
			{
				control = control->_proxy_for;
				//fprintf(stderr, "MOVE: try %s\n", control->name());
				goto __MOTION_TRY_PROXY;
			}
			
			gMouse::resetTranslate();
			//if (widget != save_control->border && widget != save_control->widget)
			//	goto __RETURN;
			
			break;
			
		case GDK_SCROLL:
			
			save_control = control = find_child(control, (int)event->scroll.x_root, (int)event->scroll.y_root);

		__SCROLL_TRY_PROXY:

			if (control->onMouseEvent && control->canRaise(control, gEvent_MouseWheel))
			{
				int dir, dt, ort;
				
				control->getScreenPos(&xc, &yc);
				xs = (int)event->scroll.x_root;
				ys = (int)event->scroll.y_root;
				x = xs - xc;
				y = ys - yc;

				dir = event->scroll.direction;

#ifdef GTK3
				if (dir == GDK_SCROLL_SMOOTH)
				{
					/*gdouble dx = 0, dy = 0;
					gdk_event_get_scroll_deltas((GdkEvent *)event, &dx, &dy);
					if (fabs(dy) > fabs(dx))
						dir = (dy < 0) ? GDK_SCROLL_UP : GDK_SCROLL_DOWN;
					else
						dir = (dx < 0) ? GDK_SCROLL_LEFT : GDK_SCROLL_RIGHT;*/
					goto __HANDLE_EVENT;
				}
#endif

				switch (dir)
				{
					case GDK_SCROLL_DOWN: dt = -1; ort = 1; break;
					case GDK_SCROLL_LEFT: dt = -1; ort = 0; break;
					case GDK_SCROLL_RIGHT:  dt = 1; ort = 0; break;
					case GDK_SCROLL_UP: default: dt = 1; ort = 1; break;
				}
				
				gMouse::validate();
				gMouse::setEvent(event);
				gMouse::setMouse(x, y, xs, ys, 0, event->scroll.state);
				gMouse::setWheel(dt, ort);
				cancel = control->onMouseEvent(control, gEvent_MouseWheel);
				gMouse::invalidate();
			}

			if (cancel)
			{
				gMouse::resetTranslate();
				goto __RETURN;
			}
				
			if (control->_proxy_for)
			{
				control = control->_proxy_for;
				goto __SCROLL_TRY_PROXY;
			}
			
			if (widget != save_control->border && widget != save_control->widget)
			{
				gMouse::resetTranslate();
				goto __RETURN;
			}
			
			break;

		case GDK_KEY_PRESS:
		case GDK_KEY_RELEASE:
		{
			gMainWindow *win;
			
			if (!control->_grab && gApplication::activeControl())
				control = gApplication::activeControl();

			//if (event->type == GDK_KEY_PRESS)
			//	fprintf(stderr, "GDK_KEY_PRESS: control = %p %s %p %08X\n", control, control ? control->name() : "", event, event->key.state);

			type =  (event->type == GDK_KEY_PRESS) ? gEvent_KeyPress : gEvent_KeyRelease;
			
			if (control)
			{
				if (gKey::enable(control, &event->key))
				{
					gKey::disable();
					if (gKey::canceled())
					{
						//if (type == gEvent_KeyPress) fprintf(stderr, "canceled\n");
						goto __RETURN;
					}
					else
					{
						//if (type == gEvent_KeyPress) fprintf(stderr, "handle\n");
						goto __HANDLE_EVENT;
					}
				}

				if (gKey::mustIgnoreEvent(&event->key))
				{
					gKey::disable();
					goto __HANDLE_EVENT;
				}

				cancel = gKey::raiseEvent(type, control, NULL);
				gKey::disable();
				
				if (cancel)
				{
					//if (type == gEvent_KeyPress) fprintf(stderr, "canceled #2\n");
					goto __RETURN;
				}
				else
				{
					//if (type == gEvent_KeyPress) fprintf(stderr, "handle #2\n");
				}

				win = control->window();

				if (event->key.keyval == GDK_Escape)
				{
					if (control->_grab)
					{
						gApplication::exitLoop(control);
						goto __RETURN;
					}

					if (check_button(win->_cancel))
					{
						win->_cancel->setFocus();
						win->_cancel->animateClick(type == gEvent_KeyRelease);
						goto __RETURN;
					}
				}
				else if (event->key.keyval == GDK_Return || event->key.keyval == GDK_KP_Enter)
				{
					if (check_button(win->_default))
					{
						win->_default->setFocus();
						win->_default->animateClick(type == gEvent_KeyRelease);
						goto __RETURN;
					}
				}

				if (control->_grab)
					goto __RETURN;
			}

			break;
		}
	}
	
__HANDLE_EVENT:

	gtk_main_do_event(event);
	
__RETURN:

	(void)0;
}



/**************************************************************************

gApplication

**************************************************************************/

int appEvents;

bool gApplication::_busy = false;
char *gApplication::_title = NULL;
int gApplication::_in_popup = 0;
GtkWidget *gApplication::_popup_grab = NULL;
int gApplication::_loopLevel = 0;
void *gApplication::_loop_owner = 0;
GtkWindowGroup *gApplication::_group = NULL;
gControl *gApplication::_enter = NULL;
gControl *gApplication::_leave = NULL;
gControl *gApplication::_ignore_until_next_enter = NULL;
gControl *gApplication::_button_grab = NULL;
gControl *gApplication::_control_grab = NULL;
gControl *gApplication::_active_control = NULL;
gControl *gApplication::_previous_control = NULL;
gControl *gApplication::_old_active_control = NULL;
bool (*gApplication::onKeyEvent)(int) = NULL;
guint32 gApplication::_event_time = 0;
bool gApplication::_close_next_window = false;
bool gApplication::_fix_printer_dialog = false;
gMainWindow *gApplication::_main_window = NULL;
void (*gApplication::onEnterEventLoop)();
void (*gApplication::onLeaveEventLoop)();
bool gApplication::_must_quit = false;
GdkEvent *gApplication::_event = NULL;

void gApplication::grabPopup()
{
	//fprintf(stderr, "grabPopup: %p\n", _popup_grab);
	
	if (!_popup_grab)
		return;
	
	//gtk_grab_add(_popup_grab);
	//return;
	
	GdkWindow *win = gtk_widget_get_window(_popup_grab);
		
  int ret = gdk_pointer_grab(win, TRUE,
			 (GdkEventMask)(GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
			 GDK_POINTER_MOTION_MASK),
			 NULL, NULL, GDK_CURRENT_TIME);
	
	if (ret == GDK_GRAB_SUCCESS)
	{
		ret = gdk_keyboard_grab(win, TRUE, GDK_CURRENT_TIME);
		
		if (ret == GDK_GRAB_SUCCESS)
			return;
		
		gdk_pointer_ungrab(GDK_CURRENT_TIME);
	}

	fprintf(stderr, "gb.gtk: warning: grab failed: %d\n", ret);
}

void gApplication::ungrabPopup()
{
	//fprintf(stderr, "ungrabPopup: %p\n", _popup_grab);
	//gtk_grab_remove(_popup_grab);
	_popup_grab = NULL;
	gdk_pointer_ungrab(GDK_CURRENT_TIME);
	gdk_keyboard_ungrab(GDK_CURRENT_TIME);
}

bool gApplication::areTooltipsEnabled()
{
  gboolean enabled;
  GtkSettings *settings;

  settings = gtk_settings_get_default();

  g_object_get (settings, "gtk-enable-tooltips", &enabled, (char *)NULL);

  return enabled;
}

void gApplication::enableTooltips(bool vl)
{
  GtkSettings *settings;
	gboolean enabled = vl;
  settings = gtk_settings_get_default();
  g_object_set (settings, "gtk-enable-tooltips", enabled, (char *)NULL);
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

static void do_nothing()
{
}

/*#ifdef GTK3
static void (*_old_scrollbar_button_press)();
static void (*_old_scrollbar_button_release)();

static gint scrollbar_button_press(GtkWidget *widget, GdkEventButton *event)
{
	gint ret = ((gint (*)(GtkWidget *, GdkEventButton *))_old_scrollbar_button_press)(widget, event);
	if (ret)
		gtk_grab_add(widget);
	return ret;
}

static gint scrollbar_button_release(GtkWidget *widget, GdkEventButton *event)
{
	gint ret = ((gint (*)(GtkWidget *, GdkEventButton *))_old_scrollbar_button_release)(widget, event);
	if (ret)
		gtk_grab_remove(widget);
	return ret;
}

#endif*/

static gboolean master_client_save_yourself(GnomeClient *client, gint phase, GnomeSaveStyle save_style, gboolean is_shutting_down, GnomeInteractStyle interact_style, gboolean fast, gpointer user_data)
{
	if (gApplication::mainWindow())
	{
		//fprintf(stderr, "master_client_save_yourself: %d\n", X11_window_get_desktop((Window)(gApplication::mainWindow()->handle())));
		session_manager_set_desktop(X11_window_get_desktop((Window)(gApplication::mainWindow()->handle())));
	}
	return true;
}

static void master_client_die(GnomeClient *client, gpointer user_data)
{
	if (gApplication::mainWindow())
		gApplication::mainWindow()->close();
	else
		gMainWindow::closeAll();

	gApplication::quit();
	MAIN_check_quit();
}

void gApplication::init(int *argc, char ***argv)
{
	appEvents=0;
	
	gtk_init(argc, argv);
	session_manager_init(argc, argv);
	g_signal_connect(gnome_master_client(), "save-yourself", G_CALLBACK(master_client_save_yourself), NULL);
	g_signal_connect(gnome_master_client(), "die", G_CALLBACK(master_client_die), NULL);

	gdk_event_handler_set((GdkEventFunc)gambas_handle_event, NULL, NULL);
	
	gClipboard::init();
	gKey::init();
	
	onEnterEventLoop = do_nothing;
	onLeaveEventLoop = do_nothing;

	_group = gtk_window_group_new();
	
	_loop_owner = 0;

	char *env = getenv("GB_GTK_DEBUG_KEYPRESS");
	if (env && strcmp(env, "0"))
		_debug_keypress = true;

/*#ifdef GTK3 // patch GtkRange class
	GtkWidgetClass *klass;

	klass = (GtkWidgetClass *)g_type_class_ref(GTK_TYPE_SCROLLBAR);

	_old_scrollbar_button_press = (void (*)())klass->button_press_event;
	klass->button_press_event = scrollbar_button_press;

	_old_scrollbar_button_release = (void (*)())klass->button_release_event;
	klass->button_release_event = scrollbar_button_release;

#endif*/
}

void gApplication::exit()
{
	session_manager_exit();
	
	if (_title)
		g_free(_title);

	gKey::exit();
	gTrayIcon::exit();
  gDesktop::exit();
  gMessage::exit();
  gDialog::exit();
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
	gControl *control;
	
	while (wid)
	{
		control = gt_get_control(wid);
		if (control)
			return control;
		wid = gtk_widget_get_parent(wid);
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

	MAIN_do_iteration_just_events();
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

void gApplication::enterLoop(void *owner, bool showIt, GtkWindow *modal)
{
	void *old_owner = _loop_owner;
	int l = _loopLevel;
	//GtkWindowGroup *oldGroup;
	
	if (showIt) ((gControl *)owner)->show();

	//oldGroup = enterGroup();
	
	_loopLevel++;
	_loop_owner = owner;
	
	(*onEnterEventLoop)();
	do
	{
		MAIN_do_iteration(false);
	}
	while (_loopLevel > l);
	(*onLeaveEventLoop)();
	
	_loop_owner = old_owner;

	//exitGroup(oldGroup);
}

void gApplication::enterPopup(gMainWindow *owner)
{
	void *old_owner = _loop_owner;
	int l = _loopLevel;
	//GtkWindowGroup *oldGroup;
	GtkWindow *window = GTK_WINDOW(owner->border);
	GtkWidget *old_popup_grab;
	
	_in_popup++;
	
	// Remove possible current button grab
	gApplication::setButtonGrab(NULL);
//
	//oldGroup = enterGroup();
	
	gtk_window_set_modal(window, true);
	gdk_window_set_override_redirect(gtk_widget_get_window(owner->border), true);
	owner->show();
	
	old_popup_grab = _popup_grab;
	_popup_grab = owner->border;
	
	if (_in_popup == 1)
		gApplication::grabPopup();
	
	_loopLevel++;
	_loop_owner = owner;
	
	(*onEnterEventLoop)();
	do
	{
		MAIN_do_iteration(false);
	}
	while (_loopLevel > l);
	(*onLeaveEventLoop)();
	
	gApplication::ungrabPopup();
	_popup_grab = old_popup_grab;
	
	_loop_owner = old_owner;

	gdk_window_set_override_redirect(gtk_widget_get_window(owner->border), false);
	gtk_window_set_modal(window, false);
	//exitGroup(oldGroup);
	
	_in_popup--;
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

void gApplication::updateLastEvent(GdkEvent *e)
{
	_event = e;
	_event_time = gdk_event_get_time(e);
}

static void post_focus_change(void *)
{
	gControl *current, *control, *next;
	
	//fprintf(stderr, "post_focus_change\n");
	
	for(;;)
	{
		current = gApplication::activeControl();
		if (current == gApplication::_old_active_control)
			break;

		control = gApplication::_old_active_control;
		while (control)
		{
			next = control->_proxy_for;
			if (control->onFocusEvent)
				control->onFocusEvent(control, gEvent_FocusOut);
			control = next;
		}
		
		current = gApplication::activeControl();
		if (current == gApplication::_old_active_control)
			break;
		
		gApplication::_old_active_control = current;
		//fprintf(stderr, "post_focus_change: setActiveWindow\n");
		gMainWindow::setActiveWindow(current);
		
		control = gApplication::activeControl();
		while (control)
		{
			next = control->_proxy_for;
			if (control->onFocusEvent)
				control->onFocusEvent(control, gEvent_FocusIn);
			control = next;
		}
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

	if (_active_control)
		_previous_control = _active_control;

	_active_control = on ? control : NULL;
	gKey::setActiveControl(_active_control);
	handle_focus_change();
}

int gApplication::getScrollbarSize()
{
	//GtkStyle* st;
	gint trough_border;
	gint slider_width;
	
	//st = gtk_rc_get_style_by_paths(gtk_settings_get_default(), NULL, "OsBar", G_TYPE_NONE);

	if (g_type_from_name("OsBar"))
	{
		char *env = getenv("LIBOVERLAY_SCROLLBAR");
		if (!env || *env != '0')
			return 1;
	}
	
  gtk_style_get(gt_get_old_style(GTK_TYPE_SCROLLBAR), GTK_TYPE_SCROLLBAR,
		"slider-width", &slider_width,
		"trough-border", &trough_border,
		/*"focus-line-width", &focus_line_width,
		"focus-padding", &focus_padding,*/
		(char *)NULL);
	
	return (trough_border) * 2 + slider_width;
}

int gApplication::getScrollbarSpacing()
{
	gint v;
	
	gtk_style_get(gt_get_old_style(GTK_TYPE_SCROLLED_WINDOW), GTK_TYPE_SCROLLED_WINDOW,
		"scrollbar-spacing", &v, 
		(char *)NULL);
	
	return v;
}

int gApplication::getInnerWidth()
{
	if (strcmp(getStyleName(), "oxygen-gtk") == 0)
		return 1;
	else
		return 0;
}

int gApplication::getFrameWidth()
{
	int w;
#ifdef GTK3
  GtkStyleContext *context = gt_get_style(GTK_TYPE_ENTRY);
  GtkBorder tmp;
  //GtkBorder border;

	gtk_style_context_get_padding(context, (GtkStateFlags)0, &tmp);
	/*gtk_style_context_get_border(context, (GtkStateFlags)0, &border);

	tmp.top += border.top;
	tmp.right += border.right;
	tmp.bottom += border.bottom;
	tmp.left += border.left;*/

	w = MIN(tmp.top, tmp.left);
	w = MIN(w, tmp.bottom);
	w = MIN(w, tmp.right);
	w = MAX(0, w - 1);

#else
	GtkStyle *style;
	gint focus_width;
	gboolean interior_focus;
	//int inner;

	style = gt_get_old_style(GTK_TYPE_ENTRY);

	gtk_style_get(style, GTK_TYPE_ENTRY,
		"focus-line-width", &focus_width,
		"interior-focus", &interior_focus,
		(char *)NULL);

	w = MIN(style->xthickness, style->ythickness);

	if (!interior_focus)
		w += focus_width;

	w += getInnerWidth();
#endif

	return w;
}

void gApplication::getBoxFrame(int *w, int *h)
{
	GtkStyle *style;
	gint focus_width;
	gboolean interior_focus;
	int inner;

	style = gt_get_old_style(GTK_TYPE_ENTRY);
	
	gtk_style_get(style, GTK_TYPE_ENTRY,
		"focus-line-width", &focus_width,
		"interior-focus", &interior_focus,
		(char *)NULL);
	
	*w = style->xthickness;
	*h = style->ythickness;
	
	if (!interior_focus)
	{
		*w += focus_width;
		*h += focus_width;
	}
	
	inner = getInnerWidth();
	*w += inner;
	*h += inner;
}

char *gApplication::getStyleName()
{
	static char *_theme = NULL;

	if (!_theme)
	{
		GtkSettings *settings = gtk_settings_get_default();
		g_object_get(settings, "gtk-theme-name", &_theme, (char *)NULL);
	}

	return _theme;
}

static GdkFilterReturn x11_event_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
	((X11_EVENT_FILTER)data)((XEvent *)xevent);
	return GDK_FILTER_CONTINUE;
}

void gApplication::setEventFilter(X11_EVENT_FILTER filter)
{
	static X11_EVENT_FILTER save_filter = NULL;

	if (save_filter)
		gdk_window_remove_filter(NULL, (GdkFilterFunc)x11_event_filter, (gpointer)save_filter);

	if (filter)
		gdk_window_add_filter(NULL, (GdkFilterFunc)x11_event_filter, (gpointer)filter);

	save_filter = filter;
}

void gApplication::setMainWindow(gMainWindow *win)
{
	_main_window = win;
}

void gApplication::quit()
{
	_must_quit = true;
}
