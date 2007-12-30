/***************************************************************************

  gIconView.cpp

  (c) 2004-2005 - Daniel Campos Fernández <danielcampos@netcourrier.com>
  
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
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <stdlib.h>

#include <stdio.h>



/**************************************************************************

gIconView

***************************************************************************/
class  iconViewWidget;
gboolean icVW_focusOut(GtkWidget *widget,GdkEventFocus *event,iconViewWidget *data);
gboolean icVW_keyPress(GtkWidget *widget,GdkEventKey *e, iconViewWidget *data);
gboolean icVW_trySelect(GtkWidget *widget,GdkEventButton *e, iconViewWidget *data);
gboolean icVW_startEdit(GtkWidget *widget,GdkEventButton *e, iconViewWidget *data);

class  iconViewWidget
{
public:
	
	gIconView *pr;
	GtkWidget *rbox;
	GtkWidget *vbox;
	GtkWidget *lbl;
	GtkWidget *img;
	GtkWidget *entry;
	bool       pos;
	int        bEdit;
	char      *key;
	
	long bufX,bufY;

	iconViewWidget(gIconView *parent,char *vl)
	{
		bEdit=0;
		pr=parent;
		bufX=0;
		bufY=0;
	
		entry=NULL;
		if (vl)
		{
			key=(char*)g_malloc( (strlen(vl)+1)*sizeof(char) );
			if (key) strcpy (key,vl);
		}
		else
			key=NULL;
			
		lbl=gtk_label_new("");
		img=gtk_image_new();
		gtk_misc_set_alignment(GTK_MISC(lbl),0.5,1);
		gtk_misc_set_alignment(GTK_MISC(img),0.5,0.5);
		vbox=gtk_event_box_new();
		rbox=gtk_vbox_new(false,8);
		gtk_box_pack_start(GTK_BOX(rbox),img,false,false,0);
		gtk_box_pack_start(GTK_BOX(rbox),lbl,true,false,0);
		gtk_container_add(GTK_CONTAINER(vbox),rbox);
		gtk_widget_show_all(vbox);
		gtk_layout_put ( GTK_LAYOUT(pr->widget),vbox,0,0);
		gtk_widget_add_events(lbl,GDK_BUTTON_PRESS_MASK);
		g_signal_connect(G_OBJECT(vbox),"button-press-event",G_CALLBACK(icVW_trySelect),(gpointer)this);
		g_signal_connect(G_OBJECT(vbox),"button-release-event",G_CALLBACK(icVW_startEdit),(gpointer)this);
		pos=false;
	};
	
	~iconViewWidget()
	{
		if (key) g_free(key);
		if (entry) gtk_widget_destroy(entry);
		if (lbl) gtk_widget_destroy(lbl);
		if (img) gtk_widget_destroy(img);
		if (vbox) gtk_widget_destroy(vbox);
	}
	
	void setEditable(bool vl)
	{
		bEdit=vl;
	}
	
	void refreshSize()
	{
		GdkPixbuf *buf;
		gint sW,sH;
		gint plusH=8;
		
		buf=gtk_image_get_pixbuf(GTK_IMAGE(img));
		
		if (buf)
		{
			sW=gdk_pixbuf_get_width(buf);
			sH=gdk_pixbuf_get_height(buf);
			if (sW<pr->grx) sW=pr->grx;
			if (sH<pr->gry) sH=pr->gry;
		}
		else
		{
			sW=pr->grx;
			sH=pr->gry;
		}
		
		plusH+=pr->font()->height(gtk_label_get_text(GTK_LABEL(lbl)));
		gtk_widget_set_size_request(vbox,sW,sH+plusH);
		gtk_widget_set_size_request(img,sW,sH);
	}

	void setPixbuf(GdkPixbuf *buf)
	{
		gint sW,sH;
		
		gtk_image_set_from_pixbuf(GTK_IMAGE(img),buf);
		refreshSize();
	}
	
	void setText(char *vl)
	{
		gtk_label_set_text(GTK_LABEL(lbl),vl);
		refreshSize();
	}	
	
