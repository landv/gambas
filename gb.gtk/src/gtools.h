#ifndef __GTOOLS_H
#define __GTOOLS_H

#include "gpicture.h"
#include "gcontrol.h"

void stub(char *function);

int clipBoard_Type();
char* clipBoard_Format();
void clipBoard_Clear();
char* clipBoard_getText();
gPicture* clipBoard_getImage();
void clipBoard_setText(char *text,char *format);
void clipBoard_setImage(gPicture *image);

bool drag_IsEnabled();
int drag_Action();
int drag_Type();
char* drag_Format();
char* drag_Text();
gPicture* drag_Image();
long drag_X();
long drag_Y();
gControl* drag_Widget();
void drag_setIcon(gPicture *pic);
gPicture* drag_Icon();

void gt_exit();

void g_stradd(gchar **res, const gchar *s);

char *gt_free_later(char *ptr);

GdkPixbuf *gt_pixbuf_create_disabled(GdkPixbuf *img);

void gt_shortcut_parse(char *shortcut, guint *key, GdkModifierType *mods);

char* gt_html_to_pango_string(char *html, bool newline);

int gt_to_alignment(double halign, double valign = 0.5);
double gt_from_alignment(int align, bool vertical = false);

// Global signal handlers
gboolean gcb_keypress (GtkWidget *widget, GdkEventKey *event, gControl *data);
gboolean gcb_keyrelease (GtkWidget *widget, GdkEventKey *event, gControl *data);

// Where to scroll to ensure that a specific area is visible

typedef
	struct {
		int clientWidth;
		int clientHeight;
		int scrollX;
		int scrollY;
		int scrollWidth;
		int scrollHeight;
		}
	GtEnsureVisible;
	
void gt_ensure_visible(GtEnsureVisible *arg, int x, int y, int w, int h);

#endif
