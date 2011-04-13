/***************************************************************************

  gtools.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
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

// HTML character entities
#include "kentities.h"

void stub(const char *function)
{
	printf("gb.gtk: warning: %s not yet implemented\n", function);
}

/*******************************************************************

Conversion between GDK and long type colors

********************************************************************/

#define SCALE(i) ((int)(i * 255.0 / 65535.0 + 0.5))
#define UNSCALE(d) ((int)(d / 255.0 * 65535.0 + 0.5))

static GtkStateType _color_style_bg[] = { GTK_STATE_INSENSITIVE, GTK_STATE_NORMAL };
static GtkStateType _color_style_fg[] = { GTK_STATE_ACTIVE, GTK_STATE_PRELIGHT, GTK_STATE_NORMAL };

void fill_gdk_color(GdkColor *gcol,gColor color, GdkColormap *cmap)
{
	int r, g, b;
	
	if (!cmap)
		cmap = gdk_colormap_get_system();
	
	gt_color_to_rgb(color, &r, &g, &b);
	
	gcol->red = UNSCALE(r);
	gcol->green = UNSCALE(g);
	gcol->blue = UNSCALE(b);
	
	gdk_colormap_alloc_color(cmap, gcol, TRUE, TRUE);
}

gColor get_gdk_color(GdkColor *gcol)
{
	return gt_rgb_to_color(SCALE(gcol->red), SCALE(gcol->green), SCALE(gcol->blue));
}

static void set_color(GtkWidget *wid, gColor color, void (*func)(GtkWidget *, GtkStateType, const GdkColor *), bool fg)
{
	GdkColor gcol;
	GdkColor *pcol;
	int i;
	GtkStateType st;

	if (color == COLOR_DEFAULT)
	{
		pcol = NULL;
	}
	else
	{
		fill_gdk_color(&gcol, color);
		pcol = &gcol;
	}
	
	for (i = 0;; i++)
	{
		st = fg ? _color_style_fg[i] : _color_style_bg[i];
		(*func)(wid, st, pcol);
		if (st == GTK_STATE_NORMAL)
			break;
	}
}

gColor get_gdk_fg_color(GtkWidget *wid)
{
	GtkStyle* st;

	st=gtk_widget_get_style(wid);	
	return get_gdk_color(&st->fg[GTK_STATE_NORMAL]);
}

void set_gdk_fg_color(GtkWidget *wid, gColor color)
{
	set_color(wid, color, gtk_widget_modify_fg, true);
}

gColor get_gdk_bg_color(GtkWidget *wid)
{
	GtkStyle* st;

	st=gtk_widget_get_style(wid);	
	return get_gdk_color(&st->bg[GTK_STATE_NORMAL]);
}

void set_gdk_bg_color(GtkWidget *wid,gColor color)
{
	set_color(wid, color, gtk_widget_modify_bg, false);
}

gColor get_gdk_text_color(GtkWidget *wid)
{
	GtkStyle* st;

	st=gtk_widget_get_style(wid);	
	return get_gdk_color(&st->text[GTK_STATE_NORMAL]);
}

void set_gdk_text_color(GtkWidget *wid,gColor color)
{
	set_color(wid, color, gtk_widget_modify_text, true);
}

gColor get_gdk_base_color(GtkWidget *wid)
{
	GtkStyle* st;

	st=gtk_widget_get_style(wid);
	return get_gdk_color(&st->base[GTK_STATE_NORMAL]);
}

