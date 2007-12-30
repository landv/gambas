/***************************************************************************

  gmenu.cpp

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
#include <string.h>
#include <stdlib.h>

#include <stdio.h>

static const char * _checked_xpm[] = {
"12 12 2 1",
" 	c None",
".	c #000000",
"            ",
" ..      .. ",
"  ..    ..  ",
"   ..  ..   ",
"    ....    ",
"     ..     ",
"     ..     ",
"    ....    ",
"   ..  ..   ",
"  ..    ..  ",
" ..      .. ",
"            "
};

GList *menus=NULL;

void mnu_destroy (GtkWidget *object,gMenu *data)
{
	if (data->aclbl && (!data->top_level) )
		gtk_size_group_remove_widget( ((gMenu*)data->pr)->sizeGroup , data->aclbl);
	
	if (data->stop_signal) { data->stop_signal=false; return; }
	if (G_OBJECT_TYPE(data->menu)!=GTK_TYPE_MENU_ITEM)
		if (data->child)  gtk_widget_destroy(GTK_WIDGET(data->child));

	if (data->sizeGroup) g_object_unref(G_OBJECT(data->sizeGroup));
	
	delete data;
	
}

void mnu_activate(GtkMenuItem *menuitem,gMenu *data)
{
	if (data->onClick) data->onClick(data);
}

gMenu::gMenu(gMainWindow *par,bool hidden)
{
	gControl *ctTop;
	GtkVBox *box;
	GtkHBox *hbox;

	sizeGroup=gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	scut=NULL;
	top_level=true;
	stop_signal=false;
	onFinish=NULL;
	onClick=NULL;
	hFree=NULL;
	chk=false;
	img_buf=NULL;
	child=NULL;
	img=NULL;
	lbl=NULL;
	aclbl=NULL;
	txt=NULL;
	menu=NULL;
	
	ctTop=(gControl*)par;
	while (ctTop->pr) ctTop=ctTop->pr;
	par=(gMainWindow*)ctTop;
	
	pr=(gpointer)par;
	accel=par->accel;
	menus=g_list_append (menus,(gpointer)this);
	
	if (!par->menuBar)
	{
		//g_object_ref(par->widget);
		//gtk_container_remove(GTK_CONTAINER(par->border),par->widget);
		box=(GtkVBox*)gtk_vbox_new(false,0);
		par->menuBar=(GtkMenuBar*)gtk_menu_bar_new();
		gtk_box_pack_start(GTK_BOX(box),GTK_WIDGET(par->menuBar),false,true,0);
		gtk_widget_reparent(par->widget,GTK_WIDGET(box));
		gtk_container_add(GTK_CONTAINER(par->border),GTK_WIDGET(box));
		
		//gtk_container_add(GTK_CONTAINER(box),par->widget);
		gtk_widget_show_all(GTK_WIDGET(box));
		//g_object_unref(par->widget);
		gtk_widget_show_all(GTK_WIDGET(par->menuBar));
		set_gdk_fg_color(GTK_WIDGET(par->menuBar),par->foreGround());
		set_gdk_bg_color(GTK_WIDGET(par->menuBar),par->backGround());
	}
	
	menu=(GtkMenuItem*)gtk_separator_menu_item_new();
	gtk_widget_show_all(GTK_WIDGET(menu));
	gtk_menu_shell_append(GTK_MENU_SHELL(par->menuBar),GTK_WIDGET(menu));
	set_gdk_fg_color(GTK_WIDGET(menu),par->foreGround());
	set_gdk_bg_color(GTK_WIDGET(menu),par->backGround());
	
	g_signal_connect(G_OBJECT(menu),"destroy",G_CALLBACK(mnu_destroy),(gpointer)this);
	g_signal_connect(G_OBJECT(menu),"activate",G_CALLBACK(mnu_activate),(gpointer)this);
}

gMenu::gMenu(gMenu *par,bool hidden)
{
	GtkHBox *hbox;
	
	sizeGroup=gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	scut=NULL;
	top_level=false;
	stop_signal=false;
	onFinish=NULL;
	onClick=NULL;
	hFree=NULL;
	chk=false;
	img_buf=NULL;
	child=NULL;
	menu=NULL;
	img=NULL;
	lbl=NULL;
	aclbl=NULL;
	txt=NULL;
	
	pr=(gpointer)par;
	menus=g_list_append (menus,(gpointer)this);
	
	if (!par) return;
	if (!par->menu) return;
	
	accel=par->accel;
	menu=(GtkMenuItem*)gtk_separator_menu_item_new();
	gtk_widget_show_all(GTK_WIDGET(menu));

	if (!par->child)
	{
		par->child=(GtkMenu*)gtk_menu_new();
		gtk_widget_show_all(GTK_WIDGET(par->child));
		if (G_OBJECT_TYPE(par->menu)==GTK_TYPE_MENU_ITEM)
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(par->menu),GTK_WIDGET(par->child));
		else
			g_object_ref(G_OBJECT(par->child));

	}
		
	gtk_menu_shell_append(GTK_MENU_SHELL(par->child),GTK_WIDGET(menu));
	
	g_signal_connect(G_OBJECT(menu),"destroy",G_CALLBACK(mnu_destroy),(gpointer)this);
	g_signal_connect(G_OBJECT(menu),"activate",G_CALLBACK(mnu_activate),(gpointer)this);
}

gMenu::~gMenu()
{
	if (onFinish) onFinish(this);
	if (img_buf) g_object_unref(G_OBJECT(img_buf));
	menus=g_list_remove(menus,(gpointer)this);
}

bool gMenu::enabled()
{
	return GTK_WIDGET_SENSITIVE(GTK_WIDGET(menu));
}

void gMenu::setEnabled(bool vl)
{
	gtk_widget_set_sensitive(GTK_WIDGET(menu),vl);
}

char* gMenu::text()
{
	return txt;
}


void gMenu::setText(char* vl)
{
	GtkHBox *hbox;
	GtkMenuShell *par;
	GList *iter;
	int npos=-1;
	char *buf=NULL;
	
	if (txt) { g_free(txt); txt=NULL; }
	
	if (vl)
	{
		vl=g_strchomp(vl);
		txt=(char*)g_malloc((strlen(vl)+1)*sizeof(char));
		strcpy(txt,vl);
	}
	
	if (!strlen(vl))
	{
		if (G_OBJECT_TYPE(menu)==GTK_TYPE_MENU_ITEM)
		{
			stop_signal=true;
			par=(GtkMenuShell*)GTK_WIDGET(menu)->parent;
			iter=g_list_first(par->children);
			while (true)
			{
				npos++;
				if (iter->data==(gpointer)menu) break; 
				iter=g_list_next(iter);
			}
			
			if (child) g_object_ref(G_OBJECT(child));
			gtk_menu_item_remove_submenu(menu);
			gtk_widget_destroy(GTK_WIDGET(menu));
			
			menu=(GtkMenuItem*)gtk_separator_menu_item_new();
			gtk_menu_shell_insert(par,GTK_WIDGET(menu),npos);
			gtk_widget_show_all(GTK_WIDGET(menu));
			
			lbl=NULL;
			aclbl=NULL;
			img=NULL;
			g_signal_connect(G_OBJECT(menu),"destroy",G_CALLBACK(mnu_destroy),(gpointer)this);
			g_signal_connect(G_OBJECT(menu),"activate",G_CALLBACK(mnu_activate),(gpointer)this);
		}
		
	}
	else
	{
		if (G_OBJECT_TYPE(menu)!=GTK_TYPE_MENU_ITEM)
		{
			stop_signal=true;
			par=(GtkMenuShell*)GTK_WIDGET(menu)->parent;
			iter=g_list_first(par->children);
			while (true)
			{
				npos++;
				if (iter->data==(gpointer)menu) break; 
				iter=g_list_next(iter);
			}
			gtk_widget_destroy(GTK_WIDGET(menu));
			
			menu=(GtkMenuItem*)gtk_menu_item_new();
			hbox=(GtkHBox*)gtk_hbox_new(false,2);
			lbl=gtk_label_new_with_mnemonic("");
			gtk_label_set_justify(GTK_LABEL(lbl),GTK_JUSTIFY_LEFT);
			aclbl=gtk_label_new("");
			if (!top_level) gtk_size_group_add_widget( ((gMenu*)pr)->sizeGroup,aclbl);
			gtk_misc_set_alignment(GTK_MISC(aclbl),0,0.5);
			if (scut) 
			{
				buf=g_strconcat(" (",scut,")",NULL);
				gtk_label_set_text(GTK_LABEL(aclbl),buf);
				g_free(buf);
				buf=NULL;
			}
			
			if (img_buf) 
			{
				img=gtk_image_new_from_pixbuf(img_buf);
				gtk_box_pack_start(GTK_BOX(hbox),img,false,false,0);
			}
			
			gtk_box_pack_start(GTK_BOX(hbox),lbl,false,false,0);
			gtk_box_pack_end(GTK_BOX(hbox),aclbl,false,false,0);
			gtk_container_add(GTK_CONTAINER(menu),GTK_WIDGET(hbox));
			gtk_widget_show_all(GTK_WIDGET(hbox));
			gtk_menu_shell_insert(par,GTK_WIDGET(menu),npos);
			gtk_widget_show_all(GTK_WIDGET(menu));
			
			set_gdk_fg_color(lbl,get_gdk_fg_color(GTK_WIDGET(par)));
			set_gdk_bg_color(lbl,get_gdk_bg_color(GTK_WIDGET(par)));
			
			if (child) 
			{
				gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu),GTK_WIDGET(child));
				g_object_unref(G_OBJECT(child));
			}
			setChecked(checked());
			g_signal_connect(G_OBJECT(menu),"destroy",G_CALLBACK(mnu_destroy),(gpointer)this);
			g_signal_connect(G_OBJECT(menu),"activate",G_CALLBACK(mnu_activate),(gpointer)this);
		}
		
		gMnemonic_correctText(vl,&buf);
		gtk_label_set_text_with_mnemonic(GTK_LABEL(lbl),buf);
		g_free(buf);
	}
}

bool gMenu::visible()
{
	if (!menu) return false;
	return GTK_WIDGET_VISIBLE(menu);	
}

void gMenu::setVisible(bool vl)
{
	GtkContainer *par;
	GList *chd,*iter;

	if (!menu) return;
	g_object_set(G_OBJECT(menu),"visible",vl,NULL);
	
	if (this->top_level)
	{
		par=GTK_CONTAINER(((gMainWindow*)pr)->menuBar);
		if (vl==true)
		{
			g_object_set(G_OBJECT(par),"visible",vl,NULL);
			return;
		}
		chd=gtk_container_get_children(par);
		iter=g_list_first(chd);
		while (iter)
		{
			if ( GTK_WIDGET_VISIBLE (GTK_WIDGET(iter->data)) )
			{
				g_list_free(chd);
				return;
			}
			iter=g_list_next(iter);
		}
		g_list_free(chd);
		g_object_set(G_OBJECT(par),"visible",vl,NULL);
	}
}

void gMenu::setImage(GdkPixbuf *buf)
{
	GtkHBox *box;
	
	if (!menu) return;
	
	if (buf)
	{
		setImage(NULL);
		img_buf=gdk_pixbuf_copy(buf);
		if ( (G_OBJECT_TYPE(menu)==GTK_TYPE_MENU_ITEM) && (!top_level))
		{
			img=gtk_image_new_from_pixbuf(buf);
			box=(GtkHBox*)gtk_bin_get_child(GTK_BIN(menu));
			g_object_ref(G_OBJECT(lbl));
			g_object_ref(G_OBJECT(aclbl));
			gtk_container_remove(GTK_CONTAINER(box),lbl);
			gtk_container_remove(GTK_CONTAINER(box),aclbl);
			gtk_box_pack_start(GTK_BOX(box),img,false,false,0);
			gtk_box_pack_start(GTK_BOX(box),lbl,false,false,0);
			gtk_box_pack_end(GTK_BOX(box),aclbl,false,false,0);
			gtk_widget_show_all(GTK_WIDGET(box));
			g_object_unref(G_OBJECT(lbl));
			g_object_unref(G_OBJECT(aclbl));
		}
		
	}
	else
	{
		if (img_buf) { g_object_unref(G_OBJECT(img_buf)); img_buf=NULL; }
		if (G_OBJECT_TYPE(menu)==GTK_TYPE_MENU_ITEM)
		{
			if (img)
			{
				box=(GtkHBox*)gtk_bin_get_child(GTK_BIN(menu));
				gtk_container_remove(GTK_CONTAINER(box),img);
			}
			img=NULL;
		}
	}
}

gPicture* gMenu::picture()
{
	GdkPixbuf *buf;
	gPicture *pic;

	if (!img_buf) return NULL;
	pic=gPicture::fromPixbuf(img_buf);
	return pic;

}


void gMenu::setPicture (gPicture *pic)
{
	GdkPixbuf *buf;

	if (!pic)
	{
		setImage(NULL);
		setChecked(checked());
		return;
	}
	
	if (!pic->pic)
	{
		setImage(NULL);
		setChecked(checked());
		return;
	}
	
	buf=pic->getPixbuf();
	setImage(buf);
	g_object_unref(G_OBJECT(buf));
	setChecked(checked());
}

bool gMenu::checked()
{
	return chk;
}

void gMenu::setChecked(bool vl)
{
	GdkPixbuf *mycheck;
	GtkHBox *box;

	if (top_level) return;
	
	if (vl)
	{
		if (G_OBJECT_TYPE(menu)==GTK_TYPE_MENU_ITEM)
		{
			if (img)
			{
				box=(GtkHBox*)gtk_bin_get_child(GTK_BIN(menu));
				gtk_container_remove(GTK_CONTAINER(box),img);
			}
			mycheck=gdk_pixbuf_new_from_xpm_data(_checked_xpm);
			img=gtk_image_new_from_pixbuf(mycheck);
			box=(GtkHBox*)gtk_bin_get_child(GTK_BIN(menu));
			g_object_ref(G_OBJECT(lbl));
			g_object_ref(G_OBJECT(aclbl));
			gtk_container_remove(GTK_CONTAINER(box),lbl);
			gtk_container_remove(GTK_CONTAINER(box),aclbl);
			gtk_box_pack_start(GTK_BOX(box),img,false,false,0);
			gtk_box_pack_start(GTK_BOX(box),lbl,false,false,0);
			gtk_box_pack_end(GTK_BOX(box),aclbl,false,false,0);
			gtk_widget_show_all(GTK_WIDGET(box));
			g_object_unref(G_OBJECT(lbl));
			g_object_unref(G_OBJECT(aclbl));
			g_object_unref(G_OBJECT(mycheck));
		}
		
	}
	else
	{
		if (img)
		{
			box=(GtkHBox*)gtk_bin_get_child(GTK_BIN(menu));
			gtk_container_remove(GTK_CONTAINER(box),img);
			img=NULL;
			if (img_buf)
			{
				img=gtk_image_new_from_pixbuf(img_buf);
				box=(GtkHBox*)gtk_bin_get_child(GTK_BIN(menu));
				g_object_ref(G_OBJECT(lbl));
				g_object_ref(G_OBJECT(aclbl));
				gtk_container_remove(GTK_CONTAINER(box),lbl);
				gtk_container_remove(GTK_CONTAINER(box),aclbl);
				gtk_box_pack_start(GTK_BOX(box),img,false,false,0);
				gtk_box_pack_start(GTK_BOX(box),lbl,false,false,0);
				gtk_box_pack_end(GTK_BOX(box),aclbl,false,false,0);
				gtk_widget_show_all(GTK_WIDGET(box));
				g_object_unref(G_OBJECT(lbl));
				g_object_unref(G_OBJECT(aclbl));
			}
		}
	}
	
	chk=vl;
}

long gMenu::childCount()
{
	GList *item;
	gMenu *mn;
	long ct=0;
	
	if (!menus) return 0;
	
	item=g_list_first(menus);
	while (item)
	{
		mn=(gMenu*)item->data;
		if (mn->pr == (void*)this) ct++;
		item=g_list_next(item);
	}
	
	return ct;
}

gMenu* gMenu::childMenu(long pos)
{
	GList *item;
	gMenu *mn;
	long ct=0;
	
	if (!menus) return NULL;
	
	item=g_list_first(menus);
	while (item)
	{
		mn=(gMenu*)item->data;
		if (mn->pr == (void*)this)
		{
			if (ct==pos) return mn;
			ct++;
		}
		item=g_list_next(item);
	}
	
	return NULL;
}

void gMenu::destroy()
{
	GtkContainer *shell;
	
	if (!menu) { delete this; return; }
	shell=(GtkContainer*)GTK_WIDGET(menu)->parent;
	gtk_container_remove(shell,GTK_WIDGET(menu));
	
	
}

void gMenu::popup()
{
	guint32 time=gtk_get_current_event_time();
	
	gtk_menu_popup(child,NULL,NULL,NULL,NULL,0,time);
}

long gMenu::winChildCount(gMainWindow *par)
{
	GList *item;
	gMenu *mn;
	long ct=0;
	
	if (!menus) return NULL;
	
	item=g_list_first(menus);
	while (item)
	{
		mn=(gMenu*)item->data;
		if (mn->pr == (void*)par) ct++;
		item=g_list_next(item);
	}
	
	return ct;
}

gMenu* gMenu::winChildMenu(gMainWindow *par,long pos)
{
	GList *item;
	gMenu *mn;
	long ct=0;
	
	if (!menus) return NULL;
	
	item=g_list_first(menus);
	while (item)
	{
		mn=(gMenu*)item->data;
		if (mn->pr == (void*)par)
		{
			if (ct==pos) return mn;
			ct++;
		}
		item=g_list_next(item);
	}
	
	return NULL;
}

char* gMenu::shortcut()
{
	return scut;
}

void gMenu::setShortcut(char *tx)
{
	gchar **cads;
	gchar *res,*cat;
	guint key;
	GdkModifierType mods;
	long bucle;
	
	res=(char*)g_malloc(sizeof(char));
	res[0]=0;
	
	if (!tx) tx="";
	cads=g_strsplit(tx,"+",0);
	
	bucle=0;
	while (cads[bucle]!=NULL)
	{
		g_strstrip(cads[bucle]);
		bucle++;
	}
	
		bucle=0;
	while (cads[bucle]!=NULL)
	{
		if (!strcasecmp(cads[bucle],"ctrl"))
		{
			cat=g_strconcat (res,"<Ctrl>",NULL);
			g_free(res);
			res=cat;
		}
		else if (!strcasecmp(cads[bucle],"shift"))
		{
			cat=g_strconcat (res,"<Shift>",NULL);
			g_free(res);
			res=cat;
		}
		else if (!strcasecmp(cads[bucle],"alt"))
		{
			cat=g_strconcat (res,"<Alt>",NULL);
			g_free(res);
			res=cat;
		}
		else
		{
			cat=g_strconcat(res,cads[bucle],NULL);
			g_free(res);
			res=cat;
		}
		
		bucle++;
	}
	g_strfreev(cads);
	gtk_accelerator_parse(res,&key,&mods);
	g_free(res);
	if (key) gtk_widget_add_accelerator(GTK_WIDGET(menu),"activate",accel,key,mods,(GtkAccelFlags)0);
	
	if (scut) { g_free(scut); scut=NULL; }
	if (strlen(tx))
	{
		scut=(char*)g_malloc(sizeof(char)*(strlen(tx)+1));
		strcpy(scut,tx);
	}
	
	if (aclbl)
	{
		if (scut)
		{
			res=g_strconcat(" (",scut,")",NULL);
			gtk_label_set_text(GTK_LABEL(aclbl),res);
			g_free(res);
		}
		else
			gtk_label_set_text(GTK_LABEL(aclbl),"");
			
	}
	
}

