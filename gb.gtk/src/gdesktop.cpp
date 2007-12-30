/***************************************************************************

  gdesktop.cpp

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

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#endif

#include "gmouse.h"
#include "gapplication.h"
#include "gmainwindow.h"
#include "gdesktop.h"

static bool gDrag_Enabled=false;
static long gDrag_X=0;
static long gDrag_Y=0;
static int gDrag_Action=0;
static int gDrag_Type=0;
static char *gDrag_Format=NULL;
static char *gDrag_Text=NULL;
static GdkPixbuf *gDrag_Pic=NULL;
static GdkPixbuf *gDrag_icon=NULL;

static GtkClipboard *clipBoard=NULL;

static int _desktop_scale = 0;

/***********************************************************************

Clipboard

************************************************************************/
void clipBoard_Init()
{
		if (!clipBoard) clipBoard=gtk_clipboard_get_for_display(gdk_display_get_default         (),GDK_SELECTION_CLIPBOARD);

}



void clipBoard_Clear()
{
	gtk_clipboard_clear(clipBoard);
}

char* clipBoard_getText()
{
	gchar *buf=NULL;
	char *bufOut;
	
	if ( gtk_clipboard_wait_is_text_available(clipBoard) )
		buf=gtk_clipboard_wait_for_text(clipBoard);
		
	if (!buf) return NULL;
	
	bufOut=(char*)g_malloc(sizeof(char)*(strlen(buf)+1));
	strcpy(bufOut,buf);
	g_free(buf);
	return bufOut;
}

char* clipBoard_Format()
{
	gint n_tg;
	gchar *buf;
	char *bufOut;
	bool ok;
	GdkAtom *targets;
	long bucle,b2,b3;
	long pos=-1;

	if (gtk_clipboard_wait_for_targets  (clipBoard,&targets,&n_tg))
	{
		for (bucle=0;bucle<n_tg;bucle++)
		{
			buf=gdk_atom_name(targets[bucle]);
			for (b2=0;b2<strlen(buf);b2++)
			{
				if ( buf[b2]=='/' )
				{
					if (pos==-1) pos=bucle;
					
					ok=true;
					for (b3=0;b3<strlen(buf);b3++)	
						if (buf[b3]==';') { ok=false; break; }
					if (ok) { pos=bucle; bucle=n_tg; }
					
					break;
				}
			}
			g_free(buf);
		}
		
		if (pos==-1) pos=0;
		
		buf=gdk_atom_name(targets[pos]);
		bufOut=(char*)g_malloc(sizeof(char)*(strlen(buf)+1));
		strcpy(bufOut,buf);
		g_free(buf);
		return bufOut;
	}
	
	return NULL;
}

void cB_clearFunc(GtkClipboard *clipboard, gpointer clipBoardText)
{
	if (clipBoardText) g_free(clipBoardText);
}

void cB_getFunc(GtkClipboard *clip,GtkSelectionData *selection,guint info,gpointer clipBoardText)
{
	gtk_selection_data_set_text(selection,(const char*)clipBoardText,-1);
}


int clipBoard_Type()
{
	if (gtk_clipboard_wait_is_text_available(clipBoard)) return 1;
	if (gtk_clipboard_wait_is_image_available(clipBoard)) return 2;
	
	return 0;
	
}

void clipBoard_setImage(gPicture *image)
{
	gtk_clipboard_set_image(clipBoard,image->getPixbuf());
}

gPicture* clipBoard_getImage()
{
  return new gPicture(gtk_clipboard_wait_for_image(clipBoard));
} 

void clipBoard_setText(char *text,char *format)
{
  GtkTargetList *list;
  GList *l;
  GtkTargetEntry *targets;
  gint n_t, i;
  gpointer clipBoardText;

  gtk_clipboard_clear(clipBoard);
  
  clipBoardText=(gpointer)g_malloc(sizeof(char)*(strlen(text)+1));
  strcpy((char*)clipBoardText,text);

  list = gtk_target_list_new (NULL, 0);
  gtk_target_list_add (list,gdk_atom_intern(format,false),0,0);
  gtk_target_list_add_text_targets (list, 0);
  
  n_t = g_list_length (list->list);
  targets = g_new0 (GtkTargetEntry, n_t);
  for (l = list->list, i = 0; l; l = l->next, i++)
  {
  	GtkTargetPair *pair = (GtkTargetPair *)l->data;
    targets[i].target = gdk_atom_name (pair->target);
  }
  

  gtk_clipboard_set_with_data (clipBoard,targets, n_t,cB_getFunc, cB_clearFunc,clipBoardText);
  gtk_clipboard_set_can_store (clipBoard,NULL,0);
  
  g_free (targets);
  gtk_target_list_unref (list);

}
/***********************************************************************
Drag & Drop
************************************************************************/
//"Private"
GdkPixbuf *gDrag_getIcon()
{
	return gDrag_icon;
}


