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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <stdlib.h>

#include "widgets.h"
#include "widgets_private.h"
#include "tablerender.h"

class gGridHeader
{
public:
	char *header;
	char *footer;
	bool resizable;

	gGridHeader();
	~gGridHeader();
	void setHeader(char *vl);
	void setFooter(char *vl);
};

class gGridRow
{
public:
	char *text;
	bool resizable;

	gGridRow();
	~gGridRow();
	void setText(char *vl);
};


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

void gGridHeader::setHeader(char *vl)
{
	if (!vl) vl="";
	if (header) g_free(header);
	header=(char*)g_malloc(sizeof(char)*(strlen(vl)+1));
	if (header) strcpy(header,vl);
}

void gGridHeader::setFooter(char *vl)
{
	if (!vl) vl="";
	if (footer) g_free(footer);
	footer=(char*)g_malloc(sizeof(char)*(strlen(vl)+1));
	if (footer) strcpy(footer,vl);
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

void gGridRow::setText(char *vl)
{
	if (!vl) vl="";
	if (text) g_free(text);
	text=(char*)g_malloc(sizeof(char)*(strlen(vl)+1));
	if (text) strcpy(text,vl);
}


/**************************************************************************

gGridView

***************************************************************************/
void gGridView_hbar(GtkAdjustment *adj,gGridView *data)
{
	GdkRectangle rect={0,0,0,0};

	gdk_drawable_get_size(data->header->window,&rect.width,&rect.height);
	gdk_window_invalidate_rect(data->header->window,&rect,TRUE);
	gdk_drawable_get_size(data->footer->window,&rect.width,&rect.height);
	gdk_window_invalidate_rect(data->footer->window,&rect,TRUE);

	data->render->setOffsetX(gtk_adjustment_get_value (adj));
	if (data->onScroll) data->onScroll(data);
}

void gGridView_vbar(GtkAdjustment *adj,gGridView *data)
{
	GdkWindow *win=data->lateral->window;
	GdkRectangle rect={0,0,0,0};
	
	gdk_drawable_get_size(win,&rect.width,&rect.height);
	gdk_window_invalidate_rect(win,&rect,TRUE);

	data->render->setOffsetY(gtk_adjustment_get_value (adj));
	if (data->onScroll) data->onScroll(data);
}

gboolean gGridView_configure(GtkWidget *w,GdkEventConfigure *e,gGridView *data)
{
	data->calculateBars();

	return FALSE;
}

gboolean tbheader_leave(GtkWidget *wid,GdkEventCrossing *e,gGridView *data)
{
	data->mouse_x=-1;
	gdk_window_set_cursor(wid->window,NULL);

	return FALSE;
}

gboolean tblateral_leave(GtkWidget *wid,GdkEventCrossing *e,gGridView *data)
{
	data->mouse_y=-1;
	gdk_window_set_cursor(wid->window,NULL);

	return FALSE;
}

gboolean tbheader_move(GtkWidget *wid,GdkEventMotion *e,gGridView *data)
{
	GdkCursor *cursor;
	int bpos;
	int cpos;
	long current;
	long bc,binit=0;
	bool benter=false;
	
	bpos=-data->render->getOffsetX();
	cpos=(int)e->x;

	if (data->last_x!=-1)  binit=data->last_x;

	for (bc=binit; bc<(data->columnCount()); bc++)
	{
		if ( ((e->x-5)<=bpos) && ((e->x+5)>=bpos) ) benter=true;
		if ( (e->state & GDK_BUTTON1_MASK) && (data->last_x!=-1) ) benter=true;

		if (benter)
		{
			if (!bc) continue;
			if (!data->columnResizable(bc-1)) return false;
			cursor=gdk_cursor_new(GDK_SB_H_DOUBLE_ARROW);
			gdk_window_set_cursor(wid->window,cursor);
			if (e->state & GDK_BUTTON1_MASK)
			{
				if (data->mouse_x==-1) 
				{ 
					data->last_x=bc;
					data->mouse_x=e->x;
					return false; 
				}
				bc--;
				current=data->columnWidth(bc)+e->x-data->mouse_x;
				if (current<15) current=15;		
				data->setColumnWidth(bc,current);
				data->mouse_x=e->x;
				return false;
			}
			data->last_x=-1;
			data->mouse_x=-1;
			return false;	
		}
	
		bpos+=data->columnWidth(bc);
	}
	
	data->last_x=-1;
	data->mouse_x=-1;
	gdk_window_set_cursor(wid->window,NULL);
	return false;
}

gboolean tblateral_move(GtkWidget *wid,GdkEventMotion *e,gGridView *data)
{
	GdkCursor *cursor;
	int bpos;
	int cpos;
	long current;
	long bc,binit=0;
	bool benter=false;
	
	bpos=-data->render->getOffsetY();
	cpos=(int)e->y;

	if (data->last_y!=-1)  binit=data->last_y;

	for (bc=binit; bc<(data->rowCount()); bc++)
	{
		if ( ((e->y-5)<=bpos) && ((e->y+5)>=bpos) ) benter=true;
		if ( (e->state & GDK_BUTTON1_MASK) && (data->last_y!=-1) ) benter=true;

		if (benter)
		{
			if (!bc) continue;
			if (!data->rowResizable(bc-1)) return false;
			cursor=gdk_cursor_new(GDK_SB_V_DOUBLE_ARROW);
			gdk_window_set_cursor(wid->window,cursor);
			if (e->state & GDK_BUTTON1_MASK)
			{
				if (data->mouse_y==-1) 
				{ 
					data->mouse_y=e->y;
					data->last_y=bc;
					return false; 
				}
				bc--;
				current=data->rowHeight(bc)+e->y-data->mouse_y;
				if (current<15) current=15;		
				data->setRowHeight(bc,current);
				data->mouse_y=e->y;
				return false;
			}
			data->mouse_y=-1;
			data->last_y=-1;
			return false;	
		}
	
		bpos+=data->rowHeight(bc);
	}
	
	data->mouse_y=-1;
	data->last_y=-1;
	gdk_window_set_cursor(wid->window,NULL);
	return false;
}

gboolean tbheader_expose(GtkWidget *wid,GdkEventExpose *e,gGridView *data)
{
	GtkCellRenderer *rend=NULL;
	GdkRectangle rect={0,0,0,0};
	GdkGC *gc;
	char *buf;
	int bpos=-data->render->getOffsetX();
	int pw,ph,maxw;
	bool footer=false;
	long bc;
	
	g_object_get(G_OBJECT(wid),"name",&buf,NULL);
	if (buf)
	{
		if (!strcmp(buf,"gambas-grid-footer")) footer=true;
		g_free(buf); buf=NULL;
	}

	gc=gdk_gc_new(wid->window);
	gdk_gc_set_clip_origin(gc,0,0);
	gdk_gc_set_clip_rectangle(gc,&e->area);
	gdk_drawable_get_size(wid->window,&maxw,&ph);

	for (bc=0; bc<data->columnCount(); bc++)
	{
		if ((bpos+data->columnWidth(bc)) < 0)  { bpos+=data->columnWidth(bc); continue; }
		if (bpos > maxw ) break;
		pw=data->columnWidth(bc);
		gtk_paint_box(data->header->style,wid->window,
			 GTK_STATE_NORMAL, GTK_SHADOW_OUT,
			 NULL, wid->parent, "buttondefault",
			 bpos,0, pw-1, ph-1);
		gtk_paint_shadow(data->header->style,wid->window,
			 GTK_STATE_NORMAL, GTK_SHADOW_OUT,
			 NULL,wid->parent, "buttondefault",
			 bpos, 0, pw-1, ph-1);

		
		if (footer) buf=data->footerText(bc);
		else        buf=data->headerText(bc);
		if ( buf != NULL )
		{
			if (!rend) rend=gtk_cell_renderer_text_new();
			g_object_set(G_OBJECT(rend),"text",buf,NULL);
			g_object_set(G_OBJECT(rend),"xalign",0.5,NULL);
			rect.x=bpos;
			rect.y=0;
			rect.width=pw-1;
			rect.height=ph-1;

			gtk_cell_renderer_render(GTK_CELL_RENDERER(rend),wid->window,wid,
                                                 &rect,&rect,NULL,(GtkCellRendererState)0);
		}
		

		bpos+=data->columnWidth(bc);
	}
	
	//if (rend) g_object_ref_sink(G_OBJECT(rend));
	g_object_unref(G_OBJECT(gc));

	return FALSE;
}
gboolean tbheader_release(GtkWidget *wid,GdkEventButton *e,gGridView *data)
{
	int bpos;
	long bc,bcurrent=-1;
	bool bfooter=false;
	char *buf=NULL;
	
	if (e->button!=1) return FALSE;
	if (data->last_x != -1) return FALSE;

	bpos=-data->render->getOffsetX();

	for (bc=0; bc<data->columnCount(); bc++)
	{
		if ( (e->x>=bpos) && (e->x<=(bpos+data->columnWidth(bc))) )
		{
			bcurrent=bc;
			break;
		}
		bpos+=data->columnWidth(bc);
	}

	if (bcurrent<0) return FALSE;

	g_object_get(G_OBJECT(wid),"name",&buf,NULL);
	if (buf)
	{
		if (!strcmp(buf,"gambas-grid-footer")) bfooter=true;
		g_free(buf); buf=NULL;
	}

	data->getGridCursor(&bc,NULL);
	data->setGridCursor(bc,bcurrent);

	if (bfooter)
		{ if (data->onFooterClick) data->onFooterClick(data,bcurrent); }
	else
		{ if (data->onColumnClick) data->onColumnClick(data,bcurrent); }
	
	return FALSE;

}

gboolean tblateral_expose(GtkWidget *wid,GdkEventExpose *e,gGridView *data)
{
	GdkGC *gc;
	GdkWindow *win=data->lateral->window;
	int bpos=-data->render->getOffsetY();
	int pw,ph,maxh;
	long bc;
	
	gc=gdk_gc_new(win);
	gdk_gc_set_clip_origin(gc,0,0);
	gdk_gc_set_clip_rectangle(gc,&e->area);
	gdk_drawable_get_size(win,&ph,&maxh);

	for (bc=0; bc<data->rowCount(); bc++)
	{
		if ((bpos+data->rowHeight(bc)) < 0)  
			{ bpos+=data->rowHeight(bc); continue; }
		if (bpos > maxh) break;
		pw=data->rowHeight(bc);
		gtk_paint_box(data->lateral->style,data->lateral->window,
			 GTK_STATE_NORMAL, GTK_SHADOW_OUT,
			 NULL, data->lateral->parent, "buttondefault",
			 0,bpos, ph-1, pw-1);
		gtk_paint_shadow(data->lateral->style,data->lateral->window,
			 GTK_STATE_NORMAL, GTK_SHADOW_OUT,
			 NULL, data->lateral->parent, "buttondefault",
			 0,bpos, ph-1, pw-1);
		bpos+=data->rowHeight(bc);
	
	}

	g_object_unref(G_OBJECT(gc));

	return FALSE;
}

gboolean tblateral_release(GtkWidget *wid,GdkEventButton *e,gGridView *data)
{
	int bpos;
	int realmode;
	int binc=1;
	long bc,bcurrent=-1;
	bool value;
	
	if (e->button!=1) return FALSE;
	if (data->last_y != -1) return FALSE;

	bpos=-data->render->getOffsetY();

	for (bc=0; bc<data->rowCount(); bc++)
	{
		if ( (e->y>=bpos) && (e->y<=(bpos+data->rowHeight(bc))) )
		{
			bcurrent=bc;
			break;
		}
		bpos+=data->rowHeight(bc);
	}

	if (bcurrent<0) return FALSE;

	realmode=data->selectionMode();
	if (realmode==2)
	{
		realmode=2;
		if ( (e->state & GDK_CONTROL_MASK) ) realmode=3;
		if ( (e->state & GDK_SHIFT_MASK) )  realmode=4;
	}

	switch(realmode)
	{
		case 0:
			data->sel_row=-1;
			break;

		case 1:
			data->sel_row=-1;
			value=data->rowSelected(bcurrent);
			data->render->clearSelection();
			data->setRowSelected(bcurrent,!value);
			break;
		case 2:
			data->sel_row=bcurrent;
			value=data->rowSelected(bcurrent);
			if (!value) data->render->clearSelection();
			data->setRowSelected(bcurrent,!value);
			break;
		case 3:
			data->sel_row=bcurrent;
			data->setRowSelected(bcurrent,!data->rowSelected(bcurrent));
			break;

		case 4:
			value=data->rowSelected(bcurrent);
			data->setRowSelected(bcurrent,!value);
			if (data->sel_row != -1)
			{
				if (data->sel_row < bcurrent) 
					for (bc=bcurrent-1;bc>=data->sel_row;bc--)
						data->setRowSelected(bc,!value);

				if (data->sel_row > bcurrent) 
					for (bc=bcurrent+1;bc<=data->sel_row;bc++)
						data->setRowSelected(bc,!value);
			}
			data->sel_row=bcurrent;
			break;

	}

	data->getGridCursor(NULL,&bc);
	data->setGridCursor(bcurrent,bc);
	if (data->onRowClick) data->onRowClick(data,bcurrent);
	
	return FALSE;

}

gboolean gridview_release(GtkWidget *wid,GdkEvent *ev,gGridView *data)
{
	int bpos;
	int realmode;
	int binc=1;
	long bc,c_row=-1,c_col=-1;
	bool value;
	GdkEventButton *e;
	
	if ( (ev->type != GDK_BUTTON_RELEASE) && (ev->type != GDK_2BUTTON_PRESS) ) return FALSE;

	e=(GdkEventButton*)ev;

	if (e->button!=1) return FALSE;

	bpos=-data->render->getOffsetY();

	for (bc=0; bc<data->rowCount(); bc++)
	{
		if ( (e->y>=bpos) && (e->y<=(bpos+data->rowHeight(bc))) )
		{
			c_row=bc;
			break;
		}
		bpos+=data->rowHeight(bc);
	}

	if (c_row<0) return FALSE;

	bpos=-data->render->getOffsetX();

	for (bc=0; bc<data->columnCount(); bc++)
	{
		if ( (e->x>=bpos) && (e->x<=(bpos+data->columnWidth(bc))) )
		{
			c_col=bc;
			break;
		}
		bpos+=data->columnWidth(bc);
	}

	if (c_col<0) return FALSE;

	data->setGridCursor(c_row,c_col);
	if (e->type == GDK_BUTTON_RELEASE)	
		{ if (data->onClick) data->onClick(data,c_row,c_col); }
	else
		{ if (data->onActivate) data->onActivate(data,c_row,c_col); }
	return FALSE;

}

gboolean gridview_keypress(GtkWidget *wid,GdkEventKey *e,gGridView *data)
{
	long row,col;
	bool bchange=false;

	data->getGridCursor(&row,&col);

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
		data->setGridCursor(row,col);
		return TRUE;
	}

	return FALSE;
}

