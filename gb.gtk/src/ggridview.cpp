/***************************************************************************

  ggridview.cpp

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
#include "tablerender.h"
#include "ggridview.h"

class gGridHeader
{
public:
	char *header;
	char *footer;
	bool resizable;

	gGridHeader();
	~gGridHeader();
	void setHeader(const char *vl);
	void setFooter(const char *vl);
};

class gGridRow
{
public:
	char *text;
	bool resizable;

	gGridRow();
	~gGridRow();
	void setText(const char *vl);
};


static char void_row_buffer[16];

/**************************************************************************

gGridHeader

***************************************************************************/
gGridHeader::gGridHeader()
{
	resizable=true;
	header=NULL;
	footer=NULL;
}

gGridHeader::~gGridHeader()
{
	if (header) g_free(header);
	if (footer) g_free(footer);
}

void gGridHeader::setHeader(const char *vl)
{
	if (!vl) vl="";
	if (header) g_free(header);
	header = g_strdup(vl);
}

void gGridHeader::setFooter(const char *vl)
{
	if (!vl) vl="";
	if (footer) g_free(footer);
	footer = g_strdup(vl);
}

/**************************************************************************

gGridRow

***************************************************************************/

gGridRow::gGridRow()
{
	resizable=false;
	text=NULL;
}

gGridRow::~gGridRow()
{
	if (text) g_free(text);
}

void gGridRow::setText(const char *vl)
{
	if (!vl) vl="";
	if (text) g_free(text);
	text = g_strdup(text);
}


/**************************************************************************

gGridView

***************************************************************************/

static void gGridView_hbar(GtkAdjustment *adj,gGridView *data)
{
	gtk_widget_queue_draw(data->header);
	gtk_widget_queue_draw(data->footer);

	data->render->setOffsetX((int)gtk_adjustment_get_value (adj));
	if (data->onScroll) data->onScroll(data);
}

static void gGridView_vbar(GtkAdjustment *adj,gGridView *data)
{
	gtk_widget_queue_draw(data->lateral);
	
	data->render->setOffsetY((int)gtk_adjustment_get_value (adj));
	if (data->onScroll) data->onScroll(data);
}

static gboolean gGridView_configure(GtkWidget *w,GdkEventConfigure *e,gGridView *data)
{
	data->updateLastColumn();
	data->calculateBars();
	return false;
}

/*gboolean tbheader_leave(GtkWidget *wid,GdkEventCrossing *e,gGridView *data)
{
	data->mouse_x=-1;
	gdk_window_set_cursor(wid->window,NULL);

	return false;
}

gboolean tblateral_leave(GtkWidget *wid,GdkEventCrossing *e,gGridView *data)
{
	data->mouse_y=-1;
	gdk_window_set_cursor(wid->window,NULL);

	return false;
}*/

static gboolean tbheader_move(GtkWidget *wid, GdkEventMotion *e, gGridView *data)
{
	int pos;
	int width, min;
	
	pos = (int)e->x + data->scrollX();
	
	if (e->state & GDK_BUTTON1_MASK)
	{
		if (data->_index >= 0)
		{
			min = data->minColumnWidth(data->_index);
			width = pos - data->columnPos(data->_index);
			width = MAX(width, min);
			if (data->_index == (data->columnCount() - 1))
				data->_last_col_width = 0;
			data->setColumnWidth(data->_index, width);
		}
	}
	else
	{
		data->_index = data->findColumnSeparation(pos);
		if (data->_index < 0)
			gdk_window_set_cursor(wid->window, NULL);
		else
			gdk_window_set_cursor(wid->window, gdk_cursor_new(GDK_SB_H_DOUBLE_ARROW));	
	}
	
	return true;
}


static void tblateral_select(gGridView *data, int bcurrent, bool move)
{
	int bc, col;
	bool sel_changed;
	
	data->getCursor(NULL, &col);
	
	switch(data->selectionMode())
	{
		case SELECT_NONE:
			return;
		
		case SELECT_SINGLE:
			if (!data->rowSelected(bcurrent))
			{
				data->setCursor(bcurrent, col);
				data->emit(SIGNAL(data->onSelect));
			}
			break;
			
		case SELECT_MULTIPLE:
			if (!move)
			{
				data->sel_row = bcurrent;
				data->setCursor(bcurrent, col);
				if (data->sel_current != bcurrent)
				{
					data->sel_current = bcurrent;
					data->clearSelection();
					data->setRowSelected(bcurrent, true);
					data->emit(SIGNAL(data->onSelect));
				}
			}
			else
			{
				//data->clearSelection();
				if (bcurrent < data->sel_row) 
				{
					if (data->sel_current > data->sel_row)
					{
						data->clearSelection();
						data->sel_current = data->sel_row;
					}
						
					if (bcurrent < data->sel_current)
					{
						for (bc = bcurrent; bc <= data->sel_current; bc++)
							data->setRowSelected(bc, true);
						sel_changed = true;
					}
					else
					{
						for (bc = data->sel_current; bc < bcurrent; bc++)
							data->setRowSelected(bc, false);
						sel_changed = true;
					}
				}
				else
				{
					if (data->sel_current < data->sel_row)
					{
						data->clearSelection();
						data->sel_current = data->sel_row;
					}
						
					if (bcurrent >= data->sel_current)
					{
						for (bc = data->sel_current; bc <= bcurrent; bc++)
							data->setRowSelected(bc, true);
					}
					else
					{
						for (bc = bcurrent + 1; bc <= data->sel_current; bc++)
							data->setRowSelected(bc, false);
					}
				}
				
				if (data->sel_current != bcurrent)
				{
					data->sel_current = bcurrent;
					data->emit(SIGNAL(data->onSelect));
				}
				
				data->setCursor(bcurrent, col);
				//data->ensureVisible(bcurrent, -1);
			}
			break;
	}
}

static gboolean tblateral_press(GtkWidget *wid,GdkEventButton *e,gGridView *data)
{
	//int realmode;
	int pos;
	int bcurrent = -1;
	bool is_lateral = (wid == data->lateral);
	
	pos = (int)e->y + data->scrollY();
	
	if (e->button != 1 || (is_lateral && data->findRowSeparation(pos) >= 0))
		return false;

	data->_index = -1;
	
	bcurrent = data->findRow(pos);
	if (bcurrent < 0) return false;

	tblateral_select(data, bcurrent, false);

	if (!is_lateral)
	{
		int row = bcurrent;
		int col = data->render->findVisibleColumn((int)e->x);
	
		data->setCursor(row, col);
	}

	return false;
}