	void move(long x,long y)
	{
		GtkLayout *ly=(GtkLayout*)pr->widget;
		
		gtk_layout_move(ly,vbox,x,y);
		bufX=x;
		bufY=y;
	}
	
	void edit()
	{
		GtkLayout *ly=(GtkLayout*)pr->widget;
		gint BufW,BufH;
		
		gtk_widget_get_size_request(img,NULL,&BufH);
		if (entry) gtk_widget_destroy(entry);
		entry=gtk_entry_new();
		gtk_layout_put (ly,entry,bufX,bufY+BufH+8);
		gtk_entry_set_alignment(GTK_ENTRY(entry),0.5);
		gtk_entry_set_text(GTK_ENTRY(entry),gtk_label_get_text(GTK_LABEL(lbl)));
		gtk_widget_get_size_request(vbox,&BufW,NULL);
		gtk_widget_get_size_request(lbl,NULL,&BufH);
		gtk_widget_set_size_request(entry,BufW,BufH);
		gtk_widget_show_all(entry);
		gtk_widget_grab_focus(entry);
		g_signal_connect(G_OBJECT(entry),"key-press-event",G_CALLBACK(icVW_keyPress),(gpointer)this);
		g_signal_connect(G_OBJECT(entry),"focus-out-event",G_CALLBACK(icVW_focusOut),(gpointer)this);
	}
	
	void stopEdit()
	{
		if (entry) {
			gtk_widget_destroy(entry);
			entry=NULL;
		}
	}
	
	void finishEdit()
	{
		GtkTreeIter iter;
		
		if (entry) {
			if ( pr->iterKey(key,&iter) )
			{
				gtk_list_store_set(pr->store,&iter,1,gtk_entry_get_text(GTK_ENTRY(entry)),-1);
				g_object_set(G_OBJECT(entry),"visible",false,NULL);
				entry=NULL;
				if (pr->onRename) pr->onRename(pr);
				gtk_widget_grab_focus(pr->widget);
			}
		}
	}
	
}; 

void IconView_finishEdit(gpointer key,iconViewWidget *wid,GdkEventButton *e)
{
	wid->finishEdit();	
}

void icVW_size(GtkWidget *widget,GtkRequisition *req,gIconView *data)
{
	if (data->arr!=-1) data->drawElements();
}

gboolean icVW_mousePress(GtkWidget *widget,GdkEventButton *e,gIconView  *data)
{
	g_hash_table_foreach(data->elements,(GHFunc)IconView_finishEdit,e);
	return false;
}

gboolean icVW_focusOut(GtkWidget *widget,GdkEventFocus *e,iconViewWidget *data)
{
	if ( G_IS_OBJECT(widget) )
	{
		gtk_widget_destroy(widget);
		data->entry=NULL;
	}
	return false;
}

gboolean icVW_startEdit(GtkWidget *widget,GdkEventButton *e, iconViewWidget *data)
{
	if (data->pr->onClick) data->pr->onClick(data->pr);
	
	if (data->bEdit==2)
	{
		data->edit();
		data->bEdit=1;
		return true;
	}
	return false;
}

gboolean icVW_tryEdit(GtkWidget *widget,GdkEventButton *e, iconViewWidget *data)
{
	GtkTreeIter iter;
	gint bufH;
	int vsel;
	
	if (data->bEdit!=1) return false;
	if (e->type!=GDK_BUTTON_PRESS) return false;
	if (e->button!=1) return false;
	if (data->entry) return false;
	
	gtk_widget_get_size_request(data->img,NULL,&bufH);
	if (e->y <= bufH ) return false;
	
	if ( data->pr->iterKey(data->key,&iter) )
	{
		gtk_tree_model_get(GTK_TREE_MODEL(data->pr->store),&iter,3,&vsel,-1);
		if (vsel)
		{
			data->bEdit=2;
			return true;
		}
	}
	
	return false;
}