void set_gdk_base_color(GtkWidget *wid,gColor color)
{
	set_color(wid, color, gtk_widget_modify_base, false);
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

	if (h < 0)
		h = 360 - ((-h) % 360);
	else
		h = h % 360;

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

gPicture *gt_grab_window(GdkWindow *win, int x, int y, int w, int h)
{
	int ow, oh;
	int ww, hh;
	int dx, dy;
	GdkPixbuf *buf = NULL;
	gPicture *pic;
	
	ow = w;
	oh = h;
	
	dx = dy = 0;
	
	gdk_window_get_geometry(win, 0, 0, &ww, &hh, 0);
	
	if (w <= 0 || h <= 0)
	{
		w = ww;
		h = hh;
	}
	
	if (x < 0)
	{
		w += x;
		dx = -x;
		x = 0;
	}
	
	if (y < 0)
	{
		h += y;
		dy = -y;
		y = 0;
	}
	
	if ((x + w) > ww)
		w = ww - x;
	
	if ((y + h) > hh)
		h = hh - y;
	
	if (w > 0 && h > 0)
		buf = gdk_pixbuf_get_from_drawable(NULL, win, NULL, x, y, 0, 0, w, h);
	
	if (w == ow && h == oh)
		return new gPicture(buf);
		
	pic = new gPicture(gPicture::MEMORY, ow, oh, false);
	pic->fill(0);
	
	if (w <= 0 || h <= 0)
		return pic;
	
	gdk_pixbuf_copy_area(buf, 0, 0, w, h, pic->getPixbuf(), dx, dy);
	return pic;
}

/*************************************************************

 GtkLabel with accelerators
 
 *************************************************************/

void gMnemonic_correctText(char *st,char **buf)
{
	int bucle,b2;
	
	if (!st || !*st)
	{
		*buf = g_strdup("");
		return;
	}
	
	int len = strlen(st);
	int len_in = len;
	
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
	int bucle,b2;
	guint retval=0;	

	if (!st || !*st)
	{
		*buf = g_strdup("");
		return retval;
	}
	
	int len = strlen(st);
	int len_in = len;
	
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
	int bucle,b2;
	
	if (!st || !*st)
	{
		*buf = g_strdup("");
		return;
	}
		
	int len = strlen(st);
	int len_in = len;
	
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
    *res = g_strconcat(*res, s, (void *)NULL);
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

static const char *html_entity(char c)
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
	return (const char *)same;
}

static void add_space(GString *str)
{
	char c;
	
	if (str->len == 0)
		return;
		
	c = str->str[str->len - 1];
	if (c != ' ' && c != '\n')
		g_string_append_c(str, ' ');
}

char *gt_html_to_pango_string(const char *html, int len_html, bool newline)
{
	static const char *title[] =
	{
		"h1", "size=\"xx-large\"",
		"h2", "size=\"x-large\"",
		"h3", "size=\"large\"",
		"h4", "size=\"medium\"",
		"h5", "size=\"small\"",
		"h6", "size=\"x-small\"",
		NULL, NULL
	};
	
	static const char *accept[] = { "b", "big", "i", "s", "sub", "sup", "small", "tt", "u", NULL };
	
	static const char *size_name[] = { "xx-small", "x-small", "small", "medium", "large", "x-large", "xx-large" };
	static const int size_stack_len = 16;
	
	GString *pango = g_string_new("");
	const char *p, *p_end, *p_markup;
	char c;
	char *token, *markup, **attr;
	const char *pp;
	gsize len;
	bool end_token = false;
	const char **pt;
	int size = 3; // medium
	char size_stack[size_stack_len];
	int size_stack_ptr = 0;
	
	p_end = &html[len_html < 0 ? strlen(html) : len_html];
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
						g_string_append_printf(pango, "</b></span>\n\n");
					else
						g_string_append_printf(pango, "<span %s><b>", pt[1]);
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
			
			if (!strcasecmp(token, "br") || !strcasecmp(token, "hr"))
			{
				if (!end_token)
					g_string_append(pango, "\n");
				goto __FOUND_TOKEN;
			}
			
			if (!strcasecmp(token, "p"))
			{
				if (!end_token)
				{
					if (pango->len > 0)
					{
						if (pango->str[pango->len - 1] != '\n')
							g_string_append(pango, "\n");
						g_string_append(pango, "\n");
					}
				}
				goto __FOUND_TOKEN;
			}
			
			if (!strcasecmp(token, "font"))
			{
				if (end_token)
				{
					if (size_stack_ptr > 0)
					{
						size_stack_ptr--;
						if (size_stack_ptr < size_stack_len)
							size = size_stack[size_stack_ptr];
					}
					g_string_append(pango, "</span>");
				}
				else
				{
					if (size_stack_ptr < size_stack_len)
						size_stack[size_stack_ptr] = size;
					size_stack_ptr++;

					g_string_append(pango, "<span");
					for (pt = (const char **)attr; *pt; pt++)
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
							pp = *pt + 6;
							if (*pp == '"')
								pp++;
							if (isdigit(*pp))
							{
								size = *pp - '1';
								pp++;
							}
							else if (*pp == '+' && isdigit(pp[1]))
							{
								size += pp[1] - '0';
								pp += 2;
							}
							else if (*pp == '-' && isdigit(pp[1]))
							{
								size -= pp[1] - '0';
								pp += 2;
							}
							
							if (size < 0)
								size = 0;
							else if (size > 6)
								size = 6;
							
							g_string_append(pango, "\"");
							g_string_append(pango, size_name[size]);
							g_string_append(pango, "\"");
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
			add_space(pango);
			continue;
		}
		
		if (c == '&')
		{
			const char *entity_start = ++p;
			int entity_len;
			int entity_unicode;
			
// 			if ((p_end - p) >= 6 && !strncasecmp(p, "&nbsp;", 6))
// 			{
// 				g_string_append(pango, " ");
// 				p += 5;
// 				continue;
// 			}
			while (p < p_end && *p != ';' && ((uchar)*p) > ' ')
				p++;
				
			entity_len = p - entity_start;
			if (*p != ';')
				entity_len = 0;
				
			if (entity_len > 1)
			{
				if ((entity_len == 3 && !strncasecmp(entity_start, "amp", 3))
					  || (entity_len == 2 && (!strncasecmp(entity_start, "lt", 2) || !strncasecmp(entity_start, "gt", 2))))
				{
					g_string_append_len(pango, entity_start - 1, entity_len + 2);
					continue;
				}
				else
				{
					const struct entity *e = kde_findEntity(entity_start, entity_len);
					if (e)
					{
						entity_unicode = e->code;
						g_string_append_unichar(pango, entity_unicode);
						continue;
					}
				}
			}
			
			g_string_append(pango, "&amp;");
			p = entity_start - 1;
			continue;
		}
	
		if (c == ' ')
		{
			add_space(pango);
			continue;
		}
			
		if (!p_markup)
			g_string_append(pango, html_entity(*p));
	}
	
	// Sometimes the first markup is not taken into account.
	// This is a workaround for this bug:
	g_string_prepend_unichar(pango, 0xFEFF);
	
	p = g_string_free(pango, false);
	//fprintf(stderr, "pango: %s\n", p);
	return (char *)p;
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
			case 0x10: return 0.0;
			case 0x20: return 1.0;
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

GtkStyle *gt_get_style(const char *name, int type)
{
	GtkSettings *set = gtk_settings_get_default();
	GtkStyle* st = gtk_rc_get_style_by_paths(set, NULL, name, type);
	if (!st) st = gtk_widget_get_default_style();
	return st;
}


void gt_drawable_fill(GdkDrawable *d, gColor col, GdkGC *gc)
{
	GdkColor color;
	gint w, h;
	bool free_gc = false;
	
	fill_gdk_color(&color, col);
	w = gdk_window_get_width(d);
	h = gdk_window_get_height(d);
	
	if (!gc)
	{
		gc = gdk_gc_new(d);
		free_gc = true;
	}
	
	gdk_gc_set_foreground(gc, &color);
	gdk_gc_set_background(gc, &color);
	gdk_draw_rectangle(d, gc, true, 0, 0, w, h);
	
	if (free_gc)
		g_object_unref(gc);
}

void gt_pixbuf_render_pixmap_and_mask(GdkPixbuf *pixbuf, GdkPixmap **pixmap_return, GdkBitmap **mask_return, int alpha_threshold)
{
	GdkColormap *colormap = gdk_rgb_get_colormap();
  GdkScreen *screen;

  screen = gdk_colormap_get_screen (colormap);
  
  if (pixmap_return)
	{
		GdkGC *gc;
		*pixmap_return = gdk_pixmap_new (gdk_screen_get_root_window (screen),
							gdk_pixbuf_get_width (pixbuf), gdk_pixbuf_get_height (pixbuf),
							gdk_colormap_get_visual (colormap)->depth);
		
		gdk_drawable_set_colormap (GDK_DRAWABLE (*pixmap_return), colormap);
		
		gc = gdk_gc_new(*pixmap_return);
		
		gt_drawable_fill(*pixmap_return, 0, gc);
		
		gdk_draw_pixbuf (*pixmap_return, gc, pixbuf, 
					0, 0, 0, 0,
					gdk_pixbuf_get_width (pixbuf), gdk_pixbuf_get_height (pixbuf),
					GDK_RGB_DITHER_NORMAL,
					0, 0);
		
		g_object_unref(gc);
	}
  
  if (mask_return)
	{
		if (gdk_pixbuf_get_has_alpha (pixbuf))
		{
			*mask_return = gdk_pixmap_new (gdk_screen_get_root_window (screen),
							gdk_pixbuf_get_width (pixbuf), gdk_pixbuf_get_height (pixbuf), 1);
		
			gdk_pixbuf_render_threshold_alpha (pixbuf, *mask_return,
									0, 0, 0, 0,
									gdk_pixbuf_get_width (pixbuf), gdk_pixbuf_get_height (pixbuf),
									alpha_threshold);
		}
		else
			*mask_return = NULL;
	}
}

#if 0
void gt_pixbuf_replace_color(GdkPixbuf *pixbuf, gColor src, gColor dst, bool noteq)
{
	guint32 *p;
	int i, n;
	
	p = (guint32 *)gdk_pixbuf_get_pixels(pixbuf);
	n = gdk_pixbuf_get_width(pixbuf) * gdk_pixbuf_get_height(pixbuf);

	src ^= 0xFF000000;
	dst ^= 0xFF000000;
	
	if (noteq)
	{
		for (i = 0; i < n; i++, p ++)
		{
			if (*p != src)
				*p = dst;
		}
	}
	else
	{
		for (i = 0; i < n; i++, p ++)
		{
			if (*p == src)
				*p = dst;
		}
	}
}
#endif

// Comes from the GIMP

typedef
	struct {
		float r;
		float b;
		float g;
		float a;
		}
	RGB;

static void color_to_alpha(RGB *src, const RGB *color)
{
  RGB alpha;

  alpha.a = src->a;

  if (color->r < 0.0001)
    alpha.r = src->r;
  else if (src->r > color->r)
    alpha.r = (src->r - color->r) / (1.0 - color->r);
  else if (src->r < color->r)
    alpha.r = (color->r - src->r) / color->r;
  else alpha.r = 0.0;

  if (color->g < 0.0001)
    alpha.g = src->g;
  else if (src->g > color->g)
    alpha.g = (src->g - color->g) / (1.0 - color->g);
  else if (src->g < color->g)
    alpha.g = (color->g - src->g) / (color->g);
  else alpha.g = 0.0;

  if (color->b < 0.0001)
    alpha.b = src->b;
  else if (src->b > color->b)
    alpha.b = (src->b - color->b) / (1.0 - color->b);
  else if (src->b < color->b)
    alpha.b = (color->b - src->b) / (color->b);
  else alpha.b = 0.0;

  if (alpha.r > alpha.g)
    {
      if (alpha.r > alpha.b)
        {
          src->a = alpha.r;
        }
      else
        {
          src->a = alpha.b;
        }
    }
  else if (alpha.g > alpha.b)
    {
      src->a = alpha.g;
    }
  else
    {
      src->a = alpha.b;
    }

  if (src->a < 0.0001)
    return;

  src->r = (src->r - color->r) / src->a + color->r;
  src->g = (src->g - color->g) / src->a + color->g;
  src->b = (src->b - color->b) / src->a + color->b;

  src->a *= alpha.a;
}

void gt_pixbuf_make_alpha(GdkPixbuf *pixbuf, gColor color)
{
	guchar *p;
	int i, n;
	RGB rgb_color, rgb_src;
	int r, g, b;
	
	p = gdk_pixbuf_get_pixels(pixbuf);
	n = gdk_pixbuf_get_width(pixbuf) * gdk_pixbuf_get_height(pixbuf);

	gt_color_to_rgb(color, &r, &g, &b);

	rgb_color.b = b / 255.0;
	rgb_color.g = g / 255.0;
	rgb_color.r = r / 255.0;
	rgb_color.a = 1.0;

	for (i = 0; i < n; i++, p += 4)
	{
		rgb_src.r = p[0] / 255.0;
		rgb_src.g = p[1] / 255.0;
		rgb_src.b = p[2] / 255.0;
		rgb_src.a = p[3] / 255.0;
		
		color_to_alpha(&rgb_src, &rgb_color);
		
		p[0] = rgb_src.r * 255.0 + 0.5;
		p[1] = rgb_src.g * 255.0 + 0.5;
		p[2] = rgb_src.b * 255.0 + 0.5;
		//p[3] = 0xFF ^ (uchar)(rgb_src.a * 255.0 + 0.5);
		p[3] = rgb_src.a * 255.0 + 0.5;
	}
}

void gt_pixbuf_make_gray(GdkPixbuf *pixbuf)
{
	guchar *p;
	int i, n;
	
	p = gdk_pixbuf_get_pixels(pixbuf);
	n = gdk_pixbuf_get_width(pixbuf) * gdk_pixbuf_get_height(pixbuf);

	for (i = 0; i < n; i++, p += 4)
		p[0] = p[1] = p[2] = (p[0] * 11 + p[1] * 16 + p[2] * 5) / 32;
}

static void gt_pixbuf_make_alpha_from_white(GdkPixbuf *pixbuf)
{
	guchar *p;
	int i, n;
	
	p = gdk_pixbuf_get_pixels(pixbuf);
	n = gdk_pixbuf_get_width(pixbuf) * gdk_pixbuf_get_height(pixbuf);

	for (i = 0; i < n; i++, p += 4)
	{
		p[3] = (p[0] * 11 + p[1] * 16 + p[2] * 5) / 32;
	}
}



GdkBitmap *gt_make_text_mask(GdkDrawable *dr, int w, int h, PangoLayout *ly, int x, int y)
{
	GdkBitmap *mask;
	GdkColor color;
	GdkPixmap *tmp_pixmap;
	GdkPixbuf *tmp_pixbuf;
	GdkGC *tmp_gc;
	
	tmp_pixmap = gdk_pixmap_new(dr, w, h, -1);
	
	tmp_gc = gdk_gc_new(tmp_pixmap);
	
	gt_drawable_fill(tmp_pixmap, 0, tmp_gc);
	
	fill_gdk_color(&color, 0, gdk_drawable_get_colormap(dr));
	gdk_gc_set_background(tmp_gc, &color);
	
	fill_gdk_color(&color, 0xFFFFFF, gdk_drawable_get_colormap(dr));
	gdk_gc_set_foreground(tmp_gc, &color);
	
	gdk_draw_layout(tmp_pixmap, tmp_gc, x, y, ly);
	
	g_object_unref(tmp_gc);
	
	tmp_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, w, h);
	gdk_pixbuf_get_from_drawable(tmp_pixbuf, tmp_pixmap, NULL, 0, 0, 0, 0, w, h);
	g_object_unref(tmp_pixmap);
	
	gt_pixbuf_make_alpha_from_white(tmp_pixbuf);
	gt_pixbuf_render_pixmap_and_mask(tmp_pixbuf, NULL, &mask, 64);
	g_object_unref(tmp_pixbuf);
	
	return mask;
}

static void disabled_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer data)
{
	// Print only debugging messages
	if (log_level & G_LOG_LEVEL_DEBUG)
		g_log_default_handler(log_domain, log_level, message, data);
}