static gboolean lateral_do_move(gGridView *data)
{
	int min, height, pos;

 	pos = data->mouse_pos + data->scrollY();

	min = data->minRowHeight(data->_index);
	height = pos - data->rowPos(data->_index);
	height = MAX(height, min);
	data->setRowHeight(data->_index, height);
	
	return true;
}

static gboolean contents_do_move(gGridView *data)
{
	int index, mode, pos;
	
	pos = data->mouse_pos + data->scrollY();
	
	index = data->findRow(pos);
	mode = data->selectionMode();
	
	if (index >= 0)
		tblateral_select(data, index, true);
		
	return true;
}

static gboolean tblateral_move(GtkWidget *wid, GdkEventMotion *e, gGridView *data)
{
	int pos;
	bool is_lateral = (wid == data->lateral);
	
	data->mouse_pos = (int)e->y;
	pos = data->mouse_pos + data->scrollY(); 
	
	if (e->state & GDK_BUTTON1_MASK)
	{
		if (is_lateral && data->_index >= 0)
			lateral_do_move(data);
		else
			data->startScrollTimer((GSourceFunc)contents_do_move);
	}
	else if (is_lateral)
	{
		data->_index = data->findRowSeparation(pos);
		if (data->_index < 0)
			gdk_window_set_cursor(wid->window, NULL);
		else
			gdk_window_set_cursor(wid->window, gdk_cursor_new(GDK_SB_V_DOUBLE_ARROW));	
	}
	
	return false;
}

static gboolean tblateral_release(GtkWidget *wid,GdkEventButton *e,gGridView *data)
{
	int pos;
	int bcurrent = -1;
	bool is_lateral = (wid == data->lateral);
	
	pos = (int)e->y + data->scrollY();
	
	data->stopScrollTimer();
	
	if (e->button != 1 || data->_index >= 0)
		return false;

	bcurrent = data->findRow(pos);
	if (bcurrent < 0) return false;

	tblateral_select(data, bcurrent, true);

	if (is_lateral)
	{
		data->emit(SIGNAL(data->onRowClick), bcurrent);	
		data->getCursor(NULL, &pos);
		data->setCursor(bcurrent, pos);
	}
	else
	{
		int row = bcurrent;
		int col = data->render->findVisibleColumn((int)e->x);
	
		data->setCursor(row, col);
		if (data->onClick)
			data->onClick(data, row, col);
	}
	
	return false;
}


static gboolean cb_widget_expose(GtkWidget *wid, GdkEventExpose *e, gGridView *data)
{
	int w, h, f;
	
	if (data->headersVisible() == 3)
	{
		GdkRectangle rect;
		
		f = data->getFrameWidth();
		w = data->lateralWidth();	
		h = data->headerHeight();
		
		rect.x = f;
		rect.y = f;
		rect.width = w;
		rect.height = h;
		
		gtk_paint_box(data->header->style,wid->window,
				GTK_STATE_NORMAL, GTK_SHADOW_OUT,
				&rect, wid->parent, NULL, // "buttondefault"
				0, 0, w + f, h + f);
	}
	
	if (data->footersVisible() && data->headersVisible() & 2)
	{
		GdkRectangle rect;
		
		f = data->getFrameWidth();
		w = data->lateralWidth();	
		h = data->footerHeight();
		
		rect.x = f;
		rect.y = data->height() - h - f;
		rect.width = w;
		rect.height = h;
		
		gtk_paint_box(data->header->style,wid->window,
				GTK_STATE_NORMAL, GTK_SHADOW_OUT,
				&rect, wid->parent, NULL, // "buttondefault"
				0, rect.y, w + f, h + f);
	}
			
	return false;
}

static gboolean tbheader_expose(GtkWidget *wid,GdkEventExpose *e,gGridView *data)
{
	GtkCellRenderer *rend=NULL;
	GdkRectangle rect={0,0,0,0};
	GdkGC *gc;
	char *buf;
	int bpos;
	int pw,ph,maxw;
	bool footer;
	int bc;
	
	footer = wid->window == data->footer->window;

	gc=gdk_gc_new(wid->window);
	gdk_gc_set_clip_origin(gc,0,0);
	gdk_gc_set_clip_rectangle(gc,&e->area);
	gdk_drawable_get_size(wid->window,&maxw,&ph);

	if (footer)
	{
		gtk_paint_box(wid->style, wid->window,
				GTK_STATE_NORMAL, GTK_SHADOW_OUT,
				NULL, wid->parent, "button", // "buttondefault"
				-4, 0, maxw + 8, ph + 4);
	}
	else
	{
		gtk_paint_box(wid->style, wid->window,
				GTK_STATE_NORMAL, GTK_SHADOW_OUT,
				NULL, wid->parent, "button", // "buttondefault"
				-4, -4, maxw + 8, ph + 4);
	}
	 
/*	if (data->render->firstCol == 0)
	{
		gtk_paint_vline(data->header->style, wid->window,
			GTK_STATE_NORMAL,
			NULL, wid->parent, NULL,
			0, ph - 1, 0);
	}*/
	 
	bpos = data->render->offCol - data->render->getOffsetX();
	for (bc = data->render->firstCol; bc < data->columnCount(); bc++)
	{
		//if ((bpos+data->columnWidth(bc)) < 0)  { bpos+=data->columnWidth(bc); continue; }
		if (bpos > maxw) 
			break;
		pw=data->columnWidth(bc);
		
		if (footer)
		{
			gtk_paint_vline(wid->style, wid->window,
				GTK_STATE_NORMAL,
				NULL, wid->parent, NULL,
				4, ph - 3, bpos + pw - 1);
		}
		else
		{
			gtk_paint_vline(wid->style, wid->window,
				GTK_STATE_NORMAL,
				NULL, wid->parent, NULL,
				3, ph - 4, bpos + pw - 1);
		}
			
		
		if (footer) buf=data->footerText(bc);
		else        buf=data->headerText(bc);
		if ( buf != NULL )
		{
			if (!rend) rend=gtk_cell_renderer_text_new();
			g_object_set(G_OBJECT(rend),
				"text", buf,
				"xalign", 0.5,
				"yalign", 0.5,
				"font-desc", data->font()->desc(),
				(void *)NULL);
			rect.x=bpos;
			rect.y=0;
			rect.width=pw-1;
			rect.height=ph-1;

			gtk_cell_renderer_render(GTK_CELL_RENDERER(rend),wid->window,wid,&rect,&rect,NULL,(GtkCellRendererState)0);
		}

		bpos += data->columnWidth(bc);
	}
	
	g_object_unref(G_OBJECT(gc));
	return false;
}

