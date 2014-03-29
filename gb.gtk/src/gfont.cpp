/***************************************************************************

  gfont.cpp

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#include "widgets.h"
#include "font-parser.h"
#include "gdesktop.h"
#include "gtools.h"
#include "gb.form.font.h"

#include <math.h>

static void set_font_from_string(gFont *font, const char *str)
{
  gchar **tokens, **p;
  gchar *copy, *elt;
  int grade;
  double size;
  
  /*font->setBold(false);
  font->setItalic(false);
  font->setUnderline(false);
  font->setStrikeOut(false);
  font->setName(gDesktop::font()->name());
  font->setSize(gDesktop::font()->size());*/
  
  if (!str || !*str)
  	return;
  	
	tokens = g_strsplit(str, ",", 0);
  
  p = tokens;
  for(p = tokens; *p; p++)
  {
		copy = g_strdup(*p);
  	elt = g_strstrip(copy);
  	
  	if (!strcasecmp(elt, "bold"))
  		font->setBold(true);
		else if (!strcasecmp(elt, "italic"))
  		font->setItalic(true);
		else if (!strcasecmp(elt, "underline"))
  		font->setUnderline(true);
		else if (!strcasecmp(elt, "strikeout"))
  		font->setStrikeout(true);
		else if (elt[0] == '+' || elt[0] == '-' || elt[0] == '0')
		{
			grade = atoi(elt);
			if (grade || elt[0] == '0')
				font->setGrade(grade);
		}
		else
		{
			size = atof(elt);
			if (isdigit(*elt) && size != 0.0)
				font->setSize(size);
			else
			{
				font->setBold(false);
				font->setItalic(false);
				font->setUnderline(false);
				font->setStrikeout(false);
				font->setName(elt);
			}
		}

		g_free(copy);
  }
  
  g_strfreev(tokens);
}


/********************************************************************************

gFont

*********************************************************************************/
static int FONT_n_families;
GList *FONT_families = NULL;

static int _nfont = 0;

void gFont::init()
{
  PangoFontFamily **_families;
  PangoContext *ct;
  char *buf1,*buf2; 
  int bucle;
 
  ct=gdk_pango_context_get();
  pango_context_list_families(ct, &_families, &FONT_n_families);
  
  for (bucle=0;bucle<FONT_n_families;bucle++)
  {
  	buf1=(char*)pango_font_family_get_name (_families[bucle]);
  	if (buf1){
		buf2=(char*)g_malloc(sizeof(char)*(strlen(buf1)+1));
		strcpy(buf2,buf1);
		FONT_families=g_list_prepend (FONT_families,buf2);
	}
  }
  
  if (FONT_families) FONT_families=g_list_sort(FONT_families,(GCompareFunc)strcasecmp);
  
  g_free(_families);
  g_object_unref(G_OBJECT(ct));
}

void gFont::exit()
{
	GList *iter;
	
	iter=FONT_families;
	
	if (iter)
	{
		iter=g_list_first(iter);
		while (iter)
		{
			g_free(iter->data);
			iter=iter->next;
		}
	}
	
	if (FONT_families) g_list_free(FONT_families);
	
	//if (_nfont)
  //  fprintf(stderr, "WARNING: %d gFont objects were not freed.\n", _nfont);
}

int gFont::count()
{
	if (!FONT_families) gFont::init();
	return FONT_n_families;
}

const char *gFont::familyItem(int pos)
{
	if (!FONT_families) gFont::init();
	if ( (pos<0) || (pos>=FONT_n_families) ) return NULL; 
	
	return (const char*)g_list_nth (FONT_families,pos)->data;
}

#if 0
void gFont::updateWidget()
{
	if (!wid) return;

	PangoFontDescription *desc=pango_context_get_font_description(ct);
	gtk_widget_modify_font(wid,desc);
	
	if (G_OBJECT_TYPE(wid)==GTK_TYPE_LABEL)
	{
		PangoAttrList *pal=pango_attr_list_new();
		if (strike)
		{
			PangoAttribute *pa=pango_attr_strikethrough_new(true);
			pa->start_index = 0;
			pa->end_index = g_utf8_strlen(gtk_label_get_text(GTK_LABEL(wid)), -1); 
			pango_attr_list_insert(pal, pa);
		}
		if (uline)
		{
			PangoAttribute *pa=pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
			pa->start_index = 0;
			pa->end_index = g_utf8_strlen(gtk_label_get_text(GTK_LABEL(wid)), -1); 
			pango_attr_list_insert(pal, pa);
		}
		gtk_label_set_attributes(GTK_LABEL(wid), pal);
		pango_attr_list_unref(pal);
	}

}
#endif

