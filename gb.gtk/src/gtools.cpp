/***************************************************************************

  gtools.cpp

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
#include "gcolor.h"
#include "gdesktop.h"
#include "gtools.h"
#include "gpicture.h"

void stub(char *function)
{
	printf("WARNING: %s not yet implemented\n",function);
}

/*******************************************************************

Conversion between GDK and long type colors

********************************************************************/

#define SCALE(i) ((int)(i * 255.0 / 65535.0 + 0.5))
#define UNSCALE(d) ((int)(d / 255.0 * 65535.0 + 0.5))

void fill_gdk_color(GdkColor *gcol,gColor color, GdkColormap *cmap)
{
	int r, g, b;
	
	if (!cmap)
		cmap=gdk_colormap_get_system();
	
	gt_color_to_rgb(color, &r, &g, &b);
	
	gcol->red = UNSCALE(r);
	gcol->green = UNSCALE(g);
	gcol->blue = UNSCALE(b);
	
	gdk_color_alloc(cmap, gcol);
}

gColor get_gdk_color(GdkColor *gcol)
{
	return gt_rgb_to_color(SCALE(gcol->red), SCALE(gcol->green), SCALE(gcol->blue));
}

gColor get_gdk_fg_color(GtkWidget *wid)
{
	GtkStyle* st;

	st=gtk_widget_get_style(wid);	
	return get_gdk_color(&st->fg[GTK_STATE_NORMAL]);
}

void set_gdk_fg_color(GtkWidget *wid, gColor color)
{
	GdkColor gcol;

	if (color == COLOR_DEFAULT)
	{
		gtk_widget_modify_fg (wid,GTK_STATE_NORMAL, NULL);
		return;	
	}

	if (get_gdk_fg_color(wid)==color) return;
	
	fill_gdk_color(&gcol,color);
	gtk_widget_modify_fg (wid,GTK_STATE_NORMAL,&gcol);	
}

gColor get_gdk_bg_color(GtkWidget *wid)
{
	GtkStyle* st;

	st=gtk_widget_get_style(wid);	
	return get_gdk_color(&st->bg[GTK_STATE_NORMAL]);
}

void set_gdk_bg_color(GtkWidget *wid,gColor color)
{
	GdkColor gcol;

	if (color == COLOR_DEFAULT)
	{
		gtk_widget_modify_bg(wid,GTK_STATE_NORMAL, NULL);
		return;	
	}

	if (get_gdk_bg_color(wid)==color) return;
	
	fill_gdk_color(&gcol,color);
	gtk_widget_modify_bg (wid,GTK_STATE_NORMAL,&gcol);
	//gtk_widget_modify_bg (wid,GTK_STATE_ACTIVE,&gcol);
	//gtk_widget_modify_bg (wid,GTK_STATE_PRELIGHT,&gcol);
	//gtk_widget_modify_bg (wid,GTK_STATE_SELECTED,&gcol);
}

gColor get_gdk_text_color(GtkWidget *wid)
{
	GtkStyle* st;

	st=gtk_widget_get_style(wid);	
	return get_gdk_color(&st->text[GTK_STATE_NORMAL]);
}

void set_gdk_text_color(GtkWidget *wid,gColor color)
{
	GdkColor gcol;

	if (color == COLOR_DEFAULT)
	{
		gtk_widget_modify_text(wid, GTK_STATE_NORMAL, NULL);
		return;	
	}

	if (get_gdk_text_color(wid)==color) return;
	
	fill_gdk_color(&gcol,color);
	gtk_widget_modify_text (wid,GTK_STATE_NORMAL,&gcol);	
}

gColor get_gdk_base_color(GtkWidget *wid)
{
	GtkStyle* st;

	st=gtk_widget_get_style(wid);
	return get_gdk_color(&st->base[GTK_STATE_NORMAL]);
}

void set_gdk_base_color(GtkWidget *wid,gColor color)
{
	GdkColor gcol;
	
	if (color == COLOR_DEFAULT)
	{
		gtk_widget_modify_base(wid, GTK_STATE_NORMAL, NULL);
		return;	
	}

	if (get_gdk_base_color(wid)==color) return;
	
	fill_gdk_color(&gcol,color);
	gtk_widget_modify_base (wid,GTK_STATE_NORMAL,&gcol);	
}

