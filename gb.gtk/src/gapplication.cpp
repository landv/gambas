/***************************************************************************

  gapplication.cpp

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

#include <ctype.h>
#include <time.h>

#include "widgets.h"
#include "widgets_private.h"
#include "gapplication.h"
#include "gtrayicon.h"
#include "gdesktop.h"
#include "gkey.h"
#include "gmenu.h"
#include "gmessage.h"
#include "gdialog.h"
#include "gclipboard.h"
#include "gmainwindow.h"

/*************************************************************************

gKey

**************************************************************************/

bool gKey::_valid = false;
GdkEventKey gKey::_event;
GtkIMContext *gKey::_im_context = NULL;
GtkWidget *gKey::_im_widget = NULL;
char *_im_text = NULL;

const char *gKey::text()
{
	if (!_valid) 
		return 0;
	else
		return _event.string;
}

int gKey::code()
{
	if (!_valid)
		return 0;
	else
		return _event.keyval;
}

int gKey::state()
{
	if (!_valid)
		return 0;
	else
		return _event.state;
}

bool gKey::alt()
{
	return state() & GDK_MOD1_MASK || code() == GDK_Alt_L || code() == GDK_Alt_R;
}

bool gKey::control()
{
	return state() & GDK_CONTROL_MASK || code() == GDK_Control_L || code() == GDK_Control_R;
}

bool gKey::meta()
{
	return state() & GDK_MOD2_MASK || code() == GDK_Meta_L || code() == GDK_Meta_R;
}

bool gKey::normal()
{
	return (state() & 0xFF) != 0;
}

bool gKey::shift()
{
	return state() & GDK_SHIFT_MASK || code() == GDK_Shift_L || code() == GDK_Shift_R;
}

int gKey::fromString(char *str)
{
	char *lstr = g_ascii_strdown(str, -1);
	int key = gdk_keyval_from_name(lstr);
	g_free(lstr);
	return key;
}

void gKey::disable()
{
	if (!_valid)
		return;
		
	_valid = false;
	_event.keyval = 0;
	_event.state = 0;
	g_free(_event.string);
}

bool gKey::enable(GtkWidget *widget, GdkEventKey *event)
{
	bool filter;
	
	if (widget != _im_widget)
		return true;
	
	if (_valid)
		disable();
		
	_valid = true;
	_event = *event;
	
  //filter = gtk_im_context_filter_keypress(_im_context, &_event);
  //fprintf(stderr, "gKey::enable: filter = %d event->string = '%s'\n", filter, event->string);
  
  if (filter && _im_text)
  {
		_event.string = g_strdup(_im_text);
  	filter = false;
  }
  else
  	_event.string = g_strdup(_event.string);
  
  if (!filter)
  {
  	//fprintf(stderr, "gKey::enable: gtk_im_context_reset\n");
  	//gtk_im_context_reset(_im_context);
  	if (_im_text)
  	{
  		g_free(_im_text);
  		_im_text = NULL;
  	}
  }
  
  //fprintf(stderr, "gKey::enable: --> %d\n", filter);
  return filter;
}

static void cb_im_commit(GtkIMContext *context, const gchar *str, gpointer pointer)
{
	//fprintf(stderr, "cb_im_commit: %s\n", str);
	
	if (_im_text)
		g_free(_im_text);
		
	_im_text = g_strdup(str);
}

void gKey::init()
{
	_im_context = gtk_im_multicontext_new();
  g_signal_connect (_im_context, "commit", G_CALLBACK(cb_im_commit), NULL);
}

void gKey::exit()
{
	disable();
	if (_im_text)
		g_free(_im_text);
	g_object_unref(_im_context);
}

void gKey::setActiveControl(gControl *control)
{
	if (_im_widget)
	{
		//fprintf(stderr, "gtm_im_context_focus_out\n");
	  gtk_im_context_set_client_window (_im_context, 0);
		gtk_im_context_focus_out(_im_context);
		_im_widget = NULL;
	}
	
	if (control)
	{
		_im_widget = control->widget;
	  gtk_im_context_set_client_window (_im_context, _im_widget->window);
		gtk_im_context_focus_in(_im_context);
		gtk_im_context_reset(_im_context);
		//fprintf(stderr, "gtm_im_context_focus_in\n");
	}
}

/********************************************************************

gApplication

*********************************************************************/

int appEvents;
GtkTooltips *app_tooltips_handle;
bool app_tooltips=true;
gFont *app_tooltips_font=NULL;

bool gApplication::_busy = false;
char *gApplication::_title = NULL;

GtkTooltips* gApplication::tipHandle()
{
	return app_tooltips_handle;
}

bool gApplication::toolTips()
{
	return app_tooltips;
}

void gApplication::setToolTipsFont(gFont *ft)
{
	GList *chd;
	PangoFontDescription *desc;
	
	gFont::assign(&app_tooltips_font, ft->copy());
	
	if (ft)
    desc = pango_context_get_font_description(ft->ct);
  else
    desc = NULL;
	
	gtk_widget_modify_font(app_tooltips_handle->tip_window,desc);
	
	chd=gtk_container_get_children(GTK_CONTAINER(app_tooltips_handle->tip_window));
	if (chd)
	{
		do { gtk_widget_modify_font(GTK_WIDGET(chd->data),desc);
		} while (chd->next);
		g_list_free(chd);
	}
}

gFont *gApplication::toolTipsFont()
{
	return app_tooltips_font;
}

void gApplication::enableTooltips(bool vl)
{
	if (vl)
		gtk_tooltips_enable(app_tooltips_handle);
	else
		gtk_tooltips_disable(app_tooltips_handle);
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

void gApplication::init(int *argc,char ***argv)
{
	appEvents=0;
	
	gtk_init(argc,argv);
	
	app_tooltips_handle = gtk_tooltips_new();
	g_object_ref(G_OBJECT(app_tooltips_handle));
	gtk_tooltips_force_window (app_tooltips_handle);
	app_tooltips_font = new gFont(app_tooltips_handle->tip_window);
	
	gClipboard::init();
	gKey::init();
}

void gApplication::exit()
{
	if (_title)
		g_free(_title);

	gKey::exit();
	gTrayIcon::exit();
  gDesktop::exit();
  gMessage::exit();
  gDialog::exit();
  gFont::assign(&app_tooltips_font);
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
	GList *iter;
	
	if (!wid) return NULL;

	if (gControl::controlList())
	{
		iter=g_list_first(gControl::controlList());
		
		while (iter)
		{
			if (((gControl*)iter->data)->border == wid )
				return (gControl*)iter->data;
				
			if (((gControl*)iter->data)->widget == wid )
				return (gControl*)iter->data;
		
			iter=iter->next;
		}
		
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
    
    if (control->mouse() != -1 || control->have_cursor)
    	control->setMouse(control->mouse());
    
    iter = g_list_next(iter);
  }
  
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
