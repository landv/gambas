/***************************************************************************

  gfont.cpp

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
#include "font-parser.h"
#include "gdesktop.h"
#include "gb.form.font.h"

#include <math.h>

static void set_font_from_string(gFont *font, char *str)
{
  gchar **tokens, **p;
  gchar *elt;
  int grade;
  double size;
  
  font->setBold(false);
  font->setItalic(false);
  font->setUnderline(false);
  font->setStrikeOut(false);
  font->setName(gDesktop::font()->name());
  font->setSize(gDesktop::font()->size());
  
  if (!str || !*str)
  	return;
  	
	tokens = g_strsplit(str, ",", 0);
  
  p = tokens;
  for(p = tokens; *p; p++)
  {
  	elt = *p;
  	
  	if (!strcasecmp(elt, "bold"))
  		font->setBold(true);
		else if (!strcasecmp(elt, "italic"))
  		font->setItalic(true);
		else if (!strcasecmp(elt, "underline"))
  		font->setUnderline(true);
		else if (!strcasecmp(elt, "strikeout"))
  		font->setStrikeOut(true);
		else if (elt[0] == '+' || elt[0] == '-' || elt[0] == '0')
		{
			grade = atoi(elt);
			if (grade || elt[0] == '0')
				font->setGrade(grade);
		}
		else
		{
			size = atof(elt);
			if (size != 0.0)
				font->setSize(size);
			else
				font->setName(elt);
		}
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

long gFont::count()
{
	if (!FONT_families) gFont::init();
	return FONT_n_families;
}

const char *gFont::familyItem(long pos)
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

void gFont::realize()
{
  gShare::gShare();
	strike=false;
	uline=false;
	_nfont++;  
	//fprintf(stderr, "new gFont() %p\n", this);
}

gFont::gFont(GtkWidget *wid)
{
	PangoAttrList *lst;
	PangoAttrIterator* iter;
	
	realize();
	ct=gtk_widget_create_pango_context(wid);
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
	
}

gFont::gFont()
{
	GtkStyle *sty=gtk_widget_get_default_style();
	
  realize();
	ct=gdk_pango_context_get();
	pango_context_set_font_description(ct,sty->font_desc);
}

gFont::gFont(PangoFontDescription *fd)
{
	//GtkStyle *sty=gtk_widget_get_default_style();
	
	realize();
	ct=gdk_pango_context_get();
	pango_context_set_font_description(ct, fd);
}

gFont::gFont(char *name)
{
	GtkStyle *sty=gtk_widget_get_default_style();
	
	realize();
	ct=gdk_pango_context_get();
	pango_context_set_font_description(ct,sty->font_desc);

	set_font_from_string(this, name);
}

gFont::~gFont()
{
	g_object_unref(ct);
	_nfont--;
  //fprintf(stderr, "delete gFont() %p\n", this);
}


long gFont::ascent()
{
	PangoFontDescription *desc=pango_context_get_font_description(ct);
	PangoFontMetrics *metric=pango_context_get_metrics(ct,desc,NULL);
	long mt;
	
	mt=pango_font_metrics_get_ascent(metric)/PANGO_SCALE;
	
	return mt;
}

long gFont::descent()
{
	PangoFontDescription *desc=pango_context_get_font_description(ct);
	PangoFontMetrics *metric=pango_context_get_metrics(ct,desc,NULL);
	long mt;
	
	mt=pango_font_metrics_get_descent(metric)/PANGO_SCALE;
	
	return mt;
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
		
	//updateWidget();
}

bool gFont::italic()
{
	PangoFontDescription *desc=pango_context_get_font_description(ct);

	if (pango_font_description_get_style(desc)==PANGO_STYLE_NORMAL) return false;
	return true;
	
	
}

void gFont::setItalic(bool vl)
{
	PangoFontDescription *desc=pango_context_get_font_description(ct);

	if (vl)
		pango_font_description_set_style(desc,PANGO_STYLE_ITALIC);
	else
		pango_font_description_set_style(desc,PANGO_STYLE_NORMAL);
		
	//updateWidget();
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
	
	//updateWidget();
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
	
	pango_font_description_set_size (desc,(int)(sz*PANGO_SCALE));
	
	//updateWidget();
}

void gFont::setGrade(int grade)
{
	double desktop = gDesktop::font()->size();
	
	if (grade < -4 || grade > 16)
		return;
	
	setSize(GRADE_TO_SIZE(grade, desktop));
}

const char *gFont::toString()
{
	//PangoFontDescription *desc=pango_context_get_font_description(ct);
	//return pango_font_description_to_string(desc);
	
	GString *desc = g_string_new(name());
	char *ret;
	
	g_string_append_printf(desc, ",%g", (double)((int)(size() * 10 + 0.5)) / 10);
	if (bold())
		g_string_append(desc, ",Bold");
	if (italic())
		g_string_append(desc, ",Italic");
	if (underline())
		g_string_append(desc, ",Underline");
	if (strikeOut())
		g_string_append(desc, ",StrikeOut");

	ret = g_string_free(desc, false);
	gt_free_later(ret);
	return ret;
}

int gFont::width(const char *text, int len)
{
	PangoLayout *ly;
	int w;
	
	if (!text) return 0;
	if (!strlen(text)) return 0;
	
	ly=pango_layout_new(ct);
	pango_layout_set_text(ly,text,len);
	pango_layout_get_size(ly,&w,NULL);
	g_object_unref(G_OBJECT(ly));
	
	return w/PANGO_SCALE;
}

int gFont::height(const char *text, int len)
{
	PangoLayout *ly;
	int h;
	
	if (!text || !*text) return 0;
	
	ly=pango_layout_new(ct);
	pango_layout_set_text(ly,text,len);
	pango_layout_get_size(ly,NULL,&h);
	g_object_unref(G_OBJECT(ly));
	
	return h/PANGO_SCALE;
}

int gFont::height()
{
	return height(" ", 1);
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

long gFont::resolution()
{
        return 0;
}


bool gFont::strikeOut()
{
	return strike;
}

bool gFont::underline()
{
	return uline;
}


void gFont::setResolution(long vl)
{
}

void gFont::setStrikeOut(bool vl)
{
	//stub("setStrikeOut(): partially working");
	strike=vl;
	//updateWidget();
}

void gFont::setUnderline(bool vl)
{
	//stub("setUnderline(): partially working");
	uline=vl;
	//updateWidget();
}