void gFont::reset()
{
	strike = false;
	uline = false;

	_bold_set = false;
	_italic_set = false;
	_name_set = false;
	_size_set = false;
	_strikeout_set = false;
	_underline_set = false;
}

void gFont::setAll(bool v)
{
	_bold_set = v;
	_italic_set = v;
	_name_set = v;
	_size_set = v;
	_strikeout_set = v;
	_underline_set = v;
}

void gFont::setAllFrom(gFont *font)
{
	if (!font)
		setAll(false);
	else
	{
		_bold_set = font->_bold_set;
		_italic_set = font->_italic_set;
		_name_set = font->_name_set;
		_size_set = font->_size_set;
		_strikeout_set = font->_strikeout_set;
		_underline_set = font->_underline_set;
	}
}

void gFont::realize()
{
  ct = NULL;
	_height = 0;

	reset();
	
	_nfont++;  
}

void gFont::initFlags()
{
	gFont *comp = new gFont();
	
	_bold_set = comp->bold() != bold();
	_italic_set = comp->italic() != italic();
	_name_set = strcmp(comp->name(), name()) != 0;
	_size_set = comp->size() != size();
	_strikeout_set = comp->strikeout() != strikeout();
	_underline_set = comp->underline() != underline();
}

gFont::gFont() : gShare()
{
#ifdef GTK3
	char *font;
  g_object_get(gtk_settings_get_default(), "gtk-font-name", &font, (char *)NULL);
	realize();
	ct = gdk_pango_context_get();
	pango_context_set_font_description(ct, pango_font_description_from_string(font));
	g_free(font);
#else
	GtkStyle *sty = gtk_widget_get_default_style();
	realize();
	ct = gdk_pango_context_get();
	pango_context_set_font_description(ct, sty->font_desc);
#endif
}

gFont::gFont(GtkWidget *wid) : gShare()
{
	PangoAttrList *lst;
	PangoAttrIterator* iter;
	
	realize();
	ct = gtk_widget_create_pango_context(wid);
	g_object_ref(ct);
	
	if (G_OBJECT_TYPE(wid)==GTK_TYPE_LABEL)
	{
		lst=gtk_label_get_attributes(GTK_LABEL(wid));
		if (lst)
		{
			iter=pango_attr_list_get_iterator(lst);
			if (pango_attr_iterator_get(iter,PANGO_ATTR_STRIKETHROUGH)) strike=true;
			if (pango_attr_iterator_get(iter,PANGO_ATTR_UNDERLINE)) uline=true;
			pango_attr_iterator_destroy(iter);
		}
	} 
	
	initFlags();
}

gFont::gFont(PangoFontDescription *fd) : gShare()
{
	realize();
	ct = gdk_pango_context_get();
	pango_context_set_font_description(ct, fd);
	initFlags();
}

gFont::gFont(const char *name) : gShare()
{
	GtkStyle *sty = gtk_widget_get_default_style();
	
	realize();
	ct = gdk_pango_context_get();
	pango_context_set_font_description(ct,sty->font_desc);

	set_font_from_string(this, name);
}

void gFont::copyTo(gFont *dst)
{
	dst->reset();
	if (_name_set) dst->setName(name());
	if (_size_set) dst->setSize(size());
	if (_bold_set) dst->setBold(bold());
	if (_italic_set) dst->setItalic(italic());
	if (_underline_set) dst->setUnderline(underline());
	if (_strikeout_set) dst->setStrikeout(strikeout());
}

void gFont::mergeFrom(gFont *src)
{
	if (!_name_set && src->_name_set) setName(src->name());
	if (!_size_set && src->_size_set) setSize(src->size());
	if (!_bold_set && src->_bold_set) setBold(src->bold());
	if (!_italic_set && src->_italic_set) setItalic(src->italic());
	if (!_underline_set && src->_underline_set) setUnderline(src->underline());
	if (!_strikeout_set && src->_strikeout_set) setStrikeout(src->strikeout());
}

gFont *gFont::copy()
{
	gFont *f = new gFont();
	copyTo(f);
	return f;
}

gFont::~gFont()
{
	g_object_unref(ct);
	_nfont--;
}


int gFont::ascent()
{
	PangoFontDescription *desc=pango_context_get_font_description(ct);
	PangoFontMetrics *metric=pango_context_get_metrics(ct,desc,NULL);
	
	//fprintf(stderr, "ascent: %d\n", pango_font_metrics_get_ascent(metric));
	return gt_pango_to_pixel(pango_font_metrics_get_ascent(metric));
}