void gt_color_to_rgb(gColor color, int *r, int *g, int *b)
{
	*b = color & 0xFF;
	*g = (color >> 8) & 0xFF;
	*r = (color >> 16) & 0xFF;
}

gColor gt_rgb_to_color(int r, int g, int b)
{
	return (gColor)(b | (g << 8) | (r << 16));
}

void gt_color_to_rgba(gColor color, int *r, int *g, int *b, int *a)
{
	*b = color & 0xFF;
	*g = (color >> 8) & 0xFF;
	*r = (color >> 16) & 0xFF;
	*a = (color >> 24) & 0xFF;
}

gColor gt_rgba_to_color(int r, int g, int b, int a)
{
	return (gColor)(b | (g << 8) | (r << 16) | (a << 24));
}

void gt_rgb_to_hsv(int r, int g, int b, int *H, int *S,int *V )
{
	float R = (float)r, G = (float)g, B = (float)b;
	float v, x, f;
	int i;

	R/=255;
	G/=255;
	B/=255;

	x=R;
	if (G<x) x=G;
	if (B<x) x=B;

	v=R;
	if (G>v) v=G;
	if (B>v) v=B;


	if(v == x) {
		*H=-1;
		*S=0;
		*V = (int)(v * 255);
		return;
	}

	f = (R == x) ? G - B : ((G == x) ? B - R : R - G);
	i = (R == x) ? 3 : ((G == x) ? 5 : 1);
	*H=(int)((i - f /(v - x))*60);
	*S=(int)(((v - x)/v)*255);
	*V=(int)(v*255);
}

void gt_hsv_to_rgb(int h, int s, int v, int *R, int *G, int *B)
{
	 double H,S,V;
	 double var_h,var_i,var_1,var_2,var_3,tmp_r,tmp_g,tmp_b;

	 H=((double)h)/360;
	 S=((double)s)/255;
	 V=((double)v)/255;

	if ( S == 0 )
	{
		*R = (int)(V * 255);
		*G = (int)(V * 255);
		*B = (int)(V * 255);
	}
	else
	{
		var_h = H * 6;
		var_i = (int)var_h;
		var_1 = V * ( 1 - S );
		var_2 = V * ( 1 - S * ( var_h - var_i ) );
		var_3 = V * ( 1 - S * ( 1 - ( var_h - var_i ) ) );

		switch ((int)var_i)
		{
			case 0:
				tmp_r = V;
				tmp_g = var_3;
				tmp_b = var_1;
				break;

			case 1:
				tmp_r = var_2;
				tmp_g = V;
				tmp_b = var_1;
				break;

			case 2:
				tmp_r = var_1;
				tmp_g = V;
				tmp_b = var_3;
				break;

			case 3:
				tmp_r = var_1;
				tmp_g = var_2;
				tmp_b = V;
				break;

			case 4:
				tmp_r = var_3;
				tmp_g = var_1;
				tmp_b = V;
				break;

			default:
				tmp_r = V;
				tmp_g = var_1;
				tmp_b = var_2;
				break;
		}

		*R = (int)(tmp_r * 255);
		*G = (int)(tmp_g * 255);
		*B = (int)(tmp_b * 255);

	}
}


#if 0
/*******************************************************************

 Border style in gWidgets that use a Frame to show it
 
 *******************************************************************/
int Frame_getBorder(GtkFrame *fr)
{
	switch (gtk_frame_get_shadow_type(fr))
	{
		case GTK_SHADOW_NONE: return BORDER_NONE;
		case GTK_SHADOW_ETCHED_OUT: return BORDER_PLAIN;
		case GTK_SHADOW_IN: return BORDER_SUNKEN;
		case GTK_SHADOW_OUT: return BORDER_RAISED;
		case GTK_SHADOW_ETCHED_IN: return BORDER_ETCHED;
	}
	
	return BORDER_NONE;
}