gboolean icVW_trySelect(GtkWidget *widget,GdkEventButton *e, iconViewWidget *data)
{
	GtkTreeIter iter;
	GtkTreeIter uiter;
	GtkTreePath *path;
	int sel;
	bool unsel=true;
	bool raise=false;
	
	if ( e->type==GDK_2BUTTON_PRESS ) {
		if (data->pr->onActivate) data->pr->onActivate(data->pr);
		return false;
	}
	
	if (icVW_tryEdit(widget,e,data)) return false;
	
	gtk_widget_grab_focus(data->pr->widget);
	
	if (!data->pr->iterKey(data->key,&iter)) return false;
	
	if (data->pr->currkey) 
	{
		if ( data->pr->iterKey(data->pr->currkey,&uiter) )
		{
			g_free(data->pr->currkey); data->pr->currkey=NULL;
			path=gtk_tree_model_get_path(GTK_TREE_MODEL(data->pr->store),&uiter);
			gtk_tree_model_row_changed(GTK_TREE_MODEL(data->pr->store),path,&uiter);
			gtk_tree_path_free(path);
		}
		else
			g_free(data->pr->currkey);
	}
	
	gtk_tree_model_get(GTK_TREE_MODEL(data->pr->store),&iter,0,&data->pr->currkey,-1);
	
	if ( (data->pr->smode==3) || (e->type==GDK_NOTHING) )
	{
		path=gtk_tree_model_get_path(GTK_TREE_MODEL(data->pr->store),&iter);
		gtk_tree_model_row_changed(GTK_TREE_MODEL(data->pr->store),path,&iter);
		gtk_tree_path_free(path);
		return false;
	}
	
	gtk_tree_model_get(GTK_TREE_MODEL(data->pr->store),&iter,3,&sel,-1);
	
	if (!sel)
	{
		if (data->pr->smode==1) unsel=false;
		if (data->pr->smode==2)
			if ( e->state & GDK_CONTROL_MASK ) unsel=false;
	
		if (unsel)
		{
			gtk_tree_model_get_iter_first(GTK_TREE_MODEL(data->pr->store),&uiter);
			do
			{
				gtk_list_store_set(data->pr->store,&uiter,3,0,-1);
			} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(data->pr->store),&uiter));
		}
	
		raise=true;
		gtk_list_store_set(data->pr->store,&iter,3,1,-1);
	}
	else
	{
		if (data->pr->smode==0) unsel=false;
		if (data->pr->smode==1)
			if ( e->state & GDK_SHIFT_MASK ) unsel=false;
		if (unsel) {
			raise=true;
			gtk_list_store_set(data->pr->store,&iter,3,0,-1);
		}
	}
	
	if (raise)
		if (data->pr->onSelect) data->pr->onSelect(data->pr);
	
	return false;
}



gboolean icVW_keyPress(GtkWidget *widget,GdkEventKey *e, iconViewWidget *data)
{
	GtkTreeIter iter;
	gIconView *pr;

	if (e->keyval==GDK_Escape)
	{
		g_object_set(G_OBJECT(widget),"visible",false,NULL);
		return true;
	}
	
	if (e->keyval==GDK_Return)
	{
		data->finishEdit();
		return true;
	}
	
	return false;
}