static gboolean tblateral_expose(GtkWidget *wid,GdkEventExpose *e,gGridView *data)
{
	GtkCellRenderer *rend=NULL;
	GdkRectangle rect={0,0,0,0};
	GdkGC *gc;
	GdkWindow *win=data->lateral->window;
	int pw,ph,maxh;
	int bc;
	int bpos;
	
	gc=gdk_gc_new(win);
	gdk_gc_set_clip_origin(gc,0,0);
	gdk_gc_set_clip_rectangle(gc,&e->area);
	gdk_drawable_get_size(win,&ph,&maxh);

	gtk_paint_box(wid->style, wid->window,
			GTK_STATE_NORMAL, GTK_SHADOW_OUT,
			NULL, wid->parent, "button", // "buttondefault"
			-4, -4, ph + 4, maxh + 8);

/*	if (data->render->firstRow == 0)
	{
		gtk_paint_hline(data->header->style, wid->window,
			GTK_STATE_NORMAL,
			NULL, wid->parent, NULL,
			0, ph - 1, 0);
	}*/
	 
	bpos = data->render->offRow - data->render->getOffsetY(); 
	for (bc=data->render->firstRow; bc<data->rowCount(); bc++)
	{
		if (bpos > maxh) break;
		pw=data->rowHeight(bc);
/*		gtk_paint_box(data->lateral->style,data->lateral->window,
			 GTK_STATE_NORMAL, GTK_SHADOW_NONE,
			 NULL, data->lateral->parent, "buttondefault",
			 0,bpos, ph-1, pw-1);
		gtk_paint_shadow(data->lateral->style,data->lateral->window,
			 GTK_STATE_NORMAL, GTK_SHADOW_NONE,
			 NULL, data->lateral->parent, "buttondefault",
			 0,bpos, ph-1, pw-1);*/
		
		gtk_paint_hline(wid->style, wid->window,
			GTK_STATE_NORMAL,
			NULL, wid->parent, NULL,
			3, ph - 4, bpos + pw - 1);
		
		if (!rend) rend=gtk_cell_renderer_text_new();
		
		g_object_set(G_OBJECT(rend),
			"text", data->rowText(bc),
			"xalign", 0.5,
			"yalign", 0.5,
			"font-desc", data->font()->desc(),
			(void *)NULL);
		
		rect.x = 0;
		rect.y = bpos;
		rect.width = ph-1;
		rect.height = pw-1;

		gtk_cell_renderer_render(GTK_CELL_RENDERER(rend),wid->window,wid,
																								&rect,&rect,NULL,(GtkCellRendererState)0);

		bpos+=data->rowHeight(bc);
	
	}

	g_object_unref(G_OBJECT(gc));

	return false;
}

static gboolean tbheader_release(GtkWidget *wid,GdkEventButton *e,gGridView *data)
{
	int bc,bcurrent=-1;
	bool bfooter=false;
	char *buf=NULL;
	
	if (e->button != 1 || data->_index >= 0) return false;

	bcurrent = data->findColumn((int)e->x + data->scrollX());
	if (bcurrent<0) return false;

	g_object_get(G_OBJECT(wid),"name",&buf,(void *)NULL);
	if (buf)
	{
		if (!strcmp(buf,"gambas-grid-footer")) bfooter=true;
		g_free(buf); buf=NULL;
	}

	data->getCursor(&bc,NULL);
	data->setCursor(bc,bcurrent);

	if (bfooter)
		{ if (data->onFooterClick) data->onFooterClick(data,bcurrent); }
	else
		{ if (data->onColumnClick) data->onColumnClick(data,bcurrent); }
	
	return TRUE;

}

static gboolean cb_contents_button_press(GtkWidget *wid, GdkEventButton *e, gGridView *data)
{
	int row, col;
	
	if (e->type != GDK_2BUTTON_PRESS) 
		return false;

	if (e->button != 1) 
		return false;

	data->getCursor(&row, &col);
	
	if (data->onActivate) data->onActivate(data, row, col);
	return false;
}

static gboolean cb_scroll(GtkWidget *wid, GdkEventScroll *e, gGridView *data)
{
	GtkAdjustment *adj;
	gdouble step;
	
	if (e->direction == GDK_SCROLL_UP || e->direction == GDK_SCROLL_DOWN)
		adj = gtk_range_get_adjustment(GTK_RANGE(data->vbar));
	else
		adj = gtk_range_get_adjustment(GTK_RANGE(data->hbar));
	
	g_object_get(G_OBJECT(adj), "step-increment", &step, (void *)NULL);
	
	switch (e->direction)
	{
		case GDK_SCROLL_UP: data->setScrollY(data->scrollY() - (int)step); break;
		case GDK_SCROLL_DOWN: data->setScrollY(data->scrollY() + (int)step); break;
		case GDK_SCROLL_LEFT: data->setScrollX(data->scrollX() - (int)step); break;
		case GDK_SCROLL_RIGHT:  data->setScrollY(data->scrollX() + (int)step); break;
	}
	
	return TRUE;
}

static gboolean cb_keypress(GtkWidget *wid,GdkEventKey *e,gGridView *data)
{
	int row,col;
	bool bchange=false;

	data->getCursor(&row,&col);

	switch(e->keyval)
	{
		case GDK_Up:
			if (row>0) { row--; bchange=true; }
			break; 

		case GDK_Down:
			if (row<(data->rowCount()-1)) { row++; bchange=true; }
			break;

		case GDK_Left:
			if (col>0) { col--; bchange=true; }
			break; 

		case GDK_Right:
			if (col<(data->columnCount()-1)) { col++; bchange=true; }
			break;
	}

	if (bchange)
	{
		data->ensureVisible(row,col);
		data->setCursor(row,col);
		return TRUE;
	}

	return false;
}