void Frame_setBorder(GtkFrame *fr,int vl)
{
	switch(vl)
	{
		case BORDER_NONE:
			gtk_frame_set_shadow_type(fr,GTK_SHADOW_NONE);
			break;
		case BORDER_PLAIN:
			gtk_frame_set_shadow_type(fr,GTK_SHADOW_ETCHED_OUT);
			break;
		case BORDER_SUNKEN:
			gtk_frame_set_shadow_type(fr,GTK_SHADOW_IN);
			break;
		case BORDER_RAISED:
			gtk_frame_set_shadow_type(fr,GTK_SHADOW_OUT);
			break;
		case BORDER_ETCHED:
			gtk_frame_set_shadow_type(fr,GTK_SHADOW_ETCHED_IN);
			break;
		default: return;
	}
}
#endif

/*************************************************************

 Takes a snapshot of a GdkWindow
 
 *************************************************************/
gPicture *Grab_gdkWindow(GdkWindow *win)
{
	GdkPixbuf *buf;
	GdkColormap* cmap;
	gPicture *ret;
	gint x,y;
	
	gdk_window_get_geometry(win,0,0,&x,&y,0);
	if ( (x<1) || (y<1) ) return NULL;
	cmap=gdk_colormap_get_system();
	buf=gdk_pixbuf_get_from_drawable(NULL,win,cmap,0,0,0,0,x,y);
	ret=new gPicture(buf);
	g_object_unref(G_OBJECT(cmap));
	return ret;
}

/*************************************************************

 GtkLabel with accelerators
 
 *************************************************************/

void gMnemonic_correctText(char *st,char **buf)
{
	long bucle,b2;
	long len=strlen(st);
	long len_in=len;
	
	if (!st || !*st)
	{
		*buf = g_strdup("");
		return;
	}
	
	for (bucle=0;bucle<len_in;bucle++)
	{
		if (st[bucle]=='&')
		{
			if (bucle<(len_in-1)) 
				if (st[bucle+1]=='&')
					len--;
				
		}
		else if (st[bucle]=='_') len++;
	}
	
	*buf=(char*)g_malloc(sizeof(char)*(len+1));
	b2=0;
	
	for (bucle=0;bucle<len_in;bucle++)
	{
		if (st[bucle]=='&')
		{
			if (bucle<(len_in-1))
			{
				if (st[bucle+1]=='&')
				{
					(*buf)[b2++]='&';
					bucle++;
				}
				else
				{
					(*buf)[b2++]='_';
				}
			}
			else
			{
				(*buf)[b2]=' ';
				b2++;
			}
		}
		else if (st[bucle]=='_')
		{
			(*buf)[b2++]='_';
			(*buf)[b2++]='_';
		}
		else
		{
			(*buf)[b2++]=st[bucle];
		}	
		(*buf)[b2]=0;
	}
}

guint gMnemonic_correctMarkup(char *st,char **buf)
{
	long bucle,b2;
	long len=strlen(st);
	long len_in=len;
	guint retval=0;	

	if (!st || !*st)
	{
		*buf = g_strdup("");
		return retval;
	}
	
	for (bucle=0;bucle<len_in;bucle++)
	{
		if (st[bucle]=='&')
		{
			if (bucle<(len_in-1)) 
			{
				if (st[bucle+1]=='&') len+-3;
				else                  len+=6;
			}
			else
			{
				len+=4;
			}
				
		}
		else if (st[bucle]=='<') 
		{
			len+=3;
		}
		else if (st[bucle]=='>')
		{
			len+=3;
		}
	}
	
	*buf=(char*)g_malloc(sizeof(char)*(len+1));
	(*buf[0])=0;
	b2=0;
	
	for (bucle=0;bucle<len_in;bucle++)
	{
		if (st[bucle]=='&')
		{
			if (bucle<(len_in-1))
			{
				if (st[bucle+1]=='&')
				{
					(*buf)[b2++]='&';
					(*buf)[b2++]='a';
					(*buf)[b2++]='m';
					(*buf)[b2++]='p';
					(*buf)[b2++]=';';
					bucle++;
				}
				else
				{
					bucle++;
					retval=(guint)st[bucle];
					(*buf)[b2++]='<';
					(*buf)[b2++]='u';
					(*buf)[b2++]='>';
					(*buf)[b2++]=st[bucle];
					(*buf)[b2++]='<';
					(*buf)[b2++]='/';
					(*buf)[b2++]='u';
					(*buf)[b2++]='>';
				}
			}
			else
			{
				(*buf)[b2++]='&';
				(*buf)[b2++]='a';
				(*buf)[b2++]='m';
				(*buf)[b2++]='p';
				(*buf)[b2++]=';';
			}
		}
		else if (st[bucle]=='<')
		{
			(*buf)[b2++]='&';
			(*buf)[b2++]='l';
			(*buf)[b2++]='t';
			(*buf)[b2++]=';';
		}
		else if (st[bucle]=='>')
		{
			(*buf)[b2++]='&';
			(*buf)[b2++]='g';
			(*buf)[b2++]='t';
			(*buf)[b2++]=';';
		}
		else
		{
			(*buf)[b2++]=st[bucle];
		}	
		(*buf)[b2]=0;
	}

	return retval;
}

