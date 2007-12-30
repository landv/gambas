/***************************************************************************

  widgets_private.h

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

#include <gtk/gtk.h>


class alphaCache
{
public:
	long count;
	long vx;
	long vy;
	unsigned char *buffer;
//"Public"
	alphaCache(GdkPixbuf *buf);
	alphaCache(long x,long y);
	~alphaCache();
	bool active();
	void fillAlpha(GdkPixbuf *buf);
	void copyArea(alphaCache *orig,long src_x,long src_y);
	void ref();
	void unref();
};


void clipBoard_Init();

GdkPixbuf *gDrag_getIcon();
void gDrag_Enable(long x,long y,int action);
void gDrag_Disable();
void gDrag_Clear();
void gDrag_setTarget(int type,char *buf);
void gDrag_setText(char *buf);
void gDrag_setImage(char *buf,long len);

void sg_destroy (GtkWidget *object,gControl *data);
gboolean sg_key_Press (GtkWidget *widget, GdkEventKey *event, gControl *data);
gboolean sg_key_Release (GtkWidget *widget, GdkEventKey *event, gControl *data);
gboolean sg_button_Press (GtkWidget *widget,GdkEventButton *event,gControl *data);
gboolean sg_button_Release (GtkWidget *widget,GdkEventButton *event,gControl *data);
gboolean sg_motion(GtkWidget *widget,GdkEventMotion *event,gControl *data);
gboolean sg_focus_In(GtkWidget *widget,GdkEventFocus *event,gControl *data);
gboolean sg_focus_Out(GtkWidget *widget,GdkEventFocus *event,gControl *data);
gboolean sg_event(GtkWidget *widget, GdkEvent *event,gControl *data);
gboolean sg_enter(GtkWidget *widget, GdkEventCrossing *e,gControl *data);

void gMnemonic_correctText(char *st,char **buf);
guint gMnemonic_correctMarkup(char *st,char **buf);
void gMnemonic_returnText(char *st,char **buf);

long gStoreList_Count(GtkListStore *store);
void gStoreList_setList(GtkListStore *store,char *vl);
void gStoreList_setArrayList(GtkListStore *store,char **vl);
void gStoreList_List(GtkListStore *store,char **ret);
char** gStoreList_ArrayList(GtkListStore *store);
void gStoreList_Remove(GtkListStore *store,long pos);
long gStoreList_Find(GtkListStore *store,char *ptr);
void gStoreList_Clear(GtkListStore *store);
char* gStoreList_itemText(GtkListStore *store,long ind);
void gStoreList_sortData(GtkListStore *store);
int gStoreList_Iter(GtkListStore *store,GtkTreeIter *iter,long ind);
void gStoreList_setItemSelected(GtkListStore *store,GtkTreeSelection *sel,long ind,bool vl);
bool gStoreList_itemSelected(GtkTreeSelection *sel,long ind);


/* Tools */
long get_gdk_color(GdkColor *gcol);
void fill_gdk_color(GdkColor *gcol,long color);
long get_gdk_text_color(GtkWidget *wid);
void set_gdk_text_color(GtkWidget *wid,long color);
long get_gdk_base_color(GtkWidget *wid);
void set_gdk_base_color(GtkWidget *wid,long color);
long get_gdk_fg_color(GtkWidget *wid);
void set_gdk_fg_color(GtkWidget *wid,long color);
long get_gdk_bg_color(GtkWidget *wid);
void set_gdk_bg_color(GtkWidget *wid,long color);
int Frame_getBorder(GtkFrame *fr);
void Frame_setBorder(GtkFrame *fr,int vl);
gPicture *Grab_gdkWindow(GdkWindow *win);