gboolean icVW_navigate(GtkWidget *widget,GdkEventKey *e, gIconView *data)
{
	GtkTreePath *path;
	GtkTreeIter iter,test;
	GdkEventButton ev;
	iconViewWidget *wid,*wtest,*wnear=NULL;
	char *buf;
	double diff1,diff2;

	if (!data->currkey) return false;
	if (!data->iterKey(data->currkey,&iter) ) return false;
	
	ev.type=GDK_NOTHING;
	
	if ( e->keyval == GDK_Right )
		if ( !gtk_tree_model_iter_next(GTK_TREE_MODEL(data->store),&iter) ) return false;
		
	if ( e->keyval == GDK_Left )
	{
		path=gtk_tree_model_get_path(GTK_TREE_MODEL(data->store),&iter);
		if (!gtk_tree_path_prev(path))
		{
			gtk_tree_path_free(path);
			return false;
		}
		gtk_tree_model_get_iter(GTK_TREE_MODEL(data->store),&iter,path);
		gtk_tree_path_free(path);
	}
	
	if ( e->keyval == GDK_Home )
	{
		if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(data->store),&iter) ) return false;
	}
	
	if ( e->keyval == GDK_End )
	{
		if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(data->store),&test) ) return false;
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(data->store),&iter);
		while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(data->store),&test) )
			gtk_tree_model_iter_next(GTK_TREE_MODEL(data->store),&iter);
	}
	
	if ( (e->keyval == GDK_Up) || (e->keyval == GDK_Down) )
	{
		if ( !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(data->store),&test) ) return false;
		
		gtk_tree_model_get(GTK_TREE_MODEL(data->store),&iter,0,&buf,-1);
		wid=(iconViewWidget*)g_hash_table_lookup(data->elements,buf);
		g_free(buf);
		
		do
		{
			gtk_tree_model_get(GTK_TREE_MODEL(data->store),&test,0,&buf,-1);
			wtest=(iconViewWidget*)g_hash_table_lookup(data->elements,buf);
			g_free(buf);
			
			if ( wtest != wid )
			{
				if ( (e->keyval == GDK_Up) && (wid->bufY <= wtest->bufY) ) continue;
				if ( (e->keyval == GDK_Down) && (wid->bufY >= wtest->bufY) ) continue;
				
				if (!wnear) wnear=wtest;
				else
				{
					diff1=abs((wtest->bufY-wid->bufY)^2)+abs((wtest->bufX-wid->bufX)^2);
					diff2=abs((wnear->bufY-wid->bufY)^2)+abs((wnear->bufX-wid->bufX)^2);
					if (diff1<diff2) wnear=wtest;
				}
			}
			
		} while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(data->store),&test) );
		
		if (wnear) 
			if (!data->iterKey(wnear->key,&iter)) return false;
	}
	
	if ( e->keyval == GDK_space )
	{
		gtk_tree_model_get(GTK_TREE_MODEL(data->store),&iter,0,&buf,-1);
		wid=(iconViewWidget*)g_hash_table_lookup(data->elements,buf);
		if ( (wid->pr->smode==1) || (wid->pr->smode==2) ) ev.type=GDK_BUTTON_PRESS;
		g_free(buf);
	}	
	
		
	gtk_tree_model_get (GTK_TREE_MODEL(data->store),&iter,0,&buf,-1);
	wid=(iconViewWidget*)g_hash_table_lookup (data->elements,buf);
	g_free(buf);
	if ( (wid->pr->smode==0) ) ev.type=GDK_BUTTON_PRESS;
	ev.state=e->state;
	icVW_trySelect(wid->vbox,&ev,wid);
	wid->pr->ensureVisible(wid->key);
	
	
	return true;
}

void gIV_rowDeleted (GtkTreeModel *md,GtkTreePath *path,gIconView *data)
{
	GtkTreeIter iter;

	printf("%d\n",gtk_tree_model_get_iter(md,&iter,path));
                                             
}

void gIV_rowInserted (GtkTreeModel *md,GtkTreePath *arg1,GtkTreeIter *iter,gIconView *data)
{
	iconViewWidget *wid;
	char *buf;
	
	buf=(char*)g_malloc(sizeof(char)*(strlen(data->adding)+1));
	strcpy(buf,data->adding);
	
	wid=new iconViewWidget(data,data->adding);
	g_hash_table_insert(data->elements,buf,(gpointer)wid);	
}

void gIV_rowChanged (GtkTreeModel *md,GtkTreePath *arg1,GtkTreeIter *iter,gIconView *data)
{
	iconViewWidget *wid;
	char *buf;
	int sel;
	GdkPixbuf *pix;
	
	gtk_tree_model_get ( md,iter,0,&buf,-1);
	if (!buf) return;
	
	wid=(iconViewWidget*)g_hash_table_lookup(data->elements,buf);
	g_free(buf);
	if (!wid) return;
	
	gtk_tree_model_get ( md,iter,1,&buf,-1);
	wid->setText(buf);
	if (buf) g_free(buf);
	
	gtk_tree_model_get ( md,iter,2,&pix,-1);
	if (pix)
	{
		wid->setPixbuf(pix);
		g_object_unref(pix);
	}
	
	gtk_tree_model_get ( md,iter,3,&sel,-1);
	if (sel)
		gtk_widget_set_state(wid->vbox,GTK_STATE_SELECTED);
	else
	{
		if (wid->pr->currkey)
		{
			if ( !strcmp( wid->pr->currkey,wid->key ) )
				gtk_widget_set_state(wid->vbox,GTK_STATE_ACTIVE);
			else
				gtk_widget_set_state(wid->vbox,GTK_STATE_NORMAL);
		}
		else
			gtk_widget_set_state(wid->vbox,GTK_STATE_NORMAL);
	}
		
}