gGridView::gGridView(gContainer *parent) : gControl(parent)
{
	GtkAdjustment *adj;

	g_typ = Type_gGridView;

	onChange = NULL;
	onActivate = NULL;
	onSelect = NULL;
	onScroll = NULL;
	onClick = NULL;
	onColumnClick = NULL;
	onFooterClick = NULL;
	onRowClick = NULL;
	onColumnResize = NULL;
	onRowResize = NULL;

	use_base = true;
	_index = -1;
	cursor_col=-1;
	cursor_row=-1;
	sel_row=-1;
	sel_mode=0;
	sel_current = -1;
	scroll=3;
	hdata=NULL;
	vdata=NULL;
	_last_col_width = 0;
	scroll_timer = 0;
	_updating_last_column = false;

	border=gtk_event_box_new();
	widget=gtk_table_new(3,3,FALSE);
	//gtk_container_set_border_width  (GTK_CONTAINER(widget),1);

	//gtk_container_add(GTK_CONTAINER(border),widget);
	hbar=gtk_hscrollbar_new(NULL);
	vbar=gtk_vscrollbar_new(NULL);
	
	contents = gtk_drawing_area_new();
	gtk_widget_add_events(contents, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
	
	header=gtk_drawing_area_new();
	footer=gtk_drawing_area_new();
	lateral=gtk_drawing_area_new();
	//gtk_widget_set_size_request(header,0,20);
	//gtk_widget_set_size_request(footer,0,20);
	//gtk_widget_set_size_request(lateral,16,0);

	gtk_table_attach_defaults(GTK_TABLE(widget),contents,1,2,1,2);
	gtk_table_attach(GTK_TABLE(widget),header,1,2,0,1,GTK_FILL,GTK_FILL,0,0);
	gtk_table_attach(GTK_TABLE(widget),footer,1,2,2,3,GTK_FILL,GTK_FILL,0,0);
	gtk_table_attach(GTK_TABLE(widget),lateral,0,1,1,2,GTK_FILL,GTK_FILL,0,0);
	gtk_table_attach(GTK_TABLE(widget),hbar,0,2,3,4,GTK_FILL,GTK_FILL,0,0);
	gtk_table_attach(GTK_TABLE(widget),vbar,2,3,0,3,GTK_FILL,GTK_FILL,0,0);
	
	render=new gTableRender(this);

	_no_default_mouse_event = true;
	realize(true);

	gtk_widget_add_events(border,GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_widget_add_events(contents,GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
	gtk_widget_add_events(header,GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);
	gtk_widget_add_events(footer,GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);
	gtk_widget_add_events(lateral,GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);	
	gtk_widget_add_events(header,GDK_BUTTON_PRESS_MASK | GDK_BUTTON1_MOTION_MASK);
	gtk_widget_add_events(footer,GDK_BUTTON_PRESS_MASK | GDK_BUTTON1_MOTION_MASK);
	gtk_widget_add_events(lateral,GDK_BUTTON_PRESS_MASK | GDK_BUTTON1_MOTION_MASK);
	gtk_widget_add_events(header,GDK_BUTTON_RELEASE_MASK);
	gtk_widget_add_events(footer,GDK_BUTTON_RELEASE_MASK);
	gtk_widget_add_events(lateral,GDK_BUTTON_RELEASE_MASK);
	
	g_object_set(G_OBJECT(header),"name","gambas-grid-header",(void *)NULL);
	g_object_set(G_OBJECT(footer),"name","gambas-grid-footer",(void *)NULL);

	g_signal_connect_after(G_OBJECT(border),"size-allocate",G_CALLBACK(gGridView_configure),(void*)this);
	g_signal_connect(G_OBJECT(border), "key-press-event", G_CALLBACK(cb_keypress), this);
	g_signal_connect(G_OBJECT(border), "scroll-event", G_CALLBACK(cb_scroll), this);
	
	g_signal_connect(G_OBJECT(header),"expose-event",G_CALLBACK(tbheader_expose),this);
	g_signal_connect(G_OBJECT(header),"motion-notify-event",G_CALLBACK(tbheader_move),this);
	g_signal_connect(G_OBJECT(header),"button-release-event",G_CALLBACK(tbheader_release),this);
	
	g_signal_connect(G_OBJECT(footer),"expose-event",G_CALLBACK(tbheader_expose),this);
	g_signal_connect(G_OBJECT(footer),"motion-notify-event",G_CALLBACK(tbheader_move),this);
	g_signal_connect(G_OBJECT(footer),"button-release-event",G_CALLBACK(tbheader_release),this);
	
	g_signal_connect(G_OBJECT(lateral),"expose-event",G_CALLBACK(tblateral_expose),this);
	g_signal_connect(G_OBJECT(lateral),"motion-notify-event",G_CALLBACK(tblateral_move),this);
	g_signal_connect(G_OBJECT(lateral),"button-press-event",G_CALLBACK(tblateral_press),this);
	g_signal_connect(G_OBJECT(lateral),"button-release-event",G_CALLBACK(tblateral_release),this);
	
	g_signal_connect(G_OBJECT(widget),"expose-event",G_CALLBACK(cb_widget_expose),this);
		
	g_signal_connect_after(G_OBJECT(contents),"event",G_CALLBACK(cb_contents_button_press),this);
	g_signal_connect(G_OBJECT(contents),"motion-notify-event",G_CALLBACK(tblateral_move),this);
	
	g_signal_connect(G_OBJECT(contents),"button-release-event",G_CALLBACK(gcb_button_release),(gpointer)this);
	g_signal_connect(G_OBJECT(contents),"button-press-event",G_CALLBACK(gcb_button_press),(gpointer)this);
	
	g_signal_connect(G_OBJECT(contents),"button-press-event",G_CALLBACK(tblateral_press),this);
	g_signal_connect(G_OBJECT(contents),"button-release-event",G_CALLBACK(tblateral_release),this);
	

	//g_signal_connect(G_OBJECT(contents),"event",G_CALLBACK(gridview_release),this);

	g_object_set(G_OBJECT(vbar),"visible",FALSE,(void *)NULL);
	g_object_set(G_OBJECT(hbar),"visible",FALSE,(void *)NULL);
	g_object_set(G_OBJECT(footer),"visible",FALSE,(void *)NULL);
	g_object_set(G_OBJECT(widget),"can-focus",TRUE,(void *)NULL);

	gtk_widget_realize(footer);
	gtk_widget_realize(contents);

	adj=gtk_range_get_adjustment(GTK_RANGE(hbar));
	g_signal_connect(G_OBJECT(adj),"value-changed",G_CALLBACK(gGridView_hbar),(void*)this);
	adj=gtk_range_get_adjustment(GTK_RANGE(vbar));
	g_signal_connect(G_OBJECT(adj),"value-changed",G_CALLBACK(gGridView_vbar),(void*)this);
	
	// BM to see the result
	//GtkStyle *st = gt_get_style("GtkEntry", GTK_TYPE_ENTRY);
	//gtk_widget_set_style(contents, st);
	//gtk_widget_modify_base(contents, GTK_STATE_NORMAL, &st->bg[GTK_STATE_NORMAL]);
	//gtk_widget_modify_bg(contents, GTK_STATE_NORMAL, &st->base[GTK_STATE_NORMAL]);
	
	setBackground();
	setForeground();
	setHeadersVisible(0);
	setFootersVisible(false);
	setBorder(true);
	setDrawGrid(true);
}

gGridView::~gGridView()
{
	setRowCount(0);
	setColumnCount(0);
	if (hdata) g_hash_table_destroy(hdata);
	if (vdata) g_hash_table_destroy(vdata);
	delete render;
}

int gGridView::selectionMode()
{
	return sel_mode;
}

void gGridView::setSelectionMode(int vl)
{
	if (vl<0) return;
	if (vl>2) return;

	sel_mode=vl;
	if (sel_mode == SELECT_NONE)
		clearSelection();
	else if (sel_mode == SELECT_SINGLE)
		setRowSelected(cursor_row, true);
}

void gGridView::setDataFunc(void *func,void *data)
{
	render->voidCell=(void(*)(gTableData *,int,int,void *))func;
	render->userData=data;
}


void gGridView::queryUpdate(int row,int col)
{
	render->queryUpdate(row,col);
}

void gGridView::calculateBars()
{
	GtkAdjustment *adj;
	int obh,obv,px,py,ovw,ovh,vw,vh,rw,rh;
	bool bh, bv;
	GtkRequisition req;

	gtk_widget_size_request(hbar, &req);
	py = req.height;
	gtk_widget_size_request(vbar, &req);
	px = req.width;
	
	obh = bh = GTK_WIDGET_VISIBLE(hbar);
	obv = bv = GTK_WIDGET_VISIBLE(vbar);
	
	vw = render->visibleWidth(); // + (obh ? px : 0);
	vh = render->visibleHeight(); // + (obv ? py : 0);
		
	rw = render->width();
	rh = render->height();
	
	if (vw > 1 && vh > 1)
	{
		ovw = vw + (obv ? px : 0);
		ovh = vh + (obh ? py : 0);
		
		//fprintf(stderr, "ovw = %d ovh = %d\n", ovw, ovh);
		
		obv = false;
		obh = false;
	
		for(;;)
		{
			vw = ovw - (obv ? px : 0);
			vh = ovh - (obh ? py : 0);
			
			//fprintf(stderr, "rw = %d rh = %d / vw = %d vh = %d / bh = %d bv = %d\n", rw, rh, vw, vh, bh, bv);
			
			bh = vw < rw && rw > 0 && (scroll & SCROLL_HORIZONTAL);
			if (bh != obh)
			{
				obh = bh;
				continue;
			}
				
			bv = vh < rh && rh > 0 && (scroll & SCROLL_VERTICAL);
			if (bv != obv)
			{
				obv = bv;
				continue;
			}	
			break;
		}
	}
	else
	{
		bh = false;
		bv = false;
	}
	
	if (bh != GTK_WIDGET_VISIBLE(hbar))
	{
		g_object_set(G_OBJECT(hbar),"visible",bh,(void *)NULL);
		if (!bh)
			setScrollX(0);
	}
	if (bv != GTK_WIDGET_VISIBLE(vbar))
	{
		g_object_set(G_OBJECT(vbar),"visible",bv,(void *)NULL);
		if (!bv)
			setScrollY(0);
	}
	
	if (bh) 
	{
		gtk_range_set_range(GTK_RANGE(hbar),0,render->width());
		gtk_range_set_increments(GTK_RANGE(hbar),render->getColumnSize(0),vw);
		adj=gtk_range_get_adjustment(GTK_RANGE(hbar));
		g_object_set(G_OBJECT(adj),"page-size",(gfloat)vw,(void *)NULL);
	}

	if (bv)
	{
		gtk_range_set_range(GTK_RANGE(vbar),0,render->height());
		gtk_range_set_increments(GTK_RANGE(vbar),render->getRowSize(0),vh);
		adj=gtk_range_get_adjustment(GTK_RANGE(vbar));
		g_object_set(G_OBJECT(adj),"page-size",(gfloat)vh,(void *)NULL);
	}
}

// bool gGridView::getBorder()
// {
// 	return (bool)gtk_container_get_border_width(GTK_CONTAINER(widget));
// }
// 
// void gGridView::setBorder(bool vl)
// {
// 	if (vl == getBorder() ) return;
// 	switch(vl)
// 	{
// 		case false:
// 			gtk_container_set_border_width(GTK_CONTAINER(widget),0); break;
// 		case true:
// 			gtk_container_set_border_width(GTK_CONTAINER(widget),1); break;
// 	}
// }

int gGridView::scrollBar()
{
	return scroll;
}

void gGridView::setScrollBar(int vl)
{
	if (vl==scroll) return;

	scroll=vl;
	if ( !(scroll & 1) ) g_object_set(G_OBJECT(hbar),"visible",FALSE,(void *)NULL);
	if ( !(scroll & 2) ) g_object_set(G_OBJECT(vbar),"visible",FALSE,(void *)NULL);
	calculateBars();
}

int gGridView::scrollX()
{
	return render->getOffsetX();
}

int gGridView::scrollY()
{
	return render->getOffsetY();
}

void gGridView::setScrollX(int vl)
{
	GtkAdjustment* adj;
	int max;
	
	adj = gtk_range_get_adjustment(GTK_RANGE(hbar));
	
	max = (int)(adj->upper - adj->page_size);
	
	if (vl < 0) 
		vl = 0;
	else if (vl > max)
		vl = max;
	
	gtk_adjustment_set_value(adj, (gdouble)vl);
}

void gGridView::setScrollY(int vl)
{
	GtkAdjustment* adj;
	int max;
	
	adj = gtk_range_get_adjustment(GTK_RANGE(vbar));
	
	max = (int)(adj->upper - adj->page_size);
	
	if (vl < 0) 
		vl = 0;
	else if (vl > max)
		vl = max;
	
	gtk_adjustment_set_value(adj, (gdouble)vl);
}

int gGridView::rowAt(int y)
{
	int bpos=-render->getOffsetX();
	int bc;

	for (bc=0; bc<render->rowCount(); bc++)
	{
		if ( (bpos<=y) && ( (bpos+render->getRowSize(bc)))>=y ) return bc;
		bpos+=render->getRowSize(bc);
	}

	return -1;
}

int gGridView::columnAt(int x)
{
	int bpos=-render->getOffsetY();
	int bc;

	for (bc=0; bc<render->columnCount(); bc++)
	{
		if ( (bpos<=x) && ( (bpos+render->getColumnSize(bc)))>=x ) return bc;
		bpos+=render->getColumnSize(bc);
	}

	return -1;
}

int gGridView::visibleLeft()
{
	gboolean vl;
	int w;

	g_object_get(G_OBJECT(lateral),"visible",&vl,(void *)NULL);
	if (!vl) return 0;
	gtk_widget_get_size_request(lateral,&w,NULL);
	return w;
}

int gGridView::visibleTop()
{
	gboolean vl;
	int h;

	g_object_get(G_OBJECT(header),"visible",&vl,(void *)NULL);
	if (!vl) return 0;
	gtk_widget_get_size_request(header,NULL,&h);
	return h;
}

int gGridView::visibleWidth()
{
	return render->visibleWidth();
}

int gGridView::visibleHeight()
{
	return render->visibleHeight();
}

void gGridView::ensureVisible(int row, int col)
{
	int x, y, w, h;
	GtEnsureVisible arg;
	bool sx, sy;
	
	arg.clientWidth = visibleWidth();
	arg.clientHeight = visibleHeight();
	arg.scrollX = scrollX();
	arg.scrollY = scrollY();
	arg.scrollWidth = render->width();
	arg.scrollHeight = render->height();
	
	sy = row >= 0 && row < rowCount();
	sx = col >= 0 && col < columnCount();
	
	if (!sy)
		row = 0;
	if (!sx)
		col = 0;
	
	x = columnPos(col);
	y = rowPos(row);
	w = columnWidth(col);
	h = rowHeight(row);
	
	gt_ensure_visible(&arg, x, y, w, h);
	
	if (sx)
		setScrollX(arg.scrollX);
	if (sy)
		setScrollY(arg.scrollY);
	
/* int vl;
	//int bc;
	GtkAdjustment *adj;

	if (col >= 0 && col < columnCount())
	{
		vl = columnPos(col) - scrollX() + columnWidth(col) / 2;
		//for (bc=0; bc<col; bc++)
		//	vl+=columnWidth(bc);
	
		if (vl<0)
		{
			vl=render->getOffsetX()+vl;
			adj=gtk_range_get_adjustment(GTK_RANGE(hbar));
			g_object_set(G_OBJECT(adj),"value",(gfloat)vl,NULL);
		}
		else if ( (vl+columnWidth(col))>visibleWidth() )
		{
			vl=render->getOffsetX()+columnWidth(col)-(visibleWidth()-vl);
			adj=gtk_range_get_adjustment(GTK_RANGE(hbar));
			g_object_set(G_OBJECT(adj),"value",(gfloat)vl,NULL);
		}
	}

	if (row >= 0 && row < rowCount())
	{
		vl = rowPos(row) - scrollY() + rowHeight(row) / 2;
		//for (bc=0; bc<row; bc++)
		//	vl+=rowHeight(bc);
	
		if (vl<0)
		{
			vl=render->getOffsetY()+vl;
			adj=gtk_range_get_adjustment(GTK_RANGE(vbar));
			g_object_set(G_OBJECT(adj),"value",(gfloat)vl,NULL);
		}
		else if ( (vl+rowHeight(col))>visibleHeight() )
		{
			vl=render->getOffsetY()+rowHeight(col)-(visibleHeight()-vl);
			adj=gtk_range_get_adjustment(GTK_RANGE(vbar));
			g_object_set(G_OBJECT(adj),"value",(gfloat)vl,NULL);
		}
	}	*/
}

bool gGridView::drawGrid()
{
	return render->drawGrid();
}

void gGridView::setDrawGrid(bool vl)
{
	render->setDrawGrid(vl);
}

void  gGridView::getCursor(int *row,int *col)
{
	if (row) *row=cursor_row;
	if (col) *col=cursor_col;
}

void  gGridView::setCursor(int row,int col)
{
	if (row < 0 || row >= rowCount() || col < 0 || col > columnCount())
	{
		row = -1;
		col = -1;
	}
	
	if (row == cursor_row && col == cursor_col)
		return;
	
	cursor_row = row;
	cursor_col = col;
	
	if (selectionMode() == SELECT_SINGLE)
		setRowSelected(cursor_row, true);
	
	ensureVisible(cursor_row, cursor_col);
	
	emit(SIGNAL(onChange));
}

void gGridView::setColumnCount(int vl)
{
	int old, i;
	
	if (vl<0) vl=0;

	old = columnCount();
	if (old == vl) return;
	
	lock();
	
	render->setColumnCount(vl);
	render->doNotInvalidate = true;
	for (i = old; i < vl; i++)
		render->setColumnSize(i, 80);
	render->doNotInvalidate = false;

	unlock();
	_last_col_width = 0;
	updateLastColumn();
	calculateBars();

	if (vl==0) cursor_col=-1;
	if (vl<=cursor_col) cursor_col=vl-1;
	if ( (rowCount()>0) && (vl>0) && (cursor_col==-1) ) 
	{
		cursor_col=0;
		cursor_row=0;
	}

	refresh();
}

void gGridView::setRowCount(int vl)
{
	int old, i, h;
	
	if (vl<0) vl=0;

	old = rowCount();
	if (old == vl) return;
	
	lock();
	
	render->setRowCount(vl);
	
	if (vl)
	{
		h = minRowHeight(vl - 1);
		render->doNotInvalidate = true;
		for (i = old; i < vl; i++)
		{
			//if ((i % 1000) == 0)
			//	fprintf(stderr, "%d\r", i);
			setRowHeight(i, h);
		}
		render->doNotInvalidate = false;
	}
	
	unlock();
	
	calculateBars();

	if (vl==0) cursor_row=-1;
	if (vl<=cursor_row) cursor_row=vl-1;
	if ( (columnCount()>0) && (vl>0) && (cursor_row==-1) ) 
	{
		cursor_row=0;
		cursor_col=0;
	}
	
	updateHeaders();
	refresh();	
}

/*
int gGridView::findColumn(int pos)
{
	int i;
	int x = 0;
	int x2;
	
	for (i = 0; i < columnCount(); i++)
	{
		x2 = x + columnWidth(i);
		if (pos >= x && pos < x2)
			return i;
		x = x2;
	}
	
	return -1;
}
*/

int gGridView::findColumnSeparation(int pos)
{
	int i;
	int x = 0;
	
	for (i = 0; i < columnCount(); i++)
	{
		x += columnWidth(i);
		if (pos > (x - 2) && pos < (x + 2))
			return columnResizable(i) ? i : -1;
	}
	
	return -1;
}

/*
int gGridView::findRow(int pos)
{
	int i;
	int y = 0;
	int y2;
	
	for (i = 0; i < rowCount(); i++)
	{
		y2 = y + rowHeight(i);
		if (pos >= y && pos < y2)
			return i;
		y = y2;
	}
	
	return -1;
}
*/

int gGridView::findRowSeparation(int pos)
{
	int i;
	int y = 0;
	
	for (i = 0; i < rowCount(); i++)
	{
		y += rowHeight(i);
		if (pos > (y - 2) && pos < (y + 2))
			return rowResizable(i) ? i : -1;
	}
	
	return -1;
}

int gGridView::minColumnWidth(int index)
{
	int w, w2;
	
	w = font()->width(headerText(index));
	w2 = font()->width(footerText(index));
	if (w2 > w)
		w = w2;
		
	return 8 + MAX(w, 8);
}

void gGridView::setColumnWidth(int index,int vl)
{
	if (index < 0 || index >= columnCount())
		return;
		
	if (vl < 0)
		vl = minColumnWidth(index);
	
	if (vl == columnWidth(index))
		return;
		
	render->setColumnSize(index,vl);
	updateLastColumn();
	
	gtk_widget_queue_draw(header);
	gtk_widget_queue_draw(footer);
	calculateBars();
	gtk_widget_queue_draw(contents);
	emit(SIGNAL(onColumnResize), index);
}

int gGridView::minRowHeight(int index)
{
	return 8 + font()->height(" "); // rowText(index)
}

void gGridView::setRowHeight(int index,int vl)
{
	if (index < 0 || index >= rowCount())
		return;
		
	if (vl < 0)
		vl = minRowHeight(index);

	render->setRowSize(index,vl);
	
	if (locked())
		return;
		
	gtk_widget_queue_draw(lateral);
	calculateBars();
	gtk_widget_queue_draw(contents);
	emit(SIGNAL(onRowResize), index);
}

int gGridView::itemX(int col)
{
	int ret,bc;

	if (col<0) return -1;
	if (col>=render->columnCount()) return -1;

	ret = -render->getOffsetX();
	for (bc=0; bc<col; bc++)
		ret+=render->getColumnSize(bc);

	ret += rowWidth();

	return ret + 2;
}

int gGridView::itemY(int row)
{
	int ret,bc;

	if (row<0) return -1;
	if (row>=render->rowCount()) return -1;

	ret = -render->getOffsetY();
	for (bc=0; bc<row; bc++)
		ret+=render->getRowSize(bc);

	ret += headerHeight();

	return ret + 2;
}

int gGridView::itemW(int col)
{
	if (col<0) return -1;
	if (col>=render->columnCount()) return -1;

	return render->getColumnSize(col) - 1;
}

int gGridView::itemH(int row)
{
	if (row<0) return -1;
	if (row>=render->rowCount()) return -1;

	return render->getRowSize(row) - 1;
}

char* gGridView::itemText(int row,int col)
{
	return render->getFieldText(col,row);
}

char* gGridView::itemRichText(int row,int col)
{
	return render->getFieldRichText(col,row);
}

gColor gGridView::itemFg(int row,int col)
{
	return render->getFieldFg(col,row);
}

gColor gGridView::itemBg(int row,int col)
{
	return render->getFieldBg(col,row);
}

int gGridView::itemPadding(int row,int col)
{
	return render->getFieldPadding(col,row);
}

int gGridView::itemAlignment(int row,int col)
{
	return render->getFieldAlignment(col,row);
}

gPicture *gGridView::itemPicture(int row, int col)
{
	return render->getFieldPicture(col, row);
}

gFont *gGridView::itemFont(int row, int col)
{
	gFont *f = render->getFieldFont(col, row);
	return f ? f : font();
}

bool gGridView::itemSelected(int row,int col)
{
	return render->getFieldSelected(col,row);
}
	
void gGridView::setItemText(int row,int col, const char *vl)
{
	render->setFieldText(col,row,vl);
}

void gGridView::setItemRichText(int row,int col, const char *vl)
{
	render->setFieldRichText(col,row,vl);
}

void gGridView::setItemFg(int row, int col, gColor vl)
{
	render->setFieldFg(col,row,vl);
}

void  gGridView::setItemBg(int row, int col, gColor vl)
{
	render->setFieldBg(col,row,vl);
}

void gGridView::setItemPadding(int row,int col,int vl)
{
	render->setFieldPadding(col,row,vl);
}

void gGridView::setItemAlignment(int row,int col,int vl)
{
	render->setFieldAlignment(col,row,vl);
}

void gGridView::setItemPicture(int row, int col, gPicture *vl)
{
	render->setFieldPicture(col, row, vl);
}

void gGridView::setItemFont(int row, int col, gFont *vl)
{
	render->setFieldFont(col, row, vl);
}

void gGridView::setItemSelected(int row,int col,bool vl)
{
	render->setFieldSelected(col,row,vl);
}

void gGridView::clearItem(int row,int col)
{
	render->clearField(col,row);
}

/*******************************************************************
Headers, Footers and Row Separators
********************************************************************/
int gGridView::headersVisible()
{	
	int val = 0;
	
	//g_object_get(G_OBJECT(header),"visible",&hv,NULL);
	//g_object_get(G_OBJECT(lateral),"visible",&lv,NULL);
	
	if (GTK_WIDGET_VISIBLE(header)) val += 1;
	if (GTK_WIDGET_VISIBLE(lateral)) val += 2;
	
	return val;
}

void gGridView::setHeadersVisible(int vl)
{
	/*gboolean now;
	
	g_object_get(G_OBJECT(header),"visible",&now,NULL);
	if (vl==now) return;
	g_object_set(G_OBJECT(header),"visible",vl,NULL);*/
	
	if (vl & 1)
		gtk_widget_show(header);
	else
		gtk_widget_hide(header);
		
	if (vl & 2)
		gtk_widget_show(lateral);
	else
		gtk_widget_hide(lateral);
}

bool gGridView::footersVisible()
{
	return GTK_WIDGET_VISIBLE(footer);
}

void gGridView::setFootersVisible(bool vl)
{
	if (vl)
		gtk_widget_show(footer);
	else
		gtk_widget_hide(footer);
}

void gridheader_destroy(gGridHeader *data)
{
	delete data;
}

void gridrow_destroy(gGridRow *data)
{
	delete data;
}

void gGridView::setHeaderText(int col, const char *value)
{
	gGridHeader *head;
	int w;

	if ((col<0) || (col>=columnCount()) ) return;
	if (!value) value="";

	if (!hdata)
		hdata=g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL, 
		(GDestroyNotify)gridheader_destroy);

	head=(gGridHeader*)g_hash_table_lookup(hdata,(gpointer)col);
	if (!head) 
	{
		head=new gGridHeader();
		g_hash_table_insert(hdata,(gpointer)col,(gpointer)head);
	}
	head->setHeader(value);

	w = font()->width(value) + 8;
	if (w > columnWidth(col))
		setColumnWidth(col, w);

	gtk_widget_queue_draw(header);
}

void gGridView::setFooterText(int col, const char *value)
{
	gGridHeader *head;

	if (!hdata)
		hdata=g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL, 
		(GDestroyNotify)gridheader_destroy);

	head=(gGridHeader*)g_hash_table_lookup(hdata,(gpointer)col);
	if (!head) 
	{
		head=new gGridHeader();
		g_hash_table_insert(hdata,(gpointer)col,(gpointer)head);
	}
	head->setFooter(value);

	gtk_widget_queue_draw(footer);
}

char* gGridView::headerText(int col)
{
	gGridHeader *head;

	if (!hdata) return NULL;
	head=(gGridHeader*)g_hash_table_lookup(hdata,(gpointer)col);
	if (!head) return NULL;
	return head->header;
}

char* gGridView::footerText(int col)
{
	gGridHeader *head;

	if (!hdata) return NULL;
	head=(gGridHeader*)g_hash_table_lookup(hdata,(gpointer)col);
	if (!head) return NULL;
	return head->footer;
}

bool gGridView::columnResizable(int index)
{
	gGridHeader *head;

	if (!hdata) return true;
	head=(gGridHeader*)g_hash_table_lookup(hdata,(gpointer)index);
	if (!head) return true;
	return head->resizable;
}

void gGridView::setColumnResizable(int col,bool value)
{
	gGridHeader *head;

	if (!hdata)
		hdata=g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL, 
		(GDestroyNotify)gridheader_destroy);

	head=(gGridHeader*)g_hash_table_lookup(hdata,(gpointer)col);
	if (!head) 
	{
		head=new gGridHeader();
		g_hash_table_insert(hdata,(gpointer)col,(gpointer)head);
	}
	head->resizable=value;
	if (!value)
	{
		gdk_window_set_cursor(header->window,NULL);
		gdk_window_set_cursor(footer->window,NULL);
	}
}