static GLogFunc old_handler = NULL;

void gt_disable_warnings(bool disable)
{
	fprintf(stderr, "disable warnings\n");
	if (disable)
		old_handler = g_log_set_default_handler(disabled_handler, NULL);		
	else
		g_log_set_default_handler(old_handler, NULL);
	fprintf(stderr, "enable warnings\n");
}

void gt_set_cell_renderer_text_from_font(GtkCellRendererText *cell, gFont *font)
{
	g_object_set(G_OBJECT(cell),
		"font-desc", font->desc(),
		"underline", font->underline() ? PANGO_UNDERLINE_SINGLE : PANGO_UNDERLINE_NONE,
		"strikethrough", font->strikeout(),
		(void *)NULL);	
}

static void set_layout_from_font(PangoLayout *layout, gFont *font, bool add, int dpi)
{
	PangoFontDescription *desc;
	PangoAttrList *attrs;
	PangoAttribute *attr;
	bool copy = false;
	
	desc = pango_context_get_font_description(font->ct);

	//desc = pango_font_description_copy(pango_layout_get_font_description(layout));
	if (dpi && dpi != gDesktop::resolution())
	{
		int size = pango_font_description_get_size(desc);
		desc = pango_font_description_copy(desc);
		copy = true;
		pango_font_description_set_size(desc, size * dpi / gDesktop::resolution());
	}

	pango_layout_set_font_description(layout, desc);
	
	if (add)
	{
		attrs = pango_layout_get_attributes(layout);
		if (!attrs)
		{
			attrs = pango_attr_list_new();
			add = false;
		}
	}
	else
		attrs = pango_attr_list_new();
	
	if (font->underline())
	{
		attr = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
		pango_attr_list_insert(attrs, attr);
	}
	
	if (font->strikeout())
	{
		attr = pango_attr_strikethrough_new(true);
		pango_attr_list_insert(attrs, attr);
	}
	
	pango_layout_set_attributes(layout, attrs);
	
	if (!add)
		pango_attr_list_unref(attrs);
	
	pango_layout_context_changed(layout);
	
	if (copy)
		pango_font_description_free(desc);
}