bool gIV_equal(char *a,char *b)
{
	if ( !strcasecmp(a,b) ) return true;
	return false;
}

gIconView::gIconView(gControl *parent) : gControl(parent)
{
	GtkSettings *set=gtk_settings_get_default();
	GtkStyle* st=gtk_rc_get_style_by_paths(set,NULL,"GtkEntry",GTK_TYPE_ENTRY);
	
	
	g_typ=Type_gIconView;
		
	currkey=NULL;
	smode=0;
	arr=-1;
	grx=64;
	gry=64;
	elements=g_hash_table_new(g_str_hash,(GEqualFunc)gIV_equal);
	border=gtk_scrolled_window_new(NULL,NULL);
	store=gtk_list_store_new(4,G_TYPE_STRING,G_TYPE_STRING,GDK_TYPE_PIXBUF,G_TYPE_INT);
	widget=gtk_layout_new(NULL,NULL);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(border),widget);
	//gtk_container_add(GTK_CONTAINER(border),widget);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(border),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_widget_set_style(widget,st);
	gtk_widget_set_style(border,st);
	
	GTK_WIDGET_SET_FLAGS(widget,GTK_CAN_FOCUS);
	
	setBorder(true);
	
	onActivate=NULL;
	onSelect=NULL;
	onRename=NULL;
	onClick=NULL;
	connectParent();
	initSignals();
	
	g_signal_connect(G_OBJECT(store),"row-inserted",G_CALLBACK(gIV_rowInserted),(gpointer)this);
	g_signal_connect(G_OBJECT(store),"row-changed",G_CALLBACK(gIV_rowChanged),(gpointer)this);
	g_signal_connect(G_OBJECT(store),"row-deleted",G_CALLBACK(gIV_rowDeleted),(gpointer)this);
	
	g_signal_connect_after(G_OBJECT(widget),"key-press-event",G_CALLBACK(icVW_navigate),(gpointer)this);
	g_signal_connect(G_OBJECT(widget),"button-press-event",G_CALLBACK(icVW_mousePress),(gpointer)this);
	
	g_signal_connect_after(G_OBJECT(border),"size-allocate",G_CALLBACK(icVW_size),(gpointer)this);


}

long gIconView::backGround()
{
	return get_gdk_bg_color(border);
}

void gIconView::setBackGround(long color)
{	
	GtkWidget *lbl;
	
	set_gdk_bg_color(border,color);
	set_gdk_bg_color(widget,color);
	if (!border->window) gtk_widget_realize(border);
	gdk_window_process_updates(border->window,true);
}

long gIconView::clientWidth()
{
	GtkAdjustment *Adj;

	Adj=gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(border));
	return (long)Adj->page_size;
}

long gIconView::clientHeight()
{
	GtkAdjustment *Adj;

	Adj=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(border));
	return (long)Adj->page_size;
}

int gIconView::add(char *key,char *after)
{
	GtkTreeIter iter,aft;
	gPicture *def=NULL;
	
	if (!key) return -3;
	if (iterKey(key,&iter)) return -2;
	if (after)
	{
		if (!iterKey(after,&aft)) return -1;
	}
	
	
	def=gStock::get("32/directory");
	adding=key;
	if (!after)
		gtk_list_store_append(store,&iter);
	else
		gtk_list_store_insert_after(store,&iter,&aft);
	

	gtk_list_store_set(store,&iter,0,key,1,"",2,def->getPixbuf(),3,0,-1);
	delete def;
	drawElements();
	return 0;
}


bool gIconView::exists(char *key)
{
	GtkTreeIter iter;
	
	return iterKey(key,&iter);
}

void gIconView::remove(char *key)
{
	GtkTreeIter iter;
	
	if (!iterKey(key,&iter)) return;
	gtk_list_store_remove(store,&iter);
}