char* gGridView::rowText(int col)
{
	gGridRow *head;

	if (vdata)
	{
		head = (gGridRow*)g_hash_table_lookup(vdata,(gpointer)col);
		if (head) 
			return head->text;
	}
	
	sprintf(void_row_buffer, "%d", col + 1);
	return void_row_buffer;
}

void gGridView::setRowText(int col, const char *value)
{
	gGridRow *head;

	if (!vdata)
		vdata=g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL, 
		(GDestroyNotify)gridrow_destroy);

	head=(gGridRow*)g_hash_table_lookup(vdata,(gpointer)col);
	if (!head) 
	{
		head=new gGridRow();
		g_hash_table_insert(vdata,(gpointer)col,(gpointer)head);
	}
	head->setText(value);
	
	updateLateralWidth(font()->width(value));
	gtk_widget_queue_draw(lateral);
}

bool gGridView::rowResizable(int index)
{
	gGridRow *head;

	if (!vdata) return true;
	head=(gGridRow*)g_hash_table_lookup(vdata,(gpointer)index);
	if (!head) return true;
	return head->resizable;
}

void gGridView::setRowResizable(int col,bool value)
{
	gGridRow *head;

	if (!vdata)
		vdata=g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL, 
		(GDestroyNotify)gridrow_destroy);

	head=(gGridRow*)g_hash_table_lookup(vdata,(gpointer)col);
	if (!head) 
	{
		head=new gGridRow();
		g_hash_table_insert(vdata,(gpointer)col,(gpointer)head);
	}
	head->resizable=value;
	if (!value) gdk_window_set_cursor(lateral->window,NULL);
}