void gt_set_layout_from_font(PangoLayout *layout, gFont *font, int dpi)
{
	set_layout_from_font(layout, font, false, dpi);
}

void gt_add_layout_from_font(PangoLayout *layout, gFont *font, int dpi)
{
	set_layout_from_font(layout, font, true, dpi);
}

void gt_layout_alignment(PangoLayout *layout, float w, float h, float *tw, float *th, int align, float *offX, float *offY)
{
	int ptw, pth;
	pango_layout_get_size(layout, &ptw, &pth);
	*tw = (float)ptw / PANGO_SCALE;
	*th = (float)pth / PANGO_SCALE;
	
	if (w < 0) w = *tw;
	if (h < 0) h = *th;

	switch (align)
	{
		case ALIGN_BOTTOM_NORMAL:
			align = gDesktop::rightToLeft() ? ALIGN_BOTTOM_RIGHT : ALIGN_BOTTOM_LEFT;
			break;
		
		case ALIGN_NORMAL:
			align = gDesktop::rightToLeft() ? ALIGN_RIGHT : ALIGN_LEFT;
			break;
			
		case ALIGN_TOP_NORMAL:
			align = gDesktop::rightToLeft() ? ALIGN_TOP_RIGHT : ALIGN_TOP_LEFT;
			break;
	}
	
	switch (align)
	{
		case ALIGN_BOTTOM: 	 
			pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);	
			*offX = (w - *tw) / 2;
			*offY = h - *th; 
			break;
		
		case ALIGN_BOTTOM_LEFT: 		
			pango_layout_set_alignment(layout, PANGO_ALIGN_LEFT);
			*offX = 0;
			*offY = h - *th;
			break;
			
		case ALIGN_BOTTOM_RIGHT: 
			pango_layout_set_alignment(layout, PANGO_ALIGN_RIGHT);
			*offX = w - *tw;
			*offY = h - *th;
			break;
			
		case ALIGN_CENTER: 
			pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);
			*offX = (w - *tw) / 2;
			*offY = (h - *th) / 2;
			break;
			
		case ALIGN_LEFT:
			pango_layout_set_alignment(layout, PANGO_ALIGN_LEFT);
			*offX = 0;
			*offY = (h - *th) / 2;
			break;
			
		case ALIGN_RIGHT:
			pango_layout_set_alignment(layout, PANGO_ALIGN_RIGHT);
			*offX = w - *tw;
			*offY = (h - *th) / 2;
			break;
			
		case ALIGN_TOP:
			pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);
			*offX = (w - *tw) / 2;
			*offY = 0;
			break;
			
		case ALIGN_TOP_LEFT:
			pango_layout_set_alignment(layout, PANGO_ALIGN_LEFT);
			*offX = 0;
			*offY = 0;
			break;
			
		case ALIGN_TOP_RIGHT:
			pango_layout_set_alignment(layout, PANGO_ALIGN_RIGHT);
			*offX = w - *tw;
			*offY = 0; 
			break;
	}
}

#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 18
#else
void
gtk_widget_set_can_focus (GtkWidget *widget,
                          gboolean   can_focus)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (can_focus != GTK_WIDGET_CAN_FOCUS (widget))
    {
      if (can_focus)
        GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_FOCUS);
      else
        GTK_WIDGET_UNSET_FLAGS (widget, GTK_CAN_FOCUS);

      gtk_widget_queue_resize (widget);
      g_object_notify (G_OBJECT (widget), "can-focus");
    }
}
#endif
