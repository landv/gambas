/***************************************************************************

  gscrollview.cpp

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
#include "gscrollview.h"

static void cb_scroll(GtkAdjustment *Adj,gScrollView *data)
{
	if (data->onScroll) data->onScroll(data);
}

/*static gboolean arrange_later(gScrollView *data)
{
	fprintf(stderr, "arrange_later\n");
	data->performArrange();
	data->_timer = 0;
	return false;
}*/

static void cb_inside_resize(GtkWidget *wid, GtkAllocation *a, gScrollView *data)
{
	data->performArrange();
	/*if (!data->_timer)
		data->_timer = g_timeout_add(50, (GSourceFunc)arrange_later, data);*/
}

gScrollView::gScrollView(gContainer *parent) : gContainer(parent)
{
	GtkAdjustment* Adj;

	g_typ=Type_gScrollView;
	
	_maxw = _maxh = 0;
	_mw = _mh = 0;
	_timer = 0;
	_no_auto_grab = TRUE;
	
	onScroll = NULL;
	
	border = gtk_scrolled_window_new(NULL, NULL);
	_scroll = GTK_SCROLLED_WINDOW(border);
	widget = gtk_layout_new(0, 0);
	//frame = 0;

	gtk_scrolled_window_set_policy(_scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  viewport = gtk_viewport_new(gtk_scrolled_window_get_hadjustment(_scroll), gtk_scrolled_window_get_vadjustment(_scroll));
  
  gtk_viewport_set_shadow_type(GTK_VIEWPORT(viewport), GTK_SHADOW_NONE);
  gtk_container_add(GTK_CONTAINER(_scroll), viewport);
	gtk_container_add(GTK_CONTAINER(viewport), widget);
	
	realize(false);

	setBorder(true);
	
	Adj=gtk_scrolled_window_get_hadjustment(_scroll);
	g_signal_connect(G_OBJECT(Adj),"value-changed",G_CALLBACK(cb_scroll),this);
	Adj=gtk_scrolled_window_get_vadjustment(_scroll);
	g_signal_connect(G_OBJECT(Adj),"value-changed",G_CALLBACK(cb_scroll),this);
	g_signal_connect(G_OBJECT(viewport), "size-allocate", G_CALLBACK(cb_inside_resize), (gpointer)this);
}


void gScrollView::updateSize()
{
	int i, p;
	int ww, hh;
	gControl *ch;
	
	ww = hh = 0;
	
	for (i = 0; i < childCount(); i++)
	{
		ch = child(i);
		if (!ch->isVisible()) continue;
		
		p = ch->left() + ch->width();
		if (p > ww)
			ww = p;
			
		p = ch->top() + ch->height();
		if (p > hh)
			hh = p;
	}
	
	_maxw = ww;
	_maxh = hh;

	gtk_widget_set_size_request(widget, scrollWidth(), scrollHeight());
}

void gScrollView::performArrange()
{
	gContainer::performArrange();
	updateSize();
}

void gScrollView::resize(int w, int h)
{
	if (w == bufW && h == bufH)
		return;
	
	gContainer::resize(w, h);
	
	//if (bufW == 0 || bufH == 0)
	//	return;
	
	updateSize();
	
	//performArrange();
	//parent()->performArrange();
}

void gScrollView::ensureVisible(int x, int y, int w, int h)
{
	if (!_scroll)
		return;
	
	GtEnsureVisible arg;
	
	arg.clientWidth = clientWidth();
	arg.clientHeight = clientHeight();
	arg.scrollX = scrollX();
	arg.scrollY = scrollY();
	arg.scrollWidth = scrollWidth();
	arg.scrollHeight = scrollHeight();
	
	gt_ensure_visible(&arg, x, y, w, h);
	
	scroll(arg.scrollX, arg.scrollY);
}

int gScrollView::scrollWidth()
{
	return MAX(_maxw, clientWidth());
}

int gScrollView::scrollHeight()
{
	return MAX(_maxh, clientHeight());
}

void gScrollView::updateCursor(GdkCursor *cursor)
{
	gContainer::updateCursor(cursor);
	
	if (childCount())
		child(0)->updateCursor(cursor);
}