void gIconView::clear()
{
	GtkTreeIter iter;
	
	while ( gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter) )
		gtk_list_store_remove(store,&iter);
}

char* gIconView::current()
{
	return currkey;
}

void gIconView::selectAll(bool vl)
{
	GtkTreeIter iter;
	int vsel=0;
	
	if ( vl && ( (smode==0) || (smode==3) ) ) return;
	
	if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter)) return;
	
	if (vl) vsel=1;
	
	do
	{
		gtk_list_store_set ( store,&iter,3,vsel,-1);
	} while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&iter) );
}

int migranpolla=0;

gint  gIV_compare(GtkTreeModel *model,GtkTreeIter *a,GtkTreeIter *b,gpointer data)
{
	
	return 0;
}

void gIconView::sort()
{
	GtkTreeIter iter,test;
	char *buf1=NULL;
	char *buf2=NULL;
	
	if (!migranpolla)
	{
		gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE(store),
                                             0,
                                             gIV_compare,
                                             NULL,NULL);
	
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store),0,GTK_SORT_ASCENDING);
	}
		
}

/************************************************************************************

gIconView properties

*************************************************************************************/

long gIconView::count()
{
	GtkTreeIter iter;
	long ct=1;
	
	if ( !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter) ) return 0;
	
	while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL(store),&iter) ) ct++;
	return ct;
}

char* gIconView::firstKey()
{
	GtkTreeIter iter;
	char *buf;
	
	if ( !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter) ) return NULL;
	gtk_tree_model_get (GTK_TREE_MODEL(store),&iter,0,&buf,-1);
	return buf;
}

char* gIconView::nextKey(char *vl)
{
	GtkTreeIter iter;
	char *buf;
	
	if (!iterKey(vl,&iter)) return NULL;
	if ( !gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&iter) ) return NULL;
	gtk_tree_model_get( GTK_TREE_MODEL(store),&iter,0,&buf,-1);
	return buf;
}

bool gIconView::getBorder()
{
	if (GTK_SHADOW_NONE==gtk_scrolled_window_get_shadow_type(GTK_SCROLLED_WINDOW(border)))
		return false;
	return true;
}

void gIconView::setBorder(bool vl)
{
	GtkScrolledWindow *wr=GTK_SCROLLED_WINDOW(border);
	
	if (vl)
		gtk_scrolled_window_set_shadow_type(wr,GTK_SHADOW_ETCHED_OUT);
	else
		gtk_scrolled_window_set_shadow_type(wr,GTK_SHADOW_NONE);
}

int gIconView::arrange()
{
	return arr;
}

void gIconView::setArrange(int vl)
{
	if ( (vl>=-1) && (vl<=1) ) 
	{
		arr=vl;
		drawElements();
	}
}

long gIconView::gridX()
{
	return grx;
}

long gIconView::gridY()
{
	return gry;
}

void gIconView::setGrid(long x,long y)
{
	if (x<1) x=1;
	if (y<1) y=1;
	
	grx=x;
	gry=y;
	drawElements();
}

int gIconView::selMode()
{
	return smode;
}

void gIconView::setSelMode(int vl)
{
	switch (vl)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			smode=vl;
			break;
	}
}

void gIconView::resize(long w,long h)
{
	GtkAdjustment *adj;
	
	gControl::resize(w,h);
	//if (arr!=-1) drawElements();
}

void gIconView::ensureVisible(char *key)
{
	GtkAdjustment* Adj;
	iconViewWidget *wid;
	gint px,py;
	
	wid=(iconViewWidget*)g_hash_table_lookup(elements,key);
	if (!wid) return;
	
	gtk_widget_get_size_request( wid->vbox,&px,&py);
	
	Adj=gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(border));
		
	if (wid->bufX < Adj->value)
	{
		gtk_adjustment_set_value(Adj,(gdouble)wid->bufX);
	}
	else if ( (Adj->value+Adj->page_size)<(px+wid->bufX) )
	{
		gtk_adjustment_set_value(Adj,(gdouble)( (px+wid->bufX)-Adj->page_size     ));
	}
	
	
	Adj=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(border));
	
	if (wid->bufY < Adj->value)
	{
		gtk_adjustment_set_value(Adj,(gdouble)wid->bufY);
	}
	else if ( (Adj->value+Adj->page_size)<(py+wid->bufY) )
	{
		gtk_adjustment_set_value(Adj,(gdouble)( (py+wid->bufY)-Adj->page_size     ));
	}
	
	
	
}

