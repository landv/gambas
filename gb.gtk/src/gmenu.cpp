/***************************************************************************

  gmenu.cpp

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
#include "gmainwindow.h"
#include "gmenu.h"

GList *menus=NULL;

static void mnu_destroy (GtkWidget *object,gMenu *data)
{
	if (data->stop_signal) 
		data->stop_signal = false; 
	else
		delete data;
}

static void mnu_activate(GtkMenuItem *menuitem,gMenu *data)
{
	if (data->onClick) data->onClick(data);
}

static gboolean cb_map(GtkWidget *menu, gMenu *data)
{
	data->hideSeparators();
	if (data->onShow) (*data->onShow)(data);
	return false;
}

static gboolean cb_unmap(GtkWidget *menu, gMenu *data)
{
	if (data->onHide) (*data->onHide)(data);
	return false;
}

static int get_menu_pos(GtkWidget *menu)
{
  GtkMenuShell *par;
	GList *iter;
	int pos;
	
  par = (GtkMenuShell*)(menu->parent);
  iter = g_list_first(par->children);
  
  for(pos = 0;; pos++) 
  {
    if (iter->data == (gpointer)menu) 
      break; 
    iter = g_list_next(iter);
  }
  
  return pos;
}

static gboolean cb_check_expose(GtkWidget *wid, GdkEventExpose *e, gMenu *menu)
{
	int x, y, w, h;
	
	if (menu->checked())
	{
		x = wid->allocation.x;
		y = wid->allocation.y;
		w = wid->allocation.width;
		h = wid->allocation.height;
		if (h > w)
		{
			y += (h - w) / 2;
			h = w;
		}
		else if (w > h)
		{
			x += (w - h) / 2;
			w = h;
		}
		
		gtk_paint_check(wid->style, wid->window, (GtkStateType)GTK_WIDGET_STATE(wid), GTK_SHADOW_IN, 
			&e->area, NULL, NULL, 
			x + 1, y + 1, w - 2, h - 2); 
	}
	
	return false;
}

void gMenu::update()
{
	GtkWidget *hbox;
	GtkMenuShell *shell;
	gint pos;
	
	if (_no_update)
		return;
	
	if (_style != _oldstyle)
	{
		if (child)
		{
			g_object_ref(G_OBJECT(child));
			if (_style == MENU)
				gtk_menu_item_remove_submenu(menu);
		}

		if (menu)
		{
			pos = get_menu_pos(GTK_WIDGET(menu));
			shell = (GtkMenuShell*)GTK_WIDGET(menu)->parent;
			gtk_widget_destroy(GTK_WIDGET(menu));
		}
		else
		{
			pos = -1;
			shell = NULL;
		}
		
		if (_style != NOTHING)
		{
			if (_style == SEPARATOR)
			{
				menu = (GtkMenuItem *)gtk_separator_menu_item_new();
			}
			else if (_style == MENU)
			{
				menu = (GtkMenuItem *)gtk_menu_item_new();
				
				hbox = gtk_hbox_new(false, 4);
				gtk_container_add(GTK_CONTAINER(menu), GTK_WIDGET(hbox));
				
				label = gtk_label_new_with_mnemonic("");
				//gtk_label_set_justify(GTK_LABEL(lbl),GTK_JUSTIFY_LEFT);
				
				if (!top_level)
				{
					image = gtk_image_new();
					
					aclbl = gtk_label_new("");
					gtk_misc_set_alignment(GTK_MISC(aclbl), 0, 0.5);
					gtk_size_group_add_widget(((gMenu*)pr)->sizeGroup, aclbl);
					
					check = gtk_image_new();
					gtk_widget_set_size_request(check, 16, 16);
					g_signal_connect_after(G_OBJECT(check), "expose-event", G_CALLBACK(cb_check_expose), (gpointer)this);
				
					gtk_box_pack_start(GTK_BOX(hbox), check, false, false, 0);
					gtk_box_pack_start(GTK_BOX(hbox), image, false, false, 0);
					gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 0);
					gtk_box_pack_end(GTK_BOX(hbox), aclbl, false, false, 0);				
				}
				else
				{
					check = NULL;
					image = NULL;
					aclbl = NULL;
					gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 0);
				}
				
				if (child)
				{
					gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu), GTK_WIDGET(child));
					g_object_unref(G_OBJECT(child));
				}
					
				
				//set_gdk_fg_color(label, get_gdk_fg_color(GTK_WIDGET(shell)));
				//set_gdk_bg_color(label, get_gdk_bg_color(GTK_WIDGET(shell)));
			}
			
			if (shell)
				gtk_menu_shell_insert(shell, GTK_WIDGET(menu), pos);
			
			gtk_widget_show_all(GTK_WIDGET(menu));
			
			if (top_level)
			{
				gMainWindow *win = (gMainWindow *)pr;
				
				gtk_menu_shell_append(GTK_MENU_SHELL(win->menuBar), GTK_WIDGET(menu));
				
				set_gdk_fg_color(GTK_WIDGET(menu), win->foreground());
				set_gdk_bg_color(GTK_WIDGET(menu), win->background());	
			}
			else
			{
				gMenu *parent = (gMenu *)pr;
				
				if (!parent->child)
				{
					parent->child = (GtkMenu*)gtk_menu_new();
					g_signal_connect(G_OBJECT(parent->child), "map", G_CALLBACK(cb_map), (gpointer)parent);
					g_signal_connect(G_OBJECT(parent->child), "unmap", G_CALLBACK(cb_unmap), (gpointer)parent);
					gtk_widget_show_all(GTK_WIDGET(parent->child));
					if (parent->style() == MENU)
						gtk_menu_item_set_submenu(GTK_MENU_ITEM(parent->menu), GTK_WIDGET(parent->child));
				}
	
				gtk_menu_shell_append(GTK_MENU_SHELL(parent->child), GTK_WIDGET(menu));
			}
	
			g_signal_connect(G_OBJECT(menu), "destroy", G_CALLBACK(mnu_destroy), (gpointer)this);
			g_signal_connect(G_OBJECT(menu), "activate", G_CALLBACK(mnu_activate), (gpointer)this);
		}
		
		_oldstyle = _style;
	}
	
	if (_style == MENU)
	{
		{
			char *buf;
			gMnemonic_correctText(_text, &buf);
			gtk_label_set_text_with_mnemonic(GTK_LABEL(label), buf);
			g_free(buf);
		}
		
		if (!top_level)
		{
			char *buf;
			
			if (_shortcut)
			{
				buf = g_strconcat("    ", _shortcut ,"  ",NULL);
				gtk_label_set_text(GTK_LABEL(aclbl), buf);
				g_free(buf);
			}
			else
				gtk_label_set_text(GTK_LABEL(aclbl), "      ");
		}
		
		if (!top_level)
		{
			GdkPixbuf *buf;
			buf = _picture ? _picture->getPixbuf() : 0;
			gtk_image_set_from_pixbuf(GTK_IMAGE(image), buf);
			
			if (buf)
				gtk_widget_show(image);
			else
				gtk_widget_hide(image);
				
			if (_checked || !buf)
				gtk_widget_show(check);
			else
				gtk_widget_hide(check);			
		}
		else
		{
			//gtk_widget_hide(image);
			//gtk_widget_hide(check);			
		}
	}
}

void gMenu::initialize()
{
	//fprintf(stderr, "gMenu::gMenu: %p (%p)\n", this, pr);
	
	stop_signal = false;
	
	onFinish = NULL;
	onClick = NULL;
	onShow = NULL;
	onHide = NULL;
	
	hFree = NULL;
	child = NULL;
	image = NULL;
	label = NULL;
	aclbl = NULL;
	check = NULL;
	menu = NULL;
	top_level=false;
	
	_text = NULL;
	_shortcut = NULL;
	_checked = false;
	_picture = NULL;
	_name = NULL;
	
	_style = NOTHING;
	_oldstyle = NOTHING;
	
	_no_update = false;
	_destroyed = false;
	_action = false;
	
	sizeGroup=gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	
	menus=g_list_append (menus,(gpointer)this);
}

gMenu::gMenu(gMainWindow *par,bool hidden)
{
	//gControl *ctTop;
	GtkVBox *box;

	pr = (gpointer)par;
  initialize();
	top_level=true;
	
	// BM: embedded windows can have menu bars
	//ctTop=(gControl*)par;
	//while (ctTop->pr) ctTop=ctTop->pr;
	//par=(gMainWindow*)ctTop;
	
	accel = par->accel;
	g_object_ref(accel);
	
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
		set_gdk_fg_color(GTK_WIDGET(par->menuBar),par->foreground());
		set_gdk_bg_color(GTK_WIDGET(par->menuBar),par->background());
	}
	
	update();
}

gMenu::gMenu(gMenu *par, bool hidden)
{
	pr=(gpointer)par;
	
	initialize();
	sizeGroup=gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	
	if (!par) return;
	if (!par->menu) return;
	
	accel = par->accel;
	g_object_ref(accel);

	update();
}

gMenu::~gMenu()
{
	GList *item;
	gMenu *mn;
	
	if (_destroyed)
		return;
		
	//fprintf(stderr, "gMenu::~gMenu: %p (%p) '%s'\n", this, pr, _text);
	
	_destroyed = true;
  // Remove references to me
  
	item = g_list_first(menus);
	while (item)
	{
		mn = (gMenu*)item->data;
		if (mn->pr == (void*)this)
		  mn->pr = NULL;
		item = g_list_next(item);
	}
	
	if (aclbl && (!top_level) && pr)
		gtk_size_group_remove_widget(((gMenu*)pr)->sizeGroup, aclbl);
	
	if (G_OBJECT_TYPE(menu)!=GTK_TYPE_MENU_ITEM && child)
		gtk_widget_destroy(GTK_WIDGET(child));

	if (sizeGroup) g_object_unref(G_OBJECT(sizeGroup));
	
	g_object_unref(accel);
	
	if (onFinish) onFinish(this);
	
	_no_update = true;
	setText(NULL);
	setShortcut(NULL);
	setPicture(NULL);
	_style = NOTHING;
	
	stop_signal = true;
	gtk_widget_destroy(GTK_WIDGET(menu));
	
	menus = g_list_remove(menus, (gpointer)this);
}

bool gMenu::enabled()
{
	return GTK_WIDGET_SENSITIVE(GTK_WIDGET(menu));
}

void gMenu::setEnabled(bool vl)
{
	gtk_widget_set_sensitive(GTK_WIDGET(menu),vl);
}


void gMenu::setText(char *text)
{
	g_free(_text);
	if (text)
		_text = g_strdup(text);
	else
		_text = NULL;
		
	if (!_text || !*_text)
		_style = SEPARATOR;
	else
		_style = MENU;
	
	update();
}

bool gMenu::isVisible()
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
	
	if (top_level && pr)
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

void gMenu::setPicture(gPicture *pic)
{
	//fprintf(stderr, "gMenu::setPicture: %p\n", pic);
	gPicture::assign(&_picture, pic);
	update();
}

void gMenu::setChecked(bool vl)
{
	_checked = vl;
	update();	
}

void gMenu::setToggle(bool vl)
{
	_toggle = vl;
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
	if (!_destroyed)
		delete this;
}

void gMenu::popup()
{
	guint32 time = gtk_get_current_event_time();
	
	gtk_menu_popup(child,NULL,NULL,NULL,NULL,0,time);
}

long gMenu::winChildCount(gMainWindow *par)
{
	GList *item;
	gMenu *mn;
	long ct=0;
	
	if (!menus) return 0;
	
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

void gMenu::setShortcut(char *shortcut)
{
	guint key;
	GdkModifierType mods;
	
	if (_shortcut)
	{
		gt_shortcut_parse(_shortcut, &key, &mods);
		if (key)
			gtk_widget_remove_accelerator(GTK_WIDGET(menu), accel, key, mods);
		g_free(_shortcut);
		_shortcut = NULL;
	}

	if (shortcut)
	{
		_shortcut = g_strdup(shortcut);
		gt_shortcut_parse(_shortcut, &key, &mods);
		if (key)
			gtk_widget_add_accelerator(GTK_WIDGET(menu),"activate",accel,key,mods,(GtkAccelFlags)0);
	}

	update();
}

gMainWindow *gMenu::window()
{
  if (!pr)
    return NULL;

  if (top_level)
    return (gMainWindow *)pr;
    
  return ((gMenu *)pr)->window();
}

void gMenu::setName(char *name)
{
	if (_name) g_free(_name);
	_name = NULL;
	if (name) _name = g_strdup(name);
}


void gMenu::hideSeparators()
{
	gMenu *ch;
	gMenu *last_ch;
	bool is_sep;
	bool last_sep;
	GList *item;

	if (!child)
		return;
		
	last_sep = true;
	last_ch = 0;
	
	item = g_list_first(menus);
	while (item)
	{
		ch = (gMenu*)item->data;
		if (ch->pr == this)
		{
			is_sep = ch->style() == SEPARATOR;
			if (is_sep)
			{
				if (last_sep)
				{
					ch->hide();
				}
				else
				{
					ch->show();
					last_sep = true;
					last_ch = ch;
				}
			}
			else
			{
				if (ch->isVisible())
					last_sep = false;
			}
		}
		
		item = g_list_next(item);
	}
	
	if (last_sep && last_ch)
		last_ch->hide();
}
