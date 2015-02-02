/***************************************************************************

  gcontainer.cpp

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

#include <gtk/gtk.h>
#include "widgets.h"

#include <stdio.h>
#include <unistd.h>

#include "gapplication.h"
#include "gdesktop.h"
#include "gmainwindow.h"
#include "gcontainer.h"

static gControl* get_next_child_widget (gContainer *gtk_control, int *gtk_list, int gtk_count)
{
	gControl *ctrl;
	
	while ( (*gtk_list) < gtk_count )
	{
		ctrl = gtk_control->child(*gtk_list);
		//fprintf(stderr, "get_next_child_widget: %d: %p: %s\n", *gtk_list, ctrl, ctrl->name());
		(*gtk_list)++;
		
		if (!ctrl->border || !ctrl->widget || !ctrl->isVisible())
			continue;
		
		// Useless, as gTabStrip reimplements child() and childCount()
		//if (gtk_widget_get_parent(ctrl->border) != cont)
		//	continue;
		
		//fprintf(stderr, "get_next_child_widget: ==> %p\n", ctrl);
		return ctrl;
	}
	
	//fprintf(stderr, "get_next_child_widget: ==> NULL\n");
	return NULL; 
}

static void cb_arrange(gContainer *sender)
{
	if (sender->onArrange)
		(*(sender->onArrange))(sender);
}

static void cb_before_arrange(gContainer *sender)
{
	if (sender->onBeforeArrange)
		(*(sender->onBeforeArrange))(sender);
}

static void resize_container(gControl *cont, int w, int h)
{
	if (w > 0 && h > 0)
		cont->resize(w, h);
}


#define WIDGET_TYPE gControl*
#define CONTAINER_TYPE gContainer*
#define ARRANGEMENT_TYPE gContainerArrangement*
#define IS_RIGHT_TO_LEFT() gDesktop::rightToLeft()
#define GET_WIDGET(_object) _object
#define GET_CONTAINER(_object) _object
#define GET_ARRANGEMENT(_object) (((gContainer*)_object)->getArrangement())
#define IS_EXPAND(_object) (((gControl*)_object)->expand())
#define IS_IGNORE(_object) (((gControl*)_object)->ignore())
#define IS_DESIGN(_object) (((gControl*)_object)->design())
#define IS_WIDGET_VISIBLE(_widget)  (((gControl*)_widget)->isVisible())

#define CAN_ARRANGE(_object) (IS_WIDGET_VISIBLE(GET_WIDGET(_object)) || (((gControl *)_object)->isTopLevel() && ((gMainWindow *)_object)->opened))

// BM: ClientX() & ClientY() are relative to the border.
// We need X & Y relative to the container widget.

#define GET_WIDGET_CONTENTS(_widget, _x, _y, _w, _h) _x=((gContainer*)_widget)->containerX(); \
                                                     _y=((gContainer*)_widget)->containerY(); \
                                                     _w=((gContainer*)_widget)->clientWidth(); \
                                                     _h=((gContainer*)_widget)->clientHeight()

#define GET_WIDGET_X(_widget)  (((gControl*)_widget)->left())
#define GET_WIDGET_Y(_widget)  (((gControl*)_widget)->top())
#define GET_WIDGET_W(_widget)  (((gControl*)_widget)->width())
#define GET_WIDGET_H(_widget)  (((gControl*)_widget)->height())
#define MOVE_WIDGET(_object, _widget, _x, _y)  (((gControl*)_widget)->move( _x, _y))
#define RESIZE_WIDGET(_object, _widget, _w, _h)  (((gControl*)_widget)->resize( _w, _h))
#define MOVE_RESIZE_WIDGET(_object, _widget, _x, _y, _w, _h) (((gControl*)_widget)->moveResize( _x, _y, _w, _h))
#define RESIZE_CONTAINER(_widget, _cont, _w, _h)  resize_container((gControl *)(_cont), _w, _h) 

#define INIT_CHECK_CHILDREN_LIST(_widget) \
	gContainer *gtk_control=(gContainer*)_widget; \
	int gtk_list=0; \
	int gtk_count = gtk_control->childCount();

#define HAS_CHILDREN() (gtk_count != 0)

#define RESET_CHILDREN_LIST() gtk_list=0;

#define GET_NEXT_CHILD_WIDGET() get_next_child_widget (gtk_control,&gtk_list,gtk_count)

#define GET_OBJECT_FROM_WIDGET(_widget) ((void*)_widget)

#define GET_OBJECT_NAME(_object) (((gControl *)_object)->name())

#define RAISE_ARRANGE_EVENT(_object) cb_arrange((gContainer *)_object);
#define RAISE_BEFORE_ARRANGE_EVENT(_object) cb_before_arrange((gContainer *)_object);

#define DESKTOP_SCALE gDesktop::scale()

#define FUNCTION_NAME arrangeContainer

#include "gb.form.arrangement.h"


int gContainer::_arrangement_level = 0;

void gContainer::performArrange()
{
	if (_no_arrangement)
	{
		_did_arrangement = true;
		return;
	}
	
	if (!gApplication::allEvents()) return;

	_did_arrangement = false;
	//if (!CAN_ARRANGE(this))
	//	return;
	
	//if (name() && !strcmp(name(), "panToolbar")) fprintf(stderr, ">>> gContainer::performArrange: %s %d\n", name(), arrangement.locked);
	//_arrangement_level++;	
	arrangeContainer((void*)this);
	//_arrangement_level--;
	//if (name() && !strcmp(name(), "panToolbar")) fprintf(stderr, "<<< gContainer::performArrange: %s %d\n", name(), arrangement.locked);
}

void gContainer::initialize()
{
	_children = g_ptr_array_new();
	
	radiogroup = NULL;
	onArrange = NULL;
	onBeforeArrange = NULL;
	_proxyContainer = NULL;
	_client_x = -1;
	_client_y = -1;
	_client_w = 0;
	_client_h = 0;
	_no_arrangement = 0;
	//onInsert = NULL;
	
	arrangement.mode = 0;
	arrangement.spacing = 0;
	arrangement.padding = 0;
	arrangement.autoresize = false;
	arrangement.locked = false;
	arrangement.user = false;
	arrangement.margin = false;
	arrangement.indent = 0;
	arrangement.invert = false;
}

gContainer::gContainer() 
{
	//g_print("gContainer() %d\n",this);
	initialize();
}

gContainer::gContainer(gContainer *parent) : gControl(parent)
{
	//g_print("gContainer(parent) %d par: %d\n",this,parent);
	initialize();
}

gContainer::~gContainer()
{
	int i;
	
	for (i = 0; i < childCount(); i++)
		child(i)->removeParent();
	
	g_ptr_array_unref(_children);
	
	if (radiogroup) { g_object_unref(G_OBJECT(radiogroup)); radiogroup=NULL; }
}

int gContainer::childCount() const
{
	return _children->len;
}

gControl* gContainer::child(int index) const
{
	if (index < 0 || index >= (int)_children->len)
		return NULL;
	else
		return (gControl *)g_ptr_array_index(_children, index);
}

int gContainer::childIndex(gControl *ch) const
{
	int i;
	
	for (i = 0; i < childCount(); i++)
	{
		if (child(i) == ch)
			return i;
	}
	
	return -1;
}

void gContainer::setArrange(int vl)
{
	switch(vl)
	{
		case ARRANGE_NONE:
		case ARRANGE_HORIZONTAL:
		case ARRANGE_VERTICAL:
		case ARRANGE_LEFT_RIGHT:
		case ARRANGE_TOP_BOTTOM:
		case ARRANGE_FILL:
			if (vl != arrangement.mode)
			{
				arrangement.mode = vl;
				performArrange();
			}
		default: 
			break;
	}
}

void gContainer::setPadding(int vl)
{
	if (vl >= 0 && vl <= 255 && vl != arrangement.padding) 
	{
		arrangement.padding = vl;
		performArrange();
	}
}

void gContainer::setSpacing(bool vl)
{
	if (vl != arrangement.spacing)
	{
		arrangement.spacing = vl;
		performArrange();
	}
}

void gContainer::setMargin(bool vl)
{
	if (vl != arrangement.margin)
	{
		arrangement.margin = vl;
		performArrange();
	}
}

void gContainer::setIndent(int vl)
{
	if (vl < 0)
		vl = 1;
	if (vl >= 0 && vl <= 7 && vl != arrangement.indent) 
	{
		arrangement.indent = vl;
		performArrange();
	}
}

void gContainer::setAutoResize(bool vl)
{
	if (vl != arrangement.autoresize)
	{
		arrangement.autoresize = vl;
		performArrange();
	}
}

void gContainer::setUser(bool vl)
{
	if (vl != arrangement.user)
	{
		arrangement.user = vl;
		performArrange();
	}
}

void gContainer::setInvert(bool vl)
{
	if (vl != arrangement.invert)
	{
		arrangement.invert = vl;
		performArrange();
	}
}

int gContainer::clientX()
{
	gint xc, yc;
	GtkWidget *cont = getContainer();
	
	if (_client_x >= 0)
		return _client_x;
	
	if (gtk_widget_get_window(cont) && gtk_widget_get_window(border))
	{
		gtk_widget_translate_coordinates(cont, border, 0, 0, &xc, &yc);
		xc += containerX();
	}
	else
		xc = getFrameWidth();
		
	return xc;
}

int gContainer::containerX()
{
	GtkWidget *cont = getContainer();
	
	if (cont == widget && widget == frame)
		return getFrameWidth();
	else
		return 0;
}

int gContainer::clientY()
{
	gint xc, yc;
	GtkWidget *cont = getContainer();
	
	if (_client_y >= 0)
		return _client_y;
	
	if (gtk_widget_get_window(cont) && gtk_widget_get_window(border))
	{
		gtk_widget_translate_coordinates(cont, border, 0, 0, &xc, &yc);
		yc += containerY();
	}
	else
		yc = getFrameWidth();
		
	return yc;
}

#if 0
int gContainer::clientY()
{
	gint xc, yc;
	GtkWidget *cont = getContainer();
	
	if (!cont->window || !border->window || cont == widget)
		return getFrameWidth();
	
// 	if (width() != border->allocation.width || height() != border->allocation.height)
// 	{
// 		updateGeometry();
// 		GtkAllocation a = { x(), y(), width(), height() };
// 		gtk_widget_size_allocate(widget, &a);
// 	}

	gtk_widget_translate_coordinates(cont, border, 0, 0, &xc, &yc);
	if (cont == widget)
		yc += getFrameWidth();
	
	return yc; // + getFrameWidth();
}
#endif

int gContainer::containerY()
{
	GtkWidget *cont = getContainer();
	
	if (cont == widget && widget == frame)
		return getFrameWidth();
	else
		return 0;
}

int gContainer::clientWidth()
{
	GtkWidget *cont = getContainer();
	
	if (_client_w > 0)
		return _client_w;
	
	if (cont != widget && gtk_widget_get_window(cont))
	{
		GtkAllocation a;

		gtk_widget_get_allocation(widget, &a);

		if ((width() != a.width || height() != a.height)
		    && a.width > 0 && a.height > 0)
		{
			//g_debug("clientWidth: %s: %d", name(), width());
			a.x = x(); a.y = y(); a.width = width(); a.height = height();
			gt_disable_warnings(true);
			gtk_widget_size_allocate(widget, &a);
			gt_disable_warnings(false);
		}
		//g_debug("ClientWidth: %s -> %d", this->name(), cont->allocation.width);

		gtk_widget_get_allocation(cont, &a);

		if (a.width > 0)
			return a.width;
	}
	
	if (_scroll)
		return (int)gtk_adjustment_get_page_size(gtk_scrolled_window_get_hadjustment(_scroll));
	
	return width() - getFrameWidth() * 2;
}

int gContainer::clientHeight()
{
	GtkWidget *cont = getContainer();
	
	if (_client_h > 0)
		return _client_h;
	
	if (cont != widget && gtk_widget_get_window(cont))
	{
		GtkAllocation a;

		gtk_widget_get_allocation(widget, &a);

		if ((width() != a.width || height() != a.height)
		    && a.width > 0 && a.height > 0)
		{
			//g_debug("clientHeight: %s: %d", name(), height());
			a.x = x(); a.y = y(); a.width = width(); a.height = height();
			//gt_disable_warnings(true);
			gtk_widget_size_allocate(widget, &a);
			//gt_disable_warnings(false);
			//gtk_container_resize_children(GTK_CONTAINER(widget));
		}
		//g_debug("ClientHeight: %s -> %d", this->name(), cont->allocation.height);
		gtk_widget_get_allocation(cont, &a);

		if (a.height > 0)
			return a.height;
	}
	
	if (_scroll)
		return (int)gtk_adjustment_get_page_size(gtk_scrolled_window_get_vadjustment(_scroll));
	
	return height() - getFrameWidth() * 2;
}

void gContainer::insert(gControl *child, bool realize)
{
	//fprintf(stderr, "insert %s into %s\n", child->name(), name());

	if (!gtk_widget_get_parent(child->border))
	{
		//gtk_layout_put(GTK_LAYOUT(getContainer()), child->border, 0, 0);
		gtk_container_add(GTK_CONTAINER(getContainer()), child->border);
	}
	child->bufX = child->bufY = 0;
	
	g_ptr_array_add(_children, child);
	
	if (realize)
		child->visible = true;
    
	//g_debug("gContainer::insert: visible = %d", isReallyVisible());
	performArrange();
	//fprintf(stderr, "--> %d %d %d %d\n", child->x(), child->y(), child->width(), child->height());
	updateFocusChain();

	if (realize)
	{
    gtk_widget_realize(child->border);
		gtk_widget_show_all(child->border);
	}

#ifndef GTK3
	if (hasBackground() && !child->_bg_set) child->setBackground();
	if (hasForeground() && !child->_fg_set) child->setForeground();
#endif
  child->updateFont();
}

void gContainer::remove(gControl *child)
{
	g_ptr_array_remove(_children, child);
	updateFocusChain();
}


gControl *gContainer::find(int x, int y)
{
	int i;
	gControl *ch;
	
	//fprintf(stderr, "gContainer::find: %s (C %d %d %d %d) (S %d %d): %d %d\n", name(), clientX(), clientY(), clientWidth(), clientHeight(), scrollX(), scrollY(), x, y);

	x -= clientX();
	y -= clientY();
	
	if (gApplication::_button_grab != this)
	{
		if (x < 0 || y < 0 || x >= clientWidth() || y >= clientHeight())
			return NULL;
	}
	
	if (_scroll)
	{
		x += scrollX();
		y += scrollY();
	}
	
	for (i = childCount() - 1; i >= 0; i--)
	{
		ch = child(i);
		//fprintf(stderr, "test: %s %d: %d %d %d %d / %d %d\n", ch->name(), ch->isVisible(), ch->x(), ch->y(), ch->width(), ch->height(), x, y);
		if (ch->isVisible() && x >= ch->left() && y >= ch->top() && x < (ch->left() + ch->width()) && y < (ch->top() + ch->height()))
		{
			//fprintf(stderr, "--> %s\n", ch->name());
			return ch;
		}
	}
	
	//fprintf(stderr, "--> NULL\n");
	return NULL;
}


#ifndef GTK3
void gContainer::setBackground(gColor color)
{
	int i;
	gControl *ch;
	
	gControl::setBackground(color);

	for (i = 0; i < childCount(); i++)
	{
		ch = gContainer::child(i);
		if (!ch->_bg_set)
			ch->setBackground();
	}
}
#endif

#ifdef GTK3
void gContainer::updateColor()
{
	int i;

	for (i = 0; i < childCount(); i++)
		gContainer::child(i)->updateColor();
}
#endif

void gContainer::setForeground(gColor color)
{
	int i;
	gControl *ch;
	
	gControl::setForeground(color);
	
	for (i = 0; i < childCount(); i++)
	{
		ch = gContainer::child(i);
		if (!ch->_fg_set)
			ch->setForeground();
	}	
}

GtkWidget *gContainer::getContainer()
{
	return widget;
}

gControl *gContainer::findFirstFocus()
{
	int i;
	gControl *ch;
	
	for (i = 0; i < childCount(); i++)
	{
		ch = child(i);
		if (ch->isContainer())
		{
			ch = ((gContainer *)ch)->findFirstFocus();
			if (ch)
				return ch;
		}
		else
		{
			if (gtk_widget_get_can_focus(ch->widget) && !((ch->getClass() == Type_gButton) && ((gButton *)ch)->hasShortcut()))
				return ch;
		}
	}
	
	return NULL;
}

void gContainer::resize(int w, int h)
{
	if (w == bufW && h == bufH)
		return;
		
	_client_w = 0;
	_client_h = 0;
	
	gControl::resize(w, h);
	performArrange();
}

void gContainer::setVisible(bool vl)
{
	bool arr;
	
	if (vl == isVisible())
		return;
	
	arr = vl && !isVisible();
	
	gControl::setVisible(vl);
	
	if (arr)
		performArrange();
}

void gContainer::updateFocusChain()
{
	GList *chain = NULL;
	int i;
	gControl *ch;
	
	for (i = 0; i < childCount(); i++)
	{
		ch = child(i);
		if (ch->isNoTabFocus())
			continue;
		chain = g_list_prepend(chain, ch->border);
	}

	chain = g_list_reverse(chain);
	
	gtk_container_set_focus_chain(GTK_CONTAINER(widget), chain);
	
	g_list_free(chain);
}

void gContainer::updateFont()
{
	int i;

	gControl::updateFont();

	for (i = 0; i < childCount(); i++)
		child(i)->updateFont();
}

void gContainer::moveChild(gControl *child, int x, int y)
{
	GtkWidget *cont = gtk_widget_get_parent(child->border); //getContainer();
	
	if (GTK_IS_LAYOUT(cont))
		gtk_layout_move(GTK_LAYOUT(cont), child->border, x, y);
	else
		gtk_fixed_move(GTK_FIXED(cont), child->border, x, y);
}

bool gContainer::hasBackground() const
{
	return _bg_set || (parent() && parent()->hasBackground());
}

bool gContainer::hasForeground() const
{
	return _fg_set || (parent() && parent()->hasForeground());
}

void gContainer::setFullArrangement(gContainerArrangement *arr)
{
	bool locked = arrangement.locked;
	
	arrangement = *arr;
	arrangement.locked = locked;
	performArrange();
}

void gContainer::disableArrangement()
{
	if (_no_arrangement == 0)
		_did_arrangement = false;
	
	_no_arrangement++;
}

void gContainer::enableArrangement()
{
	_no_arrangement--;
	if (_no_arrangement == 0 && _did_arrangement)
		performArrange();
}

void gContainer::hideHiddenChildren()
{
	int i;
	gControl *child;
	
	for (i = 0;; i++)
	{
		child = gContainer::child(i);
		if (!child)
			break;
		if (!child->isVisible())
			gtk_widget_hide(child->border);
		else if (child->isContainer())
			((gContainer *)child)->hideHiddenChildren();
	}
}

void gContainer::reparent(gContainer *newpr, int x, int y)
{
	gControl::reparent(newpr, x, y);
	hideHiddenChildren();
}