void gDrag_Enable(long x,long y,int action)
{
	gDrag_Enabled=true;
	gDrag_X=x;
	gDrag_Y=y;
	gDrag_Action=action;
}

void gDrag_Disable()
{
	gDrag_Enabled=false;
}

void gDrag_setTarget(int type,char *buf)
{
	gDrag_Type=type;
	if (gDrag_Format) { g_free(gDrag_Format); gDrag_Format=NULL;}
	if (buf)
	{
		gDrag_Format=(char*)g_malloc(sizeof(char)*(strlen(buf)+1));
		strcpy(gDrag_Format,buf);
	}
	
}

void gDrag_setText(char *buf)
{
	if (gDrag_Text) { g_free(gDrag_Text); gDrag_Text=NULL; }
	if (!buf) return;
	gDrag_Text=(char*)g_malloc(sizeof(char)*(strlen(buf)+1));
	strcpy(gDrag_Text,buf);
}

void gDrag_setImage(char *buf,long len)
{
	GdkPixbufLoader *ld;
	
	if (gDrag_Pic) { g_object_unref(G_OBJECT(gDrag_Pic)); gDrag_Pic=NULL; }
	if (!buf) return;
	
	ld = gdk_pixbuf_loader_new ();
	if (gdk_pixbuf_loader_write (ld,(const guchar*)buf,len, NULL))
	{
		gDrag_Pic=gdk_pixbuf_loader_get_pixbuf (ld);
		if (gDrag_Pic) g_object_ref(G_OBJECT(gDrag_Pic));
	}
	gdk_pixbuf_loader_close (ld, NULL);
	g_object_unref (G_OBJECT(ld));
	
}

void gDrag_Clear()
{
	if (gDrag_Format) { g_free(gDrag_Format); gDrag_Format=NULL; }
	if (gDrag_Text) { g_free(gDrag_Text); gDrag_Text=NULL; }
}

//"Public"
int drag_Action()
{
	return gDrag_Action;
}

int drag_Type()
{
	return gDrag_Type;
}

char* drag_Format()
{
	return gDrag_Format;
}

char* drag_Text()
{
	return gDrag_Text;
}

gPicture* drag_Image()
{
	gPicture *ret;

	if (!gDrag_Pic) return NULL;
	g_object_ref(G_OBJECT(gDrag_Pic));
	return new gPicture(gDrag_Pic);
}

bool drag_IsEnabled()
{
	return gDrag_Enabled;
}

long drag_X()
{
	if (!gDrag_Enabled) return -1;
	return gDrag_X;
}

long drag_Y()
{
	if (!gDrag_Enabled) return -1;
	return gDrag_Y;
}

gControl* drag_Widget()
{
	if (!gDrag_Enabled) return NULL;
	return gControl::dragWidget();
}

gPicture* drag_Icon()
{
	gPicture *ret;	

	if (!gDrag_icon) return NULL;
	ret=gPicture::fromPixbuf(gDrag_icon);
	return ret;
}

void drag_setIcon(gPicture *pic)
{
	if (gDrag_icon) { g_object_unref(G_OBJECT(gDrag_icon)); gDrag_icon=NULL; }
	if (!pic) return;
	if (!pic->pic) return;
	gDrag_icon=pic->getPixbuf();
}

/***********************************************************************

gCursor

************************************************************************/
gCursor::gCursor(gPicture *pic,long px,long py)
{
	GdkPixbuf *buf;
	GdkDisplay *dp=gdk_display_get_default();
	
	x=px;
	y=py;
	
	cur=NULL;
	if (!pic) return;
	if (!pic->pic) return;
	
	if (pic->width()<=x) x=pic->width()-1;
	if (pic->height()<=y) y=pic->height()-1;
	
	buf=pic->getPixbuf();
	cur=gdk_cursor_new_from_pixbuf(dp,buf,x,y);
	g_object_unref(buf);
	
}

gCursor::gCursor(gCursor *cursor)
{
	cur=NULL;
	if (!cursor) return;
	if (!cursor->cur) return;
	
	cur=cursor->cur;
	x=cursor->x;
	y=cursor->y;
	if (cur) gdk_cursor_ref(cur);
}

gCursor::~gCursor()
{
	if (cur) gdk_cursor_unref(cur);
}

long gCursor::left()
{
	return x;
}

long gCursor::top()
{
	return y;
}

