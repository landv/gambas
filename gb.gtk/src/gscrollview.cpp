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
#include "gapplication.h"
#include "gscrollview.h"

static void cb_scroll(GtkAdjustment *Adj,gScrollView *data)
{
	if (data->onScroll) data->onScroll(data);
}

gScrollView::gScrollView(gContainer *parent) : gContainer(parent)
{
	GtkAdjustment* Adj;

	g_typ=Type_gScrollView;
	
	_maxw = _maxh = 0;
	_timer = 0;
	_no_auto_grab = TRUE;
	_sw = _sh = 0;
	
	onScroll = NULL;
	
	border = gtk_scrolled_window_new(NULL, NULL);
	_scroll = GTK_SCROLLED_WINDOW(border);
	widget = gtk_layout_new(0, 0);
	//frame = 0;

	setScrollBar(SCROLL_BOTH);

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
	//g_signal_connect(G_OBJECT(viewport), "size-allocate", G_CALLBACK(cb_inside_resize), (gpointer)this);

	//g_timeout_add(0, (GSourceFunc)arrange_later, this);
}


void gScrollView::updateSize()
{
	int i;
	int w, h;
	int ww, hh;
	int sbsize;
	gControl *ch, *right, *bottom;
	int fw = getFrameWidth();

	sbsize = gApplication::getScrollbarSize();

	//fprintf(stderr, "sbsize = %d  fw = %d\n", sbsize, fw);

	ww = width() - fw * 2;
	hh = height() - fw * 2;

	switch (arrange())
	{
		case ARRANGE_NONE:

			w = h = 0;
			right = bottom = NULL;

			for (i = 0; i < childCount(); i++)
			{
				ch = child(i);
				if (!ch->isVisible()) continue;
				if (ch->ignore())
					continue;

				ww = ch->x() + ch->width();
				hh = ch->y() + ch->height();

				if (ww > w)
				{
					right = ch;
					w = ww;
				}

				if (hh > h)
				{
					bottom = ch;
					h = hh;
				}
			}

			w = h = 0;

			if (right)
				w = right->x() + right->width();
			if (bottom)
				h = bottom->y() + bottom->height();

			if ((w > ww) && scrollBar() & SCROLL_HORIZONTAL) //sw->horizontalScrollBarPolicy() == Qt::ScrollBarAsNeeded)
				hh -= sbsize;

			if ((h > hh) && scrollBar() & SCROLL_VERTICAL) // sw->verticalScrollBarPolicy() == Qt::ScrollBarAsNeeded)
				ww -= sbsize;

			ww = MAX(w, ww);
			hh = MAX(h, hh);

			break;

		case ARRANGE_VERTICAL:
		case ARRANGE_ROW:

			getMaxSize(0, 0, ww, 65536, &w, &h);
			if ((h > hh) && scrollBar() & SCROLL_VERTICAL)
			{
				ww -= sbsize;
				getMaxSize(0, 0, ww, 65536, &w, &hh);
			}

			break;

		case ARRANGE_HORIZONTAL:
		case ARRANGE_COLUMN:

			getMaxSize(0, 0, 65536, hh, &w, &h);
			if ((w > ww) && scrollBar() & SCROLL_HORIZONTAL)
			{
				hh -= sbsize;
				getMaxSize(0, 0, 65536, hh, &ww, &h);
			}

			break;

		case ARRANGE_FILL:

			break;
	}

	if (scrollWidth() != ww || scrollHeight() != hh)
	{
		//fprintf(stderr, "---> updateSize: %d %d (%d %d)\n", ww, hh, width(), height());
		_sw = ww;
		_sh = hh;
		gtk_widget_set_size_request(widget, -1, -1);
		gtk_widget_set_size_request(widget, ww, hh);
	}

	gContainer::performArrange();
}

void gScrollView::performArrange()
{
	updateSize();
}

void gScrollView::resize(int w, int h)
{
	if (w == bufW && h == bufH)
		return;
	
	gContainer::resize(w, h);
	updateSize();
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
	return _sw;
}

int gScrollView::scrollHeight()
{
	return _sh;
}

void gScrollView::updateCursor(GdkCursor *cursor)
{
	gContainer::updateCursor(cursor);
	
	if (childCount())
		child(0)->updateCursor(cursor);
}

void gScrollView::setBorder(bool b)
{
	gControl::setBorder(b);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(_scroll), b ? GTK_SHADOW_IN : GTK_SHADOW_NONE);
}

bool gScrollView::hasBorder() const
{
	return gtk_scrolled_window_get_shadow_type(GTK_SCROLLED_WINDOW(_scroll)) != GTK_SHADOW_NONE;
}

void gScrollView::updateScrollBar()
{
	int sb = _scrollbar;

	switch (arrange())
	{
		case ARRANGE_NONE: break;
		case ARRANGE_HORIZONTAL: case ARRANGE_COLUMN: sb &= ~SCROLL_VERTICAL; break;
		case ARRANGE_VERTICAL: case ARRANGE_ROW: sb &= ~SCROLL_HORIZONTAL; break;
		case ARRANGE_FILL: sb = SCROLL_NONE; break;
	}

	switch(sb)
	{
		case SCROLL_NONE:
			gtk_scrolled_window_set_policy(_scroll, GTK_POLICY_NEVER, GTK_POLICY_NEVER);
			break;
		case SCROLL_HORIZONTAL:
			gtk_scrolled_window_set_policy(_scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
			break;
		case SCROLL_VERTICAL:
			gtk_scrolled_window_set_policy(_scroll, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
			break;
		case SCROLL_BOTH:
			gtk_scrolled_window_set_policy(_scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
			break;
	}
}

int gScrollView::getFrameWidth()
{
	if (!hasBorder())
		return 0;

#ifdef GTK3
	GtkStyleContext *context = gtk_widget_get_style_context(border);
	GtkStateFlags state;
	GtkBorder p, b;

	state = gtk_widget_get_state_flags(border);

	gtk_style_context_save(context);
	gtk_style_context_add_class(context, GTK_STYLE_CLASS_FRAME);
	gtk_style_context_get_padding(context, state, &p);
	gtk_style_context_get_border(context, state, &b);
	gtk_style_context_restore(context);

	return p.left + b.left;
#else
	return border->style->xthickness;
#endif
}