bool gGridView::rowSelected(int index)
{
	return render->getRowSelected(index);
}

void gGridView::setRowSelected(int col,bool value)
{
	if (selectionMode() == SELECT_NONE)
		return;
	if (selectionMode() == SELECT_SINGLE)
		clearSelection();
	render->setRowSelected(col,value);
}

void gGridView::updateHeaders()
{
	int h, wlat;
	//int i, w, wd;
	//char *text;
	
	h = font()->height() + 8;
	
	gtk_widget_set_size_request(header, header->requisition.width, h);
	gtk_widget_set_size_request(footer, footer->requisition.width, h);
	
	wlat = 8 + font()->width("9") * sprintf(void_row_buffer, "%d", rowCount());
	
	/*wlat = 0;
	for (i = 0; i < rowCount(); i++)
	{
		if (rowHeight(i) < h)
			setRowHeight(i, h);
		text = rowText(i);
		if (text == void_row_buffer)
			w = wd * strlen(text);
		else
			w = font()->width(text);
		
		w += 8;
		if (w > wlat)
			wlat = w;
	}*/
	updateLateralWidth(wlat);
}

void gGridView::updateLateralWidth(int w)
{
	if (w > lateral->allocation.width)
		gtk_widget_set_size_request(lateral, w, lateral->allocation.height);
}