/***********************************************************************

gMouse

************************************************************************/
//"Properties"
bool gMouse_isValid=false;
long gMouse_screen_x=0;
long gMouse_screen_y=0;
long gMouse_x=0;
long gMouse_y=0;
long gMouse_button=0;
long gMouse_genButton=0;
long gMouse_state=0;
long gMouse_delta=0;
long gMouse_orientation=0;

void gMouse::move(long x,long y)
{
	
	GdkDisplay* dpy;
	GdkWindow*  win=gdk_get_default_root_window();
	#ifdef GDK_WINDOWING_X11
	dpy=gdk_display_get_default();
	XWarpPointer(GDK_DISPLAY_XDISPLAY(dpy),GDK_WINDOW_XID(win),GDK_WINDOW_XID(win),0, 0,0, 0,x, y);
	#else
	stub("no-X11/gMouse::move");
	#endif
}

long gMouse::button()
{
	if (!gMouse_isValid) return false;
	return gMouse_genButton; 
}

bool gMouse::left()
{
	if (!gMouse_isValid) return false;
	return gMouse_genButton & 1;
}

bool gMouse::right()
{
	if (!gMouse_isValid) return false;
	return gMouse_genButton & 2;
}

bool gMouse::middle()
{
	if (!gMouse_isValid) return false;
	return gMouse_genButton & 4;
}

bool gMouse::shift()
{
	if (!gMouse_isValid) return false;
	return gMouse_state & GDK_SHIFT_MASK;
}

bool gMouse::control()
{
	if (!gMouse_isValid) return false;
	return gMouse_state & GDK_CONTROL_MASK;
}

bool gMouse::alt()
{
	if (!gMouse_isValid) return false;
	return gMouse_state & GDK_MOD1_MASK;
}

bool gMouse::meta()
{
	if (!gMouse_isValid) return false;
	return gMouse_state & GDK_MOD2_MASK;
}

bool gMouse::normal()
{
	if (!gMouse_isValid) return false;
	return !(gMouse_state & 0xFF);
}

long gMouse::x()
{
	if (!gMouse_isValid) return -1;
	return gMouse_x;
}

long gMouse::y()
{
	if (!gMouse_isValid) return -1;
	return gMouse_y;
}

long gMouse::screenX()
{
	gint x;
	gdk_display_get_pointer(gdk_display_get_default(), NULL, &x, NULL, NULL);
	return x;
}

long gMouse::screenY()
{
	gint y;
	gdk_display_get_pointer(gdk_display_get_default(), NULL, NULL, &y, NULL);
	return y;
}

long gMouse::delta()
{
	if (!gMouse_isValid) return -1;
	return gMouse_delta;
}

long gMouse::orientation()
{
	if (!gMouse_isValid) return -1;
	return gMouse_orientation;
}

bool gMouse::valid()
{
	return gMouse_isValid;
}



//"Private"
void gMouse::setWheel(long dt,long orn)
{
	gMouse_delta=dt;
	gMouse_orientation=orn;
}

void gMouse::setValid(int vl,long xvl,long yvl,long button_val,long state_val,long sx,long sy)
{
	gMouse_delta=0;
	gMouse_orientation=0;
	gMouse_isValid=vl;
	if (vl) { 
		gMouse_x=xvl; 
		gMouse_y=yvl; 
		gMouse_screen_x=xvl+sx;
		gMouse_screen_y=yvl+sy;
		gMouse_button=button_val; 
		gMouse_state=state_val; 
		gMouse_genButton=0;

		switch(gMouse_button)
		{
			case 1: gMouse_genButton=1; break;
			case 2: gMouse_genButton=4; break;
			case 3: gMouse_genButton=2; break;
		}
	
		if (vl!=2)
		{
			if (gMouse_state & GDK_BUTTON1_MASK) gMouse_genButton+=1;
			if (gMouse_state & GDK_BUTTON2_MASK) gMouse_genButton+=4;
			if (gMouse_state & GDK_BUTTON3_MASK) gMouse_genButton+=2;
		}
	}
		
}
	
/***********************************************************************

Desktop

************************************************************************/
gFont *desktop_font=NULL;

bool gDesktop::rightToLeft()
{
	return gtk_widget_get_default_direction() == GTK_TEXT_DIR_RTL;
}

void gDesktop::init()
{
	desktop_font = new gFont();
}

void gDesktop::exit()
{
  gFont::assign(&desktop_font);
}

gFont* gDesktop::font()
{
	if (!desktop_font) gDesktop::init();
	return desktop_font;
}

void gDesktop::setFont(gFont *ft)
{
  gFont::assign(&desktop_font, ft);
  _desktop_scale = 0;
}