gboolean gridview_expose(GtkWidget *wid,GdkEventExpose *e,gGridView *data)
{	
	gint w,h;

	if (!data->getBorder()) return FALSE;
	gdk_window_get_size(data->widget->window,&w,&h);
	gtk_paint_box(data->widget->style,data->widget->window,GTK_STATE_NORMAL, GTK_SHADOW_OUT,
			 NULL, data->widget, NULL, 0,0, w, h);

	return FALSE;
}

gGridView::gGridView(gControl *parent) : gControl(parent)
{
	GtkWidget* box;
	GtkWidget* hbar;
	GtkWidget* vbar;
	GtkWidget* evb;
	GtkAdjustment *adj;
	GtkSettings *set;
	GtkStyle* st;

	g_typ=Type_gGridView;

	onActivate=NULL;
	onScroll=NULL;
	onClick=NULL;
	onColumnClick=NULL;
	onFooterClick=NULL;
	onRowClick=NULL;

	cursor_col=-1;
	cursor_row=-1;
	sel_row=-1;
	sel_mode=0;
	last_x=-1;
	last_y=-1;
	mouse_x=-1;
	mouse_y=-1;
	scroll=3;
	hdata=NULL;
	vdata=NULL;

	border=gtk_event_box_new();
	widget=gtk_table_new(3,3,FALSE);
	gtk_container_set_border_width  (GTK_CONTAINER(widget),1);

	gtk_container_add(GTK_CONTAINER(border),widget);
	hbar=gtk_hscrollbar_new(NULL);
	vbar=gtk_vscrollbar_new(NULL);
	evb=gtk_drawing_area_new();
	header=gtk_drawing_area_new();
	footer=gtk_drawing_area_new();
	lateral=gtk_drawing_area_new();
	gtk_widget_set_size_request(header,0,20);
	gtk_widget_set_size_request(footer,0,20);
	gtk_widget_set_size_request(lateral,30,0);

	gtk_table_attach_defaults(GTK_TABLE(widget),evb,1,2,1,2);
	gtk_table_attach(GTK_TABLE(widget),header,1,2,0,1,GTK_FILL,GTK_FILL,0,0);
	gtk_table_attach(GTK_TABLE(widget),footer,1,2,2,3,GTK_FILL,GTK_FILL,0,0);
	gtk_table_attach(GTK_TABLE(widget),lateral,0,1,1,2,GTK_FILL,GTK_FILL,0,0);
	gtk_table_attach(GTK_TABLE(widget),hbar,0,2,3,4,GTK_FILL,GTK_FILL,0,0);
	gtk_table_attach(GTK_TABLE(widget),vbar,2,3,0,3,GTK_FILL,GTK_FILL,0,0);
	render=new gTableRender(evb);

	set=gtk_settings_get_default();
	st=gtk_rc_get_style_by_paths(set,NULL,"GtkEntry",GTK_TYPE_ENTRY);
	gtk_widget_modify_base(evb,GTK_STATE_NORMAL,&st->bg[GTK_STATE_NORMAL]);
	gtk_widget_modify_bg(evb,GTK_STATE_NORMAL,&st->base[GTK_STATE_NORMAL]);
	

	connectParent();
	initSignals();

	gtk_widget_add_events(border,GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_widget_add_events(evb,GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
	gtk_widget_add_events(header,GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);
	gtk_widget_add_events(footer,GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);
	gtk_widget_add_events(lateral,GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);	
	gtk_widget_add_events(header,GDK_BUTTON_PRESS_MASK | GDK_BUTTON1_MOTION_MASK);
	gtk_widget_add_events(footer,GDK_BUTTON_PRESS_MASK | GDK_BUTTON1_MOTION_MASK);
	gtk_widget_add_events(lateral,GDK_BUTTON_PRESS_MASK | GDK_BUTTON1_MOTION_MASK);
	gtk_widget_add_events(header,GDK_BUTTON_RELEASE_MASK);
	gtk_widget_add_events(footer,GDK_BUTTON_RELEASE_MASK);
	gtk_widget_add_events(lateral,GDK_BUTTON_RELEASE_MASK);
	
	g_object_set(G_OBJECT(header),"name","gambas-grid-header",NULL);
	g_object_set(G_OBJECT(footer),"name","gambas-grid-footer",NULL);

	g_signal_connect_after(G_OBJECT(border),"size-allocate",G_CALLBACK(gGridView_configure),(void*)this);
	g_signal_connect(G_OBJECT(widget),"expose-event",G_CALLBACK(gridview_expose),this);
	g_signal_connect(G_OBJECT(header),"expose-event",G_CALLBACK(tbheader_expose),this);
	g_signal_connect(G_OBJECT(footer),"expose-event",G_CALLBACK(tbheader_expose),this);
	g_signal_connect(G_OBJECT(lateral),"expose-event",G_CALLBACK(tblateral_expose),this);
	g_signal_connect(G_OBJECT(header),"motion-notify-event",G_CALLBACK(tbheader_move),this);
	g_signal_connect(G_OBJECT(footer),"motion-notify-event",G_CALLBACK(tbheader_move),this);
	g_signal_connect(G_OBJECT(lateral),"motion-notify-event",G_CALLBACK(tblateral_move),this);
	g_signal_connect(G_OBJECT(header),"leave-notify-event",G_CALLBACK(tbheader_leave),this);
	g_signal_connect(G_OBJECT(footer),"leave-notify-event",G_CALLBACK(tbheader_leave),this);
	g_signal_connect(G_OBJECT(header),"button-release-event",G_CALLBACK(tbheader_release),this);
	g_signal_connect(G_OBJECT(footer),"button-release-event",G_CALLBACK(tbheader_release),this);
	g_signal_connect(G_OBJECT(lateral),"leave-notify-event",G_CALLBACK(tblateral_leave),this);
	g_signal_connect(G_OBJECT(lateral),"button-release-event",G_CALLBACK(tblateral_release),this);
	g_signal_connect(G_OBJECT(evb),"button-release-event",G_CALLBACK(gridview_release),this);
	g_signal_connect(G_OBJECT(border),"key-press-event",G_CALLBACK(gridview_keypress),this);
	g_signal_connect(G_OBJECT(evb),"event",G_CALLBACK(gridview_release),this);

	g_object_set(G_OBJECT(vbar),"visible",FALSE,NULL);
	g_object_set(G_OBJECT(hbar),"visible",FALSE,NULL);
	g_object_set(G_OBJECT(footer),"visible",FALSE,NULL);
	g_object_set(G_OBJECT(widget),"can-focus",TRUE,NULL);

	gtk_widget_realize(footer);

	adj=gtk_range_get_adjustment(GTK_RANGE(hbar));
	g_signal_connect(G_OBJECT(adj),"value-changed",G_CALLBACK(gGridView_hbar),(void*)this);
	adj=gtk_range_get_adjustment(GTK_RANGE(vbar));
	g_signal_connect(G_OBJECT(adj),"value-changed",G_CALLBACK(gGridView_vbar),(void*)this);
}

gGridView::~gGridView()
{
	if (hdata) g_hash_table_destroy(hdata);
	if (vdata) g_hash_table_destroy(vdata);
	setRowCount(0);
	setColumnCount(0);
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
}

void gGridView::setDataFunc(void *func,void *data)
{
	render->voidCell=(void(*)(gTableData **,long,long,void *))func;
	render->userData=data;
}


void gGridView::queryUpdate(long row,long col)
{
	render->queryUpdate(row,col);
}

void gGridView::calculateBars()
{
	GList *chd,*iter;
	GtkWidget *hbar,*vbar;
	GtkAdjustment *adj;
	int bh,bv,px,py,pw,ph;
	bool bchange=true;


	chd=gtk_container_get_children(GTK_CONTAINER(widget)); iter=chd;
	while (iter)
	{
		if (G_OBJECT_TYPE(iter->data)==GTK_TYPE_HSCROLLBAR) hbar=GTK_WIDGET(iter->data);
		if (G_OBJECT_TYPE(iter->data)==GTK_TYPE_VSCROLLBAR) vbar=GTK_WIDGET(iter->data);
		iter=iter->next;
	} 
	g_list_free(chd);

	g_object_get(G_OBJECT(hbar),"height-request",&py,NULL);
	g_object_get(G_OBJECT(vbar),"width-request",&px,NULL);
	g_object_get(G_OBJECT(hbar),"visible",&bh,NULL);
	g_object_get(G_OBJECT(vbar),"visible",&bv,NULL);

	while(bchange)
	{
		bchange=false;
	
		if (bh) ph=render->visibleHeight()-py;
		else    ph=render->visibleHeight();

		if (bv) pw=render->visibleWidth()-px;
		else    pw=render->visibleWidth();

		if (pw < render->width() )
		{
			if ( (!bh) && (scroll & 1) ) { bh=true; bchange=true; }
		}
		else
		{
			if (bh) { bh=false; bchange=true; }	
		}  
			
		if (ph < render->height() ) 
		{
			if ((!bv) && (scroll & 2) ) { bv=true; bchange=true; }
		}
		else
		{
			if (bv) { bv=false; bchange=true; }
		}
	}

	g_object_set(G_OBJECT(hbar),"visible",bh,NULL);
	g_object_set(G_OBJECT(vbar),"visible",bv,NULL);

	if (bh) 
	{
		gtk_range_set_range(GTK_RANGE(hbar),0,render->width());
		gtk_range_set_increments(GTK_RANGE(hbar),render->getColumnSize(0),5*render->getColumnSize(0));
		adj=gtk_range_get_adjustment(GTK_RANGE(hbar));
		g_object_set(G_OBJECT(adj),"page-size",(gfloat)render->visibleWidth(),NULL);
	}

	if (bv)
	{
		gtk_range_set_range(GTK_RANGE(vbar),0,render->height());
		gtk_range_set_increments(GTK_RANGE(vbar),render->getRowSize(0),5*render->getRowSize(0));
		adj=gtk_range_get_adjustment(GTK_RANGE(vbar));
		g_object_set(G_OBJECT(adj),"page-size",(gfloat)render->visibleHeight(),NULL);
	}
}

bool gGridView::getBorder()
{
	return (bool)gtk_container_get_border_width(GTK_CONTAINER(widget));
}

void gGridView::setBorder(bool vl)
{
	if (vl == getBorder() ) return;
	switch(vl)
	{
		case false:
			gtk_container_set_border_width(GTK_CONTAINER(widget),0); break;
		case true:
			gtk_container_set_border_width(GTK_CONTAINER(widget),1); break;
	}
}

int gGridView::scrollBar()
{
	return scroll;
}

void gGridView::setScrollBar(int vl)
{
	GList *chd,*iter;
	GtkWidget *hbar,*vbar;

	if (vl==scroll) return;

	chd=gtk_container_get_children(GTK_CONTAINER(widget)); iter=chd;
	while (iter)
	{
		if (G_OBJECT_TYPE(iter->data)==GTK_TYPE_HSCROLLBAR) hbar=GTK_WIDGET(iter->data);
		if (G_OBJECT_TYPE(iter->data)==GTK_TYPE_VSCROLLBAR) vbar=GTK_WIDGET(iter->data);
		iter=iter->next;
	} 
	g_list_free(chd);

	scroll=vl;
	if ( !(scroll & 1) ) g_object_set(G_OBJECT(hbar),"visible",FALSE,NULL);
	if ( !(scroll & 2) ) g_object_set(G_OBJECT(vbar),"visible",FALSE,NULL);
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
	GList *chd,*iter;
	GtkWidget *hbar;
	GtkAdjustment *adj;

	chd=gtk_container_get_children(GTK_CONTAINER(widget)); iter=chd;
	while (iter)
	{
		if (G_OBJECT_TYPE(iter->data)==GTK_TYPE_HSCROLLBAR)
		{
			hbar=GTK_WIDGET(iter->data);
			break;
		}
		iter=iter->next;
		
	} 
	g_list_free(chd);

	adj=gtk_range_get_adjustment(GTK_RANGE(hbar));
	g_object_set(G_OBJECT(adj),"value",(gfloat)vl,NULL);
}

void gGridView::setScrollY(int vl)
{
	GList *chd,*iter;
	GtkWidget *vbar;
	GtkAdjustment *adj;

	chd=gtk_container_get_children(GTK_CONTAINER(widget)); iter=chd;
	while (iter)
	{
		if (G_OBJECT_TYPE(iter->data)==GTK_TYPE_VSCROLLBAR)
		{
			vbar=GTK_WIDGET(iter->data);
			break;
		}
		iter=iter->next;
		
	} 
	g_list_free(chd);


	adj=gtk_range_get_adjustment(GTK_RANGE(vbar));
	g_object_set(G_OBJECT(adj),"value",(gfloat)vl,NULL);
}

long gGridView::rowAt(int y)
{
	int bpos=-render->getOffsetX();
	long bc;

	for (bc=0; bc<render->rowCount(); bc++)
	{
		if ( (bpos<=y) && ( (bpos+render->getRowSize(bc)))>=y ) return bc;
		bpos+=render->getRowSize(bc);
	}

	return -1;
}

long gGridView::columnAt(int x)
{
	int bpos=-render->getOffsetY();
	long bc;

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

	g_object_get(G_OBJECT(lateral),"visible",&vl,NULL);
	if (!vl) return 0;
	gtk_widget_get_size_request(lateral,&w,NULL);
	return w;
}

int gGridView::visibleTop()
{
	gboolean vl;
	int h;

	g_object_get(G_OBJECT(header),"visible",&vl,NULL);
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

void gGridView::ensureVisible(long row,long col)
{
	int vl;
	long bc;
	GList *chd,*iter;
	GtkWidget *hbar,*vbar;
	GtkAdjustment *adj;

	if (row<0) return;
	if (row>=rowCount()) return;
	if (col<0) return;
	if (col>=columnCount()) return;

	chd=gtk_container_get_children(GTK_CONTAINER(widget)); iter=chd;
	while (iter)
	{
		if (G_OBJECT_TYPE(iter->data)==GTK_TYPE_HSCROLLBAR) hbar=GTK_WIDGET(iter->data);
		if (G_OBJECT_TYPE(iter->data)==GTK_TYPE_VSCROLLBAR) vbar=GTK_WIDGET(iter->data);
		iter=iter->next;
		
	} 
	g_list_free(chd);

	vl=-render->getOffsetX();
	for (bc=0; bc<col; bc++)
		vl+=columnWidth(bc);

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

	vl=-render->getOffsetY();
	for (bc=0; bc<row; bc++)
		vl+=rowHeight(bc);

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
	
}

bool gGridView::drawGrid()
{
	return render->drawGrid();
}

void gGridView::setDrawGrid(bool vl)
{
	render->setDrawGrid(vl);
}

long gGridView::columnCount()
{
	return render->columnCount();
}

	
long gGridView::rowCount()
{
	return render->rowCount();
}

void  gGridView::getGridCursor(long *row,long *col)
{
	if (row) *row=cursor_row;
	if (col) *col=cursor_col;
}

void  gGridView::setGridCursor(long row,long col)
{
	if (row<0) return;
	if (row>=rowCount()) return;
	if (col<0) return;
	if (row>=columnCount()) return;

	cursor_row=row;
	cursor_col=col;
}

void gGridView::setColumnCount(long vl)
{
	GdkRectangle rect={0,0,0,0};

	if (vl<0) vl=0;

	if (columnCount()==vl) return;
	render->setColumnCount(vl);
	calculateBars();

	gdk_drawable_get_size(lateral->window,&rect.width,&rect.height);
	gdk_window_invalidate_rect(lateral->window,&rect,TRUE);
	gdk_drawable_get_size(header->window,&rect.width,&rect.height);
	gdk_window_invalidate_rect(header->window,&rect,TRUE);
	gdk_drawable_get_size(footer->window,&rect.width,&rect.height);
	gdk_window_invalidate_rect(footer->window,&rect,TRUE);

	if (vl==0) cursor_col=-1;
	if (vl<=cursor_col) cursor_col=vl-1;
	if ( (rowCount()>0) && (vl>0) && (cursor_col==-1) ) 
	{
		cursor_col=0;
		cursor_row=0;
	}
}

void gGridView::setRowCount(long vl)
{
	GdkRectangle rect={0,0,0,0};

	if (vl<0) vl=0;

	if (rowCount()==vl) return;
	render->setRowCount(vl);
	calculateBars();

	gdk_drawable_get_size(lateral->window,&rect.width,&rect.height);
	gdk_window_invalidate_rect(lateral->window,&rect,TRUE);
	gdk_drawable_get_size(header->window,&rect.width,&rect.height);
	gdk_window_invalidate_rect(header->window,&rect,TRUE);
	gdk_drawable_get_size(footer->window,&rect.width,&rect.height);
	gdk_window_invalidate_rect(footer->window,&rect,TRUE);

	if (vl==0) cursor_row=-1;
	if (vl<=cursor_row) cursor_row=vl-1;
	if ( (columnCount()>0) && (vl>0) && (cursor_row==-1) ) 
	{
		cursor_row=0;
		cursor_col=0;
	}
}

long gGridView::columnWidth(long index)
{
	return render->getColumnSize(index);
}

long gGridView::rowHeight(long index)
{
	return render->getRowSize(index);
}

void gGridView::setColumnWidth(long index,int vl)
{
	GdkWindow *win=header->window;
	GdkRectangle rect={0,0,0,0};

	render->setColumnSize(index,vl);
	gdk_drawable_get_size(win,&rect.width,&rect.height);
	gdk_window_invalidate_rect(win,&rect,TRUE);
	calculateBars();
}

void gGridView::setRowHeight(long index,int vl)
{
	GdkWindow *win=lateral->window;
	GdkRectangle rect={0,0,0,0};

	render->setRowSize(index,vl);
	gdk_drawable_get_size(win,&rect.width,&rect.height);
	gdk_window_invalidate_rect(win,&rect,TRUE);
	calculateBars();
}

int gGridView::itemX(long col)
{
	int ret,bc;

	if (col<0) return -1;
	if (col>=render->columnCount()) return -1;

	ret=render->getOffsetX();
	for (bc=0; bc<col; bc++)
		ret+=render->getColumnSize(bc);

	return ret;
}

int gGridView::itemY(long row)
{
	int ret,bc;

	if (row<0) return -1;
	if (row>=render->rowCount()) return -1;

	ret=render->getOffsetY();
	for (bc=0; bc<row; bc++)
		ret+=render->getRowSize(bc);

	return ret;
}

int gGridView::itemW(long col)
{
	if (col<0) return -1;
	if (col>=render->columnCount()) return -1;

	return render->getColumnSize(col);
}

int gGridView::itemH(long row)
{
	if (row<0) return -1;
	if (row>=render->rowCount()) return -1;

	return render->getRowSize(row);
}

char* gGridView::itemText(long row,long col)
{
	return render->getField(col,row);
}

int gGridView::itemFg(long row,long col)
{
	return render->getFieldFg(col,row);
}

int gGridView::itemBg(long row,long col)
{
	return render->getFieldBg(col,row);
}

int gGridView::itemXPad(long row,long col)
{
	return render->getFieldXPad(col,row);
}

int gGridView::itemYPad(long row,long col)
{
	return render->getFieldYPad(col,row);
}

bool gGridView::itemSelected(long row,long col)
{
	return render->getFieldSelected(col,row);
}
	
void gGridView::setItemText(long row,long col,char *vl)
{
	render->setField(col,row,vl);
}

void gGridView::setItemFg(long row,long col,int vl)
{
	render->setFieldFg(col,row,vl);
}

void  gGridView::setItemBg(long row,long col,int vl)
{
	render->setFieldBg(col,row,vl);
}

void gGridView::setItemXPad(long row,long col,int vl)
{
	render->setFieldXPad(col,row,vl);
}

void  gGridView::setItemYPad(long row,long col,int vl)
{
	render->setFieldYPad(col,row,vl);
}

void gGridView::setItemSelected(long row,long col,bool vl)
{
	render->setFieldSelected(col,row,vl);
}

void gGridView::clearItem(long row,long col)
{
	render->clearField(col,row);
}

/*******************************************************************
Headers, Footers and Row Separators
********************************************************************/
bool gGridView::headersVisible()
{	
	gboolean vl;
	
	g_object_get(G_OBJECT(header),"visible",&vl,NULL);
	return vl;
}

void gGridView::setHeadersVisible(bool vl)
{
	gboolean now;
	
	g_object_get(G_OBJECT(header),"visible",&now,NULL);
	if (vl==now) return;
	g_object_set(G_OBJECT(header),"visible",vl,NULL);
}

bool gGridView::footersVisible()
{	
	gboolean vl;
	
	g_object_get(G_OBJECT(footer),"visible",&vl,NULL);
	return vl;
}

void gGridView::setFootersVisible(bool vl)
{
	gboolean now;
	
	g_object_get(G_OBJECT(footer),"visible",&now,NULL);
	if (vl==now) return;
	g_object_set(G_OBJECT(footer),"visible",vl,NULL);
}

bool gGridView::rowSeparatorVisible()
{	
	gboolean vl;
	
	g_object_get(G_OBJECT(lateral),"visible",&vl,NULL);
	return vl;
}

void gGridView::setRowSeparatorVisible(bool vl)
{
	gboolean now;
	
	g_object_get(G_OBJECT(lateral),"visible",&now,NULL);
	if (vl==now) return;
	g_object_set(G_OBJECT(lateral),"visible",vl,NULL);
}

void gridheader_destroy(gGridHeader *data)
{
	delete data;
}

void gridrow_destroy(gGridRow *data)
{
	delete data;
}

void gGridView::setHeaderText(long col,char *value)
{
	gGridHeader *head;
	GdkRectangle rect={0,0,0,0};

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

	gdk_drawable_get_size(header->window,&rect.width,&rect.height);
	gdk_window_invalidate_rect(header->window,&rect,TRUE);
	
}

void gGridView::setFooterText(long col,char *value)
{
	gGridHeader *head;
	GdkRectangle rect={0,0,0,0};

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

	gdk_drawable_get_size(footer->window,&rect.width,&rect.height);
	gdk_window_invalidate_rect(footer->window,&rect,TRUE);

}

char* gGridView::headerText(long col)
{
	gGridHeader *head;

	if (!hdata) return NULL;
	head=(gGridHeader*)g_hash_table_lookup(hdata,(gpointer)col);
	if (!head) return NULL;
	return head->header;
}

char* gGridView::footerText(long col)
{
	gGridHeader *head;

	if (!hdata) return NULL;
	head=(gGridHeader*)g_hash_table_lookup(hdata,(gpointer)col);
	if (!head) return NULL;
	return head->footer;
}

bool gGridView::columnResizable(long index)
{
	gGridHeader *head;

	if (!hdata) return true;
	head=(gGridHeader*)g_hash_table_lookup(hdata,(gpointer)index);
	if (!head) return true;
	return head->resizable;
}

void gGridView::setColumnResizable(long col,bool value)
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

char* gGridView::rowText(long col)
{
	gGridRow *head;

	if (!vdata) return NULL;
	head=(gGridRow*)g_hash_table_lookup(vdata,(gpointer)col);
	if (!head) return NULL;
	return head->text;
}

void gGridView::setRowText(long col,char *value)
{
	gGridRow *head;
	GdkRectangle rect={0,0,0,0};

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

	gdk_drawable_get_size(lateral->window,&rect.width,&rect.height);
	gdk_window_invalidate_rect(lateral->window,&rect,TRUE);

}

bool gGridView::rowResizable(long index)
{
	gGridRow *head;

	if (!vdata) return true;
	head=(gGridRow*)g_hash_table_lookup(vdata,(gpointer)index);
	if (!head) return true;
	return head->resizable;
}

void gGridView::setRowResizable(long col,bool value)
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

bool gGridView::rowSelected(long index)
{
	return render->getRowSelected(index);
}

void gGridView::setRowSelected(long col,bool value)
{
	render->setRowSelected(col,value);
}