float gFont::ascentF()
{
	PangoFontDescription *desc=pango_context_get_font_description(ct);
	PangoFontMetrics *metric=pango_context_get_metrics(ct,desc,NULL);
	
	return (float)pango_font_metrics_get_ascent(metric) / PANGO_SCALE;
}

int gFont::descent()
{
	PangoFontDescription *desc=pango_context_get_font_description(ct);
	PangoFontMetrics *metric=pango_context_get_metrics(ct,desc,NULL);
	
	//fprintf(stderr, "descent: %d\n", pango_font_metrics_get_descent(metric));
	return gt_pango_to_pixel(pango_font_metrics_get_descent(metric));
}

bool gFont::bold()
{
	PangoFontDescription *desc=pango_context_get_font_description(ct);
	PangoWeight w;
		
	w=pango_font_description_get_weight(desc);
	if (w>PANGO_WEIGHT_NORMAL) return true;
	return false;
}

void gFont::setBold(bool vl)
{
	PangoFontDescription *desc=pango_context_get_font_description(ct);

	if (vl)
		pango_font_description_set_weight(desc,PANGO_WEIGHT_BOLD);
	else
		pango_font_description_set_weight(desc,PANGO_WEIGHT_NORMAL);
	
	_bold_set = true;
}

bool gFont::italic()
{
	PangoFontDescription *desc=pango_context_get_font_description(ct);

	return pango_font_description_get_style(desc) !=PANGO_STYLE_NORMAL; 
}

void gFont::setItalic(bool vl)
{
	PangoFontDescription *desc=pango_context_get_font_description(ct);

	if (vl)
		pango_font_description_set_style(desc,PANGO_STYLE_ITALIC);
	else
		pango_font_description_set_style(desc,PANGO_STYLE_NORMAL);
		
	_italic_set = true;
}

char* gFont::name()
{
	PangoFontDescription *desc=pango_context_get_font_description(ct);

	return (char *)pango_font_description_get_family (desc);	
}

void gFont::setName(char *nm)
{
	PangoFontDescription *desc=pango_context_get_font_description(ct);
	
	pango_font_description_set_family (desc,nm);
	
	_name_set = true;
	_height = 0;
}

double gFont::size()
{
	double size;	

	PangoFontDescription *desc=pango_context_get_font_description(ct);

	size=pango_font_description_get_size(desc);
	return size / (double)PANGO_SCALE;
}

int gFont::grade()
{
	double desktop = gDesktop::font()->size();
	return SIZE_TO_GRADE(size(), desktop);
}

void gFont::setSize(double sz)
{
	PangoFontDescription *desc=pango_context_get_font_description(ct);
	
	pango_font_description_set_size (desc,(int)(sz*PANGO_SCALE+0.5));
	
	_size_set = true;
	_height = 0;
}

void gFont::setGrade(int grade)
{
	double desktop = gDesktop::font()->size();
	
	if (grade < -4 || grade > 24)
		return;
	
	setSize(GRADE_TO_SIZE(grade, desktop));
}

const char *gFont::toString()
{
	GString *desc = g_string_new(name());
	char *ret;
	
	g_string_append_printf(desc, ",%g", (double)((int)(size() * 10 + 0.5)) / 10);
	if (bold())
		g_string_append(desc, ",Bold");
	if (italic())
		g_string_append(desc, ",Italic");
	if (underline())
		g_string_append(desc, ",Underline");
	if (strikeout())
		g_string_append(desc, ",Strikeout");

	ret = g_string_free(desc, false);
	gt_free_later(ret);
	return ret;
}

const char *gFont::toFullString()
{
	GString *desc = g_string_new("");
	char *ret;
	
	g_string_append_printf(desc, "[ ");

	if (_name_set)
		g_string_append_printf(desc, "%s ", name());

	if (_size_set)
		g_string_append_printf(desc, "%g ", (double)((int)(size() * 10 + 0.5)) / 10);
	
	if (_bold_set)
		g_string_append_printf(desc, "%s ", bold() ? "Bold" : "NotBold");
	if (_italic_set)
		g_string_append_printf(desc, "%s ", italic() ? "Italic" : "NotItalic");
	if (_underline_set)
		g_string_append_printf(desc, "%s ", underline() ? "Underline" : "NotUnderline");
	if (_strikeout_set)
		g_string_append_printf(desc, "%s ", strikeout() ? "Strikeout" : "NotStrikeout");

	g_string_append_printf(desc, "]");
	
	ret = g_string_free(desc, false);
	gt_free_later(ret);
	return ret;
}