gControl* gDesktop::activeControl()
{
	gControl *test, *curr=NULL;
	GList *iter=gControl::controlList();

	if (!iter) return NULL;

	iter=g_list_first(iter);
	while (iter)
	{
		test=(gControl*)iter->data;
		if (test->hasFocus())
		{
			curr=test;
			break;
		}
		iter=iter->next;
	}
	
	return curr;
	
}

gMainWindow* gDesktop::activeWindow()
{
	return gMainWindow::_active ? gMainWindow::_active->topLevel() : NULL;
}

gColor gDesktop::buttonfgColor()
{
	GtkSettings *set=gtk_settings_get_default();
	GtkStyle* st=gtk_rc_get_style_by_paths(set,NULL,"GtkButton",GTK_TYPE_BUTTON);
	
	return get_gdk_color(&st->fg[GTK_STATE_NORMAL]);
}

gColor gDesktop::buttonbgColor()
{
	GtkSettings *set=gtk_settings_get_default();
	GtkStyle* st=gtk_rc_get_style_by_paths(set,NULL,"GtkButton",GTK_TYPE_BUTTON);
	
	return get_gdk_color(&st->bg[GTK_STATE_NORMAL]);
}

gColor gDesktop::fgColor()
{	
	GtkSettings *set=gtk_settings_get_default();
	GtkStyle* st=gtk_rc_get_style_by_paths(set,NULL,"GtkEntry",GTK_TYPE_ENTRY);
	
	return get_gdk_color(&st->fg[GTK_STATE_NORMAL]);
}

gColor gDesktop::bgColor()
{
	GtkSettings *set=gtk_settings_get_default();
	GtkStyle* st=gtk_rc_get_style_by_paths(set,NULL,"GtkLayout",GTK_TYPE_LAYOUT);
	
	return get_gdk_color(&st->bg[GTK_STATE_NORMAL]);
}

gColor gDesktop::textfgColor()
{
	GtkSettings *set=gtk_settings_get_default();
	GtkStyle* st=gtk_rc_get_style_by_paths(set,NULL,"GtkEntry",GTK_TYPE_ENTRY);
	
	return get_gdk_color(&st->text[GTK_STATE_NORMAL]);
}

gColor gDesktop::textbgColor()
{
	GtkSettings *set=gtk_settings_get_default();
	GtkStyle* st=gtk_rc_get_style_by_paths(set,NULL,"GtkEntry",GTK_TYPE_ENTRY);
	
	return get_gdk_color(&st->base[GTK_STATE_NORMAL]);
}

gColor gDesktop::selfgColor()
{
	GtkSettings *set=gtk_settings_get_default();
	GtkStyle* st=gtk_rc_get_style_by_paths(set,NULL,"GtkEntry",GTK_TYPE_ENTRY);
	
	return get_gdk_color(&st->text[GTK_STATE_SELECTED]);
}

gColor gDesktop::selbgColor()
{
	GtkSettings *set=gtk_settings_get_default();
	GtkStyle* st=gtk_rc_get_style_by_paths(set,NULL,"GtkEntry",GTK_TYPE_ENTRY);
	
	return get_gdk_color(&st->base[GTK_STATE_SELECTED]);
}

long gDesktop::height()
{
	return gdk_screen_get_height(gdk_screen_get_default ());
}

long gDesktop::width()
{
	return gdk_screen_get_width(gdk_screen_get_default ());
}

long gDesktop::resolution()
{
	long d_pix=gdk_screen_get_width(gdk_screen_get_default());
	long d_mm=gdk_screen_get_width_mm(gdk_screen_get_default());
	
	return (d_pix*25.4)/d_mm;

}

int gDesktop::scale()
{
	PangoLanguage *lng=NULL;
	PangoContext* ct=gdk_pango_context_get();
	GtkStyle *sty=gtk_widget_get_default_style();
	PangoFontDescription *ft=sty->font_desc;
	PangoFontMetrics* fm;
	int val;
	
	if (!_desktop_scale)
	{
		if (getenv("LANG")) 
			lng = pango_language_from_string(getenv("LANG"));
			
		fm = pango_context_get_metrics (ct,ft,lng);
		
		val = (pango_font_metrics_get_ascent(fm) + pango_font_metrics_get_descent(fm)) / 2;
		val /= PANGO_SCALE;
		
		pango_font_metrics_unref(fm);
		g_object_unref(G_OBJECT(ct));
		
		if (!val) val = 1;
		_desktop_scale = val;
	}
	
	return _desktop_scale;
}

gPicture* gDesktop::grab()
{
	return Grab_gdkWindow(gdk_get_default_root_window());	
	
}