void gGridView::setFont(gFont *ft)
{
	gControl::setFont(ft);
	gtk_widget_modify_font(contents, font()->desc());
	
	updateHeaders();
}

int gGridView::rowWidth()
{
	if (GTK_WIDGET_VISIBLE(lateral))
		return lateral->allocation.width;
	else
		return 0;
}

int gGridView::headerHeight()
{
	if (GTK_WIDGET_VISIBLE(header))
		return header->allocation.height;
	else
		return 0;
}

int gGridView::footerHeight()
{
	if (GTK_WIDGET_VISIBLE(footer))
		return footer->allocation.height;
	else
		return 0;
}

void gGridView::setBackground(gColor color)
{
	gControl::setBackground(color);
	if (color == COLOR_DEFAULT)
		set_gdk_bg_color(contents, get_gdk_base_color(contents));
	else
		set_gdk_bg_color(contents, color);
	//set_gdk_bg_color(header, color);
	//set_gdk_bg_color(lateral, color);
}

void gGridView::setForeground(gColor color)
{
	gControl::setForeground(color);
	color = realForeground();
	set_gdk_fg_color(contents, color);
	//set_gdk_fg_color(header, color);
	//set_gdk_fg_color(lateral, color);
}

void gGridView::updateLastColumn()
{
	int n = columnCount() - 1;
	int vw = visibleWidth();
	
	if (n < 0)
		return;
		
	if (_updating_last_column)
		return;
		
	_updating_last_column = true;
	
	if (!_last_col_width)
		_last_col_width = columnWidth(n);
	
	//fprintf(stderr, "updateLastColumn: vw = %d columnPos = %d %d _lcw = %d \n", vw, columnPos(n), columnPos(n), _last_col_width);
	
	if (((columnPos(n) + _last_col_width) < vw) && (columnWidth(n) != (vw - columnPos(n))))
	{
		//fprintf(stderr, "updateLastColumn: vw = %d -> %d\n", vw, vw - columnPos(n));
		setColumnWidth(n, vw - columnPos(n));
	}

	_updating_last_column = false;
}

void gGridView::startScrollTimer(GSourceFunc func)
{
	if (!scroll_timer)
	{
		(*func)(this);
		scroll_timer = g_timeout_add(50, (GSourceFunc)func, this);
	}	
}

void gGridView::stopScrollTimer()
{
	if (scroll_timer)
	{
		g_source_remove(scroll_timer);
		scroll_timer = 0;
	}
}