void gMnemonic_returnText(char *st,char **buf)
{
	long bucle,b2;
	long len=strlen(st);
	long len_in=len;
	
	if (!st || !*st)
	{
		*buf = g_strdup("");
		return;
	}
		
	for (bucle=0;bucle<len_in;bucle++)
	{
		if (st[bucle]=='_')
		{
			if (bucle<(len_in-1)) 
				if (st[bucle+1]=='_')
					len--;
				
		}
		else if (st[bucle]=='&') len++;
	}
	
	*buf=(char*)g_malloc(sizeof(char)*(len+1));
	b2=0;
	
	for (bucle=0;bucle<len_in;bucle++)
	{
		if (st[bucle]=='_')
		{
			if (bucle<(len_in-1))
			{
				if (st[bucle+1]=='_')
				{
					(*buf)[b2++]='&';
					bucle++;
				}
				else
				{
					(*buf)[b2++]='_';
				}
			}
			else
			{
				(*buf)[b2]=' ';
				b2++;
			}
		}
		else if (st[bucle]=='&')
		{
			(*buf)[b2++]='&';
			(*buf)[b2++]='&';
		}
		else
		{
			(*buf)[b2++]=st[bucle];
		}	
		(*buf)[b2]=0;
	}
}

void g_stradd(gchar **res, const gchar *s)
{
  if (!*res)
    *res = g_strdup(s);
  else
  {
    gchar *old = *res;
    *res = g_strconcat(*res, s, NULL);
    g_free(old);
  }
}

GdkPixbuf *gt_pixbuf_create_disabled(GdkPixbuf *img)
{
  gint w, h;
  guchar *r, *g, *b, *end;
  GdkPixbuf *dimg;

  dimg = gdk_pixbuf_copy(img);
  w = gdk_pixbuf_get_width(dimg);
  h = gdk_pixbuf_get_height(dimg);
  r = gdk_pixbuf_get_pixels(dimg);
  g = r + 1;
  b = r + 2;
  end = r + w * h * gdk_pixbuf_get_n_channels(img);
  
	while (r != end) 
	{
    *r = *g = *b = 0x80 | (((*r + *b) >> 1) + *g) >> 2; // (r + b + g) / 3

    r += 4;
    g += 4;
    b += 4;
	}
	
	return dimg;
}

void gt_shortcut_parse(char *shortcut, guint *key, GdkModifierType *mods)
{
	gchar **cads;
	gchar *res;
	int bucle;
	
	res = NULL;
	
	if (!shortcut || !*shortcut)
	{
		*key = 0;
		return;
	}
	
	cads = g_strsplit(shortcut, "+", 0);
	
	bucle = 0;
	while (cads[bucle])
	{
		g_strstrip(cads[bucle]);
		bucle++;
	}
	
  bucle = 0;
	while (cads[bucle])
	{
		if (!strcasecmp(cads[bucle],"ctrl"))
		  g_stradd(&res, "<Ctrl>");
		else if (!strcasecmp(cads[bucle],"shift"))
		  g_stradd(&res, "<Shift>");
		else if (!strcasecmp(cads[bucle],"alt"))
		  g_stradd(&res, "<Alt>");
		else
		  g_stradd(&res, cads[bucle]);
		
		bucle++;
	}
	
	g_strfreev(cads);
	gtk_accelerator_parse(res, key, mods);
	
	if (res)
	 g_free(res);
}

