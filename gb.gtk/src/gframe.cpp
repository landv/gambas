/***************************************************************************

  gframe.cpp

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
#include "gframe.h"

static void cb_frame_resize (GtkWidget *wid, GtkAllocation *req, gFrame *data)
{
	gtk_widget_set_size_request(data->fr, req->width, req->height);
} 

/****************************************************************************

Panel

****************************************************************************/

void gPanel::create(void)
{
	int i;
	GtkWidget *ch, *box;
	bool doReparent = false;
	bool was_visible = isVisible();
	GdkRectangle rect;
	int bg, fg;
	gControl *nextSibling;
	
	if (border)
	{
		getGeometry(&rect);
		bg = background();
		fg = foreground();
		nextSibling = next();
		parent()->remove(this);
		
		for (i = 0; i < childCount(); i++)
		{
			ch = child(i)->border;
			g_object_ref(G_OBJECT(ch));
			gtk_container_remove(GTK_CONTAINER(widget), ch);
		}
		
		_no_delete = true;
		gtk_widget_destroy(border);
		_no_delete = false;
		doReparent = true;
	}
	
	if (_bg_set)
	{
		border = gtk_event_box_new();
		widget = gtk_fixed_new();
		box = widget;
		//gtk_widget_set_app_paintable(border, TRUE);
		//gtk_widget_set_app_paintable(box, TRUE);
	}
	else
	{
		border = widget = gtk_fixed_new();
		box = NULL;
	}

	frame = widget;
	realize(true);
	
	//g_signal_connect(G_OBJECT(border), "size-allocate", G_CALLBACK(cb_size), (gpointer)this);
	//g_signal_connect(G_OBJECT(border), "expose-event", G_CALLBACK(cb_expose), (gpointer)this);
	
	if (doReparent)
	{
		if (box)
			gtk_widget_realize(box);
		
		setNext(nextSibling);
		setBackground(bg);
		setForeground(fg);
		updateFont();
		bufX = bufY = bufW = bufH = -1;
		setGeometry(&rect);
		
		for (i = 0; i < childCount(); i++)
		{
			ch = child(i)->border;
			gtk_container_add(GTK_CONTAINER(widget), ch);
			moveChild(child(i), child(i)->x(), child(i)->y());
			g_object_unref(G_OBJECT(ch));
		}
		
		if (was_visible)
			show();
		else
			hide();
		
		//gApplication::checkHoveredControl(this);
		
		if (_inside)
		{
			_inside = false;
			if (gApplication::_enter == this)
				gApplication::_enter = NULL;
			gApplication::_ignore_until_next_enter = this;
		}
	}
}

gPanel::gPanel(gContainer *parent) : gContainer(parent)
{
	g_typ = Type_gPanel;
	border = NULL;
	create();
}

void gPanel::setBackground(gColor color)
{
	bool set = _bg_set;
	
	gContainer::setBackground(color);
	
	if (set != _bg_set)
		create();
}

/****************************************************************************

Frame

****************************************************************************/

gFrame::gFrame(gContainer *parent) : gContainer(parent)
{
	g_typ=Type_gFrame;

	border = widget = gtk_fixed_new();
	
	fr = gtk_frame_new(NULL);
	//label = gtk_frame_get_label_widget(GTK_FRAME(fr));
	gtk_container_set_border_width(GTK_CONTAINER(fr),0);
	gtk_frame_set_shadow_type(GTK_FRAME(fr), GTK_SHADOW_ETCHED_IN);
	gtk_container_add(GTK_CONTAINER(widget), fr);
	
  realize(false);

	g_signal_connect(G_OBJECT(border), "size-allocate", G_CALLBACK(cb_frame_resize), (gpointer)this);
}

const char *gFrame::text()
{
	const char *label = gtk_frame_get_label(GTK_FRAME(fr));
	if (!label)
		label = "";
	return label;
}

void gFrame::setText(const char *vl)
{
	if (!vl)
		vl = "";

	gtk_frame_set_label(GTK_FRAME(fr), vl);

	/*bool remove = false;
	
	remove = !vl || !*vl;
	
	if (remove)
	{
		if (label) 
		{
			gtk_frame_set_label_widget(GTK_FRAME(fr), NULL);
			label = NULL;
		}
		return;
	}
	
	if (!label)
	{
		label = gtk_label_new(vl);
		gtk_frame_set_label_widget(GTK_FRAME(fr), label);
		updateFont();
		setForeground(foreground());
		gtk_widget_show(label);
	}
	else
		gtk_label_set_text(GTK_LABEL(label), (const gchar*)vl);*/
}

void gFrame::updateFont()
{
	GtkWidget *label = gtk_frame_get_label_widget(GTK_FRAME(fr));
	gContainer::updateFont();
	if (label)
		gtk_widget_modify_font(label, font()->desc());
	performArrange();
}

#ifdef GTK3
void gFrame::updateColor()
{
	gContainer::updateColor();
	gt_widget_set_color(fr, FALSE, background());
}
#else
void gFrame::setRealForeground(gColor color)
{
	gControl::setRealForeground(color);
	//if (label) set_gdk_fg_color(label, color);
}
#endif

int gFrame::containerX()
{
	return gApplication::getFrameWidth();
}

int gFrame::clientX()
{
	return 0;
}

int gFrame::clientWidth()
{
	return width(); // - gApplication::getFrameWidth() * 2;
}

int gFrame::containerY()
{
	int y = gApplication::getFrameWidth();

	if (*text())
		y = font()->height() * 3 / 2;

	return y;
}

int gFrame::clientY()
{
	return 0;
}

int gFrame::clientHeight()
{
	return height(); // - clientY() - gApplication::getFrameWidth();
}