void gIconView::moveItem(char *key,long x,long y)
{
	iconViewWidget *wid=(iconViewWidget*)g_hash_table_lookup(elements,key);
	gint imgW,imgH;
	
	if (wid) wid->move(x,y);
}

void gIconView::renameItem(char *key)
{
	iconViewWidget *wid=(iconViewWidget*)g_hash_table_lookup(elements,key);
	
	if (wid) wid->edit();
}

long gIconView::itemLeft(char *key)
{
	iconViewWidget *wid=(iconViewWidget*)g_hash_table_lookup(elements,key);
	
	if (!wid) return 0;
	
	return wid->bufX;
}

long gIconView::itemTop(char *key)
{
	iconViewWidget *wid=(iconViewWidget*)g_hash_table_lookup(elements,key);

	if (!wid) return 0;
	
	return wid->bufY;
}

long gIconView::itemWidth(char* key)
{
	gint bufW,bufH;

	iconViewWidget *wid=(iconViewWidget*)g_hash_table_lookup(elements,key);
	
	if (!wid) return 0;
	
	gtk_widget_get_size_request(wid->vbox,&bufW,&bufH);
	
	return bufW;
}



long gIconView::itemHeight(char *key)
{
	gint bufW,bufH;

	iconViewWidget *wid=(iconViewWidget*)g_hash_table_lookup(elements,key);
	
	if (!wid) return 0;
	
	gtk_widget_get_size_request(wid->vbox,&bufW,&bufH);
	
	return bufH;
}

bool gIconView::itemSelected(char *key)
{
	GtkTreeIter iter;
	gint *buf;
	
	if (!iterKey(key,&iter)) return false;
	gtk_tree_model_get(GTK_TREE_MODEL(store),&iter,3,&buf,-1);
	return (bool)buf;
}

void  gIconView::setItemSelected(char *key,bool vl)
{
	GtkTreeIter iter;
	
	if (!iterKey(key,&iter)) return;
	if (vl)
		gtk_list_store_set(store,&iter,3,1,-1);
	else
		gtk_list_store_set(store,&iter,3,0,-1);
}

bool gIconView::itemEditable(char *key)
{
	iconViewWidget *wid;
	
	if (!key) return false;
	wid=(iconViewWidget*)g_hash_table_lookup (elements,key);
	if (!wid) return false;
	return wid->bEdit;
}

void gIconView::setItemEditable(char *key,bool vl)
{
	iconViewWidget *wid;
	
	if (!key) return;
	wid=(iconViewWidget*)g_hash_table_lookup (elements,key);
	if (!wid) return;
	return wid->setEditable(vl);
}

char* gIconView::itemText(char *key)
{
	GtkTreeIter iter;
	char *buf;
	
	if (!iterKey(key,&iter)) return NULL;
	gtk_tree_model_get(GTK_TREE_MODEL(store),&iter,1,&buf,-1);
	return buf;
}

void  gIconView::setItemText(char *key,char *vl)
{
	GtkTreeIter iter;
	
	if (!iterKey(key,&iter)) return;
	gtk_list_store_set(store,&iter,1,vl,-1);
}

gPicture* gIconView::itemPicture(char *key)
{
	GtkTreeIter iter;
	gPicture *pic=NULL;
	GdkPixbuf *buf;
	
	if (!iterKey(key, &iter)) return NULL;
	
	gtk_tree_model_get(GTK_TREE_MODEL(store),&iter,2,&buf,-1);
	if (!buf) return NULL;
	
	pic=gPicture::fromPixbuf(buf);
	g_object_unref(G_OBJECT(buf));
	return pic;
	
}