#define MAX_FREE_LATER 32

static char *_free_later_ptr[MAX_FREE_LATER] = { 0 };
static int _free_later_index = 0;

char *gt_free_later(char *ptr)
{
	if (_free_later_ptr[_free_later_index])
		g_free(_free_later_ptr[_free_later_index]);
	
	_free_later_ptr[_free_later_index] = ptr;
	
	_free_later_index++;
	
	if (_free_later_index >= MAX_FREE_LATER)
		_free_later_index = 0;
		
	return ptr;
}

static void free_all_later()
{
	int i;
	char *p;
	
	for (i = 0; i < MAX_FREE_LATER; i++)
	{
		p = _free_later_ptr[i];
		if (p)
		{
			g_free(p);
			_free_later_ptr[i] = NULL;
		}
	}
}

void gt_exit()
{
	free_all_later();
}

static char *html_entity(char c)
{
	static char same[2];
	
	if (c == '<')
		return "&lt;";
	else if (c == '>')
		return "&gt;";
	else if (c == '&')
		return "&amp;";
	
	same[0] = c;
	same[1] = 0;
	return same;
}

char* gt_html_to_pango_string(char *html, bool newline)
{
	static char *title[] =
	{
		"h1", "size=\"xx-large\"",
		"h2", "size=\"x-large\"",
		"h3", "size=\"large\"",
		"h4", "size=\"medium\"",
		"h5", "size=\"small\"",
		"h6", "size=\"x-small\"",
		NULL, NULL
	};
	
	static char *accept[] = { "b", "big", "i", "s", "sub", "sup", "small", "tt", "u", NULL };
	
	static char *size[] = 
	{
		"\"1\"", "xx-large",
		"\"2\"", "x-large",
		"\"3\"", "large",
		"\"4\"", "medium",
		"\"5\"", "small",
		"\"6\"", "x-small",
		"\"7\"", "xx-small",
		"\"+1\"", "larger",
		"\"-1\"", "smaller",
		NULL, NULL
	};
	
	GString *pango = g_string_new("");
	char *p, *p_end, *p_markup;
	char c;
	char *token, *markup, **attr;
	gsize len;
	bool end_token = false;
	char **pt, **ps;
	
	p_end = &html[strlen(html)];
	p_markup = NULL;
	
	for (p = html;; p++)
	{
		c = *p;
		if (!c)
			break;
			
		if (c == '<')
		{
			if (p[1] == '/')
			{
				p_markup = &p[2];
				end_token = true;
			}
			else
			{
				p_markup = &p[1];
				end_token = false;
			}
			continue;
		}
		
		if (c == '>')
		{
			if (!p_markup)
			{
				g_string_append(pango, "&gt;");
				continue;
			}
			len = p - p_markup;
			if (len <= 0)
			{
				if (end_token) g_string_append(pango, "/");
				g_string_append(pango, "&lt;&gt;");
				p_markup = NULL;
				continue;
			}
			
			markup = g_strndup(p_markup, len);
			attr = g_strsplit(markup, " ", -1);
			token = attr[0];
			
			for (pt = title; *pt; pt += 2)
			{
				if (!strcasecmp(*pt, token))
				{
					if (end_token)
						g_string_append_printf(pango, "</span>\n\n");
					else
						g_string_append_printf(pango, "<span %s>", pt[1]);
					goto __FOUND_TOKEN;
				}
			}
			
			for (pt = accept; *pt; pt++)
			{
				if (!strcasecmp(*pt, token))
				{
					g_string_append_printf(pango, "<%s%s>", end_token ? "/" : "", *pt);
					goto __FOUND_TOKEN;
				}
			}
			
			if (!strcasecmp(token, "br"))
			{
				if (!end_token)
					g_string_append(pango, "\n");
				goto __FOUND_TOKEN;
			}
			
			if (!strcasecmp(token, "p"))
			{
				if (!end_token)
					g_string_append(pango, "\n\n");
				goto __FOUND_TOKEN;
			}
			
			if (!strcasecmp(token, "font"))
			{
				if (end_token)
					g_string_append(pango, "</span>");
				else
				{
					g_string_append(pango, "<span");
					for (pt = attr; *pt; pt++)
					{
						if (!strncasecmp(*pt, "color=", 6))
						{
							g_string_append(pango, " foreground=");
							g_string_append(pango, *pt + 6);
						}
						else if (!strncasecmp(*pt, "face=", 5))
						{
							g_string_append(pango, " face=");
							g_string_append(pango, *pt + 5);
						}
						else if (!strncasecmp(*pt, "size=", 5))
						{
							g_string_append(pango, " size=");
							for (ps = size; *ps; ps += 2)
							{
								if (!strcasecmp(*ps, *pt + 5))
								{
									g_string_append_printf(pango, "\"%s\"", ps[1]);
									break;
								}
							}
						}
					}
					g_string_append(pango, ">");
				}
				goto __FOUND_TOKEN;
			}
			
			if (!strcasecmp(token, "a") || !strncasecmp(token, "a href", 6))
			{
				if (!end_token)
					g_string_append(pango, "<span foreground=\"blue\"><u>");
				else
					g_string_append(pango, "</u></span>");
				goto __FOUND_TOKEN;
			}
			
			g_string_append(pango, "&lt;");
			if (end_token) g_string_append(pango, "/");
			while (p_markup < p)
			{
				g_string_append(pango, html_entity(*p_markup));
				p_markup++;
			}
			g_string_append(pango, "&gt;");
			
		__FOUND_TOKEN:
		
			g_free(token);
			p_markup = NULL;
			continue;	
		}
		
		if (c == '\n' && !newline)
		{
			if (p[1] != ' ')
				g_string_append(pango, " ");
			continue;
		}
		
		if (c == '&')
		{
			if ((p_end - p) >= 6 && !strncasecmp(p, "&nbsp;", 6))
			{
				g_string_append(pango, " ");
				p += 5;
				continue;
			}
		}
	
		if (c == ' ' && p[1] == ' ')
			continue;
			
		if (!p_markup)
			g_string_append(pango, html_entity(*p));
	}
	
	p = g_string_free(pango, false);
	//fprintf(stderr, "pango: %s\n", p);
	//gt_free_later(p);
	return p;
}