int gFont::width(const char *text, int len)
{
	PangoLayout *ly;
	int w;
	
	if (!text || !*text) return 0;
	
	ly=pango_layout_new(ct);
	pango_layout_set_text(ly,text,len);
	pango_layout_get_size(ly,&w,NULL);
	g_object_unref(G_OBJECT(ly));
	
	return gt_pango_to_pixel(w);
}

int gFont::height(const char *text, int len)
{
	PangoLayout *ly;
	int h;
	
	if (len == 0 || !text || !*text) text = " ";

	ly=pango_layout_new(ct);
	pango_layout_set_text(ly,text,len);
	pango_layout_get_size(ly,NULL,&h);
	g_object_unref(G_OBJECT(ly));

	return gt_pango_to_pixel(h);
}

int gFont::height()
{
	if (!_height)
		_height = height(" ", 1);
	return _height;
}

bool gFont::scalable()
{
	bool ret=false;
	
	PangoFontDescription *desc=pango_context_get_font_description(ct);
	//PangoFontDescription *tmp;
	const char* name=pango_font_description_get_family(desc);
	PangoFontFamily **families;
	PangoFontFace **faces;
	int *sizes;
	int n_families;
	int n_faces;
	int n_sizes;
	//int b2;
	const char *buf;
	
	if (!name) return false;
	
	pango_context_list_families(ct,&families,&n_families);
	
	if (!families) return false;
	
	for (int bucle=0;bucle<n_families;bucle++)
	{
		buf=pango_font_family_get_name(families[bucle]);
		if (!strcmp(buf,name))
		{
			pango_font_family_list_faces(families[bucle],&faces,&n_faces);
			if (faces)
			{
				pango_font_face_list_sizes(faces[0],&sizes,&n_sizes);
				if (sizes) 
					g_free(sizes);
				else
					ret=true;
				g_free(faces);
				g_free(families);
				return ret;
			}
			else
			{
				g_free(families);
				return false;
			}
		}
		
	}
	
	g_free(families);
	return false;
}

bool gFont::fixed()
{
	bool ret=false;
	
	PangoFontDescription *desc=pango_context_get_font_description(ct);
	const char* name=pango_font_description_get_family(desc);
	PangoFontFamily **families;
	int n_families;
	const char *buf;
	
	if (!name) return false;
	
	pango_context_list_families(ct,&families,&n_families);
	
	if (!families) return false;
	
	for (int bucle=0;bucle<n_families;bucle++)
	{
		buf=pango_font_family_get_name(families[bucle]);
		if (!strcmp(buf,name))
		{
			ret=pango_font_family_is_monospace (families[bucle]);
			g_free(families);
			return ret;
		}
		
	}
	
	g_free(families);
	return false;
}

char** gFont::styles()
{
	return 0;
}

int gFont::resolution()
{
	return 0;
}


bool gFont::strikeout()
{
	return strike;
}

bool gFont::underline()
{
	return uline;
}


void gFont::setResolution(int vl)
{
}

void gFont::setStrikeout(bool vl)
{
	//stub("setStrikeOut(): partially working");
	strike=vl;
	_strikeout_set = true;
	//updateWidget();
}

void gFont::setUnderline(bool vl)
{
	//stub("setUnderline(): partially working");
	uline=vl;
	_underline_set = true;
	//updateWidget();
}


bool gFont::isAllSet()
{
	return 
		_bold_set
		&& _italic_set
		&& _name_set
		&& _size_set
		&& _strikeout_set
		&& _underline_set;
}

void gFont::richTextSize(const char *txt, int len, float sw, float *w, float *h)
{
	PangoLayout *ly;
	int tw = 0, th = 0;
	char *html;
	
	if (txt && len)
	{
		ly = pango_layout_new(ct);
		html = gt_html_to_pango_string(txt, len, false);
		pango_layout_set_wrap(ly, PANGO_WRAP_WORD_CHAR);
		pango_layout_set_markup(ly, html, -1);	
		if (sw > 0)
			pango_layout_set_width(ly, sw * PANGO_SCALE);
		pango_layout_get_size(ly, &tw, &th);
		g_free(html);
	}
	
	if (w) *w = (float)tw / PANGO_SCALE;
	if (h) *h = (float)th / PANGO_SCALE;
}