void gIconView::setItemPicture(char *key,gPicture *vl)
{
	GtkTreeIter iter;
	GdkPixbuf *buf=NULL;
	
	if (!iterKey(key,&iter)) return;
	
	if (vl) buf=vl->getPixbuf();
	
	if (buf)
	{
		gtk_list_store_set(store,&iter,2,(gpointer)buf,-1);
		g_object_unref(G_OBJECT(buf));
	}
}

bool gIconView::iterKey(char *key, GtkTreeIter *iter)
{
	char *cmp;
	
	if (!key) return false;
	if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),iter) ) return false;
	
	do
	{
		gtk_tree_model_get(GTK_TREE_MODEL(store),iter,0,&cmp,-1);
        if (cmp)
		{
			if ( !strcasecmp(key,cmp) )
			{
				g_free(cmp);
				return true;
			}
			g_free(cmp);
		}
	} while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(store),iter) );
	
	return false;
}


int gIconView::scrollBar()
{
	GtkPolicyType h,v;
	long ret=3;
	
	gtk_scrolled_window_get_policy(GTK_SCROLLED_WINDOW(border),&h,&v);
	if (h==GTK_POLICY_NEVER) ret--;
	if (v==GTK_POLICY_NEVER) ret-=2;
	
	return ret;
}

void gIconView::setScrollBar(int vl)
{
	GtkScrolledWindow *sc=GTK_SCROLLED_WINDOW(border);
	switch(vl)
	{
		case 0:
			gtk_scrolled_window_set_policy(sc,GTK_POLICY_NEVER,GTK_POLICY_NEVER);
			break;
		case 1:
			gtk_scrolled_window_set_policy(sc,GTK_POLICY_AUTOMATIC,GTK_POLICY_NEVER);
			break;
		case 2:
			gtk_scrolled_window_set_policy(sc,GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
			break;
		case 3:
			gtk_scrolled_window_set_policy(sc,GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
			break;
	}
}

void gIconView::drawElements()
{
	GtkTreeIter iter;
	iconViewWidget *wid;
	char *buf;
	long Xpos=0,Ypos=0;
	gint imgW,imgH;
	gint lblH;
	long res;
	long diff;
	long hStep=0;
	long Xmax;
	long layW=1,layH=1;
	GtkAdjustment* Adj;
	
	if (!gdk_window_is_viewable(border->window)) return;
	
	if ( !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter) ) return;
	
	if (arr != 1)
		Adj=gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(border));
	else
		Adj=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(border));
	
	Xmax=(long)Adj->page_size;
	
	
	do
	{
		gtk_tree_model_get(GTK_TREE_MODEL(store),&iter,0,&buf,-1);
		
		if (buf)
		{
			wid=(iconViewWidget*)g_hash_table_lookup(elements,buf);
			g_free(buf);
		}
		
		gtk_widget_get_size_request(wid->vbox,&imgW,&imgH);
		
		if (arr != 1 )
		{
			diff=imgW-grx;
			if (diff<0) diff=0;
			else        diff/=2;
			
			res=imgW;
		
			if ( (Xpos+imgW+diff+16)>=Xmax)
			{
				Xpos=0;
				Ypos+=hStep;
				hStep=0;
			}
		
			while (res>0) { Xpos+=grx; res-=grx; }
			if (layW < (Xpos-diff) ) layW=Xpos-diff;
		
			wid->move(Xpos-imgW-diff,Ypos);
		
			if ( (imgH+8)>hStep) hStep=imgH+8;
			if ( layH < (Ypos+imgH) ) layH=Ypos+imgH;
		}
		else
		{
			diff=imgH-gry;
			if (diff<0) diff=0;
			else        diff/=2;
			
			res=imgH;
		
			if ( (Ypos+imgH+diff+16)>=Xmax)
			{
				Ypos=0;
				Xpos+=hStep;
				hStep=0;
			}
		
			while (res>0) { Ypos+=gry; res-=gry; }
			if (layH < (Ypos-diff) ) layH=Ypos-diff;
		
			wid->move(Xpos,Ypos-imgH-diff);
		
			if ( (imgW+8)>hStep) hStep=imgW+8;
			if ( layW < (Xpos+imgW) ) layW=Xpos+imgW;
		}
		
	
	} while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&iter) );
	
	gtk_widget_set_size_request(widget,layW,layH);
}