int gt_to_alignment(double halign, double valign)
{
	int align = 0;
	
	if (halign == 0.0)
		align += 0x1;
	else if (halign == 0.5)
		align += 0x2;
	else if (halign == 1.0)
		align += 0x3;
		
	if (valign == 0.0)
		align += 0x10;
	else if (valign == 1.0)
		align += 0x20;
		
	return align;
}

double gt_from_alignment(int align, bool vertical)
{
	if (vertical)
	{
		switch(align & 0xF0)
		{
			case 1: return 0.0;
			case 2: return 1.0;
			default: return 0.5;
		}
	}
	else
	{
		switch (align & 0x0F)
		{
			case 1: return 0.0;
			case 2: return 1.0;
			case 3: return 0.5;
			default: return gDesktop::rightToLeft() ? 1.0 : 0.0;
		}
	}
}


void gt_ensure_visible(GtEnsureVisible *arg, int x, int y, int w, int h)
{
	w = (w + 1) / 2;
	h = (h + 1) / 2;
	x += w;
	y += h;

	int pw = arg->clientWidth;
	int ph = arg->clientHeight;

	int cx = -arg->scrollX;
	int cy = -arg->scrollY;
	int cw = arg->scrollWidth;
	int ch = arg->scrollHeight;

	if ( pw < w*2 )
			w=pw/2;
	if ( ph < h*2 )
			h=ph/2;

	if ( cw <= pw ) {
			w=0;
			cx=0;
	}
	if ( ch <= ph ) {
			h=0;
			cy=0;
	}

	if ( x < -cx+w )
			cx = -x+w;
	else if ( x >= -cx+pw-w )
			cx = -x+pw-w;

	if ( y < -cy+h )
			cy = -y+h;
	else if ( y >= -cy+ph-h )
			cy = -y+ph-h;

	if ( cx > 0 )
			cx=0;
	else if ( cx < pw-cw && cw>pw )
			cx=pw-cw;

	if ( cy > 0 )
			cy=0;
	else if ( cy < ph-ch && ch>ph )
			cy=ph-ch;

	arg->scrollX = -cx;
	arg->scrollY = -cy;
}
